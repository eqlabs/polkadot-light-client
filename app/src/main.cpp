#include <iostream>

#include <soralog/impl/configurator_from_yaml.hpp>

#include <libp2p/log/logger.hpp>
#include <libp2p/log/configurator.hpp>

#include "runner/client_runner.h"
#include "network/connection_manager.h"

namespace plc::app {

static const std::string logger_config = R"(
# ----------------
sinks:
  - name: colored_stdout
    type: console
    stream: stdout
    color: true
    thread: name
    capacity: 64
    max_message_length: 120
    buffer: 131072
    latency: 100
  - name: file
    type: file
    path: /tmp/plc_app.log
    thread: name
    capacity: 2048
    buffer: 4194304
    latency: 1000
  - name: sink_to_everywhere
    type: multisink
    sinks:
      - file
      - colored_stdout
groups:
  - name: main
    sink: sink_to_everywhere
    level: trace
    children:
      - name: libp2p
        level: off
      - name: plc
        children:
        - name: core
          children:
            - name: network
            - name: runner
            - name: utils
            - name: transaction
        - name: app
# ----------------
  )";

void prepareLogging() {

    auto logging_system = std::make_shared<soralog::LoggingSystem>(
        std::make_shared<soralog::ConfiguratorFromYAML>(
            std::make_shared<libp2p::log::Configurator>(),
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
        libp2p::log::setLevelOfGroup("plc", soralog::Level::TRACE);
    } else if (std::getenv("INFO_DEBUG") != nullptr)  {
        libp2p::log::setLevelOfGroup("main", soralog::Level::INFO);
    } else {
        libp2p::log::setLevelOfGroup("main", soralog::Level::ERROR);
    }
}



} // namespace plc::app

int main(const int count, const char** args) {
    using namespace plc::app;
    using namespace plc::core;

    prepareLogging();

    auto log_ = libp2p::log::createLogger("main","app");
    log_->trace("Example of trace log message");
    log_->debug("There is a debug value in this line: {}", 0xDEADBEEF);
    log_->verbose("Let's gossip about something");
    log_->info("This is simple info message");
    log_->warn("This is formatted message with level '{}'", "warning");
    log_->error("This is message with level '{}' and number {}", "error", 777);
    log_->critical("This is example of critical situations");

    auto runner = runner::ClientRunner();

    std::vector<std::string> peers;
    for (int i = 1; i < count; ++i) {
        peers.push_back(args[i]);
    }
    auto connection_manager = network::ConnectionManager(runner, peers);

    runner.run();

    return 0;
}
