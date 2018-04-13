#include "PCH.h"
#include "MeshLoader.h"
#include "../RaytracerLib/Logger.h"
#include "../RaytracerLib/Math/Vector4.h"


#include "../External/tiny_obj_loader.h"

namespace helpers {

using namespace rt;

struct TriangleIndices
{
    int vertex;
    int normal;
    int texCoord;

    explicit TriangleIndices(const tinyobj::index_t& idx)
        : vertex(idx.vertex_index)
        , normal(idx.normal_index)
        , texCoord(idx.texcoord_index)
    {}

    bool operator == (const TriangleIndices& rhs) const
    {
        return (vertex < rhs.vertex) && (normal < rhs.normal) && (texCoord < rhs.texCoord);
    }

    bool operator < (const TriangleIndices& rhs) const
    {
        if (vertex < rhs.vertex) return true;
        if (normal < rhs.normal) return true;
        if (texCoord < rhs.texCoord) return true;
        return false;
    }
};

std::unique_ptr<rt::Material> LoadMaterial(rt::Instance& raytracerInstance, const tinyobj::material_t& sourceMaterial)
{
    auto material = raytracerInstance.CreateMaterial();
    material->debugName = sourceMaterial.name;
    material->baseColor = math::Vector4(sourceMaterial.diffuse[0], sourceMaterial.diffuse[1], sourceMaterial.diffuse[2]);
    material->emissionColor = math::Vector4(sourceMaterial.emission[0], sourceMaterial.emission[1], sourceMaterial.emission[2]);

    // TODO textures

    return material;
}

std::unique_ptr<rt::IMesh> LoadMesh(const std::string& filePath, rt::Instance& raytracerInstance, MaterialsList& outMaterials, const Float scale)
{
    RT_LOG_DEBUG("Loading mesh file: '%s'...", filePath.c_str());

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    const std::string meshBaseDir = filePath.substr(0, filePath.find_last_of("\\/")) + "/";

    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filePath.c_str(), meshBaseDir.c_str(), true);
    if (!err.empty())
    {
        RT_LOG_INFO("Mesh '%s' loading message:\n%s", filePath.c_str(), err.c_str());
    }
    if (!ret)
    {
        RT_LOG_ERROR("Failed to load mesh '%s'", filePath.c_str());
        return nullptr;
    }


    std::vector<Uint32> vertexIndices;
    std::vector<Uint32> materialIndices;

    int numMappedIndices = 0;
    std::vector<float> vertexPositions;
    std::vector<float> vertexNormals;
    std::vector<float> vertexTangents;
    std::vector<float> vertexTexCoords;
    std::vector<const rt::Material*> materialPointers;

