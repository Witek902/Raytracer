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
        for (Uint32 i = 0; i < mDimensions; i++)
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
    for (Uint32 d = 0; d < mDimensions; d++)
    {
        for (Uint8 j = 0; j < Width; j++)
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
    for (Uint32 i = 0; i < mDimensions; i++)
    {
        Uint64 n = mStarts[i] - 1;
        Int8 j = 0;
        while (n > 0)
        {
            digit[i][j] = n % mBase[i];
            n = n / mBase[i];
            j++;
        }
        j--;
        while (j >= 0)
        {
            Uint64 d = digit[i][j];
            d = Permute(i, j);
            rnd[i][j] = rnd[i][j + 1] + d * 1.0 / mPowerBuffer[i][j];
            j--;
        }
    }
}

void HaltonSequence::NextSample()
{
    for (Uint32 i = 0; i < mDimensions; i++)
    {
        Int8 j = 0;
        while (digit[i][j] + 1 >= mBase[i])
        {
            j++;
        }
        digit[i][j]++;
        Uint64 d = digit[i][j];
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

Uint64 HaltonSequence::Permute(Uint32 i, Uint8 j)
{
    return *(*(ppm + i) + digit[i][j]);
}

void HaltonSequence::InitPermutation()
{
    ppm = new Uint64*[mDimensions];

    for (Uint32 i = 0; i < mDimensions; i++)
    {
        *(ppm + i) = new Uint64[mBase[i]];
        for (Uint64 j = 0; j < mBase[i]; j++)
        {
            *(*(ppm + i) + j) = j;
        }

        for (Uint64 j = 1; j < mBase[i]; j++)
        {
            Uint64 tmp = (Uint64)floor(mRandom.GetDouble() * mBase[i]);
            if (tmp != 0)
            {
                Uint64 k = *(*(ppm + i) + j);
                *(*(ppm + i) + j) = *(*(ppm + i) + tmp);
                *(*(ppm + i) + tmp) = k;
            }
        }
    }
}

void HaltonSequence::InitPrimes()
{
    Int64 n = mDimensions;
    Uint32 prime = 1;
    Uint32 m = 0;
    do
    {
        prime++;
        mBase[m++] = prime;
        n--;
        for (Uint64 i = 2; i <= sqrt(prime); i++)
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
    for (Uint32 i = 0; i < mDimensions; i++)
    {
        double r = mRandom.GetDouble();
        const Uint64 base = mBase[i];

        Uint64 z = 0;
        Uint64 b = base;
        while (r > 1.0e-16)
        {
            Uint64 cnt = 0;
            if (r >= 1.0 / b)
            {
                cnt = (Uint64)floor(r * b);
                r = r - cnt * 1.0 / b;
                z += cnt * b / base;
            }
            b *= base;
        }

        mStarts[i] = z;
    }
}

void HaltonSequence::Initialize(Uint32 dim)
{
    ClearPermutation();

    assert(mDimensions <= MaxDimensions);
    mDimensions = dim;

    rnd.Resize(mDimensions, DynArray<double>(Width));
    digit.Resize(mDimensions, DynArray<Uint64>(Width));
    mPowerBuffer.Resize(mDimensions, DynArray<Uint64>(Width));
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