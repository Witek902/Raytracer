#include "PCH.h"
#include "Demo.h"

#include "../RaytracerLib/Mesh/Mesh.h"
#include "../RaytracerLib/Material/Material.h"
#include "../RaytracerLib/Rendering/Context.h"
#include "../RaytracerLib/Rendering/ShadingData.h"
#include "../RaytracerLib/Traversal/TraversalContext.h"
#include "../RaytracerLib/Scene/SceneObject_Mesh.h"
#include "../RaytracerLib/Scene/SceneObject_Sphere.h"
#include "../RaytracerLib/Scene/SceneObject_Box.h"
#include "../RaytracerLib/Color/ColorHelpers.h"
#include "../RaytracerLib/Rendering/PathTracer.h"

#include "../External/imgui/imgui.h"

using namespace rt;
using namespace math;

void DemoWindow::RenderUI_Stats()
{
    ImGui::Text("Average render time: %.2f ms", 1000.0 * mAverageRenderDeltaTime);
    ImGui::Text("Minimum render time: %.2f ms", 1000.0 * mMinRenderDeltaTime);
    ImGui::Text("Total render time:   %.3f s", mTotalRenderTime);
    ImGui::Text("Post-process time:   %.2f ms", 1000.0 * mPostProcessDeltaTime);
    ImGui::Text("Samples rendered:    %u", mViewport->GetNumSamplesRendered());
    ImGui::Text("Frame number:        %u", mFrameNumber);

    ImGui::Separator();

    ImGui::Text("Delta time: %.2f ms", 1000.0 * mDeltaTime);

    ImGui::Separator();

    ImGui::Text("Avg. error: %.5f", mViewport->GetAverageError());

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    const RayTracingCounters& counters = mViewport->GetCounters();
    ImGui::Separator();
    ImGui::Text("Ray-box tests (total):  %.2fM", (float)counters.numRayBoxTests / 1000000.0f);
    ImGui::Text("Ray-box tests (passed): %.2fM", (float)counters.numPassedRayBoxTests / 1000000.0f);
    ImGui::Text("Ray-tri tests (total):  %.2fM", (float)counters.numRayTriangleTests / 1000000.0f);
    ImGui::Text("Ray-tri tests (passed): %.2fM", (float)counters.numPassedRayTriangleTests / 1000000.0f);
#endif // RT_ENABLE_INTERSECTION_COUNTERS
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
        ImGui::Text("Ray #%zu", i);

        const rt::PathDebugData::HitPointData data = mPathDebugData.data[i];
        ImGui::Text("  RayOrigin: [%f, %f, %f]", data.rayOrigin.x, data.rayDir.y, data.rayOrigin.z);
        ImGui::Text("  RayDir:    [%f, %f, %f]", data.rayDir.x, data.rayDir.y, data.rayDir.z);

        if (data.hitPoint.distance != FLT_MAX)
        {
            ImGui::Text("  Distance:    %f", data.hitPoint.distance);
            ImGui::Text("  Object ID:   %u", data.hitPoint.objectId);
            ImGui::Text("  Tri ID:      %u", data.hitPoint.triangleId);
            ImGui::Text("  Tri UV:      [%f, %f]", data.hitPoint.u, data.hitPoint.v);
            ImGui::Text("  Position:    [%f, %f, %f]", data.shadingData.position.x, data.shadingData.position.y, data.shadingData.position.z);
            ImGui::Text("  Normal:      [%f, %f, %f]", data.shadingData.normal.x, data.shadingData.normal.y, data.shadingData.normal.z);
            //ImGui::Text("  Tangent:   [%f, %f, %f]", data.shadingData.tangent.x, data.shadingData.tangent.y, data.shadingData.tangent.z);
            //ImGui::Text("  Tex coord: [%f, %f]", data.shadingData.texCoord.x, data.shadingData.texCoord.y);
            ImGui::Text("  Material:    %s", data.shadingData.material->debugName.c_str());
            ImGui::Text("  Throughput:  [%f, %f, %f, %f, %f, %f, %f, %f]",
                data.throughput.value[0], data.throughput.value[1], data.throughput.value[2], data.throughput.value[3],
                data.throughput.value[4], data.throughput.value[5], data.throughput.value[6], data.throughput.value[7]);
        }
    }

    const char* terminationReasonStr = "None";
    switch (mPathDebugData.terminationReason)
    {
    case PathTerminationReason::HitBackground: terminationReasonStr = "Hit background"; break;
    case PathTerminationReason::HitLight: terminationReasonStr = "Hit light"; break;
    case PathTerminationReason::Depth: terminationReasonStr = "Depth exeeded"; break;
    case PathTerminationReason::Throughput: terminationReasonStr = "Throughput too low"; break;
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
        const Uint32 numSamples = mViewport->GetNumSamplesRendered();

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

    if (ImGui::Button("Take screenshot"))
    {
        mViewport->GetFrontBuffer().SaveBMP("screenshot.bmp", true);
    }
}

