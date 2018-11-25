#include "PCH.h"
#include "SceneObject_Mesh.h"
#include "Mesh/Mesh.h"
#include "Math/Geometry.h"
#include "Traversal/Traversal_Single.h"
#include "Traversal/Traversal_Simd.h"
#include "Traversal/Traversal_Packet.h"

namespace rt {

using namespace math;

MeshSceneObject::MeshSceneObject(const MeshPtr mesh)
    : mMesh(mesh)
{ }

Box MeshSceneObject::GetBoundingBox() const
{
    const Box localBox = mMesh->GetBoundingBox();

    // TODO just transformed box may be bigger that bounding box of rotated triangles
    const Box box0 = mTransform.TransformBox(localBox);
    const Box box1 = ComputeTransform(1.0f).TransformBox(localBox);

    return Box(box0, box1);
}

void MeshSceneObject::Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const
{
    GenericTraverse_Single<Mesh>(context, objectID, mMesh.get());
}

bool MeshSceneObject::Traverse_Shadow_Single(const SingleTraversalContext& context) const
{
    return GenericTraverse_Shadow_Single<Mesh>(context, mMesh.get());
}

void MeshSceneObject::Traverse_Simd8(const SimdTraversalContext& context, const Uint32 objectID) const
{
    GenericTraverse_Simd8<Mesh>(context, objectID, mMesh.get());
}

void MeshSceneObject::Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID) const
{
    GenericTraverse_Packet<Mesh>(context, objectID, mMesh.get());
}

void MeshSceneObject::EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    mMesh->EvaluateShadingData_Single(hitPoint, outShadingData);
}


} // namespace rt
