cmake_minimum_required(VERSION 3.5)

if (TARGET ocpn::easywsclient)
  return ()
endif ()

set(SRC easywsclient.cpp)

set_source_files_properties(
        easywsclient.cpp PROPERTIES COMPILE_FLAGS "-O0")

add_library(EASYWSCLIENT STATIC ${SRC})
add_library(ocpn::easywsclient ALIAS EASYWSCLIENT)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(EASYWSCLIENT PRIVATE -fvisibility=default -fPIC)
endif ()

target_include_directories(EASYWSCLIENT PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32)
  target_link_libraries(EASYWSCLIENT PRIVATE ws2_32)
endif ()
