//
// Created by RGAA on 2024-02-26.
//

#include "easyhook/easyhook.h"
#include "tc_common/log.h"

#define CAPTURETEX_API __declspec(dllexport)

using namespace tc;

extern "C" CAPTURETEX_API void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO * remote_info) {
//    client_manager->CopyUserData(remote_info->UserData, (int)remote_info->UserDataSize);
//
//    auto log_path = std::string(params->host_exe_folder) + "/tc_capture_inject.log";
//    Logger::InitLog(log_path, true);

    RhWakeUpProcess();
}
