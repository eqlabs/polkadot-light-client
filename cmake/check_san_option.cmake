function(check_san_option value)
  if (value STREQUAL "${SAN}")
    set("SAN_OPTION" 1 PARENT_SCOPE)
    return()
  endif()
  set("SAN_OPTION" 0 PARENT_SCOPE)
endfunction()