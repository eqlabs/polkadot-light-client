add_conan_package(msgpack 3.2.1)
find_package(msgpack MODULE REQUIRED)
include_directories(BEFORE SYSTEM ${msgpack_INCLUDE_DIRS})
