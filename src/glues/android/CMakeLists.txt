cmake_minimum_required(VERSION 2.6)

project(glues)

SET(CMAKE_VERBOSE_MAKEFILE ON)

add_definitions(-DANDROID)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Raspberry PI
if(BCMHOST)
    include_directories(/opt/vc/include /opt/vc/include/interface/vcos/pthreads /opt/vc/include/interface/vmcs_host/linux)
    link_directories(/opt/vc/lib)
    add_definitions(-DBCMHOST)
endif()

#link_directories(${CMAKE_BINARY_DIR}/lib)
add_definitions(-g -std=c99 -funwind-tables -O2)


file(GLOB_RECURSE GLUES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../source/*.c")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../source)

add_library(GLUES SHARED ${GLUES_SOURCES})
#target_link_libraries(GL EGL)
add_library(GLUES_static STATIC ${GLUES_SOURCES})
set_target_properties(GLUES_static PROPERTIES OUTPUT_NAME GLUES)


if(BCMHOST)
    set(PI_LIBS bcm_host vcos pthread)
    target_link_libraries(GL ${PI_LIBS} GLESv1_CM)
    target_link_libraries(GL2 ${PI_LIBS} GLESv2)
endif()

#if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#    target_link_libraries(GL X11)
#    target_link_libraries(GL2 X11)
#endif()

#if(CMAKE_SHARED_LIBRARY_SUFFIX MATCHES ".so")
#    set_target_properties(GL PROPERTIES SUFFIX ".so.1")
#    set_target_properties(GL2 PROPERTIES SUFFIX ".so.1")
#endif()


