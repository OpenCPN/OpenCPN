

#Toolchain and options definition file for OpenCPN Android build


SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR arm)

# specify the cross compilers and tools

if ("${OCPN_TARGET_TUPLE}" MATCHES "Android-arm64")
  set(CMAKE_AR ${tool_base}/bin/aarch64-linux-android-ar)
  set(CMAKE_CXX_COMPILER ${tool_base}/bin/aarch64-linux-android21-clang++)
  set(CMAKE_C_COMPILER ${tool_base}/bin/aarch64-linux-android21-clang)
else ()
  set(CMAKE_AR ${tool_base}/bin/llvm-ar)
  set(CMAKE_CXX_COMPILER ${tool_base}/bin/armv7a-linux-androideabi21-clang++)
  set(CMAKE_C_COMPILER ${tool_base}/bin/armv7a-linux-androideabi21-clang)
endif ()

# Avoid compiler tests, which fail on linking from sysroot.
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

set(TARGET_SUPPORTS_SHARED_LIBS TRUE)

set(CMAKE_SYSROOT ${tool_base}/sysroot)
set(CMAKE_INCLUDE_PATH ${tool_base}/sysroot/usr/include)

