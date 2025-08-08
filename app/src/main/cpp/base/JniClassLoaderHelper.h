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

    void withEnv(std::function<void(JNIEnv*)> fn);

    // 调用静态方法（无返回值）
    void callStaticVoidMethod(JNIEnv* env, const char* className, const char* methodName, const char* sig, ...);

    // 调用静态方法（带返回值）
    jobject callStaticObjectMethod(JNIEnv* env, const char* className, const char* methodName, const char* sig, ...);

    // 调用实例方法（带返回值）
    jobject callObjectMethod(JNIEnv* env, jobject obj, const char* methodName, const char* sig, ...);

    // 获取静态字段值
    jobject getStaticFieldObject(JNIEnv* env, const char* className, const char* fieldName, const char* sig);

    // 获取实例字段值
    jobject getFieldObject(JNIEnv* env, jobject obj, const char* fieldName, const char* sig);

    // 设置实例字段值
    void setFieldObject(JNIEnv* env, jobject obj, const char* fieldName, const char* sig, jobject value);


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
