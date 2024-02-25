//
// Created by RGAA on 2024-02-18.
//

#ifndef TC_APPLICATION_HOOK_MANAGER_H
#define TC_APPLICATION_HOOK_MANAGER_H

#include <string>
#include <memory>
#include <functional>
#include "capture_message.h"

namespace tc
{

    class ClientIpcManager;
    class Data;

    class HookManager {
    public:

        static HookManager* Instance() {
            static HookManager hm;
            return &hm;
        }

        void Init();
        void Send(std::shared_ptr<Data>&& data);
        std::shared_ptr<ClientIpcManager> GetIpcManager();

    public:
        uint32_t current_pid_{};
        std::string dll_path_;
        std::shared_ptr<ClientIpcManager> client_ipc_mgr_ = nullptr;
        std::shared_ptr<CaptureHelloMessage> hello_msg_ = nullptr;
    };

}

#endif //TC_APPLICATION_HOOK_MANAGER_H
