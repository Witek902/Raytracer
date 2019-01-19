#include "PCH.h"
#include "PathTracer.h"
#include "Context.h"
#include "PathDebugging.h"
#include "Scene/Scene.h"
#include "Scene/Light/Light.h"
#include "Scene/Light/BackgroundLight.h"
#include "Scene/Object/SceneObject_Light.h"
#include "Material/Material.h"
#include "Traversal/TraversalContext.h"

// enable MIS weights coloring:
// green - BSDF sampling
// red   - light sampling
// #define RT_VISUALIZE_MIS_CONTRIBUTIONS

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

RT_FORCE_INLINE static constexpr Float PdfAtoW(const Float pdfA, const Float distance, const Float cosThere)
{
    return pdfA * Sqr(distance) / Abs(cosThere);
}

PathTracer::PathTracer(const Scene& scene)
    : IRenderer(scene)
    , mSampleLights(true)
{
}

const Color PathTracer::SampleLight(const ILight* light, const ShadingData& shadingData, RenderingContext& context) const
{
    ILight::IlluminateParam illuminateParam = { shadingData, context };

    // calculate light contribution
    Color radiance = light->Illuminate(illuminateParam);
    if (radiance.AlmostZero())
    {
        return Color::Zero();
    }

    RT_ASSERT(radiance.IsValid());
    RT_ASSERT(IsValid(illuminateParam.outDirectPdfW));
    RT_ASSERT(illuminateParam.outDirectPdfW >= 0.0f);
    RT_ASSERT(IsValid(illuminateParam.outDistance));
    RT_ASSERT(illuminateParam.outDirectPdfW >= 0.0f);

    // calculate BSDF contribution
    float bsdfPdfW;
    const Color factor = shadingData.material->Evaluate(context.wavelength, shadingData, -illuminateParam.outDirectionToLight, &bsdfPdfW);
    RT_ASSERT(factor.IsValid());

    if (factor.AlmostZero())
    {
        return Color::Zero();
    }

    RT_ASSERT(bsdfPdfW > 0.0f && IsValid(bsdfPdfW));

    // cast shadow ray
    {
        HitPoint hitPoint;
        hitPoint.distance = illuminateParam.outDistance;

        Ray shadowRay(shadingData.position, illuminateParam.outDirectionToLight);
        shadowRay.origin += shadowRay.dir * 0.001f;

        if (mScene.Traverse_Shadow_Single({ shadowRay, hitPoint, context }))
        {
            // shadow ray missed the light - light is occluded
            return Color::Zero();
        }
    }

    float weight = 1.0f;
    if (!light->IsDelta())
    {
        // TODO this should be based on material color
        const float continuationProbability = 1.0f;

        bsdfPdfW *= continuationProbability;
        weight = CombineMis(illuminateParam.outDirectPdfW, bsdfPdfW);
    }

    return (radiance * factor) * (weight / illuminateParam.outDirectPdfW);
}

const Color PathTracer::SampleLights(const ShadingData& shadingData, RenderingContext& context) const
{
    Color accumulatedColor = Color::Zero();

    // TODO check only one (or few) lights per sample instead all of them
    // TODO check only nearest lights
    for (const LightPtr& light : mScene.GetLights())
    {
        accumulatedColor += SampleLight(light.get(), shadingData, context);
    }

    // TODO background light should be on mScene.GetLights() list
    if (const BackgroundLight* light = mScene.GetBackgroundLight())
    {
        accumulatedColor += SampleLight(light, shadingData, context);
    }

#ifdef RT_VISUALIZE_MIS_CONTRIBUTIONS
    accumulatedColor *= Color::SampleRGB(context.wavelength, Vector4(1.0f, 0.0f, 0.0f, 0.0f));
#endif // RT_VISUALIZE_MIS_CONTRIBUTIONS

    return accumulatedColor;
}

