#pragma once

#include "Shape.h"

namespace rt {

class RectShape : public IShape
{
public:
    RAYLIB_API RectShape(const math::Float2 size = math::Float2(FLT_MAX), const math::Float2 texScale = math::Float2(1.0f));

private:
    virtual const math::Box GetBoundingBox() const override;
    virtual float GetSurfaceArea() const override;
    virtual bool Intersect(const math::Ray& ray, ShapeIntersection& outResult) const override;
    virtual const math::Vector4 Sample(const math::Float3& u, math::Vector4* outNormal, float* outPdf = nullptr) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    math::Float2 mSize;
    math::Float2 mTextureScale;
};

} // namespace rt
