#include "PCH.h"
#include "BVHBuilder.h"

#include <algorithm>


namespace rt {

BVHBuilder::BVHBuilder(BVH& targetBVH)
    : mTarget(targetBVH)
    , mLeafBoxes(nullptr)
    , mNumLeaves(0)
    , mLeftBoxesCache(0)
    , mRightBoxesCache(0)
{
}

BVHBuilder::~BVHBuilder()
{

}

bool BVHBuilder::Build(const math::Box* data, Uint32 numLeaves,
                       const BuildingParams& params,
                       std::vector<Uint32>& outLeavesOrder)
{
    mLeafBoxes = data;
    mNumLeaves = numLeaves;

    mLeftBoxesCache = (math::Box*)_aligned_malloc(sizeof(math::Box) * mNumLeaves, 64); 
    mRightBoxesCache = (math::Box*)_aligned_malloc(sizeof(math::Box) * mNumLeaves, 64);

    for (Uint32 i = 0; i < NUM_AXES; ++i)
    {
        mSortedLeavesIndicesCache[i] = (Uint32*)_aligned_malloc(sizeof(Uint32) * mNumLeaves, 64);
    }

    // TODO error checking

    mParams = params;


    _aligned_free(mLeftBoxesCache);
    _aligned_free(mRightBoxesCache);

    for (Uint32 i = 0; i < NUM_AXES; ++i)
    {
        _aligned_free(mSortedLeavesIndicesCache[i]);
    }

    return true;
}

void BVHBuilder::BuildNode(const WorkSet& workSet)
{
    Uint32 bestAxis = 0;
    Uint32 bestSplitPos = 0;
    Float bestCost = FLT_MAX;

    if (workSet.numLeaves <= mParams.maxLeafNodeSize)
    {
        // TODO generate leaf node
    }

    for (Uint32 axis = 0; axis < NUM_AXES; ++axis)
    {
        const Uint32* indices = workSet.sortedLeaves[axis];

        // calculate left child node AABB for each possible split position
        math::Box leftBox = mLeafBoxes[indices[0]];
        mLeftBoxesCache[0] = leftBox;
        for (Uint32 i = 1; i < workSet.numLeaves; ++i)
        {
            leftBox = math::Box(leftBox, mLeafBoxes[indices[i]]);
            mLeftBoxesCache[i] = leftBox;
        }

        // calculate right child node AABB for each possible split position
        math::Box rightBox = mLeafBoxes[indices[workSet.numLeaves - 1]];
        mRightBoxesCache[workSet.numLeaves - 1] = rightBox;
        for (Uint32 i = workSet.numLeaves - 2; i-- > 0;)
        {
            rightBox = math::Box(rightBox, mLeafBoxes[indices[i]]);
            mRightBoxesCache[i] = rightBox;
        }

        // find most optimal split position
        for (Uint32 splitPos = 1; splitPos < workSet.numLeaves; ++splitPos)
        {
            const Float leftArea = mLeftBoxesCache[splitPos - 1].SurfaceArea();
            const Float rightArea = mRightBoxesCache[splitPos].SurfaceArea();
            const Uint32 leftCount = splitPos + 1;
            const Uint32 rightCount = workSet.numLeaves - splitPos;

            const Float totalCost = leftArea * static_cast<Float>(leftCount) + rightArea * static_cast<Float>(rightCount);
            if (totalCost < bestCost)
            {
                bestCost = totalCost;
                bestAxis = axis;
                bestSplitPos = splitPos;
            }
        }
    }
}

void BVHBuilder::SortLeaves(const std::vector<Uint32>& input, WorkSet& outWorkSet) const
{
    // TODO use custom linear allocator
    for (Uint32 axis = 0; axis < NUM_AXES; ++axis)
    {
        // copy indices from source
        outWorkSet.sortedLeaves[axis] = input;
    }

    // sort indices in each axis
    for (Uint32 axis = 0; axis < NUM_AXES; ++axis)
    {
        const auto comparator = [this, axis](const Uint32 a, const Uint32 b)
        {
            const math::Box& leafA = mLeafBoxes[a];
            const math::Box& leafB = mLeafBoxes[b];

            // TODO use precalculated triangle center (experiment)
            const math::Vector4 centerA = leafA.max - leafA.min;
            const math::Vector4 centerB = leafB.max - leafB.min;
            return centerA[axis] < centerB[axis];
        };

        std::sort(outWorkSet.sortedLeaves[axis].begin(), outWorkSet.sortedLeaves[axis].end(), comparator);
    }
}

} // namespace rt
