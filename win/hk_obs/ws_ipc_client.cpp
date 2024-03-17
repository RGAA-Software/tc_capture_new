//
// Created by hy on 2024/3/17.
//

#include "ws_ipc_client.h"
#include "tc_common/log.h"
#include "tc_message.pb.h"
#include "capture_message.h"

namespace tc
{

    std::shared_ptr<WsIpcClient> WsIpcClient::Make(int port) {
        return std::make_shared<WsIpcClient>(port);
    }

    WsIpcClient::WsIpcClient(int port) {
        this->port_ = port;
    }

    void WsIpcClient::Start() {
        ws_thread_ = std::thread([=, this]() {
            ws_client_ = std::make_shared<asio2::ws_client>();
            ws_client_->set_connect_timeout(std::chrono::seconds(2));
            ws_client_->bind_init([&]() {
                ws_client_->ws_stream().binary(true);
                LOGI("ws ipc client init");
            }).bind_connect([&]() {
                if (asio2::get_last_error()) {
                    LOGI("ws client connect msg: {}", asio2::get_last_error_msg());
                } else {
                    LOGI("ws client connect msg success.");
                }
            }).bind_upgrade([&]() {
                if (asio2::get_last_error()) {
                    LOGI("update failed : {}", asio2::get_last_error_msg());
                } else {
                    const websocket::response_type &rep = ws_client_->get_upgrade_response();
                    auto it = rep.find(http::field::authentication_results);
                    if (it != rep.end()) {
                        beast::string_view auth = it->value();
                        LOGI("auth == 200 OK ? : {}", (auth == "200 OK"));
                    }
                    LOGI("update success...");
                }
            }).bind_recv([&](std::string_view data) {
                auto msg = std::make_shared<tc::Message>();
                auto ok = msg->ParseFromArray(data.data(), data.size());
                if (!ok) {
                    LOGI("Parse to ipc message failed. recv data size: {}", (int) data.size());
                    return;
                }
                this->DispatchIpcMessage(std::move(msg));
            });

            LOGI("will start at :{}, {}", port_, "/ipc");
            // the /ws is the websocket upgraged target
            if (!ws_client_->start("127.0.0.1", port_, "/ipc")) {
                LOGI("connect websocket server failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
            }
            LOGI("After start ws client....");
        });
    }

    void WsIpcClient::Exit() {
        if (ws_client_) {
            ws_client_->stop();
        }
        if (ws_thread_.joinable()) {
            ws_thread_.join();
        }
    }

    void WsIpcClient::PostIpcMessage(const std::string& msg) {
        if (!ws_client_) {
            LOGE("ws ipc client is null.");
            return;
        }
        ws_client_->async_send(msg);
    }

    void WsIpcClient::DispatchIpcMessage(std::shared_ptr<tc::Message>&& msg) {
        if (msg->type() == tc::MessageType::kIpcMouseEvent) {
            auto& ipc_msg = msg->ipc_mouse_event();
            auto mem = std::make_shared<MouseEventMessage>();
//            uint64_t hwnd_{};
            mem->hwnd_ = ipc_msg.hwnd();
//            //to do 当服务端采集方式为采集屏幕的时候，当前鼠标事件对应的屏幕索引
//            //uint32_t monitor_index_ = 0;
//            // 当前鼠标x值，相对于窗口
//            uint32_t x_ = 0;
            mem->x_ = ipc_msg.x();
//            // 当前鼠标y值，
//            uint32_t y_ = 0;
            mem->y_ = ipc_msg.y();
//            // 按键掩码, 用来表示摁下了什么按键、抬起了什么按键 ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mouse_event
//            int32_t button_ = 0;
            mem->button_ = ipc_msg.button();
//            int32_t pressed_ = 0;
            mem->pressed_ = ipc_msg.pressed();
//            int32_t released_ = 0;
            mem->released_ = ipc_msg.released();
//            // 鼠标data，滚轮等数据
//            int32_t data_ = 0;
            mem->data_ = ipc_msg.data();
//            // 当前毫秒值时间戳
//            //uint64_t timestamp_ = 0;
//            int32_t delta_x_ = 0;
            mem->delta_x_ = ipc_msg.dx();
//            int32_t delta_y_ = 0;
            mem->delta_y_ = ipc_msg.dy();
//            int32_t middle_scroll_ = 0;
            mem->middle_scroll_ = ipc_msg.middle_scroll();
//            int32_t absolute_ = 0;
            mem->absolute_ = ipc_msg.absolute();
            ipc_cbk_(mem);
        }
    }

}