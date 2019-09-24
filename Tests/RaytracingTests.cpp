#include "PCH.h"
#include "../Core/Scene/Scene.h"
#include "../Core/Scene/Camera.h"
#include "../Core/Rendering/Context.h"
#include "../Core/Material/Material.h"
#include "../Core/Rendering/Viewport.h"
#include "../Core/Rendering/PathTracer.h"
#include "../Core/Scene/Light/BackgroundLight.h"
#include "../Core/Scene/Object/SceneObject_Shape.h"
#include "../Core/Scene/Object/SceneObject_Light.h"
#include "../Core/Shapes/SphereShape.h"
#include "../Core/Shapes/MeshShape.h"

using namespace rt;
using namespace math;

static std::array<const char*, 3> gRendererNames =
{
    "Path Tracer",
    "Path Tracer MIS",
    "VCM",
};

class RenderingTest : public ::testing::Test
{
public:
    static constexpr uint32 ViewportSize = 32;

    RenderingTest() = default;

    void SetUp()
    {
        mScene = std::make_unique<Scene>();
        mViewport = std::make_unique<Viewport>();
    }

protected:
    std::unique_ptr<Scene> mScene;
    std::unique_ptr<Viewport> mViewport;
};

//////////////////////////////////////////////////////////////////////////

MaterialPtr CreateEmissiveMaterial(const Vector4& color)
{
    auto material = std::make_unique<Material>();
    material->debugName = "emissive";
    material->baseColor = Vector4::Zero();
    material->emission = color;
    material->roughness = 0.0f;
    material->Compile();
    return material;
}

MaterialPtr CreatePlasticMaterial(const Vector4& baseColor)
{
    auto material = std::make_unique<Material>();
    material->debugName = "plastic";
    material->baseColor = baseColor;
    material->emission = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    material->roughness = 0.5f;
    material->Compile();
    return material;
}

//////////////////////////////////////////////////////////////////////////

/*
rt::MeshShapePtr CreateBoxMesh(const float size, const MaterialPtr& material, bool reverseNormal)
{
    const uint32 materialIndices[6 * 2] = { 0 };

    const uint32 indices[] =
    {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23,
    };

    const float n = reverseNormal ? -1.0f : 1.0f;

    const Float3 vertices[] =
    {
        { size, -size, -size },
        { size,  size, -size },
        { size,  size,  size },
        { size, -size,  size },

        { -size, -size, -size },
        { -size,  size, -size },
        { -size,  size,  size },
        { -size, -size,  size },

        { -size, size, -size },
        {  size, size, -size },
        {  size, size,  size },
        { -size, size,  size },

        { -size, -size, -size },
        {  size, -size, -size },
        {  size, -size,  size },
        { -size, -size,  size },

        { -size, -size, size },
        {  size, -size, size },
        {  size,  size, size },
        { -size,  size, size },

        { -size, -size, -size },
        {  size, -size, -size },
        {  size,  size, -size },
        { -size,  size, -size },
    };

    const Float3 normals[] =
    {
        { n, 0.0f, 0.0f },
        { n, 0.0f, 0.0f },
        { n, 0.0f, 0.0f },
        { n, 0.0f, 0.0f },

        { -n, 0.0f, 0.0f },
        { -n, 0.0f, 0.0f },
        { -n, 0.0f, 0.0f },
        { -n, 0.0f, 0.0f },

        { 0.0f, n, 0.0f },
        { 0.0f, n, 0.0f },
        { 0.0f, n, 0.0f },
        { 0.0f, n, 0.0f },

        { 0.0f, -n, 0.0f },
        { 0.0f, -n, 0.0f },
        { 0.0f, -n, 0.0f },
        { 0.0f, -n, 0.0f },

        { 0.0f, 0.0f, n },
        { 0.0f, 0.0f, n },
        { 0.0f, 0.0f, n },
        { 0.0f, 0.0f, n },

        { 0.0f, 0.0f, -n },
        { 0.0f, 0.0f, -n },
        { 0.0f, 0.0f, -n },
        { 0.0f, 0.0f, -n },
    };

    const Float3 tangents[] =
    {
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },

        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },

        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },

        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },

        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },

        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
    };

    const Float2 texCoords[] =
    {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },

        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },

        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },

        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },

        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },

        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
    };

    MeshDesc meshDesc;
    meshDesc.path = "box";
    meshDesc.vertexBufferDesc.numTriangles = 2 * 6;
    meshDesc.vertexBufferDesc.numVertices = 4 * 6;
    meshDesc.vertexBufferDesc.numMaterials = 1;
    meshDesc.vertexBufferDesc.materials = &material;
    meshDesc.vertexBufferDesc.materialIndexBuffer = materialIndices;
    meshDesc.vertexBufferDesc.vertexIndexBuffer = indices;
    meshDesc.vertexBufferDesc.positions = vertices;
    meshDesc.vertexBufferDesc.normals = normals;
    meshDesc.vertexBufferDesc.tangents = tangents;
    meshDesc.vertexBufferDesc.texCoords = texCoords;

    rt::MeshShapePtr mesh = std::make_shared<Mesh>();
    bool result = mesh->Initialize(meshDesc);
    if (!result)
    {
        return nullptr;
    }

    return mesh;
}
*/

