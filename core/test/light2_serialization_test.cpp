#include <gtest/gtest.h>

#include "network/light2/messages.h"
#include "network/protobuf/utils.h"
#include "utils/propagate.h"

namespace plc::core::ut {

TEST(Light2Serialization, testRoundtrip) {
    network::light2::RemoteReadRequest req;
    req.block = {};
    req.block[0] = 42;
    req.keys.push_back("key");

    std::vector<uint8_t> buffer;
    auto proto_message = copy(req).toProto();
    network::protobuf::writeToVec(std::move(proto_message), buffer);

    network::light2::proto::Request deserialized_req;
    const auto parse_res = deserialized_req.ParseFromArray(buffer.data(), buffer.size());
    ASSERT_TRUE(parse_res);
    ASSERT_TRUE(deserialized_req.has_remote_read_request());
}

} // namespace plc::core::ut