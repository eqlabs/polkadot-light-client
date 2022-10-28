
# We only use boost json with packio
set (PACKIO_HAS_MSGPACK 0)
set (PACKIO_HAS_NLOHMANN_JSON 0)
set (PACKIO_HAS_BOOST_JSON 1)

execute_process(
    COMMAND git clone https://github.com/qchateau/packio.git
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

set(PACKIO_ROOT "${CMAKE_BINARY_DIR}/packio")
execute_process(
    COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/packio/packio_plc.patch
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/packio
)

include_directories(BEFORE SYSTEM "${PACKIO_ROOT}/include")
