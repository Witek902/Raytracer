#include "PCH.h"
#include "DebugRenderer.h"
#include "Scene/Scene.h"
#include "Material/Material.h"
#include "Color/ColorHelpers.h"
#include "Color/Spectrum.h"
#include "Traversal/TraversalContext.h"
#include "Rendering/Film.h"
#include "Rendering/Context.h"

namespace rt {

using namespace math;

DebugRenderer::DebugRenderer(const Scene& scene)
    : IRenderer(scene)
    , mRenderingMode(DebugRenderingMode::BaseColor)
{
}

const char* DebugRenderer::GetName() const
{
    return "Debug";
}

const RayColor DebugRenderer::RenderPixel(const math::Ray& ray, const RenderParam&, RenderingContext& ctx) const
{
    HitPoint hitPoint;
    ctx.localCounters.Reset();
    mScene.Traverse_Single({ ray, hitPoint, ctx });
    ctx.counters.Append(ctx.localCounters);

    if (hitPoint.distance == FLT_MAX)
    {
        // ray hit background
        return RayColor::Zero();
    }

    if (hitPoint.subObjectId == RT_LIGHT_OBJECT)
    {
        // ray hit a light
        const Vector4 lightColor{ 1.0, 1.0f, 0.0f };
        return RayColor::Resolve(ctx.wavelength, Spectrum(lightColor));
    }

    ShadingData shadingData;
    if (mRenderingMode != DebugRenderingMode::TriangleID && mRenderingMode != DebugRenderingMode::Depth)
    {
        mScene.ExtractShadingData(ray, hitPoint, ctx.time, shadingData);
    }

    Vector4 resultColor;

    switch (mRenderingMode)
    {
        case DebugRenderingMode::CameraLight:
        {
            const float NdotL = Vector4::Dot3(ray.dir, shadingData.frame[2]);
            resultColor = shadingData.material->baseColor.Evaluate(shadingData.texCoord) * Abs(NdotL);
            break;
        }

        // Geometry
        case DebugRenderingMode::Depth:
        {
            const float invDepth = 1.0f - 1.0f / (1.0f + hitPoint.distance / 10.0f);
            resultColor = Vector4(invDepth);
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
        case DebugRenderingMode::Tangents:
        {
            resultColor = BipolarToUnipolar(shadingData.frame[0]);
            break;
        }
        case DebugRenderingMode::Bitangents:
        {
            resultColor = BipolarToUnipolar(shadingData.frame[1]);
            break;
        }
        case DebugRenderingMode::Normals:
        {
            resultColor = BipolarToUnipolar(shadingData.frame[2]);
            break;
        }
        case DebugRenderingMode::Position:
        {
            resultColor = Vector4::Max(Vector4::Zero(), shadingData.frame[3]);
            break;
        }
        case DebugRenderingMode::TexCoords:
        {
            resultColor = Vector4::Mod1(shadingData.texCoord & Vector4::MakeMask<1,1,0,0>());
            break;
        }

        // Material
        case DebugRenderingMode::BaseColor:
        {
            resultColor = shadingData.material->baseColor.Evaluate(shadingData.texCoord);
            break;
        }
        case DebugRenderingMode::Emission:
        {
            resultColor = shadingData.material->emission.Evaluate(shadingData.texCoord);
            break;
        }
        case DebugRenderingMode::Roughness:
        {
            resultColor = Vector4(shadingData.material->roughness.Evaluate(shadingData.texCoord));
            break;
        }
        case DebugRenderingMode::Metalness:
        {
            resultColor = Vector4(shadingData.material->metalness.Evaluate(shadingData.texCoord));
            break;
        }

        // Statistics
#ifdef RT_ENABLE_INTERSECTION_COUNTERS
        case DebugRenderingMode::RayBoxIntersection:
        {
            const float num = static_cast<float>(ctx.localCounters.numRayBoxTests);
            resultColor = Vector4(num * 0.01f, num * 0.004f, num * 0.001f, 0.0f);
            break;
        }
        case DebugRenderingMode::RayBoxIntersectionPassed:
        {
            const float num = static_cast<float>(ctx.localCounters.numPassedRayBoxTests);
            resultColor = Vector4(num * 0.01f, num * 0.005f, num * 0.001f, 0.0f);
            break;
        }
        case DebugRenderingMode::RayTriIntersection:
        {
            const float num = static_cast<float>(ctx.localCounters.numRayTriangleTests);
            resultColor = Vector4(num * 0.01f, num * 0.004f, num * 0.001f, 0.0f);
            break;
        }
        case DebugRenderingMode::RayTriIntersectionPassed:
        {
            const float num = static_cast<float>(ctx.localCounters.numPassedRayTriangleTests);
            resultColor = Vector4(num * 0.01f, num * 0.004f, num * 0.001f, 0.0f);
            break;
        }
#endif // RT_ENABLE_INTERSECTION_COUNTERS

        default:
        {
            RT_FATAL("Invalid debug rendering mode");
            resultColor = Vector4::Zero();
        }
    }

    // clamp color
    resultColor = Vector4::Max(Vector4::Zero(), resultColor);

    return RayColor::Resolve(ctx.wavelength, Spectrum(resultColor));
}

void DebugRenderer::Raytrace_Packet(RayPacket& packet, const Camera&, Film& film, RenderingContext& context) const
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
                    mScene.ExtractShadingData(Ray(rayOrigins[j], rayDirs[j]), hitPoint, context.time, shadingData);
                }

                switch (mRenderingMode)
                {
                    case DebugRenderingMode::CameraLight:
                    {
                        const float NdotL = Vector4::Dot3(rayDirs[j], shadingData.frame[2]);
                        color = shadingData.material->baseColor.Evaluate(shadingData.texCoord) * Abs(NdotL);
                        break;
                    }

                    case DebugRenderingMode::Depth:
                    {
                        const float logDepth = std::max<float>(0.0f, (log2f(hitPoint.distance) + 5.0f) / 10.0f);
                        color = Vector4(logDepth);
                        break;
                    }
                    case DebugRenderingMode::Tangents:
                    {
                        color = BipolarToUnipolar(shadingData.frame[0]);
                        break;
                    }
                    case DebugRenderingMode::Bitangents:
                    {
                        color = BipolarToUnipolar(shadingData.frame[1]);
                        break;
                    }
                    case DebugRenderingMode::Normals:
                    {
                        color = BipolarToUnipolar(shadingData.frame[2]);
                        break;
                    }
                    case DebugRenderingMode::Position:
                    {
                        color = BipolarToUnipolar(shadingData.frame.GetTranslation());
                        break;
                    }
                    case DebugRenderingMode::TexCoords:
                    {
                        color = BipolarToUnipolar(shadingData.texCoord);
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

            // clamp color
            color = Vector4::Max(Vector4::Zero(), color);

            const ImageLocationInfo& imageLocation = packet.imageLocations[RayPacket::RaysPerGroup * i + j];
            film.AccumulateColor(imageLocation.x, imageLocation.y, color);
        }
    }
}

} // namespace rt
