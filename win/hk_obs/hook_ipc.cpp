//
// Created by RGAA on 2024-02-18.
//

#include "hook_ipc.h"
#include "client_ipc_manager.h"

namespace tc
{

    std::shared_ptr<HookIpc> HookIpc::Make(uint32_t pid) {
        return std::make_shared<HookIpc>(pid);
    }

    HookIpc::HookIpc(uint32_t pid) {
        current_pid_ = pid;
        client_ipc_mgr_ = ClientIpcManager::Make();
        client_ipc_mgr_->Init(pid, kHostToClientShmSize);
        client_ipc_mgr_->Wait();
        //client_ipc_mgr_->MockSend();
    }

    void HookIpc::Send(std::shared_ptr<Data>&& data) {
        client_ipc_mgr_->Send(std::move(data));
    }

}