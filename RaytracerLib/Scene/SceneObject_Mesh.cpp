#include "PCH.h"
#include "SceneObject_Mesh.h"
#include "Mesh/Mesh.h"
#include "Math/Geometry.h"
#include "Traversal/Traversal_Single.h"
#include "Traversal/Traversal_Simd.h"
#include "Traversal/Traversal_Packet.h"

namespace rt {

using namespace math;

MeshSceneObject::MeshSceneObject(const Mesh* mesh)
    : mMesh(mesh)
{ }

Box MeshSceneObject::GetBoundingBox() const
{
    const Box localBox = mMesh->GetBoundingBox();

    // TODO include rotation
    return Box(localBox + mTransform.GetTranslation(), localBox + mTransform.GetTranslation() + mTransform.GetTranslation());
}

void MeshSceneObject::Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const
{
    GenericTraverse_Single<Mesh>(context, objectID, mMesh);
}

void MeshSceneObject::Traverse_Simd8(const SimdTraversalContext& context, const Uint32 objectID) const
{
    GenericTraverse_Simd8<Mesh>(context, objectID, mMesh);
}

void MeshSceneObject::Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID) const
{
    GenericTraverse_Packet<Mesh>(context, objectID, mMesh);
}

void MeshSceneObject::EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    mMesh->EvaluateShadingData_Single(hitPoint, outShadingData);
}


} // namespace rt
