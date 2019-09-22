#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"


namespace rt {

const math::Vector4 DecodeBC1(const uint8* data, uint32 x, uint32 y, const uint32 width);
const math::Vector4 DecodeBC4(const uint8* data, uint32 x, uint32 y, const uint32 width);
const math::Vector4 DecodeBC5(const uint8* data, uint32 x, uint32 y, const uint32 width);

} // namespace rt
