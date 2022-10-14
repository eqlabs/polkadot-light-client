#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <boost/variant/variant.hpp>

#include "network/common/streams.h"
#include "utils/types.h"

namespace plc::core::network::grandpa {

using Signature = std::array<uint8_t, 64>;
using Id = std::array<uint8_t, 32>;

struct BlockInfo {
    BlockNumber number;
    BlockHash hash;
};

// TODO: make a generic solution for this to remove copy-paste
template <EncoderStream Stream>
Stream& operator<<(Stream& stream, const BlockInfo& block) {
    return stream << block.number
        << block.hash;
}

template <DecoderStream Stream>
Stream& operator>>(Stream& stream, BlockInfo& block) {
    return stream >> block.number
        >> block.hash;
}

struct Vote {
    BlockInfo message;
    Signature signature;
    Id id;
};

template <EncoderStream Stream>
Stream& operator<<(Stream& stream, const Vote& vote) {
    return stream << vote.message
        << vote.signature
        << vote.id;
}

template <DecoderStream Stream>
Stream& operator>>(Stream& stream, Vote& vote) {
    return stream >> vote.message
        >> vote.signature
        >> vote.id;
}

struct VoteMessage {
    uint64_t round_number;
    uint64_t set_id;
    Vote vote;
};

template <EncoderStream Stream>
Stream& operator<<(Stream& stream, const VoteMessage& vote) {
    return stream << vote.round_number
        << vote.set_id
        << vote.vote;
}

template <DecoderStream Stream>
Stream& operator>>(Stream& stream, VoteMessage& vote) {
    return stream >> vote.round_number
        >> vote.set_id
        >> vote.vote;
}

struct CompactCommit {
    BlockHash target_hash;
    BlockNumber target_number;
    std::vector<BlockInfo> precommits;
    std::vector<std::tuple<Signature, Id>> auth_data;
};

template <EncoderStream Stream>
Stream& operator<<(Stream& stream, const CompactCommit& commit) {
    return stream << commit.target_hash
        << commit.target_number
        << commit.precommits
        << commit.auth_data;
}

template <DecoderStream Stream>
Stream& operator>>(Stream& stream, CompactCommit& commit) {
    return stream >> commit.target_hash
        >> commit.target_number
        >> commit.precommits
        >> commit.auth_data;
}

struct FullCommitMessage {
    uint64_t round = 0;
    uint64_t set_id;
    CompactCommit message;
};

template <EncoderStream Stream>
Stream& operator<<(Stream& stream, const FullCommitMessage& commit) {
    return stream << commit.round
        << commit.set_id
        << commit.message;
}

template <DecoderStream Stream>
Stream& operator>>(Stream& stream, FullCommitMessage& commit) {
    return stream >> commit.round
        >> commit.set_id
        >> commit.message;
}

struct GrandpaNeighborMessage {
    uint8_t version = 1;
    uint64_t round_number;
    uint64_t voter_set_id;
    BlockNumber last_finalized;
};

template <EncoderStream Stream>
Stream& operator<<(Stream& stream, const GrandpaNeighborMessage& neighbor) {
    return stream << neighbor.version
        << neighbor.round_number
        << neighbor.voter_set_id
        << neighbor.last_finalized;
}

template <DecoderStream Stream>
Stream& operator>>(Stream& stream, GrandpaNeighborMessage& neighbor) {
    return stream >> neighbor.version
        >> neighbor.round_number
        >> neighbor.voter_set_id
        >> neighbor.last_finalized;
}

struct CatchUpRequest {
    uint64_t round_number;
    uint64_t voter_set_id;
};

template <EncoderStream Stream>
Stream& operator<<(Stream& stream, const CatchUpRequest& request) {
    return stream << request.round_number
        << request.voter_set_id;
}

template <DecoderStream Stream>
Stream& operator>>(Stream& stream, CatchUpRequest& request) {
    return stream >> request.round_number
        >> request.voter_set_id;
}

struct SignedMessage {
    Vote message;
    Signature signature;
    Id id;
};

template <EncoderStream Stream>
Stream& operator<<(Stream& stream, const SignedMessage& message) {
    return stream << message.message
        << message.signature
        << message.id;
}

template <DecoderStream Stream>
Stream& operator>>(Stream& stream, SignedMessage& message) {
    return stream >> message.message
        >> message.signature
        >> message.id;
}

struct CatchUpResponse {
    uint64_t voter_set_id;
    uint64_t round_number;
    std::vector<SignedMessage> prevote_justification;
    std::vector<SignedMessage> precommit_justification;
    BlockInfo best_final_candidate;
};

template <EncoderStream Stream>
Stream& operator<<(Stream& stream, const CatchUpResponse& message) {
    return stream << message.voter_set_id
        << message.round_number
        << message.prevote_justification
        << message.precommit_justification
        << message.best_final_candidate;
}

template <DecoderStream Stream>
Stream& operator>>(Stream& stream, CatchUpResponse& message) {
    return stream >> message.voter_set_id
        >> message.round_number
        >> message.prevote_justification
        >> message.precommit_justification
        >> message.best_final_candidate;
}

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
