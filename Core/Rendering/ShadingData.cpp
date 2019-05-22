#include "PCH.h"
#include "ShadingData.h"

namespace rt {

using namespace math;

const Vector4 ShadingData::LocalToWorld(const Vector4& localCoords) const
{
    return frame.TransformVector(localCoords);
}

const Vector4 ShadingData::WorldToLocal(const Vector4& worldCoords) const
{
    Vector4 worldToLocalX = frame[0];
    Vector4 worldToLocalY = frame[1];
    Vector4 worldToLocalZ = frame[2];
    Vector4::Transpose3(worldToLocalX, worldToLocalY, worldToLocalZ);

    Vector4 result = worldToLocalX * worldCoords.x;
    result = Vector4::MulAndAdd(worldToLocalY, worldCoords.y, result);
    result = Vector4::MulAndAdd(worldToLocalZ, worldCoords.z, result);
    return result;
}

float ShadingData::CosTheta(const Vector4& dir) const
{
    return Max(0.0f, Vector4::Dot3(frame[2], dir));
}

void PackShadingData(PackedShadingData& outPacked, const ShadingData& input)
{
    outPacked.material = input.material;
    
    outPacked.position = input.frame[3].ToFloat3();
    outPacked.normal = input.frame[2].ToFloat3();
    outPacked.tangent = input.frame[0].ToFloat3();
    outPacked.texCoord = input.texCoord.ToFloat2();

    outPacked.outgoingDirWorldSpace = input.outgoingDirWorldSpace.ToFloat3();

    outPacked.materialParams = input.materialParams;
}

void UnpackShadingData(ShadingData& outUnpacked, const PackedShadingData& input)
{
    outUnpacked.material = input.material;

    outUnpacked.frame[0] = Vector4(&input.tangent.x);
    outUnpacked.frame[2] = Vector4(&input.normal.x);
    outUnpacked.frame[3] = Vector4(&input.position.x);
    outUnpacked.frame[1] = Vector4::Cross3(outUnpacked.frame[0], outUnpacked.frame[2]);
    outUnpacked.texCoord = Vector4(&input.texCoord.x);

    outUnpacked.outgoingDirWorldSpace = Vector4(&input.outgoingDirWorldSpace.x);

    outUnpacked.materialParams = input.materialParams;
}

} // namespace rt
