#pragma once

#include <array>
#include <string_view>

#include <scale/outcome/outcome.hpp>

#include "utils/result.h"

namespace plc::core{

enum class ConversionError {
    InvalidSize = 1
};

template <typename T>
struct StringConverter;

template <size_t Size>
struct StringConverter<std::array<uint8_t, Size>> {
    static Result<std::array<uint8_t, Size>> fromString(std::string_view str) {
        if (str.size() != Size) {
            return ConversionError::InvalidSize;
        }

        std::array<uint8_t, Size> result;
        std::memcpy(result.data(), str.data(), Size);
        return result;
    }

    static std::string toString(const std::array<uint8_t, Size>& data) {
        std::string result;
        result.resize(Size);
        std::memcpy(result.data(), data.data(), Size);
        return result;
    }
};

template <typename T>
std::string toString(const T& val) {
    return StringConverter<T>::toString(val);
}

template <typename T>
T fromString(std::string_view str) {
    return StringConverter<T>::fromString(str);
}

} // namespace plc::core

OUTCOME_HPP_DECLARE_ERROR(plc::core, ConversionError);
