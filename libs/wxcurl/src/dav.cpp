/*
 *	dav.cpp
 *	wxCURL
 *
 *	Created by Casey O'Donnell on Wed Jun 02 2004.
 *	Copyright (c) 2004 Casey O'Donnell. All rights reserved.
 *
 *
 *	Licence: wxWidgets Licence
 */

// dav.cpp: implementation of the wxCurlDAV class.
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

#include <wx/mstream.h>

#include <wx/curl/dav.h>

//////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

wxCurlDAV::wxCurlDAV(const wxString& szURL /*= wxEmptyString*/, 
                     const wxString& szUserName /*= wxEmptyString*/, 
                     const wxString& szPassword /*= wxEmptyString*/, 
                     wxEvtHandler* pEvtHandler /*= NULL*/, 
                     long flags /*=wxCURL_DEFAULT_FLAGS*/)
: wxCurlHTTP(szURL, szUserName, szPassword, pEvtHandler, flags)
{
}

wxCurlDAV::~wxCurlDAV()
{
}

//////////////////////////////////////////////////////////////////////
// Member Data Access Methods
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Action Methods
//////////////////////////////////////////////////////////////////////

bool wxCurlDAV::Mkcol(const wxString& szRemoteCol /*= wxEmptyString*/)
{
	if(m_pCURL)
	{
        SetCurlHandleToDefaults(szRemoteCol);
		
		SetOpt(CURLOPT_CUSTOMREQUEST, "MKCOL");
		SetStringWriteFunction(m_szResponseBody);

		if(Perform())
		{
			return ((m_iResponseCode > 199) && (m_iResponseCode < 300));
		}
	}

	return false;
}

bool wxCurlDAV::Propfind(const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	return Propfind(NULL, 0, szRemoteLoc);
}

bool wxCurlDAV::Propfind(const wxArrayString& arrTags, const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	wxString szXml = ConstructPropfindXml(arrTags);

	return Propfind((const char*)(szXml.c_str()), szXml.Len(), szRemoteLoc);
}

bool wxCurlDAV::Propfind(const char* buffer, size_t size, const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	wxMemoryInputStream inStream(buffer, size);

	return Propfind(inStream, szRemoteLoc);
}

bool wxCurlDAV::Propfind(wxInputStream& buffer, const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	curl_off_t iSize = buffer.GetSize();

	if(m_pCURL && buffer.IsOk())
	{
		if(iSize == (~(ssize_t)0))	// wxCurlDAV does not know how to upload unknown length streams.
			return false;

		SetCurlHandleToDefaults(szRemoteLoc);

		m_arrHeaders.Add(wxS("Depth: 1"));
		m_arrHeaders.Add(wxS("Content-Type: text/xml; charset=\"utf-8\""));

		SetHeaders();

		if(iSize > 0)
		{
			SetOpt(CURLOPT_UPLOAD, TRUE);
			SetOpt(CURLOPT_INFILESIZE_LARGE, iSize);
			SetStreamReadFunction(buffer);
		}

		SetOpt(CURLOPT_CUSTOMREQUEST, "PROPFIND");
		SetStringWriteFunction(m_szResponseBody);

		if(Perform())
		{
			ResetHeaders();

			return ((m_iResponseCode > 199) && (m_iResponseCode < 300));
		}

		ResetHeaders();
	}

	return false;
}

bool wxCurlDAV::Proppatch(const char* buffer, size_t size, const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	wxMemoryInputStream inStream(buffer, size);

	return Proppatch(inStream, szRemoteLoc);
}

