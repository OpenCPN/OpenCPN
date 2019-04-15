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
set(SYSTEM_SOUND_CMD "\"\"")
if (APLAY)
  SET(SYSTEM_SOUND_CMD "\"aplay %s\"")
elseif (AFPLAY)
  SET(SYSTEM_SOUND_CMD "\"afplay %s\"")
elseif (WIN32)
  SET(SYSTEM_SOUND_CMD
    "\"PowerShell (New-Object Media.SoundPlayer \\\\\\\"%s\\\\\\\").PlaySync();\"")
endif ()

set(HAVE_SYSTEM_CMD_SOUND "")
if (NOT (OCPN_ENABLE_SYSTEM_CMD_SOUND MATCHES OFF))
  if (APLAY OR AFPLAY OR WIN32)
    set(HAVE_SYSTEM_CMD_SOUND 1)
  elseif (OCPN_ENABLE_SYSTEM_CMD_SOUND MATCHES "ON")
    message(STATUS "OCPN_ENABLE_SYSTEM_CMD_SOUND is set"
                   " but I cannot find aplay(1) or afplay(1)")
    set(HAVE_SYSTEM_CMD_SOUND "")
  endif ()
endif ()

set(HAVE_PORTAUDIO "")
if (NOT OCPN_ENABLE_PORTAUDIO MATCHES OFF)
  include(FindPortaudio)
  if (PORTAUDIO_FOUND)
    message(STATUS "Portaudio Found")
    include_directories(${PORTAUDIO_INCLUDE_DIRS})
    set(SOUND_SUPPORT_LIBS ${SOUND_SUPPORT_LIBS} ${PORTAUDIO_LIBRARIES})
    add_definitions(${PORTAUDIO_DEFINITIONS})
    set(HAVE_PORTAUDIO 1)
elseif (OCPN_ENABLE_PORTAUDIO MATCHES ON)
    message(STATUS "OCPN_ENABLE_PORTAUDIO is set but I cannot find portaudio")
    set(HAVE_PORTAUDIO "")
  endif ()
endif ()

set(HAVE_SNDFILE "")
if (NOT OCPN_ENABLE_SNDFILE MATCHES OFF)
  include(FindLibSndfile)
  if (LIBSNDFILE_FOUND)
    message(STATUS "libsndfile Found")
    include_directories(${LIBSNDFILE_INCLUDE_DIRS})
    set(SOUND_SUPPORT_LIBS ${SOUND_SUPPORT_LIBS} ${LIBSNDFILE_LIBRARIES})
    set(HAVE_SNDFILE 1)
elseif (OCPN_ENABLE_SNDFILE MATCHES ON)
      message(STATUS "ENABLE_LIBSNDFILE is set but I cannot find libsndfile")
  endif ()
endif ()
