add_library(plc_core
    src/chain/spec.cpp
    src/runner/client_runner.cpp
    src/runner/timer.cpp
    src/network/common/errors.cpp
    src/network/grandpa/protocol.cpp
    src/network/peer_manager.cpp
    src/network/scale_reader_writer.cpp
    src/utils/stoppable.cpp
    src/utils/hex.cpp)
message(STATUS "dir = ${CMAKE_CURRENT_LIST_DIR}/src")
target_include_directories(plc_core PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src")
target_link_libraries(plc_core PUBLIC
    p2p::p2p
    p2p::p2p_identify
    p2p::p2p_kademlia
    p2p::p2p_ping
    p2p::p2p_gossip
    p2p::p2p_loopback_stream
    scale::scale
    cppcoro::cppcoro)
