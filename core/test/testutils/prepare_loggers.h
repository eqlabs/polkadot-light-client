#pragma once

#include <iostream>

#include <libp2p/log/configurator.hpp>
#include <libp2p/log/logger.hpp>

static std::once_flag initialized;

void prepareLoggers(soralog::Level level = soralog::Level::DEBUG) {
    std::call_once(initialized, [] {
        auto testing_log_config = std::string(R"(
# ---- Begin of logging system config addon ----
sinks:
- name: console
  type: console
  capacity: 4
  buffer: 16384
  latency: 0
groups:
- name: libp2p
  sink: console
  level: info
  children:
    - name: testing
    - name: chain
    - name: host
    - name: network
    - name: runner
    - name: runtime
    - name: utils
    - name: transaction
# ----- End of logging system config addon -----
)");

        auto logging_system = std::make_shared<soralog::LoggingSystem>(
                    std::make_shared<soralog::ConfiguratorFromYAML>(
                        // Original LibP2P logging config
                        std::make_shared<libp2p::log::Configurator>(),
                        // Additional logging config for testing
                        testing_log_config));
        auto r = logging_system->configure();
        if (r.has_error) {
            std::cerr << r.message << std::endl;
            throw std::runtime_error("Can't configure logger system");
        }
        libp2p::log::setLoggingSystem(logging_system);
    });
    libp2p::log::setLevelOfGroup(libp2p::log::defaultGroupName, level);
}
