/////////////////////////////////////////////////////////////////////////////
// Name:        dialog.cpp
// Purpose:     wxCurlDownloadDialog, wxCurlUploadDialog
// Author:      Francesco Montorsi
// Created:     2007/04/14
// RCS-ID:      $Id: dialog.cpp 1240 2010-03-10 23:54:25Z frm $
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

#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/statline.h>

//class WXDLLIMPEXP_CORE wxTextCtrl;
class WXDLLIMPEXP_CORE wxCheckBox;
#include "wx/curl/dialog.h"


// ----------------------------------------------------------------------------
// wxCurlTransferDialog
// ----------------------------------------------------------------------------

enum
{
    AbortButtonId = wxID_HIGHEST+1,
    PauseResumeButtonId,
    StartButtonId,
    ConnSettingsButtonId,

    ThreadId
};

BEGIN_EVENT_TABLE( wxCurlTransferDialog, wxDialog )

    // network events
    EVT_CURL_END_PERFORM( ThreadId, wxCurlTransferDialog::OnEndPerform )

    // user events
    EVT_BUTTON( AbortButtonId, wxCurlTransferDialog::OnAbort )
    EVT_BUTTON( ConnSettingsButtonId, wxCurlTransferDialog::OnConnSettings )
    EVT_BUTTON( PauseResumeButtonId, wxCurlTransferDialog::OnPauseResume )
    EVT_BUTTON( StartButtonId, wxCurlTransferDialog::OnStart )

    // update UI
    EVT_UPDATE_UI( AbortButtonId, wxCurlTransferDialog::OnAbortUpdateUI )
    EVT_UPDATE_UI( ConnSettingsButtonId, wxCurlTransferDialog::OnConnSettingsUpdateUI )
    EVT_UPDATE_UI( PauseResumeButtonId, wxCurlTransferDialog::OnPauseResumeUpdateUI )
    EVT_UPDATE_UI( StartButtonId, wxCurlTransferDialog::OnStartUpdateUI )

    // misc
    EVT_CLOSE( wxCurlTransferDialog::OnClose )

END_EVENT_TABLE()

bool wxCurlTransferDialog::Create(const wxString &url, const wxString& title, const wxString& message,
                              const wxString &sizeLabel, const wxBitmap& bitmap, wxWindow *parent, long style)
{
    if (!wxDialog::Create(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
                          (wxDEFAULT_DIALOG_STYLE |wxRESIZE_BORDER)))
        return false;

    // save&check our style
    m_nStyle = style;
    wxASSERT_MSG(HasFlag(wxCTDS_AUTO_CLOSE) || HasFlag(wxCTDS_CAN_ABORT),
             wxS("If both these flags are missing the user will be unable to close the dialog window!"));

    // do we need to use wxCurlConnectionSettingsDialog?
    wxASSERT_MSG(!(HasFlag(wxCTDS_CONN_SETTINGS_AUTH) ||
                   HasFlag(wxCTDS_CONN_SETTINGS_PORT) ||
                   HasFlag(wxCTDS_CONN_SETTINGS_PROXY)) || HasFlag(wxCTDS_CAN_START),
                 wxS("the connection settings may only be changed before the transfer starts; if wxCTDS_CAN_START ")
                 wxS("is missing the user will be unable to use the connection settings button!"));

    // set up our controls
    CreateControls(url, message, sizeLabel, bitmap);

    return true;
}

wxCurlDialogReturnFlag wxCurlTransferDialog::RunModal()
{
    m_pThread->GetCurlSession()->SetVerbose(m_bVerbose);

    if (!HasFlag(wxCTDS_CAN_START))
    {
        wxCommandEvent fake;
        OnStart(fake);        // start immediately
    }

    CenterOnScreen();

    return (wxCurlDialogReturnFlag)wxDialog::ShowModal();
}

#define OUTER_BORDER    12
#define BORDER          5
#define MINWIDTH        300

