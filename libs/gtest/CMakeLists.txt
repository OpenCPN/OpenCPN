cmake_minimum_required(VERSION 3.12)

if (TARGET ocpn::gtest)
    return ()
endif ()

if (NOT CMAKE_MODULE_PATH)
  set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
endif ()

add_library(_OCPN_GTEST_IF INTERFACE)
add_library(ocpn::gtest ALIAS _OCPN_GTEST_IF)

find_library(GTEST_LIB gtest)
find_library(GTEST_MAIN gtest_main)
find_path(GTEST_HDR NAME gtest PATH_SUFFIXES gtest)

if (GTEST_LIB AND GTEST_MAIN AND GTEST_HDR)
  message(STATUS "Using system gtest libraries")
  target_include_directories(_OCPN_GTEST_IF INTERFACE ${GTEST_HDR})
  target_link_libraries(_OCPN_GTEST_IF INTERFACE ${GTEST_LIB})
  target_link_libraries(_OCPN_GTEST_IF INTERFACE ${GTEST_MAIN})
else ()
  message(STATUS "Downloading and building gtest from source")
  include(FetchContent)
  set(CMAKE_CXX_STANDARD 14)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  target_link_libraries(_OCPN_GTEST_IF GTest::gtest_main)
endif ()  
