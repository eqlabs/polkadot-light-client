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
    PeriodicTimer(boost::asio::io_service& io_service, std::chrono::milliseconds interval, Handler&& handler);
    PeriodicTimer(PeriodicTimer&& other) noexcept = default;
    ~PeriodicTimer() noexcept;

    void stop() noexcept;

private:
    // Use move-only pointer semantics for a timer and store all the stuff
    // in a shared pointer to structure containing all the data.
    // That allows capturing weak pointer to it in a handler, so that callback won't
    // be called after owning timer object is being destroyed.
    struct State;

private:
    std::shared_ptr<State> m_state;
    libp2p::log::Logger m_log = libp2p::log::createLogger("Timer","runner");
};

} // namespace pcl::core::runner
