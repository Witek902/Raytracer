#pragma once

#include "Shape.h"

namespace rt {

class SphereShape : public IShape
{
public:
    RAYLIB_API SphereShape(const float radius);

private:
    virtual const math::Box GetBoundingBox() const override;
    virtual float GetSurfaceArea() const override;
    virtual bool Intersect(const math::Ray& ray, ShapeIntersection& outResult) const override;
    virtual const math::Vector4 Sample(const math::Float3& u, math::Vector4* outNormal, float* outPdf) const override;
    virtual bool Sample(const math::Vector4& ref, const math::Float3& u, ShapeSampleResult& result) const override;
    virtual float Pdf(const math::Vector4& ref, const math::Vector4& point) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    double mRadiusD;
    float mRadius;
    float mInvRadius;
};

} // namespace rt
