#include "PCH.h"
#include "SceneObject_Box.h"
#include "Math/Geometry.h"
#include "Rendering/ShadingData.h"


namespace rt {

using namespace math;

namespace helper
{

RT_FORCE_NOINLINE Int32 GetCubeSide(const Vector4& p)
{
    const Vector4 abs = Vector4::Abs(p);
    const int isXPositive = p.x > 0.0f ? 1 : 0;
    const int isYPositive = p.y > 0.0f ? 1 : 0;
    const int isZPositive = p.z > 0.0f ? 1 : 0;

    if (isXPositive && abs.x >= abs.y && abs.x >= abs.z) // +X
        return 0;
    else if (!isXPositive && abs.x >= abs.y && abs.x >= abs.z) // -X
        return 1;
    if (isYPositive && abs.y >= abs.x && abs.y >= abs.z) // +Y
        return 2;
    if (!isYPositive && abs.y >= abs.x && abs.y >= abs.z) // -Y
        return 3;
    if (isZPositive && abs.z >= abs.x && abs.z >= abs.y) // +Z
        return 4;
    if (!isZPositive && abs.z >= abs.x && abs.z >= abs.y) // -Z
        return 5;

    return 0;
}

RT_FORCE_NOINLINE Int32 ConvertXYZtoCubeUV(const Vector4& p, Vector4& outUV)
{
    const Vector4 abs = Vector4::Abs(p);
    const int isXPositive = p.x > 0 ? 1 : 0;
    const int isYPositive = p.y > 0 ? 1 : 0;
    const int isZPositive = p.z > 0 ? 1 : 0;
    float maxAxis = 0.0f, uc = 0.0f, vc = 0.0f;

    int side = 0;

    if (isXPositive && abs.x >= abs.y && abs.x >= abs.z) // +X
    {
        maxAxis = abs.x;
        uc = -p.z;
        vc = p.y;
        side = 0;
    }

    if (!isXPositive && abs.x >= abs.y && abs.x >= abs.z) // -X
    {
        maxAxis = abs.x;
        uc = p.z;
        vc = p.y;
        side = 1;
    }

    if (isYPositive && abs.y >= abs.x && abs.y >= abs.z) // +Y
    {
        maxAxis = abs.y;
        uc = p.x;
        vc = -p.z;
        side = 2;
    }

    if (!isYPositive && abs.y >= abs.x && abs.y >= abs.z) // -Y
    {
        maxAxis = abs.y;
        uc = p.x;
        vc = p.z;
        side = 3;
    }

    if (isZPositive && abs.z >= abs.x && abs.z >= abs.y) // +Z
    {
        maxAxis = abs.z;
        uc = p.x;
        vc = p.y;
        side = 4;
    }

    if (!isZPositive && abs.z >= abs.x && abs.z >= abs.y) // -Z
    {
        maxAxis = abs.z;
        uc = -p.x;
        vc = p.y;
        side = 5;
    }

    // Convert range from -1 to 1 to 0 to 1
    outUV = Vector4(uc, vc, 0.0f, 0.0f) / (2.0f * maxAxis) + Vector4(0.5f);

    return side;
}

} // helper


BoxSceneObject::BoxSceneObject(const Vector4& size, const Material* material)
    : mSize(size)
    , mMaterial(material)
{ }

Box BoxSceneObject::GetBoundingBox() const
{
    const Box localBox(mPosition - mSize, mPosition + mSize);

    // TODO include rotation
    return Box(localBox, localBox + mPositionOffset);
}

void BoxSceneObject::Traverse_Single(const Uint32 objectID, const Ray& ray, HitPoint& hitPoint) const
{
    const Box box(-mSize, mSize);

    float nearDist, farDist;
    if (Intersect_BoxRay_TwoSided(ray, box, nearDist, farDist))
    {
        if (nearDist > 0.0f && nearDist < hitPoint.distance)
        {
            const Vector4 cubePoint = (ray.origin + ray.dir * nearDist) / mSize;
            const Uint32 side = helper::GetCubeSide(cubePoint);
            if (hitPoint.filterObjectId != objectID || hitPoint.filterTriangleId != side)
            {
                hitPoint.distance = nearDist;
                hitPoint.objectId = objectID;
                hitPoint.triangleId = side;
                return;
            }
        }

        if (farDist > 0.0f && farDist < hitPoint.distance)
        {
            const Vector4 cubePoint = (ray.origin + ray.dir * farDist) / mSize;
            const Uint32 side = helper::GetCubeSide(cubePoint);
            if (hitPoint.filterObjectId != objectID || hitPoint.filterTriangleId != side)
            {
                hitPoint.distance = farDist;
                hitPoint.objectId = objectID;
                hitPoint.triangleId = side;
                return;
            }
        }
    }

    /*
    // one sided
    {
        if (objectID == hitPoint.filterObjectId)
        {
            return;
        }

        float dist;
        if (Intersect_BoxRay(ray, box, dist))
        {
            if (dist > 0.0f && dist < hitPoint.distance)
            {
                hitPoint.distance = dist;
                hitPoint.objectId = objectID;
                hitPoint.triangleId = 0;
            }
        }
    }
    */
}

void BoxSceneObject::Traverse_Simd8(const Ray_Simd8& ray, HitPoint_Simd8& outHitPoint) const
{
    (void)ray;
    (void)outHitPoint;
    // TODO
}

void BoxSceneObject::Traverse_Packet(const RayPacket& rayPacket, HitPoint_Packet& outHitPoint) const
{
    (void)rayPacket;
    (void)outHitPoint;
    // TODO
}

void BoxSceneObject::EvaluateShadingData_Single(const Matrix& worldToLocal, const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    RT_UNUSED(hitPoint);
    RT_UNUSED(worldToLocal);

    const Vector4 normalsAndTangnts[] = 
    {
        Vector4(+1.0f, 0.0f, 0.0f, 0.0f),   Vector4(0.0f, 0.0f, -1.0f, 0.0f),
        Vector4(-1.0f, 0.0f, 0.0f, 0.0f),   Vector4(0.0f, 0.0f, +1.0f, 0.0f),
        Vector4(0.0f, +1.0f, 0.0f, 0.0f),   Vector4(+1.0f, 0.0f, 0.0f, 0.0f),
        Vector4(0.0f, -1.0f, 0.0f, 0.0f),   Vector4(+1.0f, 0.0f, 0.0f, 0.0f),
        Vector4(0.0f, 0.0f, +1.0f, 0.0f),   Vector4(+1.0f, 0.0f, 0.0f, 0.0f),
        Vector4(0.0f, 0.0f, -1.0f, 0.0f),   Vector4(-1.0f, 0.0f, 0.0f, 0.0f),
    };

    outShadingData.material = mMaterial;

    const Int32 side = helper::ConvertXYZtoCubeUV(outShadingData.position / mSize, outShadingData.texCoord);
    outShadingData.normal = normalsAndTangnts[2 * side];
    outShadingData.tangent = normalsAndTangnts[2 * side + 1];
    outShadingData.bitangent = Vector4::Cross3(outShadingData.tangent, outShadingData.normal);

    outShadingData.normal.FastNormalize3();
    outShadingData.tangent.FastNormalize3();
    outShadingData.bitangent.FastNormalize3();
}


} // namespace rt
