# ~~~
# Summary:      pugixml header-only library
# License:      GPLv3+
# Copyright (c) 2022 Dave Register
#
# ~~~

cmake_minimum_required(VERSION 3.5)

if (TARGET ocpn::pugixml)
  return ()
endif ()

set(SRC pugixml.cpp)

add_library(OCPN_PUGIXML STATIC ${SRC})
add_library(ocpn::pugixml ALIAS OCPN_PUGIXML)
target_include_directories(OCPN_PUGIXML INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
