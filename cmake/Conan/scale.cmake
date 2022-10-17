# TODO: we don't have Conan package for cpp-libp2p for now
# So we add it as an external project.
set(SCALE_ROOT "${CMAKE_BINARY_DIR}/scale")
file(MAKE_DIRECTORY "${SCALE_ROOT}")

set(SCALE_GITHUB_HOST "github.com" CACHE STRING "Host for scale-codec-cpp repo (maybe overriden to provide custom ssh key)")

# Configure external scale
execute_process(
    COMMAND ${CMAKE_COMMAND} "${CMAKE_CURRENT_LIST_DIR}/scale" -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DGITHUB_HOST=${SCALE_GITHUB_HOST}
    WORKING_DIRECTORY "${SCALE_ROOT}"
)

# Build external cpp-libp2p
include(ProcessorCount)
ProcessorCount(CPU_COUNT)
execute_process(
    COMMAND ${CMAKE_COMMAND} --build "${SCALE_ROOT}" --parallel ${CPU_COUNT}
)

find_package(scale CONFIG REQUIRED PATHS "${SCALE_ROOT}/install/lib/cmake/scale")
include_directories(BEFORE SYSTEM "${SCALE_ROOT}/install/include")
