#include "PCH.h"
#include "PathTracer.h"
#include "Context.h"
#include "Scene/Scene.h"
#include "Scene/Light/Light.h"
#include "Material/Material.h"
#include "Traversal/TraversalContext.h"
#include "Sampling/GenericSampler.h"

namespace rt {

using namespace math;

PathTracer::PathTracer(const Scene& scene)
    : IRenderer(scene)
{
}

const char* PathTracer::GetName() const
{
    return "Path Tracer";
}

const RayColor PathTracer::EvaluateLight(const ILight& light, const math::Ray& ray, float dist, RenderingContext& context) const
{
    const Vector4 hitPos = ray.GetAtDistance(dist);

    RayColor lightContribution = light.GetRadiance(context, ray.dir, hitPos);
    RT_ASSERT(lightContribution.IsValid());

    return lightContribution;
}

const RayColor PathTracer::EvaluateGlobalLights(const Ray& ray, RenderingContext& context) const
{
    RayColor result = RayColor::Zero();

    for (const ILight* globalLight : mScene.GetGlobalLights())
    {
        RayColor lightContribution = globalLight->GetRadiance(context, ray.dir, Vector4::Zero());
        RT_ASSERT(lightContribution.IsValid());

        result += lightContribution;
    }

    return result;
}

const RayColor PathTracer::RenderPixel(const math::Ray& primaryRay, const RenderParam&, RenderingContext& context) const
{
    HitPoint hitPoint;
    Ray ray = primaryRay;

    ShadingData shadingData;

    RayColor resultColor = RayColor::Zero();
    RayColor throughput = RayColor::One();

    Uint32 depth = 0;

    for (;;)
    {
        hitPoint.distance = FLT_MAX;
        context.localCounters.Reset();
        mScene.Traverse_Single({ ray, hitPoint, context });
        context.counters.Append(context.localCounters);

        // ray missed - return background light color
        if (hitPoint.distance == FLT_MAX)
        {
            resultColor += throughput * EvaluateGlobalLights(ray, context);
            break;
        }

        // we hit a light directly
        if (hitPoint.subObjectId == RT_LIGHT_OBJECT)
        {
            const ILight& light = mScene.Internal_GetLightByObjectId(hitPoint.objectId);
            resultColor += throughput * EvaluateLight(light, ray, hitPoint.distance, context);
            break;
        }

        // fill up structure with shading data
        {
            mScene.ExtractShadingData(ray, hitPoint, context.time, shadingData);

            RT_ASSERT(shadingData.material != nullptr);
            shadingData.material->EvaluateShadingData(context.wavelength, shadingData);
        }

        // accumulate emission color
        const RayColor emissionColor = RayColor::Resolve(context.wavelength, Spectrum(shadingData.material->emission.Evaluate(shadingData.texCoord)));
        RT_ASSERT(emissionColor.IsValid());
        resultColor += throughput * emissionColor;
        RT_ASSERT(resultColor.IsValid());

        // check if the ray depth won't be exeeded in the next iteration
        if (depth >= context.params->maxRayDepth)
        {
            break;
        }

        // Russian roulette algorithm
        if (depth >= context.params->minRussianRouletteDepth)
        {
            const float threshold = shadingData.materialParams.baseColor.Max();
#ifdef RT_ENABLE_SPECTRAL_RENDERING
            if (context.wavelength.isSingle)
            {
                threshold *= 1.0f / static_cast<float>(Wavelength::NumComponents);
            }
#endif
            if (context.sampler->GetFloat() > threshold)
            {
                break;
            }
            throughput *= 1.0f / threshold;
        }

        // sample BSDF
        Vector4 incomingDirWorldSpace;
        const RayColor bsdfValue = shadingData.material->Sample(context.wavelength, incomingDirWorldSpace, shadingData, context.sampler->GetFloat3());

        RT_ASSERT(bsdfValue.IsValid());
        throughput *= bsdfValue;

        // ray is not visible anymore
        if (throughput.AlmostZero())
        {
            break;
        }

        // generate secondary ray
        ray = Ray(shadingData.frame.GetTranslation(), incomingDirWorldSpace);
        ray.origin += ray.dir * 0.001f;

        depth++;
    }

    return resultColor;
}

} // namespace rt
