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

void BuildOrthonormalBasis(const Vector4& n, Vector4& u, Vector4& v)
{
    // algorithm based on "Building an Orthonormal Basis, Revisited" (2017) paper
    // by T. Duff, J. Burgess, P. Christensen, C. Hery, A. Kensler, M. Liani, and R. Villemin

    const float sign = CopySign(1.0f, n.z);
    const float a = -1.0f / (sign + n.z);

    u = Vector4(
        1.0f + sign * n.x * n.x * a,
        sign * n.x * n.y * a,
        -sign * n.x);

    v = Vector4(
        n.x * n.y * a,
        sign + n.y * n.y * a,
        -n.y);
}

} // namespace math
} // namespace rt
