#
#  Using options OCPN_ENABLE_SYSTEM_CMD_SOUND, OCPN_ENABLE_PORTAUDIO and
#  OCPN_ENABLE_SNDFILE sets:
#
#     HAVE_SYSTEM_CMD_SOUND - build SystemCmdSound backend
#     SYSTEM_SOUND_CMD      - CLI command used by SystemCmdSound.
#     HAVE_PORTAUDIO        - build portaudio backend.
#     HAVE_SNDFILE          - build libsndfile support for portaudio.

find_program(APLAY aplay)
find_program(AFPLAY afplay)
find_program(OMXPLAYER omxplayer)
set(SYSTEM_SOUND_CMD "" CACHE STRING
    "Hardcoded value for command used as SYSTEM_SOUND_CMD"
)
if ("${SYSTEM_SOUND_CMD}" STREQUAL "")
  if (APLAY)
    SET(SYSTEM_SOUND_CMD "\"aplay %s\"")
  elseif (AFPLAY)
    SET(SYSTEM_SOUND_CMD "\"afplay %s\"")
  elseif(OMXPLAYER)
    SET(SYSTEM_SOUND_CMD "\"omxplayer -o both %s\"")
  elseif (WIN32)
    SET(SYSTEM_SOUND_CMD
      "\"PowerShell (New-Object Media.SoundPlayer \\\\\\\"%s\\\\\\\").PlaySync();\"")
  endif ()
endif ()

message(STATUS "SYSTEM_SOUND_CMD selected : ${SYSTEM_SOUND_CMD}")

set(HAVE_SYSTEM_CMD_SOUND "")
if (OCPN_ENABLE_SYSTEM_CMD_SOUND)
  if (NOT "${SYSTEM_SOUND_CMD}" STREQUAL "")
    set(HAVE_SYSTEM_CMD_SOUND 1)
  else ()
    message(STATUS "OCPN_ENABLE_SYSTEM_CMD_SOUND is set"
                   " but I cannot find aplay(1) or afplay(1) or omxplayer(1)")
  endif ()
  message(STATUS "HAVE_SYSTEM_CMD_SOUND = ${HAVE_SYSTEM_CMD_SOUND}")
endif ()

set(HAVE_PORTAUDIO "")
if (OCPN_ENABLE_PORTAUDIO)
  include(FindPortaudio)
  if (PORTAUDIO_FOUND)
    message(STATUS "Portaudio Found")
    include_directories(${PORTAUDIO_INCLUDE_DIRS})
    set(SOUND_SUPPORT_LIBS ${SOUND_SUPPORT_LIBS} ${PORTAUDIO_LIBRARIES})
    add_definitions(${PORTAUDIO_DEFINITIONS})
    set(HAVE_PORTAUDIO 1)
  else ()
    message(STATUS "OCPN_ENABLE_PORTAUDIO is set but I cannot find portaudio")
  endif ()
endif ()
message(STATUS "HAVE_PORTAUDIO = ${HAVE_PORTAUDIO}")

set(HAVE_SNDFILE "")
if (OCPN_ENABLE_SNDFILE)
  include(FindLibSndfile)
  if (LIBSNDFILE_FOUND)
    message(STATUS "libsndfile Found")
    include_directories(${LIBSNDFILE_INCLUDE_DIRS})
    set(SOUND_SUPPORT_LIBS ${SOUND_SUPPORT_LIBS} ${LIBSNDFILE_LIBRARIES})
    set(HAVE_SNDFILE 1)
  else ()
    message(STATUS "ENABLE_LIBSNDFILE is set but I cannot find libsndfile")
  endif ()
endif ()
message(STATUS "HAVE_SNDFILE = ${HAVE_SNDFILE}")
