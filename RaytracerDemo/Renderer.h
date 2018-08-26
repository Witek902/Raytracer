#pragma once

#include "../RaytracerLib/Common.h"
#include "../RaytracerLib/Math/Random.h"
#include "../RaytracerLib/Rendering/Viewport.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

template<typename T>
using D3DPtr = Microsoft::WRL::ComPtr<T>;

class RT_ALIGN(32) Renderer : public Aligned<32>
{
public:
    Renderer();
    ~Renderer();
    bool Init(HWND window);
    bool OnWindowResized(Uint32 width, Uint32 height);
    bool Render(const float* pixels, const rt::PostprocessParams& postProcessParams, Uint32 frameNumber);

    // TODO passing raw image + parameters

private:
    bool CreateComputeShader();
    bool CreateSourceTexture();

    Uint32 mWidth;
    Uint32 mHeight;

    D3DPtr<ID3D11Device> mDevice;
    D3DPtr<ID3D11DeviceContext> mDeviceContext;
    D3DPtr<IDXGISwapChain > mSwapChain;

    D3DPtr<ID3D11RenderTargetView> mRenderTarget;
    D3DPtr<ID3D11UnorderedAccessView> mUAV;

    // compute shader for post-processing
    D3DPtr<ID3D11ComputeShader> mComputeShader;
    D3DPtr<ID3D11Buffer> mPostProcessCBuffer;

    D3DPtr<ID3D11Texture2D> mSourceTexture;
    D3DPtr<ID3D11ShaderResourceView> mSourceTextureView;

    rt::math::Random random;
    float* mTempData;
};