#include "PCH.h"
#include "Demo.h"

#include "../Core/Mesh/Mesh.h"
#include "../Core/Material/Material.h"
#include "../Core/Rendering/Context.h"
#include "../Core/Rendering/ShadingData.h"
#include "../Core/Traversal/TraversalContext.h"
#include "../Core/Scene/Object/SceneObject_Mesh.h"
#include "../Core/Scene/Object/SceneObject_Sphere.h"
#include "../Core/Scene/Object/SceneObject_Box.h"
#include "../Core/Color/ColorHelpers.h"
#include "../Core/Rendering/PathTracer.h"

#include "../External/imgui/imgui.h"

using namespace rt;
using namespace math;

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
    ImGui::Text("Ray-box tests (total)"); ImGui::NextColumn();
    ImGui::Text("%.2fM", (float)counters.numRayBoxTests / 1000000.0f); ImGui::NextColumn();

    ImGui::Text("Ray-box tests (passed)"); ImGui::NextColumn();
    ImGui::Text("%.2fM", (float)counters.numPassedRayBoxTests / 1000000.0f); ImGui::NextColumn();

    ImGui::Text("Ray-tri tests (total)"); ImGui::NextColumn();
    ImGui::Text("%.2fM", (float)counters.numRayTriangleTests / 1000000.0f); ImGui::NextColumn();

    ImGui::Text("Ray-tri tests (passed)"); ImGui::NextColumn();
    ImGui::Text("%.2fM", (float)counters.numPassedRayTriangleTests / 1000000.0f); ImGui::NextColumn();
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
}

