# FindPango.cmake
# <https://github.com/nemequ/gnome-cmake>
#
# CMake support for Pango.
#
# If pango is found create the linkable ocpn::pango interface library
# carrying headers, libraries and compilation flags.
# On a sidenote also sets PANGO_FOUND.
#
# License:
#
#   Copyright (c) 2016 Evan Nemerson <evan@nemerson.com>
#   Copyright (c) 2019 Alec Leamas
#
#   Permission is hereby granted, free of charge, to any person
#   obtaining a copy of this software and associated documentation
#   files (the "Software"), to deal in the Software without
#   restriction, including without limitation the rights to use, copy,
#   modify, merge, publish, distribute, sublicense, and/or sell copies
#   of the Software, and to permit persons to whom the Software is
#   furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be
#   included in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.

if (TARGET ocpn::pango)
    return ()
endif ()

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_search_module(PANGO_PKG pango)
endif()


find_library(PANGO_LIBRARIES pango-1.0 HINTS ${PANGO_PKG_LIBRARY_DIRS})
set(PANGO pango-1.0)

if(PANGO_LIBRARIES AND NOT PANGO_FOUND)
  add_library(_PANGO INTERFACE)
  target_link_libraries(_PANGO INTERFACE "${PANGO_LIBRARIES}")
  target_compile_options(_PANGO INTERFACE "${PANGO_PKG_CFLAGS_OTHER}")

  find_path(PANGO_INCLUDE_DIR "pango/pango.h"
      PATH_SUFFIXES pango-1.0
      HINTS ${PANGO_PKG_INCLUDE_DIRS})
  if(PANGO_INCLUDE_DIR)
    file(STRINGS "${PANGO_INCLUDE_DIR}/pango/pango-features.h"
       PANGO_MAJOR_VERSION REGEX
       "^#define PANGO_VERSION_MAJOR +\\(?([0-9]+)\\)?$")
    string(REGEX REPLACE
       "^#define PANGO_VERSION_MAJOR \\(?([0-9]+)\\)?" "\\1"
       PANGO_MAJOR_VERSION "${PANGO_MAJOR_VERSION}")
    file(STRINGS "${PANGO_INCLUDE_DIR}/pango/pango-features.h"
       PANGO_MINOR_VERSION REGEX
       "^#define PANGO_VERSION_MINOR +\\(?([0-9]+)\\)?$")
    string(REGEX REPLACE
       "^#define PANGO_VERSION_MINOR \\(?([0-9]+)\\)?" "\\1"
       PANGO_MINOR_VERSION "${PANGO_MINOR_VERSION}")
    file(STRINGS "${PANGO_INCLUDE_DIR}/pango/pango-features.h"
       PANGO_MICRO_VERSION REGEX
       "^#define PANGO_VERSION_MICRO +\\(?([0-9]+)\\)?$")
    string(REGEX REPLACE
       "^#define PANGO_VERSION_MICRO \\(?([0-9]+)\\)?" "\\1"
       PANGO_MICRO_VERSION "${PANGO_MICRO_VERSION}")
    set(
      PANGO_VERSION
      "${PANGO_MAJOR_VERSION}.${PANGO_MINOR_VERSION}.${PANGO_MICRO_VERSION}")
    unset(PANGO_MAJOR_VERSION)
    unset(PANGO_MINOR_VERSION)
    unset(PANGO_MICRO_VERSION)

    target_include_directories(_PANGO INTERFACE ${PANGO_INCLUDE_DIR})
    if (NOT PANGO_VERSION VERSION_LESS "1.44")
      find_package(HarfBuzz REQUIRED)
      target_include_directories(_PANGO INTERFACE ${HarfBuzz_INCLUDE_DIRS})
      target_link_libraries(_PANGO INTERFACE ${HarfBuzz_LIBRARIES})
    endif()
  endif()
  add_library(ocpn::pango ALIAS _PANGO)
endif()

mark_as_advanced(PANGO_INCLUDE_DIR PANGO_LIBRARIES PANGO_VERSION)
find_package_handle_standard_args(PANGO
    REQUIRED_VARS PANGO_INCLUDE_DIR PANGO_LIBRARIES
    VERSION_VAR PANGO_VERSION
)

unset(PANGO_DEPS_FOUND_VARS)
