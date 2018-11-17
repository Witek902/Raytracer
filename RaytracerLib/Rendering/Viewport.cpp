#include "PCH.h"
#include "Viewport.h"
#include "Renderer.h"
#include "Utils/Logger.h"
#include "Scene/Camera.h"
#include "Color/Color.h"
#include "Color/ColorHelpers.h"
#include "Color/LdrColor.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;

static const Uint32 MAX_IMAGE_SZIE = 1 << 16;

Viewport::Viewport()
{
    InitThreadData();
}

void Viewport::InitThreadData()
{
    const size_t numThreads = mThreadPool.GetNumThreads();

    mThreadData.resize(numThreads);
    for (size_t i = 0; i < numThreads; ++i)
    {
        mThreadData[i].randomGenerator.Reset();
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

    Reset();

    return true;
}

void Viewport::Reset()
{
    mPostprocessParams.fullUpdateRequired = true;

    mProgress = RenderingProgress();

    mSum.Clear();
    mSecondarySum.Clear();

    memset(mPassesPerPixel.data(), 0, sizeof(Uint32) * GetWidth() * GetHeight());

    BuildInitialBlocksList();
}

bool Viewport::SetRenderingParams(const RenderingParams& params)
{
    mParams = params;

    // TODO validation

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

bool Viewport::Render(const IRenderer& renderer, const Camera& camera)
{
    const Uint32 width = GetWidth();
    const Uint32 height = GetHeight();
    if (width == 0 || height == 0)
    {
        return false;
    }

    for (RenderingContext& ctx : mThreadData)
    {
        ctx.counters.Reset();
        ctx.params = &mParams;
    }

    if (mRenderingTiles.empty() || mProgress.passesFinished == 0)
    {
        GenerateRenderingTiles();
    }

    // render
    {
        // randomize pixel offset
        const Vector4 u = mThreadData[0].randomGenerator.GetFloatNormal2();

        if (!mRenderingTiles.empty())
        {
            const TileRenderingContext tileContext =
            {
                renderer,
                camera,
                u * mThreadData[0].params->antiAliasingSpread
            };

            const auto taskCallback = [&](Uint32 id, Uint32 threadID)
            {
                RenderTile(tileContext, mThreadData[threadID], mRenderingTiles[id]);
            };

            mThreadPool.RunParallelTask(taskCallback, (Uint32)(mRenderingTiles.size()));

            // flush non-temporal stores
            _mm_mfence();
        }
    }

    PerformPostProcess();

    mProgress.passesFinished++;

    if (mParams.adaptiveSettings.enable && (mProgress.passesFinished > 0) && (mProgress.passesFinished % 2 == 0))
    {
        UpdateBlocksList();
        GenerateRenderingTiles();
    }

    // accumulate counters
    mCounters.Reset();
    for (const RenderingContext& ctx : mThreadData)
    {
        mCounters.Append(ctx.counters);
    }

    return true;
}

RT_FORCE_INLINE void Store_NonTemporal(Uint32* target, const Uint32 value)
{
    _mm_stream_si32(reinterpret_cast<int*>(target), value);
}

void Viewport::RenderTile(const TileRenderingContext& tileContext, RenderingContext& renderingContext, const Block& tile)
{
    RT_ASSERT(tile.minX < tile.maxX);
    RT_ASSERT(tile.minY < tile.maxY);
    RT_ASSERT(tile.maxX <= GetWidth());
    RT_ASSERT(tile.maxY <= GetHeight());

    const Vector4 invSize = VECTOR_ONE2 / Vector4::FromIntegers(GetWidth(), GetHeight(), 1, 1);
    const Uint32 tileSize = renderingContext.params->tileSize;
    const Uint32 samplesPerPixel = renderingContext.params->samplesPerPixel;
    const Float sampleScale = 1.0f / (Float)samplesPerPixel;

    const bool verticalFlip = true;

    const Uint32 currentPass = mProgress.passesFinished + 1;
    Float3* __restrict sumPixels = mSum.GetDataAs<Float3>();
    Float3* __restrict secondarySumPixels = mSecondarySum.GetDataAs<Float3>();

    if (renderingContext.params->traversalMode == TraversalMode::Single)
    {
        for (Uint32 y = tile.minY; y < tile.maxY; ++y)
        {
            for (Uint32 x = tile.minX; x < tile.maxX; ++x)
            {
                const Uint32 realY = verticalFlip ? (GetHeight() - 1u - y) : y;
                const Vector4 coords = (Vector4::FromIntegers(x, realY, 0, 0) + tileContext.sampleOffset) * invSize;

                Vector4 sampleColor;
                for (Uint32 s = 0; s < samplesPerPixel; ++s)
                {
                    renderingContext.time = renderingContext.randomGenerator.GetFloat() * renderingContext.params->motionBlurStrength;
                    renderingContext.wavelength.Randomize(renderingContext.randomGenerator);

                    // generate primary ray
                    const Ray ray = tileContext.camera.GenerateRay(coords, renderingContext);
                    const Color color = tileContext.renderer.TraceRay_Single(ray, renderingContext);
                    sampleColor += color.Resolve(renderingContext.wavelength);
                }

                RT_ASSERT(sampleColor.IsValid());

                sampleColor = Vector4::Max(sampleColor, Vector4()); // TODO fix this
                RT_ASSERT(sampleColor >= Vector4());

                // TODO get rid of this
                sampleColor *= sampleScale;

                const size_t pixelIndex = GetWidth() * y + x;
                sumPixels[pixelIndex] += sampleColor.ToFloat3();

                Store_NonTemporal(&mPassesPerPixel[pixelIndex], currentPass);

                if (mProgress.passesFinished % 2 == 0)
                {
                    secondarySumPixels[pixelIndex] += sampleColor.ToFloat3();
                }
            }
        }

        //for (Uint32 i = 0; i < tileSize * tileSize; ++i)
        //{
        //    // fill the tile using Morton Curve for better cache locality
        //    Uint32 localX, localY;
        //    DecodeMorton(i, localX, localY);
        //    const Uint32 x = x0 + localX;
        //    const Uint32 y = y0 + localY;
        //    if (x >= maxX || y >= maxY) continue;
        //
        //}
    }

    // TODO
    /*
    else if (context.params->traversalMode == TraversalMode::Simd)
    {
        for (Uint32 y = y0; y < maxY; ++y)
        {
            const Uint32 realY = verticalFlip ? (GetHeight() - 1u - y) : y;

            for (Uint32 x = x0; x < maxX; ++x)
            {
                // TODO multisampling

                // generate primary SIMD rays
                Vector2x8 coords(Vector4::FromIntegers(x, realY));
                coords.x += (context.randomGenerator.GetVector8() - VECTOR8_HALVES) * context.params->antiAliasingSpread;
                coords.y += (context.randomGenerator.GetVector8() - VECTOR8_HALVES) * context.params->antiAliasingSpread;
                coords.x *= invSize[0];
                coords.y *= invSize[1];

                Ray_Simd8 simdRay = camera.GenerateRay_Simd8(coords, context);

                Color colors[8];
                scene.TraceRay_Simd8(simdRay, context, colors);

                Color color;
                for (Uint16 i = 0; i < 8; ++i)
                {
                    color += colors[i];
                }
                color *= 1.0f / 8.0f;

                const Vector4 cieXYZ = color.Resolve(context.wavelength);
                sumPixels[renderTargetWidth * y + x] += cieXYZ;
            }
        }
    }
    else if (context.params->traversalMode == TraversalMode::Packet)
    {
        RayPacket& primaryPacket = context.rayPackets[0];
        primaryPacket.Clear();

        // prepare packet
        for (Uint32 y = y0; y < maxY; ++y)
        {
            const Uint32 realY = verticalFlip ? (GetHeight() - 1u - y) : y;

            for (Uint32 x = x0; x < maxX; ++x)
            {
                // TODO multisampling

                // generate primary SIMD rays
                Vector2x8 coords(Vector4::FromIntegers(x, realY));
                coords.x += (context.randomGenerator.GetVector8() - VECTOR8_HALVES) * context.params->antiAliasingSpread;
                coords.y += (context.randomGenerator.GetVector8() - VECTOR8_HALVES) * context.params->antiAliasingSpread;
                coords.x *= invSize[0];
                coords.y *= invSize[1];

                const ImageLocationInfo location = { (Uint16)x, (Uint16)y };
                Ray_Simd8 simdRay = camera.GenerateRay_Simd8(coords, context);
                const Vector3x8 weight = Vector3x8(1.0f / 8.0f);

                primaryPacket.PushRays(simdRay, weight, location);
            }
        }

        LocalCounters counters;
        HitPoint_Packet& hitPoints = context.hitPoints;
        context.localCounters.Reset();
        scene.Traverse_Packet({ primaryPacket, hitPoints, context });
        context.counters.Append(context.localCounters);
        scene.Shade_Packet(primaryPacket, hitPoints, context, mAccumulated);
    }
    */

    renderingContext.counters.numPrimaryRays += tileSize * tileSize * renderingContext.params->samplesPerPixel;
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
            const Vector4 xyzColor = sumPixels[pixelIndex];
            const Vector4 rgbColor = ConvertXYZtoRGB(xyzColor);
#else
            const Vector4 rgbColor = Vector4(sumPixels[pixelIndex]);
#endif

            const Float pixelScaling = 1.0f / static_cast<Float>(mPassesPerPixel[pixelIndex]);

            const Vector4 toneMapped = ToneMap(rgbColor * mPostprocessParams.colorScale * pixelScaling);
            const Vector4 dithered = Vector4::MulAndAdd(randomGenerator.GetVector4Bipolar(), mPostprocessParams.params.ditheringStrength, toneMapped);

            dithered.StoreBGR_NonTemporal(frontBufferPixels + 4 * pixelIndex);
        }
    }
}

