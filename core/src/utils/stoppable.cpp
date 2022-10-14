

#include <libp2p/outcome/outcome.hpp>
#include "utils/stoppable.h"


namespace plc::core {

    void StopHandler::add(std::weak_ptr<Stoppable> stoppable) {
        m_stoppables.push_back(stoppable);
    }

    void StopHandler::stop() {
        m_log->info("stop handler: {} stoppables", m_stoppables.size());

        // call all stoppables in the LIFO order
        for (int i = m_stoppables.size() - 1; i >= 0; i--) {
            if (const auto stoppable = m_stoppables[i].lock(); stoppable) {
                m_log->debug("  stoppable: {}", i);
                stoppable->stop();
            }
        }
    }

} // namespace plc::core
