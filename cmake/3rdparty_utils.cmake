set(PACKAGE_MANAGER "Conan")

include("${CMAKE_SOURCE_DIR}/cmake/${PACKAGE_MANAGER}/init.cmake")

macro(add_3rdparty PACKAGE_NAME)
    include("${CMAKE_SOURCE_DIR}/cmake/${PACKAGE_MANAGER}/${PACKAGE_NAME}.cmake")
endmacro()
