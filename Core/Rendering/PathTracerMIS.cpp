#include "PCH.h"
#include "PathTracerMIS.h"
#include "Context.h"
#include "PathDebugging.h"
#include "Scene/Scene.h"
#include "Scene/Light/Light.h"
#include "Material/Material.h"
#include "Traversal/TraversalContext.h"
#include "Sampling/GenericSampler.h"

namespace rt {

using namespace math;

RT_FORCE_INLINE static constexpr float Mis(const float samplePdf)
{
    return samplePdf;
}

RT_FORCE_INLINE static constexpr float CombineMis(const float samplePdf, const float otherPdf)
{
    return Mis(samplePdf) / (Mis(samplePdf) + Mis(otherPdf));
}

RT_FORCE_INLINE static constexpr float PdfAtoW(const float pdfA, const float distance, const float cosThere)
{
    return pdfA * Sqr(distance) / Abs(cosThere);
}

PathTracerMIS::PathTracerMIS(const Scene& scene)
    : IRenderer(scene)
{
    mLightSamplingWeight = Vector4(1.0f);
    mBSDFSamplingWeight = Vector4(1.0f);
}

const char* PathTracerMIS::GetName() const
{
    return "Path Tracer MIS";
}

const RayColor PathTracerMIS::SampleLight(const ILight& light, const ShadingData& shadingData, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const
{
    const ILight::IlluminateParam illuminateParam =
    {
        shadingData,
        context.wavelength,
        context.sampler->GetFloat2(),
    };

    // calculate light contribution
    ILight::IlluminateResult illuminateResult;
    RayColor radiance = light.Illuminate(illuminateParam, illuminateResult);
    if (radiance.AlmostZero())
    {
        return RayColor::Zero();
    }

    RT_ASSERT(radiance.IsValid());
    RT_ASSERT(IsValid(illuminateResult.directPdfW) && illuminateResult.directPdfW >= 0.0f);
    RT_ASSERT(IsValid(illuminateResult.distance) && illuminateResult.directPdfW >= 0.0f);
    RT_ASSERT(illuminateResult.directionToLight.IsValid());

    // calculate BSDF contribution
    float bsdfPdfW;
    const RayColor factor = shadingData.material->Evaluate(context.wavelength, shadingData, -illuminateResult.directionToLight, &bsdfPdfW);
    RT_ASSERT(factor.IsValid());

    if (factor.AlmostZero())
    {
        return RayColor::Zero();
    }

    RT_ASSERT(bsdfPdfW >= 0.0f && IsValid(bsdfPdfW));

    // cast shadow ray
    {
        HitPoint hitPoint;
        hitPoint.distance = illuminateResult.distance * 0.999f;

        Ray shadowRay(shadingData.frame.GetTranslation(), illuminateResult.directionToLight);
        shadowRay.origin += shadingData.frame[2] * 0.0001f;

        context.counters.numShadowRays++;
        if (mScene.Traverse_Shadow_Single({ shadowRay, hitPoint, context }))
        {
            // shadow ray missed the light - light is occluded
            return RayColor::Zero();
        }
        else
        {
            context.counters.numShadowRaysHit++;
        }
    }

    float weight = 1.0f;

    // bypass MIS when this is the last path sample so the energy is not lost
    // TODO this does not include russian roulette
    const bool isLastPathSegment = pathState.depth >= context.params->maxRayDepth;

    const ILight::Flags lightFlags = light.GetFlags();
    if (!(lightFlags & ILight::Flag_IsDelta) && !isLastPathSegment)
    {
        // TODO this should be based on material color
        const float continuationProbability = 1.0f;

        bsdfPdfW *= continuationProbability;
        weight = CombineMis(illuminateResult.directPdfW * lightPickProbability, bsdfPdfW);
    }

    return (radiance * factor) * (weight / (lightPickProbability * illuminateResult.directPdfW));
}

const RayColor PathTracerMIS::SampleLights(const ShadingData& shadingData, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const
{
    RayColor accumulatedColor = RayColor::Zero();

    const auto& lights = mScene.GetLights();
    if (!lights.Empty())
    {
        switch (context.params->lightSamplingStrategy)
        {
            case LightSamplingStrategy::Single:
            {
                const Uint32 lightIndex = context.sampler->GetFallbackGenerator().GetInt() % lights.Size();
                const LightPtr& light = lights[lightIndex];
                accumulatedColor = SampleLight(*light, shadingData, pathState, context, lightPickProbability);
                break;
            }

            case LightSamplingStrategy::All:
            {
                for (const LightPtr& light : lights)
                {
                    accumulatedColor += SampleLight(*light, shadingData, pathState, context, lightPickProbability);
                }
                break;
            }
        };

        accumulatedColor *= RayColor::Resolve(context.wavelength, Spectrum(mLightSamplingWeight));
    }

    return accumulatedColor;
}

float PathTracerMIS::GetLightPickingProbability(RenderingContext& context) const
{
    switch (context.params->lightSamplingStrategy)
    {
    case LightSamplingStrategy::Single:
        return 1.0f / (float)mScene.GetLights().Size();

    case LightSamplingStrategy::All:
        return 1.0f;

    default:
        RT_FATAL("Invalid light sampling strategy");
    };

    return 0.0f;
}

const RayColor PathTracerMIS::EvaluateLight(const ILight& light, const math::Ray& ray, float dist, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const
{
    const Vector4 hitPos = ray.GetAtDistance(dist);
    const Vector4 normal = light.GetNormal(hitPos);

    float directPdfA;
    RayColor lightContribution = light.GetRadiance(context, ray, hitPos, &directPdfA);
    RT_ASSERT(lightContribution.IsValid());

    if (lightContribution.AlmostZero())
    {
        return RayColor::Zero();
    }

    RT_ASSERT(directPdfA > 0.0f && IsValid(directPdfA));

    float misWeight = 1.0f;
    if (pathState.depth > 0 && !pathState.lastSpecular)
    {
        const float cosTheta = Vector4::Dot3(-ray.dir, normal);
        const float directPdfW = PdfAtoW(directPdfA, dist, cosTheta);
        misWeight = CombineMis(pathState.lastPdfW, directPdfW * lightPickProbability);
    }

    lightContribution *= RayColor::Resolve(context.wavelength, Spectrum(mBSDFSamplingWeight));

    return lightContribution * misWeight;
}

const RayColor PathTracerMIS::EvaluateGlobalLights(const Ray& ray, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const
{
    RayColor result = RayColor::Zero();

    for (const ILight* globalLight : mScene.GetGlobalLights())
    {
        float directPdfW;
        RayColor lightContribution = globalLight->GetRadiance(context, ray, Vector4::Zero(), &directPdfW);
        RT_ASSERT(lightContribution.IsValid());

        if (!lightContribution.AlmostZero())
        {
            RT_ASSERT(directPdfW > 0.0f && IsValid(directPdfW));

            float misWeight = 1.0f;
            if (pathState.depth > 0 && !pathState.lastSpecular)
            {
                misWeight = CombineMis(pathState.lastPdfW, directPdfW * lightPickProbability);
            }

            result.MulAndAccumulate(lightContribution, misWeight);
        }
    }

    result *= RayColor::Resolve(context.wavelength, Spectrum(mBSDFSamplingWeight));

    return result;
}

const RayColor PathTracerMIS::RenderPixel(const math::Ray& primaryRay, const RenderParam&, RenderingContext& context) const
{
    HitPoint hitPoint;
    Ray ray = primaryRay;

    ShadingData shadingData;

    RayColor resultColor = RayColor::Zero();
    RayColor throughput = RayColor::One();

    PathTerminationReason pathTerminationReason = PathTerminationReason::None;

    PathState pathState;

    const float lightPickProbability = GetLightPickingProbability(context);

    for (;;)
    {
        hitPoint.distance = FLT_MAX;
        context.localCounters.Reset();
        mScene.Traverse_Single({ ray, hitPoint, context });
        context.counters.Append(context.localCounters);

        // ray missed - return background light color
        if (hitPoint.distance == FLT_MAX)
        {
            resultColor.MulAndAccumulate(throughput, EvaluateGlobalLights(ray, pathState, context, lightPickProbability));
            pathTerminationReason = PathTerminationReason::HitBackground;
            break;
        }

        // we hit a light directly
        if (hitPoint.subObjectId == RT_LIGHT_OBJECT)
        {
            const ILight& light = mScene.GetLightByObjectId(hitPoint.objectId);
            resultColor.MulAndAccumulate(throughput, EvaluateLight(light, ray, hitPoint.distance, pathState, context, lightPickProbability));
            pathTerminationReason = PathTerminationReason::HitLight;
            break;
        }

        // fill up structure with shading data
        {
            mScene.ExtractShadingData(ray, hitPoint, context.time, shadingData);

            RT_ASSERT(shadingData.material != nullptr);
            shadingData.material->EvaluateShadingData(context.wavelength, shadingData);
        }

        // accumulate emission color
        {
            RayColor emissionColor = RayColor::Resolve(context.wavelength, Spectrum(shadingData.material->emission.Evaluate(shadingData.texCoord)));
            RT_ASSERT(emissionColor.IsValid());

            emissionColor *= RayColor::Resolve(context.wavelength, Spectrum(mBSDFSamplingWeight));

            resultColor.MulAndAccumulate(throughput, emissionColor);
            RT_ASSERT(resultColor.IsValid());
        }

        // sample lights directly (a.k.a. next event estimation)
        resultColor.MulAndAccumulate(throughput, SampleLights(shadingData, pathState, context, lightPickProbability));

        // check if the ray depth won't be exeeded in the next iteration
        if (pathState.depth >= context.params->maxRayDepth)
        {
            pathTerminationReason = PathTerminationReason::Depth;
            break;
        }

        // Russian roulette algorithm
        if (pathState.depth >= context.params->minRussianRouletteDepth)
        {
            const float minColorValue = 0.125f;
            float threshold = minColorValue + (1.0f - minColorValue) * shadingData.materialParams.baseColor.Max();
#ifdef RT_ENABLE_SPECTRAL_RENDERING
            if (context.wavelength.isSingle)
            {
                threshold *= 1.0f / static_cast<float>(Wavelength::NumComponents);
            }
#endif
            if (context.sampler->GetFloat() > threshold)
            {
                pathTerminationReason = PathTerminationReason::RussianRoulette;
                break;
            }
            throughput *= 1.0f / threshold;
            RT_ASSERT(throughput.IsValid());
        }

        // sample BSDF
        float pdf;
        Vector4 incomingDirWorldSpace;
        BSDF::EventType lastSampledBsdfEvent = BSDF::NullEvent;
        const RayColor bsdfValue = shadingData.material->Sample(context.wavelength, incomingDirWorldSpace, shadingData, context.sampler->GetFloat3(), &pdf, &lastSampledBsdfEvent);

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
        pathState.lastSpecular = (lastSampledBsdfEvent & BSDF::SpecularEvent) != 0;
        pathState.lastPdfW = pdf;

        // TODO check for NaNs

#ifndef RT_CONFIGURATION_FINAL
        if (context.pathDebugData)
        {
            PathDebugData::HitPointData data;
            data.rayOrigin = ray.origin;
            data.rayDir = ray.dir;
            data.hitPoint = hitPoint;
            data.shadingData = shadingData;
            data.throughput = throughput;
            data.bsdfEvent = lastSampledBsdfEvent;
            context.pathDebugData->data.PushBack(data);
        }
#endif // RT_CONFIGURATION_FINAL

        // generate secondary ray
        ray = Ray(shadingData.frame.GetTranslation(), incomingDirWorldSpace);
        ray.origin += ray.dir * 0.001f;

        pathState.depth++;
    }

#ifndef RT_CONFIGURATION_FINAL
    if (context.pathDebugData)
    {
        PathDebugData::HitPointData data;
        data.rayOrigin = ray.origin;
        data.rayDir = ray.dir;
        data.hitPoint = hitPoint;
        data.shadingData = shadingData;
        data.throughput = throughput;
        context.pathDebugData->data.PushBack(data);
        context.pathDebugData->terminationReason = pathTerminationReason;
    }
#endif // RT_CONFIGURATION_FINAL

    context.counters.numRays += (Uint64)pathState.depth + 1;

    return resultColor;
}

} // namespace rt
