cmake_minimum_required(VERSION 2.6)

project(glu)

SET(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_definitions(-g -O2 )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

SET(GLU_SOURCES
    libutil/error.c
    libutil/glue.c
    libutil/quad.c
    libutil/mipmap.c
    libutil/project.c
    libutil/registry.c
    libtess/tess.c
    libtess/mesh.c
    libtess/priorityq.c
    libtess/sweep.c
    libtess/tessmono.c
    libtess/normal.c
    libtess/memalloc.c
    libtess/geom.c
    libtess/dict.c
    libtess/render.c
    )

add_library(GLU_static STATIC ${GLU_SOURCES})
set_target_properties(GLU_static PROPERTIES OUTPUT_NAME GLU)

#SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
#SET(BUILD_SHARED_LIBS "ON")

#add_library(GLU_dynamic SHARED ${GLU_SOURCES})
#set_target_properties(GLU_dynamic PROPERTIES OUTPUT_NAME GLU)
