#include "PCH.h"
#include "SceneObject.h"

namespace rt {

using namespace math;

ISceneObject::~ISceneObject() = default;

Transform ISceneObject::GetTransform(const float t) const
{
    RT_UNUSED(t);

    // TODO motion blur
    return mTransform;
}

Transform ISceneObject::GetInverseTransform(const float t) const
{
    RT_UNUSED(t);

    // TODO motion blur
    return mTransform.Inverted();
}


} // namespace rt
