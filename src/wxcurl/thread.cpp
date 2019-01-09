/////////////////////////////////////////////////////////////////////////////
// Name:        thread.cpp
// Purpose:     wxCurlDownloadThread, wxCurlUploadThread
// Author:      Francesco Montorsi
// Created:     2007/04/14
// RCS-ID:      $Id: thread.cpp 1240 2010-03-10 23:54:25Z frm $
// Copyright:   (c) 2007 Francesco Montorsi
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////


// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// includes
#ifndef WX_PRECOMP
    #include <wx/log.h>
#endif

#ifdef __WXMSW__
    #include <wx/msw/msvcrt.h>      // useful to catch memory leaks when compiling under MSVC 
#endif

#include <wx/wfstream.h>
#include <wx/filename.h>

#include "wx/curl/thread.h"
#include "wx/curl/http.h"
#include "wx/curl/ftp.h"


// ---------------------
// wxCurlBaseThread
// ---------------------

/* static */
wxCurlProtocol wxCurlBaseThread::GetProtocolFromURL(const wxString &url)
{
    // I'm sure this check could be done in a better way...
    if (url.StartsWith(wxS("http:")) || url.StartsWith(wxS("https:")))
        return wxCP_HTTP;
    if (url.StartsWith(wxS("ftp:")) || url.StartsWith(wxS("ftps:")))
        return wxCP_FTP;
    return wxCP_INVALID;
}

/* static */
std::shared_ptr<wxCurlBase> wxCurlBaseThread::CreateHandlerFor(wxCurlProtocol prot)
{
    switch (prot)
    {
        case wxCP_HTTP:
            return std::shared_ptr<wxCurlBase>(new wxCurlHTTP); // std::make_shared<wxCurlHTTP>();
        case wxCP_FTP:
            return std::shared_ptr<wxCurlBase>(new wxCurlFTP); // std::make_shared<wxCurlFTP>();
        default:
            return std::shared_ptr<wxCurlBase>(nullptr);
    }
}

wxCurlThreadError wxCurlBaseThread::SetURL(const wxString &url)
{
    return SetURL( url, std::shared_ptr<wxCurlBase>(nullptr));
}

wxCurlThreadError wxCurlBaseThread::SetURL(const wxString &url, std::shared_ptr<wxCurlBase> pCurl )
{
    wxCHECK_MSG(!IsAlive(), wxCTE_NO_RESOURCE, wxS("Cannot use this function after the tranfer has begun"));

    // which protocol is required by given url?
    wxCurlProtocol curr = GetProtocolFromURL(url);
    if (curr == wxCP_INVALID)
        return wxCTE_INVALID_PROTOCOL;

    if (curr != m_protocol && pCurl == 0)
    {
        wxASSERT(m_pCurl == 0);
        m_protocol = curr;

        // we need to (re)create the m_pCurl object
        m_pCurl = CreateHandlerFor(m_protocol);
    }
    else if (pCurl != 0) {
        m_protocol = curr;
        m_pCurl = pCurl;
    }

    if (!m_pCurl || !m_pCurl->IsOk())
        return wxCTE_INVALID_PROTOCOL;

    // enable event sending (it's the only way the wxCurlDownloadThread user can
    // receive info about the progress of the transfer)
    m_pCurl->SetEvtHandler(GetEvtHandler(), GetId());
    m_pCurl->SetFlags(wxCURL_SEND_PROGRESS_EVENTS | wxCURL_SEND_BEGINEND_EVENTS);

    m_url = url;

    return wxCTE_NO_ERROR;
}

void wxCurlBaseThread::OnExit()
{
    if (m_pCurl->IsVerbose())
        wxLogDebug(wxS("wxCurlBaseThread - exiting"));

}

bool wxCurlBaseThread::TestDestroy()
{
    if (wxThread::TestDestroy())
        return true;

    wxMutexLocker lock(m_bAbortMutex);
    return m_bAbort;
}

