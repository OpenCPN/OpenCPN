cmake_minimum_required(VERSION 3.5.0)

if (TARGET ocpn::geoprim)
  return ()
endif ()

SET(SRC
  src/bbox.cpp
  src/bbox.h
  src/LLRegion.cpp
  src/LLRegion.h
  src/line_clip.cpp
  src/line_clip.h
  src/poly_math.cpp
  src/poly_math.h
  src/LOD_reduce.cpp
  src/LOD_reduce.h
  src/linmath.h
)

add_library(GEOPRIM STATIC ${SRC})
add_library(ocpn::geoprim ALIAS GEOPRIM)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(GEOPRIM PRIVATE -fvisibility=default -fPIC)
endif ()

target_include_directories(GEOPRIM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_include_directories(GEOPRIM PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(GEOPRIM PRIVATE ${wxWidgets_LIBRARIES})
target_link_libraries(GEOPRIM PRIVATE ocpn::gl-headers)

find_package(OpenGL)
if (OPENGL_FOUND)
  target_include_directories(GEOPRIM PRIVATE ${OPENGL_INCLUDE_DIR})
  target_link_libraries(GEOPRIM PRIVATE ${OPENGL_LIBRARIES} )

  message(STATUS "Found OpenGL....")
  message(STATUS "    GL Lib: " ${OPENGL_LIBRARIES})
  message(STATUS "    GL Include: " ${OPENGL_INCLUDE_DIR})
endif ()
