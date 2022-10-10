#include <iostream>
#include <regex>

#include <soralog/impl/configurator_from_yaml.hpp>

#include <libp2p/log/logger.hpp>
#include <libp2p/log/configurator.hpp>

#include "chain/spec.h"
#include "runner/client_runner.h"
#include "network/peer_manager.h"

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
    level: off
    children:
      - name: libp2p
      - name: plc
        level: info
        children:
        - name: core
          children:
            - name: chain
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
    level: off
    children:
      - name: libp2p
      - name: plc
        children:
        - name: core
          children:
            - name: chain
            - name: network
            - name: runner
            - name: utils
            - name: transaction
        - name: app
# ----------------
  )";

// pass in 'true' here to get demo logging lines, one of each class
void prepareLogging() {
    // prepare log system
    // TODO: replace this with command line parameter when we have argument parsing happening
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
    if (std::getenv("LOG_TRACE") != nullptr) {
        libp2p::log::setLevelOfGroup("plc", soralog::Level::TRACE);
    } else if (std::getenv("LOG_DEBUG") != nullptr) {
        libp2p::log::setLevelOfGroup("plc", soralog::Level::DEBUG);
    } else if (std::getenv("LOG_ERROR") != nullptr)  {
        libp2p::log::setLevelOfGroup("plc", soralog::Level::ERROR);
    }
}



} // namespace plc::app

int main(const int count, const char** args) {
    using namespace plc::app;
    using namespace plc::core;

    prepareLogging();

    auto runner = runner::ClientRunner();
    std::unique_ptr<network::PeerManager> connection_manager;

    if (count == 2) {
        auto result = plc::core::chain::Spec::loadFromFile(args[1]);
        if (result.has_error()) {
            exit(EXIT_FAILURE);
        }
        auto chainSpec = result.value();
        connection_manager = std::make_unique<network::PeerManager>(runner, chainSpec.getBootNodes());
    }
    else if (count > 2){
        std::vector<std::string> peers;
        for (int i = 1; i < count; ++i) {
            peers.push_back(args[i]);
        }
        connection_manager = std::make_unique<network::PeerManager>(runner, peers);
    }
    else {
        std::cerr << "Too few arguments, needed at least 1 with chain spec file or 2 (or more) with peer addresses" << std::endl;
        exit(EXIT_FAILURE);
    }

    runner.run();

    return 0;
}
