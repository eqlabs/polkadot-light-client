#pragma once

#include <scale.hpp>
#include <shell-interface.h>

#include "runtime/ptr.h"
#include "utils/result.h"

namespace plc::core::runtime {

class Executor final {
public:
    Executor() = default;

    void init(std::shared_ptr<wasm::ModuleInstance> module_instance) {
        m_module_instance = module_instance;
    }

    template<typename... Args>
    Result<Ptr> call(std::string_view name, Args &&...args) {
        //currently the passed args are ignored
        // ByteBuffer encoded_args;
        // if constexpr (sizeof...(args) > 0) {
        //     OUTCOME_TRY(res, scale::encode(std::forward<Args>(args)...));
        //     encoded_args.push_back(std::move(res));
        // }        
        //store args

        wasm::LiteralList arguments = {wasm::Literal{0}, wasm::Literal{0}};
        auto result = m_module_instance->callExport(wasm::Name{name.data()}, arguments);

        return libp2p::outcome::success(Ptr(result[0].geti64()));
    }

private:
    std::shared_ptr<wasm::ModuleInstance> m_module_instance;
};

} //namespace plc::core::runtime