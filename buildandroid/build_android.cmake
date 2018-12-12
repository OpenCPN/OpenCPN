

#Toolchain and options definition file for OPenCPN Android build


#  Locations of the cross-compiler tools
# this one is important
SET(CMAKE_SYSTEM_NAME Generic)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)


# specify the cross compiler
SET(CMAKE_C_COMPILER     /home/dsr/Android/android-ndk-r10d/tools_19_48/bin/arm-linux-androideabi-gcc)
SET(CMAKE_CXX_COMPILER   /home/dsr/Android/android-ndk-r10d/tools_19_48/bin/arm-linux-androideabi-g++)

SET(TARGET_SUPPORTS_SHARED_LIBS TRUE)

#Location of the root of the Android NDK
SET(NDK_Base /home/dsr/Android/android-ndk-r10d )

#SET(CMAKE_C_COMPILER     /home/dsr/Projects/crystax-ndk-10.3.2/android-toolchain-10-3-2-4-9/bin/arm-linux-androideabi-gcc)
#SET(CMAKE_CXX_COMPILER   /home/dsr/Projects/crystax-ndk-10.3.2/android-toolchain-10-3-2-4-9/bin/arm-linux-androideabi-g++)

#Location of the root of the Android NDK
#SET(NDK_Base /home/dsr/Projects/crystax-ndk-10.3.2 )
