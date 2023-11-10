cmake_minimum_required(VERSION 3.5)

if (TARGET ocpn::wxservdisc)
    return ()
endif ()

set(SRC
  mdnsd.c
  1035.c
  wxServDisc.cpp
  )

add_library(WXSERVDISC STATIC ${SRC})
add_library(ocpn::wxservdisc ALIAS WXSERVDISC)

set_property(TARGET WXSERVDISC PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(WXSERVDISC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(
    WXSERVDISC PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-pointer-sign>
  )
  target_compile_options(WXSERVDISC PRIVATE -Wno-parentheses)
endif ()

if (WIN32)
  target_link_libraries(WXSERVDISC PRIVATE ws2_32)
endif ()
