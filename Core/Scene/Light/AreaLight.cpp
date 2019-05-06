#include "PCH.h"
#include "AreaLight.h"
#include "../../Rendering/Context.h"
#include "../../Rendering/ShadingData.h"
#include "../../Math/Geometry.h"
#include "../../Math/SamplingHelpers.h"
#include "../../Textures/Texture.h"
#include "../../Shapes/Shape.h"

namespace rt {

using namespace math;

AreaLight::AreaLight(ShapePtr shape, const Vector4& color)
    : ILight(color)
    , mShape(std::move(shape))
{
}

ILight::Type AreaLight::GetType() const
{
    return Type::Area;
}

const Box AreaLight::GetBoundingBox() const
{
    return mShape->GetBoundingBox();
}

bool AreaLight::TestRayHit(const Ray& ray, float& outDistance) const
{
    ShapeIntersection intersection;

    if (mShape->Intersect(ray, intersection))
    {
        outDistance = intersection.nearDist;
        return true;
    }

    return false;
}

const RayColor AreaLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    Spectrum color = GetColor();

    // TODO
    // sample texture map
    //if (mTexture)
    //{
    //    float pdf;
    //    const Vector4 textureColor = mTexture->Sample(param.sample, uv, &pdf);
    //    color.rgbValues *= textureColor / pdf;
    //}
    //else
    //{
    //    uv = Vector4(param.sample);
    //}

    if (param.rendererSupportsSolidAngleSampling)
    {
        const Vector4 ref = param.worldToLight.TransformPoint(param.intersection.frame.GetTranslation());

        ShapeSampleResult sampleResult;
        if (!mShape->Sample(ref, param.sample, sampleResult))
        {
            return RayColor::Zero();
        }

        outResult.directionToLight = param.lightToWorld.TransformVector(sampleResult.direction);
        outResult.distance = sampleResult.distance;
        outResult.cosAtLight = sampleResult.cosAtSurface;
        outResult.directPdfW = sampleResult.pdf;

        RT_ASSERT(IsValid(outResult.directPdfW));
        RT_ASSERT(outResult.directPdfW >= 0.0f);
    }
    else
    {
        // generate random point on the light surface
        Vector4 normal;
        const Vector4 samplePositionLocalSpace = mShape->Sample(param.sample, &normal);
        const Vector4 lightPointWorldSpace = param.lightToWorld.TransformPoint(samplePositionLocalSpace);

        outResult.directionToLight = lightPointWorldSpace - param.intersection.frame.GetTranslation();
        const float sqrDistance = outResult.directionToLight.SqrLength3();

        outResult.distance = sqrtf(sqrDistance);
        outResult.directionToLight /= outResult.distance;

        const float cosNormalDir = Vector4::Dot3(-normal, outResult.directionToLight);
        if (cosNormalDir < RT_EPSILON)
        {
            return RayColor::Zero();
        }

        const float invArea = 1.0f / mShape->GetSurfaceArea();
        outResult.cosAtLight = cosNormalDir;
        outResult.directPdfW = invArea * sqrDistance / cosNormalDir;
        outResult.emissionPdfW = cosNormalDir * invArea * RT_INV_PI;
    }

    return RayColor::Resolve(param.wavelength, color);
}

const RayColor AreaLight::GetRadiance(const RadianceParam& param, float* outDirectPdfA, float* outEmissionPdfW) const
{
    if (param.cosAtLight < RT_EPSILON)
    {
        return RayColor::Zero();
    }

    const float invArea = 1.0f / mShape->GetSurfaceArea();

    if (outDirectPdfA)
    {
        if (param.rendererSupportsSolidAngleSampling)
        {
            *outDirectPdfA = mShape->Pdf(param.ray.origin, param.hitPoint);
        }
        else
        {
            *outDirectPdfA = invArea;
        }
    }

    if (outEmissionPdfW)
    {
        *outEmissionPdfW = param.cosAtLight * invArea * RT_INV_PI;
    }

    Spectrum color = GetColor();

    // TODO
    //if (mTexture)
    //{
    //    const Vector4 lightSpaceHitPoint = hitPoint - p0;
    //    const float u = Vector4::Dot3(lightSpaceHitPoint, edge0 * edgeLengthInv0) * edgeLengthInv0;
    //    const float v = Vector4::Dot3(lightSpaceHitPoint, edge1 * edgeLengthInv1) * edgeLengthInv1;
    //    const Vector4 textureCoords(u, v, 0.0f, 0.0f);

    //    color.rgbValues *= mTexture->Evaluate(textureCoords);
    //}

    return RayColor::Resolve(param.context.wavelength, color);
}

const RayColor AreaLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    // TODO sample texture, like in Illuminate()

    // generate random point on the light surface
    Vector4 normal;
    const Vector4 samplePositionLocalSpace = mShape->Sample(param.positionSample, &normal);
    outResult.position = param.lightToWorld.TransformPoint(samplePositionLocalSpace);

    Vector4 tangent, bitangent;
    BuildOrthonormalBasis(normal, tangent, bitangent);

    // generate random direction
    const Vector4 randomDir = SamplingHelpers::GetHemishpereCos(param.directionSample);
    const Vector4 dirLocalSpace = randomDir.x * tangent + randomDir.y * bitangent + randomDir.z * normal;
    outResult.direction = param.lightToWorld.TransformVector(dirLocalSpace);

    const float cosAtLight = randomDir.z;
    const float invArea = 1.0f / mShape->GetSurfaceArea();
    outResult.cosAtLight = cosAtLight;
    outResult.directPdfA = invArea;
    outResult.emissionPdfW = invArea * cosAtLight * RT_INV_PI;

    Spectrum color = GetColor();

    // TODO
    // sample texture map
    //if (mTexture)
    //{
    //    color.rgbValues *= mTexture->Evaluate(Vector4(uv));
    //}

    return RayColor::Resolve(param.wavelength, color) * cosAtLight;
}

ILight::Flags AreaLight::GetFlags() const
{
    return Flag_IsFinite;
}

} // namespace rt
