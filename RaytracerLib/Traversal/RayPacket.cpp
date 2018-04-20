#include "PCH.h"
#include "RayPacket.h"

namespace rt {


void RayPacket::PushRay(const math::Ray& ray, const math::Vector4& weight, const ImageLocationInfo& imageLocation)
{
    rays[numRays] = ray;
    weights[numRays] = weight;
    imageLocations[numRays] = imageLocation;

    numRays++;
}

} // namespace rt
