/*
*	base.h
*	wxCURL
*
*	Created by Casey O'Donnell on Tue Jun 29 2004.
*	Copyright (c) 2004 Casey O'Donnell. All rights reserved.
*
*  Contributions and Assistance:
*  Ryan Wilcox - Verbose Options
*  Francesco Montorsi - Unicode compatibility mode, misc other changes
*
*	Licence: wxWidgets Licence
*/

#ifndef _WXCURLBASE_H__INCLUDED_
#define _WXCURLBASE_H__INCLUDED_

#ifdef WXMAKINGDLL_WXCURL
    #define WXDLLIMPEXP_CURL WXEXPORT
#elif defined(WXUSINGDLL)
//    #define WXDLLIMPEXP_CURL WXIMPORT
    #define WXDLLIMPEXP_CURL
#else // not making nor using DLL
    #define WXDLLIMPEXP_CURL

    // if we do not define this symbol, cURL header will assume
    // a DLL build is being done and will export symbols:
    #ifndef CURL_STATICLIB
        #define CURL_STATICLIB
    #endif
#endif

#include <wx/event.h>
#include <wx/string.h>
#include <wx/datetime.h>
#include <wx/stream.h>
#include <wx/mstream.h>
#include <wx/sstream.h>
#include <wx/intl.h>        // for _()
#include <wx/filename.h>

// The cURL library header:
#include <curl/curl.h>



// base.h: wxWidgets Declarations for Event Types.
//
//////////////////////////////////////////////////////////////////////

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CURL, wxCURL_DOWNLOAD_EVENT, 6578)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CURL, wxCURL_UPLOAD_EVENT, 6579)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CURL, wxCURL_BEGIN_PERFORM_EVENT, 6580)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CURL, wxCURL_END_PERFORM_EVENT, 6581)
END_DECLARE_EVENT_TYPES()


class WXDLLIMPEXP_CURL wxCurlBase;


//! Private internal class used as base class for wxCurlDownloadEvent and wxCurlUploadEvent.
class WXDLLIMPEXP_CURL wxCurlProgressBaseEvent : public wxEvent
{
public:
    wxCurlProgressBaseEvent(int id, wxEventType type,
                        wxCurlBase *p = NULL, const std::string &url = "")
        : wxEvent(id, type) { m_pCURL = p; m_szURL = url; m_dt = wxDateTime::Now(); }


public:     // misc getters

    //! Returns the curl session which generated this event.
    wxCurlBase *GetCurlSession() const { return m_pCURL; }

    //! Returns the date & time at which this event was generated.
    wxDateTime GetDateTime() const { return m_dt; }

    //! Returns a number in [0;100] range indicating how much has been transferred so far.
    double GetPercent() const 
        { return GetTotalBytes() == 0 ? 0 : (100.0 * (GetTransferredBytes()/GetTotalBytes())); }

    //! Returns the current transfer speed in bytes/second.
    virtual double GetSpeed() const
        { return GetTransferredBytes()/GetElapsedTime().GetSeconds().ToDouble(); }

public:     // wxTimeSpan getters

    //! Returns the time elapsed since the beginning of the download up
    //! to the time this function is called.
    virtual wxTimeSpan GetElapsedTime() const;

    //! Returns the estimated time for the total download since it started.
    virtual wxTimeSpan GetEstimatedTime() const;

    //! Returns the estimated remaining time to the completion of the download.
    virtual wxTimeSpan GetEstimatedRemainingTime() const;


public:     // wxString getters

    //! Returns the URL you are transfering from.
    std::string GetURL() const { return m_szURL; }

    //! Returns the current download/upload speed in a human readable format.
    std::string GetHumanReadableSpeed(const std::string &inv = GetNAText(), int prec = 1) const;

    //! Returns the total bytes to download in a human-readable format.
    std::string GetHumanReadableTotalBytes(const std::string &inv = GetNAText(), int prec = 1) const
        {
            wxString s(inv.c_str(), wxConvUTF8);
            return std::string(wxFileName::GetHumanReadableSize(wxULongLong((unsigned long)GetTotalBytes()), s, prec).mb_str());
        }