void ValidateBitmap(const Bitmap& bitmap, const Vector4& expectedValue, float maxError)
{
    for (uint32 y = 0; y < bitmap.GetHeight(); ++y)
    {
        SCOPED_TRACE("y=" + std::to_string(y));

        for (uint32 x = 0; x < bitmap.GetWidth(); ++x)
        {
            SCOPED_TRACE("x=" + std::to_string(x));

            const Vector4 color = bitmap.GetPixel(x, y);
            EXPECT_NEAR(expectedValue.x, color.x, maxError);
            EXPECT_NEAR(expectedValue.y, color.y, maxError);
            EXPECT_NEAR(expectedValue.z, color.z, maxError);
        }
    }

    // TODO compute average image color
}

static const std::string g_ouputFilePrefix = "RenderingTests/";

TEST_F(RenderingTest, EmptyScene)
{
    mViewport->Resize(ViewportSize, ViewportSize);

    Camera camera;
    camera.SetPerspective(1.0f, DegToRad(90.0f));

    for (const char* rendererName : gRendererNames)
    {
        SCOPED_TRACE(rendererName);

        RendererPtr renderer = CreateRenderer(rendererName, *mScene);
        mViewport->SetRenderer(renderer);
        mViewport->Reset();

        mViewport->Render(camera);

        ValidateBitmap(mViewport->GetSumBuffer(), Vector4::Zero(), 0.0f);

        std::string outputFilePath = g_ouputFilePrefix + "EmptyScene_" + rendererName + ".exr";
        mViewport->GetSumBuffer().SaveEXR(outputFilePath.c_str());
    }
}

TEST_F(RenderingTest, BackgroundLightOnly)
{
    const Vector4 lightColor(1.0f, 2.0f, 3.0f);
    auto backgroundLight = std::make_unique<BackgroundLight>(lightColor);
    auto lightObject = std::make_unique<LightSceneObject>(std::move(backgroundLight));
    mScene->AddObject(std::move(lightObject));
    mScene->BuildBVH();

    mViewport->Resize(ViewportSize, ViewportSize);

    Camera camera;
    camera.SetPerspective(1.0f, DegToRad(90.0f));

    for (const char* rendererName : gRendererNames)
    {
        SCOPED_TRACE(rendererName);

        RendererPtr renderer = CreateRenderer(rendererName, *mScene);
        mViewport->SetRenderer(renderer);
        mViewport->Reset();

        mViewport->Render(camera);

        ValidateBitmap(mViewport->GetSumBuffer(), lightColor, 0.01f);

        std::string outputFilePath = g_ouputFilePrefix + "BackgroundLightOnly_" + rendererName + ".exr";
        mViewport->GetSumBuffer().SaveEXR(outputFilePath.c_str());
    }
}

