///////////////////////////////////////////////////////////////////////////
// wxFormBuilder-generated layout; implementations split for maintainability.
///////////////////////////////////////////////////////////////////////////

#include "chartdldr_pi.h"
#include "chartdldrgui.h"
#include "chartdldr_prefs.h"
#include "chartdldr_source_catalog.h"
#include "chartdldr_temp_download.h"
#include "manual.h"
#include "../../../libs/manual/include/manual.h"
#include <wx/msgdlg.h>
#include <wx/scrolwin.h>
#include <wx/spinctrl.h>
#include <wx/textwrapper.h>

#ifdef __OCPN__ANDROID__
#include <QtWidgets/QScroller>
#include "qdebug.h"
#endif

ChartDldrPrefsDlg::ChartDldrPrefsDlg(wxWindow* parent, wxWindowID id,
                                     const wxString& title, const wxPoint& pos,
                                     const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
  wxFont* pFont = OCPNGetFont(_("Dialog"));
  if (pFont) SetFont(*pFont);

  this->SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* itemBoxSizerMainPanel = new wxBoxSizer(wxVERTICAL);
  SetSizer(itemBoxSizerMainPanel);

  wxPanel* prefsPanel = new wxPanel(this, wxID_ANY);
  itemBoxSizerMainPanel->Add(prefsPanel, 0, wxEXPAND | wxALL, 0);

  m_sdbSizerBtns = new wxStdDialogButtonSizer();
  m_bHelp = new wxButton(this, wxID_HELP);
  m_sdbSizerBtns->Add(m_bHelp);
  m_sdbSizerBtnsOK = new wxButton(this, wxID_OK);
  m_sdbSizerBtns->AddButton(m_sdbSizerBtnsOK);
  m_sdbSizerBtnsCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
  m_sdbSizerBtns->AddButton(m_sdbSizerBtnsCancel);
  m_sdbSizerBtns->Realize();

  itemBoxSizerMainPanel->Add(m_sdbSizerBtns, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer* bSizerPrefsMain = new wxBoxSizer(wxVERTICAL);
  prefsPanel->SetSizer(bSizerPrefsMain);

  wxStaticBoxSizer* sbSizerPaths;
  sbSizerPaths = new wxStaticBoxSizer(
      new wxStaticBox(prefsPanel, wxID_ANY, _("Default Path to Charts")),
      wxVERTICAL);

  //      m_dpDefaultDir = new wxDirPickerCtrl( this, wxID_ANY, wxEmptyString,
  //      _("Select a root folder for your charts"), wxDefaultPosition,
  //      wxDefaultSize, wxDIRP_USE_TEXTCTRL ); sbSizerPaths->Add(
  //      m_dpDefaultDir, 0, wxALL|wxEXPAND, 5 );

  m_tcDefaultDir = new wxTextCtrl(prefsPanel, wxID_ANY, _T(""),
                                  wxDefaultPosition, wxDefaultSize, wxHSCROLL);
  sbSizerPaths->Add(m_tcDefaultDir, 0, wxALL | wxEXPAND, 4);

  m_buttonChartDirectory =
      new wxButton(prefsPanel, wxID_ANY, _("Select a folder"));
  sbSizerPaths->Add(m_buttonChartDirectory, 0, wxALIGN_RIGHT | wxALL, 4);

  bSizerPrefsMain->Add(sbSizerPaths, 0, wxALL | wxEXPAND, 4);

  wxStaticBoxSizer* sbSizerBehavior;
  sbSizerBehavior = new wxStaticBoxSizer(
      new wxStaticBox(prefsPanel, wxID_ANY, _("Preferences")), wxVERTICAL);

  m_stPreselect = new wxStaticText(
      prefsPanel, wxID_ANY, _("After catalog update select for download"),
      wxDefaultPosition, wxDefaultSize, 0);
  m_stPreselect->Wrap(-1);
  sbSizerBehavior->Add(m_stPreselect, 0, wxALL, 4);

  m_cbSelectUpdated =
      new wxCheckBox(prefsPanel, wxID_ANY, _("All updated charts"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbSelectUpdated->SetValue(true);
  sbSizerBehavior->Add(m_cbSelectUpdated, 0, wxALL, 4);

  m_cbSelectNew = new wxCheckBox(prefsPanel, wxID_ANY, _("All new charts"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  sbSizerBehavior->Add(m_cbSelectNew, 0, wxALL, 4);

  m_staticline1 = new wxStaticLine(prefsPanel, wxID_ANY, wxDefaultPosition,
                                   wxDefaultSize, wxLI_HORIZONTAL);
  sbSizerBehavior->Add(m_staticline1, 0, wxEXPAND | wxALL, 4);

  m_cbBulkUpdate = new wxCheckBox(
      prefsPanel, wxID_ANY,
      _("Allow bulk update of all configured chart sources and charts"),
      wxDefaultPosition, wxDefaultSize, 0);
  sbSizerBehavior->Add(m_cbBulkUpdate, 0, wxALL, 4);

  m_buttonDownloadMasterCatalog =
      new wxButton(prefsPanel, wxID_ANY, _("Update chart source catalog"),
                   wxDefaultPosition, wxDefaultSize, 0);

  sbSizerBehavior->Add(m_buttonDownloadMasterCatalog, 0, wxALL, 4);

  {
    const ChartDldrScheduledPrefsWidgets scheduled =
        ChartDldrCreateScheduledUpdateWidgets(prefsPanel);
    m_sbScheduledUpdate = scheduled.sizer;
    m_cbScheduledEnable = scheduled.enable;
    m_tcScheduledTime = scheduled.time;
    m_stScheduledTimePreview = scheduled.time_preview;
    m_stScheduledLastRun = scheduled.last_run;
    m_btnRunScheduledNow = scheduled.run_now;
    sbSizerBehavior->Add(m_sbScheduledUpdate, 0, wxEXPAND | wxALL, 4);
  }

  bSizerPrefsMain->Add(sbSizerBehavior, 0, wxALL | wxEXPAND, 4);

  Layout();
  Fit();

  wxSize dialogSize = GetBestSize();
  dialogSize.x = wxMax(dialogSize.x, 60 * GetCharWidth());
  const wxSize canvas_size = GetOCPNCanvasWindow()->GetSize();
  dialogSize.x = wxMin(dialogSize.x, canvas_size.x);
  dialogSize.y = wxMin(dialogSize.y, canvas_size.y);
  SetMaxSize(canvas_size);
  SetMinSize(dialogSize);
  SetClientSize(dialogSize);
  CentreOnScreen();

  // Connect Events
  m_sdbSizerBtnsCancel->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlg::OnCancelClick), NULL, this);
  m_buttonChartDirectory->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlg::OnDirSelClick), NULL, this);
  m_buttonDownloadMasterCatalog->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlg::OnDownloadMasterCatalog), NULL,
      this);
  m_bHelp->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [&](wxCommandEvent) {
    wxString datadir = GetPluginDataDir("manual_pi");
    Manual manual(this, datadir.ToStdString());
    manual.Launch("Chartdldr");
  });
}

