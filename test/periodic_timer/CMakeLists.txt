cmake_minimum_required(VERSION 3.14)

# Also buildable on its own, without wxWidgets or the host dependencies.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(periodic_timer_tests LANGUAGES CXX)
  set(CMAKE_CXX_STANDARD 17)
  enable_testing()
  set(ENABLE_SANITIZER "none" CACHE STRING "Compiler sanitizer")
  add_subdirectory(../../libs/gtest gtest)
endif ()

find_package(Threads REQUIRED)
add_executable(periodic_timer_tests ../periodic_timer_tests.cpp
               ../../model/src/periodic_timer.cpp)
target_include_directories(periodic_timer_tests PRIVATE ../../model/include)
target_link_libraries(periodic_timer_tests PRIVATE ocpn::gtest Threads::Threads)
if (NOT "${ENABLE_SANITIZER}" STREQUAL "none")
  target_compile_options(periodic_timer_tests PRIVATE -fsanitize=${ENABLE_SANITIZER})
  target_link_libraries(periodic_timer_tests PRIVATE -fsanitize=${ENABLE_SANITIZER})
endif ()

include(GoogleTest)
gtest_add_tests(TARGET periodic_timer_tests TEST_LIST periodic_timer_test_names)
set_tests_properties(${periodic_timer_test_names} PROPERTIES TIMEOUT 15)
