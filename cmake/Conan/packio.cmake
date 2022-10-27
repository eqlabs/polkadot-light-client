# TODO: we don't have Conan package for cpp-libp2p for now
# So we add it as an external project.
set(PACKIO_ROOT "${CMAKE_BINARY_DIR}/packio")
# file(MAKE_DIRECTORY "${PACKIO_ROOT}")
#     WORKING_DIRECTORY "${PACKIO_ROOT}"


message(STATUS "---------------------- in packio.cmake")

execute_process(
    COMMAND git clone https://github.com/qchateau/packio.git
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

execute_process(
    COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/packio/packio_plc.patch
    WORKING_DIRECTORY ${PACKIO_ROOT}
)

# https://github.com/qchateau/packio

# set(SCALE_GITHUB_HOST "github.com" CACHE STRING "Host for scale-codec-cpp repo (maybe overriden to provide custom ssh key)")

# Configure external scale
# execute_process(
#     COMMAND ${CMAKE_COMMAND} "${CMAKE_CURRENT_LIST_DIR}/packio" }
#     WORKING_DIRECTORY "${PACKIO_ROOT}"
# )

# Build external cpp-libp2p
# include(ProcessorCount)
# ProcessorCount(CPU_COUNT)
# execute_process(
#     COMMAND ${CMAKE_COMMAND} --build "${SCALE_ROOT}" --parallel ${CPU_COUNT}
# )

# find_package(scale CONFIG REQUIRED PATHS "${SCALE_ROOT}/install/lib/cmake/scale")
include_directories(BEFORE SYSTEM "${PACKIO_ROOT}/include")
