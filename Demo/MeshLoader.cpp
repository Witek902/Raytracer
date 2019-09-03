#include "PCH.h"
#include "MeshLoader.h"

#include "../Core/Utils/Logger.h"
#include "../Core/Utils/Bitmap.h"
#include "../Core/Utils/Timer.h"
#include "../Core/Math/Geometry.h"
#include "../Core/Textures/BitmapTexture.h"

namespace helpers {

using namespace rt;
using namespace math;

struct TriangleIndicesComparator
{
    RT_FORCE_INLINE bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const
    {
        return (a.vertex_index == b.vertex_index) && (a.normal_index == b.normal_index) && (a.texcoord_index == b.texcoord_index);
    }
};

struct TriangleIndicesHash
{
    RT_FORCE_INLINE size_t operator()(const tinyobj::index_t& k) const
    {
        return k.vertex_index ^ k.normal_index ^ k.texcoord_index;
    }
};

BitmapPtr LoadBitmapObject(const std::string& baseDir, const std::string& path)
{
    if (path.empty())
    {
        return nullptr;
    }

    std::string fullPath = baseDir + path;
    if ((fullPath.rfind(".png") == fullPath.length() - 4) || (fullPath.rfind(".jpg") == fullPath.length() - 4))
    {
        fullPath.replace(fullPath.length() - 4, 4, ".bmp");
    }

    // cache bitmaps so they are loaded only once
    static std::map<std::string, BitmapPtr> bitmapsList;
    BitmapPtr& bitmapPtr = bitmapsList[fullPath];

    if (!bitmapPtr)
    {
        bitmapPtr = BitmapPtr(new Bitmap(path.c_str()));
        if (!bitmapPtr->Load(fullPath.c_str()))
        {
            return nullptr;
        }
    }

    return bitmapPtr;
}

TexturePtr LoadTexture(const std::string& baseDir, const std::string& path)
{
    BitmapPtr bitmap = LoadBitmapObject(baseDir, path);

    if (!bitmap)
    {
        return nullptr;
    }

    if (bitmap->GetWidth() > 0 && bitmap->GetHeight() > 0)
    {
        auto texture = std::make_shared<BitmapTexture>(bitmap);
        return texture;
    }

    return nullptr;
}

MaterialPtr LoadMaterial(const std::string& baseDir, const tinyobj::material_t& sourceMaterial)
{
    auto material = MaterialPtr(new Material);

    material->SetBsdf("diffuse"); // TODO
    material->debugName = sourceMaterial.name;
    material->baseColor = Vector4(sourceMaterial.diffuse[0], sourceMaterial.diffuse[1], sourceMaterial.diffuse[2], 0.0f);
    material->emission.baseValue = Vector4(sourceMaterial.emission[0], sourceMaterial.emission[1], sourceMaterial.emission[2], 0.0f);
    material->baseColor.texture = LoadTexture(baseDir, sourceMaterial.diffuse_texname);
    material->normalMap = LoadTexture(baseDir, sourceMaterial.normal_texname);
    material->maskMap = LoadTexture(baseDir, sourceMaterial.alpha_texname);
    material->roughness = 0.075f;

    material->Compile();

    return material;
}

MaterialPtr CreateDefaultMaterial(MaterialsMap& outMaterials)
{
    auto material = MaterialPtr(new Material);
    material->debugName = "default";
    material->baseColor = Vector4(0.8f, 0.8f, 0.8f, 0.0f);
    material->emission.baseValue = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    material->roughness = 0.75f;
    material->SetBsdf(Material::DefaultBsdfName);
    material->Compile();

    outMaterials[material->debugName] = material;
    return material;
}

class MeshLoader
{
public:
    static constexpr float MinEdgeLength = 0.001f;
    static constexpr float MinEdgeLengthSqr = Sqr(MinEdgeLength);

    MeshLoader()
    {
    }

