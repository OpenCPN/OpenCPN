cmake_minimum_required(VERSION 3.5)

if (TARGET ocpn::manual)
    return ()
endif ()

set( SRC
  src/manual.cpp
  src/manual_dlg.cpp
  include/manual.h
  include/manual_dlg.h
)
set(CMAKE_CXX_STANDARD 17)
add_library(_MANUAL STATIC ${SRC})
add_library(ocpn::manual ALIAS _MANUAL)
set_property(TARGET _MANUAL PROPERTY POSITION_INDEPENDENT_CODE ON)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(_MANUAL PRIVATE -fvisibility=default)
endif ()
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  target_compile_options(_MANUAL PRIVATE -x c++)
endif ()


target_include_directories(_MANUAL PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(_MANUAL PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(_MANUAL PRIVATE ${wxWidgets_LIBRARIES})
target_link_libraries(_MANUAL PRIVATE ocpn::filesystem)
target_link_libraries(_MANUAL PRIVATE ocpn::wxjson)
