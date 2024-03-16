//
// Created by Administrator on 2024/1/17.
//

#include "win_desktop_capture.h"
#include "tc_common/message_notifier.h"
#include "dda_capture.h"

namespace tc {
    WinDesktopCapture::WinDesktopCapture(const std::shared_ptr<MessageNotifier>& msg_notifier) : DesktopCapture(msg_notifier)
    {
        dda_capture_ = std::make_shared<DDACapture>(msg_notifier);
        dda_capture_->Init();
    }

    bool WinDesktopCapture::StartCapture()
    {
        //return wgc_capture_->Start();
        dda_capture_->Start();
        return true;
    }

    void WinDesktopCapture::StopCapture()
    {
        //wgc_capture_->Stop();
        dda_capture_->UnInit();
    }
} // namespace tc