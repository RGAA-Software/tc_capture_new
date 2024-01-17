//
// Created by Administrator on 2024/1/17.
//

#include "desktop_capture.h"

#include "dda_capture.h"

namespace tc {

/*支持dda采集了(仅在控制台会话下)，但目前暂不开启dda采集*/
    WinDesktopCapture::WinDesktopCapture()
    {
        dda_capture_ = std::make_shared<DDACapture>();
        dda_capture_->Init();
    }

    bool WinDesktopCapture::StartCapture()
    {
        //return wgc_capture_->Start();
        dda_capture_->Start();
        return true;
    }

    void WinDesktopCapture::StopCapture()
    {
        //wgc_capture_->Stop();
        dda_capture_->UnInit();
    }




} // namespace tc