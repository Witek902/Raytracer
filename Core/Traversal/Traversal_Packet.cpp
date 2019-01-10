#include "PCH.h"
#include "Traversal_Packet.h"

namespace rt {

using namespace math;

Uint32 RemoveMissedGroups(RenderingContext& context, Uint32 numGroups)
{
    /*
    Uint32 i = 0;
    while (i < numGroups)
    {
        if (context.activeRaysMask[i])
        {
            i++;
        }
        else
        {
            numGroups--;
            std::swap(context.activeGroupsIndices[i], context.activeGroupsIndices[numGroups]);
            std::swap(context.activeRaysMask[i], context.activeRaysMask[numGroups]);
        }
    }

    return numGroups;
    */

    for (Uint32 i = 0; ; )
    {
        // skip in-place hits at beginning
        while (context.activeRaysMask[i])
        {
            i++;
            if (i == numGroups)
            {
                return numGroups;
            }
        }

        // skip in-place misses at end
        Uint8 mask;
        do
        {
            numGroups--;
            if (i == numGroups)
            {
                return numGroups;
            }
            mask = context.activeRaysMask[numGroups];
        } while (!mask);

        std::swap(context.activeGroupsIndices[i], context.activeGroupsIndices[numGroups]);
        context.activeRaysMask[i] = mask;
    }
}

// TODO this is ugly AF, but actually works
static void SwapRays(RenderingContext& context, Uint32 a, Uint32 b, Uint32 traversalDepth)
{
    RayGroup& groupA = context.rayPacket.groups[context.activeGroupsIndices[a / RayPacket::RaysPerGroup]];
    RayGroup& groupB = context.rayPacket.groups[context.activeGroupsIndices[b / RayPacket::RaysPerGroup]];

    std::swap(groupA.rays[traversalDepth].dir.x[a % 8], groupB.rays[traversalDepth].dir.x[b % RayPacket::RaysPerGroup]);
    std::swap(groupA.rays[traversalDepth].dir.y[a % 8], groupB.rays[traversalDepth].dir.y[b % RayPacket::RaysPerGroup]);
    std::swap(groupA.rays[traversalDepth].dir.z[a % 8], groupB.rays[traversalDepth].dir.z[b % RayPacket::RaysPerGroup]);

    std::swap(groupA.rays[traversalDepth].origin.x[a % 8], groupB.rays[traversalDepth].origin.x[b % RayPacket::RaysPerGroup]);
    std::swap(groupA.rays[traversalDepth].origin.y[a % 8], groupB.rays[traversalDepth].origin.y[b % RayPacket::RaysPerGroup]);
    std::swap(groupA.rays[traversalDepth].origin.z[a % 8], groupB.rays[traversalDepth].origin.z[b % RayPacket::RaysPerGroup]);

    std::swap(groupA.rays[traversalDepth].invDir.x[a % 8], groupB.rays[traversalDepth].invDir.x[b % RayPacket::RaysPerGroup]);
    std::swap(groupA.rays[traversalDepth].invDir.y[a % 8], groupB.rays[traversalDepth].invDir.y[b % RayPacket::RaysPerGroup]);
    std::swap(groupA.rays[traversalDepth].invDir.z[a % 8], groupB.rays[traversalDepth].invDir.z[b % RayPacket::RaysPerGroup]);

    std::swap(groupA.maxDistances[a % 8], groupB.maxDistances[b % 8]);

    std::swap(groupA.rayOffsets[a % 8], groupB.rayOffsets[b % 8]);
}

static void SwapBits(Uint8& a, Uint8& b, Uint32 indexA, Uint32 indexB)
{
    const Uint8 bitA = (a >> indexA) & 1;
    const Uint8 bitB = (b >> indexB) & 1;
    a ^= (-bitB ^ a) & (1UL << indexA);
    b ^= (-bitA ^ b) & (1UL << indexB);
}

void ReorderRays(RenderingContext& context, Uint32 numGroups, Uint32 traversalDepth)
{
    Uint32 numRays = numGroups * RayPacket::RaysPerGroup;
    Uint32 i = 0;
    while (i < numRays)
    {
        const Uint32 groupIndex = i / RayPacket::RaysPerGroup;
        const Uint32 rayIndex = i % RayPacket::RaysPerGroup;

        if (context.activeRaysMask[groupIndex] & (1 << rayIndex))
        {
            i++;
        }
        else
        {
            numRays--;
            SwapRays(context, i, numRays, traversalDepth);
            SwapBits(context.activeRaysMask[i / 8], context.activeRaysMask[numRays / 8], i % 8, numRays % 8);
        }
    }
}

Uint32 TestRayPacket(RayPacket& packet, Uint32 numGroups, const BVH::Node& node, RenderingContext& context, Uint32 traversalDepth)
{
    Vector8 distance;

    Uint32 raysHit = 0;
    Uint32 i = 0;

    const math::Box_Simd8 box = node.GetBox_Simd8();

    // unrolled version of the loop below
    while (i + 4 <= numGroups)
    {
        const RayGroup& rayGroupA = packet.groups[context.activeGroupsIndices[i + 0]];
        const RayGroup& rayGroupB = packet.groups[context.activeGroupsIndices[i + 1]];
        const RayGroup& rayGroupC = packet.groups[context.activeGroupsIndices[i + 2]];
        const RayGroup& rayGroupD = packet.groups[context.activeGroupsIndices[i + 3]];

        const Vector3x8 rayOriginDivDirA = rayGroupA.rays[traversalDepth].origin * rayGroupA.rays[traversalDepth].invDir;
        const Vector3x8 rayOriginDivDirB = rayGroupB.rays[traversalDepth].origin * rayGroupB.rays[traversalDepth].invDir;
        const Vector3x8 rayOriginDivDirC = rayGroupC.rays[traversalDepth].origin * rayGroupC.rays[traversalDepth].invDir;
        const Vector3x8 rayOriginDivDirD = rayGroupD.rays[traversalDepth].origin * rayGroupD.rays[traversalDepth].invDir;

        const Vector8 maskA = Intersect_BoxRay_Simd8(rayGroupA.rays[traversalDepth].invDir, rayOriginDivDirA, box, rayGroupA.maxDistances, distance);
        const Vector8 maskB = Intersect_BoxRay_Simd8(rayGroupB.rays[traversalDepth].invDir, rayOriginDivDirB, box, rayGroupB.maxDistances, distance);
        const Vector8 maskC = Intersect_BoxRay_Simd8(rayGroupC.rays[traversalDepth].invDir, rayOriginDivDirC, box, rayGroupC.maxDistances, distance);
        const Vector8 maskD = Intersect_BoxRay_Simd8(rayGroupD.rays[traversalDepth].invDir, rayOriginDivDirD, box, rayGroupD.maxDistances, distance);

        const Uint32 intMaskA = maskA.GetSignMask();
        const Uint32 intMaskB = maskB.GetSignMask();
        const Uint32 intMaskC = maskC.GetSignMask();
        const Uint32 intMaskD = maskD.GetSignMask();

        const Uint32 intMaskCombined = (intMaskA | (intMaskB << 8u)) | ((intMaskC << 16u) | (intMaskD << 24u));
        *reinterpret_cast<Uint32*>(context.activeRaysMask + i) = intMaskCombined;
        raysHit += PopCount(intMaskCombined);

        i += 4;
    }

    for (; i < numGroups; ++i)
    {
        const RayGroup& rayGroup = packet.groups[context.activeGroupsIndices[i]];
        const Vector3x8 rayOriginDivDir = rayGroup.rays[traversalDepth].origin * rayGroup.rays[traversalDepth].invDir;

        const Vector8 mask = Intersect_BoxRay_Simd8(rayGroup.rays[traversalDepth].invDir, rayOriginDivDir, box, rayGroup.maxDistances, distance);
        const Uint32 intMask = mask.GetSignMask();
        context.activeRaysMask[i] = (Uint8)intMask;
        raysHit += PopCount(intMask);
    }

    return raysHit;
}

} // namespace rt
