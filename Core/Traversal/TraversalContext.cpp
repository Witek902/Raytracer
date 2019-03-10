#include "PCH.h"
#include "TraversalContext.h"
#include "Rendering/Context.h"

namespace rt {

using namespace math;

void PacketTraversalContext::StoreIntersection(RayGroup& rayGroup, const Vector8& t, const Vector8& u, const Vector8& v, const VectorBool8& mask, Uint32 objectID, Uint32 subObjectID) const
{
    const int intMask = mask.GetMask();

    const Uint64 combinedObjectId = (Uint64)objectID | ((Uint64)subObjectID << 32u);

    HitPoint* hitPoints = context.hitPoints;

    if (intMask)
    {
        rayGroup.maxDistances = Vector8::Select(rayGroup.maxDistances, t, mask);

        for (Uint32 k = 0; k < 8; ++k)
        {
            if ((intMask >> k) & 1)
            {
                HitPoint& hitPointRef = hitPoints[rayGroup.rayOffsets[k]];

                hitPointRef.distance = t[k];
                hitPointRef.u = u[k];
                hitPointRef.v = v[k];
                hitPointRef.combinedObjectId = combinedObjectId;
            }
        }
    }
}

} // namespace rt
