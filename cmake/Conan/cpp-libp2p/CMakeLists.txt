cmake_minimum_required(VERSION 3.14)

project(cpp-libp2p)
include(ExternalProject)

set(INSTALL_DIR "${CMAKE_BINARY_DIR}/install")
ExternalProject_Add(cpp-libp2p
    GIT_REPOSITORY     "git@github.com:eqlabs/cpp-libp2p.git"
    GIT_TAG            "master"
    GIT_SHALLOW        TRUE
    DOWNLOAD_DIR       "${CMAKE_BINARY_DIR}/download"
    SOURCE_DIR         "${CMAKE_BINARY_DIR}/src"
    BINARY_DIR         "${CMAKE_BINARY_DIR}/build"
    INSTALL_DIR        "${INSTALL_DIR}"
    CMAKE_ARGS         -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DPACKAGE_MANAGER=Conan -DSAN=${SAN} -DSORALOG_GITHUB_HOST=${SORALOG_GITHUB_HOST} -DSQLITE_GITHUB_HOST=${SQLITE_GITHUB_HOST})
