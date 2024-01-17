//
// Created by Administrator on 2024/1/17.
//

#ifndef TC_APPLICATION_DESKTOP_CAPTURE_H
#define TC_APPLICATION_DESKTOP_CAPTURE_H

#include <memory>

namespace tc {

    class WGCCapture;

    class DDACapture;

    class WinDesktopCapture
    {
    public:
        WinDesktopCapture();
        bool StartCapture();
        void StopCapture();
    private:
        std::shared_ptr<WGCCapture> wgc_capture_ = nullptr;
        std::shared_ptr<DDACapture> dda_capture_ = nullptr;
    };

} // na

#endif //TC_APPLICATION_DESKTOP_CAPTURE_H
