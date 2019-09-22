#include "PCH.h"
#include "../Core/Utils/Entropy.h"
#include "../Core/Math/Random.h"

using namespace rt;
using namespace rt::math;

//////////////////////////////////////////////////////////////////////////

TEST(RandomTest, Entropy)
{
    Entropy entropy;

    const uint32 iterations = 10000;

    uint64 sum = 0;
    for (uint32 i = 0; i < iterations; ++i)
    {
        sum += entropy.GetInt();
    }

    EXPECT_GE(sum, 4900ull * (uint64)UINT32_MAX);
    EXPECT_LE(sum, 5100ull * (uint64)UINT32_MAX);
}
