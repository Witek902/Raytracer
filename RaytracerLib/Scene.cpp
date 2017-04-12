#include "PCH.h"
#include "Scene.h"
#include "Bitmap.h"
#include "Camera.h"
#include "Math/Triangle.h" // TODO remove
#include "Math/Geometry.h" // TODO remove


namespace rt {

using namespace math;

bool Scene::Raytrace(const Camera& camera, Bitmap& renderTarget, const RaytracingParams& params) const
{
    // TODO parameter verification

    RaytraceSingle(camera, renderTarget, params);

    return true;
}

void Scene::RaytraceSingle(const Camera& camera, Bitmap& renderTarget, const RaytracingParams& params) const
{
    const Uint32 width = renderTarget.GetWidth();
    const Uint32 height = renderTarget.GetHeight();

    const Float invWidth = 1.0f / (Float)width;
    const Float invHeight = 1.0f / (Float)height;

    RayTracingCounters counters;

    for (Uint32 y = 0; y < height; ++y)
    {
        const Float yCoord = (Float)y * invHeight;

        for (Uint32 x = 0; x < width; ++x)
        {
            const Float xCoord = (Float)x * invWidth;

            // TODO Monte Carlo ray generation:
            // depth of field
            // antialiasing
            // motion blur

            // generate ray
            const math::Ray cameraRay = camera.GenerateRay(xCoord, yCoord);

            RayTracingContext context(mRandomGenerator, params, counters);
            const math::Vector color = TraceRaySingle(cameraRay, context);

            renderTarget.SetPixel(x, y, color);
        }
    }
}

math::Vector Scene::TraceRaySingle(const math::Ray& ray, RayTracingContext& context) const
{
    RT_UNUSED(context);

    /*
    Triangle triangle(Vector(-1.0f, -0.5f, 0.0f), Vector(1.0f, -0.5f, 0.0f), Vector(0.0f, 0.5f, 0.0f));
    if (math::Intersect(ray, triangle))
    {
        return math::VECTOR_ONE;
    }
    */

    Box box(Vector(-1.0f, -1.0f, -1.0f), Vector(1.0f, 1.0f, 1.0f));
    if (math::Intersect(ray, box))
    {
        return math::VECTOR_ONE;
    }

    return Vector();
}

} // namespace rt
