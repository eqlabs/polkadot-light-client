cmake_minimum_required(VERSION 3.12)

project(polkadot_light_client)
set (CMAKE_CXX_STANDARD 20)

option(ASAN "Enable address sanitizer" OFF)
option(LSAN "Enable leak sanitizer" OFF)
option(MSAN "Enable memory sanitizer" OFF)
option(TSAN "Enable thread sanitizer" OFF)
option(UBSAN "Enable UB sanitizer" OFF)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/3rdparty_utils.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/print.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/san.cmake")

add_3rdparty(Boost)
add_3rdparty(cpp-libp2p)
add_3rdparty(cppcoro)

add_subdirectory(core)
add_subdirectory(app)
