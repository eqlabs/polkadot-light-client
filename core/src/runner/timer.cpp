#include "runner/timer.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace plc::core::runner {

struct PeriodicTimer::State {
    std::function<void(boost::system::error_code)> handler;
    boost::asio::deadline_timer timer;
    bool running;
};

PeriodicTimer::PeriodicTimer(boost::asio::io_service& io_service,
    std::chrono::milliseconds interval, Handler&& handler) {
    m_state = std::make_shared<State>(State{nullptr, boost::asio::deadline_timer(io_service), true});

    assert(handler);
    auto timer_interval = boost::posix_time::milliseconds(interval.count());
    m_state->handler = [state_weak = std::weak_ptr{m_state}, handler = std::move(handler),
        timer_interval, logger = m_log](const boost::system::error_code& error) {
        // TODO: process errors asdf 3 - DONE
        if (!error) {
            if (const auto state = state_weak.lock(); state && state->running) {
                handler();
                state->timer.expires_from_now(timer_interval);
                state->timer.async_wait(state->handler);
            } else {
                logger->error("Could not access time state");
            }
        } else {
            logger->error("Handler is called with an error: {}", error);
        }
    };

    m_state->timer.expires_from_now(timer_interval);
    m_state->timer.async_wait(m_state->handler);
}

PeriodicTimer::~PeriodicTimer() noexcept{
    stop();
}

void PeriodicTimer::stop() noexcept {
    if (m_state) {
        m_state->running = false;
    }
}

} // namepace plc::core::runner
