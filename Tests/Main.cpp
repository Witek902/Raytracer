#include "PCH.h"
#include "../Core/Math/Math.h"

int main(int argc, char **argv)
{
    rt::math::SetFlushDenormalsToZero();

    testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();

    RT_ASSERT(rt::math::GetFlushDenormalsToZero(), "Something disabled flushing denormal float to zero");

    return result;
}
