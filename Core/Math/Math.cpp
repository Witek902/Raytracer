#include "PCH.h"
#include "Math.h"

#ifdef RT_USE_SSE
#pragma message("[rt::math] Compiling with SSE support")
#endif

#ifdef RT_USE_AVX
#pragma message("[rt::math] Compiling with AVX support")
#endif

#ifdef RT_USE_AVX2
#pragma message("[rt::math] Compiling with AVX2 support")
#endif

#ifdef RT_USE_FP16C
#pragma message("[rt::math] Compiling with FP16C support")
#endif

#ifdef RT_USE_FMA
#pragma message("[rt::math] Compiling with FMA support")
#endif

namespace rt {
namespace math {

void SetFlushDenormalsToZero(bool enable)
{
    RT_UNUSED(enable);
#ifdef RT_USE_SSE
    _MM_SET_DENORMALS_ZERO_MODE(enable ? _MM_DENORMALS_ZERO_ON : _MM_DENORMALS_ZERO_OFF);
    _MM_SET_FLUSH_ZERO_MODE(enable ? _MM_FLUSH_ZERO_ON : _MM_FLUSH_ZERO_OFF);
#endif // RT_USE_SSE
}

bool GetFlushDenormalsToZero()
{
#ifdef RT_USE_SSE
    return _MM_GET_DENORMALS_ZERO_MODE() && _MM_GET_FLUSH_ZERO_MODE();
#else
    return true;
#endif // RT_USE_SSE
}

} // namespace math
} // namespace rt
