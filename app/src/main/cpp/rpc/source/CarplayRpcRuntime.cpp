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

#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <android/surface_texture.h>
#include <media/NdkMediaFormat.h>
#include <android/looper.h>


void CarplayRpcRuntime::initCarplayRpcRuntime() {

    LOGD("initCarplayRpcRuntime start");

    dialStarted = dial.start();

    if (dialStarted) {
        worker_thread_ = std::thread(&CarplayRpcRuntime::postRunRpcDialThread, this);
    }
}

bool CarplayRpcRuntime::checkPeerRpcDialAvailable() const {
    return dialStarted && dialRunning && streamSocketFd > 0;
}

void CarplayRpcRuntime::postRunRpcDialThread() {
    if (dialStarted) {
        dialRunning = true;
        dial.run_forever();
    }

    LOGD("postRunRpcDialThread run forever");
}


void CarplayRpcRuntime::rpcRemoteCall(const std::string &method, const void *req_buf, size_t len,
                                      uint8_t *&resp_buf, size_t *resp_size) {
    bool success = dial.call_remote(method, req_buf, len, resp_buf, resp_size);
    LOGD("rpcRemoteCall name is: %s , result is :%d", method.c_str(), success);
}

bool CarplayRpcRuntime::createNativeStreamSocket(const std::string &name) {
    streamSocketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (streamSocketFd < 0) {
        LOGD("createNativeStreamSocket create failure");
        return false;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    if (name.size() > sizeof(addr.sun_path) - 1)

    addr.sun_path[0] = '\0';
    memcpy(addr.sun_path + 1, name.c_str(), name.size());

    if (bind(streamSocketFd, (struct sockaddr*)&addr, sizeof(sa_family_t) + 1 + name.size()) < 0) {
        LOGD("createNativeStreamSocket bind failure");
        return false;
    }

    if (listen(streamSocketFd, 5) < 0) {
        LOGD("createNativeStreamSocket listen failure");
        return false;
    }

    LOGD("createNativeStreamSocket init success");
    return true;
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

    initMediaCodec();

    gRenderQuit = false;

    while (!gRenderQuit.load()) {
        std::unique_lock<std::mutex> lk(gFrameMutex);
        gFrameCond.wait(lk, [this] { return gFrameAvailable || gRenderQuit.load(); });
        if (gRenderQuit.load()) break;
        gFrameAvailable = false;
        lk.unlock();
        LOGD("screenLooper need update surfaceTexture......");
        eglMakeCurrent(gDisplay, gSurface, gSurface, gContext);

        JniClassLoaderHelper::instance().withEnv([&](JNIEnv *env) {

            JniClassLoaderHelper::instance().callStaticVoidMethod(env,
                                                                  "com/kotlinx/grpcjniclient/screen/CarplayScreenStub",
                                                                  "updateSurfaceTexture",
                                                                  "()V");
        });

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
         bool ok = this->queueInputBuffer(data, len, 0, 0);
         LOGE("queueInputBuffer result = %d", ok);
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

        jobject stubWindow = JniClassLoaderHelper::instance().callStaticObjectMethod(
                env,
                "com/kotlinx/grpcjniclient/screen/CarplayScreenStub",
                "createOesSurfaceTexture",
                "(I)Landroid/view/Surface;",
                gRenderTexture
        );

        if (stubWindow != nullptr) {
            gWindow = ANativeWindow_fromSurface(env, stubWindow);
        }

        LOGI("success to create ANativeWindow");

        if (!gWindow) {
            LOGE("Failed to create ANativeWindow");
            return false;
        }
        ANativeWindow_acquire(gWindow);

        kScreenStreamMediaCodec = AMediaCodec_createDecoderByType("video/avc");

        if (!kScreenStreamMediaCodec) {
            LOGE("Failed to create codec");
            return false;
        }

        AMediaFormat* format = AMediaFormat_new();
        AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, 1920);
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, 1080);
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, 0x7F000789);

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
//    mRunning.store(false);
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


const char* vertexShaderSource = R"(
    attribute vec4 aPosition;
    attribute vec2 aTexCoord;
    varying vec2 vTexCoord;
    void main() {
        gl_Position = aPosition;
        vTexCoord = aTexCoord;
    }
)";

const char* fragmentShaderSourceOES = R"(
    #extension GL_OES_EGL_image_external : require
    precision mediump float;
    uniform samplerExternalOES uTexture;
    varying vec2 vTexCoord;
    void main() {
        gl_FragColor = texture2D(uTexture, vTexCoord);
    }
)";

