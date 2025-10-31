#include <jni.h>
#include "NetLinkMonitor.h"

extern "C"
JNIEXPORT void JNICALL
Java_com_kotlinx_grpcjniclient_net_NetService_netLinkStart(JNIEnv *env, jobject thiz) {
    if (gMonitor) return;
    gMonitor = new NetLinkMonitor(env, thiz);
    gMonitor->start();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_kotlinx_grpcjniclient_net_NetService_netLinkStop(JNIEnv *env, jobject thiz) {
    if (!gMonitor) return;
    gMonitor->stop();
    delete gMonitor;
    gMonitor = nullptr;
}