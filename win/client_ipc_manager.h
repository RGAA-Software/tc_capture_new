//
// Created by hy on 2023/12/23.
//

#ifndef TC_APPLICATION_CLIENT_IPC_MANAGER_H
#define TC_APPLICATION_CLIENT_IPC_MANAGER_H

#include <memory>
#include <string>

#include <Poco/NamedEvent.h>

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

        std::shared_ptr<Poco::NamedEvent> client_to_host_event_ = nullptr;
        std::shared_ptr<Poco::NamedEvent> host_to_client_event_ = nullptr;

    };

}

#endif //TC_APPLICATION_CLIENT_IPC_MANAGER_H
