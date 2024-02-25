//
// Created by RGAA on 2024-02-25.
//

#ifndef TC_APPLICATION_CAPTURE_MESSAGE_MAKER_H
#define TC_APPLICATION_CAPTURE_MESSAGE_MAKER_H

#include <memory>
#include "capture_message.h"

namespace tc
{

    class Data;

    // 生成通过IPC传递的消息
    class CaptureMessageMaker {
    public:

        static std::shared_ptr<Data> MakeAudioFrame(std::shared_ptr<Data>&& data);
        static std::shared_ptr<Data> MakeCaptureHelloMessage(const CaptureHelloMessage& msg);

    };

}
#endif //TC_APPLICATION_CAPTURE_MESSAGE_MAKER_H
