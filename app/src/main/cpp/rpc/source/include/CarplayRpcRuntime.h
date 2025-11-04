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
#include "JniClassLoaderHelper.h"
#include <media/NdkMediaCodec.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/surface_texture.h>
#include <jni.h>

static jobject runtimeJavaObject;

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

// rpc handle remote call function name define start
#define RECEIVE_BT_IAP2_DETECT_HANDLE_FUNCTION_NAME "sendIap2DetectPacket"
#define RECEIVE_CARPLAY_AVAILABLE_HANDLE_FUNCTION_NAME "carplayAvailable"
#define RECEIVE_DIS_ABlE_BLUETOOTH_HANDLE_FUNCTION_NAME "disableBluetooth"

#define CALL_VIDEO_STREAM_START_NAME "screenStreamStart"
#define CALL_VIDEO_STREAM_STOP_NAME "screenStreamStop"
#define CALL_VIDEO_STREAM_CONFIGURE_NAME "screenStreamConfiguration"
// rpc handle remote call function name define end

// rpc call remote function name define start
#define CALL_BT_IAP2_START_LINK_FUNCTION_NAME "startBtIap2Link"
#define CALL_RECEVIE_BT_IAP2_RFCOMM_DATA_FUNCTION_NAME "receiveRfcommDataFromBt"
#define CALL_START_WIRELESS_CARPLAY_SESSION_FUNCTION_NAME "startWirelessCarplaySession"
#define CALL_TOUCH_SCREEN_HID_FUNCTION_NAME "touchScreenHid"

// rpc call remote function name define end




class CarplayRpcRuntime {
public:

//    static CarplayRpcRuntime& instance() {
//        static CarplayRpcRuntime instance;
//        return instance;
//    }

//    explicit CarplayRpcRuntime() = default;

    explicit CarplayRpcRuntime(JNIEnv* env, jobject thiz) {
        javaObject = env->NewGlobalRef(thiz);
    }

    ~CarplayRpcRuntime() {
        shutdownPullListener();
        if (javaObject) {
            JniClassLoaderHelper::instance().withEnv([&](JNIEnv *env) {
                env->DeleteGlobalRef(javaObject);
                javaObject = nullptr;
            });
        }
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

    void rpcRemoteCall(const std::string &method, const void *req_buf, size_t len,std::unique_ptr<uint8_t[]> &resp_buf,size_t &resp_size);

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


    jobject getJavaObject() const { return javaObject; }

private:
    jobject javaObject = nullptr;

    void postRunRpcDialThread();

    NngUdsRpcPeerDial dial;
    std::thread worker_thread_;
    bool dialStarted = false;
    bool dialRunning = false;

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

    void notifyScreenAvailable();

    static void default_pipe_handler(nng_pipe p, nng_pipe_ev ev) {
        LOGE("default_pipe_handler callback ");
        uint32_t id = nng_pipe_id(p);
        if (ev == NNG_PIPE_EV_REM_POST) {
            LOGE("Default: Pipe REM_POST (ID=%u) - Lost connection", id);

        } else if (ev == NNG_PIPE_EV_ADD_POST) {
            LOGD("Default: Pipe ADD_POST (ID=%u) - Connected", id);
        }
    }
};

static CarplayRpcRuntime* gRuntime = nullptr;

#endif //GRPCJNICLIENT_CARPLAYRPCRUNTIME_H
