#*************************************************************************
#  Copyright (C) 2023  David S. Register                                 *
#  Copyright (C) 2023  Alec Leamas                                       *
#  This program is free software; you can redistribute it and/or modify  *
#  it under the terms of the GNU General Public License as published by  *
#  the Free Software Foundation; either version 2 of the License, or     *
#  (at your option) any later version.                                   *
#                                                                        *
#  This program is distributed in the hope that it will be useful,       *
#  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
#  GNU General Public License for more details.                          *
#                                                                        *
#  You should have received a copy of the GNU General Public License     *
#  along with this program; if not, write to the                         *
#  Free Software Foundation, Inc.,                                       *
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
#*************************************************************************

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

if (UNIX AND NOT APPLE AND NOT QT_ANDROID)
  set(LINUX "1")
endif ()

set(MODEL_HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/model)
set(
  HDRS
  ${MODEL_HDR_DIR}/ais_bitstring.h
  ${MODEL_HDR_DIR}/ais_decoder.h
  ${MODEL_HDR_DIR}/ais_defs.h
  ${MODEL_HDR_DIR}/ais_state_vars.h
  ${MODEL_HDR_DIR}/ais_target_data.h
  ${MODEL_HDR_DIR}/atomic_queue.h
  ${MODEL_HDR_DIR}/autopilot_output.h
  ${MODEL_HDR_DIR}/base_platform.h
  ${MODEL_HDR_DIR}/catalog_handler.h
  ${MODEL_HDR_DIR}/catalog_parser.h
  ${MODEL_HDR_DIR}/certificates.h
  ${MODEL_HDR_DIR}/chartdata_input_stream.h
  ${MODEL_HDR_DIR}/cli_platform.h
  ${MODEL_HDR_DIR}/cmdline.h
  ${MODEL_HDR_DIR}/comm_ais.h
  ${MODEL_HDR_DIR}/comm_appmsg_bus.h
  ${MODEL_HDR_DIR}/comm_appmsg.h
  ${MODEL_HDR_DIR}/comm_bridge.h
  ${MODEL_HDR_DIR}/comm_buffers.h
  ${MODEL_HDR_DIR}/comm_can_util.h
  ${MODEL_HDR_DIR}/comm_decoder.h
  ${MODEL_HDR_DIR}/comm_driver.h
  ${MODEL_HDR_DIR}/comm_drv_factory.h
  ${MODEL_HDR_DIR}/comm_drv_file.h
  ${MODEL_HDR_DIR}/comm_drv_internal.h
  ${MODEL_HDR_DIR}/comm_drv_n0183_android_bt.h
  ${MODEL_HDR_DIR}/comm_drv_n0183_android_int.h
  ${MODEL_HDR_DIR}/comm_drv_n0183.h
  ${MODEL_HDR_DIR}/comm_drv_n0183_net.h
  ${MODEL_HDR_DIR}/comm_drv_n0183_serial.h
  ${MODEL_HDR_DIR}/comm_drv_n2k.h
  ${MODEL_HDR_DIR}/comm_drv_n2k_net.h
  ${MODEL_HDR_DIR}/comm_drv_n2k_serial.h
  ${MODEL_HDR_DIR}/comm_drv_registry.h
  ${MODEL_HDR_DIR}/comm_drv_signalk.h
  ${MODEL_HDR_DIR}/comm_drv_signalk_net.h
  ${MODEL_HDR_DIR}/comm_drv_stats.h
  ${MODEL_HDR_DIR}/comm_n0183_output.h
  ${MODEL_HDR_DIR}/comm_navmsg_bus.h
  ${MODEL_HDR_DIR}/comm_navmsg.h
  ${MODEL_HDR_DIR}/comm_out_queue.h
  ${MODEL_HDR_DIR}/comm_util.h
  ${MODEL_HDR_DIR}/comm_vars.h
  ${MODEL_HDR_DIR}/config_vars.h
  ${MODEL_HDR_DIR}/conn_params.h
  ${MODEL_HDR_DIR}/conn_states.h
  ${MODEL_HDR_DIR}/cutil.h
  ${MODEL_HDR_DIR}/data_monitor_src.h
  ${MODEL_HDR_DIR}/datetime.h
  ${MODEL_HDR_DIR}/downloader.h
  ${MODEL_HDR_DIR}/ds_porttype.h
  ${MODEL_HDR_DIR}/filters_on_disk.h
  ${MODEL_HDR_DIR}/garmin_protocol_mgr.h
  ${MODEL_HDR_DIR}/garmin_wrapper.h
  ${MODEL_HDR_DIR}/geodesic.h
  ${MODEL_HDR_DIR}/georef.h
  ${MODEL_HDR_DIR}/gpx_document.h
  ${MODEL_HDR_DIR}/hyperlink.h
  ${MODEL_HDR_DIR}/idents.h
  ${MODEL_HDR_DIR}/instance_check.h
  ${MODEL_HDR_DIR}/ipc_api.h
  ${MODEL_HDR_DIR}/json_event.h
  ${MODEL_HDR_DIR}/local_api.h
  ${MODEL_HDR_DIR}/logger.h
  ${MODEL_HDR_DIR}/MarkIcon.h
  ${MODEL_HDR_DIR}/mdns_query.h
  ${MODEL_HDR_DIR}/mdns_cache.h
  ${MODEL_HDR_DIR}/mdns_service.h
  ${MODEL_HDR_DIR}/meteo_points.h
  ${MODEL_HDR_DIR}/multiplexer.h
  ${MODEL_HDR_DIR}/nav_object_database.h
  ${MODEL_HDR_DIR}/navmsg_filter.h
  ${MODEL_HDR_DIR}/navobj_db.h
  ${MODEL_HDR_DIR}/navutil_base.h
  ${MODEL_HDR_DIR}/nmea_log.h
  ${MODEL_HDR_DIR}/nmea_ctx_factory.h
  ${MODEL_HDR_DIR}/notification.h
  ${MODEL_HDR_DIR}/notification_manager.h
  ${MODEL_HDR_DIR}/ocpn_types.h
  ${MODEL_HDR_DIR}/ocpn_utils.h
  ${MODEL_HDR_DIR}/own_ship.h
  ${MODEL_HDR_DIR}/peer_client.h
  ${MODEL_HDR_DIR}/periodic_timer.h
  ${MODEL_HDR_DIR}/pincode.h
  ${MODEL_HDR_DIR}/periodic_timer.h
  ${MODEL_HDR_DIR}/plugin_blacklist.h
  ${MODEL_HDR_DIR}/plugin_cache.h
  ${MODEL_HDR_DIR}/plugin_comm.h
  ${MODEL_HDR_DIR}/plugin_handler.h
  ${MODEL_HDR_DIR}/plugin_loader.h
  ${MODEL_HDR_DIR}/plugin_paths.h
  ${MODEL_HDR_DIR}/position_parser.h
  ${MODEL_HDR_DIR}/rest_server.h
  ${MODEL_HDR_DIR}/route.h
  ${MODEL_HDR_DIR}/routeman.h
  ${MODEL_HDR_DIR}/route_point.h
  ${MODEL_HDR_DIR}/safe_mode.h
  ${MODEL_HDR_DIR}/select.h
  ${MODEL_HDR_DIR}/select_item.h
  ${MODEL_HDR_DIR}/semantic_vers.h
  ${MODEL_HDR_DIR}/serial_io.h
  ${MODEL_HDR_DIR}/ser_ports.h
  ${MODEL_HDR_DIR}/sys_events.h
  ${MODEL_HDR_DIR}/thread_ctrl.h
  ${MODEL_HDR_DIR}/track.h
  ${MODEL_HDR_DIR}/usb_watch_daemon.h
  ${MODEL_HDR_DIR}/wait_continue.h
  ${MODEL_HDR_DIR}/wx28compat.h
  ${MODEL_HDR_DIR}/wx_instance_chk.h
)

