#include "PCH.h"
#include "Viewport.h"
#include "Renderer.h"
#include "Utils/Logger.h"
#include "Scene/Camera.h"
#include "Color/Color.h"
#include "Color/ColorHelpers.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;

static const Uint32 MAX_IMAGE_SZIE = 1 << 16;

Viewport::Viewport()
    : mNumSamplesRendered(0)
    , mAverageError(0.0f)
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

    Reset();

    return true;
}

bool Viewport::Render(const IRenderer& renderer, const Camera& camera, const RenderingParams& params)
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
        ctx.params = &params;
    }

    // render
    {
        // randomize pixel offset
        const Vector4 u = mThreadData[0].randomGenerator.GetFloatNormal2();

        const TileRenderingContext tileContext =
        {
            renderer,
            camera,
            u * mThreadData[0].params->antiAliasingSpread
        };

        const Uint32 tileSize = 1u << (Uint32)params.tileOrder;
        const Uint32 rows = 1 + (GetHeight() - 1) / tileSize;
        const Uint32 columns = 1 + (GetWidth() - 1) / tileSize;

        const auto taskCallback = [&](Uint32 tileX, Uint32 tileY, Uint32 threadID)
        {
            RenderTile(tileContext, mThreadData[threadID], tileSize * tileX, tileSize * tileY);
        };

        mThreadPool.RunParallelTask(taskCallback, rows, columns);
    }

    mNumSamplesRendered += params.samplesPerPixel;

    EstimateError();

    // accumulate counters
    mCounters.Reset();
    for (const RenderingContext& ctx : mThreadData)
    {
        mCounters.Append(ctx.counters);
    }

    return true;
}

bool Viewport::PostProcess(const PostprocessParams& params)
{
    const Uint32 numTiles = mThreadPool.GetNumThreads();

    const auto taskCallback = [&](Uint32, Uint32 tileY, Uint32 threadID)
    {
        const Uint32 ymin = GetHeight() * tileY / numTiles;
        const Uint32 ymax = GetHeight() * (tileY + 1) / numTiles;
        PostProcessTile(params, ymin, ymax, threadID);
    };

    mThreadPool.RunParallelTask(taskCallback, numTiles, 1);

    // flush non-temporal stores
    _mm_mfence();

    return true;
}

void Viewport::RenderTile(const TileRenderingContext& tileContext, RenderingContext& renderingContext, Uint32 x0, Uint32 y0)
{
    const Vector4 invSize = VECTOR_ONE2 / Vector4::FromIntegers(GetWidth(), GetHeight(), 1, 1);
    const Uint32 tileSize = 1u << renderingContext.params->tileOrder;
    const Uint32 samplesPerPixel = renderingContext.params->samplesPerPixel;
    const Uint32 maxX = Min(x0 + tileSize, GetWidth());
    const Uint32 maxY = Min(y0 + tileSize, GetHeight());

    const bool verticalFlip = true;

    Float3* __restrict sumPixels = mSum.GetDataAs<Float3>();
    Float3* __restrict secondarySumPixels = mSecondarySum.GetDataAs<Float3>();

    const Vector4 sampleOffset = renderingContext.randomGenerator.GetFloatNormal2();

    if (renderingContext.params->traversalMode == TraversalMode::Single)
    {
        for (Uint32 y = y0; y < maxY; ++y)
        {
            for (Uint32 x = x0; x < maxX; ++x)
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

                const size_t pixelIndex = GetWidth() * y + x;
                sumPixels[pixelIndex] += sampleColor.ToFloat3();

                if (mNumSamplesRendered % 2 == 0)
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

template<typename T>
RT_FORCE_INLINE static T ToneMap(T color)
{
    const T b = T(6.2f);
    const T c = T(1.7f);
    const T d = T(0.06f);

    // Jim Hejl and Richard Burgess-Dawson formula
    const T t0 = color * T::MulAndAdd(color, b, T(0.5f));
    const T t1 = T::MulAndAdd(color, b, c);
    const T t2 = T::MulAndAdd(color, t1, d);
    return t0 * T::FastReciprocal(t2);
}


void Viewport::PostProcessTile(const PostprocessParams& params, Uint32 ymin, Uint32 ymax, Uint32 threadID)
{
    const size_t width = GetWidth();
    const size_t minIndex = (size_t)ymin * width;
    const size_t maxIndex = (size_t)ymax * width;

    Random& randomGenerator = mThreadData[threadID].randomGenerator;

    const Float scalingFactor = exp2f(params.exposure) / (Float)mNumSamplesRendered;
    const Vector4 colorMultiplier = params.colorFilter * scalingFactor;

    const Float3* __restrict sumPixels = mSum.GetDataAs<Float3>();
    Uint8* __restrict frontBufferPixels = mFrontBuffer.GetDataAs<Uint8>();

    for (size_t i = minIndex; i < maxIndex; ++i)
    {
#ifdef RT_ENABLE_SPECTRAL_RENDERING
        const Vector4 xyzColor = sumPixels[i];
        const Vector4 rgbColor = ConvertXYZtoRGB(xyzColor);
#else
        const Vector4 rgbColor = Vector4(sumPixels[i]);
#endif

        const Vector4 toneMapped = ToneMap(rgbColor * colorMultiplier);
        const Vector4 dithered = Vector4::MulAndAdd(randomGenerator.GetVector4Bipolar(), params.ditheringStrength, toneMapped);

        dithered.StoreBGR_NonTemporal(frontBufferPixels + 4 * i);
    }
}

void Viewport::EstimateError()
{
    // we need even number of samples
    if (mNumSamplesRendered > 0 && (mNumSamplesRendered % 2 == 0))
    {
        const Float3* __restrict sumPixels = mSum.GetDataAs<Float3>();
        const Float3* __restrict secondarySumPixels = mSecondarySum.GetDataAs<Float3>();

        const size_t width = GetWidth();
        const size_t height = GetHeight();

        const float scalingFactor = 1.0f / static_cast<Float>(mNumSamplesRendered);

        Vector4 totalError = Vector4();

        size_t index = 0;
        for (size_t i = 0; i < height; ++i)
        {
            Vector4 rowError;
            for (size_t j = 0; j < width; ++j)
            {
                const Vector4 diff = (Vector4(sumPixels[index]) - 2.0f * Vector4(secondarySumPixels[index])) * scalingFactor;
                rowError += diff * diff;
                index++;
            }
            totalError += rowError;
        }

        mAverageError = (totalError.x + totalError.y + totalError.z) / static_cast<Float>(3 * width * height);
    }
}

void Viewport::Reset()
{
    mNumSamplesRendered = 0;
    mAverageError = 0.0f;

    mSum.Clear();
    mSecondarySum.Clear();
}

} // namespace rt
