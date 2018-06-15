#include "PCH.h"
#include "SceneObject_Sphere.h"
#include "Math/Geometry.h"
#include "Rendering/ShadingData.h"


namespace rt {

using namespace math;


SphereSceneObject::SphereSceneObject(const float radius, const Material* material)
    : mRadius(radius)
    , mMaterial(material)
{ }

Box SphereSceneObject::GetBoundingBox() const
{
    const Vector4 radius = Vector4(mRadius, mRadius, mRadius, 0.0f);

    const Box localBox(mPosition - radius, mPosition + radius);

    // TODO include rotation
    return Box(localBox, localBox + mPositionOffset);
}

void SphereSceneObject::Traverse_Single(const Uint32 objectID, const Ray& ray, HitPoint& hitPoint) const
{
    if (hitPoint.filterObjectId == objectID)
    {
        return;
    }

    float dist;
    if (Intersect_RaySphere(ray, mRadius, dist))
    {
        if (dist > 0.0f && dist < hitPoint.distance)
        {
            hitPoint.distance = dist;
            hitPoint.objectId = objectID;
            hitPoint.triangleId = 0;
        }
    }
}

void SphereSceneObject::Traverse_Simd8(const Ray_Simd8& ray, HitPoint_Simd8& outHitPoint) const
{
    (void)ray;
    (void)outHitPoint;
    // TODO
}

void SphereSceneObject::Traverse_Packet(const RayPacket& rayPacket, HitPoint_Packet& outHitPoint) const
{
    (void)rayPacket;
    (void)outHitPoint;
    // TODO
}

void SphereSceneObject::EvaluateShadingData_Single(const Matrix& worldToLocal, const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    RT_UNUSED(hitPoint);

    outShadingData.material = mMaterial;

    outShadingData.texCoord = Vector4(); // TODO
    outShadingData.normal = outShadingData.position + worldToLocal.GetRow(3);
    outShadingData.tangent = Vector4::Cross3(outShadingData.normal, VECTOR_Y);
    outShadingData.bitangent = Vector4::Cross3(outShadingData.tangent, outShadingData.normal);

    outShadingData.normal.FastNormalize3();
    outShadingData.tangent.FastNormalize3();
    outShadingData.bitangent.FastNormalize3();
}


} // namespace rt
