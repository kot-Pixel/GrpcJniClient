#include "CarplayBtRfcommClient.h"

class CarplayRpcClient {
public:
    CarplayRpcClient(const OnMessageCallback& rfcomm) {
        rfcommClient_.StartStream(rfcomm);
    }

    ~CarplayRpcClient() {
        rfcommClient_.Stop();
    }

    bool sendRfcommData(std::string rfcommData) {
        return rfcommClient_.SendPayload(rfcommData);
    }

private:
    CarplayBtRfcommClient rfcommClient_;
};
