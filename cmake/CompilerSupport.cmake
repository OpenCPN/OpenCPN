#
# Check for supported compiler flags
#

if (_COMPILER_FLAGS_PROBED)
  return ()
endif ()
set(_COMPILER_FLAGS_PROBED 1 CACHE INTERNAL "Multiple run guard")

function (print_ok_fail flag)
  if (${flag})
      message(STATUS "OK")
  else ()
      message(STATUS "Fail")
  endif ()
endfunction (print_ok_fail)

function (print_no_nl message)
  execute_process(COMMAND ${CMAKE_COMMAND} -E echo_append "--" ${message})
endfunction (print_no_nl)

print_no_nl("Probing for compiler -msse support ")
try_compile(HAVE_MSSE ${CMAKE_BINARY_DIR}/msse
            SOURCES  ${CMAKE_SOURCE_DIR}/cmake/dummy.c
            COMPILE_DEFINITIONS -msse)
print_ok_fail(${HAVE_MSSE})

print_no_nl("Probing for compiler -msse2 support ")
try_compile(HAVE_MSSE2 ${CMAKE_BINARY_DIR}/msse2
            SOURCES  ${CMAKE_SOURCE_DIR}/cmake/dummy.c
            COMPILE_DEFINITIONS -msse2)
print_ok_fail(${HAVE_MSSE2})

print_no_nl("Probing for compiler -msse3 support ")
try_compile(HAVE_MSSE3 ${CMAKE_BINARY_DIR}/msse3
            SOURCES  ${CMAKE_SOURCE_DIR}/cmake/dummy.c
            COMPILE_DEFINITIONS -msse3)
print_ok_fail(${HAVE_MSSE3})

print_no_nl("Probing for compiler -mavx2 support ")
try_compile(HAVE_MAVX2 ${CMAKE_BINARY_DIR}/mavx2
            SOURCES  ${CMAKE_SOURCE_DIR}/cmake/dummy.c
            COMPILE_DEFINITIONS -mavx2)
print_ok_fail(${HAVE_MAVX2})

print_no_nl("Probing for compiler -mfpu=neon support ")
try_compile(HAVE_MFPU_NEON ${CMAKE_BINARY_DIR}/fpu_neon
            SOURCES  ${CMAKE_SOURCE_DIR}/cmake/dummy.c
            COMPILE_DEFINITIONS -mfpu=neon)
print_ok_fail(${HAVE_MFPU_NEON})

