#include "PCH.h"
#include "AreaLight.h"
#include "../../Rendering/Context.h"
#include "../../Rendering/ShadingData.h"
#include "../../Math/Geometry.h"
#include "../../Math/SamplingHelpers.h"
#include "../../Utils/Texture.h"

namespace rt {

using namespace math;

AreaLight::AreaLight(const Vector4& p0, const Vector4& edge0, const Vector4& edge1, const Vector4& color)
    : ILight(color)
    , p0(p0)
    , edge0(edge0)
    , edge1(edge1)
{
    RT_ASSERT(p0.IsValid());
    RT_ASSERT(edge0.IsValid());
    RT_ASSERT(edge1.IsValid());
    RT_ASSERT(edge0.Length3() > 0.0f);
    RT_ASSERT(edge1.Length3() > 0.0f);
    RT_ASSERT(Abs(Vector4::Dot3(edge0.Normalized3(), edge1.Normalized3())) < 0.001f);

    edgeLengthInv0 = 1.0f / edge0.Length3();
    edgeLengthInv1 = 1.0f / edge1.Length3();

    const Vector4 cross = Vector4::Cross3(edge1, edge0);
    normal = cross.Normalized3();

    float surfaceArea = cross.Length3();
    if (isTriangle)
    {
        surfaceArea *= 0.5f;
    }

    invArea = 1.0f / surfaceArea;
}

const Box AreaLight::GetBoundingBox() const
{
    Box box(p0, p0 + edge0, p0 + edge1);
    if (!isTriangle)
    {
        box.AddPoint(p0 + edge0 + edge1);
    }

    return box;
}

bool AreaLight::TestRayHit(const Ray& ray, float& outDistance) const
{
    float u, v; // unused

    if (Intersect_TriangleRay(ray, p0, edge0, edge1, u, v, outDistance))
    {
        return true;
    }

    if (!isTriangle)
    {
        const Vector4 oppositePoint = p0 + edge0 + edge1;

        if (Intersect_TriangleRay(ray, oppositePoint, -edge0, -edge1, u, v, outDistance))
        {
            return true;
        }
    }

    return false;
}

const RayColor AreaLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    const Vector4 uv = isTriangle ? SamplingHelpers::GetTriangle(param.sample) : Vector4(param.sample);

    Spectrum color = mColor;

    // sample texture map
    if (mTexture)
    {
        color.rgbValues *= mTexture->Evaluate(uv, SamplerDesc());
    }

    // p0 + edge0 * uv.x + edge1 * uv.y;
    const Vector4 lightPoint = Vector4::MulAndAdd(edge0, uv.x, Vector4::MulAndAdd(edge1, uv.y, p0));

    outResult.directionToLight = lightPoint - param.shadingData.frame.GetTranslation();
    const float sqrDistance = outResult.directionToLight.SqrLength3();

    outResult.distance = sqrtf(sqrDistance);
    outResult.directionToLight /= outResult.distance;

    const float cosNormalDir = Vector4::Dot3(normal, -outResult.directionToLight);
    if (cosNormalDir < RT_EPSILON)
    {
        return RayColor::Zero();
    }

    outResult.cosAtLight = cosNormalDir;
    outResult.directPdfW = invArea * sqrDistance / cosNormalDir;
    outResult.emissionPdfW = cosNormalDir * invArea * RT_INV_PI;

    return RayColor::Resolve(param.wavelength, color);
}

const RayColor AreaLight::GetRadiance(RenderingContext& context, const Ray& ray, const Vector4& hitPoint, float* outDirectPdfA, float* outEmissionPdfW) const
{
    const float cosNormalDir = Vector4::Dot3(normal, -ray.dir);
    if (cosNormalDir < RT_EPSILON)
    {
        return RayColor::Zero();
    }

    if (outDirectPdfA)
    {
        *outDirectPdfA = invArea;
    }

    if (outEmissionPdfW)
    {
        *outEmissionPdfW = cosNormalDir * invArea * RT_INV_PI;
    }

    Spectrum color = mColor;

    if (mTexture)
    {
        const Vector4 lightSpaceHitPoint = hitPoint - p0;
        const float u = Vector4::Dot3(lightSpaceHitPoint, edge0 * edgeLengthInv0) * edgeLengthInv0;
        const float v = Vector4::Dot3(lightSpaceHitPoint, edge1 * edgeLengthInv1) * edgeLengthInv1;
        const Vector4 textureCoords(u, v, 0.0f, 0.0f);

        color.rgbValues *= mTexture->Evaluate(textureCoords, SamplerDesc());
    }

    return RayColor::Resolve(context.wavelength, color);
}

const RayColor AreaLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    // generate random point on the light surface
    const Vector4 uv = isTriangle ? SamplingHelpers::GetTriangle(param.sample) : Vector4(param.sample);
    // p0 + edge0 * uv.x + edge1 * uv.y;
    outResult.position = Vector4::MulAndAdd(edge0, uv.x, Vector4::MulAndAdd(edge1, uv.y, p0));

    // generate random direction
    Vector4 dirLocalSpace = SamplingHelpers::GetHemishpereCos(param.sample2);
    dirLocalSpace.z = Max(dirLocalSpace.z, 0.001f);
    outResult.direction = dirLocalSpace.x * edge0 * edgeLengthInv0 + dirLocalSpace.y * edge1 * edgeLengthInv1 + dirLocalSpace.z * normal;

    outResult.cosAtLight = dirLocalSpace.z;
    outResult.directPdfA = invArea;
    outResult.emissionPdfW = invArea * dirLocalSpace.z * RT_INV_PI;

    Spectrum color = mColor;

    // sample texture map
    if (mTexture)
    {
        color.rgbValues *= mTexture->Evaluate(Vector4(uv), SamplerDesc());
    }

    // TODO texture
    return RayColor::Resolve(param.wavelength, color) * dirLocalSpace.z;
}

const Vector4 AreaLight::GetNormal(const Vector4&) const
{
    return normal;
}

bool AreaLight::IsFinite() const
{
    return true;
}

bool AreaLight::IsDelta() const
{
    return false;
}

} // namespace rt
