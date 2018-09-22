#include "PCH.h"
#include "Renderer.h"
#include "Window.h"
#include "../RaytracerLib/Utils/Logger.h"
#include "../External/imgui/imgui_impl_dx11.h"

namespace {

struct RT_ALIGN(16) PostProcessCBuffer
{
    rt::math::Vector4 exposure;
};

} // namespace

Renderer::Renderer()
    : mWidth(0)
    , mHeight(0)
{
}

Renderer::~Renderer()
{

}

bool Renderer::Init(Window& window)
{
    window.GetSize(mWidth, mHeight);

    mTempData = (float*)_aligned_malloc(mWidth * mHeight * 4 * sizeof(float), 4096);

    DXGI_SWAP_CHAIN_DESC scd;
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
    scd.OutputWindow = reinterpret_cast<HWND>(window.GetHandle());
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    UINT32 flags = 0;
#ifdef _DEBUG
    flags = D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        flags,
        NULL,
        NULL,
        D3D11_SDK_VERSION,
        &scd,
        mSwapChain.GetAddressOf(),
        mDevice.GetAddressOf(),
        NULL,
        mDeviceContext.GetAddressOf());

    if (FAILED(hr))
    {
        RT_LOG_INFO("Failed to initialize D3D device.");
        return false;
    }

    // create render target for the back buffer
    ID3D11Texture2D* backBuffer;
    if (FAILED(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer)))
    {
        RT_LOG_INFO("Failed to obtain back buffer texture.");
        return false;
    }

    if (FAILED(mDevice->CreateRenderTargetView(backBuffer, NULL, mRenderTarget.GetAddressOf())))
    {
        RT_LOG_INFO("Failed to create render target for the back buffer.");
        return false;
    }
    if (FAILED(mDevice->CreateUnorderedAccessView(backBuffer, NULL, mUAV.GetAddressOf())))
    {
        RT_LOG_INFO("Failed to create render target for the back buffer.");
        return false;
    }
    backBuffer->Release();

    if (!CreateSourceTexture())
        return false;

    if (!CreateBuffers())
        return false;

    if (!CreateComputeShader())
        return false;

    ImGui_ImplDX11_Init(mDevice.Get(), mDeviceContext.Get());

    RT_LOG_INFO("Renderer initialized.");
    return true;
}

bool Renderer::OnWindowResized(Uint32 width, Uint32 height)
{
    mWidth = width;
    mHeight = height;

    ImGui_ImplDX11_InvalidateDeviceObjects();

    if (mRenderTarget)
    {
        mRenderTarget->Release();
        mUAV->Release();

        mSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

        ID3D11Texture2D* pBackBuffer;
        mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
        mDevice->CreateRenderTargetView(pBackBuffer, NULL, mRenderTarget.GetAddressOf());
        mDevice->CreateUnorderedAccessView(pBackBuffer, NULL, mUAV.GetAddressOf());
        pBackBuffer->Release();
    }

    CreateSourceTexture();

    ImGui_ImplDX11_CreateDeviceObjects();

    return true;
}

