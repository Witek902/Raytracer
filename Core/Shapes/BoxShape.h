#pragma once

#include "Shape.h"

namespace rt {

class BoxShape : public IShape
{
public:
    RAYLIB_API BoxShape(const math::Vector4& size);

private:
    virtual const math::Box GetBoundingBox() const override;
    virtual float GetSurfaceArea() const override;
    virtual bool Intersect(const math::Ray& ray, ShapeIntersection& outResult) const override;
    virtual const math::Vector4 Sample(const math::Float3& u, math::Vector4* outNormal, float* outPdf = nullptr) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    // half size
    math::Vector4 mSize;
    math::Vector4 mInvSize;

    // unnormalized face distribution (for box face sampling)
    math::Float3 mFaceCdf;
};

} // namespace rt
