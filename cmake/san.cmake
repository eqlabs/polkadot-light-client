include(${CMAKE_CURRENT_LIST_DIR}/check_san_option.cmake)

check_san_option("ASAN")
set(ASAN ${SAN_OPTION})
check_san_option("LSAN")
set(LSAN ${SAN_OPTION})
check_san_option("MSAN")
set(MSAN ${SAN_OPTION})
check_san_option("TSAN")
set(TSAN ${SAN_OPTION})
check_san_option("UBSAN")
set(UBSAN ${SAN_OPTION})

if(ASAN)
  print("Address Sanitizer is enabled")
  include(${CMAKE_CURRENT_LIST_DIR}/toolchain/san/asan.cmake)
endif()
if(LSAN)
  print("Leak Sanitizer is enabled")
  include(${CMAKE_CURRENT_LIST_DIR}/toolchain/san/lsan.cmake)
endif()
if(MSAN)
  print("Memory Sanitizer is enabled")
  include(${CMAKE_CURRENT_LIST_DIR}/toolchain/san/msan.cmake)
endif()
if(TSAN)
  print("Thread Sanitizer is enabled")
  include(${CMAKE_CURRENT_LIST_DIR}/toolchain/san/tsan.cmake)
endif()
if(UBSAN)
  print("Undefined Behavior Sanitizer is enabled")
  include(${CMAKE_CURRENT_LIST_DIR}/toolchain/san/ubsan.cmake)
endif()
