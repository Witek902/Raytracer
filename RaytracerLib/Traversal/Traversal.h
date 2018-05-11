#pragma once

#include "RayPacket.h"
#include "HitPoint.h"
#include "Math/Ray.h"
#include "BVH/BVH.h"
#include "Math/Geometry.h"
#include "Math/Simd8Geometry.h"
#include "Utils/iacaMarks.h"


//#define RT_DISABLE_PREFETCHING

namespace rt {

// simple single-ray traversal
template <typename ObjectType>
void GenericTraverse_Single(const BVH& bvh, const math::Ray& ray, HitPoint& outHitPoint, const ObjectType* object)
{
    float distanceA, distanceB;

    if (bvh.GetNumNodes() == 0)
    {
        // tree is empty
        return;
    }

    // all nodes
    const BVH::Node* __restrict nodes = bvh.GetNodes();

    // "nodes to visit" stack
    Uint32 stackSize = 0;
    const BVH::Node* __restrict nodesStack[BVH::MaxDepth];

    // BVH traversal
    for (const BVH::Node* __restrict currentNode = nodes;;)
    {
        if (currentNode->IsLeaf())
        {
            object->Traverse_Leaf_Single(ray, *currentNode, outHitPoint);
        }
        else
        {
            const BVH::Node* __restrict childA = nodes + currentNode->data.childIndex;
            const BVH::Node* __restrict childB = childA + 1;

#ifndef RT_DISABLE_PREFETCHING
            // prefetch grand-children
            RT_PREFETCH_L1(nodes + childA->data.childIndex);
#endif // RT_DISABLE_PREFETCHING

            bool hitA = Intersect_BoxRay(ray, childA->GetBox(), distanceA);

#ifndef RT_DISABLE_PREFETCHING
            // Note: according to Intel manuals, prefetch instructions should not be grouped together
            RT_PREFETCH_L1(nodes + childB->data.childIndex);
#endif // RT_DISABLE_PREFETCHING

            bool hitB = Intersect_BoxRay(ray, childB->GetBox(), distanceB);

            // box occlusion
            hitA &= (distanceA < outHitPoint.distance);
            hitB &= (distanceB < outHitPoint.distance);

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

// traverse 8 rays at a time
// no ray reordering/masking is performed
template <typename ObjectType>
void GenericTraverse_Simd8(const BVH& bvh, const math::Ray_Simd8& ray, HitPoint_Simd8& outHitPoint, const ObjectType* object)
{
    Vector8 distanceA, distanceB;

    // all nodes
    const BVH::Node* __restrict nodes = bvh.GetNodes();

    // "nodes to visit" stack
    Uint32 stackSize = 0;
    const BVH::Node* __restrict nodesStack[BVH::MaxDepth];

    // BVH traversal
    for (const BVH::Node* __restrict currentNode = nodes;;)
    {
        //IACA_VC64_START

        if (currentNode->IsLeaf())
        {
            object->Traverse_Leaf_Simd8(ray, *currentNode, outHitPoint);
        }
        else
        {
            const BVH::Node* __restrict childA = nodes + currentNode->data.childIndex;
            const BVH::Node* __restrict childB = childA + 1;

#ifndef RT_DISABLE_PREFETCHING
            RT_PREFETCH_L1(nodes + childA->data.childIndex);
#endif // RT_DISABLE_PREFETCHING

            const Vector8 maskA = Intersect_BoxRay_Simd8(ray, childA->GetBox_Simd8(), outHitPoint.distance, distanceA);

#ifndef RT_DISABLE_PREFETCHING
            // Note: according to Intel manuals, prefetch instructions should not be grouped together
            RT_PREFETCH_L1(nodes + childB->data.childIndex);
#endif // RT_DISABLE_PREFETCHING

            const Vector8 maskB = Intersect_BoxRay_Simd8(ray, childB->GetBox_Simd8(), outHitPoint.distance, distanceB);

            const int intMaskA = maskA.GetSignMask();
            const int intMaskB = maskB.GetSignMask();
            const int intMaskAB = intMaskA & intMaskB;

            if (intMaskAB)
            {
                const Vector8 orderMask(_mm256_cmp_ps(distanceA, distanceB, _CMP_LT_OQ));
                const int intOrderMask = orderMask.GetSignMask();
                const int orderMaskA = intOrderMask & intMaskAB;
                const int orderMaskB = (~intOrderMask) & intMaskAB;
                const bool cond = __popcnt(orderMaskA) > __popcnt(orderMaskB);

                // traverse to child node A if majority rays hit it before the child B
                if (cond)
                {
                    nodesStack[stackSize] = childB;
                    currentNode = childA;
                }
                else
                {
                    nodesStack[stackSize] = childA;
                    currentNode = childB;
                }

                stackSize++;
                continue;
            }

            if (intMaskA)
            {
                nodesStack[stackSize++] = childA;
            }

            if (intMaskB)
            {
                nodesStack[stackSize++] = childB;
            }
        }

        if (stackSize == 0)
        {
            break;
        }

        // pop a node
        currentNode = nodesStack[--stackSize];
    }

    //IACA_VC64_END
}


template <typename ObjectType>
void GenericTraverse_Packet(const BVH& bvh, const math::Ray_Simd8& ray, HitPoint_Simd8& outHitPoint, const ObjectType* object)
{
}

} // namespace rt
