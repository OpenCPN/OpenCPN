cmake_minimum_required(VERSION 3.1)

if (TARGET ocpn::easywsclient)
    return ()
endif ()

set(SRC easywsclient.cpp)

set_source_files_properties(
        easywsclient.cpp PROPERTIES COMPILE_FLAGS "-O0")

add_library(EASYWSCLIENT STATIC ${SRC})
add_library(ocpn::easywsclient ALIAS EASYWSCLIENT)

set_property(TARGET EASYWSCLIENT PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(EASYWSCLIENT PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32)
  target_link_libraries(EASYWSCLIENT PRIVATE ws2_32)
endif ()
