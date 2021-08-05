/////////////////////////////////////////////////////////////////////////////
// Name:        dialog.h
// Purpose:     wxCurlDownloadDialog, wxCurlUploadDialog
// Author:      Francesco Montorsi
// Created:     2007/04/14
// RCS-ID:      $Id: dialog.h 1240 2010-03-10 23:54:25Z frm $
// Copyright:   (c) 2007 Francesco Montorsi
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WXCURL_DIALOG_H_
#define _WXCURL_DIALOG_H_

#include "wx/dialog.h"
#include "wx/curl/thread.h"
#include "wx/curl/panel.h"

// forward declarations
//class WXDLLIMPEXP_CORE wxStaticText;
//class WXDLLIMPEXP_CORE wxTextCtrl;
//class WXDLLIMPEXP_CORE wxGauge;
//class WXDLLIMPEXP_CORE wxStaticBitmap;


//! Possible wxCurlTransferDialog styles.
enum wxCurlTransferDialogStyle
{
    wxCTDS_ELAPSED_TIME = 0x0001,       //!< The dialog shows the elapsed time.
    wxCTDS_ESTIMATED_TIME = 0x0002,     //!< The dialog shows the estimated total time.
    wxCTDS_REMAINING_TIME = 0x0004,     //!< The dialog shows the remaining time.
    wxCTDS_SPEED = 0x0008,              //!< The dialog shows the transfer speed.
    wxCTDS_SIZE = 0x0010,               //!< The dialog shows the size of the resource to download/upload.
    wxCTDS_URL = 0x0020,                //!< The dialog shows the URL involved in the transfer.

    // styles related to the use of wxCurlConnectionSettingsDialog:

    wxCTDS_CONN_SETTINGS_AUTH = 0x0040,  //!< The dialog allows the user to change the authentication settings.
    wxCTDS_CONN_SETTINGS_PORT = 0x0080,  //!< The dialog allows the user to change the port for the transfer.
    wxCTDS_CONN_SETTINGS_PROXY = 0x0100, //!< The dialog allows the user to change the proxy settings.

    wxCTDS_CONN_SETTINGS_ALL = wxCTDS_CONN_SETTINGS_AUTH|wxCTDS_CONN_SETTINGS_PORT|wxCTDS_CONN_SETTINGS_PROXY,

    wxCTDS_SHOW_ALL = wxCTDS_ELAPSED_TIME|wxCTDS_ESTIMATED_TIME|wxCTDS_REMAINING_TIME|
                     wxCTDS_SPEED|wxCTDS_SIZE|wxCTDS_URL|wxCTDS_CONN_SETTINGS_ALL,

    wxCTDS_CAN_ABORT = 0x0200,          //!< The transfer can be aborted by the user.
    wxCTDS_CAN_START = 0x0400,          //!< The transfer won't start automatically. The user needs to start it.
    wxCTDS_CAN_PAUSE = 0x0800,          //!< The transfer can be paused.

    wxCTDS_AUTO_CLOSE = 0x1000,         //!< The dialog auto closes when transfer is complete.

    // by default all available features are enabled:
    wxCTDS_DEFAULT_STYLE = wxCTDS_CAN_START|wxCTDS_CAN_PAUSE|wxCTDS_CAN_ABORT|wxCTDS_SHOW_ALL|wxCTDS_AUTO_CLOSE
};

//! The return flag of .
enum wxCurlDialogReturnFlag
{
    wxCDRF_SUCCESS,        //!< Download successfully completed.
    wxCDRF_USER_ABORTED,   //!< User aborted the download.
    wxCDRF_FAILED          //!< Invalid URL / other networking problem happened.
};


// ----------------------------------------------------------------------------
// wxCurlTransferDialog
// ----------------------------------------------------------------------------

