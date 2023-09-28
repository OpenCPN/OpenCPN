#
# - Try to find libsndfile
# Once done, this will define
#
#  LIBSNDFILE_FOUND - system has libsndfile
#  LIBSNDFILE_INCLUDE_DIRS - the libsndfile include directories
#  LIBSNDFILE_LIBRARIES - link these to use libsndfile
#
#  Copied from:
#  https://github.com/neXyon/audaspace/blob/master/cmake/FindLibSndFile.cmake
#
#  Copyright (c) 2009-2015 Jörg Müller. All rights reserved.
#  License: Apache 2.0 (not distributed in binary packages).

if(APPLE AND OCPN_USE_DEPS_BUNDLE)
  set(LIBSNDFILE_INCLUDE_DIR "${OCPN_DEPS_BUNDLE_PATH}/include")
  set(LIBSNDFILE_LIBRARY "${OCPN_DEPS_BUNDLE_PATH}/lib/libsndfile.dylib")
else()
  # Use pkg-config to get hints about paths
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBSNDFILE_PKGCONF sndfile)
  endif(PKG_CONFIG_FOUND)

  # Include dir
  find_path(LIBSNDFILE_INCLUDE_DIR
	NAMES sndfile.h
  )

  # Library
  find_library(LIBSNDFILE_LIBRARY
	NAMES sndfile libsndfile-1
	PATHS ${LIBSNDFILE_PKGCONF_LIBRARY_DIRS}
  )
endif()

find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(LibSndFile  DEFAULT_MSG  LIBSNDFILE_LIBRARY LIBSNDFILE_INCLUDE_DIR)

if(LIBSNDFILE_FOUND)
  set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIBRARY})
  set(LIBSNDFILE_INCLUDE_DIRS ${LIBSNDFILE_INCLUDE_DIR})
endif(LIBSNDFILE_FOUND)

mark_as_advanced(LIBSNDFILE_LIBRARY LIBSNDFILE_LIBRARIES LIBSNDFILE_INCLUDE_DIR LIBSNDFILE_INCLUDE_DIRS)