set(MODEL_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SRC
  ${MODEL_SRC_DIR}/ais_bitstring.cpp
  ${MODEL_SRC_DIR}/ais_decoder.cpp
  ${MODEL_SRC_DIR}/ais_state_vars.cpp
  ${MODEL_SRC_DIR}/ais_target_data.cpp
  ${MODEL_SRC_DIR}/autopilot_output.cpp
  ${MODEL_SRC_DIR}/base_platform.cpp
  ${MODEL_SRC_DIR}/catalog_handler.cpp
  ${MODEL_SRC_DIR}/catalog_parser.cpp
  ${MODEL_SRC_DIR}/certificates.cpp
  ${MODEL_SRC_DIR}/chartdata_input_stream.cpp
  ${MODEL_SRC_DIR}/cli_platform.cpp
  ${MODEL_SRC_DIR}/cmdline.cpp
  ${MODEL_SRC_DIR}/comm_ais.cpp
  ${MODEL_SRC_DIR}/comm_appmsg_bus.cpp
  ${MODEL_SRC_DIR}/comm_appmsg.cpp
  ${MODEL_SRC_DIR}/comm_bridge.cpp
  ${MODEL_SRC_DIR}/comm_buffers.cpp
  ${MODEL_SRC_DIR}/comm_can_util.cpp
  ${MODEL_SRC_DIR}/comm_decoder.cpp
  #${MODEL_SRC_DIR}/comm_driver.cpp
  ${MODEL_SRC_DIR}/comm_drv_factory.cpp
  ${MODEL_SRC_DIR}/comm_drv_file.cpp
  ${MODEL_SRC_DIR}/comm_drv_internal.cpp
  ${MODEL_SRC_DIR}/comm_drv_n0183.cpp
  ${MODEL_SRC_DIR}/comm_drv_n0183_net.cpp
  ${MODEL_SRC_DIR}/comm_drv_n0183_serial.cpp
  ${MODEL_SRC_DIR}/comm_drv_n2k.cpp
  ${MODEL_SRC_DIR}/comm_drv_n2k_net.cpp
  ${MODEL_SRC_DIR}/comm_drv_n2k_serial.cpp
  ${MODEL_SRC_DIR}/comm_drv_registry.cpp
  ${MODEL_SRC_DIR}/comm_drv_signalk.cpp
  ${MODEL_SRC_DIR}/comm_drv_signalk_net.cpp
  ${MODEL_SRC_DIR}/comm_n0183_output.cpp
  ${MODEL_SRC_DIR}/comm_navmsg_bus.cpp
  ${MODEL_SRC_DIR}/comm_navmsg.cpp
  ${MODEL_SRC_DIR}/comm_navmsg.cpp
  ${MODEL_SRC_DIR}/comm_out_queue.cpp
  ${MODEL_SRC_DIR}/comm_util.cpp
  ${MODEL_SRC_DIR}/comm_vars.cpp
  ${MODEL_SRC_DIR}/config_vars.cpp
  ${MODEL_SRC_DIR}/conn_params.cpp
  ${MODEL_SRC_DIR}/conn_states.cpp
  ${MODEL_SRC_DIR}/cutil.cpp
  ${MODEL_SRC_DIR}/data_monitor_src.cpp
  ${MODEL_SRC_DIR}/datetime.cpp
  ${MODEL_SRC_DIR}/downloader.cpp
  ${MODEL_SRC_DIR}/ds_porttype.cpp
  ${MODEL_SRC_DIR}/filters_on_disk.cpp
  ${MODEL_SRC_DIR}/garmin_protocol_mgr.cpp
  ${MODEL_SRC_DIR}/geodesic.cpp
  ${MODEL_SRC_DIR}/georef.cpp
  ${MODEL_SRC_DIR}/gpx_document.cpp
  ${MODEL_SRC_DIR}/hyperlink.cpp
  ${MODEL_SRC_DIR}/instance_handler.cpp
  ${MODEL_SRC_DIR}/ipc_api.cpp
  ${MODEL_SRC_DIR}/ipc_factories.cpp
  ${MODEL_SRC_DIR}/local_api.cpp
  ${MODEL_SRC_DIR}/logger.cpp
  ${MODEL_SRC_DIR}/mdns_query.cpp
  ${MODEL_SRC_DIR}/mdns_cache.cpp
  ${MODEL_SRC_DIR}/mdns_service.cpp
  ${MODEL_SRC_DIR}/multiplexer.cpp
  ${MODEL_SRC_DIR}/nav_object_database.cpp
  ${MODEL_SRC_DIR}/navobj_db.cpp
  ${MODEL_SRC_DIR}/navmsg_filter.cpp
  ${MODEL_SRC_DIR}/navutil_base.cpp
  ${MODEL_SRC_DIR}/notification.cpp
  ${MODEL_SRC_DIR}/notification_manager.cpp
  ${MODEL_SRC_DIR}/ocpn_plugin.cpp
  ${MODEL_SRC_DIR}/ocpn_utils.cpp
  ${MODEL_SRC_DIR}/own_ship.cpp
  ${MODEL_SRC_DIR}/peer_client.cpp
  ${MODEL_SRC_DIR}/periodic_timer.cpp
  ${MODEL_SRC_DIR}/pincode.cpp
  ${MODEL_SRC_DIR}/plugin_api.cpp
  ${MODEL_SRC_DIR}/plugin_blacklist.cpp
  ${MODEL_SRC_DIR}/plugin_cache.cpp
  ${MODEL_SRC_DIR}/plugin_comm.cpp
  ${MODEL_SRC_DIR}/plugin_handler.cpp
  ${MODEL_SRC_DIR}/plugin_loader.cpp
  ${MODEL_SRC_DIR}/plugin_paths.cpp
  ${MODEL_SRC_DIR}/position_parser.cpp
  ${MODEL_SRC_DIR}/rest_server.cpp
  ${MODEL_SRC_DIR}/route.cpp
  ${MODEL_SRC_DIR}/routeman.cpp
  ${MODEL_SRC_DIR}/route_point.cpp
  ${MODEL_SRC_DIR}/safe_mode.cpp
  ${MODEL_SRC_DIR}/select.cpp
  ${MODEL_SRC_DIR}/select_item.cpp
  ${MODEL_SRC_DIR}/semantic_vers.cpp
  ${MODEL_SRC_DIR}/ser_ports.cpp
  ${MODEL_SRC_DIR}/thread_ctrl.cpp
  ${MODEL_SRC_DIR}/track.cpp
  ${MODEL_SRC_DIR}/usb_watch_factory.cpp
  ${MODEL_SRC_DIR}/wx_instance_chk.cpp
)

