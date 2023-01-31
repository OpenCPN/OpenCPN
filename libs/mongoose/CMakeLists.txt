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

if (NOT WIN32)
  find_package(OpenSSL)
  if (OPENSSL_FOUND)
    message(STATUS "OpenSSL found   ${OPENSSL_INCLUDE_DIR} ${OPENSSL_LIBRARIES}")
    target_include_directories(MONGOOSE PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(MONGOOSE PRIVATE ${OPENSSL_LIBRARIES})
    add_definitions(-DMG_ENABLE_OPENSSL)
  else (OPENSSL_FOUND)
    if (APPLE)
      message(STATUS "Mongoose: APPLE: Setting OpenSSL...")
      target_include_directories(
        MONGOOSE
        PUBLIC /usr/local/opt/openssl@3/include
      )
      target_link_libraries(
        MONGOOSE
        PRIVATE /usr/local/opt/openssl@3/lib/libcrypto.3.dylib
        /usr/local/opt/openssl@3/lib/libssl.3.dylib
      )
    endif (APPLE)
  endif (OPENSSL_FOUND)
else (NOT WIN32)
    target_include_directories(
      MONGOOSE
      PRIVATE ${CMAKE_SOURCE_DIR}/cache/buildwin/include/openssl
    )
    target_link_libraries(
      MONGOOSE
      PRIVATE ${CMAKE_SOURCE_DIR}/cache/buildwin/libssl.lib
    )
   target_link_libraries(
      MONGOOSE
      PRIVATE ${CMAKE_SOURCE_DIR}/cache/buildwin/libcrypto.lib
    )
endif (NOT WIN32)

add_definitions(-DMG_ENABLE_OPENSSL)

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

if (MSVC)
target_include_directories(
  MONGOOSE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../cache/buildwin/include
)
endif (MSVC)