bool DemoWindow::RenderUI_Settings_Rendering()
{
    bool resetFrame = false;

    resetFrame |= ImGui::Checkbox("Use debug renderer", &mUseDebugRenderer);
    if (mUseDebugRenderer)
    {
        int debugRenderingModeIndex = static_cast<int>(mDebugRenderer->mRenderingMode);

        const char* renderingModeItems[] =
        {
            "Material Base Color",
            "Material Emission Color",
            "Material Roughness",
            "Material Metalness",
            "Depth", "Position", "Normals", "Tangents", "Bitangents", "TexCoords", "TriangleID",
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
    int tileOrder = static_cast<int>(mRenderingParams.tileOrder);

    const char* traversalModeItems[] = { "Single", "SIMD", "Packet" };
    resetFrame |= ImGui::Combo("Traversal mode", &traversalModeIndex, traversalModeItems, IM_ARRAYSIZE(traversalModeItems));

    ImGui::SliderInt("Tile order", (int*)&tileOrder, 0, 8); // max 256x256 tile

    resetFrame |= ImGui::SliderInt("Max ray depth", (int*)&mRenderingParams.maxRayDepth, 0, 50);
    ImGui::SliderInt("Samples per pixel", (int*)&mRenderingParams.samplesPerPixel, 1, 64);
    resetFrame |= ImGui::SliderInt("Russian roulette depth", (int*)&mRenderingParams.minRussianRouletteDepth, 1, 64);
    resetFrame |= ImGui::SliderFloat("Antialiasing spread", &mRenderingParams.antiAliasingSpread, 0.0f, 3.0f);
    resetFrame |= ImGui::SliderFloat("Motion blur strength", &mRenderingParams.motionBlurStrength, 0.0f, 1.0f);
    
    mRenderingParams.traversalMode = static_cast<TraversalMode>(traversalModeIndex);
    mRenderingParams.tileOrder = static_cast<Uint8>(tileOrder);

    return resetFrame;
}

bool DemoWindow::RenderUI_Settings_Camera()
{
    bool resetFrame = false;

    const char* bokehTypeNames[] = { "Circle", "Hexagon", "Box" };
    int bokehTypeIndex = static_cast<int>(mCamera.mDOF.bokehType);

    resetFrame |= ImGui::InputFloat3("Position", &mCameraSetup.position.x, 3);

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

    resetFrame |= ImGui::Combo("Bokeh Shape", &bokehTypeIndex, bokehTypeNames, IM_ARRAYSIZE(bokehTypeNames));
    resetFrame |= ImGui::Checkbox("Enable lens distortions", &mCamera.enableBarellDistortion);
    resetFrame |= ImGui::SliderFloat("Barrel distortion", &mCamera.barrelDistortionConstFactor, 0.0f, 0.2f);
    resetFrame |= ImGui::SliderFloat("Lens distortion", &mCamera.barrelDistortionVariableFactor, 0.0f, 0.2f);

    mCamera.mDOF.bokehType = static_cast<BokehShape>(bokehTypeIndex);

    return resetFrame;
}

bool DemoWindow::RenderUI_Settings_PostProcess()
{
    ImGui::SliderFloat("Exposure", &mPostprocessParams.exposure, -8.0f, 8.0f, "%+.3f EV");
    ImGui::SliderFloat("Dithering", &mPostprocessParams.ditheringStrength, 0.0f, 0.1f);
    ImGui::ColorEdit3("Color filter", &mPostprocessParams.colorFilter.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

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

    // TODO
    //for (Uint32 i = 0; i < 256; ++i)
    //{
    //    io.KeysDown[i] = IsKeyPressed(i);
    //}

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