    //! Returns the currently transferred bytes in a human-readable format.
    std::string GetHumanReadableTransferredBytes(const std::string &inv = GetNAText(), int prec = 1) const
        {
            wxString s(inv.c_str(), wxConvUTF8);
            return std::string(wxFileName::GetHumanReadableSize(wxULongLong((unsigned long)GetTransferredBytes()), s, prec).mb_str());
        }


public:     // pure virtual functions

    //! Returns the total bytes to transfer.
    virtual double GetTotalBytes() const = 0;

    //! Returns the bytes transferred so far.
    virtual double GetTransferredBytes() const = 0;

protected:
    wxCurlBase *m_pCURL;
    std::string m_szURL;

    // NOTE: we need to store this date time to use it in GetElapsedTime:
    //       we cannot use wxDateTime::Now() there because once the event is constructed,
    //       GetElapsedTime() needs to return always the same value!
    wxDateTime m_dt;
    
public:
    static std::string GetNAText()
    {
        wxString s = _("Not available");
        return std::string(s.mb_str());
    }
};


// base.h: interface for the wxCurlDownloadEvent class.
//
//////////////////////////////////////////////////////////////////////

//! This event gets posted by wxCURL with a frequent interval during operation
//! (roughly once per second) no matter if data is being transfered or not.
//! Unknown/unused argument values passed to the callback will be set to zero 
//! (like if you only download data, the upload size will remain 0).
//! Use the EVT_CURL_PROGRESS(id, function) macro to intercept this event.
class WXDLLIMPEXP_CURL wxCurlDownloadEvent : public wxCurlProgressBaseEvent
{
public:
    wxCurlDownloadEvent();
    wxCurlDownloadEvent(int id, wxCurlBase *originator,
                        const double& rDownloadTotal, const double& rDownloadNow, 
                        const std::string& szURL = "");
    wxCurlDownloadEvent(const wxCurlDownloadEvent& event);

    virtual wxEvent* Clone() const { return new wxCurlDownloadEvent(*this); }


    //! Returns the number of bytes downloaded so far.
    double GetDownloadedBytes() const { return m_rDownloadNow; }
    double GetTransferredBytes() const { return m_rDownloadNow; }

    //! Returns the total number of bytes to download.
    double GetTotalBytes() const { return m_rDownloadTotal; }

    //! Returns the currently downloaded bytes in a human-readable format.
    std::string GetHumanReadableDownloadedBytes(const std::string &inv = wxCurlProgressBaseEvent::GetNAText(), int prec = 1) const
        {
            wxString s(inv.c_str(), wxConvUTF8);
            return std::string(wxFileName::GetHumanReadableSize(wxULongLong((unsigned long)m_rDownloadNow), s, prec).mb_str());
        }


protected:
    double m_rDownloadTotal, m_rDownloadNow;

private:
    DECLARE_DYNAMIC_CLASS(wxCurlDownloadEvent);
};

typedef void (wxEvtHandler::*wxCurlDownloadEventFunction)(wxCurlDownloadEvent&);

#define wxCurlDownloadEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxCurlDownloadEventFunction, &func)

#define EVT_CURL_DOWNLOAD(id, fn) \
    wx__DECLARE_EVT1(wxCURL_DOWNLOAD_EVENT, id, wxCurlDownloadEventHandler(fn))


//! This event gets posted by wxCURL with a frequent interval during operation
//! (roughly once per second) no matter if data is being transfered or not.
//! Unknown/unused argument values passed to the callback will be set to zero 
//! (like if you only download data, the upload size will remain 0).
//! Use the EVT_CURL_PROGRESS(id, function) macro to intercept this event.
class WXDLLIMPEXP_CURL wxCurlUploadEvent : public wxCurlProgressBaseEvent
{
public:
    wxCurlUploadEvent();
    wxCurlUploadEvent(int id, wxCurlBase *originator,
                        const double& rUploadTotal, const double& rUploadNow, 
                        const std::string& szURL = "");
    wxCurlUploadEvent(const wxCurlUploadEvent& event);

