#pragma once

#include "RayPacket.h"
#include "HitPoint.h"
#include "TraversalContext.h"
#include "Math/Ray.h"
#include "BVH/BVH.h"
#include "Math/Geometry.h"
#include "Math/Simd8Geometry.h"
#include "Utils/iacaMarks.h"
#include "Rendering/Counters.h"
#include "Rendering/Context.h"

namespace rt {

struct RenderingContext;

RT_FORCE_NOINLINE void RemoveMissedGroups(RenderingContext& context, Uint32& numGroups);

// test all alive groups in a packet agains a BVH node
Uint32 TestRayPacket(RayPacket& packet, Uint32 numGroups, const BVH::Node& node, RenderingContext& context);

template <typename ObjectType>
void GenericTraverse_Packet(const PacketTraversalContext& context, const Uint32 objectID, const ObjectType* object)
{
    // TODO this should be done when building a packet
    for (Uint32 i = 0; i < context.ray.GetNumGroups(); ++i)
    {
        context.context.activeGroupsIndices[i] = (Uint16)i;
    }

    // all nodes
    const BVH::Node* __restrict nodes = object->GetBVH().GetNodes();

    struct StackFrame
    {
        const BVH::Node* node;
        Uint32 numActiveGroups;
        Uint32 numActiveRays;
    };

    StackFrame stack[BVH::MaxDepth];

    // push root
    Uint32 stackSize = 1;
    stack[0].node = nodes;
    stack[0].numActiveGroups = context.ray.GetNumGroups();
    stack[0].numActiveRays = context.ray.numRays; // all rays are active at the beginning

    // TODO packets should be octant-sorted
    Uint32 rayOctant = 0;
    rayOctant = context.ray.groups[0].rays.dir.x[0] < 0.0f ? 1 : 0;
    rayOctant |= context.ray.groups[0].rays.dir.y[0] < 0.0f ? 2 : 0;
    rayOctant |= context.ray.groups[0].rays.dir.z[0] < 0.0f ? 4 : 0;

    // BVH traversal
    while (stackSize > 0)
    {
        // pop element from stack
        const StackFrame& frame = stack[--stackSize];

        Uint32 numGroups = frame.numActiveGroups;
        Uint32 raysHit = TestRayPacket(context.ray, numGroups, *frame.node, context.context);

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
        context.context.localCounters.numRayBoxTests += 8 * numGroups;
        context.context.localCounters.numPassedRayBoxTests += raysHit;
#endif // RT_ENABLE_INTERSECTION_COUNTERS

        if (raysHit == 0)
        {
            // all rays missed the node - skip it
            continue;
        }

        // remove missed groups from the list
        if (raysHit < frame.numActiveRays)
        {
            RemoveMissedGroups(context.context, numGroups);
        }

        // TODO switching to Simd traversal if only one group left

        if (frame.node->IsLeaf())
        {
            object->Traverse_Leaf_Packet(context, objectID, *frame.node, numGroups);
        }
        else
        {
            const BVH::Node* __restrict children = nodes + frame.node->childIndex;
            RT_PREFETCH_L1(children);

            // stored split axis trick: pust stack elements based on current node's split axis
            const Uint32 firstIndex = (rayOctant >> frame.node->GetSplitAxis()) & 1u;
            const Uint32 secondIndex = firstIndex ^ 1u;

            stack[stackSize].node = children + secondIndex;
            stack[stackSize].numActiveGroups = numGroups;
            stack[stackSize].numActiveRays = raysHit;
            stackSize++;

            stack[stackSize].node = children + firstIndex;
            stack[stackSize].numActiveGroups = numGroups;
            stack[stackSize].numActiveRays = raysHit;
            stackSize++;
        }
    }
}

} // namespace rt
