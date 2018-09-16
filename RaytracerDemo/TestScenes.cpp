#include "PCH.h"
#include "Demo.h"
#include "MeshLoader.h"

#include "../RaytracerLib/Mesh/Mesh.h"
#include "../RaytracerLib/Material/Material.h"
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
        material->baseColor = math::Vector4(0.2f, 0.5f, 0.8f, 0.0f);
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
        material->roughness = 0.002f;
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

void InitScene_Simple_AreaLight(rt::Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
    InitScene_Simple(scene, materials, meshes, camera);

    SceneEnvironment env;
    env.backgroundColor = Vector4();
    scene.SetEnvironment(env);

    {
        auto material = std::make_unique<rt::Material>();
        material->debugName = "light";
        material->emissionColor = math::Vector4(10.0f, 10.0f, 10.0f, 0.0f);
        material->light = true;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<SphereSceneObject>(2.0f, material.get());
        instance->mTransform.SetTranslation(Vector4(4.0f, 4.0f, 4.0f, 0.0f));
        scene.AddObject(std::move(instance));
        materials.push_back(std::move(material));
    }
}

void InitScene_Stress_MillionObjects(rt::Scene& scene, DemoWindow::Materials& materials, DemoWindow::Meshes& meshes, CameraSetup& camera)
{
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
    mRegisteredScenes["Simple + Background Light"] = InitScene_Simple_BackgroundLight;
    mRegisteredScenes["Simple + Area Light"] = InitScene_Simple_AreaLight;
    mRegisteredScenes["Stress (million spheres)"] = InitScene_Stress_MillionObjects;
}