    virtual wxEvent* Clone() const { return new wxCurlUploadEvent(*this); }


    //! Returns the number of bytes uploaded so far.
    double GetUploadedBytes() const { return m_rUploadNow; }
    double GetTransferredBytes() const { return m_rUploadNow; }

    //! Returns the total number of bytes to upload.
    double GetTotalBytes() const { return m_rUploadTotal; }

    //! Returns the currently uploaded bytes in a human-readable format.
    std::string GetHumanReadableUploadedBytes(const std::string &inv = wxCurlProgressBaseEvent::GetNAText(), int prec = 1) const
        {
            wxString s(inv.c_str(), wxConvUTF8);
            return std::string(wxFileName::GetHumanReadableSize(wxULongLong((unsigned long)m_rUploadNow), s, prec).mb_str());
        }

protected:
    double m_rUploadTotal, m_rUploadNow;

private:
    DECLARE_DYNAMIC_CLASS(wxCurlUploadEvent);
};

typedef void (wxEvtHandler::*wxCurlUploadEventFunction)(wxCurlUploadEvent&);

#define wxCurlUploadEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxCurlUploadEventFunction, &func)

#define EVT_CURL_UPLOAD(id, fn) \
    wx__DECLARE_EVT1(wxCURL_UPLOAD_EVENT, id, wxCurlUploadEventHandler(fn))



// base.h: interface for the wxCurlBeginPerformEvent class.
//
//////////////////////////////////////////////////////////////////////

//! This event get posted before the beginning of any tranfer operation.
//! Use the EVT_CURL_BEGIN_PERFORM(id, function) macro to intercept this event.
class WXDLLIMPEXP_CURL wxCurlBeginPerformEvent : public wxEvent
{
public:
    wxCurlBeginPerformEvent();
    wxCurlBeginPerformEvent(int id, const std::string& szURL);
    wxCurlBeginPerformEvent(const wxCurlBeginPerformEvent& event);

    virtual wxEvent* Clone() const { return new wxCurlBeginPerformEvent(*this); }

    //! Returns the URL you are going to transfering from/to.
    std::string GetURL() const { return m_szURL; }

protected:
    std::string m_szURL;

private:
    DECLARE_DYNAMIC_CLASS(wxCurlBeginPerformEvent);
};

typedef void (wxEvtHandler::*wxCurlBeginPerformEventFunction)(wxCurlBeginPerformEvent&);

#define wxCurlBeginPerformEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxCurlBeginPerformEventFunction, &func)

#define EVT_CURL_BEGIN_PERFORM(id, fn) \
    wx__DECLARE_EVT1(wxCURL_BEGIN_PERFORM_EVENT, id, wxCurlBeginPerformEventHandler(fn))



// base.h: interface for the wxCurlEndPerformEvent class.
//
//////////////////////////////////////////////////////////////////////

//! This event get posted at the end of any tranfer operation.
//! Use the EVT_CURL_END_PERFORM(id, function) macro to intercept this event.
class WXDLLIMPEXP_CURL wxCurlEndPerformEvent : public wxEvent
{
public:
    wxCurlEndPerformEvent();
    wxCurlEndPerformEvent(int id, const std::string& szURL, const long& iResponseCode);
    wxCurlEndPerformEvent(const wxCurlEndPerformEvent& event);

    virtual wxEvent* Clone() const { return new wxCurlEndPerformEvent(*this); }

    //! Returns the URL you are going to transfering from/to.
    std::string GetURL() const { return m_szURL; }

    //! Returns the response code for the operation.
    long GetResponseCode() const { return m_iResponseCode; }