wxStaticText *wxCurlTransferDialog::AddSizerRow(wxSizer *sz, const wxString &name)
{
    // the static text
    wxStaticText *st = new wxStaticText( this, wxID_STATIC, name, wxDefaultPosition, wxDefaultSize );
    st->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxS("")));

    // the value
    wxStaticText *ret = new wxStaticText( this, wxID_STATIC, _("Not available"),
                                          wxDefaultPosition, wxDefaultSize,
                                          wxALIGN_LEFT|wxST_NO_AUTORESIZE );

    wxBoxSizer* time = new wxBoxSizer(wxHORIZONTAL);
    time->Add(st, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5);
    time->Add(ret, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5);

    sz->Add(time, 0, wxGROW|wxLEFT|wxRIGHT, OUTER_BORDER);

    return ret;
}

void wxCurlTransferDialog::CreateControls(const wxString &url, const wxString &msg, 
                                      const wxString &sizeLabel, const wxBitmap &bitmap)
{
    wxBoxSizer* main = new wxBoxSizer(wxVERTICAL);

    // message row
    if (!msg.IsEmpty())
    {
        wxStaticText *st = new wxStaticText( this, wxID_STATIC, msg );
        st->SetMinSize(wxSize(MINWIDTH, -1));
        main->Add(st, 0, wxLEFT|wxTOP|wxRIGHT|wxBOTTOM|wxGROW, OUTER_BORDER);
    }

    // URL row
    if (HasFlag(wxCTDS_URL))
    {
        wxBoxSizer* downloading = new wxBoxSizer(wxHORIZONTAL);

        wxStaticText *st = new wxStaticText( this, wxID_STATIC, _("URL:") );
        wxFont boldFont(st->GetFont());
        boldFont.SetWeight(wxFONTWEIGHT_BOLD);
        st->SetFont(boldFont);
        downloading->Add(st, 0, wxRIGHT|wxTOP|wxALIGN_CENTER_VERTICAL, BORDER);

        m_pURL = new wxStaticText( this, wxID_STATIC, url, wxDefaultPosition,
                                   wxSize(MINWIDTH, -1));//, wxST_ELLIPSIZE_MIDDLE);
        downloading->Add(m_pURL, 1, wxALIGN_CENTER_VERTICAL|wxTOP, BORDER);

        main->Add(downloading, 0, wxGROW|wxLEFT|wxRIGHT, OUTER_BORDER);
        main->AddSpacer(5);
    }


    wxSizer *leftcolumn = new wxBoxSizer(wxVERTICAL);

    // speed & size row
    if (HasFlag(wxCTDS_SPEED))
        m_pSpeed = AddSizerRow(leftcolumn, _("Speed:"));
    if (HasFlag(wxCTDS_SIZE))
        m_pSize = AddSizerRow(leftcolumn, sizeLabel);

    // a spacer
    leftcolumn->AddSpacer(5);

    // the time rows
    if (HasFlag(wxCTDS_ELAPSED_TIME))
        m_pElapsedTime = AddSizerRow(leftcolumn, _("Elapsed time:"));
    if (HasFlag(wxCTDS_ESTIMATED_TIME))
        m_pEstimatedTime = AddSizerRow(leftcolumn, _("Estimated total time:"));
    if (HasFlag(wxCTDS_REMAINING_TIME))
        m_pRemainingTime = AddSizerRow(leftcolumn, _("Estimated remaining time:"));

    if (bitmap.IsOk())
    {
        wxSizer *rightcolumn = new wxBoxSizer(wxVERTICAL);
        rightcolumn->Add( new wxStaticBitmap(this, wxID_ANY, bitmap),
                          0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, BORDER);

        wxSizer *both = new wxBoxSizer(wxHORIZONTAL);
        both->Add(leftcolumn);
        both->Add(rightcolumn, 1, wxGROW);
        main->Add(both, 0, wxGROW);
    }
    else
        main->Add(leftcolumn);


    // the gauge
    // VERY IMPORTANT: we set as range 101 so that when, because of some approximation,
    //                 the update event handler will SetValue(100), while the transfer
    //                 is not yet complete, the gauge will not appear completely filled.
    m_pGauge = new wxGauge( this, wxID_ANY, 101 );

    main->AddSpacer(5);
    main->Add(m_pGauge, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP, OUTER_BORDER);

    // do we need to use wxCurlConnectionSettingsDialog?
    bool needsConnSettings = HasFlag(wxCTDS_CONN_SETTINGS_AUTH) ||
                             HasFlag(wxCTDS_CONN_SETTINGS_PORT) ||
                             HasFlag(wxCTDS_CONN_SETTINGS_PROXY);

    // an horizontal line of buttons
    if (HasFlag(wxCTDS_CAN_ABORT) || HasFlag(wxCTDS_CAN_PAUSE) || HasFlag(wxCTDS_CAN_START) ||
        needsConnSettings)
    {
        main->AddStretchSpacer(1);
        main->AddSpacer(BORDER*2);
        main->Add( new wxStaticLine(this), 0, wxGROW|wxLEFT|wxRIGHT, OUTER_BORDER);

        // the button row
        wxBoxSizer *btn = new wxBoxSizer(wxHORIZONTAL);

        if (HasFlag(wxCTDS_CAN_ABORT))
            btn->Add(new wxButton( this, AbortButtonId, _("Abort") ), 0, wxRIGHT, BORDER);
        if (needsConnSettings)
            btn->Add(new wxButton( this, ConnSettingsButtonId, _("Settings") ), 0);

        btn->AddStretchSpacer(1);

        if (HasFlag(wxCTDS_CAN_PAUSE))
            btn->Add(new wxButton( this, PauseResumeButtonId, _("Pause") ), 0);
        if (HasFlag(wxCTDS_CAN_START))
            btn->Add(new wxButton( this, StartButtonId, _("Start") ), 0, wxLEFT, BORDER);
            
        btn->SetMinSize( wxSize( -1, wxButton::GetDefaultSize().GetHeight() + 2 * OUTER_BORDER ) );

        main->Add(btn, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP|wxBOTTOM, OUTER_BORDER);
    }

    this->SetSizerAndFit(main);
    main->SetSizeHints(this);
}

