cmake_minimum_required(VERSION 3.14)


project(opencpn_tests)
set(CMAKE_CXX_STANDARD 14)
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(PROJ_SRC ${PROJECT_SOURCE_DIR}/../src)

add_executable(
  test1 test1.cpp
  ${PROJ_SRC}/observable.cpp
  ${PROJ_SRC}/observable_msg.cpp
)

find_package(wxWidgets REQUIRED core base)
include(${wxWidgets_USE_FILE})
target_link_libraries(test1 PRIVATE ${wxWidgets_LIBRARIES})
target_include_directories(test1 PRIVATE ${PROJECT_SOURCE_DIR}/../include)

add_subdirectory(${PROJECT_SOURCE_DIR}/../libs/wxJSON ${CMAKE_BINARY_DIR}/wxJSON)
target_link_libraries(test1 PRIVATE ocpn::wxjson)

target_link_libraries(test1 PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test1)