TEST_F(RenderingTest, FurnaceTest_Diffuse)
{
    const Vector4 materialColor(0.4f, 0.6f, 0.8f);
    MaterialPtr material = std::make_unique<Material>();
    material->SetBsdf("diffuse");
    material->baseColor = materialColor;
    material->Compile();

    const Vector4 lightColor(1.0f, 2.0f, 3.0f);
    auto backgroundLight = std::make_unique<BackgroundLight>(lightColor);
    auto lightObject = std::make_unique<LightSceneObject>(std::move(backgroundLight));
    mScene->AddObject(std::move(lightObject));

    ShapePtr shape = std::make_unique<SphereShape>(1.0f);
    ShapeSceneObjectPtr sceneObject = std::make_unique<ShapeSceneObject>(std::move(shape));
    sceneObject->SetDefaultMaterial(material);
    mScene->AddObject(std::move(sceneObject));

    mScene->BuildBVH();

    mViewport->Resize(ViewportSize, ViewportSize);

    Camera camera;
    camera.SetPerspective(1.0f, DegToRad(10.0f));
    camera.SetTransform(Transform(Vector4(0.0f, 0.0f, -3.0f)));

    uint32 numPasses = 100;

    for (const char* rendererName : gRendererNames)
    {
        SCOPED_TRACE(rendererName);

        RendererPtr renderer = CreateRenderer(rendererName, *mScene);
        mViewport->SetRenderer(renderer);
        mViewport->Reset();

        for (uint32 i = 0; i < numPasses; ++i)
        {
            mViewport->Render(camera);
        }

        Bitmap bitmap = mViewport->GetSumBuffer();
        bitmap.Scale(Vector4(1.0f / numPasses));

        ValidateBitmap(bitmap, lightColor * materialColor, 0.05f);

        std::string outputFilePath = g_ouputFilePrefix + "FurnaceTest_Diffuse_" + rendererName + ".exr";
        bitmap.SaveEXR(outputFilePath.c_str());
    }
}

TEST_F(RenderingTest, FurnaceTest_Emissive)
{
    const Vector4 emissionColor(3.0f, 2.0f, 1.0f);

    MaterialPtr material = std::make_unique<Material>();
    material->SetBsdf("null");
    material->baseColor = Vector4::Zero();
    material->emission = emissionColor;
    material->Compile();

    const Vector4 lightColor(1.0f, 2.0f, 3.0f);
    auto backgroundLight = std::make_unique<BackgroundLight>(lightColor);
    auto lightObject = std::make_unique<LightSceneObject>(std::move(backgroundLight));
    mScene->AddObject(std::move(lightObject));

    ShapePtr shape = std::make_unique<SphereShape>(1.0f);
    ShapeSceneObjectPtr sceneObject = std::make_unique<ShapeSceneObject>(std::move(shape));
    sceneObject->SetDefaultMaterial(material);
    mScene->AddObject(std::move(sceneObject));

    mScene->BuildBVH();

    mViewport->Resize(ViewportSize, ViewportSize);

    Camera camera;
    camera.SetPerspective(1.0f, DegToRad(10.0f));
    camera.SetTransform(Transform(Vector4(0.0f, 0.0f, -3.0f)));

    uint32 numPasses = 1;

    for (const char* rendererName : gRendererNames)
    {
        SCOPED_TRACE(rendererName);

        RendererPtr renderer = CreateRenderer(rendererName, *mScene);
        mViewport->SetRenderer(renderer);
        mViewport->Reset();

        for (uint32 i = 0; i < numPasses; ++i)
        {
            mViewport->Render(camera);
        }

        Bitmap bitmap = mViewport->GetSumBuffer();
        bitmap.Scale(Vector4(1.0f / numPasses));

        ValidateBitmap(bitmap, emissionColor, 0.0f);

        std::string outputFilePath = g_ouputFilePrefix + "FurnaceTest_Diffuse_" + rendererName + ".exr";
        bitmap.SaveEXR(outputFilePath.c_str());
    }
}

