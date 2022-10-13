#include <iostream>
#include <regex>

#include <soralog/impl/configurator_from_yaml.hpp>

#include <boost/outcome/try.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/program_options.hpp>
#include <iostream>

#include <libp2p/log/logger.hpp>
#include <libp2p/log/configurator.hpp>

#include "chain/spec.h"
#include "runner/client_runner.h"
#include "network/peer_manager.h"

namespace plc::app {

using namespace boost::program_options;

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

const static std::map<std::string,soralog::Level> log_levels = {
    {"off", soralog::Level::OFF},
    {"critical", soralog::Level::CRITICAL},
    {"error", soralog::Level::ERROR},
    {"warn", soralog::Level::WARN},
    {"info", soralog::Level::INFO},
    {"verbose", soralog::Level::VERBOSE},
    {"debug", soralog::Level::DEBUG},
    {"trace", soralog::Level::TRACE}
};

void prepareLogging(std::string log_level, std::string log_file) {
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

    auto level = log_levels.find(log_level);
    if (level != log_levels.end()) {
        libp2p::log::setLevelOfGroup("plc", level->second);
    } else {
        std::cout << "Did not find log level " << log_level << " -- setting log level to default, info.\n";
    }
}

std::unordered_map<std::string,std::string> parseArgs(const int count, const char** args) {
  try {
    options_description desc{"Options", 120, 40};
    desc.add_options()
      ("help,h", "Help screen")
      ("spec,s", value<std::string>()->default_value(""), "Chain spec file: mandatory")
      ("log-file,f", value<std::string>()->default_value(""), "Logger file: optional, for multi-sink logging to both console and file")
      ("log-level,l", value<std::string>()->default_value("info"), "Logger level: [ off | critical | error | warn | info | verbose | debug | trace ]");

    variables_map vm;
    store(parse_command_line(count, args, desc), vm);

    if (vm.count("help")) {
      std::cout << desc << '\n';
      exit(EXIT_FAILURE);
    }

    auto spec = vm["spec"].as<std::string>();
    if (spec.size() == 0) {
      std::cout << "No chain spec file specified in command line" << std::endl;
      std::cout << desc << std::endl;
      exit(EXIT_FAILURE);
    }
    std::unordered_map<std::string,std::string> result;
    result.emplace("log-level", vm["log-level"].as<std::string>());
    result.emplace("log-file", vm["log-file"].as<std::string>());
    result.emplace("spec", vm["spec"].as<std::string>());
    return result;
  } catch (const error &ex) {
    std::cerr << ex.what() << '\n';
    exit(EXIT_FAILURE);
  }
}


} // namespace plc::app

int main(const int count, const char** args) {
    using namespace plc::app;
    using namespace plc::core;

    auto varmap = parseArgs(count, args);

    prepareLogging(varmap.at("log-level"), varmap.at("log-file"));
    auto mainLogger = libp2p::log::createLogger("main","plc");

    auto stop_handler = std::make_shared<plc::core::StopHandler>();
    auto runner = std::make_shared<runner::ClientRunner>(stop_handler);
    stop_handler->add(runner);
    std::shared_ptr<network::PeerManager> connection_manager;

    auto result = plc::core::chain::Spec::loadFromFile(varmap.at("spec"));
    if (result.has_error()) {
        exit(EXIT_FAILURE);
    }
    auto chainSpec = result.value();
    connection_manager = std::make_shared<network::PeerManager>(runner, chainSpec.getBootNodes(), stop_handler);
    stop_handler->add(connection_manager);

    runner->run();

    mainLogger->info("Exiting application");

    return 0;
}
