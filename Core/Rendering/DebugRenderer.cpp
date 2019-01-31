#include "PCH.h"
#include "DebugRenderer.h"
#include "ShadingData.h"
#include "Context.h"
#include "Scene/Scene.h"
#include "Material/Material.h"
#include "Color/ColorHelpers.h"
#include "Color/Spectrum.h"
#include "Traversal/TraversalContext.h"
#include "Traversal/RayPacket.h"
#include "Rendering/Viewport.h"

namespace rt {

using namespace math;

// convert [-1..1] range to [0..1]
static RT_FORCE_INLINE const Vector4 ScaleBipolarRange(const Vector4 x)
{
    return Vector4::Max(Vector4::Zero(), Vector4::MulAndAdd(x, VECTOR_HALVES, VECTOR_HALVES));
}

DebugRenderer::DebugRenderer(const Scene& scene)
    : IRenderer(scene)
    , mRenderingMode(DebugRenderingMode::BaseColor)
{
}

const RayColor DebugRenderer::TraceRay_Single(const Ray& ray, RenderingContext& context) const
{
    HitPoint hitPoint;
    context.localCounters.Reset();
    mScene.Traverse_Single({ ray, hitPoint, context });
    context.counters.Append(context.localCounters);

    if (hitPoint.distance == FLT_MAX)
    {
        // ray hit background
        return RayColor::Zero();
    }

    ShadingData shadingData;
    if (mRenderingMode != DebugRenderingMode::TriangleID && mRenderingMode != DebugRenderingMode::Depth)
    {
        mScene.ExtractShadingData(ray.origin, ray.dir, hitPoint, context.time, shadingData);
    }

    Vector4 resultColor;

    switch (mRenderingMode)
    {
        // Geometry
        case DebugRenderingMode::Depth:
        {
            const float logDepth = std::max<float>(0.0f, (log2f(hitPoint.distance) + 5.0f) / 10.0f);
            resultColor = Vector4(logDepth);
            break;
        }
        case DebugRenderingMode::TriangleID:
        {
            const Uint64 hash = Hash((Uint64)hitPoint.objectId | ((Uint64)hitPoint.subObjectId << 32));
            const float hue = (float)(Uint32)hash / (float)UINT32_MAX;
            const float saturation = 0.5f + 0.5f * (float)(Uint32)(hash >> 32) / (float)UINT32_MAX;
            resultColor = HSVtoRGB(hue, saturation, 1.0f);
            break;
        }

        case DebugRenderingMode::Normals:
        {
            resultColor = ScaleBipolarRange(shadingData.normal);
            break;
        }
        case DebugRenderingMode::Tangents:
        {
            resultColor = ScaleBipolarRange(shadingData.tangent);
            break;
        }
        case DebugRenderingMode::Bitangents:
        {
            resultColor = ScaleBipolarRange(shadingData.bitangent);
            break;
        }
        case DebugRenderingMode::Position:
        {
            resultColor = Vector4::Max(Vector4::Zero(), shadingData.position);
            break;
        }
        case DebugRenderingMode::TexCoords:
        {
            resultColor = Vector4(fmodf(shadingData.texCoord[0], 1.0f), fmodf(shadingData.texCoord[1], 1.0f), 0.0f, 0.0f);
            break;
        }

        // Material
        case DebugRenderingMode::BaseColor:
        {
            if (shadingData.material)
            {
                resultColor = shadingData.material->baseColor.Evaluate(shadingData.texCoord);
            }
            break;
        }
        case DebugRenderingMode::Emission:
        {
            if (shadingData.material)
            {
                resultColor = shadingData.material->emission.Evaluate(shadingData.texCoord);
            }
            break;
        }
        case DebugRenderingMode::Roughness:
        {
            if (shadingData.material)
            {
                resultColor = Vector4(shadingData.material->roughness.Evaluate(shadingData.texCoord));
            }
            break;
        }
        case DebugRenderingMode::Metalness:
        {
            if (shadingData.material)
            {
                resultColor = Vector4(shadingData.material->metalness.Evaluate(shadingData.texCoord));
            }
            break;
        }

        // Statistics
#ifdef RT_ENABLE_INTERSECTION_COUNTERS
        case DebugRenderingMode::RayBoxIntersection:
        {
            const float num = static_cast<float>(context.localCounters.numRayBoxTests);
            resultColor = Vector4(num * 0.01f, num * 0.004f, num * 0.001f, 0.0f);
            break;
        }
        case DebugRenderingMode::RayBoxIntersectionPassed:
        {
            const float num = static_cast<float>(context.localCounters.numPassedRayBoxTests);
            resultColor = Vector4(num * 0.01f, num * 0.005f, num * 0.001f, 0.0f);
            break;
        }
        case DebugRenderingMode::RayTriIntersection:
        {
            const float num = static_cast<float>(context.localCounters.numRayTriangleTests);
            resultColor = Vector4(num * 0.01f, num * 0.004f, num * 0.001f, 0.0f);
            break;
        }
        case DebugRenderingMode::RayTriIntersectionPassed:
        {
            const float num = static_cast<float>(context.localCounters.numPassedRayTriangleTests);
            resultColor = Vector4(num * 0.01f, num * 0.004f, num * 0.001f, 0.0f);
            break;
        }
#endif // RT_ENABLE_INTERSECTION_COUNTERS

        default:
            RT_FATAL("Invalid debug rendering mode");
    }

    return RayColor::Resolve(context.wavelength, Spectrum(resultColor));
}

void DebugRenderer::Raytrace_Packet(RayPacket& packet, RenderingContext& context, Viewport& viewport) const
{
    mScene.Traverse_Packet({ packet, context });

    ShadingData shadingData;

    const Uint32 numGroups = packet.GetNumGroups();
    for (Uint32 i = 0; i < numGroups; ++i)
    {
        Vector4 weights[RayPacket::RaysPerGroup];
        packet.rayWeights[i].Unpack(weights);

        Vector4 rayOrigins[RayPacket::RaysPerGroup];
        Vector4 rayDirs[RayPacket::RaysPerGroup];
        packet.groups[i].rays[0].origin.Unpack(rayOrigins);
        packet.groups[i].rays[0].dir.Unpack(rayDirs);

        for (Uint32 j = 0; j < RayPacket::RaysPerGroup; ++j)
        {
            const HitPoint& hitPoint = context.hitPoints[RayPacket::RaysPerGroup * i + j];

            Vector4 color = Vector4::Zero();

            if (hitPoint.distance != FLT_MAX)
            {
                if (mRenderingMode != DebugRenderingMode::TriangleID && mRenderingMode != DebugRenderingMode::Depth)
                {
                    mScene.ExtractShadingData(rayOrigins[j], rayDirs[j], hitPoint, context.time, shadingData);
                }

                switch (mRenderingMode)
                {
                    case DebugRenderingMode::Depth:
                    {
                        const float logDepth = std::max<float>(0.0f, (log2f(hitPoint.distance) + 5.0f) / 10.0f);
                        color = Vector4(logDepth);
                        break;
                    }
                    case DebugRenderingMode::Normals:
                    {
                        color = ScaleBipolarRange(shadingData.normal);
                        break;
                    }
                    case DebugRenderingMode::Position:
                    {
                        color = ScaleBipolarRange(shadingData.position);
                        break;
                    }
                    case DebugRenderingMode::TriangleID:
                    {
                        const Uint64 hash = Hash((Uint64)hitPoint.objectId | ((Uint64)hitPoint.subObjectId << 32));
                        const float hue = (float)(Uint32)hash / (float)UINT32_MAX;
                        const float saturation = 0.5f + 0.5f * (float)(Uint32)(hash >> 32) / (float)UINT32_MAX;
                        color = weights[j] * HSVtoRGB(hue, saturation, 1.0f);
                        break;
                    }
                }
            }

            const ImageLocationInfo& imageLocation = packet.imageLocations[RayPacket::RaysPerGroup * i + j];
            viewport.Internal_AccumulateColor(imageLocation.x, imageLocation.y, color);
        }
    }
}

} // namespace rt
