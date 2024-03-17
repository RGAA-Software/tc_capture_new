//
// Created by RGAA on 2024-03-17.
//

#ifndef TC_APPLICATION_IPC_MSG_MAKER_H
#define TC_APPLICATION_IPC_MSG_MAKER_H

#include <string>

#include "capture_message.h"

namespace tc
{

    class IpcMsgMaker {
    public:

        static std::string MakeVideoIpcMessage(const CaptureVideoFrame& frame);

    };

}

#endif //TC_APPLICATION_IPC_MSG_MAKER_H