    bool LoadMesh(const std::string& filePath, MaterialsMap& outMaterials, const float scale)
    {
        RT_LOG_DEBUG("Loading mesh file: '%s'...", filePath.c_str());

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        const std::string meshBaseDir = filePath.substr(0, filePath.find_last_of("\\/")) + "/";

        {
            Timer timer;
            std::string warning, err;
            bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &err, filePath.c_str(), meshBaseDir.c_str(), true, false);
            if (!warning.empty())
            {
                RT_LOG_WARNING("Mesh '%s' loading message:\n%s", filePath.c_str(), err.c_str());
            }
            if (!err.empty())
            {
                RT_LOG_ERROR("Mesh '%s' loading message:\n%s", filePath.c_str(), err.c_str());
            }
            if (!ret)
            {
                RT_LOG_ERROR("Failed to load mesh '%s'", filePath.c_str());
                return false;
            }

            RT_LOG_INFO("Mesh file '%s' parsed in %.3f seconds", filePath.c_str(), timer.Stop());
        }

        // Loop over shapes
        for (size_t shapeIndex = 0; shapeIndex < shapes.size(); shapeIndex++)
        {
            // Loop over faces
            const tinyobj::mesh_t& mesh = shapes[shapeIndex].mesh;

            for (size_t faceIndex = 0; faceIndex < mesh.num_face_vertices.size(); faceIndex++)
            {
                const int numFaceVertices = mesh.num_face_vertices[faceIndex];
                if (numFaceVertices != 3)
                {
                    RT_LOG_ERROR("Expected only triangles (shape index = %zu, face index = %zu)", shapeIndex, faceIndex);
                    return false;
                }

                const tinyobj::index_t idx[3] =
                {
                    mesh.indices[3u * faceIndex + 0],
                    mesh.indices[3u * faceIndex + 1],
                    mesh.indices[3u * faceIndex + 2],
                };

                const bool hasNormals = idx[0].normal_index >= 0 && idx[1].normal_index >= 0 && idx[2].normal_index >= 0;
                const bool hasTexCoords = idx[0].texcoord_index >= 0 && idx[1].texcoord_index >= 0 && idx[2].texcoord_index >= 0;

                Vector4 verts[3];
                for (size_t i = 0; i < 3; i++)
                {
                    verts[i] = scale * Vector4(
                        attrib.vertices[3 * idx[i].vertex_index + 0],
                        attrib.vertices[3 * idx[i].vertex_index + 1],
                        attrib.vertices[3 * idx[i].vertex_index + 2]);
                }

                // discard degenerate triangles
                const Vector4 edge1 = verts[1] - verts[0];
                const Vector4 edge2 = verts[2] - verts[0];
                const Vector4 edge3 = verts[2] - verts[1];
                if (edge1.SqrLength3() < MinEdgeLengthSqr ||
                    edge2.SqrLength3() < MinEdgeLengthSqr ||
                    edge3.SqrLength3() < MinEdgeLengthSqr ||
                    TriangleSurfaceArea(edge1, edge2) < MinEdgeLengthSqr)
                {
                    RT_LOG_WARNING("Mesh has degenerate triangle (shape index = %zu, face index = %zu)", shapeIndex, faceIndex);
                    continue;
                }

                // compute per-face normal
                const Vector4 faceNormal = Vector4::Cross3(verts[1] - verts[0], verts[2] - verts[0]).Normalized3();
                RT_ASSERT(faceNormal.IsValid());

                for (size_t i = 0; i < 3; i++)
                {
                    const tinyobj::index_t indices = idx[i];
                    const auto iter = mUniqueIndices.find(indices);

                    Uint32 uniqueIndex = 0;
                    if (iter != mUniqueIndices.end())
                    {
                        uniqueIndex = (*iter).second;
                    }
                    else
                    {
                        uniqueIndex = (Uint32)mUniqueIndices.size();
                        mUniqueIndices[indices] = uniqueIndex;

                        mVertexPositions.push_back(verts[i].ToFloat3());

                        if (hasNormals)
                        {
                            const Vector4 normal(
                                attrib.normals[3 * indices.normal_index + 0],
                                attrib.normals[3 * indices.normal_index + 1],
                                attrib.normals[3 * indices.normal_index + 2]);
                            mVertexNormals.push_back(normal.Normalized3().ToFloat3());
                        }
                        else
                        {
                            // fallback to face normal
                            // TODO smooth shading
                            mVertexNormals.push_back(faceNormal.ToFloat3());
                        }

                        if (hasTexCoords)
                        {
                            mVertexTexCoords.push_back(Float2(attrib.texcoords.data() + 2 * idx[i].texcoord_index));
                        }
                        else
                        {
                            mVertexTexCoords.push_back(Float2());
                        }
                    }

                    mVertexIndices.push_back(uniqueIndex);
                }

                mMaterialIndices.push_back(mesh.material_ids[faceIndex]);
            }
        }

