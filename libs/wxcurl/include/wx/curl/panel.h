/////////////////////////////////////////////////////////////////////////////
// Name:        panel.h
// Purpose:     wxCurlConnectionSettingsPanel
// Author:      Francesco Montorsi
// Created:     2007/05/5
// RCS-ID:      $Id: panel.h 1237 2010-03-10 21:52:47Z frm $
// Copyright:   (c) 2007 Francesco Montorsi
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WXCURL_PANEL_H_
#define _WXCURL_PANEL_H_

#include "wx/panel.h"
#include "wx/curl/base.h"

// forward declarations
//class WXDLLIMPEXP_CORE wxStaticText;
//class WXDLLIMPEXP_CORE wxTextCtrl;
//class WXDLLIMPEXP_CORE wxCheckBox;
//class WXDLLIMPEXP_CORE wxStaticBitmap;

// ----------------------------------------------------------------------------
// wxCurlConnectionSettingsPanel
// ----------------------------------------------------------------------------

//! Possible wxCurlConnectionSettingsPanel styles.
enum
{
    wxCCSP_PROXY_OPTIONS = 0x0040,               //!< Shows the proxy options.
    wxCCSP_AUTHENTICATION_OPTIONS = 0x0080,      //!< Shows username&password fields.
    wxCCSP_PORT_OPTION = 0x0100,                 //!< Shows the port option.

    wxCCSP_DEFAULT_STYLE = wxCCSP_PROXY_OPTIONS|wxCCSP_AUTHENTICATION_OPTIONS|wxCCSP_PORT_OPTION
};

//! A configurable panel which shows connection settings which can be
//! modified by the user (whose changes will be reflected on a libCURL handle,
//! i.e. on the wxCurlBase class passed to #SetCURLOptions).
class WXDLLIMPEXP_CURL wxCurlConnectionSettingsPanel : public wxPanel
{
public:
    wxCurlConnectionSettingsPanel()
    {
        m_pUsername = NULL;
        m_pPassword = NULL;
        m_pPort = NULL;

        m_pProxyCheckBox = NULL;
        m_pProxyHost = NULL;
        m_pProxyUsername = NULL;
        m_pProxyPassword = NULL;
        m_pProxyPort = NULL;
    }

    wxCurlConnectionSettingsPanel(wxWindow* parent, wxWindowID id = wxID_ANY,
                                  const wxString &msg = wxEmptyString,
                                  const wxPoint& pos = wxDefaultPosition,
                                  const wxSize& size = wxDefaultSize,
                                  long style = wxCCSP_DEFAULT_STYLE,
                                  const wxString& name = wxS("connsettingspanel"))
        { Create(parent, id, msg, pos, size, style, name); }

    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString &msg = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                long style = wxCCSP_DEFAULT_STYLE, const wxString& name = wxS("connsettingspanel"));


    //! Sets all user-defined options of this panel into the given curl object.
    void SetCURLOptions(wxCurlBase *);


public:     // event handlers

    void OnUseProxy(wxCommandEvent &ev);

protected:      // utils

    wxTextCtrl *AddSizerRow(wxSizer *sz, const wxString &name, bool grow = true);
    void CreateControls(const wxString &msg);

protected:      // controls

    wxTextCtrl* m_pUsername;
    wxTextCtrl* m_pPassword;
    wxTextCtrl* m_pPort;

    wxCheckBox* m_pProxyCheckBox;
    wxTextCtrl* m_pProxyHost;
    wxTextCtrl* m_pProxyUsername;
    wxTextCtrl* m_pProxyPassword;
    wxTextCtrl* m_pProxyPort;

private:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS(wxCurlConnectionSettingsPanel)
};


#endif // _WXCURL_PANEL_H_

