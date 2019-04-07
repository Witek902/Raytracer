#include "PCH.h"
#include "../Core/Utils/HashGrid.h"

#include "gtest/gtest.h"

using namespace rt;
using namespace rt::math;


TEST(MathTest, HashGrid_RandomPoints)
{
    const Uint32 numPoints = 50000;
    const Uint32 numQueries = 10000;
    const float particleRadius = 1.0f;
    const float boxSize = 100.0f;

    Random random;

    struct Particle
    {
        Vector4 pos;
        RT_FORCE_INLINE const Vector4& GetPosition() const { return pos; }
    };

    DynArray<Particle> particles;
    for (Uint32 i = 0; i < numPoints; ++i)
    {
        particles.PushBack({ random.GetVector4() * boxSize });
    }

    HashGrid grid;
    grid.Build(particles, particleRadius);
    const Box& box = grid.GetBox();

    std::vector<Uint32> referenceIndices;

    struct Query
    {
        void operator()(Uint32 index)
        {
            collectedIndices.push_back(index);
        }

        std::vector<Uint32> collectedIndices;
    };

    Query query;

    const float queryMarigin = 1.0f;
    for (Uint32 i = 0; i < numQueries; ++i)
    {
        const Vector4 queryPoint = random.GetVector4() * (box.max - box.min) + box.min;
        SCOPED_TRACE("Query point: [" + std::to_string(queryPoint.x) + ',' + std::to_string(queryPoint.y) + ',' + std::to_string(queryPoint.z) + "]");

        // collect using hash grid
        query.collectedIndices.clear();
        grid.Process(queryPoint, particles, query);
        std::sort(query.collectedIndices.begin(), query.collectedIndices.end());

        // collect via brute force check
        referenceIndices.clear();
        for (Uint32 j = 0; j < numPoints; ++j)
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
