cmake_minimum_required(VERSION 2.6)

project(glshim)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    link_directories(/opt/X11/lib)
endif()

link_directories(${CMAKE_BINARY_DIR}/lib)
add_definitions(-O2)
include(flags.cmake)

include_directories(include)
add_subdirectory(src)
