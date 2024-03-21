#include "dda_capture.h"
#include <algorithm>
#include <iostream>
#include <timeapi.h>
#include <functional>
#include "tc_common/string_ext.h"
#include "tc_common/message_notifier.h"
#include "tc_capture_new/capture_message.h"
#include "cursor_capture.h"


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "Winmm.lib")

namespace tc {

    DDACapture::DDACapture(std::shared_ptr<MessageNotifier> msg_notifier)
    {
        msg_notifier_ = msg_notifier;
        cursor_capturer_ = std::make_shared<CursorCapture>(msg_notifier_);
    }

    DDACapture::~DDACapture()
    {
    }

    bool DDACapture::Init()
    {
        HRESULT res = NULL;
        int adapter_index = 0; //显卡适配器索引
        res = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)&factory1_);
        if (res != S_OK) {
            printf("CreateDXGIFactory1 failed\n");
            return false;
        }
        res = factory1_->EnumAdapters1(adapter_index, &adapter1_);
        if (res != S_OK) {
            printf("EnumAdapters1 index:%d failed\n", adapter_index);
            return false;
        }
        D3D_FEATURE_LEVEL featureLevel;
        DXGI_ADAPTER_DESC desc;
        adapter1_->GetDesc(&desc);
        printf("Adapter Index:%d Name:%s\n", adapter_index, StringExt::ToUTF8(desc.Description).c_str());
        adapter_uid_ = desc.AdapterLuid.LowPart;
        res = D3D11CreateDevice(adapter1_, D3D_DRIVER_TYPE_UNKNOWN, NULL,
                                D3D11_CREATE_DEVICE_BGRA_SUPPORT /*| D3D11_CREATE_DEVICE_SINGLETHREADED*/,
                                NULL, 0, D3D11_SDK_VERSION, &d3d11_device_, &featureLevel, &d3d11_device_context_);
        if (res != S_OK || !d3d11_device_) {
            printf("D3D11CreateDevice failed: %s\n", StringExt::GetErrorStr(res).c_str());
            return false;
        }
        if (featureLevel < D3D_FEATURE_LEVEL_11_0)
        {
            std::cout << "D3D11CreateDevice returns an instance without DirectX 11 support, level "
                << featureLevel << ". Following initialization may fail" << std::endl;
            // D3D_FEATURE_LEVEL_11_0 is not officially documented on MSDN to be a requirement of Dxgi
            // duplicator APIs.
        }
        CComPtr<IDXGIDevice> dxgi_device;
        res = d3d11_device_.QueryInterface(&dxgi_device);
        if (res != S_OK || !dxgi_device)
        {
            std::cout << "ID3D11Device is not an implementation of IDXGIDevice, this usually "
                "means the system does not support DirectX 11. Error "
                << StringExt::GetErrorStr(res) << " with code: " << res << std::endl;
            return false;
        }
        int numbers = GetSystemMetrics(SM_CMONITORS);
        monitor_count_ = numbers;
        dxgi_output_duplication_.clear();
        for(int i=0; i<numbers; ++i) {
            DXGIOutputDupl dupl{};
            dxgi_output_duplication_.emplace_back(dupl);
        }
        muti_screen_mode_ = numbers > 1 ? true : false;
        printf("Total Monitors : %d\n", numbers);
        for (int index = 0; index < numbers; ++index)
        {
            CComPtr<IDXGIOutput> output;
            // 枚举所有的显示器
            res = adapter1_->EnumOutputs(index, &output);
            if (res == DXGI_ERROR_NOT_FOUND)
            {
                printf("adapter1_->EnumOutputs return DXGI_ERROR_NOT_FOUND,Please Check RDP connect.\n");
                break;
            }
            if (res == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
            {
                printf("IDXGIAdapter::EnumOutputs returns NOT_CURRENTLY_AVAILABLE. This may happen when running in session 0\n");
                break;
            }
            if (res != S_OK || !output)
            {
                printf("IDXGIAdapter::EnumOutputs returns an unexpected result %s with error code %p\n", StringExt::GetErrorStr(res).c_str(), res);
                continue;
            }
            DXGI_OUTPUT_DESC desc;
            res = output->GetDesc(&desc);
            if (res == S_OK)
            {
                dxgi_output_duplication_[index].output_desc_ = desc;
                printf("index index = %d, desc.DesktopCoordinates.left = %ld\n", index, desc.DesktopCoordinates.left);
                printf("Output Index:%d Name:%s AttachedToDesktop:%d\n", index, StringExt::ToUTF8(desc.DeviceName).c_str(), desc.AttachedToDesktop);
                if (desc.AttachedToDesktop && IsValidRect(desc.DesktopCoordinates))
                {
                    CComPtr<IDXGIOutput1> output1;
                    res = output.QueryInterface(&output1);
                    if (res != S_OK || !output1)
                    {
                        printf("Failed to convert IDXGIOutput to IDXGIOutput1, this usually means the system does not support DirectX 11");
                        continue;
                    }
                    static const int kRetryCount = 3;
                    for (int j = 0;; ++j)
                    {
                        // 执行屏幕复制。
                        HRESULT error = output1->DuplicateOutput(d3d11_device_, &(dxgi_output_duplication_[index].duplication_));
                        if (error != S_OK || !dxgi_output_duplication_[index].duplication_)
                        {
                            // DuplicateOutput may temporarily return E_ACCESSDENIED.
                            if (error == E_UNEXPECTED)
                            {
                                // to do
                            }
                            else
                            {
                                // to do
                            }
                            if (error == E_ACCESSDENIED && j < kRetryCount)
                            {
                                // why 如果是rdp会话，这里应该怎么弄
                                const ACCESS_MASK desiredAccess = GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE;
                                HDESK inputDesk = ::OpenInputDesktop(NULL, 0, desiredAccess);
                                if (inputDesk)
                                    ::SetThreadDesktop(inputDesk);
                                CloseDesktop(inputDesk);
                                continue;
                            }
                            printf("Failed to duplicate output from IDXGIOutput1, error %s with code %d\n", StringExt::GetErrorStr(error).c_str(), error);
                            return false;
                        }
                        else
                        {
                            printf("Init DDA moudle successfully. %d \n", index);
                            //return true;
                            break;
                        }
                    }
                }
                else
                {
                    std::cout << (desc.AttachedToDesktop ? "Attached" : "Detached")
                              << " output " << index << " ("
                        << desc.DesktopCoordinates.top << ", "
                        << desc.DesktopCoordinates.left << ") - ("
                        << desc.DesktopCoordinates.bottom << ", "
                        << desc.DesktopCoordinates.right << ") is ignored" << std::endl;
                }
            }
            else
            {
                std::cout << "Failed to get output description of device " << index << ", ignore" << std::endl;
            }
        }
        if(!dxgi_output_duplication_[0].duplication_) {
            return false;
        }
        std::sort(dxgi_output_duplication_.begin(), dxgi_output_duplication_.end(), [=](DXGIOutputDupl a, DXGIOutputDupl b) {
            return a.output_desc_.DesktopCoordinates.left < b.output_desc_.DesktopCoordinates.left;
        });

        for(auto& dup : dxgi_output_duplication_) {
            SharedD3d11Texture2D shared_texture;
            last_list_texture_.emplace_back(shared_texture);
        }

        printf("Init DDA moudle successfully.\n");
        return true;
    }

    bool DDACapture::UnInit()
    {
        stop_flag_ = true;
        if (capture_thread_.joinable()) {
            capture_thread_.join();
        }
        for (auto& dp : dxgi_output_duplication_) {
            if (dp.duplication_) {
                dp.duplication_->ReleaseFrame();
            }
            dp.duplication_.Release();
        }
        d3d11_device_.Release();
        d3d11_device_context_.Release();
        dxgi_output_.Release();
        adapter1_.Release();
        factory1_.Release();
        return true;
    }

    DDACapture::ECapRes DDACapture::CaptureNextFrame(int waitTime, CComPtr<ID3D11Texture2D>& OutTexture, int monIndex)
    {
        DXGI_OUTDUPL_FRAME_INFO info;
        CComPtr<IDXGIResource> resource;
        CComPtr<ID3D11Texture2D> source;
        HRESULT res;
        ECapRes ret = ECapRes::kSuccessfully;
        if (!dxgi_output_duplication_[monIndex].duplication_)
        {
            if (!Init())
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                return ECapRes::kTryAgain;
            }
            ret = ECapRes::kReInit;
        }
        dxgi_output_duplication_[monIndex].duplication_->ReleaseFrame();
        // 获取桌面帧。
        res = dxgi_output_duplication_[monIndex].duplication_->AcquireNextFrame(waitTime, &info, &resource);
        if (res != S_OK)
        {
            if (res == DXGI_ERROR_WAIT_TIMEOUT)
            {
                if (ret == ECapRes::kReInit) {
                    return ECapRes::kReInit;
                }
                //printf("AcquireNextFrame DXGI_ERROR_WAIT_TIMEOUT");
                return ECapRes::kTryAgain;
            }
            else if (res == DXGI_ERROR_ACCESS_LOST)
            {
                UnInit();
                printf("DXGI_ERROR_ACCESS_LOST");
                return ECapRes::kReInit;
            }
            else if (res == DXGI_ERROR_INVALID_CALL)
            {
                UnInit();
                printf("DXGI_ERROR_INVALID_CALL");
                return ECapRes::kReInit;
            }
            printf("AcquireNextFrame failed:%p ", res);
            return ECapRes::kTryAgain;
        }
        res = resource->QueryInterface(__uuidof(ID3D11Texture2D), (void **)&source);
        if (res != S_OK)
        {
            std::cout << StringExt::GetErrorStr(res) << std::endl;
            return ECapRes::kFailed;
        }
        if (info.AccumulatedFrames == 0) {
            //printf("AcquireNextFrame AccumulatedFrames 0");
            return ECapRes::kTryAgain;
        }
        OutTexture = source;
        return ret;
    }

    void DDACapture::Start() {
        capture_thread_ = std::thread(std::bind(&DDACapture::DoCapture, this));
    }

    void DDACapture::DoCapture()
    {
        timeBeginPeriod(1);
        while (!stop_flag_)
        {
            uint8_t captured_texture_count = 0;
            auto time_before_capture = std::chrono::high_resolution_clock::now();
            for(uint8_t index = 0; index < monitor_count_; ++index) {
                CComPtr<ID3D11Texture2D> texture = nullptr;
                {
                    DDACapture::ECapRes res = CaptureNextFrame(0, texture, index);
                    if (res == DDACapture::ECapRes::kFailed) {
                        printf("CaptureNextFrame index = %d failed.\n", index);
                        continue;
                    }

                    // to do 兼容性处理

                    if (texture) {
                        ++captured_texture_count;
                        //printf("OnCaptureFrame index = %d\n", index);
                        std::string dds_name = "frame_index_" + std::to_string(index);
                        //DebugOutDDS(texture, dds_name);
                        OnCaptureFrame(texture, index);
                    }
                }
            }
            auto time_after_capture = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_after_capture - time_before_capture).count();
            if(captured_texture_count == monitor_count_) {
                int sleep_ms = 16 - duration;
                auto beg = std::chrono::high_resolution_clock::now();
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
                auto end = std::chrono::high_resolution_clock::now();
            }
        }
    }

    void DDACapture::OnCaptureFrame(ID3D11Texture2D* texture, uint8_t monitor_index)
    {
        int width = 0;
        int height = 0;
        DXGI_FORMAT format;
        // to do 考虑分辨率动态变化的情况
        if(!last_list_texture_[monitor_index].texture2d_) {
            HRESULT hres;
            D3D11_TEXTURE2D_DESC desc;
            texture->GetDesc(&desc);

            D3D11_TEXTURE2D_DESC create_desc;
            ZeroMemory(&create_desc, sizeof(create_desc));
            create_desc.Format = desc.Format;
            create_desc.Width = desc.Width;
            create_desc.Height = desc.Height;
            create_desc.MipLevels = 1;
            create_desc.ArraySize = 1;
            create_desc.SampleDesc.Count = 1;
            create_desc.Usage = D3D11_USAGE_DEFAULT;
            create_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            create_desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

            hres = d3d11_device_->CreateTexture2D(&create_desc, NULL, &last_list_texture_[monitor_index].texture2d_);
            if (FAILED(hres))
            {
                printf("desktop capture create texture failed with:%s", StringExt::GetErrorStr(hres).c_str());
                return;
            }

            ComPtr<IDXGIResource> dxgiResource;
            hres = last_list_texture_[monitor_index].texture2d_.As<IDXGIResource>(&dxgiResource);
            if (FAILED(hres))
            {
                printf("desktop capture as IDXGIResource failed with:%s", StringExt::GetErrorStr(hres).c_str());
                return;
            }
            HANDLE handle;
            hres = dxgiResource->GetSharedHandle(&handle);
            if (FAILED(hres))
            {
                printf("desktop capture get shared handle failed with:%s", StringExt::GetErrorStr(hres).c_str());
                return;
            }
            last_list_texture_[monitor_index].shared_haneld_ = handle;
        }

        HRESULT hres;
        ComPtr<IDXGIKeyedMutex> keyMutex;
        hres = last_list_texture_[monitor_index].texture2d_.As<IDXGIKeyedMutex>(&keyMutex);
        if(FAILED(hres))
        {
            printf("desktop frame capture as IDXGIKeyedMutex failed:%s", StringExt::GetErrorStr(hres).c_str());
            return ;
        }
        hres = keyMutex->AcquireSync(0, INFINITE);
        if (FAILED(hres))
        {
            printf("desktop frame capture texture AcquireSync failed with:%s", StringExt::GetErrorStr(hres).c_str());
            return;
        }

        d3d11_device_context_->CopyResource(last_list_texture_[monitor_index].texture2d_.Get(), texture);

        {
            D3D11_TEXTURE2D_DESC desc;
            texture->GetDesc(&desc);
            width = desc.Width;
            height = desc.Height;
            format = desc.Format;
        }

        if(keyMutex) {
            keyMutex->ReleaseSync(0);
        }

        SendTextureHandle(last_list_texture_[monitor_index].shared_haneld_, static_cast<EMonitorIndex>(monitor_index), width, height, format);
    }

    void DDACapture::SendTextureHandle(const HANDLE& shared_handle, EMonitorIndex current_monitor_index, int width, int height, DXGI_FORMAT format) {
        //printf("current_monitor_index_ = %d, shared_handle = %p\n", current_monitor_index, shared_handle);
        //RecvD3D11Texture2DShareHandle(d3d11_device_, shared_handle, static_cast<int>(current_monitor_index_));
        if(cursor_capturer_) {
            cursor_capturer_->Capture();
        }

        if(msg_notifier_) {
            CaptureVideoFrame capture_video_frame_msg{};
            capture_video_frame_msg.type_ = kCaptureVideoFrame;
            capture_video_frame_msg.capture_type_ = kCaptureVideoByHandle;
            capture_video_frame_msg.data_length = 0;
            capture_video_frame_msg.frame_width_ = width;
            capture_video_frame_msg.frame_height_ = height;
            capture_video_frame_msg.frame_index_ = GetFrameIndex(current_monitor_index);
            capture_video_frame_msg.handle_ = reinterpret_cast<uint64_t>(shared_handle);
            capture_video_frame_msg.frame_format_ = format;
            capture_video_frame_msg.adapter_uid_ = adapter_uid_;
            capture_video_frame_msg.capture_index_ = static_cast<int8_t>(current_monitor_index);
            // to do  暂时先只发主屏的
            if(EMonitorIndex::kFirst == current_monitor_index) {
                msg_notifier_->SendAppMessage(capture_video_frame_msg);
            }
        }
    }

    int DDACapture::GetFrameIndex(EMonitorIndex monitor_index) {
        if(monitor_frame_index_.count(monitor_index) > 0) {
            monitor_frame_index_[monitor_index] = ++monitor_frame_index_[monitor_index];
        } else {
            monitor_frame_index_[monitor_index] = 0;
        }
        return monitor_frame_index_[monitor_index];
    }

} // tc