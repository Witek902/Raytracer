#pragma once

#include "../RayLib.h"

#include "../Math/Vector4.h"
#include "../Math/Float3.h"
#include "../Math/Box.h"
#include "../Math/Simd8Box.h"
#include "../Utils/Memory.h"
#include "../Containers/DynArray.h"

#include <string>

namespace rt {

// binary Bounding Volume Hierarchy
class BVH
{
public:
    static constexpr uint32 MaxDepth = 128;

    struct RT_ALIGN(32) Node
    {
        // TODO revisit this structure: keeping a pointer to child would be faster than index
        math::Float3 min;
        uint32 childIndex; // first child node / leaf index
        math::Float3 max;
        uint32 numLeaves : 30;
        uint32 splitAxis : 2;

        RT_FORCE_INLINE const math::Box GetBox() const
        {
            const math::Vector4 mask = math::Vector4::MakeMask<1,1,1,0>();

            return { math::Vector4(&min.x) & mask, math::Vector4(&max.x) & mask };
        }

        RT_FORCE_INLINE math::Box_Simd8 GetBox_Simd8() const
        {
            math::Box_Simd8 ret;

            ret.min.x = math::Vector8(min.x);
            ret.min.y = math::Vector8(min.y);
            ret.min.z = math::Vector8(min.z);

            ret.max.x = math::Vector8(max.x);
            ret.max.y = math::Vector8(max.y);
            ret.max.z = math::Vector8(max.z);

            return ret;
        }

        RT_FORCE_INLINE bool IsLeaf() const
        {
            return numLeaves != 0;
        }

        RT_FORCE_INLINE uint32 GetSplitAxis() const
        {
            return splitAxis;
        }
    };

    struct Stats
    {
        uint32 maxDepth;    // max leaf depth
        double totalNodesArea;
        double totalNodesVolume;
        DynArray<uint32> leavesCountHistogram;

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

    RT_FORCE_INLINE const Node* GetNodes() const { return mNodes.Data(); }
    RT_FORCE_INLINE uint32 GetNumNodes() const { return mNumNodes; }

private:
    void CalculateStatsForNode(uint32 node, Stats& outStats, uint32 depth) const;
    bool AllocateNodes(uint32 numNodes);

    DynArray<Node, SystemAllocator> mNodes;
    uint32 mNumNodes;

    friend class BVHBuilder;
};


} // namespace rt
