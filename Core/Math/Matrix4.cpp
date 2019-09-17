#include "PCH.h"
#include "Matrix4.h"


namespace rt {
namespace math {

const Matrix4 Matrix4::MakeTranslation(const Vector4& pos)
{
    Matrix4 m = Identity();
    m.rows[3] = Vector4(pos.x, pos.y, pos.z, 1.0f);
    return m;
}

const Matrix4 Matrix4::MakePerspective(float aspect, float fovY, float nearZ, float farZ)
{
    float yScale = 1.0f / tanf(fovY * 0.5f);
    float xScale = yScale / aspect;

    return Matrix4(Vector4(xScale, 0.0f, 0.0f, 0.0f),
        Vector4(0.0f, yScale, 0.0f, 0.0f),
        Vector4(0.0f, 0.0f, farZ / (farZ - nearZ), 1.0f),
        Vector4(0.0f, 0.0f, -nearZ * farZ / (farZ - nearZ), 0.0f));
}

const Matrix4 Matrix4::MakeScaling(const Vector4& scale)
{
    Matrix4 m = Identity();
    m.rows[0] *= scale.SplatX();
    m.rows[1] *= scale.SplatY();
    m.rows[2] *= scale.SplatZ();
    return m;
}

const Matrix4 Matrix4::operator * (const Matrix4& b) const
{
#ifdef RT_USE_AVX

    // based on:
    // https://stackoverflow.com/a/46108376/10061517

    Vector8 a0, a1, b0, b1;
    Vector8 c0, c1;

    b0 = _mm256_permute2f128_ps(b.vec[0], b.vec[0], 0x00);
    a0 = _mm256_shuffle_ps(vec[0], vec[0], _MM_SHUFFLE(0, 0, 0, 0));
    a1 = _mm256_shuffle_ps(vec[1], vec[1], _MM_SHUFFLE(0, 0, 0, 0));
    c0 = a0 * b0;
    c1 = a1 * b0;

    b0 = _mm256_permute2f128_ps(b.vec[0], b.vec[0], 0x11);
    a0 = _mm256_shuffle_ps(vec[0], vec[0], _MM_SHUFFLE(1, 1, 1, 1));
    a1 = _mm256_shuffle_ps(vec[1], vec[1], _MM_SHUFFLE(1, 1, 1, 1));
    c0 = Vector8::MulAndAdd(a0, b0, c0);
    c1 = Vector8::MulAndAdd(a1, b0, c1);

    b1 = _mm256_permute2f128_ps(b.vec[1], b.vec[1], 0x00);
    a0 = _mm256_shuffle_ps(vec[0], vec[0], _MM_SHUFFLE(2, 2, 2, 2));
    a1 = _mm256_shuffle_ps(vec[1], vec[1], _MM_SHUFFLE(2, 2, 2, 2));
    c0 = Vector8::MulAndAdd(a0, b1, c0);
    c1 = Vector8::MulAndAdd(a1, b1, c1);

    b1 = _mm256_permute2f128_ps(b.vec[1], b.vec[1], 0x11);
    a0 = _mm256_shuffle_ps(vec[0], vec[0], _MM_SHUFFLE(3, 3, 3, 3));
    a1 = _mm256_shuffle_ps(vec[1], vec[1], _MM_SHUFFLE(3, 3, 3, 3));
    c0 = Vector8::MulAndAdd(a0, b1, c0);
    c1 = Vector8::MulAndAdd(a1, b1, c1);

    return Matrix4{ c0, c1 };

#else

    return Matrix4{ b * rows[0], b * rows[1], b * rows[2], b * rows[3] };

#endif
}

Matrix4& Matrix4::operator *= (const Matrix4& b)
{
    rows[0] = b * rows[0];
    rows[1] = b * rows[1];
    rows[2] = b * rows[2];
    rows[3] = b * rows[3];
    return *this;
}

float Matrix4::Determinant() const
{
    Vector4 v0 = rows[2].Swizzle<1, 0, 0, 0>();
    Vector4 v1 = rows[3].Swizzle<2, 2, 1, 1>();
    Vector4 v2 = rows[2].Swizzle<1, 0, 0, 0>();
    Vector4 v3 = rows[3].Swizzle<3, 3, 3, 2>();
    Vector4 v4 = rows[2].Swizzle<2, 2, 1, 1>();
    Vector4 v5 = rows[3].Swizzle<3, 3, 3, 2>();

    Vector4 p0 = v0 * v1;
    Vector4 p1 = v2 * v3;
    Vector4 p2 = v4 * v5;

    v0 = rows[2].Swizzle<2, 2, 1, 1>();
    v1 = rows[3].Swizzle<1, 0, 0, 0>();
    v2 = rows[2].Swizzle<3, 3, 3, 2>();
    v3 = rows[3].Swizzle<1, 0, 0, 0>();
    v4 = rows[2].Swizzle<3, 3, 3, 2>();
    v5 = rows[3].Swizzle<2, 2, 1, 1>();

    p0 = Vector4::NegMulAndAdd(v0, v1, p0);
    p1 = Vector4::NegMulAndAdd(v2, v3, p1);
    p2 = Vector4::NegMulAndAdd(v4, v5, p2);

    v0 = rows[1].Swizzle<3, 3, 3, 2>();
    v1 = rows[1].Swizzle<2, 2, 1, 1>();
    v2 = rows[1].Swizzle<1, 0, 0, 0>();

    Vector4 r = v0 * p0;
    r = Vector4::NegMulAndAdd(v1, p1, r);
    r = Vector4::MulAndAdd(v2, p2, r);

    Vector4 s = rows[0] * Vector4{ 1.0f, -1.0f, 1.0f, -1.0f };
    return Vector4::Dot4(s, r);
}

const Matrix4 Matrix4::Inverse() const
{
    float inv[16], det;
    int i;

    inv[0] = f[5] * f[10] * f[15] -
        f[5] * f[11] * f[14] -
        f[9] * f[6] * f[15] +
        f[9] * f[7] * f[14] +
        f[13] * f[6] * f[11] -
        f[13] * f[7] * f[10];

    inv[4] = -f[4] * f[10] * f[15] +
        f[4] * f[11] * f[14] +
        f[8] * f[6] * f[15] -
        f[8] * f[7] * f[14] -
        f[12] * f[6] * f[11] +
        f[12] * f[7] * f[10];

    inv[8] = f[4] * f[9] * f[15] -
        f[4] * f[11] * f[13] -
        f[8] * f[5] * f[15] +
        f[8] * f[7] * f[13] +
        f[12] * f[5] * f[11] -
        f[12] * f[7] * f[9];

    inv[12] = -f[4] * f[9] * f[14] +
        f[4] * f[10] * f[13] +
        f[8] * f[5] * f[14] -
        f[8] * f[6] * f[13] -
        f[12] * f[5] * f[10] +
        f[12] * f[6] * f[9];

    inv[1] = -f[1] * f[10] * f[15] +
        f[1] * f[11] * f[14] +
        f[9] * f[2] * f[15] -
        f[9] * f[3] * f[14] -
        f[13] * f[2] * f[11] +
        f[13] * f[3] * f[10];

    inv[5] = f[0] * f[10] * f[15] -
        f[0] * f[11] * f[14] -
        f[8] * f[2] * f[15] +
        f[8] * f[3] * f[14] +
        f[12] * f[2] * f[11] -
        f[12] * f[3] * f[10];

    inv[9] = -f[0] * f[9] * f[15] +
        f[0] * f[11] * f[13] +
        f[8] * f[1] * f[15] -
        f[8] * f[3] * f[13] -
        f[12] * f[1] * f[11] +
        f[12] * f[3] * f[9];

    inv[13] = f[0] * f[9] * f[14] -
        f[0] * f[10] * f[13] -
        f[8] * f[1] * f[14] +
        f[8] * f[2] * f[13] +
        f[12] * f[1] * f[10] -
        f[12] * f[2] * f[9];

    inv[2] = f[1] * f[6] * f[15] -
        f[1] * f[7] * f[14] -
        f[5] * f[2] * f[15] +
        f[5] * f[3] * f[14] +
        f[13] * f[2] * f[7] -
        f[13] * f[3] * f[6];

    inv[6] = -f[0] * f[6] * f[15] +
        f[0] * f[7] * f[14] +
        f[4] * f[2] * f[15] -
        f[4] * f[3] * f[14] -
        f[12] * f[2] * f[7] +
        f[12] * f[3] * f[6];

    inv[10] = f[0] * f[5] * f[15] -
        f[0] * f[7] * f[13] -
        f[4] * f[1] * f[15] +
        f[4] * f[3] * f[13] +
        f[12] * f[1] * f[7] -
        f[12] * f[3] * f[5];

    inv[14] = -f[0] * f[5] * f[14] +
        f[0] * f[6] * f[13] +
        f[4] * f[1] * f[14] -
        f[4] * f[2] * f[13] -
        f[12] * f[1] * f[6] +
        f[12] * f[2] * f[5];

    inv[3] = -f[1] * f[6] * f[11] +
        f[1] * f[7] * f[10] +
        f[5] * f[2] * f[11] -
        f[5] * f[3] * f[10] -
        f[9] * f[2] * f[7] +
        f[9] * f[3] * f[6];

    inv[7] = f[0] * f[6] * f[11] -
        f[0] * f[7] * f[10] -
        f[4] * f[2] * f[11] +
        f[4] * f[3] * f[10] +
        f[8] * f[2] * f[7] -
        f[8] * f[3] * f[6];

    inv[11] = -f[0] * f[5] * f[11] +
        f[0] * f[7] * f[9] +
        f[4] * f[1] * f[11] -
        f[4] * f[3] * f[9] -
        f[8] * f[1] * f[7] +
        f[8] * f[3] * f[5];

    inv[15] = f[0] * f[5] * f[10] -
        f[0] * f[6] * f[9] -
        f[4] * f[1] * f[10] +
        f[4] * f[2] * f[9] +
        f[8] * f[1] * f[6] -
        f[8] * f[2] * f[5];

    det = f[0] * inv[0] + f[1] * inv[4] + f[2] * inv[8] + f[3] * inv[12];
    det = 1.0f / det;

    Matrix4 result;
    for (i = 0; i < 16; i++)
    {
        result.f[i] = inv[i] * det;
    }

    return result;
}

const Box Matrix4::TransformBox(const Box& box) const
{
    // based on:
    // http://dev.theomader.com/transform-bounding-boxes/

    const Vector4 xa = rows[0] * box.min.x;
    const Vector4 xb = rows[0] * box.max.x;
    const Vector4 ya = rows[1] * box.min.y;
    const Vector4 yb = rows[1] * box.max.y;
    const Vector4 za = rows[2] * box.min.z;
    const Vector4 zb = rows[2] * box.max.z;

    return Box(
        Vector4::Min(xa, xb) + Vector4::Min(ya, yb) + Vector4::Min(za, zb) + rows[3],
        Vector4::Max(xa, xb) + Vector4::Max(ya, yb) + Vector4::Max(za, zb) + rows[3]
    );
}

} // namespace math
} // namespace rt
