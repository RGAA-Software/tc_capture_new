//
// Created by hy on 2023/12/22.
//

#include <gtest/gtest.h>
#include <fstream>

#include "tc_common/image.h"
#include "tc_common/data.h"
#include "tc_common/file.h"
#include "tc_common/log.h"
#include "../audio/audio_capture.h"
#include "../audio/audio_capture_factory.h"
#include "../audio/audio_file_saver.h"

using namespace tc;

int main(int argc, char** argv) {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

TEST(Test_audio_capture, capture) {
    auto audio_capture = AudioCaptureFactory::Make();
    auto file_saver = std::make_shared<WAVAudioFileSaver>(L"test_record_audio.wav");
    audio_capture->SetAudioFileSaver(file_saver);

    audio_capture->RegisterDataCallback([=](const DataPtr& data) {
        LOGI("audio callback size: {}", data->Size());
    });

    audio_capture->RegisterSplitDataCallback([=](const DataPtr& left_ch, const DataPtr& right_ch) {
        LOGI("audio callback left size: {}, right size: {}", left_ch->Size(), right_ch->Size());
    });

    std::thread record_thread([=] () {
        audio_capture->Prepare();
        audio_capture->StartRecording();
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));
    audio_capture->Stop();
    if (record_thread.joinable()) {
        record_thread.join();
    }
}