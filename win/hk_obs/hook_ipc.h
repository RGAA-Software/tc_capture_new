//
// Created by RGAA on 2024-02-18.
//

#ifndef TC_APPLICATION_HOOK_IPC_H
#define TC_APPLICATION_HOOK_IPC_H

#include <memory>

namespace tc
{
    class ClientIpcManager;

    class HookIpc {
    public:

        static std::shared_ptr<HookIpc> Make(uint32_t pid);

        HookIpc(uint32_t pid);

    private:
        uint32_t current_pid_{};
        std::shared_ptr<ClientIpcManager> client_ipc_mgr_ = nullptr;

    };

}
#endif //TC_APPLICATION_HOOK_IPC_H