#include "PCH.h"
#include "Demo.h"
#include "MeshLoader.h"

#include "../RaytracerLib/Mesh/Mesh.h"
#include "../RaytracerLib/Material/Material.h"
#include "../RaytracerLib/Scene/Light/PointLight.h"
#include "../RaytracerLib/Scene/Light/AreaLight.h"
#include "../RaytracerLib/Scene/Light/BackgroundLight.h"
#include "../RaytracerLib/Scene/Light/DirectionalLight.h"
#include "../RaytracerLib/Scene/Object/SceneObject_Mesh.h"
#include "../RaytracerLib/Scene/Object/SceneObject_Sphere.h"
#include "../RaytracerLib/Scene/Object/SceneObject_Box.h"
#include "../RaytracerLib/Scene/Object/SceneObject_Plane.h"

using namespace rt;
using namespace math;

namespace
{

void InitScene_Empty(Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    RT_UNUSED(scene);
    RT_UNUSED(materials);
    RT_UNUSED(meshes);

    camera = CameraSetup();
}

void InitScene_Background(Scene& scene, DemoWindow::Materials&, DemoWindow::Meshes&, CameraSetup& camera)
{
    const Vector4 lightColor(1.0f, 1.0f, 1.0f, 0.0f);

    auto background = std::make_unique<BackgroundLight>(lightColor);
    if (!gOptions.envMapPath.empty())
    {
        background->mTexture = helpers::LoadTexture(gOptions.dataPath, gOptions.envMapPath);
    }
    scene.SetBackgroundLight(std::move(background));

    {
        camera = CameraSetup();
        camera.position = Vector4(0.11f, 10.6f, 2.6f, 0.0f);
        camera.orientation.y = 0.198f;
        camera.orientation.x = 0.184f;
    }
}

void InitScene_Mesh(Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    {
        const Vector4 lightColor(2.0f, 2.0f, 2.0f, 0.0f);
        auto background = std::make_unique<BackgroundLight>(lightColor);
        if (!gOptions.envMapPath.empty())
        {
            background->mTexture = helpers::LoadTexture(gOptions.dataPath, gOptions.envMapPath);
        }
        scene.SetBackgroundLight(std::move(background));
    }

    {
        auto mesh = helpers::LoadMesh(gOptions.dataPath + "/" + gOptions.modelPath, materials);
        SceneObjectPtr instance = std::make_unique<MeshSceneObject>(mesh);
        instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        meshes.push_back(std::move(mesh));
    }

    {
        camera = CameraSetup();
        camera.position = Vector4(0.0f, 0.2f, 3.0f, 0.0f);
        camera.orientation.y = 0.05f;
        camera.orientation.x = 3.0f;
    }
}

void InitScene_Plane(Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    // floor
    {
        auto material = Material::Create();
        material->debugName = "floor";
        material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
        material->emission = math::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
        material->roughness = 0.8f;
        material->baseColor.texture = helpers::LoadTexture(gOptions.dataPath + "TEXTURES/", "default.bmp");
        material->Compile();

        SceneObjectPtr instance = std::make_unique<PlaneSceneObject>();
        instance->mDefaultMaterial = material;
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        const Vector4 lightColor(10.0f, 10.0f, 10.0f, 0.0f);
        const Vector4 lightPosition(0.0f, 1.0f, 0.0f, 0.0f);
        scene.AddLight(std::make_unique<PointLight>(lightPosition, lightColor));
    }

    {
        camera = CameraSetup();
        camera.position = Vector4(0.11f, 0.4f, 2.6f, 0.0f);
        camera.orientation.y = -0.5f;
        camera.orientation.x = -3.0f;
    }
}

void InitScene_Materials(Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    // floor
    {
        auto material = Material::Create();
        material->debugName = "floor";
        material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
        material->emission = math::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
        material->roughness = 0.8f;
        material->baseColor.texture = helpers::LoadTexture(gOptions.dataPath + "TEXTURES/", "default.bmp");
        material->Compile();

        SceneObjectPtr instance = std::make_unique<PlaneSceneObject>(Vector4(0.5f));
        instance->mDefaultMaterial = material;
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    const Uint32 numRows = 8;

    for (Uint32 i = 0; i <= numRows; ++i)
    {
        for (Uint32 j = 0; j <= numRows; ++j)
        {
            auto material = Material::Create();
            material->debugName = "mat_roughness" + std::to_string(i) + "_metalness" + std::to_string(j);
            material->baseColor = math::Vector4(0.9f, 0.1f, 0.1f, 0.0f);
            material->roughness = (Float)i / (Float)numRows;
            material->metalness = (Float)j / (Float)numRows;
            material->Compile();

            SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.4f);
            instance->mDefaultMaterial = material;
            instance->mTransform.SetTranslation(Vector4(1.0f * (Float)i, 0.4f, 1.0f * (Float)j, 0.0f));
            scene.AddObject(std::move(instance));
            materials.push_back(std::move(material));
        }
    }

    {
        const float size = 1.0f;

        const Vector4 lightColor = Vector4(20.0f, 20.0f, 20.0f, 0.0f) / (size * size);
        const Vector4 lightPosition(12.0f, 12.0f, 12.0f, 0.0f);
        const Vector4 lightEdge0(-size, 2.0f * size, -size, 0.0f);
        const Vector4 lightEdge1(-2.0f * size, 0.0f, 2.0f * size, 0.0f);
        scene.AddLight(std::make_unique<AreaLight>(lightPosition, lightEdge0, lightEdge1, lightColor));
    }

    {
        const Vector4 lightColor(0.5f, 0.5f, 0.5f, 0.0f);
        auto background = std::make_unique<BackgroundLight>(lightColor);
        if (!gOptions.envMapPath.empty())
        {
            background->mTexture = helpers::LoadTexture(gOptions.dataPath, gOptions.envMapPath);
        }
        scene.SetBackgroundLight(std::move(background));
    }

    {
        camera = CameraSetup();
        camera.position = Vector4(0.11f, 1.6f, 2.6f, 0.0f);
        camera.orientation.y = -0.3f;
        camera.orientation.x = -3.11f;
    }
}

void InitScene_Simple(Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    // floor
    {
        auto material = Material::Create();
        material->debugName = "floor";
        material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
        material->emission = math::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
        material->roughness = 0.8f;
        material->baseColor.texture = helpers::LoadTexture(gOptions.dataPath + "TEXTURES/", "default.bmp");
        material->Compile();

        SceneObjectPtr instance = std::make_unique<PlaneSceneObject>();
        instance->mDefaultMaterial = material;
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        auto material = Material::Create();
        material->debugName = "diffuse";
        material->baseColor = math::Vector4(0.05f, 0.1f, 0.8f, 0.0f);
        material->roughness = 0.0f;
        material->transparent = false;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f);
        instance->mDefaultMaterial = material;
        instance->mTransform.SetTranslation(Vector4(-1.5f, 0.5f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        auto material = Material::Create();
        material->debugName = "glossy";
        material->baseColor = math::Vector4(1.0f, 0.7f, 0.2f, 0.0f);
        material->roughness = 0.4f;
        material->metalness = 1.0f;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f);
        instance->mDefaultMaterial = material;
        instance->mTransform.SetTranslation(Vector4(0.0f, 0.5f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        auto material = Material::Create();
        material->debugName = "specular";
        material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
        material->roughness = 0.0f;
        material->metalness = 0.0f;
        material->transparent = true;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f);
        instance->mDefaultMaterial = material;
        instance->mTransform.SetTranslation(Vector4(1.5f, 0.5f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        auto mesh = helpers::LoadMesh(gOptions.dataPath + "/" + gOptions.modelPath, materials);
        SceneObjectPtr instance = std::make_unique<MeshSceneObject>(mesh);
        instance->mTransform.SetTranslation(Vector4(0.0f, 0.75f, -2.0f, 0.0f));
        scene.AddObject(std::move(instance));
        meshes.push_back(std::move(mesh));
    }

    //{
    //    auto material = Material::Create();
    //    material->debugName = "wall";
    //    material->baseColor = math::Vector4(0.8f, 0.8f, 0.8f, 0.0f);
    //    material->roughness = 0.05f;
    //    material->metalness = 0.0f;
    //    material->Compile();

    //    SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(10.0f, 10.0f, 1.0f, 0.0f), material.get());
    //    instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, -2.0f, 0.0f));
    //    scene.AddObject(std::move(instance));
    //    materials.push_back(std::move(material));
    //}

    {
        camera = CameraSetup();
        camera.position = Vector4(0.11f, 1.6f, 2.6f, 0.0f);
        camera.orientation.y = -0.3f;
        camera.orientation.x = -3.11f;
    }
}

void InitScene_Simple_BackgroundLight(Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    InitScene_Simple(scene, materials, meshes, camera);

    const Vector4 lightColor(1.0f, 1.0f, 1.0f, 0.0f);

    auto background = std::make_unique<BackgroundLight>(lightColor);
    if (!gOptions.envMapPath.empty())
    {
        background->mTexture = helpers::LoadTexture(gOptions.dataPath, gOptions.envMapPath);
    }
    scene.SetBackgroundLight(std::move(background));
}

void InitScene_Simple_PointLight(Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    InitScene_Simple(scene, materials, meshes, camera);

    const Vector4 lightColor(10.0f, 10.0f, 10.0f, 0.0f);
    const Vector4 lightPosition(4.0f, 4.0f, 4.0f, 0.0f);
    scene.AddLight(std::make_unique<PointLight>(lightPosition, lightColor));
}

void InitScene_Simple_AreaLight(Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    InitScene_Simple(scene, materials, meshes, camera);

    {
        const float size = 1.0f;

        const Vector4 lightColor = Vector4(8.0f, 8.0f, 8.0f, 0.0f) / (size * size);
        const Vector4 lightPosition(4.0f, 4.0f, 4.0f, 0.0f);
        const Vector4 lightEdge0(-size, 2.0f * size, -size, 0.0f);
        const Vector4 lightEdge1(-2.0f * size, 0.0f, 2.0f * size, 0.0f);
        scene.AddLight(std::make_unique<AreaLight>(lightPosition, lightEdge0, lightEdge1, lightColor));
    }
}

void InitScene_Simple_DirectionalLight(Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    InitScene_Simple(scene, materials, meshes, camera);

    const Vector4 lightColor(1.0f, 1.0f, 1.0f, 0.0f);
    const Vector4 lightDirection(-1.0f, -1.0f, -1.0f, 0.0f);
    scene.AddLight(std::make_unique<DirectionalLight>(lightDirection, lightColor));
}

void InitScene_MultipleImportanceSamplingTest(Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    // floor
    {
        auto mesh = helpers::CreatePlane(materials, 100.0f, 1.0f);
        SceneObjectPtr instance = std::make_unique<MeshSceneObject>(mesh);
        instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        meshes.push_back(std::move(mesh));
    }

    // lights
    {
        {
            const Vector4 lightColor(25.0f, 0.05f, 0.05f, 0.0f);
            const Vector4 lightPosition(2.0f, 0.0f, -1.0f, 0.0f);
            const Vector4 lightEdge0(0.0f, 3.0f, 0.0f, 0.0f);
            const Vector4 lightEdge1(0.02f, 0.0f, 0.0f, 0.0f);
            scene.AddLight(std::make_unique<AreaLight>(lightPosition, lightEdge0, lightEdge1, lightColor));
        }

        {
            const Vector4 lightColor(0.01f, 5.0f, 0.01f, 0.0f);
            const Vector4 lightPosition(0.0f, 0.0f, -1.0f, 0.0f);
            const Vector4 lightEdge0(0.0f, 3.0f, 0.0f, 0.0f);
            const Vector4 lightEdge1(0.1f, 0.0f, 0.0f, 0.0f);
            scene.AddLight(std::make_unique<AreaLight>(lightPosition, lightEdge0, lightEdge1, lightColor));
        }

        {
            const Vector4 lightColor(0.002f, 0.002f, 1.0f, 0.0f);
            const Vector4 lightPosition(-2.0f, 0.0f, -1.0f, 0.0f);
            const Vector4 lightEdge0(0.0f, 3.0f, 0.0f, 0.0f);
            const Vector4 lightEdge1(0.5f, 0.0f, 0.0f, 0.0f);
            scene.AddLight(std::make_unique<AreaLight>(lightPosition, lightEdge0, lightEdge1, lightColor));
        }
    }

    // mirrors
    {
        {
            auto material = Material::Create();
            material->debugName = "rougness_0";
            material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
            material->roughness = 0.0f;
            material->metalness = 1.0f;
            material->Compile();

            SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(10.0f, 0.02f, 0.25f, 0.0f));
            instance->mDefaultMaterial = material;
            instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
            scene.AddObject(std::move(instance));
            materials.push_back(std::move(material));
        }

        {
            auto material = Material::Create();
            material->debugName = "rougness_1";
            material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
            material->roughness = 0.1f;
            material->metalness = 1.0f;
            material->Compile();

            SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(10.0f, 0.02f, 0.25f, 0.0f));
            instance->mDefaultMaterial = material;
            instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, 0.5f, 0.0f));
            scene.AddObject(std::move(instance));
            materials.push_back(std::move(material));
        }

        {
            auto material = Material::Create();
            material->debugName = "rougness_2";
            material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
            material->roughness = 0.25f;
            material->metalness = 1.0f;
            material->Compile();

            SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(10.0f, 0.02f, 0.25f, 0.0f));
            instance->mDefaultMaterial = material;
            instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, 1.0f, 0.0f));
            scene.AddObject(std::move(instance));
            materials.push_back(std::move(material));
        }

        {
            auto material = Material::Create();
            material->debugName = "rougness_3";
            material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
            material->roughness = 0.5f;
            material->metalness = 1.0f;
            material->Compile();

            SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(10.0f, 0.02f, 0.25f, 0.0f));
            instance->mDefaultMaterial = material;
            instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, 1.5f, 0.0f));
            scene.AddObject(std::move(instance));
            materials.push_back(std::move(material));
        }
    }

    {
        camera = CameraSetup();
        camera.position = Vector4(0.11f, 0.8f, 3.5f, 0.0f);
        camera.orientation.y = -0.1f;
        camera.orientation.x = -3.11f;
    }
}

