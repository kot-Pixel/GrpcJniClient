#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "message.pb.h"
#include "ipc.pb.h"

#define UUID_BUFFER_LEN 128

static pthread_once_t seed_once = PTHREAD_ONCE_INIT;

static void init_rand_seed() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand(ts.tv_nsec ^ ts.tv_sec ^ getpid());
}

static void* zSocketIpcClientCxt;

static void* zSocketClient;

void initClientSocketIpcEnv();

void connectServer();

void generate_uuid(const char*, char*, size_t);