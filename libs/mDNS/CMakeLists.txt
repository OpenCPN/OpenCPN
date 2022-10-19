cmake_minimum_required(VERSION 3.1.0)

if (TARGET ocpn::mDNS)
    return ()
endif ()

set (SRC
  src/mdns_util.cpp
  include/mdns_util.h
  include/mdns.h
)
add_library(MDNS STATIC ${SRC})
target_include_directories(MDNS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_property(TARGET MDNS PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
add_library(ocpn::mDNS ALIAS MDNS)