//! The base class for wxCurlDownloadDialog and wxCurlUploadDialog.
class WXDLLIMPEXP_CURL wxCurlTransferDialog : public wxDialog
{
public:
    wxCurlTransferDialog()
    {
        m_bTransferComplete = false;
#ifdef __WXDEBUG__
        m_bVerbose = true;
#else
        m_bVerbose = false;
#endif

        // some of these may remain NULL:
        m_pElapsedTime = m_pEstimatedTime = m_pRemainingTime = NULL;
        m_pSpeed = m_pSize = NULL;
        m_pBitmap = NULL;
        m_pURL = NULL;
        m_pGauge = NULL;
        m_pLastEvent = NULL;
        m_pThread = NULL;
        m_nStyle = 0;
    }

    bool Create(const wxString &url,
                const wxString& title,
                const wxString& message = wxEmptyString,
                const wxString& sizeLabel = _("Transferred:"),
                const wxBitmap& bitmap = wxNullBitmap,
                wxWindow *parent = NULL,
                long style = wxCTDS_DEFAULT_STYLE);

    ~wxCurlTransferDialog()
        {
            wxDELETE(m_pLastEvent);
            wxDELETE(m_pThread);
        }


    //! Shows the dialog as modal. If the wxCTDS_CAN_START flag was not given,
    //! then the transfer starts automatically.
    //! Note that you should use this function instead of wxDialog::ShowModal().
    wxCurlDialogReturnFlag RunModal();

/* not yet tested
    //! Shows the dialog as modeless.
    bool Show(const bool show);
*/

    //! Returns true if the creation of the dialog was successful.
    bool IsOk() const { return m_pThread != NULL && m_pThread->IsOk(); }

    //! Returns the exit code of the dialog (call after #RunModal).
    virtual wxCurlDialogReturnFlag GetReturnCode() const
        { return (wxCurlDialogReturnFlag)wxDialog::GetReturnCode(); }

    //! Sets the internal wxCurlBase object to be verbose.
    void SetVerbose(bool enable)
        { m_bVerbose=enable; }

    //! Returns true if the internal wxCurlBase is verbose (on by default in debug builds).
    bool IsVerbose() const
        { return m_bVerbose; }

    virtual void EndModal(int retCode);

protected:     // internal utils


    wxStaticText *AddSizerRow(wxSizer *sz, const wxString &name);
    void CreateControls(const wxString &url, const wxString &msg,
                        const wxString& sizeLabel, const wxBitmap &bitmap);
    void UpdateLabels(wxCurlProgressBaseEvent *ev);

    // returns true if the error can be ignored
    bool HandleCurlThreadError(wxCurlThreadError err, wxCurlBaseThread *p,
                               const wxString &url = wxEmptyString);

    bool HasFlag(wxCurlTransferDialogStyle flag) const
        { return (m_nStyle & flag) != 0; }


    // change access policy:

    virtual int ShowModal()
        { return wxDialog::ShowModal(); }

    virtual void SetReturnCode(wxCurlDialogReturnFlag ret)
        { wxDialog::SetReturnCode(ret); }

public:     // event handlers

    void OnEndPerform(wxCurlEndPerformEvent &);

    void OnAbort(wxCommandEvent &);
    void OnConnSettings(wxCommandEvent &);
    void OnPauseResume(wxCommandEvent &);
    void OnStart(wxCommandEvent &);

    void OnAbortUpdateUI(wxUpdateUIEvent &);
    void OnConnSettingsUpdateUI(wxUpdateUIEvent &);
    void OnStartUpdateUI(wxUpdateUIEvent &);
    void OnPauseResumeUpdateUI(wxUpdateUIEvent &);

    void OnClose(wxCloseEvent &ev);

protected:

    wxCurlBaseThread *m_pThread;
    wxCurlProgressBaseEvent *m_pLastEvent;
    bool m_bTransferComplete;

    // wxWindow's style member is too small for all our flags and wxWindow/wxDialog ones.
    // So we use our own...
    long m_nStyle;

    // should we be verbose?
    bool m_bVerbose;

protected:      // controls

    wxStaticText* m_pURL;
    wxStaticText* m_pSpeed;
    wxStaticText* m_pSize;
    wxGauge* m_pGauge;
    wxStaticBitmap* m_pBitmap;

    wxStaticText* m_pElapsedTime;
    wxStaticText* m_pRemainingTime;
    wxStaticText* m_pEstimatedTime;

private:
    DECLARE_EVENT_TABLE()
};


