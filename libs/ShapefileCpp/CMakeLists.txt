cmake_minimum_required(VERSION 3.10)

if (TARGET ocpn::ShapefileCpp)
    return ()
endif ()

project(ShapefileCpp)


list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
#find_package(shapelib REQUIRED)
#find_package(CLI11 REQUIRED)
#find_package(GTest REQUIRED)

#enable_testing()
add_subdirectory(lib)
#add_subdirectory(app)

add_library(ocpn::ShapefileCpp ALIAS ShapefileCppLib)
target_compile_features(ShapefileCppLib PUBLIC cxx_std_17)

set(SHAPEFILECPP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/include")
