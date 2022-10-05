cmake_minimum_required(VERSION 3.12)

project(polkadot_light_client)
set (CMAKE_CXX_STANDARD 20)

option(TESTING "Build tests" ON)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/3rdparty_utils.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/functions.cmake")

# the property is out of "if TESTING" scope due to addtest func is out too
set_property(GLOBAL PROPERTY TEST_TARGETS)

if (TESTING)
  add_3rdparty(GTest)
endif()
add_3rdparty(Boost)
add_3rdparty(cpp-libp2p)
add_3rdparty(cppcoro)
add_3rdparty(scale)

add_subdirectory(core)
add_subdirectory(app)

if(TESTING)
  enable_testing()
  add_subdirectory(core/test)
endif()
