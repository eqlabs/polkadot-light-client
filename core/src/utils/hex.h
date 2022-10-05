#pragma once

#include <string>
#include <vector>

#include "result.h"
#include "types.h"

namespace plc::core {

enum class UnhexError {
    NotEnoughInput = 1,
    NonHexInput,
    ValueOutOfRange,
    Missing0xPrefix,
    Unknown
};

Result<BlockHash> fromHexWithPrefix(std::string_view block_id_str);
Result<std::vector<uint8_t>> unhexWith0x(std::string_view hex_with_prefix);
Result<std::vector<uint8_t>> unhex(std::string_view hex);
std::string hex(const std::vector<uint8_t> &hexValue);

}

OUTCOME_HPP_DECLARE_ERROR(plc::core, UnhexError);
