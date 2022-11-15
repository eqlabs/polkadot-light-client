cmake_minimum_required(VERSION 3.12)

project(polkadot_light_client)
set (CMAKE_CXX_STANDARD 20)

option(TESTING "Build tests" ON)
option(SAN "Enable sanitizer" OFF)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/3rdparty_utils.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/san.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/functions.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/utils.cmake")

# the property is out of "if TESTING" scope due to addtest func is out too
set_property(GLOBAL PROPERTY TEST_TARGETS)

if (TESTING)
  add_3rdparty(GTest)
endif()
if (LINTER)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endif()
if (CLANG_TIDY)
  include(cmake/clang-tidy.cmake)
endif ()
add_3rdparty(Boost)
add_3rdparty(cpp-libp2p)
add_3rdparty(cppcoro)
add_3rdparty(scale)
add_3rdparty(Protobuf)
add_3rdparty(binaryen)
add_3rdparty(zstd)

add_subdirectory(core)
add_subdirectory(app)

if(TESTING)
  enable_testing()
  add_subdirectory(core/test)
endif()
