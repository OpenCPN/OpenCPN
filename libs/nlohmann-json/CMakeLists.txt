#
#  Wrapper for https://github.com/nlohmann/json
#
#  Exports: nlohmann::json transitive link target
#
#  Usage: #include  nlohmann/json.hpp
#

cmake_minimum_required(VERSION 3.10)

if (TARGET nlohmann::json)
  return ()
endif ()

add_library(_NLOHMANN_JSON_ INTERFACE)
add_library(nlohmann::json ALIAS _NLOHMANN_JSON_)

target_include_directories(
  _NLOHMANN_JSON_ INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
