//
// Created by RGAA on 2024-04-20.
//

#ifndef GAMMARAY_DESKTOP_CAPTURE_FACTORY_H
#define GAMMARAY_DESKTOP_CAPTURE_FACTORY_H

#include "desktop_capture.h"
#include "win/desktop_capture/dda_capture.h"

namespace tc
{

    class MessageNotifier;

    class DesktopCaptureFactory {
    public:

        static std::shared_ptr<DesktopCapture> Make(const std::shared_ptr<MessageNotifier>& msg_notifier, const std::string& monitor) {
            // windows
            auto capture =  std::make_shared<DDACapture>(msg_notifier, monitor);
            capture->Init();
            return capture;

            // linux
        }

    };

}

#endif //GAMMARAY_DESKTOP_CAPTURE_FACTORY_H
