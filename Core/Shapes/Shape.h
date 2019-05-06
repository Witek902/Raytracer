#pragma once

#include "../RayLib.h"
#include "../Math/Box.h"
#include "../Math/Matrix4.h"
#include "../Utils/AlignmentAllocator.h"
#include "../Traversal/HitPoint.h"

#include <memory>

namespace rt {

struct HitPoint;
struct IntersectionData;
struct SingleTraversalContext;

class Material;
using MaterialPtr = std::shared_ptr<rt::Material>;

struct ShapeIntersection
{
    float nearDist;
    float farDist;
    Uint32 subObjectId = UINT32_MAX;
};

struct ShapeSampleResult
{
    math::Vector4 position;
    math::Vector4 normal;
    math::Vector4 direction;
    float distance = -1.0f;
    float pdf = -1.0f;
    float cosAtSurface = -1.0f;
};

class IShape : public Aligned<16>
{
public:
    RAYLIB_API IShape();
    RAYLIB_API virtual ~IShape();

    // get total surface area
    virtual float GetSurfaceArea() const;

    // traverse the object and find nearest intersection
    virtual void Traverse(const SingleTraversalContext& context, const Uint32 objectID) const;

    // traverse the object and check if the ray is occluded
    virtual bool Traverse_Shadow(const SingleTraversalContext& context) const;

    // intersect with a ray and return hit points
    // TODO return array of all hit points along the ray
    virtual bool Intersect(const math::Ray& ray, ShapeIntersection& outResult) const;

    // generate random point on the shape's surface
    // optionaly returns normal vector and sampling probability (with respect to area on the surface)
    virtual const math::Vector4 Sample(const math::Float3& u, math::Vector4* outNormal = nullptr, float* outPdf = nullptr) const = 0;

    // generate random point on the shape's surface for given reference point
    // optionaly returns normal vector and sampling probability (with respect to solid angle visible from ref point)
    virtual bool Sample(const math::Vector4& ref, const math::Float3& u, ShapeSampleResult& result) const;
    virtual float Pdf(const math::Vector4& ref, const math::Vector4& point) const;

    // Calculate intersection data (tangent frame, tex coords, etc.) at given intersection point
    // NOTE: all calculations are performed in local space
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const = 0;

    // Get world-space bounding box
    virtual const math::Box GetBoundingBox() const = 0;
};

using ShapePtr = std::shared_ptr<IShape>;

} // namespace rt
