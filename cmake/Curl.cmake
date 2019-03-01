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

IF (CURL_FOUND)
    return()
ENDIF ()

IF(CMAKE_HOST_WIN32)
  if (NOT EXISTS ${PROJECT_SOURCE_DIR}/buildwin/libcurl.lib)
      message(FATAL_ERROR "Cannot find bundled windows files.")
  endif ()
  ADD_LIBRARY(WIN32_LIBCURL SHARED IMPORTED)
  SET_TARGET_PROPERTIES(WIN32_LIBCURL PROPERTIES
      IMPORTED_IMPLIB ${PROJECT_SOURCE_DIR}/buildwin/libcurl.lib
      IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/buildwin/libcurl.dll
  )
  ADD_LIBRARY(WIN32_ZLIB1 SHARED IMPORTED)
  SET_TARGET_PROPERTIES(WIN32_ZLIB1 PROPERTIES
      IMPORTED_IMPLIB ${PROJECT_SOURCE_DIR}/buildwin/zlib1.lib
      IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/buildwin/zlib1.dll
  )
  IF(MSVC)
    INSTALL(FILES "buildwin/libcurl.dll" DESTINATION ".")
    INSTALL(FILES "buildwin/zlib1.dll" DESTINATION ".")
    #INSTALL(FILES "buildwin/libeay32.dll" DESTINATION ".")
    #INSTALL(FILES "buildwin/ssleay32.dll" DESTINATION ".")
    #INSTALL(FILES "buildwin/curl-ca-bundle.crt" DESTINATION ".")
    SET(CURL_LIBRARIES WIN32_LIBCURL WIN32_ZLIB1)
  ELSE(MSVC)
    # mingw
    SET(CURL_LIBRARIES WIN32_LIBCURL)
    INSTALL(FILES "buildwin/libcurl.dll" DESTINATION ".")
  ENDIF(MSVC)
    
  set(CURL_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/buildwin/include)
  SET(CURL_FOUND 1)
ENDIF(CMAKE_HOST_WIN32)

IF(UNIX)
    FIND_PACKAGE(CURL REQUIRED)
ENDIF(UNIX)
