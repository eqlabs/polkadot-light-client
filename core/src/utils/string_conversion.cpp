#include "string_conversion.h"

OUTCOME_CPP_DEFINE_CATEGORY(plc::core, ConversionError, e) {
  using E = plc::core::ConversionError;
  switch (e) {
    case E::InvalidSize:
        return "Invalid string length";
  }
  return "Unknown error (plc::core::ConversionError)";
}
