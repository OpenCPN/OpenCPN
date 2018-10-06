/////////////////////////////////////////////////////////////////////////////
// Name:        utils.cpp
// Purpose:     wxCurlSizeQueryThread
// Author:      Francesco Montorsi
// Created:     2005/04/23
// RCS-ID:      $Id: utils.cpp 1240 2010-03-10 23:54:25Z frm $
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

#include "wx/curl/utils.h"
#include "wx/curl/http.h"
#include "wx/curl/ftp.h"


DEFINE_EVENT_TYPE(wxCURL_SIZE_QUERY_EVENT)



// -----------------------------------
// wxCurlSizeQueryOutput
// -----------------------------------

extern "C"
{
    int wxcurl_size_query_progress_func(void* ptr, double rDlTotal, double WXUNUSED(rDlNow),
                                        double WXUNUSED(rUlTotal), double WXUNUSED(rUlNow))
    {
        unsigned long *p = (unsigned long *)ptr;
        if (p) *p = (unsigned long)rDlTotal;

        // a non-zero value means to abort the transfer
        // and that's what we want as we don't want to download the
        // entire file but just get the rDlTotal info from the server
        return 1;
    }
}

// ---------------------
// wxCurlSizeQueryThread
// ---------------------

void *wxCurlSizeQueryThread::Entry()
{
    wxLogDebug(wxS("wxSizeCacherThread::Entry - caching file sizes"));

    wxMemoryOutputStream os;
    bool allok = true;
    wxCurlHTTP http;
    wxCurlFTP ftp;

    m_urlSize.Clear();
    for (size_t i=0; i<m_urls.GetCount() && !TestDestroy(); i++)
    {
        unsigned long sz;

        wxCurlProtocol prot = GetProtocolFromURL(m_urls[i]);
        switch (prot)
        {
            case wxCP_HTTP:
            {
                http.OverrideProgressCallback(wxcurl_size_query_progress_func, &sz);
                allok &= http.Get(os, m_urls[i]);
            }
            break;

            case wxCP_FTP:
            {
                ftp.OverrideProgressCallback(wxcurl_size_query_progress_func, &sz);
                allok &= ftp.Get(os, m_urls[i]);
            }
            break;

            default:
                sz = (unsigned long)-1;
                wxFAIL;
        }

        m_urlSize.Add(sz);

        // send the event
        wxCurlSizeEvent ev(GetId(), m_urls[i], sz);
        wxPostEvent(GetEvtHandler(), ev);
    }

    wxLogDebug(wxS("wxSizeCacherThread::Entry - caching of file sizes completed"));
    return (void *)allok;
}

void wxCurlSizeQueryThread::OnExit()
{/*
    // we'll use wxPostEvent to post this event since this is the
    // only thread-safe way to post events !
    wxCommandEvent updatevent(wxEVT_COMMAND_CACHESIZE_COMPLETE);

    // the event handler must delete the wxArrayLong which we pass to it in the event !
    updatevent.SetClientData(m_urlSizes);
    if (m_pHandler)
        wxPostEvent(m_pHandler, updatevent);*/
}

