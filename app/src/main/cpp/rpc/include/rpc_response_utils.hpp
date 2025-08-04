//
// Created by Tom on 2025/7/29.
//

#ifndef GRPCJNISERVER_RPC_RESPONSE_UTILS_HPP
#define GRPCJNISERVER_RPC_RESPONSE_UTILS_HPP


#pragma once

#include <string>
#include <vector>
#include <type_traits>
#include <flatbuffers/flatbuffers.h>

namespace rpc_utils {

    template <typename T>
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>>
    to_result_bytes(flatbuffers::FlatBufferBuilder &builder, const T &value)
    {
        static_assert(sizeof(T) == 0, "Unsupported ResponseT type for serialization");
        return {};
    }

    template <>
    inline flatbuffers::Offset<flatbuffers::Vector<uint8_t>>
    to_result_bytes<std::string>(flatbuffers::FlatBufferBuilder &builder, const std::string &value)
    {
        return builder.CreateVector(reinterpret_cast<const uint8_t *>(value.data()), value.size());
    }

    template <>
    inline flatbuffers::Offset<flatbuffers::Vector<uint8_t>>
    to_result_bytes<const char *>(flatbuffers::FlatBufferBuilder &builder, const char *const &value)
    {
        return builder.CreateVector(reinterpret_cast<const uint8_t *>(value), strlen(value));
    }

    template <>
    inline flatbuffers::Offset<flatbuffers::Vector<uint8_t>>
    to_result_bytes<std::vector<uint8_t>>(flatbuffers::FlatBufferBuilder &builder, const std::vector<uint8_t> &value)
    {
        return builder.CreateVector(value);
    }

    template <typename T>
    typename std::enable_if<std::is_trivially_copyable<T>::value &&
                            !std::is_same<T, std::string>::value &&
                            !std::is_same<T, std::vector<uint8_t>>::value &&
                            !std::is_pointer<T>::value,
            flatbuffers::Offset<flatbuffers::Vector<uint8_t>>>::type
    to_result_bytes(flatbuffers::FlatBufferBuilder &builder, const T &value)
    {
        return builder.CreateVector(reinterpret_cast<const uint8_t *>(&value), sizeof(T));
    }

} // namespace rpc_utils


#endif //GRPCJNISERVER_RPC_RESPONSE_UTILS_HPP
