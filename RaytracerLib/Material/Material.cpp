#include "PCH.h"
#include "Material.h"
#include "BSDF/GlossyReflectiveBSDF.h"
#include "BSDF/SpecularReflectiveBSDF.h"
#include "BSDF/SpecularTransmissiveBSDF.h"
#include "BSDF/OrenNayarBSDF.h"
#include "Mesh/Mesh.h"
#include "Rendering/ShadingData.h"
#include "Utils/Bitmap.h"
#include "Utils/Logger.h"
#include "Math/Random.h"
#include "Math/Utils.h"

namespace rt {

using namespace math;

static const Material gDefaultMaterial;

DispersionParams::DispersionParams()
{
    // BK7 glass
    B[0] = 1.03961212f;
    B[1] = 0.231792344f;
    B[2] = 1.01046945f;
    C[0] = 6.00069867e-3f;
    C[1] = 2.00179144e-2f;
    C[2] = 1.03560653e+2f;
}

Material::Material(const char* debugName)
    : debugName(debugName)
{
}

MaterialPtr Material::Create()
{
    return MaterialPtr(new Material);
}

const Material* Material::GetDefaultMaterial()
{
    return &gDefaultMaterial;
}

Material::~Material()
{
    RT_LOG_INFO("Releasing material '%s'", debugName.c_str());
}

Material::Material(Material&&) = default;
Material& Material::operator = (Material&&) = default;

void Material::Compile()
{
    emission.baseValue = Vector4::Max(Vector4::Zero(), emission.baseValue);
    baseColor.baseValue = Vector4::Max(Vector4::Zero(), Vector4::Min(VECTOR_ONE, baseColor.baseValue));

    if (transparent)
    {
        mDiffuseBSDF = std::make_unique<SpecularTransmissiveBSDF>();
    }
    else
    {
        mDiffuseBSDF = std::make_unique<OrenNayarBSDF>();
    }

    if (roughness.baseValue >= 0.01f)
    {
        mSpecularBSDF = std::make_unique<GlossyReflectiveBSDF>();
    }
    else
    {
        mSpecularBSDF = std::make_unique<SpecularReflectiveBSDF>();
    }
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

Bool Material::GetMaskValue(const Vector4 uv) const
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
    shadingData.materialParams.roughness = 1.0f - roughness.Evaluate(shadingData.texCoord);
    shadingData.materialParams.metalness = metalness.Evaluate(shadingData.texCoord);
    shadingData.materialParams.IoR = IoR;
}

const Color Material::Evaluate(
    const Wavelength& wavelength,
    const ShadingData& shadingData,
    const Vector4& incomingDirWorldSpace,
    Float* outPdfW) const
{
    const Vector4 incomingDirLocalSpace = shadingData.WorldToLocal(incomingDirWorldSpace);
    const float NdotV = shadingData.outgoingDirLocalSpace.z;

    if (NdotV < FLT_EPSILON || incomingDirLocalSpace.z > FLT_EPSILON)
    {
        return Color::Zero();
    }

    if (outPdfW)
    {
        *outPdfW = 0.0f;
    }

    Color metalValue = Color::Zero();
    Color dielectricValue = Color::Zero();

    const BSDF::EvaluationContext samplingContext =
    {
        *this,
        shadingData.materialParams,
        wavelength,
        shadingData.outgoingDirLocalSpace,
        incomingDirLocalSpace
    };

    if (shadingData.materialParams.metalness > 0.0f)
    {
        metalValue = shadingData.materialParams.baseColor;
        metalValue *= mSpecularBSDF->Evaluate(samplingContext, outPdfW);
        metalValue *= FresnelMetal(NdotV, IoR, K);
    }

    if (shadingData.materialParams.metalness < 1.0f)
    {
        bool totalInternalReflection = false;
        const float F = FresnelDielectric(NdotV, IoR, totalInternalReflection);
        const float specularWeight = totalInternalReflection ? 1.0f : F;
        const float diffuseWeight = 1.0f - specularWeight;

        float specularPdf;
        dielectricValue = specularWeight * mSpecularBSDF->Evaluate(samplingContext, &specularPdf);

        float diffusePdf;
        dielectricValue += diffuseWeight * shadingData.materialParams.baseColor * mDiffuseBSDF->Evaluate(samplingContext, &diffusePdf);

        if (outPdfW)
        {
            *outPdfW = specularWeight * specularPdf + diffuseWeight * diffusePdf;
        }
    }

    return Color::Lerp(dielectricValue, metalValue, shadingData.materialParams.metalness);
}

const Color Material::Sample(
    Wavelength& wavelength,
    Vector4& outIncomingDirWorldSpace,
    const ShadingData& shadingData,
    Random& randomGenerator,
    Float& outPdfW,
    BSDF::EventType& outSampledEvent) const
{
    const float NdotV = shadingData.outgoingDirLocalSpace.z;

    const BSDF* bsdf = nullptr;
    Color value;

    // TODO enclose into "FresnelBSDF"
    if (randomGenerator.GetFloat() < shadingData.materialParams.metalness)
    {
        if (NdotV > 0.0f)
        {
            value = shadingData.materialParams.baseColor;
            value *= FresnelMetal(NdotV, IoR, K);
        }
        bsdf = mSpecularBSDF.get();
    }
    else
    {
        bool totalInternalReflection = false;
        const float F = FresnelDielectric(NdotV, IoR, totalInternalReflection);

        if (randomGenerator.GetFloat() < F || totalInternalReflection) // glossy reflection
        {
            value = Color::One();
            bsdf = mSpecularBSDF.get();
        }
        else // diffuse reflection / refraction
        {
            value = shadingData.materialParams.baseColor;
            bsdf = mDiffuseBSDF.get();
        }
    }

    BSDF::SamplingContext samplingContext =
    {
        *this,
        shadingData.materialParams,
        shadingData.outgoingDirLocalSpace,
        wavelength,
        randomGenerator,
    };

    // BSDF sampling (in local space)
    if (!bsdf->Sample(samplingContext))
    {
        return Color();
    }

    RT_ASSERT(IsValid(samplingContext.outPdf));
    RT_ASSERT(samplingContext.outPdf > 0.0f);
    RT_ASSERT(samplingContext.outIncomingDir.IsValid());
    RT_ASSERT(samplingContext.outColor.IsValid());

    // convert incoming light direction back to world space
    outIncomingDirWorldSpace = shadingData.LocalToWorld(samplingContext.outIncomingDir);
    outPdfW = samplingContext.outPdf;
    outSampledEvent = samplingContext.outEventType;

    return samplingContext.outColor * value;
}

///


} // namespace rt
