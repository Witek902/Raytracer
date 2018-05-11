#include "PCH.h"
#include "Viewport.h"
#include "Utils/Logger.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"
#include "Color/Color.h"


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

    if (!mBlurred.Init(width, height, Bitmap::Format::R32G32B32A32_Float))
        return false;

    if (!mFrontBuffer.Init(width, height, Bitmap::Format::B8G8R8A8_Uint))
        return false;

    Reset();

    return true;
}

RT_FORCE_INLINE static Vector4 ToneMap(Vector4 color)
{
    const Vector4 a = Vector4::Splat(0.004f);
    const Vector4 b = Vector4::Splat(6.2f);
    const Vector4 c = Vector4::Splat(1.7f);
    const Vector4 d = Vector4::Splat(0.06f);

    // Jim Hejl and Richard Burgess-Dawson formula
    color = Vector4::Max(Vector4(), color - a);

    const Vector4 t0 = color *  Vector4::MulAndAdd(color, b, VECTOR_HALVES);
    const Vector4 t1 = Vector4::MulAndAdd(color, b, c);
    const Vector4 t2 = Vector4::MulAndAdd(color, t1, d);
    return t0 * Vector4::FastReciprocal(t2);
}

bool Viewport::Render(const Scene* scene, const Camera& camera, const RenderingParams& params)
{
    if (!scene)
    {
        RT_LOG_ERROR("Invalid scene");
        return false;
    }

    // TODO split task into packets and execute on all threads

    const Uint32 width = GetWidth();
    const Uint32 height = GetHeight();
    if (width == 0 || height == 0)
    {
        return false;
    }

    // render
    {
        const Uint32 tileSize = params.tileSize;

        const auto taskCallback = [&](Uint32 tileX, Uint32 tileY, Uint32 threadID)
        {
            RenderingContext& context = mThreadData[threadID];
            context.params = &params;
            context.localCounters.Reset();
            RenderTile(*scene, camera, context, tileSize * tileX, tileSize * tileY, tileSize, tileSize);
        };

        mRenderTarget.Clear();

        const Uint32 rows = 1 + (GetHeight() - 1) / tileSize;
        const Uint32 columns = 1 + (GetWidth() - 1) / tileSize;
        mThreadPool.RunParallelTask(taskCallback, rows, columns);
    }

    // post process
    {
        mNumSamplesRendered += params.samplesPerPixel;

        if (mPostprocessingParams.bloomStrength > 0.0f)
        {
            // bloom cannot be done multithreaded
            PostProcess(0, GetHeight(), 0);
        }
        else
        {
            const Uint32 numTiles = (Uint32)std::thread::hardware_concurrency();

            const auto taskCallback = [&](Uint32, Uint32 tileY, Uint32 threadID)
            {
                const Uint32 ymin = GetHeight() * tileY / numTiles;
                const Uint32 ymax = GetHeight() * (tileY + 1) / numTiles;
                PostProcess(ymin, ymax, threadID);
            };

            mThreadPool.RunParallelTask(taskCallback, numTiles, 1);
        }
    }

    return true;
}

//#define RT_SINGLE_TRACING

