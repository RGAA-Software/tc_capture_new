//
// Created by hy on 2023/12/23.
//

#ifndef TC_APPLICATION_CLIENT_IPC_MANAGER_H
#define TC_APPLICATION_CLIENT_IPC_MANAGER_H

#include <memory>
#include <string>
#include <thread>
#include <mutex>

#include <Poco/NamedEvent.h>
#include <Poco/SharedMemory.h>
#include <Poco/NamedMutex.h>

namespace tc
{

    constexpr auto kHostToClientShmSize = 2 * 1024 * 1024;

    struct FixHeader {
        uint32_t buffer_length = 0;
        uint32_t buffer_index = 0;
        uint64_t buffer_timestamp = 0;
        // data below
    };

    class Data;

    class ClientIpcManager {
    public:

        // for easy-hook
        static ClientIpcManager* Instance() {
            static ClientIpcManager mgr;
            return &mgr;
        }

        // for obs hook
        static std::shared_ptr<ClientIpcManager> Make();

        ClientIpcManager();

        void Init(uint32_t pid, uint32_t shm_buffer_size);
        void Send(const std::string& data);
        void Send(const std::shared_ptr<Data>& data);
        void Send(const char* data, int size);
        void Wait();
        void Exit();

        void MockSend();

    private:

    private:
        uint32_t pid_;
        bool exit_ = false;
        uint32_t buffer_index_ = 0;
        uint32_t shm_buffer_size_ = 0;
        std::shared_ptr<std::thread> recv_thread_ = nullptr;

        std::shared_ptr<Poco::NamedEvent> client_to_host_event_ = nullptr;
        std::shared_ptr<Poco::SharedMemory> client_to_host_shm_ = nullptr;
        std::shared_ptr<Poco::NamedMutex> client_to_host_mtx_ = nullptr;

        std::shared_ptr<Poco::NamedEvent> host_to_client_event_ = nullptr;
        std::shared_ptr<Poco::SharedMemory> host_to_client_shm_ = nullptr;
        std::shared_ptr<Poco::NamedMutex> host_to_client_mtx_ = nullptr;

        std::mutex shm_send_mtx_;

    };

}

#endif //TC_APPLICATION_CLIENT_IPC_MANAGER_H
