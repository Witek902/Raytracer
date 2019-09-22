#include "PCH.h"
#include "BVH.h"
#include "Utils/Logger.h"


namespace rt {

static const uint32 BvhFileVersion = 0;
static const uint32 BvhMagic = 'bvhc';

struct BVHFileHeader
{
    uint32 magic;
    uint32 version;
    uint32 numNodes;
    // TODO checksum, mesh/scene name, etc.
};

static_assert(sizeof(BVH::Node) == 32, "Invalid node size");

BVH::BVH()
    : mNumNodes(0)
{ }

bool BVH::AllocateNodes(uint32 numNodes)
{
    mNodes.Resize(numNodes);
    mNumNodes = numNodes;
    return true;
}

bool BVH::SaveToFile(const std::string& filePath) const
{
    FILE* file = fopen(filePath.c_str(), "wb");
    if (!file)
    {
        RT_LOG_ERROR("Failed to open output BVH file '%s' for writing. Error code: %i", filePath.c_str(), errno);
        return false;
    }

    BVHFileHeader header;
    header.magic = BvhMagic;
    header.version = BvhFileVersion;
    header.numNodes = mNumNodes;

    if (fwrite(&header, sizeof(BVHFileHeader), 1, file) != 1)
    {
        fclose(file);
        RT_LOG_ERROR("Failed to write BVH file header");
        return false;
    }

    if (fwrite(mNodes.Data(), sizeof(Node), mNumNodes, file) != mNumNodes)
    {
        fclose(file);
        RT_LOG_ERROR("Failed to write BVH nodes");
        return false;
    }

    fclose(file);
    return true;
}

bool BVH::LoadFromFile(const std::string& filePath)
{
    FILE* file = fopen(filePath.c_str(), "rb");
    if (!file)
    {
        RT_LOG_ERROR("Failed to open BVH file '%s' for reading. Error code: %i", filePath.c_str(), errno);
        return false;
    }

    BVHFileHeader header;
    if (fread(&header, sizeof(BVHFileHeader), 1, file) != 1)
    {
        fclose(file);
        RT_LOG_ERROR("Failed to read BVH file header");
        return false;
    }

    if (header.magic != BvhMagic)
    {
        fclose(file);
        RT_LOG_ERROR("Corrupted BVH file (invalid magic value)");
        return false;
    }

    if (header.version != BvhFileVersion)
    {
        fclose(file);
        RT_LOG_ERROR("Unsupported BVH file version %u (expected %u)", header.version, BvhFileVersion);
        return false;
    }

    if (!AllocateNodes(header.numNodes))
    {
        fclose(file);
        return false;
    }

    if (fread(mNodes.Data(), sizeof(Node), header.numNodes, file) != header.numNodes)
    {
        fclose(file);
        RT_LOG_ERROR("Failed to read BVH nodes");
        return false;
    }

    fclose(file);
    return true;
}

void BVH::CalculateStats(Stats& outStats) const
{
    if (mNumNodes == 0)
    {
        outStats = Stats();
        return;
    }

    CalculateStatsForNode(0, outStats, 1);
}

void BVH::CalculateStatsForNode(uint32 nodeIndex, Stats& outStats, uint32 depth) const
{
    const Node& node = mNodes[nodeIndex];
    const math::Box box = node.GetBox();

    outStats.totalNodesArea += box.SurfaceArea();
    outStats.totalNodesVolume += box.Volume();
    outStats.maxDepth = std::max(outStats.maxDepth, depth);

    if (node.numLeaves + 1u > outStats.leavesCountHistogram.Size())
    {
        outStats.leavesCountHistogram.Resize(node.numLeaves + 1);
    }
    outStats.leavesCountHistogram[node.numLeaves]++;

    if (node.numLeaves == 0u)
    {
        CalculateStatsForNode(node.childIndex, outStats, depth + 1);
        CalculateStatsForNode(node.childIndex + 1, outStats, depth + 1);
    }
}

} // namespace rt