void wxCurlTransferDialog::EndModal(int retCode)
{
    wxDialog::EndModal(retCode);

    // before dying we must be sure our thread has completed, too
    // otherwise it will try to send events to a non-existent handler
    // NB: this must be done *after* calling wxDialog::EndModal
    //     so that while we wait we are hidden
    HandleCurlThreadError(m_pThread->Wait(), m_pThread);
}

void wxCurlTransferDialog::UpdateLabels(wxCurlProgressBaseEvent *ev)
{
    // ignore this update if the thread has been paused
    // since this event was generated...
    if (m_pThread->IsPaused())
        return;

    // NOTE: we need instead to process this update if the thread has completed
    //       but that's for another reason: see m_pLastEvent stuff

    double fraction = ev->GetPercent();
    if (fraction != 0)
    {
        m_pGauge->SetValue((int)fraction);
    }
    else
    {
        // we don't know how much we progressed...
        m_pGauge->Pulse();
    }

    //  Correct an occasional ASSERT for invalid wxTimeSpan.   dsr
    if(ev->GetElapsedTime().IsShorterThan( wxTimeSpan::Minutes(120))){
        if (m_pElapsedTime)
            m_pElapsedTime->SetLabel(ev->GetElapsedTime().Format());
        if (m_pRemainingTime)
            m_pRemainingTime->SetLabel(ev->GetEstimatedRemainingTime().Format());
        if (m_pEstimatedTime)
            m_pEstimatedTime->SetLabel(ev->GetEstimatedTime().Format());
    }
    
    if (m_pSize)
    {
        wxString currsize(ev->GetHumanReadableTransferredBytes().c_str(), wxConvUTF8);
        wxString totalsize(ev->GetHumanReadableTotalBytes().c_str(), wxConvUTF8);
        m_pSize->SetLabel(
            wxString::Format(wxS("%s / %s  (%0.1f%%)"),
                                currsize.c_str(), totalsize.c_str(), ev->GetPercent()));
    }

    if (m_pSpeed)
    {
        wxString s(ev->GetHumanReadableSpeed().c_str(), wxConvUTF8);
        m_pSpeed->SetLabel(s);
    }
}

