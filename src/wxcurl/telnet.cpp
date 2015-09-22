/*
 *	telnet.cpp
 *	wxCURL
 *
 *	Created by Casey O'Donnell on Tue Jun 29 2004.
 *	Copyright (c) 2004 Casey O'Donnell. All rights reserved.
 *
 *
 *	Licence: wxWidgets Licence
 */

// telnet.cpp: implementation of the wxCurlTelnet class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Headers
//////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#ifdef __WXMSW__
    #include <wx/msw/msvcrt.h>      // useful to catch memory leaks when compiling under MSVC 
#endif

#include <wx/curl/telnet.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

wxCurlTelnet::wxCurlTelnet()
{

}

wxCurlTelnet::~wxCurlTelnet()
{

}
