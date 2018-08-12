#pragma once

#include "HitPoint.h"
#include "TraversalContext.h"
#include "Math/Ray.h"
#include "BVH/BVH.h"
#include "Math/Geometry.h"
#include "Utils/iacaMarks.h"
#include "Rendering/Counters.h"


namespace rt {

// simple single-ray traversal
template <typename ObjectType>
void GenericTraverse_Single(const SingleTraversalContext& context, const Uint32 objectID, const ObjectType* object)
{
    float distanceA, distanceB;

    if (object->GetBVH().GetNumNodes() == 0)
    {
        // tree is empty
        return;
    }

    // all nodes
    const BVH::Node* __restrict nodes = object->GetBVH().GetNodes();

    // "nodes to visit" stack
    Uint32 stackSize = 0;
    const BVH::Node* __restrict nodesStack[BVH::MaxDepth];

    // BVH traversal
    for (const BVH::Node* __restrict currentNode = nodes;;)
    {
        if (currentNode->IsLeaf())
        {
            object->Traverse_Leaf_Single(context, objectID, *currentNode);
        }
        else
        {
            const BVH::Node* __restrict childA = nodes + currentNode->childIndex;
            const BVH::Node* __restrict childB = childA + 1;

            // prefetch grand-children
            RT_PREFETCH_L1(nodes + childA->childIndex);

            bool hitA = Intersect_BoxRay(context.ray, childA->GetBox(), distanceA);

            // Note: according to Intel manuals, prefetch instructions should not be grouped together
            RT_PREFETCH_L1(nodes + childB->childIndex);

            bool hitB = Intersect_BoxRay(context.ray, childB->GetBox(), distanceB);

            // box occlusion
            hitA &= (distanceA < context.hitPoint.distance);
            hitB &= (distanceB < context.hitPoint.distance);

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
            context.context.localCounters.numRayBoxTests += 2;
            context.context.localCounters.numPassedRayBoxTests += hitA ? 1 : 0;
            context.context.localCounters.numPassedRayBoxTests += hitB ? 1 : 0;
#endif // RT_ENABLE_INTERSECTION_COUNTERS

            if (hitA && hitB)
            {
                // will push [childA, childB] or [childB, childA] depending on distances
                if (distanceB < distanceA)
                {
                    std::swap(childA, childB);
                }
                nodesStack[stackSize++] = childB;
                currentNode = childA;
                continue;
            }
            if (hitA)
            {
                currentNode = childA;
                continue;
            }
            if (hitB)
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
