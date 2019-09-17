#include "PCH.h"
#include "SceneObject_Decal.h"
#include "../../Rendering/ShadingData.h"
#include "../../Rendering/Context.h"

namespace rt {

using namespace math;

DecalSceneObject::DecalSceneObject()
{ }

ISceneObject::Type DecalSceneObject::GetType() const
{
    return Type::Decal;
}

Box DecalSceneObject::GetBoundingBox() const
{
    const Box localSpaceBox(Vector4::Zero(), 1.0f);
    return { GetBaseTransform().TransformBox(localSpaceBox), GetTransform(1.0f).TransformBox(localSpaceBox) };
}

void DecalSceneObject::Apply(ShadingData& shadingData, RenderingContext& context) const
{
    Vector4 decalSpacePos = GetBaseInverseTransform().TransformPoint(shadingData.intersection.frame.GetTranslation());

    decalSpacePos = BipolarToUnipolar(decalSpacePos);

    if (decalSpacePos.x < 0.0f || decalSpacePos.y < 0.0f || decalSpacePos.z < 0.0f ||
        decalSpacePos.x > 1.0f || decalSpacePos.y > 1.0f || decalSpacePos.z > 1.0f)
    {
        return;
    }

    const Vector4 decalBaseColorRgs = baseColor.Evaluate(decalSpacePos);
    const RayColor decalBaseColor = RayColor::Resolve(context.wavelength, Spectrum(decalBaseColorRgs));
    const float alpha = Lerp(alphaMin, alphaMax, decalBaseColorRgs.w);

    shadingData.materialParams.baseColor = RayColor::Lerp(shadingData.materialParams.baseColor, decalBaseColor, alpha);

    const float decalRoughness = roughness.Evaluate(decalSpacePos);
    shadingData.materialParams.roughness = Lerp(shadingData.materialParams.roughness, decalRoughness, alpha);
}

} // namespace rt
