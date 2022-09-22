#include "runner/timer.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace plc::core::runner {

struct PeriodicTimer::State {
    bool running;
    boost::asio::deadline_timer timer;
    std::function<void(boost::system::error_code)> handler;
};

PeriodicTimer::PeriodicTimer(boost::asio::io_service& io_service,
    std::chrono::milliseconds interval, Handler&& handler) noexcept {
    m_state = std::make_shared<State>(true, boost::asio::deadline_timer(io_service), nullptr);

    assert(handler);
    m_state->handler = [state_weak = std::weak_ptr{m_state}, handler = std::move(handler)](const boost::system::error_code& error) {
        // TODO: process errors
        if (const auto state = state_weak.lock(); !error && state && state->running) {
            handler();
            state->timer.async_wait(state->handler);
        }
    };

    m_state->timer.expires_from_now(boost::posix_time::milliseconds(interval.count()));
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
