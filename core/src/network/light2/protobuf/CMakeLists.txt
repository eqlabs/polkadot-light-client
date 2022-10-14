set(LIGHT_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/")

file(MAKE_DIRECTORY "${LIGHT_GENERATED_DIR}")

execute_process(
    COMMAND "${Protobuf_PROTOC_EXECUTABLE}" light.proto --proto_path=${CMAKE_CURRENT_LIST_DIR} --cpp_out=${LIGHT_GENERATED_DIR})

file(GLOB LIGHT_GENERATED_FILES "${LIGHT_GENERATED_DIR}/*pb.cc")
add_library(light2_protobuf ${LIGHT_GENERATED_FILES})
target_link_libraries(light2_protobuf protobuf::libprotobuf)
target_include_directories(light2_protobuf SYSTEM PUBLIC ${LIGHT_GENERATED_DIR})
