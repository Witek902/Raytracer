#include "PCH.h"
#include "PathTracerMIS.h"
#include "Context.h"
#include "PathDebugging.h"
#include "Scene/Scene.h"
#include "Scene/Light/Light.h"
#include "Scene/Object/SceneObject_Light.h"
#include "Material/Material.h"
#include "Traversal/TraversalContext.h"
#include "Sampling/GenericSampler.h"

namespace rt {

using namespace math;

RT_FORCE_INLINE static float Mis(const float samplePdf)
{
    return samplePdf;
}

RT_FORCE_INLINE static float CombineMis(const float samplePdf, const float otherPdf)
{
    return FastDivide(Mis(samplePdf), Mis(samplePdf) + Mis(otherPdf));
}

RT_FORCE_INLINE static float PdfAtoW(const float pdfA, const float distance, const float cosThere)
{
    return FastDivide(pdfA * Sqr(distance), Abs(cosThere));
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

const RayColor PathTracerMIS::SampleLight(const LightSceneObject* lightObject, const ShadingData& shadingData, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const
{
    const ILight& light = lightObject->GetLight();

    const ILight::IlluminateParam illuminateParam =
    {
        lightObject->GetInverseTransform(context.time),
        lightObject->GetTransform(context.time),
        shadingData.intersection,
        context.wavelength,
        context.sampler.GetFloat3(),
    };

    // calculate light contribution
    ILight::IlluminateResult illuminateResult;
    const RayColor radiance = light.Illuminate(illuminateParam, illuminateResult);
    RT_ASSERT(radiance.IsValid());

    if (radiance.AlmostZero())
    {
        return RayColor::Zero();
    }

    RT_ASSERT(IsValid(illuminateResult.directPdfW) && illuminateResult.directPdfW >= 0.0f);
    RT_ASSERT(IsValid(illuminateResult.distance) && illuminateResult.distance >= 0.0f);
    RT_ASSERT(IsValid(illuminateResult.cosAtLight) && illuminateResult.cosAtLight >= 0.0f);
    RT_ASSERT(illuminateResult.directionToLight.IsValid());

    // calculate BSDF contribution
    float bsdfPdfW;
    const RayColor factor = shadingData.intersection.material->Evaluate(context.wavelength, shadingData, -illuminateResult.directionToLight, &bsdfPdfW);
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

        Ray shadowRay(shadingData.intersection.frame.GetTranslation(), illuminateResult.directionToLight);
        shadowRay.origin += shadowRay.dir * 0.0001f;

        context.counters.numShadowRays++;
        if (mScene.Traverse_Shadow({ shadowRay, hitPoint, context }))
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

    const RayColor result = (radiance * factor) * FastDivide(weight, lightPickProbability * illuminateResult.directPdfW);
    RT_ASSERT(result.IsValid());

    return result;
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
                const uint32 lightIndex = context.randomGenerator.GetInt() % lights.Size();
                accumulatedColor = SampleLight(lights[lightIndex], shadingData, pathState, context, lightPickProbability);
                break;
            }

            case LightSamplingStrategy::All:
            {
                for (const LightSceneObject* lightObject : lights)
                {
                    accumulatedColor += SampleLight(lightObject, shadingData, pathState, context, lightPickProbability);
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

const RayColor PathTracerMIS::EvaluateLight(const LightSceneObject* lightObject, const math::Ray& ray, float dist, const IntersectionData& intersection, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const
{
    const ILight& light = lightObject->GetLight();

    const Matrix4 worldToLight = lightObject->GetInverseTransform(context.time);
    const Ray lightSpaceRay = worldToLight.TransformRay_Unsafe(ray);
    const Vector4 lightSpaceHitPoint = worldToLight.TransformPoint(intersection.frame.GetTranslation());
    const float cosAtLight = -intersection.CosTheta(ray.dir);

    const ILight::RadianceParam param =
    {
        context,
        lightSpaceRay,
        lightSpaceHitPoint,
        cosAtLight,
    };

    float directPdfA;
    RayColor lightContribution = light.GetRadiance(param, &directPdfA);
    RT_ASSERT(lightContribution.IsValid());

    if (lightContribution.AlmostZero())
    {
        return RayColor::Zero();
    }

    RT_ASSERT(directPdfA > 0.0f && IsValid(directPdfA));

    float misWeight = 1.0f;
    if (pathState.depth > 0 && !pathState.lastSpecular)
    {
        const float directPdfW = PdfAtoW(directPdfA, dist, cosAtLight);
        misWeight = CombineMis(pathState.lastPdfW, directPdfW * lightPickProbability);
    }

    lightContribution *= RayColor::Resolve(context.wavelength, Spectrum(mBSDFSamplingWeight));

    return lightContribution * misWeight;
}

const RayColor PathTracerMIS::EvaluateGlobalLights(const Ray& ray, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const
{
    RayColor result = RayColor::Zero();

    for (const LightSceneObject* globalLightObject : mScene.GetGlobalLights())
    {
        const Matrix4 worldToLight = globalLightObject->GetInverseTransform(context.time);
        const Ray lightSpaceRay = worldToLight.TransformRay_Unsafe(ray);

        const ILight& light = globalLightObject->GetLight();

        const ILight::RadianceParam param =
        {
            context,
            lightSpaceRay,
        };

        float directPdfW;
        RayColor lightContribution = light.GetRadiance(param, &directPdfW);
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
        hitPoint.objectId = RT_INVALID_OBJECT;
        hitPoint.distance = HitPoint::DefaultDistance;
        mScene.Traverse({ ray, hitPoint, context });

        // ray missed - return background light color
        if (hitPoint.objectId == RT_INVALID_OBJECT)
        {
            resultColor.MulAndAccumulate(throughput, EvaluateGlobalLights(ray, pathState, context, lightPickProbability));
            pathTerminationReason = PathTerminationReason::HitBackground;
            break;
        }

        if (hitPoint.distance < FLT_MAX)
        {
            mScene.EvaluateIntersection(ray, hitPoint, context.time, shadingData.intersection);
        }

        // we hit a light directly
        if (hitPoint.subObjectId == RT_LIGHT_OBJECT)
        {
            const ISceneObject* sceneObject = mScene.GetHitObject(hitPoint.objectId);
            RT_ASSERT(sceneObject->GetType() == ISceneObject::Type::Light);
            const LightSceneObject* lightObject = static_cast<const LightSceneObject*>(sceneObject);
            
            const RayColor lightColor = EvaluateLight(lightObject, ray, hitPoint.distance, shadingData.intersection, pathState, context, lightPickProbability);
            RT_ASSERT(lightColor.IsValid());
            resultColor.MulAndAccumulate(throughput, lightColor);

            pathTerminationReason = PathTerminationReason::HitLight;
            break;
        }

        // fill up structure with shading data
        shadingData.outgoingDirWorldSpace = -ray.dir;
        mScene.EvaluateShadingData(shadingData, context);

        // accumulate emission color
        {
            RayColor emissionColor = shadingData.materialParams.emissionColor;
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
            if (context.sampler.GetFloat() > threshold)
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
        const RayColor bsdfValue = shadingData.intersection.material->Sample(context.wavelength, incomingDirWorldSpace, shadingData, context.sampler.GetFloat3(), &pdf, &lastSampledBsdfEvent);

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
        ray = Ray(shadingData.intersection.frame.GetTranslation(), incomingDirWorldSpace);
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

    context.counters.numRays += (uint64)pathState.depth + 1;

    return resultColor;
}

} // namespace rt
