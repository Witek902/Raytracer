#pragma once

#include "HitPoint.h"
#include "Math/Ray.h"
#include "BVH/BVH.h"
#include "Math/Geometry.h"
#include "Math/Simd8Geometry.h"
#include "Utils/iacaMarks.h"
#include "Rendering/Counters.h"


namespace rt {

// traverse 8 rays at a time
// no ray reordering/masking is performed
template <typename ObjectType>
static void GenericTraverse(const SimdTraversalContext& context, const uint32 objectID, const ObjectType* object)
{
    const math::Vector3x8 rayInvDir = context.ray.invDir;
    const math::Vector3x8 rayOriginDivDir = context.ray.origin * context.ray.invDir;

    // all nodes
    const BVH::Node* __restrict nodes = object->GetBVH().GetNodes();

    // "nodes to visit" stack
    uint32 stackSize = 0;
    const BVH::Node* __restrict nodesStack[BVH::MaxDepth];

    // BVH traversal
    for (const BVH::Node* __restrict currentNode = nodes;;)
    {
        if (currentNode->IsLeaf())
        {
            object->Traverse_Leaf(context, objectID, *currentNode);
        }
        else
        {
            const BVH::Node* __restrict childA = nodes + currentNode->childIndex;
            const BVH::Node* __restrict childB = childA + 1;

            RT_PREFETCH_L1(nodes + childA->childIndex);

            math::Vector8 distanceA;
            const math::Vector8 maskA = Intersect_BoxRay(rayInvDir, rayOriginDivDir, childA->GetBox(), context.hitPoint.distance, distanceA);
            const int32 intMaskA = maskA.GetSignMask();

            // Note: according to Intel manuals, prefetch instructions should not be grouped together
            RT_PREFETCH_L1(nodes + childB->childIndex);

            math::Vector8 distanceB;
            const math::Vector8 maskB = Intersect_BoxRay(rayInvDir, rayOriginDivDir, childB->GetBox(), context.hitPoint.distance, distanceB);
            const int32 intMaskB = maskB.GetSignMask();

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
            context.context.localCounters.numRayBoxTests += 2 * 8;
            context.context.localCounters.numPassedRayBoxTests += math::PopCount(intMaskA);
            context.context.localCounters.numPassedRayBoxTests += math::PopCount(intMaskB);
#endif // RT_ENABLE_INTERSECTION_COUNTERS

            if (const int32 intMaskAB = intMaskA & intMaskB)
            {
                const int32 intOrderMask = (distanceA < distanceB).GetMask();
                const int32 orderMaskA = intOrderMask & intMaskAB;
                const int32 orderMaskB = (~intOrderMask) & intMaskAB;

                // traverse to child node A if majority rays hit it before the child B
                if (math::PopCount(orderMaskB) > math::PopCount(orderMaskA))
                {
                    std::swap(childB, childA);
                }

                currentNode = childA;
                nodesStack[stackSize++] = childB;

                // TODO switching to single-ray traversal if pop count is equal to one?
                continue;
            }

            if (intMaskA)
            {
                currentNode = childA;
                continue;
            }

            if (intMaskB)
            {
                currentNode = childB;
                continue;
            }
        }

        if (stackSize == 0)
        {
            break;
        }

        // pop a node
        currentNode = nodesStack[--stackSize];
    }
}

} // namespace rt
