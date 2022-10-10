#pragma once

#include <string>
#include <vector>

#include "utils/result.h"
#include "utils/types.h"

namespace plc::core {

enum class UnhexError {
    NotEnoughInput = 1,
    NonHexInput,
    ValueOutOfRange,
    Missing0xPrefix,
    Unknown
};

Result<std::vector<uint8_t>> unhex(std::string_view hex);
Result<std::vector<uint8_t>> unhexWith0x(std::string_view hex_with_prefix);
Result<BlockHash> unhexWith0xToBlockHash(std::string_view hex_with_prefix);
std::string hex(const std::vector<uint8_t> &hex_value);

} //namespace plc::core

OUTCOME_HPP_DECLARE_ERROR(plc::core, UnhexError);