bool wxCurlTransferDialog::HandleCurlThreadError(wxCurlThreadError err, wxCurlBaseThread *p, const wxString &url)
{
    switch (err)
    {
        case wxCTE_NO_ERROR:
            return true;        // ignore this

        case wxCTE_NO_RESOURCE:
            wxLogError(wxS("Insufficient resources for correct execution of the program."));
            break;

        case wxCTE_ALREADY_RUNNING:
            wxFAIL;      // should never happen!
            break;

        case wxCTE_INVALID_PROTOCOL:
            wxLogError(wxS("The URL '%s' uses an unsupported protocol."), url.c_str());
            break;

        case wxCTE_NO_VALID_STREAM:
            wxFAIL;     // should never happen - the user streams should always be valid!
            break;

        case wxCTE_ABORTED:
            return true;        // ignore this

        case wxCTE_CURL_ERROR:
            {
                wxString err = wxS("unknown");
                if (p->GetCurlSession())
                    err = wxString(p->GetCurlSession()->GetErrorString().c_str(), wxConvUTF8);
                wxLogError(wxS("Network error: %s"), err.c_str());
            }
            break;
    }

    // stop the thread
    if (p->IsAlive()) p->Abort();

    // this is an unrecoverable error:
    return false;
}


// ----------------------------------------------------------------------------
// wxCurlTransferDialog - button events
// ----------------------------------------------------------------------------

void wxCurlTransferDialog::OnClose(wxCloseEvent &WXUNUSED(ev))
{
    wxCommandEvent fake;
    OnAbort(fake);
}

void wxCurlTransferDialog::OnAbort(wxCommandEvent &WXUNUSED(ev))
{
    // NOTE: the wxCTDS_ABORT flag may be absent if the user wxASSERT(HasFlag(wxCTDS_CAN_ABORT));

    if (m_pThread->IsAlive())
    {
        m_pThread->Abort();
        EndModal(wxCDRF_USER_ABORTED);
    }
    else
    {
        wxASSERT(HasFlag(wxCTDS_CAN_START) || !HasFlag(wxCTDS_AUTO_CLOSE));
            // thread is not alive: means the user has not
            // clicked on Start button yet or the download is complete
            // and the dialog does not auto close

        // if the transfer has been completed, then the OnEndPerform event handler
        // has already set the return code to a valid value:
        EndModal(m_bTransferComplete ? GetReturnCode() : wxCDRF_USER_ABORTED);
    }
}

void wxCurlTransferDialog::OnAbortUpdateUI(wxUpdateUIEvent &ev)
{
    ev.SetText(m_pThread->IsAlive() ? _("Abort") : _("Close"));
}

void wxCurlTransferDialog::OnPauseResume(wxCommandEvent &WXUNUSED(ev))
{
    wxASSERT(HasFlag(wxCTDS_CAN_PAUSE));

    if (m_pThread->IsRunning())
    {
        if (HandleCurlThreadError(m_pThread->Pause(), m_pThread))
        {
            FindWindowById(PauseResumeButtonId)->SetLabel(_("Resume"));

            if (m_pSpeed)
                m_pSpeed->SetLabel(_("0 (transfer paused)"));
        }
    }
    else
    {
        if (HandleCurlThreadError(m_pThread->Resume(), m_pThread))
        {
            FindWindowById(PauseResumeButtonId)->SetLabel(_("Pause"));
        }
    }
}

void wxCurlTransferDialog::OnPauseResumeUpdateUI(wxUpdateUIEvent &ev)
{
    ev.Enable(m_pThread->IsAlive());
}

