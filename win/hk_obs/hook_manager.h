//
// Created by RGAA on 2024-02-18.
//

#ifndef TC_APPLICATION_HOOK_MANAGER_H
#define TC_APPLICATION_HOOK_MANAGER_H

#include <memory>
#include <functional>

namespace tc
{

    class HookIpc;
    class Data;

    class HookManager {
    public:

        static HookManager* Instance() {
            static HookManager hm;
            return &hm;
        }

        void Init();

        std::shared_ptr<HookIpc> GetIpcChannel();

    private:

        std::shared_ptr<HookIpc> hook_ipc_ = nullptr;

    };

}

#endif //TC_APPLICATION_HOOK_MANAGER_H
