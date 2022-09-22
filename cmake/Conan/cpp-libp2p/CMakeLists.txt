cmake_minimum_required(VERSION 3.14)

project(cpp-libp2p)
include(ExternalProject)

message(STATUS "---- cpp-libp2p/cmakefiles.txt-----")
message(STATUS "PLC_PROFILE_PATH is ${PLC_PROFILE_PATH}")


set(INSTALL_DIR "${CMAKE_BINARY_DIR}/install")
ExternalProject_Add(cpp-libp2p
    GIT_REPOSITORY     "git@github.com:eqlabs/cpp-libp2p.git"
    GIT_TAG            "cmake-propagation"
    GIT_SHALLOW        TRUE
    DOWNLOAD_DIR       "${CMAKE_BINARY_DIR}/download"
    SOURCE_DIR         "${CMAKE_BINARY_DIR}/src"
    BINARY_DIR         "${CMAKE_BINARY_DIR}/build"
    INSTALL_DIR        "${INSTALL_DIR}"
    CMAKE_ARGS         -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DPACKAGE_MANAGER=Conan -DPLC_PROFILE_PATH=${PLC_PROFILE_PATH} )
