#include "PCH.h"
#include "Material.h"
#include "BSDF/GlossyReflectiveBSDF.h"
#include "BSDF/SpecularTransmissiveBSDF.h"
#include "BSDF/OrenNayarBSDF.h"
#include "Mesh/Mesh.h"
#include "Rendering/ShadingData.h"
#include "Utils/Bitmap.h"
#include "Math/Random.h"
#include "Math/Utils.h"

namespace rt {

using namespace math;

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

Material::~Material() = default;
Material::Material(Material&&) = default;
Material& Material::operator = (Material&&) = default;

void Material::Compile()
{
    emission.baseValue = Vector4::Max(Vector4(), emission.baseValue);
    baseColor.baseValue = Vector4::Max(Vector4(), Vector4::Min(VECTOR_ONE, baseColor.baseValue));

    if (transparent)
    {
        mDiffuseBSDF = std::make_unique<SpecularTransmissiveBSDF>();
    }
    else
    {
        mDiffuseBSDF = std::make_unique<OrenNayarBSDF>();
    }

    mSpecularBSDF = std::make_unique<GlossyReflectiveBSDF>();
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
        normal.z = Sqrt(1.0f - normal.x * normal.x - normal.y * normal.y);
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

const Color Material::Evaluate(
    const Wavelength& wavelength,
    const ShadingData& shadingData,
    const Vector4& outgoingDirWorldSpace,
    const Vector4& incomingDirWorldSpace,
    Float* outPdfW) const
{
    // TODO this is already done in Sample()
    const Vector4 outgoingDirLocalSpace = shadingData.WorldToLocal(outgoingDirWorldSpace);
    const Vector4 incomingDirLocalSpace = shadingData.WorldToLocal(incomingDirWorldSpace);
    const float NdotV = outgoingDirLocalSpace.z;

    if (outgoingDirLocalSpace.z < FLT_EPSILON || incomingDirLocalSpace.z > FLT_EPSILON)
    {
        return Color();
    }

    if (outPdfW)
    {
        *outPdfW = 0.0f;
    }

    Vector4 metalValue;
    Vector4 dielectricValue;

    const Vector4 baseColorValue = baseColor.Evaluate(shadingData.texCoord);
    const Float metalnessValue = metalness.Evaluate(shadingData.texCoord);

    SampledMaterialParameters materialParam;
    materialParam.roughness = roughness.Evaluate(shadingData.texCoord);
    materialParam.IoR = IoR;

    const BSDF::EvaluationContext samplingContext =
    {
        *this,
        materialParam,
        wavelength,
        outgoingDirLocalSpace,
        incomingDirLocalSpace
    };

    if (metalnessValue > 0.0f)
    {
        metalValue = baseColorValue;
        metalValue *= mSpecularBSDF->Evaluate(samplingContext, outPdfW);
        metalValue *= FresnelMetal(NdotV, IoR, K);
    }

    if (metalnessValue < 1.0f)
    {
        bool totalInternalReflection = false;
        const float F = FresnelDielectric(NdotV, IoR, totalInternalReflection);
        const float specularWeight = totalInternalReflection ? 1.0f : F;
        const float diffuseWeight = 1.0f - specularWeight;

        float specularPdf;
        dielectricValue = specularWeight * mSpecularBSDF->Evaluate(samplingContext, &specularPdf);

        float diffusePdf;
        dielectricValue += diffuseWeight * baseColorValue * mDiffuseBSDF->Evaluate(samplingContext, &diffusePdf);

        if (outPdfW)
        {
            *outPdfW = specularWeight * specularPdf + diffuseWeight * diffusePdf;
        }
    }

    const Vector4 value = Vector4::Lerp(dielectricValue, metalValue, metalnessValue);
    return Color::SampleRGB(wavelength, value);
}

const Color Material::Sample(
    Wavelength& wavelength,
    const Vector4& outgoingDirWorldSpace,
    Vector4& outIncomingDirWorldSpace,
    const ShadingData& shadingData,
    Random& randomGenerator,
    Float& outPdfW,
    BSDF::EventType& outSampledEvent) const
{
    const Vector4 outgoingDirLocalSpace = shadingData.WorldToLocal(outgoingDirWorldSpace);
    const float NdotV = outgoingDirLocalSpace.z;

    const BSDF* bsdf = nullptr;
    float bsdfPdf = 1.0f;

    Vector4 value; // TODO spectral color definitions

    const Vector4 baseColorValue = baseColor.Evaluate(shadingData.texCoord);
    const Float metalnessValue = metalness.Evaluate(shadingData.texCoord);

    // TODO enclose into "FresnelBSDF"
    if (randomGenerator.GetFloat() < metalnessValue)
    {
        if (NdotV > 0.0f)
        {
            value = baseColorValue;
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
            value = VECTOR_ONE;
            bsdf = mSpecularBSDF.get();
            //bsdfPdf = F;
        }
        else // diffuse reflection / refraction
        {
            value = baseColorValue;
            bsdf = mDiffuseBSDF.get();
            //bsdfPdf = 1.0f - F;
        }
    }

    SampledMaterialParameters materialParam;
    materialParam.roughness = roughness.Evaluate(shadingData.texCoord);
    materialParam.IoR = IoR;

    BSDF::SamplingContext samplingContext =
    {
        *this,
        materialParam,
        outgoingDirLocalSpace,
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
    outPdfW = samplingContext.outPdf * bsdfPdf;
    outSampledEvent = samplingContext.outEventType;

    return samplingContext.outColor * Color::SampleRGB(wavelength, value);
}

///


} // namespace rt
