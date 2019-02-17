#pragma once

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
        mRadius = radius;
        mRadiusSqr = Sqr(radius);
        mCellSize = radius * 2.0f;
        mInvCellSize = 1.0f / mCellSize;

        // compute overall bounding box
        mBox = math::Box::Empty();
        for (size_t i = 0; i < particles.size(); i++)
        {
            const math::Vector4& pos = particles[i].GetPosition();
            mBox.AddPoint(pos);
        }

        // determine number of particles in each hash table entry
        {
            // TODO may be too small
            mCellEnds.resize(particles.size());

            memset(mCellEnds.data(), 0, mCellEnds.size() * sizeof(int));

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
                int temp = mCellEnds[i];
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
            mIndices[targetIdx] = int(i);
        }
    }

    template<typename ParticleType, typename Query>
    RT_FORCE_NOINLINE void Process(const math::Vector4& queryPos, const std::vector<ParticleType>& particles, Query& query) const
    {
        const math::Vector4 distMin = queryPos - mBox.min;
        const math::Vector4 distMax = mBox.max - queryPos;
        for (int i = 0; i < 3; i++)
        {
            if (distMin[i] < 0.0f || distMax[i] < 0.0f)
            {
                return;
            }
        }

        const math::Vector4 cellPt = mInvCellSize * distMin;
        const math::Vector4 coordF(std::floor(cellPt.x), std::floor(cellPt.y), std::floor(cellPt.z));

        // TODO SSE
        const int px = int(coordF.x);
        const int py = int(coordF.y);
        const int pz = int(coordF.z);

        const math::Vector4 fractCoord = cellPt - coordF;

        // TODO SSE
        const int pxo = px + (fractCoord.x < 0.5f ? -1 : 1);
        const int pyo = py + (fractCoord.y < 0.5f ? -1 : 1);
        const int pzo = pz + (fractCoord.z < 0.5f ? -1 : 1);

        Uint32 numCellsToVisit = 0;
        Uint32 cellsToVisit[8];

        const auto checkCellToVisit = [&](Uint32 cellIndex)
        {
            // check if the cell is not already marked to visit
            for (Uint32 i = 0; i < numCellsToVisit; ++i)
            {
                if (cellsToVisit[i] == cellIndex)
                {
                    return;
                }
            }

            cellsToVisit[numCellsToVisit++] = cellIndex;
        };

        checkCellToVisit(GetCellIndex(px, py, pz));
        checkCellToVisit(GetCellIndex(px, py, pzo));
        checkCellToVisit(GetCellIndex(px, pyo, pz));
        checkCellToVisit(GetCellIndex(px, pyo, pzo));
        checkCellToVisit(GetCellIndex(pxo, py, pz));
        checkCellToVisit(GetCellIndex(pxo, py, pzo));
        checkCellToVisit(GetCellIndex(pxo, pyo, pz));
        checkCellToVisit(GetCellIndex(pxo, pyo, pzo));


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

    Uint32 GetCellIndex(Uint32 x, Uint32 y, Uint32 z) const
    {
        // TODO get rid of division
        return ((x * 73856093u) ^ (y * 19349663u) ^ (z * 83492791u)) % Uint32(mCellEnds.size());
    }


    Uint32 GetCellIndex(const math::Vector4& p) const
    {
        const math::Vector4 distMin = p - mBox.min;
        const math::Vector4 coordF = mInvCellSize * distMin;

        return GetCellIndex(Uint32(std::floor(coordF.x)), Uint32(std::floor(coordF.y)), Uint32(std::floor(coordF.z)));
    }

    math::Box mBox;
    std::vector<int> mIndices;
    std::vector<int> mCellEnds;

    float mRadius;
    float mRadiusSqr;
    float mCellSize;
    float mInvCellSize;
};

} // namespace rt
