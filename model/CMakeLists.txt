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

##set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_MODULE_PATH})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
message(STATUS "CMAKE_MODULE_PATH : ${CMAKE_MODULE_PATH}")

set(MODEL_HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/model)
set(
  HDRS
  ${MODEL_HDR_DIR}/ais_bitstring.h
  ${MODEL_HDR_DIR}/ais_decoder.h
  ${MODEL_HDR_DIR}/ais_defs.h
  ${MODEL_HDR_DIR}/ais_target_data.h
  ${MODEL_HDR_DIR}/atomic_queue.h
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
  ${MODEL_HDR_DIR}/comm_decoder.h
  ${MODEL_HDR_DIR}/comm_driver.h
  ${MODEL_HDR_DIR}/comm_drv_factory.h
  ${MODEL_HDR_DIR}/comm_drv_file.h
  ${MODEL_HDR_DIR}/comm_drv_n0183_android_bt.h
  ${MODEL_HDR_DIR}/comm_drv_n0183_android_int.h
  ${MODEL_HDR_DIR}/comm_drv_n0183.h
  ${MODEL_HDR_DIR}/comm_drv_n0183_net.h
  ${MODEL_HDR_DIR}/comm_drv_n0183_serial.h
  ${MODEL_HDR_DIR}/comm_drv_n2k.h
  ${MODEL_HDR_DIR}/comm_drv_n2k_serial.h
  ${MODEL_HDR_DIR}/comm_drv_registry.h
  ${MODEL_HDR_DIR}/comm_drv_signalk.h
  ${MODEL_HDR_DIR}/comm_drv_signalk_net.h
  ${MODEL_HDR_DIR}/comm_n0183_output.h
  ${MODEL_HDR_DIR}/comm_navmsg_bus.h
  ${MODEL_HDR_DIR}/comm_navmsg.h
  ${MODEL_HDR_DIR}/comm_util.h
  ${MODEL_HDR_DIR}/comm_vars.h
  ${MODEL_HDR_DIR}/config_vars.h
  ${MODEL_HDR_DIR}/conn_params.h
  ${MODEL_HDR_DIR}/cutil.h
  ${MODEL_HDR_DIR}/downloader.h
  ${MODEL_HDR_DIR}/dsPortType.h
  ${MODEL_HDR_DIR}/garmin_protocol_mgr.h
  ${MODEL_HDR_DIR}/garmin_wrapper.h
  ${MODEL_HDR_DIR}/geodesic.h
  ${MODEL_HDR_DIR}/georef.h
  ${MODEL_HDR_DIR}/hyperlink.h
  ${MODEL_HDR_DIR}/idents.h
  ${MODEL_HDR_DIR}/instance_check.h
  ${MODEL_HDR_DIR}/ipc_api.h
  ${MODEL_HDR_DIR}/json_event.h
  ${MODEL_HDR_DIR}/local_api.h
  ${MODEL_HDR_DIR}/logger.h
  ${MODEL_HDR_DIR}/MarkIcon.h
  ${MODEL_HDR_DIR}/mDNS_query.h
  ${MODEL_HDR_DIR}/mDNS_service.h
  ${MODEL_HDR_DIR}/meteo_points.h
  ${MODEL_HDR_DIR}/multiplexer.h
  ${MODEL_HDR_DIR}/nav_object_database.h
  ${MODEL_HDR_DIR}/navutil_base.h
  ${MODEL_HDR_DIR}/nmea_log.h
  ${MODEL_HDR_DIR}/ocpn_types.h
  ${MODEL_HDR_DIR}/ocpn_utils.h
  ${MODEL_HDR_DIR}/own_ship.h
  ${MODEL_HDR_DIR}/pincode.h
  ${MODEL_HDR_DIR}/plugin_blacklist.h
  ${MODEL_HDR_DIR}/plugin_cache.h
  ${MODEL_HDR_DIR}/plugin_handler.h
  ${MODEL_HDR_DIR}/plugin_loader.h
  ${MODEL_HDR_DIR}/plugin_paths.h
  ${MODEL_HDR_DIR}/position_parser.h
  ${MODEL_HDR_DIR}/rest_server.h
  ${MODEL_HDR_DIR}/route.h
  ${MODEL_HDR_DIR}/routeman.h
  ${MODEL_HDR_DIR}/route_point.h
  ${MODEL_HDR_DIR}/s57.h
  ${MODEL_HDR_DIR}/S57ClassRegistrar.h
  ${MODEL_HDR_DIR}/safe_mode.h
  ${MODEL_HDR_DIR}/select.h
  ${MODEL_HDR_DIR}/select_item.h
  ${MODEL_HDR_DIR}/semantic_vers.h
  ${MODEL_HDR_DIR}/ser_ports.h
  ${MODEL_HDR_DIR}/track.h
  ${MODEL_HDR_DIR}/wx28compat.h
)

