cmake_minimum_required(VERSION 3.1.0)

if (TARGET ocpn::mipmap)
    return ()
endif ()

if (NOT CMAKE_MODULE_PATH)
  set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
endif ()

include(GetArch)
GetArch()

set(SRC
  include/mipmap/mipmap.h
  src/mipmap.c
  src/mipmap_sse.c
  src/mipmap_sse2.c
  src/mipmap_ssse3.c
  src/mipmap_avx2.c
  src/mipmap_neon.c
)

add_library(MIPMAP STATIC ${SRC})
add_library(ocpn::mipmap ALIAS MIPMAP)
target_include_directories(MIPMAP
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/mipmap
)

# always use optimizations for the texture compression which is much faster
if (NOT MSVC)
    set_property(TARGET MIPMAP PROPERTY COMPILE_FLAGS "-fvisibility=hidden -O3")

    if ((ARCH MATCHES "i386" OR ARCH MATCHES "amd64" OR ARCH MATCHES "x86_64")
         AND NOT QT_ANDROID
    )
        set_source_files_properties(
            src/mipmap_sse.c PROPERTIES COMPILE_FLAGS "-msse")
        set_source_files_properties(
            src/mipmap_sse2.c PROPERTIES COMPILE_FLAGS "-msse2")
        set_source_files_properties(
            src/mipmap_ssse3.c PROPERTIES COMPILE_FLAGS "-mssse3")
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.7)
                # require at least gcc 4.8
                set_source_files_properties(
                    src/mipmap_avx2.c PROPERTIES COMPILE_FLAGS "-mavx2")
            endif ()
        endif ()
    else ()
        if(NOT (ARCH MATCHES "arm64" OR ARCH MATCHES "aarch64"))
            set_source_files_properties(
                src/mipmap_neon.c PROPERTIES COMPILE_FLAGS "-mfpu=neon")
        endif ()
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