void wxCurlTransferDialog::OnStart(wxCommandEvent &WXUNUSED(ev))
{
    //wxASSERT(HasFlag(wxCTDS_CAN_START));      // OnStart() must be usable also from RunModal() when !HasFlag(wxCTDS_CAN_START)

    wxCurlThreadError err = m_pThread->StartTransfer();
    if (err != wxCTE_NO_ERROR)
    {
        HandleCurlThreadError(err, m_pThread);     // shows a message to the user
        m_pThread->Abort();
        EndModal(wxCDRF_FAILED);
    }
}

void wxCurlTransferDialog::OnStartUpdateUI(wxUpdateUIEvent &ev)
{
    ev.Enable(!m_pThread->IsAlive() && !m_bTransferComplete);
}

void wxCurlTransferDialog::OnConnSettings(wxCommandEvent &WXUNUSED(ev))
{
    long style = 0;

    // convert our style into wxCurlConnectionSettingsDialog style:
    if (HasFlag(wxCTDS_CONN_SETTINGS_AUTH))
        style |= wxCCSP_AUTHENTICATION_OPTIONS;
    if (HasFlag(wxCTDS_CONN_SETTINGS_PORT))
        style |= wxCCSP_PORT_OPTION;
    if (HasFlag(wxCTDS_CONN_SETTINGS_PROXY))
        style |= wxCCSP_PROXY_OPTIONS;

    wxCurlConnectionSettingsDialog
        dlg(_("Connection settings"), _("Connection settings used for the transfer:"),
            this, style);

    dlg.RunModal(m_pThread->GetCurlSession());
}

void wxCurlTransferDialog::OnConnSettingsUpdateUI(wxUpdateUIEvent &ev)
{
    ev.Enable(!m_pThread->IsAlive() && !m_bTransferComplete);
}


// ----------------------------------------------------------------------------
// wxCurlTransferDialog - network events
// ----------------------------------------------------------------------------

void wxCurlTransferDialog::OnEndPerform(wxCurlEndPerformEvent &ev)
{
    wxLogDebug(wxS("wxCurlTransferDialog::OnEndPerform"));

    // in case the very last transfer update event was skipped because
    // of our anti-flickering label update policy, force the update with
    // that event now (otherwise the dialog may remain open showing data
    // related not to the end of the download!)
    if (m_pLastEvent)
        UpdateLabels(m_pLastEvent);
    //else: we may have transferred a file of zero size and thus have never received any progress event...

    // now we're sure the transfer has completed: make the gauge
    // completely "filled"
    m_pGauge->SetValue(101);

    // this flag is used for updating labels etc:
    m_bTransferComplete = true;

    // transfer has completed...
    wxCurlDialogReturnFlag retCode = ev.IsSuccessful() ? wxCDRF_SUCCESS : wxCDRF_FAILED;
#if 0 // below is bugged because m_pThread->GetCurlSession() is NULL at this point...
    if (retCode == wxCDRF_FAILED)
    {
        // show the user a message...
        wxLogError(wxS("The transfer failed: %s (%s)"),
                   m_pThread->GetCurlSession()->GetErrorString().c_str(),
                   m_pThread->GetCurlSession()->GetDetailedErrorString().c_str());
    }
#endif

    // do we need to close this window?
    if (HasFlag(wxCTDS_AUTO_CLOSE))
        EndModal(retCode);
    else
    {
        SetReturnCode(retCode);     // will exit later in OnAbort()

        if (m_pSpeed)
            m_pSpeed->SetLabel(_("0 (transfer completed)"));
    }
}




// ----------------------------------------------------------------------------
// wxCurlDownloadDialog
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( wxCurlDownloadDialog, wxCurlTransferDialog )
BEGIN_EVENT_TABLE( wxCurlDownloadDialog, wxCurlTransferDialog )
    EVT_CURL_DOWNLOAD( ThreadId, wxCurlDownloadDialog::OnDownload )
END_EVENT_TABLE()

