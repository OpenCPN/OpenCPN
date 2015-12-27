/*
*	http.cpp
*	wxCURL
*
*	Created by Casey O'Donnell on Tue Jun 29 2004.
*	Copyright (c) 2004 Casey O'Donnell. All rights reserved.
*
*
*	Licence: wxWidgets Licence
*/

// http.cpp: implementation of the wxCurlHTTP class.
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
#include <wx/wfstream.h>

#include <wx/curl/http.h>

//////////////////////////////////////////////////////////////////////
// C Functions for LibCURL
//////////////////////////////////////////////////////////////////////
extern "C"
{
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

wxCurlHTTP::wxCurlHTTP(const wxString& szURL /*= wxEmptyString*/, 
                    const wxString& szUserName /*= wxEmptyString*/, 
                    const wxString& szPassword /*= wxEmptyString*/, 
                    wxEvtHandler* pEvtHandler /*= NULL*/, 
                    int id /*= wxID_ANY*/,
                    long flags /*= wxCURL_DEFAULT_FLAGS*/)
: wxCurlBase(szURL, szUserName, szPassword, pEvtHandler, id, flags),
m_pPostHead(NULL), m_pPostTail(NULL), m_bUseCookies(false), m_szCookieFile("-"),
m_pszPostFieldsData(NULL), m_iPostDataSize(0)
{
}

wxCurlHTTP::~wxCurlHTTP()
{
    ResetPostData();
}

bool wxCurlHTTP::IsResponseOk() const
{
    return ((m_iResponseCode > 199) && (m_iResponseCode < 300));
}

//////////////////////////////////////////////////////////////////////
// Member Data Access Methods
//////////////////////////////////////////////////////////////////////

void wxCurlHTTP::UseCookies(const bool& bUseCookies)
{
    m_bUseCookies = bUseCookies;
}

bool wxCurlHTTP::UseCookies() const
{
    return m_bUseCookies;
}

void wxCurlHTTP::SetCookieFile(const wxString& szFilePath)
{
    m_szCookieFile = wxCURL_STRING2BUF(szFilePath);
}

wxString wxCurlHTTP::GetCookieFile() const
{
    return wxCURL_BUF2STRING(m_szCookieFile);
}

//////////////////////////////////////////////////////////////////////
// Post Data Methods
//////////////////////////////////////////////////////////////////////

bool wxCurlHTTP::AddForm(const bool& bClear, const wxString& szName,
                        struct curl_forms* pForms, CURLFORMcode* outErr)
{
    if(bClear)
        ResetPostData();

    CURLFORMcode res = CURL_FORMADD_OK;

    res = curl_formadd(&m_pPostHead, &m_pPostTail, CURLFORM_COPYNAME,
                       (const char*)szName.ToAscii(),
                       CURLFORM_ARRAY, pForms, CURLFORM_END);

    if (outErr != NULL)
        *outErr = res;  // sometimes you want to/need to see what's going on

    return (res == CURL_FORMADD_OK);
}

bool wxCurlHTTP::AddBufferToForm(const bool& bClear, const wxString& szName,
                                char* buffer, size_t len, CURLFORMcode* outErr)
{
    if(bClear)
        ResetPostData();

    CURLFORMcode res = CURL_FORMADD_OK;

    res = curl_formadd(&m_pPostHead, &m_pPostTail, CURLFORM_COPYNAME,
                       (const char*)szName.ToAscii(),
                       CURLFORM_COPYCONTENTS, buffer, CURLFORM_CONTENTSLENGTH, 
                       len, CURLFORM_END);

    if (outErr != NULL)
        *outErr = res;  // sometimes you want to/need to see what's going on

    return (res == CURL_FORMADD_OK);


}

//////////////////////////////////////////////////////////////////////
// Action Methods
//////////////////////////////////////////////////////////////////////

bool wxCurlHTTP::Options(const wxString& szRemoteFile /*= wxEmptyString*/)
{
    if(m_pCURL)
    {
        SetCurlHandleToDefaults(szRemoteFile);

        SetOpt(CURLOPT_CUSTOMREQUEST, "OPTIONS");
        SetStringWriteFunction(m_szResponseBody);

        if(Perform())
        {
            return IsResponseOk();
        }
    }

    return false;
}

bool wxCurlHTTP::Head(const wxString& szRemoteFile /*= wxEmptyString*/)
{
    if(m_pCURL)
    {
        SetCurlHandleToDefaults(szRemoteFile);

        SetOpt(CURLOPT_HTTPGET, TRUE);
        SetOpt(CURLOPT_NOBODY, TRUE);
        
        if(Perform())
        {
            return IsResponseOk();
        }
    }

    return false;
}

bool wxCurlHTTP::Post(const char* buffer, size_t size, const wxString& szRemoteFile /*= wxEmptyString*/)
{
    wxMemoryInputStream inStream(buffer, size);

    return Post(inStream, szRemoteFile);
}

bool wxCurlHTTP::Post(wxInputStream& buffer, const wxString& szRemoteFile /*= wxEmptyString*/)
{
    curl_off_t iSize = 0;

    if(m_pCURL && buffer.IsOk())
    {
        SetCurlHandleToDefaults(szRemoteFile);

        SetHeaders();
        iSize = buffer.GetSize();

        if(iSize == (~(ssize_t)0))	// wxCurlHTTP does not know how to upload unknown length streams.
            return false;

        SetOpt(CURLOPT_POST, TRUE);
        SetOpt(CURLOPT_POSTFIELDSIZE_LARGE, iSize);
        SetStreamReadFunction(buffer);
        SetStringWriteFunction(m_szResponseBody);

        if(Perform())
        {
            ResetHeaders();
            return IsResponseOk();
        }
    }

    return false;
}

bool wxCurlHTTP::Post(const wxString& szRemoteFile /*= wxEmptyString*/)
{
    if(m_pCURL && m_pPostHead && m_pPostTail)
    {
        SetCurlHandleToDefaults(szRemoteFile);

        SetOpt(CURLOPT_POST, TRUE);
        SetOpt(CURLOPT_HTTPPOST, m_pPostHead);
        SetStringWriteFunction(m_szResponseBody);

        if(Perform())
        {
            return IsResponseOk();
        }
    }

    return false;
}

bool wxCurlHTTP::Trace(const wxString& szRemoteFile /*= wxEmptyString*/)
{
    if(m_pCURL)
    {
        SetCurlHandleToDefaults(szRemoteFile);

        m_arrHeaders.Add(wxS("Content-type: message/http"));

        SetHeaders();

        SetOpt(CURLOPT_CUSTOMREQUEST, "TRACE");
        SetStringWriteFunction(m_szResponseBody);

        if(Perform())
        {
            ResetHeaders();

            return IsResponseOk();
        }

        ResetHeaders();
    }

    return false;
}

bool wxCurlHTTP::Get(const wxString& szFilePath, const wxString& szRemoteFile /*= wxEmptyString*/)
{
    wxFFileOutputStream outStream(szFilePath);

    return Get(outStream, szRemoteFile);
}

size_t wxCurlHTTP::Get(char*& buffer, const wxString& szRemoteFile /*= wxEmptyString*/)
{
    wxMemoryOutputStream outStream;

    buffer = NULL;
    size_t iRetVal = 0;

    if(Get(outStream, szRemoteFile))
    {
        iRetVal = outStream.GetSize();
        buffer = (char*)malloc(iRetVal + 1);

        if(buffer)
        {
            outStream.CopyTo(buffer, outStream.GetSize());
            buffer[iRetVal] = '\0';
            return iRetVal;
        }
        
        free(buffer);
        buffer = NULL;
    }

    return 0;
}

bool wxCurlHTTP::Get(wxOutputStream& buffer, const wxString& szRemoteFile /*=wxEmptyString*/)
{
    if(m_pCURL && buffer.IsOk())
    {
        SetCurlHandleToDefaults(szRemoteFile);

        SetHeaders();
        SetOpt(CURLOPT_HTTPGET, TRUE);
        SetStreamWriteFunction(buffer);

        if(Perform())
        {
            ResetHeaders();
            return IsResponseOk();
        }
    }
    
    return false;
}

bool wxCurlHTTP::Put(const wxString& szFilePath, const wxString& szRemoteFile /*= wxEmptyString*/)
{
    wxFFileInputStream inStream(szFilePath);

    return Put(inStream, szRemoteFile);
}

bool wxCurlHTTP::Put(const char* buffer, size_t size, const wxString& szRemoteFile /*= wxEmptyString*/)
{
    wxMemoryInputStream inStream(buffer, size);

    return Put(inStream, szRemoteFile);
}

bool wxCurlHTTP::Put(wxInputStream& buffer, const wxString& szRemoteFile /*= wxEmptyString*/)
{
    curl_off_t iSize = 0;

    if(m_pCURL && buffer.IsOk())
    {
        SetCurlHandleToDefaults(szRemoteFile);

        iSize = buffer.GetSize();

        if(iSize == (~(ssize_t)0))	// wxCurlHTTP does not know how to upload unknown length streams.
            return false;

        SetOpt(CURLOPT_UPLOAD, TRUE);
        SetOpt(CURLOPT_PUT, TRUE);
        SetStreamReadFunction(buffer);
        SetOpt(CURLOPT_INFILESIZE_LARGE, (curl_off_t)iSize);
        SetStringWriteFunction(m_szResponseBody);

        if(Perform())
        {
            return IsResponseOk();
        }
    }

    return false;
}

bool wxCurlHTTP::Delete(const wxString& szRemoteLoc /*= wxEmptyString*/)
{
    if(m_pCURL)
    {
        SetCurlHandleToDefaults(szRemoteLoc);

        SetOpt(CURLOPT_CUSTOMREQUEST, "DELETE");
        SetStringWriteFunction(m_szResponseBody);

        if(Perform())
        {
            return IsResponseOk();
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// Helper Methods
//////////////////////////////////////////////////////////////////////

void wxCurlHTTP::ResetPostData()
{
    m_pszPostFieldsData = NULL;
    m_iPostDataSize = 0;

    if(m_pPostHead && m_pPostTail)
    {
        curl_formfree(m_pPostHead);

        m_pPostHead = NULL;
        m_pPostTail = NULL;
    }
}

void wxCurlHTTP::SetCurlHandleToDefaults(const wxString& relativeURL)
{
    wxCurlBase::SetCurlHandleToDefaults(relativeURL);

    if(m_bUseCookies)
    {
        SetStringOpt(CURLOPT_COOKIEJAR, m_szCookieFile);
    }
}

