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

string(CONCAT filesys_url
  https://dl.cloudsmith.io/public/alec-leamas/opencpn-support/raw
  /names/boost-filesystem-ndk/versions/25c-1.82.0/boost-filesystem.tar.gz
)
set(url_hash MD5=a5baaca0f86fd852f40e1f5943386ff1)

add_library(_FILESYS INTERFACE)
add_library(ocpn::filesystem ALIAS _FILESYS)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
  target_link_libraries(_FILESYS INTERFACE stdc++fs)
endif ()

string(TOLOWER ${CMAKE_CXX_COMPILER_ID} lc_compiler_id)
if (NOT ${lc_compiler_id} MATCHES clang
		OR ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL 15.0
)
  # Use existing std::filesystem
  return ()
endif ()

if (QT_ANDROID)
  # Use prebuilt libraries:
  include(FetchContent)
  FetchContent_Declare(filesys URL ${filesys_url} URL_HASH ${url_hash})
  FetchContent_Populate(filesys)
  FetchContent_GetProperties(filesys SOURCE_DIR filesys_src_dir)
  set(_fsys_libdirs
    ${filesys_src_dir}/arm64-v8a/lib
    ${filesys_src_dir}/armeabi-v7a/lib
    ${filesys_src_dir}/x86_64/lib
  )
  set(_fsys_headers ${filesys_src_dir}/include/boost-1_82)
endif ()

if (NOT DEFINED ARCH)
  include(GetArch)
  GetArch()
endif ()

if (${ARCH} STREQUAL armhf)
  set(_ndk_sys_lib boost_system boost_system-clang-mt-a32-1_82)
  set(_ndk_filesys_lib boost_filesystem boost_filesystem-clang-mt-a32-1_82)
elseif (${ARCH} STREQUAL arm64)
  set(_ndk_sys_lib boost_system boost_system-clang-mt-a64-1_82)
  set(_ndk_filesys_lib boost_filesystem boost_filesystem-clang-mt-a64-1_82)
else ()
  set(_ndk_sys_lib boost_system boost_system-clang-mt-x64-1_82)
  set(_ndk_filesys_lib boost_filesystem boost_filesystem-clang-mt-x64-1_82)
endif ()

find_library(
  BOOST_SYSTEM NAMES boost_system ${_ndk_sys_lib}
  HINTS ${_fsys_libdirs} REQUIRED
)
find_library(
  BOOST_FILESYSTEM NAMES boost_filesystem ${_ndk_filesys_lib}
  HINTS ${_fsys_libdirs} REQUIRED
)
find_path(
  BOOST_HDRS NAMES boost/filesystem.hpp HINTS ${_fsys_headers} REQUIRED
)
target_link_libraries(_FILESYS INTERFACE ${BOOST_SYSTEM} ${BOOST_FILESYSTEM})
target_include_directories(_FILESYS INTERFACE ${BOOST_HDRS})
