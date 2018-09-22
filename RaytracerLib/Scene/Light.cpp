#include "PCH.h"
#include "Light.h"
#include "../Rendering/Context.h"
#include "../Math/Geometry.h"

namespace rt {

using namespace math;

const Box PointLight::GetBoundingBox() const
{
    return Box(position, position);
}

bool PointLight::TestRayHit(const math::Ray& ray, Float& outDistance) const
{
    RT_UNUSED(ray);
    RT_UNUSED(outDistance);

    // we assume that a ray can never hit a point light source
    return false;
}

const Color PointLight::Illuminate(const Vector4& scenePoint, RenderingContext& context, Vector4& outDirectionToLight, float& outDistance) const
{
    outDirectionToLight = position - scenePoint;
    outDistance = outDirectionToLight.Length3();
    outDirectionToLight /= outDistance;

    return Color::SampleRGB(context.wavelength, color);
}

const Color PointLight::GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint) const
{
    RT_UNUSED(rayDirection);
    RT_UNUSED(hitPoint);
    RT_UNUSED(context);

    RT_FATAL("Cannot hit point light");
    return Color();
}

bool PointLight::IsFinite() const
{
    return true;
}

bool PointLight::IsDelta() const
{
    return true;
}

/////////////////////////////////////////////////////////////////

const Box AreaLight::GetBoundingBox() const
{
    Box box(p0, p0 + edge0, p0 + edge1);
    if (!isTriangle)
    {
        box.AddPoint(p0 + edge0 + edge1);
    }

    return box;
}

bool AreaLight::TestRayHit(const math::Ray& ray, Float& outDistance) const
{
    Float u, v; // unused

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

const Color AreaLight::Illuminate(const Vector4& scenePoint, RenderingContext& context, Vector4& outDirectionToLight, float& outDistance) const
{
    const Float2 uv = isTriangle ? context.randomGenerator.GetTriangle() : context.randomGenerator.GetFloat2();
    const Vector4 lightPoint = p0 + edge0 * uv.x + edge1 * uv.y;

    outDirectionToLight = lightPoint - scenePoint;
    outDistance = outDirectionToLight.Length3();
    outDirectionToLight /= outDistance;

    // TODO include light area

    return Color::SampleRGB(context.wavelength, color);
}

const Color AreaLight::GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint) const
{
    RT_UNUSED(rayDirection);
    RT_UNUSED(hitPoint);
    // TODO

    return Color::SampleRGB(context.wavelength, color);
}

bool AreaLight::IsFinite() const
{
    return true;
}

bool AreaLight::IsDelta() const
{
    return false;
}

/////////////////////////////////////////////////////////////////

const Box DirectionalLight::GetBoundingBox() const
{
    return Box::Empty();
}

bool DirectionalLight::TestRayHit(const math::Ray& ray, Float& outDistance) const
{
    RT_UNUSED(ray);
    RT_UNUSED(outDistance);

    // we assume that a ray can never hit a directional light source
    return false;
}

const Color DirectionalLight::Illuminate(const Vector4& scenePoint, RenderingContext& context, Vector4& outDirectionToLight, float& outDistance) const
{
    RT_UNUSED(context);
    RT_UNUSED(scenePoint);

    outDirectionToLight = -direction;
    outDistance = 1.0f;

    return Color::SampleRGB(context.wavelength, color);
}

const Color DirectionalLight::GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint) const
{
    RT_UNUSED(rayDirection);
    RT_UNUSED(hitPoint);
    RT_UNUSED(context);

    RT_FATAL("Cannot hit directinal");
    return Color();
}

bool DirectionalLight::IsFinite() const
{
    return false;
}

bool DirectionalLight::IsDelta() const
{
    return true;
}

} // namespace rt