const char* fragmentShaderSource2D = R"(
    precision mediump float;
    uniform sampler2D uTexture;
    varying vec2 vTexCoord;
    void main() {
        gl_FragColor = texture2D(uTexture, vTexCoord);
    }
)";

bool CarplayRpcRuntime::initShaders() {
    // OES 着色器（用于 SurfaceTexture 纹理）
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    GLint status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (!status) {
        LOGE("Vertex shader compilation failed");
        return false;
    }

    GLuint fragmentShaderOES = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderOES, 1, &fragmentShaderSourceOES, nullptr);
    glCompileShader(fragmentShaderOES);
    glGetShaderiv(fragmentShaderOES, GL_COMPILE_STATUS, &status);
    if (!status) {
        LOGE("OES fragment shader compilation failed");
        return false;
    }

    gProgramOES = glCreateProgram();
    glAttachShader(gProgramOES, vertexShader);
    glAttachShader(gProgramOES, fragmentShaderOES);
    glLinkProgram(gProgramOES);
    GLint linked;
    glGetProgramiv(gProgramOES, GL_LINK_STATUS, &linked);
    if (!linked) {
        LOGE("OES shader link failed");
        return false;
    }

    // 2D 着色器（用于 gRenderTexture 渲染到额外 Surface）
    GLuint fragmentShader2D = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader2D, 1, &fragmentShaderSource2D, nullptr);
    glCompileShader(fragmentShader2D);
    glGetShaderiv(fragmentShader2D, GL_COMPILE_STATUS, &status);
    if (!status) {
        LOGE("2D fragment shader compilation failed");
        return false;
    }

    gProgram2D = glCreateProgram();
    glAttachShader(gProgram2D, vertexShader);
    glAttachShader(gProgram2D, fragmentShader2D);
    glLinkProgram(gProgram2D);
    glGetProgramiv(gProgram2D, GL_LINK_STATUS, &linked);
    if (!linked) {
        LOGE("2D shader link failed");
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShaderOES);
    glDeleteShader(fragmentShader2D);
    return true;
}

void CarplayRpcRuntime::initOpenGL() {
    glGenFramebuffers(1, &gFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gFbo);
    glGenTextures(1, &gRenderTexture);
    glBindTexture(GL_TEXTURE_2D, gRenderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1920, 1080, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gRenderTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("FBO incomplete: %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
    if (!initShaders()) {
        LOGE("Shader initialization failed");
    } else {
        LOGD("Shader initialization success");
    }

    LOGD("FBO complete....openGL has bind texture...");
}

void CarplayRpcRuntime::startScreenStreamThread() {
    mVideoStreamThread = std::thread(&CarplayRpcRuntime::screenLooper, this);
    mVideoStreamThread.detach();
}

void CarplayRpcRuntime::createOESTexture() {
    glGenTextures(1, &gRenderTexture);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, gRenderTexture );
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    LOGI("OES Texture created: %u", gRenderTexture);

    if (gRenderTexture == 0) {
        LOGE("OES texture ID invalid");
    } else {
        LOGI("OES texture ID=%u", gRenderTexture);
    }
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
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRuntime_initCarplayRpc(JNIEnv *env, jobject thiz) {
    LOGD("init jni carplay rpc");
    auto &rpcRuntime = CarplayRpcRuntime::instance();
    rpcRuntime.initCarplayRpcRuntime();
    rpcRuntime.createNativeStreamSocket("nativeStreamSocket");
    return rpcRuntime.checkPeerRpcDialAvailable();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRuntime_initCarplayRpc22(JNIEnv *env, jobject thiz,
                                                                   jobject surface) {
    LOGD("init jni carplay rpc");
    auto &rpcRuntime = CarplayRpcRuntime::instance();
    return JNI_TRUE;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_kotlinx_grpcjniclient_rpc_CarplayRuntime_initCarplayRpc333(JNIEnv *env, jobject thiz,
                                                                    jobject surface) {
    auto &rpcRuntime = CarplayRpcRuntime::instance();
    rpcRuntime.initEGL();
    rpcRuntime.initOpenGL();
//    rpcRuntime.initMediaCodec(surface);

    return JNI_TRUE;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_kotlinx_grpcjniclient_screen_CarplayScreenStub_notifyFrameAvailable(JNIEnv *env,
                                                                             jobject thiz) {
    auto &rpcRuntime = CarplayRpcRuntime::instance();
    {
        std::lock_guard<std::mutex> lk(rpcRuntime.gFrameMutex);
        rpcRuntime.gFrameAvailable = true;
    }
    rpcRuntime.gFrameCond.notify_one();
}