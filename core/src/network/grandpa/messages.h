#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <boost/variant/variant.hpp>

#include "network/scale/streams.h"
#include "utils/types.h"

namespace plc::core::network::grandpa {

using Signature = std::array<uint8_t, 64>;
using Id = std::array<uint8_t, 32>;

struct BlockInfo {
    BlockNumber number;
    BlockHash hash;

    template <typename Self, typename Archive>
    static void serialize(Self& self, Archive& ar) {
        ar(self.number);
        ar(self.hash);
    }
};

PLC_DEFINE_STREAM_OPERATORS(BlockInfo)

struct Vote {
    BlockInfo message;
    Signature signature;
    Id id;

    template <typename Self, typename Archive>
    static void serialize(Self& self, Archive& ar) {
        ar(self.message);
        ar(self.signature);
        ar(self.id);
    }
};


PLC_DEFINE_STREAM_OPERATORS(Vote)

struct VoteMessage {
    uint64_t round_number;
    uint64_t set_id;
    Vote vote;

    template <typename Self, typename Archive>
    static void serialize(Self& self, Archive& ar) {
        ar(self.round_number);
        ar(self.set_id);
        ar(self.vote);
    }
};

PLC_DEFINE_STREAM_OPERATORS(VoteMessage)

struct CompactCommit {
    BlockHash target_hash;
    BlockNumber target_number;
    std::vector<BlockInfo> precommits;
    std::vector<std::tuple<Signature, Id>> auth_data;

    template <typename Self, typename Archive>
    static void serialize(Self& self, Archive& ar) {
        ar(self.target_hash);
        ar(self.target_number);
        ar(self.precommits);
        ar(self.auth_data);
    }
};

PLC_DEFINE_STREAM_OPERATORS(CompactCommit)

struct FullCommitMessage {
    uint64_t round = 0;
    uint64_t set_id;
    CompactCommit message;

    template <typename Self, typename Archive>
    static void serialize(Self& self, Archive& ar) {
        ar(self.round);
        ar(self.set_id);
        ar(self.message);
    }
};

PLC_DEFINE_STREAM_OPERATORS(FullCommitMessage)

struct GrandpaNeighborMessage {
    uint8_t version = 1;
    uint64_t round_number;
    uint64_t voter_set_id;
    BlockNumber last_finalized;

    template <typename Self, typename Archive>
    static void serialize(Self& self, Archive& ar) {
        ar(self.version);
        ar(self.round_number);
        ar(self.voter_set_id);
        ar(self.last_finalized);
    }
};

PLC_DEFINE_STREAM_OPERATORS(GrandpaNeighborMessage)

struct CatchUpRequest {
    uint64_t round_number;
    uint64_t voter_set_id;

    template <typename Self, typename Archive>
    static void serialize(Self& self, Archive& ar) {
        ar(self.round_number);
        ar(self.voter_set_id);
    }
};

PLC_DEFINE_STREAM_OPERATORS(CatchUpRequest)

struct SignedMessage {
    Vote message;
    Signature signature;
    Id id;

    template <typename Self, typename Archive>
    static void serialize(Self& self, Archive& ar) {
        ar(self.message);
        ar(self.signature);
        ar(self.id);
    }
};

PLC_DEFINE_STREAM_OPERATORS(SignedMessage)

struct CatchUpResponse {
    uint64_t voter_set_id;
    uint64_t round_number;
    std::vector<SignedMessage> prevote_justification;
    std::vector<SignedMessage> precommit_justification;
    BlockInfo best_final_candidate;

    template <typename Self, typename Archive>
    static void serialize(Self& self, Archive& ar) {
        ar(self.voter_set_id);
        ar(self.round_number);
        ar(self.prevote_justification);
        ar(self.precommit_justification);
        ar(self.best_final_candidate);
    }
};

PLC_DEFINE_STREAM_OPERATORS(CatchUpResponse)

using Message =
    // Note: order of types in variant matters
    // TODO: switch to std::variant. Now we use boost::variant
    // since scale library already is able to encode/decode them
    boost::variant<VoteMessage,          // 0
                FullCommitMessage,       // 1
                GrandpaNeighborMessage,  // 2
                CatchUpRequest,          // 3
                CatchUpResponse>;        // 4

} // namespace plc::core::network::grandpa
