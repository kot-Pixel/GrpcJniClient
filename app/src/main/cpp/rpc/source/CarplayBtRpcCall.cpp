//
// Created by Tom on 2025/8/7.
//

#include "CarplayRpcRuntime.h"
#include <jni.h>

#include "bluetooth_generated.h"
#include "carplay_generated.h"
#include "request_param_generated.h"
#include "response_param_generated.h"
#include "wifi_generated.h"
#include "JniClassLoaderHelper.h"

// rpc handle remote call function name define start
#define RECEIVE_BT_IAP2_DETECT_HANDLE_FUNCTION_NAME "sendIap2DetectPacket"
#define RECEIVE_CARPLAY_AVAILABLE_HANDLE_FUNCTION_NAME "carplayAvailable"
// rpc handle remote call function name define end

// rpc call remote function name define start
#define CALL_BT_IAP2_START_LINK_FUNCTION_NAME "startBtIap2Link"
#define CALL_START_WIRELESS_CARPLAY_SESSION_FUNCTION_NAME "startWirelessCarplaySession"
// rpc call remote function name define end


//-------------------------------------------下面是rpc的handle-----------------------------------------------------------------//

flatbuffers::Offset<response::VoidResponse> handleSendIap2DetectPacket(
        const bluetooth::BtRfcommData *req,
        flatbuffers::FlatBufferBuilder &fbb) {
    auto payload = req->data();
    int len = payload->size();

    if (len == 0) {
        LOGE("Payload is empty");
        return response::CreateVoidResponse(fbb);
    }

    JniClassLoaderHelper::instance().withEnv([&](JNIEnv *env) {
        jobject byteBufferObj = env->NewDirectByteBuffer((void *) payload->data(), len);
        if (!byteBufferObj) {
            LOGE("Failed to create ByteBuffer");
            return;
        }

        JniClassLoaderHelper::instance().callStaticVoidMethod(env,
                                                              "com/kotlinx/grpcjniclient/bt/BluetoothRfcommManager",
                                                              "callbackWithByteArray",
                                                              "(Ljava/nio/ByteBuffer;)V",
                                                              byteBufferObj);
        env->DeleteLocalRef(byteBufferObj);
    });

    return response::CreateVoidResponse(fbb);
}

flatbuffers::Offset<response::VoidResponse> handleCarplayAvailablePacket(
        const carplay::CarPlayAvailability *req,
        flatbuffers::FlatBufferBuilder &fbb) {
    JniClassLoaderHelper::instance().withEnv([&](JNIEnv *env) {

        jstring usbStr = env->NewStringUTF(req->usb_transport_identifier()->c_str());
        jstring btStr  = env->NewStringUTF(req->bluetooth_transport_identifier()->c_str());

        JniClassLoaderHelper::instance().callStaticVoidMethod(env,
                                                              "com/kotlinx/grpcjniclient/bt/BluetoothRfcommManager",
                                                              "callbackCarplayAvailable",
                                                              "(ZLjava/lang/String;ZLjava/lang/String;)V",
                                                              (jboolean) req->wired_available(),
                                                              usbStr,
                                                              (jboolean) req->wireless_available(),
                                                              btStr
        );
        env->DeleteLocalRef(usbStr);
        env->DeleteLocalRef(btStr);
    });

    return response::CreateVoidResponse(fbb);
}

