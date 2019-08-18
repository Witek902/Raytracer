#pragma once

#include "../RayLib.h"

#include "Timer.h"
#include "Logger.h"
#include "AlignmentAllocator.h"
#include "../Math/Box.h"
#include "../Containers/DynArray.h"

#include <numeric>

namespace rt {

class KdTree
{
private:

    struct Node
    {
        Uint32 axisIndex : 2;
        Uint32 pointIndex : 30;
        Uint32 left;
        Uint32 right;

        // TODO can store only first child node index + 2-bit mask specifying which children are present
        // TODO cache point coordinate
        // TODO reorder particles so that node index corresponds to particle index
    };

public:
    KdTree()
    {}

    template<typename ParticleType>
    RT_FORCE_NOINLINE void Build(DynArray<ParticleType>& particles)
    {
        Timer timer;

        DynArray<Uint32> indices;
        indices.Resize(particles.Size());
        std::iota(std::begin(indices), std::end(indices), 0);

        if (!particles.Empty())
        {
            mNumGeneratedNodes = 1;
            mNodes.Resize(particles.Size());
            BuildRecursive(mNodes.Front(), particles, indices.Data(), indices.Size(), 0);
        }

        RT_LOG_INFO("Building kd-tree for %u points took %.2f ms", particles.Size(), timer.Stop() * 1000.0);
    }

    template<typename ParticleType, typename Query>
    void Find(const math::Vector4& queryPos, const float radius, const DynArray<ParticleType>& particles, Query& query) const
    {
        const math::Box queryBox(queryPos, radius);
        const float sqrRadius = math::Sqr(radius);

        // "nodes to visit" stack
        Uint32 stackSize = 0;
        Uint32 nodesStack[32];

        if (!mNodes.Empty())
        {
            nodesStack[stackSize++] = 0;
        }

        while (stackSize > 0)
        {
            const Node& node = mNodes[nodesStack[--stackSize]];
            const ParticleType& particle = particles[node.pointIndex];
            const math::Vector4 particlePos = particle.GetPosition();

            {
                const float distSqr = (queryPos - particlePos).SqrLength3();
                if (distSqr <= sqrRadius)
                {
                    query(node.pointIndex);
                }
            }

            if (node.left != 0 && queryBox.min[node.axisIndex] <= particlePos[node.axisIndex])
            {
                nodesStack[stackSize++] = node.left;
            }

            if (node.right != 0 && queryBox.max[node.axisIndex] >= particlePos[node.axisIndex])
            {
                nodesStack[stackSize++] = node.right;
            }
        }
    }

private:
    DynArray<Node> mNodes;
    Uint32 mNumGeneratedNodes = 0;

    template<typename ParticleType>
    void BuildRecursive(Node& targetNode, DynArray<ParticleType>& particles, Uint32* indices, Uint32 npoints, Uint32 depth)
    {
        const Uint32 axis = depth % 3u; // TODO select longer axis?
        const Uint32 mid = (npoints - 1) / 2;

        std::nth_element(indices, indices + mid, indices + npoints, [&](int lhs, int rhs)
        {
            const math::Vector4 lPos = particles[lhs].GetPosition();
            const math::Vector4 rPos = particles[rhs].GetPosition();
            return lPos[axis] < rPos[axis];
        });

        targetNode.pointIndex = indices[mid];
        targetNode.axisIndex = axis;

        const bool hasLeft = mid > 0u;
        const bool hasRight = npoints - mid > 1u;

        if (hasLeft)
        {
            targetNode.left = mNumGeneratedNodes++;
        }
        else
        {
            targetNode.left = 0;
        }

        if (hasRight)
        {
            targetNode.right = mNumGeneratedNodes++;
        }
        else
        {
            targetNode.right = 0;
        }

        if (hasLeft)
        {
            BuildRecursive(mNodes[targetNode.left], particles, indices, mid, depth + 1);
        }

        if (hasRight)
        {
            BuildRecursive(mNodes[targetNode.right], particles, indices + mid + 1, npoints - mid - 1, depth + 1);
        }
    }
};


} // namespace rt