set(MODEL_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SRC
  ${MODEL_SRC_DIR}/ais_bitstring.cpp
  ${MODEL_SRC_DIR}/ais_decoder.cpp
  ${MODEL_SRC_DIR}/ais_target_data.cpp
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
  ${MODEL_SRC_DIR}/comm_decoder.cpp
  ${MODEL_SRC_DIR}/comm_drv_factory.cpp
  ${MODEL_SRC_DIR}/comm_drv_file.cpp
  ${MODEL_SRC_DIR}/comm_drv_n0183.cpp
  ${MODEL_SRC_DIR}/comm_drv_n0183_net.cpp
  ${MODEL_SRC_DIR}/comm_drv_n0183_serial.cpp
  ${MODEL_SRC_DIR}/comm_drv_n2k.cpp
  ${MODEL_SRC_DIR}/comm_drv_n2k_serial.cpp
  ${MODEL_SRC_DIR}/comm_drv_registry.cpp
  ${MODEL_SRC_DIR}/comm_drv_signalk.cpp
  ${MODEL_SRC_DIR}/comm_drv_signalk_net.cpp
  ${MODEL_SRC_DIR}/comm_n0183_output.cpp
  ${MODEL_SRC_DIR}/comm_navmsg_bus.cpp
  ${MODEL_SRC_DIR}/comm_navmsg.cpp
  ${MODEL_SRC_DIR}/comm_navmsg.cpp
  ${MODEL_SRC_DIR}/comm_plugin_api.cpp
  ${MODEL_SRC_DIR}/comm_util.cpp
  ${MODEL_SRC_DIR}/comm_vars.cpp
  ${MODEL_SRC_DIR}/config_vars.cpp
  ${MODEL_SRC_DIR}/conn_params.cpp
  ${MODEL_SRC_DIR}/cutil.cpp
  ${MODEL_SRC_DIR}/downloader.cpp
  ${MODEL_SRC_DIR}/garmin_protocol_mgr.cpp
  ${MODEL_SRC_DIR}/geodesic.cpp
  ${MODEL_SRC_DIR}/georef.cpp
  ${MODEL_SRC_DIR}/hyperlink.cpp
  ${MODEL_SRC_DIR}/instance_handler.cpp
  ${MODEL_SRC_DIR}/ipc_api.cpp
  ${MODEL_SRC_DIR}/local_api.cpp
  ${MODEL_SRC_DIR}/logger.cpp
  ${MODEL_SRC_DIR}/mDNS_query.cpp
  ${MODEL_SRC_DIR}/mDNS_service.cpp
  ${MODEL_SRC_DIR}/multiplexer.cpp
  ${MODEL_SRC_DIR}/nav_object_database.cpp
  ${MODEL_SRC_DIR}/navutil_base.cpp
  ${MODEL_SRC_DIR}/ocpn_plugin.cpp
  ${MODEL_SRC_DIR}/ocpn_utils.cpp
  ${MODEL_SRC_DIR}/own_ship.cpp
  ${MODEL_SRC_DIR}/pincode.cpp
  ${MODEL_SRC_DIR}/plugin_blacklist.cpp
  ${MODEL_SRC_DIR}/plugin_cache.cpp
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
  ${MODEL_SRC_DIR}/track.cpp
  ${MODEL_SRC_DIR}/wx_instance_chk.cpp
)

if (NOT MSVC)
  list(APPEND HDRS ${MODEL_HDR_DIR}/std_instance_chk.h )
  list(APPEND SRC ${MODEL_SRC_DIR}/std_instance_chk.cpp)
endif ()

if (NOT QT_ANDROID)
  list(APPEND SRC ${MODEL_SRC_DIR}/ipc_factories.cpp)
endif ()

