//
// Created by hy on 2024/3/17.
//

#include "ws_ipc_client.h"
#include "tc_common/log.h"

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
                // Set the binary message write option.
                ws_client_->ws_stream().binary(true);

                // Set the text message write option. The sent text must be utf8 format.
                //ws_client_->ws_stream().text(true);

                // how to set custom websocket request data :
//            ws_client_->ws_stream().set_option(
//                    websocket::stream_base::decorator([](websocket::request_type &req) {
//                        req.set(http::field::authorization, "websocket-client-authorization");
//                    }));
                LOGI("ws client init...");
            }).bind_connect([&]() {
                if (asio2::get_last_error()) {
                    LOGI("ws client connect msg: {}", asio2::get_last_error_msg());
                } else {
                    LOGI("ws client connect msg success.");
                }

//            std::string s;
//            s += '<';
//            int len = 128 + std::rand() % 512;
//            for (int i = 0; i < len; i++) {
//                s += (char) ((std::rand() % 26) + 'a');
//            }
//            s += '>';
//
//            ws_client_->async_send(std::move(s), [](std::size_t bytes_sent) { std::ignore = bytes_sent; });

            }).bind_upgrade([&]() {

                if (asio2::get_last_error()) {
//                std::cout << "upgrade failure : " << asio2::last_error_val() << " " << asio2::last_error_msg()
//                          << std::endl;
                    LOGI("update failed : {}", asio2::get_last_error_msg());
                } else {
                    const websocket::response_type &rep = ws_client_->get_upgrade_response();
                    auto it = rep.find(http::field::authentication_results);
                    if (it != rep.end()) {
                        beast::string_view auth = it->value();
//                    std::cout << auth << std::endl;
//                            ASIO2_ASSERT(auth == "200 OK");
                    }
                    LOGI("update success...");
//                std::cout << "upgrade success : " << rep << std::endl;
                }
            }).bind_recv([&](std::string_view data) {
                printf("recv : %zu %.*s\n", data.size(), (int) data.size(), data.data());

//            std::string s;
//            s += '<';
//            int len = 128 + std::rand() % 512;
//            for (int i = 0; i < len; i++) {
//                s += (char) ((std::rand() % 26) + 'a');
//            }
//            s += '>';

//            ws_client_->async_send(std::move(s));
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

    }

    void WsIpcClient::PostIpcMessage(const std::string& msg) {
        if (!ws_client_) {
            LOGE("ws ipc client is null.");
            return;
        }
        ws_client_->async_send(msg);
    }

}