#include "PCH.h"
#include "LightTracer.h"
#include "Film.h"
#include "Context.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"
#include "Scene/Light/Light.h"
#include "Sampling/GenericSampler.h"
#include "Material/Material.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;

LightTracer::LightTracer(const Scene& scene)
    : IRenderer(scene)
{
}

const char* LightTracer::GetName() const
{
    return "Light Tracer";
}

const RayColor LightTracer::RenderPixel(const Ray&, const RenderParam& param, RenderingContext& ctx) const
{
    Uint32 depth = 0;

    const auto& allLocalLights = mScene.GetLights();
    if (allLocalLights.Empty())
    {
        // no lights on the scene
        return RayColor::Zero();
    }

    const float lightPickingProbability = 1.0f / (float)allLocalLights.Size();
    const Uint32 lightIndex = ctx.randomGenerator.GetInt() % allLocalLights.Size();
    const LightPtr& light = allLocalLights[lightIndex];

    const ILight::EmitParam emitParam =
    {
        ctx.wavelength,
        ctx.sampler->GetFloat2(),
        ctx.sampler->GetFloat2(),
    };

    ILight::EmitResult emitResult;
    RayColor throughput = light->Emit(emitParam, emitResult);

    if (throughput.AlmostZero())
    {
        // generated too weak sample - skip it
        return RayColor::Zero();
    }

    emitResult.emissionPdfW *= lightPickingProbability;
    RT_ASSERT(emitResult.emissionPdfW > 0.0f);

    Ray ray = Ray(emitResult.position, emitResult.direction);

    // TODO don't divide by pdf in ILight::Emit()
    throughput *= 1.0f / emitResult.emissionPdfW;

    HitPoint hitPoint;

    for (;;)
    {
        hitPoint.distance = FLT_MAX;
        mScene.Traverse_Single({ ray, hitPoint, ctx });

        if (hitPoint.distance == FLT_MAX)
        {
            break; // ray missed
        }

        if (hitPoint.subObjectId == RT_LIGHT_OBJECT)
        {
            break; // we hit a light directly
        }

        // fill up structure with shading data
        ShadingData shadingData;
        {
            mScene.ExtractShadingData(ray, hitPoint, ctx.time, shadingData);

            RT_ASSERT(shadingData.material != nullptr);
            shadingData.material->EvaluateShadingData(ctx.wavelength, shadingData);
        }

        // check if the ray depth won't be exeeded in the next iteration
        if (depth >= ctx.params->maxRayDepth)
        {
            break;
        }

//        // Russian roulette algorithm
//        if (depth >= ctx.params->minRussianRouletteDepth)
//        {
//            float threshold = throughput.Max();
//#ifdef RT_ENABLE_SPECTRAL_RENDERING
//            if (ctx.wavelength.isSingle)
//            {
//                threshold *= 1.0f / static_cast<float>(Wavelength::NumComponents);
//            }
//#endif
//            if (ctx.randomGenerator.GetFloat() > threshold)
//            {
//                break;
//            }
//            throughput *= 1.0f / threshold;
//        }

        // connect to camera
        {
            const Vector4 cameraPos = param.camera.GetTransform().GetTranslation();
            const Vector4 samplePos = shadingData.frame.GetTranslation();

            Vector4 dirToCamera = cameraPos - samplePos;

            const float cameraDistanceSqr = dirToCamera.SqrLength3();
            const float cameraDistance = sqrtf(cameraDistanceSqr);

            dirToCamera /= cameraDistance;

            // calculate BSDF contribution
            float bsdfPdfW;
            const RayColor cameraFactor = shadingData.material->Evaluate(ctx.wavelength, shadingData, -dirToCamera, &bsdfPdfW);
            RT_ASSERT(cameraFactor.IsValid());

            if (!cameraFactor.AlmostZero())
            {
                Vector4 filmPos;

                if (param.camera.WorldToFilm(samplePos, filmPos))
                {
                    HitPoint shadowHitPoint;
                    shadowHitPoint.distance = cameraDistance * 0.999f;

                    const Ray shadowRay(samplePos + shadingData.frame[2] * 0.0001f, dirToCamera);

                    if (!mScene.Traverse_Shadow_Single({ shadowRay, shadowHitPoint, ctx }))
                    {
                        const float cameraPdfA = param.camera.PdfW(-dirToCamera) / cameraDistanceSqr;
                        const RayColor contribution = (cameraFactor * throughput) * cameraPdfA;
                        const Vector4 value = contribution.ConvertToTristimulus(ctx.wavelength);
                        param.film.AccumulateColor(filmPos, value, ctx.randomGenerator);
                    }
                }
            }
        }

        // sample BSDF
        Vector4 incomingDirWorldSpace;
        const RayColor bsdfValue = shadingData.material->Sample(ctx.wavelength, incomingDirWorldSpace, shadingData, ctx.sampler->GetFloat3());

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

    ctx.counters.numRays += (Uint64)depth + 1;

    return RayColor::Zero();
}

} // namespace rt