Float Viewport::ComputeBlockError(const Block& block) const
{
    if (mProgress.passesFinished == 0)
    {
        return std::numeric_limits<Float>::max();
    }

    RT_ASSERT(mProgress.passesFinished % 2 == 0, "This funcion can be only called after even number of passes");

    const Float3* sumPixels = mSum.GetDataAs<Float3>();
    const Float3* secondarySumPixels = mSecondarySum.GetDataAs<Float3>();

    const Float imageScalingFactor = 1.0f / (Float)mProgress.passesFinished;

    Float totalError = 0.0f;
    for (Uint32 y = block.minY; y < block.maxY; ++y)
    {
        Float rowError = 0.0f;
        for (Uint32 x = block.minX; x < block.maxX; ++x)
        {
            const size_t pixelIndex = GetWidth() * y + x;
            const Vector4 a = imageScalingFactor * Vector4(sumPixels[pixelIndex]);
            const Vector4 b = (2.0f * imageScalingFactor) * Vector4(secondarySumPixels[pixelIndex]);
            const Vector4 diff = Vector4::Abs(a - b);
            const Float error = (diff.x + 2.0f * diff.y + diff.z) / Sqrt(RT_EPSILON + a.x + 2.0f * a.y + a.z);
            rowError += error;
        }
        totalError += rowError;
    }

    const Uint32 totalArea = GetWidth() * GetHeight();
    const Uint32 blockArea = block.Width() * block.Height();
    return totalError * Sqrt((Float)blockArea / (Float)totalArea) / (Float)blockArea;
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
        const Float blockError = ComputeBlockError(block);

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

    mProgress.converged = 1.0f - (Float)mProgress.activePixels / (Float)(GetWidth() * GetHeight());
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
