#include "PCH.h"
#include "../Core/Utils/KdTree.h"
#include "../Core/Math/Random.h"

using namespace rt;
using namespace rt::math;


TEST(UtilsTest, KdTree_RandomPoints)
{
    const Uint32 numPoints = 500000;
    const Uint32 numQueries = 1000;
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
    for (Uint32 i = 0; i < numPoints; ++i)
    {
        particles.PushBack({ random.GetVector4Bipolar() * boxSize });
    }

    KdTree kdTree;
    kdTree.Build(particles);

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

    Timer timer;
    double totalTime = 0.0;

    for (Uint32 i = 0; i < numQueries; ++i)
    {
        const Vector4 queryPoint = random.GetVector4Bipolar() * (boxSize + queryBoxMarigin);
        SCOPED_TRACE("Query point: [" + std::to_string(queryPoint.x) + ',' + std::to_string(queryPoint.y) + ',' + std::to_string(queryPoint.z) + "]");

        // collect using hash grid
        query.collectedIndices.clear();
        timer.Start();
        kdTree.Find(queryPoint, particleRadius, particles, query);
        totalTime += timer.Stop();
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

    RT_LOG_INFO("Avg. query time: %.3f us", totalTime * 1000000.0 / numQueries);
}
