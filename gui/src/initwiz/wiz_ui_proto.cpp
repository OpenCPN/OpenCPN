///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-5-gc2f65a65-dirty)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wiz_ui_proto.h"

///////////////////////////////////////////////////////////////////////////

FirstUseWiz::FirstUseWiz(wxWindow* parent, wxWindowID id, const wxString& title,
                         const wxBitmap& bitmap, const wxPoint& pos,
                         long style) {
  this->Create(parent, id, title, bitmap, pos, style);

  this->SetSizeHints(wxSize(700, 600), wxDefaultSize);

  wxWizardPageSimple* m_wpLangUnits = new wxWizardPageSimple(this);
  m_pages.Add(m_wpLangUnits);

  wxBoxSizer* bSizerLangUnitsTop;
  bSizerLangUnitsTop = new wxBoxSizer(wxVERTICAL);

  m_swLangUnits =
      new wxScrolledWindow(m_wpLangUnits, wxID_ANY, wxDefaultPosition,
                           wxDefaultSize, wxHSCROLL | wxVSCROLL);
  m_swLangUnits->SetScrollRate(5, 5);
  wxBoxSizer* bSizerLangUnits;
  bSizerLangUnits = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer* sbSizerUnits;
  sbSizerUnits = new wxStaticBoxSizer(
      new wxStaticBox(m_swLangUnits, wxID_ANY, _("Units and formats")),
      wxVERTICAL);

  wxFlexGridSizer* fgSizerUnits;
  fgSizerUnits = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizerUnits->SetFlexibleDirection(wxBOTH);
  fgSizerUnits->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_stDepth = new wxStaticText(sbSizerUnits->GetStaticBox(), wxID_ANY,
                               _("Depth"), wxDefaultPosition, wxDefaultSize, 0);
  m_stDepth->Wrap(-1);
  fgSizerUnits->Add(m_stDepth, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxString m_cDepthChoices[] = {_("Feet"), _("Meters"), _("Fathoms")};
  int m_cDepthNChoices = sizeof(m_cDepthChoices) / sizeof(wxString);
  m_cDepth =
      new wxChoice(sbSizerUnits->GetStaticBox(), wxID_ANY, wxDefaultPosition,
                   wxDefaultSize, m_cDepthNChoices, m_cDepthChoices, 0);
  m_cDepth->SetSelection(0);
  fgSizerUnits->Add(m_cDepth, 0, wxALL, 5);

  m_stDistance =
      new wxStaticText(sbSizerUnits->GetStaticBox(), wxID_ANY, _("Distance"),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_stDistance->Wrap(-1);
  fgSizerUnits->Add(m_stDistance, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxString m_cDistanceChoices[] = {_("Nautical miles"), _("Statute miles"),
                                   _("Kilometers"), _("Meters")};
  int m_cDistanceNChoices = sizeof(m_cDistanceChoices) / sizeof(wxString);
  m_cDistance =
      new wxChoice(sbSizerUnits->GetStaticBox(), wxID_ANY, wxDefaultPosition,
                   wxDefaultSize, m_cDistanceNChoices, m_cDistanceChoices, 0);
  m_cDistance->SetSelection(0);
  fgSizerUnits->Add(m_cDistance, 0, wxALL, 5);

  m_stSpeed = new wxStaticText(sbSizerUnits->GetStaticBox(), wxID_ANY,
                               _("Speed"), wxDefaultPosition, wxDefaultSize, 0);
  m_stSpeed->Wrap(-1);
  fgSizerUnits->Add(m_stSpeed, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxString m_cSpeedChoices[] = {_("Knots"), _("mph"), _("km/h"), _("m/s")};
  int m_cSpeedNChoices = sizeof(m_cSpeedChoices) / sizeof(wxString);
  m_cSpeed =
      new wxChoice(sbSizerUnits->GetStaticBox(), wxID_ANY, wxDefaultPosition,
                   wxDefaultSize, m_cSpeedNChoices, m_cSpeedChoices, 0);
  m_cSpeed->SetSelection(0);
  fgSizerUnits->Add(m_cSpeed, 0, wxALL, 5);

  m_stWind =
      new wxStaticText(sbSizerUnits->GetStaticBox(), wxID_ANY, _("Wind speed"),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_stWind->Wrap(-1);
  fgSizerUnits->Add(m_stWind, 0, wxALL, 5);

  wxString m_cWindChoices[] = {_("Knots"), _("mph"), _("km/h"), _("m/s")};
  int m_cWindNChoices = sizeof(m_cWindChoices) / sizeof(wxString);
  m_cWind =
      new wxChoice(sbSizerUnits->GetStaticBox(), wxID_ANY, wxDefaultPosition,
                   wxDefaultSize, m_cWindNChoices, m_cWindChoices, 0);
  m_cWind->SetSelection(0);
  fgSizerUnits->Add(m_cWind, 0, wxALL, 5);

  m_stPosition =
      new wxStaticText(sbSizerUnits->GetStaticBox(), wxID_ANY, _("Position"),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_stPosition->Wrap(-1);
  fgSizerUnits->Add(m_stPosition, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxString m_cPositionChoices[] = {
      _("Degrees, decimal minutes (DD MM.MM)"), _("Decimal degrees (DD.DDDDD)"),
      _("Degrees, minutes, seconds (DD MM SS)"), wxEmptyString};
  int m_cPositionNChoices = sizeof(m_cPositionChoices) / sizeof(wxString);
  m_cPosition =
      new wxChoice(sbSizerUnits->GetStaticBox(), wxID_ANY, wxDefaultPosition,
                   wxDefaultSize, m_cPositionNChoices, m_cPositionChoices, 0);
  m_cPosition->SetSelection(0);
  fgSizerUnits->Add(m_cPosition, 0, wxALL, 5);

  fgSizerUnits->Add(0, 0, 1, wxEXPAND, 5);

  m_cbShowTrue = new wxCheckBox(sbSizerUnits->GetStaticBox(), wxID_ANY,
                                _("Show true headings and bearings"),
                                wxDefaultPosition, wxDefaultSize, 0);
  m_cbShowTrue->SetValue(true);
  fgSizerUnits->Add(m_cbShowTrue, 0, wxALL, 5);

  fgSizerUnits->Add(0, 0, 1, wxEXPAND, 5);

  m_cbShowMagnetic = new wxCheckBox(sbSizerUnits->GetStaticBox(), wxID_ANY,
                                    _("Show magnetic headings and bearings"),
                                    wxDefaultPosition, wxDefaultSize, 0);
  fgSizerUnits->Add(m_cbShowMagnetic, 0, wxALL, 5);

  sbSizerUnits->Add(fgSizerUnits, 1, wxEXPAND, 5);

  bSizerLangUnits->Add(sbSizerUnits, 1, wxEXPAND, 5);

  bSizerLangUnits->Add(0, 20, 0, wxEXPAND, 5);

  m_rtLangUnitInfo = new wxRichTextCtrl(
      m_swLangUnits, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
      0 | wxVSCROLL | wxHSCROLL | wxNO_BORDER | wxWANTS_CHARS);
  bSizerLangUnits->Add(m_rtLangUnitInfo, 1, wxEXPAND | wxALL, 5);

  m_swLangUnits->SetSizer(bSizerLangUnits);
  m_swLangUnits->Layout();
  bSizerLangUnits->Fit(m_swLangUnits);
  bSizerLangUnitsTop->Add(m_swLangUnits, 1, wxEXPAND | wxALL, 5);

  m_wpLangUnits->SetSizer(bSizerLangUnitsTop);
  m_wpLangUnits->Layout();
  wxWizardPageSimple* m_wpConnections = new wxWizardPageSimple(this);
  m_pages.Add(m_wpConnections);

  wxBoxSizer* bSizerConnectionsTop;
  bSizerConnectionsTop = new wxBoxSizer(wxVERTICAL);

  m_swConnections =
      new wxScrolledWindow(m_wpConnections, wxID_ANY, wxDefaultPosition,
                           wxDefaultSize, wxHSCROLL | wxVSCROLL);
  m_swConnections->SetScrollRate(5, 5);
  wxBoxSizer* bSizerConnections;
  bSizerConnections = new wxBoxSizer(wxVERTICAL);

  m_stSources = new wxStaticText(m_swConnections, wxID_ANY,
                                 _("Detected navigation data sources:"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_stSources->Wrap(-1);
  bSizerConnections->Add(m_stSources, 0, wxALL, 5);

  wxArrayString m_clSourcesChoices;
  m_clSources = new wxCheckListBox(m_swConnections, wxID_ANY, wxDefaultPosition,
                                   wxDefaultSize, m_clSourcesChoices, 0);
  bSizerConnections->Add(m_clSources, 1, wxALL | wxEXPAND, 5);

  m_btnRescanSources = new wxButton(m_swConnections, wxID_ANY, _("Rescan..."),
                                    wxDefaultPosition, wxDefaultSize, 0);
  bSizerConnections->Add(m_btnRescanSources, 0, wxALL, 5);

  bSizerConnections->Add(0, 20, 0, wxEXPAND, 5);

  m_rtConnectionInfo = new wxRichTextCtrl(
      m_swConnections, wxID_ANY, wxEmptyString, wxDefaultPosition,
      wxDefaultSize, 0 | wxVSCROLL | wxHSCROLL | wxNO_BORDER | wxWANTS_CHARS);
  bSizerConnections->Add(m_rtConnectionInfo, 2, wxEXPAND | wxALL, 5);

  m_swConnections->SetSizer(bSizerConnections);
  m_swConnections->Layout();
  bSizerConnections->Fit(m_swConnections);
  bSizerConnectionsTop->Add(m_swConnections, 1, wxEXPAND | wxALL, 5);

  m_wpConnections->SetSizer(bSizerConnectionsTop);
  m_wpConnections->Layout();
  bSizerConnectionsTop->Fit(m_wpConnections);
  wxWizardPageSimple* m_wpCharts = new wxWizardPageSimple(this);
  m_pages.Add(m_wpCharts);

  wxBoxSizer* bSizerChartsTop;
  bSizerChartsTop = new wxBoxSizer(wxVERTICAL);

  m_swCharts = new wxScrolledWindow(m_wpCharts, wxID_ANY, wxDefaultPosition,
                                    wxDefaultSize, wxHSCROLL | wxVSCROLL);
  m_swCharts->SetScrollRate(5, 5);
  wxBoxSizer* bSizerCharts;
  bSizerCharts = new wxBoxSizer(wxVERTICAL);

  m_stAddCharts =
      new wxStaticText(m_swCharts, wxID_ANY, _("Add existing charts"),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_stAddCharts->Wrap(-1);
  bSizerCharts->Add(m_stAddCharts, 0, wxALL, 5);

  m_lbChartsDirs = new wxListBox(m_swCharts, wxID_ANY, wxDefaultPosition,
                                 wxDefaultSize, 0, NULL, 0);
  bSizerCharts->Add(m_lbChartsDirs, 0, wxALL | wxEXPAND, 5);

  m_btnAddChartDir =
      new wxButton(m_swCharts, wxID_ANY, _("Add chart directory..."),
                   wxDefaultPosition, wxDefaultSize, 0);
  bSizerCharts->Add(m_btnAddChartDir, 0, wxALL, 5);

  bSizerCharts->Add(0, 20, 0, wxEXPAND, 5);

  m_rtChartDirInfo = new wxRichTextCtrl(
      m_swCharts, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
      0 | wxVSCROLL | wxHSCROLL | wxNO_BORDER | wxWANTS_CHARS);
  bSizerCharts->Add(m_rtChartDirInfo, 2, wxEXPAND | wxALL, 5);

  m_swCharts->SetSizer(bSizerCharts);
  m_swCharts->Layout();
  bSizerCharts->Fit(m_swCharts);
  bSizerChartsTop->Add(m_swCharts, 1, wxEXPAND | wxALL, 5);

  m_wpCharts->SetSizer(bSizerChartsTop);
  m_wpCharts->Layout();
  wxWizardPageSimple* m_wpFinish = new wxWizardPageSimple(this);
  m_pages.Add(m_wpFinish);

  wxBoxSizer* bSizerFinish;
  bSizerFinish = new wxBoxSizer(wxVERTICAL);

  m_htmlWinFinish = new wxHtmlWindow(m_wpFinish, wxID_ANY, wxDefaultPosition,
                                     wxDefaultSize, wxHW_SCROLLBAR_AUTO);

  // Try to detect smaller displays, and adjust wizard size accordingly
  //  Looking for small devices in landscape mode.
  wxSize displaySize = wxGetDisplaySize();
  if ((displaySize.y < 500) && (displaySize.x > displaySize.y)) {
    m_htmlWinFinish->SetMinSize(
        wxSize(displaySize.x * 8 / 10, displaySize.y * 65 / 100));
  } else
    m_htmlWinFinish->SetMinSize(wxSize(800, 600));

  bSizerFinish->Add(m_htmlWinFinish, 1, wxALL | wxEXPAND, 5);

  m_wpFinish->SetSizer(bSizerFinish);
  m_wpFinish->Layout();

  this->Centre(wxBOTH);

  for (unsigned int i = 1; i < m_pages.GetCount(); i++) {
    m_pages.Item(i)->SetPrev(m_pages.Item(i - 1));
    m_pages.Item(i - 1)->SetNext(m_pages.Item(i));
  }

  // Connect Events
  this->Connect(wxEVT_INIT_DIALOG,
                wxInitDialogEventHandler(FirstUseWiz::OnInitDialog));
  this->Connect(wxID_ANY, wxEVT_WIZARD_CANCEL,
                wxWizardEventHandler(FirstUseWiz::OnWizardCancel));
  this->Connect(wxID_ANY, wxEVT_WIZARD_FINISHED,
                wxWizardEventHandler(FirstUseWiz::OnWizardFinished));
  this->Connect(wxID_ANY, wxEVT_WIZARD_HELP,
                wxWizardEventHandler(FirstUseWiz::OnWizardHelp));
  this->Connect(wxID_ANY, wxEVT_WIZARD_PAGE_CHANGED,
                wxWizardEventHandler(FirstUseWiz::OnWizardPageChanged));
  this->Connect(wxID_ANY, wxEVT_WIZARD_PAGE_CHANGING,
                wxWizardEventHandler(FirstUseWiz::OnWizardPageChanging));
  this->Connect(wxID_ANY, wxEVT_WIZARD_PAGE_SHOWN,
                wxWizardEventHandler(FirstUseWiz::OnWizardPageShown));
  m_btnRescanSources->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(FirstUseWiz::m_btnRescanSourcesOnButtonClick), NULL,
      this);
  m_btnAddChartDir->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(FirstUseWiz::m_btnAddChartDirOnButtonClick), NULL,
      this);
}

FirstUseWiz::~FirstUseWiz() {
  // Disconnect Events
  this->Disconnect(wxEVT_INIT_DIALOG,
                   wxInitDialogEventHandler(FirstUseWiz::OnInitDialog));
  this->Disconnect(wxID_ANY, wxEVT_WIZARD_CANCEL,
                   wxWizardEventHandler(FirstUseWiz::OnWizardCancel));
  this->Disconnect(wxID_ANY, wxEVT_WIZARD_FINISHED,
                   wxWizardEventHandler(FirstUseWiz::OnWizardFinished));
  this->Disconnect(wxID_ANY, wxEVT_WIZARD_HELP,
                   wxWizardEventHandler(FirstUseWiz::OnWizardHelp));
  this->Disconnect(wxID_ANY, wxEVT_WIZARD_PAGE_CHANGED,
                   wxWizardEventHandler(FirstUseWiz::OnWizardPageChanged));
  this->Disconnect(wxID_ANY, wxEVT_WIZARD_PAGE_CHANGING,
                   wxWizardEventHandler(FirstUseWiz::OnWizardPageChanging));
  this->Disconnect(wxID_ANY, wxEVT_WIZARD_PAGE_SHOWN,
                   wxWizardEventHandler(FirstUseWiz::OnWizardPageShown));
  m_btnRescanSources->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(FirstUseWiz::m_btnRescanSourcesOnButtonClick), NULL,
      this);
  m_btnAddChartDir->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(FirstUseWiz::m_btnAddChartDirOnButtonClick), NULL,
      this);

  m_pages.Clear();
}
