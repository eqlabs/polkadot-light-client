cmake_minimum_required(VERSION 3.12)

project(polcadot_light_client)
set (CMAKE_CXX_STANDARD 20)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/3rdparty_utils.cmake")

add_3rdparty(Boost)
add_3rdparty(cpp-libp2p)
add_3rdparty(cppcoro)

add_subdirectory(core)
add_subdirectory(app)
