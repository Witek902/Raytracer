#include "PCH.h"
#include "SceneObject_Light.h"
#include "../Light/Light.h"
#include "../Light/AreaLight.h"
#include "../../Shapes/Shape.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;

LightSceneObject::LightSceneObject(LightPtr light)
    : mLight(std::move(light))
{ }

ISceneObject::Type LightSceneObject::GetType() const
{
    return Type::Light;
}

Box LightSceneObject::GetBoundingBox() const
{
    const Box localSpaceBox = mLight->GetBoundingBox();
    return { GetBaseTransform().TransformBox(localSpaceBox), GetTransform(1.0f).TransformBox(localSpaceBox) };
}

void LightSceneObject::Traverse(const SingleTraversalContext& context, const Uint32 objectID) const
{
    float lightDistance;
    if (mLight->TestRayHit(context.ray, lightDistance))
    {
        if (lightDistance < context.hitPoint.distance)
        {
            // mark as light
            context.hitPoint.Set(lightDistance, objectID, RT_LIGHT_OBJECT);
        }
    }
}

bool LightSceneObject::Traverse_Shadow(const SingleTraversalContext& context) const
{
    float lightDistance;
    if (mLight->TestRayHit(context.ray, lightDistance))
    {
        if (lightDistance < context.hitPoint.distance)
        {
            context.hitPoint.distance = lightDistance;
            return true;
        }
    }

    return false;
}

void LightSceneObject::Traverse(const PacketTraversalContext& context, const Uint32 objectID, const Uint32 numActiveGroups) const
{
    RT_UNUSED(context);
    RT_UNUSED(objectID);
    RT_UNUSED(numActiveGroups);
    // TODO
}

void LightSceneObject::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const
{
    if (mLight->GetType() == ILight::Type::Area)
    {
        const AreaLight& areaLight = static_cast<const AreaLight&>(*mLight);
        areaLight.GetShape()->EvaluateIntersection(hitPoint, outIntersectionData);
    }
    else
    {
        RT_FATAL("Cannot evaluate intersection for non-area lights");
    }
}


} // namespace rt
