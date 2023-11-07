cmake_minimum_required(VERSION 3.1.0)

if (TARGET ocpn::mipmap)
    return ()
endif ()

if (NOT CMAKE_MODULE_PATH)
  set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
endif ()

include(GetArch)
GetArch()

include(CompilerSupport)

set(SRC
  include/mipmap/mipmap.h
  src/mipmap.c
)

if( NOT QT_ANDROID)
    set(SRC_IPML
        src/mipmap_sse.c
        src/mipmap_sse2.c
        src/mipmap_ssse3.c
        src/mipmap_avx2.c
        src/mipmap_neon.c
    )
endif( NOT QT_ANDROID)

add_library(MIPMAP STATIC ${SRC} ${SRC_IPML})
add_library(ocpn::mipmap ALIAS MIPMAP)
target_include_directories(MIPMAP
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/mipmap
)

if( NOT QT_ANDROID)
  # always use optimizations for the texture compression which is much faster
  if (NOT MSVC)
      set_property(TARGET MIPMAP PROPERTY COMPILE_FLAGS "-fvisibility=hidden -O3")
      if (NOT APPLE)
          set_source_files_properties(
            src/mipmap.c PROPERTIES COMPILE_FLAGS "-march=native")
      endif()
      if (HAVE_MSSE)
          message(STATUS "mipmap SSE support enabled")
          set_source_files_properties(
              src/mipmap_sse.c PROPERTIES COMPILE_FLAGS "-msse")
      endif ()
      if (HAVE_MSSE2)
          message(STATUS "mipmap SSE2 support enabled")
          set_source_files_properties(
              src/mipmap_sse2.c PROPERTIES COMPILE_FLAGS "-msse2")
      endif ()
      if (HAVE_MSSSE3)
          message(STATUS "mipmap SSSE3 support enabled")
          set_source_files_properties(
              src/mipmap_ssse3.c PROPERTIES COMPILE_FLAGS "-mssse3")
      endif ()
      if (HAVE_MAVX2)
          message(STATUS "mipmap AVX2 support enabled")
          set_source_files_properties(
              src/mipmap_avx2.c PROPERTIES COMPILE_FLAGS "-mavx2")
      endif ()
      if (HAVE_MFPU_NEON)
          message(STATUS "mipmap NEON support enabled")
          set_source_files_properties(
              src/mipmap_neon.c PROPERTIES COMPILE_FLAGS "-mfpu=neon")
      endif ()
  else (NOT MSVC)
      # try to use sse on x86 based systems
      if (ARCH MATCHES "i386" OR ARCH MATCHES "amd64" OR ARCH MATCHES "x86_64")
          set_source_files_properties(
              src/mipmap_sse.c PROPERTIES COMPILE_FLAGS "/arch:SSE")
          set_source_files_properties(
              src/mipmap_sse2.c PROPERTIES COMPILE_FLAGS "/arch:SSE2")
          set_source_files_properties(
              src/mipmap_avx2.c PROPERTIES COMPILE_FLAGS "/arch:AVX")
      endif ()
  endif (NOT MSVC)
else (NOT QT_ANDROID)
  set_source_files_properties(
      src/mipmap/mipmap_neon.c
      PROPERTIES COMPILE_FLAGS "-mfloat-abi=softfp -mfpu=neon")
endif (NOT QT_ANDROID)
