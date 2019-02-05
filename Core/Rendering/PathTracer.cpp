#include "PCH.h"
#include "PathTracer.h"
#include "Context.h"
#include "PathDebugging.h"
#include "Scene/Scene.h"
#include "Scene/Light/Light.h"
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

const RayColor PathTracer::SampleLight(const ILight& light, const ShadingData& shadingData, const PathState& pathState, RenderingContext& context) const
{
    ILight::IlluminateParam illuminateParam = { shadingData, context };

    // calculate light contribution
    RayColor radiance = light.Illuminate(illuminateParam);
    if (radiance.AlmostZero())
    {
        return RayColor::Zero();
    }

    RT_ASSERT(radiance.IsValid());
    RT_ASSERT(IsValid(illuminateParam.outDirectPdfW));
    RT_ASSERT(illuminateParam.outDirectPdfW >= 0.0f);
    RT_ASSERT(IsValid(illuminateParam.outDistance));
    RT_ASSERT(illuminateParam.outDirectPdfW >= 0.0f);

    // calculate BSDF contribution
    float bsdfPdfW;
    const RayColor factor = shadingData.material->Evaluate(context.wavelength, shadingData, -illuminateParam.outDirectionToLight, &bsdfPdfW);
    RT_ASSERT(factor.IsValid());

    if (factor.AlmostZero())
    {
        return RayColor::Zero();
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
            return RayColor::Zero();
        }
    }

    float weight = 1.0f;

    // bypass MIS when this is the last path sample so the energy is not lost
    // TODO this does not include russian roulette
    const bool isLastPathSegment = pathState.depth >= context.params->maxRayDepth;

    if (!light.IsDelta() && !isLastPathSegment)
    {
        // TODO this should be based on material color
        const float continuationProbability = 1.0f;

        bsdfPdfW *= continuationProbability;
        weight = CombineMis(illuminateParam.outDirectPdfW, bsdfPdfW);
    }

    return (radiance * factor) * (weight / illuminateParam.outDirectPdfW);
}

const RayColor PathTracer::SampleLights(const ShadingData& shadingData, const PathState& pathState, RenderingContext& context) const
{
    RayColor accumulatedColor = RayColor::Zero();

    // TODO check only one (or few) lights per sample instead all of them
    // TODO check only nearest lights
    for (const LightPtr& light : mScene.GetLights())
    {
        accumulatedColor += SampleLight(*light, shadingData, pathState, context);
    }

#ifdef RT_VISUALIZE_MIS_CONTRIBUTIONS
    accumulatedColor *= RayColor::SampleRGB(context.wavelength, Vector4(1.0f, 0.0f, 0.0f, 0.0f));
#endif // RT_VISUALIZE_MIS_CONTRIBUTIONS

    return accumulatedColor;
}

const RayColor PathTracer::EvaluateLight(const ILight& light, const math::Ray& ray, Float dist, const PathState& pathState, RenderingContext& context) const
{
    const Vector4 hitPos = ray.GetAtDistance(dist);
    const Vector4 normal = light.GetNormal(hitPos);

    float directPdfA;
    RayColor lightContribution = light.GetRadiance(context, ray.dir, hitPos, &directPdfA);
    RT_ASSERT(lightContribution.IsValid());

    if (lightContribution.AlmostZero())
    {
        return RayColor::Zero();
    }

    RT_ASSERT(directPdfA > 0.0f && IsValid(directPdfA));

    float misWeight = 1.0f;
    if (mSampleLights && pathState.depth > 0 && !pathState.lastSpecular)
    {
        const float cosTheta = Vector4::Dot3(-ray.dir, normal);
        const float directPdfW = PdfAtoW(directPdfA, dist, cosTheta);
        misWeight = CombineMis(pathState.lastPdfW, directPdfW);
    }

#ifdef RT_VISUALIZE_MIS_CONTRIBUTIONS
    lightContribution *= RayColor::Resolve(context.wavelength, Spectrum(Vector4(0.0f, 1.0f, 0.0f, 0.0f)));
#endif // RT_VISUALIZE_MIS_CONTRIBUTIONS

    return lightContribution * misWeight;
}

const RayColor PathTracer::EvaluateGlobalLights(const Ray& ray, const PathState& pathState, RenderingContext& context) const
{
    RayColor result = RayColor::Zero();

    for (const ILight* globalLight : mScene.GetGlobalLights())
    {
        float directPdfW;
        RayColor lightContribution = globalLight->GetRadiance(context, ray.dir, Vector4::Zero(), &directPdfW);
        RT_ASSERT(lightContribution.IsValid());

        if (!lightContribution.AlmostZero())
        {
            RT_ASSERT(directPdfW > 0.0f && IsValid(directPdfW));

            float misWeight = 1.0f;
            if (mSampleLights && pathState.depth > 0 && !pathState.lastSpecular)
            {
                misWeight = CombineMis(pathState.lastPdfW, directPdfW);
            }

#ifdef RT_VISUALIZE_MIS_CONTRIBUTIONS
            lightContribution *= RayColor::Resolve(context.wavelength, Spectrum(Vector4(0.0f, 1.0f, 0.0f, 0.0f)));
#endif // RT_VISUALIZE_MIS_CONTRIBUTIONS

            result += lightContribution * misWeight;
        }
    }

    return result;
}

