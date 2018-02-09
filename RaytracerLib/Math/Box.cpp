#include "PCH.h"
#include "Box.h"


namespace rt {
namespace math {


Box::Box(const Vector4& a, const Vector4& b, const Vector4& c)
{
    min = Vector4::Min(a, Vector4::Min(b, c));
    max = Vector4::Max(a, Vector4::Max(b, c));
}


} // namespace math
} // namespace rt
