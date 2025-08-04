// #include "zmq.h"
#include <thread>

// #define QUEUE_SIZE 10

// typedef void (*ZmqMessageCallback)(void* data, size_t size, void (*release_fn)(void*));

// typedef struct {
//     zmq_msg_t identity;
//     zmq_msg_t msg;
// } task_t;


// typedef struct {
//     pthread_t thread;
//     void* zSocketIpcCxt;
//     void* proxySocket;

//     int index;
//     ZmqMessageCallback msgCallBack;
// } IpcBinderWorker;

// void* socketWorkerReceiveThread(void* arg);

// class ZUdsRpcWorker {
// public:
//     ZUdsRpcWorker(void* zmqCtx);
//     void startZUdsWork();
// private:
//     void* mUdsSocketZmqCtx;
//     void* mUdsServerProxySocket;
//     uint8_t mIndex;
//     std::thread work_thread;
// };


