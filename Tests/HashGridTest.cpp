#include "PCH.h"
#include "../Core/Utils/HashGrid.h"

using namespace rt;
using namespace rt::math;


TEST(UtilsTest, HashGrid_RandomPoints)
{
    const uint32 numPoints = 50000;
    const uint32 numQueries = 10000;
    const float particleRadius = 1.0f;
    const float boxSize = 100.0f;
    const float queryBoxMarigin = 2.0f;

    Random random;

    struct Particle
    {
        Vector4 pos;
        RT_FORCE_INLINE const Vector4& GetPosition() const { return pos; }
    };

    DynArray<Particle> particles;
    for (uint32 i = 0; i < numPoints; ++i)
    {
        particles.PushBack({ random.GetVector4Bipolar() * boxSize });
    }

    HashGrid grid;
    grid.Build(particles, particleRadius);

    std::vector<uint32> referenceIndices;

    struct Query
    {
        void operator()(uint32 index)
        {
            collectedIndices.push_back(index);
        }

        std::vector<uint32> collectedIndices;
    };

    Query query;

    for (uint32 i = 0; i < numQueries; ++i)
    {
        const Vector4 queryPoint = random.GetVector4Bipolar() * (boxSize + queryBoxMarigin);
        SCOPED_TRACE("Query point: [" + std::to_string(queryPoint.x) + ',' + std::to_string(queryPoint.y) + ',' + std::to_string(queryPoint.z) + "]");

        // collect using hash grid
        query.collectedIndices.clear();
        grid.Process(queryPoint, particles, query);
        std::sort(query.collectedIndices.begin(), query.collectedIndices.end());

        // collect via brute force check
        referenceIndices.clear();
        for (uint32 j = 0; j < numPoints; ++j)
        {
            if ((queryPoint - particles[j].pos).SqrLength3() <= particleRadius * particleRadius)
            {
                referenceIndices.push_back(j);
            }
        }

        ASSERT_EQ(referenceIndices.size(), query.collectedIndices.size());

        for (size_t j = 0; j < referenceIndices.size(); ++j)
        {
            ASSERT_EQ(referenceIndices[j], query.collectedIndices[j]) << j;
        }
    }
}
