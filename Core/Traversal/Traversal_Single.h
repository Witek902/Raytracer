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
void GenericTraverse(const SingleTraversalContext& context, const uint32 objectID, const ObjectType* object)
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

            bool hitA = Intersect_BoxRay(context.ray, childA->GetBox(), distanceA);

            // prefetch grand-children
            RT_PREFETCH_L1(nodes + childA->childIndex);

            bool hitB = Intersect_BoxRay(context.ray, childB->GetBox(), distanceB);

            // Note: according to Intel manuals, prefetch instructions should not be grouped together
            RT_PREFETCH_L1(nodes + childB->childIndex);

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
                currentNode = childA;
                nodesStack[stackSize++] = childB;
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

template <typename ObjectType>
bool GenericTraverse_Shadow(const SingleTraversalContext& context, const ObjectType* object)
{
    float distanceA, distanceB;

    if (object->GetBVH().GetNumNodes() == 0)
    {
        // tree is empty
        return false;
    }

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
            if (object->Traverse_Leaf_Shadow(context, *currentNode))
            {
                return true;
            }
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
                currentNode = childA;
                nodesStack[stackSize++] = childB;
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

    return false;
}

} // namespace rt
