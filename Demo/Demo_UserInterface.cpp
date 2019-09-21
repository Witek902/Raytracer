#include "PCH.h"
#include "Demo.h"

#include "../Core/Material/Material.h"
#include "../Core/Scene/Object/SceneObject.h"
#include "../Core/Scene/Light/BackgroundLight.h"
#include "../Core/Rendering/PathTracerMIS.h"
#include "../Core/Rendering/VertexConnectionAndMerging.h"
#include "../Core/Rendering/DebugRenderer.h"
#include "../Core/Color/ColorHelpers.h"

using namespace rt;
using namespace math;

static bool EditEulerAngles(const char* name, Float3& angles)
{
    Float3 orientation = angles * (180.0f / RT_PI);
    if (ImGui::InputFloat3(name, &orientation.x, 3, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        angles = orientation * (RT_PI / 180.0f);
        return true;
    }
    return false;
}

static bool EditRotation(const char* name, Quaternion& quat)
{
    Float3 orientation = quat.ToEulerAngles();
    orientation *= 180.0f / RT_PI;
    if (ImGui::InputFloat3(name, &orientation.x, 3, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        orientation *= RT_PI / 180.0f;
        quat = Quaternion::FromEulerAngles(orientation);
        return true;
    }
    return false;
}

void DemoWindow::RenderUI_Stats()
{
    const RenderingProgress& progress = mViewport->GetProgress();

    ImGui::Columns(2);

    ImGui::Text("Average render time"); ImGui::NextColumn();
    ImGui::Text("%.2f ms", 1000.0 * mAverageRenderDeltaTime); ImGui::NextColumn();

    ImGui::Text("Minimum render time"); ImGui::NextColumn();
    ImGui::Text("%.2f ms", 1000.0 * mMinimumRenderDeltaTime); ImGui::NextColumn();

    ImGui::Text("Total render time"); ImGui::NextColumn();
    ImGui::Text("%.3f s", mTotalRenderTime); ImGui::NextColumn();

    ImGui::Separator();

    ImGui::Text("Passes finished"); ImGui::NextColumn();
    ImGui::Text("%u", progress.passesFinished); ImGui::NextColumn();

    ImGui::Text("Error"); ImGui::NextColumn();
    ImGui::Text("%.3f dB", 10.0f * log10f(progress.averageError)); ImGui::NextColumn();

    ImGui::Text("Progress"); ImGui::NextColumn();
    ImGui::Text("%.2f%%", 100.0f * progress.converged); ImGui::NextColumn();

    ImGui::Text("Active blocks"); ImGui::NextColumn();
    ImGui::Text("%u", progress.activeBlocks); ImGui::NextColumn();

    ImGui::Separator();

    ImGui::Text("Delta time"); ImGui::NextColumn();
    ImGui::Text("%.2f ms", 1000.0 * mDeltaTime); ImGui::NextColumn();

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    const RayTracingCounters& counters = mViewport->GetCounters();
    ImGui::Separator();
    {
        ImGui::Text("Rays"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numRays / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Primary rays"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numPrimaryRays / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Shadow rays"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numShadowRays / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Shadow rays (hit)"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numShadowRaysHit / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Ray-box tests (total)"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numRayBoxTests / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Ray-box tests (passed)"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numPassedRayBoxTests / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Ray-tri tests (total)"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numRayTriangleTests / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Ray-tri tests (passed)"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numPassedRayTriangleTests / 1.0e+6f); ImGui::NextColumn();
    }
#endif // RT_ENABLE_INTERSECTION_COUNTERS

    ImGui::Columns(1);
}

void DemoWindow::RenderUI_Debugging()
{
    if (ImGui::TreeNode("Path"))
    {
        RenderUI_Debugging_Path();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Color"))
    {
        RenderUI_Debugging_Color();
        ImGui::TreePop();
    }

    if (ImGui::Button("Debug pixel"))
    {
        mPixelDebuggingPicking = true;
    }

    if (mPixelDebuggingPicking)
    {
        ImGui::SameLine();
        ImGui::Text("Picking...");
    }
}

void DemoWindow::RenderUI_Debugging_Path()
{
    for (Uint32 i = 0; i < mPathDebugData.data.Size(); ++i)
    {
        ImGui::Separator();
        ImGui::Text("Ray #%u", i);

        ImGui::Columns(2);

        const rt::PathDebugData::HitPointData data = mPathDebugData.data[i];

        ImGui::Text("RayOrigin"); ImGui::NextColumn();
        ImGui::Text("[%f, %f, %f]", data.rayOrigin.x, data.rayDir.y, data.rayOrigin.z); ImGui::NextColumn();

        ImGui::Text("RayDir"); ImGui::NextColumn();
        ImGui::Text("[%f, %f, %f]", data.rayDir.x, data.rayDir.y, data.rayDir.z); ImGui::NextColumn();

        if (data.hitPoint.distance != FLT_MAX)
        {
            ImGui::Text("Distance"); ImGui::NextColumn();
            ImGui::Text("%f", data.hitPoint.distance); ImGui::NextColumn();

            ImGui::Text("Object ID"); ImGui::NextColumn();
            ImGui::Text("%u", data.hitPoint.objectId); ImGui::NextColumn();

            ImGui::Text("Sub Obj ID"); ImGui::NextColumn();
            ImGui::Text("%u", data.hitPoint.subObjectId); ImGui::NextColumn();

            ImGui::Text("Tri UV"); ImGui::NextColumn();
            ImGui::Text("[%f, %f]", data.hitPoint.u, data.hitPoint.v); ImGui::NextColumn();

            const Vector4 pos = data.shadingData.intersection.frame.GetTranslation();
            ImGui::Text("Position"); ImGui::NextColumn();
            ImGui::Text("[%f, %f, %f]", pos.x, pos.y, pos.z); ImGui::NextColumn();

            const Vector4 normal = data.shadingData.intersection.frame.GetTranslation();
            ImGui::Text("Normal"); ImGui::NextColumn();
            ImGui::Text("[%f, %f, %f]", normal.x, normal.y, normal.z); ImGui::NextColumn();

            //ImGui::Text("Tangent"); ImGui::NextColumn();
            //ImGui::Text("[%f, %f, %f]", data.shadingData.tangent.x, data.shadingData.tangent.y, data.shadingData.tangent.z); ImGui::NextColumn();

            //ImGui::Text("Tex coord"); ImGui::NextColumn();
            //ImGui::Text("[%f, %f]", data.shadingData.texCoord.x, data.shadingData.texCoord.y); ImGui::NextColumn();

            ImGui::Text("Material"); ImGui::NextColumn();
            ImGui::Text("%s", data.shadingData.intersection.material->debugName.c_str()); ImGui::NextColumn();

            ImGui::Text("Throughput"); ImGui::NextColumn();
#ifdef RT_ENABLE_SPECTRAL_RENDERING
            ImGui::Text("[%f, %f, %f, %f, %f, %f, %f, %f]",
                data.throughput.value[0], data.throughput.value[1], data.throughput.value[2], data.throughput.value[3],
                data.throughput.value[4], data.throughput.value[5], data.throughput.value[6], data.throughput.value[7]);
#else
            ImGui::Text("[%f, %f, %f]",
                data.throughput.value[0], data.throughput.value[1], data.throughput.value[2]);
#endif // RT_ENABLE_SPECTRAL_RENDERING
            ImGui::NextColumn();

            const char* bsdfEventStr = "Null";
            switch (data.bsdfEvent)
            {
            case BSDF::DiffuseReflectionEvent: bsdfEventStr = "Diffuse Reflection"; break;
            case BSDF::DiffuseTransmissionEvent: bsdfEventStr = "Diffuse Transmission"; break;
            case BSDF::GlossyReflectionEvent: bsdfEventStr = "Glossy Reflection"; break;
            case BSDF::GlossyRefractionEvent: bsdfEventStr = "Glossy Refraction"; break;
            case BSDF::SpecularReflectionEvent: bsdfEventStr = "Specular Reflection"; break;
            case BSDF::SpecularRefractionEvent: bsdfEventStr = "Specular Refraction"; break;
            }
            ImGui::Text("BSDF Event"); ImGui::NextColumn();
            ImGui::Text("%s", bsdfEventStr); ImGui::NextColumn();
        }

        ImGui::Columns(1);
    }

    const char* terminationReasonStr = "None";
    switch (mPathDebugData.terminationReason)
    {
    case PathTerminationReason::HitBackground: terminationReasonStr = "Hit background"; break;
    case PathTerminationReason::HitLight: terminationReasonStr = "Hit light"; break;
    case PathTerminationReason::Depth: terminationReasonStr = "Depth exeeded"; break;
    case PathTerminationReason::Throughput: terminationReasonStr = "Throughput too low"; break;
    case PathTerminationReason::NoSampledEvent: terminationReasonStr = "No sampled BSDF event"; break;
    case PathTerminationReason::RussianRoulette: terminationReasonStr = "Russian roulette"; break;
    }

    ImGui::Text("Path termination reason: %s", terminationReasonStr);
}

void DemoWindow::RenderUI_Debugging_Color()
{
    Int32 x, y;
    GetMousePosition(x, y);

    Uint32 width, height;
    GetSize(width, height);

    Vector4 hdrColor, ldrColor;
    if (x >= 0 && y >= 0 && (Uint32)x < width && (Uint32)y < height)
    {
        // TODO this is incorrect, each pixel can have different number of samples
        const Uint32 numSamples = mViewport->GetProgress().passesFinished;
        hdrColor = mViewport->GetSumBuffer().GetPixel(x, y, true) / static_cast<float>(numSamples);
        ldrColor = mViewport->GetFrontBuffer().GetPixel(x, y, true);
    }

    ImGui::Text("HDR color:");
#ifdef RT_ENABLE_SPECTRAL_RENDERING
    const Vector4 rgbHdrColor = rt::ConvertXYZtoRGB(hdrColor);
    ImGui::Text("  R: %f", rgbHdrColor.x);
    ImGui::Text("  G: %f", rgbHdrColor.y);
    ImGui::Text("  B: %f", rgbHdrColor.z);
    ImGui::Text("  X: %f", hdrColor.x);
    ImGui::Text("  Y: %f", hdrColor.y);
    ImGui::Text("  Z: %f", hdrColor.z);
#else
    ImGui::Text("  R: %f", hdrColor.x);
    ImGui::Text("  G: %f", hdrColor.y);
    ImGui::Text("  B: %f", hdrColor.z);
#endif // RT_ENABLE_SPECTRAL_RENDERING

    ImGui::Text("LDR color:");
    ImGui::Text("  R: %u", (Uint32)(255.0f * ldrColor.x + 0.5f));
    ImGui::Text("  G: %u", (Uint32)(255.0f * ldrColor.y + 0.5f));
    ImGui::Text("  B: %u", (Uint32)(255.0f * ldrColor.z + 0.5f));
}

bool DemoWindow::RenderUI_Settings()
{
    bool resetFrame = false;

    if (ImGui::TreeNode("Rendering"))
    {
        resetFrame |= RenderUI_Settings_Rendering();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Sampling"))
    {
        resetFrame |= RenderUI_Settings_Sampling();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Adaptive Rendering"))
    {
        resetFrame |= RenderUI_Settings_AdaptiveRendering();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Camera"))
    {
        resetFrame |= RenderUI_Settings_Camera();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Postprocess"))
    {
        resetFrame |= RenderUI_Settings_PostProcess();
        ImGui::TreePop();
    }

    if (mSelectedObject)
    {
        if (ImGui::TreeNode("Object"))
        {
            resetFrame |= RenderUI_Settings_Object();
            ImGui::TreePop();
        }
    }

    if (mSelectedLight)
    {
        if (ImGui::TreeNode("Light"))
        {
            resetFrame |= RenderUI_Settings_Light();
            ImGui::TreePop();
        }
    }

    if (mSelectedMaterial)
    {
        if (ImGui::TreeNode("Material", "Material (%s)", mSelectedMaterial->debugName.c_str()))
        {
            resetFrame |= RenderUI_Settings_Material();
            ImGui::TreePop();
        }
    }

    // screenshot saving
    {
        if (ImGui::Button("LDR screenshot"))
        {
            mViewport->GetFrontBuffer().SaveBMP("screenshot.bmp", true);
        }

        ImGui::SameLine();

        if (ImGui::Button("HDR screenshot"))
        {
            // TODO this is incorrect
            const float colorScale = 1.0f / (float)mViewport->GetProgress().passesFinished;
            mViewport->GetSumBuffer().SaveEXR("screenshot.exr", colorScale);
        }
    }

    return resetFrame;
}

bool DemoWindow::RenderUI_Settings_Rendering()
{
    bool resetFrame = false;

    {
        Uint32 maxThreads = std::thread::hardware_concurrency();
        ImGui::SliderInt("Threads", (int*)&mRenderingParams.numThreads, 1, 2 * maxThreads);
    }

    // renderer selection
    {
        // TODO use reflection
        const char* rendererNames[] =
        {
            "Debug",
            "Path Tracer",
            "Path Tracer MIS",
            "Light Tracer",
            "VCM",
        };

        int currentRendererIndex = 0;
        for (int i = 0; i < IM_ARRAYSIZE(rendererNames); ++i)
        {
            if (0 == strcmp(rendererNames[i], mRenderer->GetName()))
            {
                currentRendererIndex = i;
                break;
            }
        }

        if (ImGui::Combo("Renderer", &currentRendererIndex, rendererNames, IM_ARRAYSIZE(rendererNames)))
        {
            mRendererName = rendererNames[currentRendererIndex];
            mRenderer = CreateRenderer(mRendererName, *mScene);
            mViewport->SetRenderer(mRenderer);
            resetFrame = true;
        }
    }

    if (mRendererName == "Debug")
    {
        rt::DebugRenderer* debugRenderer = (DebugRenderer*)mRenderer.get();
        int debugRenderingModeIndex = static_cast<int>(debugRenderer->mRenderingMode);

        const char* renderingModeItems[] =
        {
            "CameraLight",
            "TriangleID", "Depth", "Position", "Normals", "Tangents", "Bitangents", "TexCoords",
            "Material Base Color",
            "Material Emission Color",
            "Material Roughness",
            "Material Metalness",
            "Material Index of Refraction",
#ifdef RT_ENABLE_INTERSECTION_COUNTERS
            "RayBoxIntersection", "RayBoxIntersectionPassed", "RayTriIntersection", "RayTriIntersectionPassed",
#endif // RT_ENABLE_INTERSECTION_COUNTERS
        };
        resetFrame |= ImGui::Combo("Rendering mode", &debugRenderingModeIndex, renderingModeItems, IM_ARRAYSIZE(renderingModeItems));
        debugRenderer->mRenderingMode = static_cast<DebugRenderingMode>(debugRenderingModeIndex);
    }
    else if (mRendererName == "Path Tracer MIS")
    {
        rt::PathTracerMIS* renderer = (PathTracerMIS*)mRenderer.get();
        resetFrame |= ImGui::ColorEdit3("Light sampling weight", &renderer->mLightSamplingWeight.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
        resetFrame |= ImGui::ColorEdit3("BSDF sampling weight", &renderer->mBSDFSamplingWeight.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
    }
    else if (mRendererName == "VCM")
    {
        rt::VertexConnectionAndMerging* renderer = (VertexConnectionAndMerging*)mRenderer.get();
        resetFrame |= ImGui::Checkbox("Vertex connection", &renderer->mUseVertexConnection);
        ImGui::SameLine();
        resetFrame |= ImGui::Checkbox("Vertex merging", &renderer->mUseVertexMerging);
        resetFrame |= ImGui::SliderInt("Max path length", (Int32*)&renderer->mMaxPathLength, 1, 20);
        resetFrame |= ImGui::SliderFloat("Initial merging radius", &renderer->mInitialMergingRadius, renderer->mMinMergingRadius, 10.0f, "%.4f", 10.0f);
        resetFrame |= ImGui::SliderFloat("Min merging radius", &renderer->mMinMergingRadius, 0.0001f, renderer->mInitialMergingRadius, "%.4f", 10.0f);
        resetFrame |= ImGui::SliderFloat("Merging radius multiplier", &renderer->mMergingRadiusMultiplier, 0.5f, 0.9999f);

        if (ImGui::TreeNode("Debug coloring"))
        {
            resetFrame |= ImGui::ColorEdit3("Light sampling weight", &renderer->mLightSamplingWeight.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            resetFrame |= ImGui::ColorEdit3("BSDF sampling weight", &renderer->mBSDFSamplingWeight.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            resetFrame |= ImGui::ColorEdit3("Vertex connecting weight", &renderer->mVertexConnectingWeight.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            resetFrame |= ImGui::ColorEdit3("Vertex merging weight", &renderer->mVertexMergingWeight.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            resetFrame |= ImGui::ColorEdit3("Camera connecting weight", &renderer->mCameraConnectingWeight.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            ImGui::TreePop(); // Transform
        }
    }

    int traversalModeIndex = static_cast<int>(mRenderingParams.traversalMode);
    int lightSamplingStrategyIndex = static_cast<int>(mRenderingParams.lightSamplingStrategy);
    int tileSize = static_cast<int>(mRenderingParams.tileSize);

    const char* traversalModeItems[] = { "Single", "Packet" };
    resetFrame |= ImGui::Combo("Traversal mode", &traversalModeIndex, traversalModeItems, IM_ARRAYSIZE(traversalModeItems));

    const char* lightSamplingStrategyItems[] = { "Single", "All" };
    resetFrame |= ImGui::Combo("Light sampling strategy", &lightSamplingStrategyIndex, lightSamplingStrategyItems, IM_ARRAYSIZE(lightSamplingStrategyItems));

    ImGui::SliderInt("Tile size", (int*)&tileSize, 2, 256);

    resetFrame |= ImGui::SliderInt("Max ray depth", (int*)&mRenderingParams.maxRayDepth, 0, 200);
    resetFrame |= ImGui::Checkbox("Visualize time per pixel", &mRenderingParams.visualizeTimePerPixel);
    resetFrame |= ImGui::SliderInt("Russian roulette depth", (int*)&mRenderingParams.minRussianRouletteDepth, 1, 64);
    resetFrame |= ImGui::SliderFloat("Antialiasing spread", &mRenderingParams.antiAliasingSpread, 0.0f, 3.0f);
    resetFrame |= ImGui::SliderFloat("Motion blur strength", &mRenderingParams.motionBlurStrength, 0.0f, 1.0f);

    mRenderingParams.traversalMode = static_cast<TraversalMode>(traversalModeIndex);
    mRenderingParams.lightSamplingStrategy = static_cast<LightSamplingStrategy>(lightSamplingStrategyIndex);
    mRenderingParams.tileSize = static_cast<Uint16>(tileSize);

    return resetFrame;
}

bool DemoWindow::RenderUI_Settings_Sampling()
{
    bool resetFrame = false;

    resetFrame |= ImGui::SliderInt("Sample dimensions", (int*)&mRenderingParams.samplingParams.dimensions, 0, 256);
    resetFrame |= ImGui::Checkbox("Blue noise dithering", &mRenderingParams.samplingParams.useBlueNoiseDithering);

    return resetFrame;
}

bool DemoWindow::RenderUI_Settings_AdaptiveRendering()
{
    bool resetFrame = false;

    AdaptiveRenderingSettings& settings = mRenderingParams.adaptiveSettings;

    resetFrame |= ImGui::Checkbox("Enable", &settings.enable);
    resetFrame |= ImGui::SliderInt("Num initial passes", (int*)&settings.numInitialPasses, 1, 100);
    resetFrame |= ImGui::SliderInt("Max block size", (int*)&settings.maxBlockSize, settings.minBlockSize, 1024);
    resetFrame |= ImGui::SliderInt("Min block size", (int*)&settings.minBlockSize, 1, settings.maxBlockSize);
    resetFrame |= ImGui::SliderFloat("Convergence treshold", &settings.convergenceTreshold, 1.0e-8f, settings.subdivisionTreshold, "%.2e", 10.0f);
    resetFrame |= ImGui::SliderFloat("Subdivision treshold", &settings.subdivisionTreshold, settings.convergenceTreshold, 1.0f, "%.2e", 10.0f);

    ImGui::Checkbox("(Debug) Visualize", &mVisualizeAdaptiveRenderingBlocks);

    return resetFrame;
}

bool DemoWindow::RenderUI_Settings_Camera()
{
    bool resetFrame = false;

    if (ImGui::TreeNode("Transform"))
    {
        resetFrame |= ImGui::InputFloat3("Position", &mCameraSetup.position.x, 3);
        resetFrame |= EditEulerAngles("Orientation", mCameraSetup.orientation);

        resetFrame |= ImGui::InputFloat3("Velocity", &mCameraSetup.linearVelocity.x, 3);
        resetFrame |= ImGui::InputFloat3("Angular velocity", &mCameraSetup.angularVelocity.x, 3);

        ImGui::TreePop(); // Transform
    }

    if (ImGui::TreeNode("Lens"))
    {
        resetFrame |= ImGui::SliderFloat("Field of view", &mCameraSetup.fov, 0.5f, 120.0f);

        resetFrame |= ImGui::Checkbox("Enable DoF", &mCamera.mDOF.enable);
        resetFrame |= ImGui::SliderFloat("Aperture", &mCamera.mDOF.aperture, 0.0f, 10.0f, "%.3f", 5.0f);
        {
            ImGui::Columns(2, nullptr, false);
            resetFrame |= ImGui::SliderFloat("Focal distance", &mCamera.mDOF.focalPlaneDistance, 0.1f, 1000.0f, "%.3f", 2.0f);
            ImGui::NextColumn();
            if (ImGui::Button("Pick..."))
            {
                mFocalDistancePicking = true;
            }
            ImGui::Columns(1);
        }

        const char* bokehTypeNames[] = { "Circle", "Hexagon", "Box", "n-gon", "Texture" };
        int bokehTypeIndex = static_cast<int>(mCamera.mDOF.bokehShape);

        resetFrame |= ImGui::Combo("Bokeh Shape", &bokehTypeIndex, bokehTypeNames, IM_ARRAYSIZE(bokehTypeNames));
        if (mCamera.mDOF.bokehShape == BokehShape::NGon)
        {
            resetFrame |= ImGui::SliderInt("No. of blades", (int*)&mCamera.mDOF.apertureBlades, 3, 20);
        }

        resetFrame |= ImGui::Checkbox("Enable lens distortions", &mCamera.enableBarellDistortion);
        resetFrame |= ImGui::SliderFloat("Barrel distortion", &mCamera.barrelDistortionConstFactor, 0.0f, 0.2f);
        resetFrame |= ImGui::SliderFloat("Lens distortion", &mCamera.barrelDistortionVariableFactor, 0.0f, 0.2f);

        mCamera.mDOF.bokehShape = static_cast<BokehShape>(bokehTypeIndex);

        ImGui::TreePop(); // Lens
    }

    return resetFrame;
}

bool DemoWindow::RenderUI_Settings_PostProcess()
{
    bool changed = false;

    changed |= ImGui::SliderFloat("Exposure", &mPostprocessParams.exposure, -8.0f, 8.0f, "%+.3f EV");
    changed |= ImGui::SliderFloat("Saturation", &mPostprocessParams.saturation, 0.0f, 2.0f);
    changed |= ImGui::SliderFloat("Contrast", &mPostprocessParams.contrast, 0.125f, 4.0f, "%.2f", 2.0f);
    changed |= ImGui::SliderFloat("Bloom factor", &mPostprocessParams.bloomFactor, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Dithering", &mPostprocessParams.ditheringStrength, 0.0f, 0.1f);
    changed |= ImGui::ColorEdit3("Color filter", &mPostprocessParams.colorFilter.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

    int tonemapperIndex = static_cast<int>(mPostprocessParams.tonemapper);
    const char* tonemapperNames[] =
    {
        "Clamped",
        "Reinhard",
        "Jim Hejland and Richard Burgess Dawson",
        "ACES (approx)",
    };
    if (ImGui::Combo("Tonemapper", &tonemapperIndex, tonemapperNames, IM_ARRAYSIZE(tonemapperNames)))
    {
        changed = true;
        mPostprocessParams.tonemapper = static_cast<Tonemapper>(tonemapperIndex);
    }

    if (changed)
    {
        mViewport->SetPostprocessParams(mPostprocessParams);
    }

    return false;
}

bool DemoWindow::RenderUI_Settings_Light()
{
    bool changed = false;

    {
        Spectrum color = mSelectedLight->GetColor();
        if (ImGui::ColorEdit3("Color", &color.rgbValues.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR))
        {
            if ((color.rgbValues >= Vector4::Zero()).All())
            {
                mSelectedLight->SetColor(color);
                changed = true;
            }
        }
    }

    return changed;
}

bool DemoWindow::RenderUI_Settings_Object()
{
    bool changed = false;

    {
        Float3 position = mSelectedObject->GetBaseTransform().GetTranslation().ToFloat3();
        if (ImGui::InputFloat3("Position", &position.x, 2, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            mSelectedObject->SetTransform(Matrix4::MakeTranslation(Vector4(position)));
            changed = true;
        }
    }

    // TODO
    /*
    {
        Float3 orientation = mSelectedObject->mTransform.GetRotation().ToEulerAngles();
        orientation *= 180.0f / RT_PI;
        if (ImGui::InputFloat3("Orientation", &orientation.x, 2, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            orientation *= RT_PI / 180.0f;
            mSelectedObject->mTransform.SetRotation(Quaternion::FromEulerAngles(orientation));
            changed = true;
        }
    }


    {
        Float3 velocity = mSelectedObject->mLinearVelocity.ToFloat3();
        if (ImGui::InputFloat3("Linear Velocity", &velocity.x, 2, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            mSelectedObject->mLinearVelocity = Vector4(velocity);
            changed = true;
        }
    }

    {
        Float3 angularVelocity = mSelectedObject->mAngularVelocity.ToEulerAngles();
        angularVelocity *= 180.0f / RT_PI;
        if (ImGui::InputFloat3("Angular Velocity", &angularVelocity.x, 2, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            angularVelocity *= RT_PI / 180.0f;
            mSelectedObject->mAngularVelocity = Quaternion::FromEulerAngles(angularVelocity);
            changed = true;
        }
    }
    */

    if (changed)
    {
        mScene->BuildBVH();
    }

    return changed;
}

bool DemoWindow::RenderUI_Settings_Material()
{
    bool changed = false;

    const char* bsdfs[] =
    {
        "diffuse",
        "roughDiffuse",
        "dielectric",
        "roughDielectric",
        "metal",
        "roughMetal",
        "plastic",
        "roughPlastic",
    };

    const int numBsdfs = 8;
    int currentBsdfIndex = 0;
    for (int i = 0; i < numBsdfs; ++i)
    {
        if (0 == strcmp(bsdfs[i], mSelectedMaterial->GetBSDF()->GetName()))
        {
            currentBsdfIndex = i;
            break;
        }
    }

    if (ImGui::Combo("BSDF", &currentBsdfIndex, bsdfs, numBsdfs))
    {
        mSelectedMaterial->SetBsdf(bsdfs[currentBsdfIndex]);
        changed = true;
    }

    changed |= ImGui::ColorEdit3("Emission color", &mSelectedMaterial->emission.baseValue.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
    changed |= ImGui::ColorEdit3("Base color", &mSelectedMaterial->baseColor.baseValue.x, ImGuiColorEditFlags_Float);
    changed |= ImGui::SliderFloat("Roughness", &mSelectedMaterial->roughness.baseValue, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Metalness", &mSelectedMaterial->metalness.baseValue, 0.0f, 1.0f);
    changed |= ImGui::Checkbox("Dispersive", &mSelectedMaterial->isDispersive);

    if (mSelectedMaterial->isDispersive)
    {
        changed |= ImGui::SliderFloat("C", &mSelectedMaterial->dispersionParams.C, 0.0f, 1.0f);
        changed |= ImGui::SliderFloat("D", &mSelectedMaterial->dispersionParams.D, 0.0f, 10.0f);
    }

    {
        const float iorRange = 5.0f;
        changed |= ImGui::SliderFloat("Refractive index", &mSelectedMaterial->IoR, 1.0f / iorRange, iorRange);
    }

    changed |= ImGui::SliderFloat("Extinction coeff. (K)", &mSelectedMaterial->K, 0.0f, 10.0f);

    if (mSelectedMaterial->normalMap)
    {
        changed |= ImGui::SliderFloat("Normal map strength", &mSelectedMaterial->normalMapStrength, 0.0f, 5.0f);
    }

    if (changed)
    {
        mSelectedMaterial->Compile();
    }

    return changed;
}

bool DemoWindow::RenderUI()
{
    bool resetFrame = false;

    Uint32 width, height;
    GetSize(width, height);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = (float)mDeltaTime;
    io.KeyCtrl = IsKeyPressed(KeyCode::Control);
    io.KeyShift = IsKeyPressed(KeyCode::Shift);
    io.KeyAlt = IsKeyPressed(KeyCode::Alt);

    for (Uint32 i = 0; i < 256; ++i)
    {
        io.KeysDown[i] = IsKeyPressed(static_cast<KeyCode>(i));
    }

    ImGui::NewFrame();
    {
        static bool showStats = true;
        static bool showDebugging = false;
        static bool showRenderSettings = true;

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Tools"))
            {
                ImGui::Checkbox("Settings", &showRenderSettings);
                ImGui::Checkbox("Debugging", &showDebugging);
                ImGui::Checkbox("Stats", &showStats);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (showStats)
        {
            if (ImGui::Begin("Stats", &showStats))
            {
                RenderUI_Stats();
            }
            ImGui::End();
        }

        if (showDebugging)
        {
            if (ImGui::Begin("Debugging", &showDebugging))
            {
                RenderUI_Debugging();
            }
            ImGui::End();
        }

        if (showRenderSettings)
        {
            if (ImGui::Begin("Settings", &showRenderSettings))
            {
                resetFrame = RenderUI_Settings();
            }
            ImGui::End();
        }
    }
    ImGui::EndFrame();

    ImGui::Render();

    memset(io.KeysDown, 0, sizeof(io.KeysDown));

    return resetFrame;
}
