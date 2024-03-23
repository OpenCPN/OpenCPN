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

if(WIN32)
  add_executable(opencpn-glutil WIN32 ${SRC_CONSOLE})
else()
  add_executable(opencpn-glutil ${SRC_CONSOLE})
endif()

if (NOT "${ENABLE_SANITIZER}" MATCHES "none")
  target_link_libraries(opencpn-glutil PRIVATE -fsanitize=${ENABLE_SANITIZER})
endif ()

target_include_directories(
  opencpn-glutil
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(opencpn-glutil PRIVATE ${wxWidgets_LIBRARIES} ${OPENGL_LIBRARIES} ocpn::wxjson)

if (CMAKE_HOST_WIN32)
  target_link_libraries(
    opencpn-glutil PRIVATE setupapi.lib "GLU_static" psapi.lib
      iphlpapi.lib # glu32.lib
  ) 
  # use gdi plus only on MSVC, it is not a free library
  if (MSVC)
    target_link_libraries(opencpn-glutil PRIVATE gdiplus.lib)
  endif ()
endif ()


if (WIN32)
  install(TARGETS opencpn-glutil RUNTIME DESTINATION ".")
else()
  install(TARGETS opencpn-glutil RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

if (APPLE)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/opencpn-glutil
    DESTINATION "bin/OpenCPN.app/Contents/MacOS"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
      WORLD_EXECUTE GROUP_EXECUTE OWNER_EXECUTE
  )
endif ()
