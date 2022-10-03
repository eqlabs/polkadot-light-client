add_library(plc_core
    src/runner/client_runner.cpp
    src/runner/timer.cpp
    src/utils/stop.cpp
    src/network/peer_manager.cpp)
message(STATUS "dir = ${CMAKE_CURRENT_LIST_DIR}/src")
target_include_directories(plc_core PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src")
target_link_libraries(plc_core PUBLIC
    p2p::p2p
    p2p::p2p_identify
    p2p::p2p_kademlia
    p2p::p2p_ping
    p2p::p2p_gossip
    cppcoro::cppcoro)
