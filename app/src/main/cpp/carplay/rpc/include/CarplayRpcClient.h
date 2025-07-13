#include <jni.h>
#include "CarplayBtRfcommClient.h"

class CarplayRpcClient {
public:
    explicit CarplayRpcClient() = default;

    ~CarplayRpcClient() {
        rfcommClient_->stopStream();
    }

    bool sendRfcommData(const std::string& rfcommData) {
        return rfcommClient_->sendPayload(rfcommData);
    }

    bool initCarplayBtRfcommClient() {
        rfcommClient_ = new CarplayBtRfcommClient();
        return rfcommClient_->startStream();
    }

private:
    // carplay rfcomm rpc transform socket client
    CarplayBtRfcommClient *rfcommClient_;
};
