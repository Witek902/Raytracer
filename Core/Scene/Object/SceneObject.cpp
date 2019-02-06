#include "PCH.h"
#include "SceneObject.h"

namespace rt {

using namespace math;

ISceneObject::ISceneObject() = default;

ISceneObject::~ISceneObject() = default;

const Matrix4 ISceneObject::ComputeTransform(const float t) const
{
    RT_UNUSED(t);

    return mTransform;

    // TODO
    /*
    const Vector4 position = Vector4::MulAndAdd(mLinearVelocity, t, mTransform.GetTranslation());
    const Quaternion rotation0 = mTransform.GetRotation();
    Quaternion rotation;

    if (Quaternion::AlmostEqual(mAngularVelocity, Quaternion::Identity()))
    {
        rotation = rotation0;
    }
    else
    {
        const Quaternion rotation1 = rotation0 * mAngularVelocity;
        rotation = Quaternion::Interpolate(rotation0, rotation1, t);
    }

    // TODO motion blur
    return Transform(position, rotation);
    */
}


} // namespace rt
