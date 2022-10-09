cmake_minimum_required(VERSION 3.1.0)

if (TARGET ocpn::geoprim)
    return ()
endif ()

if (NOT CMAKE_MODULE_PATH)
  set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
endif ()

SET(SRC
    src/bbox.cpp
    src/bbox.h
    src/LLRegion.cpp
    src/LLRegion.h
    )

if (NOT wxWidgets_INCLUDE_DIRS)
    include(SimpleWxConfig)
    SimpleWxConfig()
endif ()

add_library(GEOPRIM STATIC ${SRC})
add_library(ocpn::geoprim ALIAS GEOPRIM)

set_property(TARGET GEOPRIM PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(GEOPRIM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(GEOPRIM PRIVATE ${wxWidgets_INCLUDE_DIRS})
