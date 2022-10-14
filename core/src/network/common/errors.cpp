#include "errors.h"

OUTCOME_CPP_DEFINE_CATEGORY(plc::core::network, ProtocolError, e) {
  using E = plc::core::network::ProtocolError;
  switch (e) {
    case E::OwnerDestroyed:
        return "Protocol stopped";
  }
  return "Unknown error (plc::core::network::ProtocolError)";
}
