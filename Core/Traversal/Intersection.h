#pragma once

namespace rt {

class Material;

// more detailed ray-shape intersection data
struct IntersectionData
{
    // geometry data
    math::Matrix4 frame;
    math::Vector4 texCoord;
    const Material* material = nullptr;

    RT_FORCE_INLINE const math::Vector4 LocalToWorld(const math::Vector4& localCoords) const
    {
        return frame.TransformVector(localCoords);
    }

    RT_FORCE_INLINE const math::Vector4 WorldToLocal(const math::Vector4& worldCoords) const
    {
        math::Vector4 worldToLocalX = frame[0];
        math::Vector4 worldToLocalY = frame[1];
        math::Vector4 worldToLocalZ = frame[2];
        math::Vector4::Transpose3(worldToLocalX, worldToLocalY, worldToLocalZ);

        math::Vector4 result = worldToLocalX * worldCoords.x;
        result = math::Vector4::MulAndAdd(worldToLocalY, worldCoords.y, result);
        result = math::Vector4::MulAndAdd(worldToLocalZ, worldCoords.z, result);
        return result;
    }

    RT_FORCE_INLINE float CosTheta(const math::Vector4& dir) const
    {
        return math::Vector4::Dot3(frame[2], dir);
    }
};

} // namespace rt
