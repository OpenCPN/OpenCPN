#
# Find the cairo libraries. On exit sets linkable library ocpn::cairo
#

if (CAIRO_INCLUDE_DIRS)
  # Already in cache, be silent
  set(CAIRO_FIND_QUIETLY TRUE)
endif ()

if (CMAKE_HOST_WIN32)
  set(CAIRO_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/cache/buildwin/gtk/include)
else (CMAKE_HOST_WIN32)
  set(CAIRO_INC_LOOK_PATHS /usr/local/include /usr/include)
  if (APPLE)
    set(CAIRO_INC_LOOK_PATHS /usr/local/include /usr/include /opt/local/include)
    if (NOT APPLE_MODERN)
      file(GLOB _cairo_home /usr/local/Cellar/cairo/*/include)
      set(CAIRO_INC_LOOK_PATHS ${_cairo_home})
      set(LOOK_OPTION NO_DEFAULT_PATH)
    endif ()
  endif (APPLE)
  find_path(CAIRO_INCLUDE_DIRS cairo.h
            PATHS ${CAIRO_INC_LOOK_PATHS}
            PATH_SUFFIXES cairo/ libcairo/ cairo/libcairo/
            ${LOOK_OPTION})
endif (CMAKE_HOST_WIN32)
message(STATUS " Cairo includes: ${CAIRO_INCLUDE_DIRS}")

if (CMAKE_HOST_WIN32)
  set(CAIRO_LIBRARIES
    ${CMAKE_SOURCE_DIR}/cache/buildwin/gtk/cairo.lib
    ${CMAKE_SOURCE_DIR}/cache/buildwin/archive.lib
  )
  set(CAIRO_FOUND 1)
else (CMAKE_HOST_WIN32)
  set(CAIRO_LIB_LOOK_PATHS ${LINUX_LIB_PATHS})
  if (APPLE AND NOT APPLE_MODERN)
    set(CAIRO_LIB_LOOK_PATHS /usr/local/Cellar/cairo/1.14.6/lib /opt/local/lib)
    set(LOOK_OPTION NO_DEFAULT_PATH)
  endif ()
  find_library(CAIRO_LIBRARIES
      NAMES libcairo cairo
      PATHS ${CAIRO_LIB_LOOK_PATHS}
      ${LOOK_OPTION}
  )
  message(STATUS " Cairo library found: ${CAIRO_LIBRARIES}")
  # handle the QUIETLY and REQUIRED arguments and set CAIRO_FOUND to TRUE if
  # all listed variables are TRUE
  include( "FindPackageHandleStandardArgs" )
  find_package_handle_standard_args("CAIRO"
      DEFAULT_MSG CAIRO_INCLUDE_DIRS CAIRO_LIBRARIES
  )
  mark_as_advanced(CAIRO_INCLUDE_DIRS CAIRO_LIBRARIES )
endif ()

if (NOT CAIRO_FOUND)
  message(FATAL_ERROR "Cairo component required, but not found!")
endif ()

add_library(_CAIRO INTERFACE)
# Some systems (e.g ARMHF RPI2) require some extra libraries
# This is not exactly a general solution, but probably harmless where
# not needed.
if (NOT APPLE AND NOT WIN32)
  find_package(PANGO REQUIRED)
  target_link_libraries(_CAIRO INTERFACE ocpn::pango)
  find_library(PANGOFT2_LIBRARY NAMES pangoft2-1.0 PATHS ${LINUX_LIB_PATHS})
  find_library(PANGOXFT_LIBRARY NAMES pangoxft-1.0 PATHS ${LINUX_LIB_PATHS})
  find_library(
    GDK_PIXBUF_LIBRARY
    NAMES gdk_pixbuf-2.0 PATHS ${LINUX_LIB_PATHS}
  )
  find_package_handle_standard_args("CAIRO_EXTRAS" DEFAULT_MSG
    PANGOFT2_LIBRARY PANGOXFT_LIBRARY GDK_PIXBUF_LIBRARY
  )
endif ()

if (CAIRO_EXTRAS_FOUND)
  set(CAIRO_LIBRARIES ${CAIRO_LIBRARIES}
    ${PANGOFT2_LIBRARY} ${PANGOXFT_LIBRARY}
    ${GDK_PIXBUF_LIBRARY}
  )
endif ()
target_link_libraries(_CAIRO INTERFACE ${CAIRO_LIBRARIES})

target_include_directories(_CAIRO INTERFACE ${CAIRO_INCLUDE_DIRS})
if (APPLE)
  target_include_directories(_CAIRO INTERFACE ${CAIRO_INCLUDE_DIRS}/..)
endif ()
if (HarfBuzz_FOUND)
    target_include_directories(_CAIRO INTERFACE ${HarfBuzz_INCLUDE_DIRS})
    target_link_libraries(_CAIRO INTERFACE ${Harfbuzz_LIBRARIES})
endif ()

add_library(ocpn::cairo ALIAS _CAIRO)