TEST_F(RenderingTest, FurnaceTest_Metal)
{
    const Vector4 materialColor(0.4f, 0.6f, 0.8f);

    MaterialPtr material = std::make_unique<Material>();
    material->SetBsdf("metal");
    material->baseColor = materialColor;
    material->IoR = 0.0f;
    material->K = 100.0;
    material->Compile();

    const Vector4 lightColor(1.0f, 2.0f, 3.0f);
    auto backgroundLight = std::make_unique<BackgroundLight>(lightColor);
    auto lightObject = std::make_unique<LightSceneObject>(std::move(backgroundLight));
    mScene->AddObject(std::move(lightObject));

    ShapePtr shape = std::make_unique<SphereShape>(1.0f);
    ShapeSceneObjectPtr sceneObject = std::make_unique<ShapeSceneObject>(std::move(shape));
    sceneObject->SetDefaultMaterial(material);
    mScene->AddObject(std::move(sceneObject));

    mScene->BuildBVH();

    mViewport->Resize(ViewportSize, ViewportSize);

    Camera camera;
    camera.SetPerspective(1.0f, DegToRad(10.0f));
    camera.SetTransform(Transform(Vector4(0.0f, 0.0f, -3.0f)));

    for (const char* rendererName : gRendererNames)
    {
        SCOPED_TRACE(rendererName);

        RendererPtr renderer = CreateRenderer(rendererName, *mScene);
        mViewport->SetRenderer(renderer);
        mViewport->Reset();

        uint32 numPasses = 20;

        for (uint32 i = 0; i < numPasses; ++i)
        {
            mViewport->Render(camera);
        }

        Bitmap bitmap = mViewport->GetSumBuffer();
        bitmap.Scale(Vector4(1.0f / numPasses));

        ValidateBitmap(bitmap, lightColor * materialColor, 0.05f);

        std::string outputFilePath = g_ouputFilePrefix + "FurnaceTest_Metal_" + rendererName + ".exr";
        bitmap.SaveEXR(outputFilePath.c_str());
    }
}

TEST_F(RenderingTest, FurnaceTest_Dielectric)
{
    MaterialPtr material = std::make_unique<Material>();
    material->SetBsdf("dielectric");
    material->baseColor = Vector4(1.0f);
    material->Compile();

    const Vector4 lightColor(1.0f, 2.0f, 3.0f);
    auto backgroundLight = std::make_unique<BackgroundLight>(lightColor);
    auto lightObject = std::make_unique<LightSceneObject>(std::move(backgroundLight));
    mScene->AddObject(std::move(lightObject));

    ShapePtr shape = std::make_unique<SphereShape>(1.0f);
    ShapeSceneObjectPtr sceneObject = std::make_unique<ShapeSceneObject>(std::move(shape));
    sceneObject->SetDefaultMaterial(material);
    mScene->AddObject(std::move(sceneObject));

    mScene->BuildBVH();

    mViewport->Resize(ViewportSize, ViewportSize);

    Camera camera;
    camera.SetPerspective(1.0f, DegToRad(10.0f));
    camera.SetTransform(Transform(Vector4(0.0f, 0.0f, -3.0f)));

    for (const char* rendererName : gRendererNames)
    {
        SCOPED_TRACE(rendererName);

        RendererPtr renderer = CreateRenderer(rendererName, *mScene);
        mViewport->SetRenderer(renderer);
        mViewport->Reset();

        uint32 numPasses = 1000;

        for (uint32 i = 0; i < numPasses; ++i)
        {
            mViewport->Render(camera);
        }

        Bitmap bitmap = mViewport->GetSumBuffer();
        bitmap.Scale(Vector4(1.0f / numPasses));

        ValidateBitmap(bitmap, lightColor, 0.075f);

        std::string outputFilePath = g_ouputFilePrefix + "FurnaceTest_Dielectric_" + rendererName + ".exr";
        bitmap.SaveEXR(outputFilePath.c_str());
    }
}

// TODO