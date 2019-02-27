#include "PCH.h"
#include "Demo.h"
#include "MeshLoader.h"

#include "../Core/Scene/Light/DirectionalLight.h"
#include "../Core/Scene/Object/SceneObject_Box.h"
#include "../Core/Scene/Object/SceneObject_Plane.h"

namespace helpers {

using namespace rt;
using namespace math;

bool LoadCustomScene(Scene& scene, rt::Camera& camera)
{
    // floor
    {
        auto material = Material::Create();
        material->debugName = "floor";
        material->SetBsdf("diffuse");
        material->baseColor = math::Vector4(0.9f, 0.9f, 0.9f);
        material->baseColor.texture = helpers::LoadTexture(gOptions.dataPath, "TEXTURES/default.bmp");
        material->roughness = 0.2f;
        material->Compile();

        const Float2 size(1000.0f, 1000.0f);
        const Float2 texScale(0.1f, 0.1f);
        std::unique_ptr<PlaneSceneObject> instance = std::make_unique<PlaneSceneObject>(size, texScale);
        instance->mDefaultMaterial = material;
        scene.AddObject(std::move(instance));
    }

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
        instance->mDefaultMaterial = material;
        instance->mTransform = rotationMatrix * translationMatrix;
        scene.AddObject(std::move(instance));
    }

    {
        const Vector4 lightColor(10.0f, 10.0f, 10.0f);
        const Vector4 lightDirection(1.1f, -0.7f, 0.9f);
        auto background = std::make_unique<DirectionalLight>(lightDirection, lightColor, 0.15f);
        scene.AddLight(std::move(background));
    }

    {
        Transform transform(Vector4(870.0f, 103.0f, -867.0f), Quaternion::FromEulerAngles(Float3(0.588f, -0.75f, 0.0f)));
        camera.SetTransform(transform);
    }

    return true;
}

} // namespace helpers