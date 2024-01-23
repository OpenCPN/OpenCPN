cmake_minimum_required(VERSION 3.12)

if (TARGET ocpn::gtest)
    return ()
endif ()

# Fixme (#3527) dependency on main opencpn
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
  find_path(GTEST_SRC_LOCATION
    NAMES googlemock/src/gmock-all.cc googletest/src/gtest-all.cc
    HINTS ENV GTEST_SRC_ROOT
    PATHS "/usr/src/googletest"
    NO_DEFAULT_PATH
  )
  include(FetchContent)
  set(CMAKE_CXX_STANDARD 14)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(BUILD_GMOCK OFF)
  set(INSTALL_GTEST OFF)
  if (GTEST_SRC_LOCATION)
    message(STATUS "Building gtest from installed sources")
    add_subdirectory(${GTEST_SRC_LOCATION} ${CMAKE_BINARY_DIR}/gtest)
  else ()
    message(STATUS "Downloading and building gtest from source")
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
  endif ()
  target_link_libraries(_OCPN_GTEST_IF INTERFACE GTest::gtest_main)
endif ()