const Color PathTracer::TraceRay_Single(const Ray& primaryRay, RenderingContext& context) const
{
    Uint32 depth = 0;
    HitPoint hitPoint;
    Ray ray = primaryRay;

    ShadingData shadingData;

    Color resultColor = Color::Zero();
    Color throughput = Color::One();

    PathTerminationReason pathTerminationReason = PathTerminationReason::None;

    bool lastSpecular = true;
    float lastPdfW = 1.0f;
    BSDF::EventType lastSampledBsdfEvent = BSDF::NullEvent;

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
                Color lightContribution = light->GetRadiance(context, ray.dir, Vector4::Zero(), &directPdfW);
                RT_ASSERT(lightContribution.IsValid());

                if (!lightContribution.AlmostZero())
                {
                    RT_ASSERT(directPdfW > 0.0f && IsValid(directPdfW));

                    float misWeight = 1.0f;
                    if (mSampleLights && depth > 0 && !lastSpecular)
                    {
                        misWeight = CombineMis(lastPdfW, directPdfW);
                    }

#ifdef RT_VISUALIZE_MIS_CONTRIBUTIONS
                    lightContribution *= Color::SampleRGB(context.wavelength, Vector4(0.0f, 1.0f, 0.0f, 0.0f));
#endif // RT_VISUALIZE_MIS_CONTRIBUTIONS

                    resultColor += throughput * lightContribution * misWeight;
                }
            }

            pathTerminationReason = PathTerminationReason::HitBackground;
            break;
        }

        mScene.ExtractShadingData(ray.origin, ray.dir, hitPoint, context.time, shadingData);

        // we hit a light directly
        if (hitPoint.subObjectId == RT_LIGHT_OBJECT)
        {
            // HACK
            const LightSceneObject* lightSceneObj = static_cast<const LightSceneObject*>(mScene.GetObjects()[hitPoint.objectId].get());
            const ILight& light = lightSceneObj->GetLight();

            const Vector4 hitPos = ray.GetAtDistance(hitPoint.distance);

            float directPdfA;
            Color lightContribution = light.GetRadiance(context, ray.dir, hitPos, &directPdfA);
            RT_ASSERT(lightContribution.IsValid());

            if (!lightContribution.AlmostZero())
            {
                RT_ASSERT(directPdfA > 0.0f && IsValid(directPdfA));

                float misWeight = 1.0f;
                if (mSampleLights && depth > 0 && !lastSpecular)
                {
                    const float cosTheta = Vector4::Dot3(-ray.dir, shadingData.normal);
                    const float directPdfW = PdfAtoW(directPdfA, hitPoint.distance, cosTheta);
                    misWeight = CombineMis(lastPdfW, directPdfW);
                }

#ifdef RT_VISUALIZE_MIS_CONTRIBUTIONS
                lightContribution *= Color::SampleRGB(context.wavelength, Vector4(0.0f, 1.0f, 0.0f, 0.0f));
#endif // RT_VISUALIZE_MIS_CONTRIBUTIONS

                resultColor += throughput * lightContribution * misWeight;
            }

            pathTerminationReason = PathTerminationReason::HitLight;
            break;
        }

        // fill up structure with shading data
        {
            shadingData.outgoingDirWorldSpace = -ray.dir;
            shadingData.outgoingDirLocalSpace = shadingData.WorldToLocal(shadingData.outgoingDirWorldSpace);

            RT_ASSERT(shadingData.material != nullptr);
            shadingData.material->EvaluateShadingData(context.wavelength, shadingData);
        }

        // accumulate emission color
        const Color emissionColor = Color::SampleRGB(context.wavelength, shadingData.material->emission.Evaluate(shadingData.texCoord));
        RT_ASSERT(emissionColor.IsValid());
        resultColor += throughput * emissionColor;
        RT_ASSERT(resultColor.IsValid());

        if (mSampleLights)
        {
            // sample lights directly (a.k.a. next event estimation)
            resultColor += throughput * SampleLights(shadingData, context);
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
        float pdf;
        Vector4 incomingDirWorldSpace;
        lastSampledBsdfEvent = BSDF::NullEvent;
        const Color bsdfValue = shadingData.material->Sample(context.wavelength, incomingDirWorldSpace, shadingData, context.randomGenerator, pdf, lastSampledBsdfEvent);

        if (lastSampledBsdfEvent == BSDF::NullEvent)
        {
            pathTerminationReason = PathTerminationReason::NoSampledEvent;
            break;
        }

        RT_ASSERT(bsdfValue.IsValid());
        throughput *= bsdfValue;

        // ray is not visible anymore
        if (throughput.AlmostZero())
        {
            pathTerminationReason = PathTerminationReason::Throughput;
            break;
        }

        RT_ASSERT(pdf >= 0.0f);
        lastSpecular = (lastSampledBsdfEvent & BSDF::SpecularEvent) != 0;
        lastPdfW = pdf;

        // TODO check for NaNs

        if (context.pathDebugData)
        {
            PathDebugData::HitPointData data;
            data.rayOrigin = ray.origin;
            data.rayDir = ray.dir;
            data.hitPoint = hitPoint;
            data.shadingData = shadingData;
            data.throughput = throughput;
            data.bsdfEvent = lastSampledBsdfEvent;
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
        data.bsdfEvent = lastSampledBsdfEvent;
        context.pathDebugData->data.push_back(data);
        context.pathDebugData->terminationReason = pathTerminationReason;
    }

    return resultColor;
}

} // namespace rt
