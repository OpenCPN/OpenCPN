set(SRC_CONSOLE console.cpp api_shim.cpp)
add_executable(opencpn-cmd ${SRC_CONSOLE})

if (NOT "${ENABLE_SANITIZER}" MATCHES "none")
  target_link_libraries(opencpn-cmd PRIVATE -fsanitize=${ENABLE_SANITIZER})
endif ()

target_include_directories(
  opencpn-cmd
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include
      # ${PROJECT_SOURCE_DIR}/test/include
      #      ${PROJECT_SOURCE_DIR}/model/include
      ${PROJECT_SOURCE_DIR}/gui/include 
)

target_compile_definitions(opencpn-cmd PUBLIC USE_MOCK_DEFS)
if (MSVC)
  target_link_libraries(opencpn-cmd PRIVATE setupapi.lib psapi.lib)
endif ()

if (TARGET ocpn::wxcurl)
  target_link_libraries(opencpn-cmd PRIVATE ocpn::wxcurl)
else ()
  target_link_libraries(opencpn-cmd PRIVATE ${SYS_WXCURL_LIBRARIES})
endif ()

target_link_libraries(opencpn-cmd PRIVATE ${wxWidgets_LIBRARIES})
if (TARGET ocpn::wxsvg)
  target_link_libraries(opencpn-cmd PRIVATE ocpn::wxsvg)
endif ()

target_link_libraries(opencpn-cmd PRIVATE ocpn::model)

target_link_libraries(opencpn-cmd PRIVATE ocpn::serial)
target_link_libraries(opencpn-cmd PRIVATE ocpn::tinyxml)
target_link_libraries(opencpn-cmd PRIVATE ocpn::filesystem)

if (HAVE_LIBUDEV)
  target_link_libraries(opencpn-cmd PRIVATE ocpn::libudev)
endif ()

if(APPLE AND OCPN_USE_DEPS_BUNDLE)
  target_include_directories(
    opencpn-cmd PUBLIC ${OCPN_DEPS_BUNDLE_PATH}/include
  )
endif()

if (NOT WIN32)
  if(APPLE AND OCPN_USE_DEPS_BUNDLE)
    set(OPENSSL_ROOT_DIR ${OCPN_DEPS_BUNDLE_PATH})
  endif()
  find_package(OpenSSL)
  if (OPENSSL_FOUND)
    message(STATUS
      "OpenSSL found ${OPENSSL_INCLUDE_DIR} ${OPENSSL_LIBRARIES}"
    )
    target_include_directories(opencpn-cmd PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_include_directories(${PACKAGE_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(opencpn-cmd PRIVATE ${OPENSSL_LIBRARIES})
    add_definitions(-DMG_ENABLE_OPENSSL)
  endif ()
else ()
  target_include_directories(
    opencpn-cmd
    PRIVATE ${CMAKE_SOURCE_DIR}/cache/buildwin/include/openssl
  )
  target_link_libraries(
    opencpn-cmd
    PRIVATE ${CMAKE_SOURCE_DIR}/cache/buildwin/libssl.lib
  )
 target_link_libraries(
    opencpn-cmd
    PRIVATE ${CMAKE_SOURCE_DIR}/cache/buildwin/libcrypto.lib
  )
endif ()

install(TARGETS opencpn-cmd RUNTIME DESTINATION ${PREFIX_BIN})

if (APPLE)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/opencpn-cmd
    DESTINATION "bin/OpenCPN.app/Contents/MacOS"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
      WORLD_EXECUTE GROUP_EXECUTE OWNER_EXECUTE
  )
endif () 
