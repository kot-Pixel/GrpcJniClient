#include <functional>

#include "flatbuffers/flatbuffers.h"
#include "IHandler.hpp"

template <typename ReqT, typename ResT>
class HandlerImpl : public IHandler {
public:
    using HandlerFunc = std::function<ResT(const ReqT&)>;

    HandlerImpl(HandlerFunc func) : m_func(std::move(func)) {}

    std::vector<uint8_t> handle(const uint8_t* data, size_t size) override {
        const ReqT* req = flatbuffers::GetRoot<ReqT>(data);

        ResT res = m_func(*req);

        flatbuffers::FlatBufferBuilder builder;
        auto resOffset = res.Pack(builder);
        builder.Finish(resOffset);

        return std::vector<uint8_t>(builder.GetBufferPointer(), builder.GetBufferPointer() + builder.GetSize());
    }

private:
    HandlerFunc m_func;
};
