set(LIGHT_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/")

file(MAKE_DIRECTORY "${LIGHT_GENERATED_DIR}")

add_custom_target(
    generate_light2_protobuf
    COMMAND "${Protobuf_PROTOC_EXECUTABLE}" schema.proto --proto_path="${CMAKE_CURRENT_LIST_DIR}" --cpp_out="${LIGHT_GENERATED_DIR}"
    SOURCES schema.proto)
include_directories(SYSTEM ${LIGHT_GENERATED_DIR})

file(GLOB LIGHT_GENERATED_FILES "${LIGHT_GENERATED_DIR}/*pb.cc")
add_library(light2_protobuf ${LIGHT_GENERATED_FILES})
target_link_libraries(light2_protobuf protobuf::libprotobuf)
add_dependencies(light2_protobuf generate_light2_protobuf)