bool wxCurlDAV::Proppatch(wxInputStream& buffer, const wxString& szRemoteLoc /*= wxEmptyString*/)
{
	curl_off_t iSize = buffer.GetSize();

	if(m_pCURL && buffer.IsOk())
	{
		if(iSize == (~(ssize_t)0))	// wxCurlDAV does not know how to upload unknown length streams.
			return false;

		SetCurlHandleToDefaults(szRemoteLoc);

		m_arrHeaders.Add(wxS("Content-Type: text/xml; charset=\"utf-8\""));

		SetHeaders();

		if(iSize > 0)
		{
			SetOpt(CURLOPT_UPLOAD, TRUE);
			SetOpt(CURLOPT_INFILESIZE_LARGE, iSize);
			SetStreamReadFunction(buffer);
		}

		SetOpt(CURLOPT_CUSTOMREQUEST, "PROPPATCH");
		SetStringWriteFunction(m_szResponseBody);

		if(Perform())
		{
			ResetHeaders();

			return ((m_iResponseCode > 199) && (m_iResponseCode < 300));
		}

		ResetHeaders();
	}

	return false;
}

bool wxCurlDAV::Copy(const wxString& szRemoteLocDest, const bool& bOverwrite /*= true*/, 
                     const wxString& szRemoteLocSrc /*= wxEmptyString*/)
{
	if(m_pCURL)
	{
		SetCurlHandleToDefaults(szRemoteLocSrc);

		m_arrHeaders.Add(wxS("Destination: ") + szRemoteLocDest);
		
		if(!bOverwrite)
			m_arrHeaders.Add(wxS("Overwrite: F"));

		SetHeaders();

		SetOpt(CURLOPT_CUSTOMREQUEST, "COPY");
		SetStringWriteFunction(m_szResponseBody);

		if(Perform())
		{
			ResetHeaders();

			return ((m_iResponseCode > 199) && (m_iResponseCode < 300));
		}

		ResetHeaders();
	}

	return false;
}

bool wxCurlDAV::Move(const wxString& szRemoteLocDest, const bool& bOverwrite /*= true*/, 
                     const wxString& szRemoteLocSrc /*= wxEmptyString*/)
{
	if(m_pCURL)
	{
		SetCurlHandleToDefaults(szRemoteLocSrc);

		m_arrHeaders.Add(wxS("Destination: ") + szRemoteLocDest);
		
		if(!bOverwrite)
			m_arrHeaders.Add(wxS("Overwrite: F"));

		SetHeaders();

		SetOpt(CURLOPT_CUSTOMREQUEST, "MOVE");
		SetStringWriteFunction(m_szResponseBody);

		if(Perform())
		{
			ResetHeaders();

			return ((m_iResponseCode > 199) && (m_iResponseCode < 300));
		}

		ResetHeaders();
	}

	return false;
}

bool wxCurlDAV::Lock(const wxString& WXUNUSED(szRemoteLoc /*=wxEmptyString*/))
{
	return false;
}

bool wxCurlDAV::Unlock(const wxString& WXUNUSED(szRemoteLoc /*=wxEmptyString*/))
{
	return false;
}

//////////////////////////////////////////////////////////////////////
// Helper Methods
//////////////////////////////////////////////////////////////////////

wxString wxCurlDAV::ConstructPropfindXml(const wxArrayString& arrTags)
{
	wxString szXml;

	szXml += wxS("<?xml version=\"1.0\" encoding=\"utf-8\" ?>") wxS("\n");
	szXml += wxS("<D:propfind xmlns:D=\"DAV:\">") wxS("\n");

	if(arrTags.Count() > 0)
	{
		szXml += wxS("\t") wxS("<D:prop>") wxS("\n");
	
		for(unsigned int i = 0; i < arrTags.Count(); i++)
		{
			szXml += wxS("\t\t") wxS("<D:");
			szXml += arrTags[i];
			szXml += wxS("/>") wxS("\n");
		}

		szXml += wxS("\t") wxS("</D:prop>") wxS("\n");
	}
	else
	{
		szXml += wxS("\t") wxS("<D:allprop/>") wxS("\n");
	}

	szXml += wxS("</D:propfind>") wxS("\n");

	return szXml;
}

