///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////
#include <sstream>

#include "AboutFrame.h"
#include "ocpn_plugin.h"

///////////////////////////////////////////////////////////////////////////

AboutFrame::AboutFrame(wxWindow* parent, wxWindowID id, const wxString& title,
                       const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style) {
  this->SetSizeHints(wxSize(500, 300), wxDefaultSize);

  wxBoxSizer* bSizerMain;
  bSizerMain = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizerContentMain;
  bSizerContentMain = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer* bSizerLeftColumn;
  bSizerLeftColumn = new wxBoxSizer(wxVERTICAL);

  m_bitmapLogo = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap,
                                    wxDefaultPosition, wxSize(100, 100), 0);
  m_bitmapLogo->SetMaxSize(wxSize(100, 100));

  bSizerLeftColumn->Add(m_bitmapLogo, 0, wxALL, 10);

  bSizerLeftColumn->Add(0, 0, 1, wxEXPAND, 5);

  m_btnBack = new wxButton(this, wxID_ANY, _("< Back"), wxDefaultPosition,
                           wxDefaultSize, wxBU_EXACTFIT);
  bSizerLeftColumn->Add(m_btnBack, 0, wxALL, 5);

  bSizerContentMain->Add(bSizerLeftColumn, 0, wxEXPAND, 5);

  wxBoxSizer* bSizerContent;
  bSizerContent = new wxBoxSizer(wxVERTICAL);

  m_scrolledWindowAbout = new wxScrolledWindow(
      this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
  m_scrolledWindowAbout->SetScrollRate(5, 5);
  wxBoxSizer* bSizerAbout;
  bSizerAbout = new wxBoxSizer(wxVERTICAL);

  m_staticTextOCPN =
      new wxStaticText(m_scrolledWindowAbout, wxID_ANY, _("OpenCPN"),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_staticTextOCPN->Wrap(-1);
  m_staticTextOCPN->SetFont(wxFont(30, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                                   wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizerAbout->Add(m_staticTextOCPN, 0, wxALL, 5);

  m_staticTextVersion =
      new wxStaticText(m_scrolledWindowAbout, wxID_ANY, _T("5.6.0"),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_staticTextVersion->Wrap(-1);
  m_staticTextVersion->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(),
                                      wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                                      wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizerAbout->Add(m_staticTextVersion, 0, wxALL, 5);

  wxBoxSizer* bSizerAuthors1;
  bSizerAuthors1 = new wxBoxSizer(wxHORIZONTAL);

  m_staticTextCopyYears =
      new wxStaticText(m_scrolledWindowAbout, wxID_ANY, _("(c) 2000-2023"),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_staticTextCopyYears->Wrap(-1);
  bSizerAuthors1->Add(m_staticTextCopyYears, 0, wxALL, 5);

  m_hyperlinkAuthors =
      new wxHyperlinkCtrl(m_scrolledWindowAbout, wxID_ANY,
                          _("The OpenCPN Authors"), wxT("https://opencpn.org"),
                          wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
  m_hyperlinkAuthors->SetFont(
      wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT,
             wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

  bSizerAuthors1->Add(m_hyperlinkAuthors, 0, wxALIGN_CENTER_VERTICAL, 5);

  bSizerAbout->Add(bSizerAuthors1, 0, wxEXPAND, 5);

  m_staticTextDescription = new wxStaticText(
      m_scrolledWindowAbout, wxID_ANY,
      _("OpenCPN is a Free Software project, built by sailors.\nIt is freely "
        "available to download and distribute without charge."),
      wxDefaultPosition, wxDefaultSize, 0);
  m_staticTextDescription->Wrap(380);
  bSizerAbout->Add(m_staticTextDescription, 0, wxALL, 5);

  wxBoxSizer* bSizerParticipate;
  bSizerParticipate = new wxBoxSizer(wxHORIZONTAL);

  m_staticTextHelp =
      new wxStaticText(m_scrolledWindowAbout, wxID_ANY, _("Want to help?"),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_staticTextHelp->Wrap(-1);
  bSizerParticipate->Add(m_staticTextHelp, 0, wxALL | wxTOP, 5);

  m_hyperlinkDonate = new wxHyperlinkCtrl(
      m_scrolledWindowAbout, wxID_ANY, _("Donate"),
      wxT("https://sourceforge.net/donate/index.php?group_id=180842"),
      wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
  m_hyperlinkDonate->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(),
                                    wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                                    wxFONTWEIGHT_NORMAL, false, wxEmptyString));

  bSizerParticipate->Add(m_hyperlinkDonate, 0, wxALIGN_CENTER_VERTICAL, 5);

  m_staticTextOr = new wxStaticText(m_scrolledWindowAbout, wxID_ANY, _("or"),
                                    wxDefaultPosition, wxDefaultSize, 0);
  m_staticTextOr->Wrap(-1);
  bSizerParticipate->Add(m_staticTextOr, 0, wxALL, 5);

  m_hyperlinkGetInvolved =
      new wxHyperlinkCtrl(m_scrolledWindowAbout, wxID_ANY, _("get involved!"),
                          wxT("https://github.com/OpenCPN/OpenCPN"),
                          wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
  m_hyperlinkGetInvolved->SetFont(
      wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT,
             wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

  bSizerParticipate->Add(m_hyperlinkGetInvolved, 0, wxALIGN_CENTER_VERTICAL, 5);

  bSizerAbout->Add(bSizerParticipate, 0, wxEXPAND, 5);

  wxBoxSizer* bSizerLogFile;
  bSizerLogFile = new wxBoxSizer(wxHORIZONTAL);

  m_staticTextLogfile =
      new wxStaticText(m_scrolledWindowAbout, wxID_ANY, _("Logfile:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_staticTextLogfile->Wrap(-1);
  bSizerLogFile->Add(m_staticTextLogfile, 0, wxALL, 5);

  m_hyperlinkLogFile = new wxHyperlinkCtrl(
      m_scrolledWindowAbout, wxID_ANY,
      _T("/Users/nohal/Library/Logs/opencpn.log"),
      wxT("http://www.wxformbuilder.org"), wxDefaultPosition, wxDefaultSize,
      wxHL_CONTEXTMENU | wxHL_ALIGN_LEFT);
  m_hyperlinkLogFile->SetFont(
      wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT,
             wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

  bSizerLogFile->Add(m_hyperlinkLogFile, 0, wxALL, 5);

  bSizerAbout->Add(bSizerLogFile, 1, wxEXPAND | wxTOP, 15);

  wxBoxSizer* bSizerIniFile;
  bSizerIniFile = new wxBoxSizer(wxHORIZONTAL);

  m_staticTextIniFile =
      new wxStaticText(m_scrolledWindowAbout, wxID_ANY, _("Config file:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_staticTextIniFile->Wrap(-1);
  bSizerIniFile->Add(m_staticTextIniFile, 0, wxALL, 5);

  m_hyperlinkIniFile = new wxHyperlinkCtrl(
      m_scrolledWindowAbout, wxID_ANY,
      _T("/Users/nohal/Library/Preferences/opencpn/opencpn.ini"),
      wxT("http://www.wxformbuilder.org"), wxDefaultPosition, wxDefaultSize,
      wxHL_CONTEXTMENU | wxHL_ALIGN_LEFT);
  m_hyperlinkIniFile->SetFont(
      wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT,
             wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));
  m_hyperlinkIniFile->SetToolTip(_("Copy to clipboard"));

  bSizerIniFile->Add(m_hyperlinkIniFile, 0, wxALL, 5);

  bSizerAbout->Add(bSizerIniFile, 1, wxEXPAND, 5);

  auto bApiInfo = new wxBoxSizer(wxHORIZONTAL);
  std::ostringstream api_os;
  api_os << _("Plugin API: ") << API_VERSION_MAJOR * 100 + API_VERSION_MINOR;
  auto API_info =
      new wxStaticText(m_scrolledWindowAbout, wxID_ANY, api_os.str());
  bApiInfo->Add(API_info, 0, wxALL, 5);
  bSizerAbout->Add(bApiInfo, 1, wxEXPAND, 5);

  m_scrolledWindowAbout->SetSizer(bSizerAbout);
  m_scrolledWindowAbout->Layout();
  bSizerAbout->Fit(m_scrolledWindowAbout);
  bSizerContent->Add(m_scrolledWindowAbout, 1, wxEXPAND | wxALL, 5);

  m_htmlWinAuthors = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition,
                                      wxDefaultSize, wxHW_SCROLLBAR_AUTO);
  bSizerContent->Add(m_htmlWinAuthors, 1, wxALL | wxEXPAND, 5);

  m_htmlWinLicense = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition,
                                      wxDefaultSize, wxHW_SCROLLBAR_AUTO);
  bSizerContent->Add(m_htmlWinLicense, 1, wxALL | wxEXPAND, 5);

#if wxUSE_WEBVIEW && defined(HAVE_WEBVIEW)
  m_htmlWinHelp = wxWebView::New(this, wxID_ANY);
#else
  m_htmlWinHelp = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition,
                                   wxDefaultSize, wxHW_SCROLLBAR_AUTO);
#endif
  bSizerContent->Add(m_htmlWinHelp, 1, wxALL | wxEXPAND, 5);

  bSizerContentMain->Add(bSizerContent, 1, wxEXPAND, 5);

  bSizerMain->Add(bSizerContentMain, 1, wxEXPAND, 5);

  wxBoxSizer* bSizerMainLnks;
  bSizerMainLnks = new wxBoxSizer(wxHORIZONTAL);

  m_panelMainLinks = new wxPanel(this, wxID_ANY, wxDefaultPosition,
                                 wxDefaultSize, wxTAB_TRAVERSAL);
  m_panelMainLinks->SetBackgroundColour(wxColour(153, 153, 153));

  wxBoxSizer* bSizerLinksInner;
  bSizerLinksInner = new wxBoxSizer(wxHORIZONTAL);

  bSizerLinksInner->Add(0, 0, 1, wxEXPAND, 5);

  m_hyperlinkWebsite =
      new wxHyperlinkCtrl(m_panelMainLinks, wxID_ANY, _("OpenCPN Website"),
                          wxT("https://opencpn.org"), wxDefaultPosition,
                          wxDefaultSize, wxHL_DEFAULT_STYLE);

  m_hyperlinkWebsite->SetNormalColour(wxColour(255, 255, 255));
  m_hyperlinkWebsite->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(),
                                     wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                                     wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizerLinksInner->Add(m_hyperlinkWebsite, 0, wxALL, 20);

  m_hyperlinkHelp = new wxHyperlinkCtrl(
      m_panelMainLinks, wxID_ANY, _("Help"), wxT("file://index.html"),
      wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);

  m_hyperlinkHelp->SetNormalColour(wxColour(255, 255, 255));
  m_hyperlinkHelp->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(),
                                  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                                  wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizerLinksInner->Add(m_hyperlinkHelp, 0, wxALL, 20);

  m_hyperlinkLicense = new wxHyperlinkCtrl(
      m_panelMainLinks, wxID_ANY, _("License"), wxT("file://license.html"),
      wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);

  m_hyperlinkLicense->SetNormalColour(wxColour(255, 255, 255));
  m_hyperlinkLicense->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(),
                                     wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                                     wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizerLinksInner->Add(m_hyperlinkLicense, 0, wxALL, 20);

  bSizerLinksInner->Add(0, 0, 1, wxEXPAND, 5);

  m_panelMainLinks->SetSizer(bSizerLinksInner);
  m_panelMainLinks->Layout();
  bSizerLinksInner->Fit(m_panelMainLinks);
  bSizerMainLnks->Add(m_panelMainLinks, 1, wxEXPAND, 0);

  bSizerMain->Add(bSizerMainLnks, 0, wxEXPAND, 0);

  this->SetSizer(bSizerMain);
  this->Layout();

  this->Centre(wxBOTH);

  // Connect Events
  this->Connect(wxEVT_ACTIVATE,
                wxActivateEventHandler(AboutFrame::AboutFrameOnActivate));
  this->Connect(wxEVT_CLOSE_WINDOW,
                wxCloseEventHandler(AboutFrame::AboutFrameOnClose));
  m_btnBack->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(AboutFrame::m_btnBackOnButtonClick),
                     NULL, this);
  m_hyperlinkAuthors->Connect(
      wxEVT_COMMAND_HYPERLINK,
      wxHyperlinkEventHandler(AboutFrame::OnLinkAuthors), NULL, this);
  m_hyperlinkDonate->Connect(wxEVT_COMMAND_HYPERLINK,
                             wxHyperlinkEventHandler(AboutFrame::OnLinkDonate),
                             NULL, this);
  m_hyperlinkGetInvolved->Connect(
      wxEVT_COMMAND_HYPERLINK,
      wxHyperlinkEventHandler(AboutFrame::OnLinkGetInvolved), NULL, this);
  m_hyperlinkLogFile->Connect(
      wxEVT_COMMAND_HYPERLINK,
      wxHyperlinkEventHandler(AboutFrame::OnLinkLogfile), NULL, this);
  m_hyperlinkIniFile->Connect(
      wxEVT_COMMAND_HYPERLINK,
      wxHyperlinkEventHandler(AboutFrame::OnLinkConfigfile), NULL, this);
  m_htmlWinHelp->Connect(
      wxEVT_COMMAND_HTML_LINK_CLICKED,
      wxHtmlLinkEventHandler(AboutFrame::m_htmlWinHelpOnHtmlLinkClicked), NULL,
      this);
  m_hyperlinkHelp->Connect(wxEVT_COMMAND_HYPERLINK,
                           wxHyperlinkEventHandler(AboutFrame::OnLinkHelp),
                           NULL, this);
  m_hyperlinkLicense->Connect(
      wxEVT_COMMAND_HYPERLINK,
      wxHyperlinkEventHandler(AboutFrame::OnLinkLicense), NULL, this);
}

AboutFrame::~AboutFrame() {
  // Disconnect Events
  this->Disconnect(wxEVT_ACTIVATE,
                   wxActivateEventHandler(AboutFrame::AboutFrameOnActivate));
  this->Disconnect(wxEVT_CLOSE_WINDOW,
                   wxCloseEventHandler(AboutFrame::AboutFrameOnClose));
  m_btnBack->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(AboutFrame::m_btnBackOnButtonClick), NULL, this);
  m_hyperlinkAuthors->Disconnect(
      wxEVT_COMMAND_HYPERLINK,
      wxHyperlinkEventHandler(AboutFrame::OnLinkAuthors), NULL, this);
  m_hyperlinkDonate->Disconnect(
      wxEVT_COMMAND_HYPERLINK,
      wxHyperlinkEventHandler(AboutFrame::OnLinkDonate), NULL, this);
  m_hyperlinkGetInvolved->Disconnect(
      wxEVT_COMMAND_HYPERLINK,
      wxHyperlinkEventHandler(AboutFrame::OnLinkGetInvolved), NULL, this);
  m_hyperlinkLogFile->Disconnect(
      wxEVT_COMMAND_HYPERLINK,
      wxHyperlinkEventHandler(AboutFrame::OnLinkLogfile), NULL, this);
  m_hyperlinkIniFile->Disconnect(
      wxEVT_COMMAND_HYPERLINK,
      wxHyperlinkEventHandler(AboutFrame::OnLinkConfigfile), NULL, this);
  m_htmlWinHelp->Disconnect(
      wxEVT_COMMAND_HTML_LINK_CLICKED,
      wxHtmlLinkEventHandler(AboutFrame::m_htmlWinHelpOnHtmlLinkClicked), NULL,
      this);
  m_hyperlinkHelp->Disconnect(wxEVT_COMMAND_HYPERLINK,
                              wxHyperlinkEventHandler(AboutFrame::OnLinkHelp),
                              NULL, this);
  m_hyperlinkLicense->Disconnect(
      wxEVT_COMMAND_HYPERLINK,
      wxHyperlinkEventHandler(AboutFrame::OnLinkLicense), NULL, this);
}
