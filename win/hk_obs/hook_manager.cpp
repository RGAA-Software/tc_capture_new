//
// Created by RGAA on 2024-02-18.
//

#include "hook_manager.h"
#include "hook_ipc.h"
#include "tc_common/data.h"
#include "client_ipc_manager.h"
#include "tc_common/log.h"
#include "hk_video/shared_texture.h"
#include <Windows.h>

namespace tc
{

    void HookManager::Init() {
        auto pid = GetCurrentProcessId();
        hello_msg_ = std::make_shared<CaptureHelloMessage>();
        current_pid_ = pid;
        client_ipc_mgr_ = ClientIpcManager::Make();
        client_ipc_mgr_->Init(pid, kHostToClientShmSize);
        client_ipc_mgr_->WaitForMessage();
        client_ipc_mgr_->RegisterHelloMessageCallback([=, this](std::shared_ptr<CaptureHelloMessage>&& msg) {
            LOGI("Hello msg : present:{}, present1: {}, resize: {}, release: {}", msg->dxgi_present, msg->dxgi_present1, msg->dxgi_resize, msg->dxgi_release);
            this->hello_msg_ = std::move(msg);
        });

        shared_texture_ = std::make_shared<SharedTexture>();
    }

    void HookManager::Send(std::shared_ptr<Data>&& data) {
        client_ipc_mgr_->Send(std::move(data));
    }

}