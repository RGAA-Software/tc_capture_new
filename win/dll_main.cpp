//
// Created by RGAA on 2023-12-20.
//

#include "easyhook/easyhook.h"
#include "tc_capture/inject_params.h"
#include "tc_common/log.h"

//#ifdef CAPTURETEX_EXPORTS
#define CAPTURETEX_API __declspec(dllexport)
//#else
//#define CAPTURETEX_API __declspec(dllimport)
//#endif

using namespace tc;

extern "C" CAPTURETEX_API void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO * remote_info) {
    InjectParams inject_data;
    memset(&inject_data, 0, sizeof(InjectParams));
    bool has_user_data = remote_info->UserData != nullptr;
    if (has_user_data) {
        memcpy(&inject_data, remote_info->UserData, remote_info->UserDataSize);
    }

    Logger::InitLog("tc_capture_inject.log", true);
    LOGI("----------------------------------------------------");
    LOGI("Inject host : {}", inject_data.host_exe_folder);

    for (int i = 0; i < 100; i++) {
        LOGI("----------------------------------------------------");
    }
//    auto log_suffix_path = "rgaa_injector_" + std::to_string(remote_info->HostPID) + ".log";
//    std::string log_file = log_suffix_path;
//
//    if (has_user_data) {
//        log_file = std::string(inject_data.host_exe_folder) + "/logs/" + log_suffix_path;
//    }
//
//    rgaa::Logger::Init(log_file, true);
//    auto conn = rgaa::InterCommClient::Instance();
//    conn->Init(remote_info->HostPID);
//
//    LOG_INFO("Injected DLL : %d", remote_info->HostPID);
//    LOG_INFO("UserData : %s", log_file.c_str());
//    LOG_INFO("Enable Gpu router ? : %d", inject_data.enable_gpu_router);
//    LOG_INFO("Enable Audio capture ? : %d", inject_data.enable_hook_audio);
//
//    g_hook_event->Init();
//
//    if (inject_data.enable_hook_audio) {
//        g_hook_core_audio->Init();
//    }
//
//    if (inject_data.enable_gpu_router) {
//        g_hook_gpu->Init(inject_data.gpu_router_target_lid);
//    }
//
//    bool hook_video_frame = true;
//    if (hook_video_frame) {
//        if (!g_capture_tex.Run()) {
//            LOG_ERROR("g_capture_tex run %s: failed!");
//            return;
//        }
//    }
//
//    g_hook_event->ReceiveIPCMessages();

    RhWakeUpProcess();
}
