#include "client_runner.h"

namespace plc {
namespace core {
namespace runner {

void ClientRunner::run() {
    _io_service.run();
}

} // runner
} // core
} // namespace plc