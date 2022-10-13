#pragma once

#include <chrono>
#include <functional>
#include <memory>

#include <boost/asio.hpp>

#include <libp2p/log/logger.hpp>

namespace plc::core::runner {

// TODO: make a coroutine version of this, so that we could use:
// Timer timer(...);
// while (await timer.next()) {
//     body of handler
// }
class PeriodicTimer {
public:
    using Handler = std::function<void()>;

public:
    // may optionally pass in logger, used instead of default logger
    PeriodicTimer(boost::asio::io_service& io_service, std::chrono::milliseconds interval, Handler&& handler, 
        libp2p::log::Logger logger = nullptr);
    PeriodicTimer(PeriodicTimer&& other) noexcept = default;
    ~PeriodicTimer() noexcept;

    void stop() noexcept;
    void setLogger(libp2p::log::Logger _log);

private:
    // Use move-only pointer semantics for a timer and store all the stuff
    // in a shared pointer to structure containing all the data.
    // That allows capturing weak pointer to it in a handler, so that callback won't
    // be called after owning timer object is being destroyed.
    struct State;

private:
    std::shared_ptr<State> m_state;
    libp2p::log::Logger m_log;
};

} // namespace pcl::core::runner
