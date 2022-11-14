#include "runtime/executor.h"

OUTCOME_CPP_DEFINE_CATEGORY(plc::core::runtime, Executor::Error, e) {
    using E = plc::core::runtime::Executor::Error;
    switch (e) {
    case E::MissingReturnValue:
        return "Missing return value from runtime API call!";
    }
    return "Unknown error";
}