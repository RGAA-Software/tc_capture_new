//
// Created by Administrator on 2024/1/3.
//

#ifndef WIN_CAPTURE_MONITOR_UTIL_H
#define WIN_CAPTURE_MONITOR_UTIL_H

#include <dxgi.h>
#include <string>
#include "tc_common_new/string_ext.h"

namespace tc
{
    using MonitorIndex = uint32_t;

    enum class EMonitorIndex {
        kFirst,
        kSecond,
        kThird,
        kFourth,
    };

    class DxgiMonitorInfo {
    public:
        MonitorIndex index_{};
        std::string name_;
        bool attached_desktop_{};
    };

}
#endif //WIN_CAPTURE_MONITOR_UTIL_H
