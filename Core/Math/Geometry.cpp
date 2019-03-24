#include "PCH.h"
#include "Geometry.h"
#include "Transcendental.h"

namespace rt {
namespace math {

const Vector4 CartesianToSphericalCoordinates(const Vector4& input)
{
    const float theta = FastACos(Clamp(input.y, -1.0f, 1.0f));
    const float phi = Abs(input.x) > FLT_EPSILON ? FastATan2(input.z, input.x) : 0.0f;
    return Vector4(phi / (2.0f * RT_PI) + 0.5f, theta / RT_PI, 0.0f, 0.0f);
}

} // namespace math
} // namespace rt
