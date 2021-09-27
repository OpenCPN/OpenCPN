/////////////////////////////////////////////////////////////////////////////
// Name:        thread.h
// Purpose:     wxCurlDownloadThread, wxCurlUploadThread
// Author:      Francesco Montorsi
// Created:     2007/04/14
// RCS-ID:      $Id: thread.h 1237 2010-03-10 21:52:47Z frm $
// Copyright:   (c) 2007 Francesco Montorsi
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WXCURL_THREAD_H_
#define _WXCURL_THREAD_H_

// wxWidgets headers
#include "wx/defs.h"
#include "wx/thread.h"

#include "wx/curl/base.h"

#include <memory>

//! One of the protocols supported by wxCurl.
enum wxCurlProtocol
{
    wxCP_INVALID = -1,

    wxCP_HTTP,
    wxCP_FTP
};

//! One of the possible errors code returned by wxCurl.
enum wxCurlThreadError
{
    wxCTE_NO_ERROR = wxTHREAD_NO_ERROR,          //!< There was no error.

    wxCTE_NO_RESOURCE = wxTHREAD_NO_RESOURCE,    //!< There were insufficient resources to create a new thread.
    wxCTE_ALREADY_RUNNING = wxTHREAD_RUNNING,    //!< The thread is already running.
    wxCTE_INVALID_PROTOCOL,                      //!< The given URL requires an unspported protocol.
    wxCTE_NO_VALID_STREAM,                       //!< The input/output stream could not be created or is invalid.
    wxCTE_ABORTED,                               //!< The thread was user-aborted through wxCurlBaseThread::Abort().
    wxCTE_CURL_ERROR
            //!< LibCURL failed. You can use thread->GetCurlSession()->GetErrorString() to get more info.
};

//! The stack size for wxCurl threads.
#define wxCURL_THREAD_STACK_SIZE            2048


// ----------------------------------------------------------------------------
// wxCurlBaseThread
// ----------------------------------------------------------------------------

//! Base class for wxCurl threads.
class WXDLLIMPEXP_CURL wxCurlBaseThread : public wxThread
{
protected:

    //! The URL identifying the resource to download/upload.
    wxString m_url;

    //! The libcurl handle being used for the transfer.
    std::shared_ptr<wxCurlBase> m_pCurl;

    //! The protocol being used for the transfer.
    wxCurlProtocol m_protocol;

    //! The event handler which gets m_pCurl's events.
    wxEvtHandler *m_pHandler;
    int m_nId;

    //! This flag is set to true when the thread has been cancelled using Delete().
    //! Since it's continuosly read by the running thread and maybe written from
    //! other threads, it needs a mutex.
    bool m_bAbort;
    wxMutex m_bAbortMutex;

public:
    wxCurlBaseThread(wxEvtHandler *handler = NULL,
                     int id = wxID_ANY)
        : wxThread(wxTHREAD_JOINABLE)
    {
        m_protocol = wxCP_INVALID;
        m_pCurl = 0;

        m_nId = id;
        m_pHandler = handler;

        m_bAbort = false;
    }

    ~wxCurlBaseThread()
    {
        m_pCurl = 0;
    }

public:     // thread execution management

    //! Returns true if this thread is ready to be started using e.g. #StartTransfer.
    virtual bool IsOk() const
        { return !m_url.empty() && m_pCurl != 0; }

    //! Starts the transfer. This is equivalent to call wxCurlDownloadThread::Download or
    //! wxCurlUploadThread::Upload.
    virtual wxCurlThreadError StartTransfer() = 0;

    //! Aborts this thread.
    virtual void Abort();

    //! Waits for the completion of the transfer.
    virtual wxCurlThreadError Wait();

    //! Pauses the transfer.
    virtual wxCurlThreadError Pause();

    //! Resumes the transfer.
    virtual wxCurlThreadError Resume();


public:     // setters

    //! Sets the event handler to which wxCurlBeginPerformEvent, wxCurlEndPerformEvent
    //! and wxCurlDownloadEvent/wxCurlUploadEvents will be posted.
    void SetEvtHandler(wxEvtHandler *handler, int id = wxID_ANY)
        {
            wxCHECK_RET(!IsAlive(), wxS("Cannot use this function after the tranfer has begun"));
            m_pHandler=handler; m_nId=id;
        }

