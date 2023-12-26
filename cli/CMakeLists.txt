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
if (LIBELF_INCLUDE_DIR AND LIBELF_LIBRARY)
  target_include_directories(opencpn-cmd PUBLIC "${LIBELF_INCLUDE_DIR}")
  target_link_libraries(opencpn-cmd PRIVATE "${LIBELF_LIBRARY}")
endif ()

target_link_libraries(opencpn-cmd PRIVATE ocpn::opencpn)
target_link_libraries(opencpn-cmd PRIVATE ocpn::model)

target_link_libraries(opencpn-cmd PRIVATE ocpn::wxjson)

target_link_libraries(opencpn-cmd PRIVATE observable::observable)
target_link_libraries(opencpn-cmd PRIVATE ocpn::easywsclient)
target_link_libraries(opencpn-cmd PRIVATE ocpn::garminhost)
target_link_libraries(opencpn-cmd PRIVATE ocpn::gdal)
target_link_libraries(opencpn-cmd PRIVATE ocpn::geoprim)
target_link_libraries(opencpn-cmd PRIVATE ocpn::iso8211)
target_link_libraries(opencpn-cmd PRIVATE ocpn::libarchive)
target_link_libraries(opencpn-cmd PRIVATE ocpn::mongoose)
target_link_libraries(opencpn-cmd PRIVATE ocpn::N2KParser)
target_link_libraries(opencpn-cmd PRIVATE ocpn::nmea0183)
target_link_libraries(opencpn-cmd PRIVATE pico_sha2)
target_link_libraries(opencpn-cmd PRIVATE ocpn::pugixml)
target_link_libraries(opencpn-cmd PRIVATE ocpn::rapidjson)
target_link_libraries(opencpn-cmd PRIVATE ocpn::s52plib)
target_link_libraries(opencpn-cmd PRIVATE ocpn::serial)
target_link_libraries(opencpn-cmd PRIVATE ocpn::sound)
target_link_libraries(opencpn-cmd PRIVATE ocpn::tinyxml)
target_link_libraries(opencpn-cmd PRIVATE ocpn::wxjson)
target_link_libraries(opencpn-cmd PRIVATE ocpn::filesystem)

if (TARGET glib::glib)
target_link_libraries(opencpn-cmd PRIVATE glib::glib)
endif ()
if (HAVE_LIBUDEV)
  target_link_libraries(opencpn-cmd PRIVATE ocpn::libudev)
endif ()
if (LIBLZMA_FOUND)
  if (TARGET LibLZMA::LibLZMA)
    target_link_libraries(opencpn-cmd PRIVATE LibLZMA::LibLZMA)
  else ()
    target_link_libraries(opencpn-cmd PRIVATE ${LIBLZMA_LIBRARIES})
  endif ()
endif ()

if(APPLE AND OCPN_USE_DEPS_BUNDLE)
  target_include_directories(
    opencpn-cmd
      PUBLIC
        ${OCPN_DEPS_BUNDLE_PATH}/include
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
