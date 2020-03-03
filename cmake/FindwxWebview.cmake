
# - Try to find wxWebview
# Once done this will define
#
# wxWebview_FOUND - system has wxWebview
# WXWEBVIEW_LIBRARIES - Link these to use wxWebview
#
# Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
# Redistribution and use is allowed according to the terms of the New BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions are met:
#
#* Redistributions of source code must retain the above copyright notice, this
#list of conditions and the following disclaimer.
#
#* Redistributions in binary form must reproduce the above copyright notice,
#this list of conditions and the following disclaimer in the
#documentation and/or other materials provided with the distribution.
#
#* Neither the name of the CMake-Modules Project nor the names of its
#contributors may be used to endorse or promote products derived from this
#software without specific prior written permission.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
#ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
#ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



if (WXWEBVIEW_LIBRARIES)
# in cache already
	set(wxWebview_FOUND TRUE)
else (WXWEBVIEW_LIBRARIES)
	find_library(WXWEBVIEW_LIBRARY
	NAMES
	wx_gtk2u_webview-3.0
	PATHS
	/usr/lib
	/usr/local/lib
	/opt/local/lib
	/usr/lib/i386-linux-gnu
	/sw/lib
	)

	set(WXWEBVIEW_LIBRARIES ${WXWEBVIEW_LIBRARY} )

	if (WXWEBVIEW_LIBRARIES)
		set(wxWebview_FOUND TRUE)
endif (WXWEBVIEW_LIBRARIES)

if (wxWebview_FOUND)
	if (NOT wxWebview_FIND_QUIETLY)
		message(STATUS "Found wxWebview: ${WXWEBVIEW_LIBRARIES}")
	endif (NOT wxWebview_FIND_QUIETLY)
else (wxWebview_FOUND)
	if (wxWebview_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find wxWebview")
	endif (wxWebview_FIND_REQUIRED)
endif (wxWebview_FOUND)

# show the WXWEBVIEW_LIBRARIES variables only in the advanced view
mark_as_advanced(WXWEBVIEW_LIBRARIES)

endif (WXWEBVIEW_LIBRARIES)
