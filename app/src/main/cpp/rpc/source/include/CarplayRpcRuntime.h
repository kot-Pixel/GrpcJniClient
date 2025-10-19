//
// Created by Tom on 2025/8/4.
//

#ifndef GRPCJNICLIENT_CARPLAYRPCRUNTIME_H
#define GRPCJNICLIENT_CARPLAYRPCRUNTIME_H

#include <thread>
#include "NngUdsRpcPeerDial.hpp"
#include "CarplayNativeLogger.h"
#include "NngUdsRpcPullListener.hpp"
#include "OesRenderer.h"

#include <media/NdkMediaCodec.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/surface_texture.h>
#include <jni.h>

enum class MediaCodecStatus : int {
    IDLE = 0,
    STARTED,
    AVAILABLE,
    STOPPED
};

struct NativeDecoderSurface {
    GLuint oesTex = 0;
    ASurfaceTexture* st = nullptr;
    ANativeWindow* window = nullptr;
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
    void configureMediaCodec();

    bool initPullListener(const std::string& url);
    void shutdownPullListener();

    bool initEGL();

    void initOpenGL();

    bool initMediaCodec(jobject surface);

    std::thread mVideoStreamThread;

    void startScreenStreamThread();

    std::mutex gFrameMutex;
    std::condition_variable gFrameCond;
    bool gFrameAvailable = false;
    std::atomic<bool> gRenderQuit = false;
    std::atomic<MediaCodecStatus> kScreenStreamMediaCodecStatus = MediaCodecStatus::IDLE;

    ANativeWindow* mAttachNativeWindow;
    int mAttachNativeWindowWidth;
    int mAttachNativeWindowHeight;
    EGLSurface mAttachEGLSurface = EGL_NO_SURFACE;
    std::mutex mAttachNativeWindowMutex;
    bool mAttachedNativeWindowUpdated = false;

    void surfaceAvailable(jobject pJobject);

private:

    void postRunRpcDialThread();

    NngUdsRpcPeerDial dial;
    std::thread worker_thread_;
    bool dialStarted = false;
    bool dialRunning = false;

    int streamSocketFd = -1;

    AMediaCodec* kScreenStreamMediaCodec;


    std::thread mOutputThread;

    void outputLoop();

    std::unique_ptr<NngUdsRpcPullListener> pullListener;

    GLuint gOesTexture = 0;
    EGLDisplay gDisplay = EGL_NO_DISPLAY;
    EGLSurface gSurface = EGL_NO_SURFACE;
    EGLContext gContext = EGL_NO_CONTEXT;
    EGLConfig gConfig; // 保存 EGL 配置

    ANativeWindow* gWindow;
    ASurfaceTexture* mSurfaceTexture;
    OesRenderer oesRenderer;

    void screenLooper();
    void createOESTexture();
};

#endif //GRPCJNICLIENT_CARPLAYRPCRUNTIME_H
