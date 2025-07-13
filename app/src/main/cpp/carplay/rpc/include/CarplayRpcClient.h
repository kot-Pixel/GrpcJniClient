#include <jni.h>
#include "CarplayBtRfcommClient.h"

class CarplayRpcClient {
public:
    explicit CarplayRpcClient(JavaVM *javaVm) {
        rpcJvm_ = javaVm;
    }

    ~CarplayRpcClient() {
        rfcommClient_->stopStream();
    }

    bool sendRfcommData(const std::string& rfcommData) {
        return rfcommClient_->sendPayload(rfcommData);
    }

    bool judgeJavaVmIsNullptr() {
        return rpcJvm_ != nullptr;
    }

    bool initCarplayBtRfcommClient() {
        if (!judgeJavaVmIsNullptr())
            return false;

        rfcommClient_ = new CarplayBtRfcommClient(rpcJvm_);
        return rfcommClient_->startStream();
    }

private:
    JavaVM* rpcJvm_;

    // carplay rfcomm rpc transform socket client
    CarplayBtRfcommClient *rfcommClient_{};
};
