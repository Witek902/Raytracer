#pragma once

#include "Renderer.h"
#include "../Material/BSDF/BSDF.h"

namespace rt {

struct ShadingData;
class ILight;

// Unidirectional path tracer
class BidirectionalPathTracer : public IRenderer
{
public:
    BidirectionalPathTracer(const Scene& scene);
    ~BidirectionalPathTracer();

    virtual const char* GetName() const override;
    virtual const RayColor TraceRay_Single(const math::Ray& ray, const Camera& camera, Film& film, RenderingContext& ctx) const override;

    // for debugging
    math::Vector4 mBSDFSamplingWeight;
    math::Vector4 mLightSamplingWeight;
    math::Vector4 mVertexConnectingWeight;
    math::Vector4 mCameraConnectingWeight;

private:

    static constexpr Uint32 MaxLightPathLength = 16;

    // describes current state of path coming from camera or a light
    struct PathState
    {
        math::Ray ray;
        RayColor throughput = RayColor::One();

        // quantities for MIS weight calculation
        Float dVC = 0.0f;
        Float dVM = 0.0f;
        Float dVCM = 0.0f;

        Uint32 length = 1u;
        BSDF::EventType lastSampledBsdfEvent = BSDF::NullEvent;
        bool lastSpecular = true;
        bool isFiniteLight;
    };

    struct LightVertex
    {
        ShadingData shadingData;    // TODO pack it, it's way too big
        RayColor throughput;        // TODO should be Float3

        // quantities for MIS weight calculation
        Float dVC;
        Float dVM;
        Float dVCM;

        Uint32 pathLength;
    };

    static_assert(sizeof(LightVertex) <= 192, "Don't make this struct even bigger");

    struct LightPath
    {
        Uint32 length = 0;
        LightVertex vertices[MaxLightPathLength];
    };

    // importance sample light sources
    const RayColor SampleLights(const ShadingData& shadingData, const PathState& pathState, RenderingContext& ctx) const;

    // importance sample single light source
    const RayColor SampleLight(const ILight& light, const ShadingData& shadingData, const PathState& pathState, RenderingContext& ctx) const;

    // compute radiance from a hit local lights
    const RayColor EvaluateLight(const ILight& light, Float dist, const PathState& pathState, RenderingContext& ctx) const;

    // compute radiance from global lights
    const RayColor EvaluateGlobalLights(const PathState& pathState, RenderingContext& ctx) const;

    // generate initial camera ray
    bool GenerateCameraPath(PathState& path, RenderingContext& ctx) const;

    // generate initial light ray
    bool GenerateLightSample(PathState& pathState, RenderingContext& ctx) const;
    void TraceLightPath(const Camera& camera, Film& film, LightPath& path, RenderingContext& ctx) const;

    // evaluate BSDF at ray's intersection and generate scattered ray
    bool AdvancePath(PathState& path, const ShadingData& shadingData, RenderingContext& ctx) const;

    // connect a camera path end to a light path end and return contribution
    const RayColor ConnectVertices(PathState& cameraPathState, const ShadingData& shadingData, const LightVertex& lightVertex, RenderingContext& ctx) const;

    // connect a light path to camera directly and splat the contribution onto film
    void ConnectToCamera(const Camera& camera, Film& film, const LightVertex& lightVertex, RenderingContext& ctx) const;

    Uint32 mMinPathLength;
    Uint32 mMaxPathLength;

    Uint32 mLightPathsCount;
    float mMisVertexMergingWeightFactor;
    float mMisVertexConnectionWeightFactor;
};

} // namespace rt
