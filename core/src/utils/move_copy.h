#pragma once

namespace plc::core {

#define PLC_DISABLE_COPY(TypeName) \
    TypeName(const TypeName&) = delete;\
    TypeName& operator=(const TypeName&) = delete

#define PLC_DISABLE_MOVE(TypeName) \
    TypeName(TypeName&&) = delete;\
    TypeName& operator=(TypeName&&) = delete

} // namespace plc::core
