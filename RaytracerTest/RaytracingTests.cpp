#include "PCH.h"
#include "../RaytracerLib/Scene/Scene.h"
#include "../RaytracerLib/Rendering/Context.h"
#include "../RaytracerLib/Mesh/Mesh.h"
#include "../RaytracerLib/Material/Material.h"
#include "../RaytracerLib/Scene/SceneObject_Mesh.h"

#include "gtest/gtest.h"

using namespace rt;
using namespace math;

class TestFixture : public ::testing::Test
{
public:
    TestFixture() = default;

    void SetUp()
    {
        mScene = std::make_unique<Scene>();
    }

protected:
    std::unique_ptr<Scene> mScene;
};

//////////////////////////////////////////////////////////////////////////

std::unique_ptr<Material> CreateEmissiveMaterial(const Vector4& color)
{
    auto material = std::make_unique<Material>();
    material->debugName = "emissive";
    material->baseColor = Vector4();
    material->emissionColor = color;
    material->roughness = 0.0f;
    material->Compile();
    return material;
}

std::unique_ptr<Material> CreatePlasticMaterial(const Vector4& baseColor)
{
    auto material = std::make_unique<Material>();
    material->debugName = "plastic";
    material->baseColor = baseColor;
    material->emissionColor = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    material->roughness = 0.5f;
    material->Compile();
    return material;
}

//////////////////////////////////////////////////////////////////////////

