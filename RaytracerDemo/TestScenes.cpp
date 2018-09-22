#include "PCH.h"
#include "Demo.h"
#include "MeshLoader.h"

#include "../RaytracerLib/Mesh/Mesh.h"
#include "../RaytracerLib/Material/Material.h"
#include "../RaytracerLib/Scene/Light.h"
#include "../RaytracerLib/Scene/SceneObject_Mesh.h"
#include "../RaytracerLib/Scene/SceneObject_Sphere.h"
#include "../RaytracerLib/Scene/SceneObject_Box.h"

using namespace rt;
using namespace math;

namespace
{

void InitScene_Empty(rt::Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    RT_UNUSED(scene);
    RT_UNUSED(materials);
    RT_UNUSED(meshes);
    
    camera = CameraSetup();
}

void InitScene_Simple(rt::Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    // floor
    {
        auto mesh = helpers::CreatePlaneMesh(materials, 100.0f, 1.0f);
        SceneObjectPtr instance = std::make_unique<MeshSceneObject>(mesh.get());
        instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        meshes.push_back(std::move(mesh));
    }

    {
        auto material = std::make_unique<rt::Material>();
        material->debugName = "default";
        material->baseColor = math::Vector4(0.05f, 0.1f, 0.8f, 0.0f);
        material->roughness = 0.2f;
        material->transparent = false;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f, material.get());
        instance->mTransform.SetTranslation(Vector4(-1.5f, 0.5f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        auto material = std::make_unique<rt::Material>();
        material->debugName = "glass";
        material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
        material->roughness = 0.001f;
        material->transparent = true;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f, material.get());
        instance->mTransform.SetTranslation(Vector4(0.0f, 0.5f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        auto material = std::make_unique<rt::Material>();
        material->debugName = "mirror";
        material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
        material->roughness = 0.05f;
        material->metalness = 1.0f;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f, material.get());
        instance->mTransform.SetTranslation(Vector4(1.5f, 0.5f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        auto material = std::make_unique<rt::Material>();
        material->debugName = "wall";
        material->baseColor = math::Vector4(0.8f, 0.8f, 0.8f, 0.0f);
        material->roughness = 0.2f;
        material->metalness = 0.0f;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(10.0f, 10.0f, 1.0f, 0.0f), material.get());
        instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, -2.0f, 0.0f));
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        camera = CameraSetup();
        camera.position = Vector4(0.11f, 1.6f, 2.6f, 0.0f);
        camera.pitch = -0.3f;
        camera.yaw = -3.11f;
    }
}

void InitScene_Simple_BackgroundLight(rt::Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    InitScene_Simple(scene, materials, meshes, camera);

    SceneEnvironment env;
    env.backgroundColor = Vector4(2.0f, 2.0f, 2.0f, 0.0f);
    scene.SetEnvironment(env);
}

void InitScene_Simple_PointLight(rt::Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    InitScene_Simple(scene, materials, meshes, camera);

    SceneEnvironment env;
    env.backgroundColor = Vector4();
    scene.SetEnvironment(env);

    const Vector4 lightColor(10.0f, 10.0f, 10.0f, 0.0f);
    const Vector4 lightPosition(4.0f, 4.0f, 4.0f, 0.0f);
    scene.AddLight(std::make_unique<PointLight>(lightPosition, lightColor));
}

void InitScene_Simple_AreaLight(rt::Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    InitScene_Simple(scene, materials, meshes, camera);

    SceneEnvironment env;
    env.backgroundColor = Vector4();
    scene.SetEnvironment(env);

    {
        const Vector4 lightColor(10.0f, 10.0f, 10.0f, 0.0f);
        const Vector4 lightPosition(4.0f, 4.0f, 4.0f, 0.0f);
        const Vector4 lightEdge0(-1.0f, 0.0f, 1.0f, 0.0f);
        const Vector4 lightEdge1(0.0f, 1.0f, 0.0f, 0.0f);
        scene.AddLight(std::make_unique<AreaLight>(lightPosition, lightEdge0, lightEdge1, lightColor));

        /*
        SceneObjectPtr instance = std::make_unique<SphereSceneObject>(2.0f, material.get());
        instance->mTransform.SetTranslation(Vector4(4.0f, 4.0f, 4.0f, 0.0f));
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
        */
    }
}

void InitScene_Simple_DirectionalLight(rt::Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    InitScene_Simple(scene, materials, meshes, camera);

    SceneEnvironment env;
    env.backgroundColor = Vector4();
    scene.SetEnvironment(env);

    const Vector4 lightColor(1.0f, 1.0f, 1.0f, 0.0f);
    const Vector4 lightDirection(-1.0f, -1.0f, -1.0f, 0.0f);
    scene.AddLight(std::make_unique<DirectionalLight>(lightDirection, lightColor));
}

void InitScene_MultipleImportanceSamplingTest(rt::Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    // floor
    {
        auto mesh = helpers::CreatePlaneMesh(materials, 100.0f, 1.0f);
        SceneObjectPtr instance = std::make_unique<MeshSceneObject>(mesh.get());
        instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
        scene.AddObject(std::move(instance));
        meshes.push_back(std::move(mesh));
    }

    // lights
    {
        {
            const Vector4 lightColor(25.0f, 0.5f, 0.5f, 0.0f);
            const Vector4 lightPosition(2.0f, 0.0f, -1.0f, 0.0f);
            const Vector4 lightEdge0(0.0f, 2.0f, 0.0f, 0.0f);
            const Vector4 lightEdge1(0.02f, 0.0f, 0.0f, 0.0f);
            scene.AddLight(std::make_unique<AreaLight>(lightPosition, lightEdge0, lightEdge1, lightColor));
        }

        {
            const Vector4 lightColor(0.1f, 5.0f, 0.1f, 0.0f);
            const Vector4 lightPosition(0.0f, 0.0f, -1.0f, 0.0f);
            const Vector4 lightEdge0(0.0f, 2.0f, 0.0f, 0.0f);
            const Vector4 lightEdge1(0.1f, 0.0f, 0.0f, 0.0f);
            scene.AddLight(std::make_unique<AreaLight>(lightPosition, lightEdge0, lightEdge1, lightColor));
        }

        {
            const Vector4 lightColor(0.02f, 0.02f, 1.0f, 0.0f);
            const Vector4 lightPosition(-2.0f, 0.0f, -1.0f, 0.0f);
            const Vector4 lightEdge0(0.0f, 2.0f, 0.0f, 0.0f);
            const Vector4 lightEdge1(0.5f, 0.0f, 0.0f, 0.0f);
            scene.AddLight(std::make_unique<AreaLight>(lightPosition, lightEdge0, lightEdge1, lightColor));
        }
    }

    // mirrors
    {
        {
            auto material = std::make_unique<rt::Material>();
            material->debugName = "rougness_0";
            material->baseColor = math::Vector4(0.9f, 0.9f, 0.9f, 0.0f);
            material->roughness = 0.001f;
            material->metalness = 1.0f;
            material->Compile();

            SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(10.0f, 0.1f, 0.25f, 0.0f), material.get());
            instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
            scene.AddObject(std::move(instance));
            materials.push_back(std::move(material));
        }

        {
            auto material = std::make_unique<rt::Material>();
            material->debugName = "rougness_1";
            material->baseColor = math::Vector4(0.9f, 0.9f, 0.9f, 0.0f);
            material->roughness = 0.1f;
            material->metalness = 1.0f;
            material->Compile();

            SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(10.0f, 0.1f, 0.25f, 0.0f), material.get());
            instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, 0.5f, 0.0f));
            scene.AddObject(std::move(instance));
            materials.push_back(std::move(material));
        }

        {
            auto material = std::make_unique<rt::Material>();
            material->debugName = "rougness_2";
            material->baseColor = math::Vector4(0.9f, 0.9f, 0.9f, 0.0f);
            material->roughness = 0.25f;
            material->metalness = 1.0f;
            material->Compile();

            SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(10.0f, 0.1f, 0.25f, 0.0f), material.get());
            instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, 1.0f, 0.0f));
            scene.AddObject(std::move(instance));
            materials.push_back(std::move(material));
        }

        {
            auto material = std::make_unique<rt::Material>();
            material->debugName = "rougness_3";
            material->baseColor = math::Vector4(0.9f, 0.9f, 0.9f, 0.0f);
            material->roughness = 0.5f;
            material->metalness = 1.0f;
            material->Compile();

            SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(10.0f, 0.1f, 0.25f, 0.0f), material.get());
            instance->mTransform.SetTranslation(Vector4(0.0f, 0.0f, 1.5f, 0.0f));
            scene.AddObject(std::move(instance));
            materials.push_back(std::move(material));
        }
    }

    {
        camera = CameraSetup();
        camera.position = Vector4(0.11f, 0.8f, 3.0f, 0.0f);
        camera.pitch = -0.1f;
        camera.yaw = -3.11f;
    }

    SceneEnvironment env;
    env.backgroundColor = Vector4();
    scene.SetEnvironment(env);
}

