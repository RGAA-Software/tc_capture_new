//
// Created by hy on 2023/12/22.
//

#include "audio_capture_factory.h"

#include "wasapi_audio_capture.h"

namespace tc
{

    std::shared_ptr<IAudioCapture> AudioCaptureFactory::Make() {
        return WASAPIAudioCapture::Make();
        // todo FFmpeg
    }

}