#include "PCH.h"
#include "Math.h"

namespace rt {
namespace math {

void SetFlushDenormalsToZero(bool enable)
{
    _MM_SET_DENORMALS_ZERO_MODE(enable ? _MM_DENORMALS_ZERO_ON : _MM_DENORMALS_ZERO_OFF);
    _MM_SET_FLUSH_ZERO_MODE(enable ? _MM_FLUSH_ZERO_ON : _MM_FLUSH_ZERO_OFF);
}

bool GetFlushDenormalsToZero()
{
    return _MM_GET_DENORMALS_ZERO_MODE() && _MM_GET_FLUSH_ZERO_MODE();
}

} // namespace math
} // namespace rt
