/////////////////////////////////////////////////////////////////////////////
// Name:        panel.cpp
// Purpose:     wxCurlConnectionSettingsPanel
// Author:      Francesco Montorsi
// Created:     2007/05/5
// RCS-ID:      $Id: panel.cpp 1240 2010-03-10 23:54:25Z frm $
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
    #include <wx/intl.h>        // for _() support
    #include <wx/utils.h>       // for wxMilliSleep

    #if wxUSE_GUI
        #include <wx/textctrl.h>
        #include <wx/gauge.h>
        #include <wx/stattext.h>
        #include <wx/sizer.h>
        #include <wx/msgdlg.h>
        #include <wx/settings.h>
        #include <wx/button.h>
        #include <wx/statbmp.h>
    #endif
#endif

#ifdef __WXMSW__
    #include <wx/msw/msvcrt.h>      // useful to catch memory leaks when compiling under MSVC 
#endif

#include <wx/checkbox.h>
#include <wx/filename.h>
#include <wx/statline.h>
#include "wx/curl/panel.h"


// ----------------------------------------------------------------------------
// wxCurlConnectionSettingsPanel
// ----------------------------------------------------------------------------

enum
{
    UseProxy = wxID_HIGHEST+1
};

IMPLEMENT_DYNAMIC_CLASS( wxCurlConnectionSettingsPanel, wxPanel )
BEGIN_EVENT_TABLE( wxCurlConnectionSettingsPanel, wxPanel )
    EVT_CHECKBOX( UseProxy, wxCurlConnectionSettingsPanel::OnUseProxy )
END_EVENT_TABLE()

bool wxCurlConnectionSettingsPanel::Create(wxWindow* parent, wxWindowID id,
                                           const wxString &msg,
                                           const wxPoint& pos, const wxSize& size,
                                           long style, const wxString& name)
{
    if (!wxPanel::Create(parent, id, pos, size, style, name))
        return false;

    // set up our controls
    CreateControls(msg);

    if (GetMinSize().GetWidth() < 250)
        SetMinSize(wxSize(250, -1));

    return true;
}

#define BORDER          5

wxTextCtrl *wxCurlConnectionSettingsPanel::AddSizerRow(wxSizer *sz, const wxString &name, bool grow)
{
    // the static text
    wxStaticText *st = new wxStaticText( this, wxID_STATIC, name, wxDefaultPosition, wxDefaultSize );

    // the value
    wxTextCtrl *ret = new wxTextCtrl( this, wxID_ANY );

    long flags = wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT;
    sz->Add(st, 0, flags, BORDER);
    sz->Add(ret, 1, grow ? flags|wxGROW : flags, BORDER);

    return ret;
}

void wxCurlConnectionSettingsPanel::CreateControls(const wxString &msg)
{
    wxBoxSizer* main = new wxBoxSizer(wxVERTICAL);

    // message row
    if (!msg.IsEmpty())
    {
        wxStaticText *st = new wxStaticText( this, wxID_STATIC, msg );
        main->Add(st, 0, wxLEFT|wxTOP|wxRIGHT|wxBOTTOM|wxGROW, BORDER);
    }

    wxFlexGridSizer *gs = new wxFlexGridSizer(0 /* calc automatically the # of rows */, 2, 0, 0);

    // authentication options
    if (HasFlag(wxCCSP_AUTHENTICATION_OPTIONS))
    {
        m_pUsername = AddSizerRow(gs, _("Username:"));
        m_pPassword = AddSizerRow(gs, _("Password:"));
        gs->AddStretchSpacer(1);
        gs->AddStretchSpacer(1);
    }

    // port options
    if (HasFlag(wxCCSP_PORT_OPTION))
    {
        m_pPort = AddSizerRow(gs, _("Port:"), false);
        gs->AddStretchSpacer(1);
        gs->AddStretchSpacer(1);

        m_pPort->SetValue(wxS("-1"));
    }

    // column 1 contains the text controls:
    gs->AddGrowableCol(1);
    main->Add(gs, 1, wxGROW|wxALL, BORDER);

    // proxy options
    if (HasFlag(wxCCSP_PROXY_OPTIONS))
    {
        wxSizer* proxy = new wxStaticBoxSizer(wxVERTICAL, this, _("Proxy settings"));

        m_pProxyCheckBox = new wxCheckBox(this, UseProxy, _("Use proxy"));
        m_pProxyCheckBox->SetValue(false);
        proxy->Add(m_pProxyCheckBox, 0, wxLEFT|wxTOP, BORDER);

        wxFlexGridSizer *gs = new wxFlexGridSizer(0 /* calc automatically the # of rows */, 2, 0, 0);
        m_pProxyHost = AddSizerRow(gs, _("Proxy host:"));
        m_pProxyUsername = AddSizerRow(gs, _("Proxy username:"));
        m_pProxyPassword = AddSizerRow(gs, _("Proxy password:"));
        m_pProxyPort = AddSizerRow(gs, _("Proxy port:"), false);
        m_pProxyPort->SetValue(wxS("-1"));
        proxy->Add(gs, 0, wxGROW|wxALL, BORDER);

        // column 1 contains the text controls:
        gs->AddGrowableCol(1);

        main->AddSpacer(10);
        main->Add(proxy, 0, wxGROW|wxLEFT|wxRIGHT, BORDER);
#if wxCHECK_VERSION(2,9,0)
        main->AddStretchSpacer(1);
#endif

        // be default disable the proxy controls
        wxCommandEvent fake;
        fake.SetInt(0);
        OnUseProxy(fake);
    }

    this->SetSizerAndFit(main);
    main->SetSizeHints(this);
}

void wxCurlConnectionSettingsPanel::SetCURLOptions(wxCurlBase *p)
{
    wxASSERT(p);

    if (HasFlag(wxCCSP_PORT_OPTION))
    {
        long port = -1;     // -1 tell libCURL to use the default port
        m_pPort->GetValue().ToLong(&port);
        p->SetPort(port);
    }

    if (HasFlag(wxCCSP_AUTHENTICATION_OPTIONS))
    {
        p->SetUsername(m_pUsername->GetValue());
        p->SetPassword(m_pPassword->GetValue());
    }

    if (HasFlag(wxCCSP_PROXY_OPTIONS))
    {
        p->UseProxy(m_pProxyCheckBox->GetValue());

        p->SetProxyHost(m_pProxyHost->GetValue());
        p->SetProxyUsername(m_pProxyUsername->GetValue());
        p->SetProxyPassword(m_pProxyPassword->GetValue());

        long port = -1;     // -1 tell libCURL to use the default port
        m_pProxyPort->GetValue().ToLong(&port);
        p->SetProxyPort(port);
    }
}


// ----------------------------------------------------------------------------
// wxCurlConnectionSettingsPanel - button events
// ----------------------------------------------------------------------------

void wxCurlConnectionSettingsPanel::OnUseProxy(wxCommandEvent &ev)
{
    m_pProxyHost->Enable(ev.IsChecked());
    m_pProxyUsername->Enable(ev.IsChecked());
    m_pProxyPassword->Enable(ev.IsChecked());
    m_pProxyPort->Enable(ev.IsChecked());
}


