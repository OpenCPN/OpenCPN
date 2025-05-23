##---------------------------------------------------------------------------
## Author:      Pavel Kalian, Sean D'Epagnier
## Copyright:
## License:     GPLv3+
##
## Invoke with windows libs available in ${PROJECT_SOURCE_DIR}/buildwin/
## After invocation defines:
##    CURL_INCLUDE_DIRS   - headers location
##    CURL_LIBRARIES      - libraries, full path(s).
##    CURL_FOUND          - reflects found or not.
##
## As a side-effect, windows dlls are installed in main build directory.
##---------------------------------------------------------------------------

# configure curl library

if (CURL_FOUND)
    return()
endif ()

if (CMAKE_HOST_WIN32)
  if (NOT EXISTS ${PROJECT_SOURCE_DIR}/cache/buildwin/libcurl.lib)
      message(FATAL_ERROR "Cannot find bundled windows files.")
  endif ()
  add_library(WIN32_LIBCURL SHARED IMPORTED)
  set_target_properties(WIN32_LIBCURL PROPERTIES
      IMPORTED_IMPLIB ${PROJECT_SOURCE_DIR}/cache/buildwin/libcurl.lib
      IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/cache/buildwin/libcurl.dll
  )
  add_library(WIN32_ZLIB1 SHARED IMPORTED)
  set_target_properties(WIN32_ZLIB1 PROPERTIES
      IMPORTED_IMPLIB ${PROJECT_SOURCE_DIR}/cache/buildwin/zlib1.lib
      IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/cache/buildwin/zlib1.dll
  )
  set(CURL_LIBRARIES WIN32_LIBCURL WIN32_ZLIB1)
  set(CURL_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/cache/buildwin/include)
  set(CURL_FOUND 1)
  install(
    FILES
      "${CMAKE_SOURCE_DIR}/cache/buildwin/curl-ca-bundle.crt"
      "${CMAKE_SOURCE_DIR}/cache/buildwin/libeay32.dll"
      "${CMAKE_SOURCE_DIR}/cache/buildwin/ssleay32.dll"
	    "${CMAKE_SOURCE_DIR}/cache/buildwin/libcurl.dll"
    DESTINATION "."
  )

endif ()

if (UNIX)
  find_package(CURL REQUIRED)
endif()
