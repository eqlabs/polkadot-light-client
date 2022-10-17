#include <iostream>

#include <boost/program_options.hpp>
#include <iostream>

#include "chain/spec.h"
#include "runner/client_runner.h"
#include "network/peer_manager.h"
#include "logger.h"

namespace plc::app {

struct CommandLineArgs {
    soralog::Level log_level;
    std::string log_file;
    std::string spec_file;
};

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

const static std::string help_label = "help";
const static std::string spec_file_label = "spec-file";
const static std::string log_file_label = "log-file";
const static std::string log_level_label = "log-level";

CommandLineArgs parseArgs(const int &count, const char** &args) {
    using namespace boost::program_options;
    try {
        options_description desc{"Options", 120, 40};
        desc.add_options()
            ((help_label + ",h").c_str(), "Help screen")
            ((spec_file_label + ",s").c_str(), value<std::string>()->default_value(""), "Chain spec file: mandatory")
            ((log_file_label + ",f").c_str(), value<std::string>()->default_value(""), "Logger file: optional, for multi-sink logging to both console and file")
            ((log_level_label + ",l").c_str(), value<std::string>()->default_value("info"), "Logger level: [ off | critical | error | warn | info | verbose | debug | trace ]");

        variables_map vm;
        store(parse_command_line(count, args, desc), vm);

        if (vm.count(help_label)) {
            std::cout << desc << '\n';
            exit(EXIT_SUCCESS);
        }

        CommandLineArgs result;
        result.log_file = vm[log_file_label].as<std::string>();
        result.spec_file = vm[spec_file_label].as<std::string>();
        if (result.spec_file.size() == 0) {
            std::cout << "No chain spec file specified in command line" << std::endl;
            std::cout << desc << std::endl;
            exit(EXIT_FAILURE);
        }

        result.log_level = soralog::Level::INFO;
        auto log_level_param = vm[log_level_label].as<std::string>();
        auto log_level_entry = log_levels.find(log_level_param);
        if (log_level_entry != log_levels.end()) {
            result.log_level = log_level_entry->second;
            std::cout << "Setting log level to " << log_level_entry->first << '\n';
        } else {
            std::cout << "Did not find log level " << log_level_param << "\n";
            exit(EXIT_FAILURE);
        }
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

    auto command_line_args = parseArgs(count, args);

    prepareLogging(command_line_args.log_level, command_line_args.log_file);
    auto mainLogger = libp2p::log::createLogger("main","plc");

    auto stop_handler = std::make_shared<plc::core::StopHandler>();
    auto runner = std::make_shared<runner::ClientRunner>(stop_handler);
    stop_handler->add(runner);
    std::shared_ptr<network::PeerManager> connection_manager;

    auto result = plc::core::chain::Spec::loadFromFile(command_line_args.spec_file);
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
