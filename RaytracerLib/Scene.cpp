#include "PCH.h"
#include "Scene.h"
#include "Bitmap.h"
#include "Camera.h"
#include "Math/Triangle.h" // TODO remove
#include "Math/Geometry.h" // TODO remove


#define RT_FILL_RATE_TEST 0

namespace rt {

using namespace math;

bool Scene::Raytrace(const Camera& camera, Bitmap& renderTarget, const RaytracingParams& params)
{
    // TODO parameter verification

    RaytraceSingle(camera, renderTarget, params);

    return true;
}

void Scene::RaytraceSingle(const Camera& camera, Bitmap& renderTarget, const RaytracingParams& params)
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

#if RT_FILL_RATE_TEST

            // just filling with white noise
            renderTarget.SetPixel(x, y, mRandomGenerator.GetVector4());

#else // !RT_FILL_RATE_TEST

            // TODO Monte Carlo ray generation:
            // depth of field
            // antialiasing
            // motion blur

            // generate ray
            const math::Ray cameraRay = camera.GenerateRay(xCoord, yCoord);

            RayTracingContext context(mRandomGenerator, params, counters);
            math::Vector color = TraceRaySingle(cameraRay, context);

            renderTarget.SetPixel(x, y, color);

#endif RT_FILL_RATE_TEST
        }
    }
}

math::Vector Scene::TraceRaySingle(const math::Ray& ray, RayTracingContext& context)
{
    RT_UNUSED(context);

    Triangle triangle(Vector(-1.0f, -0.5f, 0.0f), Vector(1.0f, -0.5f, 0.0f), Vector(0.0f, 0.5f, 0.0f));
    if (math::Intersect(ray, triangle))
    {
        return Vector(1.0f, 1.0f, 1.0f);
    }

    /*
    Box box(Vector(-10.0f, -2.0f, -10.0f), Vector(10.0f, -0.5f, 10.0f));
    if (math::Intersect(ray, box))
    {
        return Vector(0.5f, 0.5f, 0.5f);
    }
    */

    return Vector();
}

} // namespace rt
