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

Vector4 CpuScene::TraceRay_Single(const Ray& ray, RayTracingContext& context, Uint32 rayDepth) const
{
    if (rayDepth >= context.params.maxRayDepth)
    {
        // ray depth exceeded limit
        return Vector4();
    }

    const MeshInstance* bestMeshInstance = nullptr;
    float distance = FLT_MAX;
    MeshIntersectionData intersectionData;
    intersectionData.u = intersectionData.v = 0.0f;

    for (const MeshInstance& meshInstance : mMeshInstances)
    {
        const CpuMesh* mesh = static_cast<const CpuMesh*>(meshInstance.mMesh);

        // TODO transform ray
        if (mesh->RayTrace_Single(ray, distance, intersectionData))
        {
            if (intersectionData.distance < distance)
            {
                distance = intersectionData.distance;
                bestMeshInstance = &meshInstance;
            }
        }
    }

    if (!bestMeshInstance)
    {
        return Vector4(0.1f, 0.2f, 0.3f);
    }

    ShadingData shadingData;
    const CpuMesh* mesh = static_cast<const CpuMesh*>(bestMeshInstance->mMesh);
    mesh->EvaluateShadingData_Single(ray, intersectionData, shadingData);

    math::Vector4 resultColor = shadingData.material->emissionColor;

    // diffuse rays
    if (!shadingData.material->diffuseColor.IsZero())
    {
        const Vector4 localDir = context.randomGenerator.GetHemishpereCos();
        const Vector4 origin = shadingData.position + shadingData.normal * 0.001f;
        const Vector4 globalDir = shadingData.tangent * localDir[0] + shadingData.binormal * localDir[1] + shadingData.normal * localDir[2];

        const Ray secondaryRay(origin, globalDir);
        context.counters.numDiffuseRays++;

        resultColor += shadingData.material->diffuseColor * TraceRay_Single(secondaryRay, context, rayDepth + 1);
    }

    return resultColor;
}

} // namespace rt