bool Renderer::Render(const float* pixels, const rt::PostprocessParams& postProcessParams, Uint32 frameNumber)
{
    // upload source texture to the GPU
    {
        D3D11_MAPPED_SUBRESOURCE mapped;

        mDeviceContext->Map(mSourceTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        for (Uint32 i = 0; i < mHeight; ++i)
        {
            char* target = (char*)mapped.pData + mapped.RowPitch * i;
            const float* source = pixels + mWidth * i * 4;
            memcpy(target, source, mWidth * 4 * sizeof(float));
        }
        mDeviceContext->Unmap(mSourceTexture.Get(), 0);
    }

    // setup c-buffer
    {
        PostProcessCBuffer cbuffer;
        cbuffer.exposure = rt::math::Vector4(exp2f(postProcessParams.exposure) / (float)frameNumber);
        mDeviceContext->UpdateSubresource(mPostProcessCBuffer.Get(), 0, NULL, &cbuffer, 0, 0);
    }

    {
        mDeviceContext->CSSetUnorderedAccessViews(0, 1, mUAV.GetAddressOf(), NULL);
        mDeviceContext->CSSetShaderResources(0, 1, mSourceTextureView.GetAddressOf());
        mDeviceContext->CSSetConstantBuffers(0, 1, mPostProcessCBuffer.GetAddressOf());
        mDeviceContext->CSSetShader(mComputeShader.Get(), NULL, 0);
        mDeviceContext->Dispatch(1 + mWidth / 32, 1 + mHeight / 32, 1);

        ID3D11UnorderedAccessView* nullUAV = nullptr;
        mDeviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, NULL);
    }

    {
        mDeviceContext->OMSetRenderTargets(1, mRenderTarget.GetAddressOf(), NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        mDeviceContext->OMSetRenderTargets(0, NULL, NULL);
    }

    return SUCCEEDED(mSwapChain->Present(0, 0));
}

bool Renderer::CreateSourceTexture()
{
    mSourceTextureView.Reset();
    mSourceTexture.Reset();

    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
    texDesc.Width = mWidth;
    texDesc.Height = mHeight;
    texDesc.SampleDesc.Count = 1;
    texDesc.ArraySize = 1;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.MipLevels = 1;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    texDesc.Usage = D3D11_USAGE_DYNAMIC;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

    if (FAILED(mDevice->CreateTexture2D(&texDesc, NULL, mSourceTexture.GetAddressOf())))
    {
        RT_LOG_INFO("Failed to create source texture object.");
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    if (FAILED(mDevice->CreateShaderResourceView(mSourceTexture.Get(), &srvDesc, mSourceTextureView.GetAddressOf())))
    {
        RT_LOG_INFO("Failed to create source texture view object.");
        return false;
    }

    return true;
}

bool Renderer::CreateBuffers()
{
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.ByteWidth = sizeof(PostProcessCBuffer);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    if (FAILED(mDevice->CreateBuffer(&bufferDesc, NULL, mPostProcessCBuffer.GetAddressOf())))
    {
        RT_LOG_INFO("Failed to create post process c-buffer.");
        return false;
    }

    return true;
}

bool Renderer::CreateComputeShader()
{
    static const char* computeShader =
R"(
Texture2D<float3> inputBuffer : register(t0);
RWTexture2D<float3> outputBuffer : register(u0);

cbuffer MyBuffer : register(b0)
{
    float4 g_exposure;
}

static uint rng_state;

uint Hash(uint a)
{
    a = (a ^ 61) ^ (a >> 16);
    a += (a << 3);
    a ^= (a >> 4);
    a *= 0x27d4eb2d;
    a ^= (a >> 15);
    return a;
}

uint Rand()
{
    rng_state ^= (rng_state << 13);
    rng_state ^= (rng_state >> 17);
    rng_state ^= (rng_state << 5);
    return rng_state;
}

uint3 Rand3()
{
    return uint3(Rand(), Rand(), Rand());
}

float3 xyz2rgb(float3 c)
{
    const float3x3 mat = float3x3(
        3.240479f, -1.537150f, -0.498535f,
        -0.969256f,  1.875991f,  0.041556f,
        0.055648f, -0.204043f,  1.057311f
    );
    return max(float3(0.0, 0.0, 0.0), mul(c, transpose(mat)));
}

float3 ToneMap(float3 x)
{
    // Jim Hejl and Richard Burgess-Dawson formula
    return x * (6.2 * x + 0.5) / (x * (6.2 * x + 1.7) + 0.06);
}

[numthreads(32, 32, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    float3 input = xyz2rgb(inputBuffer.Load(int3(id.xy, 0)));

    rng_state = Hash((id.x << 16) + id.y);
    float3 dither = float3(Rand3()) * (1.0 / 4294967296.0);

    outputBuffer[id.xy] = ToneMap(input * g_exposure.xyz) + 0.025 * (dither - 0.5);
}
)";

    D3DPtr<ID3DBlob> blob, error, disasm;
    D3DCompile(computeShader, strlen(computeShader), NULL, NULL, NULL, "main", "cs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, blob.GetAddressOf(), error.GetAddressOf());
    if (error)
    {
        OutputDebugStringA((const char*)error->GetBufferPointer());
    }
    if (!blob)
    {
        RT_LOG_ERROR("Failed to compile compute shader");
        return false;
    }

    /*
    D3DDisassemble((DWORD*)blob->GetBufferPointer(), blob->GetBufferSize(), 0, NULL, disasm.GetAddressOf());
    if (disasm)
    {
        OutputDebugStringA((const char*)disasm->GetBufferPointer());
    }
    */

    if (mDevice->CreateComputeShader((DWORD*)blob->GetBufferPointer(), blob->GetBufferSize(), NULL, mComputeShader.GetAddressOf()) != S_OK)
    {
        RT_LOG_ERROR("Failed to create compute shader");
        return false;
    }

    return true;
}