wxCurlThreadError wxCurlBaseThread::Wait()
{
    // Entry() returns 1 for success, 0 for failure
    bool ret = (bool)(wxThread::Wait() != 0);

    if (m_bAbort)
        // ret is false but that's not due to a network error:
        // user stopped the thread through an Abort() call
        return wxCTE_ABORTED;

    // if it was not user-aborted but ret is still false, then
    // a network error occurred:
    return ret ? wxCTE_NO_ERROR : wxCTE_CURL_ERROR;
}

void wxCurlBaseThread::Abort()
{
    {
        wxMutexLocker lock(m_bAbortMutex);
        m_bAbort = true;
    }

    if (IsPaused())
        Resume();
	else
        if (m_pCurl)
            m_pCurl->SetAbort(true);

    Wait();     // should always return wxCTE_ABORTED in this case
}

wxCurlThreadError wxCurlBaseThread::Pause()
{
    if (m_pCurl)
        m_pCurl->EndTransferSpan();

    return (wxCurlThreadError)wxThread::Pause();
}

wxCurlThreadError wxCurlBaseThread::Resume()
{
    if (m_pCurl)
        m_pCurl->BeginTransferSpan();

    return (wxCurlThreadError)wxThread::Resume();
}


// -----------------------------------
// wxCurlDownloadThreadOutputFilter
// -----------------------------------

size_t wxCurlDownloadThreadOutputFilter::OnSysWrite(const void *buffer, size_t bufsize)
{
    // VERY IMPORTANT: we need to call TestDestroy() on the associated
    //                 thread to make sure it can be paused/stopped (see wxThread docs)

    if (m_thread->TestDestroy())
    {
        if (m_thread->m_pCurl->IsVerbose())
            wxLogDebug(wxS("[wxCURL] a wxCurlDownloadThread has been aborted - ignore following message:"));

        // returning a size != bufsize we tell libcurl to stop the tranfer
        // and thus the wxCurlBase::Perform() call done (indirectly) by wxCurlDownloadThread::Entry()
        // will immediately complete and we'll exit the thread.
        // NOTE: we could also call wxThread::Exit() from here... but notifying libcurl
        //       of a "fake error" seems a better solution (allows libcurl to cleanup its internal)
        return (size_t)-1;
    }

    return m_stream->Write(buffer, bufsize).LastWrite();
}


// ---------------------
// wxCurlDownloadThread
// ---------------------

wxCurlThreadError wxCurlDownloadThread::SetOutputStream(wxOutputStream *out)
{
    wxCHECK_MSG(!IsAlive(), wxCTE_NO_RESOURCE, wxS("Cannot use this function after the transfer has begun"));

    if (!out)
    {
        if (!m_output)
        {
            // create a temporary file output stream
            // NOTE: by default we create a wxFileOutputStream and not e.g. a wxMemoryOutputStream
            //       because the downloaded file may be quite large and thus we don't want to
            //       eat lots of RAM. Also, the hard disk should be fast enough for any download.
            m_output.SetStream( new wxFileOutputStream(wxFileName::CreateTempFileName(wxS("download"))) );
            if (!m_output.IsOk())
                return wxCTE_NO_VALID_STREAM;
        }
    }
    else
    {
        m_output.SetStream(out);

        // the user-provided output stream must be valid
        if (!m_output.IsOk())
            return wxCTE_NO_VALID_STREAM;
    }

    return wxCTE_NO_ERROR;
}

wxCurlThreadError wxCurlDownloadThread::Download()
{
    wxCHECK_MSG(!IsAlive(), wxCTE_NO_RESOURCE, wxS("Cannot use this function after the transfer has begun"));

    // create & run this thread
    wxCurlThreadError ret;
    if ((ret=Create(wxCURL_THREAD_STACK_SIZE)) != wxCTE_NO_ERROR)
        return ret;
    if ((ret=Run()) != wxCTE_NO_ERROR)
        return ret;

    return wxCTE_NO_ERROR;
}

wxCurlThreadError wxCurlDownloadThread::Download(const wxString &url, wxOutputStream *out)
{
    wxCurlThreadError ret;

    if ((ret=SetURL(url)) != wxCTE_NO_ERROR)
        return ret;
    if ((ret=SetOutputStream(out)) != wxCTE_NO_ERROR)
        return ret;

    return Download();
}

