//
// Created by hy on 2024/3/17.
//

#ifndef TC_APPLICATION_WS_IPC_CLIENT_H
#define TC_APPLICATION_WS_IPC_CLIENT_H

#include <asio2/websocket/ws_client.hpp>

#include <memory>
#include <thread>

namespace tc
{

    class WsIpcClient {
    public:

        static std::shared_ptr<WsIpcClient> Make(int port);
        explicit WsIpcClient(int port);

        void Start();
        void Exit();

        void PostIpcMessage(const std::string& msg);

    private:

        int port_{0};
        std::shared_ptr<asio2::ws_client> ws_client_ = nullptr;
        std::thread ws_thread_;
    };

}

#endif //TC_APPLICATION_WS_IPC_CLIENT_H
