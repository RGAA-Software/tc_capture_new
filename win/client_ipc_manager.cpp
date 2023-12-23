//
// Created by hy on 2023/12/23.
//

#include "client_ipc_manager.h"

#include "tc_common/ipc_shm.h"
#include "tc_common/ipc_msg_queue.h"
#include "tc_common/log.h"

#include <thread>

namespace tc
{

    ClientIpcManager::ClientIpcManager() = default;

    void ClientIpcManager::Init(uint32_t listening_port) {

        auto ipc_shm_host_to_client_name = "ipc_shm_host_to_client_" + std::to_string(listening_port);
        auto ipc_shm_client_to_host_name = "ipc_shm_client_to_host_" + std::to_string(listening_port);

        auto ipc_event_host_to_client_name = "ipc_event_host_to_client_" + std::to_string(listening_port);
        auto ipc_event_client_to_host_name = "ipc_event_client_to_host_" + std::to_string(listening_port);

        host_to_client_event_ = std::make_shared<Poco::NamedEvent>(ipc_event_host_to_client_name);
        client_to_host_event_ = std::make_shared<Poco::NamedEvent>(ipc_event_client_to_host_name);
    }

    void ClientIpcManager::MockSend() {
        std::thread t([=]() {
            for (int i = 0; i < 100000000; i++) {
                this->client_to_host_event_->set();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                LOGI("ipc msg send: {}", i);
            }
        });
        t.detach();
    }

    void ClientIpcManager::MockReceive() {
        std::thread t([=] () {
            for (;;) {
                this->host_to_client_event_->wait();
                LOGI("Receive from Host...");
            }
        });
        t.detach();
    }

}