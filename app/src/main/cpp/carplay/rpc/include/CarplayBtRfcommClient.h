#include "grpcpp/grpcpp.h"
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include "rfcomm.protoc.grpc.pb.h"
#include "CarplayNativeLogger.h"

#define CARPLAY_BT_RFCOMM_MANAGER "com/kotlinx/grpcjniclient/bt/BluetoothRfcommManager"
#define CARPLAY_BT_RFCOMM_MANAGER_INSTANCE "INSTANCE"
#define CARPLAY_BT_RFCOMM_MANAGER_INSTANCE_SIG "Lcom/kotlinx/grpcjniclient/bt/BluetoothRfcommManager;"

#define CARPLAY_BT_RFCOMM_MANAGER_INSTANCE_CALL_BACK_METHOD_NAME "callbackWithByteArray"
#define CARPLAY_BT_RFCOMM_MANAGER_INSTANCE_CALL_BACK_METHOD_SIG "([B)V"

using OnMessageCallback = std::function<void(const carplay::bt::RfcommPacket&)>;

class CarplayBtRfcommClient {
public:

    explicit CarplayBtRfcommClient(JavaVM* jvm, const std::string& address = "unix-abstract:carplay_bt")
            : jvm_(jvm), running_(false), attachJvm_(false) {
        channel_ = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
        stub_ = carplay::bt::CarplayBtService::NewStub(channel_);
    }

    ~CarplayBtRfcommClient() {
        stopStream();
    }

    bool startStream() {
        if (running_) return false;
        if (jvm_ == nullptr) return false;

        context_ = std::make_unique<grpc::ClientContext>();
        stream_ = stub_->RfcommTransport(context_.get());

        running_ = true;

        recv_thread_ = std::thread([this]() {
            JNIEnv* env = nullptr;
            attachJvm_ = false;

            //rfcomm stream attach java vm thread
            if(jvm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
                LOGD("current thread not attach to JVM");
                if (jvm_->AttachCurrentThread(&env, nullptr) == JNI_OK) {
                    attachJvm_ = true;
                    LOGD("attached to JVM");
                    bool result = initJavaVmEnvSetCbMethodId(env);
                    if (!result) {
                        return false;
                    }
                } else {
                    LOGE("Failed to attach thread to JVM");
                    return false;
                }
            } else {
                LOGD("current thread attached to JVM");
            }

            carplay::bt::RfcommPacket resp;
            while (running_ && stream_->Read(&resp)) {
                size_t size = resp.payload().size();
                if (javaCbObject_ != nullptr && javaCallbackMethod_ != nullptr) {
                    jobject stream = env->NewDirectByteBuffer((void*)resp.payload().data(), size);
                    env->CallVoidMethod(javaCbObject_, javaCallbackMethod_, stream);
                    env->DeleteLocalRef(stream);
                }
            }
            running_ = false;

            if (attachJvm_) {
                jvm_->DetachCurrentThread();
                attachJvm_ = false;
            }
            if (javaCbObject_!= nullptr) {
                env->DeleteGlobalRef(javaCbObject_);
                javaCbObject_ = nullptr;
            }
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
        LOGI("Stream closed. Status: %s", status.ok() ? "OK" : status.error_message().c_str());

        running_ = false;

        if (recv_thread_.joinable()) {
            recv_thread_.join();
        }
    }

private:
    JavaVM* jvm_;
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
        if(env != nullptr) {
            jclass clazz = env->FindClass("com/kotlinx/grpcjniclient/bt/BluetoothRfcommManager");

            if(clazz == nullptr) {
                LOGE("Failed to find java object class");
                return false;
            }

            jfieldID instanceField = env->GetStaticFieldID(clazz,
               "INSTANCE",
               CARPLAY_BT_RFCOMM_MANAGER_INSTANCE_SIG);
            if (instanceField == nullptr) {
                LOGE("Failed to get INSTANCE field");
                env->DeleteLocalRef(clazz);
                return false;
            }

            jobject instance = env->GetStaticObjectField(clazz, instanceField);
            javaCbObject_ = env->NewGlobalRef(instance);

            if (javaCbObject_ == nullptr) {
                LOGE("Failed to get java callback object");
                env->DeleteLocalRef(instance);
                env->DeleteLocalRef(clazz);
            }

            LOGD("success get global call back ref");

            javaCallbackMethod_ = env->GetMethodID(clazz,
               CARPLAY_BT_RFCOMM_MANAGER_INSTANCE_CALL_BACK_METHOD_NAME,
               CARPLAY_BT_RFCOMM_MANAGER_INSTANCE_CALL_BACK_METHOD_SIG);

            LOGD("success get method id");

            if (javaCallbackMethod_ == nullptr) {
                LOGE("Failed to get method ID");
                env->DeleteLocalRef(instance);
                env->DeleteLocalRef(clazz);
                return false;
            }

            env->DeleteLocalRef(clazz);
            env->DeleteLocalRef(instance);

            return true;
        } else {
            return false;
        }
    }
};