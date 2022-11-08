cmake_minimum_required(VERSION 3.14)

project(binaryen)
include(ExternalProject)

set(INSTALL_DIR "${CMAKE_BINARY_DIR}/install")
ExternalProject_Add(binaryen
    GIT_REPOSITORY     "https://${GITHUB_HOST}/WebAssembly/binaryen.git"
    GIT_TAG            "version_100"
    GIT_SHALLOW        TRUE
    DOWNLOAD_DIR       "${CMAKE_BINARY_DIR}/download"
    SOURCE_DIR         "${CMAKE_BINARY_DIR}/src"
    BINARY_DIR         "${CMAKE_BINARY_DIR}/build"
    INSTALL_DIR        "${INSTALL_DIR}"
    PATCH_COMMAND      git apply ${CMAKE_CURRENT_LIST_DIR}/binaryen_plc.patch
    CMAKE_ARGS         -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DBUILD_TESTS=OFF -DBUILD_TOOLS=OFF -DBUILD_STATIC_LIB=ON)
