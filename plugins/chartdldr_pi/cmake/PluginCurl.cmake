##---------------------------------------------------------------------------
## Author:      Sean D'Epagnier
## Copyright:   
## License:     GPLv3+
##---------------------------------------------------------------------------

# configure curl library

INCLUDE_DIRECTORIES(src/wxcurl)
IF(WIN32)
  IF(MSVC)
    SET(CURL_LIBRARIES "../buildwin/libcurl")
    INSTALL(FILES "buildwin/libcurl.dll" DESTINATION ".")
    INSTALL(FILES "buildwin/libeay32.dll" DESTINATION ".")
    INSTALL(FILES "buildwin/ssleay32.dll" DESTINATION ".")
    INSTALL(FILES "buildwin/zlib.dll" DESTINATION ".")
    INSTALL(FILES "buildwin/curl-ca-bundle.crt" DESTINATION ".")

  ELSE(MSVC) ## mingw
    SET(CURL_LIBRARIES "curl.dll")
    INSTALL(FILES "buildwin/libcurl.dll" DESTINATION ".")
  ENDIF(MSVC)
    
  TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${CURL_LIBRARIES})
  INCLUDE_DIRECTORIES(src/wxcurl/include)
ENDIF(WIN32)

IF(UNIX)
    FIND_PACKAGE(CURL REQUIRED)
    INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${CURL_LIBRARIES} )
ENDIF(UNIX)
