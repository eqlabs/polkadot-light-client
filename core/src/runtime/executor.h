#pragma once

#include <scale.hpp>
#include <shell-interface.h>

#include "runtime/memory.h"
#include "runtime/ptr.h"
#include "utils/result.h"

namespace plc::core::runtime {

class Executor final {
public:
    Executor() = default;

    void init(std::shared_ptr<wasm::ModuleInstance> module_instance, 
              std::shared_ptr<plc::core::runtime::Memory> memory) {
        m_module_instance = module_instance;
        m_memory = memory;
    }

    template<typename R, typename... Args>
    Result<R> call(std::string_view name, Args &&...args) {
        wasm::LiteralList arguments;
        
        if constexpr (sizeof...(args) > 0) {
            ByteBuffer encoded_args;
            OUTCOME_TRY(res, scale::encode(std::forward<Args>(args)...));
            encoded_args.push_back(std::move(res));        
            //store args
            auto ptr = m_memory->storeBytes(encoded_args);
            arguments = {wasm::Literal{ptr.m_addr}, wasm::Literal{ptr.m_size}};
        }
        else {
            arguments = {wasm::Literal{0}, wasm::Literal{0}};
        }
        auto result = m_module_instance->callExport(wasm::Name{name.data()}, arguments);

        if constexpr (std::is_void_v<R>) {
            return libp2p::outcome::success();
        } else {
            auto bytes = m_memory->loadBytes(Ptr(result[0].geti64()));
            R t;
            scale::ScaleDecoderStream s(bytes);
            try {
            s >> t;
            // Check whether the whole byte buffer was consumed
            if (s.hasMore(1)) {
                // m_log->error("Runtime API call result size exceeds the size of the type to initialize {}",
                //               typeid(R).name());
                return libp2p::outcome::failure(std::errc::illegal_byte_sequence);
            }
                return libp2p::outcome::success(std::move(t));
            } catch (std::system_error &e) {
                return libp2p::outcome::failure(e.code());
            }
        }
    }

private:
    std::shared_ptr<wasm::ModuleInstance> m_module_instance;
    std::shared_ptr<plc::core::runtime::Memory> m_memory;
};

} //namespace plc::core::runtime
