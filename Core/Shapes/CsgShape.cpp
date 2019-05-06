#include "PCH.h"
#include "CsgShape.h"
#include "Math/Geometry.h"
#include "Rendering/ShadingData.h"
#include "Traversal/TraversalContext.h"

#include "BoxShape.h"
#include "SphereShape.h"

namespace rt {

using namespace math;

CsgShape::CsgShape()
    : mOperator(CsgOperator::Union)
{
    mShapeA = std::make_unique<BoxShape>(Vector4(0.5f));
    mShapeB = std::make_unique<SphereShape>(0.5f);
    mOperator = CsgOperator::Intersection;
}

const Box CsgShape::GetBoundingBox() const
{
    return Box::Empty();
}

bool CsgShape::Intersect(const math::Ray& ray, ShapeIntersection& outResult) const
{
    ShapeIntersection intersectionA, intersectionB;

    if (!mShapeA->Intersect(ray, intersectionA))
    {
        intersectionA.nearDist = FLT_MAX;
        intersectionA.farDist = -FLT_MAX;
    }

    {
        math::Ray transformedRay = ray;
        transformedRay.origin -= Vector4(0.25f, 0.5f, 0.1f); // TODO

        if (!mShapeB->Intersect(transformedRay, intersectionB))
        {
            intersectionB.nearDist = FLT_MAX;
            intersectionB.farDist = -FLT_MAX;
        }
    }

    if (mOperator == CsgOperator::Union)
    {
        // TODO this is completely wrong:
        // union may generate one or two intervals

        if (intersectionA.farDist > 0.0f && intersectionB.farDist <= 0.0f)
        {
            outResult = intersectionA;
        }
        else if (intersectionB.farDist > 0.0f && intersectionA.farDist <= 0.0f)
        {
            outResult = intersectionB;
        }
        else
        {
            outResult.nearDist = Min(intersectionA.nearDist, intersectionB.nearDist);
            outResult.farDist = Max(intersectionA.farDist, intersectionB.farDist);
        }
    }
    else if (mOperator == CsgOperator::Intersection)
    {
        outResult.nearDist = Max(intersectionA.nearDist, intersectionB.nearDist);
        outResult.farDist = Min(intersectionA.farDist, intersectionB.farDist);
    }
    else if (mOperator == CsgOperator::Difference)
    {
        // TODO
        RT_FATAL("Not implemented");
    }
    else
    {
        RT_FATAL("Invalid CSG operator");
    }

    // store object we hit
    if (outResult.nearDist == intersectionA.nearDist)
    {
        outResult.subObjectId = 0;
    }
    else
    {
        outResult.subObjectId = 1;
    }

    return outResult.nearDist < outResult.farDist;
}

const Vector4 CsgShape::Sample(const Float3& u, math::Vector4* outNormal, float* outPdf) const
{
    RT_FATAL("Not implemented");

    RT_UNUSED(u);
    RT_UNUSED(outPdf);
    RT_UNUSED(outNormal);

    return Vector4::Zero();
}

void CsgShape::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outData) const
{
    const IShape* hitShape = nullptr;

    if (hitPoint.subObjectId == 0)
    {
        hitShape = mShapeA.get();
    }
    else if (hitPoint.subObjectId == 1)
    {
        hitShape = mShapeB.get();
    }
    else
    {
        RT_FATAL("Invalid sub-object");
    }

    if (hitPoint.subObjectId == 1)
    {
        outData.frame[3] -= Vector4(0.25f, 0.5f, 0.1f);
    }

    hitShape->EvaluateIntersection(hitPoint, outData);

    //if (hitPoint.subObjectId == 1)
    //{
    //    outShadingData.frame[3] -= Vector4(0.5f);
    //}
}


} // namespace rt
