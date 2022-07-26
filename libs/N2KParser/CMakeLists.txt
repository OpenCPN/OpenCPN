cmake_minimum_required(VERSION 3.1.0)

if (TARGET ocpn::N2KParser)
    return ()
endif ()

set (SRC
  src/N2KParser.cpp
  include/N2KParser.h
)
add_library(N2KPARSER STATIC ${SRC})
set_property(TARGET N2KPARSER PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(N2KPARSER PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(ocpn::N2KParser ALIAS N2KPARSER)
