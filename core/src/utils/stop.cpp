

#include <libp2p/outcome/outcome.hpp>

namespace plc::core {

    // this function should be called following an acute error, where
    // the application cannot continue any longer and must
    // shutdown (but gracefully, closing connections and release resources)
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
