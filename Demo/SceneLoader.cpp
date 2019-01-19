#include "PCH.h"
#include "SceneLoader.h"
#include "Demo.h"
#include "MeshLoader.h"

#include "../Core/Utils/Bitmap.h"
#include "../Core/Utils/Logger.h"
#include "../Core/Math/Vector4.h"
#include "../Core/Mesh/Mesh.h"
#include "../Core/Material/Material.h"
#include "../Core/Scene/Light/PointLight.h"
#include "../Core/Scene/Light/AreaLight.h"
#include "../Core/Scene/Light/BackgroundLight.h"
#include "../Core/Scene/Light/DirectionalLight.h"
#include "../Core/Scene/Light/SphereLight.h"
#include "../Core/Scene/Object/SceneObject_Mesh.h"
#include "../Core/Scene/Object/SceneObject_Sphere.h"
#include "../Core/Scene/Object/SceneObject_Box.h"
#include "../Core/Scene/Object/SceneObject_Plane.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"

namespace helpers {

using namespace rt;
using namespace math;

static bool ParseVector2(const rapidjson::Value& value, Vector4& outVector)
{
    if (!value.IsArray())
    {
        RT_LOG_ERROR("2D vector description must be an array");
        return false;
    }

    if (value.Size() != 2)
    {
        RT_LOG_ERROR("Invalid array size for 2D vector");
        return false;
    }

    const Float x = value[0].GetFloat();
    const Float y = value[1].GetFloat();

    outVector = Vector4(x, y, 0.0f, 0.0f);

    return true;
}

static bool ParseVector3(const rapidjson::Value& value, Vector4& outVector)
{
    if (!value.IsArray())
    {
        RT_LOG_ERROR("3D vector description must be an array");
        return false;
    }

    if (value.Size() != 3)
    {
        RT_LOG_ERROR("Invalid array size for 3D vector");
        return false;
    }

    const Float x = value[0].GetFloat();
    const Float y = value[1].GetFloat();
    const Float z = value[2].GetFloat();

    outVector = Vector4(x, y, z, 0.0f);

    return true;
}

static bool TryParseBool(const rapidjson::Value& value, const char* name, bool optional, bool& outValue)
{
    if (!value.HasMember(name))
    {
        if (optional)
        {
            return true;
        }
        else
        {
            RT_LOG_ERROR("Missing '%hs' property", name);
            return false;
        }
    }

    if (!value[name].IsBool())
    {
        RT_LOG_ERROR("Property '%s' must be a bool", name);
        return false;
    }

    outValue = value[name].GetBool();
    return true;
}

static bool TryParseFloat(const rapidjson::Value& value, const char* name, bool optional, Float& outValue)
{
    if (!value.HasMember(name))
    {
        if (optional)
        {
            return true;
        }
        else
        {
            RT_LOG_ERROR("Missing '%hs' property", name);
            return false;
        }
    }

    if (!value[name].IsFloat())
    {
        RT_LOG_ERROR("Property '%s' must be a float", name);
        return false;
    }

    outValue = value[name].GetFloat();
    return true;
}

static bool TryParseVector2(const rapidjson::Value& value, const char* name, bool optional, Vector4& outValue)
{
    if (!value.HasMember(name))
    {
        if (optional)
        {
            return true;
        }
        else
        {
            RT_LOG_ERROR("Missing '%hs' property", name);
            return false;
        }
    }

    return ParseVector2(value[name], outValue);
}

static bool TryParseVector3(const rapidjson::Value& value, const char* name, bool optional, Vector4& outValue)
{
    if (!value.HasMember(name))
    {
        if (optional)
        {
            return true;
        }
        else
        {
            RT_LOG_ERROR("Missing '%hs' property", name);
            return false;
        }
    }

    return ParseVector3(value[name], outValue);
}

static bool TryParseTransform(const rapidjson::Value& parentValue, const char* name, Transform& outValue)
{
    if (!parentValue.HasMember(name))
    {
        return true;
    }

    const rapidjson::Value& value = parentValue[name];
    if (!value.IsObject())
    {
        RT_LOG_ERROR("Transform description must be a structure");
        return false;
    }

    Vector4 translation = Vector4::Zero();
    if (!TryParseVector3(value, "translation", true, translation))
        return false;

    Vector4 orientation = Vector4::Zero();
    if (!TryParseVector3(value, "orientation", true, orientation))
        return false;

    outValue = Transform(translation, Quaternion::FromEulerAngles(orientation.ToFloat3()));

    return true;
}

static bool TryParseTexture(const rapidjson::Value& value, const char* name, BitmapPtr& outValue)
{
    if (!value.HasMember(name))
    {
        return true;
    }

    if (!value[name].IsString())
    {
        RT_LOG_ERROR("Texture path '%s' must be a string", name);
        return false;
    }

    const char* path = value[name].GetString();
    outValue = helpers::LoadTexture(gOptions.dataPath, path);
    return true;
}

static bool TryParseMaterialName(const MaterialsMap& materials, const rapidjson::Value& value, const char* name, MaterialPtr& outValue)
{
    if (!value.HasMember(name))
    {
        return true;
    }

    if (!value[name].IsString())
    {
        RT_LOG_ERROR("Material name '%s' must be a string", name);
        return false;
    }

    const std::string materialName = value[name].GetString();
    const auto iter = materials.find(materialName);
    if (iter == materials.end())
    {
        RT_LOG_ERROR("Material '%s' does not exist", materialName.c_str());
        return false;
    }

    outValue = iter->second;
    return true;
}

static MaterialPtr ParseMaterial(const rapidjson::Value& value)
{
    if (!value.IsObject())
    {
        RT_LOG_ERROR("Material description must be a structure");
        return nullptr;
    }

    if (!value.HasMember("name"))
    {
        RT_LOG_ERROR("Material is missing 'name' field");
        return nullptr;
    }

    const std::string name = value["name"].GetString();
    if (name.empty())
    {
        RT_LOG_ERROR("Material name cannot be empty");
        return nullptr;
    }

    std::string bsdfName = Material::DefaultBsdfName;
    if (value.HasMember("bsdf"))
    {
        bsdfName = value["bsdf"].GetString();
    }

    MaterialPtr material = Material::Create();
    material->debugName = std::move(name);
    material->SetBsdf(bsdfName);

    if (!TryParseBool(value, "dispersive", true, material->isDispersive)) return nullptr;

    if (!TryParseVector3(value, "baseColor", true, material->baseColor.baseValue)) return nullptr;
    if (!TryParseVector3(value, "emissionColor", true, material->emission.baseValue)) return nullptr;
    if (!TryParseFloat(value, "roughness", true, material->roughness.baseValue)) return nullptr;
    if (!TryParseFloat(value, "metalness", true, material->metalness.baseValue)) return nullptr;

    if (!TryParseTexture(value, "baseColorTexture", material->baseColor.texture)) return nullptr;
    if (!TryParseTexture(value, "emissionTexture", material->emission.texture)) return nullptr;
    if (!TryParseTexture(value, "roughnessTexture", material->roughness.texture)) return nullptr;
    if (!TryParseTexture(value, "metalnessTexture", material->metalness.texture)) return nullptr;
    if (!TryParseTexture(value, "normalMap", material->normalMap)) return nullptr;
    if (!TryParseTexture(value, "maskMap", material->maskMap)) return nullptr;

    if (!TryParseFloat(value, "normalMapStrength", true, material->normalMapStrength)) return nullptr;
    if (!TryParseFloat(value, "IoR", true, material->IoR)) return nullptr;
    if (!TryParseFloat(value, "K", true, material->K)) return nullptr;

    material->Compile();

    return material;
}

static bool ParseLight(const rapidjson::Value& value, Scene& scene)
{
    if (!value.IsObject())
    {
        RT_LOG_ERROR("Light description must be a structure");
        return false;
    }

    if (!value.HasMember("type"))
    {
        RT_LOG_ERROR("Light is missing 'type' field");
        return false;
    }

    Vector4 lightColor;
    if (!TryParseVector3(value, "color", false, lightColor))
    {
        return false;
    }

    // parse type
    const std::string typeStr = value["type"].GetString();
    if (typeStr == "area")
    {
        Vector4 lightPosition, lightEdge0, lightEdge1;
        if (!TryParseVector3(value, "position", false, lightPosition) ||
            !TryParseVector3(value, "edge0", false, lightEdge0) ||
            !TryParseVector3(value, "edge1", false, lightEdge1))
        {
            return false;
        }

        scene.AddLight(std::make_unique<AreaLight>(lightPosition, lightEdge0, lightEdge1, lightColor));
    }
    else if (typeStr == "point")
    {
        Vector4 lightPosition;
        if (!TryParseVector3(value, "position", false, lightPosition))
        {
            return false;
        }

        scene.AddLight(std::make_unique<PointLight>(lightPosition, lightColor));
    }
    else if (typeStr == "directional")
    {
        Vector4 lightDirection;
        if (!TryParseVector3(value, "direction", false, lightDirection))
        {
            return false;
        }

        scene.AddLight(std::make_unique<DirectionalLight>(lightDirection, lightColor));
    }
    else if (typeStr == "background")
    {
        auto backgroundLight = std::make_unique<BackgroundLight>(lightColor);

        if (!TryParseTexture(value, "texture", backgroundLight->mTexture))
            return false;

        scene.SetBackgroundLight(std::move(backgroundLight));
    }
    else if (typeStr == "sphere")
    {
        Vector4 lightPosition;
        if (!TryParseVector3(value, "position", false, lightPosition))
        {
            return false;
        }

        Float radius = 0.0f;
        if (!TryParseFloat(value, "radius", false, radius))
        {
            return false;
        }

        scene.AddLight(std::make_unique<SphereLight>(lightPosition, radius, lightColor));
    }
    else
    {
        RT_LOG_ERROR("Unknown light type: '%s'", typeStr.c_str());
        return false;
    }

    return true;
}

static bool ParseObject(const rapidjson::Value& value, Scene& scene, MaterialsMap& materials)
{
    if (!value.IsObject())
    {
        RT_LOG_ERROR("Object description must be a structure");
        return false;
    }

    if (!value.HasMember("type"))
    {
        RT_LOG_ERROR("Object is missing 'type' field");
        return false;
    }

    SceneObjectPtr sceneObject;

    // parse type
    const std::string typeStr = value["type"].GetString();
    if (typeStr == "sphere")
    {
        float radius = 1.0f;
        if (!TryParseFloat(value, "radius", false, radius))
        {
            return false;
        }

        sceneObject = std::make_unique<SphereSceneObject>(radius);
    }
    else if (typeStr == "box")
    {
        Vector4 size;
        if (!TryParseVector3(value, "size", false, size))
        {
            return false;
        }
        sceneObject = std::make_unique<BoxSceneObject>(size);
    }
    else if (typeStr == "plane")
    {
        Vector4 size(FLT_MAX);
        if (!TryParseVector3(value, "size", true, size))
        {
            return false;
        }
        Vector4 textureScale(1.0f);
        if (!TryParseVector2(value, "textureScale", true, textureScale))
        {
            return false;
        }
        sceneObject = std::make_unique<PlaneSceneObject>(size.ToFloat2(), textureScale.ToFloat2());
    }
    else if (typeStr == "mesh")
    {
        if (!value.HasMember("path"))
        {
            RT_LOG_ERROR("Missing 'path' property");
            return false;
        }

        if (!value["path"].IsString())
        {
            RT_LOG_ERROR("Mesh path must be a string");
            return false;
        }

        const std::string path = gOptions.dataPath + value["path"].GetString();
        MeshPtr mesh = helpers::LoadMesh(path, materials);

        sceneObject = std::make_unique<MeshSceneObject>(mesh);
    }
    else
    {
        RT_LOG_ERROR("Unknown scene object type: '%s'", typeStr.c_str());
        return false;
    }

    // TODO velocity

    if (!TryParseMaterialName(materials, value, "material", sceneObject->mDefaultMaterial))
        return false;

    Transform transform;
    if (!TryParseTransform(value, "transform", transform))
        return false;

    sceneObject->mTransform = transform.ToMatrix4();

    scene.AddObject(std::move(sceneObject));
    return true;
}

static bool ParseCamera(const rapidjson::Value& value, rt::Camera& camera)
{
    if (!value.IsObject())
    {
        RT_LOG_ERROR("Light description must be a structure");
        return false;
    }

    math::Transform transform;
    if (!TryParseTransform(value, "transform", transform))
        return false;

    Float fov = 60.0f;
    if (!TryParseFloat(value, "fieldOfView", true, fov))
        return false;

    camera.SetTransform(transform);
    camera.SetPerspective(1.0f, fov / 180.0f * RT_PI);

    if (!TryParseFloat(value, "aperture", true, camera.mDOF.aperture))
        return false;

    if (!TryParseFloat(value, "focalPlaneDistance", true, camera.mDOF.focalPlaneDistance))
        return false;

    return true;
}

bool LoadScene(const std::string& path, Scene& scene, rt::Camera& camera)
{
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp)
    {
        RT_LOG_ERROR("Failed to open file: %s", path.c_str());
        return false;
    }