    //! Sets the URL to download/upload from/to.
    wxCurlThreadError SetURL(const wxString &url);
    wxCurlThreadError SetURL(const wxString &url, std::shared_ptr<wxCurlBase> pCurl);

public:     // getters

    wxEvtHandler *GetEvtHandler() const
        { return m_pHandler; }
    int GetId() const
        { return m_nId; }

    bool IsAborting()
        { return m_bAbort; }


    //! Returns the wxCurlBase-derived object which is being used for the transfer.
    //! Note that the returned value will be NULL if you've not called #SetURL yet.
    //! You can cast it to the wxCurlBase-derived class associated with the return
    //! value of GetProtocol() (e.g. if GetProtocol() returns wxCP_HTTP, you can cast
    //! GetCurlSession() to wxCurlHTTP).
    wxCurlBase *GetCurlSession() const
        { return m_pCurl.get(); }

    std::shared_ptr<wxCurlBase> &GetCurlSharedPtr()
        { return m_pCurl; }

    //! Returns the protocol used for the transfer.
    //! This function will return something different from wxCP_INVALID only after
    //! a call to #SetURL with a valid URL.
    wxCurlProtocol GetProtocol() const
        { return m_protocol; }

    //! Returns the URL of current transfer.
    wxString GetURL() const
        { return m_url; }

public:     // public utils

    //! Returns the protocol which should be used to download the resource
    //! associated with the given URL.
    static wxCurlProtocol GetProtocolFromURL(const wxString &url);

    //! Returns a pointer to a wxCurlBase-derived class suitable for handling
    //! transfers on the given protocol.
    //! You'll need to wx_static_cast the return value to the
    //! right class in order to be able to set/get further options
    //! (e.g. url/username/password/proxy/etc etc).
    static std::shared_ptr<wxCurlBase>CreateHandlerFor(wxCurlProtocol prot);

protected:

    virtual bool TestDestroy();
    virtual void OnExit();


    // change the access type of some wxThread functions which should not
    // be used on wxCurlBaseThread-derived classes.

    virtual wxCurlThreadError Create(unsigned int stackSize)
        { return (wxCurlThreadError)wxThread::Create(stackSize); }
    virtual wxCurlThreadError Run()
        { return (wxCurlThreadError)wxThread::Run(); }
    virtual wxCurlThreadError Delete()
        { return (wxCurlThreadError)wxThread::Delete(); }
};


// ----------------------------------------------------------------------------
// wxCurlDownloadThreadOutputFilter
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CURL wxCurlDownloadThread;

// private class used by wxCurlDownloadThread
class wxCurlDownloadThreadOutputFilter : public wxOutputStream
{
protected:
    wxCurlDownloadThread *m_thread;
    wxOutputStream *m_stream;

public:
    wxCurlDownloadThreadOutputFilter(wxCurlDownloadThread *thread)
        { m_thread = thread; m_stream = NULL; }

    void SetStream(wxOutputStream *realStream)
        { m_stream = realStream; }

    virtual size_t OnSysWrite(const void *buffer, size_t bufsize);

    virtual bool IsOk() const
        { return m_thread && m_stream && m_stream->IsOk(); }

    wxOutputStream *GetRealStream() const
        { return m_stream; }

    wxFileOffset GetLength() const
        { return m_stream->GetLength(); }

    bool Close()
        { return m_stream->Close(); }
};


// ----------------------------------------------------------------------------
// wxCurlDownloadThread
// ----------------------------------------------------------------------------

//! A simple joinable thread which allows downloading
//! resources from the net without blocking the GUI of your app.
class WXDLLIMPEXP_CURL wxCurlDownloadThread : public wxCurlBaseThread
{
    friend class wxCurlDownloadThreadOutputFilter;      // needs to access our TestDestroy()

protected:

    //! The output stream for downloaded data.
    wxCurlDownloadThreadOutputFilter m_output;

public:
    wxCurlDownloadThread(wxEvtHandler *handler = NULL,
                         int id = wxID_ANY,
                         const wxString &url = wxEmptyString,
                         wxOutputStream *out = NULL)
        : wxCurlBaseThread(handler, id),
          m_output(this)
    {
        if (!url.IsEmpty())
            Download(url, out);
    }

public:     // public API

