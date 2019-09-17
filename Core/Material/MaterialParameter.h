#pragma once

#include "../Textures/Texture.h"

namespace rt {

class ITexture;
using TexturePtr = std::shared_ptr<ITexture>;

template<typename T>
struct MaterialParameter
{
    T baseValue = T(1.0f);
    TexturePtr texture = nullptr;

    MaterialParameter() = default;

    RT_FORCE_INLINE MaterialParameter(const T baseValue)
        : baseValue(baseValue)
    {}

    RT_FORCE_INLINE const T Evaluate(const math::Vector4& uv) const
    {
        T value = baseValue;

        if (texture)
        {
            value = static_cast<T>(value * texture->Evaluate(uv));
        }

        return value;
    };
};


} // namespace rt
