#include "PCH.h"
#include "RectShape.h"
#include "Rendering/ShadingData.h"
#include "Traversal/TraversalContext.h"
#include "Math/Geometry.h"
#include "Math/SamplingHelpers.h"

namespace rt {

using namespace math;

RectShape::RectShape(const Float2 size, const Float2 texScale)
    : mSize(size)
    , mTextureScale(texScale)
{
    RT_ASSERT(mSize.x > 0.0f);
    RT_ASSERT(mSize.y > 0.0f);
}

const Box RectShape::GetBoundingBox() const
{
    return Box(Vector4(-mSize.x, -mSize.y, 0.0f, 0.0f), Vector4(mSize.x, mSize.y, 0.0f, 0.0f));
}

float RectShape::GetSurfaceArea() const
{
    return 4.0f * mSize.x * mSize.y;
}

bool RectShape::Intersect(const math::Ray& ray, ShapeIntersection& outResult) const
{
    const float t = -ray.origin.z * ray.invDir.z;

    if (t > FLT_EPSILON)
    {
        const Vector4 pos = ray.GetAtDistance(t);

        if ((Vector4::Abs(pos) < Vector4(mSize)).GetMask() == 0x3)
        {
            outResult.nearDist = t;
            outResult.farDist = t;
            return true;
        }
    }

    return false;
}

const Vector4 RectShape::Sample(const Float3& u, math::Vector4* outNormal, float* outPdf) const
{
    if (outPdf)
    {
        *outPdf = 1.0f / GetSurfaceArea();
    }

    if (outNormal)
    {
        *outNormal = VECTOR_Z;
    }

    return Vector4(mSize) * (2.0f * Vector4(Float2(u)) - VECTOR_ONE);
}

/*
void PlaneSceneObject::Traverse_Packet(const PacketTraversalContext& context, const uint32 objectID, const uint32 numActiveGroups) const
{
    for (uint32 i = 0; i < numActiveGroups; ++i)
    {
        RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[i]];

        const Vector8 t = -rayGroup.rays[1].origin.y * rayGroup.rays[1].invDir.y;
        VectorBool8 mask = (t > Vector8::Zero()) & (t < rayGroup.maxDistances);

        if (mask.None())
        {
            continue;
        }

        const Vector8 x = Vector8::MulAndAdd(rayGroup.rays[1].dir.x, t, rayGroup.rays[1].origin.x);
        const Vector8 z = Vector8::MulAndAdd(rayGroup.rays[1].dir.z, t, rayGroup.rays[1].origin.z);
        mask = mask & (Vector8::Abs(x) < Vector8(mSize.x)) & (Vector8::Abs(z) < Vector8(mSize.y));

        const Vector8 u = Vector8::Zero(); // TODO
        const Vector8 v = Vector8::Zero(); // TODO

        context.StoreIntersection(rayGroup, t, u, v, mask, objectID);
    }
}
*/

void RectShape::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outData) const
{
    RT_UNUSED(hitPoint);

    outData.texCoord = (outData.frame.GetTranslation() & Vector4::MakeMask<1, 1, 0, 0>()) * Vector4(mTextureScale);
    outData.frame[0] = VECTOR_X;
    outData.frame[1] = VECTOR_Y;
    outData.frame[2] = VECTOR_Z;
}


} // namespace rt
