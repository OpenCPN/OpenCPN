cmake_minimum_required(VERSION 3.7)

if (TARGET ocpn::ShapefileCpp)
    return ()
endif ()

project(ShapefileCpp)

add_definitions("-std=c++17")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
#find_package(shapelib REQUIRED)
#find_package(CLI11 REQUIRED)
#find_package(GTest REQUIRED)

#enable_testing()
add_subdirectory(lib)
#add_subdirectory(app)

add_library(ocpn::ShapefileCpp ALIAS ShapefileCppLib)

set(SHAPEFILECPP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/include")
