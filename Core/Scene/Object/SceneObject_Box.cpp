#include "PCH.h"
#include "SceneObject_Box.h"
#include "Math/Geometry.h"
#include "Math/Simd8Geometry.h"
#include "Rendering/ShadingData.h"
#include "Rendering/Context.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;

namespace helper
{

RT_FORCE_NOINLINE Int32 ConvertXYZtoCubeUV(const Vector4& p, Vector4& outUV)
{
    const Vector4 abs = Vector4::Abs(p);
    const int isXPositive = p.x > 0 ? 1 : 0;
    const int isYPositive = p.y > 0 ? 1 : 0;
    const int isZPositive = p.z > 0 ? 1 : 0;

    float maxAxis, uc, vc;
    int side;

    if (abs.x >= abs.y && abs.x >= abs.z)
    {
        if (isXPositive) // +X
        {
            uc = -p.z;
        }
        else // -X
        {
            uc = p.z;
        }

        side = isXPositive;
        maxAxis = abs.x;
        vc = p.y;
    }
    else if (abs.y >= abs.x && abs.y >= abs.z)
    {
        if (isYPositive) // +Y
        {
            vc = -p.z;
        }
        else // -Y
        {
            vc = p.z;
        }

        side = isYPositive + 2;
        maxAxis = abs.y;
        uc = p.x;
    }
    else
    {
        if (isZPositive) // +Z
        {
            uc = p.x;
        }
        else // -Z
        {
            uc = -p.x;
        }

        side = isZPositive + 4;
        maxAxis = abs.z;
        vc = p.y;
    }

    // Convert range from -1 to 1 to 0 to 1
    outUV = Vector4(uc, vc, 0.0f, 0.0f) / (2.0f * maxAxis) + Vector4(0.5f);

    return side;
}

} // helper


BoxSceneObject::BoxSceneObject(const Vector4& size)
    : mSize(size)
    , mInvSize(VECTOR_ONE / mSize)
{ }

Box BoxSceneObject::GetBoundingBox() const
{
    const Box localBox(-mSize, mSize);
    const Box box0 = mTransform.TransformBox(localBox);
    const Box box1 = ComputeTransform(1.0f).TransformBox(localBox);
    return Box(box0, box1);
}

void BoxSceneObject::Traverse_Single(const SingleTraversalContext& context, const Uint32 objectID) const
{
    const Box box(-mSize, mSize);

    float nearDist, farDist;
    if (Intersect_BoxRay_TwoSided(context.ray, box, nearDist, farDist))
    {
        HitPoint& hitPoint = context.hitPoint;

        if (nearDist > 0.0f && nearDist < hitPoint.distance)
        {       
            //if (hitPoint.filterObjectId != objectID || hitPoint.filterSubObjectId != side)
            {
                hitPoint.distance = nearDist;
                hitPoint.objectId = objectID;
                hitPoint.subObjectId = 0;
                return;
            }
        }

        if (farDist > 0.0f && farDist < hitPoint.distance)
        {
            //if (hitPoint.filterObjectId != objectID || hitPoint.filterSubObjectId != side)
            {
                hitPoint.distance = farDist;
                hitPoint.objectId = objectID;
                hitPoint.subObjectId = 0;
                return;
            }
        }
    }
}

bool BoxSceneObject::Traverse_Shadow_Single(const SingleTraversalContext& context) const
{
    const Box box(-mSize, mSize);

    float dist;
    if (Intersect_BoxRay(context.ray, box, dist))
    {
        if (dist > 0.0f && dist < context.hitPoint.distance)
        {
            context.hitPoint.distance = dist;
            return true;
        }
    }

    return false;
}

void BoxSceneObject::Traverse_Packet(const PacketTraversalContext& context, const Uint32 objectID, const Uint32 numActiveGroups) const
{
    for (Uint32 i = 0; i < numActiveGroups; ++i)
    {
        RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[i]];
        const Ray_Simd8& ray = rayGroup.rays[1];

        const Box_Simd8 box(Box(-mSize, mSize));

        Vector8 nearDist, farDist;
        const VectorBool8 mask = Intersect_BoxRay_TwoSided_Simd8(ray.invDir, ray.origin * ray.invDir, box, rayGroup.maxDistances, nearDist, farDist);

        const Vector8 t = Vector8::Select(nearDist, farDist, nearDist < Vector8::Zero());

        context.StoreIntersection(rayGroup, t, mask, objectID);
    }
}

void BoxSceneObject::EvaluateShadingData_Single(const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    RT_UNUSED(hitPoint);

    const Vector4 normalsAndTangnts[] =
    {
        Vector4(-1.0f, 0.0f, 0.0f, 0.0f),   Vector4(0.0f, 0.0f, +1.0f, 0.0f),
        Vector4(+1.0f, 0.0f, 0.0f, 0.0f),   Vector4(0.0f, 0.0f, -1.0f, 0.0f),
        Vector4(0.0f, -1.0f, 0.0f, 0.0f),   Vector4(+1.0f, 0.0f, 0.0f, 0.0f),
        Vector4(0.0f, +1.0f, 0.0f, 0.0f),   Vector4(+1.0f, 0.0f, 0.0f, 0.0f),
        Vector4(0.0f, 0.0f, -1.0f, 0.0f),   Vector4(-1.0f, 0.0f, 0.0f, 0.0f),
        Vector4(0.0f, 0.0f, +1.0f, 0.0f),   Vector4(+1.0f, 0.0f, 0.0f, 0.0f),
    };

    outShadingData.material = mDefaultMaterial.get();

    const Int32 side = helper::ConvertXYZtoCubeUV(outShadingData.frame.GetTranslation() * mInvSize, outShadingData.texCoord);
    outShadingData.frame[2] = normalsAndTangnts[2 * side];
    outShadingData.frame[0] = normalsAndTangnts[2 * side + 1];
    outShadingData.frame[1] = Vector4::Cross3(outShadingData.frame[0], outShadingData.frame[2]);
}


} // namespace rt
