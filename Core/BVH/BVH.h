#pragma once

#include "../RayLib.h"

#include "../Math/Vector4.h"
#include "../Math/Float3.h"
#include "../Math/Box.h"
#include "../Math/Simd8Box.h"
#include "../Utils/AlignmentAllocator.h"

#include <string>


namespace rt {

// binary Bounding Volume Hierarchy
class BVH
{
public:
    static constexpr Uint32 MaxDepth = 128;

    struct RT_ALIGN(32) Node
    {
        // TODO revisit this structure: keeping a pointer to child would be faster than index
        math::Float3 min;
        Uint32 childIndex; // first child node / leaf index
        math::Float3 max;
        Uint32 numLeaves : 30;
        Uint32 splitAxis : 2;

        RT_FORCE_INLINE const math::Box GetBox() const
        {
            const math::Vector4 mask = math::Vector4::MakeMask<1,1,1,0>();

            return { math::Vector4(&min.x) & mask, math::Vector4(&max.x) & mask };
        }

        RT_FORCE_INLINE math::Box_Simd8 GetBox_Simd8() const
        {
            math::Box_Simd8 ret;

            ret.min.x = _mm256_broadcast_ss(&min.x);
            ret.min.y = _mm256_broadcast_ss(&min.y);
            ret.min.z = _mm256_broadcast_ss(&min.z);

            ret.max.x = _mm256_broadcast_ss(&max.x);
            ret.max.y = _mm256_broadcast_ss(&max.y);
            ret.max.z = _mm256_broadcast_ss(&max.z);

            return ret;
        }

        RT_FORCE_INLINE bool IsLeaf() const
        {
            return numLeaves != 0;
        }

        RT_FORCE_INLINE Uint32 GetSplitAxis() const
        {
            return splitAxis;
        }
    };

    struct Stats
    {
        Uint32 maxDepth;    // max leaf depth
        double totalNodesArea;
        double totalNodesVolume;
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
