#include "PCH.h"
#include "BoxShape.h"
#include "Math/Geometry.h"
#include "Math/Simd8Geometry.h"
#include "Rendering/ShadingData.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;

namespace helper
{

const Matrix4 g_faceFrames[] =
{
    { { 0.0f, 0.0f,  1.0f, 0.0f}, { 0.0f, 1.0f,  0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f, 0.0f}, { 0.0f,  0.0f, -1.0f, 0.0f} },
    { { 0.0f, 0.0f, -1.0f, 0.0f}, { 0.0f, 1.0f,  0.0f, 0.0f}, {+1.0f,  0.0f,  0.0f, 0.0f}, { 0.0f,  0.0f,  1.0f, 0.0f} },
    { {+1.0f, 0.0f,  0.0f, 0.0f}, { 0.0f, 0.0f,  1.0f, 0.0f}, { 0.0f, -1.0f,  0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f, 0.0f} },
    { {+1.0f, 0.0f,  0.0f, 0.0f}, { 0.0f, 0.0f, -1.0f, 0.0f}, { 0.0f, +1.0f,  0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f, 0.0f} },
    { {-1.0f, 0.0f,  0.0f, 0.0f}, { 0.0f, 1.0f,  0.0f, 0.0f}, { 0.0f,  0.0f, -1.0f, 0.0f}, { 1.0f,  0.0f,  0.0f, 0.0f} },
    { {+1.0f, 0.0f,  0.0f, 0.0f}, { 0.0f, 1.0f,  0.0f, 0.0f}, { 0.0f,  0.0f, +1.0f, 0.0f}, {-1.0f,  0.0f,  0.0f, 0.0f} },
};

RT_FORCE_INLINE int32 ConvertXYZtoCubeUV(const Vector4& p, Vector4& outUV)
{
    const Vector4 abs = Vector4::Abs(p);
    const int32 isXPositive = p.x > 0 ? 1 : 0;
    const int32 isYPositive = p.y > 0 ? 1 : 0;
    const int32 isZPositive = p.z > 0 ? 1 : 0;

    float maxAxis, uc, vc;
    int32 side;

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


BoxShape::BoxShape(const Vector4& size)
    : mSize(size)
    , mInvSize(VECTOR_ONE / mSize)
{
    RT_ASSERT(mSize.x > 0.0f);
    RT_ASSERT(mSize.y > 0.0f);
    RT_ASSERT(mSize.z > 0.0f);

    mSize.w = 0.0f;
    mInvSize.w = 0.0f;

    {
        mFaceCdf.x = mSize.y * mSize.z;
        mFaceCdf.y = mFaceCdf.x + mSize.z * mSize.x;
        mFaceCdf.z = mFaceCdf.y + mSize.x * mSize.y;
    }
}

const Box BoxShape::GetBoundingBox() const
{
    return Box(-mSize, mSize);
}

float BoxShape::GetSurfaceArea() const
{
    return 8.0f * (mSize.x * (mSize.y + mSize.z) + mSize.y * mSize.z);
}

bool BoxShape::Intersect(const math::Ray& ray, ShapeIntersection& outResult) const
{
    const Box box(-mSize, mSize);

    outResult.subObjectId = 0;

    return Intersect_BoxRay_TwoSided(ray, box, outResult.nearDist, outResult.farDist);
}

const Vector4 BoxShape::Sample(const Float3& u, math::Vector4* outNormal, float* outPdf) const
{
    float v = u.z;

    // select dimension for the normal vector (Z axis in local space)
    uint32 zAxis;
    {
        // TODO could be optimized by storing normalized CDF

        v *= mFaceCdf.z;
        if (v < mFaceCdf.x)
        {
            v /= mFaceCdf.x;
            zAxis = 0;
        }
        else if (v < mFaceCdf.y)
        {
            v = (v - mFaceCdf.x) / (mFaceCdf.y - mFaceCdf.x);
            zAxis = 1;
        }
        else
        {
            v = (v - mFaceCdf.y) / (mFaceCdf.z - mFaceCdf.y);
            zAxis = 2;
        }
    }

    // compute remaining axes
    const uint32 xAxis = (zAxis + 1) % 3u;
    const uint32 yAxis = (zAxis + 2) % 3u;

    // generate normal vector (2 possible directions for already chosen axis)
    Vector4 normal = Vector4::Zero();
    normal[zAxis] = v < 0.5f ? -1.0f : 1.0f;

    // generate position by filling up remaining coordinates
    Vector4 pos = Vector4::Zero();
    pos[xAxis] = (2.0f * u.x - 1.0f) * mSize[xAxis];
    pos[yAxis] = (2.0f * u.y - 1.0f) * mSize[yAxis];
    pos[zAxis] = normal[zAxis] * mSize[zAxis];

    if (outPdf)
    {
        *outPdf = 1.0f / GetSurfaceArea();
    }

    if (outNormal)
    {
        *outNormal = normal;
    }

    return pos;
}

void BoxShape::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outData) const
{
    using namespace helper;

    RT_UNUSED(hitPoint);

    const int32 side = ConvertXYZtoCubeUV(outData.frame.GetTranslation() * mInvSize, outData.texCoord);
    outData.frame[0] = g_faceFrames[side][0];
    outData.frame[1] = g_faceFrames[side][1];
    outData.frame[2] = g_faceFrames[side][2];
}


} // namespace rt
