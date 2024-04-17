# FindUsblib-10.cmake
# <https://github.com/nemequ/gnome-cmake>
#
# CMake support for libusb1.0
#
# If  libusb-1.0 is found create the linkable ocpn::libusb interface library
# carrying headers, libraries and compilation flags.
# On a sidenote also sets USBLIB_FOUND.
#
# License:
#
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

if (TARGET ocpn::libusb)
    return ()
endif ()

if(APPLE AND OCPN_USE_DEPS_BUNDLE)
  set(LIBUSB_INCLUDE_DIR "${OCPN_DEPS_BUNDLE_PATH}/include")
  set(LIBUSB_LIBRARIES "${OCPN_DEPS_BUNDLE_PATH}/lib/libusb-1.0.dylib")
else()
  find_package(PkgConfig)
  if(PKG_CONFIG_FOUND)
    pkg_search_module(LIBUSB_PKG libusb-1.0)
  endif()

  find_library(LIBUSB_LIBRARIES
    NAMES libusb-1.0 usb-1.0
    HINTS ${LIBUSB_PKG_LIBRARY_DIRS}
  )
endif()

if (LIBUSB_LIBRARIES)
  add_library(_LIBUSB INTERFACE)
  target_link_libraries(_LIBUSB INTERFACE "${LIBUSB_LIBRARIES}")
  target_compile_options(_LIBUSB INTERFACE "${LIBUSB_PKG_CFLAGS_OTHER}")

  find_path(LIBUSB_INCLUDE_DIR "libusb.h"
            PATH_SUFFIXES libusb-1.0
            HINTS ${LIBUSB_PKG_INCLUDE_DIRS}
  )
  if (LIBUSB_INCLUDE_DIR)
      target_include_directories(_LIBUSB INTERFACE ${LIBUSB_INCLUDE_DIR})
  endif ()
  add_library(ocpn::libusb ALIAS _LIBUSB)
endif()

mark_as_advanced(LIBUSB_INCLUDE_DIR LIBUSB_LIBRARIES)
find_package_handle_standard_args(
    LIBUSB
    REQUIRED_VARS LIBUSB_INCLUDE_DIR LIBUSB_LIBRARIES
)
unset(LIBUSB_DEPS_FOUND_VARS)
