//
// Created by Tom on 2025/10/30.
//

#ifndef GRPCJNICLIENT_NETLINKMONITOR_H
#define GRPCJNICLIENT_NETLINKMONITOR_H

#include <jni.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <atomic>
#include <string>
#include <net/if.h>
#include <utility>
#include "JniClassLoaderHelper.h"
#include "CarplayNativeLogger.h"

#define IF_NAMESIZE 32

class NetLinkMonitor {
public:
    NetLinkMonitor(JNIEnv *env, jobject obj)
            : running_(false), sock_(-1), jvm_(nullptr), callbackObj_(nullptr) {
        callbackObj_ = env->NewGlobalRef(obj);
    }

    ~NetLinkMonitor() {
        stop();
        if (callbackObj_) {
            JniClassLoaderHelper::instance().withEnv([&](JNIEnv *env) {
                env->DeleteGlobalRef(callbackObj_);
                callbackObj_ = nullptr;
            });
        }
    }

    void start() {
        if (running_) return;
        running_ = true;

        sock_ = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
        sockaddr_nl sa{};
        sa.nl_family = AF_NETLINK;
        sa.nl_groups = RTMGRP_IPV6_IFADDR;
        bind(sock_, (sockaddr *) &sa, sizeof(sa));

        thread_ = std::thread([this]() { threadLoop(); });
    }

    void stop() {
        if (!running_) return;
        running_ = false;
        if (sock_ >= 0) shutdown(sock_, SHUT_RD);  // 解除阻塞 recv
        if (thread_.joinable()) thread_.join();
        if (sock_ >= 0) close(sock_);
        sock_ = -1;
    }

private:
    void threadLoop() {
        char buf[4096];
        while (running_) {
            ssize_t len = recv(sock_, buf, sizeof(buf), 0);
            if (len <= 0) continue;

            for (auto *nh = (nlmsghdr *) buf; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len)) {
                if (nh->nlmsg_type != RTM_NEWADDR) continue;

                auto *ifa = (ifaddrmsg *) NLMSG_DATA(nh);
                if (ifa->ifa_family != AF_INET6) continue;

                char ifname[IF_NAMESIZE] = {0};
                if_indextoname(ifa->ifa_index, ifname);

                rtattr *rta = IFA_RTA(ifa);
                int rta_len = IFA_PAYLOAD(nh);
                for (; RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
                    if (rta->rta_type != IFA_ADDRESS) continue;

                    char addr_str[INET6_ADDRSTRLEN] = {0};
                    inet_ntop(AF_INET6, RTA_DATA(rta), addr_str, sizeof(addr_str));

                    LOGD("IPv6 address detected: iface=%s, addr=%s", ifname, addr_str);

                    JniClassLoaderHelper::instance().withEnv(
                            [&, ifname = std::string(ifname), addr = std::string(addr_str)](
                                    JNIEnv *env) {

                                jstring jIface = env->NewStringUTF(ifname.c_str());
                                jstring jAddr = env->NewStringUTF(addr.c_str());

                                JniClassLoaderHelper::instance().callVoidMethod(env,
                                                                                callbackObj_,
                                                                                "onIpv6Ready",
                                                                                "(Ljava/lang/String;Ljava/lang/String;)V",
                                                                                jIface, jAddr);
                                env->DeleteLocalRef(jIface);
                                env->DeleteLocalRef(jAddr);
                            });
                }
            }
        }
    }

private:
    std::atomic<bool> running_;
    int sock_;
    std::thread thread_;
    JavaVM *jvm_;
    jobject callbackObj_;
};

static NetLinkMonitor *gMonitor = nullptr;

#endif //GRPCJNICLIENT_NETLINKMONITOR_H
