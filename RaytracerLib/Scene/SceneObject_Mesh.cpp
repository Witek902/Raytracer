#include "PCH.h"
#include "SceneObject_Mesh.h"
#include "Mesh/Mesh.h"
#include "Math/Geometry.h"


namespace rt {

using namespace math;

MeshSceneObject::MeshSceneObject(const Mesh* mesh)
    : mMesh(mesh)
{ }

Box MeshSceneObject::GetBoundingBox() const
{
    const Box localBox = mMesh->GetBoundingBox();

    // TODO include rotation
    return Box(localBox + mPosition, localBox + (mPosition + mPositionOffset));
}

void MeshSceneObject::Traverse_Single(const Uint32 objectID, const Ray& ray, HitPoint& hitPoint) const
{
    mMesh->Traverse_Single(ray, hitPoint, objectID);
}

void MeshSceneObject::Traverse_Simd8(const Ray_Simd8& ray, HitPoint_Simd8& outHitPoint) const
{
    mMesh->Traverse_Simd8(ray, outHitPoint);
}

void MeshSceneObject::Traverse_Packet(const RayPacket& rayPacket, HitPoint_Packet& outHitPoint) const
{
    mMesh->Traverse_Packet(rayPacket, outHitPoint);
}

void MeshSceneObject::EvaluateShadingData_Single(const Matrix& worldToLocal, const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    RT_UNUSED(worldToLocal);

    mMesh->EvaluateShadingData_Single(hitPoint, outShadingData);
}


} // namespace rt