if (NOT MSVC)
  list(APPEND HDRS ${MODEL_HDR_DIR}/std_instance_chk.h )
  list(APPEND SRC ${MODEL_SRC_DIR}/std_instance_chk.cpp)
endif ()

if (QT_ANDROID)
  list(APPEND HDRS ${MODEL_SRC_DIR}/android_serial_io.cpp)
else ()
  list(APPEND SRC ${MODEL_SRC_DIR}/std_serial_io.cpp)
endif ()

if (LINUX)
  list(APPEND HDRS ${MODEL_HDR_DIR}/comm_drv_n2k_socketcan.h )
  list(APPEND SRC ${MODEL_SRC_DIR}/comm_drv_n2k_socketcan.cpp)
  list(APPEND HDRS ${MODEL_HDR_DIR}/linux_devices.h)
  list(APPEND SRC ${MODEL_SRC_DIR}/linux_devices.cpp)
endif ()

if (LINUX AND NOT QT_ANDROID)
  list(APPEND HDRS ${MODEL_HDR_DIR}/dbus_client.h )
  list(APPEND SRC ${MODEL_SRC_DIR}/dbus_client.cpp)
  list(APPEND HDRS ${MODEL_HDR_DIR}/dbus_server.h )
  list(APPEND SRC ${MODEL_SRC_DIR}/dbus_server.cpp)
  list(APPEND HDRS ${MODEL_HDR_DIR}/linux_usb_watch.h)
  list(APPEND SRC ${MODEL_SRC_DIR}/linux_usb_watch.cpp)
