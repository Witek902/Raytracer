#include "PCH.h"
#include "BVH.h"
#include "Logger.h"


namespace rt {

static const Uint32 BVH_FILE_VERSION = 0;

struct BVHFileHeader
{
    Uint32 version;
    Uint32 numNodes;
    // TODO checksum, mesh/scene name, etc.
};

BVH::BVH()
    : mNodes(nullptr)
    , mNumNodes(0)
{ }

BVH::~BVH()
{
    if (mNodes)
    {
        _aligned_free(mNodes);
        mNodes = nullptr;
    }

    mNumNodes = 0;
}

BVH::BVH(BVH&& rhs)
{
    mNumNodes = rhs.mNumNodes;
    mNodes = rhs.mNodes;
    rhs.mNodes = nullptr;
    rhs.mNumNodes = 0;
}

BVH& BVH::operator = (BVH&& rhs)
{
    AllocateNodes(0);
    mNumNodes = rhs.mNumNodes;
    mNodes = rhs.mNodes;
    rhs.mNodes = nullptr;
    rhs.mNumNodes = 0;
}

bool BVH::AllocateNodes(Uint32 numNodes)
{
    if (mNodes)
    {
        _aligned_free(mNodes);
        mNodes = nullptr;
    }

    if (numNodes > 0)
    {
        mNodes = (Node*)_aligned_malloc(numNodes * sizeof(Node), 64);
        if (!mNodes)
        {
            mNumNodes = 0;
            RT_LOG_ERROR("Failed to allocate BVH nodes (num = %u)", numNodes);
            return false;
        }
    }

    mNumNodes = numNodes;
    return true;
}

bool BVH::SaveToFile(const char* filePath) const
{
    FILE* file = fopen(filePath, "wb");
    if (!file)
    {
        RT_LOG_ERROR("Failed to open output BVH file '%s' for writing. Error code: %i", filePath, errno);
        return false;
    }

    BVHFileHeader header;
    header.version = BVH_FILE_VERSION;
    header.numNodes = mNumNodes;

    if (fwrite(&header, sizeof(BVHFileHeader), 1, file) != 1)
    {
        fclose(file);
        RT_LOG_ERROR("Failed to write BVH file header");
        return false;
    }

    if (fwrite(mNodes, sizeof(Node), mNumNodes, file) != mNumNodes)
    {
        fclose(file);
        RT_LOG_ERROR("Failed to write BVH nodes");
        return false;
    }

    fclose(file);
    return true;
}

bool BVH::LoadFromFile(const char* filePath)
{
    FILE* file = fopen(filePath, "rb");
    if (!file)
    {
        RT_LOG_ERROR("Failed to open BVH file '%s' for reading. Error code: %i", filePath, errno);
        return false;
    }

    BVHFileHeader header;
    if (fread(&header, sizeof(BVHFileHeader), 1, file) != 1)
    {
        fclose(file);
        RT_LOG_ERROR("Failed to read BVH file header");
        return false;
    }

    if (header.version != BVH_FILE_VERSION)
    {
        fclose(file);
        RT_LOG_ERROR("Unsupported BVH file version %u (expected %u)", header.version, BVH_FILE_VERSION);
        return false;
    }

    if (!AllocateNodes(header.numNodes))
    {
        fclose(file);
        return false;
    }

    if (fread(mNodes, sizeof(Node), header.numNodes, file) != header.numNodes)
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

    CalculateStatsForNode(0, outStats);
}

void BVH::CalculateStatsForNode(Uint32 node, Stats& outStats) const
{

}

} // namespace rt
