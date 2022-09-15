#include <iostream>

#include <soralog/impl/configurator_from_yaml.hpp>

#include <libp2p/log/logger.hpp>
#include <libp2p/log/configurator.hpp>

#include "runner/client_runner.h"
#include "network/connection.h"

namespace plc::app {

static const std::string logger_config = R"(
# ----------------
sinks:
  - name: console
    type: console
    color: true
groups:
  - name: main
    sink: console
    level: info
    children:
      - name: libp2p
# ----------------
  )";

void prepare_logging() {
    // prepare log system
    auto logging_system = std::make_shared<soralog::LoggingSystem>(
        std::make_shared<soralog::ConfiguratorFromYAML>(
            // Original LibP2P logging config
            std::make_shared<libp2p::log::Configurator>(),
            // Additional logging config for application
            logger_config));
    auto r = logging_system->configure();
    if (!r.message.empty()) {
        (r.has_error ? std::cerr : std::cout) << r.message << std::endl;
    }
    if (r.has_error) {
        exit(EXIT_FAILURE);
    }

    libp2p::log::setLoggingSystem(logging_system);
    if (std::getenv("TRACE_DEBUG") != nullptr) {
        libp2p::log::setLevelOfGroup("main", soralog::Level::TRACE);
    } else {
        libp2p::log::setLevelOfGroup("main", soralog::Level::ERROR);
    }
}

} // namespace plc::app

int main(const int count, const char** args) {
    plc::app::prepare_logging();

    using plc::core::runner::ClientRunner;
    auto runner = ClientRunner();
    runner.post_task(plc::core::network::handshake(runner.get_service()));
    runner.run();

    return 0;
}