void Viewport::RenderTile(const Scene& scene, const Camera& camera, RenderingContext& context,
                          Uint32 x0, Uint32 y0, Uint32 tileWidth, Uint32 tileHeight)
{
    const Vector4 invSize = Vector4(VECTOR_ONE2) / Vector4::FromIntegers(GetWidth(), GetHeight(), 1, 1);

    const Uint32 maxX = Min(x0 + tileWidth, GetWidth());
    const Uint32 maxY = Min(y0 + tileHeight, GetHeight());

    const bool verticalFlip = true;

    context.time = 0.0f; // TODO randomize time

#ifdef RT_SINGLE_TRACING

    // single ray tracing
    for (Uint32 y = y0; y < maxY; ++y)
    {
        const Uint32 realY = verticalFlip ? (GetHeight() - 1u - y) : y;

        for (Uint32 x = x0; x < maxX; ++x)
        {
            const Vector4 baseCoords = Vector4::FromIntegers(x, realY);

            RayColor color;
            for (Uint16 i = 0; i < context.params->samplesPerPixel; ++i)
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

#else // !RT_SINGLE_TRACING

    // RayPacket& rayPacket = context.rayPacket;
    for (Uint32 y = y0; y < maxY; ++y)
    {
        const Uint32 realY = verticalFlip ? (GetHeight() - 1u - y) : y;

        for (Uint32 x = x0; x < maxX; ++x)
        {
            // generate primary SIMD rays
            Vector2_Simd8 coords(Vector4::FromIntegers(x, realY));
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

#endif // RT_SINGLE_TRACING

    /*
    HitPoint_Packet& hitPoints = context.hitPoints;
    scene.Traverse_Packet(rayPacket, context, hitPoints);

    scene.ShadePacket(rayPacket, hitPoints, context, mRenderTarget);
    */

    // fill rate test
    /*
    (void)camera;
    (void)scene;
    for (Uint32 y = y0; y < maxY; ++y)
    {
        const Uint32 realY = verticalFlip ? (GetHeight() - 1u - y) : y;

        for (Uint32 x = x0; x < maxX; ++x)
        {
            Vector4 color = Vector4::FromIntegers(x, realY) * invSize;
            mRenderTarget.SetPixel(x, y, color);
        }
    }
    */

    context.counters.numPrimaryRays += tileWidth * tileHeight * context.params->samplesPerPixel;
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

void Viewport::PostProcess(Uint32 ymin, Uint32 ymax, Uint32 threadID)
{
    const Uint32 width = GetWidth();
    Random& randomGenerator = mThreadData[threadID].randomGenerator;

    const Float scalingFactor = mPostprocessingParams.exposure / (Float)mNumSamplesRendered;

    /*
    if (mPostprocessingParams.bloomStrength > 0.0f)
    {
        // composite "summed" image
        {
            for (Uint32 y = ymin; y < ymax; ++y)
            {
                Vector4* sumLine = mSum.GetPixels(0, y);
                const Vector4* renderTargetLine = mRenderTarget.GetPixels(0, y);

                for (Uint32 x = 0; x < width; ++x)
                {
                    const Vector4 newSum = sumLine[x] + renderTargetLine[x];
                    sumLine[x] = newSum;
                }
            }
        }

        Bitmap::Blur(mBlurred, mSum, mPostprocessingParams.bloomSize, 3);

        // generate front buffer
        {
            Vector4 frontBufferLine[Bitmap::MaxSize];
            Vector4 blurredLine[Bitmap::MaxSize];
            Vector4 sumLine[Bitmap::MaxSize];

            for (Uint32 y = ymin; y < ymax; ++y)
            {
                mBlurred.ReadHorizontalLine(y, blurredLine);
                mSum.ReadHorizontalLine(y, sumLine);

                for (Uint32 x = 0; x < width; ++x)
                {
                    const Vector4 color = mPostprocessingParams.bloomStrength * blurredLine[x] + sumLine[x];
                    const Vector4 toneMappedColor = ToneMap(color * (scalingFactor * mPostprocessingParams.exposure));
                    const Vector4 noiseValue = (randomGenerator.GetVector4() - VECTOR_HALVES) * mPostprocessingParams.noiseStrength;
                    frontBufferLine[x] = toneMappedColor + noiseValue;
                }

                mFrontBuffer.WriteHorizontalLine(y, frontBufferLine);
            }
        }
    }
    else
    {
    */
        Vector4 frontBufferLine[Bitmap::MaxSize];

        // TODO use AVX
        for (Uint32 y = ymin; y < ymax; ++y)
        {
            Vector4* sumLine = mSum.GetPixels(0, y);
            const Vector4* renderTargetLine = mRenderTarget.GetPixels(0, y);

            for (Uint32 x = 0; x < width; ++x)
            {
                const Vector4 newSum = sumLine[x] + renderTargetLine[x];

                Vector4 filmGrain;
                if (mPostprocessingParams.filmGrainStrength > 0.0f)
                {
                    filmGrain = randomGenerator.GetVector4Bipolar() + randomGenerator.GetVector4Bipolar() + randomGenerator.GetVector4Bipolar();
                    filmGrain *= mPostprocessingParams.filmGrainStrength;
                }

                const Vector4 dither = randomGenerator.GetVector4Bipolar() * (mPostprocessingParams.ditheringStrength * 0.5f);

                sumLine[x] = newSum;
                frontBufferLine[x] = ToneMap(newSum * scalingFactor + filmGrain) + dither;
            }

            mFrontBuffer.WriteHorizontalLine(y, frontBufferLine);
        }
    //}
}

void Viewport::Reset()
{
    mNumSamplesRendered = 0;
    mSum.Zero();
}

} // namespace rt
