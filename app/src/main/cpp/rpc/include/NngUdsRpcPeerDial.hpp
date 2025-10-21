#pragma once
#include <nng/nng.h>
#include <utility>
#include <vector>
#include <string>
#include <future>
#include <unordered_map>
#include <android/log.h>
#include "flatbuffers/verifier.h"
#include "rpc_generated.h"

#define PEER_DIAL_LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RpcPeerDial", __VA_ARGS__)
#define PEER_DIAL_LOG_D(...) __android_log_print(ANDROID_LOG_DEBUG, "RpcPeerDial", __VA_ARGS__)
#define PEER_DIAL_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "RpcPeerDial", __VA_ARGS__)

struct NngPeerDialAioWithContext {
    nng_aio *aio = nullptr;
    class NngUdsRpcPeerDial *server = nullptr;
};

class NngUdsRpcPeerDial {
public:
    using HandlerFunc = std::function<void(const void *req_buf, size_t len, std::string &response_out)>;

    std::function<void(nng_pipe, nng_pipe_ev)> on_pipe_event;

    explicit NngUdsRpcPeerDial(std::string addr = "abstract://nng.rpc", const int workers = 16,
                               const bool debugMode = false)
        : address(std::move(addr)), num_workers(workers), sock{}, isDebugMode(debugMode) {
    }

    ~NngUdsRpcPeerDial() {
        stop();
    }

    bool start();

    void stop();

    template<typename RequestT, typename ResponseT>
    void register_method(const std::string &method_name,
                         std::function<flatbuffers::Offset<ResponseT>(const RequestT *,
                                                                      flatbuffers::FlatBufferBuilder &)> handler) {
        HandlerFunc wrapper = [handler](const void *req_buf, const size_t len, std::string &resp_out) {
            const auto *data = static_cast<const uint8_t *>(req_buf);
            if (flatbuffers::Verifier verifier(data, len); !verifier.VerifyBuffer<RequestT>()) {
                PEER_DIAL_LOGE("register_method failure, verifier verify failed");
                return;
            }

            const RequestT *req_obj = flatbuffers::GetRoot<RequestT>(data);
            flatbuffers::FlatBufferBuilder fbb;

            auto resp_offset = handler(req_obj, fbb);
            fbb.Finish(resp_offset);

            resp_out.assign(reinterpret_cast<const char *>(fbb.GetBufferPointer()), fbb.GetSize());
        };

        method_map[method_name] = wrapper;
    }

    void dispatch(const std::string &method_name, const void *req_buf, size_t len, std::string &resp_out);

    /**
     *
     * @param method rpc remote call method name
     * @param req_buf rpc remote call method param
     * @param len rpc remote call method param size
     * @param resp_buf rpc remote call method return param
     * @param resp_size rpc remote call method return size
     * @return
     */
    bool call_remote(const std::string &method, const void *req_buf, size_t len,std::unique_ptr<uint8_t[]> &resp_buf,
                     size_t &resp_size);


    void set_pipe_handler(std::function<void(nng_pipe, nng_pipe_ev)> handler) {
        on_pipe_event = std::move(handler);
        if (isDebugMode) {
            PEER_DIAL_LOG_D("pipe handler updated.");
        }
    }

    void setRpcDebugMode(const bool enableDebug) {
        isDebugMode = enableDebug;
    }

    void dumpRegisteredMethods() const {
        PEER_DIAL_LOG_D("--------------------METHOD START-----------------------");
        for (const auto& pair : method_map) {
            if (isDebugMode) {
                PEER_DIAL_LOG_D("method name %s ", pair.first.data());
            }
        }
        PEER_DIAL_LOG_D("---------------------METHOD END----------------------");
    }

private:
    static void recv_cb_static(void *arg) {
        auto *ctx = static_cast<NngPeerDialAioWithContext *>(arg);
        if (ctx && ctx->server) {
            ctx->server->recv_cb(ctx->aio);
        }
    }

    static void pipe_cb(nng_pipe p, nng_pipe_ev ev, void *arg) {
        auto *self = static_cast<NngUdsRpcPeerDial*>(arg);
        if (self && self->on_pipe_event) {
            self->on_pipe_event(p, ev);
            self->internalNngPipeEvent(p, ev);
        }
    }

    void recv_cb(nng_aio *aio);

    uint32_t generate_request_id();

    std::string address;
    int num_workers;
    nng_socket sock;
    std::vector<NngPeerDialAioWithContext *> aio_list;
    bool isDebugMode;

    std::unordered_map<std::string, HandlerFunc> method_map;

    std::atomic<uint32_t> next_request_id{1};
    std::mutex pending_mutex;
    std::unordered_map<uint32_t, std::promise<std::shared_ptr<std::vector<uint8_t> > > > pending_requests;

    std::atomic<bool> dialRunning {false};

    void internalNngPipeEvent(nng_pipe p, nng_pipe_ev ev);
};
