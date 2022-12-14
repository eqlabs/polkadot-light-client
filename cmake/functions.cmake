function(add_test_module test_name)
  add_executable(${test_name} ${ARGN})
  if (POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
  endif ()
  target_sources(${test_name} PUBLIC
      ${ARGN}
      )
  target_link_libraries(${test_name} GTest::gtest GTest::gmock_main)
  add_test(
      NAME ${test_name}
      COMMAND $<TARGET_FILE:${test_name}>
  )
  set_target_properties(${test_name} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test_bin
      ARCHIVE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/test_lib
      LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/test_lib
      )
  set_property(GLOBAL APPEND PROPERTY TEST_TARGETS ${test_name})
endfunction()
