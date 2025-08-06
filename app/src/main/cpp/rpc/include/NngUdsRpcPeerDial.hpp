#include <nng/nng.h>
#include <string.h>
#include <vector>
#include <string>
#include <mutex>
#include <future>
#include <unordered_map>

#include <nng/protocol/pair0/pair.h>
#include <android/log.h>

#include "flatbuffers/verifier.h"
#include "rpc_generated.h"

#define PROJECT_TAG "Rpc"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, PROJECT_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, PROJECT_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, PROJECT_TAG, __VA_ARGS__)

struct NngPeerDialAioWithContext
{
    nng_aio *aio = nullptr;
    class NngUdsRpcPeerDial *server = nullptr;
};

class NngUdsRpcPeerDial
{
public:

    using HandlerFunc = std::function<void(const void* req_buf, size_t len, std::string& response_out)>;

    NngUdsRpcPeerDial(const std::string &addr = "abstract://nng.rpc", int workers = 16)
        : address(addr), num_workers(workers), sock{} {}

    ~NngUdsRpcPeerDial()
    {
        stop();
    }

    bool start();

    void run_forever();

    void stop();

    template <typename RequestT, typename ResponseT>
    void register_method(const std::string& method_name,
                         std::function<flatbuffers::Offset<ResponseT>(const RequestT*, flatbuffers::FlatBufferBuilder&)> handler)
    {
        HandlerFunc wrapper = [handler](const void* req_buf, size_t len, std::string& resp_out) {
            const uint8_t* data = reinterpret_cast<const uint8_t*>(req_buf);
            flatbuffers::Verifier verifier(data, len);
            if (!verifier.VerifyBuffer<RequestT>()) {
                LOGE("flatbuffer verify failed");
                return;
            }

            const RequestT* req_obj = flatbuffers::GetRoot<RequestT>(data);
            flatbuffers::FlatBufferBuilder fbb;

            auto resp_offset = handler(req_obj, fbb);
            fbb.Finish(resp_offset);

            resp_out.assign(reinterpret_cast<const char*>(fbb.GetBufferPointer()), fbb.GetSize());
        };
    }

    void dispatch(const std::string& method_name, const void* req_buf, size_t len, std::string& resp_out);

    bool call_remote(const std::string &method, const void *req_buf, size_t len, uint8_t* &resp_buf, size_t* resp_size);
private:
    static void recv_cb_static(void *arg)
    {
        auto *ctx = static_cast<NngPeerDialAioWithContext *>(arg);
        if (ctx && ctx->server)
        {
            ctx->server->recv_cb(ctx->aio);
        }
    }

    void recv_cb(nng_aio *aio);
    uint32_t generate_request_id();

private:
    std::string address;
    int num_workers;
    nng_socket sock;
    std::vector<NngPeerDialAioWithContext *> aio_list;

    std::unordered_map<std::string, HandlerFunc> method_map;

    std::atomic<uint32_t> next_request_id{1};
    std::mutex pending_mutex;
    std::unordered_map<uint32_t, std::promise<std::shared_ptr<std::vector<uint8_t>>>> pending_requests;
};