#include "PCH.h"
#include "Traversal_Packet.h"

namespace rt {

using namespace math;

void RemoveMissedGroups(RenderingContext& context, Uint32& numGroups)
{
    for (Uint32 i = 0; i < numGroups; )
    {
        if (context.activeRaysMask[i] > 0)
        {
            i++;
            continue;
        }

        // TODO shifting instead of swapping
        std::swap(context.activeGroupsIndices[i], context.activeGroupsIndices[numGroups - 1]);
        std::swap(context.activeRaysMask[i], context.activeRaysMask[numGroups - 1]);
        numGroups--;
    }
}

Uint32 TestRayPacket(RayPacket& packet, Uint32 numGroups, const BVH::Node& node, RenderingContext& context)
{
    Vector8 distanceA, distanceB;

    Uint32 raysHit = 0;
    Uint32 i = 0;

    const math::Box_Simd8 box = node.GetBox_Simd8();

    // unrolled version of the loop below
    while (i + 2 <= numGroups)
    {
        const RayGroup& rayGroupA = packet.groups[context.activeGroupsIndices[i]];
        const RayGroup& rayGroupB = packet.groups[context.activeGroupsIndices[i + 1]];
        const Vector3x8 rayOriginDivDirA = rayGroupA.rays.origin * rayGroupA.rays.invDir;
        const Vector3x8 rayOriginDivDirB = rayGroupB.rays.origin * rayGroupB.rays.invDir;

        const Vector8 maskA = Intersect_BoxRay_Simd8(rayGroupA.rays.invDir, rayOriginDivDirA, box, rayGroupA.maxDistances, distanceA);
        const Vector8 maskB = Intersect_BoxRay_Simd8(rayGroupB.rays.invDir, rayOriginDivDirB, box, rayGroupB.maxDistances, distanceB);

        const Uint32 intMaskA = maskA.GetSignMask();
        const Uint32 intMaskB = maskB.GetSignMask();

        const Uint32 mergedMask = intMaskA | (intMaskB << 8u);
        *reinterpret_cast<Uint16*>(context.activeRaysMask + i) = (Uint16)mergedMask;
        raysHit += __popcnt(mergedMask);

        i += 2;
    }

    for (; i < numGroups; ++i)
    {
        const RayGroup& rayGroup = packet.groups[context.activeGroupsIndices[i]];
        const Vector3x8 rayOriginDivDir = rayGroup.rays.origin * rayGroup.rays.invDir;

        Vector8 distance;
        const Vector8 mask = Intersect_BoxRay_Simd8(rayGroup.rays.invDir, rayOriginDivDir, box, rayGroup.maxDistances, distance);
        const Uint32 intMask = mask.GetSignMask();
        context.activeRaysMask[i] = (Uint8)intMask;
        raysHit += __popcnt(intMask);
    }

    return raysHit;
}

} // namespace rt