    Uint32 totalIndices = 0;
    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++)
    {
        // Loop over faces
        Uint32 indexOffset = 0;
        const auto& mesh = shapes[s].mesh;

        for (size_t f = 0; f < mesh.num_face_vertices.size(); f++)
        {
            int fv = mesh.num_face_vertices[f];
            if (fv != 3)
            {
                RT_LOG_ERROR("Expected only triangles (shape index = %zu, face index = %zu)", s, f);
                return nullptr;
            }

            const tinyobj::index_t idx[3] =
            {
                mesh.indices[indexOffset + 0],
                mesh.indices[indexOffset + 1],
                mesh.indices[indexOffset + 2],
            };

            bool hasNormals = idx[0].normal_index != -1 && idx[1].normal_index != -1 && idx[2].normal_index != -1;
            bool hasTexCoords = idx[0].texcoord_index != -1 && idx[1].texcoord_index != -1 && idx[2].texcoord_index != -1;

            // Loop over vertices in the face.
            math::Vector4 verts[3];
            for (size_t i = 0; i < 3; i++)
            {
                verts[i] = math::Vector4(
                    attrib.vertices[3 * idx[i].vertex_index + 0],
                    attrib.vertices[3 * idx[i].vertex_index + 1],
                    attrib.vertices[3 * idx[i].vertex_index + 2]);

                vertexPositions.push_back(verts[i][0]);
                vertexPositions.push_back(verts[i][1]);
                vertexPositions.push_back(verts[i][2]);
            }

            math::Vector4 normals[3];
            if (hasNormals)
            {
                // OBJ has normals
                for (size_t i = 0; i < 3; i++)
                {
                    normals[i] = math::Vector4(
                        attrib.normals[3 * idx[i].normal_index + 0],
                        attrib.normals[3 * idx[i].normal_index + 1],
                        attrib.normals[3 * idx[i].normal_index + 2]);
                }
            }
            else
            {
                // calculate normal
                math::Vector4 normal = math::Vector4::Cross3(verts[1] - verts[0], verts[2] - verts[0]);
                normal.Normalize3();

                normals[0] = normal;
                normals[1] = normal;
                normals[2] = normal;
            }

            for (size_t i = 0; i < 3; i++)
            {
                vertexNormals.push_back(normals[i][0]);
                vertexNormals.push_back(normals[i][1]);
                vertexNormals.push_back(normals[i][2]);
            }

            // calculate tangent vector
            // TODO use tex coord !!!
            const math::Vector4 tangent = (verts[2] - verts[0]).Normalized3();
            for (size_t i = 0; i < 3; i++)
            {
                vertexTangents.push_back(tangent[0]);
                vertexTangents.push_back(tangent[1]);
                vertexTangents.push_back(tangent[2]);
            }

            /*
            if (idx.texcoord_index >= 0)
            {
                vertexTexCoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                vertexTexCoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
            }
            */

            // TODO compress vertex data (right now index buffer is useless)
            vertexIndices.push_back(totalIndices + indexOffset + 0);
            vertexIndices.push_back(totalIndices + indexOffset + 1);
            vertexIndices.push_back(totalIndices + indexOffset + 2);

            materialIndices.push_back(mesh.material_ids[f]);

            indexOffset += fv;
        }

        totalIndices += indexOffset;
    }

    // load materials
    materialPointers.reserve(materials.size());
    outMaterials.reserve(materials.size());
    for (size_t i = 0; i < materials.size(); i++)
    {
        auto material = LoadMaterial(raytracerInstance, materials[i]);
        materialPointers.push_back(material.get());
        outMaterials.emplace_back(std::move(material));
    }


    RT_LOG_DEBUG("Mesh file '%s' loaded, shapes = %zu, materials = %zu, indices = %u",
                 filePath.c_str(), shapes.size(), materials.size(), totalIndices);

    rt::MeshDesc meshDesc;
    meshDesc.path = filePath;
    meshDesc.vertexBufferDesc.numTriangles = static_cast<Uint32>(vertexIndices.size() / 3);
    meshDesc.vertexBufferDesc.numVertices = static_cast<Uint32>(vertexPositions.size() / 3);
    meshDesc.vertexBufferDesc.numMaterials = static_cast<Uint32>(materials.size());
    meshDesc.vertexBufferDesc.materials = materialPointers.data();
    meshDesc.vertexBufferDesc.materialIndexBuffer = materialIndices.data();
    meshDesc.vertexBufferDesc.vertexIndexBuffer = vertexIndices.data();
    meshDesc.vertexBufferDesc.positions = vertexPositions.data();
    meshDesc.vertexBufferDesc.normals = vertexNormals.data();
    meshDesc.vertexBufferDesc.tangents = vertexTangents.data();
    //meshDesc.vertexBufferDesc.texCoords = texCoords;
    meshDesc.vertexBufferDesc.positionsFormat = rt::VertexDataFormat::Float;
    meshDesc.vertexBufferDesc.normalsFormat = rt::VertexDataFormat::Float;
    meshDesc.vertexBufferDesc.tangentsFormat = rt::VertexDataFormat::Float;
    meshDesc.vertexBufferDesc.vertexIndexFormat = rt::IndexDataFormat::Int32;
    meshDesc.vertexBufferDesc.materialIndexFormat = rt::VertexDataFormat::Int32;
    meshDesc.vertexBufferDesc.scale = scale;

    std::unique_ptr<rt::IMesh> mesh = raytracerInstance.CreateMesh();
    bool result = mesh->Initialize(meshDesc);
    if (!result)
    {
        return nullptr;
    }

    return mesh;
}

} // namespace helpers