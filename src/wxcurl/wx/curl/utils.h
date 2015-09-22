/////////////////////////////////////////////////////////////////////////////
// Name:        utils.h
// Purpose:     wxCurlSizeEvent, wxCurlSizeQueryThread
// Author:      Francesco Montorsi
// Created:     2007/04/14
// RCS-ID:      $Id: utils.h 1237 2010-03-10 21:52:47Z frm $
// Copyright:   (c) 2007 Francesco Montorsi
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DOWNLOAD_H_
#define _WX_DOWNLOAD_H_

#include "wx/curl/thread.h"


// ----------------------------------------------------------------------------
// wxCurlSizeEvent
// ----------------------------------------------------------------------------

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CURL, wxCURL_SIZE_QUERY_EVENT, 66987)
END_DECLARE_EVENT_TYPES()

//! This event gets posted by wxCurlSizeQueryThread. Use the EVT_CURL_SIZE_QUERY(id, func)
//! macro to intercept it.
class WXDLLIMPEXP_CURL wxCurlSizeEvent : public wxEvent
{
protected:
    wxString m_url;
    double m_size;

public:
    wxCurlSizeEvent(int id, const wxString &url, double size)
        : wxEvent(id, wxCURL_SIZE_QUERY_EVENT), m_url(url), m_size(size) {}

    double GetSize() const
        { return m_size; }
    wxString GetURL() const
        { return m_url; }

    wxEvent* Clone() const
        { return new wxCurlSizeEvent(*this); }
};

typedef void (wxEvtHandler::*wxCurlSizeEventFunction)(wxCurlSizeEvent&);

#define wxCurlSizeEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxCurlSizeEventFunction, &func)

#define EVT_CURL_SIZE_QUERY(id, fn) \
    wx__DECLARE_EVT1(wxCURL_SIZE_EVENT, id, wxCurlSizeEventHandler(fn))



// ----------------------------------------------------------------------------
// wxCurlSizeQueryThread
// ----------------------------------------------------------------------------

//! This thread just retrieve the sizes of the given list of files and post them to
//! the registered event handler.
//! This is useful to show the size of one or more files to the user before downloading them.
class WXDLLIMPEXP_CURL wxCurlSizeQueryThread : public wxCurlBaseThread
{
    friend class wxCurlSizeQueryOutput;

protected:

    //! The array with our retrieved sizes.
    wxArrayLong m_urlSize;

    //! The list of the URLs for the files whose size must be queried.
    wxArrayString m_urls;

public:
    wxCurlSizeQueryThread(wxEvtHandler *handler = NULL,
                          int id = wxID_ANY)
        : wxCurlBaseThread(handler, id) { }

    //! Creates the thread for querying the size of given array of urls.
    //! After calling the Run() function, the given event handler will get
    //! a wxCurlSizeEvent for each URL passed in the list.
    wxCurlSizeQueryThread(wxEvtHandler *handler,
                          int id,
                          const wxArrayString& urls)
        : wxCurlBaseThread(handler, id) { m_urls=urls; }

    //! Sets the list of URLs to query the size for.
    void SetURLs(const wxArrayString &urls)
        {
            wxCHECK_RET(!IsAlive(), wxS("Call this function before creating the thread!")); 
            m_urls=urls;
        }

protected:

    //! Caches the file sizes and then exits.
    virtual void *Entry();

    //! Sends the wxEVT_COMMAND_CACHESIZE_COMPLETE event
    //! to the event handler.
    virtual void OnExit();
};


#endif // _WX_DOWNLOAD_H_

