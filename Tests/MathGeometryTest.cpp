#include "PCH.h"
#include "../Core/Math/Geometry.h"
#include "../Core/Math/Random.h"
#include "../Core/Math/SamplingHelpers.h"

using namespace rt::math;

//////////////////////////////////////////////////////////////////////////

TEST(MathTest, Geometry_BuildOrthonormalBasis)
{
    const uint32 numIterations = 1000;

    Random random;

    for (uint32 i = 0; i < numIterations; ++i)
    {
        const Vector4 x = SamplingHelpers::GetSphere(random.GetFloat2());

        Vector4 u, v;
        BuildOrthonormalBasis(x, u, v);

        SCOPED_TRACE("x=[" + std::to_string(x.x) + ',' + std::to_string(x.y) + ',' + std::to_string(x.z) + ']');
        SCOPED_TRACE("u=[" + std::to_string(u.x) + ',' + std::to_string(u.y) + ',' + std::to_string(u.z) + ']');
        SCOPED_TRACE("v=[" + std::to_string(v.x) + ',' + std::to_string(v.y) + ',' + std::to_string(v.z) + ']');

        EXPECT_TRUE(Vector4::AlmostEqual(x, Vector4::Cross3(u, v), 0.00001f));
        EXPECT_TRUE(Vector4::AlmostEqual(v, Vector4::Cross3(x, u), 0.00001f));
        EXPECT_TRUE(Vector4::AlmostEqual(u, Vector4::Cross3(v, x), 0.00001f));

        EXPECT_NEAR(0.0f, Vector4::Dot3(u, v), 0.00001f);
        EXPECT_NEAR(0.0f, Vector4::Dot3(u, x), 0.00001f);
        EXPECT_NEAR(0.0f, Vector4::Dot3(x, v), 0.00001f);
    }
}
