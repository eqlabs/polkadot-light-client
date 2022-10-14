#pragma once

#include <variant>

#include <light.pb.h>

#include "network/protobuf/utils.h"
#include "utils/string_conversion.h"
#include "utils/types.h"

namespace plc::core::network::light2 {

struct RemoteCallRequest {
    BlockHash block;
    std::string method;
    std::string data;

    inline auto toProto() && {
        proto::RemoteCallRequest msg;
        msg.set_block(toString(block));
        msg.set_method(std::move(method));
        msg.set_data(std::move(data));

        return msg;
    }
};

struct RemoteCallResponse {
    static constexpr auto message_index = 1;
    using ProtoMessageType = proto::RemoteCallResponse;

    std::string proof;

    static inline Result<RemoteCallResponse> fromProto(ProtoMessageType&& msg) {
        return RemoteCallResponse{protobuf::takeFromPtr(msg.release_proof())};
    }
};

struct RemoteReadRequest {
    BlockHash block;
    std::vector<std::string> keys;

    inline auto toProto() && {
        proto::RemoteReadRequest msg;
        msg.set_block(toString(block));
        for (auto& key: keys) {
            msg.add_keys(std::move(key));
        }

        return msg;
    }
};


struct RemoteReadResponse {
    static constexpr auto message_index = 2;
    using ProtoMessageType = proto::RemoteReadResponse;

    std::string proof;

    static inline Result<RemoteReadResponse> fromProto(ProtoMessageType&& msg) {
        return RemoteReadResponse{protobuf::takeFromPtr(msg.release_proof())};
    }
};

struct RemoteReadChildRequest {
    BlockHash block;
    std::string storage_key;
    std::vector<std::string> keys;

    inline auto toProto() && {
        proto::RemoteReadChildRequest msg;
        msg.set_block(toString(block));
        msg.set_storage_key(std::move(storage_key));
        for (auto& key: keys) {
            msg.add_keys(std::move(key));
        }

        return msg;
    }
};

struct RemoteHeaderRequest {
    BlockHash block;

    inline auto toProto() && {
        proto::RemoteHeaderRequest msg;
        msg.set_block(toString(block));

        return msg;
    }
};

struct RemoteHeaderResponse {
    static constexpr auto message_index = 3;
    using ProtoMessageType = proto::RemoteHeaderResponse;

    std::string header;
    std::string proof;

    static inline Result<RemoteHeaderResponse> fromProto(proto::RemoteHeaderResponse&& msg) {
        return RemoteHeaderResponse{
            protobuf::takeFromPtr(msg.release_header()),
            protobuf::takeFromPtr(msg.release_proof()),
        };
    }
};

struct RemoteChangesRequest {
    std::string first;
    std::string last;
    std::string min;
    std::string max;
    std::string storage_key;
    std::string key;

    inline auto toProto() && {
        proto::RemoteChangesRequest msg;
        msg.set_first(std::move(first));
        msg.set_first(std::move(last));
        msg.set_first(std::move(min));
        msg.set_first(std::move(max));
        msg.set_first(std::move(storage_key));
        msg.set_first(std::move(key));

        return msg;
    }
};

struct RemoteChangesResponse {
    static constexpr auto message_index = 3;
    using ProtoMessageType = proto::RemoteChangesResponse;

    std::string max;
    std::vector<std::string> proof;
    std::vector<std::pair<std::string, std::string>> roots;
    std::string roots_proof;

    static inline Result<RemoteChangesResponse> fromProto(ProtoMessageType&& msg) {
        return RemoteChangesResponse{
            protobuf::takeFromPtr(msg.release_max()),
            protobuf::takeFromCollection(*msg.mutable_proof()),
            protobuf::takeFromCollection(*msg.mutable_roots(), [](auto& pair) {
                return std::make_pair(protobuf::takeFromPtr(pair.release_first()),
                    protobuf::takeFromPtr(pair.release_second()));
                }),
            protobuf::takeFromPtr(msg.release_roots_proof()),
        };
    }
};

} // namespace plc::core::network::light2