void InitScene_Specular_Test(rt::Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    {
        auto material = std::make_unique<rt::Material>();
        material->debugName = "mirror";
        material->baseColor = math::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
        material->roughness = 0.1f;
        material->metalness = 0.0f;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f, material.get());
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }

    {
        const Vector4 lightColor(10000.0f, 10000.0f, 10000.0f, 0.0f);
        const Vector4 lightPosition(100.0f, 0.0f, 0.0f, 0.0f);
        scene.AddLight(std::make_unique<PointLight>(lightPosition, lightColor));
    }

    SceneEnvironment env;
    env.backgroundColor = Vector4();
    scene.SetEnvironment(env);

    {
        camera = CameraSetup();
        camera.position = Vector4(0.f, 0.0f, 2.6f, 0.0f);
        camera.pitch = -0.01f;
        camera.yaw = -3.14f;
    }
}

void InitScene_Stress_MillionObjects(rt::Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    RT_UNUSED(meshes);

    auto material = std::make_unique<rt::Material>();
    material->debugName = "default";
    material->baseColor = math::Vector4(0.2f, 0.5f, 0.8f, 0.0f);
    material->roughness = 0.2f;
    material->transparent = false;
    material->Compile();

    for (Int32 i = 0; i < 1000; ++i)
    {
        for (Int32 j = 0; j < 1000; ++j)
        {
            SceneObjectPtr instance = std::make_unique<SphereSceneObject>(0.5f, material.get());
            instance->mTransform.SetTranslation(Vector4((float)i, 0.0f, (float)j, 0.0f));
            scene.AddObject(std::move(instance));
        }
    }

    materials.push_back(std::move(material));

    {
        camera = CameraSetup();
        camera.position = Vector4(10.0f, 20.6f, 10.0f, 0.0f);
        camera.pitch = -0.6f;
        camera.yaw = RT_PI / 4.0f;
    }
}

} // namespace

void DemoWindow::RegisterTestScenes()
{
    mRegisteredScenes["Empty"] = InitScene_Empty;
    mRegisteredScenes["Specular Test"] = InitScene_Specular_Test;
    mRegisteredScenes["Simple + Background Light"] = InitScene_Simple_BackgroundLight;
    mRegisteredScenes["Simple + Point Light"] = InitScene_Simple_PointLight;
    mRegisteredScenes["Simple + Area Light"] = InitScene_Simple_AreaLight;
    mRegisteredScenes["Simple + Directional Light"] = InitScene_Simple_DirectionalLight;
    mRegisteredScenes["MIS Test"] = InitScene_MultipleImportanceSamplingTest;
    mRegisteredScenes["Stress (million spheres)"] = InitScene_Stress_MillionObjects;
}