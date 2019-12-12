# Find FFMPEG - we have it disabled in our wxSVG
#
# This code is UNTESTED snd UNUSED

if (TARGET ocpn::ffmpeg)
    return ()
endif ()

add_library(_FFMPEG INTERFACE)

set(FFmpeg_FIND_COMPONENTS
    AVCODEC AVFORMAT AVUTIL SWSCALE SWRESAMPLE AVRESAMPLE)
unset(_FFMPEG_LIBRARIES CACHE)
find_package(FFmpeg)
if (NOT (AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND AND SWSCALE_FOUND))
  message(FATAL_ERROR "FFmpeg component required, but not found!")
endif ()

target_link_libraries(_FFMPEG ${FFMPEG_LIBRARIES ${SWSCALE_LIBRARIES})
target_include_directories(_FFMPEG ${FFMPEG_INCLUDE_DIRS})
add_library(ocpn::ffmpeg ALIAS _FFMPEG)

