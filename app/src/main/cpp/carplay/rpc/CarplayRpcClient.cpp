#include <jni.h>
#include "CarplayRpcClient.h"

CarplayRpcClient * rpcClient;

const OnMessageCallback callback = [](const carplay::bt::RfcommPacket& packet) {

};

/**
 * init all rpc
 */
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRpcManager_initCarplayRpcClient(JNIEnv *env, jobject thiz) {
    rpcClient = new CarplayRpcClient(callback);
    return true;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_kotlinx_grpcjniclient_rpc_BluetoothRpc_bluetoothDeviceIap2DeviceState(JNIEnv *env,
                                                                               jobject thiz) {

}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_BluetoothRpc_receiveBtIap2Data(JNIEnv *env, jobject thiz,
                                                                  jbyteArray rfcomm_data,
                                                                  jint data_length) {
    jsize len = env->GetArrayLength(rfcomm_data);
    jbyte* bytes = env->GetByteArrayElements(rfcomm_data, nullptr);
    std::string payload(reinterpret_cast<const char*>(bytes), len);
    env->ReleaseByteArrayElements(rfcomm_data, bytes, JNI_ABORT);

    return rpcClient->sendRfcommData(payload);
}