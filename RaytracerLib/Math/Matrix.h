/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix class declaration.
 */

#pragma once

#include "RayLib.h"
#include "Math.h"
#include "Vector.h"

#include <initializer_list>


namespace rt {
namespace math {

/**
 * 4x4 matrix
 */
RT_ALIGN(16)
class RAYLIB_API Matrix final
{
public:
    union
    {
        Vector r[4];   //< rows
        float f[16];
        float m[4][4];
    };

    RT_FORCE_INLINE Vector& GetRow(int i)
    {
        return reinterpret_cast<Vector&>(r[i]);
    }

    RT_FORCE_INLINE const Vector& GetRow(int i) const
    {
        return reinterpret_cast<const Vector&>(r[i]);
    }

    RT_FORCE_INLINE Vector& operator[] (int i)
    {
        return reinterpret_cast<Vector&>(r[i]);
    }

    RT_FORCE_INLINE const Vector& operator[] (int i) const
    {
        return reinterpret_cast<const Vector&>(r[i]);
    }

    /**
     * Default constructor - create identity matrix.
     */
    RT_FORCE_INLINE Matrix()
    {
        r[0] = VECTOR_X;
        r[1] = VECTOR_Y;
        r[2] = VECTOR_Z;
        r[3] = VECTOR_W;
    }

    /**
     * Create matrix from rows.
     */
    RT_FORCE_INLINE Matrix(const Vector& r0, const Vector& r1, const Vector& r2, const Vector& r3)
    {
        r[0] = r0;
        r[1] = r1;
        r[2] = r2;
        r[3] = r3;
    }

    /**
     * Create matrix from element values.
     */
    RT_FORCE_INLINE Matrix(const std::initializer_list<float>& list)
    {
        int i = 0;
        for (float x : list)
        {
            f[i++] = x;
        }
    }

    RT_FORCE_INLINE Matrix operator+ (const Matrix& b) const
    {
        return Matrix(*this) += b;
    }

    RT_FORCE_INLINE Matrix operator- (const Matrix& b) const
    {
        return Matrix(*this) -= b;
    }

    RT_FORCE_INLINE Matrix& operator+= (const Matrix& b)
    {
        r[0] += b.r[0];
        r[1] += b.r[1];
        r[2] += b.r[2];
        r[3] += b.r[3];
        return *this;
    }

    RT_FORCE_INLINE Matrix& operator-= (const Matrix& b)
    {
        r[0] -= b.r[0];
        r[1] -= b.r[1];
        r[2] -= b.r[2];
        r[3] -= b.r[3];
        return *this;
    }

    RT_FORCE_INLINE Matrix operator* (float b) const
    {
        return Matrix(*this) *= b;
    }

    RT_FORCE_INLINE Matrix operator/ (float b) const
    {
        return Matrix(*this) /= b;
    }

    RT_FORCE_INLINE Matrix& operator*= (float b)
    {
        r[0] *= b;
        r[1] *= b;
        r[2] *= b;
        r[3] *= b;
        return *this;
    }

    RT_FORCE_INLINE Matrix& operator/= (float b)
    {
        r[0] /= b;
        r[1] /= b;
        r[2] /= b;
        r[3] /= b;
        return *this;
    }

    Matrix operator* (const Matrix& b) const;
    Matrix& operator*= (const Matrix& b);

    /**
     * Returns true if all the corresponding elements are equal.
     */
    RT_FORCE_INLINE bool operator== (const Matrix& b) const
    {
        int tmp0 = r[0] == b.r[0];
        int tmp1 = r[1] == b.r[1];
        int tmp2 = r[2] == b.r[2];
        int tmp3 = r[3] == b.r[3];
        return (tmp0 && tmp1) & (tmp2 && tmp3);
    }

    /**
     * Calculate matrix inverse.
     */
    Matrix Inverted() const;

    /**
     * Calculate matrix inverse.
     */
    RT_FORCE_INLINE Matrix& Invert()
    {
        *this = Inverted();
        return *this;
    }

    /**
     * Create rotation matrix.
     * @param normalAxis Normalized axis.
     * @param angle Rotation angle (in radians).
     */
    static Matrix MakeRotationNormal(const Vector& normalAxis, float angle);

