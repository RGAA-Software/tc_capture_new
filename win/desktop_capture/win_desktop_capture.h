//
// Created by Administrator on 2024/1/17.
//

#ifndef TC_WIN_APPLICATION_DESKTOP_CAPTURE_H
#define TC_WIN_APPLICATION_DESKTOP_CAPTURE_H

#include <memory>
#include "tc_capture/desktop_capture.h"

namespace tc {

    class WGCCapture; // to do
    class DDACapture;
    class MessageNotifier;

    class WinDesktopCapture : public DesktopCapture
    {
    public:
        WinDesktopCapture(const std::shared_ptr<MessageNotifier>& msg_notifier);
        bool StartCapture();
        void StopCapture();
    private:
        std::shared_ptr<WGCCapture> wgc_capture_ = nullptr;
        std::shared_ptr<DDACapture> dda_capture_ = nullptr;
    };

} // na

#endif //TC_APPLICATION_DESKTOP_CAPTURE_H
