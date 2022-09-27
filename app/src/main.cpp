#include <iostream>

#include <soralog/impl/configurator_from_yaml.hpp>

#include <libp2p/log/logger.hpp>
#include <libp2p/log/configurator.hpp>

#include "runner/client_runner.h"
#include "network/connection_manager.h"

namespace plc::app {

void prepareLogging() {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    std::shared_ptr<soralog::Configurator> yaml_configurator_from_file =
    std::make_shared<soralog::ConfiguratorFromYAML>(
        std::filesystem::path("./config/logger.yml"));

    std::shared_ptr<soralog::Configurator> configurator = yaml_configurator_from_file;

    auto logging_system = std::make_shared<soralog::LoggingSystem>(configurator);

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

    auto log_ = libp2p::log::createLogger("main");
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
