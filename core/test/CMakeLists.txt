add_test_module(move_on_copy_test
    move_on_copy_test.cpp)

add_test_module(chain_spec_parse_test
    chain_spec_parse_test.cpp)

add_test_module(hex_test
    hex_test.cpp)

add_test_module(jrpc_server_test
jrpc_server_test.cpp)

target_include_directories(move_on_copy_test PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../src")
target_include_directories(chain_spec_parse_test PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../src")
target_include_directories(hex_test PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../src")
target_include_directories(jrpc_server_test PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../src")

target_link_libraries(chain_spec_parse_test p2p::p2p plc_core)
target_link_libraries(hex_test p2p::p2p plc_core)
target_link_libraries(jrpc_server_test p2p::p2p plc_core boost_json)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/core/test)
file(COPY ${CMAKE_CURRENT_LIST_DIR}/nodeutils DESTINATION ${CMAKE_BINARY_DIR}/core/test)
