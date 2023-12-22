#include "wasapi_audio_capture.h"
#include "tc_common/log.h"
#include "tc_common/time_ext.h"

#pragma comment(lib, "Winmm.lib")

WCHAR fileName[] = L"loopback-capture.wav";
HMMIO hFile = nullptr;

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
                  if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
                  if ((punk) != nullptr)  \
                    { (punk)->Release(); (punk) = nullptr; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

//class MyAudioSink
//{
//public:
//	HRESULT CopyData(BYTE* pData, UINT32 NumFrames, BOOL* pDone, WAVEFORMATEX* pwfx, HMMIO hFile);
//};
//
//HRESULT WriteWaveHeader(HMMIO hFile, LPCWAVEFORMATEX pwfx, MMCKINFO* pckRIFF, MMCKINFO* pckData);
//HRESULT FinishWaveFile(HMMIO hFile, MMCKINFO* pckRIFF, MMCKINFO* pckData);
//HRESULT RecordAudioStream(MyAudioSink* pMySink);

namespace tc
{

	AudioCapturePtr WASAPIAudioCapture::Make() {
		return std::make_shared<WASAPIAudioCapture>();
	}

	WASAPIAudioCapture::WASAPIAudioCapture() {

	}

	WASAPIAudioCapture::~WASAPIAudioCapture() { 
	}

	int WASAPIAudioCapture::Prepare() {
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		return SUCCEEDED(hr) ? 0 : -1;
	}

	int WASAPIAudioCapture::StartRecording() {
        HRESULT hr;
        REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
        REFERENCE_TIME hnsActualDuration;
        UINT32 bufferFrameCount;
        UINT32 numFramesAvailable;
        IMMDeviceEnumerator* pEnumerator = nullptr;
        IMMDevice* pDevice = nullptr;
        IAudioClient* pAudioClient = nullptr;
        IAudioCaptureClient* pCaptureClient = nullptr;
        WAVEFORMATEX* pwfx = nullptr;
        UINT32 packetLength = 0;

        BYTE* pData;
        DWORD flags;

        MMCKINFO ckRIFF = { 0 };
        MMCKINFO ckData = { 0 };

        hr = CoCreateInstance(
            CLSID_MMDeviceEnumerator, nullptr,
            CLSCTX_ALL, IID_IMMDeviceEnumerator,
            (void**)&pEnumerator);
        EXIT_ON_ERROR(hr)

        hr = pEnumerator->GetDefaultAudioEndpoint(
            eRender, eConsole, &pDevice);
        EXIT_ON_ERROR(hr)

        hr = pDevice->Activate(
            IID_IAudioClient, CLSCTX_ALL,
            nullptr, (void**)&pAudioClient);
        EXIT_ON_ERROR(hr)

        //hr = pAudioClient->GetMixFormat(&pwfx);
        //EXIT_ON_ERROR(hr)

        WAVEFORMATEX waveFormat;
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.nChannels = 2;
        waveFormat.nSamplesPerSec = 48000;
        waveFormat.nAvgBytesPerSec = 192000;
        waveFormat.wBitsPerSample = 16;
        waveFormat.nBlockAlign = 4;
        waveFormat.cbSize = 0;

        if (format_callback_) {
            format_callback_(waveFormat.nSamplesPerSec, waveFormat.nChannels, waveFormat.wBitsPerSample);
        }

        pwfx = &waveFormat;

   
        hr = pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_LOOPBACK,
            hnsRequestedDuration,
            0,
            pwfx,
            nullptr);
        EXIT_ON_ERROR(hr)

        // Get the size of the allocated buffer.
        hr = pAudioClient->GetBufferSize(&bufferFrameCount);
        EXIT_ON_ERROR(hr)

        hr = pAudioClient->GetService(
            IID_IAudioCaptureClient,
            (void**)&pCaptureClient);
        EXIT_ON_ERROR(hr)
        if (file_saver_) {
            hr = std::static_pointer_cast<WAVAudioFileSaver>(file_saver_)->WriteWaveHeader(pwfx, &ckRIFF, &ckData);
        }
            
        if (FAILED(hr)) {
            // WriteWaveHeader does its own logging
            return hr;
        }

        // Calculate the actual duration of the allocated buffer.
        hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

        hr = pAudioClient->Start();  // Start recording.
        EXIT_ON_ERROR(hr)

        // Each loop fills about half of the shared buffer.
        while (!exit_) {
            // Sleep for half the buffer duration.
            //Sleep(hnsActualDuration / REFTIMES_PER_MILLISEC / 2);
     
            Sleep(16);
            //std::cout << "------- duration : " << (hnsActualDuration / REFTIMES_PER_MILLISEC / 2) << std::endl;

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            EXIT_ON_ERROR(hr)

            while (packetLength != 0) {
                if (exit_) {
                    std::cout << "Exit inner loop" << std::endl;
                    break;
                }

                // Get the available data in the shared buffer.
                hr = pCaptureClient->GetBuffer(
                    &pData,
                    &numFramesAvailable,
                    &flags, nullptr, nullptr);
                EXIT_ON_ERROR(hr)

                if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                    pData = nullptr;  // Tell CopyData to write silence.
                }

                LONG bytes_to_write = numFramesAvailable * pwfx->nBlockAlign;
                if (pData && bytes_to_write > 0) {
                    if (data_callback_) {
                        auto data = Data::Make((char*)pData, bytes_to_write);
                        data_callback_(data);
                    }

                    if (split_data_callback_) {
                        auto left_data = Data::Make(nullptr, bytes_to_write / 2);
                        auto right_data = Data::Make(nullptr, bytes_to_write / 2);
                        for (int i = 0; i < bytes_to_write; i += 4) {
                            memcpy((left_data->DataAddr() + i / 4 * 2), ((char*)pData + i), 2);
                            memcpy((right_data->DataAddr() + i / 4 * 2), ((char*)pData + i + 2), 2);
                        }
                        //LOGI("l : {0:d}, r : {1:d}", left_data->Size(), right_data->Size());
                        split_data_callback_(left_data, right_data);
                    }   
                }

                if (file_saver_ && pData && bytes_to_write > 0) {
                    file_saver_->WriteData((char*)pData, bytes_to_write);
                }
                    
                EXIT_ON_ERROR(hr)

                hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
                EXIT_ON_ERROR(hr)

                hr = pCaptureClient->GetNextPacketSize(&packetLength);
                EXIT_ON_ERROR(hr)
            }
        }

        hr = pAudioClient->Stop();
        std::cout << "stopped recording .." << hr << " " << GetLastError() << std::endl;
        EXIT_ON_ERROR(hr)
        if (file_saver_) {
            std::cout << "finish recoding ..." << std::endl;
            hr = std::static_pointer_cast<WAVAudioFileSaver>(file_saver_)->FinishWaveFile(&ckData, &ckRIFF);
        }
            
        if (FAILED(hr)) {
            // FinishWaveFile does it's own logging
            return hr;
        }

    Exit:
        //CoTaskMemFree(pwfx);
        LOGI("WASAPIAudioCapture Release device .");
        SAFE_RELEASE(pCaptureClient)
        SAFE_RELEASE(pAudioClient)
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pEnumerator)
        return hr;
	}

	int WASAPIAudioCapture::Pause() {
        return 0;
	}

	int WASAPIAudioCapture::Stop() {
        exit_ = TRUE;
        return 0;
	}


}