//
// Created by RGAA on 2023-12-24.
//

#ifndef TC_APPLICATION_CAPTURE_MESSAGE_H
#define TC_APPLICATION_CAPTURE_MESSAGE_H

#include <cstdint>

namespace tc
{

    struct CaptureMessage {
        uint64_t frame_index_;
        uint64_t handle_;
    };

}

#endif //TC_APPLICATION_CAPTURE_MESSAGE_H
