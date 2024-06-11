//
// Created by Administrator on 2024/1/3.
//

#ifndef WIN_CAPTURE_MONITOR_UTIL_H
#define WIN_CAPTURE_MONITOR_UTIL_H

#include <dxgi.h>
#include <string>
#include <string>
#include <sstream>
#include "tc_common_new/string_ext.h"

namespace tc
{
    using MonitorIndex = uint32_t;

    class DxgiMonitorInfo {
    public:
        MonitorIndex index_{};
        std::string name_;
        bool attached_desktop_{};
        uint32_t top_{};
        uint32_t left_{};
        uint32_t right_{};
        uint32_t bottom_{};

    public:

        [[nodiscard]] bool Valid() const {
            return !name_.empty() && right_ > left_ && bottom_ > top_;
        }

        [[nodiscard]] std::string Dump() const {
            std::stringstream ss;
            ss << "Monitor index: " << index_ << ", name: " << name_ << std::endl;
            ss << "attached desktop: " << attached_desktop_ << std::endl;
            ss << "left: " << left_ << ", top: " << top_ << ", right: " << right_ << ", bottom: " << bottom_ << std::endl;
            return ss.str();
        }
    };

}
#endif //WIN_CAPTURE_MONITOR_UTIL_H
