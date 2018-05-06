#pragma once

#include "RayPacket.h"
#include "HitPoint.h"
#include "Math/Ray.h"
#include "BVH/BVH.h"
#include "Math/Geometry.h"

namespace rt {

template <typename LeafTraverseFuncType>
void GenericTraverse_Single(const BVH& bvh, const math::Ray& ray, HitPoint& outHutPoint)
{
    float distanceA, distanceB;

    if (bvh.GetNumNodes() == 0)
    {
        // tree is empty
        return;
    }

    // all nodes
    const BVH::Node* nodes = bvh.GetNodes();

    // "nodes to visit" stack
    Uint32 stackSize = 0;
    const BVH::Node* nodesStack[BVH::MaxDepth];

    // BVH traversal
    for (const BVH::Node* currentNode = nodes;;)
    {
        if (currentNode->IsLeaf())
        {
            Traverse_Leaf_Single(ray, *currentNode, outHutPoint);
        }
        else
        {
            const BVH::Node* childA = nodes + currentNode->data.childIndex;
            const BVH::Node* childB = childA + 1;

            // prefetch grand-children
            RT_PREFETCH(nodes + childA->data.childIndex);
            RT_PREFETCH(nodes + childB->data.childIndex);

            bool hitA = Intersect_BoxRay(ray, childA->GetBox(), distanceA);
            bool hitB = Intersect_BoxRay(ray, childB->GetBox(), distanceB);

            // box occlusion
            hitA &= (distanceA < data.distance);
            hitB &= (distanceB < data.distance);

            if (hitA && hitB)
            {
                // will push [childA, childB] or [childB, childA] depending on distances
                const bool order = distanceA < distanceB;
                nodesStack[stackSize++] = order ? childB : childA; // push node
                currentNode = order ? childA : childB;
                continue;
            }
            else if (hitA)
            {
                currentNode = childA;
                continue;
            }
            else if (hitB)
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
