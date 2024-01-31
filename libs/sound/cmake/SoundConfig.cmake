#
#  Using options OCPN_ENABLE_SYSTEM_CMD_SOUND, OCPN_ENABLE_PORTAUDIO and
#  OCPN_ENABLE_SNDFILE sets:
#
#     HAVE_SYSTEM_CMD_SOUND - build SystemCmdSound backend
#     OCPN_SOUND_CMD        - CLI command used by SystemCmdSound.
#     HAVE_PORTAUDIO        - build portaudio backend.
#     HAVE_SNDFILE          - build libsndfile support for portaudio.

if ("${OCPN_SOUND_CMD}" STREQUAL "")
  find_program(APLAY aplay)
  find_program(AFPLAY afplay)
  find_program(OMXPLAYER omxplayer)
  if (APLAY)
    set(OCPN_SOUND_CMD "${APLAY} %s")
  elseif (AFPLAY)
    set(OCPN_SOUND_CMD "${AFPLAY} %s")
  elseif (OMXPLAYER)
    set(OCPN_SOUND_CMD "${OMXPLAYER} -o both %s")
  elseif (WIN32)
    string(CONCAT _win_cmd
      "PowerShell "
      "(New-Object Media.SoundPlayer \\\\\\\"%s\\\\\\\").PlaySync();"
    )
    set(OCPN_SOUND_CMD ${_win_cmd})
  endif ()
endif ()
set(OCPN_SOUND_CMD  \"${OCPN_SOUND_CMD}\")

set(HAVE_SYSTEM_CMD_SOUND "")
if (OCPN_ENABLE_SYSTEM_CMD_SOUND)
  if (NOT "${OCPN_SOUND_CMD}" STREQUAL "")
    message(STATUS "CLI sound command: ${OCPN_SOUND_CMD}")
    set(HAVE_SYSTEM_CMD_SOUND 1)
  else ()
    message(STATUS "OCPN_ENABLE_SYSTEM_CMD_SOUND is set"
                   " but I cannot find aplay(1) or afplay(1) or omxplayer(1)")
  endif ()
else ()
  message(STATUS "SystemCmdSound i. e., using CLI sound tools is disabled")
endif ()

set(HAVE_PORTAUDIO "")
if (OCPN_ENABLE_PORTAUDIO)
  find_package(Portaudio)
  if (PORTAUDIO_FOUND)
    message(STATUS "Portaudio Found")
    set(HAVE_PORTAUDIO 1)
    target_include_directories(SOUND PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
    target_link_libraries(SOUND PUBLIC ${PORTAUDIO_LIBRARIES})
    target_compile_definitions(SOUND PRIVATE ${PORTAUDIO_DEFINITIONS})
  else ()
    message(STATUS "OCPN_ENABLE_PORTAUDIO is set but I cannot find portaudio")
  endif ()
else ()
  message(STATUS "portaudio is disabled")
endif ()

set(HAVE_SNDFILE "")
if (OCPN_ENABLE_SNDFILE)
  find_package(LibSndfile)
  if (LIBSNDFILE_FOUND)
    message(STATUS "libsndfile Found")
    set(HAVE_SNDFILE 1)
    target_include_directories(SOUND PRIVATE ${LIBSNDFILE_INCLUDE_DIRS})
    target_link_libraries(SOUND PUBLIC ${LIBSNDFILE_LIBRARIES})
  else ()
    message(STATUS "ENABLE_LIBSNDFILE is set but I cannot find libsndfile")
  endif ()
else ()
  message(STATUS "libsndfile is disabled")
endif ()
