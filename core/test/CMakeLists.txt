function (add_core_test test_name)
    add_test_module(${test_name} ${ARGN})

    target_include_directories(${test_name} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../src")
    target_link_libraries(${test_name} p2p::p2p plc_core)
endfunction()

add_core_test(move_on_copy_test
    move_on_copy_test.cpp)

add_core_test(chain_spec_parse_test
    chain_spec_parse_test.cpp)

add_core_test(hex_test
    hex_test.cpp)

add_core_test(light2_test
    light2_serialization_test.cpp)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/core/test)