void InitScene_Furnace_Test(Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes&, CameraSetup& camera)
{
    {
        auto material = Material::Create();
        material->debugName = "mirror";
        material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
        material->roughness = 0.9f;
        material->metalness = 1.0f;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f);
        instance->mDefaultMaterial = material;
        instance->mTransform.SetTranslation(Vector4(0.0f, -0.5f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        auto material = Material::Create();
        material->debugName = "diffuse";
        material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
        material->roughness = 0.0f;
        material->metalness = 0.0f;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f);
        instance->mDefaultMaterial = material;
        instance->mTransform.SetTranslation(Vector4(0.0f, 0.5f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        const Vector4 lightColor(1.0f, 1.0f, 1.0f, 0.0f);
        scene.SetBackgroundLight(std::make_unique<BackgroundLight>(lightColor));
    }

    {
        camera = CameraSetup();
        camera.position = Vector4(0.f, 0.0f, 3.0f, 0.0f);
        camera.orientation.y = -0.01f;
        camera.orientation.x = -3.14f;
    }
}

void InitScene_Specular_Test(Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes&, CameraSetup& camera)
{
    {
        auto material = Material::Create();
        material->debugName = "mirror";
        material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
        material->roughness = 0.9f;
        material->metalness = 1.0f;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f);
        instance->mDefaultMaterial = material;
        instance->mTransform.SetTranslation(Vector4(0.0f, -0.5f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        auto material = Material::Create();
        material->debugName = "diffuse";
        material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
        material->roughness = 0.0f;
        material->metalness = 0.0f;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f);
        instance->mDefaultMaterial = material;
        instance->mTransform.SetTranslation(Vector4(0.0f, 0.5f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        const Vector4 lightColor(10000.0f, 10000.0f, 10000.0f, 0.0f);
        const Vector4 lightPosition(100.0f, 0.0f, 0.0f, 0.0f);
        scene.AddLight(std::make_unique<PointLight>(lightPosition, lightColor));
    }

    {
        camera = CameraSetup();
        camera.position = Vector4(0.f, 0.0f, 3.0f, 0.0f);
        camera.orientation.y = -0.01f;
        camera.orientation.x = -3.14f;
    }
}

void InitScene_Stress_MillionObjects(Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    RT_UNUSED(meshes);

    auto material = Material::Create();
    material->debugName = "default";
    material->baseColor = math::Vector4(0.2f, 0.5f, 0.8f, 0.0f);
    material->roughness = 0.2f;
    material->transparent = false;
    material->Compile();

    for (Int32 i = 0; i < 1000; ++i)
    {
        for (Int32 j = 0; j < 1000; ++j)
        {
            SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f);
            instance->mDefaultMaterial = material;
            instance->mTransform.SetTranslation(Vector4((float)i, 0.0f, (float)j, 0.0f));
            scene.AddObject(std::move(instance));
        }
    }

    materials.push_back(std::move(material));

    {
        const Vector4 lightColor(1.0f, 1.0f, 1.0f, 0.0f);
        auto background = std::make_unique<BackgroundLight>(lightColor);
        if (!gOptions.envMapPath.empty())
        {
            background->mTexture = helpers::LoadTexture(gOptions.dataPath, gOptions.envMapPath);
        }
        scene.SetBackgroundLight(std::move(background));
    }

    {
        camera = CameraSetup();
        camera.position = Vector4(10.0f, 20.6f, 10.0f, 0.0f);
        camera.orientation.y = -0.6f;
        camera.orientation.x = RT_PI / 4.0f;
    }
}

} // namespace

void DemoWindow::RegisterTestScenes()
{
    mRegisteredScenes["Empty"] = InitScene_Empty;
    mRegisteredScenes["Background"] = InitScene_Background;
    mRegisteredScenes["Plane"] = InitScene_Plane;
    mRegisteredScenes["Materials"] = InitScene_Materials;
    mRegisteredScenes["Furnace Test"] = InitScene_Furnace_Test;
    mRegisteredScenes["Specular Test"] = InitScene_Specular_Test;
    mRegisteredScenes["Mesh"] = InitScene_Mesh;
    mRegisteredScenes["Simple + Background Light"] = InitScene_Simple_BackgroundLight;
    mRegisteredScenes["Simple + Point Light"] = InitScene_Simple_PointLight;
    mRegisteredScenes["Simple + Area Light"] = InitScene_Simple_AreaLight;
    mRegisteredScenes["Simple + Directional Light"] = InitScene_Simple_DirectionalLight;
    mRegisteredScenes["MIS Test"] = InitScene_MultipleImportanceSamplingTest;
    mRegisteredScenes["Stress (million spheres)"] = InitScene_Stress_MillionObjects;
}