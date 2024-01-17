//
// Created by Administrator on 2024/1/3.
//

#ifndef WIN_CAPTURE_MONITOR_UTIL_H
#define WIN_CAPTURE_MONITOR_UTIL_H

#include <dxgi.h>

//用来表示屏幕索引顺序
namespace tc {
    enum class EMonitorIndex {
        kFirst,
        kSecond,
        kThird,
        kFourth,
    };

    class MonitorWinInfo {
    public:
        MonitorWinInfo() = default;

        ~MonitorWinInfo() = default;

        DXGI_OUTPUT_DESC dxgi_output_desc_;
    };
}
#endif //WIN_CAPTURE_MONITOR_UTIL_H