void *wxCurlDownloadThread::Entry()
{
    if (!m_pCurl || !m_pCurl->IsOk() || !m_output.IsOk())
        return NULL;

    // NOTE: the TestDestroy() function will still be called in this thread
    //       context by the m_output's OnSysWrite function which in turn is
    //       called from libcurl whenever some new data arrives
    if (m_pCurl->IsVerbose())
        wxLogDebug(wxS("wxCurlDownloadThread - downloading from %s"), m_url.c_str());
    switch (m_protocol)
    {
        case wxCP_HTTP:
            return (void*)wx_static_cast(wxCurlHTTP*, m_pCurl.get())->Get(m_output, m_url);

        case wxCP_FTP:
            return (void*)wx_static_cast(wxCurlFTP*, m_pCurl.get())->Get(m_output, m_url);

        default: break;
    }

    return NULL;
}



// -----------------------------------
// wxCurlUploadThreadInputFilter
// -----------------------------------

size_t wxCurlUploadThreadInputFilter::OnSysRead(void *buffer, size_t bufsize)
{
    // VERY IMPORTANT: we need to call TestDestroy() on the associated
    //                 thread to make sure it can be paused/stopped (see wxThread docs)

    if (m_thread->TestDestroy())
    {
        if (m_thread->m_pCurl->IsVerbose())
            wxLogDebug(wxS("[wxCURL] a wxCurlUploadThread has been aborted - ignore following message:"));

        // returning a size != bufsize we tell libcurl to stop the tranfer
        // and thus the wxCurlBase::Perform() call done (indirectly) by wxCurlUploadThread::Entry()
        // will immediately complete and we'll exit the thread.
        // NOTE: we could also call wxThread::Exit() from here... but notifying libcurl
        //       of a "fake error" seems a better solution (allows libcurl to cleanup its internal)
        return (size_t)-1;
    }

    return m_stream->Read(buffer, bufsize).LastRead();
}


// ---------------------
// wxCurlUploadThread
// ---------------------

wxCurlThreadError wxCurlUploadThread::SetInputStream(wxInputStream *in)
{
    wxCHECK_MSG(!IsAlive(), wxCTE_NO_RESOURCE, wxS("Cannot use this function after the transfer has begun"));

    m_input.SetStream(in);

    // the user-provided input stream must be valid
    if (!m_input.IsOk())
        return wxCTE_NO_VALID_STREAM;

    return wxCTE_NO_ERROR;
}

wxCurlThreadError wxCurlUploadThread::Upload()
{
    wxCHECK_MSG(!IsAlive(), wxCTE_NO_RESOURCE, wxS("Cannot use this function after the tranfer has begun"));

    // create & run this thread
    wxCurlThreadError ret;
    if ((ret=Create(wxCURL_THREAD_STACK_SIZE)) != wxCTE_NO_ERROR)
        return ret;
    if ((ret=Run()) != wxCTE_NO_ERROR)
        return ret;

    return wxCTE_NO_ERROR;
}

wxCurlThreadError wxCurlUploadThread::Upload(const wxString &url, wxInputStream *in)
{
    wxCurlThreadError ret;

    if ((ret=SetURL(url)) != wxCTE_NO_ERROR)
        return ret;
    if ((ret=SetInputStream(in)) != wxCTE_NO_ERROR)
        return ret;

    return Upload();
}

void *wxCurlUploadThread::Entry()
{
    if (!m_pCurl || !m_pCurl->IsOk() || !m_input.IsOk())
        return NULL;

    // NOTE: the TestDestroy() function will still be called in this thread
    //       context by the m_output's OnSysWrite function which in turn is
    //       called from libcurl whenever some new data arrives
    if (m_pCurl->IsVerbose())
        wxLogDebug(wxS("wxCurlUploadThread - uploading to %s"), m_url.c_str());
    switch (m_protocol)
    {
        case wxCP_HTTP:
            return (void*)wx_static_cast(wxCurlHTTP*, m_pCurl.get())->Put(m_input, m_url);

        case wxCP_FTP:
            return (void*)wx_static_cast(wxCurlFTP*, m_pCurl.get())->Put(m_input, m_url);
            
        default: break;
    }

    return NULL;
}
