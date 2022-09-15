add_library(plc_core
    src/runner/client_runner.cpp
    src/network/connection.cpp)
message(STATUS "dir = ${CMAKE_CURRENT_LIST_DIR}/src")
target_include_directories(plc_core PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src")
target_link_libraries(plc_core PUBLIC p2p::p2p cppcoro::cppcoro)