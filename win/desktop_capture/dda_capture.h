#pragma once
#include <atlbase.h>
#include <wrl/client.h>
#include <string>
#include <DXGI.h>
#include <d3d11.h>
#include <DXGI1_2.h>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <map>
#include "monitor_util.h"

using namespace Microsoft::WRL;
namespace tc {
    class MessageNotifier;
    class DDACapture
    {
        // to do 考虑屏幕分辨率变化的问题
        // 如果有多张卡的话，应该一个DDACapture 采集一张卡
    public:
        enum class ECapRes
        {
            kSuccessfully,
            kFailed,
            kReInit,
            kTryAgain,
        };
        class DXGIOutputDupl{
        public:
            CComPtr<IDXGIOutputDuplication> duplication_;
            DXGI_OUTPUT_DESC output_desc_;
            DXGIOutputDupl() {
                memset(&output_desc_, 0, sizeof(DXGI_OUTPUT_DESC));
            }
        };
        class SharedD3d11Texture2D {
        public:
            HANDLE shared_haneld_ = 0;
            ComPtr<ID3D11Texture2D> texture2d_;
        };
        DDACapture();
        virtual ~DDACapture();
        bool Init();
        bool UnInit();
        void Start();
        bool IsValidRect(const RECT& rect)
        {
            return rect.right > rect.left && rect.bottom > rect.top;
        }
        // 这里加了一个参数，是屏幕的索引，方便采集特定的屏幕
        ECapRes CaptureNextFrame(int waitTime,CComPtr<ID3D11Texture2D>& OutTexture, int monIndex = 0);

        std::vector<SharedD3d11Texture2D> last_list_texture_;

        CComPtr<IDXGIFactory1> factory1_;
        CComPtr<IDXGIAdapter1> adapter1_;
        CComPtr<IDXGIOutput> dxgi_output_;
        CComPtr<ID3D11Device> d3d11_device_;
        CComPtr<ID3D11DeviceContext> d3d11_device_context_;
        std::vector<DXGIOutputDupl> dxgi_output_duplication_;

        CComPtr<ID3D11Texture2D> shared_texture_;

        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
    private:
        bool muti_screen_mode_ = false;
        std::atomic<bool> stop_flag_ = false;
        std::thread capture_thread_;
        void DoCapture();
        void OnCaptureFrame(ID3D11Texture2D* texture, uint8_t monitor_index);
        void SendTextureHandle(const HANDLE& shared_handle, EMonitorIndex current_monitor_index_, int width, int height, DXGI_FORMAT format);

        uint8_t monitor_count_ = 0;

        // 显卡设备uid
        int64_t adapter_uid_ = -1;

        int GetFrameIndex(EMonitorIndex monitor_index);
        std::map<EMonitorIndex, int> monitor_frame_index_;
    };
}