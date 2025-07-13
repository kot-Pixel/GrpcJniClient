package com.kotlinx.grpcjniclient.rpc

/**
 * https://developer.android.com/training/articles/perf-jni?utm_source=chatgpt.com&hl=zh-cn
 * findClass not found.
 * classloader result to.
 * JavaVm create JNIEnv default use system classloader,not associate to app code.so cache a
 * class loader to replace findClass
 */
object JniClassLoader {
    @JvmStatic
    fun getAppClassLoader(): ClassLoader {
        return JniClassLoader::class.java.classLoader!!
    }
}