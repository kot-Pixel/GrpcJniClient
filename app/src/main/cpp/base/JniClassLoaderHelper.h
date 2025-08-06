#ifndef JNI_CLASS_LOADER_HELPER_H
#define JNI_CLASS_LOADER_HELPER_H

#include <jni.h>
#include <string>
#include <mutex>

class JniClassLoaderHelper {
public:
    static JniClassLoaderHelper& instance();

    void initialize(JavaVM* jvm, JNIEnv* env);
    jclass loadClass(JNIEnv* env, const char* className);

    JavaVM* getJvm() const {
        return jvm_;
    }

private:
    JniClassLoaderHelper() = default;
    ~JniClassLoaderHelper() = default;

    JniClassLoaderHelper(const JniClassLoaderHelper&) = delete;
    JniClassLoaderHelper& operator=(const JniClassLoaderHelper&) = delete;

    JavaVM* jvm_ = nullptr;
    jobject classLoader_ = nullptr;
    jmethodID loadClassMethod_ = nullptr;
    std::mutex mutex_;
};

#endif // JNI_CLASS_LOADER_HELPER_H
