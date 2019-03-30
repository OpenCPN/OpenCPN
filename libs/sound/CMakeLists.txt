cmake_minimum_required(VERSION 3.1.0)

if (NOT CMAKE_MODULE_PATH)
  set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
endif ()

set(SRC
    src/AndroidSound.cpp
    src/OCPN_Sound.cpp
    src/OcpnWxSound.cpp
    src/SoundFactory.cpp
    src/SoundFileLoader.cpp
    src/SoundLoaderFactory.cpp
    src/SystemCmdSound.cpp
    include/AndroidSound.h
    include/OCPN_Sound.h
    include/OcpnWxSound.h
    include/SoundFactory.h
    include/SoundFileLoader.h
    include/SoundLoaderFactory.h
    include/SystemCmdSound.h
)

include(FindPortaudio)
include(FindLibSndfile)

#if (NOT wxWidgets_INCLUDE_DIRS)
#    include(SimpleWxConfig)
#    SimpleWxConfig()
#endif ()

if (PORTAUDIO_FOUND)
    set(SRC ${SRC} include/PortAudioSound.h)
    set(SRC ${SRC} src/PortAudioSound.cpp)
endif ()

if (LIBSNDFILE_FOUND)
    set(SRC ${SRC} include/SndfileSoundLoader.h)
    set(SRC ${SRC} src/SndfileSoundLoader.cpp)
endif ()

add_library(SOUND STATIC ${SRC})
add_library(sound::sound ALIAS SOUND)

set_property(TARGET SOUND PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(SOUND 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../build/include
        ${wxWidgets_INCLUDE_DIRS}
)
