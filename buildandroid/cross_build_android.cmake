
#Toolchain and options definition file for OPenCPN Android build


#  Locations of the cross-compiler tools
# this one is important
SET(CMAKE_SYSTEM_NAME Generic)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /home/dsr/Projects/android-ndk/ndk-toolchain/bin/arm-linux-androideabi-gcc)
SET(CMAKE_CXX_COMPILER   /home/dsr/Projects/android-ndk/ndk-toolchain/bin/arm-linux-androideabi-g++)

# Location of the generic wxWidgets base
SET(wxQt_Base /home/dsr/Projects/wxqt/wxWidgets)

#Location of the specific wxWidgets build (for Qt_Android)

#  This one for static build
SET(wxQt_Build build_android_53)

#  This one for dynamic library build
#SET(wxQt_Build build_android_so)

#Location of the root of the Qt installation
SET(Qt_Base /home/dsr/Qt)