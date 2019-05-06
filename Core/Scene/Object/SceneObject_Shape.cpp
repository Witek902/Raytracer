#include "PCH.h"
#include "SceneObject_Shape.h"
#include "Shapes/Shape.h"
#include "Math/Simd8Geometry.h"
#include "Rendering/ShadingData.h"
#include "Rendering/Context.h"
#include "Traversal/TraversalContext.h"
#include "Material/Material.h"

namespace rt {

using namespace math;

ShapeSceneObject::ShapeSceneObject(const ShapePtr& shape)
    : mShape(shape)
{
    RT_ASSERT(mShape, "Invalid shape");
}

ISceneObject::Type ShapeSceneObject::GetType() const
{
    return Type::Shape;
}

Box ShapeSceneObject::GetBoundingBox() const
{
    const Box localSpaceBox = mShape->GetBoundingBox();
    return { GetBaseTransform().TransformBox(localSpaceBox), GetTransform(1.0f).TransformBox(localSpaceBox) };
}

void ShapeSceneObject::SetDefaultMaterial(const MaterialPtr& material)
{
    mDefaultMaterial = material;

    if (!mDefaultMaterial)
    {
        mDefaultMaterial = Material::GetDefaultMaterial();
    }
}

void ShapeSceneObject::Traverse(const SingleTraversalContext& context, const Uint32 objectID) const
{
    return mShape->Traverse(context, objectID);
}

bool ShapeSceneObject::Traverse_Shadow(const SingleTraversalContext& context) const
{
    return mShape->Traverse_Shadow(context);
}

void ShapeSceneObject::Traverse(const PacketTraversalContext& context, const Uint32 objectID, const Uint32 numActiveGroups) const
{
    // TODO

    RT_UNUSED(context);
    RT_UNUSED(objectID);
    RT_UNUSED(numActiveGroups);
}

void ShapeSceneObject::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const
{
    outIntersectionData.material = mDefaultMaterial.get();

    mShape->EvaluateIntersection(hitPoint, outIntersectionData);
}


} // namespace rt
