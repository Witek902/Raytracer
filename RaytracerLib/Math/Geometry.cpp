#include "PCH.h"
#include "Geometry.h"
#include "Box.h"
#include "Sphere.h"
#include "Triangle.h"


namespace rt {
namespace math {


// Box-box intersection test
template<> RAYLIB_API
bool Intersect(const Box& box1, const Box& box2)
{
    Vector4 temp0 = _mm_cmpge_ps(box1.min, box2.max);
    Vector4 temp1 = _mm_cmpge_ps(box2.min, box1.max);
    return (_mm_movemask_ps(_mm_or_ps(temp0, temp1)) & 7) == 0;
}

// Box-point intersection test
template<> RAYLIB_API
bool Intersect(const Box& box, const Vector4& point)
{
    Vector4 cmpMax = _mm_cmpge_ps(box.max, point);
    Vector4 cmpMin = _mm_cmpge_ps(point, box.min);
    return (_mm_movemask_ps(_mm_and_ps(cmpMax, cmpMin)) & 7) == 7;
}

template<> RAYLIB_API
IntersectionResult IntersectEx(const Box& box1, const Box& box2)
{
    Vector4 temp0 = _mm_cmpge_ps(box1.min, box2.max);
    Vector4 temp1 = _mm_cmpge_ps(box2.min, box1.max);
    if (_mm_movemask_ps(_mm_or_ps(temp0, temp1)) & 7)
        return IntersectionResult::Outside;

    temp0 = _mm_cmpge_ps(box1.min, box2.min);
    temp1 = _mm_cmpge_ps(box2.max, box1.max);
    if ((_mm_movemask_ps(_mm_and_ps(temp0, temp1)) & 7) == 7)
        return IntersectionResult::Inside;

    return IntersectionResult::Intersect;
}

// Point-sphere intersection test
template<> RAYLIB_API
bool Intersect(const Vector4& point, const Sphere& sphere)
{
    Vector4 segment = point - sphere.origin;
    return Vector4::Dot3(segment, segment) <= sphere.r * sphere.r;
}

// Sphere-sphere intersection test
template<> RAYLIB_API
bool Intersect(const Sphere& sphere1, const Sphere& sphere2)
{
    Vector4 segment = sphere1.origin - sphere2.origin;
    float radiiSum = sphere1.r + sphere2.r;
    return Vector4::Dot3(segment, segment) <= radiiSum * radiiSum;
}


// Ray-Box intersection functions =================================================================

template<> RAYLIB_API
bool Intersect(const Ray& ray, const Box& box)
{
    float unusedDist;
    return RayBoxIntersectInline(ray, box, unusedDist);
}

template<> RAYLIB_API
bool Intersect(const Ray& ray, const Box& box, float& dist)
{
    return RayBoxIntersectInline(ray, box, dist);
}


// Ray-Triangle intersection functions ============================================================

template<> RAYLIB_API
bool Intersect(const Ray& ray, const Triangle& tri)
{
    float unusedDist;
    return RayTriangleIntersectInline(ray, tri, unusedDist);
}

template<> RAYLIB_API
bool Intersect(const Ray& ray, const Triangle& tri, float& dist)
{
    return RayTriangleIntersectInline(ray, tri, dist);
}


// Ray-Sphere intersection functions ==============================================================

RT_FORCE_INLINE bool RaySphereIntersectInline(const Ray& ray, const Sphere& sphere, float& dist)
{
    Vector4 d = sphere.origin - ray.origin;
    float v = Vector4::Dot3(ray.dir, d);
    float det = sphere.r * sphere.r - Vector4::Dot3(d, d) + v * v;

    if (det > 0.0f)
    {
        dist = v - sqrtf(det);
        return true;
    }
    return false;
}

template<> RAYLIB_API
bool Intersect(const Ray& ray, const Sphere& sphere)
{
    float unusedDist;
    return RaySphereIntersectInline(ray, sphere, unusedDist);
}

template<> RAYLIB_API
bool Intersect(const Ray& ray, const Sphere& sphere, float& dist)
{
    return RaySphereIntersectInline(ray, sphere, dist);
}


} // namespace Math
} // namespace NFE