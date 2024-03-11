cmake_minimum_required(VERSION 3.5.0)

if (TARGET ocpn::s52plib)
    return ()
endif ()

SET(SRC
    src/chartsymbols.cpp
    src/s52plib.cpp
    src/s52cnsy.cpp
    src/s52utils.cpp
    src/s52shaders.cpp
    src/TexFont.cpp
    src/DepthFont.cpp
    src/mygeom.cpp
    src/color_types.h
)
if (OCPN_USE_GL)
    SET(SRC ${SRC} src/Cs52_shaders.cpp)
endif()

if (NOT wxWidgets_INCLUDE_DIRS)
    include(SimpleWxConfig)
    SimpleWxConfig()
endif ()

add_library(S52PLIB STATIC ${SRC})
add_library(ocpn::s52plib ALIAS S52PLIB)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(S52PLIB PRIVATE -fvisibility=default -fPIC)
endif ()

target_include_directories(S52PLIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Fixme (#3527) Dependency on main opencpn
target_include_directories(
  S52PLIB PRIVATE
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/include
)
if (ANDROID)
  target_include_directories(S52PLIB PRIVATE ${CMAKE_SOURCE_DIR}/android)
endif ()
if (MSVC)
  target_include_directories(S52PLIB PRIVATE ${CMAKE_SOURCE_DIR}/buildwin)
endif ()

target_include_directories(S52PLIB PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(S52PLIB PRIVATE ${wxWidgets_LIBRARIES})

target_link_libraries(S52PLIB PRIVATE ocpn::geoprim)
target_link_libraries(S52PLIB PRIVATE ocpn::gl-headers)
target_link_libraries(S52PLIB PRIVATE ocpn::pugixml)
target_link_libraries(S52PLIB PRIVATE ocpn::gdal)
target_link_libraries(S52PLIB PRIVATE ocpn::tess2)
