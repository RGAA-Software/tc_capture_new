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
        ipc_shm_host_to_client_ = std::make_shared<IpcShm>(ipc_shm_host_to_client_name);

        auto ipc_shm_client_to_host_name = "ipc_shm_client_to_host_" + std::to_string(listening_port);
        ipc_shm_host_to_client_ = std::make_shared<IpcShm>(ipc_shm_client_to_host_name);

        auto ipc_host_to_client_name = "ipc_mq_host_to_client_" + std::to_string(listening_port);
        ipc_mq_host_to_client_ = std::make_shared<IpcMsgQueue>(ipc_host_to_client_name, 5);
        bool ok = ipc_mq_host_to_client_->Open();
        if (!ok) {
            LOGE("Open host_to_client msg queue failed !");
        }

        auto ipc_client_to_host_name = "ipc_mq_client_to_host_" + std::to_string(listening_port);
        ipc_mq_client_to_host_ = std::make_shared<IpcMsgQueue>(ipc_client_to_host_name, 5);
        ok = ipc_mq_client_to_host_->Create();
        LOGI("Create ipc_mq_client_to_host : {}", ok);
        if (!ok) {
            LOGE("Create client_to_host msg queue failed !");
        }
//        LOGI("SHM : {} {} ", ipc_shm_host_to_client_name, ipc_shm_client_to_host_name);
//        LOGI("MSG : {} {} ", ipc_host_to_client_name, ipc_client_to_host_name);
    }

    void ClientIpcManager::MockSend() {
        std::thread t([=]() {
            for (int i = 0; i < 100000000; i++) {
                this->ipc_mq_client_to_host_->Send(i);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                LOGI("ipc msg send: {}", i);
            }
        });
        t.detach();
    }

    void ClientIpcManager::MockReceive() {
        std::thread t([=] () {
            for (;;) {
                auto value = this->ipc_mq_host_to_client_->Receive();
                LOGI("Receive from Host: {}", value);
            }
        });
        t.detach();
    }

}