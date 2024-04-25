# ************************************************************************
#  Copyright (C) 2023 Alec Leamas                                        *
#  Copyright (C) 2010 David S Register                                   *
#                                                                        *
#  This program is free software; you can redistribute it and/or modify  *
#  it under the terms of the GNU General Public License as published by  *
#  the Free Software Foundation; either version 2 of the License, or     *
#  (at your option) any later version.                                   *
#                                                                        .
#  This program is distributed in the hope that it will be useful,       *
#  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
#  GNU General Public License for more details.                          *
#                                                                        *
#  You should have received a copy of the GNU General Public License     *
#  along with this program; if not, write to the                         *
#  Free Software Foundation, Inc.,                                       *
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
# ************************************************************************


set(SRC_CONSOLE console.cpp)
set(APP_TARGET  opencpn-glutil)
if(WIN32)
  add_executable(${APP_TARGET} WIN32 ${SRC_CONSOLE})
else()
  add_executable(${APP_TARGET} ${SRC_CONSOLE})
endif()

if (NOT "${ENABLE_SANITIZER}" MATCHES "none")
  target_link_libraries(${APP_TARGET} PRIVATE -fsanitize=${ENABLE_SANITIZER})
endif ()

target_include_directories(
  ${APP_TARGET}
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${APP_TARGET} PRIVATE ${wxWidgets_LIBRARIES} ${OPENGL_LIBRARIES} ocpn::wxjson)

if (CMAKE_HOST_WIN32)
  target_link_libraries(
    ${APP_TARGET} PRIVATE setupapi.lib "GLU_static" psapi.lib
      iphlpapi.lib # glu32.lib
  ) 
  # use gdi plus only on MSVC, it is not a free library
  if (MSVC)
    target_link_libraries(${APP_TARGET} PRIVATE gdiplus.lib)
  endif ()
endif ()


if (WIN32)
  install(TARGETS ${APP_TARGET} RUNTIME DESTINATION ".")
else()
  install(TARGETS ${APP_TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
#
# If this is a local MSVC Windows build, copy output products to
# correct configuration runtime folder
#
if (WIN32 AND (NOT OCPN_CI_BUILD) AND MSVC)
  add_custom_command(
    TARGET ${APP_TARGET}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${APP_TARGET}>"
              "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/$<TARGET_FILE_NAME:${APP_TARGET}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_PDB_FILE:${APP_TARGET}>"
              "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/$<TARGET_PDB_FILE_NAME:${APP_TARGET}>"
  )
endif (WIN32 AND (NOT OCPN_CI_BUILD) AND MSVC)

if (APPLE)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${APP_TARGET}
    DESTINATION "bin/OpenCPN.app/Contents/MacOS"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
      WORLD_EXECUTE GROUP_EXECUTE OWNER_EXECUTE
  )
endif ()
