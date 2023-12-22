#pragma once

#include "audio_capture.h"

#include <Windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <time.h>
#include <mmeapi.h>
#include <iostream>
#include <mutex>
#include <condition_variable>

namespace tc
{

	class WASAPIAudioCapture : public IAudioCapture {
	public:

		static AudioCapturePtr Make();

		WASAPIAudioCapture();
		~WASAPIAudioCapture();

		int Prepare() override;
		int StartRecording() override;
		int Pause() override;
		int Stop() override;
	
	private:

		bool exit_ = false;
	};

}