#pragma once

#include "SceneObject.h"
#include "../../Material/MaterialParameter.h"

namespace rt {

class ITexture;
using TexturePtr = std::shared_ptr<ITexture>;

struct ShadingData;
struct RenderingContext;

enum class BlendingMode
{
    Alpha,
    Additive,
    Multiplicative,
};

class DecalSceneObject : public ISceneObject
{
public:
    RAYLIB_API explicit DecalSceneObject();

    virtual Type GetType() const override;
    virtual math::Box GetBoundingBox() const override;

    void Apply(ShadingData& shadingData, RenderingContext& context) const;

    MaterialParameter<math::Vector4> baseColor;
    MaterialParameter<float> roughness;

    float alphaMin = 0.0f;
    float alphaMax = 1.0f;

    Uint32 order = 0;
};

} // namespace rt
