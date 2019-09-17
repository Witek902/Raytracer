#include "PCH.h"
#include "PathTracer.h"
#include "Context.h"
#include "Scene/Scene.h"
#include "Scene/Light/Light.h"
#include "Scene/Object/SceneObject.h"
#include "Scene/Object/SceneObject_Light.h"
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

const RayColor PathTracer::EvaluateLight(const LightSceneObject* lightObject, const math::Ray& ray, const IntersectionData& intersection, RenderingContext& context) const
{
    const float cosAtLight = -intersection.CosTheta(ray.dir);

    const Matrix4 worldToLight = lightObject->GetInverseTransform(context.time);
    const Ray lightSpaceRay = worldToLight.TransformRay_Unsafe(ray);
    const Vector4 lightSpaceHitPoint = worldToLight.TransformPoint(intersection.frame.GetTranslation());

    const ILight& light = lightObject->GetLight();

    const ILight::RadianceParam param =
    {
        context,
        lightSpaceRay,
        lightSpaceHitPoint,
        cosAtLight,
    };

    return light.GetRadiance(param);
}

const RayColor PathTracer::EvaluateGlobalLights(const Ray& ray, RenderingContext& context) const
{
    RayColor result = RayColor::Zero();

    for (const LightSceneObject* globalLightObject : mScene.GetGlobalLights())
    {
        const Matrix4 worldToLight = globalLightObject->GetInverseTransform(context.time);
        const Ray lightSpaceRay = worldToLight.TransformRay_Unsafe(ray);

        const ILight& globalLight = globalLightObject->GetLight();

        const ILight::RadianceParam param =
        {
            context,
            lightSpaceRay,
        };

        const float cosAtLight = 1.0f;
        RayColor lightContribution = globalLight.GetRadiance(param);
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
        hitPoint.distance = HitPoint::DefaultDistance;
        mScene.Traverse({ ray, hitPoint, context });

        // ray missed - return background light color
        if (hitPoint.distance == HitPoint::DefaultDistance)
        {
            resultColor.MulAndAccumulate(throughput, EvaluateGlobalLights(ray, context));
            break;
        }

        // fill up structure with shading data
        mScene.EvaluateIntersection(ray, hitPoint, context.time, shadingData.intersection);
        shadingData.outgoingDirWorldSpace = -ray.dir;

        // we hit a light directly
        if (hitPoint.subObjectId == RT_LIGHT_OBJECT)
        {
            const ISceneObject* sceneObject = mScene.GetHitObject(hitPoint.objectId);
            RT_ASSERT(sceneObject->GetType() == ISceneObject::Type::Light);
            const LightSceneObject* lightObject = static_cast<const LightSceneObject*>(sceneObject);

            const RayColor lightColor = EvaluateLight(lightObject, ray, shadingData.intersection, context);
            RT_ASSERT(lightColor.IsValid());
            resultColor.MulAndAccumulate(throughput, lightColor);

            break;
        }

        mScene.EvaluateShadingData(shadingData, context);

        // accumulate emission color
        RT_ASSERT(shadingData.materialParams.emissionColor.IsValid());
        resultColor.MulAndAccumulate(throughput, shadingData.materialParams.emissionColor);
        RT_ASSERT(resultColor.IsValid());

        // check if the ray depth won't be exeeded in the next iteration
        if (depth >= context.params->maxRayDepth)
        {
            break;
        }

        // Russian roulette algorithm
        if (depth >= context.params->minRussianRouletteDepth)
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
                break;
            }

            throughput *= 1.0f / threshold;
            RT_ASSERT(throughput.IsValid());
        }

        // sample BSDF
        Vector4 incomingDirWorldSpace;
        const RayColor bsdfValue = shadingData.intersection.material->Sample(context.wavelength, incomingDirWorldSpace, shadingData, context.sampler.GetFloat3());

        RT_ASSERT(bsdfValue.IsValid());
        throughput *= bsdfValue;

        // ray is not visible anymore
        if (throughput.AlmostZero())
        {
            break;
        }

        // generate secondary ray
        ray = Ray(shadingData.intersection.frame.GetTranslation(), incomingDirWorldSpace);
        ray.origin += ray.dir * 0.001f;

        depth++;
    }

    context.counters.numRays += (Uint64)depth + 1;

    return resultColor;
}

} // namespace rt
