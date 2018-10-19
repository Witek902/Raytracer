#include "PCH.h"
#include "DebugRenderer.h"
#include "ShadingData.h"
#include "Context.h"
#include "Scene/Scene.h"
#include "Material/Material.h"
#include "Color/ColorHelpers.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;

// convert [-1..1] range to [0..1]
static RT_FORCE_INLINE const Vector4 ScaleBipolarRange(const Vector4 x)
{
    return Vector4::MulAndAdd(x, VECTOR_HALVES, VECTOR_HALVES);
}

DebugRenderer::DebugRenderer(const Scene& scene)
    : IRenderer(scene)
    , mRenderingMode(DebugRenderingMode::BaseColor)
{
}

const Color DebugRenderer::TraceRay_Single(const Ray& ray, RenderingContext& context) const
{
    HitPoint hitPoint;
    context.localCounters.Reset();
    mScene.Traverse_Single({ ray, hitPoint, context });
    context.counters.Append(context.localCounters);

    ShadingData shadingData;
    mScene.ExtractShadingData(ray.origin, ray.dir, hitPoint, context.time, shadingData);

    if (hitPoint.distance == FLT_MAX)
    {
        // ray hit background
        return Color();
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
            resultColor = ScaleBipolarRange(shadingData.position);
            break;
        }
        case DebugRenderingMode::TexCoords:
        {
            resultColor = Vector4(fmodf(shadingData.texCoord[0], 1.0f), fmodf(shadingData.texCoord[1], 1.0f), 0.0f, 0.0f);
            break;
        }
        case DebugRenderingMode::TriangleID:
        {
            const Uint32 hash = Hash(hitPoint.objectId + hitPoint.triangleId);
            const float hue = (float)hash / (float)UINT32_MAX;
            resultColor = HSVtoRGB(hue, 0.95f, 1.0f);
            break;
        }

        // Material
        case DebugRenderingMode::BaseColor:
        {
            if (shadingData.material)
            {
                resultColor = shadingData.material->GetBaseColor(shadingData.texCoord);
            }
            break;
        }
        case DebugRenderingMode::Emission:
        {
            if (shadingData.material)
            {
                resultColor = shadingData.material->GetEmissionColor(shadingData.texCoord);
            }
            break;
        }
        case DebugRenderingMode::Roughness:
        {
            if (shadingData.material)
            {
                resultColor = Vector4(shadingData.material->GetRoughness(shadingData.texCoord));
            }
            break;
        }
        case DebugRenderingMode::Metalness:
        {
            if (shadingData.material)
            {
                resultColor = Vector4(shadingData.material->GetMetalness(shadingData.texCoord));
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
    }

    return Color::SampleRGB(context.wavelength, resultColor);
}

} // namespace rt
