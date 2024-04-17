cmake_minimum_required(VERSION 3.5.0)

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

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(MDNS PRIVATE -fvisibility=default -fPIC)
endif ()

if (NOT "${wxWidgets_LIBRARIES}" STREQUAL "")
  target_include_directories(MDNS PRIVATE ${wxWidgets_INCLUDE_DIRS})
  target_link_libraries(MDNS PRIVATE ${wxWidgets_LIBRARIES})
  target_compile_definitions(MDNS PRIVATE HAVE_WXWIDGETS)
endif ()

add_library(ocpn::mDNS ALIAS MDNS)
