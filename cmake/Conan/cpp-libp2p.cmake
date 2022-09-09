# TODO: we don't have Conan package for cpp-libp2p for now
# So we add it as an external project.
set(CPP_LIBP2P_ROOT "${CMAKE_BINARY_DIR}/cpp-libp2p.cmake")
file(MAKE_DIRECTORY "${CPP_LIBP2P_ROOT}")

# Configure external cpp-libp2p
execute_process(
    COMMAND ${CMAKE_COMMAND} "${CMAKE_CURRENT_LIST_DIR}/cpp-libp2p"
    WORKING_DIRECTORY "${CPP_LIBP2P_ROOT}"
)

# Build external cpp-libp2p
include(ProcessorCount)
ProcessorCount(CPU_COUNT)
execute_process(
    COMMAND ${CMAKE_COMMAND} --build "${CPP_LIBP2P_ROOT}" --parallel ${CPU_COUNT}
)

find_package(libp2p CONFIG REQUIRED PATHS "${CPP_LIBP2P_ROOT}/install/lib/cmake/libp2p")
include_directories(BEFORE SYSTEM "${CPP_LIBP2P_ROOT}/install/include")