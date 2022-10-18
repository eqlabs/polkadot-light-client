# TODO: we don't have Conan package for binaryen for now
# So we add it as an external project.
set(BINARYEN_ROOT "${CMAKE_BINARY_DIR}/binaryen")
file(MAKE_DIRECTORY "${BINARYEN_ROOT}")

set(BINARYEN_GITHUB_HOST "github.com" CACHE STRING "Host for binaryen repo")

# Configure external binaryen
execute_process(
    COMMAND ${CMAKE_COMMAND} "${CMAKE_CURRENT_LIST_DIR}/binaryen" -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DGITHUB_HOST=${BINARYEN_GITHUB_HOST}
    WORKING_DIRECTORY "${BINARYEN_ROOT}"
)

# Build external binaryen
include(ProcessorCount)
ProcessorCount(CPU_COUNT)
execute_process(
    COMMAND git submodule init
    COMMAND git submodule update
    COMMAND ${CMAKE_COMMAND} --build "${BINARYEN_ROOT}" --parallel ${CPU_COUNT}
)

find_package(binaryen CONFIG REQUIRED PATHS "${BINARYEN_ROOT}/install/lib/cmake/binaryen")
include_directories(BEFORE SYSTEM "${BINARYEN_ROOT}/install/include")
