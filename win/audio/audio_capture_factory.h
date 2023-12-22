//
// Created by hy on 2023/12/22.
//

#ifndef TC_APPLICATION_AUDIO_CAPTURE_FACTORY_H
#define TC_APPLICATION_AUDIO_CAPTURE_FACTORY_H

#include <memory>

namespace tc
{

    class IAudioCapture;

    class AudioCaptureFactory {
    public:

        static std::shared_ptr<IAudioCapture> Make();

    };

}

#endif //TC_APPLICATION_AUDIO_CAPTURE_FACTORY_H
