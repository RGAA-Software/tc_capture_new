//
// Created by RGAA  on 2024/1/18.
//

#ifndef TC_APPLICATION_DESKTOP_CAPTURE_H
#define TC_APPLICATION_DESKTOP_CAPTURE_H

#include <memory>
#include <string>
#include <vector>
#if WIN32
#include "win/desktop_capture/monitor_util.h"
#endif

namespace tc
{

    class MessageNotifier;
    class MessageListener;

    class DesktopCapture {
    public:
        explicit DesktopCapture(const std::shared_ptr<MessageNotifier>& msg_notifier, const std::string& monitor);
        virtual bool StartCapture() = 0;
        virtual void StopCapture() = 0;
        void SetCaptureMonitor(std::string& name);
        void SetCaptureFps(int fps);
        std::vector<CaptureMonitorInfo> GetCaptureMonitorInfo();

    private:
        static void RefreshScreen();

    protected:
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::string capture_monitor_;
        int capture_fps_ = 60;
        std::vector<CaptureMonitorInfo> sorted_monitors_;
    };
}

#endif //TC_APPLICATION_DESKTOP_CAPTURE_H
