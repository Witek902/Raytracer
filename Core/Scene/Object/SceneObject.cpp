#include "PCH.h"
#include "SceneObject.h"

namespace rt {

using namespace math;

ISceneObject::ISceneObject()
{
    mTransform = Matrix4::Identity();
    mInverseTranform = Matrix4::Identity();
}

ISceneObject::~ISceneObject() = default;

void ISceneObject::SetTransform(const math::Matrix4& matrix)
{
    RT_ASSERT(matrix.IsValid());

    mTransform = matrix;

    // TODO scaling support
    mInverseTranform = matrix.FastInverseNoScale();
}

const Matrix4 ISceneObject::GetTransform(const float t) const
{
    RT_UNUSED(t);
    RT_ASSERT(t >= 0.0f && t <= 1.0f);

    // TODO motion blur

    return mTransform;
}

const Matrix4 ISceneObject::GetInverseTransform(const float t) const
{
    RT_UNUSED(t);
    RT_ASSERT(t >= 0.0f && t <= 1.0f);

    // TODO motion blur

    return mInverseTranform;
}

} // namespace rt
