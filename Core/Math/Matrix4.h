#pragma once

#include "../RayLib.h"

#include "Math.h"
#include "Vector4.h"
#include "Vector3x8.h"
#include "Box.h"
#include "Ray.h"


namespace rt {
namespace math {

// 4x4 matrix
class RT_ALIGN(16) Matrix4 final
{
public:
    union
    {
        Vector4 r[4];   //< rows
        float f[16];
        float m[4][4];
    };

    // Default constructor - create identity matrix.
    RT_FORCE_INLINE Matrix4()
    {
        r[0] = VECTOR_X;
        r[1] = VECTOR_Y;
        r[2] = VECTOR_Z;
        r[3] = VECTOR_W;
    }

    // Create matrix from rows.
    RT_FORCE_INLINE Matrix4(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3)
    {
        r[0] = r0;
        r[1] = r1;
        r[2] = r2;
        r[3] = r3;
    }

    RT_FORCE_INLINE Matrix4(const Matrix4& other)
    {
        r[0].v = other.r[0].v;
        r[1].v = other.r[1].v;
        r[2].v = other.r[2].v;
        r[3].v = other.r[3].v;
    }

    RT_FORCE_INLINE Matrix4& operator = (const Matrix4& other)
    {
        r[0].v = other.r[0].v;
        r[1].v = other.r[1].v;
        r[2].v = other.r[2].v;
        r[3].v = other.r[3].v;
        return *this;
    }

    RT_FORCE_INLINE Vector4& operator[] (int i)
    {
        return r[i];
    }

    RT_FORCE_INLINE const Vector4& operator[] (int i) const
    {
        return r[i];
    }

    RT_FORCE_INLINE const Vector4& GetTranslation() const
    {
        return r[3];
    }

    RAYLIB_API Matrix4 operator * (const Matrix4& b) const;
    RAYLIB_API Matrix4& operator *= (const Matrix4& b);

    RT_FORCE_INLINE bool IsValid() const
    {
        return r[0].IsValid() && r[1].IsValid() && r[2].IsValid() && r[3].IsValid();
    }

    // Returns true if all the corresponding elements are equal.
    RT_FORCE_INLINE bool operator == (const Matrix4& b) const
    {
        bool tmp0 = (r[0] == b.r[0]).All();
        bool tmp1 = (r[1] == b.r[1]).All();
        bool tmp2 = (r[2] == b.r[2]).All();
        bool tmp3 = (r[3] == b.r[3]).All();
        return (tmp0 && tmp1) & (tmp2 && tmp3);
    }

    /**
     * Create scaling matrix
     * @param scale Scaling factor (only XYZ components are taken into account).
     */
    RAYLIB_API static Matrix4 MakeScaling(const Vector4& scale);

    // Multiply a 3D vector by a 4x4 matrix (affine transform).
    // Equivalent of a[0] * m.r[0] + a[1] * m.r[1] + a[2] * m.r[2] + m.r[3]
    RT_FORCE_INLINE Vector4 TransformPoint(const Vector4& a) const
    {
        Vector4 t;
        t = Vector4::MulAndAdd(a.SplatX(), r[0], r[3]);
        t = Vector4::MulAndAdd(a.SplatY(), r[1], t);
        t = Vector4::MulAndAdd(a.SplatZ(), r[2], t);
        return t;
    }

    Vector3x8 TransformPoint(const Vector3x8& a) const
    {
        const Vector3x8 row0(r[0]);
        const Vector3x8 row1(r[1]);
        const Vector3x8 row2(r[2]);
        const Vector3x8 row3(r[3]);

        Vector3x8 t;
        t = Vector3x8::MulAndAdd(row0, a.x, row3);
        t = Vector3x8::MulAndAdd(row1, a.y, t);
        t = Vector3x8::MulAndAdd(row2, a.z, t);
        return t;
    }

    RT_FORCE_INLINE Vector4 TransformVector(const Vector4& a) const
    {
        Vector4 t = a.SplatX() * r[0];
        t = Vector4::MulAndAdd(a.SplatY(), r[1], t);
        t = Vector4::MulAndAdd(a.SplatZ(), r[2], t);
        return t;
    }

    // transform and negate a vector
    // Note: faster than TransformVector(-a)
    RT_FORCE_INLINE Vector4 TransformVectorNeg(const Vector4& a) const
    {
        Vector4 t = a.SplatX() * r[0];
        t = Vector4::NegMulAndSub(a.SplatY(), r[1], t);
        t = Vector4::NegMulAndAdd(a.SplatZ(), r[2], t);
        return t;
    }

    Vector3x8 TransformVector(const Vector3x8& a) const
    {
        const Vector3x8 row0(r[0]);
        const Vector3x8 row1(r[1]);
        const Vector3x8 row2(r[2]);

        Vector3x8 t = row0 * a.x;
        t = Vector3x8::MulAndAdd(row1, a.y, t);
        t = Vector3x8::MulAndAdd(row2, a.z, t);
        return t;
    }

    // Multiply a 4D vector by a 4x4 matrix.
    // Equivalent of a[0] * m.r[0] + a[1] * m.r[1] + a[2] * m.r[2] + a[3] * m.r[3]
    RT_FORCE_INLINE Vector4 LinearCombination4(const Vector4& a) const
    {
        Vector4 t = a.SplatX() * r[0];
        t = Vector4::MulAndAdd(a.SplatY(), r[1], t);
        t = Vector4::MulAndAdd(a.SplatZ(), r[2], t);
        t = Vector4::MulAndAdd(a.SplatW(), r[3], t);
        return t;
    }

    // Create perspective projection matrix
    static const Matrix4 MakePerspective(float aspect, float fovY, float nearZ, float farZ);

    // Create matrix representing a translation by 3D vector.
    RAYLIB_API static Matrix4 MakeTranslation(const Vector4& pos);


    RT_FORCE_INLINE const Matrix4 FastInverseNoScale() const
    {
        Matrix4 result = *this;
        result.r[3] = VECTOR_W;
        Vector4::Transpose3(result[0], result[1], result[2]);
        result.r[3] = result.TransformVectorNeg(r[3]);
        return result;
    }

    RT_FORCE_INLINE Matrix4& Transpose()
    {
        Vector4& row0 = r[0];
        Vector4& row1 = r[1];
        Vector4& row2 = r[2];
        Vector4& row3 = r[3];

        _MM_TRANSPOSE4_PS(row0, row1, row2, row3);

        return *this;
    }

    RT_FORCE_INLINE Matrix4 Transposed() const
    {
        Vector4 row0 = r[0];
        Vector4 row1 = r[1];
        Vector4 row2 = r[2];
        Vector4 row3 = r[3];

        _MM_TRANSPOSE4_PS(row0, row1, row2, row3);

        return Matrix4(row0, row1, row2, row3);
    }

    const Box TransformBox(const Box& box) const;

    RT_FORCE_INLINE const Ray TransformRay(const Ray& ray) const
    {
        const Vector4 origin = TransformPoint(ray.origin);
        const Vector4 dir = TransformVector(ray.dir);
        return Ray(origin, dir);
    }

    RT_FORCE_INLINE const Ray TransformRay_Unsafe(const Ray& ray) const
    {
        const Vector4 origin = TransformPoint(ray.origin);
        const Vector4 dir = TransformVector(ray.dir);
        return Ray::BuildUnsafe(origin, dir);
    }
};


/**
 * Alias of @p Matrix4::LinearCombination4 function.
 */
RT_FORCE_INLINE Vector4 operator* (const Vector4& vector, const Matrix4& m)
{
    return m.LinearCombination4(vector);
}


} // namespace math
} // namespace rt
