#include <thread>
#include <jni.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "CarplayRpcRuntime.h"
#include "bluetooth_generated.h"
#include "request_param_generated.h"
#include "response_param_generated.h"
#include "JniClassLoaderHelper.h"

#include "CarplayBtRpcReceive.cpp"
#include "carplay_generated.h"
#include "wifi_generated.h"

#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <android/surface_texture.h>
#include <android/surface_texture_jni.h>
#include <media/NdkMediaFormat.h>
#include <android/looper.h>


void CarplayRpcRuntime::initCarplayRpcRuntime() {

    LOGD("initCarplayRpcRuntime start");

    dial.set_pipe_handler(default_pipe_handler);
    dial.setRpcDebugMode(true);

    dialStarted = dial.start();
}

bool CarplayRpcRuntime::checkPeerRpcDialAvailable() const {
    return dialStarted;
}

void CarplayRpcRuntime::rpcRemoteCall(const std::string &method, const void *req_buf, size_t len,
                                      std::unique_ptr<uint8_t[]> &resp_buf,
                                      size_t &resp_size) {
    bool success = dial.call_remote(method, req_buf, len, resp_buf, resp_size);
    LOGD("rpcRemoteCall name is: %s , result is :%d", method.c_str(), success);
}


void* outputThreadFunc(void* arg) {
    auto* codec = (AMediaCodec*)arg;
    AMediaCodecBufferInfo info;

    while (true) {
        ssize_t status = AMediaCodec_dequeueOutputBuffer(codec, &info, 1000000); // 1s
        LOGD("Got decoded buffer index %zd", status);
        if (status >= 0) {
            AMediaCodec_releaseOutputBuffer(codec, status, true);

            if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
                LOGD("Decoder EOS");
                break;
            }
        } else if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            AMediaFormat* newFormat = AMediaCodec_getOutputFormat(codec);
            LOGD("Output format changed: %s", AMediaFormat_toString(newFormat));
            AMediaFormat_delete(newFormat);

        } else if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
            LOGD("No output buffer, try again later");

        } else {
            LOGE("Unexpected dequeueOutputBuffer status: %zd", status);
        }
    }
    return nullptr;
}


void* inputThreadFunc(void* arg) {

    return nullptr;
}
// 全局 codec 指针
//AMediaCodec* kScreenStreamMediaCodec = nullptr;

// -------------------- 回调函数 --------------------
extern "C" void kMediaCodecInputBufAvailable(AMediaCodec *codec, void *userdata, int32_t index) {
    LOGD("onAsyncInputAvailable index = %d", index);

//    // 投喂 SPS/PPS + IDR 示例
//    static const uint8_t sps[] = {0x27,0x64,0x00,0x2a,0xac,0x13,0x14,0x50,0x1e,0x01,0xe6,0x9b,0x80,0x86,0x83,0x03,0x68,0x22,0x11,0x96};
//    static const uint8_t pps[] = {0x28,0xee,0x06,0xf2};
//
//    size_t inputSize;
//    uint8_t* buffer = AMediaCodec_getInputBuffer(codec, index, &inputSize);
//    if (!buffer) return;
//
//    // 简单拼接 SPS + PPS
//    size_t totalSize = sizeof(sps) + sizeof(pps);
//    if (totalSize > inputSize) return;
//    memcpy(buffer, sps, sizeof(sps));
//    memcpy(buffer + sizeof(sps), pps, sizeof(pps));
//
//    media_status_t status = AMediaCodec_queueInputBuffer(
//            codec, index, 0, totalSize, 0, AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG);
//    if (status != AMEDIA_OK) {
//        LOGE("queueInputBuffer failed: %d", status);
//    } else {
//        LOGD("queueInputBuffer success");
//    }
}


