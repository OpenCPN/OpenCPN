# ~~~
# Build the basic s57 library
#
# Exports: ocpn::s57-charts transitive link target
#
# License:      GPLv3+
# Copyright (c) 2024 Alec Leamas
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

cmake_minimum_required(VERSION 3.5.0)

if (TARGET ocpn::s57-charts)
    return ()
endif ()

set(SRC
  src/ogrs57datasource.cpp
  src/ogrs57layer.cpp
  src/s57classregistrar.cpp
  src/s57featuredefns.cpp
  src/s57reader.cpp
  src/s57registrar_mgr.cpp
)

set(HEADERS
  include/ogr_s57.h
  include/s57class_registrar.h
  include/s57.h
  include/s57registrar_mgr.h
)



add_library(_S57_CHARTS STATIC ${SRC} ${HEADERS})
add_library(ocpn::s57-charts ALIAS _S57_CHARTS)

target_include_directories(_S57_CHARTS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(_S57_CHARTS ocpn::iso8211 ocpn::gdal)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(_S57_CHARTS PRIVATE -fvisibility=default -fPIC)
endif ()
