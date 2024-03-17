//
// Created by RGAA on 2024-03-17.
//

#include "ipc_msg_maker.h"

#include "tc_message.pb.h"

namespace tc
{

    std::string IpcMsgMaker::MakeVideoIpcMessage(const CaptureVideoFrame& frame) {
//        uint32_t capture_type_ = 0;
//        uint32_t frame_width_ = 0;
//        uint32_t frame_height_ = 0;
//        uint64_t frame_index_ = 0;
//        uint64_t frame_format_ = 0;
//        uint64_t handle_ = 0;
//        int64_t adapter_uid_ = -1; // -1表示没获取到
//        int8_t capture_index_ = -1; //采集画面的索引，通常用于桌面采集，比如第一块屏幕的画面，第二块屏幕的画面 ...
        tc::Message msg;
        auto tvf = new tc::IpcVideoFrame();
        tvf->set_capture_type(frame.capture_type_);
        tvf->set_frame_width(frame.frame_width_);
        tvf->set_frame_height(frame.frame_height_);
        tvf->set_frame_index(frame.frame_index_);
        tvf->set_frame_format(frame.frame_format_);
        tvf->set_handle(frame.handle_);
        tvf->set_adapter_uid(frame.adapter_uid_);
        tvf->set_capture_index(frame.capture_index_);
        msg.set_type(tc::MessageType::kIpcVideoFrame);
        msg.set_allocated_ipc_video_frame(tvf);
        return msg.SerializeAsString();
    }

}
