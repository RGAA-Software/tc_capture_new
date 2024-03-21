//
// Created by Administrator on 2024/1/18.
//

#include "desktop_capture.h"
#include "tc_common_new/message_notifier.h"

namespace tc {

    DesktopCapture::DesktopCapture(const std::shared_ptr<MessageNotifier> &msg_notifier) {
        msg_notifier_ = msg_notifier;
    }
}