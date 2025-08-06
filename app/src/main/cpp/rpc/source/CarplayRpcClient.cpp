#include <jni.h>
//#include "JniClassLoaderHelper.h"
#include "CarplayNativeLogger.h"


//CarplayRpcClient * rpcClient = nullptr;

//JNIEXPORT jint JNI_OnLoad(JavaVM *jvm, void *reserved) {
//    JNIEnv* env = nullptr;
//    if (jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
//        return JNI_ERR;
//    }
//    JniClassLoaderHelper::instance().initialize(jvm, env);
//
//    return JNI_VERSION_1_6;
//}

jobject g_kotlinObject = nullptr;
jmethodID g_callbackMethod = nullptr;

JNIEnv *g_env = nullptr;

/**
 * init all rpc
 */
//extern "C"
//JNIEXPORT jboolean JNICALL
//Java_com_kotlinx_grpcjniclient_rpc_CarplayRpcManager_initCarplayRpcClient(JNIEnv *env, jobject thiz) {

//    if (rpcClient == nullptr) {
//        rpcClient = new CarplayRpcClient();
//    }
//    return true;
//}

extern "C"
JNIEXPORT void JNICALL
Java_com_kotlinx_grpcjniclient_rpc_BluetoothRpc_bluetoothDeviceIap2DeviceState(JNIEnv *env, jobject thiz) {

}

//extern "C"
//JNIEXPORT jboolean JNICALL
//Java_com_kotlinx_grpcjniclient_rpc_BluetoothRpc_receiveBtIap2Data(JNIEnv *env, jobject thiz,
//                                                                  jbyteArray rfcomm_data,
//                                                                  jint data_length) {

//    jsize len = env->GetArrayLength(rfcomm_data);
//    jbyte* bytes = env->GetByteArrayElements(rfcomm_data, nullptr);
//    std::string payload(reinterpret_cast<const char*>(bytes), len);
//    env->ReleaseByteArrayElements(rfcomm_data, bytes, JNI_ABORT);
//
//    return rpcClient->sendRfcommData(payload);
//}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_BluetoothRpc_startBtIap2Link(JNIEnv *env, jobject thiz) {
//    if (rpcClient != nullptr) {
//        bool linkStatus = rpcClient->startBtRfcommIap2Link();
//        if (linkStatus) {
//            LOGD("start bt rfcomm iap2 link result success");
//            rpcClient->initCarplayBtRfcommClient();
//        } else {
//            LOGD("start bt rfcomm iap2 link result failure");
//        }
//    }
    return false;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRpcManager_initCarplayRpcPeerDial(JNIEnv *env,
                                                                            jobject thiz) {
    LOGD("Java_com_kotlinx_grpcjniclient_rpc_CarplayRpcManager_initCarplayRpcPeerDial");
}