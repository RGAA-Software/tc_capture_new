#include <d3d11.h>
#include <dxgi.h>
#include <atlbase.h>

#include "dxgi-helpers.hpp"
#include "graphics-hook.h"
#include "tc_common/log.h"
#include "tc_common/data.h"
#include "hook_manager.h"
#include "hook_ipc.h"
#include "capture_message.h"
#include "../hk_video/d3d_utils.h"
#include "capture_message_maker.h"

using namespace tc;

static uint64_t g_frame_index = 0;

struct d3d11_data {
	ID3D11Device *device;         /* do not release */
	ID3D11DeviceContext *context; /* do not release */
	uint32_t cx;
	uint32_t cy;
	DXGI_FORMAT format;
	bool using_shtex;
	bool multisampled;

	union {
		/* shared texture */
		struct {
			struct shtex_data *shtex_info;
			ID3D11Texture2D *texture;
			HANDLE handle;
		};
		/* shared memory */
		struct {
			ID3D11Texture2D *copy_surfaces[NUM_BUFFERS];
			bool texture_ready[NUM_BUFFERS];
			bool texture_mapped[NUM_BUFFERS];
			uint32_t pitch;
			struct shmem_data *shmem_info;
			int cur_tex;
			int copy_wait;
		};
	};
};

static struct d3d11_data data = {};

void d3d11_free(void)
{
	capture_free();

	if (data.using_shtex) {
		if (data.texture)
			data.texture->Release();
	} else {
		for (size_t i = 0; i < NUM_BUFFERS; i++) {
			if (data.copy_surfaces[i]) {
				if (data.texture_mapped[i])
					data.context->Unmap(
						data.copy_surfaces[i], 0);
				data.copy_surfaces[i]->Release();
			}
		}
	}

	memset(&data, 0, sizeof(data));

	hlog("----------------- d3d11 capture freed ----------------");
}

static bool create_d3d11_stage_surface(ID3D11Texture2D **tex)
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = data.cx;
	desc.Height = data.cy;
	desc.Format = data.format;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	hr = data.device->CreateTexture2D(&desc, nullptr, tex);
	if (FAILED(hr)) {
		hlog_hr("create_d3d11_stage_surface: failed to create texture",
			hr);
		return false;
	}

	return true;
}

static bool create_d3d11_tex(uint32_t cx, uint32_t cy, ID3D11Texture2D **tex, HANDLE *handle)
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = cx;
	desc.Height = cy;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = apply_dxgi_format_typeless(
		data.format, global_hook_info->allow_srgb_alias);
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

	hr = data.device->CreateTexture2D(&desc, nullptr, tex);
	if (FAILED(hr)) {
		hlog_hr("create_d3d11_tex: failed to create texture", hr);
		return false;
	}

	if (!!handle) {
		IDXGIResource *dxgi_res;
		hr = (*tex)->QueryInterface(__uuidof(IDXGIResource),
					    (void **)&dxgi_res);
		if (FAILED(hr)) {
			hlog_hr("create_d3d11_tex: failed to query "
				"IDXGIResource interface from texture",
				hr);
			return false;
		}

		hr = dxgi_res->GetSharedHandle(handle);
		dxgi_res->Release();
		if (FAILED(hr)) {
			hlog_hr("create_d3d11_tex: failed to get shared handle",
				hr);
			return false;
		}
	}

	return true;
}

static inline bool d3d11_init_format(IDXGISwapChain *swap, HWND &window)
{
	DXGI_SWAP_CHAIN_DESC desc;
	HRESULT hr;

	hr = swap->GetDesc(&desc);
	if (FAILED(hr)) {
		hlog_hr("d3d11_init_format: swap->GetDesc failed", hr);
		return false;
	}

	print_swap_desc(&desc);

	data.format = strip_dxgi_format_srgb(desc.BufferDesc.Format);
	data.multisampled = desc.SampleDesc.Count > 1;
	window = desc.OutputWindow;
	data.cx = desc.BufferDesc.Width;
	data.cy = desc.BufferDesc.Height;

	return true;
}

