#pragma once

#include "Renderer.h"
#include "../Material/BSDF/BSDF.h"
#include "../Utils/HashGrid.h"

namespace rt {

struct ShadingData;
class ILight;

// Vertex Connection and Merging
//
// Implements "Light Transport Simulation with Vertex Connection and Merging"
// Iliyan Georgiev, Jaroslav Krivanek, Tomas Davidovic, Philipp Slusallek
// ACM Transactions on Graphics 31(6) (SIGGRAPH Asia 2012).
//
// Based on https://github.com/SmallVCM/SmallVCM
//
class VertexConnectionAndMerging : public IRenderer
{
public:
    VertexConnectionAndMerging(const Scene& scene);
    ~VertexConnectionAndMerging();

    virtual const char* GetName() const override;
    virtual RendererContextPtr CreateContext() const;

    virtual void PreRender(const Film& film) override;
    virtual void PreRender(RenderingContext& ctx) override;
    virtual void PreRenderPixel(const RenderParam& param, RenderingContext& ctx) const override;
    virtual void PreRenderGlobal(RenderingContext& ctx) override;
    virtual void PreRenderGlobal() override;
    virtual const RayColor RenderPixel(const math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const override;

    // for debugging
    math::Vector4 mBSDFSamplingWeight;
    math::Vector4 mLightSamplingWeight;
    math::Vector4 mVertexConnectingWeight;
    math::Vector4 mVertexMergingWeight;
    math::Vector4 mCameraConnectingWeight;

    bool mUseVertexConnection = true;
    bool mUseVertexMerging = true;

    struct LightVertex
    {
        PackedShadingData shadingData;
        RayColor throughput;            // TODO should be Float3

        // quantities for MIS weight calculation
        float dVC;
        float dVM;
        float dVCM;

        Uint8 pathLength;

        // used by hash grid query
        RT_FORCE_INLINE const math::Vector4 GetPosition() const { return math::Vector4(shadingData.position); }
    };

private:

    enum class PathType
    {
        Camera,
        Light,
    };

    // describes current state of path coming from camera or a light
    struct PathState
    {
        math::Ray ray;
        RayColor throughput = RayColor::One();

        // quantities for MIS weight calculation
        float dVC = 0.0f;
        float dVM = 0.0f;
        float dVCM = 0.0f;

        Uint32 length = 1u;
        BSDF::EventType lastSampledBsdfEvent = BSDF::NullEvent;
        bool lastSpecular = true;
        bool isFiniteLight = false;
    };

    // importance sample light sources
    const RayColor SampleLights(const ShadingData& shadingData, const PathState& pathState, RenderingContext& ctx) const;

    // importance sample single light source
    const RayColor SampleLight(const ILight& light, const ShadingData& shadingData, const PathState& pathState, RenderingContext& ctx) const;

    // compute radiance from a hit local lights
    const RayColor EvaluateLight(const ILight& light, float dist, const PathState& pathState, RenderingContext& ctx) const;

    // compute radiance from global lights
    const RayColor EvaluateGlobalLights(const PathState& pathState, RenderingContext& ctx) const;

    // generate initial camera ray
    bool GenerateCameraPath(PathState& path, RenderingContext& ctx) const;

    // generate initial light ray
    bool GenerateLightSample(PathState& pathState, RenderingContext& ctx) const;
    void TraceLightPath(const Camera& camera, Film& film, RenderingContext& ctx) const;

    // evaluate BSDF at ray's intersection and generate scattered ray
    bool AdvancePath(PathState& path, const ShadingData& shadingData, RenderingContext& ctx, PathType pathType) const;

    // connect a camera path end to a light path end and return contribution
    const RayColor ConnectVertices(PathState& cameraPathState, const ShadingData& shadingData, const LightVertex& lightVertex, RenderingContext& ctx) const;

    // merge a camera path vertex to light vertices nearby and return contribution
    const RayColor MergeVertices(PathState& cameraPathState, const ShadingData& shadingData, RenderingContext& ctx) const;

    // connect a light path to camera directly and splat the contribution onto film
    void ConnectToCamera(const Camera& camera, Film& film, const LightVertex& lightVertex, RenderingContext& ctx) const;

    Uint32 mMaxPathLength;

    Uint32 mLightPathsCount;

    float mMergingRadius;
    float mMinMergingRadius;
    float mMergingRadiusMultiplier;

    float mVertexMergingNormalizationFactor;
    float mMisVertexMergingWeightFactor;
    float mMisVertexConnectionWeightFactor;

    // acceleration structure used for vertex merging
    HashGrid mHashGrid;

    // list of all recorded light vertices
    std::vector<LightVertex> mLightVertices;

    // marks each path end index in the 'mLightVertices' array
    // Note: path length is obtained by comparing two consequent values
    std::vector<Uint32> mLightPathEnds;
};

} // namespace rt
