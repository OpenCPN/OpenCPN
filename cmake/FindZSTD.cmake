# - Try to find  zstd library
# Defines ZSTD_FOUND and the interface library ocpn::zstd.

if(APPLE AND OCPN_USE_DEPS_BUNDLE)
  set(ZSTD_INCLUDE_DIR "${OCPN_DEPS_BUNDLE_PATH}/include")
  set(ZSTD_LIBRARY "${OCPN_DEPS_BUNDLE_PATH}/lib/libzstd.dylib")
else()
  find_path(ZSTD_INCLUDE_DIR NAMES zstd.h)

  find_library(ZSTD_LIBRARY NAMES zstd zstd_static)
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  ZSTD
  DEFAULT_MSG
  ZSTD_LIBRARY ZSTD_INCLUDE_DIR
)

if (ZSTD_FOUND)
  add_library(_zstd INTERFACE)
  target_link_libraries(_zstd INTERFACE ${ZSTD_LIBRARY})
  target_include_directories(_zstd INTERFACE ${ZSTD_INCLUDE_DIR})
  add_library(zstd::zstd ALIAS _zstd)
endif()

mark_as_advanced(ZSTD_INCLUDE_DIR ZSTD_LIBRARY)
