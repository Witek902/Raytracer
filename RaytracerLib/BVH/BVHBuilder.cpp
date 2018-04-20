#include "PCH.h"
#include "BVHBuilder.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"

#include <algorithm>
#include <assert.h>

namespace rt {

using namespace math;

BVHBuilder::Context::Context(Uint32 numLeaves)
{
    mLeftBoxesCache.resize(numLeaves);
    mRightBoxesCache.resize(numLeaves);

    for (Uint32 i = 0; i < NumAxes; ++i)
    {
        mSortedLeavesIndicesCache[i].resize(numLeaves);
    }
}

//////////////////////////////////////////////////////////////////////////

BVHBuilder::BVHBuilder(BVH& targetBVH)
    : mTarget(targetBVH)
    , mLeafBoxes(nullptr)
    , mNumLeaves(0)
    , mNumGeneratedNodes(0)
{
}

BVHBuilder::~BVHBuilder()
{

}

bool BVHBuilder::Build(const Box* data, const Uint32 numLeaves,
                       const BuildingParams& params,
                       std::vector<Uint32>& outLeavesOrder)
{
    mLeafBoxes = data;
    mNumLeaves = numLeaves;
    mParams = params;
    mTarget.AllocateNodes(2 * mNumLeaves - 1); // TODO this is too big, reallocate at the end

    mNumGeneratedNodes = 0;
    mNumGeneratedLeaves = 0;
    mLeavesOrder.clear();
    mLeavesOrder.reserve(mNumLeaves);

    if (mNumLeaves == 0)
    {
        RT_LOG_INFO("Skipped empty BVH generation");
        return true;
    }

    // calculate overall bounding box
    Box overallBox = Box::Empty();
    for (Uint32 i = 0; i < mNumLeaves; ++i)
    {
        overallBox = Box(overallBox, mLeafBoxes[i]);
    }

    RT_LOG_INFO("BVH statistics: num leaves = %u, overall box = [%f, %f, %f], [%f, %f, %f]",
                mNumLeaves,
                overallBox.min.f[0], overallBox.min.f[1], overallBox.min.f[2],
                overallBox.max.f[0], overallBox.max.f[1], overallBox.max.f[2]);

    WorkSet rootWorkSet;
    rootWorkSet.box = overallBox;
    rootWorkSet.numLeaves = mNumLeaves;
    rootWorkSet.leafIndices.reserve(mNumLeaves);
    for (Uint32 i = 0; i < mNumLeaves; ++i)
    {
        rootWorkSet.leafIndices.push_back(i);
    }

    Timer timer;
    timer.Start();

    {
        Context context(mNumLeaves);

        BVH::Node& rootNode = mTarget.mNodes[mNumGeneratedNodes++];
        BuildNode(rootWorkSet, context, rootNode);
    }

    assert(mNumGeneratedLeaves == mNumLeaves); // Number of generated leaves is invalid
    assert(mNumGeneratedNodes < 2 * mNumLeaves); // Number of generated nodes is invalid

    // shrink BVH nodes array
    mTarget.mNumNodes = mNumGeneratedNodes;
    mTarget.mNodes.resize(mNumGeneratedNodes);
    mTarget.mNodes.shrink_to_fit();

    const Float millisecondsElapsed = (Float)(1000.0 * timer.Stop());
    RT_LOG_INFO("Finished BVH generation in %.9g ms (num nodes = %u)", millisecondsElapsed, mNumGeneratedNodes);

    outLeavesOrder = mLeavesOrder;
    return true;
}

void BVHBuilder::GenerateLeaf(const WorkSet& workSet, BVH::Node& targetNode)
{
    targetNode.data.numLeaves = workSet.numLeaves;
    targetNode.data.childIndex = mNumGeneratedLeaves;

    for (Uint32 i = 0; i < workSet.numLeaves; ++i)
    {
        mLeavesOrder.push_back(workSet.leafIndices[i]);
    }

    mNumGeneratedLeaves += workSet.numLeaves;
}

void BVHBuilder::BuildNode(const WorkSet& workSet, Context& context, BVH::Node& targetNode)
{
    assert(workSet.numLeaves <= mNumLeaves);
    assert(workSet.numLeaves > 0);
    assert(workSet.depth < mNumLeaves);
    assert(workSet.depth <= BVH::MaxDepth);

    targetNode.box = workSet.box;

    if (workSet.numLeaves <= mParams.maxLeafNodeSize)
    {
        GenerateLeaf(workSet, targetNode);
        return;
    }

    Uint32 bestAxis = 0;
    Uint32 bestSplitPos = 0;
    Float bestCost = FLT_MAX;
    Box bestLeftBox = Box::Empty();
    Box bestRightBox = Box::Empty();

    SortLeaves(workSet, context);

    for (Uint32 axis = 0; axis < NumAxes; ++axis)
    {
        const Indices& sortedIndices = context.mSortedLeavesIndicesCache[axis];

        // calculate left child node AABB for each possible split position
        {
            Box accumulatedBox = Box::Empty();
            for (Uint32 i = 0; i < workSet.numLeaves; ++i)
            {
                accumulatedBox = Box(accumulatedBox, mLeafBoxes[sortedIndices[i]]);
                context.mLeftBoxesCache[i] = accumulatedBox;
            }
        }

        // calculate right child node AABB for each possible split position
        {
            Box accumulatedBox = Box::Empty();
            for (Uint32 i = workSet.numLeaves; i-- > 0; )
            {
                accumulatedBox = Box(accumulatedBox, mLeafBoxes[sortedIndices[i]]);
                context.mRightBoxesCache[i] = accumulatedBox;
            }
        }

        // find optimal split position (surface area heuristics)
        for (Uint32 splitPos = 0; splitPos < workSet.numLeaves - 1; ++splitPos)
        {
            const Box& leftBox = context.mLeftBoxesCache[splitPos];
            const Box& rightBox = context.mRightBoxesCache[splitPos + 1];

            const Float leftArea = leftBox.SurfaceArea();
            const Float rightArea = rightBox.SurfaceArea();
            const Uint32 leftCount = splitPos + 1;
            const Uint32 rightCount = workSet.numLeaves - leftCount;

            const Float totalCost =
                leftArea * static_cast<Float>(leftCount) +
                rightArea * static_cast<Float>(rightCount);

            if (totalCost < bestCost)
            {
                bestCost = totalCost;
                bestAxis = axis;
                bestSplitPos = splitPos;
                bestLeftBox = leftBox;
                bestRightBox = rightBox;
            }
        }
    }

    const Uint32 leftCount = bestSplitPos + 1;
    const Uint32 rightCount = workSet.numLeaves - leftCount;

    const Uint32 leftNodeIndex = mNumGeneratedNodes;
    mNumGeneratedNodes += 2;

    targetNode.data.childIndex = leftNodeIndex;
    targetNode.data.numLeaves = 0;

    WorkSet childWorkSet;
    childWorkSet.sortedBy = bestAxis;
    childWorkSet.depth = workSet.depth + 1;

    const Indices& sortedIndices = context.mSortedLeavesIndicesCache[bestAxis];
    Indices leftIndices = Indices(sortedIndices.begin(), sortedIndices.begin() + leftCount);
    Indices rightIndices = Indices(sortedIndices.begin() + leftCount, sortedIndices.end());

    // generate left node
    {
        BVH::Node& childNode = mTarget.mNodes[leftNodeIndex];

        childWorkSet.box = bestLeftBox;
        childWorkSet.numLeaves = leftCount;
        childWorkSet.leafIndices = std::move(leftIndices);
        BuildNode(childWorkSet, context, childNode);
    }

    // generate right node
    {
        BVH::Node& childNode = mTarget.mNodes[leftNodeIndex + 1];

        childWorkSet.box = bestRightBox;
        childWorkSet.numLeaves = rightCount;
        childWorkSet.leafIndices = std::move(rightIndices);
        BuildNode(childWorkSet, context, childNode);
    }
}

void BVHBuilder::SortLeaves(const WorkSet& workSet, Context& context) const
{
    for (Uint32 axis = 0; axis < NumAxes; ++axis)
    {
        Indices& indicesToSort = context.mSortedLeavesIndicesCache[axis];

        if (workSet.sortedBy != axis) // sort only what needs to be sorted
        {
            indicesToSort = workSet.leafIndices;

            const auto comparator = [this, axis](const Uint32 a, const Uint32 b)
            {
                assert(a < mNumLeaves);
                assert(b < mNumLeaves);

                const Box& leafA = mLeafBoxes[a];
                const Box& leafB = mLeafBoxes[b];

                // TODO use precalculated triangle center (experiment)
                const Vector4 centerA = leafA.max + leafA.min;
                const Vector4 centerB = leafB.max + leafB.min;
                return centerA[axis] < centerB[axis];
            };

            std::sort(indicesToSort.begin(), indicesToSort.end(), comparator);
        }
    }

    if (workSet.sortedBy < NumAxes)
    {
        context.mSortedLeavesIndicesCache[workSet.sortedBy] = std::move(workSet.leafIndices);
    }
}

} // namespace rt
