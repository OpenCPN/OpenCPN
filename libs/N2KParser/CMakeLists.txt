cmake_minimum_required(VERSION 3.1.0)

if (TARGET ocpn::N2KParser)
    return ()
endif ()

set (SRC
  src/N2KParser.cpp
  include/N2KParser.h
  src/N2kMessages.cpp
  src/N2kMsg.cpp
)
add_library(N2KPARSER STATIC ${SRC})
add_library(ocpn::N2KParser ALIAS N2KPARSER)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(
    N2KPARSER PRIVATE
      -fvisibility=default -Wno-unknown-pragmas -fPIC
  )
endif ()
target_include_directories(
  N2KPARSER PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
