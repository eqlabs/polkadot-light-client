#include <iostream>
#include <regex>

#include <soralog/impl/configurator_from_yaml.hpp>

#include <iostream>

#include <libp2p/log/logger.hpp>
#include <libp2p/log/configurator.hpp>


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

void prepareLogging(soralog::Level &log_level, const std::string &log_file) {
    std::string config = simple_config;
    if (log_file.size() > 0) {
        config = std::regex_replace(multisink_config, std::regex("(_PLCLOGFILE_)(.*)"), log_file);
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
    libp2p::log::setLevelOfGroup("plc", log_level);
}


} // namespace plc::app
