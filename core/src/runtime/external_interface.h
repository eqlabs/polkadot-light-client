#pragma once

#include <string>

#include <shell-interface.h>

#include "host/api.h"

namespace plc::core::runtime {

class ExternalInterface final : public wasm::ShellExternalInterface {
public:
    ExternalInterface() {
        m_host_api = std::make_shared<plc::core::host::Api>();
        registerImports();
    }

    void registerImports();

    wasm::Literal callImport(wasm::Function* import, wasm::LiteralList& arguments) override;

private:
    std::shared_ptr<plc::core::host::Api> m_host_api;
};

} //namespace plc::core::runtime
