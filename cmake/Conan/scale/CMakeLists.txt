cmake_minimum_required(VERSION 3.14)

project(scale)
include(ExternalProject)

set(INSTALL_DIR "${CMAKE_BINARY_DIR}/install")
ExternalProject_Add(cpp-libp2p
    GIT_REPOSITORY     "git@github.com:eqlabs/scale-codec-cpp.git"
    GIT_TAG            "bump_boost_version"
    GIT_SHALLOW        TRUE
    DOWNLOAD_DIR       "${CMAKE_BINARY_DIR}/download"
    SOURCE_DIR         "${CMAKE_BINARY_DIR}/src"
    BINARY_DIR         "${CMAKE_BINARY_DIR}/build"
    INSTALL_DIR        "${INSTALL_DIR}"
    CMAKE_ARGS         -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DPACKAGE_MANAGER=Conan)
