#pragma once

#include "RayPacket.h"


namespace rt {


// Ray stream - generator of ray packets
// Push incoherent rays, pops coherent ray packets
class RayStream
{
public:
    static constexpr uint32 MaxRays = 1024 * 1024;

    RayStream();
    ~RayStream();

    // push a new ray to the stream
    void PushRay(const math::Ray& ray, const math::Vector4& weight, const ImageLocationInfo& imageLocation);

    // Convert collected rays into ray packets.
    // This will flush all the pushed rays and generate list of fresh ray packets
    void Sort();

    // Pop generated packet
    // If there's no packets pending the function returns false
    // Note: ray stream keeps the packet ownership
    bool PopPacket(RayPacket& outPacket);

private:

    struct PendingRay
    {
        math::Vector4 rayWeight;
        math::Float3 rayOrigin;
        math::Float3 rayDir;
        ImageLocationInfo imageLocation;
    };

    uint32 mNumRays;
    PendingRay mRays[MaxRays];
};


} // namespace rt
