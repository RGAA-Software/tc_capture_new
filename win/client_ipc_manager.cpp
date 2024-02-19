//
// Created by hy on 2023/12/23.
//

#include "client_ipc_manager.h"

#include "tc_common/ipc_shm.h"
#include "tc_common/ipc_msg_queue.h"
#include "tc_common/log.h"
#include "tc_common/data.h"
#include "tc_common/time_ext.h"
#include "capture_message.h"

#include <thread>

namespace tc
{

    std::shared_ptr<ClientIpcManager> ClientIpcManager::Make() {
        return std::make_shared<ClientIpcManager>();
    }

    ClientIpcManager::ClientIpcManager() = default;

    void ClientIpcManager::Init(uint32_t listening_port, uint32_t shm_buffer_size) {
        this->listen_port_ = listening_port;
        this->shm_buffer_size_ = shm_buffer_size;
        auto ipc_shm_client_to_host_name = "ipc_shm_client_to_host_" + std::to_string(listening_port);

        auto ipc_event_host_to_client_name = "ipc_event_host_to_client_" + std::to_string(listening_port);
        auto ipc_event_client_to_host_name = "ipc_event_client_to_host_" + std::to_string(listening_port);

        auto mtx_host_to_client_name = "mtx_host_to_client_" + std::to_string(listening_port);
        auto mtx_client_to_host_name = "mtx_client_to_host_" + std::to_string(listening_port);

        host_to_client_event_ = std::make_shared<Poco::NamedEvent>(ipc_event_host_to_client_name);
        client_to_host_event_ = std::make_shared<Poco::NamedEvent>(ipc_event_client_to_host_name);

        client_to_host_shm_ = std::make_shared<Poco::SharedMemory>(ipc_shm_client_to_host_name, shm_buffer_size, Poco::SharedMemory::AccessMode::AM_WRITE);
        LOGI("Shm buffer size: {}", shm_buffer_size);

        host_to_client_mtx_ = std::make_shared<Poco::NamedMutex>(mtx_host_to_client_name);
        client_to_host_mtx_ = std::make_shared<Poco::NamedMutex>(mtx_client_to_host_name);
    }

    static uint64_t last_send_time = TimeExt::GetCurrentTimestamp();

    void ClientIpcManager::Send(const char* data, int size) {
        std::lock_guard<std::mutex> guard(shm_send_mtx_);

        if (size > shm_buffer_size_) {
            return;
        }

        auto current_time = TimeExt::GetCurrentTimestamp();
        auto diff = current_time - last_send_time;
        last_send_time = current_time;
        LOGI("diff: {}", diff);

        client_to_host_mtx_->lock();
        auto begin = client_to_host_shm_->begin();
        auto header = FixHeader {
            .buffer_length = static_cast<uint32_t>(size),
            .buffer_index = buffer_index_++,
            .buffer_timestamp = TimeExt::GetCurrentTimePointUS(),
        };
        memcpy(begin, (char*)&header, sizeof(FixHeader));
        memcpy(begin + sizeof(FixHeader), data, size);
        client_to_host_mtx_->unlock();

        client_to_host_event_->set();
    }

    void ClientIpcManager::Send(const std::shared_ptr<Data>& data) {
        this->Send(data->CStr(), data->Size());
    }

    void ClientIpcManager::Send(const std::string &data) {
        this->Send(data.c_str(), data.size());
    }

    void ClientIpcManager::MockSend() {
        std::thread t([this]() {
            std::ostringstream ss;
            ss << "Good...";
            for (int i = 0; i < 100000000; i++) {
                //std::string data = "current index = " + std::to_string(i) + ", ok.";
//                ss.clear();
//                ss << "current index : " << i << ", ok.";
                CaptureAudioFrame capture_audio_frame_msg{};
                capture_audio_frame_msg.frame_index_ = i;
                auto data = Data::Make(nullptr, sizeof(CaptureAudioFrame));
                memcpy(data->DataAddr(), &capture_audio_frame_msg, sizeof(CaptureAudioFrame));
                this->Send(data);
                std::this_thread::sleep_for(std::chrono::milliseconds(17));

//                auto current_time = TimeExt::GetCurrentTimestamp();
//                auto diff = current_time - last_send_time;
//                last_send_time = current_time;
//                LOGI("diff: {}", diff);
            }
        });
        t.detach();
    }

    void ClientIpcManager::Wait() {
        recv_thread_ = std::make_shared<std::thread>([=, this] () {
            while(!exit_) {
                host_to_client_event_->wait();
                if (!host_to_client_shm_) {
                    auto ipc_shm_host_to_client_name = "ipc_shm_host_to_client_" + std::to_string(listen_port_);
                    host_to_client_shm_ = std::make_shared<Poco::SharedMemory>(ipc_shm_host_to_client_name, kHostToClientShmSize, Poco::SharedMemory::AccessMode::AM_READ);
                }
                host_to_client_mtx_->lock();
                auto begin = host_to_client_shm_->begin();
                auto header = (FixHeader*)begin;
                auto buffer = Data::Make(begin + sizeof(FixHeader), (int)header->buffer_length);
                host_to_client_mtx_->unlock();

                LOGI("WaitForMessage from Host: {}", buffer->AsString());
            }
        });
    }

    void ClientIpcManager::Exit() {
        exit_ = true;
        if (recv_thread_ && recv_thread_->joinable()) {
            recv_thread_->join();
        }
    }

}