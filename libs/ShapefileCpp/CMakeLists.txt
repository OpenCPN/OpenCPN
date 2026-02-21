# ~~~
# Add linkage to the ShapeFileCpp library
#
# On hosts having the package installed in system locations found by
# cmake use the system code. If not, download sources from ShapeFileCpp
# github site and apply patches.
#
# Exports: ocpn::shapefile_cpp  transitive link target
#
# License:      GPLv3+
# Copyright (c) 2023 Alec Leamas
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

cmake_minimum_required(VERSION 3.10.0)

if (TARGET ocpn::shapefile_cpp)
  return ()
endif ()

list(APPEND patches
  0001-ShapeFileReader-Fix-annoying-name-clash-add-accessor.patch
  0002-Point-Fix-gcc-initializer-ordering-warning.patch
  0003-ShapeFileReader-Fix-signedness-gcc-warning.patch
  0004-cmake-Only-build-the-lib-directory.patch
)
set(orig_src_dir ${CMAKE_CURRENT_LIST_DIR}/ShapeFileCpp-b2681f8)
set(src_dir ${CMAKE_BINARY_DIR}/b2681f8)

if (POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif ()

# Copy sources and apply patches
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${orig_src_dir} ${src_dir}
  COMMAND ${CMAKE_COMMAND} -E echo "PatchFileCpp: using patched sources at ${src_dir}"
)
foreach (patch ${patches})
  execute_process(
    COMMAND ${CMAKE_COMMAND}
      -Dpatch_file=${CMAKE_CURRENT_SOURCE_DIR}/patches/${patch}
      -Dpatch_dir=${src_dir}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PatchFile.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "-- Applying patch ${patch}"
  )
endforeach ()

set(ShapefileCppLib_SOURCES
  ${src_dir}/lib/src/ShapefileReader.cpp
  ${src_dir}/lib/src/Schema.cpp
  ${src_dir}/lib/src/FeatureIterator.cpp
  ${src_dir}/lib/src/GeometryType.cpp
  ${src_dir}/lib/src/Bounds.cpp
  ${src_dir}/lib/src/Geometry.cpp
  ${src_dir}/lib/src/Point.cpp
  ${src_dir}/lib/src/Line.cpp
  ${src_dir}/lib/src/Ring.cpp
  ${src_dir}/lib/src/Polygon.cpp
  ${src_dir}/lib/src/MultiPoint.cpp
  ${src_dir}/lib/src/MultiLine.cpp
  ${src_dir}/lib/src/MultiPolygon.cpp
  ${src_dir}/lib/src/Feature.cpp
  ${src_dir}/lib/src/FieldType.cpp
  ${src_dir}/lib/src/Field.cpp
)

add_library(ShapefileCppLib STATIC ${ShapefileCppLib_SOURCES})
add_library(ocpn::shapefile_cpp ALIAS ShapefileCppLib)

target_include_directories(ShapefileCppLib
	PUBLIC ${src_dir}/lib/include
	PRIVATE ${src_dir}/lib/src
)
target_link_libraries(ShapefileCppLib ocpn::gdal ocpn::shapelib)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # The calloc usage looks OK, but the macro stuff seems to confuse gcc
  target_compile_options(
    ShapefileCppLib PRIVATE -Wno-calloc-transposed-args
  )
endif ()
