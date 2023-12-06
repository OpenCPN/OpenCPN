cmake_minimum_required(VERSION 3.14)

project(opencpn_tests)
set(CMAKE_CXX_STANDARD 17)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
message(STATUS "Building tests")


enable_testing ()

set(PROJ_SRC ${PROJECT_SOURCE_DIR}/../src)

set(SRC
  tests.cpp
  ${MODEL_SRC}
  ${CMAKE_SOURCE_DIR}/src/api_shim.cpp
  ${CMAKE_SOURCE_DIR}/src/base_platform.cpp
  ${CMAKE_SOURCE_DIR}/src/certificates.cpp
  ${CMAKE_SOURCE_DIR}/src/mDNS_query.cpp
)

if (LINUX)
  list(APPEND SRC n2k_tests.cpp)
endif ()

if (MSVC)
  set(CURL_HINTS C:\\mingw64\\bin C:\\ProgramData\\chocolatey\\bin)
endif ()
find_program(CURL NAMES curl HINTS ${CURL_HINTS})

if (CURL)
  list(APPEND SRC rest-tests.cpp)
endif ()

add_executable(tests ${SRC})

if (CURL)
  target_compile_definitions(tests PUBLIC CURLPROG="${CURL}")
endif ()

if (NOT MSVC)
  target_compile_options(tests PUBLIC "-O0")
endif ()
target_compile_definitions(tests
  PUBLIC
    CLIAPP USE_MOCK_DEFS CMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}"
    TESTDATA="${CMAKE_CURRENT_LIST_DIR}/testdata"
    UNIT_TESTS
)
if (MSVC)
  target_link_libraries(tests
	  PRIVATE setupapi.lib psapi.lib ${CMAKE_SOURCE_DIR}/cache/buildwin/iphlpapi.lib
  )
endif ()
target_include_directories(
  tests
  PRIVATE
  ${PROJECT_SOURCE_DIR}/../include
  ${PROJECT_SOURCE_DIR}/include
  ${CMAKE_BINARY_DIR}/include
  ${PROJECT_SOURCE_DIR}/../libs/sound/include
  ${PROJECT_SOURCE_DIR}/../libs/sound/include
  ${PROJECT_SOURCE_DIR}/../buildandroid/libcurl/include
)
if(APPLE AND OCPN_USE_DEPS_BUNDLE)
  target_include_directories(
    tests
    PRIVATE
    ${OCPN_DEPS_BUNDLE_PATH}/include
  )
endif()

if (CMAKE_VERSION VERSION_GREATER 3.4)
  if (NOT "${ENABLE_SANITIZER}" MATCHES "none")
    target_link_libraries(tests
        PRIVATE -fsanitize=${ENABLE_SANITIZER}
    )
  endif ()
endif ()

target_link_libraries(tests PRIVATE ${wxWidgets_LIBRARIES})
if (DEFINED LIBELF_LIBRARY)
  target_link_libraries(tests PRIVATE "${LIBELF_LIBRARY}")
endif ()

target_link_libraries(tests PRIVATE observable::observable)
target_link_libraries(tests PRIVATE ocpn::easywsclient)
target_link_libraries(tests PRIVATE ocpn::garminhost)
target_link_libraries(tests PRIVATE ocpn::gdal)
target_link_libraries(tests PRIVATE ocpn::geoprim)
target_link_libraries(tests PRIVATE ocpn::iso8211)
target_link_libraries(tests PRIVATE ocpn::libarchive)
target_link_libraries(tests PRIVATE ocpn::mDNS)
target_link_libraries(tests PRIVATE ocpn::mongoose)
target_link_libraries(tests PRIVATE ocpn::N2KParser)
target_link_libraries(tests PRIVATE ocpn::nmea0183)
target_link_libraries(tests PRIVATE pico_sha2)
target_link_libraries(tests PRIVATE ocpn::pugixml)
target_link_libraries(tests PRIVATE ocpn::rapidjson)
target_link_libraries(tests PRIVATE ocpn::s52plib)
target_link_libraries(tests PRIVATE ocpn::serial)
target_link_libraries(tests PRIVATE ocpn::tinyxml)
target_link_libraries(tests PRIVATE ocpn::wxjson)

if (DEFINED CURL_LIBRARIES)
  target_link_libraries(tests PRIVATE ${CURL_LIBRARIES})
endif ()
if (DEFINED WXCURL_LIBRARIES)
  target_link_libraries(tests PRIVATE ${WXCURL_LIBRARIES})
elseif (DEFINED WXSYS_CURL_LIBRARIES)
  target_link_libraries(tests PRIVATE ${SYS_WXCURL_LIBRARIES})
elseif (TARGET ocpn::wxcurl)
  target_link_libraries(tests PRIVATE ocpn::wxcurl)
endif ()

if (DEFINED LIBELF_LIBRARY)
  target_link_libraries(tests PRIVATE ${LIBELF_LIBRARY})
endif ()
if (HAVE_LIBUDEV)
  target_link_libraries(tests PRIVATE ocpn::libudev)
endif ()
if (LIBLZMA_FOUND)
  if (TARGET LibLZMA::LibLZMA)
    target_link_libraries(tests PRIVATE LibLZMA::LibLZMA)
  else ()
    target_link_libraries(tests PRIVATE ${LIBLZMA_LIBRARIES})
  endif ()
endif ()

if (NOT WIN32)
  if(APPLE AND OCPN_USE_DEPS_BUNDLE)
    set(OPENSSL_ROOT_DIR ${OCPN_DEPS_BUNDLE_PATH})
  endif()
  find_package(OpenSSL)
  if (OPENSSL_FOUND)
    message(STATUS "OpenSSL found   ${OPENSSL_INCLUDE_DIR} ${OPENSSL_LIBRARIES}")
    target_include_directories(tests PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(tests PRIVATE ${OPENSSL_LIBRARIES})
    add_definitions(-DMG_ENABLE_OPENSSL)
  endif (OPENSSL_FOUND)
else (NOT WIN32)
    target_include_directories(
      tests
      PRIVATE ${CMAKE_SOURCE_DIR}/cache/buildwin/include/openssl
    )
    target_link_libraries(
      tests
      PRIVATE ${CMAKE_SOURCE_DIR}/cache/buildwin/libssl.lib
    )
   target_link_libraries(
      tests
      PRIVATE ${CMAKE_SOURCE_DIR}/cache/buildwin/libcrypto.lib
    )
endif (NOT WIN32)


target_link_libraries(tests PRIVATE ocpn::gtest)
include(GoogleTest)
gtest_discover_tests(tests)


add_test(NAME tests COMMAND tests)

# Create a batch file which can be used to add paths to downloaded
# stuff so the tests runs on windows. Assumes things in top_dir\cache
# This is not that intelligent and needs manual updates when wxWidgets
# version is updated
if (MSVC)
  set(_cache_dir ${CMAKE_SOURCE_DIR}\\\\cache)
  set(_script_path ${CMAKE_BINARY_DIR}\\set_test_path.cmake)
  set(_bat_path ${CMAKE_BINARY_DIR}\\\\set_test_path.bat)
set(_script
"file(WRITE ${_bat_path} \"
PATH=%PATH%\;${_cache_dir}\\\\buildwin
PATH=%PATH%\;${_cache_dir}\\\\wxWidgets-3.2.3\\\\lib\\\\vc14x_dll\")
")
  file(WRITE ${_script_path} ${_script})
  add_custom_target(win-path-setup
    COMMAND cmake -P "${_script_path}"
    COMMAND cmake -E echo "Creating ${_bat_path}"
  )
endif ()