        RT_ASSERT(mVertexPositions.size() == mUniqueIndices.size());
        RT_ASSERT(mVertexPositions.size() == mVertexNormals.size());
        RT_ASSERT(mVertexPositions.size() == mVertexTexCoords.size());

        ComputeTangentVectors();

        // load materials
        mMaterialPointers.reserve(materials.size());
        for (size_t i = 0; i < materials.size(); i++)
        {
            auto material = LoadMaterial(meshBaseDir, materials[i]);
            mMaterialPointers.push_back(material);
            outMaterials[material->debugName] = material;
        }

        // fallback to default material
        if (materials.empty())
        {
            RT_LOG_WARNING("No materials found in mesh '%s'. Falling back to the default material.", filePath.c_str());

            mMaterialPointers.push_back(CreateDefaultMaterial(outMaterials));

            for (Uint32& index : mMaterialIndices)
            {
                index = 0;
            }
        }

        RT_LOG_DEBUG("Mesh file '%s' loaded, vertices = %zu, indices = %zu, materials = %zu", filePath.c_str(), mVertexPositions.size(), mVertexIndices.size(), mMaterialPointers.size());
        mFilePath = filePath;

        return true;
    }

    void ComputeTangentVectors()
    {
        mVertexTangents.resize(mVertexNormals.size());

        std::vector<Vector4, AlignmentAllocator<Vector4>> bitangents;
        bitangents.resize(mVertexNormals.size());
        memset((void*)bitangents.data(), 0, bitangents.size() * sizeof(Vector4));

        Uint32 numTriangles = static_cast<Uint32>(mVertexIndices.size() / 3);
        for (Uint32 i = 0; i < numTriangles; ++i)
        {
            // algorithm based on: http://www.terathon.com/code/tangent.html
            // (Lengyel�s Method)

            const Uint32 i0 = mVertexIndices[3 * i + 0];
            const Uint32 i1 = mVertexIndices[3 * i + 1];
            const Uint32 i2 = mVertexIndices[3 * i + 2];

            const Vector4 p0(mVertexPositions[i0]);
            const Vector4 p1(mVertexPositions[i1]);
            const Vector4 p2(mVertexPositions[i2]);
            const Vector4 e1 = p1 - p0;
            const Vector4 e2 = p2 - p0;

            const Float2& w0 = mVertexTexCoords[i0];
            const Float2& w1 = mVertexTexCoords[i1];
            const Float2& w2 = mVertexTexCoords[i2];
            const float s1 = w1.x - w0.x;
            const float t1 = w1.y - w0.y;
            const float s2 = w2.x - w0.x;
            const float t2 = w2.y - w0.y;

            const float det = s1 * t2 - s2 * t1;
            if (Abs(det) < 1.0e-10f)
            {
                continue;
            }

            const float r = 1.0f / det;
            const Vector4 sdir = (t2 * e1 - t1 * e2) * r;
            const Vector4 tdir = (s1 * e2 - s2 * e1) * r;

            RT_ASSERT(sdir.IsValid());
            RT_ASSERT(tdir.IsValid());

            mVertexTangents[i0] += sdir.ToFloat3();
            mVertexTangents[i1] += sdir.ToFloat3();
            mVertexTangents[i2] += sdir.ToFloat3();

            bitangents[i0] += tdir;
            bitangents[i1] += tdir;
            bitangents[i2] += tdir;
        }

        Uint32 numVertices = static_cast<Uint32>(mVertexPositions.size());
        for (Uint32 i = 0; i < numVertices; ++i)
        {
            Vector4 tangent(mVertexTangents[i]);
            Vector4 normal(mVertexNormals[i]);
            Vector4 bitangent(bitangents[i]);

            RT_ASSERT(tangent.IsValid());
            RT_ASSERT(normal.IsValid());
            RT_ASSERT(bitangent.IsValid());

            bool tangentIsValid = false;
            if (tangent.SqrLength3() > 0.1f)
            {
                tangent.Normalize3();
                if (Vector4::Cross3(tangent, normal).SqrLength3() > 0.01f)
                {
                    tangent = Vector4::Orthogonalize(tangent, normal);
                    tangentIsValid = true;
                }
            }

            if (!tangentIsValid)
            {
                BuildOrthonormalBasis(normal, tangent, bitangent);
            }
            tangent.Normalize3();

            RT_ASSERT(tangent.IsValid());
            RT_ASSERT(Abs(Vector4::Dot3(normal, tangent)) < 0.0001f, "Normal and tangent vectors are not orthogonal");

            // Calculate handedness
            const Vector4 computedBitangent = Vector4::Cross3(normal, tangent);
            float headedness = Vector4::Dot3(computedBitangent, bitangent) < 0.0f ? -1.0f : 1.0f;
            (void)headedness; // TODO

            mVertexTangents[i] = tangent.ToFloat3();
        }
    }

