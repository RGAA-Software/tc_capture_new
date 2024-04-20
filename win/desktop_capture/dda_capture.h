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
#include "desktop_capture.h"

using namespace Microsoft::WRL;

namespace tc
{
    class MessageNotifier;
    class CursorCapture;

    class DDACapture : public DesktopCapture  {
    public:
        enum class ECapRes {
            kSuccessfully,
            kFailed,
            kReInit,
            kTryAgain,
        };

        class DXGIOutputDuplication {
        public:
            CComPtr<IDXGIOutputDuplication> duplication_;
            DXGI_OUTPUT_DESC output_desc_{};

            DXGIOutputDuplication() {
                memset(&output_desc_, 0, sizeof(DXGI_OUTPUT_DESC));
            }
        };

        class SharedD3d11Texture2D {
        public:
            HANDLE shared_handle_ = 0;
            ComPtr <ID3D11Texture2D> texture2d_;
        };

        explicit DDACapture(const std::shared_ptr<MessageNotifier>& msg_notifier);
        virtual ~DDACapture();

        bool StartCapture() override;
        void StopCapture() override;

        bool Init();
        void Start();
        bool Exit();

        static bool IsValidRect(const RECT &rect) {
            return rect.right > rect.left && rect.bottom > rect.top;
        }

        ECapRes CaptureNextFrame(int waitTime, CComPtr<ID3D11Texture2D> &OutTexture, int monIndex = 0);

    private:
        void Capture();
        void OnCaptureFrame(ID3D11Texture2D *texture, uint8_t monitor_index);
        void SendTextureHandle(const HANDLE &shared_handle, EMonitorIndex current_monitor_index_, int width, int height, DXGI_FORMAT format);
        int GetFrameIndex(EMonitorIndex monitor_index);
        bool IsTargetMonitor(int index);

    private:
        std::atomic<bool> stop_flag_ = false;
        std::thread capture_thread_;
        uint8_t monitor_count_ = 0;
        int64_t adapter_uid_ = -1;
        std::map<EMonitorIndex, int> monitor_frame_index_;

        std::vector<SharedD3d11Texture2D> last_list_texture_;
        CComPtr<IDXGIFactory1> factory1_ = nullptr;
        CComPtr<IDXGIAdapter1> adapter1_ = nullptr;
        CComPtr<IDXGIOutput> dxgi_output_ = nullptr;
        CComPtr<ID3D11Device> d3d11_device_ = nullptr;
        CComPtr<ID3D11DeviceContext> d3d11_device_context_ = nullptr;
        std::vector<DXGIOutputDuplication> dxgi_output_duplication_;
        CComPtr<ID3D11Texture2D> shared_texture_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        std::shared_ptr<CursorCapture> cursor_capture_ = nullptr;
    };
}