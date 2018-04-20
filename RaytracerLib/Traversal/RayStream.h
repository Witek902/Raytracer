#pragma once

#include "RayPacket.h"


namespace rt {


// Ray stream - generator of ray packets
// Input single incoherent rays, outputs sorted ray packets
class RayStream
{
public:

    // TODO packet generation for primary rays (can skip sorting)

    // push a new ray to the stream
    void PushRay(const math::Ray& ray, const math::Vector4& weight, const ImageLocationInfo& imageLocation);

    // Convert collected rays into ray packets.
    // This will flush all the pushed rays and generate list of fresh ray packets
    void Sort();

    // Pop generated packet
    // If there's no packets pending the function returns NULL
    // Note: ray stream keeps the packet ownership
    const RayPacket* PopPacket();

private:

    // TODO sort rays by position (median axis split partitioning) - O(N log N) or can be better?

    // TODO sort rays by direction (map ray direction to cube-map grids) - O(N)
};


} // namespace rt
