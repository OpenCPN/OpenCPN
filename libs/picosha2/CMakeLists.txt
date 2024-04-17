cmake_minimum_required(VERSION 3.5.0)

if (TARGET ocpn::pico_sha2)
    return ()
endif ()

add_library(pico_sha2 INTERFACE)
target_include_directories(pico_sha2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
