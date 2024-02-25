//
// Created by RGAA on 2023-12-24.
//

#ifndef TC_APPLICATION_CAPTURE_MESSAGE_H
#define TC_APPLICATION_CAPTURE_MESSAGE_H

#include <cstdint>

namespace tc
{
    // type
    constexpr auto kCaptureVideoFrame = 0x0001;
    constexpr auto kCaptureAudioFrame = 0x0002;
    constexpr auto kCaptureDebugInfo = 0x0003;
    constexpr auto kCaptureHelloMessage = 0x0004;

    // capture_type_
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
        uint32_t capture_type_ = 0;
        uint32_t frame_width_ = 0;
        uint32_t frame_height_ = 0;
        uint64_t frame_index_ = 0;
        uint64_t frame_format_ = 0;
        uint64_t handle_ = 0;
        int64_t adapter_uid_ = -1; // -1表示没获取到
        int8_t capture_index_ = -1; //采集画面的索引，通常用于桌面采集，比如第一块屏幕的画面，第二块屏幕的画面 ...
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

    // app 会在刚注入dll时，通过IPC发送这个消息到dll中
    class CaptureHelloMessage : public CaptureBaseMessage {
    public:
        CaptureHelloMessage() : CaptureBaseMessage() {
            type = kCaptureHelloMessage;
        }
    public:
#ifdef WIN32
        // [d3d8]
        // present=0x0
        // [d3d9]
        // present=0xb73f0
        // present_ex=0xb7490
        // present_swap=0xc470
        // d3d9_clsoff=0x4030
        // is_d3d9ex_clsoff=0x55a0
        // [dxgi]
        // present=0x15e0
        // present1=0x68dc0
        // resize=0x22f40
        // release=0x3240

        uint64_t d3d9_present = 0;
        uint64_t d3d9_present_ex = 0;
        uint64_t d3d9_present_swap = 0;
        uint64_t d3d9_d3d9_clsoff = 0;
        uint64_t d3d9_is_d3d9ex_clsoff = 0;

        uint64_t dxgi_present = 0;
        uint64_t dxgi_present1 = 0;
        uint64_t dxgi_resize = 0;
        uint64_t dxgi_release = 0;

#endif
    };

}

#endif //TC_APPLICATION_CAPTURE_MESSAGE_H
