#include <thread>
#include <jni.h>
#include "CarplayRpcRuntime.h"
#include "bluetooth_generated.h"
#include "request_param_generated.h"
#include "response_param_generated.h"
#include "JniClassLoaderHelper.h"

#include "CarplayBtRpcReceive.cpp"


void CarplayRpcRuntime::initCarplayRpcRuntime() {

    LOGD("initCarplayRpcRuntime start");

    dialStarted = dial.start();

    if (dialStarted) {
        worker_thread_ = std::thread(&CarplayRpcRuntime::postRunRpcDialThread, this);
    }
}

bool CarplayRpcRuntime::checkPeerRpcDialAvailable() {
    return dialStarted && dialRunning;
}

void CarplayRpcRuntime::postRunRpcDialThread() {
    if (dialStarted) {
        dialRunning = true;
        dial.run_forever();
    }

    LOGD("postRunRpcDialThread run forever");
}


void CarplayRpcRuntime::rpcRemoteCall(const std::string &method, const void *req_buf, size_t len,
                                      uint8_t *&resp_buf, size_t *resp_size) {
    bool success = dial.call_remote(method, req_buf, len, resp_buf, resp_size);
    LOGD("rpcRemoteCall name is: %s , result is :%d", method.c_str(), success);
}


JNIEXPORT jint JNI_OnLoad(JavaVM *jvm, void *reserved) {
    JNIEnv *env = nullptr;
    if (jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    JniClassLoaderHelper::instance().initialize(jvm, env);

    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRuntime_initCarplayRpc(JNIEnv *env, jobject thiz) {
    LOGD("init jni carplay rpc");
    auto &rpcRuntime = CarplayRpcRuntime::instance();
    rpcRuntime.initCarplayRpcRuntime();
    return rpcRuntime.checkPeerRpcDialAvailable();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRuntime_startCarplaySession(JNIEnv *env, jobject thiz,
                                                                      jstring hostapd_ssid,
                                                                      jstring hostapd_pwd,
                                                                      jstring hostapd_net_interface_v6_address,
                                                                      jint hostapd_security_type) {
    return JNI_TRUE;
}