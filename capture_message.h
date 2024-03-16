//
// Created by RGAA on 2023-12-24.
//

#ifndef TC_APPLICATION_CAPTURE_MESSAGE_H
#define TC_APPLICATION_CAPTURE_MESSAGE_H

#include <cstdint>

namespace tc
{
    // type_
    // dll -> app
    class Data;

    constexpr auto kCaptureVideoFrame = 0x0001;
    // dll -> app
    constexpr auto kCaptureAudioFrame = 0x0002;
    // dll -> app
    constexpr auto kCaptureDebugInfo = 0x0003;
    // app -> dll
    constexpr auto kCaptureHelloMessage = 0x0004;
    // app -> dll
    constexpr auto kMouseEventMessage = 0x0005;
    // app -> dll
    constexpr auto kKeyboardEventMessage = 0x0006;

    // capture_type_
    constexpr auto kCaptureVideoByHandle = 0x1000;
    constexpr auto kCaptureVideoBySharedMemory = 0x1001;

    class CaptureBaseMessage {
    public:
        uint32_t type_ = 0;
        // shm 中的数据大小
        uint32_t data_length = 0;
    };

    class CaptureVideoFrame : public CaptureBaseMessage {
    public:
        CaptureVideoFrame() : CaptureBaseMessage() {
            type_ = kCaptureVideoFrame;
        }
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
            type_ = kCaptureAudioFrame;
        }
    public:
        uint64_t frame_index_{};
    };

    class CaptureDebugInfo : public CaptureBaseMessage {
    public:

    };

<<<<<<< Updated upstream
    // app 会在刚注入dll时，通过IPC发送这个消息到dll中
    class CaptureHelloMessage : public CaptureBaseMessage {
    public:
        CaptureHelloMessage() : CaptureBaseMessage() {
            type_ = kCaptureHelloMessage;
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

    class MouseEventMessage : public CaptureBaseMessage {
    public:
        MouseEventMessage() : CaptureBaseMessage() {
            type_ = kMouseEventMessage;
        }
    public:
        uint64_t hwnd_{};
        //to do 当服务端采集方式为采集屏幕的时候，当前鼠标事件对应的屏幕索引
        //uint32_t monitor_index_ = 0;
        // 当前鼠标x值，相对于窗口
        uint32_t x_ = 0;
        // 当前鼠标y值，
        uint32_t y_ = 0;
        // 按键掩码, 用来表示摁下了什么按键、抬起了什么按键 ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mouse_event
        int32_t button_ = 0;
        int32_t pressed_ = 0;
        int32_t released_ = 0;
        // 鼠标data，滚轮等数据
        int32_t data_ = 0;
        // 当前毫秒值时间戳
        //uint64_t timestamp_ = 0;
        int32_t delta_x_ = 0;
        int32_t delta_y_ = 0;
        int32_t middle_scroll_ = 0;
        int32_t absolute_ = 0;
    };

    class KeyboardEventMessage : public CaptureBaseMessage {
    public:
        KeyboardEventMessage() : CaptureBaseMessage() {
            type_ = kKeyboardEventMessage;
        }
    public:
        uint64_t hwnd_{};
        uint32_t key_{};
        uint32_t down_{};
        uint32_t num_lock_state_{};
        uint32_t caps_lock_state_{};
    };

=======
    // 桌面模式下采集鼠标的信息
    class CaptureCursorBitmap : public CaptureBaseMessage {
    public:
        uint32_t width_ = 0;
        uint32_t height_ = 0;
        uint32_t hotspot_x_ = 0;
        uint32_t hotspot_y_ = 0;
        uint32_t x_ = 0;
        uint32_t y_ = 0;
        bool visable_ = true;
        std::shared_ptr<Data> data_ = nullptr;  //存放图片
    };
>>>>>>> Stashed changes
}

#endif //TC_APPLICATION_CAPTURE_MESSAGE_H