// ----------------------------------------------------------------------------
// wxCurlDownloadDialog
// ----------------------------------------------------------------------------

//! An highly-configurable dialog which shows progress about a download.
class WXDLLIMPEXP_CURL wxCurlDownloadDialog : public wxCurlTransferDialog
{
public:
    wxCurlDownloadDialog() { }

    wxCurlDownloadDialog(const wxString &url,
                        wxOutputStream *out,
                        const wxString& title = wxS("Downloading..."),
                        const wxString& message = wxEmptyString,
                        const wxBitmap& bitmap = wxNullBitmap,
                        wxWindow *parent = NULL,
                        long style = wxCTDS_DEFAULT_STYLE)
        { Create(url, out, title, message, bitmap, parent, style); }

    bool Create(const wxString &url,
                wxOutputStream *out,
                const wxString& title = wxS("Downloading..."),
                const wxString& message = wxEmptyString,
                const wxBitmap& bitmap = wxNullBitmap,
                wxWindow *parent = NULL,
                long style = wxCTDS_DEFAULT_STYLE);

    //! Returns the output stream where data has been downloaded.
    //! This function can be used only when the download has been completed.
    wxOutputStream *GetOutputStream() const
        { return wx_static_cast(wxCurlDownloadThread*, m_pThread)->GetOutputStream(); }

public:     // event handlers

    void OnDownload(wxCurlDownloadEvent &);

private:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS(wxCurlDownloadDialog)
};


// ----------------------------------------------------------------------------
// wxCurlUploadDialog
// ----------------------------------------------------------------------------

//! An highly-configurable dialog which shows progress about an upload.
class WXDLLIMPEXP_CURL wxCurlUploadDialog : public wxCurlTransferDialog
{
public:
    wxCurlUploadDialog() { }

    wxCurlUploadDialog(const wxString &url,
                        wxInputStream *in,
                        const wxString& title = wxS("Uploading..."),
                        const wxString& message = wxEmptyString,
                        const wxBitmap& bitmap = wxNullBitmap,
                        wxWindow *parent = NULL,
                        long style = wxCTDS_DEFAULT_STYLE)
        { Create(url, in, title, message, bitmap, parent, style); }

    bool Create(const wxString &url,
                wxInputStream *in,
                const wxString& title = wxS("Uploading..."),
                const wxString& message = wxEmptyString,
                const wxBitmap& bitmap = wxNullBitmap,
                wxWindow *parent = NULL,
                long style = wxCTDS_DEFAULT_STYLE);


public:     // event handlers

    void OnUpload(wxCurlUploadEvent &);

private:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS(wxCurlUploadDialog)
};


// ----------------------------------------------------------------------------
// wxCurlConnectionSettingsDialog
// ----------------------------------------------------------------------------

//! A dialog which simply wraps a wxCurlConnectionSettingsPanel.
//! For a list of supported styles, please see wxCurlConnectionSettingsPanel.
class WXDLLIMPEXP_CURL wxCurlConnectionSettingsDialog : public wxDialog
{
public:
    wxCurlConnectionSettingsDialog() { m_pPanel = NULL; }

    wxCurlConnectionSettingsDialog(const wxString& title,
                                   const wxString& message = wxEmptyString,
                                   wxWindow *parent = NULL,
                                   long style = wxCCSP_DEFAULT_STYLE)
        { Create(title, message, parent, style); }

    bool Create(const wxString& title = wxS("Connection settings..."),
                const wxString& message = wxEmptyString,
                wxWindow *parent = NULL,
                long style = wxCCSP_DEFAULT_STYLE);

public:

    //! Runs this dialog as modal and updates the given wxCurlBase instance if needed
    //! (i.e. if the user hits OK and not Cancel).
    void RunModal(wxCurlBase *pcurl);

protected:

    wxCurlConnectionSettingsPanel *m_pPanel;

    // change access policy:

    virtual int ShowModal()
        { return wxDialog::ShowModal(); }

private:
    DECLARE_DYNAMIC_CLASS(wxCurlConnectionSettingsDialog)
};


#endif // _WXCURL_DIALOG_H_

