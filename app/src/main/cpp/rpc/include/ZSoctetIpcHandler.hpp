#pragma once

#include <functional>
#include <string>

using IpcHandler = std::function<bool(const std::string& req_payload, std::string& resp_payload)>;

// template remains unchanged:
template <typename Req, typename Resp>
IpcHandler make_ipc_handler(std::function<void(const Req&, Resp&)> func) {

    return [func](const std::string& req_payload, std::string& resp_payload) -> bool {
        Req req;
        if (!req.ParseFromString(req_payload)) {
            return false;
        }

        Resp resp;
        func(req, resp);

        return resp.SerializeToString(&resp_payload);
    };
}
