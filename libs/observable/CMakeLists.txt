cmake_minimum_required(VERSION 3.1.0)

if (TARGET observable::observable)
  return ()
endif ()

set(SRC
  include/observable_confvar.h
  include/observable_evt.h
  include/observable_evtvar.h
  include/observable_globvar.h
  include/observable.h
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
