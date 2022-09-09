add_conan_package(andreasbuhr-cppcoro cci.20210113)
find_package(cppcoro MODULE REQUIRED)
include_directories(BEFORE SYSTEM ${cppcoro_INCLUDE_DIRS})