void DemoWindow::RenderUI_Debugging_Path()
{
    for (size_t i = 0; i < mPathDebugData.data.size(); ++i)
    {
        ImGui::Separator();
        ImGui::Text("Ray #%zu", i);

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

            ImGui::Text("Position"); ImGui::NextColumn();
            ImGui::Text("[%f, %f, %f]", data.shadingData.position.x, data.shadingData.position.y, data.shadingData.position.z); ImGui::NextColumn();

            ImGui::Text("Normal"); ImGui::NextColumn();
            ImGui::Text("[%f, %f, %f]", data.shadingData.normal.x, data.shadingData.normal.y, data.shadingData.normal.z); ImGui::NextColumn();

            //ImGui::Text("Tangent"); ImGui::NextColumn();
            //ImGui::Text("[%f, %f, %f]", data.shadingData.tangent.x, data.shadingData.tangent.y, data.shadingData.tangent.z); ImGui::NextColumn();

            //ImGui::Text("Tex coord"); ImGui::NextColumn();
            //ImGui::Text("[%f, %f]", data.shadingData.texCoord.x, data.shadingData.texCoord.y); ImGui::NextColumn();

            ImGui::Text("Material"); ImGui::NextColumn();
            ImGui::Text("%s", data.shadingData.material->debugName.c_str()); ImGui::NextColumn();

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
        hdrColor = mViewport->GetSumBuffer().GetPixel(x, y, true) / static_cast<Float>(numSamples);
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

void DemoWindow::RenderUI_Settings()
{
    bool resetFrame = false;

    if (ImGui::TreeNode("Rendering"))
    {
        resetFrame |= RenderUI_Settings_Rendering();
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

    if (mSelectedMaterial)
    {
        if (ImGui::TreeNode("Material", "Material (%s)", mSelectedMaterial->debugName.c_str()))
        {
            resetFrame |= RenderUI_Settings_Material();
            ImGui::TreePop();
        }
    }

    if (resetFrame)
    {
        ResetFrame();
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
            const Float colorScale = 1.0f / (Float)mViewport->GetProgress().passesFinished;
            mViewport->GetSumBuffer().SaveEXR("screenshot.exr", colorScale);
        }
    }
}

bool DemoWindow::RenderUI_Settings_Rendering()
{
    bool resetFrame = false;

    {
        Uint32 maxThreads = std::thread::hardware_concurrency();
        ImGui::SliderInt("Threads", (int*)&mRenderingParams.numThreads, 1, 2 * maxThreads);
    }

    resetFrame |= ImGui::Checkbox("Use debug renderer", &mUseDebugRenderer);
    if (mUseDebugRenderer)
    {
        int debugRenderingModeIndex = static_cast<int>(mDebugRenderer->mRenderingMode);

        const char* renderingModeItems[] =
        {
            "TriangleID", "Depth", "Position", "Normals", "Tangents", "Bitangents", "TexCoords",
            "Material Base Color",
            "Material Emission Color",
            "Material Roughness",
            "Material Metalness",
#ifdef RT_ENABLE_INTERSECTION_COUNTERS
            "RayBoxIntersection", "RayBoxIntersectionPassed", "RayTriIntersection", "RayTriIntersectionPassed",
#endif // RT_ENABLE_INTERSECTION_COUNTERS
        };
        resetFrame |= ImGui::Combo("Rendering mode", &debugRenderingModeIndex, renderingModeItems, IM_ARRAYSIZE(renderingModeItems));
        mDebugRenderer->mRenderingMode = static_cast<DebugRenderingMode>(debugRenderingModeIndex);
    }
    else
    {
        rt::PathTracer* pathTracer = (PathTracer*)mRenderer.get();
        resetFrame |= ImGui::Checkbox("Light sampling", &pathTracer->mSampleLights);
    }
  
    int traversalModeIndex = static_cast<int>(mRenderingParams.traversalMode);
    int tileOrder = static_cast<int>(mRenderingParams.tileSize);

    const char* traversalModeItems[] = { "Single", "Packet" };
    resetFrame |= ImGui::Combo("Traversal mode", &traversalModeIndex, traversalModeItems, IM_ARRAYSIZE(traversalModeItems));

    ImGui::SliderInt("Tile size", (int*)&tileOrder, 1, 1024);

    resetFrame |= ImGui::SliderInt("Max ray depth", (int*)&mRenderingParams.maxRayDepth, 0, 50);
    ImGui::SliderInt("Samples per pixel", (int*)&mRenderingParams.samplesPerPixel, 1, 64);
    resetFrame |= ImGui::SliderInt("Russian roulette depth", (int*)&mRenderingParams.minRussianRouletteDepth, 1, 64);
    resetFrame |= ImGui::SliderFloat("Antialiasing spread", &mRenderingParams.antiAliasingSpread, 0.0f, 3.0f);
    resetFrame |= ImGui::SliderFloat("Motion blur strength", &mRenderingParams.motionBlurStrength, 0.0f, 1.0f);
    
    mRenderingParams.traversalMode = static_cast<TraversalMode>(traversalModeIndex);
    mRenderingParams.tileSize = static_cast<Uint16>(tileOrder);

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
        resetFrame |= ImGui::InputFloat3("Orientation", &mCameraSetup.orientation.x, 3);

        resetFrame |= ImGui::InputFloat3("Velocity", &mCameraSetup.linearVelocity.x, 3);
        resetFrame |= ImGui::InputFloat3("Angular velocity", &mCameraSetup.angularVelocity.x, 3);

        ImGui::TreePop(); // Transform
    }

    if (ImGui::TreeNode("Lens"))
    {
        resetFrame |= ImGui::SliderFloat("Field of view", &mCameraSetup.fov, 0.5f, 120.0f);
        resetFrame |= ImGui::SliderFloat("Aperture", &mCamera.mDOF.aperture, 0.0f, 0.1f);
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

        const char* bokehTypeNames[] = { "Circle", "Hexagon", "Box", "n-gon" };
        int bokehTypeIndex = static_cast<int>(mCamera.mDOF.bokehType);

        resetFrame |= ImGui::Combo("Bokeh Shape", &bokehTypeIndex, bokehTypeNames, IM_ARRAYSIZE(bokehTypeNames));
        if (mCamera.mDOF.bokehType == BokehShape::NGon)
        {
            resetFrame |= ImGui::SliderInt("No. of blades", (int*)&mCamera.mDOF.apertureBlades, 3, 20);
        }

        resetFrame |= ImGui::Checkbox("Enable lens distortions", &mCamera.enableBarellDistortion);
        resetFrame |= ImGui::SliderFloat("Barrel distortion", &mCamera.barrelDistortionConstFactor, 0.0f, 0.2f);
        resetFrame |= ImGui::SliderFloat("Lens distortion", &mCamera.barrelDistortionVariableFactor, 0.0f, 0.2f);

        mCamera.mDOF.bokehType = static_cast<BokehShape>(bokehTypeIndex);

        ImGui::TreePop(); // Lens
    }

    return resetFrame;
}

bool DemoWindow::RenderUI_Settings_PostProcess()
{
    bool changed = false;

    changed |= ImGui::SliderFloat("Exposure", &mPostprocessParams.exposure, -8.0f, 8.0f, "%+.3f EV");
    changed |= ImGui::SliderFloat("Dithering", &mPostprocessParams.ditheringStrength, 0.0f, 0.1f);
    changed |= ImGui::ColorEdit3("Color filter", &mPostprocessParams.colorFilter.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

    if (changed)
    {
        mViewport->SetPostprocessParams(mPostprocessParams);
    }

    return false;
}

bool DemoWindow::RenderUI_Settings_Object()
{
    bool positionChanged = false;

    {
        Float3 position = mSelectedObject->mTransform.GetTranslation().ToFloat3();
        if (ImGui::InputFloat3("Position", &position.x, 2, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            mSelectedObject->mTransform.SetTranslation(Vector4(position));
            positionChanged = true;
        }
    }

    {
        Float3 orientation;
        mSelectedObject->mTransform.GetRotation().ToAngles(orientation.x, orientation.y, orientation.z);
        orientation *= 180.0f / RT_PI;
        if (ImGui::InputFloat3("Orientation", &orientation.x, 2, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            orientation *= RT_PI / 180.0f;
            mSelectedObject->mTransform.SetRotation(Quaternion::FromAngles(orientation.x, orientation.y, orientation.z));
            positionChanged = true;
        }
    }

    {
        Float3 velocity = mSelectedObject->mLinearVelocity.ToFloat3();
        if (ImGui::InputFloat3("Linear Velocity", &velocity.x, 2, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            mSelectedObject->mLinearVelocity = Vector4(velocity);
            positionChanged = true;
        }
    }

    {
        Float3 angularVelocity;
        mSelectedObject->mAngularVelocity.ToAngles(angularVelocity.x, angularVelocity.y, angularVelocity.z);
        angularVelocity *= 180.0f / RT_PI;
        if (ImGui::InputFloat3("Angular Velocity", &angularVelocity.x, 2, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            angularVelocity *= RT_PI / 180.0f;
            mSelectedObject->mAngularVelocity = Quaternion::FromAngles(angularVelocity.x, angularVelocity.y, angularVelocity.z);
            positionChanged = true;
        }
    }

    if (positionChanged)
    {
        mScene->BuildBVH();
    }

    return positionChanged;
}

bool DemoWindow::RenderUI_Settings_Material()
{
    bool changed = false;

    changed |= ImGui::Checkbox("Transparent", &mSelectedMaterial->transparent);
    changed |= ImGui::ColorEdit3("Emission color", &mSelectedMaterial->emission.baseValue.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
    changed |= ImGui::ColorEdit3("Base color", &mSelectedMaterial->baseColor.baseValue.x, ImGuiColorEditFlags_Float);
    changed |= ImGui::SliderFloat("Roughness", &mSelectedMaterial->roughness.baseValue, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Metalness", &mSelectedMaterial->metalness.baseValue, 0.0f, 1.0f);
    changed |= ImGui::Checkbox("Dispersive", &mSelectedMaterial->isDispersive);

    if (mSelectedMaterial->isDispersive)
    {
        changed |= ImGui::InputFloat3("B", mSelectedMaterial->dispersionParams.B);
        changed |= ImGui::InputFloat3("C", mSelectedMaterial->dispersionParams.C);
    }

    if (mSelectedMaterial->metalness.baseValue > 0.0f || !mSelectedMaterial->isDispersive)
    {
        changed |= ImGui::SliderFloat("Refractive index", &mSelectedMaterial->IoR, 0.0f, 6.0f);
    }

    if (mSelectedMaterial->metalness.baseValue > 0.0f)
    {
        changed |= ImGui::SliderFloat("Extinction coefficient", &mSelectedMaterial->K, 0.0f, 10.0f);
    }

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

void DemoWindow::RenderUI()
{
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
            if (ImGui::BeginMenu("Scene"))
            {
                // TODO opening/saving scene

                for (const auto& iter : mRegisteredScenes)
                {
                    if (ImGui::MenuItem(iter.first.c_str()))
                    {
                        SwitchScene(iter.second);
                    }
                }

                ImGui::EndMenu();
            }

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
                RenderUI_Settings();
            }
            ImGui::End();
        }
    }
    ImGui::EndFrame();

    ImGui::Render();

    memset(io.KeysDown, 0, sizeof(io.KeysDown));
}