//--------------------------------------下面是rpc的Call---------------------------------------------------------//


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_BluetoothRpc_receiveBtIap2Data(JNIEnv *env, jobject thiz,
                                                                  jbyteArray rfcomm_data,
                                                                  jint data_length) {
    auto &rpcRuntime = CarplayRpcRuntime::instance();
    if (rpcRuntime.checkPeerRpcDialAvailable()) {
        uint8_t *resp_buffer = nullptr;
        size_t resp_length;

        jbyte *native_data = env->GetByteArrayElements(rfcomm_data, nullptr);
        if (native_data == nullptr) {
            return JNI_FALSE;
        }

        std::vector<uint8_t> payload_data(reinterpret_cast<uint8_t *>(native_data),
                                          reinterpret_cast<uint8_t *>(native_data + data_length));
        env->ReleaseByteArrayElements(rfcomm_data, native_data, JNI_ABORT);

        flatbuffers::FlatBufferBuilder builder;

        auto payload = builder.CreateVector(payload_data);

        bluetooth::BtRfcommDataBuilder req_builder(builder);
        req_builder.add_data(payload);
        auto req_data = req_builder.Finish();

        builder.Finish(req_data);

        const void *req_buf = builder.GetBufferPointer();

        size_t req_size = builder.GetSize();

        rpcRuntime.rpcRemoteCall("receiveRfcommDataFromBt", req_buf, req_size, resp_buffer,
                                 &resp_length);

        delete[] resp_buffer;

        return JNI_TRUE;
    } else {
        return JNI_FALSE;
    }
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_BluetoothRpc_startBtIap2Link(JNIEnv *env, jobject thiz) {
    auto &rpcRuntime = CarplayRpcRuntime::instance();
    if (rpcRuntime.checkPeerRpcDialAvailable()) {

        //注册远程代码调用
        rpcRuntime.resigteRpcMethod<bluetooth::BtRfcommData, response::VoidResponse>(
                RECEIVE_BT_IAP2_DETECT_HANDLE_FUNCTION_NAME, handleSendIap2DetectPacket);

        rpcRuntime.resigteRpcMethod<carplay::CarPlayAvailability, response::VoidResponse>(
                RECEIVE_CARPLAY_AVAILABLE_HANDLE_FUNCTION_NAME, handleCarplayAvailablePacket);

        flatbuffers::FlatBufferBuilder builder;
        auto req = request::CreateVoidRequest(builder);
        builder.Finish(req);
        const void *req_buf = builder.GetBufferPointer();
        size_t req_size = builder.GetSize();
        uint8_t *resp_buf = nullptr;
        size_t resp_len = 0;
        rpcRuntime.rpcRemoteCall(CALL_BT_IAP2_START_LINK_FUNCTION_NAME, req_buf, req_size, resp_buf,
                                 &resp_len);
        delete[] resp_buf;

        return JNI_TRUE;
    } else {
        return JNI_FALSE;
    }
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRuntime_startCarplaySession(
        JNIEnv *env,
        jobject thiz,
        jstring hostapd_ssid,
        jstring hostapd_pwd,
        jint hostapd_channel,
        jstring hostapd_net_interface_v6_address,
        jint hostapd_security_type) {

    auto &rpcRuntime = CarplayRpcRuntime::instance();

    if (!rpcRuntime.checkPeerRpcDialAvailable()) {
        return JNI_FALSE;
    }

    const char *ssid_str = env->GetStringUTFChars(hostapd_ssid, nullptr);
    const char *pwd_str = env->GetStringUTFChars(hostapd_pwd, nullptr);
    const char *ipv6_str = env->GetStringUTFChars(hostapd_net_interface_v6_address, nullptr);

    flatbuffers::FlatBufferBuilder builder;

    auto ssid_fb = builder.CreateString(ssid_str);
    auto pwd_fb = builder.CreateString(pwd_str);
    auto ipv6_fb = builder.CreateString(ipv6_str);

    auto req = wifi::CreateHostapdInfo(
            builder,
            ssid_fb,
            pwd_fb,
            hostapd_channel,
            ipv6_fb,
            hostapd_security_type
    );

    env->ReleaseStringUTFChars(hostapd_ssid, ssid_str);
    env->ReleaseStringUTFChars(hostapd_pwd, pwd_str);
    env->ReleaseStringUTFChars(hostapd_net_interface_v6_address, ipv6_str);

    builder.Finish(req);

    const void *req_buf = builder.GetBufferPointer();
    size_t req_size = builder.GetSize();
    uint8_t *resp_buf = nullptr;
    size_t resp_len = 0;

    rpcRuntime.rpcRemoteCall(
            CALL_START_WIRELESS_CARPLAY_SESSION_FUNCTION_NAME,
            req_buf,
            req_size,
            resp_buf,
            &resp_len
    );

    delete[] resp_buf;

    return JNI_TRUE;
}