elseif(WIN32)
  list(APPEND HDRS ${MODEL_HDR_DIR}/win_usb_watch.h )
  list(APPEND SRC ${MODEL_SRC_DIR}/win_usb_watch.cpp)
endif ()

if (OCPN_USE_GARMINHOST)
  list(APPEND SRC ${MODEL_SRC_DIR}/garmin_wrapper.cpp)
endif ()

if (APPLE)
  list(APPEND HDRS ${MODEL_HDR_DIR}/macutils.h )
  list(APPEND SRC ${MODEL_SRC_DIR}/macutils.c)
endif ()

if (QT_ANDROID)
  list(APPEND SRC ${MODEL_SRC_DIR}/comm_drv_n0183_android_bt.cpp)
  list(APPEND SRC ${MODEL_SRC_DIR}/comm_drv_n0183_android_int.cpp)
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(_OCPN_MODEL INTERFACE)
add_library(ocpn::model ALIAS _OCPN_MODEL)

add_library(_model_src OBJECT ${SRC} ${HDRS})
add_library(ocpn::model-src ALIAS _model_src)
target_link_libraries(_model_src PUBLIC _OCPN_MODEL)

target_include_directories(
  _OCPN_MODEL BEFORE INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_SOURCE_DIR}/model/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_SOURCE_DIR}/libs/sqlite/include
)

