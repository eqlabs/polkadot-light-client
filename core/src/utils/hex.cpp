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
        return "Unknown error";
    }
    return "Unknown error (error id not listed)";
}

namespace plc::core {

Result<std::vector<uint8_t>> unhexWith0x(std::string_view hex_with_prefix) {
    const static std::string leading_chrs = "0x";

    if (hex_with_prefix.substr(0, leading_chrs.size()) != leading_chrs) {
        return UnhexError::Missing0xPrefix;
    }

    auto without_prefix = hex_with_prefix.substr(leading_chrs.size());

    return unhex(without_prefix);
}

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

Result<BlockHash> fromHexWithPrefix(std::string_view block_id_str) {
    const static std::string leading_chrs = "0x";

    if (block_id_str.substr(0, leading_chrs.size()) != leading_chrs) {
        return UnhexError::Missing0xPrefix;
    }

    auto without_prefix = block_id_str.substr(leading_chrs.size());

    OUTCOME_TRY(blob, unhex(without_prefix));

    try {
        boost::algorithm::unhex(without_prefix.begin(), without_prefix.end(), std::back_inserter(blob));
        BlockHash block_hash;
        std::copy_n(std::make_move_iterator(blob.begin()), 32, block_hash.begin());
        return block_hash;

    } catch (const boost::algorithm::not_enough_input &e) {
        return UnhexError::NotEnoughInput;

    } catch (const boost::algorithm::non_hex_input &e) {
        return UnhexError::NonHexInput;

    } catch (const std::exception &e) {
        return UnhexError::Unknown;
    }
}

std::string hex(const std::vector<uint8_t> &hexValue) {
    std::string prefix = "0x";
    std::string result(hexValue.size() * 2 + prefix.size(), '\x00');
    result.replace(0, prefix.size(), "0x");
    boost::algorithm::hex_lower(hexValue.begin(), hexValue.end(), result.begin() + prefix.size());
    return result;
}

}
