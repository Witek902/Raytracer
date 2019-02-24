#include "PCH.h"
#include "VertexConnectionAndMerging.h"
#include "RendererContext.h"
#include "Context.h"
#include "Film.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"
#include "Scene/Light/Light.h"
#include "Material/Material.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;

RT_FORCE_INLINE static constexpr float Mis(const float samplePdf)
{
    return samplePdf;
}

RT_FORCE_INLINE static constexpr float PdfAtoW(const float pdfA, const float distance, const float cosThere)
{
    return pdfA * Sqr(distance) / Abs(cosThere);
}

RT_FORCE_INLINE static constexpr float PdfWtoA(const float pdfW, const float distance, const float cosThere)
{
    return pdfW * Abs(cosThere) / Sqr(distance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class BidirectionalPathTracerContext : public IRendererContext
{
public:
    using LightVertex = VertexConnectionAndMerging::LightVertex;

    // list of all recorded light vertices
    std::vector<LightVertex> lightVertices;

    // marks each path end index in the 'mLightVertices' array
    // Note: path length is obtained by comparing two consequent values
    std::vector<Uint32> pathEnds;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

VertexConnectionAndMerging::VertexConnectionAndMerging(const Scene& scene)
    : IRenderer(scene)
    , mLightPathsCount(0)
{
    mBSDFSamplingWeight = Vector4(1.0f);
    mLightSamplingWeight = Vector4(1.0f);
    mVertexConnectingWeight = Vector4(1.0f);
    mCameraConnectingWeight = Vector4(1.0f);
    mVertexMergingWeight = Vector4(1.0f);

    mMaxPathLength = 15;

    mMinMergingRadius = 0.005f;
    mMergingRadius = 0.05f;
    mMergingRadiusMultiplier = 0.98f;
}

VertexConnectionAndMerging::~VertexConnectionAndMerging() = default;

const char* VertexConnectionAndMerging::GetName() const
{
    return "VCM";
}

RendererContextPtr VertexConnectionAndMerging::CreateContext() const
{
    return std::make_unique<BidirectionalPathTracerContext>();
}

void VertexConnectionAndMerging::PreRender(const Film& film)
{
    mLightPathsCount = film.GetHeight() * film.GetWidth();

    mMergingRadius *= mMergingRadiusMultiplier;
    mMergingRadius = Max(mMergingRadius, mMinMergingRadius);

    // Factor used to normalize vertex merging contribution.
    // We divide the summed up energy by disk radius and number of light paths
    mVertexMergingNormalizationFactor = 1.0f / (Sqr(mMergingRadius) * RT_PI * mLightPathsCount);

    const float etaVCM = RT_PI * Sqr(mMergingRadius) * mLightPathsCount;
    mMisVertexMergingWeightFactor = mUseVertexMerging ? Mis(etaVCM) : 0.0f;
    mMisVertexConnectionWeightFactor = mUseVertexConnection ?  Mis(1.f / etaVCM) : 0.0f;
}

void VertexConnectionAndMerging::PreRender(RenderingContext& ctx)
{
    RT_ASSERT(ctx.rendererContext);
    BidirectionalPathTracerContext& rendererContext = *static_cast<BidirectionalPathTracerContext*>(ctx.rendererContext.get());

    // Stage 1 - prepare data structures
    rendererContext.lightVertices.clear();
    rendererContext.pathEnds.clear();

    // TODO this is duplicated
    mLightVertices.clear();
    mLightPathEnds.clear();
}

void VertexConnectionAndMerging::PreRenderPixel(const RenderParam& param, RenderingContext& ctx) const
{
    // Stage 2 - trace light paths
    TraceLightPath(param.camera, param.film, ctx);
}

void VertexConnectionAndMerging::PreRenderGlobal(RenderingContext& ctx)
{
    RT_ASSERT(ctx.rendererContext);
    BidirectionalPathTracerContext& rendererContext = *static_cast<BidirectionalPathTracerContext*>(ctx.rendererContext.get());

    // Stage 3a - merge vertices list
    // TODO is there any way to get rid of this? (or at least make it multithreaded)
    {
        const Uint32 pathEndsOffset = Uint32(mLightVertices.size());

        mLightVertices.reserve(mLightVertices.size() + rendererContext.lightVertices.size());
        for (const LightVertex& vertex : rendererContext.lightVertices)
        {
            mLightVertices.push_back(vertex);
        }

        mLightPathEnds.reserve(mLightPathEnds.size() + rendererContext.pathEnds.size());
        for (const Uint32 pathEndIndex : rendererContext.pathEnds)
        {
            mLightPathEnds.push_back(pathEndsOffset + pathEndIndex);
        }
    }
}

void VertexConnectionAndMerging::PreRenderGlobal()
{
    // Stage 3b - build hash grid of all light vertices
    // TODO make it multithreaded
    if (mUseVertexMerging)
    {
        mHashGrid.Build(mLightVertices, mMergingRadius);
    }
}

const RayColor VertexConnectionAndMerging::RenderPixel(const math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const
{
    // Stage 4 - trace camera paths

    RayColor resultColor = RayColor::Zero();

    // initialize camera path
    PathState pathState{ ray };
    {
        const float cameraPdf = param.camera.PdfW(ray.dir);

        pathState.dVC = 0.0f;
        pathState.dVM = 0.0f;
        pathState.dVCM = Mis(1.0f / cameraPdf);
        pathState.lastSpecular = true;
    }

    for (;;)
    {
        RT_ASSERT(pathState.ray.IsValid());

        HitPoint hitPoint;
        ctx.localCounters.Reset();
        mScene.Traverse_Single({ pathState.ray, hitPoint, ctx });
        ctx.counters.Append(ctx.localCounters);

        // ray missed - return background light color
        if (hitPoint.distance == FLT_MAX)
        {
            resultColor += pathState.throughput * EvaluateGlobalLights(pathState, ctx);
            break;
        }

        // we hit a light directly
        if (hitPoint.subObjectId == RT_LIGHT_OBJECT)
        {
            const ILight& light = mScene.Internal_GetLightByObjectId(hitPoint.objectId);

            // TODO this duplicates the code from below - requires some refactoring
            {
                const Vector4 hitPos = pathState.ray.GetAtDistance(hitPoint.distance);
                const Vector4 normal = light.GetNormal(hitPos);
                const float cosTheta = Vector4::Dot3(pathState.ray.dir, normal);
                pathState.dVCM *= Mis(Sqr(hitPoint.distance));
                pathState.dVCM /= Mis(Abs(cosTheta));
                pathState.dVC /= Mis(Abs(cosTheta));
                pathState.dVM /= Mis(Abs(cosTheta));
            }

            resultColor += pathState.throughput * EvaluateLight(light, hitPoint.distance, pathState, ctx);
            break;
        }

        // fill up structure with shading data
        ShadingData shadingData;
        {
            mScene.ExtractShadingData(pathState.ray, hitPoint, ctx.time, shadingData);

            RT_ASSERT(shadingData.material != nullptr);
            shadingData.material->EvaluateShadingData(ctx.wavelength, shadingData);
        }

        // TODO move it above "EvaluateLight" section - requires getting "shading data" for light surface...
        // update MIS quantities
        {
            const float cosTheta = Vector4::Dot3(pathState.ray.dir, shadingData.frame[2]);
            pathState.dVCM *= Mis(Sqr(hitPoint.distance));
            pathState.dVCM /= Mis(Abs(cosTheta));
            pathState.dVC /= Mis(Abs(cosTheta));
            pathState.dVM /= Mis(Abs(cosTheta));
        }

        // accumulate material emission color
        // Note: no importance sampling for this
        {
            const Spectrum emissionSpectrum(shadingData.material->emission.Evaluate(shadingData.texCoord));
            const RayColor emissionColor = RayColor::Resolve(ctx.wavelength, emissionSpectrum);
            RT_ASSERT(emissionColor.IsValid());
            resultColor += pathState.throughput * emissionColor;
        }

        if (pathState.length >= mMaxPathLength)
        {
            // path is too long for any other connection
            break;
        }

        const bool isDeltaBsdf = shadingData.material->GetBSDF()->IsDelta();

        // Vertex Connection -sample lights directly (a.k.a. next event estimation)
        if (!isDeltaBsdf && mUseVertexConnection)
        {
            resultColor += pathState.throughput * SampleLights(shadingData, pathState, ctx);
        }

        // Vertex Connection - connect camera vertex to light vertices (bidirectional path tracing)
        if (!isDeltaBsdf && mUseVertexConnection && !mLightPathEnds.empty())
        {
            RayColor vertexConnectionColor = RayColor::Zero();

            // get indices of the light vertices that build a bath for given pixel
            // Note: the path used here does not come from the same pixel index as in pre-render step,
            // but it doesn't matter, because all the path are random anyways
            Uint32 lightPathStart = (param.pixelIndex == 0) ? 0 : mLightPathEnds[param.pixelIndex - 1];
            Uint32 lightPathEnd = mLightPathEnds[param.pixelIndex];

            for (Uint32 i = lightPathStart; i < lightPathEnd; ++i)
            {
                const LightVertex& lightVertex = mLightVertices[i];

                // full path would be too long
                // Note: all other light vertices can be skiped, as they will produce even longer paths
                if (lightVertex.pathLength + pathState.length + 1u > mMaxPathLength)
                {
                    break;
                }

                vertexConnectionColor += lightVertex.throughput * ConnectVertices(pathState, shadingData, lightVertex, ctx);
            }

            vertexConnectionColor *= RayColor::Resolve(ctx.wavelength, Spectrum(mVertexConnectingWeight));

            resultColor += pathState.throughput * vertexConnectionColor;
        }

        // Vertex Merging - merge camera vertex to light vertices nearby
        if (!isDeltaBsdf && mUseVertexMerging && !mLightPathEnds.empty())
        {
            RayColor vertexMergingColor = MergeVertices(pathState, shadingData, ctx);
            vertexMergingColor *= RayColor::Resolve(ctx.wavelength, Spectrum(mVertexMergingWeight));
            resultColor += (pathState.throughput * vertexMergingColor) * mVertexMergingNormalizationFactor;
        }

        // check if the ray depth won't be exeeded in the next iteration
        if (pathState.length > mMaxPathLength)
        {
            break;
        }

        // continue random walk
        if (!AdvancePath(pathState, shadingData, ctx, PathType::Camera))
        {
            break;
        }
    }

    // param.film.AccumulateColor(param.x, param.y, resultColor);
    return resultColor;
}

void VertexConnectionAndMerging::TraceLightPath(const Camera& camera, Film& film, RenderingContext& ctx) const
{
    RT_ASSERT(ctx.rendererContext);
    BidirectionalPathTracerContext& rendererContext = *static_cast<BidirectionalPathTracerContext*>(ctx.rendererContext.get());

    PathState pathState;

    if (!GenerateLightSample(pathState, ctx))
    {
        // failed to generate initial ray - return empty path
        return;
    }

    for (;;)
    {
        RT_ASSERT(pathState.ray.IsValid());

        HitPoint hitPoint;
        ctx.localCounters.Reset();
        mScene.Traverse_Single({ pathState.ray, hitPoint, ctx });
        ctx.counters.Append(ctx.localCounters);

        if (hitPoint.distance == FLT_MAX)
        {
            break; // ray missed - terminate the path
        }

        if (hitPoint.subObjectId == RT_LIGHT_OBJECT)
        {
            break; // for now, light sources do not reflect light
        }

        // fill up structure with shading data
        ShadingData shadingData;
        {
            // TODO this is copy-paste from TraceRay_Single
            mScene.ExtractShadingData(pathState.ray, hitPoint, ctx.time, shadingData);

            RT_ASSERT(shadingData.material != nullptr);
            shadingData.material->EvaluateShadingData(ctx.wavelength, shadingData);
        }

        // update MIS quantities
        {
            if (pathState.length > 1 || pathState.isFiniteLight)
            {
                pathState.dVCM *= Mis(Sqr(hitPoint.distance));
            }

            const float cosTheta = Vector4::Dot3(pathState.ray.dir, shadingData.frame[2]);
            pathState.dVCM /= Mis(Abs(cosTheta));
            pathState.dVC  /= Mis(Abs(cosTheta));
            pathState.dVM  /= Mis(Abs(cosTheta));
        }

        // record the vertex for non-specular materials
        const bool isDeltaBsdf = shadingData.material->GetBSDF()->IsDelta();
        if (!isDeltaBsdf && (mUseVertexConnection || mUseVertexMerging))
        {
            rendererContext.lightVertices.emplace_back();
            LightVertex& vertex = rendererContext.lightVertices.back();

            vertex.pathLength = Uint8(pathState.length);
            vertex.throughput = pathState.throughput;
            vertex.dVC = pathState.dVC;
            vertex.dVM = pathState.dVM;
            vertex.dVCM = pathState.dVCM;
            PackShadingData(vertex.shadingData, shadingData);

            // connect vertex to camera directly
            if (mUseVertexConnection)
            {
                ConnectToCamera(camera, film, vertex, ctx);
            }
        }

        if (pathState.length + 2 > mMaxPathLength)
        {
            break; // check if the ray depth won't be exeeded in the next iteration
        }

        if (!AdvancePath(pathState, shadingData, ctx, PathType::Light))
        {
            break;
        }
    }

    const Uint32 numVertces = Uint32(rendererContext.lightVertices.size());
    rendererContext.pathEnds.push_back(numVertces);
}

bool VertexConnectionAndMerging::GenerateLightSample(PathState& outPath, RenderingContext& ctx) const
{
    const auto& allLocalLights = mScene.GetLights();
    if (allLocalLights.empty())
    {
        // no lights on the scene
        return false;
    }

    const float lightPickProbability = 1.0f / (float)allLocalLights.size();
    const Uint32 lightIndex = ctx.randomGenerator.GetInt() % (Uint32)allLocalLights.size(); // TODO get rid of division
    const LightPtr& light = allLocalLights[lightIndex];

    ILight::EmitResult emitResult;
    const RayColor throughput = light->Emit(ctx, emitResult);

    if (throughput.AlmostZero())
    {
        // generated sample is too weak - skip it
        return false;
    }

    emitResult.directPdfA *= lightPickProbability;
    emitResult.emissionPdfW *= lightPickProbability;
    
    if (emitResult.emissionPdfW < FLT_EPSILON)
    {
        return false;
    }

    const float emissionInvPdfW = 1.0f / emitResult.emissionPdfW;

    outPath.ray = Ray(emitResult.position, emitResult.direction);
    outPath.ray.origin += outPath.ray.dir * 0.0001f;
    outPath.throughput = throughput * emissionInvPdfW;
    outPath.isFiniteLight = light->IsFinite();

    // setup MIS weights
    {
        outPath.dVCM = Mis(emitResult.directPdfA * emissionInvPdfW);

        if (!light->IsDelta())
        {
            const float cosAtLight = outPath.isFiniteLight ? emitResult.cosAtLight : 1.0f;
            outPath.dVC = Mis(cosAtLight * emissionInvPdfW);
        }
        else
        {
            outPath.dVC = 0.0f;
        }

        outPath.dVM = outPath.dVC * mMisVertexConnectionWeightFactor;
    }

    return true;
}

bool VertexConnectionAndMerging::AdvancePath(PathState& path, const ShadingData& shadingData, RenderingContext& ctx, PathType pathType) const
{
    RT_UNUSED(pathType);

    // sample BSDF
    Vector4 incomingDirWorldSpace;
    float bsdfDirPdf;
    BSDF::EventType sampledEvent = BSDF::NullEvent;
    const RayColor bsdfValue = shadingData.material->Sample(ctx.wavelength, incomingDirWorldSpace, shadingData, ctx.randomGenerator, &bsdfDirPdf, &sampledEvent);

    const float cosThetaOut = Abs(Vector4::Dot3(incomingDirWorldSpace, shadingData.frame[2]));

    if (sampledEvent == BSDF::NullEvent)
    {
        return false;
    }

    RT_ASSERT(bsdfValue.IsValid());

    path.throughput *= bsdfValue;
    if (path.throughput.AlmostZero())
    {
        return false;
    }

    RT_ASSERT(bsdfDirPdf >= 0.0f);
    RT_ASSERT(IsValid(bsdfDirPdf));

    // TODO Russian roulette

    float bsdfRevPdf = bsdfDirPdf;

    // TODO for some reason this gives wrong result
    // evaluate reverse PDF
    if ((sampledEvent & BSDF::SpecularEvent) == 0)
    {
        const BSDF::EvaluationContext evalContext =
        {
            *shadingData.material,
            shadingData.materialParams,
            ctx.wavelength,
            shadingData.WorldToLocal(shadingData.outgoingDirWorldSpace),
            -shadingData.WorldToLocal(incomingDirWorldSpace),
        };
    
        bsdfRevPdf = shadingData.material->GetBSDF()->Pdf(evalContext, BSDF::ReversePdf);
    }

    if (sampledEvent & BSDF::SpecularEvent)
    {
        RT_ASSERT(bsdfRevPdf == bsdfDirPdf);
        path.dVC *= Mis(cosThetaOut);
        path.dVM *= Mis(cosThetaOut);
        path.dVCM = 0.0f;
        path.lastSpecular = true;
    }
    else
    {
        const float invBsdfDirPdf = 1.0f / bsdfDirPdf;
        path.dVC = Mis(cosThetaOut * invBsdfDirPdf) * (path.dVC * Mis(bsdfRevPdf) + path.dVCM + mMisVertexMergingWeightFactor);
        path.dVM = Mis(cosThetaOut * invBsdfDirPdf) * (path.dVM * Mis(bsdfRevPdf) + path.dVCM * mMisVertexConnectionWeightFactor + 1.0f);
        path.dVCM = Mis(invBsdfDirPdf);
        path.lastSpecular = false;
    }

    RT_ASSERT(IsValid(path.dVC) && path.dVC >= 0.0f);
    RT_ASSERT(IsValid(path.dVM) && path.dVM >= 0.0f);
    RT_ASSERT(IsValid(path.dVCM) && path.dVCM >= 0.0f);

    // generate secondary ray
    path.ray = Ray(shadingData.frame.GetTranslation(), incomingDirWorldSpace);
    path.ray.origin += path.ray.dir * 0.001f;

    path.lastSampledBsdfEvent = sampledEvent;
    path.length++;

    RT_ASSERT(path.ray.IsValid());

    return true;
}

const RayColor VertexConnectionAndMerging::EvaluateLight(const ILight& light, float dist, const PathState& pathState, RenderingContext& ctx) const
{
    const Vector4 hitPos = pathState.ray.GetAtDistance(dist);

    float directPdfA, emissionPdfW;
    RayColor lightContribution = light.GetRadiance(ctx, pathState.ray.dir, hitPos, &directPdfA, &emissionPdfW);
    RT_ASSERT(lightContribution.IsValid());

    if (lightContribution.AlmostZero())
    {
        return RayColor::Zero();
    }

    RT_ASSERT(directPdfA >= 0.0f && IsValid(directPdfA));
    RT_ASSERT(emissionPdfW >= 0.0f && IsValid(emissionPdfW));

    // no weighting required for directly visible lights
    if (pathState.length > 1)
    {
        if (mUseVertexMerging && !mUseVertexConnection) // special case for photon mapping
        {
            if (!pathState.lastSpecular)
            {
                return RayColor::Zero();
            }
        }
        else
        {
            // TODO Russian roulette

            // compute MIS weight
            const float wCamera = Mis(directPdfA) * pathState.dVCM + Mis(emissionPdfW) * pathState.dVC;
            const float misWeight = 1.0f / (1.0f + wCamera);
            RT_ASSERT(misWeight >= 0.0f);

            lightContribution *= misWeight;
        }
    }

    lightContribution *= RayColor::Resolve(ctx.wavelength, Spectrum(mBSDFSamplingWeight));

    return lightContribution;
}

const RayColor VertexConnectionAndMerging::SampleLight(const ILight& light, const ShadingData& shadingData, const PathState& pathState, RenderingContext& ctx) const
{
    ILight::IlluminateParam illuminateParam = { shadingData, ctx };

    // calculate light contribution
    RayColor radiance = light.Illuminate(illuminateParam);
    if (radiance.AlmostZero())
    {
        return RayColor::Zero();
    }

    RT_ASSERT(radiance.IsValid());
    RT_ASSERT(IsValid(illuminateParam.outDirectPdfW) && illuminateParam.outDirectPdfW >= 0.0f);
    RT_ASSERT(IsValid(illuminateParam.outEmissionPdfW) && illuminateParam.outEmissionPdfW >= 0.0f);
    RT_ASSERT(IsValid(illuminateParam.outDistance) && illuminateParam.outDistance >= 0.0f);
    RT_ASSERT(IsValid(illuminateParam.outCosAtLight) && illuminateParam.outCosAtLight >= 0.0f);

    // calculate BSDF contribution
    float bsdfPdfW, bsdfRevPdfW;
    const RayColor bsdfFactor = shadingData.material->Evaluate(ctx.wavelength, shadingData, -illuminateParam.outDirectionToLight, &bsdfPdfW, &bsdfRevPdfW);
    RT_ASSERT(bsdfFactor.IsValid());

    if (bsdfFactor.AlmostZero())
    {
        return RayColor::Zero();
    }

    RT_ASSERT(bsdfPdfW > 0.0f && IsValid(bsdfPdfW));

    // cast shadow ray
    {
        HitPoint hitPoint;
        hitPoint.distance = illuminateParam.outDistance * 0.999f;

        Ray shadowRay(shadingData.frame.GetTranslation(), illuminateParam.outDirectionToLight);
        shadowRay.origin += shadowRay.dir * 0.001f;

        if (mScene.Traverse_Shadow_Single({ shadowRay, hitPoint, ctx }))
        {
            // shadow ray missed the light - light is occluded
            return RayColor::Zero();
        }
    }

    // TODO
    //const auto& allLocalLights = mScene.GetLights();
    //const float lightPickProbability = 1.0f / (float)allLocalLights.size();
    const float lightPickProbability = 1.0f;

    // TODO
    const float continuationProbability = 1.0f;
    bsdfPdfW *= light.IsDelta() ? 0.0f : continuationProbability;
    bsdfRevPdfW *= continuationProbability;

    const float cosToLight = Vector4::Dot3(shadingData.frame[2], illuminateParam.outDirectionToLight);
    if (cosToLight <= FLT_EPSILON)
    {
        return RayColor::Zero();
    }

    const float wLight = Mis(bsdfPdfW / (lightPickProbability * illuminateParam.outDirectPdfW));
    const float wCamera = Mis(illuminateParam.outEmissionPdfW * cosToLight / (illuminateParam.outDirectPdfW * illuminateParam.outCosAtLight)) * (mMisVertexMergingWeightFactor + pathState.dVCM + pathState.dVC * Mis(bsdfRevPdfW));
    const float misWeight = 1.0f / (wLight + 1.0f + wCamera);
    RT_ASSERT(misWeight >= 0.0f);

    return (radiance * bsdfFactor) * (misWeight / (lightPickProbability * illuminateParam.outDirectPdfW));
}

const RayColor VertexConnectionAndMerging::SampleLights(const ShadingData& shadingData, const PathState& pathState, RenderingContext& ctx) const
{
    RayColor accumulatedColor = RayColor::Zero();

    // TODO check only one (or few) lights per sample instead all of them
    // TODO check only nearest lights
    for (const LightPtr& light : mScene.GetLights())
    {
        accumulatedColor += SampleLight(*light, shadingData, pathState, ctx);
    }

    accumulatedColor *= RayColor::Resolve(ctx.wavelength, Spectrum(mLightSamplingWeight));

    return accumulatedColor;
}

const RayColor VertexConnectionAndMerging::EvaluateGlobalLights(const PathState& pathState, RenderingContext& ctx) const
{
    RayColor result = RayColor::Zero();

    for (const ILight* globalLight : mScene.GetGlobalLights())
    {
        result += EvaluateLight(*globalLight, FLT_MAX, pathState, ctx);
    }

    return result;
}

const RayColor VertexConnectionAndMerging::ConnectVertices(PathState& cameraPathState, const ShadingData& shadingData, const LightVertex& lightVertex, RenderingContext& ctx) const
{
    ShadingData lightVertexShadingData;
    UnpackShadingData(lightVertexShadingData, lightVertex.shadingData);

    // compute connection direction (from camera vertex to light vertex)
    Vector4 lightDir = lightVertexShadingData.frame.GetTranslation() - shadingData.frame.GetTranslation();
    const float distanceSqr = lightDir.SqrLength3();
    const float distance = sqrtf(distanceSqr);
    lightDir /= distance;

    const float cosCameraVertex = shadingData.CosTheta(lightDir);
    const float cosLightVertex = lightVertexShadingData.CosTheta(-lightDir);

    if (cosCameraVertex <= 0.0f || cosLightVertex <= 0.0f)
    {
        // line between vertices is occluded (due to backface culling)
        return RayColor::Zero();
    }

    // compute geometry term
    const float geometryTerm = 1.0f / distanceSqr;

    // evaluate BSDF at camera vertex
    float cameraBsdfPdfW, cameraBsdfRevPdfW;
    const RayColor cameraFactor = shadingData.material->Evaluate(ctx.wavelength, shadingData, -lightDir, &cameraBsdfPdfW, &cameraBsdfRevPdfW);
    RT_ASSERT(cameraFactor.IsValid());
    if (cameraFactor.AlmostZero())
    {
        return RayColor::Zero();
    }

    // evaluate BSDF at light vertex
    float lightBsdfPdfW, lightBsdfRevPdfW;
    const RayColor lightFactor = lightVertex.shadingData.material->Evaluate(ctx.wavelength, lightVertexShadingData, lightDir, &lightBsdfPdfW, &lightBsdfRevPdfW);
    RT_ASSERT(lightFactor.IsValid());
    if (lightFactor.AlmostZero())
    {
        return RayColor::Zero();
    }

    //// cast shadow ray
    {
        HitPoint hitPoint;
        hitPoint.distance = distance * 0.999f;

        Ray shadowRay(shadingData.frame.GetTranslation(), lightDir);
        shadowRay.origin += shadowRay.dir * 0.0001f;

        if (mScene.Traverse_Shadow_Single({ shadowRay, hitPoint, ctx }))
        {
            // line between vertices is occluded by other geometry
            return RayColor::Zero();
        }
    }

    // TODO
    const float continuationProbability = 1.0f;
    lightBsdfPdfW *= continuationProbability;
    lightBsdfRevPdfW *= continuationProbability;

    // compute MIS weight
    const float cameraBsdfPdfA = PdfWtoA(cameraBsdfPdfW, distance, cosLightVertex);
    const float lightBsdfPdfA = PdfWtoA(lightBsdfPdfW, distance, cosCameraVertex);

    const float wLight = Mis(cameraBsdfPdfA) * (mMisVertexMergingWeightFactor + lightVertex.dVCM + lightVertex.dVC * Mis(lightBsdfRevPdfW));
    RT_ASSERT(IsValid(wLight) && wLight >= 0.0f);

    const float wCamera = Mis(lightBsdfPdfA) * (mMisVertexMergingWeightFactor + cameraPathState.dVCM + cameraPathState.dVC * Mis(cameraBsdfRevPdfW));
    RT_ASSERT(IsValid(wCamera) && wCamera >= 0.0f);

    const float misWeight = 1.0f / (wLight + 1.0f + wCamera);
    RT_ASSERT(misWeight >= 0.0f);

    const RayColor contribution = (cameraFactor * lightFactor) * (geometryTerm * misWeight);
    RT_ASSERT(contribution.IsValid());

    return contribution;
}

const RayColor VertexConnectionAndMerging::MergeVertices(PathState& cameraPathState, const ShadingData& shadingData, RenderingContext& ctx) const
{
    class RangeQuery
    {
    public:
        RT_FORCE_INLINE RangeQuery(const VertexConnectionAndMerging& renderer, const PathState& cameraPathState, const ShadingData& shadingData, RenderingContext& ctx)
            : mRenderer(renderer)
            , mShadingData(shadingData)
            , mCameraPathState(cameraPathState)
            , mContribution(RayColor::Zero())
            , mContext(ctx)
        {}

        RT_FORCE_INLINE const RayColor& GetContribution() const { return mContribution; }

        void operator()(Uint32 lightVectexIndex)
        {
            const LightVertex& lightVertex = mRenderer.mLightVertices[lightVectexIndex];

            if (lightVertex.pathLength + mCameraPathState.length > mRenderer.mMaxPathLength)
            {
                return;
            }

            // Retrieve light incoming direction in world coordinates
            const Vector4 lightDirection = -Vector4(&lightVertex.shadingData.outgoingDirWorldSpace.x);

            float cameraBsdfDirPdfW, cameraBsdfRevPdfW;
            const RayColor cameraBsdfFactor = mShadingData.material->Evaluate(mContext.wavelength, mShadingData, lightDirection, &cameraBsdfDirPdfW, &cameraBsdfRevPdfW);
            if (cameraBsdfFactor.AlmostZero())
            {
                return;
            }

            const float cosToLight = mShadingData.CosTheta(-lightDirection);
            if (cosToLight < FLT_EPSILON)
            {
                return;
            }

            // TODO
            //cameraBsdfDirPdfW *= mCameraBsdf.ContinuationProb();
            //cameraBsdfRevPdfW *= aLightVertex.mBSDF.ContinuationProb();

            // Partial light sub-path MIS weight [tech. rep. (38)]
            const float wLight = lightVertex.dVCM * mRenderer.mMisVertexConnectionWeightFactor + lightVertex.dVM * Mis(cameraBsdfDirPdfW);
            const float wCamera = mCameraPathState.dVCM * mRenderer.mMisVertexConnectionWeightFactor + mCameraPathState.dVM * Mis(cameraBsdfRevPdfW);
            const float misWeight = 1.0f / (wLight + 1.0f + wCamera);

            mContribution += (cameraBsdfFactor * lightVertex.throughput) * (misWeight / cosToLight);
        }

    private:
        RenderingContext& mContext;
        const VertexConnectionAndMerging& mRenderer;
        const ShadingData& mShadingData;
        const PathState& mCameraPathState;
        RayColor mContribution;
    };

    const Vector4& cameraVertexPos = shadingData.frame.GetTranslation();

    RangeQuery query(*this, cameraPathState, shadingData, ctx);
    mHashGrid.Process(cameraVertexPos, mLightVertices, query);
    return query.GetContribution();
}

void VertexConnectionAndMerging::ConnectToCamera(const Camera& camera, Film& film, const LightVertex& lightVertex, RenderingContext& ctx) const
{
    ShadingData lightVertexShadingData;
    UnpackShadingData(lightVertexShadingData, lightVertex.shadingData);

    const Vector4 cameraPos = camera.GetTransform().GetTranslation();
    const Vector4 samplePos = lightVertexShadingData.frame.GetTranslation();

    Vector4 dirToCamera = cameraPos - samplePos;

    const float cameraDistanceSqr = dirToCamera.SqrLength3();
    const float cameraDistance = sqrtf(cameraDistanceSqr);

    dirToCamera /= cameraDistance;

    // calculate BSDF contribution
    float bsdfPdfW, bsdfRevPdfW;
    const RayColor cameraFactor = lightVertex.shadingData.material->Evaluate(ctx.wavelength, lightVertexShadingData, -dirToCamera, &bsdfPdfW, &bsdfRevPdfW);
    RT_ASSERT(cameraFactor.IsValid());

    if (cameraFactor.AlmostZero())
    {
        return;
    }

    Vector4 filmPos;
    if (!camera.WorldToFilm(samplePos, filmPos))
    {
        // vertex is not visible in the viewport
        return;
    }

    HitPoint shadowHitPoint;
    shadowHitPoint.distance = cameraDistance * 0.999f;

    Ray shadowRay(samplePos, dirToCamera);
    shadowRay.origin += shadowRay.dir * 0.0001f;

    if (mScene.Traverse_Shadow_Single({ shadowRay, shadowHitPoint, ctx }))
    {
        // vertex is occluded
        return;
    }

    const float cosToCamera = Vector4::Dot3(dirToCamera, lightVertexShadingData.frame[2]);
    if (cosToCamera <= FLT_EPSILON)
    {
        return;
    }

    const float cameraPdfW = camera.PdfW(-dirToCamera);
    const float cameraPdfA = cameraPdfW * cosToCamera / cameraDistanceSqr;

    // compute MIS weight
    const float wLight = Mis(cameraPdfA) * (mMisVertexMergingWeightFactor + lightVertex.dVCM + lightVertex.dVC * Mis(bsdfRevPdfW));
    const float misWeight = 1.0f / (wLight + 1.0f);
    RT_ASSERT(misWeight >= 0.0f);

    RayColor contribution = (cameraFactor * lightVertex.throughput) * (misWeight * cameraPdfA / (cosToCamera));
    contribution *= RayColor::Resolve(ctx.wavelength, Spectrum(mCameraConnectingWeight));

    const Vector4 value = contribution.ConvertToTristimulus(ctx.wavelength);
    film.AccumulateColor(filmPos, value, ctx.randomGenerator);
}

} // namespace rt