static bool d3d11_shmem_init_buffers(size_t idx)
{
	bool success;

	success = create_d3d11_stage_surface(&data.copy_surfaces[idx]);
	if (!success) {
		hlog("d3d11_shmem_init_buffers: failed to create copy surface");
		return false;
	}

	if (idx == 0) {
		D3D11_MAPPED_SUBRESOURCE map = {};
		HRESULT hr;

		hr = data.context->Map(data.copy_surfaces[idx], 0,
				       D3D11_MAP_READ, 0, &map);
		if (FAILED(hr)) {
			hlog_hr("d3d11_shmem_init_buffers: failed to get "
				"pitch",
				hr);
			return false;
		}

		data.pitch = map.RowPitch;
		data.context->Unmap(data.copy_surfaces[idx], 0);
	}

	return true;
}

static bool d3d11_shmem_init(HWND window)
{
	data.using_shtex = false;

	for (size_t i = 0; i < NUM_BUFFERS; i++) {
		if (!d3d11_shmem_init_buffers(i)) {
			return false;
		}
	}
	if (!capture_init_shmem(&data.shmem_info, window, data.cx, data.cy,
				data.pitch, data.format, false)) {
		return false;
	}

	hlog("d3d11 memory capture successful");
	return true;
}

static bool d3d11_shtex_init(HWND window)
{
	bool success;
	data.using_shtex = true;
	success = create_d3d11_tex(data.cx, data.cy, &data.texture, &data.handle);

	if (!success) {
		hlog("d3d11_shtex_init: failed to create texture");
		return false;
	}
	if (!capture_init_shtex(&data.shtex_info, window, data.cx, data.cy,
				data.format, false, (uintptr_t)data.handle)) {
		return false;
	}

	hlog("d3d11 shared texture capture successful");
	return true;
}

static void d3d11_init(IDXGISwapChain *swap)
{
	HWND window;
	HRESULT hr;

	hr = swap->GetDevice(__uuidof(ID3D11Device), (void **)&data.device);
	if (FAILED(hr)) {
		hlog_hr("d3d11_init: failed to get device from swap", hr);
		return;
	}

	data.device->Release();

	data.device->GetImmediateContext(&data.context);
	data.context->Release();

	if (!d3d11_init_format(swap, window)) {
		return;
	}

	const bool success = global_hook_info->force_shmem
				     ? d3d11_shmem_init(window)
				     : d3d11_shtex_init(window);
	if (!success) {
        LOGI("init ... free");
        d3d11_free();
    }
}

static inline void d3d11_copy_texture(ID3D11Resource *dst, ID3D11Resource *src)
{
	if (data.multisampled) {
		data.context->ResolveSubresource(dst, 0, src, 0, data.format);
	} else {
		data.context->CopyResource(dst, src);
	}
}

static inline void d3d11_shtex_capture(ID3D11Resource *backbuffer)
{
	d3d11_copy_texture(data.texture, backbuffer);
}

static void d3d11_shmem_capture_copy(int i)
{
	D3D11_MAPPED_SUBRESOURCE map;
	HRESULT hr;

	if (data.texture_ready[i]) {
		data.texture_ready[i] = false;

		hr = data.context->Map(data.copy_surfaces[i], 0, D3D11_MAP_READ,
				       0, &map);
		if (SUCCEEDED(hr)) {
			data.texture_mapped[i] = true;
			shmem_copy_data(i, map.pData);
		}
	}
}

