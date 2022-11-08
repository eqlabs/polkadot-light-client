#include "runtime/external_interface.h"

namespace plc::core::runtime {

const static wasm::Name env = "env";

void ExternalInterface::registerImports() {
}

wasm::Literals ExternalInterface::callImport(wasm::Function* import, wasm::LiteralList& arguments) {
    if (import->module == env) {
        // auto it = m_imports.find(import->base.c_str(), m_imports.hash_function(), m_imports.key_eq());        
        // if (it != m_imports.end()) {

        //     // return it->second();//it->second(*this, import, arguments);
        // }
        return m_host_api->ext_logging_max_level_version_1();
    }

    wasm::Fatal() << "callImport: unknown import: " << import->module.str << "."
                  << import->name.str;
    return wasm::Literals();
}

} //namespace plc::core::runtime
