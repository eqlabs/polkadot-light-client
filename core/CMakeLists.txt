add_library(plc-core
    src/runner/client_runner.cpp)

target_link_libraries(plc-core p2p cppcoro)
