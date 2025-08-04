#pragma once

#include <mutex>
#include <unordered_map>
#include "ZSoctetIpcHandler.hpp"

#ifdef __cplusplus
extern "C" {
#endif

bool ipc_dispatch(const char* method,
                  const void* req_buf, int req_len,
                  void* resp_buf, int* resp_len, int max_resp_len);

#ifdef __cplusplus
}
#endif

bool ipc_register(const std::string& method, IpcHandler handler);

class IpcDispatcher {
public:
    bool register_method(const std::string& method, IpcHandler handler) {
        return handlers_.emplace(method, std::move(handler)).second;
    }

    bool dispatch(const std::string& method,
                  const std::string& req_payload,
                  std::string& resp_payload) const {
        auto it = handlers_.find(method);
        if (it == handlers_.end()) return false;
        return it->second(req_payload, resp_payload);
    }

private:
    std::unordered_map<std::string, IpcHandler> handlers_;
};

