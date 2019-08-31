#include "PCH.h"
#include "GenericSampler.h"
#include "../Math/Random.h"
#include "../Utils/Logger.h"

namespace rt {

using namespace math;

namespace BlueNoise
{

static const char* FilePath = "../Data/BlueNoise128_RGBA16.dat";
static constexpr Uint32 TextureLayers = 4;
static constexpr Uint32 TextureBits = 16;
static constexpr Uint32 TextureSize = 128;

static const Uint16* LoadTexture()
{
    FILE* file = fopen(FilePath, "rb");
    if (!file)
    {
        RT_LOG_ERROR("Failed to load blue noise texture");
        return nullptr;
    }

    const size_t dataSize = (TextureBits / 8) * TextureLayers * TextureSize * TextureSize;
    Uint16* data = (Uint16*)DefaultAllocator::Allocate(dataSize, RT_CACHE_LINE_SIZE);
    if (!data)
    {
        fclose(file);
        RT_LOG_ERROR("Failed to allocate memory for blue noise texture");
        return nullptr;
    }

    if (fread(data, dataSize, 1, file) != 1)
    {
        fclose(file);
        RT_LOG_ERROR("Failed to read blue noise texture");
        return nullptr;
    }

    RT_LOG_INFO("Blue noise texture loaded successfully");
    fclose(file);
    return data;
}

static const Uint16* GetTexture()
{
    static const Uint16* texture = LoadTexture();
    return texture;
}

} // BlueNoise

RT_FORCE_INLINE static Uint32 XorShift(Uint32 x)
{
    x ^= x << 13u;
    x ^= x >> 17u;
    x ^= x << 5u;
    return x;
}

GenericSampler::GenericSampler()
    : mBlueNoiseTexture(BlueNoise::GetTexture())
{
}

void GenericSampler::ResetFrame(const DynArray<Uint32>& seed, bool useBlueNoise)
{
    mCurrentSample = seed;
    mBlueNoiseTextureLayers = mBlueNoiseTexture && useBlueNoise ? BlueNoise::TextureLayers : 0;
}

void GenericSampler::ResetPixel(const Uint32 x, const Uint32 y)
{
    mBlueNoisePixelX = x & (BlueNoise::TextureSize - 1u);
    mBlueNoisePixelY = y & (BlueNoise::TextureSize - 1u);
    mSalt = (Uint32)Hash((Uint64)(x | (y << 16)));
    mSamplesGenerated = 0;
}

Uint32 GenericSampler::GetInt()
{
    Uint32 sample;

    if (mSamplesGenerated < mCurrentSample.Size())
    {
        sample = mCurrentSample[mSamplesGenerated];

        if (mSamplesGenerated < mBlueNoiseTextureLayers) // blue noise dithering
        {
            const Uint32 pixelIndex = BlueNoise::TextureSize * mBlueNoisePixelY + mBlueNoisePixelX;
            const Uint16* blueNoiseData = mBlueNoiseTexture + BlueNoise::TextureLayers * pixelIndex;
            sample += static_cast<Uint32>(blueNoiseData[mSamplesGenerated]) << 16;
        }
        else
        {
            Uint32 salt = mSalt;
            mSalt = XorShift(salt);
            sample += salt;
        }

        mSamplesGenerated++;
    }
    else // fallback to uniform random sampling
    {
        sample = fallbackGenerator->GetInt();
    }

    return sample;
}

} // namespace rt