#pragma once

#include <string>

#include <unordered_map>
#include <shell-interface.h>

#include "host/api.h"
#include "runtime/memory.h"

namespace plc::core::runtime {

using host_api_function = wasm::Literals(plc::core::host::Api::*)(const wasm::LiteralList&);

class ExternalInterface final : public wasm::ShellExternalInterface {
public:
    ExternalInterface(std::shared_ptr<plc::core::host::Api> host_api) : m_host_api(host_api) {
        registerImports();
    }

    void registerImports();
    void initMemory(uint32_t heap_base);

    wasm::Literals callImport(wasm::Function* import, wasm::LiteralList& arguments) override;

private:    
    std::unordered_map<std::string, host_api_function> m_imports;
    std::shared_ptr<plc::core::host::Api> m_host_api;
    std::shared_ptr<plc::core::runtime::Memory> m_memory;
};

} //namespace plc::core::runtime
