#include <iostream>
#include <regex>

#include <soralog/impl/configurator_from_yaml.hpp>

#include <libp2p/log/logger.hpp>
#include <libp2p/log/configurator.hpp>

#include "runner/client_runner.h"
#include "network/connection_manager.h"

namespace plc::app {

static const std::string replacement = "_PLCLOGFILE_";

static const std::string simple_config = R"(
# ----------------
sinks:
  - name: console
    type: console
    stream: stdout
    color: true
    thread: name
    latency: 0
groups:
  - name: main
    sink: console
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

static const std::string multisink_config = R"(
# ----------------
sinks:
  - name: console
    type: console
    stream: stdout
    color: true
    thread: name
    latency: 0
  - name: file
    type: file
    path: _PLCLOGFILE_
    thread: name
    capacity: 2048
    buffer: 4194304
    latency: 1000
  - name: sink_to_everywhere
    type: multisink
    sinks:
      - file
      - console
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
    // prepare log system
    auto envfile = std::getenv(replacement.c_str());
    std::string config = simple_config;
    if (envfile != nullptr) {
        std::string logfile = envfile;
        if (logfile.size() > 0) {
            config = std::regex_replace(multisink_config, std::regex("(_PLCLOGFILE_)(.*)"), logfile);
        }
    }

    auto logging_system = std::make_shared<soralog::LoggingSystem>(
        std::make_shared<soralog::ConfiguratorFromYAML>(
            std::make_shared<libp2p::log::Configurator>(),
            config));
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

    auto runner = runner::ClientRunner();

    std::vector<std::string> peers;
    for (int i = 1; i < count; ++i) {
        peers.push_back(args[i]);
    }
    auto connection_manager = network::ConnectionManager(runner, peers);

    runner.run();

    return 0;
}
