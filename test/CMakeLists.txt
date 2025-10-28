cmake_minimum_required(VERSION 3.14)

project(opencpn_tests)
set(CMAKE_CXX_STANDARD 17)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
message(STATUS "Building tests")

enable_testing ()

add_library(win32_libs INTERFACE)
if (MSVC)
  target_link_libraries(win32_libs
    INTERFACE
      setupapi.lib psapi.lib
      ${CMAKE_SOURCE_DIR}/cache/buildwin/iphlpapi.lib
  )
endif ()

set(MODEL_SRC_DIR ${CMAKE_SOURCE_DIR}/model/src)

set(SRC tests.cpp filter_tests.cpp ${CMAKE_SOURCE_DIR}/cli/api_shim.cpp)
if ("${OCPN_WX_VERSION}" GREATER_EQUAL 32)
  list(APPEND SRC datetime_tests.cpp)
endif ()

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
    FILTER_DIR="${CMAKE_SOURCE_DIR}/data/filters"
    UNIT_TESTS
)

target_link_libraries(tests PRIVATE ocpn::model-src ocpn::gtest win32_libs)
if (NOT "${ENABLE_SANITIZER}" STREQUAL "none")
  target_link_libraries(tests PRIVATE -fsanitize=${ENABLE_SANITIZER})
endif ()

find_program(DBUS_SEND NAMES dbus-send)
if (DBUS_SEND)
  target_compile_definitions(tests PRIVATE HAVE_DBUS_SEND)
endif ()

if (GLIB_FOUND)
  target_compile_definitions(tests PRIVATE HAVE_GLIB)
endif ()


if (UNIX AND NOT DEFINED ENV{FLATPAK_ID})
  set(_IPC_SRV_SRC ipc-srv-tests.cpp ${CMAKE_SOURCE_DIR}/cli/api_shim.cpp)
  add_executable(ipc-srv-tests ${_IPC_SRV_SRC} )
  target_link_libraries(ipc-srv-tests
    PRIVATE ocpn::gtest ocpn::model-src ocpn::gl-headers win32_libs
  )
  if (NOT "${ENABLE_SANITIZER}" STREQUAL "none")
    target_link_libraries(ipc-srv-tests PUBLIC -fsanitize=${ENABLE_SANITIZER})
  endif ()
  # See note in ipc-srv-tests.cpp
  target_compile_options(ipc-srv-tests PRIVATE -O0)
  add_dependencies(ipc-srv-tests cli-server ipc-client)
endif ()

set(_BUF_TEST_SRC buffer_tests.cpp ${CMAKE_SOURCE_DIR}/cli/api_shim.cpp)
add_executable(buffer_tests ${_BUF_TEST_SRC})
target_link_libraries(
  buffer_tests PRIVATE  ocpn::model-src ocpn::gtest ocpn::gl-headers win32_libs
)
target_compile_definitions(
  buffer_tests PUBLIC TESTDATA="${CMAKE_CURRENT_LIST_DIR}/testdata"
)

if (LINUX)
  set(_DBUS_TEST_SRC dbus_tests.cpp ${CMAKE_SOURCE_DIR}/cli/api_shim.cpp)
  add_executable(dbus_tests ${_DBUS_TEST_SRC})
  target_link_libraries(dbus_tests PRIVATE ocpn::gtest ocpn::model-src)
  if (NOT "${ENABLE_SANITIZER}" STREQUAL "none")
    target_link_libraries(dbus_tests PUBLIC -fsanitize=${ENABLE_SANITIZER})
  endif ()
endif ()

include(GoogleTest)
gtest_add_tests(TARGET tests)
gtest_add_tests(TARGET buffer_tests)

if (LINUX AND NOT DEFINED ENV{FLATPAK_ID} AND NOT OCPN_DISTRO_BUILD)
  # We don't have a session bus available when testing flatpak
  # so these can just be run in native builds.
  gtest_add_tests(TARGET ipc-srv-tests)
  # gtest_add_tests(TARGET dbus_tests)      dbus tests broken on wayland
endif ()

set(_CLI_SRV_SRC cli_server.cpp ${CMAKE_SOURCE_DIR}/cli/api_shim.cpp)
add_executable(cli-server ${_CLI_SRV_SRC})
target_link_libraries(cli-server PRIVATE ocpn::model-src win32_libs)
if (NOT "${ENABLE_SANITIZER}" STREQUAL "none")
  target_link_libraries(cli-server PRIVATE -fsanitize=${ENABLE_SANITIZER})
endif ()

set(_CLI_CLIENT_SRC ipc_client.cpp ${CMAKE_SOURCE_DIR}/cli/api_shim.cpp)
add_executable(ipc-client ${_CLI_CLIENT_SRC})
target_link_libraries(ipc-client PRIVATE ocpn::model-src win32_libs)
if (NOT "${ENABLE_SANITIZER}" STREQUAL "none")
  target_link_libraries(ipc-client PRIVATE -fsanitize=${ENABLE_SANITIZER})
endif ()

set(_WX_INST_SRC wx_instance.cpp ${CMAKE_SOURCE_DIR}/cli/api_shim.cpp)
add_executable(wx-instance ${_WX_INST_SRC})
target_link_libraries(wx-instance PRIVATE ocpn::model-src win32_libs)
if (NOT "${ENABLE_SANITIZER}" STREQUAL "none")
  target_link_libraries(wx-instance PRIVATE -fsanitize=${ENABLE_SANITIZER})
endif ()

if (UNIX)
  set(_STD_INST_SRC std_instance.cpp ${CMAKE_SOURCE_DIR}/cli/api_shim.cpp)
  add_executable(std-instance ${_STD_INST_SRC})
  target_link_libraries(std-instance PRIVATE ocpn::model-src)
  if (NOT "${ENABLE_SANITIZER}" STREQUAL "none")
    target_link_libraries(std-instance PRIVATE -fsanitize=${ENABLE_SANITIZER})
  endif ()
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
PATH=%PATH%\;${_cache_dir}\\\\wxWidgets-3.2.5\\\\lib\\\\vc14x_dll\")
")
  file(WRITE ${_script_path} ${_script})
  add_custom_target(win-path-setup
    COMMAND cmake -P "${_script_path}"
    COMMAND cmake -E echo "Creating ${_bat_path}"
  )
endif ()

add_dependencies(tests cli-server ipc-client)
