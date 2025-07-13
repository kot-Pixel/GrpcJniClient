#include <jni.h>
#include "CarplayRpcClient.h"
#include "CarplayNativeLogger.h"

CarplayRpcClient * rpcClient = nullptr;

JNIEXPORT jint JNI_OnLoad(JavaVM *jvm, void *reserved) {
    rpcClient = new CarplayRpcClient(jvm);
    return JNI_VERSION_1_6;
}

jobject g_kotlinObject = nullptr;
jmethodID g_callbackMethod = nullptr;

JNIEnv *g_env = nullptr;

//const OnMessageCallback callback = [](const carplay::bt::RfcommPacket& packet) {
//    LOGD("call back rfcomm data %s", packet.payload().data());
//    LOGD("call back rfcomm data %zu", packet.payload().size());
//
//    if (g_kotlinObject != nullptr && g_callbackMethod != nullptr && g_env != nullptr) {
//        jobject stream = g_env->NewDirectByteBuffer((void*)packet.payload().data(),packet.payload().size());
//        g_env->CallVoidMethod(g_kotlinObject, g_callbackMethod, stream, packet.payload().size());
//        g_env->DeleteLocalRef(stream);
//    }
//};

/**
 * init all rpc
 */
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRpcManager_initCarplayRpcClient(JNIEnv *env, jobject thiz) {



    if (rpcClient == nullptr)
        return false;
    else {
        return rpcClient->initCarplayBtRfcommClient();
    }

//    rpcClient = new CarplayRpcClient(callback);

    g_env = env;

    jclass clazz = env->FindClass("com/kotlinx/grpcjniclient/bt/BluetoothRfcommManager");


    jmethodID getClassLoaderMethod = env->GetMethodID(clazz, "", "()Ljava/lang/ClassLoader;");

// 获取类加载器
    jobject classLoader = env->CallObjectMethod(clazz, getClassLoaderMethod);

// 打印类加载器信息
    if (classLoader != nullptr) {
        jclass classLoaderClass = env->GetObjectClass(classLoader);
        jmethodID toStringMethod = env->GetMethodID(classLoaderClass, "toString", "()Ljava/lang/String;");
        jstring classLoaderString = (jstring) env->CallObjectMethod(classLoader, toStringMethod);
        const char* classLoaderStr = env->GetStringUTFChars(classLoaderString, nullptr);
        LOGI("ClassLoader for BluetoothRfcommManager: %s", classLoaderStr);
        env->ReleaseStringUTFChars(classLoaderString, classLoaderStr);
    }

    if (clazz == nullptr) {
        LOGD("Failed to find JniCallbackHelper class");
        return false;
    }

    LOGD("Success to find JniCallbackHelper class");


    // 2. 获取 INSTANCE 字段（Kotlin object 的单例实例）
    jfieldID instanceField = env->GetStaticFieldID(clazz, "INSTANCE", "Lcom/kotlinx/grpcjniclient/bt/BluetoothRfcommManager;");
    if (instanceField == nullptr) {
        LOGD("Failed to get INSTANCE field");
        env->DeleteLocalRef(clazz);
        return false;
    }

    // 3. 获取单例对象并创建全局引用
    jobject instance = env->GetStaticObjectField(clazz, instanceField);
    g_kotlinObject = env->NewGlobalRef(instance);

    // 4. 获取方法ID
    g_callbackMethod = env->GetMethodID(clazz, "callbackWithByteArray", "([B)V");

    if (g_callbackMethod == nullptr) {
        LOGD("Failed to get method ID");
        env->DeleteLocalRef(instance);
        env->DeleteLocalRef(clazz);
        return false;
    }

    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(instance);

    return true;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_kotlinx_grpcjniclient_rpc_BluetoothRpc_bluetoothDeviceIap2DeviceState(JNIEnv *env, jobject thiz) {

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