void ChartDldrPrefsDlg::OnDownloadMasterCatalog(wxCommandEvent& event) {
  const wxString url =
      "https://raw.githubusercontent.com/OpenCPN/OpenCPN/master/plugins/"
      "chartdldr_pi/data/chart_sources.xml";

  wxFileName fn;
  fn.SetPath(*GetpPrivateApplicationDataLocation());
  fn.SetFullName(_T("chartdldr_pi-chart_sources.xml"));

  // Download beside the live catalog (same filesystem so publish is an atomic
  // rename), validate the source-catalog XML, then promote. This keeps a good
  // on-disk catalog intact when the transfer is truncated or the server hands
  // back an HTTP 200 error page instead of the catalog.
  const ChartDldrTempDownloadPaths paths =
      ChartDldrTempDownloadPathsFor(fn.GetFullPath());

  _OCPN_DLStatus ret = OCPN_downloadFile(
      url, paths.download_target, _("Downloading chart sources"),
      _("Downloading chart sources"), wxNullBitmap, this,
      OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_ESTIMATED_TIME |
          OCPN_DLDS_REMAINING_TIME | OCPN_DLDS_SPEED | OCPN_DLDS_SIZE |
          OCPN_DLDS_URL | OCPN_DLDS_CAN_PAUSE | OCPN_DLDS_CAN_ABORT |
          OCPN_DLDS_AUTO_CLOSE,
      15);

  const bool aborted = ret == OCPN_DL_ABORTED || ret == OCPN_DL_USER_TIMEOUT;
  ret = ChartDldrCompleteTempDownloadPaths(
      paths, ret == OCPN_DL_NO_ERROR, aborted,
      ChartDldrSourceCatalogXmlIsPublishable);

  switch (ret) {
    case OCPN_DL_NO_ERROR:
      break;
    case OCPN_DL_FAILED: {
      OCPNMessageBox_PlugIn(
          this,
          wxString::Format(_("Failed to download a valid chart source "
                             "catalog from: %s \nVerify there is a working "
                             "Internet connection."),
                           url.c_str()),
          _("Chart downloader"), wxOK | wxICON_ERROR);
      break;
    }
    case OCPN_DL_USER_TIMEOUT:
    case OCPN_DL_ABORTED:
    case OCPN_DL_UNKNOWN:
    case OCPN_DL_STARTED:
      break;

    default:
      wxASSERT(false);  // This should never happen because we handle all
                        // possible cases of ret
  }
}

void ChartDldrPrefsDlg::OnDirSelClick(wxCommandEvent& event) {
  wxString dir_spec;
  int response = PlatformDirSelectorDialog(this, &dir_spec,
                                           _("Choose Chart File Directory"),
                                           m_tcDefaultDir->GetValue());

  if (response == wxID_OK) {
    m_tcDefaultDir->SetValue(dir_spec);
  }
}

void ChartDldrPrefsDlg::OnCancelClick(wxCommandEvent& event) {
  event.Skip();
  EndModal(wxID_CANCEL);
}

void ChartDldrPrefsDlg::OnOkClick(wxCommandEvent& event) { event.Skip(); }

ChartDldrPrefsDlg::~ChartDldrPrefsDlg() {
  // Disconnect Events
  m_sdbSizerBtnsCancel->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlg::OnCancelClick), NULL, this);
  m_buttonChartDirectory->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlg::OnDirSelClick), NULL, this);
  m_buttonDownloadMasterCatalog->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlg::OnDownloadMasterCatalog), NULL,
      this);
}
