cmake_minimum_required(VERSION 3.5)

if (TARGET ocpn::tess2)
    return ()
endif ()

project(libtess2)

SET(CMAKE_VERBOSE_MAKEFILE ON)

IF(NOT WIN32 AND NOT APPLE )
    #  ADD_COMPILE_OPTIONS( "-Wall" "-Wno-unused" "-fexceptions" "-rdynamic" )
    #  ADD_COMPILE_OPTIONS( " -g" "-fno-strict-aliasing" "-O2")
ENDIF(NOT WIN32 AND NOT APPLE)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options("-Wno-unknown-pragmas")
endif ()
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang")
    add_compile_options("-Wno-format" "-Wno-unknown-pragmas")
endif ()

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
        SET(CMAKE_EXE_LINKER_FLAGS "/DEBUG ${CMAKE_EXE_LINKER_FLAGS}")
    ENDIF(MSVC)
ENDIF(WIN32)

SET(SRC
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
    Source/Striper.cpp
    Source/CustomArray.cpp
    Source/Adjacency.cpp
    Source/RevisitedRadix.cpp
)
add_library(TESS2 STATIC ${SRC})
add_library(ocpn::tess2 ALIAS TESS2)

# FixME (#3527) Dependency on main CMakeLists.txt
set_target_properties(TESS2 PROPERTIES COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(TESS2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
