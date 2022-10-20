#pragma once

#include <variant>

#include <light.pb.h>

#include "network/common/errors.h"
#include "network/protobuf/utils.h"
#include "utils/string_conversion.h"
#include "utils/types.h"

namespace plc::core::network::light2 {

struct RemoteCallRequest {
    BlockHash block;
    std::string method;
    std::string data;

    inline auto toProto() && {
        proto::Request req;
        auto& msg = *req.mutable_remote_call_request();
        msg.set_block(toString(block));
        msg.set_method(std::move(method));
        msg.set_data(std::move(data));

        return req;
    }
};

struct RemoteCallResponse {
    using ProtoMessageType = proto::Response;

    std::string proof;

    static inline Result<RemoteCallResponse> fromProto(ProtoMessageType&& msg) {
        if (msg.response_case() != proto::Response::kRemoteCallResponse) {
            return ProtocolError::InvalidResponse;
        }

        auto& resp = *msg.mutable_remote_call_response();
        return RemoteCallResponse{protobuf::takeFromPtr(resp.release_proof())};
    }
};

struct RemoteReadRequest {
    BlockHash block;
    std::vector<std::string> keys;

    inline auto toProto() && {
        proto::Request req;
        auto& msg = *req.mutable_remote_read_request();
        msg.set_block(toString(block));
        for (auto& key: keys) {
            msg.add_keys(std::move(key));
        }

        return msg;
    }
};


struct RemoteReadResponse {
    using ProtoMessageType = proto::Response;

    std::string proof;

    static inline Result<RemoteReadResponse> fromProto(ProtoMessageType&& msg) {
        if (msg.response_case() != proto::Response::kRemoteReadResponse) {
            return ProtocolError::InvalidResponse;
        }

        auto& resp = *msg.mutable_remote_read_response();
        return RemoteReadResponse{protobuf::takeFromPtr(resp.release_proof())};
    }
};

struct RemoteReadChildRequest {
    BlockHash block;
    std::string storage_key;
    std::vector<std::string> keys;

    inline auto toProto() && {
        proto::Request req;
        auto& msg = *req.mutable_remote_read_child_request();
        msg.set_block(toString(block));
        msg.set_storage_key(std::move(storage_key));
        for (auto& key: keys) {
            msg.add_keys(std::move(key));
        }

        return msg;
    }
};

} // namespace plc::core::network::light2
