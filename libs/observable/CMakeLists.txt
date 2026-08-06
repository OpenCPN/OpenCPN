cmake_minimum_required(VERSION 3.10.0)

if (TARGET observable::observable)
  return ()
endif ()

set(SRC
  include/observable/configvar.h
  include/observable/event.h
  include/observable/evtvar.h
  include/observable/global_var.h
  include/observable/observable.h
  src/observable_confvar.cpp
  src/observable.cpp
)
add_library(_OBSERVABLE STATIC ${SRC})
add_library(observable::observable ALIAS _OBSERVABLE)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(_OBSERVABLE PRIVATE -fvisibility=default -fPIC)
endif ()

target_include_directories(
  _OBSERVABLE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(
  _OBSERVABLE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/observable
)