    MeshPtr BuildMesh()
    {
        MeshDesc meshDesc;
        meshDesc.path = mFilePath;
        meshDesc.vertexBufferDesc.numTriangles = static_cast<Uint32>(mVertexIndices.size() / 3);
        meshDesc.vertexBufferDesc.numVertices = static_cast<Uint32>(mVertexPositions.size());
        meshDesc.vertexBufferDesc.numMaterials = static_cast<Uint32>(mMaterialPointers.size());
        meshDesc.vertexBufferDesc.materials = mMaterialPointers.data();
        meshDesc.vertexBufferDesc.materialIndexBuffer = mMaterialIndices.data();
        meshDesc.vertexBufferDesc.vertexIndexBuffer = mVertexIndices.data();
        meshDesc.vertexBufferDesc.positions = mVertexPositions.data();
        meshDesc.vertexBufferDesc.normals = mVertexNormals.data();
        meshDesc.vertexBufferDesc.tangents = mVertexTangents.data();
        meshDesc.vertexBufferDesc.texCoords = mVertexTexCoords.data();

        MeshPtr mesh = MeshPtr(new Mesh);
        bool result = mesh->Initialize(meshDesc);
        if (!result)
        {
            return nullptr;
        }

        return mesh;
    }

private:
    std::string mFilePath;

    std::vector<Uint32> mVertexIndices;
    std::vector<Uint32> mMaterialIndices;
    std::vector<Float3> mVertexPositions;
    std::vector<Float3> mVertexNormals;
    std::vector<Float3> mVertexTangents;
    std::vector<Float2> mVertexTexCoords;
    std::vector<MaterialPtr> mMaterialPointers;
    std::unordered_map<tinyobj::index_t, Uint32, TriangleIndicesHash, TriangleIndicesComparator> mUniqueIndices;
};

rt::MeshPtr LoadMesh(const std::string& filePath, MaterialsMap& outMaterials, const float scale)
{
    MeshLoader loader;
    if (!loader.LoadMesh(filePath, outMaterials, scale))
    {
        return nullptr;
    }

    return loader.BuildMesh();
}

} // namespace helpers