    //! Returns true if the response code indicates a valid transfer.
    bool IsSuccessful() const { return ((m_iResponseCode > 199) && (m_iResponseCode < 300)); }

protected:
    std::string	m_szURL;
    long		m_iResponseCode;

private:
    DECLARE_DYNAMIC_CLASS(wxCurlEndPerformEvent);
};

typedef void (wxEvtHandler::*wxCurlEndPerformEventFunction)(wxCurlEndPerformEvent&);

#define wxCurlEndPerformEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxCurlEndPerformEventFunction, &func)

#define EVT_CURL_END_PERFORM(id, fn) \
    wx__DECLARE_EVT1(wxCURL_END_PERFORM_EVENT, id, wxCurlEndPerformEventHandler(fn))


// C Function Declarations for LibCURL
//
//////////////////////////////////////////////////////////////////////

extern "C"
{
    int wxcurl_evt_progress_func(void* ptr, double rDlTotal, double rDlNow,
                                 double rUlTotal, double rUlNow);
    int wxcurl_verbose_stream_write (CURL * crlptr , curl_infotype info, char * cStrMessage,
                                     size_t msgSize, void * buffer);
    size_t wxcurl_header_func(void *ptr, size_t size, size_t nmemb, void *stream);

    size_t wxcurl_string_write(void* ptr, size_t size, size_t nmemb, void* stream);
    size_t wxcurl_stream_write(void* ptr, size_t size, size_t nmemb, void* stream);

    size_t wxcurl_string_read(void* ptr, size_t size, size_t nmemb, void* stream);
    size_t wxcurl_stream_read(void* ptr, size_t size, size_t nmemb, void* stream);
}

// base.h: interface for the wxCurlBase class.
//
//////////////////////////////////////////////////////////////////////

//! Tells wxCurlBase to send wxCurlDownloadEvent events
#define wxCURL_SEND_PROGRESS_EVENTS     0x01

//! Tells wxCurlBase to send wxCurlBeginPerformEvent and wxCurlEndPerformEvent events
#define wxCURL_SEND_BEGINEND_EVENTS     0x02

//! By default wxCurlBase won't send events
#define wxCURL_DEFAULT_FLAGS            (0)

//! The "easy" unspecialized interface to libCURL.
//! You may want to look at wxCurlFTP, wxCurlHTTP, wxCurlDAV if you want to have a specialized
//! interface for respectively the FTP, HTTP and WebDAV protocols.
//!
//! wxCurlBase represents a libCURL handle to a "session".
//! To use this interface you should:
//! - create an instance of wxCurlBase
//! - use #SetOpt to set libCURL options you're interested to
//!   or alternatively the other various Set*() functions
//! - call #Perform to perform the operation
class WXDLLIMPEXP_CURL wxCurlBase
{
public:
    wxCurlBase(const wxString& szURL = wxEmptyString, 
               const wxString& szUserName = wxEmptyString,
               const wxString& szPassword = wxEmptyString, 
               wxEvtHandler* pEvtHandler = NULL, int id = wxID_ANY,
               long flags = wxCURL_DEFAULT_FLAGS);

    virtual ~wxCurlBase();


    // LibCURL Abstraction Methods - Wrapping curl_easy calls...

    //! Sets a transfer option for this libCURL session instance.
    //! See the curl_easy_setopt() function call for more info.
    bool SetOpt(CURLoption option, ...);

    //! Gets an info from this libCURL session instance.
    //! See the curl_easy_getinfo() function call for more info.
    bool GetInfo(CURLINFO info, ...) const;

    //! Start the operation as described by the options set previously with #SetOpt.
    //! If you set CURLOPT_UPLOAD to zero and the CURLOPT_WRITEFUNCTION and CURLOPT_WRITEDATA
    //! options to suitable values, a download will be performed.
    //! If you set CURLOPT_UPLOAD to nonzero and the CURLOPT_READFUNCTION and CURLOPT_READDATA
    //! options to suitable values, an upload will be performed.
    //! See the curl_easy_perform() function call for more info.
    bool Perform();


