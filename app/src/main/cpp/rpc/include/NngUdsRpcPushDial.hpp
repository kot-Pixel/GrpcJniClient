#pragma once
#include <string>
#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>
#include <android/log.h>


#define PROJECT_TAG "RpcStreamPush"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, PROJECT_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, PROJECT_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, PROJECT_TAG, __VA_ARGS__)

class NngUdsRpcPushDial {
public:
    NngUdsRpcPushDial();
    ~NngUdsRpcPushDial();

    bool init(const std::string& url);

    bool sendData(const uint8_t* data, size_t len);

    void close();

private:
    nng_socket sock{};
};
