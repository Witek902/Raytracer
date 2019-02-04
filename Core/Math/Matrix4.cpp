#include "PCH.h"
#include "Matrix4.h"


namespace rt {
namespace math {

Matrix4 Matrix4::MakeTranslation(const Vector4& pos)
{
    Matrix4 m;
    m.r[0] = VECTOR_X;
    m.r[1] = VECTOR_Y;
    m.r[2] = VECTOR_Z;
    m.r[3] = Vector4(pos.x, pos.y, pos.z, 1.0f);
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

Matrix4 Matrix4::MakeScaling(const Vector4& scale)
{
    Matrix4 m;
    m.r[0] = VECTOR_X * scale.SplatX();
    m.r[1] = VECTOR_Y * scale.SplatY();
    m.r[2] = VECTOR_Z * scale.SplatZ();
    return m;
}

Matrix4 Matrix4::operator * (const Matrix4& b) const
{
    return Matrix4(b.LinearCombination4(r[0]),
                   b.LinearCombination4(r[1]),
                   b.LinearCombination4(r[2]),
                   b.LinearCombination4(r[3]));
}

Matrix4& Matrix4::operator *= (const Matrix4& b)
{
    r[0] = b.LinearCombination4(r[0]);
    r[1] = b.LinearCombination4(r[1]);
    r[2] = b.LinearCombination4(r[2]);
    r[3] = b.LinearCombination4(r[3]);
    return *this;
}

const Box Matrix4::TransformBox(const Box& box) const
{
    // based on:
    // http://dev.theomader.com/transform-bounding-boxes/

    const Vector4 xa = r[0] * box.min.x;
    const Vector4 xb = r[0] * box.max.x;
    const Vector4 ya = r[1] * box.min.y;
    const Vector4 yb = r[1] * box.max.y;
    const Vector4 za = r[2] * box.min.z;
    const Vector4 zb = r[2] * box.max.z;

    return Box(
        Vector4::Min(xa, xb) + Vector4::Min(ya, yb) + Vector4::Min(za, zb) + r[3],
        Vector4::Max(xa, xb) + Vector4::Max(ya, yb) + Vector4::Max(za, zb) + r[3]
    );
}

} // namespace math
} // namespace rt
