/*
 *	dav.h
 *	wxCURL
 *
 *	Created by Casey O'Donnell on Wed Jun 02 2004.
 *	Copyright (c) 2004 Casey O'Donnell. All rights reserved.
 *
 *	Licence: wxWidgets Licence
 */

// dav.h: interface for the wxCurlDAV class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WXCURLDAV_H__INCLUDED_
#define _WXCURLDAV_H__INCLUDED_

#include <stddef.h>

#include <wx/event.h>
#include <wx/window.h>
#include <wx/string.h>
#include <wx/datetime.h>
#include <wx/dynarray.h>

#include <wx/curl/http.h>

class wxXmlNode;


//! A specialized interface to WebDAV derived from wxCurlBase.
class WXDLLIMPEXP_CURL wxCurlDAV : public wxCurlHTTP
{
public:
	wxCurlDAV(const wxString& szURL = wxEmptyString, 
              const wxString& szUserName = wxEmptyString, 
              const wxString& szPassword = wxEmptyString, 
              wxEvtHandler* pEvtHandler = NULL, 
              long flags = wxCURL_DEFAULT_FLAGS);
	virtual ~wxCurlDAV();

	// Action Methods - These All Make Calls To: curl_easy_perform()

	// MKCOL - Creates a Collection Resource at the Location Specified.
	bool		Mkcol(const wxString& szRemoteCol = wxEmptyString);

	// PROPFIND - Queries the Properties of the Resource Specified.  An XML Buffer Can be Specified for Detailed Data.
	bool		Propfind(const wxString& szRemoteLoc = wxEmptyString);
	bool		Propfind(const wxArrayString& arrTags, const wxString& szRemoteLoc = wxEmptyString);
	bool		Propfind(const char* buffer, size_t size, const wxString& szRemoteLoc = wxEmptyString);
	bool		Propfind(wxInputStream& buffer, const wxString& szRemoteLoc = wxEmptyString);

	// PROPATCH - Alters the Properties of the Resource Specified.  An XML Buffer Must be Specified to Modify Data.
	bool		Proppatch(const char* buffer, size_t size, const wxString& szRemoteLoc = wxEmptyString);
	bool		Proppatch(wxInputStream& buffer, const wxString& szRemoteLoc = wxEmptyString);

	// COPY - Copies the Specified Resource from one Location to Another.
	bool		Copy(const wxString& szRemoteLocDest, const bool& bOverwrite = true, const wxString& szRemoteLocSrc = wxEmptyString);

	// MOVE - Moves the Specified Resource from one Location to Another.
	bool		Move(const wxString& szRemoteLocDest, const bool& bOverwrite = true, const wxString& szRemoteLocSrc = wxEmptyString);

	// LOCK - Requests a Lock on the Specified Resource.
	bool		Lock(const wxString& szRemotLoc = wxEmptyString);

	// UNLOCk - Gives up a Lock on the Specified Resource.
	bool		Unlock(const wxString& szRemoteLoc = wxEmptyString);

protected:

	// XML Helper Methods
	virtual wxString ConstructPropfindXml(const wxArrayString& arrTags);

private:

};

#endif // _WXCURLDAV_H__INCLUDED_
