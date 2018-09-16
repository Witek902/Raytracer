#include "PCH.h"
#include "Demo.h"
#include "Renderer.h"

#include "../RaytracerLib/Mesh/Mesh.h"
#include "../RaytracerLib/Material/Material.h"
#include "../RaytracerLib/Rendering/Context.h"
#include "../RaytracerLib/Rendering/ShadingData.h"
#include "../RaytracerLib/Traversal/TraversalContext.h"
#include "../RaytracerLib/Scene/SceneObject_Mesh.h"
#include "../RaytracerLib/Scene/SceneObject_Sphere.h"
#include "../RaytracerLib/Scene/SceneObject_Box.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>

using namespace rt;
using namespace math;

void DemoWindow::RenderUI_Stats()
{
    ImGui::Text("Average render time: %.2f ms", 1000.0 * mAverageRenderDeltaTime);
    ImGui::Text("Minimum render time: %.2f ms", 1000.0 * mMinRenderDeltaTime);
    ImGui::Text("Total render time:   %.3f s", mTotalRenderTime);
    ImGui::Text("Samples rendered:    %u", mViewport->GetNumSamplesRendered());
    ImGui::Text("Frame number:        %u", mFrameNumber);

    ImGui::Separator();

    ImGui::Text("Delta time: %.2f ms", 1000.0 * mDeltaTime);

    ImGui::Separator();

    const RayTracingCounters& counters = mViewport->GetCounters();
#ifdef RT_ENABLE_INTERSECTION_COUNTERS
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
        for (size_t i = 0; i < mPathDebugData.data.size(); ++i)
        {
            ImGui::Text("Ray #%zu", i);

            const rt::PathDebugData::HitPointData data = mPathDebugData.data[i];
            ImGui::Text("  RayOrigin: [%f, %f, %f]", data.rayOrigin.x, data.rayDir.y, data.rayOrigin.z);
            ImGui::Text("  RayDir:    [%f, %f, %f]", data.rayDir.x, data.rayDir.y, data.rayDir.z);

            if (data.hitPoint.distance != FLT_MAX)
            {
                ImGui::Text("  Distance:  %f", data.hitPoint.distance);
                ImGui::Text("  Object ID: %u", data.hitPoint.objectId);
                ImGui::Text("  Tri ID:    %u", data.hitPoint.triangleId);
                ImGui::Text("  Tri UV:    [%f, %f]", data.hitPoint.u, data.hitPoint.v);
                ImGui::Text("  Position:  [%f, %f, %f]", data.shadingData.position.x, data.shadingData.position.y, data.shadingData.position.z);
                ImGui::Text("  Normal:    [%f, %f, %f]", data.shadingData.normal.x, data.shadingData.normal.y, data.shadingData.normal.z);
                //ImGui::Text("  Tangent:   [%f, %f, %f]", data.shadingData.tangent.x, data.shadingData.tangent.y, data.shadingData.tangent.z);
                //ImGui::Text("  Tex coord: [%f, %f]", data.shadingData.texCoord.x, data.shadingData.texCoord.y);
                ImGui::Text("  Material:  %s", data.shadingData.material->debugName);
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

        ImGui::TreePop();
    }
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
}

bool DemoWindow::RenderUI_Settings_Rendering()
{
    bool resetFrame = false;

    int renderingModeIndex = static_cast<int>(mRenderingParams.renderingMode);
    int traversalModeIndex = static_cast<int>(mRenderingParams.traversalMode);
    int tileOrder = static_cast<int>(mRenderingParams.tileOrder);

    const char* renderingModeItems[] =
    {
        "Regular", "BaseColor",
        "Depth", "Position", "Normals", "Tangents", "Bitangents", "TexCoords", "TriangleID",
#ifdef RT_ENABLE_INTERSECTION_COUNTERS
        "RayBoxIntersection", "RayBoxIntersectionPassed", "RayTriIntersection", "RayTriIntersectionPassed",
#endif // RT_ENABLE_INTERSECTION_COUNTERS
    };
    resetFrame |= ImGui::Combo("Rendering mode", &renderingModeIndex, renderingModeItems, IM_ARRAYSIZE(renderingModeItems));

    const char* traversalModeItems[] = { "Single", "SIMD", "Packet" };
    resetFrame |= ImGui::Combo("Traversal mode", &traversalModeIndex, traversalModeItems, IM_ARRAYSIZE(traversalModeItems));

    ImGui::SliderInt("Tile order", (int*)&tileOrder, 0, 8); // max 256x256 tile

    resetFrame |= ImGui::SliderInt("Max ray depth", (int*)&mRenderingParams.maxRayDepth, 1, 50);
    ImGui::SliderInt("Samples per pixel", (int*)&mRenderingParams.samplesPerPixel, 1, 64);
    resetFrame |= ImGui::SliderInt("Russian roulette depth", (int*)&mRenderingParams.minRussianRouletteDepth, 1, 64);
    resetFrame |= ImGui::SliderFloat("Antialiasing spread", &mRenderingParams.antiAliasingSpread, 0.0f, 3.0f);
    resetFrame |= ImGui::SliderFloat("Motion blur strength", &mRenderingParams.motionBlurStrength, 0.0f, 1.0f);

    mRenderingParams.renderingMode = static_cast<RenderingMode>(renderingModeIndex);
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
    resetFrame |= ImGui::SliderFloat("Focal distance", &mCamera.mDOF.focalPlaneDistance, 0.1f, 1000.0f, "%.3f", 2.0f);
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
    changed |= ImGui::ColorEdit3("Emission color", &mSelectedMaterial->emissionColor[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
    changed |= ImGui::ColorEdit3("Base color", &mSelectedMaterial->baseColor[0], ImGuiColorEditFlags_Float);
    changed |= ImGui::SliderFloat("Roughness", &mSelectedMaterial->roughness, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Metalness", &mSelectedMaterial->metalness, 0.0f, 1.0f);
    changed |= ImGui::Checkbox("Dispersive", &mSelectedMaterial->isDispersive);

    if (mSelectedMaterial->isDispersive)
    {
        changed |= ImGui::InputFloat3("B", mSelectedMaterial->dispersionParams.B);
        changed |= ImGui::InputFloat3("C", mSelectedMaterial->dispersionParams.C);
    }

    if (mSelectedMaterial->metalness > 0.0f || !mSelectedMaterial->isDispersive)
    {
        changed |= ImGui::SliderFloat("Refractive index", &mSelectedMaterial->IoR, 0.0f, 6.0f);
    }

    if (mSelectedMaterial->metalness > 0.0f)
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
    ImGui_ImplDX11_NewFrame();

    Uint32 width, height;
    GetSize(width, height);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = (float)mDeltaTime;
    io.KeyCtrl = IsKeyPressed(VK_CONTROL);
    io.KeyShift = IsKeyPressed(VK_SHIFT);
    io.KeyAlt = IsKeyPressed(VK_MENU);

    for (Uint32 i = 0; i < 256; ++i)
    {
        io.KeysDown[i] = IsKeyPressed(i);
    }

    ImGui::NewFrame();
    {
        static bool showStats = false;
        static bool showDebugging = false;
        static bool showRenderSettings = false;

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
