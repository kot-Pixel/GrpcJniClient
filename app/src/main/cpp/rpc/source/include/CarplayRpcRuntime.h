//
// Created by Tom on 2025/8/4.
//

#ifndef GRPCJNICLIENT_CARPLAYRPCRUNTIME_H
#define GRPCJNICLIENT_CARPLAYRPCRUNTIME_H

#include <thread>
#include "NngUdsRpcPeerDial.hpp"
#include "CarplayNativeLogger.h"

class CarplayRpcRuntime {
public:
    CarplayRpcRuntime() = default;


    ~CarplayRpcRuntime() {

    }

    void initCarplayRpcRuntime();

    bool checkPeerRpcDialAvailable();

    void rpcRemoteCall(const std::string &method, const void *req_buf, size_t len, uint8_t* &resp_buf, size_t* resp_size);
private:

    void postRunRpcDialThread();

    NngUdsRpcPeerDial dial;
    std::thread worker_thread_;
    bool dialStarted = false;
    bool dialRunning = false;
};

#endif //GRPCJNICLIENT_CARPLAYRPCRUNTIME_H