    // Internal handle management:

    //! Initializes the internal libCURL handle. This function is automatically called by
    //! the constructor.
    bool InitHandle();

    //! Closes this libCURL session. This will effectively close all connections this handle 
    //! has used and possibly has kept open until now.
    //! This function is automatically called by the destructor.
    bool CleanupHandle();

    //! Reinit the handle of this libCURL session. Equivalent to call #CleanupHandle and then #InitHandle.
    bool ReInitHandle();

    //! Re-initializes all options previously set on this libCURL session to the default values.
    bool ResetHandle();

    //! Is the underlying libCURL handle valid?
    bool IsOk() const { return m_pCURL != NULL; }

    // Member Data Access Methods (MDA)

    //! Sets the event handler to which the wxCurlDownloadEvent, wxCurlBeginPerformEvent and
    //! wxCurlEndPerformEvent will be sent if they are enabled (see #SetFlags).
    bool			SetEvtHandler(wxEvtHandler* pParent, int id = wxID_ANY);
    wxEvtHandler*	GetEvtHandler() const;
    int             GetId() const;
	void SetAbort(bool a);
	bool GetAbort() const;

    //! Sets the "event policy" of wxCURL: if you pass zero, then no events will ever be sent.
    //! The wxCURL_SEND_PROGRESS_EVENTS and wxCURL_SEND_BEGINEND_EVENTS flags instead tell
    //! wxCURL to send respectively the wxCurlDownloadEvent and wxCurlBeginPerformEvent,
    //! wxCurlEndPerformEvent events.
    void		SetFlags(long flags);
    long        GetFlags() const;

    //! Sets the base URL. This allows you to specify a 'base' URL if you
    //! are performing multiple actions.
    void		SetBaseURL(const wxString& szBaseURL);
    std::string	GetBaseURL() const;

    //! Sets the current URL. The 'base url' will be prepended to the given string.
    void        SetURL(const wxString &szRelativeURL);

    //! Returns the current 'full' URL. I.e. the real URL being used for the transfer.
    std::string    GetURL() const;

    //! Sets the host Port.  This allows you to specify a specific (non-
    //! default port) if you like.  The value -1 means that the default port
    //! will be used.
    void		SetPort(const long& iPort);
    long		GetPort() const;

    //! Sets the Username. If no username is
    //! needed, simply assign an empty string (which is the default).
    void		SetUsername(const wxString& szUsername);
    std::string	GetUsername() const;

    //! Sets the Password. If no password is
    //! needed, simply assign an empty string (which is the default).
    void		SetPassword(const wxString& szPassword);
    std::string	GetPassword() const;

    //! Returns the header of the response.
    std::string	GetResponseHeader() const;
    std::string	GetResponseBody() const;		// May only contain data on NON-GET calls.
    long		GetResponseCode() const;

    //! Should the proxy be used?
    void		UseProxy(const bool& bUseProxy);
    bool		UseProxy() const;

    //! Sets proxy host.
    void		SetProxyHost(const wxString& szProxyHost);
    std::string	GetProxyHost() const;

    //! Sets the username for proxy access (if needed).
    void		SetProxyUsername(const wxString& szProxyUsername);
    std::string	GetProxyUsername() const;

    //! Sets the password for proxy access (if needed).
    void		SetProxyPassword(const wxString& szProxyPassword);
    std::string	GetProxyPassword() const;

    //! Sets the port for proxy access.
    void		SetProxyPort(const long& iProxyPort);
    long		GetProxyPort() const;

    //! Sets verbose mode on/off. Note that in verbose mode a lot of info
    //! will be printed into an internal memory stream which can be queried
    //! using #GetVerboseStream and #GetVerboseString.
    void		SetVerbose(const bool& bVerbose);
    bool		IsVerbose() const;

    //! Writes into the given stream the verbose messages collected so far.
    bool		GetVerboseStream(wxOutputStream& destStream) const;

