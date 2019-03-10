#include "PCH.h"
#include "Viewport.h"
#include "Film.h"
#include "Renderer.h"
#include "RendererContext.h"
#include "Utils/Logger.h"
#include "Scene/Camera.h"
#include "Color/LdrColor.h"
#include "Color/ColorHelpers.h"
#include "Math/SamplingHelpers.h"

namespace rt {

using namespace math;

static const Uint32 MAX_IMAGE_SZIE = 1 << 16;

Viewport::Viewport()
{
    InitThreadData();
}

Viewport::~Viewport() = default;

void Viewport::InitThreadData()
{
    const size_t numThreads = mThreadPool.GetNumThreads();

    mThreadData.resize(numThreads);

    mSamplers.clear();
    mSamplers.reserve(numThreads);

    for (size_t i = 0; i < numThreads; ++i)
    {
        RenderingContext& ctx = mThreadData[i];
        ctx.randomGenerator.Reset();

        if (mRenderer)
        {
            ctx.rendererContext = mRenderer->CreateContext();
        }

        mSamplers.emplace_back(ctx.randomGenerator);
    }
}

bool Viewport::Resize(Uint32 width, Uint32 height)
{
    if (width > MAX_IMAGE_SZIE || height > MAX_IMAGE_SZIE || width == 0 || height == 0)
    {
        RT_LOG_ERROR("Invalid viewport size");
        return false;
    }

    if (width == GetWidth() && height == GetHeight())
        return true;

    if (!mSum.Init(width, height, Bitmap::Format::R32G32B32_Float))
        return false;

    if (!mSecondarySum.Init(width, height, Bitmap::Format::R32G32B32_Float))
        return false;

    if (!mFrontBuffer.Init(width, height, Bitmap::Format::B8G8R8A8_Uint))
        return false;

    mPassesPerPixel.resize(width * height);

    mPixelSalt.resize(width * height);
    for (Uint32 i = 0; i < width * height; ++i)
    {
        mPixelSalt[i] = mRandomGenerator.GetVector4().ToFloat2();
    }

    Reset();

    return true;
}

void Viewport::SetPixelBreakpoint(Uint32 x, Uint32 y)
{
    mPendingPixelBreakpoint.x = x;
    mPendingPixelBreakpoint.y = y;
}

void Viewport::Reset()
{
    mPostprocessParams.fullUpdateRequired = true;

    mProgress = RenderingProgress();

    mHaltonSequence.Initialize(mParams.sampleDimensions);

    mSum.Clear();
    mSecondarySum.Clear();

    memset(mPassesPerPixel.data(), 0, sizeof(Uint32) * GetWidth() * GetHeight());

    BuildInitialBlocksList();
}

bool Viewport::SetRenderer(const RendererPtr& renderer)
{
    mRenderer = renderer;

    InitThreadData();

    return true;
}

bool Viewport::SetRenderingParams(const RenderingParams& params)
{
    RT_ASSERT(params.maxRayDepth < 255u);
    RT_ASSERT(params.antiAliasingSpread >= 0.0f);
    RT_ASSERT(params.motionBlurStrength >= 0.0f && params.motionBlurStrength <= 1.0f);

    if (mParams.numThreads != params.numThreads)
    {
        mThreadPool.SetNumThreads(params.numThreads);
        InitThreadData();
    }

    mParams = params;

    return true;
}

bool Viewport::SetPostprocessParams(const PostprocessParams& params)
{
    if (mPostprocessParams.params != params)
    {
        mPostprocessParams.params = params;
        mPostprocessParams.fullUpdateRequired = true;
    }

    // TODO validation

    return true;
}

void Viewport::ComputeError()
{
    const Block fullImageBlock(0, GetWidth(), 0, GetHeight());
    mProgress.averageError = ComputeBlockError(fullImageBlock);
}

bool Viewport::Render(const Camera& camera)
{
    RT_ASSERT(GetFlushDenormalsToZero(), "Flushing denormal float to zero is disabled");

    const Uint32 width = GetWidth();
    const Uint32 height = GetHeight();
    if (width == 0 || height == 0)
    {
        return false;
    }

    if (!mRenderer)
    {
        RT_LOG_ERROR("Viewport: Missing renderer");
        return false;
    }

    mHaltonSequence.NextSample();
    std::vector<float> seed(mHaltonSequence.GetNumDimensions());
    for (size_t i = 0; i < mHaltonSequence.GetNumDimensions(); ++i)
    {
        seed[i] = (float)(mHaltonSequence.GetValue(i));
    }

    for (size_t i = 0; i < mThreadData.size(); ++i)
    {
        RenderingContext& ctx = mThreadData[i];
        ctx.counters.Reset();
        ctx.params = &mParams;
        ctx.camera = &camera;
        ctx.pixelBreakpoint = mPendingPixelBreakpoint;
        ctx.sampler = &mSamplers[i];
        ctx.sampler->ResetFrame(seed);

        mRenderer->PreRender(ctx);
    }

    mPendingPixelBreakpoint.x = UINT32_MAX;
    mPendingPixelBreakpoint.y = UINT32_MAX;

    if (mRenderingTiles.empty() || mProgress.passesFinished == 0)
    {
        GenerateRenderingTiles();
    }

    // render
    {
        // randomize pixel offset
        const Vector4 u = SamplingHelpers::GetFloatNormal2(mRandomGenerator.GetFloat2());

        const TileRenderingContext tileContext =
        {
            *mRenderer,
            camera,
            u * mThreadData[0].params->antiAliasingSpread
        };

        {
            const Film film(mSum, mProgress.passesFinished % 2 == 0 ? &mSecondarySum : nullptr);
            mRenderer->PreRender(film);
        }

        const auto preRenderCallback = [&](Uint32 id, Uint32 threadID)
        {
            PreRenderTile(tileContext, mThreadData[threadID], mRenderingTiles[id]);
        };

        const auto renderCallback = [&](Uint32 id, Uint32 threadID)
        {
            RenderTile(tileContext, mThreadData[threadID], mRenderingTiles[id]);
        };

        mThreadPool.RunParallelTask(preRenderCallback, (Uint32)(mRenderingTiles.size()));

        for (RenderingContext& ctx : mThreadData)
        {
            mRenderer->PreRenderGlobal(ctx);
        }

        mRenderer->PreRenderGlobal();

        mThreadPool.RunParallelTask(renderCallback, (Uint32)(mRenderingTiles.size()));
    }

    PerformPostProcess();

    mProgress.passesFinished++;

    if ((mProgress.passesFinished > 0) && (mProgress.passesFinished % 2 == 0))
    {
        if (mParams.adaptiveSettings.enable)
        {
            UpdateBlocksList();
            GenerateRenderingTiles();
        }
        else
        {
            ComputeError();
        }
    }

    // accumulate counters
    mCounters.Reset();
    for (const RenderingContext& ctx : mThreadData)
    {
        mCounters.Append(ctx.counters);
    }

    return true;
}

void Viewport::PreRenderTile(const TileRenderingContext& tileCtx, RenderingContext& ctx, const Block& tile)
{
    RT_ASSERT(tile.minX < tile.maxX);
    RT_ASSERT(tile.minY < tile.maxY);
    RT_ASSERT(tile.maxX <= GetWidth());
    RT_ASSERT(tile.maxY <= GetHeight());

    Film film(mSum, mProgress.passesFinished % 2 == 0 ? &mSecondarySum : nullptr);

    if (ctx.params->traversalMode == TraversalMode::Single)
    {
        for (Uint32 y = tile.minY; y < tile.maxY; ++y)
        {
            for (Uint32 x = tile.minX; x < tile.maxX; ++x)
            {
                // TODO this won't work with bidirectional path tracing
                ctx.time = ctx.randomGenerator.GetFloat() * ctx.params->motionBlurStrength;
                ctx.wavelength.Randomize(ctx.randomGenerator);

                const Uint32 pixelIndex = y * GetHeight() + x;
                const IRenderer::RenderParam renderParam = { pixelIndex, tileCtx.camera, film };

                tileCtx.renderer.PreRenderPixel(renderParam, ctx);
            }
        }
    }
}

void Viewport::RenderTile(const TileRenderingContext& tileContext, RenderingContext& ctx, const Block& tile)
{
    RT_ASSERT(tile.minX < tile.maxX);
    RT_ASSERT(tile.minY < tile.maxY);
    RT_ASSERT(tile.maxX <= GetWidth());
    RT_ASSERT(tile.maxY <= GetHeight());

    const Vector4 filmSize = Vector4::FromIntegers(GetWidth(), GetHeight(), 1, 1);
    const Vector4 invSize = VECTOR_ONE2 / filmSize;
    const Uint32 tileSize = ctx.params->tileSize;

    Film film(mSum, mProgress.passesFinished % 2 == 0 ? &mSecondarySum : nullptr);

    if (ctx.params->traversalMode == TraversalMode::Single)
    {
        for (Uint32 y = tile.minY; y < tile.maxY; ++y)
        {
            const Uint32 realY = GetHeight() - 1u - y;

            for (Uint32 x = tile.minX; x < tile.maxX; ++x)
            {
                if (ctx.pixelBreakpoint.x == x && ctx.pixelBreakpoint.y == y)
                {
                    RT_BREAK();
                }

                const Uint32 pixelIndex = y * GetHeight() + x;
                const Vector4 coords = (Vector4::FromIntegers(x, realY, 0, 0) + tileContext.sampleOffset) * invSize;

                ctx.sampler->ResetPixel(pixelIndex);
                ctx.time = ctx.randomGenerator.GetFloat() * ctx.params->motionBlurStrength;
                ctx.wavelength.Randomize(ctx.randomGenerator);

                // generate primary ray
                const Ray ray = tileContext.camera.GenerateRay(coords, ctx);

                const IRenderer::RenderParam renderParam = { pixelIndex, tileContext.camera, film };
                const RayColor color = tileContext.renderer.RenderPixel(ray, renderParam, ctx);

                const Vector4 sampleColor = color.ConvertToTristimulus(ctx.wavelength);
                RT_ASSERT(sampleColor.IsValid());

#ifndef RT_ENABLE_SPECTRAL_RENDERING
                // exception: in spectral rendering these values can get below zero due to RGB->Spectrum conversion
                RT_ASSERT((sampleColor >= Vector4::Zero()).All());
#endif // RT_ENABLE_SPECTRAL_RENDERING

                film.AccumulateColor(x, y, sampleColor);
            }
        }
    }
    else if (ctx.params->traversalMode == TraversalMode::Packet)
    {
        ctx.time = ctx.randomGenerator.GetFloat() * ctx.params->motionBlurStrength;
        ctx.wavelength.Randomize(ctx.randomGenerator);

        RayPacket& primaryPacket = ctx.rayPacket;
        primaryPacket.Clear();

        // TODO multisampling
        // TODO handle case where tile size does not fit ray group size
        RT_ASSERT((tile.maxY - tile.minY) % 2 == 0);
        RT_ASSERT((tile.maxX - tile.minX) % 4 == 0);
        /*
        for (Uint32 y = tile.minY; y < tile.maxY; ++y)
        {
            const Uint32 realY = GetHeight() - 1u - y;

            for (Uint32 x = tile.minX; x < tile.maxX; ++x)
            {
                const Vector4 coords = (Vector4::FromIntegers(x, realY, 0, 0) + tileContext.sampleOffset) * invSize;

                for (Uint32 s = 0; s < samplesPerPixel; ++s)
                {
                    // generate primary ray
                    const Ray ray = tileContext.camera.GenerateRay(coords, ctx);

                    const ImageLocationInfo location = { (Uint16)x, (Uint16)y };
                    primaryPacket.PushRay(ray, Vector4(sampleScale), location);
                }
            }
        }
        */

        constexpr Uint32 rayGroupSizeX = 4;
        constexpr Uint32 rayGroupSizeY = 2;

        for (Uint32 y = tile.minY; y < tile.maxY; y += rayGroupSizeY)
        {
            const Uint32 realY = GetHeight() - 1u - y;

            for (Uint32 x = tile.minX; x < tile.maxX; x += rayGroupSizeX)
            {
                // generate ray group with following layout:
                //  0 1 2 3
                //  4 5 6 7
                Vector2x8 coords{ Vector8::FromInteger(x), Vector8::FromInteger(realY) };
                coords.x += Vector8(0.0f, 1.0f, 2.0f, 3.0f, 0.0f, 1.0f, 2.0f, 3.0f);
                coords.y -= Vector8(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
                coords.x += Vector8(tileContext.sampleOffset.x);
                coords.y += Vector8(tileContext.sampleOffset.y);
                coords.x *= invSize.x;
                coords.y *= invSize.y;

                const ImageLocationInfo locations[] =
                {
                    { x + 0, y + 0 }, { x + 1, y + 0 }, { x + 2, y + 0 }, { x + 3, y + 0 },
                    { x + 0, y + 1 }, { x + 1, y + 1 }, { x + 2, y + 1 }, { x + 3, y + 1 },
                };

                const Ray_Simd8 simdRay = tileContext.camera.GenerateRay_Simd8(coords, ctx);
                primaryPacket.PushRays(simdRay, Vector3x8(1.0f), locations);
            }
        }

        ctx.localCounters.Reset();
        tileContext.renderer.Raytrace_Packet(primaryPacket, tileContext.camera, film, ctx);
        ctx.counters.Append(ctx.localCounters);
    }

    ctx.counters.numPrimaryRays += tileSize * tileSize;
}

void Viewport::PerformPostProcess()
{
    mPostprocessParams.colorScale = mPostprocessParams.params.colorFilter * exp2f(mPostprocessParams.params.exposure);

    if (mPostprocessParams.fullUpdateRequired)
    {
        // post processing params has changed, perfrom full image update

        const Uint32 numTiles = mThreadPool.GetNumThreads();

        const auto taskCallback = [this, numTiles](Uint32 id, Uint32 threadID)
        {
            Block block;
            block.minY = GetHeight() * id / numTiles;
            block.maxY = GetHeight() * (id + 1) / numTiles;
            block.minX = 0;
            block.maxX = GetWidth();

            PostProcessTile(block, threadID);
        };

        mThreadPool.RunParallelTask(taskCallback, numTiles);

        mPostprocessParams.fullUpdateRequired = false;
    }
    else
    {
        // apply post proces on active blocks only

        if (!mRenderingTiles.empty())
        {
            const auto taskCallback = [this](Uint32 id, Uint32 threadID)
            {
                PostProcessTile(mRenderingTiles[id], threadID);
            };

            mThreadPool.RunParallelTask(taskCallback, (Uint32)(mRenderingTiles.size()));
        }
    }

    // flush non-temporal stores
    _mm_mfence();
}

void Viewport::PostProcessTile(const Block& block, Uint32 threadID)
{
    Random& randomGenerator = mThreadData[threadID].randomGenerator;

    const Float3* __restrict sumPixels = mSum.GetDataAs<Float3>();
    Uint8* __restrict frontBufferPixels = mFrontBuffer.GetDataAs<Uint8>();

    for (Uint32 y = block.minY; y < block.maxY; ++y)
    {
        for (Uint32 x = block.minX; x < block.maxX; ++x)
        {
            const size_t pixelIndex = GetWidth() * y + x;

#ifdef RT_ENABLE_SPECTRAL_RENDERING
            const Vector4 xyzColor = Vector4(sumPixels[pixelIndex]);
            const Vector4 rgbColor = Vector4::Max(Vector4::Zero(), ConvertXYZtoRGB(xyzColor));
#else
            const Vector4 rgbColor = Vector4(sumPixels[pixelIndex]);
#endif

            // TODO
            const float pixelScaling = 1.0f / (1 + mProgress.passesFinished);
            //const float pixelScaling = 1.0f / static_cast<float>(mPassesPerPixel[pixelIndex]);

            const Vector4 toneMapped = ToneMap(rgbColor * mPostprocessParams.colorScale * pixelScaling);
            const Vector4 dithered = Vector4::MulAndAdd(randomGenerator.GetVector4Bipolar(), mPostprocessParams.params.ditheringStrength, toneMapped);

            dithered.StoreBGR_NonTemporal(frontBufferPixels + 4 * pixelIndex);
        }
    }
}

float Viewport::ComputeBlockError(const Block& block) const
{
    if (mProgress.passesFinished == 0)
    {
        return std::numeric_limits<float>::max();
    }

    RT_ASSERT(mProgress.passesFinished % 2 == 0, "This funcion can be only called after even number of passes");

    const Float3* sumPixels = mSum.GetDataAs<Float3>();
    const Float3* secondarySumPixels = mSecondarySum.GetDataAs<Float3>();

    const float imageScalingFactor = 1.0f / (float)mProgress.passesFinished;

    float totalError = 0.0f;
    for (Uint32 y = block.minY; y < block.maxY; ++y)
    {
        float rowError = 0.0f;
        for (Uint32 x = block.minX; x < block.maxX; ++x)
        {
            const size_t pixelIndex = GetWidth() * y + x;
            const Vector4 a = imageScalingFactor * Vector4(sumPixels[pixelIndex]);
            const Vector4 b = (2.0f * imageScalingFactor) * Vector4(secondarySumPixels[pixelIndex]);
            const Vector4 diff = Vector4::Abs(a - b);
            const float error = (diff.x + 2.0f * diff.y + diff.z) / Sqrt(RT_EPSILON + a.x + 2.0f * a.y + a.z);
            rowError += error;
        }
        totalError += rowError;
    }

    const Uint32 totalArea = GetWidth() * GetHeight();
    const Uint32 blockArea = block.Width() * block.Height();
    return totalError * Sqrt((float)blockArea / (float)totalArea) / (float)blockArea;
}

void Viewport::GenerateRenderingTiles()
{
    mRenderingTiles.clear();
    mRenderingTiles.reserve(mBlocks.size());

    const Uint32 tileSize = mParams.tileSize;

    for (const Block& block : mBlocks)
    {
        const Uint32 rows = 1 + (block.Height() - 1) / tileSize;
        const Uint32 columns = 1 + (block.Width() - 1) / tileSize;

        Block tile;

        for (Uint32 j = 0; j < rows; ++j)
        {
            tile.minY = block.minY + j * tileSize;
            tile.maxY = Min(block.maxY, block.minY + j * tileSize + tileSize);
            RT_ASSERT(tile.maxY > tile.minY);

            for (Uint32 i = 0; i < columns; ++i)
            {
                tile.minX = block.minX + i * tileSize;
                tile.maxX = Min(block.maxX, block.minX + i * tileSize + tileSize);
                RT_ASSERT(tile.maxX > tile.minX);

                mRenderingTiles.push_back(tile);
            }
        }
    }
}

void Viewport::BuildInitialBlocksList()
{
    mBlocks.clear();

    const Uint32 blockSize = mParams.adaptiveSettings.maxBlockSize;
    const Uint32 rows = 1 + (GetHeight() - 1) / blockSize;
    const Uint32 columns = 1 + (GetWidth() - 1) / blockSize;

    for (Uint32 j = 0; j < rows; ++j)
    {
        Block block;

        block.minY = j * blockSize;
        block.maxY = Min(GetHeight(), (j + 1) * blockSize);
        RT_ASSERT(block.maxY > block.minY);

        for (Uint32 i = 0; i < columns; ++i)
        {
            block.minX = i * blockSize;
            block.maxX = Min(GetWidth(), (i + 1) * blockSize);
            RT_ASSERT(block.maxX > block.minX);

            mBlocks.push_back(block);
        }
    }

    mProgress.activeBlocks = (Uint32)mBlocks.size();
}

void Viewport::UpdateBlocksList()
{
    std::vector<Block> newBlocks;

    const AdaptiveRenderingSettings& settings = mParams.adaptiveSettings;

    if (mProgress.passesFinished < settings.numInitialPasses)
    {
        return;
    }

    for (size_t i = 0; i < mBlocks.size(); ++i)
    {
        const Block block = mBlocks[i];
        const float blockError = ComputeBlockError(block);

        if (blockError < settings.convergenceTreshold)
        {
            // block is fully converged - remove it
            mBlocks[i] = mBlocks.back();
            mBlocks.pop_back();
            continue;
        }

        if ((blockError < settings.subdivisionTreshold) &&
            (block.Width() > settings.minBlockSize || block.Height() > settings.minBlockSize))
        {
            // block is somewhat converged - split it into two parts

            mBlocks[i] = mBlocks.back();
            mBlocks.pop_back();

            Block childA, childB;

            // TODO split the block so the error is equal on both sides

            if (block.Width() > block.Height())
            {
                const Uint32 halfPoint = (block.minX + block.maxX) / 2u;

                childA.minX = block.minX;
                childA.maxX = halfPoint;
                childA.minY = block.minY;
                childA.maxY = block.maxY;

                childB.minX = halfPoint;
                childB.maxX = block.maxX;
                childB.minY = block.minY;
                childB.maxY = block.maxY;
            }
            else
            {
                const Uint32 halfPoint = (block.minY + block.maxY) / 2u;

                childA.minX = block.minX;
                childA.maxX = block.maxX;
                childA.minY = block.minY;
                childA.maxY = halfPoint;

                childB.minX = block.minX;
                childB.maxX = block.maxX;
                childB.minY = halfPoint;
                childB.maxY = block.maxY;
            }

            newBlocks.push_back(childA);
            newBlocks.push_back(childB);
        }
    }

    // add splitted blocks to the list
    mBlocks.reserve(mBlocks.size() + newBlocks.size());
    for (const Block& block : newBlocks)
    {
        mBlocks.push_back(block);
    }

    // calculate number of active pixels
    {
        mProgress.activePixels = 0;
        for (const Block& block : mBlocks)
        {
            mProgress.activePixels += block.Width() * block.Height();
        }
    }

    mProgress.converged = 1.0f - (float)mProgress.activePixels / (float)(GetWidth() * GetHeight());
    mProgress.activeBlocks = (Uint32)mBlocks.size();
}

void Viewport::VisualizeActiveBlocks(Bitmap& bitmap) const
{
    LdrColor* frontBufferPixels = bitmap.GetDataAs<LdrColor>();

    const LdrColor color(255, 0, 0);
    const Uint8 alpha = 64;

    for (const Block& block : mBlocks)
    {
        for (Uint32 y = block.minY; y < block.maxY; ++y)
        {
            for (Uint32 x = block.minX; x < block.maxX; ++x)
            {
                const size_t pixelIndex = GetWidth() * y + x;
                frontBufferPixels[pixelIndex] = Lerp(frontBufferPixels[pixelIndex], color, alpha);
            }
        }

        for (Uint32 y = block.minY; y < block.maxY; ++y)
        {
            const size_t pixelIndex = GetWidth() * y + block.minX;
            frontBufferPixels[pixelIndex] = color;
        }

        for (Uint32 y = block.minY; y < block.maxY; ++y)
        {
            const size_t pixelIndex = GetWidth() * y + (block.maxX - 1);
            frontBufferPixels[pixelIndex] = color;
        }

        for (Uint32 x = block.minX; x < block.maxX; ++x)
        {
            const size_t pixelIndex = GetWidth() * block.minY + x;
            frontBufferPixels[pixelIndex] = color;
        }

        for (Uint32 x = block.minX; x < block.maxX; ++x)
        {
            const size_t pixelIndex = GetWidth() * (block.maxY - 1) + x;
            frontBufferPixels[pixelIndex] = color;
        }
    }
}

} // namespace rt