const RayColor PathTracer::TraceRay_Single(const Ray& primaryRay, RenderingContext& context) const
{
    HitPoint hitPoint;
    Ray ray = primaryRay;

    ShadingData shadingData;

    RayColor resultColor = RayColor::Zero();
    RayColor throughput = RayColor::One();

    PathTerminationReason pathTerminationReason = PathTerminationReason::None;

    PathState pathState;

    for (;;)
    {
        hitPoint.distance = FLT_MAX;
        context.localCounters.Reset();
        mScene.Traverse_Single({ ray, hitPoint, context });
        context.counters.Append(context.localCounters);

        // ray missed - return background light color
        if (hitPoint.distance == FLT_MAX)
        {
            resultColor += throughput * EvaluateGlobalLights(ray, pathState, context);
            pathTerminationReason = PathTerminationReason::HitBackground;
            break;
        }

        // we hit a light directly
        if (hitPoint.subObjectId == RT_LIGHT_OBJECT)
        {
            // HACK
            const LightSceneObject* lightSceneObj = static_cast<const LightSceneObject*>(mScene.GetObjects()[hitPoint.objectId].get());
            
            const ILight& light = lightSceneObj->GetLight();
            resultColor += throughput * EvaluateLight(light, ray, hitPoint.distance, pathState, context);
            pathTerminationReason = PathTerminationReason::HitLight;
            break;
        }

        // fill up structure with shading data
        {
            mScene.ExtractShadingData(ray.origin, ray.dir, hitPoint, context.time, shadingData);

            shadingData.outgoingDirWorldSpace = -ray.dir;
            shadingData.outgoingDirLocalSpace = shadingData.WorldToLocal(shadingData.outgoingDirWorldSpace);

            RT_ASSERT(shadingData.material != nullptr);
            shadingData.material->EvaluateShadingData(context.wavelength, shadingData);
        }

        // accumulate emission color
        const RayColor emissionColor = RayColor::Resolve(context.wavelength, Spectrum(shadingData.material->emission.Evaluate(shadingData.texCoord)));
        RT_ASSERT(emissionColor.IsValid());
        resultColor += throughput * emissionColor;
        RT_ASSERT(resultColor.IsValid());

        if (mSampleLights)
        {
            // sample lights directly (a.k.a. next event estimation)
            resultColor += throughput * SampleLights(shadingData, pathState, context);
        }

        // check if the ray depth won't be exeeded in the next iteration
        if (pathState.depth >= context.params->maxRayDepth)
        {
            pathTerminationReason = PathTerminationReason::Depth;
            break;
        }

        // Russian roulette algorithm
        if (pathState.depth >= context.params->minRussianRouletteDepth)
        {
            Float threshold = throughput.Max();
#ifdef RT_ENABLE_SPECTRAL_RENDERING
            if (context.wavelength.isSingle)
            {
                threshold *= 1.0f / static_cast<Float>(Wavelength::NumComponents);
            }
#endif
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
        pathState.lastSampledBsdfEvent = BSDF::NullEvent;
        const RayColor bsdfValue = shadingData.material->Sample(context.wavelength, incomingDirWorldSpace, shadingData, context.randomGenerator, pdf, pathState.lastSampledBsdfEvent);

        if (pathState.lastSampledBsdfEvent == BSDF::NullEvent)
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
        pathState.lastSpecular = (pathState.lastSampledBsdfEvent & BSDF::SpecularEvent) != 0;
        pathState.lastPdfW = pdf;

        // TODO check for NaNs

        if (context.pathDebugData)
        {
            PathDebugData::HitPointData data;
            data.rayOrigin = ray.origin;
            data.rayDir = ray.dir;
            data.hitPoint = hitPoint;
            data.shadingData = shadingData;
            data.throughput = throughput;
            data.bsdfEvent = pathState.lastSampledBsdfEvent;
            context.pathDebugData->data.push_back(data);
        }

        // generate secondary ray
        ray = Ray(shadingData.position, incomingDirWorldSpace);
        ray.origin += ray.dir * 0.001f;

        pathState.depth++;
    }

    if (context.pathDebugData)
    {
        PathDebugData::HitPointData data;
        data.rayOrigin = ray.origin;
        data.rayDir = ray.dir;
        data.hitPoint = hitPoint;
        data.shadingData = shadingData;
        data.throughput = throughput;
        data.bsdfEvent = pathState.lastSampledBsdfEvent;
        context.pathDebugData->data.push_back(data);
        context.pathDebugData->terminationReason = pathTerminationReason;
    }

    return resultColor;
}

} // namespace rt
