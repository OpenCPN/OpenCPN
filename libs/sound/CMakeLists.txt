cmake_minimum_required(VERSION 3.1.0)

if (TARGET ocpn::sound)
    return ()
endif ()

if (NOT CMAKE_MODULE_PATH)
    set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
endif ()

set(SRC
    src/OCPN_Sound.cpp
    src/OcpnWxSound.cpp
    src/SoundFactory.cpp
    src/SoundFileLoader.cpp
    src/SoundLoaderFactory.cpp
    src/SystemCmdSound.cpp
    include/OCPN_Sound.h
    include/OcpnWxSound.h
    include/SoundFactory.h
    include/SoundFileLoader.h
    include/SoundLoaderFactory.h
    include/SystemCmdSound.h
)

if (WIN32)
    list(APPEND SRC src/MswSound.cpp include/MswSound.h)
endif ()

if (QT_ANDROID)
    list(APPEND SRC src/AndroidSound.cpp include/AndroidSound.h)
endif ()

if (PORTAUDIO_FOUND)
    list(APPEND SRC include/PortAudioSound.h src/PortAudioSound.cpp)
endif ()

if (LIBSNDFILE_FOUND)
    list(APPEND SRC include/SndfileSoundLoader.h src/SndfileSoundLoader.cpp)
endif ()

add_library(SOUND STATIC ${SRC})
add_library(ocpn::sound ALIAS SOUND)

set_property(TARGET SOUND PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(SOUND
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_BINARY_DIR}/include
        ${wxWidgets_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}/../../include
        ${CMAKE_CURRENT_BINARY_DIR}/../../../include
)

target_link_libraries(SOUND PRIVATE "${SOUND_SUPPORT_LIBS}")
