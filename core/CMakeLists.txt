add_subdirectory(src/network/light2/protobuf)
add_subdirectory(src/network/sync2/protobuf)
add_library(plc_core
    src/chain/spec.cpp
    src/network/common/errors.cpp
    src/network/grandpa/protocol.cpp
    src/network/light2/protocol.cpp
    src/network/sync2/protocol.cpp
    src/network/peer_manager.cpp
    src/network/protobuf/message_reader_writer.cpp
    src/network/scale/message_reader_writer.cpp
    src/runner/client_runner.cpp
    src/runner/timer.cpp
    src/utils/hex.cpp
    src/utils/stoppable.cpp
    src/utils/string_conversion.cpp
    src/runtime/module.cpp
    src/runtime/service.cpp
    src/runtime/api.cpp)
message(STATUS "dir = ${CMAKE_CURRENT_LIST_DIR}/src")
target_include_directories(plc_core PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src")
target_link_libraries(plc_core
    PUBLIC
        light2_protobuf
        sync2_protobuf
        p2p::p2p
        p2p::p2p_identify
        p2p::p2p_kademlia
        p2p::p2p_ping
        p2p::p2p_gossip
        p2p::p2p_loopback_stream
        scale::scale
        cppcoro::cppcoro
        binaryen
        zstd::zstd)
