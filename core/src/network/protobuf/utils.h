#pragma once

#include <cassert>
#include <concepts>
#include <cstdint>
#include <memory>
#include <stddef.h>
#include <vector>

namespace plc::core::network::protobuf {

template <typename T>
concept ProtobufMessage = requires(const T& t, void* buf, int size) {
    { t.ByteSizeLong() } -> std::same_as<size_t>;
    { t.SerializeToArray(buf, size) } -> std::same_as<bool>;
};

template <typename T>
concept ConvertibleToProtobuf = requires() {
    { std::declval<T&&>().toProto() } -> ProtobufMessage;
};

template <typename T>
concept ConvertibleFromProtobuf = requires {
    typename T::ProtoMessageType;
    { T::fromProto(std::declval<typename T::ProtoMessageType&&>()) } -> std::same_as<T>;
};

void writeToVec(
    const ProtobufMessage auto& msg,
    std::vector<uint8_t>& buf) {
    const auto old_size = buf.size();
    const auto new_size = old_size + msg.ByteSizeLong();
    buf.reserve(new_size);
    msg.SerializeToArray(&buf[old_size], msg.ByteSizeLong());

    buf.resize(new_size);
}

void writeToVec(ConvertibleToProtobuf auto&& msg,
    std::vector<uint8_t>& buf) {
    writeToVec(std::move(msg).toProto(), buf);
}

// Unfortunately protobuf doesn't give a chance to create a generic solution for getting
// the desired type
#define PLC_DEFINE_FROM_ONE_OF_MESSAGE(ResultMessageType, GenericProtoMessageType, field_name) \
inline std::optional<ResultMessageType> fromProto(GenericProtoMessageType&& msg) { \
    if (auto* typed_message = msg.release_##field_name()) { \
        std::unique_ptr<std::remove_pointer_t<decltype(typed_message)>> message_holder{typed_message}; \
        return {fromProto(std::move(*message_holder))}; \
    } else { \
        return std::nullopt; \
    } \
}


/// @brief This method is handy to extract values from protobuf objects
/// extracted from release_XXX methods (see https://developers.google.com/protocol-buffers/docs/reference/cpp-generated)
/// @tparam T
/// @param value
/// @return
template <typename T>
T takeFromPtr(T* value) {
    assert(value);
    std::unique_ptr<T> val(value);
    return std::move(*val);
}

namespace details {

struct Take {
    template <std::move_constructible T>
    T operator()(T& value) const {
        return std::move(value);
    }
};

} // details

template <typename T>
concept ProtobufCollection = requires(T& t) {
    typename T::value_type;

    {t.size()} -> std::same_as<int>;
    {t[int()]} -> std::same_as<typename T::value_type&>;
};

template <ProtobufCollection Collection, std::invocable<typename Collection::value_type&> Extract = details::Take>
auto takeFromCollection(Collection& protobuf_collection, Extract extract = {}) {
    using Element = decltype(extract(protobuf_collection[0]));
    std::vector<Element> result(protobuf_collection.size());
    for (size_t i = 0; i < protobuf_collection.size(); ++i) {
        result[i] = extract(protobuf_collection[i]);
    }

    return result;
}

} // namespace plc::core::network::protobuf
