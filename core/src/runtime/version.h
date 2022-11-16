#pragma once

#include <string>

#include "network/scale/streams.h"

namespace plc::core::runtime {

struct Version {
    using ApiId = std::array<uint8_t, 8>;
    using Api = std::pair<ApiId, uint32_t>;

    std::string m_spec_name;
    std::string m_impl_name;
    uint32_t m_authoring_version = 0u;
    uint32_t m_spec_version = 0u;
    uint32_t m_impl_version = 0u;
    std::vector<Api> m_apis;
    uint32_t m_transaction_version = 1u;
    uint32_t m_state_version = 0u;

    template <typename Self, typename Archive>
    static void serialize(Self& self, Archive& ar) {
        ar(self.m_spec_name);
        ar(self.m_impl_name);
        ar(self.m_authoring_version);
        ar(self.m_spec_version);
        ar(self.m_impl_version);
        ar(self.m_apis);

        //TODO: decode apis
        if (ar.m_stream.hasMore(sizeof(self.m_transaction_version))) {
            ar(self.m_transaction_version);
        }
        if (ar.m_stream.hasMore(sizeof(self.m_state_version))) {
            ar(self.m_state_version);
        }
    }
};

PLC_DEFINE_STREAM_OPERATORS(Version)

} //namespace plc::core::runtime
