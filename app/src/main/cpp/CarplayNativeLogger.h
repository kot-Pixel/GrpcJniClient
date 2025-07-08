#ifndef GRPCJNISERVER_CARPLAYNATIVELOGGER_H
#define GRPCJNISERVER_CARPLAYNATIVELOGGER_H

#endif //GRPCJNISERVER_CARPLAYNATIVELOGGER_H

#include <android/log.h>  // Android 日志头文件

#define LOG_TAG "GrpcClient"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
