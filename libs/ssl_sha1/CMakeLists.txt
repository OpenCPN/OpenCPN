cmake_minimum_required(VERSION 3.5.0)

if (TARGET ssl::sha1)
    return ()
endif ()

add_library(SHA1 STATIC include/ssl/sha1.h src/sha1.c)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(SHA1 PRIVATE -fvisibility=default -fPIC)
endif ()

target_include_directories(SHA1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(ssl::sha1 ALIAS SHA1)
