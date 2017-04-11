#include "PCH.h"
#include "Random.h"

#include <time.h>


union Bits32
{
    Float f;
    Uint32 ui;
    Int32 si;
};

union Bits64
{
    Double f;
    Uint64 ui;
    Int64 si;
};


Random::Random()
{
    mSeed = static_cast<Uint64>(time(0));
}

Random::Random(Uint64 seed)
{
    if (seed == 0)
    {
        mSeed = static_cast<Uint64>(time(0));
    }
    else
    {
        mSeed = seed;
    }
}

Uint64 Random::GetLong()
{
    Shuffle();
    return mSeed;
}

Uint32 Random::GetInt()
{
    Shuffle();
    return static_cast<Uint32>(mSeed);
}


float Random::GetFloat()
{
    Shuffle();
    Bits32 myrand;
    myrand.ui = 1 + ((static_cast<Uint32>(mSeed) & 0x007fffff) | 0x3f800000);
    return myrand.f - 1.0f;
}

float Random::GetFloatBipolar()
{
    Shuffle();
    Bits32 myrand;
    myrand.ui = 1 + ((static_cast<Uint32>(mSeed) & 0x007fffff) | 0x40000000);
    return myrand.f - 3.0f;
}

double Random::GetDouble()
{
    Shuffle();
    Bits64 myrand;
    myrand.ui = 1L + ((mSeed & 0x000fffffffffffffUL) | 0x3ff0000000000000UL);
    return myrand.f - 1.0;
}


/*

Float2 Random::GetFloat2()
{
    Float2 result;
    Bits32 myrand;

    Shuffle();

    myrand.ui = 1 + ((static_cast<int>(mSeed) & 0x007fffff) | 0x3f800000);
    result.x = myrand.f - 1.0f;
    myrand.ui = 1 + ((static_cast<int>(mSeed >> 32) & 0x007fffff) | 0x3f800000);
    result.y = myrand.f - 1.0f;

    return result;
}

Float3 Random::GetFloat3()
{
    Float3 result;
    Bits32 myrand;

    Shuffle();
    myrand.ui = 1 + ((static_cast<int>(mSeed) & 0x007fffff) | 0x3f800000);
    result.x = myrand.f - 1.0f;
    myrand.ui = 1 + ((static_cast<int>(mSeed >> 32) & 0x007fffff) | 0x3f800000);
    result.y = myrand.f - 1.0f;

    Shuffle();
    myrand.ui = 1 + ((static_cast<int>(mSeed) & 0x007fffff) | 0x3f800000);
    result.z = myrand.f - 1.0f;

    return result;
}

Float4 Random::GetFloat4()
{
    Float4 result;
    Bits32 myrand;

    Shuffle();
    myrand.ui = 1 + ((static_cast<int>(mSeed) & 0x007fffff) | 0x3f800000);
    result.x = myrand.f - 1.0f;
    myrand.ui = 1 + ((static_cast<int>(mSeed >> 32) & 0x007fffff) | 0x3f800000);
    result.y = myrand.f - 1.0f;

    Shuffle();
    myrand.ui = 1 + ((static_cast<int>(mSeed) & 0x007fffff) | 0x3f800000);
    result.z = myrand.f - 1.0f;
    myrand.ui = 1 + ((static_cast<int>(mSeed >> 32) & 0x007fffff) | 0x3f800000);
    result.w = myrand.f - 1.0f;

    return result;
}


float Random::GetFloatNormal()
{
    // Box-Muller method
    Float2 uv = GetFloat2();
    return sqrtf(- 2.0f * logf(uv.x)) * cosf(6.2831853f * uv.y);
}

Float2 Random::GetFloatNormal2()
{
    Float2 result;

    // Box-Muller method
    Float2 uv = GetFloat2();
    float temp = sqrtf(- 2.0f * logf(uv.x));

    result.x = temp * cosf(6.2831853f * uv.y);
    result.y = temp * sinf(6.2831853f * uv.y);
    return result;
}

Float3 Random::GetFloatNormal3()
{
    Float3 result;

    // Box-Muller method
    Float2 uv = GetFloat2();
    float temp = sqrtf(- 2.0f * logf(uv.x));
    result.x = temp * cosf(6.2831853f * uv.y);
    result.y = temp * sinf(6.2831853f * uv.y);

    uv = GetFloat2();
    temp = sqrtf(- 2.0f * logf(uv.x));
    result.z = temp * cosf(6.2831853f * uv.y);

    return result;
}

Float4 Random::GetFloatNormal4()
{
    Float4 result;

    // Box-Muller method
    Float2 uv = GetFloat2();
    float temp = sqrtf(- 2.0f * logf(uv.x));
    result.x = temp * cosf(6.2831853f * uv.y);
    result.y = temp * sinf(6.2831853f * uv.y);

    uv = GetFloat2();
    temp = sqrtf(- 2.0f * logf(uv.x));
    result.z = temp * cosf(6.2831853f * uv.y);
    result.w = temp * sinf(6.2831853f * uv.y);

    return result;
}

*/
