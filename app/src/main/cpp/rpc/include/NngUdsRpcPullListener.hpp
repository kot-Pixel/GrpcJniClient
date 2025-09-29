#pragma once
#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>
#include <atomic>
#include <thread>
#include <functional>
#include <cstdint>
#include <android/log.h>

#define PROJECT_TAG_PULL "RpcStreamPull"

#define LOGD_PULL(...) __android_log_print(ANDROID_LOG_INFO, PROJECT_TAG_PULL, __VA_ARGS__)
#define LOGI_PULL(...) __android_log_print(ANDROID_LOG_DEBUG, PROJECT_TAG_PULL, __VA_ARGS__)
#define LOGE_PULL(...) __android_log_print(ANDROID_LOG_ERROR, PROJECT_TAG_PULL, __VA_ARGS__)

class NngUdsRpcPullListener {
public:
    using RecvCallback = std::function<void(const uint8_t* data, size_t len)>;

    NngUdsRpcPullListener();
    ~NngUdsRpcPullListener();

    bool init(const char* url);

    void startRecvLoop(RecvCallback cb);

    void close();

private:
    nng_socket sock{};
    std::atomic<bool> running{false};
    std::thread recvThread;
};
