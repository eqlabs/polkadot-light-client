#pragma once

#include <cstdint>
#include <type_traits>

#include "network/scale/streams.h"

namespace plc::core::network {

union Roles {
    struct {
        /**
         * Full node, does not participate in consensus.
         */
        uint8_t full : 1;

        /**
         * Light client node.
         */
        uint8_t light : 1;

        /**
         * Act as an authority
         */
        uint8_t authority : 1;

    } flags;
    uint8_t value;

    explicit Roles(uint8_t val = 0) : value(val) {}

    template <typename Self, typename Archive>
    static void serialize(Self& self, Archive& archive) {
        archive(self.value);
    }
};

inline bool operator==(const Roles &lhs, const Roles &rhs) {
    return lhs.value == rhs.value;
}

PLC_DEFINE_STREAM_OPERATORS(Roles);

}  // namespace plc::core::network
