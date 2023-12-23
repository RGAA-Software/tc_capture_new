#include "framework.h"
#include "capture_dxgi_d3d11.h"
#include "capture_texture.h"
#include "d3d_utils.h"
#include "hk_utils/time_measure.hpp"
#include "shared_texture.h"

#include <libyuv.h>

#include <d3d11.h>
#include <fstream>

#include "client_ipc_manager.h"
#include "capture_message.h"
#include "tc_common/data.h"
#include "tc_common/log.h"

using namespace tc;

static std::shared_ptr<tc::SharedTexture> shared_texture = std::make_shared<tc::SharedTexture>();

namespace tc_capture_d3d11
{

    namespace
    {
        bool initialized_ = false;
        ID3D11Device *device_ = nullptr;
        ID3D11DeviceContext *context_ = nullptr;

        // feature_level >= D3D_FEATURE_LEVEL_11_1
        //PFN_D3D11_CREATE_DEVICE D3D11CreateDevice_;
        //ID3D11Device* device11_ = nullptr;
        //ID3D11DeviceContext* context11_ = nullptr;

        //UINT width_ = 0;
        //UINT height_ = 0;
        //HWND window_ = nullptr;

        //HRESULT CreateD3D11DeviceOnLevel11_1() {
        //if (nullptr == D3D11CreateDevice_) {
        //	D3D11CreateDevice_ = reinterpret_cast<PFN_D3D11_CREATE_DEVICE>(GetProcAddress(GetModuleHandle(_T("d3d11.dll")), "D3D11CreateDevice"));
        //	if (nullptr == D3D11CreateDevice_) {
        //		ATLTRACE2(atlTraceException, 0,
        //			"!GetProcAddress(D3D11CreateDevice), #%d\n", GetLastError());
        //		return E_NOINTERFACE;
        //	}
        //	ATLTRACE2(atlTraceUtil, 0, "D3D11CreateDevice = 0x%p\n",
        //		D3D11CreateDevice_);
        //}

        //D3D_DRIVER_TYPE driver_types[]{
        //	D3D_DRIVER_TYPE_HARDWARE,
        //	// D3D_DRIVER_TYPE_WARP,
        //	// D3D_DRIVER_TYPE_REFERENCE,
        //};
        //
        //D3D_FEATURE_LEVEL feature_level;
        //D3D_FEATURE_LEVEL feature_levels[]{
        //	D3D_FEATURE_LEVEL_11_1,
        //	D3D_FEATURE_LEVEL_12_0,
        //	D3D_FEATURE_LEVEL_12_1,
        //};

        //HRESULT hr;
        //for (int driver_type_index = 0; driver_type_index < ARRAYSIZE(driver_types);
        //	driver_type_index++) {
        //	hr = D3D11CreateDevice_(NULL, driver_types[driver_type_index], NULL, 0,
        //		feature_levels, ARRAYSIZE(feature_levels),
        //		D3D11_SDK_VERSION, &device11_, &feature_level,
        //		&context11_);
        //	if (SUCCEEDED(hr)) {
        //		ATLTRACE2(atlTraceUtil, 0, "D3D11CreateDevice() driver type %d\n",
        //			driver_types[driver_type_index]);
        //		break;
        //	}
        //}

        //if (FAILED(hr)) {
        //	ATLTRACE2(atlTraceException, 0, "!D3D11CreateDevice(), #0x%08X\n", hr);
        //	return hr;
        //}

        //	return S_OK;
        //}

        HRESULT Initialize(IDXGISwapChain *swap) {
            HRESULT hr = swap->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void **>(&device_));
            if (FAILED(hr)) {
                ATLTRACE2(atlTraceException, 0, "%s: Failed to get device from swap, #0x%08X\n", __func__, hr);
                return hr;
            }

            device_->GetImmediateContext(&context_);
            ULONG ref = context_->Release();
            LOGI(" // ID3D11DeviceContext::Release() = {}", ref);
            ref = device_->Release();
            LOGI(" // ID3D11Device::Release() = {}", ref);

            //DXGI_SWAP_CHAIN_DESC desc;
            //hr = swap->GetDesc(&desc);
            //if (FAILED(hr)) {
            //	ATLTRACE2(atlTraceException, 0,
            //		"%s: Failed to get desc from swap, #0x%08X\n", __func__, hr);
            //	return hr;
            //}
            //window_ = desc.OutputWindow;

