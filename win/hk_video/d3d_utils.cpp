#include "framework.h"

#include "d3d_utils.h"
#include "tc_common/log.h"

using namespace tc;

static DXGI_FORMAT EnsureNotTypeless(DXGI_FORMAT format) noexcept {
    // Assumes UNORM or FLOAT; doesn't use UINT or SINT
    switch (format) {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case DXGI_FORMAT_R32G32B32_TYPELESS:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case DXGI_FORMAT_R32G32_TYPELESS:
            return DXGI_FORMAT_R32G32_FLOAT;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
            return DXGI_FORMAT_R10G10B10A2_UNORM;
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_R16G16_TYPELESS:
            return DXGI_FORMAT_R16G16_UNORM;
        case DXGI_FORMAT_R32_TYPELESS:
            return DXGI_FORMAT_R32_FLOAT;
        case DXGI_FORMAT_R8G8_TYPELESS:
            return DXGI_FORMAT_R8G8_UNORM;
        case DXGI_FORMAT_R16_TYPELESS:
            return DXGI_FORMAT_R16_UNORM;
        case DXGI_FORMAT_R8_TYPELESS:
            return DXGI_FORMAT_R8_UNORM;
        case DXGI_FORMAT_BC1_TYPELESS:
            return DXGI_FORMAT_BC1_UNORM;
        case DXGI_FORMAT_BC2_TYPELESS:
            return DXGI_FORMAT_BC2_UNORM;
        case DXGI_FORMAT_BC3_TYPELESS:
            return DXGI_FORMAT_BC3_UNORM;
        case DXGI_FORMAT_BC4_TYPELESS:
            return DXGI_FORMAT_BC4_UNORM;
        case DXGI_FORMAT_BC5_TYPELESS:
            return DXGI_FORMAT_BC5_UNORM;
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            return DXGI_FORMAT_B8G8R8X8_UNORM;
        case DXGI_FORMAT_BC7_TYPELESS:
            return DXGI_FORMAT_BC7_UNORM;
        default:
            return format;
    }
}

HRESULT CaptureTexture(ID3D11Device *device,
                       ID3D11DeviceContext *context,
                       ID3D11Resource *source,
                       D3D11_TEXTURE2D_DESC &desc,
                       CComPtr<ID3D11Texture2D> &staging,
                       std::shared_ptr<tc::SharedTexture> shared_texture) {

    assert(nullptr != context);
    assert(nullptr != source);

    if (nullptr == context || nullptr == source) {
        return E_INVALIDARG;
    }

    D3D11_RESOURCE_DIMENSION type = D3D11_RESOURCE_DIMENSION_UNKNOWN;
    source->GetType(&type);

    if (D3D11_RESOURCE_DIMENSION_TEXTURE2D != type) {
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }

    CComPtr<ID3D11Texture2D> acquired_texture;
    HRESULT hr = source->QueryInterface(IID_PPV_ARGS(&acquired_texture));
    if (FAILED(hr)) {
        return hr;
    }

    assert(acquired_texture);
    acquired_texture->GetDesc(&desc);

    //LOGI("Capture acquired_texture : {}", (int) desc.Format);

    if (desc.SampleDesc.Count > 1) {
        // MSAA content must be resolved before being copied to a staging texture
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        CComPtr<ID3D11Texture2D> new_texture;
        hr = device->CreateTexture2D(&desc, nullptr, &new_texture);
        if (FAILED(hr)) {
            return hr;
        }
        assert(new_texture);

        DXGI_FORMAT format = EnsureNotTypeless(desc.Format);
        LOGI("Capture texture 2 : {}", (int) format);

        UINT support = 0;
        hr = device->CheckFormatSupport(format, &support);
        if (FAILED(hr)) {
            LOGE("! support format");
            return hr;
        }
        if (!(support & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE)) {
            LOGE("! D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE");
            return E_FAIL;
        }

        for (UINT item = 0; item < desc.ArraySize; ++item) {
            for (UINT level = 0; level < desc.MipLevels; ++level) {
                UINT index = D3D11CalcSubresource(level, item, desc.MipLevels);
                context->ResolveSubresource(new_texture, index, source, index, format);
            }
        }

        LOGI("CreateTexture2D desc.SampleDesc.Count > 1");

        desc.BindFlags = 0;
        desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.Usage = D3D11_USAGE_STAGING;
        hr = device->CreateTexture2D(&desc, nullptr, &staging);
        if (FAILED(hr))
            return hr;

        assert(staging);

        context->CopyResource(staging, new_texture);

        //
        shared_texture->CopyCapturedTexture(device, context, new_texture);
    } else if ((desc.Usage == D3D11_USAGE_STAGING) && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ)) {
        // Handle case where the source is already a staging texture we can use
        // directly
        staging = acquired_texture;
        LOGI("staging = acquired_texture;");

        //
        shared_texture->CopyCapturedTexture(device, context, acquired_texture);
    } else {
        // Otherwise, create a staging texture from the non-MSAA source
        // 导致编码非常慢
        //// desc.BindFlags = 0;
        //// desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
        //// //desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        //// desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        //// desc.Usage = D3D11_USAGE_STAGING;
        ////
        //// //LOGI("CreateTexture2D default");
        ////
        //// hr = device->CreateTexture2D(&desc, nullptr, &staging);
        //// if (FAILED(hr))
        //// 	return hr;
        ////
        //// assert(staging);
        ////
        //// context->CopyResource(staging, source);

        //
        //LOGI("COPY>>>>>>>");
        shared_texture->CopyCapturedTexture(device, context, acquired_texture);
    }

    return S_OK;
}
