# - Try to find Portaudio Once done this will define
#
# PORTAUDIO_FOUND - system has Portaudio PORTAUDIO_INCLUDE_DIRS - the Portaudio
# include directory PORTAUDIO_LIBRARIES - Link these to use Portaudio
# PORTAUDIO_DEFINITIONS - Compiler switches required for using Portaudio
#
# Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
# Redistribution and use is allowed according to the terms of the New BSD
# license. For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# - Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# - Neither the name of the CMake-Modules Project nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# FIXME: Needs indentation...
# FIXME: Seem to provoke bugs if invoked on Windows
# after -DOCPN_ENABLE_PORTAUDIO=ON

if (TARGET portaudio::portaudio)
  return ()
endif ()

if (PORTAUDIO_LIBRARIES AND PORTAUDIO_INCLUDE_DIRS)
  # in cache already
  set(PORTAUDIO_FOUND TRUE)
  return()
endif ()

find_path(PORTAUDIO_INCLUDE_DIR NAMES portaudio.h)
find_library(PORTAUDIO_LIBRARY NAMES portaudio)

set(PORTAUDIO_INCLUDE_DIRS ${PORTAUDIO_INCLUDE_DIR})
set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIBRARY})

if (PORTAUDIO_INCLUDE_DIRS AND PORTAUDIO_LIBRARIES)
  set(PORTAUDIO_FOUND TRUE)
endif ()

if (PORTAUDIO_FOUND)
  if (NOT Portaudio_FIND_QUIETLY)
    message(STATUS "Found Portaudio: ${PORTAUDIO_LIBRARIES}")
  endif ()
elseif (Portaudio_FIND_REQUIRED)
   message(FATAL_ERROR "Could not find Portaudio")
endif ()

# show the PORTAUDIO_INCLUDE_DIRS and PORTAUDIO_LIBRARIES variables only in
# the advanced view
mark_as_advanced(PORTAUDIO_INCLUDE_DIRS PORTAUDIO_LIBRARIES)

if (PORTAUDIO_FOUND)
  add_library(_portaudio_if INTERFACE)
  add_library(portaudio::portaudio ALIAS _portaudio_if)
  target_include_directories(
    _portaudio_if INTERFACE ${PORTAUDIO_INCLUDE_DIRS}
  )
  target_link_libraries(_portaudio_if INTERFACE ${PORTAUDIO_LIBRARIES})
endif ()
