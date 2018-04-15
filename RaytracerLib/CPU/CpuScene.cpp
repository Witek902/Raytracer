#include "PCH.h"
#include "CpuScene.h"
#include "Bitmap.h"
#include "Camera.h"
#include "CpuMesh.h"
#include "Math/Triangle.h" // TODO remove
#include "Math/Geometry.h" // TODO remove
#include "Math/Simd4Geometry.h" // TODO remove
#include "Math/Simd8Geometry.h" // TODO remove


namespace rt {

using namespace math;

CpuScene::CpuScene()
{
}

MeshInstanceID CpuScene::CreateMeshInstance(const MeshInstance& data)
{
    MeshInstanceID id = static_cast<MeshInstanceID>(mMeshInstances.size());
    mMeshInstances.push_back(data);
    return id;
}

void CpuScene::DestroyMeshInstance(MeshInstanceID id)
{
    // TODO
    (void)id;
}

void CpuScene::UpdateMeshInstance(MeshInstanceID id, const MeshInstance& data)
{
    (void)id;
    (void)data;
}

LightID CpuScene::CreateLightInstance(const LightInstance& data)
{
    LightID id = static_cast<LightID>(mLightInstances.size());
    mLightInstances.push_back(data);
    return id;
}

Vector4 CpuScene::TraceRay_Single(const Ray& ray, RayTracingContext& context, const Uint32 rayDepth) const
{
    RT_UNUSED(rayDepth);

    /*
    RT_UNUSED(context);
    const Box box((Vector4)VECTOR_ONE / 3.0f, -(Vector4)VECTOR_ONE / 3.0f);

    if (math::Intersect(ray, box))
        return Vector4(3.0f, 3.2f, 3.3f);

    return Vector4(0.1f, 0.1f, 0.1f);
    */

    Ray currentRay = ray;

    Vector4 resultColor;
    Vector4 throughput = VECTOR_ONE;

    for (Uint32 i = 0; i < context.params.maxRayDepth; ++i)
    {
        const MeshInstance* bestMeshInstance = nullptr;
        float distance = FLT_MAX;
        MeshIntersectionData intersectionData;
        intersectionData.u = intersectionData.v = 0.0f;

        for (const MeshInstance& meshInstance : mMeshInstances)
        {
            const CpuMesh* mesh = static_cast<const CpuMesh*>(meshInstance.mMesh);

            // TODO transform ray
            if (mesh->RayTrace_Single(currentRay, distance, intersectionData))
            {
                if (intersectionData.distance < distance)
                {
                    distance = intersectionData.distance;
                    bestMeshInstance = &meshInstance;
                }
            }
        }

        // ray missed - return background color
        if (!bestMeshInstance)
        {
            resultColor += throughput * mEnvironment.backgroundColor;
            break;
        }

        ShadingData shadingData;
        const CpuMesh* mesh = static_cast<const CpuMesh*>(bestMeshInstance->mMesh);
        mesh->EvaluateShadingData_Single(currentRay, intersectionData, shadingData);

        // accumulate emission color
        resultColor += throughput * shadingData.material->emissionColor;

        const Vector4 baseColor = shadingData.material->GetBaseColor(shadingData.texCoord);
        if (baseColor.IsZero())
        {
            break;
        }

        // accumulate attenuation
        throughput *= baseColor;

        // Russian roulette algorithm
        {
            const float threshold = Max(throughput[0], Max(throughput[1], throughput[2]));
            if (context.randomGenerator.GetFloat() > threshold)
                break;

            throughput /= threshold;
        }

        // generate secondary ray
        const Vector4 incomingDir = currentRay.dir;
        math::Vector4 factor;
        if (!shadingData.material->GenerateSecondaryRay(incomingDir, shadingData, context.randomGenerator, currentRay, factor))
        {
            break;
        }
        throughput *= factor;
    }

    return resultColor;
}

} // namespace rt
