#include "IHandlerImpl.hpp"
#include "ZSocketIpcDefine.hpp"
#include "ZSocketIpcWorker.hpp"


class ZUdsRpcServer {
    public:
    template <typename ReqT, typename ResT>
    void registerHandler(const std::string& method, std::function<ResT(const ReqT&)> handler) {
        m_handlers[method] = std::make_unique<HandlerImpl<ReqT, ResT>>(std::move(handler));
    }

    std::vector<uint8_t> dispatch(const std::string& method, const uint8_t* data, size_t size) {
        auto it = m_handlers.find(method);
        if (it != m_handlers.end()) {
            return it->second->handle(data, size);
        } else {
            return {};
        }
    }

    int initZUdsRpcServer();
private:
    std::unordered_map<std::string, std::unique_ptr<IHandler>> m_handlers;

    // ZUdsRpcWorker *rpcWorker = nullptr;

    void* zUdsSocketZmqCtx;
    void* mUdsServerSocket;
    void* mUdsServerProxySocket;
};