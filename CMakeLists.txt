cmake_minimum_required(VERSION 3.12)

project(polkadot_light_client)
set (CMAKE_CXX_STANDARD 20)

if (NOT PLC_PROFILE)
  set(PLC_PROFILE "clang_debug")
endif()
set(PLC_PROFILE_PATH "${CMAKE_SOURCE_DIR}/cmake/${PLC_PROFILE}_profile.cmake")
include("${PLC_PROFILE_PATH}")

message(STATUS "---- top level polkadot-light-client cmakelists.txt-----")
message(STATUS "PLC_PROFILE_PATH is ${PLC_PROFILE_PATH}")
message(STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_COMPILER is ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_C_COMPILER is ${CMAKE_C_COMPILER}")
message(STATUS "PACKAGE_MANAGER is ${PACKAGE_MANAGER}")

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
