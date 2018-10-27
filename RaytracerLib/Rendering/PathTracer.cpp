#include "PCH.h"
#include "PathTracer.h"
#include "Context.h"
#include "PathDebugging.h"
#include "Scene/Scene.h"
#include "Scene/Light.h"
#include "Scene/SceneObject_Light.h"
#include "Material/Material.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;

RT_FORCE_INLINE static constexpr Float Mis(const Float samplePdf)
{
    return samplePdf;
}

RT_FORCE_INLINE static constexpr Float CombineMis(const Float samplePdf, const Float otherPdf)
{
    return Mis(samplePdf) / (Mis(samplePdf) + Mis(otherPdf));
}

RT_FORCE_INLINE static constexpr Float PdfWtoA(const Float aPdfW, const Float aDist, const Float aCosThere)
{
    return aPdfW * Abs(aCosThere) / Sqr(aDist);
}

RT_FORCE_INLINE static constexpr Float PdfAtoW(const Float aPdfA, const Float aDist, const Float aCosThere)
{
    return aPdfA * Sqr(aDist) / Abs(aCosThere);
}

PathTracer::PathTracer(const Scene& scene)
    : IRenderer(scene)
{
}

const Color PathTracer::SampleLight(const ILight* light, const Ray& ray, const ShadingData& shadingData, RenderingContext& context) const
{
    Vector4 dirToLight;

    // calculate light contribution
    float distanceToLight;
    float lightDirectPdfW;
    Color radiance = light->Illuminate(shadingData.position, context, dirToLight, distanceToLight, lightDirectPdfW);
    if (radiance.AlmostZero())
    {
        return Color();
    }

    // calculate BSDF contribution
    float bsdfPdfW;
    const Color factor = shadingData.material->Evaluate(context.wavelength, shadingData, -ray.dir, -dirToLight, &bsdfPdfW);
    if (factor.AlmostZero())
    {
        return Color();
    }

    // cast shadow ray
    {
        HitPoint hitPoint;
        hitPoint.distance = distanceToLight;

        Ray shadowRay(shadingData.position, dirToLight);
        shadowRay.origin += shadowRay.dir * 0.001f;

        if (mScene.Traverse_Shadow_Single({ shadowRay, hitPoint, context }))
        {
            // shadow ray missed the light - light is occluded
            return Color();
        }
    }

    float weight = 1.0f;
    if (!light->IsDelta())
    {
        // TODO this should be based on material color
        const float continuationProbability = 1.0f;

        bsdfPdfW *= continuationProbability;
        weight = CombineMis(lightDirectPdfW, bsdfPdfW);
    }

    //const float NdotL = Abs(Vector4::Dot3(dirToLight, shadingData.normal));
    return (radiance * factor) * (weight / lightDirectPdfW);
}

const Color PathTracer::SampleLights(const Ray& ray, const ShadingData& shadingData, RenderingContext& context) const
{
    Color accumulatedColor;

    // TODO check only one (or few) lights per sample instead all of them
    // TODO check only nearest lights
    for (const LightPtr& light : mScene.GetLights())
    {
        accumulatedColor += SampleLight(light.get(), ray, shadingData, context);
    }

    // TODO background light should be on mScene.GetLights() list
    if (const BackgroundLight* light = mScene.GetBackgroundLight())
    {
        accumulatedColor += SampleLight(light, ray, shadingData, context);
    }

    return accumulatedColor;
}

