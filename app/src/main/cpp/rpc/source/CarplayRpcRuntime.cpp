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

    LOGD("outputLoop start......");

    AMediaCodecBufferInfo info;
    while (mRunning.load()) {
        if (!kScreenStreamMediaCodec) break;

        ssize_t outputIndex = AMediaCodec_dequeueOutputBuffer(kScreenStreamMediaCodec, &info, 1000000);

        LOGD("outputIndex index is %zd......", outputIndex);

        if (outputIndex >= 0) {
            AMediaCodec_releaseOutputBuffer(kScreenStreamMediaCodec, outputIndex, false);
            if (info.flags & AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG) continue;
            LOGD("Output buffer dequeued, size=%d, flags=%d", info.size, info.flags);
        } else if (outputIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            AMediaFormat* newFormat = AMediaCodec_getOutputFormat(kScreenStreamMediaCodec);
            LOGD("Output format changed: %s", AMediaFormat_toString(newFormat));
            AMediaFormat_delete(newFormat);
        } else if (outputIndex == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
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
        LOGE("pullListener recv len=%zu", len);
        // 这里直接用 this 来调用成员函数，比如写到 MediaCodec
        // bool ok = this->queueInputBuffer(data, len, 0, 0);
        // LOGE("queueInputBuffer result = %d", ok);
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
    if (!kScreenStreamMediaCodec) return false;

    ssize_t inputIndex = AMediaCodec_dequeueInputBuffer(kScreenStreamMediaCodec, -1);
    if (inputIndex < 0) return false;

    size_t bufferSize = 0;
    uint8_t* buffer = AMediaCodec_getInputBuffer(kScreenStreamMediaCodec, inputIndex, &bufferSize);
    if (!buffer || bufferSize < size) return false;

    memcpy(buffer, data, size);
    media_status_t status = AMediaCodec_queueInputBuffer(kScreenStreamMediaCodec, inputIndex, 0, size, pts, flags);
    return status == AMEDIA_OK;
}


bool CarplayRpcRuntime::initMediaCodec() {
    JniClassLoaderHelper::instance().withEnv([&](JNIEnv *env) {

        kScreenStreamMediaCodec = AMediaCodec_createDecoderByType("video/avc");

        if (!kScreenStreamMediaCodec) {
            LOGE("Failed to create codec");
            return false;
        }

        AMediaFormat* format = AMediaFormat_new();
        AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, 1920);
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, 1080);

        media_status_t status =  AMediaCodec_configure(kScreenStreamMediaCodec, format, nullptr, nullptr, 0);
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

        // 启动输出线程
        mRunning.store(true);
        mOutputThread = std::thread(&CarplayRpcRuntime::outputLoop, this);
        mOutputThread.detach();

        return true;
    });

    return false;
}

void CarplayRpcRuntime::stopMediaCodec() {
    mRunning.store(false);
    if (kScreenStreamMediaCodec) {
        AMediaCodec_stop(kScreenStreamMediaCodec);
        AMediaCodec_delete(kScreenStreamMediaCodec);
        kScreenStreamMediaCodec = nullptr;
    }
}

bool CarplayRpcRuntime::initMediaCodec2(jobject surface) {
    if (surface == nullptr) {
        LOGE("failed to create stub surface");
        return false;
    }

    JniClassLoaderHelper::instance().withEnv([&](JNIEnv *env) {
        ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
        if (!window) {
            LOGE("ANativeWindow_fromSurface failed");
            return false;
        }

        kScreenStreamMediaCodec = AMediaCodec_createDecoderByType("video/avc");
        if (!kScreenStreamMediaCodec) {
            LOGE("Failed to create codec");
            return false;
        }

        AMediaFormat *format = AMediaFormat_new();
        AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, 1920);
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, 1080);;

        media_status_t status = AMediaCodec_configure(
                kScreenStreamMediaCodec, format, window, nullptr, 0);
        AMediaFormat_delete(format);

        if (status != AMEDIA_OK) {
            LOGE("AMediaCodec_configure failed: %d", status);
            return false;
        }

        status = AMediaCodec_start(kScreenStreamMediaCodec);
        if (status != AMEDIA_OK) {
            LOGE("AMediaCodec_start failed: %d", status);
            return false;
        }

        kScreenStreamMediaCodecStatus = MediaCodecStatus::STARTED;
        LOGD("media codec initialized");

        return true;
    });

    return true;
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
    rpcRuntime.initCarplayRpcRuntime();
    rpcRuntime.createNativeStreamSocket("nativeStreamSocket");
    rpcRuntime.initMediaCodec();
    return rpcRuntime.checkPeerRpcDialAvailable();
}