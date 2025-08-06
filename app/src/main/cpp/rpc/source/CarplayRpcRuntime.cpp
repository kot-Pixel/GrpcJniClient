#include <thread>
#include <jni.h>
#include "CarplayRpcRuntime.h"
#include "bluetooth_generated.h"
#include "request_param_generated.h"

CarplayRpcRuntime *rpcRuntime = nullptr;

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

void CarplayRpcRuntime::postRunRpcDialThread(){
    if (dialStarted) {
        dialRunning = true;
        dial.run_forever();
    }

    LOGD("postRunRpcDialThread run forever");
}


void CarplayRpcRuntime::rpcRemoteCall(const std::string &method, const void *req_buf, size_t len, uint8_t* &resp_buf, size_t* resp_size) {
    bool success = dial.call_remote(method, req_buf, len, resp_buf, resp_size);
    LOGD("rpcRemoteCall name is: %s , result is :%d", method.c_str(), success);
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRpcManager_initCarplayRpc(JNIEnv *env, jobject thiz) {
    LOGD("init jni carplay rpc");

    if (rpcRuntime == nullptr) {
        rpcRuntime = new CarplayRpcRuntime();
    }

    if (rpcRuntime != nullptr) {
        rpcRuntime->initCarplayRpcRuntime();

        return rpcRuntime->checkPeerRpcDialAvailable();
    } else {
        return false;
    }
}



extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_BluetoothRpc_receiveBtIap2Data(JNIEnv *env, jobject thiz,
                                                                  jbyteArray rfcomm_data,
                                                                  jint data_length) {
    if (rpcRuntime != nullptr && rpcRuntime->checkPeerRpcDialAvailable()) {
        uint8_t * resp_buffer = nullptr;
        size_t resp_length;

        jbyte* native_data = env->GetByteArrayElements(rfcomm_data, nullptr);
        if (native_data == nullptr) {
            return JNI_FALSE;
        }

        std::vector<uint8_t> payload_data(reinterpret_cast<uint8_t*>(native_data),
                                          reinterpret_cast<uint8_t*>(native_data + data_length));
        env->ReleaseByteArrayElements(rfcomm_data, native_data, JNI_ABORT);

        flatbuffers::FlatBufferBuilder builder;

        auto payload = builder.CreateVector(payload_data);

        bluetooth::BtRfcommDataBuilder req_builder(builder);
        req_builder.add_data(payload);
        auto req_data = req_builder.Finish();

        builder.Finish(req_data);

        const void* req_buf = builder.GetBufferPointer();

        size_t req_size = builder.GetSize();

        rpcRuntime->rpcRemoteCall("receiveRfcommDataFromBt", req_buf, req_size, resp_buffer, &resp_length);

        delete[] resp_buffer;

        return true;
    } else {
        return JNI_FALSE;
    }
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_BluetoothRpc_startBtIap2Link(JNIEnv *env, jobject thiz) {
    if (rpcRuntime != nullptr && rpcRuntime->checkPeerRpcDialAvailable()) {

        flatbuffers::FlatBufferBuilder builder;
        auto req = request::CreateVoidRequest(builder);
        builder.Finish(req);

        const void* req_buf = builder.GetBufferPointer();
        size_t req_size = builder.GetSize();

        uint8_t* resp_buf = nullptr;
        size_t resp_len = 0;

        rpcRuntime->rpcRemoteCall("startBtIap2Link", req_buf, req_size, resp_buf, &resp_len);
        delete[] resp_buf;

        return JNI_TRUE;
    } else {
        return JNI_FALSE;
    }
}