const Color PathTracer::TraceRay_Single(const Ray& primaryRay, RenderingContext& context) const
{
    Uint32 depth = 0;
    HitPoint hitPoint;
    Ray ray = primaryRay;

    ShadingData shadingData;
    Vector4 incomingDirWorldSpace;

    Color resultColor;
    Color throughput = Color::One();

    PathTerminationReason pathTerminationReason = PathTerminationReason::None;

    bool lastSpecular = true;
    float lastPdfW = 1.0f;

    for (;;)
    {
        hitPoint.distance = FLT_MAX;
        context.localCounters.Reset();
        mScene.Traverse_Single({ ray, hitPoint, context });
        context.counters.Append(context.localCounters);

        // ray missed - return background color
        if (hitPoint.distance == FLT_MAX)
        {
            if (const BackgroundLight* light = mScene.GetBackgroundLight())
            {
                float directPdfW;
                const Color lightContribution = light->GetRadiance(context, ray.dir, Vector4(), &directPdfW);

                if (!lightContribution.AlmostZero())
                {
                    float misWeight = 1.0f;
                    if (mSampleLights && depth > 0 && !lastSpecular)
                    {
                        misWeight = CombineMis(lastPdfW, directPdfW);
                    }

                    resultColor += throughput * lightContribution * misWeight;
                }
            }

            pathTerminationReason = PathTerminationReason::HitBackground;
            break;
        }

        mScene.ExtractShadingData(ray.origin, ray.dir, hitPoint, context.time, shadingData);

        // TODO evaluate material parameters

        // we hit a light directly
        if (hitPoint.triangleId == RT_LIGHT_OBJECT)
        {
            // HACK
            const LightSceneObject* lightSceneObj = static_cast<const LightSceneObject*>(mScene.GetObjects()[hitPoint.objectId].get());
            const ILight& light = lightSceneObj->GetLight();

            const Vector4 hitPos = ray.GetAtDistance(hitPoint.distance);

            float directPdfA;
            const Color lightContribution = light.GetRadiance(context, ray.dir, hitPos, &directPdfA);
 
            if (!lightContribution.AlmostZero())
            {
                float misWeight = 1.0f;
                if (mSampleLights && depth > 0 && !lastSpecular)
                {
                    const float cosTheta = Abs(Vector4::Dot3(ray.dir, shadingData.normal));
                    const float directPdfW = PdfAtoW(directPdfA, hitPoint.distance, cosTheta);
                    misWeight = CombineMis(lastPdfW, directPdfW);
                }

                resultColor += throughput * lightContribution * misWeight;
            }

            pathTerminationReason = PathTerminationReason::HitLight;
            break;
        }

        // accumulate emission color
        const Color emissionColor = Color::SampleRGB(context.wavelength, shadingData.material->emission.Evaluate(shadingData.texCoord));
        resultColor += throughput * emissionColor;

        if (mSampleLights)
        {
            // sample lights directly (a.k.a. next event estimation)
            resultColor += throughput * SampleLights(ray, shadingData, context);
        }

        // check if the ray depth won't be exeeded in the next iteration
        if (depth >= context.params->maxRayDepth)
        {
            pathTerminationReason = PathTerminationReason::Depth;
            break;
        }

        // Russian roulette algorithm
        if (depth >= context.params->minRussianRouletteDepth)
        {
            const Float threshold = throughput.Max();
            if (context.randomGenerator.GetFloat() > threshold)
            {
                pathTerminationReason = PathTerminationReason::RussianRoulette;
                break;
            }
            throughput *= 1.0f / threshold;
        }

        // sample BSDF
        float pdf = 0.0f;
        BSDF::EventType sampledEvent = BSDF::NullEvent;
        throughput *= shadingData.material->Sample(context.wavelength, -ray.dir, incomingDirWorldSpace, shadingData, context.randomGenerator, pdf, sampledEvent);

        // ray is not visible anymore
        if (throughput.AlmostZero())
        {
            pathTerminationReason = PathTerminationReason::Throughput;
            break;
        }

        RT_ASSERT(sampledEvent != BSDF::NullEvent);
        RT_ASSERT(pdf > 0.0f);

        lastSpecular = (sampledEvent & BSDF::SpecularEvent) != 0;
        lastPdfW = pdf;
        throughput *= 1.0f / pdf;

        // TODO check for NaNs

        if (context.pathDebugData)
        {
            PathDebugData::HitPointData data;
            data.rayOrigin = ray.origin;
            data.rayDir = ray.dir;
            data.hitPoint = hitPoint;
            data.shadingData = shadingData;
            data.throughput = throughput;
            context.pathDebugData->data.push_back(data);
        }

        // generate secondary ray
        ray = Ray(shadingData.position, incomingDirWorldSpace);
        ray.origin += ray.dir * 0.001f;

        depth++;
    }

    if (context.pathDebugData)
    {
        PathDebugData::HitPointData data;
        data.rayOrigin = ray.origin;
        data.rayDir = ray.dir;
        data.hitPoint = hitPoint;
        data.shadingData = shadingData;
        data.throughput = throughput;
        context.pathDebugData->data.push_back(data);
        context.pathDebugData->terminationReason = pathTerminationReason;
    }

    return resultColor;
}

} // namespace rt
