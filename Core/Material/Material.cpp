#include "PCH.h"
#include "Material.h"

#include "BSDF/DiffuseBSDF.h"
#include "BSDF/RoughDiffuseBSDF.h"
#include "BSDF/DielectricBSDF.h"
#include "BSDF/RoughDielectricBSDF.h"
#include "BSDF/MetalBSDF.h"
#include "BSDF/RoughMetalBSDF.h"

#include "Mesh/Mesh.h"
#include "Rendering/ShadingData.h"
#include "Utils/Bitmap.h"
#include "Utils/Logger.h"
#include "Math/Random.h"

namespace rt {

using namespace math;

const char* Material::DefaultBsdfName = "diffuse";

DispersionParams::DispersionParams()
{
    // BK7 glass
    C = 0.00420f;
    D = 0.0f;
}

Material::Material(const char* debugName)
    : debugName(debugName)
{
}

MaterialPtr Material::Create()
{
    return MaterialPtr(new Material);
}

void Material::SetBsdf(const std::string& bsdfName)
{
    if (bsdfName == "diffuse")
    {
        mBSDF = std::make_unique<DiffuseBSDF>();
    }
    else if (bsdfName == "roughDiffuse")
    {
        mBSDF = std::make_unique<RoughDiffuseBSDF>();
    }
    else if (bsdfName == "dielectric")
    {
        mBSDF = std::make_unique<DielectricBSDF>();
    }
    else if (bsdfName == "roughDielectric")
    {
        mBSDF = std::make_unique<RoughDielectricBSDF>();
    }
    else if (bsdfName == "metal")
    {
        mBSDF = std::make_unique<MetalBSDF>();
    }
    else if (bsdfName == "roughMetal")
    {
        mBSDF = std::make_unique<RoughMetalBSDF>();
    }
    else
    {
        RT_LOG_ERROR("Unknown BSDF name: '%s'", bsdfName.c_str());
    }
}

static std::unique_ptr<Material> CreateDefaultMaterial()
{
    std::unique_ptr<Material> material = std::make_unique<Material>("default");
    material->SetBsdf(Material::DefaultBsdfName);
    material->Compile();
    return material;
}

const Material* Material::GetDefaultMaterial()
{
    static std::unique_ptr<Material> sDefaultMaterial = CreateDefaultMaterial();
    return sDefaultMaterial.get();
}

Material::~Material()
{
    RT_LOG_INFO("Releasing material '%s'", debugName.c_str());
}

Material::Material(Material&&) = default;
Material& Material::operator = (Material&&) = default;

void Material::Compile()
{
    RT_ASSERT(emission.baseValue.IsValid());
    RT_ASSERT(baseColor.baseValue.IsValid());
    RT_ASSERT(IsValid(roughness.baseValue));
    RT_ASSERT(IsValid(metalness.baseValue));
    RT_ASSERT(IsValid(normalMapStrength) && normalMapStrength >= 0.0f);
    RT_ASSERT(IsValid(IoR) && IoR >= 0.0f);
    RT_ASSERT(IsValid(K) && K >= 0.0f);

    emission.baseValue = Vector4::Max(Vector4::Zero(), emission.baseValue);
    baseColor.baseValue = Vector4::Max(Vector4::Zero(), Vector4::Min(VECTOR_ONE, baseColor.baseValue));
}

const Vector4 Material::GetNormalVector(const Vector4 uv) const
{
    Vector4 normal(0.0f, 0.0f, 1.0f, 0.0f);

    if (normalMap)
    {
        SamplerDesc sampler;
        sampler.forceLinearSpace = true;

        normal = normalMap->Sample(uv, sampler);

        // scale from [0...1] to [-1...1]
        normal += normal;
        normal -= VECTOR_ONE;

        // reconstruct Z
        normal.z = Sqrt(Max(0.0f, 1.0f - normal.x * normal.x - normal.y * normal.y));

        normal = Vector4::Lerp(VECTOR_Z, normal, normalMapStrength);
    }

    return normal;
}

bool Material::GetMaskValue(const Vector4 uv) const
{
    if (maskMap)
    {
        const float maskTreshold = 0.5f;
        return maskMap->Sample(uv, SamplerDesc()).x > maskTreshold;
    }

    return true;
}

void Material::EvaluateShadingData(const Wavelength& wavelength, ShadingData& shadingData) const
{
    shadingData.materialParams.baseColor = Color::SampleRGB(wavelength, baseColor.Evaluate(shadingData.texCoord));
    shadingData.materialParams.roughness = roughness.Evaluate(shadingData.texCoord);
    shadingData.materialParams.metalness = metalness.Evaluate(shadingData.texCoord);
    shadingData.materialParams.IoR = IoR;
}

const Color Material::Evaluate(
    const Wavelength& wavelength,
    const ShadingData& shadingData,
    const Vector4& incomingDirWorldSpace,
    Float* outPdfW) const
{
    RT_ASSERT(mBSDF, "Material must have a BSDF assigned");

    const Vector4 incomingDirLocalSpace = shadingData.WorldToLocal(incomingDirWorldSpace);

    const BSDF::EvaluationContext evalContext =
    {
        *this,
        shadingData.materialParams,
        wavelength,
        shadingData.outgoingDirLocalSpace,
        incomingDirLocalSpace
    };

    return mBSDF->Evaluate(evalContext, outPdfW);
}

const Color Material::Sample(
    Wavelength& wavelength,
    Vector4& outIncomingDirWorldSpace,
    const ShadingData& shadingData,
    Random& randomGenerator,
    Float& outPdfW,
    BSDF::EventType& outSampledEvent) const
{
    RT_ASSERT(mBSDF, "Material must have a BSDF assigned");

    BSDF::SamplingContext samplingContext =
    {
        *this,
        shadingData.materialParams,
        shadingData.outgoingDirLocalSpace,
        wavelength,
        randomGenerator,
    };

    // BSDF sampling (in local space)
    if (!mBSDF->Sample(samplingContext))
    {
        outSampledEvent = BSDF::NullEvent;
        return Color();
    }

    RT_ASSERT(IsValid(samplingContext.outPdf));
    RT_ASSERT(samplingContext.outPdf >= 0.0f);
    RT_ASSERT(samplingContext.outIncomingDir.IsValid());
    RT_ASSERT(samplingContext.outColor.IsValid());

    // convert incoming light direction back to world space
    outIncomingDirWorldSpace = shadingData.LocalToWorld(samplingContext.outIncomingDir);
    outPdfW = samplingContext.outPdf;
    outSampledEvent = samplingContext.outEventType;

    return samplingContext.outColor;
}

///


} // namespace rt