    //! Appends to the given stream the verbose messages collected so far.
    bool		GetVerboseString(wxString& szStream) const;

    //! Returns a generic, short string describing the last occurred error.
    std::string    GetErrorString() const;

    //! Returns a short string with a detailed description of last occurred error.
    //! This is typically something technical which you may want to hide from the
    //! end users of your application (and e.g. show only in log files).
    std::string    GetDetailedErrorString() const;


    // Static LibCURL Initialization Methods - Call At Program Init and Close...

    //! Initializes the libCURL. Call this only once at the beginning of your program.
    static void Init();

    //! Clean up libCURL. Call this only once at the end of your program.
    static void Shutdown();


    // Static LibCURL Utility Methods

    static wxDateTime GetDateFromString(const wxString& szDate);
    static std::string GetURLEncodedString(const wxString& szData);
    static std::string GetStringFromURLEncoded(const wxString& szData);

    static std::string GetCURLVersion();


protected:      // protected utils used by wxCurl*Thread classes

    friend class wxCurlBaseThread;
    friend class wxCurlSizeQueryThread;
    friend class wxCurlProgressBaseEvent;

    //! Sets a custom callback as the progress callback.
    //! Note that using this function you'll break the dispatching of
    //! wxCurlDownloadEvent and wxCurlUploadEvent unless your own callback
    //! does dispatch the events itself.
    //! wxCURL users should never need to use this function.
    void OverrideProgressCallback(curl_progress_callback newcallback, void *data)
        { m_progressCallback=newcallback; m_progressData=data; }

    //! Returns the time at which started the last transfer "span".
    wxDateTime GetBeginTransferSpan() const
        { return m_dtBeginTransferSpan; }

    //! Returns the current time offset, i.e. the time elapsed in all previous
    //! transfer spans.
    wxTimeSpan GetElapsedTimeOffset() const
        { return m_tsElapsedOffset; }

    //! A transfer span has been completed (i.e. the user paused the transfer).
    void EndTransferSpan()
    {
        wxDateTime now = wxDateTime::Now();
        m_tsElapsedOffset += now - m_dtBeginTransferSpan;
    }

    //! A new transfer span has begun (i.e. the user resumed the transfer).
    void BeginTransferSpan()
    {
        m_dtBeginTransferSpan = wxDateTime::Now();
    }

protected:

    // The internal pointer to the libCURL session.
    CURL*                   m_pCURL;
	// Flag for terminating a possibly hung transfer
	bool                    m_bAbortHungTransfer;

    // libCURL <-> wxString conversions helpers (see below)

    #define wxCURL_BUF2STRING(x)         wxString((const char*)(x), wxConvLibc)

#if wxUSE_UNICODE
    #define wxCURL_STRING2BUF(x)         ((x).ToAscii())
#else
    #define wxCURL_STRING2BUF(x)         wxCharBuffer((x).c_str())
#endif

    #define wxCURL_BUF_ISEMPTY(x)        (strlen(x) == 0)



    // VERY IMPORTANT: all these wxCharBuffers are passed to curl_easy_setopt()
    //                 which does not take ownership of them. Thus we need
    //                 to keep them alive here for all the time m_pCURL handle
    //                 is valid. Also note that we don't use wxStrings to store
    //                 them as libCURL always take char* and doesn't like wchar_t*
    //                 and thus we always need wxCharBuffer storage while wxString
    //                 in wxWidgets Unicode builds would use a wxWCharBuffer internally.
    //                 wxCharBuffer is basically a thin class which handles auto-delete
    //                 of a char[] array.

    // basic connection settings:

    wxCharBuffer            m_szBaseURL;
    wxCharBuffer            m_szCurrFullURL;
    wxCharBuffer            m_szUsername;
    wxCharBuffer            m_szPassword;
    wxCharBuffer            m_szUserPass;

    long                    m_iHostPort;

    // about received headers:

    wxCharBuffer            m_szResponseHeader;
    wxCharBuffer            m_szResponseBody;
    long                    m_iResponseCode;