if (LINUX)
  find_package(LIBUDEV)
  if (LIBUDEV_FOUND)
    target_link_libraries(_OCPN_MODEL INTERFACE ocpn::libudev)
    set(HAVE_LIBUDEV 1)
  endif ()
endif ()

if (OCPN_USE_CURL)

  if (NOT CMAKE_HOST_WIN32)
    set(OCPN_USE_EXTERN_CURL ON)
  endif ()

  if (NOT QT_ANDROID)
    if (OCPN_USE_EXTERN_CURL)
      find_package(CURL REQUIRED)
      message(STATUS "ocpn: using external curl libraries...")
      message(STATUS "  curl includes: " ${CURL_INCLUDE_DIRS})
      message(STATUS "  curl libraries: " ${CURL_LIBRARIES})

      target_include_directories(_OCPN_MODEL INTERFACE ${CURL_INCLUDE_DIRS})
    else ()
      include("Curl")
      message(STATUS "Using bundled curl library...")
    endif ()

    pkg_search_module(SYS_WXCURL libwxcurl wxcurl)
    use_bundled_lib(USE_BUNDLED_WXCURL wxcurl)
    if (SYS_WXCURL_FOUND AND NOT USE_BUNDLED_WXCURL)
      message(STATUS "Building with system wxcurl")
      target_include_directories(
        _OCPN_MODEL INTERFACE ${CURL_INCLUDE_DIRS} ${SYS_WXCURL_INCLUDE_DIR}
      )
      target_link_libraries(_OCPN_MODEL INTERFACE ${SYS_WXCURL_LIBRARIES})
    else ()
      message(STATUS "Building with bundled wxcurl")
      include("Curl")
      set(WXCURL_ABI ${PKG_TARGET} CACHE INTERNAL "")
      set(WXCURL_ABI_VERSION ${PKG_TARGET_VERSION} CACHE INTERNAL "")
      set(WXCURL_OCPN_VERSION ${PACKAGE_VERSION} CACHE INTERNAL "")
      add_subdirectory("${CMAKE_SOURCE_DIR}/libs/wxcurl" ${CMAKE_BINARY_DIR}/curl2)
      target_link_libraries(_OCPN_MODEL INTERFACE ocpn::wxcurl)
    endif ()

    target_link_libraries(_OCPN_MODEL INTERFACE ${CURL_LIBRARIES})

    if (WIN32 AND OCPN_USE_EXTERN_CURL)
      find_library(
        CURL_LIBRARY_DLL
        NAMES curl curllib libcurl_imp curllib_static libcurl
        PATH_SUFFIXES dll
      )
      install(FILES ${CURL_LIBRARY_DLL} DESTINATION ".")
    endif ()
  else ()
    target_include_directories(
      _OCPN_MODEL
        INTERFACE ${CMAKE_SOURCE_DIR}/buildandroid/libcurl/${ARCH}/include
    )
    target_link_libraries(
      _OCPN_MODEL
        INTERFACE
          ${CMAKE_SOURCE_DIR}/buildandroid/libcurl/${ARCH}/lib/libcurl.a
    )
  endif ()
