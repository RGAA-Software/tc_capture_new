//
// Created by RGAA on 2023-12-24.
//

#ifndef TC_APPLICATION_CAPTURE_MESSAGE_H
#define TC_APPLICATION_CAPTURE_MESSAGE_H

#include <cstdint>

namespace tc
{

    constexpr auto kCaptureVideoFrame = 0x0001;
    constexpr auto kCaptureAudioFrame = 0x0002;
    constexpr auto kCaptureDebugInfo = 0x0003;

    constexpr auto kCaptureVideoByHandle = 0x1000;
    constexpr auto kCaptureVideoBySharedMemory = 0x1001;

    class CaptureBaseMessage {
    public:
        uint32_t type = 0;
        // shm 中的数据大小
        uint32_t data_length = 0;
    };

    class CaptureVideoFrame : public CaptureBaseMessage {
    public:
        // constexpr auto kCaptureVideoByHandle = 0x1000;
        // constexpr auto kCaptureVideoBySharedMemory = 0x1001;
        uint32_t capture_type_;
        uint32_t frame_width_;
        uint32_t frame_height_;
        uint64_t frame_index_;
        uint64_t handle_;
    };

    class CaptureAudioFrame: public CaptureBaseMessage {
    public:
        CaptureAudioFrame() : CaptureBaseMessage() {
            type = kCaptureAudioFrame;
        }
    public:
        uint64_t frame_index_{};
    };

    class CaptureDebugInfo : public CaptureBaseMessage {
    public:

    };

}

#endif //TC_APPLICATION_CAPTURE_MESSAGE_H
