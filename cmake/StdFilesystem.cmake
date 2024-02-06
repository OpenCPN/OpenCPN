# ************************************************************************
#    Copyright (C) 2023  Alec Leamas                                  *
#                                                                          *
#    This program is free software; you can redistribute it and/or modify  *
#    it under the terms of the GNU General Public License as published by  *
#    the Free Software Foundation; either version 2 of the License, or     *
#    (at your option) any later version.                                   *
#                                                                          *
#    This program is distributed in the hope that it will be useful,       *
#    but WITHOUT ANY WARRANTY; without even the implied warranty of        *
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
#    GNU General Public License for more details.                          *
#                                                                          *
#    You should have received a copy of the GNU General Public License     *
#    along with this program; if not, write to the                         *
#    Free Software Foundation, Inc.,                                       *
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
# *************************************************************************/

#
# Get a compatible std::filesystem. On gcc < 8 add required stdc++fs linkage.
# On clang < 15 (NDK < 26, MacOS < 10.15) add headers and linkage for
# boost::filesystem
#
# Exports: ocpn::filesystem transitive link target
#

if (TARGET ocpn::filesystem)
  return ()
endif ()


add_library(_FILESYS INTERFACE)
add_library(ocpn::filesystem ALIAS _FILESYS)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
  target_link_libraries(_FILESYS INTERFACE stdc++fs)
endif ()

string(TOLOWER ${CMAKE_CXX_COMPILER_ID} lc_compiler_id)
if (QT_ANDROID OR (${lc_compiler_id} MATCHES clang
    AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 15.0)
)
  # MacOS 10.13
  # Boost pulled in from Homebrew on the builders is not good enough as it is not ABI compatible with older targets
  #find_library(BOOST_SYSTEM NAMES boost_system  REQUIRED)
  #find_library(BOOST_FILESYSTEM NAMES boost_filesystem REQUIRED)
  #find_path(BOOST_HDRS NAMES boost/filesystem.hpp REQUIRED)
  #target_link_libraries(_FILESYS INTERFACE ${BOOST_SYSTEM} ${BOOST_FILESYSTEM})
  #target_include_directories(_FILESYS INTERFACE ${BOOST_HDRS})
  target_include_directories(_FILESYS INTERFACE libs/ghc/include)
endif ()
