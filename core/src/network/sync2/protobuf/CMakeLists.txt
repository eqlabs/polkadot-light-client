set(SYNC_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/")

file(MAKE_DIRECTORY "${SYNC_GENERATED_DIR}")

execute_process(
        COMMAND "${Protobuf_PROTOC_EXECUTABLE}" sync.proto --proto_path=${CMAKE_CURRENT_LIST_DIR} --cpp_out=${SYNC_GENERATED_DIR})

file(GLOB SYNC_GENERATED_FILES "${SYNC_GENERATED_DIR}/*pb.cc")
add_library(sync2_protobuf ${SYNC_GENERATED_FILES})
target_link_libraries(sync2_protobuf protobuf::libprotobuf)
target_include_directories(sync2_protobuf SYSTEM PUBLIC ${SYNC_GENERATED_DIR})
