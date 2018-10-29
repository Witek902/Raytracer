#include "PCH.h"
#include "SceneObject_Light.h"
#include "../Light/Light.h"
#include "Traversal/TraversalContext.h"
#include "Math/Geometry.h"
#include "Rendering/ShadingData.h"

namespace rt {

using namespace math;

LightSceneObject::LightSceneObject(const ILight& light)
    : mLight(light)
{ }

Box LightSceneObject::GetBoundingBox() const
{
    return mLight.GetBoundingBox();
}

void LightSceneObject::Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const
{
    Float lightDistance;
    if (mLight.TestRayHit(context.ray, lightDistance))
    {
        if (lightDistance < context.hitPoint.distance)
        {
            context.hitPoint.distance = lightDistance;
            context.hitPoint.objectId = objectID;
            context.hitPoint.triangleId = RT_LIGHT_OBJECT; // mark as light
        }
    }
}

bool LightSceneObject::Traverse_Shadow_Single(const SingleTraversalContext& context) const
{
    RT_UNUSED(context);
    return false;
}

void LightSceneObject::Traverse_Simd8(const SimdTraversalContext& context, const Uint32 objectID) const
{
    RT_UNUSED(context);
    RT_UNUSED(objectID);

    // TODO
}

void LightSceneObject::Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID) const
{
    RT_UNUSED(context);
    RT_UNUSED(objectID);

    // TODO
}

void LightSceneObject::EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    RT_UNUSED(hitPoint);

    outShadingData.material = nullptr;
    outShadingData.normal = Vector4(0.0f, 0.0f, 1.0f, 0.0f); // HACK
    outShadingData.tangent = Vector4();
    outShadingData.bitangent = Vector4();
}


} // namespace rt
