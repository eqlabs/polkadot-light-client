

#include <libp2p/outcome/outcome.hpp>

namespace plc::core {

    std::function<void()> stop_func;
    void setStop(std::function<void()> stop) {
        stop_func = stop;
    }
    void stop() {
        if (stop_func != nullptr) {
            stop_func();
        }

    }
} // namespace plc::core
