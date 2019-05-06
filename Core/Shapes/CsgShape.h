#pragma once

#include "Shape.h"

namespace rt {

enum class CsgOperator : Uint8
{
    Union,
    Difference,
    Intersection,
};

class CsgShape : public IShape
{
public:
    RAYLIB_API CsgShape();

    virtual const math::Box GetBoundingBox() const override;

    virtual bool Intersect(const math::Ray& ray, ShapeIntersection& outResult) const override;
    virtual const math::Vector4 Sample(const math::Float3& u, math::Vector4* outNormal, float* outPdf = nullptr) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

private:
    ShapePtr mShapeA;
    ShapePtr mShapeB;
    CsgOperator mOperator;
};

} // namespace rt
