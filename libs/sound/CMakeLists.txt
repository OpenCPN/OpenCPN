cmake_minimum_required(VERSION 3.5.0)

# Debug junk
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

if (TARGET ocpn::sound)
  return ()
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(PORTAUDIO_MSG "Use portaudio(3) to play sounds if available")
if (WIN32 OR APPLE OR QT_ANDROID)
  option(OCPN_ENABLE_PORTAUDIO ${PORTAUDIO_MSG} OFF)
else ()
  option(OCPN_ENABLE_PORTAUDIO ${PORTAUDIO_MSG} ON)
endif ()
cmake_dependent_option(
  OCPN_ENABLE_SNDFILE "Use libsndfile for portaudio if available." ON
  "OCPN_ENABLE_PORTAUDIO" ON
)

option(OCPN_ENABLE_SYSTEM_CMD_SOUND
       "Use aplay(1), afplay(1) etc. to play sounds if available"
       ${cmd_snd_default}
)
set(OCPN_SOUND_CMD "" CACHE STRING
    "Hardcoded value for CLI command used in SystemCmdSound backend"
)

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
find_package(Portaudio)
find_package(LibSndfile)

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

include(SoundConfig)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(SOUND PRIVATE -fvisibility=default -fPIC)
endif ()

target_include_directories(SOUND
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_BINARY_DIR}/include
    ${wxWidgets_INCLUDE_DIRS}
)
target_link_libraries(SOUND PRIVATE ${wxWidgets_LIBRARIES})

if (QT_ANDROID)
  target_link_libraries(SOUND PRIVATE ocpn::java-jvm)
endif ()
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/snd_config.h.in
  ${CMAKE_BINARY_DIR}/include/snd_config.h
)
