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

RT_FORCE_INLINE static Float Mis(const Float samplePdf)
{
    return samplePdf;
}

RT_FORCE_INLINE static Float CombineMis(const Float samplePdf, const Float otherPdf)
{
    return Mis(samplePdf) / (Mis(samplePdf) + Mis(otherPdf));
}

PathTracer::PathTracer(const Scene& scene)
    : IRenderer(scene)
{
}

const Color PathTracer::SampleLights(const Ray& ray, const ShadingData& shadingData, RenderingContext& context) const
{
    Color accumulatedColor;
    Vector4 dirToLight;

    // TODO check only one (or few) lights per sample instead all of them
    // TODO check only nearest lights
    for (const LightPtr& light : mScene.GetLights())
    {
        // TODO multiple importance sampling

        float distanceToLight;

        // calculate light contribution
        Color lightColor = light->Illuminate(shadingData.position, context, dirToLight, distanceToLight);
        if (lightColor.AlmostZero())
        {
            continue;
        }

        // calculate BSDF contribution
        lightColor *= shadingData.material->Evaluate(context.wavelength, shadingData, -ray.dir, -dirToLight);
        if (lightColor.AlmostZero())
        {
            continue;
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
                continue;
            }
        }

        //const float NdotL = Vector4::Dot3(dirToLight, shadingData.normal);
        accumulatedColor += lightColor / (distanceToLight * distanceToLight);
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

    for (;;)
    {
        hitPoint.distance = FLT_MAX;
        context.localCounters.Reset();
        mScene.Traverse_Single({ ray, hitPoint, context });
        context.counters.Append(context.localCounters);

        // ray missed - return background color
        if (hitPoint.distance == FLT_MAX)
        {
            resultColor += throughput * mScene.GetBackgroundColor(ray, context);
            pathTerminationReason = PathTerminationReason::HitBackground;
            break;
        }

        // we hit a light directly
        if (hitPoint.triangleId == RT_LIGHT_OBJECT)
        {
            // HACK
            const LightSceneObject* lightSceneObj = static_cast<const LightSceneObject*>(mScene.GetObjects()[hitPoint.objectId].get());
            const ILight& light = lightSceneObj->GetLight();

            const Vector4 hitPos = ray.origin + ray.dir * hitPoint.distance;
            const Color lightContribution = light.GetRadiance(context, ray.dir, hitPos);

            resultColor += throughput * lightContribution;
            pathTerminationReason = PathTerminationReason::HitLight;
            break;
        }

        mScene.ExtractShadingData(ray.origin, ray.dir, hitPoint, context.time, shadingData);

        // accumulate emission color
        const Color emissionColor = Color::SampleRGB(context.wavelength, shadingData.material->GetEmissionColor(shadingData.texCoord));
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
        throughput *= shadingData.material->Sample(context.wavelength, -ray.dir, incomingDirWorldSpace, shadingData, context.randomGenerator);

        // ray is not visible anymore
        if (throughput.AlmostZero())
        {
            pathTerminationReason = PathTerminationReason::Throughput;
            break;
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
