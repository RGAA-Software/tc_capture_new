//
// Created by Administrator on 2024/1/18.
//

#ifndef TC_APPLICATION_DESKTOP_CAPTURE_H
#define TC_APPLICATION_DESKTOP_CAPTURE_H
#include <memory>

namespace tc {

    class MessageNotifier;
    class MessageListener;

    class DesktopCapture {
    public:
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        DesktopCapture(const std::shared_ptr<MessageNotifier>& msg_notifier);
        virtual bool StartCapture() = 0;
        virtual void StopCapture() = 0;
    };
}

#endif //TC_APPLICATION_DESKTOP_CAPTURE_H