#include "PCH.h"
#include "Scene.h"
#include "Bitmap.h"
#include "Camera.h"
#include "Math/Triangle.h" // TODO remove
#include "Math/Geometry.h" // TODO remove


namespace rt {

using namespace math;

bool Scene::Raytrace(const Camera& camera, Bitmap& renderTarget, const RaytracingParams& params)
{
    // TODO parameter verification

    Raytrace_Single(camera, renderTarget, params);

    return true;
}

void Scene::Raytrace_Single(const Camera& camera, Bitmap& renderTarget, const RaytracingParams& params)
{
    // TODO split task into packets and execute on all threads

    const Uint32 width = renderTarget.GetWidth();
    const Uint32 height = renderTarget.GetHeight();

    const Float invWidth = 1.0f / (Float)width;
    const Float invHeight = 1.0f / (Float)height;

    RayTracingCounters counters;

    Float yCoord = 0.0f;
    for (Uint32 y = 0; y < height; ++y)
    {
        yCoord += invHeight;

        Float xCoord = 0.0f;
        for (Uint32 x = 0; x < width; ++x)
        {
            xCoord += invWidth;

            // TODO Monte Carlo ray generation:
            // depth of field
            // antialiasing
            // motion blur

            // generate ray
            const math::Ray cameraRay = camera.GenerateRay(xCoord, yCoord);

            RayTracingContext context(mRandomGenerator, params, counters);
            counters.numPrimaryRays++;
            math::Vector color = TraceRay_Single(cameraRay, context);

            renderTarget.SetPixel(x, y, color);
        }
    }
}

math::Vector Scene::TraceRay_Single(const math::Ray& ray, RayTracingContext& context) const
{
    RT_UNUSED(context);

    /*
    Triangle triangle(Vector(-1.0f, -0.5f, 0.0f), Vector(1.0f, -0.5f, 0.0f), Vector(0.0f, 0.5f, 0.0f));
    if (math::Intersect(ray, triangle))
    {
        return Vector(1.0f, 1.0f, 1.0f);
    }
    */

    Box box(Vector(-10.0f, -2.0f, -10.0f), Vector(10.0f, -0.5f, 10.0f));
    if (math::Intersect(ray, box))
    {
        return Vector(0.5f, 0.5f, 0.5f);
    }

    return Vector();
}

} // namespace rt
