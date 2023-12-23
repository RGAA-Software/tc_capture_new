//
// Created by hy on 2023/12/23.
//

#ifndef TC_APPLICATION_CLIENT_IPC_MANAGER_H
#define TC_APPLICATION_CLIENT_IPC_MANAGER_H

#include <memory>
#include <string>

namespace tc
{

    class IpcShm;
    class IpcMsgQueue;

    class ClientIpcManager {
    public:

        static ClientIpcManager* Instance() {
            static ClientIpcManager mgr;
            return &mgr;
        }

        ClientIpcManager();

        void Init(uint32_t listening_port);

        void MockSend();
        void MockReceive();

    private:

        std::shared_ptr<IpcShm> ipc_shm_host_to_client_ = nullptr;
        std::shared_ptr<IpcMsgQueue> ipc_mq_host_to_client_ = nullptr;

        std::shared_ptr<IpcShm> ipc_shm_client_to_host_ = nullptr;
        std::shared_ptr<IpcMsgQueue> ipc_mq_client_to_host_ = nullptr;

    };

}

#endif //TC_APPLICATION_CLIENT_IPC_MANAGER_H
