//
// Created by Tom on 2025/8/4.
//

#ifndef GRPCJNICLIENT_CARPLAYRPCRUNTIME_H
#define GRPCJNICLIENT_CARPLAYRPCRUNTIME_H

#include <thread>
#include "NngUdsRpcPeerDial.hpp"
#include "CarplayNativeLogger.h"
#include "NngUdsRpcPullListener.hpp"

#include <media/NdkMediaCodec.h>
#include <jni.h>

enum class MediaCodecStatus : int {
    IDLE = 0,
    STARTED,
    AVAILABLE,
    STOPPED
};

class CarplayRpcRuntime {
public:

    static CarplayRpcRuntime& instance() {
        static CarplayRpcRuntime instance;
        return instance;
    }

    CarplayRpcRuntime() = default;


    ~CarplayRpcRuntime() {
        shutdownPullListener();
    }

    template <typename RequestT, typename ResponseT>
    bool resigteRpcMethod(
            const std::string& method_name,
            std::function<flatbuffers::Offset<ResponseT>(const RequestT*, flatbuffers::FlatBufferBuilder&)> handler)
    {
        dial.register_method<RequestT, ResponseT>(method_name, handler);
        return true;
    }

    void initCarplayRpcRuntime();

    bool checkPeerRpcDialAvailable() const;

    void rpcRemoteCall(const std::string &method, const void *req_buf, size_t len, uint8_t* &resp_buf, size_t* resp_size);

//    void rpcRemoteSendFileDescriptor(int fd);

    //create native stream socket
    bool createNativeStreamSocket(const std::string& name);

//    int releaseNativeStreamSocket();

    bool initMediaCodec();

    bool queueInputBuffer(const uint8_t* data, size_t size, int64_t pts, uint32_t flags);
    void stopMediaCodec();

    bool initMediaCodec2(jobject);
    void configureMediaCodec();

    bool initPullListener(const std::string& url);
    void shutdownPullListener();

private:

    void postRunRpcDialThread();

    NngUdsRpcPeerDial dial;
    std::thread worker_thread_;
    bool dialStarted = false;
    bool dialRunning = false;

    int streamSocketFd = -1;

    AMediaCodec* kScreenStreamMediaCodec;
    std::atomic<MediaCodecStatus> kScreenStreamMediaCodecStatus = MediaCodecStatus::IDLE;


    std::atomic<bool> mRunning;
    std::thread mOutputThread;

    void outputLoop();

    std::unique_ptr<NngUdsRpcPullListener> pullListener;
};

#endif //GRPCJNICLIENT_CARPLAYRPCRUNTIME_H
