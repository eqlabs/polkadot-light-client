add_conan_package(nlohmann_json 3.9.1)
find_package(nlohmann_json MODULE REQUIRED)
include_directories(BEFORE SYSTEM ${nlohmann_json_INCLUDE_DIRS})
