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
#include "Sampling/GenericSampler.h"

namespace rt {

using namespace math;

RT_FORCE_INLINE static constexpr float Mis(const float samplePdf)
{
    return samplePdf;
}

RT_FORCE_INLINE static constexpr float PdfWtoA(const float pdfW, const float distance, const float cosThere)
{
    return pdfW * Abs(cosThere) / Sqr(distance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static constexpr Uint32 g_MaxLightVertices = 256;

class RT_ALIGN(64) VertexConnectionAndMergingContext : public IRendererContext, public Aligned<64>
{
public:
    using Photon = VertexConnectionAndMerging::Photon;
    using LightVertex = VertexConnectionAndMerging::LightVertex;

    // list of photons recorded from a single thread
    DynArray<Photon> photons;

    // list of light vertices used in current pixel processing
    Uint32 numLightVertices = 0;
    LightVertex lightVertices[g_MaxLightVertices];
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static_assert(sizeof(VertexConnectionAndMerging::Photon) == 32, "Invalid photon size");
static_assert(sizeof(VertexConnectionAndMerging::LightVertex) == 192, "Invalid light vertex size");

VertexConnectionAndMerging::VertexConnectionAndMerging(const Scene& scene)
    : IRenderer(scene)
    , mLightPathsCount(0)
{
    mBSDFSamplingWeight = Vector4(1.0f);
    mLightSamplingWeight = Vector4(1.0f);
    mVertexConnectingWeight = Vector4(1.0f);
    mCameraConnectingWeight = Vector4(1.0f);
    mVertexMergingWeight = Vector4(1.0f);

    mUseVertexConnection = true;
    mUseVertexMerging = true;
    mMaxPathLength = 12;
    mInitialMergingRadius = 0.01f;
    mMergingRadiusVC = mMergingRadiusVM = mInitialMergingRadius;
    mMinMergingRadius = 0.001f;
    mMergingRadiusMultiplier = 1.0f; // 0.98f; // VCM_TODO
}

VertexConnectionAndMerging::~VertexConnectionAndMerging() = default;

const char* VertexConnectionAndMerging::GetName() const
{
    return "VCM";
}

RendererContextPtr VertexConnectionAndMerging::CreateContext() const
{
    return std::make_unique<VertexConnectionAndMergingContext>();
}

void VertexConnectionAndMerging::PreRender(Uint32 passNumber, const Film& film)
{
    RT_ASSERT(mInitialMergingRadius >= mMinMergingRadius);
    RT_ASSERT(mMergingRadiusMultiplier > 0.0f);
    RT_ASSERT(mMergingRadiusMultiplier <= 1.0f);
    RT_ASSERT(mMaxPathLength > 0);

    mLightPathsCount = film.GetHeight() * film.GetWidth();

    if (passNumber == 0)
    {
        mMergingRadiusVC = mInitialMergingRadius;
        mMergingRadiusVM = mInitialMergingRadius;
    }
    else
    {
        // merging radius for vertex merging is delayed by 1 frame
        mMergingRadiusVM = mMergingRadiusVC;

        mMergingRadiusVC *= mMergingRadiusMultiplier;
        mMergingRadiusVC = Max(mMergingRadiusVC, mMinMergingRadius);
    }

    // Factor used to normalize vertex merging contribution.
    // We divide the summed up energy by disk radius and number of light paths
    mVertexMergingNormalizationFactor = 1.0f / (Sqr(mMergingRadiusVM) * RT_PI * mLightPathsCount);

    // compute MIS weights for vertex connection
    {
        const float etaVCM = RT_PI * Sqr(mMergingRadiusVC) * mLightPathsCount;
        // Note: we don't use merging in the first iteration
        mMisVertexMergingWeightFactorVC = (mUseVertexMerging && passNumber > 0) ? Mis(etaVCM) : 0.0f;
        mMisVertexConnectionWeightFactorVC = mUseVertexConnection ? Mis(1.f / etaVCM) : 0.0f;
    }

    // set MIS weights for vertex merging (delayed by 1 iteration)
    {
        const float etaVCM = RT_PI * Sqr(mMergingRadiusVM) * mLightPathsCount;
        mMisVertexMergingWeightFactorVM = mUseVertexMerging ? Mis(etaVCM) : 0.0f;
        mMisVertexConnectionWeightFactorVM = mUseVertexConnection ? Mis(1.f / etaVCM) : 0.0f;
    }
}

void VertexConnectionAndMerging::PreRender(Uint32 passNumber, RenderingContext& ctx)
{
    RT_ASSERT(ctx.rendererContext);
    VertexConnectionAndMergingContext& rendererContext = *static_cast<VertexConnectionAndMergingContext*>(ctx.rendererContext.get());

    if (passNumber == 0)
    {
        rendererContext.photons.Clear();
    }

    // TODO this is duplicated
    mPhotons.Clear();
}

void VertexConnectionAndMerging::PreRenderGlobal(RenderingContext& ctx)
{
    RT_ASSERT(ctx.rendererContext);
    VertexConnectionAndMergingContext& rendererContext = *static_cast<VertexConnectionAndMergingContext*>(ctx.rendererContext.get());

    // merge photon lists
    // TODO is there any way to get rid of this? (or at least make it multithreaded)
    const Uint32 oldPhotonsSize = mPhotons.Size();
    const Uint32 numPhotonsToAdd = rendererContext.photons.Size();
    mPhotons.Resize_SkipConstructor(oldPhotonsSize + numPhotonsToAdd);
    memcpy(mPhotons.Data() + oldPhotonsSize, rendererContext.photons.Data(), numPhotonsToAdd * sizeof(Photon));

    // prepare data structures
    rendererContext.photons.Clear();
}

void VertexConnectionAndMerging::PreRenderGlobal()
{
    // build hash grid of all light vertices
    // TODO make it multithreaded
    if (mUseVertexMerging)
    {
#ifdef RT_VCM_USE_KD_TREE
        mKdTree.Build(mPhotons);
#else
        mHashGrid.Build(mPhotons, mMergingRadiusVM);
#endif // RT_VCM_USE_KD_TREE
    }
}

const RayColor VertexConnectionAndMerging::RenderPixel(const math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const
{
    // step 1: trace light paths & record photons

    TraceLightPath(param.camera, param.film, ctx);


    // step 2: trace camera paths:

    const VertexConnectionAndMergingContext& rendererContext = *static_cast<const VertexConnectionAndMergingContext*>(ctx.rendererContext.get());

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

    HitPoint hitPoint;

    for (;;)
    {
        RT_ASSERT(pathState.ray.IsValid());

        hitPoint.distance = FLT_MAX;
        mScene.Traverse_Single({ pathState.ray, hitPoint, ctx });

        // ray missed - return background light color
        if (hitPoint.distance == FLT_MAX)
        {
            resultColor.MulAndAccumulate(pathState.throughput, EvaluateGlobalLights(param.iteration, pathState, ctx));
            break;
        }

        // we hit a light directly
        if (hitPoint.subObjectId == RT_LIGHT_OBJECT)
        {
            const ILight& light = mScene.GetLightByObjectId(hitPoint.objectId);

            // TODO this duplicates the code from below - requires some refactoring
            {
                const Vector4 hitPos = pathState.ray.GetAtDistance(hitPoint.distance);
                const Vector4 normal = light.GetNormal(hitPos);
                const float cosTheta = Vector4::Dot3(pathState.ray.dir, normal);
                const float invMis = 1.0f / Mis(Abs(cosTheta));
                pathState.dVCM *= Mis(Sqr(hitPoint.distance));
                pathState.dVCM *= invMis;
                pathState.dVC *= invMis;
                pathState.dVM *= invMis;
            }

            const RayColor lightColor = EvaluateLight(param.iteration, light, hitPoint.distance, pathState, ctx);
            RT_ASSERT(lightColor.IsValid());
            resultColor.MulAndAccumulate(pathState.throughput, lightColor);
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
            const float invMis = 1.0f / Mis(Abs(cosTheta));
            pathState.dVCM *= Mis(Sqr(hitPoint.distance));
            pathState.dVCM *= invMis;
            pathState.dVC *= invMis;
            pathState.dVM *= invMis;
        }

        // accumulate material emission color
        // Note: no importance sampling for this
        {
            const Spectrum emissionSpectrum(shadingData.material->emission.Evaluate(shadingData.texCoord));
            const RayColor emissionColor = RayColor::Resolve(ctx.wavelength, emissionSpectrum);
            RT_ASSERT(emissionColor.IsValid());
            resultColor.MulAndAccumulate(pathState.throughput, emissionColor);
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
            const RayColor lightColor = SampleLights(shadingData, pathState, ctx);
            RT_ASSERT(lightColor.IsValid());
            resultColor.MulAndAccumulate(pathState.throughput, lightColor);
        }

        // Vertex Connection - connect camera vertex to light vertices (bidirectional path tracing)
        const Uint32 numLightVertices = rendererContext.numLightVertices;
        if (!isDeltaBsdf && mUseVertexConnection && rendererContext.numLightVertices > 0)
        {
            RayColor vertexConnectionColor = RayColor::Zero();

            for (Uint32 i = 0; i < numLightVertices; ++i)
            {
                const LightVertex& lightVertex = rendererContext.lightVertices[i];

                // full path would be too long
                // Note: all other light vertices can be skiped, as they will produce even longer paths
                if (lightVertex.pathLength + pathState.length + 1u > mMaxPathLength)
                {
                    break;
                }

                vertexConnectionColor.MulAndAccumulate(lightVertex.throughput, ConnectVertices(pathState, shadingData, lightVertex, ctx));
            }

            vertexConnectionColor *= RayColor::Resolve(ctx.wavelength, Spectrum(mVertexConnectingWeight));
            RT_ASSERT(vertexConnectionColor.IsValid());
            resultColor.MulAndAccumulate(pathState.throughput, vertexConnectionColor);
        }

        // Vertex Merging - merge camera vertex to light vertices nearby
        if (!isDeltaBsdf && mUseVertexMerging && param.iteration > 0)
        {
            RayColor vertexMergingColor = MergeVertices(pathState, shadingData, ctx);
            RT_ASSERT(vertexMergingColor.IsValid());
            vertexMergingColor *= RayColor::Resolve(ctx.wavelength, Spectrum(mVertexMergingWeight));
            resultColor.MulAndAccumulate(pathState.throughput * vertexMergingColor, mVertexMergingNormalizationFactor);
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
    VertexConnectionAndMergingContext& rendererContext = *static_cast<VertexConnectionAndMergingContext*>(ctx.rendererContext.get());

    rendererContext.numLightVertices = 0;

    PathState pathState;

    if (!GenerateLightSample(pathState, ctx))
    {
        // failed to generate initial ray - return empty path
        return;
    }

    HitPoint hitPoint;

    for (;;)
    {
        RT_ASSERT(pathState.ray.IsValid());

        hitPoint.distance = FLT_MAX;
        mScene.Traverse_Single({ pathState.ray, hitPoint, ctx });

        if (hitPoint.distance == FLT_MAX)
        {
            break; // ray missed - terminate the path
        }

        if (hitPoint.subObjectId == RT_LIGHT_OBJECT)
        {
            break; // for now, light sources do not reflect light
        }

        RT_ASSERT(rendererContext.numLightVertices < g_MaxLightVertices);
        LightVertex& vertex = rendererContext.lightVertices[rendererContext.numLightVertices];

        // fill up structure with shading data
        ShadingData& shadingData = vertex.shadingData;
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
            const float invMis = 1.0f / Mis(Abs(cosTheta));
            pathState.dVCM *= invMis;
            pathState.dVC  *= invMis;
            pathState.dVM  *= invMis;
        }

        // record the vertex for non-specular materials
        if (!shadingData.material->GetBSDF()->IsDelta())
        {
            // store light vertex for connection
            if (mUseVertexConnection)
            {
                rendererContext.numLightVertices++;

                vertex.pathLength = Uint8(pathState.length);
                vertex.throughput = pathState.throughput;
                vertex.dVC = pathState.dVC;
                vertex.dVM = pathState.dVM;
                vertex.dVCM = pathState.dVCM;

                // connect vertex to camera directly
                ConnectToCamera(camera, film, vertex, ctx);
            }

            // store simplified light vertex (photon) for merging
            if (mUseVertexMerging)
            {
                rendererContext.photons.EmplaceBack();
                Photon& photon = rendererContext.photons.Back();

                photon.position = shadingData.frame[3].ToFloat3();
                photon.direction.FromVector(shadingData.outgoingDirWorldSpace);
                photon.throughput.FromVector(pathState.throughput.ConvertToTristimulus(ctx.wavelength));
                photon.dVM = pathState.dVM;
                photon.dVCM = pathState.dVCM;
                //photon.pathLength = Uint8(pathState.length);
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
}

bool VertexConnectionAndMerging::GenerateLightSample(PathState& outPath, RenderingContext& ctx) const
{
    const auto& allLocalLights = mScene.GetLights();
    if (allLocalLights.Empty())
    {
        // no lights on the scene
        return false;
    }

    const float lightPickProbability = 1.0f / (float)allLocalLights.Size();
    const Uint32 lightIndex = ctx.randomGenerator.GetInt() % allLocalLights.Size(); // TODO get rid of division
    const LightPtr& light = allLocalLights[lightIndex];

    const ILight::EmitParam emitParam =
    {
        ctx.wavelength,
        ctx.sampler->GetFloat2(),
        ctx.sampler->GetFloat2(),
    };

    ILight::EmitResult emitResult;
    const RayColor throughput = light->Emit(emitParam, emitResult);

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

    const ILight::Flags lightFlags = light->GetFlags();
    outPath.isFiniteLight = lightFlags & ILight::Flag_IsFinite;

    // setup MIS weights
    {
        outPath.dVCM = Mis(emitResult.directPdfA * emissionInvPdfW);

        if ((lightFlags & ILight::Flag_IsDelta) == 0)
        {
            const float cosAtLight = outPath.isFiniteLight ? emitResult.cosAtLight : 1.0f;
            outPath.dVC = Mis(cosAtLight * emissionInvPdfW);
        }
        else
        {
            outPath.dVC = 0.0f;
        }

        outPath.dVM = outPath.dVC * mMisVertexConnectionWeightFactorVC;
    }

    return true;
}

bool VertexConnectionAndMerging::AdvancePath(PathState& path, const ShadingData& shadingData, RenderingContext& ctx, PathType pathType) const
{
    Float3 sample;
    if (pathType == PathType::Camera)
    {
        sample = ctx.sampler->GetFloat3();
    }
    else
    {
        sample = ctx.randomGenerator.GetFloat3();
    }

    RT_ASSERT(sample.x >= 0.0f && sample.x < 1.0f);
    RT_ASSERT(sample.y >= 0.0f && sample.y < 1.0f);
    RT_ASSERT(sample.z >= 0.0f && sample.z < 1.0f);

    // sample BSDF
    Vector4 incomingDirWorldSpace;
    float bsdfDirPdf;
    BSDF::EventType sampledEvent = BSDF::NullEvent;
    const RayColor bsdfValue = shadingData.material->Sample(ctx.wavelength, incomingDirWorldSpace, shadingData, sample, &bsdfDirPdf, &sampledEvent);

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

    // generate secondary ray
    path.ray = Ray(shadingData.frame.GetTranslation(), incomingDirWorldSpace);
    path.ray.origin += path.ray.dir * 0.001f;
    RT_ASSERT(path.ray.IsValid());

    path.lastSampledBsdfEvent = sampledEvent;
    path.length++;

    // TODO Russian roulette

    if (sampledEvent & BSDF::SpecularEvent)
    {
        path.dVC *= Mis(cosThetaOut);
        path.dVM *= Mis(cosThetaOut);
        path.dVCM = 0.0f;
        path.lastSpecular = true;
    }
    else
    {
        // TODO for some reason this gives wrong result
        // evaluate reverse PDF
        const BSDF::EvaluationContext evalContext =
        {
            *shadingData.material,
            shadingData.materialParams,
            ctx.wavelength,
            shadingData.WorldToLocal(shadingData.outgoingDirWorldSpace),
            -shadingData.WorldToLocal(incomingDirWorldSpace),
        };

        const float bsdfRevPdf = shadingData.material->GetBSDF()->Pdf(evalContext, BSDF::ReversePdf);
        const float invBsdfDirPdf = 1.0f / bsdfDirPdf;

        path.dVC = Mis(cosThetaOut * invBsdfDirPdf) * (path.dVC * Mis(bsdfRevPdf) + path.dVCM + mMisVertexMergingWeightFactorVC);
        path.dVM = Mis(cosThetaOut * invBsdfDirPdf) * (path.dVM * Mis(bsdfRevPdf) + path.dVCM * mMisVertexConnectionWeightFactorVC + 1.0f);
        path.dVCM = Mis(invBsdfDirPdf);
        path.lastSpecular = false;
    }

    RT_ASSERT(IsValid(path.dVC) && path.dVC >= 0.0f);
    RT_ASSERT(IsValid(path.dVM) && path.dVM >= 0.0f);
    RT_ASSERT(IsValid(path.dVCM) && path.dVCM >= 0.0f);

    return true;
}

const RayColor VertexConnectionAndMerging::EvaluateLight(Uint32 iteration, const ILight& light, float dist, const PathState& pathState, RenderingContext& ctx) const
{
    const Vector4 hitPos = pathState.ray.GetAtDistance(dist);

    float directPdfA, emissionPdfW;
    RayColor lightContribution = light.GetRadiance(ctx, pathState.ray, hitPos, &directPdfA, &emissionPdfW);
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
        const bool useVertexMerging = mUseVertexMerging && iteration > 0;
        if (useVertexMerging && !mUseVertexConnection) // special case for photon mapping
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
    const ILight::IlluminateParam illuminateParam =
    {
        shadingData,
        ctx.wavelength,
        ctx.sampler->GetFloat2(),
    };

    // calculate light contribution
    ILight::IlluminateResult illuminateResult;
    RayColor radiance = light.Illuminate(illuminateParam, illuminateResult);
    if (radiance.AlmostZero())
    {
        return RayColor::Zero();
    }

    RT_ASSERT(radiance.IsValid());
    RT_ASSERT(IsValid(illuminateResult.directPdfW) && illuminateResult.directPdfW >= 0.0f);
    RT_ASSERT(IsValid(illuminateResult.emissionPdfW) && illuminateResult.emissionPdfW >= 0.0f);
    RT_ASSERT(IsValid(illuminateResult.distance) && illuminateResult.distance >= 0.0f);
    RT_ASSERT(IsValid(illuminateResult.cosAtLight) && illuminateResult.cosAtLight >= 0.0f);

    // calculate BSDF contribution
    float bsdfPdfW, bsdfRevPdfW;
    const RayColor bsdfFactor = shadingData.material->Evaluate(ctx.wavelength, shadingData, -illuminateResult.directionToLight, &bsdfPdfW, &bsdfRevPdfW);
    RT_ASSERT(bsdfFactor.IsValid());

    if (bsdfFactor.AlmostZero())
    {
        return RayColor::Zero();
    }

    RT_ASSERT(bsdfPdfW > 0.0f && IsValid(bsdfPdfW));

    // cast shadow ray
    {
        HitPoint hitPoint;
        hitPoint.distance = illuminateResult.distance * 0.999f;

        Ray shadowRay(shadingData.frame.GetTranslation(), illuminateResult.directionToLight);
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
    const bool isDeltaLight = light.GetFlags() & ILight::Flag_IsDelta;
    const float continuationProbability = 1.0f;
    bsdfPdfW *= isDeltaLight ? 0.0f : continuationProbability;
    bsdfRevPdfW *= continuationProbability;

    const float cosToLight = Vector4::Dot3(shadingData.frame[2], illuminateResult.directionToLight);
    if (cosToLight <= FLT_EPSILON)
    {
        return RayColor::Zero();
    }

    const float wLight = Mis(bsdfPdfW / (lightPickProbability * illuminateResult.directPdfW));
    const float wCamera = Mis(illuminateResult.emissionPdfW * cosToLight / (illuminateResult.directPdfW * illuminateResult.cosAtLight)) * (mMisVertexMergingWeightFactorVC + pathState.dVCM + pathState.dVC * Mis(bsdfRevPdfW));
    const float misWeight = 1.0f / (wLight + 1.0f + wCamera);
    RT_ASSERT(misWeight >= 0.0f);

    return (radiance * bsdfFactor) * (misWeight / (lightPickProbability * illuminateResult.directPdfW));
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

const RayColor VertexConnectionAndMerging::EvaluateGlobalLights(Uint32 iteration, const PathState& pathState, RenderingContext& ctx) const
{
    RayColor result = RayColor::Zero();

    for (const ILight* globalLight : mScene.GetGlobalLights())
    {
        result += EvaluateLight(iteration, *globalLight, FLT_MAX, pathState, ctx);
    }

    return result;
}

const RayColor VertexConnectionAndMerging::ConnectVertices(PathState& cameraPathState, const ShadingData& shadingData, const LightVertex& lightVertex, RenderingContext& ctx) const
{
    // compute connection direction (from camera vertex to light vertex)
    Vector4 lightDir = lightVertex.shadingData.frame.GetTranslation() - shadingData.frame.GetTranslation();
    const float distanceSqr = lightDir.SqrLength3();
    const float distance = sqrtf(distanceSqr);
    lightDir /= distance;

    const float cosCameraVertex = shadingData.CosTheta(lightDir);
    const float cosLightVertex = lightVertex.shadingData.CosTheta(-lightDir);

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
    const RayColor lightFactor = lightVertex.shadingData.material->Evaluate(ctx.wavelength, lightVertex.shadingData, lightDir, &lightBsdfPdfW, &lightBsdfRevPdfW);
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

    const float wLight = Mis(cameraBsdfPdfA) * (mMisVertexMergingWeightFactorVC + lightVertex.dVCM + lightVertex.dVC * Mis(lightBsdfRevPdfW));
    RT_ASSERT(IsValid(wLight) && wLight >= 0.0f);

    const float wCamera = Mis(lightBsdfPdfA) * (mMisVertexMergingWeightFactorVC + cameraPathState.dVCM + cameraPathState.dVC * Mis(cameraBsdfRevPdfW));
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
            , mContext(ctx)
            , mContribution(RayColor::Zero())
        {}

        RT_FORCE_INLINE const RayColor& GetContribution() const { return mContribution; }

        RT_FORCE_NOINLINE void operator()(Uint32 photonIndex)
        {
            const Photon& photon = mRenderer.mPhotons[photonIndex];

            // TODO russian roulette
            //if (photon.pathLength + mCameraPathState.length > mRenderer.mMaxPathLength)
            //{
            //    return;
            //}

            // decompress light incoming direction in world coordinates
            const Vector4 lightDirection{ photon.direction.ToVector() };
            RT_ASSERT(lightDirection.IsValid());

            // decompress photon throughput
            const RayColor throughput{ photon.throughput.ToVector() };
            RT_ASSERT(throughput.IsValid());

            const float cosToLight = mShadingData.CosTheta(lightDirection);
            if (cosToLight < FLT_EPSILON)
            {
                return;
            }

            float cameraBsdfDirPdfW, cameraBsdfRevPdfW;
            const RayColor cameraBsdfFactor = mShadingData.material->Evaluate(mContext.wavelength, mShadingData, -lightDirection, &cameraBsdfDirPdfW, &cameraBsdfRevPdfW);
            RT_ASSERT(cameraBsdfFactor.IsValid());
            
            if (cameraBsdfFactor.AlmostZero())
            {
                return;
            }

            // TODO russian roulette
            //cameraBsdfDirPdfW *= mCameraBsdf.ContinuationProb();
            //cameraBsdfRevPdfW *= aLightVertex.mBSDF.ContinuationProb();

            // Partial light sub-path MIS weight [tech. rep. (38)]
            const float wLight = photon.dVCM * mRenderer.mMisVertexConnectionWeightFactorVM + photon.dVM * Mis(cameraBsdfDirPdfW);
            const float wCamera = mCameraPathState.dVCM * mRenderer.mMisVertexConnectionWeightFactorVM + mCameraPathState.dVM * Mis(cameraBsdfRevPdfW);
            const float misWeight = 1.0f / (wLight + 1.0f + wCamera);
            const float weight = misWeight / cosToLight;
            RT_ASSERT(IsValid(weight));
            RT_ASSERT(weight > 0.0f);

            mContribution.MulAndAccumulate(cameraBsdfFactor * throughput, weight);
        }

    private:
        const VertexConnectionAndMerging& mRenderer;
        const ShadingData& mShadingData;
        const PathState& mCameraPathState;
        RenderingContext& mContext;
        RayColor mContribution;
    };

    const Vector4& cameraVertexPos = shadingData.frame.GetTranslation();

    RangeQuery query(*this, cameraPathState, shadingData, ctx);
#ifdef RT_VCM_USE_KD_TREE
    mKdTree.Find(cameraVertexPos, mMergingRadiusVM, mPhotons, query);
#else
    mHashGrid.Process(cameraVertexPos, mPhotons, query);
#endif // RT_VCM_USE_KD_TREE

    return query.GetContribution();
}

void VertexConnectionAndMerging::ConnectToCamera(const Camera& camera, Film& film, const LightVertex& lightVertex, RenderingContext& ctx) const
{
    const Vector4 cameraPos = camera.GetTransform().GetTranslation();
    const Vector4 samplePos = lightVertex.shadingData.frame.GetTranslation();

    Vector4 dirToCamera = cameraPos - samplePos;

    const float cameraDistanceSqr = dirToCamera.SqrLength3();
    const float cameraDistance = sqrtf(cameraDistanceSqr);

    dirToCamera /= cameraDistance;

    // calculate BSDF contribution
    float bsdfPdfW, bsdfRevPdfW;
    const RayColor cameraFactor = lightVertex.shadingData.material->Evaluate(ctx.wavelength, lightVertex.shadingData, -dirToCamera, &bsdfPdfW, &bsdfRevPdfW);
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

    const float cosToCamera = Vector4::Dot3(dirToCamera, lightVertex.shadingData.frame[2]);
    if (cosToCamera <= FLT_EPSILON)
    {
        return;
    }

    const float cameraPdfW = camera.PdfW(-dirToCamera);
    const float cameraPdfA = cameraPdfW * cosToCamera / cameraDistanceSqr;

    // compute MIS weight
    const float wLight = Mis(cameraPdfA) * (mMisVertexMergingWeightFactorVC + lightVertex.dVCM + lightVertex.dVC * Mis(bsdfRevPdfW));
    const float misWeight = 1.0f / (wLight + 1.0f);
    RT_ASSERT(misWeight >= 0.0f);

    RayColor contribution = (cameraFactor * lightVertex.throughput) * (misWeight * cameraPdfA / (cosToCamera));
    contribution *= RayColor::Resolve(ctx.wavelength, Spectrum(mCameraConnectingWeight));

    const Vector4 value = contribution.ConvertToTristimulus(ctx.wavelength);
    film.AccumulateColor(filmPos, value, ctx.randomGenerator);
}

} // namespace rt
