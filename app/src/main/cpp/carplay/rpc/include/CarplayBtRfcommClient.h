#include "grpcpp/grpcpp.h"
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include "rfcomm.protoc.grpc.pb.h"
#include "CarplayNativeLogger.h"
#include "JniClassLoaderHelper.h"

#define CARPLAY_BT_RFCOMM_MANAGER_INSTANCE "INSTANCE"
#define CARPLAY_BT_RFCOMM_MANAGER "com/kotlinx/grpcjniclient/bt/BluetoothRfcommManager"
#define CARPLAY_BT_RFCOMM_MANAGER_INSTANCE_SIG "Lcom/kotlinx/grpcjniclient/bt/BluetoothRfcommManager;"

#define CARPLAY_BT_RFCOMM_MANAGER_INSTANCE_CALL_BACK_METHOD_NAME "callbackWithByteArray"
#define CARPLAY_BT_RFCOMM_MANAGER_INSTANCE_CALL_BACK_METHOD_SIG "(Ljava/nio/ByteBuffer;)V"

class CarplayBtRfcommClient {
public:
    explicit CarplayBtRfcommClient(const std::string& address = "unix-abstract:carplay_bt")
            : running_(false), attachJvm_(false) {
        channel_ = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
        stub_ = carplay::bt::CarplayBtService::NewStub(channel_);
    }

    ~CarplayBtRfcommClient() {
        stopStream();
    }

    bool startStream() {
        if (running_) return false;
        context_ = std::make_unique<grpc::ClientContext>();
        stream_ = stub_->RfcommTransport(context_.get());
        running_ = true;

        recv_thread_ = std::thread([this]() {
            JNIEnv* env = nullptr;
            JavaVM *jvm_ = JniClassLoaderHelper::instance().getJvm();
            if(jvm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
                if (jvm_->AttachCurrentThread(&env, nullptr) == JNI_OK) {
                    attachJvm_ = true;
                    bool methodJvmResult = initJavaVmEnvSetCbMethodId(env);
                    if (!methodJvmResult) {
                        return;
                    }
                    LOGD("attached to jvm and set java method callback");
                } else {
                    attachJvm_ = false;
                    LOGE("failed to attach thread to jvm");
                    return;
                }
            } else {
                LOGD("current thread attached to jvm");
            }

            carplay::bt::RfcommPacket resp;
            while (running_ && stream_->Read(&resp)) {
                size_t size = resp.payload().size();
                if (javaCbObject_ != nullptr && javaCallbackMethod_ != nullptr) {
                    jobject stream = env->NewDirectByteBuffer((void*)resp.payload().data(), size);
                    env->CallVoidMethod(javaCbObject_, javaCallbackMethod_, stream);
                    env->DeleteLocalRef(stream);
                } else {
                    LOGE("not call back to java, object or method is null");
                }
            }
            running_ = false;
            if (attachJvm_) {
                jvm_->DetachCurrentThread();
                attachJvm_ = false;
            }
            LOGD("rfcomm stream thread shut down, and detach to jvm");
            if (javaCbObject_!= nullptr) {
                env->DeleteGlobalRef(javaCbObject_);
                javaCbObject_ = nullptr;
            }
            return;
        });
        return true;
    }

    bool sendPayload(const std::string& payload, carplay::bt::RfcommTransportDirection dir = carplay::bt::IN) {
        if (!stream_) return false;

        carplay::bt::RfcommPacket pkt;
        pkt.set_direction(dir);
        pkt.set_payload(payload);

        return stream_->Write(pkt);
    }

    void stopStream() {
        if (!running_) return;

        stream_->WritesDone();
        grpc::Status status = stream_->Finish();
        LOGI("stream closed. Status: %s", status.ok() ? "OK" : status.error_message().c_str());

        running_ = false;

        if (recv_thread_.joinable()) {
            recv_thread_.join();
        }
    }

private:
    jobject javaCbObject_;
    jmethodID javaCallbackMethod_;

    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<carplay::bt::CarplayBtService::Stub> stub_;
    std::unique_ptr<grpc::ClientContext> context_;
    std::shared_ptr<grpc::ClientReaderWriter<carplay::bt::RfcommPacket, carplay::bt::RfcommPacket>> stream_;

    std::thread recv_thread_;
    std::atomic<bool> running_;
    std::atomic<bool> attachJvm_;

    bool initJavaVmEnvSetCbMethodId(JNIEnv* env) {
        jclass clazz = JniClassLoaderHelper::instance().loadClass(env, CARPLAY_BT_RFCOMM_MANAGER);
        if (clazz == nullptr) {
            LOGE("failed to load class");
            return false;
        }
        jfieldID instanceField = env->GetStaticFieldID(clazz,
           CARPLAY_BT_RFCOMM_MANAGER_INSTANCE,
           CARPLAY_BT_RFCOMM_MANAGER_INSTANCE_SIG);
        if (instanceField == nullptr) {
            LOGE("failed to get INSTANCE field");
            return false;
        }
        jobject instance = env->GetStaticObjectField(clazz, instanceField);
        javaCbObject_ = env->NewGlobalRef(instance);
        if (javaCbObject_ == nullptr) {
            LOGE("failed to get java callback object");
            return false;
        }
        javaCallbackMethod_ = env->GetMethodID(clazz,
           CARPLAY_BT_RFCOMM_MANAGER_INSTANCE_CALL_BACK_METHOD_NAME,
           CARPLAY_BT_RFCOMM_MANAGER_INSTANCE_CALL_BACK_METHOD_SIG);
        if (javaCallbackMethod_ == nullptr) {
            LOGD("failed to get method ID");
            return false;
        }
        return true;
    }
};