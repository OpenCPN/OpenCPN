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
if (DEFINED ENV{FLATPAK_ID})
  target_compile_definitions(tests PRIVATE OCPN_FLATPAK)
endif ()

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
if (UNIX AND NOT DEFINED ENV{FLATPAK_ID})
  set(IPC_SRV_TESTS_SRC
    ipc-srv-tests.cpp
    ${CMAKE_SOURCE_DIR}/src/local_api.cpp
    ${CMAKE_SOURCE_DIR}/src/ipc_api.cpp
    ${CMAKE_SOURCE_DIR}/src/ocpn_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/base_platform.cpp
    ${CMAKE_SOURCE_DIR}/src/logger.cpp
  )
  if (APPLE)
    list(APPEND IPC_SRV_TESTS_SRC ../src/macutils.c)
  endif ()
  add_executable(ipc-srv-tests ${IPC_SRV_TESTS_SRC})

  # See note in ipc-srv-tests.cpp
  target_compile_options(ipc-srv-tests PRIVATE -O0)
  target_link_libraries(ipc-srv-tests PRIVATE observable::observable)
  target_link_libraries(ipc-srv-tests PRIVATE ${wxWidgets_LIBRARIES})
  target_link_libraries(ipc-srv-tests PRIVATE ocpn::gtest)
  target_include_directories(ipc-srv-tests PRIVATE ../include)
  add_dependencies(ipc-srv-tests cli-server ipc-client)
endif ()

target_compile_definitions(tests
  PUBLIC
    CLIAPP USE_MOCK_DEFS CMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}"
    TESTDATA="${CMAKE_CURRENT_LIST_DIR}/testdata"
)

if (MSVC)
  target_link_libraries(tests
	  PRIVATE setupapi.lib psapi.lib ${CMAKE_SOURCE_DIR}/cache/buildwin/iphlpapi.lib
 )
endif ()
if (APPLE)
  # 10.13 compat support
  target_link_libraries(tests PUBLIC BOOST_SYSTEM BOOST_FILESYSTEM)
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
if (LINUX)
  target_link_libraries(tests PRIVATE glib::glib)
endif ()
target_link_libraries(tests PRIVATE observable::observable)
target_link_libraries(tests PRIVATE ocpn::easywsclient)
target_link_libraries(tests PRIVATE ocpn::filesystem)
target_link_libraries(tests PRIVATE ocpn::garminhost)
target_link_libraries(tests PRIVATE ocpn::gdal)
target_link_libraries(tests PRIVATE ocpn::geoprim)
target_link_libraries(tests PRIVATE ocpn::iso8211)
target_link_libraries(tests PRIVATE ocpn::libarchive)
target_link_libraries(tests PRIVATE ocpn::mDNS)
target_link_libraries(tests PRIVATE ocpn::mongoose)
target_link_libraries(tests PRIVATE ocpn::N2KParser)
target_link_libraries(tests PRIVATE ocpn::nmea0183)
target_link_libraries(tests PRIVATE ocpn::opencpn)
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

if (LINUX)
  add_executable(dbus_tests
    dbus_tests.cpp
    ${CMAKE_SOURCE_DIR}/src/dbus_client.cpp
    ${CMAKE_SOURCE_DIR}/src/dbus_server.cpp
    ${CMAKE_SOURCE_DIR}/src/ipc_api.cpp
    ${CMAKE_SOURCE_DIR}/src/local_api.cpp
    ${CMAKE_SOURCE_DIR}/src/ocpn_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/base_platform.cpp
    ${CMAKE_SOURCE_DIR}/src/logger.cpp
  )
 target_link_libraries(dbus_tests PRIVATE observable::observable)
  target_link_libraries(dbus_tests PRIVATE ${wxWidgets_LIBRARIES})
  target_link_libraries(dbus_tests PRIVATE ocpn::gtest)
  target_link_libraries(dbus_tests PRIVATE glib::glib)
  target_include_directories(dbus_tests PRIVATE ../include)
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
find_program(DBUS_SEND NAMES dbus-send)
if (DBUS_SEND)
  target_compile_definitions(tests PRIVATE HAVE_DBUS_SEND)
endif ()
if (GLIB_FOUND)
  target_compile_definitions(tests PRIVATE HAVE_GLIB)
endif ()


target_link_libraries(tests PRIVATE ocpn::gtest)
include(GoogleTest)
gtest_add_tests(TARGET tests)
if (UNIX AND NOT DEFINED ENV{FLATPAK_ID})
  gtest_add_tests(TARGET ipc-srv-tests)
  # We don't have a session bus available when testing flatpak
  # so these can just be run in native builds
  gtest_add_tests(TARGET dbus_tests)
endif ()

add_executable(cli-server cli_server.cpp ../src/ocpn_utils.cpp)
target_link_libraries(cli-server PRIVATE ${wxWidgets_LIBRARIES})
target_include_directories(cli-server PRIVATE ../include)

add_executable(ipc-client ipc_client.cpp ../src/ocpn_utils.cpp)
target_link_libraries(ipc-client PRIVATE ${wxWidgets_LIBRARIES})
target_link_libraries(ipc-client PRIVATE observable::observable)
target_include_directories(ipc-client  PRIVATE ../include)


set (WX_INSTANCE_SRC
  wx_instance.cpp
  ${CMAKE_SOURCE_DIR}/src/base_platform.cpp
  ${CMAKE_SOURCE_DIR}/src/ocpn_utils.cpp
  ${CMAKE_SOURCE_DIR}/src/logger.cpp
)
if (APPLE)
  list(APPEND WX_INSTANCE_SRC ../src/macutils.c)
endif ()
add_executable(wx-instance ${WX_INSTANCE_SRC})
target_link_libraries(wx-instance PRIVATE ${wxWidgets_LIBRARIES})
target_link_libraries(wx-instance PRIVATE ocpn::opencpn)
target_include_directories(wx-instance PRIVATE ../include)
if (MSVC)
  target_link_libraries(wx-instance PRIVATE setupapi)
endif ()

if (UNIX)
  set(STD_INSTANCE_SRC
    std_instance.cpp
    ${CMAKE_SOURCE_DIR}/src/std_instance_chk.cpp
    ${CMAKE_SOURCE_DIR}/src/base_platform.cpp
    ${CMAKE_SOURCE_DIR}/src/ocpn_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/logger.cpp
  )
  if (APPLE)
    list(APPEND STD_INSTANCE_SRC ../src/macutils.c)
  endif ()
  add_executable(std-instance ${STD_INSTANCE_SRC})
  target_link_libraries(std-instance PRIVATE ${wxWidgets_LIBRARIES})
  target_include_directories(std-instance PRIVATE ../include)
  add_test(NAME tests COMMAND tests)
endif ()

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
PATH=%PATH%\;${_cache_dir}\\\\wxWidgets-3.2.4\\\\lib\\\\vc14x_dll\")
")
  file(WRITE ${_script_path} ${_script})
  add_custom_target(win-path-setup
    COMMAND cmake -P "${_script_path}"
    COMMAND cmake -E echo "Creating ${_bat_path}"
  )
endif ()

add_dependencies(tests cli-server ipc-client)
