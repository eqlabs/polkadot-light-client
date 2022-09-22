#pragma once

#include <chrono>
#include <functional>
#include <memory>

#include <boost/asio.hpp>

namespace plc::core::runner {

class PeriodicTimer {
public:
    using Handler = std::function<void()>;

public:
    PeriodicTimer(boost::asio::io_service& io_service, std::chrono::milliseconds interval, Handler&& handler) noexcept;
    PeriodicTimer(PeriodicTimer&& other) noexcept = default;
    ~PeriodicTimer() noexcept;

    void stop() noexcept;

private:
    struct State;

private:
    std::shared_ptr<State> m_state;
};

} // namespace pcl::core::runner