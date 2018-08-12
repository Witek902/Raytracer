#include "PCH.h"
#include "Viewport.h"
#include "Utils/Logger.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"
#include "Color/Color.h"
#include "Traversal/TraversalContext.h"

namespace rt {

using namespace math;

static const Uint32 MAX_IMAGE_SZIE = 8192;

Viewport::Viewport()
    : mNumSamplesRendered(0)
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

    if (!mRenderTarget.Init(width, height, Bitmap::Format::R32G32B32A32_Float))
        return false;

    if (!mSum.Init(width, height, Bitmap::Format::R32G32B32A32_Float))
        return false;

    if (!mFrontBuffer.Init(width, height, Bitmap::Format::B8G8R8A8_Uint))
        return false;

    Reset();

    return true;
}

template<typename T>
RT_FORCE_INLINE static T ToneMap(T color)
{
    const T a = T(0.004f);
    const T b = T(6.2f);
    const T c = T(1.7f);
    const T d = T(0.06f);

    // Jim Hejl and Richard Burgess-Dawson formula
    const T t0 = color *  T::MulAndAdd(color, b, T(0.5f));
    const T t1 = T::MulAndAdd(color, b, c);
    const T t2 = T::MulAndAdd(color, t1, d);
    return t0 * T::FastReciprocal(t2);
}

bool Viewport::Render(const Scene* scene, const Camera& camera, const RenderingParams& params)
{
    if (!scene)
    {
        RT_LOG_ERROR("Invalid scene");
        return false;
    }

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
        const Uint32 tileSize = 1u << (Uint32)params.tileOrder;
        const Uint32 rows = 1 + (GetHeight() - 1) / tileSize;
        const Uint32 columns = 1 + (GetWidth() - 1) / tileSize;

        const auto taskCallback = [&](Uint32 tileX, Uint32 tileY, Uint32 threadID)
        {
            RenderTile(*scene, camera, mThreadData[threadID], tileSize * tileX, tileSize * tileY);
        };

        if (params.traversalMode == TraversalMode::Packet)
        {
            mRenderTarget.Zero();
        }

        mThreadPool.RunParallelTask(taskCallback, rows, columns);
    }

    // post process
    {
        mNumSamplesRendered += params.samplesPerPixel;

        const Uint32 numTiles = (Uint32)std::thread::hardware_concurrency();

        const auto taskCallback = [&](Uint32, Uint32 tileY, Uint32 threadID)
        {
            RenderingContext& context = mThreadData[threadID];
            context.params = &params;

            const Uint32 ymin = GetHeight() * tileY / numTiles;
            const Uint32 ymax = GetHeight() * (tileY + 1) / numTiles;
            PostProcess(context, ymin, ymax, threadID);
        };

        mThreadPool.RunParallelTask(taskCallback, numTiles, 1);
    }

    // accumulate counters
    mCounters.Reset();
    for (const RenderingContext& ctx : mThreadData)
    {
        mCounters.Append(ctx.counters);
    }

    return true;
}

// morton order -> 2D cooridnates
// basically deinterleaves bits
RT_FORCE_INLINE void DecodeMorton(const Uint32 order, Uint32& x, Uint32& y)
{
    x = _pext_u32(order, 0x55555555);
    y = _pext_u32(order, 0xAAAAAAAA);
}