void CarplayRpcRuntime::outputLoop() {
    LOGD("outputLoop start......, %d", kScreenStreamMediaCodecStatus.load());

    AMediaCodecBufferInfo info;
    while (kScreenStreamMediaCodecStatus >= MediaCodecStatus::STARTED) {

        LOGD("start media codec output thread loop");

        if (kScreenStreamMediaCodec == nullptr) break;

        ssize_t outputIndex = AMediaCodec_dequeueOutputBuffer(kScreenStreamMediaCodec, &info, -1);

        LOGD("outputIndex index is %zd......", outputIndex);

        if (outputIndex >= 0) {
            media_status_t releaseOutputBufferResult = AMediaCodec_releaseOutputBuffer(kScreenStreamMediaCodec, outputIndex, true);
            LOGD("Output buffer dequeued, size= %d, result =%d", info.size, releaseOutputBufferResult);
        } else if (outputIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            AMediaFormat* newFormat = AMediaCodec_getOutputFormat(kScreenStreamMediaCodec);
            LOGD("Output format changed: %s", AMediaFormat_toString(newFormat));
            AMediaFormat_delete(newFormat);
            notifyScreenAvailable();
        } else if (outputIndex == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    LOGD("outputLoop stop......");
}

void CarplayRpcRuntime::screenLooper() {
    LOGD("screenLooper start......");
    initEGL();

    createOESTexture();

    oesRenderer.init();

    initMediaCodec();

    gRenderQuit = false;

    while (!gRenderQuit.load()) {
        std::unique_lock<std::mutex> lk(gFrameMutex);
        gFrameCond.wait(lk, [this] { return gFrameAvailable || gRenderQuit.load(); });
        if (gRenderQuit.load()) break;
        gFrameAvailable = false;
        lk.unlock();
        LOGD("screenLooper need update surfaceTexture......");
        float texMatrix[16];
        if (mSurfaceTexture) {
            int updateRet = ASurfaceTexture_updateTexImage(mSurfaceTexture);
            if (updateRet == 0) {
                ASurfaceTexture_getTransformMatrix(mSurfaceTexture, texMatrix);

                LOGD("updateTexImage success");

                LOGD("texMatrix: [%.2f, %.2f, %.2f, %.2f,  %.2f, %.2f, %.2f, %.2f,  %.2f, %.2f, %.2f, %.2f,  %.2f, %.2f, %.2f, %.2f]",
                     texMatrix[0], texMatrix[1], texMatrix[2], texMatrix[3],
                     texMatrix[4], texMatrix[5], texMatrix[6], texMatrix[7],
                     texMatrix[8], texMatrix[9], texMatrix[10], texMatrix[11],
                     texMatrix[12], texMatrix[13], texMatrix[14], texMatrix[15]);
            } else {
                LOGE("updateTexImage failed: %d (no new frame? check MediaCodec output)", updateRet);
            }
        }

        if (mAttachedNativeWindowUpdated) {
            std::lock_guard<std::mutex> wlk(mAttachNativeWindowMutex);
            if (mAttachEGLSurface != EGL_NO_SURFACE) {
                eglDestroySurface(gDisplay, mAttachEGLSurface);
            }
            EGLint attrs[] = {EGL_NONE};
            mAttachEGLSurface = eglCreateWindowSurface(gDisplay, gConfig, mAttachNativeWindow, attrs);
            eglMakeCurrent(gDisplay, mAttachEGLSurface, mAttachEGLSurface, gContext);
            mAttachedNativeWindowUpdated = false;
        }

        if (mAttachNativeWindow != EGL_NO_SURFACE) {
            oesRenderer.draw(gOesTexture, texMatrix, mAttachNativeWindowWidth, mAttachNativeWindowHeight);
            EGLBoolean swapBufferResult = eglSwapBuffers(gDisplay, mAttachEGLSurface);
            LOGD("swapBufferResult is %d", swapBufferResult);
        }

    }
}

bool CarplayRpcRuntime::initPullListener(const std::string& url) {
    if (pullListener) {
        LOGE("pullListener already initialized");
        return true;
    }

    pullListener = std::make_unique<NngUdsRpcPullListener>();
    if (!pullListener->init(url.c_str())) {
        LOGE("pullListener init failure");
        pullListener.reset();
        return false;
    }

    LOGE("pullListener init success");

    pullListener->startRecvLoop([this](const uint8_t* data, size_t len) {
        while(true) {
            bool ok = this->queueInputBuffer(data, len, 0, 0);
            LOGE("queueInputBuffer len is %zu, result = %d, data[0] = %d, data[1] = %d, data[2] = %d, data[3] = %d, data[4] = %d", len, ok, data[0], data[1], data[2], data[3], data[4]);
            if (ok) {
                break;
            } else {
                LOGE("no available input buffer index");
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
    });

    return true;
}

void CarplayRpcRuntime::shutdownPullListener() {
    if (pullListener) {
        pullListener->close();
        pullListener.reset();
        LOGE("pullListener shutdown");
    }
}

bool CarplayRpcRuntime::queueInputBuffer(const uint8_t* data, size_t size, int64_t pts, uint32_t flags) {
    if (!kScreenStreamMediaCodec) {
        LOGE("queueInputBuffer kScreenStreamMediaCodec is null");
        return false;
    }

    if (kScreenStreamMediaCodecStatus < MediaCodecStatus::STARTED) {
        LOGE("queueInputBuffer kScreenStreamMediaCodec status not started");
        return false;
    }

    ssize_t inputIndex = AMediaCodec_dequeueInputBuffer(kScreenStreamMediaCodec, -1);
    if (inputIndex < 0) {
        LOGE("queueInputBuffer inputIndex < 0");
        return false;
    }

    size_t bufferSize = 0;
    uint8_t* buffer = AMediaCodec_getInputBuffer(kScreenStreamMediaCodec, inputIndex, &bufferSize);
    if (!buffer || bufferSize < size) {
        LOGE("queueInputBuffer buffer size is small or input buffer is null ptr < 0");
        return false;
    }

    memcpy(buffer, data, size);
    media_status_t status = AMediaCodec_queueInputBuffer(kScreenStreamMediaCodec, inputIndex, 0, size, pts, flags);
    return status == AMEDIA_OK;
}

bool CarplayRpcRuntime::initMediaCodec() {
    JniClassLoaderHelper::instance().withEnv([&](JNIEnv *env) {

        EGLContext currentContext = eglGetCurrentContext();
        LOGI("Current EGLContext: %p", currentContext);

        jobject surfaceTextureObj = JniClassLoaderHelper::instance().callStaticObjectMethod(
                env,
                "com/kotlinx/grpcjniclient/screen/CarplayScreenStub",
                "createOesSurfaceTexture",
                "(I)Landroid/graphics/SurfaceTexture;",
                gOesTexture
        );

        if (surfaceTextureObj != nullptr) {
            mSurfaceTexture = ASurfaceTexture_fromSurfaceTexture(env, surfaceTextureObj);
        } else {
            LOGI("jni load surface texture failure");
            return false;
        }

        if (mSurfaceTexture == nullptr) {
            LOGI("surface texture create failure");
            return false;
        }

        int detachResult =  ASurfaceTexture_detachFromGLContext(mSurfaceTexture);
        LOGD("ASurfaceTexture_detachFromGLContext result is %d", detachResult);

        int attachResult = ASurfaceTexture_attachToGLContext(mSurfaceTexture, gOesTexture);
        LOGD("ASurfaceTexture_attachToGLContext result is %d", attachResult);

        gWindow = ASurfaceTexture_acquireANativeWindow(mSurfaceTexture);

        if (!gWindow) {
            LOGE("Failed to create ANativeWindow");
            return false;
        }

        LOGI("success to create ANativeWindow");

        ANativeWindow_acquire(gWindow);

        kScreenStreamMediaCodec = AMediaCodec_createDecoderByType("video/avc");

        if (!kScreenStreamMediaCodec) {
            LOGE("Failed to create codec");
            return false;
        }

        AMediaFormat* format = AMediaFormat_new();
        AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, 1872);
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, 756);

        media_status_t status =  AMediaCodec_configure(kScreenStreamMediaCodec, format, gWindow, nullptr, 0);
        AMediaFormat_delete(format);

        if (status != AMEDIA_OK) {
            LOGE("AMediaCodec_configure failed: %d", status);
            return false;
        }

        LOGD("AMediaCodec_configure success: %d", status);

        status = AMediaCodec_start(kScreenStreamMediaCodec);
        if (status != AMEDIA_OK) {
            LOGE("AMediaCodec_start failed: %d", status);
            return false;
        }

        LOGD("AMediaCodec_start success: %d", status);

        kScreenStreamMediaCodecStatus = MediaCodecStatus::STARTED;

        LOGD("media codec initialized");

        mOutputThread = std::thread(&CarplayRpcRuntime::outputLoop, this);
        mOutputThread.detach();

        return true;
    });

    return false;
}

void CarplayRpcRuntime::stopMediaCodec() {
    kScreenStreamMediaCodecStatus = MediaCodecStatus::STOPPED;
    if (kScreenStreamMediaCodec) {
        AMediaCodec_stop(kScreenStreamMediaCodec);
        AMediaCodec_delete(kScreenStreamMediaCodec);
        kScreenStreamMediaCodec = nullptr;
    }
}

bool CarplayRpcRuntime::initEGL() {
    gDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (gDisplay == EGL_NO_DISPLAY) {
        LOGE("Failed to get EGL display");
        return false;
    }

    LOGD("Success to get EGL display");

    if (!eglInitialize(gDisplay, nullptr, nullptr)) {
        LOGE("Failed to initialize EGL");
        return false;
    }

    LOGD("Success to initialize EGL");

    EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT | EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                        EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE};
    EGLint numConfigs;
    if (!eglChooseConfig(gDisplay, attribs, &gConfig, 1, &numConfigs)) {
        LOGE("Failed to choose EGL config");
        return false;
    }

    LOGD("Success to choose EGL config");

    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    gContext = eglCreateContext(gDisplay, gConfig, EGL_NO_CONTEXT, contextAttribs);
    if (gContext == EGL_NO_CONTEXT) {
        LOGE("Failed to create EGL context");
        return false;
    }

    LOGD("Success to create EGL context");

    EGLint pbufferAttribs[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
    gSurface = eglCreatePbufferSurface(gDisplay, gConfig, pbufferAttribs);
    if (gSurface == EGL_NO_SURFACE) {
        LOGE("Failed to create Pbuffer surface");
        return false;
    }

    LOGD("Success to create Pbuffer surface");

    if (!eglMakeCurrent(gDisplay, gSurface, gSurface, gContext)) {
        LOGE("Failed to make EGL context current");
        return false;
    }

    LOGD("make EGL context current success");

    char name[32];
    pthread_getname_np(pthread_self(), name, sizeof(name));
    LOGD("EGL context link to current thread name is: %s", name);

    return true;
}

void CarplayRpcRuntime::initOpenGL() {
//    glGenFramebuffers(1, &gFbo);
//    glBindFramebuffer(GL_FRAMEBUFFER, gFbo);
//    glGenTextures(1, &gRenderTexture);
//    glBindTexture(GL_TEXTURE_2D, gRenderTexture);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1920, 1080, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gRenderTexture, 0);
//
//    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//        LOGE("FBO incomplete: %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
//    }
//    if (!initShaders()) {
//        LOGE("Shader initialization failed");
//    } else {
//        LOGD("Shader initialization success");
//    }
//
//    LOGD("FBO complete....openGL has bind texture...");
}

void CarplayRpcRuntime::startScreenStreamThread() {
    mVideoStreamThread = std::thread(&CarplayRpcRuntime::screenLooper, this);
    mVideoStreamThread.detach();
}

void CarplayRpcRuntime::createOESTexture() {
    glGenTextures(1, &gOesTexture);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, gOesTexture);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    LOGI("OES Texture created: %u", gOesTexture);

    if (gOesTexture == 0) {
        LOGE("OES texture ID invalid");
    } else {
        LOGI("OES texture ID=%u", gOesTexture);
    }
}

void CarplayRpcRuntime::surfaceAvailable(jobject surface) {

    std::lock_guard<std::mutex> lk(mAttachNativeWindowMutex);

    JniClassLoaderHelper::instance().withEnv([&](JNIEnv *env) {
        if (surface) {
            mAttachNativeWindow = ANativeWindow_fromSurface(env, surface);
            mAttachNativeWindowWidth = ANativeWindow_getWidth(mAttachNativeWindow);
            mAttachNativeWindowHeight = ANativeWindow_getHeight(mAttachNativeWindow);
            ANativeWindow_acquire(mAttachNativeWindow);
            LOGI("mAttachedNativeWindowUpdated mAttachNativeWindow %p", mAttachNativeWindow);
            LOGI("mAttachedNativeWindowUpdated mAttachNativeWindowWidth %d", mAttachNativeWindowWidth);
            LOGI("mAttachedNativeWindowUpdated mAttachNativeWindowHeight %d", mAttachNativeWindowHeight);
        }
    });

    LOGI("mAttachedNativeWindowUpdated invoke");

    mAttachedNativeWindowUpdated = true;
}

void CarplayRpcRuntime::notifyScreenAvailable() {
    JniClassLoaderHelper::instance().withEnv([&](JNIEnv *env) {
        JniClassLoaderHelper::instance().callVoidMethod(env,
                                                        gRuntime->getJavaObject(),"mediaCodecFormatChanged",
                                                        "()V");
    });
}

JNIEXPORT jint JNI_OnLoad(JavaVM *jvm, void *reserved) {
    JNIEnv *env = nullptr;
    if (jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    JniClassLoaderHelper::instance().initialize(jvm, env);

    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_kotlinx_grpcjniclient_screen_CarplayScreenStub_notifyFrameAvailable(JNIEnv *env,
                                                                             jobject thiz) {
    if (gRuntime != nullptr) {
        {
            std::lock_guard<std::mutex> lk(gRuntime->gFrameMutex);
            gRuntime->gFrameAvailable = true;
        }
        gRuntime->gFrameCond.notify_one();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_kotlinx_grpcjniclient_screen_CarplayScreenStub_notifySurfaceAvailable(JNIEnv *env,
                                                                               jobject thiz,
                                                                               jobject surface) {
    if (gRuntime != nullptr) {
        gRuntime->surfaceAvailable(surface);
    }
}

flatbuffers::Offset<response::VoidResponse> handleSendIap2DetectPacket(
        const bluetooth::BtRfcommData *req,
        flatbuffers::FlatBufferBuilder &fbb) {
    auto payload = req->data();
    auto len = payload->size();

    if (len == 0) {
        LOGE("Payload is empty");
        return response::CreateVoidResponse(fbb);
    }

    LOGI("len size is %d", len);

    JniClassLoaderHelper::instance().withEnv([&](JNIEnv *env) {
        jobject byteBufferObj = env->NewDirectByteBuffer((void *) payload->data(), len);
        if (!byteBufferObj) {
            LOGE("Failed to create ByteBuffer");
            return;
        }

        JniClassLoaderHelper::instance().callVoidMethod(env,
                                                          gRuntime->getJavaObject(),"callbackWithByteArray",
                                                              "(Ljava/nio/ByteBuffer;)V",
                                                              byteBufferObj);
        env->DeleteLocalRef(byteBufferObj);
    });

    return response::CreateVoidResponse(fbb);
}


flatbuffers::Offset<response::VoidResponse> handleCarplayAvailablePacket(
        const carplay::CarPlayAvailability *req,
        flatbuffers::FlatBufferBuilder &fbb) {
    JniClassLoaderHelper::instance().withEnv([&](JNIEnv *env) {

        jstring usbStr = env->NewStringUTF(req->usb_transport_identifier()->c_str());
        jstring btStr = env->NewStringUTF(req->bluetooth_transport_identifier()->c_str());

        JniClassLoaderHelper::instance().callVoidMethod(env, gRuntime->getJavaObject(),
                                                        "callbackCarplayAvailable",
                                                        "(ZLjava/lang/String;ZLjava/lang/String;)V",
                                                        (jboolean) req->wired_available(),
                                                        usbStr,
                                                        (jboolean) req->wireless_available(),
                                                        btStr
        );
        env->DeleteLocalRef(usbStr);
        env->DeleteLocalRef(btStr);
    });

    return response::CreateVoidResponse(fbb);
}

flatbuffers::Offset<response::VoidResponse> handleScreenStreamStart(
        const request::VoidRequest *req,
        flatbuffers::FlatBufferBuilder &fbb) {

    LOGE("handleScreenStreamStart invoke");

    gRuntime->startScreenStreamThread();

    return response::CreateVoidResponse(fbb);
}

flatbuffers::Offset<response::VoidResponse> handleScreenStreamStop(
        const request::VoidRequest *req,
        flatbuffers::FlatBufferBuilder &fbb) {

//    auto &rpcRuntime = CarplayRpcRuntime::instance();

    gRuntime->stopMediaCodec();

    return response::CreateVoidResponse(fbb);
}

flatbuffers::Offset<response::VoidResponse> handleScreenStreamConfiguration(
        const request::BytesRequest *req,
        flatbuffers::FlatBufferBuilder &fbb) {

    auto data_vec = req->value();
    const uint8_t* data_ptr = data_vec->data();
    size_t data_len = data_vec->size();
    while(true) {
        bool spsPps = gRuntime->queueInputBuffer(data_ptr, data_len, 0, AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG);
        LOGE("sps pps configure info send mediaCodec result is %d", spsPps);
        if (spsPps) {
            gRuntime->kScreenStreamMediaCodecStatus.store(MediaCodecStatus::AVAILABLE);
            break;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

    return response::CreateVoidResponse(fbb);
}

flatbuffers::Offset<response::VoidResponse> handleDisableBluetooth(
        const request::StringRequest *req,
        flatbuffers::FlatBufferBuilder &fbb) {
    JniClassLoaderHelper::instance().withEnv([&](JNIEnv *env) {

        const char* addr_cstr = req->value()->c_str();
        jstring j_addr = env->NewStringUTF(addr_cstr);

        JniClassLoaderHelper::instance().callVoidMethod(env,
                                                        gRuntime->getJavaObject(),
                                                        "disableBluetooth",
                                                        "(Ljava/lang/String;)V",
                                                        j_addr);
    });

    return response::CreateVoidResponse(fbb);
}


/**
 * init carplay rpc framework
 */
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRpcManager_initCarplayRpc(JNIEnv *env, jobject thiz) {

    LOGD("init jni carplay rpc");
    if (!gRuntime) gRuntime = new CarplayRpcRuntime(env, thiz);
    gRuntime->initCarplayRpcRuntime();

    gRuntime->resigteRpcMethod<bluetooth::BtRfcommData, response::VoidResponse>(
            RECEIVE_BT_IAP2_DETECT_HANDLE_FUNCTION_NAME, handleSendIap2DetectPacket);

    gRuntime->resigteRpcMethod<carplay::CarPlayAvailability, response::VoidResponse>(
            RECEIVE_CARPLAY_AVAILABLE_HANDLE_FUNCTION_NAME, handleCarplayAvailablePacket);

    gRuntime->resigteRpcMethod<request::StringRequest, response::VoidResponse>(
            RECEIVE_DIS_ABlE_BLUETOOTH_HANDLE_FUNCTION_NAME, handleDisableBluetooth);

    return gRuntime->checkPeerRpcDialAvailable();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRpcManager_startCarplaySession(JNIEnv *env, jobject thiz,
                                                                         jstring hostapd_ssid,
                                                                         jstring hostapd_pwd,
                                                                         jint hostapd_channel,
                                                                         jstring hostapd_net_interface_v6_address,
                                                                         jint hostapd_security_type) {
    if (gRuntime->checkPeerRpcDialAvailable()) {

        if (!gRuntime->initPullListener("abstract://carplay.video.rpc")) {
            LOGE("initPullListener result false");
            return JNI_FALSE;
        }
        LOGE("initPullListener result success");

        gRuntime->resigteRpcMethod<request::VoidRequest, response::VoidResponse>(
            CALL_VIDEO_STREAM_START_NAME, handleScreenStreamStart);
        gRuntime->resigteRpcMethod<request::VoidRequest, response::VoidResponse>(
            CALL_VIDEO_STREAM_STOP_NAME, handleScreenStreamStop);
        gRuntime->resigteRpcMethod<request::BytesRequest, response::VoidResponse>(
            CALL_VIDEO_STREAM_CONFIGURE_NAME, handleScreenStreamConfiguration);

        const char *ssid_str = env->GetStringUTFChars(hostapd_ssid, nullptr);
        const char *pwd_str = env->GetStringUTFChars(hostapd_pwd, nullptr);
        const char *ipv6_str = env->GetStringUTFChars(hostapd_net_interface_v6_address, nullptr);

        flatbuffers::FlatBufferBuilder builder;

        auto ssid_fb = builder.CreateString(ssid_str);
        auto pwd_fb = builder.CreateString(pwd_str);
        auto ipv6_fb = builder.CreateString(ipv6_str);

        auto req = wifi::CreateHostapdInfo(
                builder,
                ssid_fb,
                pwd_fb,
                hostapd_channel,
                ipv6_fb,
                hostapd_security_type
        );

        env->ReleaseStringUTFChars(hostapd_ssid, ssid_str);
        env->ReleaseStringUTFChars(hostapd_pwd, pwd_str);
        env->ReleaseStringUTFChars(hostapd_net_interface_v6_address, ipv6_str);

        builder.Finish(req);

        const void *req_buf = builder.GetBufferPointer();
        size_t req_size = builder.GetSize();
        std::unique_ptr<uint8_t[]> resp_buf = nullptr;
        size_t resp_len = 0;

        gRuntime->rpcRemoteCall(
                CALL_START_WIRELESS_CARPLAY_SESSION_FUNCTION_NAME,
                req_buf,
                req_size,
                resp_buf,
                resp_len
        );

        return JNI_TRUE;
    } else {
        return JNI_FALSE;
    }
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRpcManager_startBtIap2Link(JNIEnv *env, jobject thiz,
                                                                     jstring mac_string) {

    if (gRuntime->checkPeerRpcDialAvailable()) {

        const char* chars = env->GetStringUTFChars(mac_string, nullptr);
        std::string str(chars);
        env->ReleaseStringUTFChars(mac_string, chars);

        flatbuffers::FlatBufferBuilder builder;
        auto req = request::CreateStringRequest(builder, builder.CreateString(str));
        builder.Finish(req);
        const void *req_buf = builder.GetBufferPointer();
        size_t req_size = builder.GetSize();
        std::unique_ptr<uint8_t[]> resp_buf = nullptr;
        size_t resp_len = 0;
        gRuntime->rpcRemoteCall(CALL_BT_IAP2_START_LINK_FUNCTION_NAME, req_buf, req_size, resp_buf,resp_len);
        return JNI_TRUE;
    } else {
        return JNI_FALSE;
    }
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRpcManager_receiveBtIap2Data(JNIEnv *env, jobject thiz,
                                                                       jbyteArray rfcomm_data,
                                                                       jint data_length) {
    if (gRuntime->checkPeerRpcDialAvailable()) {
        std::unique_ptr<uint8_t[]> resp_buffer = nullptr;
        size_t resp_length;

        jbyte *native_data = env->GetByteArrayElements(rfcomm_data, nullptr);
        if (native_data == nullptr) {
            return JNI_FALSE;
        }

        std::vector<uint8_t> payload_data(reinterpret_cast<uint8_t *>(native_data),
                                          reinterpret_cast<uint8_t *>(native_data + data_length));
        env->ReleaseByteArrayElements(rfcomm_data, native_data, JNI_ABORT);

        flatbuffers::FlatBufferBuilder builder;

        auto payload = builder.CreateVector(payload_data);

        bluetooth::BtRfcommDataBuilder req_builder(builder);
        req_builder.add_data(payload);
        auto req_data = req_builder.Finish();

        builder.Finish(req_data);

        const void *req_buf = builder.GetBufferPointer();

        size_t req_size = builder.GetSize();

        gRuntime->rpcRemoteCall(CALL_RECEVIE_BT_IAP2_RFCOMM_DATA_FUNCTION_NAME, req_buf, req_size, resp_buffer,resp_length);

        return JNI_TRUE;
    } else {
        return JNI_FALSE;
    }
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRpcManager_destroyCarplayRpc(JNIEnv *env, jobject thiz) {

}


extern "C"
JNIEXPORT void JNICALL
Java_com_kotlinx_grpcjniclient_screen_CarplayScreenStub_notifySurfaceUnavailable(JNIEnv *env,
                                                                                 jobject thiz) {
    if (gRuntime != nullptr) {
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_kotlinx_grpcjniclient_screen_CarplayScreenStub_notifySurfaceInputEvent(JNIEnv *env,
                                                                                jobject thiz) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRpcManager_touchScreenHid(JNIEnv *env, jobject thiz, jlong
                                                                    hidValue) {
    if (gRuntime->checkPeerRpcDialAvailable()) {
        int64_t value = static_cast<int64_t>(hidValue);
        flatbuffers::FlatBufferBuilder builder;
        auto req = request::CreateInt64Request(builder, value);
        builder.Finish(req);
        const void *req_buf = builder.GetBufferPointer();
        size_t req_size = builder.GetSize();
        std::unique_ptr<uint8_t[]> resp_buf = nullptr;
        size_t resp_len = 0;
        gRuntime->rpcRemoteCall(CALL_TOUCH_SCREEN_HID_FUNCTION_NAME, req_buf, req_size, resp_buf,resp_len);
    }
}