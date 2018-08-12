#include "PCH.h"
#include "SceneObject.h"

namespace rt {

using namespace math;

ISceneObject::~ISceneObject() = default;

Matrix ISceneObject::GetTransform(const float t) const
{
    Matrix mat;
    mat.r[3] = mPosition + mPositionOffset * t;
    return mat;
}

Matrix ISceneObject::GetInverseTransform(const float t) const
{
    Matrix mat;
    mat.r[3] = -(mPosition + mPositionOffset * t);
    return mat;
}


} // namespace rt