std::unique_ptr<Mesh> CreateBoxMesh(const float size, const Material* material, bool reverseNormal)
{
    const Uint32 materialIndices[6 * 2] = { 0 };

    const Uint32 indices[] =
    {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23,
    };

    const float n = reverseNormal ? -1.0f : 1.0f;

    const Float vertices[] =
    {
        size, -size, -size,
        size,  size, -size,
        size,  size,  size,
        size, -size,  size,

        -size, -size, -size,
        -size,  size, -size,
        -size,  size,  size,
        -size, -size,  size,

        -size, size, -size,
         size, size, -size,
         size, size,  size,
        -size, size,  size,

        -size, -size, -size,
         size, -size, -size,
         size, -size,  size,
        -size, -size,  size,

        -size, -size, size,
         size, -size, size,
         size,  size, size,
        -size,  size, size,

        -size, -size, -size,
         size, -size, -size,
         size,  size, -size,
        -size,  size, -size,
    };

    const Float normals[] =
    {
        n, 0.0f, 0.0f,
        n, 0.0f, 0.0f,
        n, 0.0f, 0.0f,
        n, 0.0f, 0.0f,

        -n, 0.0f, 0.0f,
        -n, 0.0f, 0.0f,
        -n, 0.0f, 0.0f,
        -n, 0.0f, 0.0f,

        0.0f, n, 0.0f,
        0.0f, n, 0.0f,
        0.0f, n, 0.0f,
        0.0f, n, 0.0f,

        0.0f, -n, 0.0f,
        0.0f, -n, 0.0f,
        0.0f, -n, 0.0f,
        0.0f, -n, 0.0f,

        0.0f, 0.0f, n,
        0.0f, 0.0f, n,
        0.0f, 0.0f, n,
        0.0f, 0.0f, n,

        0.0f, 0.0f, -n,
        0.0f, 0.0f, -n,
        0.0f, 0.0f, -n,
        0.0f, 0.0f, -n,
    };

    const Float tangents[] =
    {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };

    const Float texCoords[] =
    {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
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

    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
    bool result = mesh->Initialize(meshDesc);
    if (!result)
    {
        return nullptr;
    }

    return mesh;
}

TEST_F(TestFixture, BackgroundOnly)
{
    SceneEnvironment env;
    env.backgroundColor = Vector4(100.0f, 0.0f, 0.0f, 0.0f);
    mScene->SetEnvironment(env);

    Random random;
    const RenderingParams params;
    RenderingContext context;
    context.params = &params;

    const size_t maxProbes = 100;
    for (size_t i = 0; i < maxProbes; ++i)
    {
        const Ray ray(Vector4(), random.GetVector4());
        const Color color = mScene->TraceRay_Single(ray, context);

        //EXPECT_EQ(env.backgroundColor.x, rgbColor.x);
        //EXPECT_EQ(env.backgroundColor.y, rgbColor.y);
        //EXPECT_EQ(env.backgroundColor.z, rgbColor.z);
    }
}

/*
TEST_F(TestFixture, Furnace_EmissionOnly)
{
    SceneEnvironment env;
    env.backgroundColor = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
    mScene->SetEnvironment(env);

    const Vector4 color(0.0f, 1.0f, 0.0f, 0.0f);
    auto material = CreateEmissiveMaterial(color);
    auto mesh = CreateBoxMesh(1.0f, material.get(), true);

    SceneObjectPtr meshInstance = std::make_unique<MeshSceneObject>(mesh.get());
    mScene->AddObject(std::move(meshInstance));
    ASSERT_TRUE(mScene->BuildBVH());

    Random random;
    const RenderingParams params;
    RenderingContext context(&params);

    const size_t maxProbes = 50;
    const size_t maxSamples = 100;
    for (size_t i = 0; i < maxProbes; ++i)
    {
        const Ray ray(Vector4(), random.GetVector4());
        Vector4 rgbColor;
        for (size_t j = 0; j < maxSamples; ++j)
        {
            rgbColor += mScene->TraceRay_Single(ray, context).values;
        }
        rgbColor /= static_cast<float>(maxSamples);

        SCOPED_TRACE("x=" + std::to_string(ray.dir.x));
        SCOPED_TRACE("y=" + std::to_string(ray.dir.y));
        SCOPED_TRACE("z=" + std::to_string(ray.dir.z));

        EXPECT_NEAR(color.x, rgbColor.x, 0.05f);
        EXPECT_NEAR(color.y, rgbColor.y, 0.05f);
        EXPECT_NEAR(color.z, rgbColor.z, 0.05f);
    }
}
*/

TEST_F(TestFixture, Furnace_Metal)
{
    SceneEnvironment env;
    // crazy high background color to detect any leaking
    env.backgroundColor = Vector4(100000.0f, 0.0f, 0.0f, 0.0f);
    mScene->SetEnvironment(env);

    auto material = std::make_unique<Material>();
    material->metalness = 1.0f;
    material->debugName = "metal";
    material->baseColor = Vector4(0.5f, 0.5f, 0.5f, 0.0f);
    material->emissionColor = Vector4(1.0f, 2.0f, 3.0f, 0.0f);
    material->roughness = 0.1f;
    material->IoR = 100.0f;
    material->IoR = 0.0f;
    material->Compile();

    const Vector4 expectedColor = material->emissionColor / (VECTOR_ONE - material->baseColor);

    auto mesh = CreateBoxMesh(1.0f, material.get(), true);

    SceneObjectPtr meshInstance = std::make_unique<MeshSceneObject>(mesh.get());
    mScene->AddObject(std::move(meshInstance));
    ASSERT_TRUE(mScene->BuildBVH());

    Random random;
    const RenderingParams params;
    RenderingContext context;
    context.params = &params;

    const size_t maxProbes = 100;
    const size_t maxSamples = 50;
    for (size_t i = 0; i < maxProbes; ++i)
    {
        /*
        const Ray ray(Vector4(), random.GetVector4());
        Vector4 rgbColor;
        for (size_t j = 0; j < maxSamples; ++j)
        {
            rgbColor += mScene->TraceRay_Single(ray, context).ToXYZ();
        }
        rgbColor /= static_cast<float>(maxSamples);

        SCOPED_TRACE("x=" + std::to_string(ray.dir.x));
        SCOPED_TRACE("y=" + std::to_string(ray.dir.y));
        SCOPED_TRACE("z=" + std::to_string(ray.dir.z));

        EXPECT_NEAR(expectedColor.x, rgbColor.x, 0.05f);
        EXPECT_NEAR(expectedColor.y, rgbColor.y, 0.05f);
        EXPECT_NEAR(expectedColor.z, rgbColor.z, 0.05f);
        */
    }
}