#[[
Copyright (C) 2026  Alec Leamas

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <https://www.gnu.org/licenses/>.
#]]


#
#  Wrapper for https://github.com/nlohmann/json
#
#  Exports: nlohmann::json transitive link target
#
#  Usage: #include  ocpn-nlohmann/json.hpp
#
#  Searches for installed package and uses this if found. Otherwise
#  downloads sources from Github.
#

cmake_minimum_required(VERSION 3.12)

if (TARGET nlohmann::json)
  return ()
endif ()


add_library(_NLOHMANN_JSON_ INTERFACE)
add_library(nlohmann::json ALIAS _NLOHMANN_JSON_)

find_package(nlohmann_json)
if (nlohmann_json_FOUND)
  target_link_libraries(_NLOHMANN_JSON_ INTERFACE nlohmann_json::nlohmann_json)
else ()
  include(FetchContent)

  FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
  )
  FetchContent_MakeAvailable(json)
  target_link_libraries(_NLOHMANN_JSON_ INTERFACE nlohmann_json::nlohmann_json)
endif ()
target_include_directories(
  _NLOHMANN_JSON_ INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
