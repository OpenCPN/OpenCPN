cmake_minimum_required(VERSION 3.5)

if (TARGET ocpn::texcmp)
    return ()
endif ()

option(USE_SYSTEM_LZ4 "Use system lz4 if available" ON)
option(HAVE_MSSE2  "Use sse2 optimizations" OFF)

if (NOT DEFINED ARCH)
  set(ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
  if ("${ARCH}" STREQUAL "AMD64")
    set(ARCH x86_64)
  endif()
endif ()

include(FindPkgConfig)

# Fixme (#3527) dependency on main opencpn
include(CompilerSupport)

set(SRC
    squish/alpha.cpp
    squish/clusterfit.cpp
    squish/colourblock.cpp
    squish/colourfit.cpp
    squish/colourset.cpp
    squish/maths.cpp
    squish/rangefit.cpp
    squish/singlecolourfit.cpp
    squish/singlecolourfitfast.cpp
    squish/twocolourfitfast.cpp
    squish/squish.cpp
    etcpak.cpp
)

add_library(TEXCMP STATIC ${SRC})
add_library(ocpn::texcmp  ALIAS TEXCMP)

IF( NOT QT_ANDROID)
  if (NOT wxWidgets_INCLUDE_DIRS)
    find_package(GTK3)
    if (GTK3_FOUND)
        target_include_directories(TEXCMP PRIVATE ${GTK3_INCLUDE_DIRS})
        set(GTK_LIBRARIES ${GTK3_LIBRARIES})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__WXGTK3__")
        set(wxWidgets_CONFIG_OPTIONS
            ${wxWidgets_CONFIG_OPTIONS} --toolkit=gtk3)
        message(STATUS "Building against GTK3...")
    endif ()
    find_package(wxWidgets REQUIRED)
  endif ()
  include(${wxWidgets_USE_FILE})
ENDIF ()

target_include_directories(TEXCMP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/squish)
target_include_directories(TEXCMP PRIVATE ${wxWidgets_INCLUDE_DIRS})

pkg_search_module(LZ4 liblz4 lz4)
if (LZ4_FOUND AND USE_SYSTEM_LZ4)
    message(STATUS "Building with system lz4")
    include_directories(${LZ4_INCLUDE_DIR})
    if (COMMAND target_link_directories)
      target_link_directories(TEXCMP PRIVATE ${LZ4_LIBRARY_DIRS})
    else ()
      link_directories(${LZ4_LIBRARY_DIRS})
    endif ()
else ()
    message(STATUS "Building with bundled lz4")
    set(SRC ${SRC} lz4/lz4.c lz4/lz4hc.c)
    target_include_directories(
      TEXCMP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lz4
    )
endif ()

# always use optimizations for the texture compression which is much faster

IF (NOT MSVC)
  IF (HAVE_MSSE2 AND NOT QT_ANDROID)
    # Using sse which makes the compression run about 50% faster
    message (STATUS "Compiling texture compression library with sse support")
    set_property(
      TARGET TEXCMP PROPERTY COMPILE_FLAGS
          "-fvisibility=hidden -O3 -msse2 -DSQUISH_USE_SSE=2 -fPIC"
    )
  ELSE ()
    # use standard optimizations for other architectures
    set_property(TARGET TEXCMP PROPERTY COMPILE_FLAGS "-O3 -fPIC")
  ENDIF ()
ELSE (NOT MSVC)
  # try to use sse on x86 based systems
  IF (ARCH MATCHES "i386" OR ARCH MATCHES "amd64" OR ARCH MATCHES "x86_64")
    # using sse which makes the compression run about 50% faster
    message (STATUS "Compiling texture compression library with sse support")
    set_property(TARGET TEXCMP PROPERTY COMPILE_FLAGS "/arch:SSE2 -DSQUISH_USE_SSE=2")
  ENDIF ()
ENDIF (NOT MSVC)
