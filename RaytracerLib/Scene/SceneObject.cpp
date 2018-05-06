#include "PCH.h"
#include "SceneObject.h"
#include "Mesh/Mesh.h"
#include "Math/Geometry.h"


namespace rt {

using namespace math;

ISceneObject::~ISceneObject() = default;

Matrix ISceneObject::GetTransform(const float t) const
{
    Matrix mat;
    mat.r[3] = mPosition + mPositionOffset * t;
    return mat;
}

Matrix ISceneObject::GetInverseTransform(const float t) const
{
    Matrix mat;
    mat.r[3] = -(mPosition + mPositionOffset * t);
    return mat;
}

///

MeshSceneObject::MeshSceneObject(const Mesh* mesh)
    : mMesh(mesh)
{ }

Box MeshSceneObject::GetBoundingBox() const
{
    const Box localBox = mMesh->GetBoundingBox();

    // TODO include rotation
    return Box(localBox + mPosition, localBox + (mPosition + mPositionOffset));
}

void MeshSceneObject::Traverse_Single(const Ray& ray, HitPoint& hitPoint) const
{
    mMesh->Traverse_Single(ray, hitPoint);
}

void MeshSceneObject::Traverse_Simd8(const Ray_Simd8& ray, HitPoint_Simd8& outHitPoint) const
{
    mMesh->Traverse_Simd8(ray, outHitPoint, 0); // TODO
}

void MeshSceneObject::EvaluateShadingData_Single(const Matrix& worldToLocal, const HitPoint& hitPoint, ShadingData& outShadingData) const
{
    RT_UNUSED(worldToLocal);

    mMesh->EvaluateShadingData_Single(hitPoint, outShadingData);
}

///

SphereSceneObject::SphereSceneObject(const float radius, const Material* material)
    : mRadius(radius)
    , mMaterial(material)
{ }

Box SphereSceneObject::GetBoundingBox() const
{
    const Vector4 radius = Vector4::Splat(mRadius);

    const Box start(mPosition - radius, mPosition + radius);
    const Box end(mPosition + mPositionOffset - radius, mPosition + mPositionOffset + radius);
    return Box(start, end);
}

void SphereSceneObject::Traverse_Single(const Ray& ray, HitPoint& hitPoint) const
{
    float dist;
    if (Intersect_RaySphere(ray, mRadius, dist))
    {
        if (dist > 0.0f && dist < hitPoint.distance)
        {
            hitPoint.distance = dist;
        }
    }
}

void SphereSceneObject::Traverse_Simd8(const Ray_Simd8& ray, HitPoint_Simd8& outHitPoint) const
{
    (void)ray;
    (void)outHitPoint;
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
