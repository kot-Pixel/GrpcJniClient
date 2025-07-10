#include "grpcpp/grpcpp.h"
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include "rfcomm.protoc.grpc.pb.h"
#include "CarplayNativeLogger.h"

using OnMessageCallback = std::function<void(const carplay::bt::RfcommPacket&)>;

class CarplayBtRfcommClient {
public:

    explicit CarplayBtRfcommClient(const std::string& address = "unix-abstract:carplay_bt")
            : running_(false) {
        channel_ = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
        stub_ = carplay::bt::CarplayBtService::NewStub(channel_);
    }

    bool StartStream(const OnMessageCallback& cb) {
        if (running_) return false;

        context_ = std::make_unique<grpc::ClientContext>();
        stream_ = stub_->RfcommTransport(context_.get());

        running_ = true;

        recv_thread_ = std::thread([this, cb]() {
            carplay::bt::RfcommPacket resp;
            while (running_ && stream_->Read(&resp)) {
                if (cb) cb(resp);
            }
            running_ = false;
        });

        return true;
    }

    bool SendPayload(const std::string& payload, carplay::bt::RfcommTransportDirection dir = carplay::bt::IN) {
        if (!stream_) return false;

        carplay::bt::RfcommPacket pkt;
        pkt.set_direction(dir);
        pkt.set_payload(payload);

        return stream_->Write(pkt);
    }

    void Stop() {
        if (!running_) return;

        stream_->WritesDone();
        grpc::Status status = stream_->Finish();
        LOGI("Stream closed. Status: %s", status.ok() ? "OK" : status.error_message().c_str());

        running_ = false;

        if (recv_thread_.joinable()) {
            recv_thread_.join();
        }
    }

private:
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<carplay::bt::CarplayBtService::Stub> stub_;
    std::unique_ptr<grpc::ClientContext> context_;
    std::shared_ptr<grpc::ClientReaderWriter<carplay::bt::RfcommPacket, carplay::bt::RfcommPacket>> stream_;

    std::thread recv_thread_;
    std::atomic<bool> running_;
};