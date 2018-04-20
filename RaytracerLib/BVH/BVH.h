#pragma once

#include "../RayLib.h"

#include "../Math/Vector4.h"
#include "../Math/Box.h"
#include "../Utils/AlignmentAllocator.h"

#include <string>


namespace rt {

// binary Bounding Volume Hierarchy
class BVH
{
public:
    static constexpr Uint32 MaxDepth = 64;

    struct RT_ALIGN(16) Node
    {
        union
        {
            math::Box box;

            struct Data
            {
                float min[3];
                Uint32 childIndex; // first child node / leaf index
                float max[3];
                Uint32 numLeaves;
            } data;
        };

        Node()
            : box()
        { }

        RT_FORCE_INLINE math::Box GetBox() const
        {
            return math::Box(box.min & math::VECTOR_MASK_XYZ, box.max & math::VECTOR_MASK_XYZ);
        }

        RT_FORCE_INLINE bool IsLeaf() const
        {
            return data.numLeaves > 0;
        }
    };

    struct Stats
    {
        Uint32 maxDepth;    // max leaf depth
        Double totalNodesArea;
        Double totalNodesVolume;
        std::vector<Uint32> leavesCountHistogram;

        // TODO overlap factor, etc.

        Stats()
            : maxDepth(0)
            , totalNodesArea(0.0)
            , totalNodesVolume(0.0)
        { }
    };

    BVH();
    BVH(BVH&& rhs) = default;
    BVH& operator = (BVH&& rhs) = default;

    // calculate whole BVH stats
    void CalculateStats(Stats& outStats) const;

    bool SaveToFile(const std::string& filePath) const;
    bool LoadFromFile(const std::string& filePath);

    RT_FORCE_INLINE const Node* GetNodes() const { return mNodes.data(); }
    RT_FORCE_INLINE Uint32 GetNumNodes() const { return mNumNodes; }

private:
    void CalculateStatsForNode(Uint32 node, Stats& outStats, Uint32 depth) const;
    bool AllocateNodes(Uint32 numNodes);

    std::vector<Node, AlignmentAllocator<Node, RT_CACHE_LINE_SIZE>> mNodes;
    Uint32 mNumNodes;

    friend class BVHBuilder;
};


} // namespace rt
