#pragma once

#include "Vector4.h"
#include "VectorInt4.h"
#include "Half.h"

namespace rt {
namespace math {


RT_FORCE_INLINE const Vector4 Vector4_Load_Float2_Unsafe(const Float2* src)
{
    return Vector4(reinterpret_cast<const float*>(src));
}

RT_FORCE_INLINE const Vector4 Vector4_Load_Float3_Unsafe(const Float3* src)
{
    return Vector4(reinterpret_cast<const float*>(src));
}

RT_FORCE_INLINE const Vector4 Vector4_Load_Float2_Unsafe(const Float2& src)
{
    return Vector4(reinterpret_cast<const float*>(&src));
}

RT_FORCE_INLINE const Vector4 Vector4_Load_Float3_Unsafe(const Float3& src)
{
    return Vector4(reinterpret_cast<const float*>(&src));
}

RT_FORCE_INLINE const Vector4 Vector4_Load_Half2(const Half src[2])
{
#ifdef RT_USE_FP16C
    const __m128i v = _mm_loadu_si32(src);
    return _mm_cvtph_ps(v);
#else // RT_USE_FP16C
    return Vector4(src[0].ToFloat(), src[1].ToFloat(), 0.0f, 0.0f);
#endif // RT_USE_FP16C
}

RT_FORCE_INLINE const Vector4 Vector4_Load_Half4(const Half src[4])
{
#ifdef RT_USE_FP16C
    const __m128i v = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(src));
    return _mm_cvtph_ps(v);
#else // RT_USE_FP16C
    return Vector4(src[0].ToFloat(), src[1].ToFloat(), src[2].ToFloat(), src[3].ToFloat());
#endif // RT_USE_FP16C
}

// Convert uint16 (B5G6R5 format) to a Vector4 (normalized range)
RT_FORCE_INLINE const Vector4 Vector4_Load_B5G6R5_Norm(const uint16* src)
{
    const VectorInt4 mask{ 0x1F << 11, 0x3F << 5, 0x1F, 0 };
    const Vector4 scale{ 1.0f / 2048.0f / 31.0f, 1.0f / 32.0f / 63.0f, 1.0f / 31.0f, 0.0f };
    const VectorInt4 raw = VectorInt4(*reinterpret_cast<const int32*>(src)) & mask;
    return raw.ConvertToFloat() * scale;
}

// Convert 2 uint8 to a Vector4 (normalized range)
RT_FORCE_INLINE const Vector4 Vector4_Load_2xUint8_Norm(const uint8* src)
{
#ifdef RT_USE_SSE
    const Vector4 mask{ 0xFFu, 0xFF00u, 0u, 0u };
    const Vector4 scale{ 1.0f / 255.0f, 1.0f / (256.0f * 255.0f), 0.0f, 0.0f };
    const Vector4 unsignedOffset{ 0.0f, 0.0f, 0.0f, 32768.0f * 65536.0f };

    __m128 vTemp = _mm_load_ps1((const float*)src);
    vTemp = _mm_and_ps(vTemp, mask.v);
    vTemp = _mm_xor_ps(vTemp, VECTOR_MASK_SIGN_W);

    // convert to float
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    vTemp = _mm_add_ps(vTemp, unsignedOffset);
    return _mm_mul_ps(vTemp, scale);
#else
    return Vector4
    {
        static_cast<float>(src[0]) / 255.0f,
        static_cast<float>(src[1]) / 255.0f,
    };
#endif // RT_USE_SSE
}

// Convert 4 uint8 to a Vector4
RT_FORCE_INLINE const Vector4 Vector4_Load_4xUint8(const uint8* src)
{
#ifdef RT_USE_SSE
    const Vector4 mask{ 0xFFu, 0xFF00u, 0xFF0000u, 0xFF000000u };
    const Vector4 scale{ 1.0f, 1.0f / 256.0f, 1.0f / 65536.0f, 1.0f / (65536.0f * 256.0f) };
    const Vector4 unsignedOffset{ 0.0f, 0.0f, 0.0f, 32768.0f * 65536.0f };

    __m128 vTemp = _mm_load_ps1((const float*)src);
    vTemp = _mm_and_ps(vTemp, mask.v);
    vTemp = _mm_xor_ps(vTemp, VECTOR_MASK_SIGN_W);

    // convert to float
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    vTemp = _mm_add_ps(vTemp, unsignedOffset);
    return _mm_mul_ps(vTemp, scale);
#else
    return Vector4
    {
        static_cast<float>(src[0]),
        static_cast<float>(src[1]),
        static_cast<float>(src[2]),
        static_cast<float>(src[3]),
    };
#endif // RT_USE_SSE
}

// Convert 2 uint16 to a Vector4 (normalized range)
RT_FORCE_INLINE const Vector4 Vector4_Load_2xUint16_Norm(const uint16* src)
{
#ifdef RT_USE_SSE
    const Vector4 maskX16Y16{ 0x0000FFFFu, 0xFFFF0000u, 0u, 0u };
    const Vector4 flipY{ 0u, 0x80000000u, 0u, 0u };
    const Vector4 fixUpY16{ 1.0f / 65535.0f, 1.0f / (65535.0f * 65536.0f), 0.0f, 0.0f };
    const Vector4 fixAddY16{ 0.0f, 32768.0f * 65536.0f, 0.0f, 0.0f };

    __m128 vTemp = _mm_load_ps1(reinterpret_cast<const float *>(src));
    // Mask x&0xFFFF, y&0xFFFF0000,z&0,w&0
    vTemp = _mm_and_ps(vTemp, maskX16Y16);
    // y needs to be sign flipped
    vTemp = _mm_xor_ps(vTemp, flipY);
    // Convert to floating point numbers
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    // y + 0x8000 to undo the signed order.
    vTemp = _mm_add_ps(vTemp, fixAddY16);
    // Y is 65536 times too large
    return _mm_mul_ps(vTemp, fixUpY16);
#else
    return Vector4
    {
        static_cast<float>(src[0]) / 65535.0f,
        static_cast<float>(src[1]) / 65535.0f,
    };
#endif // RT_USE_SSE
}

// Convert 4 uint16 to a Vector4
RT_FORCE_INLINE const Vector4 Vector4_Load_4xUint16(const uint16* src)
{
#ifdef RT_USE_SSE
    const Vector4 maskX16Y16Z16W16{ 0x0000FFFFu, 0x0000FFFFu, 0xFFFF0000u, 0xFFFF0000u };
    const __m128i shufflePattern = _mm_set_epi8(3, 2, 15, 14, 3, 2, 5, 4, 3, 2, 11, 10, 3, 2, 1, 0);
    // XXYYZZWWXXYYZZWW
    __m128d vIntd = _mm_load1_pd(reinterpret_cast<const double *>(src));
    // XX--ZZ----YY--WW
    __m128 masked = _mm_and_ps(_mm_castpd_ps(vIntd), maskX16Y16Z16W16);
    // --WW--ZZ--YY--XX
    __m128i reordered = _mm_shuffle_epi8(_mm_castps_si128(masked), shufflePattern);
    return _mm_cvtepi32_ps(reordered);
#else
    return Vector4
    {
        static_cast<float>(src[0]),
        static_cast<float>(src[1]),
        static_cast<float>(src[2]),
        static_cast<float>(src[3]),
    };
#endif // RT_USE_SSE
}

// Convert 3 uint8 to a Vector4 and scale to 0...1 range
RT_FORCE_INLINE const Vector4 Vector4_LoadBGR_UNorm(const uint8* src)
{
#ifdef RT_USE_SSE
    const Vector4 mask{ 0xFF0000u, 0xFF00u, 0xFFu, 0x0u };
    const Vector4 scale{ 1.0f / 65536.0f / 255.0f, 1.0f / 256.0f / 255.0f, 1.0f / 255.0f, 0.0f };

    __m128 vTemp = _mm_load_ps1((const float*)src);
    vTemp = _mm_and_ps(vTemp, mask.v);

    // convert to float
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    return _mm_mul_ps(vTemp, scale);
#else
    return Vector4
    {
        static_cast<float>(src[2]) / 255.0f,
        static_cast<float>(src[1]) / 255.0f,
        static_cast<float>(src[0]) / 255.0f,
    };
#endif // RT_USE_SSE
}


} // namespace math
} // namespace rt
