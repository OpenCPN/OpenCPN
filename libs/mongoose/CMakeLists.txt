cmake_minimum_required(VERSION 3.1.0)

if (TARGET ocpn::mongoose)
    return ()
endif ()

set (SRC
  src/mongoose.c
  include/mongoose.h
)
add_library(MONGOOSE STATIC ${SRC})
add_library(ocpn::mongoose ALIAS MONGOOSE)

#set_property(TARGET MONGOOSE PROPERTY COMPILE_FLAGS "-fvisibility=default")

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(
    MONGOOSE PRIVATE
      -fvisibility=default -Wno-unknown-pragmas -fPIC
  )
endif ()
target_include_directories(
  MONGOOSE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