    char readBuffer[4096];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document d;
    d.ParseStream(is);
    fclose(fp);

    if (!d.IsObject())
    {
        const char* errorStr = rapidjson::GetParseError_En(d.GetParseError());
        RT_LOG_ERROR("Failed to parse scene file '%s': %s", path.c_str(), errorStr);
        return false;
    }

    MaterialsMap materialsMap;

    if (d.HasMember("materials"))
    {
        const rapidjson::Value& materialsArray = d["materials"];
        if (materialsArray.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < materialsArray.Size(); i++)
            {
                const MaterialPtr material = ParseMaterial(materialsArray[i]);
                if (!material)
                    return false;

                if (materialsMap.count(material->debugName) > 0)
                {
                    RT_LOG_ERROR("Duplicated material: '%s'", material->debugName.c_str());
                    return false;
                }

                materialsMap[material->debugName] = material;
                RT_LOG_INFO("Created material: '%s'", material->debugName.c_str());
            }
        }
        else
        {
            RT_LOG_ERROR("'materials' is expected to be an array");
            return false;
        }
    }

    if (d.HasMember("objects"))
    {
        const rapidjson::Value& objectsArray = d["objects"];
        if (objectsArray.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < objectsArray.Size(); i++)
            {
                if (!ParseObject(objectsArray[i], scene, materialsMap))
                    return false;
            }
        }
        else
        {
            RT_LOG_ERROR("'objects' is expected to be an array");
            return false;
        }
    }

    if (d.HasMember("lights"))
    {
        const rapidjson::Value& lightsArray = d["lights"];
        if (lightsArray.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < lightsArray.Size(); i++)
            {
                if (!ParseLight(lightsArray[i], scene))
                    return false;
            }
        }
        else
        {
            RT_LOG_ERROR("'lights' is expected to be an array");
            return false;
        }
    }

    if (d.HasMember("camera"))
    {
        const rapidjson::Value& cameraObject = d["camera"];
        if (!ParseCamera(cameraObject, camera))
        {
            return false;
        }
    }

    return true;
}

} // namespace helpers