if (LINUX)
  list(APPEND HDRS ${MODEL_HDR_DIR}/comm_drv_n2k_socketcan.h )
  list(APPEND SRC ${MODEL_SRC_DIR}/comm_drv_n2k_socketcan.cpp)
endif ()

if (LINUX)
  list(APPEND HDRS ${MODEL_HDR_DIR}/dbus_client.h )
  list(APPEND SRC ${MODEL_SRC_DIR}/dbus_client.cpp)
  list(APPEND HDRS ${MODEL_HDR_DIR}/dbus_server.h )
  list(APPEND SRC ${MODEL_SRC_DIR}/dbus_server.cpp)
endif ()

if (OCPN_USE_GARMINHOST)
  list(APPEND SRC ${MODEL_SRC_DIR}/garmin_wrapper.cpp)
endif ()

if (APPLE)
  list(APPEND HDRS ${MODEL_HDR_DIR}/macutils.h )
  list(APPEND SRC ${MODEL_SRC_DIR}/macutils.c)
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(_OCPN_MODEL STATIC ${SRC} ${HDRS})
add_library(ocpn::model ALIAS _OCPN_MODEL)

target_include_directories(
  _OCPN_MODEL PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_SOURCE_DIR}/gui/include/gui
    ${CMAKE_SOURCE_DIR}/model/include/model
    ${CMAKE_BINARY_DIR}/include
)

# check for lzma support for reading compressed charts
if (OCPN_USE_LZMA)
  find_package(LibLZMA)
  if (LIBLZMA_FOUND)
    message(STATUS "Building with lzma support: ${LIBLZMA_LIBRARIES}")
    if (TARGET LibLZMA::LibLZMA)
      target_link_libraries(_OCPN_MODEL PUBLIC LibLZMA::LibLZMA)
    else ()
      target_include_directories(_OCPN_MODEL PUBLIC ${LIBLZMA_INCLUDE_DIRS})
      target_link_libraries(_OCPN_MODEL PUBLIC ${LIBLZMA_LIBRARIES})
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
    target_include_directories(_OCPN_MODEL PUBLIC "${LIBELF_INCLUDE_DIR}")
    target_link_libraries(_OCPN_MODEL PUBLIC "${LIBELF_LIBRARY}")

    set(USE_LIBELF ON PARENT_SCOPE)   # => config.h
  else ()
    message(WARNING "LibELF not found, using simplified compatibility check")
  endif ()
endif ()

if (UNIX AND NOT APPLE)
  find_package(GLIB COMPONENTS gio gobject REQUIRED)
  if (GLIB_FOUND)
    target_link_libraries(_OCPN_MODEL PUBLIC glib::glib)
  endif ()
endif ()

target_link_libraries(
  _OCPN_MODEL PUBLIC
    observable::observable
    ocpn::easywsclient
    ocpn::garminhost
    ocpn::gdal
    ocpn::geoprim
    ocpn::iso8211
    ocpn::libarchive
    ocpn::mDNS
    ocpn::mongoose
    ocpn::N2KParser
    ocpn::nmea0183
    ocpn::pugixml
    ocpn::rapidjson
    ocpn::s52plib
    ocpn::s57enc
    ocpn::sound
    ocpn::wxjson
    pico_sha2
)

if (OCPN_USE_NEWSERIAL)
  target_link_libraries(_OCPN_MODEL PUBLIC ocpn::serial)
endif ()

if (QT_ANDROID)
  target_include_directories(_OCPN_MODEL PRIVATE
    ${CMAKE_SOURCE_DIR}/buildandroid/libopenssl/${ARCH}/openssl/include
  )
  target_link_libraries(_OCPN_MODEL PRIVATE
    ${CMAKE_SOURCE_DIR}/buildandroid/libopenssl/${ARCH}/openssl/lib/libssl.a
    ${CMAKE_SOURCE_DIR}/buildandroid/libopenssl/${ARCH}/openssl/lib/libcrypto.a
  )
  target_compile_definitions(_OCPN_MODEL PUBLIC -DMG_ENABLE_OPENSSL)
  target_include_directories(_OCPN_MODEL PUBLIC
    ${CMAKE_SOURCE_DIR}/buildandroid/libcurl/${ARCH}/include
  )
  target_link_libraries(_OCPN_MODEL PUBLIC
    ${CMAKE_SOURCE_DIR}/buildandroid/libcurl/${ARCH}/lib/libcurl.a
  )
endif ()
