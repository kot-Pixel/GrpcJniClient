#include <jni.h>
#include <string>
#include "rfcomm.protoc.grpc.pb.h"
#include "grpc/include/grpcpp/support/channel_arguments.h"
#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>
#include "grpc/include/grpcpp/channel.h"
#include "grpc/include/grpcpp/client_context.h"
#include "grpc/include/grpcpp/create_channel.h"
#include "grpc/include/grpcpp/impl/status.h"
#include "grpc/include/grpcpp/security/credentials.h"
#include "CarplayNativeLogger.h"


using grpc::Channel;
using grpc::ClientContext;
using grpc::CreateChannel;
using grpc::Status;

int test() {
    // 1. 使用 abstract UDS 路径创建 channel
    std::string address = "unix-abstract:carplay_bt";
    auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());

    // 2. 创建 Stub
    std::unique_ptr<carplay::bt::CarplayBtService::Stub> stub =
            carplay::bt::CarplayBtService::NewStub(channel);

    // 3. 开始调用 RfcommTransport（双向流）
    grpc::ClientContext context;
    std::shared_ptr<grpc::ClientReaderWriter<carplay::bt::RfcommPacket, carplay::bt::RfcommPacket>> stream =
            stub->RfcommTransport(&context);

    carplay::bt::RfcommPacket pkt;
    pkt.set_direction(carplay::bt::RfcommTransportDirection::IN);
    pkt.set_payload("AT+CARPLAY\r\n");
    stream->Write(pkt);

    carplay::bt::RfcommPacket response;
    while (stream->Read(&response)) {
        LOGI("Received: %s",response.payload().data());
        break;
    }

    stream->WritesDone();
    grpc::Status status = stream->Finish();

    LOGI("invoke stream finish");

    return 0;
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_kotlinx_grpcjniclient_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";

    test();

    return env->NewStringUTF(hello.c_str());
}