    //! Sets the output stream where the downloaded data are written.
    //! If you pass NULL to this function, then a new temporary file will be used.
    wxCurlThreadError SetOutputStream(wxOutputStream *out = NULL);

    //! Returns the output stream for downloaded data.
    wxOutputStream *GetOutputStream() const
        {
            wxCHECK_MSG(!IsRunning(), NULL,
                        wxS("You cannot access the output stream while the thread is running!"));
            return m_output.GetRealStream();
        }

    //! Returns true if this thread is ready to be started using #Download.
    virtual bool IsOk() const
        { return wxCurlBaseThread::IsOk() && m_output.IsOk(); }

    //! Creates and runs this thread for download of the given URL in the given
    //! output stream (internally calls #SetURL and #SetOutputStream).
    wxCurlThreadError Download(const wxString &url, wxOutputStream *out = NULL);

    //! Downloads the URL previously set with #SetURL using the output stream
    //! previously set with #SetOutputStream.
    wxCurlThreadError Download();

protected:

    // change access policy to force the user of the better-readable Download() method:
    virtual wxCurlThreadError StartTransfer()
        { return Download(); }

    virtual void *Entry();
};


// ----------------------------------------------------------------------------
// wxCurlUploadThreadInputFilter
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CURL wxCurlUploadThread;

// private class
class wxCurlUploadThreadInputFilter : public wxInputStream
{
protected:
    wxCurlUploadThread *m_thread;
    wxInputStream *m_stream;

public:
    wxCurlUploadThreadInputFilter(wxCurlUploadThread *thread)
        { m_thread = thread; m_stream = NULL; }

    void SetStream(wxInputStream *realStream)
        { m_stream = realStream; }

    virtual size_t OnSysRead(void *buffer, size_t bufsize);

    virtual bool IsOk() const
        { return m_thread && m_stream && m_stream->IsOk(); }

    wxInputStream *GetRealStream() const
        { return m_stream; }

    wxFileOffset GetLength() const
        { return m_stream->GetLength(); }

    char Peek()
        { return m_stream->Peek(); }
};


// ----------------------------------------------------------------------------
// wxCurlUploadThread
// ----------------------------------------------------------------------------

//! A simple joinable thread which allows uploading
//! resources to the net without blocking the GUI of your app.
class WXDLLIMPEXP_CURL wxCurlUploadThread : public wxCurlBaseThread
{
    friend class wxCurlUploadThreadInputFilter;      // needs to access our TestDestroy()

protected:

    //! The input stream for uploaded data.
    wxCurlUploadThreadInputFilter m_input;

public:
    wxCurlUploadThread(wxEvtHandler *handler = NULL,
                         int id = wxID_ANY,
                         const wxString &url = wxEmptyString,
                         wxInputStream *in = NULL)
        : wxCurlBaseThread(handler, id),
          m_input(this)
    {
        if (!url.IsEmpty())
            Upload(url, in);
    }

public:     // public API

    //! Sets the output stream where the downloaded data are written.
    //! If you pass NULL to this function, then a new temporary file will be used.
    wxCurlThreadError SetInputStream(wxInputStream *in = NULL);

    //! Returns the output stream for downloaded data.
    wxInputStream *GetInputStream() const
        {
            wxCHECK_MSG(!IsRunning(), NULL,
                        wxS("You cannot access the output stream while the thread is running!"));
            return m_input.GetRealStream();
        }

    //! Returns true if this thread is ready to be started using #Upload.
    virtual bool IsOk() const
        { return wxCurlBaseThread::IsOk() && m_input.IsOk(); }

    //! Creates and runs this thread for upload to the given URL of the given
    //! input stream (internally calls #SetURL and #SetOutputStream).
    wxCurlThreadError Upload(const wxString &url, wxInputStream *in = NULL);

    //! Uploads the URL previously set with #SetURL using the input stream
    //! previously set with #SetInputStream.
    wxCurlThreadError Upload();

protected:

    // change access policy to force the user of the better-readable Upload() method:
    virtual wxCurlThreadError StartTransfer()
        { return Upload(); }

    virtual void *Entry();
};


#endif // _WXCURL_THREAD_H_

