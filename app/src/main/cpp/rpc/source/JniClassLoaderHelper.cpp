////#include "JniClassLoaderHelper.h"
////#include "CarplayNativeLogger.h"
//
//#define DEFAULT_HELPER_CLASS_LOADER "com/kotlinx/grpcjniclient/rpc/JniClassLoader"
//#define DEFAULT_HELPER_CLASS_METHOD_NAME "getAppClassLoader"
//#define DEFAULT_HELPER_CLASS_METHOD_SIG "()Ljava/lang/ClassLoader;"
//
//JniClassLoaderHelper& JniClassLoaderHelper::instance() {
//    static JniClassLoaderHelper inst;
//    return inst;
//}
//
//void JniClassLoaderHelper::initialize(JavaVM* jvm, JNIEnv* env) {
//    std::lock_guard<std::mutex> lock(mutex_);
//    jvm_ = jvm;
//
//    jclass helperClass = env->FindClass(DEFAULT_HELPER_CLASS_LOADER);
//    if (!helperClass) {
//        LOGE("JniClassLoaderHelper: Cannot find helper class: %s", DEFAULT_HELPER_CLASS_LOADER);
//        return;
//    }
//
//    jmethodID getClassLoaderMethod = env->GetStaticMethodID(helperClass, DEFAULT_HELPER_CLASS_METHOD_NAME, DEFAULT_HELPER_CLASS_METHOD_SIG);
//    if (!getClassLoaderMethod) {
//        LOGE("JniClassLoaderHelper: Cannot find getAppClassLoader");
//        return;
//    }
//
//    jobject localLoader = env->CallStaticObjectMethod(helperClass, getClassLoaderMethod);
//    if (!localLoader) {
//        LOGE("JniClassLoaderHelper: getAppClassLoader returned null");
//        return;
//    }
//
//    classLoader_ = env->NewGlobalRef(localLoader);
//
//    jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
//    loadClassMethod_ = env->GetMethodID(classLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
//
//    LOGD("app class loader cache success....");
//}
//
//jclass JniClassLoaderHelper::loadClass(JNIEnv* env, const char* className) {
//    if (!classLoader_ || !loadClassMethod_) {
//        LOGE("JniClassLoaderHelper: classLoader_ or loadClassMethod_ is null");
//        return nullptr;
//    }
//    jstring strClassName = env->NewStringUTF(className);
//    jobject clazzObj = env->CallObjectMethod(classLoader_, loadClassMethod_, strClassName);
//    env->DeleteLocalRef(strClassName);
//    return static_cast<jclass>(clazzObj);
//}
