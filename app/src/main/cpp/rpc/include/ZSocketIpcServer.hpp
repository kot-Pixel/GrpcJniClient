#pragma once

#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "ZSocketIpcWorker.hpp"
#include "ZSocketIpcUtils.hpp"
#include "ZSocketIpcDefine.hpp"
#include "ZSocketIpcDispatcher.hpp"
#include "message.pb.h"

#define WORKER_COUNT 8

typedef struct {
    void* zSocketIpcCxt;
    void* mServerSocket;
    void* mServerProxySocket;

    IpcBinderWorker mServerProxyWorks[WORKER_COUNT];

    IpcDispatcher mIpcDispatcher;
} ZSocketIpcEnv;

static ZSocketIpcEnv g_zsocket_env;

//init a zmq socket ipc server, return zmq socket pointer
void initSocketIpcServer(ZSocketIpcEnv *);

void initSocketIpcDispather();