bool wxCurlDownloadDialog::Create(const wxString &url, wxOutputStream *out,
                                  const wxString& title, const wxString& message,
                                  const wxBitmap& bitmap,
                                  wxWindow *parent, long style)
{
    if (!wxCurlTransferDialog::Create(url, title, message, _("Downloaded:"), bitmap, parent, style))
        return false;

    // register as the thread's event handler
    wxCurlDownloadThread *thread = new wxCurlDownloadThread(this, ThreadId);

    m_pThread = thread;     // downcast our pointer for usage by wxCurlTransferDialog

    if (!HandleCurlThreadError(thread->SetURL(url), thread, url))
        return false;
    if (!HandleCurlThreadError(thread->SetOutputStream(out), thread))
        return false;

    return true;
}

void wxCurlDownloadDialog::OnDownload(wxCurlDownloadEvent &ev)
{
    static wxDateTime lastLabelUpdate(0, 0, 0, 0);      // zero is to force always at least an update
    if ((wxDateTime::Now() - lastLabelUpdate).GetMilliseconds() > 200)   // avoid flickering
    {
        UpdateLabels(&ev);

        lastLabelUpdate = wxDateTime::Now();
    }

    // see OnEndPerform for more info.
    if (m_pLastEvent)
        delete m_pLastEvent;
    m_pLastEvent = wx_static_cast(wxCurlProgressBaseEvent*, ev.Clone());
}



// ----------------------------------------------------------------------------
// wxCurlUploadDialog
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( wxCurlUploadDialog, wxCurlTransferDialog )
BEGIN_EVENT_TABLE( wxCurlUploadDialog, wxCurlTransferDialog )
    EVT_CURL_UPLOAD( ThreadId, wxCurlUploadDialog::OnUpload )
END_EVENT_TABLE()

bool wxCurlUploadDialog::Create(const wxString &url, wxInputStream *in,
                                  const wxString& title, const wxString& message,
                                  const wxBitmap& bitmap,
                                  wxWindow *parent, long style)
{
    if (!wxCurlTransferDialog::Create(url, title, message, _("Uploaded:"), bitmap, parent, style))
        return false;

    // register as the thread's event handler
    wxCurlUploadThread *thread = new wxCurlUploadThread(this, ThreadId);

    m_pThread = thread;     // downcast our pointer for usage by wxCurlTransferDialog

    if (!HandleCurlThreadError(thread->SetURL(url), thread, url))
        return false;
    if (!HandleCurlThreadError(thread->SetInputStream(in), thread))
        return false;

    return true;
}

void wxCurlUploadDialog::OnUpload(wxCurlUploadEvent &ev)
{
    static wxDateTime lastLabelUpdate(0, 0, 0, 0);      // zero is to force always at least an update
    if ((wxDateTime::Now() - lastLabelUpdate).GetMilliseconds() > 200)   // avoid flickering
    {
        UpdateLabels(&ev);

        lastLabelUpdate = wxDateTime::Now();
    }

    // see OnEndPerform for more info.
    if (m_pLastEvent)
        delete m_pLastEvent;
    m_pLastEvent = wx_static_cast(wxCurlProgressBaseEvent*, ev.Clone());
}



// ----------------------------------------------------------------------------
// wxCurlConnectionSettingsDialog
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS( wxCurlConnectionSettingsDialog, wxDialog )

bool wxCurlConnectionSettingsDialog::Create(const wxString& title,
                                            const wxString& message,
                                            wxWindow *parent,
                                            long style)
{
    if (!wxDialog::Create(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, 
                          wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER))
        return false;

    m_pPanel = new wxCurlConnectionSettingsPanel(this, wxID_ANY, message, wxDefaultPosition,
                                                 wxDefaultSize, style);

    wxSizer *main = new wxBoxSizer(wxVERTICAL);
    wxSizer *buttons = CreateSeparatedButtonSizer(wxOK|wxCANCEL);

    main->Add(m_pPanel, 1, wxGROW|wxALL, 10);
    main->Add(buttons, 0, wxGROW|wxALL, 10);

    SetSizerAndFit(main);

    return true;
}

void wxCurlConnectionSettingsDialog::RunModal(wxCurlBase *pCURL)
{
    if (ShowModal() == wxID_OK)
        m_pPanel->SetCURLOptions(pCURL);
}

