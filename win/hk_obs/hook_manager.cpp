//
// Created by RGAA on 2024-02-18.
//

#include "hook_manager.h"
#include "hook_ipc.h"
#include "tc_common/data.h"
#include <Windows.h>

namespace tc
{

    void HookManager::Init() {
        auto pid = GetCurrentProcessId();
        hook_ipc_ = HookIpc::Make(pid);
    }

    std::shared_ptr<HookIpc> HookManager::GetIpcChannel() {
        return hook_ipc_;
    }

}