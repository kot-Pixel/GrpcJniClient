#include <string>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>

#include <android/log.h>

#define PROJECT_TAG_2 "RpcClient"

#define Client_LOGI(...) __android_log_print(ANDROID_LOG_INFO, PROJECT_TAG_2, __VA_ARGS__)
#define Client_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, PROJECT_TAG_2, __VA_ARGS__)
#define Client_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, PROJECT_TAG_2, __VA_ARGS__)

#define Client_LOGD_TAG(tag, ...) __android_log_print(ANDROID_LOG_DEBUG, tag, __VA_ARGS__)
#define Client_LOGI_TAG(tag, ...) __android_log_print(ANDROID_LOG_INFO,  tag, __VA_ARGS__)
#define Client_LOGE_TAG(tag, ...) __android_log_print(ANDROID_LOG_ERROR, tag, __VA_ARGS__)

class NngUdsRpcClient {
public:
    NngUdsRpcClient(const std::string& url);
    ~NngUdsRpcClient();

    bool connect();
    std::string call(const std::string& method, const std::string& param);

private:
    std::string m_url;
    nng_socket m_sock;
    bool m_connected;
};