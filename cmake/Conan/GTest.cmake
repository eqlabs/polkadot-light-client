add_conan_package(gtest cci.20210126)
find_package(GTest MODULE REQUIRED)
include_directories(BEFORE SYSTEM ${GTest_INCLUDE_DIRS})
