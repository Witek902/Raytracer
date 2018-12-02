#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"


namespace rt {

const math::Vector4 DecodeBC1(const Uint8* data, Uint32 x, Uint32 y, const Uint32 width);
const math::Vector4 DecodeBC4(const Uint8* data, Uint32 x, Uint32 y, const Uint32 width);
const math::Vector4 DecodeBC5(const Uint8* data, Uint32 x, Uint32 y, const Uint32 width);

} // namespace rt
