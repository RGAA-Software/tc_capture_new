//
// Created by RGAA on 2024-02-25.
//

#include "capture_message_maker.h"

#include "tc_common/data.h"
#include "capture_message.h"

namespace tc
{

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

}
