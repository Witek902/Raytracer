#pragma once

#include "RayLib.h"
#include "BVH.h"


namespace rt {


// helper class for constructing BVH using SAH algorithm
class BVHBuilder
{
public:

    struct BuildingParams
    {
        const math::Box* leafBoxes;
        Uint32 numLeaves;

        Uint32 maxLeafNodeSize; // max number of objects in leaf nodes

        BuildingParams()
            : maxLeafNodeSize(2)
        { }
    };

    BVHBuilder(BVH& targetBVH);
    ~BVHBuilder();

    void SetLeafData();

    // construct the BVH and return new leaves order
    bool Build(const math::Box* data, Uint32 numLeaves,
               const BuildingParams& params,
               std::vector<Uint32>& outLeavesOrder);

private:

    constexpr static Uint32 NUM_AXES = 3;

    struct WorkSet
    {
        // leaves indices (sorted in each axis)
        Uint32* sortedLeaves[NUM_AXES];
        Uint32 numLeaves;
    };

    // sort leaf indices in each axis
    void SortLeaves(const std::vector<Uint32>& input, WorkSet& outWorkSet) const;

    void BuildNode(const WorkSet& workSet);

    // input data
    BuildingParams mParams;
    const math::Box* mLeafBoxes;
    Uint32 mNumLeaves;

    // target BVH
    BVH& mTarget;

    // temporary data
    math::Box* mLeftBoxesCache;
    math::Box* mRightBoxesCache;
    Uint32* mSortedLeavesIndicesCache[NUM_AXES];
};


} // namespace rt