    /**
     * Create perspective projection matrix.
     * @param aspect Aspect ratio.
     * @param fovY Vertical field of view angle.
     * @param farZ,nearZ Far and near distances.
     */
    static Matrix MakePerspective(float aspect, float fovY, float farZ, float nearZ);

    /**
     * Create orthographic projection matrix.
     * @param left,right X-axis boundaries.
     * @param bottom,top Y-axis boundaries.
     * @param zNear,zFar Z-axis boundaries.
     */
    static Matrix MakeOrtho(float left, float right, float bottom, float top, float zNear, float zFar);

    /**
     * Create scaling matrix
     * @param scale Scaling factor (only XYZ components are taken into account).
     */
    static Matrix MakeScaling(const Vector& scale);

    /**
     * Create view matrix.
     * @param eyePosition  Observer's position.
     * @param eyeDirection Observer's direction.
     * @param upDirection  "Up" vector.
     */
    static Matrix MakeLookTo(const Vector& eyePosition, const Vector& eyeDirection, const Vector& upDirection);

    /**
     * Multiply a 3D vector by a 4x4 matrix (affine transform).
     * Equivalent of a[0] * m.r[0] + a[1] * m.r[1] + a[2] * m.r[2] + m.r[3]
     */
    RT_FORCE_INLINE Vector LinearCombination3(const Vector& a) const
    {
        const Vector tmp0 = Vector::MulAndAdd(a.SplatX(), r[0], a.SplatY() * r[1]);
        const Vector tmp1 = Vector::MulAndAdd(a.SplatZ(), r[2], r[3]);
        return tmp0 + tmp1;
    }

    /**
     * Multiply a 4D vector by a 4x4 matrix.
     * Equivalent of a[0] * m.r[0] + a[1] * m.r[1] + a[2] * m.r[2] + a[3] * m.r[3]
     */
    RT_FORCE_INLINE Vector LinearCombination4(const Vector& a) const
    {
        const Vector tmp0 = Vector::MulAndAdd(a.SplatX(), r[0], a.SplatY() * r[1]);
        const Vector tmp1 = Vector::MulAndAdd(a.SplatZ(), r[2], a.SplatW() * r[3]);
        return tmp0 + tmp1;
    }

    /**
     * Calculate matrix containing absolute values of another.
     */
    RT_FORCE_INLINE static Matrix Abs(const Matrix& m)
    {
        return Matrix(Vector::Abs(m[0]), Vector::Abs(m[1]), Vector::Abs(m[2]), Vector::Abs(m[3]));
    }

    /**
     * Check if two matrices are (almost) equal.
     */
    RT_FORCE_INLINE static bool Equal(const Matrix& m1, const Matrix& m2, float epsilon)
    {
        Matrix diff = Abs(m1 - m2);
        Vector epsilonV = Vector::Splat(epsilon);
        return ((diff[0] < epsilonV) && (diff[1] < epsilonV)) &&
               ((diff[2] < epsilonV) && (diff[3] < epsilonV));
    }

    /**
     * Create matrix representing a translation by 3D vector.
     */
    RT_FORCE_INLINE static Matrix MakeTranslation3(const Vector& pos)
    {
        Matrix m;
        m.r[0] = VECTOR_X;
        m.r[1] = VECTOR_Y;
        m.r[2] = VECTOR_Z;
        m.r[3] = Vector(pos.f[0], pos.f[1], pos.f[2], 1.0f);
        return m;
    }

    /**
     * Calculate transpose matrix.
     */
    RT_FORCE_INLINE Matrix& Matrix::Transpose()
    {
        Vector& row0 = r[0];
        Vector& row1 = r[1];
        Vector& row2 = r[2];
        Vector& row3 = r[3];

        _MM_TRANSPOSE4_PS(row0, row1, row2, row3);

        return *this;
    }

    /**
     * Calculate transpose matrix.
     */
    RT_FORCE_INLINE Matrix Matrix::Transposed() const
    {
        Vector row0 = r[0];
        Vector row1 = r[1];
        Vector row2 = r[2];
        Vector row3 = r[3];

        _MM_TRANSPOSE4_PS(row0, row1, row2, row3);

        return Matrix(row0, row1, row2, row3);
    }
};


/**
 * Alias of @p Matrix::LinearCombination4 function.
 */
RT_FORCE_INLINE Vector operator* (const Vector& vector, const Matrix& m)
{
    return m.LinearCombination4(vector);
}


} // namespace math
} // namespace rt
