#include "PCH.h"
#include "SceneObject.h"
#include "Material/Material.h"

namespace rt {

using namespace math;

ISceneObject::ISceneObject()
{
    mTransform = Matrix4::Identity();
    mInverseTranform = Matrix4::Identity();
}

ISceneObject::~ISceneObject() = default;

void ISceneObject::SetDefaultMaterial(const MaterialPtr& material)
{
    mDefaultMaterial = material;

    if (!mDefaultMaterial)
    {
        mDefaultMaterial = Material::GetDefaultMaterial();
    }
}

void ISceneObject::SetTransform(const math::Matrix4& matrix)
{
    RT_ASSERT(matrix.IsValid());

    mTransform = matrix;

    // TODO scaling support
    mInverseTranform = matrix.FastInverseNoScale();
}

} // namespace rt
