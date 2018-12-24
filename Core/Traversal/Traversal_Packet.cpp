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
static void SwapRays(RenderingContext& context, Uint32 a, Uint32 b)
{
    RayGroup& groupA = context.rayPacket.groups[context.activeGroupsIndices[a / 8]];
    RayGroup& groupB = context.rayPacket.groups[context.activeGroupsIndices[b / 8]];

    std::swap(groupA.rays.dir.x[a % 8], groupB.rays.dir.x[b % 8]);
    std::swap(groupA.rays.dir.y[a % 8], groupB.rays.dir.y[b % 8]);
    std::swap(groupA.rays.dir.z[a % 8], groupB.rays.dir.z[b % 8]);

    std::swap(groupA.rays.origin.x[a % 8], groupB.rays.origin.x[b % 8]);
    std::swap(groupA.rays.origin.y[a % 8], groupB.rays.origin.y[b % 8]);
    std::swap(groupA.rays.origin.z[a % 8], groupB.rays.origin.z[b % 8]);

    std::swap(groupA.rays.invDir.x[a % 8], groupB.rays.invDir.x[b % 8]);
    std::swap(groupA.rays.invDir.y[a % 8], groupB.rays.invDir.y[b % 8]);
    std::swap(groupA.rays.invDir.z[a % 8], groupB.rays.invDir.z[b % 8]);

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

void ReorderRays(RenderingContext& context, Uint32 numGroups)
{
    Uint32 numRays = numGroups * 8;
    Uint32 i = 0;
    while (i < numRays)
    {
        const Uint32 groupIndex = i / 8;
        const Uint32 rayIndex = i % 8;

        if (context.activeRaysMask[groupIndex] & (1 << rayIndex))
        {
            i++;
        }
        else
        {
            numRays--;
            SwapRays(context, i, numRays);
            SwapBits(context.activeRaysMask[i / 8], context.activeRaysMask[numRays / 8], i % 8, numRays % 8);
        }
    }
}

Uint32 TestRayPacket(RayPacket& packet, Uint32 numGroups, const BVH::Node& node, RenderingContext& context)
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

        const Vector3x8 rayOriginDivDirA = rayGroupA.rays.origin * rayGroupA.rays.invDir;
        const Vector3x8 rayOriginDivDirB = rayGroupB.rays.origin * rayGroupB.rays.invDir;
        const Vector3x8 rayOriginDivDirC = rayGroupC.rays.origin * rayGroupC.rays.invDir;
        const Vector3x8 rayOriginDivDirD = rayGroupD.rays.origin * rayGroupD.rays.invDir;
        
        const Vector8 maskA = Intersect_BoxRay_Simd8(rayGroupA.rays.invDir, rayOriginDivDirA, box, rayGroupA.maxDistances, distance);
        const Vector8 maskB = Intersect_BoxRay_Simd8(rayGroupB.rays.invDir, rayOriginDivDirB, box, rayGroupB.maxDistances, distance);
        const Vector8 maskC = Intersect_BoxRay_Simd8(rayGroupC.rays.invDir, rayOriginDivDirC, box, rayGroupC.maxDistances, distance);
        const Vector8 maskD = Intersect_BoxRay_Simd8(rayGroupD.rays.invDir, rayOriginDivDirD, box, rayGroupD.maxDistances, distance);

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
        const Vector3x8 rayOriginDivDir = rayGroup.rays.origin * rayGroup.rays.invDir;

        const Vector8 mask = Intersect_BoxRay_Simd8(rayGroup.rays.invDir, rayOriginDivDir, box, rayGroup.maxDistances, distance);
        const Uint32 intMask = mask.GetSignMask();
        context.activeRaysMask[i] = (Uint8)intMask;
        raysHit += PopCount(intMask);
    }

    return raysHit;
}

} // namespace rt