void Viewport::RenderTile(const Scene& scene, const Camera& camera, RenderingContext& context, Uint32 x0, Uint32 y0)
{
    const Vector4 invSize = Vector4(VECTOR_ONE2) / Vector4::FromIntegers(GetWidth(), GetHeight(), 1, 1);
    const Uint32 tileSize = 1u << context.params->tileOrder;
    const Uint32 samplesPerPixel = context.params->samplesPerPixel;
    const Uint32 maxX = Min(x0 + tileSize, GetWidth());
    const Uint32 maxY = Min(y0 + tileSize, GetHeight());

    const bool verticalFlip = true;

    context.time = 0.0f; // TODO randomize time

    if (context.params->traversalMode == TraversalMode::Single)
    {
        for (Uint32 i = 0; i < tileSize * tileSize; ++i)
        {
            // fill the tile using Morton Curve for better cache locality
            Uint32 localX, localY;
            DecodeMorton(i, localX, localY);
            const Uint32 x = x0 + localX;
            const Uint32 y = y0 + localY;
            if (x >= maxX || y >= maxY) continue;

            const Uint32 realY = verticalFlip ? (GetHeight() - 1u - y) : y;
            const Vector4 baseCoords = Vector4::FromIntegers(x, realY);

            RayColor color;

            for (Uint32 s = 0; s < samplesPerPixel; ++s)
            {
                const Vector4 u = context.randomGenerator.GetVector4();

                // randomize pixel offset
                // TODO stratified sampling
                const Vector4 coords = baseCoords + (u - VECTOR_HALVES) * context.params->antiAliasingSpread;

                // generate primary ray
                const Ray ray = camera.GenerateRay(coords * invSize, context);
                color += scene.TraceRay_Single(ray, context);
            }

            // TODO spectral rendering
            mRenderTarget.SetPixel(x, y, color.values);
        }
    }
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

                RayColor colors[8];
                scene.TraceRay_Simd8(simdRay, context, colors);

                RayColor color;
                for (Uint16 i = 0; i < 8; ++i)
                {
                    color += colors[i];
                }
                color *= 1.0f / 8.0f;

                mRenderTarget.SetPixel(x, y, color.values);
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
        scene.Shade_Packet(primaryPacket, hitPoints, context, mRenderTarget);

        /*
        for (Uint32 y = y0; y < maxY; ++y)
        {
            for (Uint32 x = x0; x < maxX; ++x)
            {
                const Vector4 color = context.randomGenerator.GetVector4();
                mRenderTarget.SetPixel(x, y, color);
            }
        }
        */
    }

    context.counters.numPrimaryRays += tileSize * tileSize * context.params->samplesPerPixel;

    // flush non-temporal stores
    _mm_mfence();
}

bool Viewport::SetPostprocessParams(const PostprocessParams& params)
{
    mPostprocessingParams = params;
    return true;
}

void Viewport::GetPostprocessParams(PostprocessParams& params)
{
    params = mPostprocessingParams;
}

void Viewport::PostProcess(RenderingContext& context, Uint32 ymin, Uint32 ymax, Uint32 threadID)
{
    (void)context;

    const size_t width = GetWidth();
    const size_t minIndex = (size_t)ymin * width;
    const size_t maxIndex = (size_t)ymax * width;

    Random& randomGenerator = mThreadData[threadID].randomGenerator;

    const Float scalingFactor = exp2f(mPostprocessingParams.exposure) / (Float)mNumSamplesRendered;

    Vector4* __restrict sumPixels = reinterpret_cast<Vector4*>(mSum.GetData());
    const Vector4* __restrict renderTargetPixel = reinterpret_cast<const Vector4*>(mRenderTarget.GetData());
    Uint8* frontBufferPixels = reinterpret_cast<Uint8*>(mFrontBuffer.GetData());

    for (size_t i = minIndex; i < maxIndex; ++i)
    {
        const Vector4 sum = sumPixels[i];
        const Vector4 renderTargetValue = renderTargetPixel[i];
        const Vector4 newSum = sum + renderTargetValue;

        const Vector4 dither = randomGenerator.GetVector4Bipolar() * mPostprocessingParams.ditheringStrength;
        const Vector4 toneMapped = ToneMap(newSum * scalingFactor) + dither;

        sumPixels[i] = newSum;

        const Vector4 clampedValue = toneMapped.Swizzle<2, 1, 0, 3>().Clamped(Vector4(), VECTOR_ONE) * 255.0f;
        clampedValue.Store4_NonTemporal(frontBufferPixels + 4 * i);
    }


    // flush non-temporal stores
    _mm_mfence();
}

void Viewport::Reset()
{
    mNumSamplesRendered = 0;
    mSum.Zero();
}

} // namespace rt
