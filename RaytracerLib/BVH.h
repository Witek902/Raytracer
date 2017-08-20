#pragma once

#include "RayLib.h"
#include "Math/Vector4.h"
#include "Math/Box.h"


namespace rt {

// binary Bounding Volume Hierarchy
class BVH
{
public:
    struct RT_ALIGN(16) Node
    {
        union
        {
            math::Box box;

            struct Data
            {
                float min[3];
                int childIndex;
                float max[3];
                int flags;
            } data;
        };
    };

    struct Stats
    {
        Uint32 minDepth;    // min leaf depth
        Uint32 maxDepth;    // max leaf depth
        Double avgDepth;    // average leaf depth

        Double totalNodesArea;
        Double totalNodesVolume;
        // TODO overlap factor, etc.

        Stats()
            : minDepth(0)
            , maxDepth(0)
            , avgDepth(0.0)
            , totalNodesArea(0.0)
            , totalNodesVolume(0.0)
        { }
    };

    BVH();
    ~BVH();
    BVH(BVH&& rhs);
    BVH& operator = (BVH&& rhs);

    // calculate whole BVH stats
    void CalculateStats(Stats& outStats) const;

    bool SaveToFile(const char* filePath) const;
    bool LoadFromFile(const char* filePath);

    RT_FORCE_INLINE const Node* GetNodes() const { return mNodes; }
    RT_FORCE_INLINE Uint32 GetNumNodes() const { return mNumNodes; }

private:
    void CalculateStatsForNode(Uint32 node, Stats& outStats) const;
    bool AllocateNodes(Uint32 numNodes);

    Node* mNodes;
    Uint32 mNumNodes;

    friend class BVHBuilder;
};


} // namespace rt
