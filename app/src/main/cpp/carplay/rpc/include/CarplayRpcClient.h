#include <jni.h>
#include "CarplayBtRfcommClient.h"

class CarplayRpcClient {
public:
    explicit CarplayRpcClient() {
        rfcommClient_ = new CarplayBtRfcommClient();
    }

    ~CarplayRpcClient() {
        rfcommClient_->stopStream();
    }

    bool sendRfcommData(const std::string& rfcommData) {
        return rfcommClient_->sendPayload(rfcommData);
    }

    bool initCarplayBtRfcommClient() {
        return rfcommClient_->startStream();
    }

    bool startBtRfcommIap2Link() {
        return rfcommClient_->startBtIap2Link();
    }

private:
    // carplay rfcomm rpc transform socket client
    CarplayBtRfcommClient *rfcommClient_;
};
