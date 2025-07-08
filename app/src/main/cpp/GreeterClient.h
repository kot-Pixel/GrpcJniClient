//
// Created by Tom on 2025/7/7.
//
#ifndef GRPCJNICLIENT_GREETERCLIENT_H
#define GRPCJNICLIENT_GREETERCLIENT_H

#endif //GRPCJNICLIENT_GREETERCLIENT_H

#include "grpc/include/grpcpp/channel.h"
#include "grpc/include/grpcpp/client_context.h"
#include "grpc/include/grpcpp/create_channel.h"
#include "helloworld.protoc.pb.h"
#include "helloworld.protoc.grpc.pb.h"
#include "grpc/include/grpcpp/impl/status.h"
#include "grpc/include/grpcpp/security/credentials.h"

#include <android/log.h>  // Android 日志头文件

#define LOG_TAG "GrpcClient"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using grpc::Channel;
using grpc::ClientContext;
using grpc::CreateChannel;
using grpc::Status;

class GreeterClient {
public:
    GreeterClient(std::shared_ptr<Channel> channel)
            : stub_(Greeter::NewStub(channel)) {}

    // 单向RPC调用
    std::string SayHello(const std::string& name) {
        HelloRequest request;
        request.set_name(name);

        HelloReply reply;
        ClientContext context;


        grpc::Status status = stub_->SayHello(&context, request, &reply);

        if (status.ok()) {
            return reply.message();
        } else {
            return "RPC failed: " + std::to_string(status.error_code()) +
                   ", " + status.error_message();
        }
    }

private:
    std::unique_ptr<Greeter::Stub> stub_;
};
