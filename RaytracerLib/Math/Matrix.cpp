#include "PCH.h"
#include "Matrix.h"


namespace rt {
namespace math {


Matrix Matrix::MakeLookTo(const Vector4& EyePosition, const Vector4& EyeDirection,
                    const Vector4& UpDirection)
{
    Vector4 zaxis = EyeDirection.Normalized3();
    Vector4 xaxis = Vector4::Cross3(UpDirection, zaxis).Normalized3();
    Vector4 yaxis = Vector4::Cross3(zaxis, xaxis);

    return Matrix(Vector4(xaxis.f[0], yaxis.f[0], zaxis.f[0], 0.0f),
                  Vector4(xaxis.f[1], yaxis.f[1], zaxis.f[1], 0.0f),
                  Vector4(xaxis.f[2], yaxis.f[2], zaxis.f[2], 0.0f),
                  Vector4(-Vector4::Dot3(xaxis, EyePosition),
                         -Vector4::Dot3(yaxis, EyePosition),
                         -Vector4::Dot3(zaxis, EyePosition),
                         1.0f));
}

Matrix Matrix::MakePerspective(float aspect, float fovY, float farZ, float nearZ)
{
    float yScale = 1.0f / tanf(fovY * 0.5f);
    float xScale = yScale / aspect;

    return Matrix(Vector4(xScale, 0.0f,   0.0f,                           0.0f),
                  Vector4(0.0f,   yScale, 0.0f,                           0.0f),
                  Vector4(0.0f,   0.0f,   farZ / (farZ - nearZ),          1.0f),
                  Vector4(0.0f,   0.0f,   -nearZ * farZ / (farZ - nearZ), 0.0f));
}

Matrix Matrix::MakeOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    return Matrix(
               Vector4(2.0f / (right - left), 0.0f,                  0.0f,                  0.0f),
               Vector4(0.0f,                  2.0f / (top - bottom), 0.0f,                  0.0f),
               Vector4(0.0f,                  0.0f,                  1.0f / (zFar - zNear), 0.0f),
               Vector4((left + right) / (left - right),
                      (top + bottom) / (bottom - top),
                      zNear / (zNear - zFar),
                      1.0f));
}

Matrix Matrix::MakeScaling(const Vector4& scale)
{
    return Matrix(Vector4(scale.f[0], 0.0f, 0.0f, 0.0f),
                  Vector4(0.0f, scale.f[1], 0.0f, 0.0f),
                  Vector4(0.0f, 0.0f, scale.f[2], 0.0f),
                  Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

Matrix Matrix::operator* (const Matrix& b) const
{
    return Matrix(b.LinearCombination4(r[0]),
                  b.LinearCombination4(r[1]),
                  b.LinearCombination4(r[2]),
                  b.LinearCombination4(r[3]));
}

Matrix& Matrix::operator*= (const Matrix& b)
{
    r[0] = b.LinearCombination4(r[0]);
    r[1] = b.LinearCombination4(r[1]);
    r[2] = b.LinearCombination4(r[2]);
    r[3] = b.LinearCombination4(r[3]);
    return *this;
}

Matrix Matrix::MakeRotationNormal(const Vector4& normalAxis, float angle)
{
    Matrix result;

    float sinAngle = sinf(angle);
    float cosAngle = cosf(angle);
    Vector4 N0, N1;
    Vector4 V0, V1, V2;
    Vector4 R0, R1, R2;
    Vector4 C0, C1, C2;

    C2 = _mm_set_ps1(1.0f - cosAngle);
    C1 = _mm_set_ps1(cosAngle);
    C0 = _mm_set_ps1(sinAngle);

    N0 = _mm_shuffle_ps(normalAxis, normalAxis, _MM_SHUFFLE(3, 0, 2, 1));
    N1 = _mm_shuffle_ps(normalAxis, normalAxis, _MM_SHUFFLE(3, 1, 0, 2));

    V0 = _mm_mul_ps(C2, N0);
    V0 = _mm_mul_ps(V0, N1);

    R0 = _mm_mul_ps(C2, normalAxis);
    R0 = _mm_mul_ps(R0, normalAxis);
    R0 = _mm_add_ps(R0, C1);

    R1 = _mm_mul_ps(C0, normalAxis);
    R1 = _mm_add_ps(R1, V0);
    R2 = _mm_mul_ps(C0, normalAxis);
    R2 = _mm_sub_ps(V0, R2);

    V0 = _mm_and_ps(R0, VECTOR_MASK_XYZ);

    V1 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(2, 1, 2, 0));
    V1 = _mm_shuffle_ps(V1, V1, _MM_SHUFFLE(0, 3, 2, 1));
    V2 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(0, 0, 1, 1));
    V2 = _mm_shuffle_ps(V2, V2, _MM_SHUFFLE(2, 0, 2, 0));

    R2 = _mm_shuffle_ps(V0, V1, _MM_SHUFFLE(1, 0, 3, 0));
    R2 = _mm_shuffle_ps(R2, R2, _MM_SHUFFLE(1, 3, 2, 0));
    result.r[0] = R2;
    R2 = _mm_shuffle_ps(V0, V1, _MM_SHUFFLE(3, 2, 3, 1));
    R2 = _mm_shuffle_ps(R2, R2, _MM_SHUFFLE(1, 3, 0, 2));
    result.r[1] = R2;
    V2 = _mm_shuffle_ps(V2, V0, _MM_SHUFFLE(3, 2, 1, 0));
    result.r[2] = V2;
    result.r[3] = VECTOR_Z;
    return result;
}

Matrix Matrix::Inverted() const
{
    Matrix MT = Transposed();
    Vector4 V00 = _mm_shuffle_ps(MT.r[2], MT.r[2], _MM_SHUFFLE(1, 1, 0, 0));
    Vector4 V10 = _mm_shuffle_ps(MT.r[3], MT.r[3], _MM_SHUFFLE(3, 2, 3, 2));
    Vector4 V01 = _mm_shuffle_ps(MT.r[0], MT.r[0], _MM_SHUFFLE(1, 1, 0, 0));
    Vector4 V11 = _mm_shuffle_ps(MT.r[1], MT.r[1], _MM_SHUFFLE(3, 2, 3, 2));
    Vector4 V02 = _mm_shuffle_ps(MT.r[2], MT.r[0], _MM_SHUFFLE(2, 0, 2, 0));
    Vector4 V12 = _mm_shuffle_ps(MT.r[3], MT.r[1], _MM_SHUFFLE(3, 1, 3, 1));

    Vector4 D0 = _mm_mul_ps(V00, V10);
    Vector4 D1 = _mm_mul_ps(V01, V11);
    Vector4 D2 = _mm_mul_ps(V02, V12);

    V00 = _mm_shuffle_ps(MT.r[2], MT.r[2], _MM_SHUFFLE(3, 2, 3, 2));
    V10 = _mm_shuffle_ps(MT.r[3], MT.r[3], _MM_SHUFFLE(1, 1, 0, 0));
    V01 = _mm_shuffle_ps(MT.r[0], MT.r[0], _MM_SHUFFLE(3, 2, 3, 2));
    V11 = _mm_shuffle_ps(MT.r[1], MT.r[1], _MM_SHUFFLE(1, 1, 0, 0));
    V02 = _mm_shuffle_ps(MT.r[2], MT.r[0], _MM_SHUFFLE(3, 1, 3, 1));
    V12 = _mm_shuffle_ps(MT.r[3], MT.r[1], _MM_SHUFFLE(2, 0, 2, 0));

    V00 = _mm_mul_ps(V00, V10);
    V01 = _mm_mul_ps(V01, V11);
    V02 = _mm_mul_ps(V02, V12);
    D0 = _mm_sub_ps(D0, V00);
    D1 = _mm_sub_ps(D1, V01);
    D2 = _mm_sub_ps(D2, V02);
    // V11 = D0Y,D0W,D2Y,D2Y
    V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 1, 3, 1));
    V00 = _mm_shuffle_ps(MT.r[1], MT.r[1], _MM_SHUFFLE(1, 0, 2, 1));
    V10 = _mm_shuffle_ps(V11, D0, _MM_SHUFFLE(0, 3, 0, 2));
    V01 = _mm_shuffle_ps(MT.r[0], MT.r[0], _MM_SHUFFLE(0, 1, 0, 2));
    V11 = _mm_shuffle_ps(V11, D0, _MM_SHUFFLE(2, 1, 2, 1));
    // V13 = D1Y,D1W,D2W,D2W
    Vector4 V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 3, 3, 1));
    V02 = _mm_shuffle_ps(MT.r[3], MT.r[3], _MM_SHUFFLE(1, 0, 2, 1));
    V12 = _mm_shuffle_ps(V13, D1, _MM_SHUFFLE(0, 3, 0, 2));
    Vector4 V03 = _mm_shuffle_ps(MT.r[2], MT.r[2], _MM_SHUFFLE(0, 1, 0, 2));
    V13 = _mm_shuffle_ps(V13, D1, _MM_SHUFFLE(2, 1, 2, 1));

    Vector4 C0 = _mm_mul_ps(V00, V10);
    Vector4 C2 = _mm_mul_ps(V01, V11);
    Vector4 C4 = _mm_mul_ps(V02, V12);
    Vector4 C6 = _mm_mul_ps(V03, V13);

    // V11 = D0X,D0Y,D2X,D2X
    V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(0, 0, 1, 0));
    V00 = _mm_shuffle_ps(MT.r[1], MT.r[1], _MM_SHUFFLE(2, 1, 3, 2));
    V10 = _mm_shuffle_ps(D0, V11, _MM_SHUFFLE(2, 1, 0, 3));
    V01 = _mm_shuffle_ps(MT.r[0], MT.r[0], _MM_SHUFFLE(1, 3, 2, 3));
    V11 = _mm_shuffle_ps(D0, V11, _MM_SHUFFLE(0, 2, 1, 2));
    // V13 = D1X,D1Y,D2Z,D2Z
    V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(2, 2, 1, 0));
    V02 = _mm_shuffle_ps(MT.r[3], MT.r[3], _MM_SHUFFLE(2, 1, 3, 2));
    V12 = _mm_shuffle_ps(D1, V13, _MM_SHUFFLE(2, 1, 0, 3));
    V03 = _mm_shuffle_ps(MT.r[2], MT.r[2], _MM_SHUFFLE(1, 3, 2, 3));
    V13 = _mm_shuffle_ps(D1, V13, _MM_SHUFFLE(0, 2, 1, 2));

    V00 = _mm_mul_ps(V00, V10);
    V01 = _mm_mul_ps(V01, V11);
    V02 = _mm_mul_ps(V02, V12);
    V03 = _mm_mul_ps(V03, V13);
    C0 = _mm_sub_ps(C0, V00);
    C2 = _mm_sub_ps(C2, V01);
    C4 = _mm_sub_ps(C4, V02);
    C6 = _mm_sub_ps(C6, V03);

    V00 = _mm_shuffle_ps(MT.r[1], MT.r[1], _MM_SHUFFLE(0, 3, 0, 3));
    // V10 = D0Z,D0Z,D2X,D2Y
    V10 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 0, 2, 2));
    V10 = _mm_shuffle_ps(V10, V10, _MM_SHUFFLE(0, 2, 3, 0));
    V01 = _mm_shuffle_ps(MT.r[0], MT.r[0], _MM_SHUFFLE(2, 0, 3, 1));
    // V11 = D0X,D0W,D2X,D2Y
    V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 0, 3, 0));
    V11 = _mm_shuffle_ps(V11, V11, _MM_SHUFFLE(2, 1, 0, 3));
    V02 = _mm_shuffle_ps(MT.r[3], MT.r[3], _MM_SHUFFLE(0, 3, 0, 3));
    // V12 = D1Z,D1Z,D2Z,D2W
    V12 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 2, 2, 2));
    V12 = _mm_shuffle_ps(V12, V12, _MM_SHUFFLE(0, 2, 3, 0));
    V03 = _mm_shuffle_ps(MT.r[2], MT.r[2], _MM_SHUFFLE(2, 0, 3, 1));
    // V13 = D1X,D1W,D2Z,D2W
    V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 2, 3, 0));
    V13 = _mm_shuffle_ps(V13, V13, _MM_SHUFFLE(2, 1, 0, 3));

    V00 = _mm_mul_ps(V00, V10);
    V01 = _mm_mul_ps(V01, V11);
    V02 = _mm_mul_ps(V02, V12);
    V03 = _mm_mul_ps(V03, V13);
    Vector4 C1 = _mm_sub_ps(C0, V00);
    C0 = _mm_add_ps(C0, V00);
    Vector4 C3 = _mm_add_ps(C2, V01);
    C2 = _mm_sub_ps(C2, V01);
    Vector4 C5 = _mm_sub_ps(C4, V02);
    C4 = _mm_add_ps(C4, V02);
    Vector4 C7 = _mm_add_ps(C6, V03);
    C6 = _mm_sub_ps(C6, V03);

    C0 = _mm_shuffle_ps(C0, C1, _MM_SHUFFLE(3, 1, 2, 0));
    C2 = _mm_shuffle_ps(C2, C3, _MM_SHUFFLE(3, 1, 2, 0));
    C4 = _mm_shuffle_ps(C4, C5, _MM_SHUFFLE(3, 1, 2, 0));
    C6 = _mm_shuffle_ps(C6, C7, _MM_SHUFFLE(3, 1, 2, 0));
    C0 = _mm_shuffle_ps(C0, C0, _MM_SHUFFLE(3, 1, 2, 0));
    C2 = _mm_shuffle_ps(C2, C2, _MM_SHUFFLE(3, 1, 2, 0));
    C4 = _mm_shuffle_ps(C4, C4, _MM_SHUFFLE(3, 1, 2, 0));
    C6 = _mm_shuffle_ps(C6, C6, _MM_SHUFFLE(3, 1, 2, 0));

    // Get the determinate
    Vector4 vTemp = Vector4::Dot4V(C0, MT.r[0]);
    vTemp = _mm_div_ps(VECTOR_ONE, vTemp);

    return Matrix(_mm_mul_ps(C0, vTemp),
                  _mm_mul_ps(C2, vTemp),
                  _mm_mul_ps(C4, vTemp),
                  _mm_mul_ps(C6, vTemp));
}

} // namespace math
} // namespace rt
