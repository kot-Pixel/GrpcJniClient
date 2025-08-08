#include "JniClassLoaderHelper.h"
#include "CarplayNativeLogger.h"

#define DEFAULT_HELPER_CLASS_LOADER "com/kotlinx/grpcjniclient/rpc/JniClassLoader"
#define DEFAULT_HELPER_CLASS_METHOD_NAME "getAppClassLoader"
#define DEFAULT_HELPER_CLASS_METHOD_SIG "()Ljava/lang/ClassLoader;"

JniClassLoaderHelper& JniClassLoaderHelper::instance() {
    static JniClassLoaderHelper inst;
    return inst;
}

void JniClassLoaderHelper::initialize(JavaVM* jvm, JNIEnv* env) {
    std::lock_guard<std::mutex> lock(mutex_);
    jvm_ = jvm;

    jclass helperClass = env->FindClass(DEFAULT_HELPER_CLASS_LOADER);
    if (!helperClass) {
        LOGE("JniClassLoaderHelper: Cannot find helper class: %s", DEFAULT_HELPER_CLASS_LOADER);
        return;
    }

    jmethodID getClassLoaderMethod = env->GetStaticMethodID(helperClass, DEFAULT_HELPER_CLASS_METHOD_NAME, DEFAULT_HELPER_CLASS_METHOD_SIG);
    if (!getClassLoaderMethod) {
        LOGE("JniClassLoaderHelper: Cannot find getAppClassLoader");
        return;
    }

    jobject localLoader = env->CallStaticObjectMethod(helperClass, getClassLoaderMethod);
    if (!localLoader) {
        LOGE("JniClassLoaderHelper: getAppClassLoader returned null");
        return;
    }

    classLoader_ = env->NewGlobalRef(localLoader);

    jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
    loadClassMethod_ = env->GetMethodID(classLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    LOGD("app class loader cache success....");
}

jclass JniClassLoaderHelper::loadClass(JNIEnv* env, const char* className) {
    if (!classLoader_ || !loadClassMethod_) {
        LOGE("JniClassLoaderHelper: classLoader_ or loadClassMethod_ is null");
        return nullptr;
    }
    jstring strClassName = env->NewStringUTF(className);
    jobject clazzObj = env->CallObjectMethod(classLoader_, loadClassMethod_, strClassName);
    env->DeleteLocalRef(strClassName);
    return static_cast<jclass>(clazzObj);
}

void JniClassLoaderHelper::withEnv(std::function<void(JNIEnv*)> fn) {
    JNIEnv* env = nullptr;
    bool needDetach = false;

    if (jvm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        if (jvm_->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            LOGE("Failed to attach thread to JVM");
            return;
        }
        needDetach = true;
    }

    fn(env);

    if (needDetach) {
        jvm_->DetachCurrentThread();
    }
}

void JniClassLoaderHelper::callStaticVoidMethod(JNIEnv* env, const char* className, const char* methodName, const char* sig, ...) {
    jclass clazz = loadClass(env, className);
    jmethodID methodID = env->GetStaticMethodID(clazz, methodName, sig);

    va_list args;
    va_start(args, sig);
    env->CallStaticVoidMethodV(clazz, methodID, args);
    va_end(args);
}

jobject JniClassLoaderHelper::callStaticObjectMethod(JNIEnv* env, const char* className, const char* methodName, const char* sig, ...) {
    jclass clazz = loadClass(env, className);
    jmethodID methodID = env->GetStaticMethodID(clazz, methodName, sig);

    va_list args;
    va_start(args, sig);
    jobject result = env->CallStaticObjectMethodV(clazz, methodID, args);
    va_end(args);
    return result;
}

jobject JniClassLoaderHelper::callObjectMethod(JNIEnv* env, jobject obj, const char* methodName, const char* sig, ...) {
    jclass clazz = env->GetObjectClass(obj);
    jmethodID methodID = env->GetMethodID(clazz, methodName, sig);

    va_list args;
    va_start(args, sig);
    jobject result = env->CallObjectMethodV(obj, methodID, args);
    va_end(args);
    return result;
}

jobject JniClassLoaderHelper::getStaticFieldObject(JNIEnv* env, const char* className, const char* fieldName, const char* sig) {
    jclass clazz = loadClass(env, className);
    jfieldID fieldID = env->GetStaticFieldID(clazz, fieldName, sig);
    return env->GetStaticObjectField(clazz, fieldID);
}

jobject JniClassLoaderHelper::getFieldObject(JNIEnv* env, jobject obj, const char* fieldName, const char* sig) {
    jclass clazz = env->GetObjectClass(obj);
    jfieldID fieldID = env->GetFieldID(clazz, fieldName, sig);
    return env->GetObjectField(obj, fieldID);
}

void JniClassLoaderHelper::setFieldObject(JNIEnv* env, jobject obj, const char* fieldName, const char* sig, jobject value) {
    jclass clazz = env->GetObjectClass(obj);
    jfieldID fieldID = env->GetFieldID(clazz, fieldName, sig);
    env->SetObjectField(obj, fieldID, value);
}

