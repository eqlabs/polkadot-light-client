#include "errors.h"

OUTCOME_CPP_DEFINE_CATEGORY(plc::core::network, ProtocolError, e) {
  using E = plc::core::network::ProtocolError;
  switch (e) {
    case E::OwnerDestroyed:
        return "Protocol stopped";
    case E::InvalidResponse:
        return "Invalid response message type";
    case E::ProtobufParsingFailed:
        return "Failed to parse protobuf message";
  }
  return "Unknown error (plc::core::network::ProtocolError)";
}
