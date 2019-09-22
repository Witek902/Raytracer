#pragma once

#include "../RayLib.h"

namespace rt {
namespace math {

// Utility class for fast sampling 1D probability distribution function
// (piecewise constant)
class RAYLIB_API Distribution : public NoCopyable
{
public:
    Distribution();
    ~Distribution();

    // initialize with 1D pdf function
    bool Initialize(const float* pdfValues, uint32 numValues);

    // sample discrete
    uint32 SampleDiscrete(const float u, float& outPdf) const;

private:
    float* mPDF;
    float* mCDF; // Cumulative distribution function
    uint32 mSize;
};

} // namespace math
} // namespace rt