#include "PCH.h"
#include "Viewport.h"
#include "Utils/Logger.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"


namespace rt {

using namespace math;

static const Uint32 MAX_IMAGE_SZIE = 8192;

Viewport::Viewport()
    : mNumSamplesRendered(0)
    , mWindow(0)
    , mDC(0)
    , mFrameID(0)
{
    InitThreadData();
}

bool Viewport::Initialize(HWND windowHandle)
{
    mWindow = windowHandle;
    mDC = GetDC(mWindow);

    return true;
}

void Viewport::InitThreadData()
{
    mThreadData.resize(std::thread::hardware_concurrency());
    for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i)
    {
        mThreadData[i].random.Reset(2654435761u * static_cast<Uint32>(i));
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

    mDC = GetDC(mWindow);
    return true;
}

RT_FORCE_INLINE static Vector4 ToneMap(Vector4 color)
{
    // Jim Hejl and Richard Burgess-Dawson formula
    color = Vector4::Max(Vector4(), color - Vector4::Splat(0.004f));
    return (color * (6.2f * color + Vector4::Splat(0.5f))) / (color * (6.2f * color + Vector4::Splat(1.7f)) + Vector4::Splat(0.06f));
}

bool Viewport::Render(const Scene* scene, const Camera& camera)
{
    RenderingParams params;

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

    const Uint32 tileSize = 32;

    const auto taskCallback = [&](Uint32 tileX, Uint32 tileY, Uint32 threadID)
    {
        Random& randomGenerator = mThreadData[threadID].random;
        RayTracingCounters counters;
        RenderingContext context(randomGenerator, params, counters);
        RenderTile(*scene, camera, context, tileSize * tileX, tileSize * tileY, tileSize, tileSize);
    };

    const Uint32 rows = 1 + (GetHeight() - 1) / tileSize;
    const Uint32 columns = 1 + (GetWidth() - 1) / tileSize;
    mThreadPool.RunParallelTask(taskCallback, rows, columns);

    PostProcess();
    Paint();
    mFrameID++;

    return true;
}

void Viewport::RenderTile(const Scene& scene, const Camera& camera, RenderingContext& context,
                             Uint32 x0, Uint32 y0, Uint32 width, Uint32 height)
{
    const Vector4 invSize = Vector4(VECTOR_ONE2) / Vector4::FromIntegers(GetWidth(), GetHeight(), 1, 1);

    const Uint32 maxX = Min(x0 + width, GetWidth());
    const Uint32 maxY = Min(y0 + height, GetHeight());

    //RayPacket rayPacket;

    for (Uint32 y = y0; y < maxY; ++y)
    {
        for (Uint32 x = x0; x < maxX; ++x)
        {
            Vector4 coords = Vector4::FromIntegers(x, y);
            coords += (context.randomGenerator.GetVector4() - VECTOR_HALVES) * context.params.antiAliasingSpread;

            // TODO Monte Carlo ray generation:
            // motion blur
            // chromatic aberration

            // generate primary ray
            const Ray cameraRay = camera.GenerateRay(coords * invSize, context.randomGenerator);

            //rayPacket.PushRay(cameraRay, math::VECTOR_ONE3, ImageLocationInfo{ x, y });

            const Vector4 color = scene.TraceRay_Single(cameraRay, context);
            mRenderTarget.SetPixel(x, y, color);
        }
    }

    context.counters.numPrimaryRays += width * height;
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

void Viewport::PostProcess()
{
    const Uint32 width = GetWidth();
    const Uint32 height = GetHeight();
    Random& randomGenerator = mThreadData[0].random;

    mNumSamplesRendered++;
    const Float scalingFactor = 1.0f / (Float)mNumSamplesRendered;

    if (mPostprocessingParams.bloomStrength > 0.0f)
    {
        // composite "summed" image
        {
            Vector4 renderTargetLine[Bitmap::MaxSize];
            Vector4 sumLine[Bitmap::MaxSize];

            for (Uint32 y = 0; y < height; ++y)
            {
                mSum.ReadHorizontalLine(y, sumLine);
                mRenderTarget.ReadHorizontalLine(y, renderTargetLine);

                for (Uint32 x = 0; x < width; ++x)
                {
                    const Vector4 newSum = sumLine[x] + renderTargetLine[x];
                    sumLine[x] = newSum;
                }

                mSum.WriteHorizontalLine(y, sumLine);
            }
        }

        Bitmap::Blur(mBlurred, mSum, mPostprocessingParams.bloomSize, 3);

        // generate front buffer
        {
            Vector4 frontBufferLine[Bitmap::MaxSize];
            Vector4 blurredLine[Bitmap::MaxSize];
            Vector4 sumLine[Bitmap::MaxSize];

            for (Uint32 y = 0; y < height; ++y)
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
        Vector4 renderTargetLine[Bitmap::MaxSize];
        Vector4 sumLine[Bitmap::MaxSize];
        Vector4 frontBufferLine[Bitmap::MaxSize];

        for (Uint32 y = 0; y < height; ++y)
        {
            mSum.ReadHorizontalLine(y, sumLine);
            mRenderTarget.ReadHorizontalLine(y, renderTargetLine);

            for (Uint32 x = 0; x < width; ++x)
            {
                const Vector4 newSum = sumLine[x] + renderTargetLine[x];
                sumLine[x] = newSum;

                const Vector4& color = sumLine[x];
                const Vector4 toneMappedColor = ToneMap(color * (scalingFactor * mPostprocessingParams.exposure));
                const Vector4 noiseValue = (randomGenerator.GetVector4() - VECTOR_HALVES) * mPostprocessingParams.noiseStrength;
                frontBufferLine[x] = toneMappedColor + noiseValue;
            }

            mSum.WriteHorizontalLine(y, sumLine);
            mFrontBuffer.WriteHorizontalLine(y, frontBufferLine);
        }
    }
}

void Viewport::Paint()
{
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = GetWidth();
    bmi.bmiHeader.biHeight = GetHeight();
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 4 * GetWidth() * GetHeight();
    bmi.bmiHeader.biXPelsPerMeter = 1;
    bmi.bmiHeader.biYPelsPerMeter = 1;

    if (0 == StretchDIBits(mDC,
                           0, 0, GetWidth(), GetHeight(),
                           0, 0, GetWidth(), GetHeight(),
                           mFrontBuffer.GetData(),
                           &bmi, DIB_RGB_COLORS, SRCCOPY))
    {
        RT_LOG_ERROR("Paint failed");
    }
}

void Viewport::Reset()
{
    mNumSamplesRendered = 0;
    mSum.Zero();
}

} // namespace rt