    // about headers to send:

    wxArrayString           m_arrHeaders;
    struct curl_slist*      m_pHeaders;

    // proxy:

    bool                    m_bUseProxy;
    wxCharBuffer            m_szProxyHost;
    wxCharBuffer            m_szProxyUsername;
    wxCharBuffer            m_szProxyPassword;
    wxCharBuffer            m_szProxyUserPass;
    long                    m_iProxyPort;

    // debugging/verbose mode:

    bool                    m_bVerbose;
    wxStringOutputStream    m_mosVerbose;

    char                    m_szDetailedErrorBuffer[CURL_ERROR_SIZE];
    wxCharBuffer            m_szLastError;

    // for events:
    wxEvtHandler*           m_pEvtHandler;
    int                     m_nId;
    long                    m_nFlags;

    // callbacks which can be overridden by the user:
    curl_progress_callback  m_progressCallback;
    void*                   m_progressData;

    // these two are required in order to be able to support PAUSE/RESUME
    // features in wxCurl*Thread classes. In fact, since the pausing happens
    // is not directly supported by libCURL needs to happen at wxThread::Entry
    // level. This also means that the events which wxCurlBase's callbacks
    // post to the event handler cannot trust libCURL's TOTAL_TIME as reported
    // by easy_curl_getinfo. Rather we need to keep our own timing mechanism.
    //
    // It's very simple: a single transfer session may be divided in one or more
    // transfer "spans" and idle periods.
    // Resuming from idle means start a new transfer span. Pausing means ending
    // the current transfer span.
    // When a new transfer span starts (from Perform() if it's the first span or
    // from BeginTransferSpan when resuming), its beginning is registered into
    // m_dtBeginTransferSpan. When a transfer span ends, its length is added to
    // the "elapsed time offset" m_tsElapsedOffset.
    // This allows wxCurlProgressEvents to calculate the elapsed time just as:
    //    wxDateTime::Now() - m_dtBeginTransferSpan + m_tsElapsedOffset
    wxDateTime              m_dtBeginTransferSpan;
    wxTimeSpan              m_tsElapsedOffset;


protected:      // internal functions

    // CURL Handle Initialization Helper Method
    virtual void	SetCurlHandleToDefaults(const wxString& relativeURL);
    virtual void	SetHeaders();
    virtual void	ResetHeaders();
    virtual void	ResetResponseVars();

    // Output additional warnings/errors when in verbose mode.
    void DumpErrorIfNeed(CURLcode error) const;

protected:      // specialized safe SetOpt-like functions

    // handy overload for char buffers
    bool SetStringOpt(CURLoption option, const wxCharBuffer &str);


    bool SetStringWriteFunction(const wxCharBuffer& str)
    {
        bool ret = true;
        ret &= SetOpt(CURLOPT_WRITEFUNCTION, wxcurl_string_write);
        ret &= SetOpt(CURLOPT_WRITEDATA, (void*)&str);
        return ret;
    }

    bool SetStreamWriteFunction(const wxOutputStream& buf)
    {
        bool ret = true;
        ret &= SetOpt(CURLOPT_WRITEFUNCTION, wxcurl_stream_write);
        ret &= SetOpt(CURLOPT_WRITEDATA, (void*)&buf);
        return ret;
    }

    bool SetStringReadFunction(const wxCharBuffer& str)
    {
        bool ret = true;
        ret &= SetOpt(CURLOPT_READFUNCTION, wxcurl_string_read);
        ret &= SetOpt(CURLOPT_READDATA, (void*)&str);
        return ret;
    }

    bool SetStreamReadFunction(const wxInputStream& buf)
    {
        bool ret = true;
        ret &= SetOpt(CURLOPT_READFUNCTION, wxcurl_stream_read);
        ret &= SetOpt(CURLOPT_READDATA, (void*)&buf);
        return ret;
    }
};

#endif // _WXCURLBASE_H__INCLUDED_
