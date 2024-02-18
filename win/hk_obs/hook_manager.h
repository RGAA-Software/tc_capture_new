//
// Created by RGAA on 2024-02-18.
//

#ifndef TC_APPLICATION_HOOK_MANAGER_H
#define TC_APPLICATION_HOOK_MANAGER_H

#include <memory>
#include <functional>

namespace tc
{

    class HookManager {
    public:

        static HookManager* Instance() {
            static HookManager hm;
            return &hm;
        }

        void Init();

    };

}

#endif //TC_APPLICATION_HOOK_MANAGER_H
