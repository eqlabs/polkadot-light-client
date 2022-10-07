#include "hex.h"

#include <boost/algorithm/hex.hpp>

OUTCOME_CPP_DEFINE_CATEGORY(plc::core, UnhexError, e) {
    using plc::core::UnhexError;
    switch (e) {
    case UnhexError::NonHexInput:
        return "Input contains non-hex characters";
    case UnhexError::NotEnoughInput:
        return "Input contains odd number of characters";
    case UnhexError::ValueOutOfRange:
        return "Decoded value is out of range of requested type";
    case UnhexError::Missing0xPrefix:
        return "Missing expected 0x prefix";
    case UnhexError::Unknown:
    default:
        return "Unknown error";
    }
}

namespace plc::core {

Result<std::vector<uint8_t>> unhex(std::string_view hex) {
    std::vector<uint8_t> blob;
    blob.reserve((hex.size() + 1) / 2);
    try {
        boost::algorithm::unhex(hex.begin(), hex.end(), std::back_inserter(blob));
    } catch (const boost::algorithm::not_enough_input &e) {
        return UnhexError::NotEnoughInput;
    } catch (const boost::algorithm::non_hex_input &e) {
        return UnhexError::NonHexInput;
    } catch (const std::exception &e) {
        return UnhexError::Unknown;
    }
    return blob;
}

Result<std::vector<uint8_t>> unhexWith0x(std::string_view hex_with_prefix) {
    const static std::string leading_chars = "0x";
    if (hex_with_prefix.substr(0, leading_chars.size()) != leading_chars) {
        return UnhexError::Missing0xPrefix;
    }
    auto without_prefix = hex_with_prefix.substr(leading_chars.size());
    return unhex(without_prefix);
}


Result<BlockHash> unhexWith0xToBlockHash(std::string_view hex_with_prefix) {
    OUTCOME_TRY(blob, unhexWith0x(hex_with_prefix));
    BlockHash block_hash;
    std::copy_n(std::make_move_iterator(blob.begin()), 32, block_hash.begin());
    return block_hash;
}

std::string hex(const std::vector<uint8_t> &hex_value) {
    std::string prefix = "0x";
    std::string result(hex_value.size() * 2 + prefix.size(), '\x00');
    result.replace(0, prefix.size(), "0x");
    boost::algorithm::hex_lower(hex_value.begin(), hex_value.end(), result.begin() + prefix.size());
    return result;
}

} //namespace plc::core
