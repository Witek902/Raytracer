#include "PCH.h"
#include "Demo.h"
#include "MeshLoader.h"

#include "../Core/Textures/NoiseTexture.h"
#include "../Core/Textures/CheckerboardTexture.h"
#include "../Core/Textures/MixTexture.h"

#include "../Core/Scene/Light/DirectionalLight.h"
#include "../Core/Scene/Light/BackgroundLight.h"

#include "../Core/Scene/Object/SceneObject_Box.h"
#include "../Core/Scene/Object/SceneObject_Plane.h"

namespace helpers {

using namespace rt;
using namespace math;

bool LoadCustomScene(Scene& scene, rt::Camera& camera)
{
    auto bitmapTextureA = helpers::LoadTexture(gOptions.dataPath, "TEXTURES/default.bmp");
    auto bitmapTextureB = helpers::LoadTexture(gOptions.dataPath, "TEXTURES/Portal/dirty4x4.bmp");
    auto noiseTexture = std::shared_ptr<ITexture>(new NoiseTexture(Vector4(1.0f), Vector4(0.0f)));
    auto texture = std::shared_ptr<ITexture>(new MixTexture(bitmapTextureA, bitmapTextureB, noiseTexture));

    // floor
    {
        auto material = Material::Create();
        material->debugName = "floor";
        material->SetBsdf("diffuse");
        material->baseColor = math::Vector4(0.9f, 0.9f, 0.9f);
        material->baseColor.texture = bitmapTextureA;
        //material->emission = math::Vector4(4.0f, 4.0f, 4.0f);
        //material->emission.texture = emissionTexture;
        //material->baseColor.texture = helpers::LoadTexture(gOptions.dataPath, "TEXTURES/default.bmp");
        material->roughness = 0.2f;
        material->Compile();

        const Float2 size(1000.0f, 1000.0f);
        const Float2 texScale(0.6f, 0.6f);
        std::unique_ptr<PlaneSceneObject> instance = std::make_unique<PlaneSceneObject>(size, texScale);
        instance->SetDefaultMaterial(material);
        scene.AddObject(std::move(instance));
    }

    /*
    Random random;

    for (Int32 i = 0; i < 2000; ++i)
    {
        auto material = Material::Create();
        material->debugName = "default";
        material->SetBsdf("diffuse");
        material->baseColor = Vector4(1.0f);
        material->roughness = random.GetFloat() * 0.6f;
        material->Compile();

        const Vector4 size = Vector4(0.5f, 0.5f, 0.5f) + random.GetVector4() * Vector4(5.0f, 10.0f, 5.0f);
        const Vector4 pos = random.GetVector4Bipolar() * 1000.0f;

        const Matrix4 translationMatrix = Matrix4::MakeTranslation(Vector4(pos.x, size.y / 2.0f, pos.y));
        const Matrix4 rotationMatrix = Quaternion::RotationY(pos.z * RT_2PI).ToMatrix4();

        SceneObjectPtr instance = std::make_unique<BoxSceneObject>(size);
        instance->SetDefaultMaterial(material);
        instance->SetTransform(rotationMatrix * translationMatrix);
        scene.AddObject(std::move(instance));
    }
    */

    {
        auto material = Material::Create();
        material->debugName = "default";
        material->SetBsdf("diffuse");
        material->baseColor = Vector4(0.9f);
        material->baseColor.texture = texture;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(1.0f));
        instance->SetDefaultMaterial(material);
        instance->SetTransform(Matrix4::MakeTranslation(Vector4(0.0f, 1.0f, 0.0f)));
        scene.AddObject(std::move(instance));
    }

    {
        auto material = Material::Create();
        material->debugName = "default";
        material->SetBsdf("diffuse");
        material->baseColor = Vector4(0.9f);
        material->baseColor.texture = bitmapTextureA;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(1.0f));
        instance->SetDefaultMaterial(material);
        instance->SetTransform(Matrix4::MakeTranslation(Vector4(-2.4f, 1.0f, 0.0f)));
        scene.AddObject(std::move(instance));
    }

    {
        auto material = Material::Create();
        material->debugName = "default";
        material->SetBsdf("diffuse");
        material->baseColor = Vector4(0.9f);
        material->baseColor.texture = bitmapTextureB;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(1.0f));
        instance->SetDefaultMaterial(material);
        instance->SetTransform(Matrix4::MakeTranslation(Vector4(2.4f, 1.0f, 0.0f)));
        scene.AddObject(std::move(instance));
    }

    {
        auto material = Material::Create();
        material->debugName = "default";
        material->SetBsdf("diffuse");
        material->baseColor = Vector4(0.9f);
        material->baseColor.texture = noiseTexture;
        material->Compile();

        SceneObjectPtr instance = std::make_unique<BoxSceneObject>(Vector4(1.0f));
        instance->SetDefaultMaterial(material);
        instance->SetTransform(Matrix4::MakeTranslation(Vector4(0.0f, 3.5f, 0.0f)));
        scene.AddObject(std::move(instance));
    }

    {
        const Vector4 lightColor(500.0f, 400.0f, 300.0f);
        const Vector4 lightDirection(1.1f, -0.7f, 0.9f);
        auto light = std::make_unique<DirectionalLight>(lightDirection, lightColor, 0.15f);
        scene.AddLight(std::move(light));
    }

    {
        const Vector4 lightColor(1.0f, 1.5f, 2.0f);
        auto background = std::make_unique<BackgroundLight>(lightColor);
        scene.AddLight(std::move(background));
    }

    {
        Transform transform(Vector4(2.0f, 6.0f, -5.0f), Quaternion::FromEulerAngles(Float3(0.588f, -0.75f, 0.0f)));
        camera.SetTransform(transform);
    }

    return true;
}

} // namespace helpers