endif ()

if (NOT WIN32)
  if(APPLE AND OCPN_USE_DEPS_BUNDLE)
    set(OPENSSL_ROOT_DIR ${OCPN_DEPS_BUNDLE_PATH})
  endif()
  find_package(OpenSSL)
  if (OPENSSL_FOUND)
    message(STATUS
      "OpenSSL found ${OPENSSL_INCLUDE_DIR} ${OPENSSL_LIBRARIES}"
    )
    target_include_directories(_OCPN_MODEL INTERFACE ${OPENSSL_INCLUDE_DIR})
    target_include_directories(_OCPN_MODEL INTERFACE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(_OCPN_MODEL INTERFACE ${OPENSSL_LIBRARIES})
  elseif (APPLE)
    message(STATUS "Model: APPLE: Hardwiring /usr/local OpenSSL...")
    target_include_directories(
      _OCPN_MODEL INTERFACE /usr/local/opt/openssl@3/include
    )
    target_link_libraries(
      _OCPN_MODEL
        INTERFACE
          /usr/local/opt/openssl@3/lib/libcrypto.3.dylib
          /usr/local/opt/openssl@3/lib/libssl.3.dylib
    )

  endif ()
else ()
  target_include_directories(
    _OCPN_MODEL
    INTERFACE ${CMAKE_SOURCE_DIR}/cache/buildwin/include/openssl
  )
  target_link_libraries(
    _OCPN_MODEL
    INTERFACE ${CMAKE_SOURCE_DIR}/cache/buildwin/libssl.lib
  )
 target_link_libraries(
    _OCPN_MODEL
    INTERFACE ${CMAKE_SOURCE_DIR}/cache/buildwin/libcrypto.lib
  )
endif ()




# check for lzma support for reading compressed charts
if (OCPN_USE_LZMA)
  find_package(LibLZMA)
  if (LIBLZMA_FOUND)
    message(STATUS "Building with lzma support: ${LIBLZMA_LIBRARIES}")
    if (TARGET LibLZMA::LibLZMA)
      target_link_libraries(_OCPN_MODEL INTERFACE LibLZMA::LibLZMA)
    else ()
      target_include_directories(_OCPN_MODEL INTERFACE ${LIBLZMA_INCLUDE_DIR})
      target_link_libraries(_OCPN_MODEL INTERFACE ${LIBLZMA_LIBRARIES})
    endif ()
  else ()
    message(STATUS "lzma library not found")
  endif ()
endif ()

if (UNIX AND NOT APPLE)
  find_path(LIBELF_INCLUDE_DIR NAMES libelf.h gelf.h PATH_SUFFIXES libelf)
  find_library(LIBELF_LIBRARY NAMES elf)
  if (LIBELF_INCLUDE_DIR AND LIBELF_LIBRARY)
    message(STATUS "Found LibELF...")
    message(STATUS "    ELF Lib: ${LIBELF_INCLUDE_DIR}")
    message(STATUS "    ELF Include: ${LIBELF_LIBRARY}")
    target_include_directories(_OCPN_MODEL INTERFACE "${LIBELF_INCLUDE_DIR}")
    target_link_libraries(_OCPN_MODEL INTERFACE "${LIBELF_LIBRARY}")

    set(USE_LIBELF ON PARENT_SCOPE)   # => config.h
  else ()
    message(WARNING "LibELF not found, using simplified compatibility check")
  endif ()
endif ()

if (UNIX AND NOT APPLE)
  find_package(GLIB COMPONENTS gio gobject REQUIRED)
  if (GLIB_FOUND)
    target_link_libraries(_OCPN_MODEL INTERFACE glib::glib)
  endif ()
endif ()

if (NOT QT_ANDROID)
  find_package(LIBUSB)
endif ()

if (LIBUSB_FOUND)
  target_link_libraries(_OCPN_MODEL INTERFACE ocpn::libusb)
  add_compile_definitions(_OCPN_MODEL INTERFACE HAVE_LIBUSB_10)
endif ()

if (LINUX)
  target_link_libraries(_OCPN_MODEL INTERFACE ocpn::gtk)
  target_link_libraries(_OCPN_MODEL INTERFACE ${X11_LIBRARIES} ${X11_Xrandr_LIB})
endif ()

if (NOT "${ENABLE_SANITIZER}" STREQUAL "none")
  target_link_libraries(_OCPN_MODEL INTERFACE -fsanitize=${ENABLE_SANITIZER})
endif ()

target_link_libraries(
  _OCPN_MODEL INTERFACE
    observable::observable
    ocpn::ixwebsocket
    ocpn::garminhost
    ocpn::gdal
    ocpn::geoprim
    ocpn::iso8211
    ocpn::libarchive
    ocpn::mdns
    ocpn::mongoose
    ocpn::N2KParser
    ocpn::nmea0183
    ocpn::pugixml
    ocpn::rapidjson
    ocpn::sqlite
    ocpn::s52plib
    ocpn::sound
    ocpn::wxjson
    ocpn::filesystem
    ocpn::wxservdisc
    pico_sha2
)

if (OCPN_USE_NEWSERIAL)
  target_link_libraries(_OCPN_MODEL INTERFACE ocpn::serial)
endif ()


if (MSVC)
  target_link_libraries(_OCPN_MODEL INTERFACE iphlpapi)
endif ()

if (QT_ANDROID)
  target_link_libraries(_OCPN_MODEL INTERFACE ocpn::java-jvm)
  target_include_directories(_OCPN_MODEL INTERFACE
    ${CMAKE_SOURCE_DIR}/buildandroid/libopenssl/${ARCH}/openssl/include
  )
  target_link_libraries(_OCPN_MODEL INTERFACE
    ${CMAKE_SOURCE_DIR}/buildandroid/libopenssl/${ARCH}/openssl/lib/libssl.a
    ${CMAKE_SOURCE_DIR}/buildandroid/libopenssl/${ARCH}/openssl/lib/libcrypto.a
  )
  target_include_directories(_OCPN_MODEL INTERFACE
    ${CMAKE_SOURCE_DIR}/buildandroid/libcurl/${ARCH}/include
    ${CMAKE_SOURCE_DIR}/android
  )
  target_link_libraries(_OCPN_MODEL INTERFACE
    ${CMAKE_SOURCE_DIR}/buildandroid/libcurl/${ARCH}/lib/libcurl.a
  )
endif ()
