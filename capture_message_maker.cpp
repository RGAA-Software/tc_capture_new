//
// Created by RGAA on 2024-02-25.
//

#include "capture_message_maker.h"

#include "tc_common/data.h"
#include "capture_message.h"

namespace tc
{

//    template<typename T>
//    std::shared_ptr<Data> CaptureMessageMaker::ConvertMessageToData(const T& frame) {
//
//    }

    std::shared_ptr<Data> CaptureMessageMaker::MakeAudioFrame(std::shared_ptr<Data>&& audio_data) {
        CaptureAudioFrame capture_audio_frame_msg{};
        capture_audio_frame_msg.frame_index_ = 0;
        auto data = Data::Make(nullptr, sizeof(CaptureAudioFrame) + audio_data->Size());
        memcpy(data->DataAddr(), &capture_audio_frame_msg, sizeof(CaptureAudioFrame));
        memcpy(data->DataAddr() + sizeof(CaptureAudioFrame), audio_data->CStr(), audio_data->Size());
        return data;
    }

    std::shared_ptr<Data> CaptureMessageMaker::MakeCaptureHelloMessage(const CaptureHelloMessage& msg) {
        auto data = Data::Make(nullptr, sizeof(CaptureHelloMessage));
        memcpy(data->DataAddr(), &msg, sizeof(CaptureHelloMessage));
        return data;
    }

    std::shared_ptr<Data> CaptureMessageMaker::MakeMouseEventMessage(uint32_t x_radio, uint32_t y_radio, int32_t btn, int32_t data, int32_t dx, int32_t dy) {
        MouseEventMessage msg{};
        msg.x_ratio_ = x_radio;
        msg.y_ratio_ = y_radio;
        msg.button_ = btn;
        msg.data_ = data;
        msg.delta_x_ = dx;
        msg.delta_y_ = dy;
        auto msg_data = Data::Make(nullptr, sizeof(MouseEventMessage));
        memcpy(msg_data->DataAddr(), &msg, sizeof(MouseEventMessage));
        return msg_data;
    }

}
