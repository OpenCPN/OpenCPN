/*
 *	ftp.h
 *	wxCURL
 *
 *	Created by Casey O'Donnell on Tue Jun 29 2004.
 *	Copyright (c) 2004 Casey O'Donnell. All rights reserved.
 *
 *	Licence: wxWidgets Licence
 */

#ifndef _WXCURLFTP_H__INCLUDED_
#define _WXCURLFTP_H__INCLUDED_

#include <wx/curl/base.h>

// C Function Declarations for LibCURL
//
//////////////////////////////////////////////////////////////////////

extern "C"
{
}

// ftp.h: interface for the wxCurlFTP class.
//
//////////////////////////////////////////////////////////////////////

//! A specialized interface to FTP derived from wxCurlBase.
class WXDLLIMPEXP_CURL wxCurlFTP : public wxCurlBase
{
public:
	enum TransferMode
	{
		kASCII,
		kBINARY
	};

	wxCurlFTP(const wxString& szURL = wxEmptyString,
              const wxString& szUserName = wxEmptyString,
              const wxString& szPassword = wxEmptyString,
              wxEvtHandler* pEvtHandler = NULL, int id = wxID_ANY,
              long flags = wxCURL_DEFAULT_FLAGS);
	virtual ~wxCurlFTP();

	// Member Data Access Methods (MDA)

	// MDA - Get/Set Transfer Mode
	void			SetTransferMode(const TransferMode& tmMode);
	TransferMode	GetTransferMode() const;

	void			SetToBinary();
	void			SetToAscii();

	bool			IsAscii() const;
	bool			IsBinary() const;

	// MDA - Get/Set PORT Options
	void		UsePortOption(const bool& bUsePort);
	bool		UsePortOption() const;

	void		SetPortParam(const wxString& szParam = wxS("-"));
	wxString	GetPortParam() const;

	// MDA - Get/Set EPRT Use
	void		UseEPRT(const bool& bUseEPRT);
	bool		UseEPRT() const;

	// MDA - Get/Set EPSV Use
	void		UseEPSV(const bool& bUseEPSV);
	bool		UseEPSV() const;

	// MDA - Get/Set Append Setting
	void		AppendOnUpload(const bool& bAppend);
	bool		AppendOnUpload() const;

	// MDA - Get/Set Create Missing Dirs Setting
	void		CreateMissingDirs(const bool& bCreateMissing);
	bool		CreateMissingDirs() const;

	// MDA - Set Quote Strings
	bool		SetQuoteList(const wxArrayString& arrQuote);
	bool		AppendQuote(const wxString& szQuote, const bool& bClear = false);

	// MDA - Set Pre Quote Strings
	bool		SetPreQuoteList(const wxArrayString& arrQuote);
	bool		AppendPreQuote(const wxString& szQuote, const bool& bClear = false);

	// MDA - Set Post Quote Strings
	bool		SetPostQuoteList(const wxArrayString& arrQuote);
	bool		AppendPostQuote(const wxString& szQuote, const bool& bClear = false);

	// Action Methods - These All Make Calls To: curl_easy_perform()

	// GET - These Methods Allow a Caller to Retrieve a Given Resource
	bool		Get(const wxString& szFilePath, const wxString& szRemoteFile = wxEmptyString);
	size_t		Get(char*& buffer, const wxString& szRemoteFile = wxEmptyString);
	bool		Get(wxOutputStream& buffer, const wxString& szRemoteFile = wxEmptyString);

	// PUT - These Methods Allow a Caller to Place a Resource on the Host
	bool		Put(const wxString& szFilePath, const wxString& szRemoteFile = wxEmptyString);
	bool		Put(const char* buffer, size_t size, const wxString& szRemoteFile = wxEmptyString);
	bool		Put(wxInputStream& buffer, const wxString& szRemoteFile = wxEmptyString);

	// MKDIR - This Method Allows a Caller to Create a Directory
	bool		MkDir(const wxString& szRemoteLoc = wxEmptyString);

	// RMDIR - This Method Allows a Caller to Remove a Directory
	bool		RmDir(const wxString& szRemoteLoc = wxEmptyString);

	// DELETE - This Method Allows a Caller to Remove a Resource
	bool		Delete(const wxString& szRemoteLoc = wxEmptyString);

	// RENAME - This Method Allows a Caller to Rename a Resource
	bool		Rename(const wxString& szRemoteLocName, const wxString& szRemoteFile = wxEmptyString);

	// LIST - This Method Allows a Caller to Retrieve the Contents of a Resource
	bool		List(const wxString& szRemoteLoc = wxEmptyString);
	bool		Nlst(const wxString& szRemoteLoc = wxEmptyString);

	// INFO - This Method Allows a Caller to Retrieve Information Regarding a Resource
	bool		Info(const wxString& szRemoteLoc = wxEmptyString);

protected:

	virtual void SetCurlHandleToDefaults(const wxString& relativeURL);
	virtual void SetCurlHandleQuoteOpts();

	void	ResetAllQuoteLists();
	void	ResetQuoteList();
	void	ResetPreQuoteList();
	void	ResetPostQuoteList();

	struct curl_slist*	m_pQuote;
	struct curl_slist*	m_pPreQuote;
	struct curl_slist*	m_pPostQuote;

	wxString			m_szCurrFullPath;
	wxString			m_szCurrFilename;

	bool				m_bCreateMissingDirs;

	bool				m_bAppend;
	bool				m_bUsePortOption;

	TransferMode		m_tmMode;

	bool				m_bUseEPRT;
	bool				m_bUseEPSV;

	wxCharBuffer		m_szPortParam;

private:

};

#endif // _WXCURLFTP_H__INCLUDED_