            //hr = CreateD3D11DeviceOnLevel11_1();
            //if (FAILED(hr)) {
            //	ATLTRACE2(atlTraceException, 0,
            //		"%s: Failed to CreateD3D11DeviceOnLevel11_1, #0x%08X\n", __func__,
            //		hr);
            //	return hr;
            //}
            return hr;
        }
    }  // namespace

    static uint64_t g_frame_index = 0;

    void Capture(void *swap, void* back_buffer) {
        bool should_update = false;
        if (!initialized_) {
            HRESULT hr = Initialize(static_cast<IDXGISwapChain *>(swap));
            if (FAILED(hr)) {
                return;
            }
            initialized_ = true;
            should_update = true;
        }

        auto dxgi_backbuffer = static_cast<IDXGIResource *>(back_buffer);
        CComQIPtr<ID3D11Texture2D> acquired_texture(dxgi_backbuffer);
        if (!acquired_texture) {
            ATLTRACE2(atlTraceException, 0, "!QueryInterface(ID3D11Texture2D)\n");
            return;
        }

        LARGE_INTEGER tick;
        QueryPerformanceCounter(&tick);

        D3D11_TEXTURE2D_DESC desc;
        CComPtr<ID3D11Texture2D> new_texture;
        HRESULT hr = CaptureTexture(device_, context_, acquired_texture, desc, new_texture, shared_texture);
        if (FAILED(hr)) {
            LOGE("CaptureTexture failed!");
            return;
        }

        if (!shared_texture->texture) {
            LOGE("Not have Texture....");
            return;
        }

        uint64_t handle = shared_texture->GetSharedHandle();
        //LOGI("shared handle : %llu, format : %d, frame_index: %llu", handle, desc.Format, g_frame_index);
//		auto ipc_message = IPCFrameMessage::MakeEmptyMessage();
//		ipc_message->type = IPCMessageType::kSharedTextureHandle;
//		ipc_message->sender = IPCMessageSender::kSenderClient;
//		ipc_message->handle = handle;
//		ipc_message->format = desc.Format;
//		ipc_message->width = desc.Width;
//		ipc_message->height = desc.Height;
//		ipc_message->frame_index = g_frame_index++;
//		InterCommClient::Instance()->SendBack(IPCFrameMessage::ConvertToData(ipc_message));
        g_frame_index++;
        //LOGI("Hook....{}", g_frame_index);

        CaptureMessage msg{};
        msg.frame_index_ = g_frame_index;
        msg.handle_ = handle;
        auto data = Data::Make((char*)&msg, sizeof(CaptureMessage));
        ClientIpcManager::Instance()->Send(data);

#if 0
        {
            D3D11_TEXTURE2D_DESC desc;
            shared_texture->texture->GetDesc(&desc);

            auto width = desc.Width;
            auto height = desc.Height;
            std::string msg = " --> width : " + std::to_string(width) + " height : " + std::to_string(height);
            LOGI(msg);

            std::vector<uint8_t> yuv_frame_data_;
            yuv_frame_data_.resize(4 * width * height);
            size_t pixel_size = width * height;

            const int uv_stride = width >> 1;
            uint8_t* y = yuv_frame_data_.data();
            uint8_t* u = y + pixel_size;
            uint8_t* v = u + (pixel_size >> 2);

            CComPtr<IDXGISurface> staging_surface = nullptr;
            hr = shared_texture->texture->QueryInterface(IID_PPV_ARGS(&staging_surface));
            if (FAILED(hr)) {
                LOGE("!QueryInterface(IDXGISurface) err");
                return;
            }
            DXGI_MAPPED_RECT mapped_rect{};
            hr = staging_surface->Map(&mapped_rect, DXGI_MAP_READ);
            if (FAILED(hr)) {
                LOGE("!Map(IDXGISurface)");
                return;
            }

            LOGI("the format is : " + std::to_string(desc.Format));

            {
                std::ofstream rgba_file("capture_yuv_tex.rgba", std::ios::binary);
                rgba_file.write((char*)mapped_rect.pBits, width * height * 4);
                rgba_file.close();
            }

            if (DXGI_FORMAT_R8G8B8A8_UNORM == desc.Format) {
                libyuv::ARGBToI420(mapped_rect.pBits, mapped_rect.Pitch, y, width, u, uv_stride, v, uv_stride, width, height);
            }
            else if (DXGI_FORMAT_B8G8R8A8_UNORM == desc.Format) {
                libyuv::ABGRToI420(mapped_rect.pBits, mapped_rect.Pitch, y, width, u, uv_stride, v, uv_stride, width, height);
            }
            else {
                libyuv::ARGBToI420(mapped_rect.pBits, mapped_rect.Pitch, y, width, u, uv_stride, v, uv_stride, width, height);
            }

            LOGI("desc.Format: " + std::to_string(desc.Format) + " the yuv size : " + std::to_string(yuv_frame_data_.size()));
            std::ofstream yuv_file("capture_yuv_tex.yuv", std::ios::binary);
            yuv_file.write((char*)yuv_frame_data_.data(), width * height * 1.5);
            yuv_file.close();

        }
#endif
    }

    void FreeResource() {
        //if (nullptr != device11_) {
        //	device11_->Release();
        //	device11_ = nullptr;
        //}
        //if (nullptr != context11_) {
        //	context11_->Release();
        //	context11_ = nullptr;
        //}
    }

    void Free() {
        g_capture_tex.FreeSharedTexture();
        FreeResource();
        initialized_ = false;
    }

}  // namespace capture_d3d11
