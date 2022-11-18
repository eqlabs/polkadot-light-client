# TODO: we don't have Conan package for binaryen for now
# So we add it as an external project.
set(BINARYEN_ROOT "${CMAKE_BINARY_DIR}/binaryen")
file(MAKE_DIRECTORY "${BINARYEN_ROOT}")

# Configure external binaryen
execute_process(
    COMMAND ${CMAKE_COMMAND} "${CMAKE_CURRENT_LIST_DIR}/binaryen" -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    WORKING_DIRECTORY "${BINARYEN_ROOT}"
)

# Build external binaryen
include(ProcessorCount)
ProcessorCount(CPU_COUNT)
execute_process(
    COMMAND ${CMAKE_COMMAND} --build "${BINARYEN_ROOT}" --parallel ${CPU_COUNT}
)

link_directories("${BINARYEN_ROOT}/install/lib")
include_directories(BEFORE SYSTEM "${BINARYEN_ROOT}/install/include")
include_directories(BEFORE SYSTEM "${BINARYEN_ROOT}/src/src")
