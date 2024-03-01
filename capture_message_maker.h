//
// Created by RGAA on 2024-02-25.
//

#ifndef TC_APPLICATION_CAPTURE_MESSAGE_MAKER_H
#define TC_APPLICATION_CAPTURE_MESSAGE_MAKER_H

#include <memory>
#include "capture_message.h"
#include "tc_common/data.h"

namespace tc
{

    // 生成通过IPC传递的消息
    class CaptureMessageMaker {
    public:

        template<typename T>
        static std::shared_ptr<Data> ConvertMessageToData(const T& frame) {
            auto msg_data = Data::Make(nullptr, sizeof(T));
            memcpy(msg_data->DataAddr(), &frame, sizeof(T));
            return msg_data;
        }

        static std::shared_ptr<Data> MakeAudioFrame(std::shared_ptr<Data>&& data);
        static std::shared_ptr<Data> MakeCaptureHelloMessage(const CaptureHelloMessage& msg);
        static std::shared_ptr<Data> MakeMouseEventMessage(uint64_t hwnd, uint32_t x, uint32_t y,
                                                           int32_t btn, int32_t data, int32_t dx, int32_t dy,
                                                           bool pressed, bool released);

    };

}
#endif //TC_APPLICATION_CAPTURE_MESSAGE_MAKER_H
