#pragma once

#include "Logger.h"
#include "Profiler.h"
#include "../Math/Box.h"
#include "../Math/Random.h"
#include "../Containers/DynArray.h"

namespace rt {

class HashGrid
{
public:
    RT_FORCE_INLINE const math::Box& GetBox() const { return mBox; }

    template<typename ParticleType>
    RT_FORCE_NOINLINE void Build(const DynArray<ParticleType>& particles, float radius)
    {
        RT_SCOPED_TIMER(HashGrid_Build);

        mRadiusSqr = math::Sqr(radius);
        mCellSize = radius * 2.0f;
        mInvCellSize = 1.0f / mCellSize;

        // compute overall bounding box
        mBox = math::Box::Empty();
        for (Uint32 i = 0; i < particles.Size(); i++)
        {
            const math::Vector4& pos = particles[i].GetPosition();
            mBox.AddPoint(pos);
        }

        Uint32 maxPerticlesPerCell = 0;

        // determine number of particles in each hash table entry
        {
            // TODO tweak this
            Uint32 hashTableSize = math::NextPowerOfTwo(particles.Size());
            mHashTableMask = hashTableSize - 1;
            mCellEnds.Resize(hashTableSize);

            memset(mCellEnds.Data(), 0, mCellEnds.Size() * sizeof(Uint32));

            // set mCellEnds[x] to number of particles within x
            for (Uint32 i = 0; i < particles.Size(); i++)
            {
                const math::Vector4& pos = particles[i].GetPosition();
                mCellEnds[GetCellIndex(pos)]++;
            }

            // run exclusive prefix sum to really get the cell starts
            // mCellEnds[x] is now where the cell starts
            Uint32 sum = 0;
            for (Uint32 i = 0; i < mCellEnds.Size(); i++)
            {
                Uint32 temp = mCellEnds[i];
                maxPerticlesPerCell = math::Max(maxPerticlesPerCell, temp);
                mCellEnds[i] = sum;
                sum += temp;
            }
        }

        // fill up particle indices
        mIndices.Resize(particles.Size());
        for (Uint32 i = 0; i < particles.Size(); i++)
        {
            const math::Vector4& pos = particles[i].GetPosition();
            const int targetIdx = mCellEnds[GetCellIndex(pos)]++;
            mIndices[targetIdx] = Uint32(i);
        }
    }

    template<typename ParticleType, typename Query>
    RT_FORCE_NOINLINE void Process(const math::Vector4& queryPos, const DynArray<ParticleType>& particles, Query& query) const
    {
        if (mIndices.Empty())
        {
            return;
        }

        const math::Vector4 distMin = queryPos - mBox.min;
        const math::Vector4 cellCoords = math::Vector4::MulAndSub(distMin, mInvCellSize, math::Vector4(0.5f));
        const math::VectorInt4 coordI = math::VectorInt4::TruncateAndConvert(cellCoords);

        Uint32 numVisitedCells = 0;
        Uint32 visitedCells[8];

        // find neigboring (potential) cells - 2x2x2 block
        for (Uint32 i = 0; i < 8; ++i)
        {
            //const Uint32 cellIndex = GetCellIndex(coordI + offsets[i]);

            const Uint32 x = coordI.x + ( i       & 1);
            const Uint32 y = coordI.y + ((i >> 1) & 1);
            const Uint32 z = coordI.z + ((i >> 2)    );
            const Uint32 cellIndex = GetCellIndex(x, y, z);

            // check if the cell is not already marked to visit
            bool visited = false;
            for (Uint32 j = 0; j < numVisitedCells; ++j)
            {
                if (visitedCells[j] == cellIndex)
                {
                    visited = true;
                    break;
                }
            }

            if (!visited)
            {
                visitedCells[numVisitedCells++] = cellIndex;

                // prefetch cell range to avoid cache miss in GetCellRange
                RT_PREFETCH_L1(mCellEnds.Data() + cellIndex);
            }
        }

        // collect particles from potential cells
        for (Uint32 i = 0; i < numVisitedCells; ++i)
        {
            const Uint32 cellIndex = visitedCells[i];

            Uint32 rangeStart, rangeEnd;
            GetCellRange(cellIndex, rangeStart, rangeEnd);

            // prefetch all the particles up front
            for (Uint32 j = rangeStart; j < rangeEnd; ++j)
            {
                RT_PREFETCH_L1(&particles[mIndices[j]]);
            }

            for (Uint32 j = rangeStart; j < rangeEnd; ++j)
            {
                const Uint32 particleIndex = mIndices[j];
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

    RT_FORCE_INLINE void GetCellRange(Uint32 cellIndex, Uint32& outStart, Uint32& outEnd) const
    { 
        outStart = cellIndex == 0 ? 0 : mCellEnds[cellIndex - 1];
        outEnd = mCellEnds[cellIndex];
    }

    RT_FORCE_INLINE Uint32 GetCellIndex(Uint32 x, Uint32 y, Uint32 z) const
    {
        // "Optimized Spatial Hashing for Collision Detection of Deformable Objects", Matthias Teschner, 2003
        return ((x * 73856093u) ^ (y * 19349663u) ^ (z * 83492791u)) & mHashTableMask;
    }

    RT_FORCE_INLINE Uint32 GetCellIndex(const math::VectorInt4& p) const
    {
        // "Optimized Spatial Hashing for Collision Detection of Deformable Objects", Matthias Teschner, 2003
        return ((p.x * 73856093u) ^ (p.y * 19349663u) ^ (p.z * 83492791u)) & mHashTableMask;
    }

    Uint32 GetCellIndex(const math::Vector4& p) const
    {
        const math::Vector4 distMin = p - mBox.min;
        const math::Vector4 coordF = mInvCellSize * distMin;
        const math::VectorInt4 coordI = math::VectorInt4::TruncateAndConvert(coordF);

        return GetCellIndex(coordI);
    }

    math::Box mBox;
    DynArray<Uint32> mIndices;
    DynArray<Uint32> mCellEnds;

    float mRadiusSqr;
    float mCellSize;
    float mInvCellSize;

    Uint32 mHashTableMask;
};

} // namespace rt
