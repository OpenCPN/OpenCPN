/*
 *	davtool.h
 *	wxCURL
 *
 *	Created by Casey O'Donnell on Thu Jul 15 2004.
 *	Copyright (c) 2004 Casey O'Donnell. All rights reserved.
 *
 *	Licence: wxWidgets Licence
 */

// wxCurlDAVTool.h
//
//////////////////////////////////////////////////////////////////////

#ifndef _WXCURLDAVTOOL_H__INCLUDED_
#define _WXCURLDAVTOOL_H__INCLUDED_

#include <wx/curl/dav.h>

// davtool.h: interface for the wxCurlDAVFs class.
//
//////////////////////////////////////////////////////////////////////

class wxCurlDAVTool;

class WXDLLIMPEXP_CURL wxCurlDAVFs
{
friend class wxCurlDAVTool;

public:
	wxCurlDAVFs();
	wxCurlDAVFs(const wxString& szHREF			,
				const wxString& szStatus		,
				const wxString& szCreationDate	,
				const wxString& szLastModified	,
				const long& iContentLength		,
				const wxString& szContentType	);
	~wxCurlDAVFs();

	bool IsDirectory() const			{ return m_szContentType == wxS("httpd/unix-directory"); }
	bool IsOK() const					{ return m_szStatus.Right(6) == wxS("200 OK"); }

	wxString GetHREF() const			{ return m_szHREF; }

	wxString GetStatus() const			{ return m_szStatus; }

	wxDateTime GetCreationDate() const	{ return m_dtCreationDate; }
	wxDateTime GetLastModified() const	{ return m_dtLastModified; }

	long GetContentLength() const		{ return m_iContentLength; }

	wxString GetContentType() const		{ return m_szContentType; }
	wxString GetType() const			{ return m_szContentType.BeforeFirst('/'); }
	wxString GetSubType() const			{ return m_szContentType.AfterLast('/'); }


protected:
	wxString		m_szHREF;

	wxString		m_szStatus;

	wxDateTime		m_dtCreationDate;
	wxDateTime		m_dtLastModified;

	long			m_iContentLength;

	wxString		m_szContentType;
};

WX_DECLARE_USER_EXPORTED_OBJARRAY(wxCurlDAVFs, wxArrayDAVFs, WXDLLIMPEXP_CURL);

// davtool.h: interface for the wxCurlDAVTool class.
//
//////////////////////////////////////////////////////////////////////

class WXDLLIMPEXP_CURL wxCurlDAVTool : public wxCurlDAV
{
public:
	wxCurlDAVTool(const wxString& szURL = wxEmptyString,
                  const wxString& szUserName = wxEmptyString,
                  const wxString& szPassword = wxEmptyString,
                  wxEvtHandler* pEvtHandler = NULL,
                  long flags = wxCURL_DEFAULT_FLAGS);
	virtual ~wxCurlDAVTool();

	// More Complex Action Methods - These All Make Calls To: curl_easy_perform()
	// These routines have more 'intelligence' than simple WebDAV calls.
	bool		GetDAVFs(wxArrayDAVFs& fs, const wxString& szRemoteLoc = wxEmptyString);
	bool		Exists(const wxString& szRemoteLoc = wxEmptyString);
	bool		IsDirectory(const wxString& szRemoteLoc = wxEmptyString);
	bool		HasDirectory(const wxString& szRemoteLoc = wxEmptyString) { return IsDirectory(szRemoteLoc); }
	bool		IsOk(const wxString& szRemoteLoc = wxEmptyString);
	wxDateTime	GetLastModified(const wxString& szRemoteLoc = wxEmptyString);
	wxDateTime	GetCreationDate(const wxString& szRemoteLoc = wxEmptyString);
	long		GetContentLength(const wxString& szRemoteLoc = wxEmptyString);
	wxString	GetContentType(const wxString& szRemoteLoc = wxEmptyString);

protected:

	// XML Helper Methods
	bool		ParseResponseXml(wxCurlDAVFs& fsItem, wxXmlNode* pNode);
	bool		ParsePropstatXml(wxCurlDAVFs& fsItem, wxXmlNode* pNode);
	bool		ParsePropsXml(wxCurlDAVFs& fsItem, wxXmlNode* pNode);

private:
};

#endif // _WXCURLDAVTOOL_H__INCLUDED_
