cmake_minimum_required(VERSION 2.6)

project(libtess2)

SET(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})


IF(NOT WIN32 AND NOT APPLE )
  ADD_DEFINITIONS( "-Wall -Wno-unused -fexceptions -rdynamic" )
  ADD_DEFINITIONS( " -g -fno-strict-aliasing -O2")
ENDIF(NOT WIN32 AND NOT APPLE)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(Include)

IF(WIN32)
    IF(MSVC)
        SET(CMAKE_C_FLAGS_DEBUG               "/MDd /Ob0 /Od  /D_DEBUG  /Zi /RTC1" )
        SET(CMAKE_C_FLAGS_MINSIZEREL          "/MD  /O1  /Ob1 /D NDEBUG")
        SET(CMAKE_C_FLAGS_RELEASE             "/MD  /O2  /Ob2 /D NDEBUG /Zi")
        SET(CMAKE_C_FLAGS_RELWITHDEBINFO      "/MD  /O2  /Ob1 /D NDEBUG /Zi")
        SET(CMAKE_CXX_FLAGS_DEBUG             "/MDd /Ob0 /Od  /D_DEBUG  /Zi /RTC1 /EHa")
        SET(CMAKE_CXX_FLAGS_MINSIZEREL        "/MD  /O1  /Ob1 /D NDEBUG /EHa")
        SET(CMAKE_CXX_FLAGS_RELEASE           "/MD  /O2  /Ob2 /D NDEBUG /Zi /EHa")
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO    "/MD  /O2  /Ob1 /D NDEBUG /Zi /EHa" )
        SET(CMAKE_EXE_LINKER_FLAGS /DEBUG)

    ENDIF(MSVC)
ENDIF(WIN32)

SET(LIBTESS2_SOURCES
    Source/bucketalloc.c
    Source/bucketalloc.h
    Source/dict.c
    Source/dict.h
    Source/geom.c  
    Source/geom.h  
    Source/mesh.c  
    Source/mesh.h  
    Source/priorityq.c  
    Source/priorityq.h  
    Source/sweep.c  
    Source/sweep.h  
    Source/tess.c  
    Source/tess.h
    )

add_library(LIBTESS2_static STATIC ${LIBTESS2_SOURCES})
set_target_properties(LIBTESS2_static PROPERTIES OUTPUT_NAME LIBTESS2)
 
