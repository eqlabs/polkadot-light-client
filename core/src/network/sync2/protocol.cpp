#include "protocol.h"

#include "network/common/errors.h"
#include "network/protobuf/message_reader_writer.h"
#include "utils/callback_to_coro.h"

namespace plc::core::network::sync2 {

// TODO: use prefix from chainspec (use of {} instead of dot)
const libp2p::peer::Protocol Protocol::protocol = "/dot/sync/2";


} // namespace plc::core::network::sync2