#include "PCH.h"
#include "HaltonSampler.h"

namespace rt {

using namespace math;

HaltonSequence::HaltonSequence()
{
    mDimensions = 0;
    ppm = nullptr;
}

HaltonSequence::~HaltonSequence()
{
    ClearPermutation();
}

void HaltonSequence::ClearPermutation()
{
    if (ppm)
    {
        for (uint32 i = 0; i < mDimensions; i++)
        {
            delete[] * (ppm + i);
            *(ppm + i) = nullptr;
        }
        delete[] ppm;
        ppm = nullptr;
    }
}

void HaltonSequence::InitPowerBuffer()
{
    for (uint32 d = 0; d < mDimensions; d++)
    {
        for (uint8 j = 0; j < Width; j++)
        {
            if (j == 0)
            {
                mPowerBuffer[d][j] = mBase[d];
            }
            else
            {
                mPowerBuffer[d][j] = mPowerBuffer[d][j - 1] * mBase[d];
            }
        }
    }

    for (auto &v : rnd)
    {
        std::fill(v.Begin(), v.End(), 0.0);
    }

    for (auto &v : digit)
    {
        std::fill(v.Begin(), v.End(), 0);
    }
}

void HaltonSequence::InitExpansion()
{
    for (uint32 i = 0; i < mDimensions; i++)
    {
        uint64 n = mStarts[i] - 1;
        int8 j = 0;
        while (n > 0)
        {
            digit[i][j] = n % mBase[i];
            n = n / mBase[i];
            j++;
        }
        j--;
        while (j >= 0)
        {
            uint64 d = digit[i][j];
            d = Permute(i, j);
            rnd[i][j] = rnd[i][j + 1] + d * 1.0 / mPowerBuffer[i][j];
            j--;
        }
    }
}

void HaltonSequence::NextSample()
{
    for (uint32 i = 0; i < mDimensions; i++)
    {
        int8 j = 0;
        while (digit[i][j] + 1 >= mBase[i])
        {
            j++;
        }
        digit[i][j]++;
        uint64 d = digit[i][j];
        d = Permute(i, j);
        rnd[i][j] = rnd[i][j + 1] + d * 1.0 / mPowerBuffer[i][j];

        for (j = j - 1; j >= 0; j--)
        {
            digit[i][j] = 0;
            d = 0;
            d = Permute(i, j);
            rnd[i][j] = rnd[i][j + 1] + d * 1.0 / mPowerBuffer[i][j];
        }
    }
}

uint64 HaltonSequence::Permute(uint32 i, uint8 j)
{
    return *(*(ppm + i) + digit[i][j]);
}

void HaltonSequence::InitPermutation()
{
    ppm = new uint64*[mDimensions];

    for (uint32 i = 0; i < mDimensions; i++)
    {
        *(ppm + i) = new uint64[mBase[i]];
        for (uint64 j = 0; j < mBase[i]; j++)
        {
            *(*(ppm + i) + j) = j;
        }

        for (uint64 j = 1; j < mBase[i]; j++)
        {
            uint64 tmp = (uint64)floor(mRandom.GetDouble() * mBase[i]);
            if (tmp != 0)
            {
                uint64 k = *(*(ppm + i) + j);
                *(*(ppm + i) + j) = *(*(ppm + i) + tmp);
                *(*(ppm + i) + tmp) = k;
            }
        }
    }
}

void HaltonSequence::InitPrimes()
{
    int64 n = mDimensions;
    uint32 prime = 1;
    uint32 m = 0;
    do
    {
        prime++;
        mBase[m++] = prime;
        n--;
        for (uint64 i = 2; i <= sqrt(prime); i++)
        {
            if (prime % i == 0)
            {
                n++;
                m--;
                break;
            }
        }
    } while (n > 0);
}

void HaltonSequence::InitStart()
{
    for (uint32 i = 0; i < mDimensions; i++)
    {
        double r = mRandom.GetDouble();
        const uint64 base = mBase[i];

        uint64 z = 0;
        uint64 b = base;
        while (r > 1.0e-16)
        {
            uint64 cnt = 0;
            if (r >= 1.0 / b)
            {
                cnt = (uint64)floor(r * b);
                r = r - cnt * 1.0 / b;
                z += cnt * b / base;
            }
            b *= base;
        }

        mStarts[i] = z;
    }
}

void HaltonSequence::Initialize(uint32 dim)
{
    ClearPermutation();

    assert(mDimensions <= MaxDimensions);
    mDimensions = dim;

    rnd.Resize(mDimensions, DynArray<double>(Width));
    digit.Resize(mDimensions, DynArray<uint64>(Width));
    mPowerBuffer.Resize(mDimensions, DynArray<uint64>(Width));
    mStarts.Resize(mDimensions);
    mBase.Resize(mDimensions);

    if (mDimensions > 0)
    {
        InitPrimes();
        InitStart();
        InitPowerBuffer();
        InitPermutation();
        InitExpansion();
    }
}


} // namespace rt