# ~~~
# Include mdns library from https://github.com/mjansson/mdns and
# add some local add-ons.
#
# The mdns.c file from the library is not compiled and is only
# available as reference. The actually exported stuff is the
# mdns.h header from the package and the files in src which
# is code from mdns.c reimplemented for opencpn.
#
# Exports: ocpn::mdns transitive link target
#
# License:      GPLv3+
# Copyright (c) 2024 Alec Leamas
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

cmake_minimum_required(VERSION 3.10.0)

if (TARGET ocpn::mdns)
  return ()
endif ()

set(MDNS_ROOT ${CMAKE_CURRENT_LIST_DIR}/mdns-1.4.3)

# The overall ocpn::mdns library
#
set(SRC src/mdns_util.cpp include/mdns_util.h)
add_library(_MDNS STATIC ${SRC})
add_library(ocpn::mdns ALIAS _MDNS)
target_include_directories(_MDNS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(_MDNS PUBLIC ${MDNS_ROOT})
if (QT_ANDROID)
  target_include_directories(
    _MDNS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/android
  )
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(_MDNS PRIVATE -fvisibility=default -fPIC)
endif ()
target_compile_definitions(_MDNS PRIVATE HAVE_WXWIDGETS)
