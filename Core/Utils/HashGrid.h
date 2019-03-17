#pragma once

#include "Timer.h"
#include "Logger.h"
#include "../Math/Box.h"
#include "../Math/Random.h"

#include <vector>

namespace rt {

class HashGrid
{
public:
    RT_FORCE_INLINE const math::Box& GetBox() const { return mBox; }

    template<typename ParticleType>
    RT_FORCE_NOINLINE void Build(const std::vector<ParticleType>& particles, float radius)
    {
        //Timer timer;

        mRadius = radius;
        mRadiusSqr = math::Sqr(radius);
        mCellSize = radius * 2.0f;
        mInvCellSize = 1.0f / mCellSize;

        // compute overall bounding box
        mBox = math::Box::Empty();
        for (size_t i = 0; i < particles.size(); i++)
        {
            const math::Vector4& pos = particles[i].GetPosition();
            mBox.AddPoint(pos);
        }

        Uint32 maxPerticlesPerCell = 0;

        // determine number of particles in each hash table entry
        {
            // TODO tweak this
            Uint32 hashTableSize = math::NextPowerOfTwo(Uint32(particles.size()));
            mHashTableMask = hashTableSize - 1;
            mCellEnds.resize(hashTableSize);

            memset(mCellEnds.data(), 0, mCellEnds.size() * sizeof(Uint32));

            // set mCellEnds[x] to number of particles within x
            for (size_t i = 0; i < particles.size(); i++)
            {
                const math::Vector4& pos = particles[i].GetPosition();
                mCellEnds[GetCellIndex(pos)]++;
            }

            // run exclusive prefix sum to really get the cell starts
            // mCellEnds[x] is now where the cell starts
            int sum = 0;
            for (size_t i = 0; i < mCellEnds.size(); i++)
            {
                Uint32 temp = mCellEnds[i];
                maxPerticlesPerCell = math::Max(maxPerticlesPerCell, temp);
                mCellEnds[i] = sum;
                sum += temp;
            }
        }

        // fill up particle indices
        mIndices.resize(particles.size());
        for (size_t i = 0; i < particles.size(); i++)
        {
            const math::Vector4& pos = particles[i].GetPosition();
            const int targetIdx = mCellEnds[GetCellIndex(pos)]++;
            mIndices[targetIdx] = Uint32(i);
        }

        //RT_LOG_INFO("Building hash grid took %.2f ms, %zu particels, max perticles per cell = %u, min=[%f,%f,%f], max=[%f,%f,%f]",
        //    timer.Stop() * 1000.0, particles.size(), maxPerticlesPerCell,
        //    mBox.min.x, mBox.min.y, mBox.min.z, mBox.max.x, mBox.max.y, mBox.max.z);
    }

    template<typename ParticleType, typename Query>
    RT_FORCE_NOINLINE void Process(const math::Vector4& queryPos, const std::vector<ParticleType>& particles, Query& query) const
    {
        if (mIndices.empty())
        {
            return;
        }

        const math::Vector4 distMin = queryPos - mBox.min;
        // TODO check if point is inside box?

        const math::Vector4 cellCoords = mInvCellSize * distMin;
        // Note: -0.5 is to reduce cells search from 3x3x3 to 2x2x2 (emulates spheres shift in the cells)
        const math::Vector4 coordF = math::Vector4::Floor(cellCoords - math::Vector4(0.5f));
        const math::VectorInt4 coordI = math::VectorInt4::Convert(coordF);

        const math::VectorInt4 offsets[] = 
        {
            math::VectorInt4(0, 0, 0, 0),
            math::VectorInt4(0, 0, 1, 0),
            math::VectorInt4(0, 1, 0, 0),
            math::VectorInt4(0, 1, 1, 0),
            math::VectorInt4(1, 0, 0, 0),
            math::VectorInt4(1, 0, 1, 0),
            math::VectorInt4(1, 1, 0, 0),
            math::VectorInt4(1, 1, 1, 0),
        };

        Uint32 numCellsToVisit = 0;
        Uint32 cellsToVisit[8];

        // find neigboring (potential) cells - 2x2x2 block
        for (Uint32 i = 0; i < 8; ++i)
        {
            const Uint32 cellIndex = GetCellIndex(coordI + offsets[i]);

            // check if the cell is not already marked to visit
            for (Uint32 j = 0; j < numCellsToVisit; ++j)
            {
                if (cellsToVisit[j] == cellIndex)
                {
                    continue;
                }
            }

            cellsToVisit[numCellsToVisit++] = cellIndex;
        }

        // collect particles from potential cells
        for (Uint32 j = 0; j < numCellsToVisit; j++)
        {
            Uint32 rangeStart, rangeEnd;
            GetCellRange(cellsToVisit[j], rangeStart, rangeEnd);

            for (Uint32 i = rangeStart; i < rangeEnd; ++i)
            {
                const Uint32 particleIndex = mIndices[i];
                const ParticleType& particle = particles[particleIndex];

                const float distSqr = (queryPos - particle.GetPosition()).SqrLength3();
                if (distSqr <= mRadiusSqr)
                {
                    query(particleIndex);
                }
            }
        }
    }

private:

    void GetCellRange(Uint32 cellIndex, Uint32& outStart, Uint32& outEnd) const
    {
        if (cellIndex == 0)
        {
            outStart = 0;
            outEnd = mCellEnds[0];
        }
        else
        {
            outStart = mCellEnds[cellIndex - 1];
            outEnd = mCellEnds[cellIndex];
        }
    }

    Uint32 GetCellIndex(const math::VectorInt4& p) const
    {
        // "Optimized Spatial Hashing for Collision Detection of Deformable Objects", Matthias Teschner, 2003
        return ((p.x * 73856093u) ^ (p.y * 19349663u) ^ (p.z * 83492791u)) & mHashTableMask;
    }


    Uint32 GetCellIndex(const math::Vector4& p) const
    {
        const math::Vector4 distMin = p - mBox.min;
        const math::Vector4 coordF = mInvCellSize * distMin;
        const math::VectorInt4 coordI = math::VectorInt4::Convert(math::Vector4::Floor(coordF));

        return GetCellIndex(coordI);
    }

    math::Box mBox;
    std::vector<Uint32> mIndices;
    std::vector<Uint32> mCellEnds;

    float mRadius;
    float mRadiusSqr;
    float mCellSize;
    float mInvCellSize;

    Uint32 mHashTableMask;
};

} // namespace rt
