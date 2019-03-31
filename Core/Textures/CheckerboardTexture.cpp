#include "PCH.h"
#include "CheckerboardTexture.h"
#include "Color/ColorHelpers.h"

namespace rt {

using namespace math;

CheckerboardTexture::CheckerboardTexture(const math::Vector4& colorA, math::Vector4& colorB)
    : mColorA(colorA)
    , mColorB(colorB)
    , mPdf(0.5f)
{
    RT_ASSERT(colorA.IsValid());
    RT_ASSERT(colorB.IsValid());

    // compute probability of sampling color A
    const float colorWeightA = Vector4::Dot3(colorA, c_rgbIntensityWeights);
    const float colorWeightB = Vector4::Dot3(colorB, c_rgbIntensityWeights);
    if (colorWeightA > FLT_EPSILON || colorWeightB > FLT_EPSILON)
    {
        mPdf = colorWeightA / (colorWeightA + colorWeightB);
    }
}

const char* CheckerboardTexture::GetName() const
{
    return "checkerboard";
}

const Vector4 CheckerboardTexture::Evaluate(const Vector4& coords) const
{
    // wrap to 0..1 range
    const Vector4 warpedCoords = Vector4::Mod1(coords);

    VectorBool4 conditionVec = warpedCoords > VECTOR_HALVES;
    conditionVec = conditionVec ^ conditionVec.Swizzle<1,1,3,3>();

    return conditionVec.Get<0>() ? mColorA : mColorB;
}

const Vector4 CheckerboardTexture::Sample(const Float2 u, Vector4& outCoords, float* outPdf) const
{
    // TODO

    outCoords = Vector4(u);

    if (outPdf)
    {
        *outPdf = 1.0f;
    }

    return CheckerboardTexture::Evaluate(outCoords);
}

} // namespace rt