#include "PCH.h"
#include "Vector4.h"

namespace rt {
namespace math {

const Vector4 Vector4::RefractZ(const Vector4& i, float eta)
{
    float NdotV = i.z;
    if (NdotV < 0.0f)
    {
        eta = 1.0f / eta;
    }

    const float k = 1.0f - eta * eta * (1.0f - NdotV * NdotV);
    if (k <= 0.0f)
    {
        return Vector4::Zero();
    }

    Vector4 transmitted = Vector4::NegMulAndAdd(Vector4(eta * NdotV + sqrtf(k)), VECTOR_Z, i * eta);
    RT_ASSERT(math::Abs(1.0f - transmitted.Length3()) < 0.01f);

    if (NdotV > 0.0f)
    {
        transmitted.z = -transmitted.z;
    }

    return transmitted.Normalized3();
}

} // namespace math
} // namespace rt