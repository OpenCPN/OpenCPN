# Toolchain and options definition file for OpenCPN Android build

SET(CMAKE_SYSTEM_NAME Android)
SET(CMAKE_SYSTEM_VERSION 21)

set(CMAKE_ANDROID_NDK $ENV{HOME}/android-sdk/ndk/26.1.10909125)

if ("${OCPN_TARGET_TUPLE}" MATCHES "Android-arm64")
  set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
else ()
  set(CMAKE_ANDROID_ARCH_ABI armeabi-v7a)
endif ()

set(TARGET_SUPPORTS_SHARED_LIBS TRUE)
