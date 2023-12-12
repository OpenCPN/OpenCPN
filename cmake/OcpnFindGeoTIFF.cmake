#
# Find the GeoTIFF libraries. On exit sets linkable library ocpn::geotiff
#

if (GEOTIFF_INCLUDE_DIRS)
  # Already in cache, be silent
  set(GEOTIFF_FIND_QUIETLY TRUE)
endif ()

if (CMAKE_HOST_WIN32)
  set(GEOTIFF_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/cache/buildwin/include)
else (CMAKE_HOST_WIN32)
  set(GEOTIFF_INC_LOOK_PATHS /usr/local/include /usr/include)
  if (APPLE)
    set(GEOTIFF_INC_LOOK_PATHS /usr/local/include /usr/include /opt/local/include)
    if (NOT APPLE_MODERN)
      file(GLOB _geotiff_home /usr/local/Cellar/libgeotiff/*/include)
      set(GEOTIFF_INC_LOOK_PATHS ${_geotiff_home})
      set(LOOK_OPTION NO_DEFAULT_PATH)
    endif ()
  endif (APPLE)
  find_path(GEOTIFF_INCLUDE_DIRS geotiff.h
            PATHS ${GEOTIFF_INC_LOOK_PATHS}
            PATH_SUFFIXES geotiff/ libgeotiff/ geotiff/libgeotiff/
            ${LOOK_OPTION})
endif (CMAKE_HOST_WIN32)
message(STATUS " Geotiff includes: ${GEOTIFF_INCLUDE_DIRS}")

if (CMAKE_HOST_WIN32)
  set(GEOTIFF_LIBRARIES
    ${CMAKE_SOURCE_DIR}/cache/buildwin/geotiff.lib
    ${CMAKE_SOURCE_DIR}/cache/buildwin/proj.lib
    ${CMAKE_SOURCE_DIR}/cache/buildwin/tiff.lib
  )
  set(GEOTIFF_FOUND 1)
else (CMAKE_HOST_WIN32)
  set(GEOTIFF_LIB_LOOK_PATHS ${LINUX_LIB_PATHS})
  if (APPLE AND NOT APPLE_MODERN)
    set(GEOTIFF_LIB_LOOK_PATHS /usr/local/Cellar/geotiff/1.7.1_3/lib /opt/local/lib)
    set(LOOK_OPTION NO_DEFAULT_PATH)
  endif ()
  find_library(GEOTIFF_LIBRARIES
      NAMES libgeotiff geotiff
      PATHS ${GEOTIFF_LIB_LOOK_PATHS}
      ${LOOK_OPTION}
  )
  message(STATUS " GeOTIFF library found: ${GEOTIFF_LIBRARIES}")
  set(TIFF_LIB_LOOK_PATHS ${LINUX_LIB_PATHS})
  if (APPLE AND NOT APPLE_MODERN)
    set(TIFF_LIB_LOOK_PATHS /usr/local/Cellar/libtiff/4.6.0/lib /opt/local/lib)
    set(LOOK_OPTION NO_DEFAULT_PATH)
  endif ()
  find_library(TIFF_LIBRARIES
      NAMES libtiff tiff
      PATHS ${TIFF_LIB_LOOK_PATHS}
      ${LOOK_OPTION}
  )
  message(STATUS " TIFF library found: ${TIFF_LIBRARIES}")
  # handle the QUIETLY and REQUIRED arguments and set GEOTIFF_FOUND to TRUE if
  # all listed variables are TRUE
  include( "FindPackageHandleStandardArgs" )
  find_package_handle_standard_args("GEOTIFF"
      DEFAULT_MSG GEOTIFF_INCLUDE_DIRS GEOTIFF_LIBRARIES
  )
  mark_as_advanced(GEOTIFF_INCLUDE_DIRS GEOTIFF_LIBRARIES )
  find_package_handle_standard_args("TIFF"
      DEFAULT_MSG TIFF_INCLUDE_DIRS TIFF_LIBRARIES
  )
  mark_as_advanced(TIFF_INCLUDE_DIRS TIFF_LIBRARIES )
endif ()

if (NOT GEOTIFF_FOUND)
  message(FATAL_ERROR "GeoTIFF component required, but not found!")
endif ()

add_library(_GEOTIFF INTERFACE)

target_link_libraries(_GEOTIFF INTERFACE ${GEOTIFF_LIBRARIES} ${TIFF_LIBRARIES})

target_include_directories(_GEOTIFF INTERFACE ${GEOTIFF_INCLUDE_DIRS})
if (APPLE)
  target_include_directories(_GEOTIFF INTERFACE ${GEOTIFF_INCLUDE_DIRS}/..)
endif ()

add_library(ocpn::geotiff ALIAS _GEOTIFF)
