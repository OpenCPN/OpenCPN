

#Toolchain and options definition file for OpenCPN Android build


#  Locations of the cross-compiler tools
# this one is important
SET(CMAKE_SYSTEM_NAME Generic)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# Specify the cross compilation sysroot location
set(CMAKE_SYSROOT /home/dsr/lib/android-sysroot)

# specify the cross compiler and tools
SET(CMAKE_C_COMPILER     /home/dsr/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi28-clang)
SET(CMAKE_CXX_COMPILER   /home/dsr/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi28-clang++)
SET(CMAKE_AR   /home/dsr/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/arm-linux-androideabi-ar)
#SET(CMAKE_C_CREATE_SHARED_LIBRARY aarch64-linux-android-ld)
#SET(CMAKE_CXX_FLAGS="-fuse-ld=lld")
#SET(CMAKE_C_FLAGS="-fuse-ld=lld")

#set(CMAKE_FIND_ROOT_PATH ~/Projects/sysroot/bin)
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# -DCMAKE_AR=/home/dsr/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android-ar
# -DCMAKE_CXX_COMPILER=/home/dsr/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang++
# -DCMAKE_C_COMPILER=/home/dsr/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang

SET(TARGET_SUPPORTS_SHARED_LIBS TRUE)

#Location of the root of the Android NDK
#SET(NDK_Base /home/dsr/Android/android-ndk-r10d )
