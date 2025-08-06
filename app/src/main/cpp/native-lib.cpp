#include <jni.h>
#include <string>
#include <sys/stat.h>
#include <thread>
//#include "CarplayBtServiceImpl.h"
//#include "ZUdsRpcServer.hpp"
#include "NngUdsRpcServer.hpp"
#include "NngUdsRpcClient.hpp"
#include "NngUdsRpcPeerListener.hpp"
#include "NngUdsRpcPeerDial.hpp"
#include "bluetooth_generated.h"
#include "object_generated.h"

//void RunUDSServer() {
//    grpc_init();
//    const std::string uds_address = "unix-abstract:carplay_bt";
//    CarplayBtServiceImpl service;
//    ServerBuilder builder;
//    builder.AddListeningPort(uds_address, grpc::InsecureServerCredentials());
//    builder.RegisterService(&service);
//
//    std::unique_ptr<Server> server(builder.BuildAndStart());
//    if (!server) {
//        LOGE("Failed to start server, error: %s", strerror(errno));
//        grpc_shutdown();
//        return;
//    }
//
//    LOGI("Server listening on Unix Domain Socket:  %s", uds_address.data());
//    server->Wait();
//    grpc_shutdown();
//    LOGI("grpc server shut down");
//}


extern "C"
JNIEXPORT void JNICALL
Java_com_kotlinx_grpcjniserver_MainActivity_runUdpCarplayServer(JNIEnv *env, jobject thiz) {
//    std::thread grpc_thread(RunUDSServer);
//    grpc_thread.detach();
//    ZUdsRpcServer ser;
//    ser.initZUdsRpcServer();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_kotlinx_grpcjniclient_MainActivity_stringFromJNI(JNIEnv *env, jobject thiz) {
    return 1;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_kotlinx_grpcjniclient_CarplayApplication_stringFromJNI(JNIEnv *env, jobject thiz) {
    NngUdsRpcPeerDial dial;


    if (dial.start()) {
        std::thread([&dial]() {
            dial.run_forever();
        }).detach();
    }

    flatbuffers::FlatBufferBuilder builder;

    std::vector<uint8_t> payload_data = { 0x01, 0x02, 0x03, 0x04, 0x05 };

    auto payload = builder.CreateVector(payload_data);

    bluetooth::BtRfcommDataBuilder req_builder(builder);
    req_builder.add_data(payload);
    auto req_data = req_builder.Finish();

    builder.Finish(req_data);

    const void* req_buf = builder.GetBufferPointer();
    size_t req_size = builder.GetSize();

    std::string response2;

    bool success = dial.call_remote("receiveRfcommData", req_buf, req_size, response2);

    LOGI("dial invoke remote function response %s", response2.c_str());
    LOGI("dial invoke remote function response %zu", response2.length());
    LOGI("dial invoke remote function response length %d", success);

    const uint8_t* buf = reinterpret_cast<const uint8_t*>(response2.data());
    size_t buf_len = response2.size();

    flatbuffers::Verifier verifier(buf, buf_len);
    if (verifier.VerifyBuffer<response::IntResponse>()) {
        const response::IntResponse *int_resp = flatbuffers::GetRoot<response::IntResponse>(buf);
        if (int_resp) {
            int value = int_resp->value();
            LOGI("Got IntResponse value = %d", value);
        } else {
            LOGE("IntResponse parse failed");
        }
    } else {
        LOGE("Failed to verify IntResponse buffer");
    }
}