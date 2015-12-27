/*
 *	telnet.h
 *	wxCURL
 *
 *	Created by Casey O'Donnell on Tue Jun 29 2004.
 *	Copyright (c) 2004 Casey O'Donnell. All rights reserved.
 *
 *	Licence: wxWidgets Licence
 */

// wxCurlTelnet.h: interface for the wxCurlTelnet class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WXCURLTELNET_H__INCLUDED_
#define _WXCURLTELNET_H__INCLUDED_

#include <wx/curl/base.h>

//! A specialized interface to TELNET derived from wxCurlBase.
class WXDLLIMPEXP_CURL wxCurlTelnet : public wxCurlBase
{
public:
	wxCurlTelnet();
	virtual ~wxCurlTelnet();
};

#endif // _WXCURLTELNET_H__INCLUDED_
