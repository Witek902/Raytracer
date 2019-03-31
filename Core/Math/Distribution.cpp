#include "PCH.h"
#include "Distribution.h"
#include "Math.h"
#include "Utils/AlignmentAllocator.h"
#include "Utils/Logger.h"

#include <algorithm>

namespace rt {
namespace math {

Distribution::Distribution()
    : mPDF(nullptr)
    , mCDF(nullptr)
    , mSize(0)
{}

Distribution::~Distribution()
{
    AlignedFree(mCDF);
    AlignedFree(mPDF);

    mCDF = nullptr;
    mPDF = nullptr;
}

bool Distribution::Initialize(const float* pdfValues, Uint32 numValues)
{
    if (numValues == 0)
    {
        RT_LOG_ERROR("Empty distribution");
        return false;
    }

    if (!pdfValues)
    {
        RT_LOG_ERROR("Invalid distribution pdf");
        return false;
    }

    mPDF = (float*)AlignedMalloc(sizeof(float) * (size_t)numValues, RT_CACHE_LINE_SIZE);
    if (!mPDF)
    {
        RT_LOG_ERROR("Failed to allocate memory for PDF");
        return false;
    }

    mCDF = (float*)AlignedMalloc(sizeof(float) * ((size_t)numValues + 1), RT_CACHE_LINE_SIZE);
    if (!mCDF)
    {
        RT_LOG_ERROR("Failed to allocate memory for CDF");
        return false;
    }

    // compute cumulated distribution function
    float accumulated = 0.0f;
    mCDF[0] = 0.0f;
    for (Uint32 i = 0; i < numValues; ++i)
    {
        RT_ASSERT(IsValid(pdfValues[i]), "Corrupted pdf");
        RT_ASSERT(pdfValues[i] >= 0.0f, "Pdf must be non-negative");

        accumulated += pdfValues[i];
        mCDF[i + 1] = accumulated;
    }

    RT_ASSERT(accumulated > 0.0f, "Pdf must be non-zero");

    // normalize
    const float cdfNormFactor = 1.0f / accumulated;
    const float pdfNormFactor = cdfNormFactor * static_cast<float>(numValues);
    for (Uint32 i = 0; i < numValues; ++i)
    {
        mCDF[i] *= cdfNormFactor;
        mPDF[i] = pdfValues[i] * pdfNormFactor;
    }
    mCDF[numValues] *= cdfNormFactor;

    // TODO Cumulative distribution function should be stored as unsigned integers, as it's only in 0-1 range

    mSize = numValues;
    return false;
}

Uint32 Distribution::SampleDiscrete(const float u, float& outPdf) const
{
    Uint32 low = 0u;
    Uint32 high = mSize;

    // binary search
    while (low < high)
    {
        Uint32 mid = (low + high) / 2u;
        if (u >= mCDF[mid])
        {
            low = mid + 1u;
        }
        else
        {
            high = mid;
        }
    }
    
    Uint32 offset = low - 1u;

    outPdf = mPDF[offset];

    return offset;
}

} // namespace math
} // namespace rt