static inline void d3d11_shmem_capture(ID3D11Resource *backbuffer)
{
	int next_tex;

	next_tex = (data.cur_tex + 1) % NUM_BUFFERS;
	d3d11_shmem_capture_copy(next_tex);

	if (data.copy_wait < NUM_BUFFERS - 1) {
		data.copy_wait++;
	} else {
		if (shmem_texture_data_lock(data.cur_tex)) {
			data.context->Unmap(data.copy_surfaces[data.cur_tex],
					    0);
			data.texture_mapped[data.cur_tex] = false;
			shmem_texture_data_unlock(data.cur_tex);
		}

		d3d11_copy_texture(data.copy_surfaces[data.cur_tex],
				   backbuffer);
		data.texture_ready[data.cur_tex] = true;
	}

	data.cur_tex = next_tex;
}

#if 0
uint64_t GetSharedHandle() {
    if (!data.texture) {
        return 0;
    }
    HANDLE tex_handle = nullptr;
    CComPtr<IDXGIResource> resource = nullptr;
    auto hr = data.texture->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void**>(&resource));
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
#endif

void d3d11_capture(void *swap_ptr, void *backbuffer_ptr)
{
	IDXGIResource *dxgi_backbuffer = (IDXGIResource *)backbuffer_ptr;
	IDXGISwapChain *swap = (IDXGISwapChain *)swap_ptr;

	HRESULT hr;
	if (/*capture_should_stop()*/false) {
		//d3d11_free();
	}

    //static bool should_init = true;
	if (!capture_active()/*capture_should_init()*/) {
        //should_init = false;
        LOGI("-----------------> d3d11 init ....");
		d3d11_init(swap);
	}
    LOGI("d3d11 apture.... : {}", capture_ready());
    LOGI("capture active: {}, frame_ready: {}", capture_active(), frame_ready(global_hook_info->frame_interval));
	if (capture_ready()) {
		ID3D11Resource *backbuffer;

		hr = dxgi_backbuffer->QueryInterface(__uuidof(ID3D11Resource),
						     (void **)&backbuffer);
		if (FAILED(hr)) {
			hlog_hr("d3d11_shtex_capture: failed to get "
				"backbuffer",
				hr);
			return;
		}

        LOGI("capture data... : {}", data.using_shtex);

		if (data.using_shtex) {
            d3d11_shtex_capture(backbuffer);

            auto shared_texture = HookManager::Instance()->shared_texture_;
            shared_texture->CopyCapturedTexture(data.device, data.context, data.texture);

            D3D11_TEXTURE2D_DESC desc;
            data.texture->GetDesc(&desc);
            auto hook_mgr = HookManager::Instance();
            auto adapter_uid = tc::GetAdapterUid(data.device);
            CaptureVideoFrame capture_video_frame_msg{};
            capture_video_frame_msg.capture_type_ = kCaptureVideoByHandle;
            capture_video_frame_msg.data_length = 0;
            capture_video_frame_msg.frame_width_ = desc.Width;
            capture_video_frame_msg.frame_height_ = desc.Height;
            capture_video_frame_msg.frame_index_ = g_frame_index++;
            capture_video_frame_msg.handle_ = shared_texture->GetSharedHandle();
            capture_video_frame_msg.frame_format_ = desc.Format;
            if(adapter_uid.has_value()) {
                capture_video_frame_msg.adapter_uid_ = adapter_uid.value();
            }

            LOGI("Shared handle: {}", capture_video_frame_msg.handle_);
            LOGI("adapter uid: {}", capture_video_frame_msg.adapter_uid_);
            auto msg_data = CaptureMessageMaker::ConvertMessageToData(capture_video_frame_msg);
            hook_mgr->Send(std::move(msg_data));

            //capture_video_frame_msg.adapter_uid_ = 78007;
//            auto msg_data = Data::Make(nullptr, sizeof(CaptureVideoFrame));
//            memcpy(msg_data->DataAddr(), &capture_video_frame_msg, sizeof(CaptureVideoFrame));
//            hook_mgr->Send(std::move(msg_data));
        }
		else {
            d3d11_shmem_capture(backbuffer);
        }

		backbuffer->Release();
	}
}
