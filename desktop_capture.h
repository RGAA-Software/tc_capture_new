//
// Created by Administrator on 2024/1/18.
//

#ifndef TC_APPLICATION_DESKTOP_CAPTURE_H
#define TC_APPLICATION_DESKTOP_CAPTURE_H

namespace tc {
    class DesktopCapture {
    public:
        virtual bool StartCapture() = 0;
        virtual void StopCapture() = 0;
    };
}

#endif //TC_APPLICATION_DESKTOP_CAPTURE_H
