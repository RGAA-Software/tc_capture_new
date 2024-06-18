//
// Created by Administrator on 2024/1/18.
//

#include "desktop_capture.h"
#include "capture_message.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"
#include <Shlobj.h>

namespace tc
{

    DesktopCapture::DesktopCapture(const std::shared_ptr<MessageNotifier>& msg_notifier, const std::string& monitor) {
        msg_notifier_ = msg_notifier;
        capture_monitor_ = monitor;
        msg_listener_ = msg_notifier->CreateListener();
        msg_listener_->Listen<RefreshScreenMessage>([](const RefreshScreenMessage& msg) {
            tc::DesktopCapture::RefreshScreen();
        });
    }

    void DesktopCapture::SetCaptureMonitor(std::string& name) {
        capture_monitor_ = name;
    }

    void DesktopCapture::SetCaptureFps(int fps) {
        capture_fps_ = fps;
    }

    std::vector<CaptureMonitorInfo> DesktopCapture::GetCaptureMonitorInfo() {
        return sorted_monitors_;
    }

    void DesktopCapture::RefreshScreen() {
        SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, nullptr, SPIF_SENDCHANGE);
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    }

}