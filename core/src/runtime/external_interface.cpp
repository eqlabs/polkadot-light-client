#include "runtime/external_interface.h"

namespace plc::core::runtime {

const static wasm::Name env = "env";

#define REGISTER_HOST_API_FUNCTION(name) \
    m_imports[#name] = &plc::core::host::Api::name

void ExternalInterface::registerImports() {
    REGISTER_HOST_API_FUNCTION(ext_logging_max_level_version_1);
    REGISTER_HOST_API_FUNCTION(ext_logging_log_version_1);
    REGISTER_HOST_API_FUNCTION(ext_allocator_malloc_version_1);
    REGISTER_HOST_API_FUNCTION(ext_allocator_free_version_1);
}

void ExternalInterface::initMemory(WasmSize heap_base) {
    m_memory = std::make_shared<plc::core::runtime::Memory>(&memory, heap_base);
    m_host_api->setMemory(m_memory);
}

wasm::Literals ExternalInterface::callImport(wasm::Function* import, wasm::LiteralList& arguments) {
    if (import->module == env) {        
        if (auto it = m_imports.find(import->base.c_str()); it != m_imports.end()) {
            return ((*m_host_api).*(it->second))(arguments);
        }
    }
    wasm::Fatal() << "callImport: unknown import: " << import->module.str << "."
                  << import->name.str;
    return wasm::Literals();
}

} //namespace plc::core::runtime
