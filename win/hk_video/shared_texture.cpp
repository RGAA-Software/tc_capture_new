#include "shared_texture.h"
#include "tc_common/log.h"

namespace tc
{

	bool SharedTexture::CopyCapturedTexture(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* src) {
		D3D11_TEXTURE2D_DESC in_desc;
		src->GetDesc(&in_desc);

		if (in_desc.Width != curr_desc.Width || in_desc.Height != curr_desc.Height || in_desc.Format != curr_desc.Format) {
			
			if (texture) {
				texture.Release();
				texture = nullptr;
			}

			D3D11_TEXTURE2D_DESC desc = {};
			memset(&desc, 0, sizeof(D3D11_TEXTURE2D_DESC));
			desc.Width = in_desc.Width;
			desc.Height = in_desc.Height;
			desc.Format = in_desc.Format;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.SampleDesc.Quality = 0;
			desc.SampleDesc.Count = 1;
			//desc.Usage = D3D11_USAGE_STAGING;// ;D3D11_USAGE_DEFAULT;//
			//desc.BindFlags = 0;// D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;;// 0;
			//desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			//desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED;

			desc.BindFlags = 0;
			desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
			desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED; //D3D11_RESOURCE_MISC_SHARED_NTHANDLE | 
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;


			//desc.Usage = D3D11_USAGE_DEFAULT;
            // JUST TEST...
            desc.Usage = D3D11_USAGE_STAGING;

			LOGI("Create Texture : width : {}, height : {}, the old format is :{} , new format : {}",
					  desc.Width, desc.Height, (int)in_desc.Format, (int)desc.Format);

			D3D11_TEXTURE2D_DESC desc11 = {};
			desc11.Width = in_desc.Width;
			desc11.Height = in_desc.Height;
			desc11.Format = in_desc.Format;
			desc11.MipLevels = 1;
			desc11.ArraySize = 1;
			desc11.SampleDesc.Count = 1;
			desc11.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc11.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

            // JUST TEST...
            desc11.BindFlags = 0;
            desc11.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            desc11.Usage = D3D11_USAGE_STAGING;
            desc11.SampleDesc.Quality = 0;
            desc11.SampleDesc.Count = 1;

			auto hr = device->CreateTexture2D(&desc11, nullptr, &texture);
			if (FAILED(hr)) {
				LOGE("create_d3d11_stage_surface: failed to create texture {} , {}", hr, GetLastError());
				return false;
			}

			this->curr_desc = in_desc;
		}

		//
		context->CopyResource(texture, src);

		return true;
	}

	uint64_t SharedTexture::GetSharedHandle() {
		if (!texture) {
			return 0;
		}
		HANDLE tex_handle = nullptr;
		CComPtr<IDXGIResource> resource = nullptr;
		auto hr = texture->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void**>(&resource));
		if (SUCCEEDED(hr)) {
			hr = resource->GetSharedHandle(&tex_handle);
			if (FAILED(hr)) {
				LOGE("GetSharedHandle Failed !!!");
				return 0;
			}
		}
		else {
			LOGE("Query resource error : {0:x}", hr);
		}
		if (!tex_handle) {
			return 0;
		}
		return (uint64_t)tex_handle;
	}
}