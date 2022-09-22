cmake_minimum_required(VERSION 3.12)

project(polkadot_light_client)
set (CMAKE_CXX_STANDARD 20)

if (NOT PLC_PROFILE)
  set(PLC_PROFILE "clang_debug")
endif()
set(PLC_PROFILE_PATH "${CMAKE_SOURCE_DIR}/cmake/${PLC_PROFILE}_profile.cmake")
include("${PLC_PROFILE_PATH}")

include("${CMAKE_CURRENT_LIST_DIR}/cmake/3rdparty_utils.cmake")

add_3rdparty(Boost)
add_3rdparty(cpp-libp2p)
add_3rdparty(cppcoro)

add_subdirectory(core)
add_subdirectory(app)
