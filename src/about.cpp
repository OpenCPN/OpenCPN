/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  About Dialog
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010-2023 by David S. Register                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 *
 *
 */

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/textfile.h>
#include <wx/ffile.h>
#include <wx/clipbrd.h>
#include <wx/html/htmlwin.h>
#include <wx/tokenzr.h>
#include <config.h>

#include "about.h"
#include "gui_lib.h"
#include "chcanv.h"
#include "styles.h"
#include "OCPNPlatform.h"
#include "FontMgr.h"
#include "navutil.h"
#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif
#include "ocpn_frame.h"

extern OCPNPlatform* g_Platform;
extern MyFrame* gFrame;
extern ocpnStyle::StyleManager* g_StyleManager;
extern about* g_pAboutDlgLegacy;
extern bool g_bresponsive;

wxString OpenCPNVersion =
    wxString::Format(wxT("\n      Version %s"), VERSION_FULL);

#define xID_OK 10009

// clang-format off

const wxString AboutText =
    wxT("<br>OpenCPN<br>")
    wxT("(c) 2000-2023  The OpenCPN Authors<br><br>");

const wxString OpenCPNInfo =
    wxT("<br><br>")
    wxT("OpenCPN is a Free Software project, built by sailors. ")
    wxT("It is freely available to download and distribute ")
    wxT("without charge at opencpn.org.<br><br>")
    wxT("If you use OpenCPN, please consider contributing ")
    wxT("or donating funds to the project.<br><br>")
    wxT("For more information, visit http://opencpn.org<br><br>");

const wxString OpenCPNInfoAlt =
    wxT("<br><br>")
    wxT("OpenCPN is a Free Software project, built by sailors.")
    wxT("The complete source code and many other resources ")
    wxT("are freely available for your download and use, ")
    wxT("subject to applicable License agreements.")
    wxT("<br><br>")
    wxT("For more information, visit http://opencpn.org<br><br>");


const wxString AuthorText =
    wxT("   David S Register\n")
    wxT("      OpenCPN Lead Developer\n\n")
    wxT("    Pavel Kalian\n")
    wxT("      Packaging and PlugIn development\n\n")
    wxT("    Håkan Svensson\n")
    wxT("      Comms and Plugin integration\n\n")
    wxT("    Rick Gleason\n")
    wxT("      Plugin adaptation and maintenance\n\n")
    wxT("    Sean D'Epagnier\n")
    wxT("      OpenGL Architecture\n\n")
    wxT("    J.P. Joubert\n")
    wxT("      GRIB PlugIn enhancements\n\n")
    wxT("    Thomas Höckne\n")
    wxT("      Documentation and Wiki support\n\n")
    wxT("    Didier Gautheron\n")
    wxT("      App debugging and optimization\n\n")
    wxT("    Wiets Wilken\n")
    wxT("      Extended vector Icon implementation\n\n")
    wxT("    Gene Seybold\n")
    wxT("      Extended vector Icon design\n\n")
    wxT("    Caesar Schinas\n")
    wxT("      User Interface and OS X improvements\n\n")
    wxT("    Jesper Weissglas\n")
    wxT("      Vector Chart Rendering\n\n")
    wxT("    Jean-Eudes Onfray\n")
    wxT("      Dashboard and Dialog enhancements\n\n")
    wxT("    Kathleen Boswell\n")
    wxT("      Icon design\n\n")
    wxT("    Flavius Bindea\n")
    wxT("      CM93 Offset and AIS enhancements\n\n")
    wxT("    Gunther Pilz\n")
    wxT("      Windows Installer enhancements\n\n")
    wxT("    Alan Bleasby\n")
    wxT("      Garmin jeeps module\n\n")
    wxT("    Piotr Carlson\n")
    wxT("      General usability enhancements\n\n")
    wxT("    Anders Lund\n")
    wxT("      RouteManagerDialog\n\n")
    wxT("    Gordon Mau\n")
    wxT("      OpenCPN Documentation\n\n")
    wxT("    Tim Francis\n")
    wxT("      OpenCPN Documentation\n\n")
    wxT("    Mark A Sikes\n")
    wxT("      OpenCPN CoDeveloper\n\n")
    wxT("    Thomas Haller\n")
    wxT("      GPX Import/Export Implementation\n\n")
    wxT("    Will Kamp\n")
    wxT("      Toolbar Icon design\n\n")
    wxT("    Richard Smith\n")
    wxT("      OpenCPN CoDeveloper, MacOSX\n\n")
    wxT("    David Herring\n")
    wxT("      OpenCPN CoDeveloper, MacOSX\n\n")
    wxT("    Philip Lange\n")
    wxT("      OpenCPN Documentation\n\n")
    wxT("    Ron Kuris\n")
    wxT("      wxWidgets Support\n\n")
    wxT("    Julian Smart, Robert Roebling et al\n")
    wxT("      wxWidgets Authors\n\n")
    wxT("    Sylvain Duclos\n")
    wxT("      S52 Presentation Library code\n\n")
    wxT("    Manish P. Pagey\n")
    wxT("      Serial Port Library\n\n")
    wxT("    David Flater\n")
    wxT("      XTIDE tide and current code\n\n")
    wxT("    Frank Warmerdam\n")
    wxT("      GDAL Class Library\n\n")
    wxT("    Mike Higgins\n")
    wxT("      BSB Chart Format Detail\n\n")
    wxT("    Samuel R. Blackburn\n")
    wxT("      NMEA0183 Class Library\n\n")
    wxT("    Atul Narkhede\n")
    wxT("      Polygon Graphics utilities\n\n")
    wxT("    Jan C. Depner\n")
    wxT("      WVS Chart Library\n\n")
    wxT("    Stuart Cunningham, et al\n")
    wxT("      BSB Chart Georeferencing Algorithms\n\n")
    wxT("    John F. Waers\n")
    wxT("      UTM Conversion Algorithms\n\n")
    wxT("    Carsten Tschach\n")
    wxT("      UTM Conversion Algorithms\n\n")
    wxT("    Ed Williams\n")
    wxT("      Great Circle Formulary\n\n")
    wxT("    Philippe Bekaert\n")
    wxT("      CIE->RGB Color Conversion Matrix\n\n")
    wxT("    Robert Lipe\n")
    wxT("      Garmin USB GPS Interface\n");

// clang-format on

IMPLEMENT_DYNAMIC_CLASS(about, wxDialog)

BEGIN_EVENT_TABLE(about, wxDialog)
EVT_BUTTON(xID_OK, about::OnXidOkClick)
EVT_BUTTON(ID_DONATE, about::OnDonateClick)
EVT_BUTTON(ID_COPYINI, about::OnCopyClick)
EVT_BUTTON(ID_COPYLOG, about::OnCopyClick)
EVT_CLOSE(about::OnClose)
END_EVENT_TABLE()

about::about(void)
    : m_DataLocn(wxEmptyString), m_parent(NULL), m_btips_loaded(FALSE) {
  pAboutHTMLCtl = NULL;
  pLicenseHTMLCtl = NULL;
  pAuthorHTMLCtl = NULL;
  m_blicensePageSet = false;
}

about::about(wxWindow* parent, wxString Data_Locn, wxWindowID id,
             const wxString& caption, const wxPoint& pos, const wxSize& size,
             long style)
    : m_DataLocn(Data_Locn), m_parent(parent), m_btips_loaded(FALSE) {
  pAboutHTMLCtl = NULL;
  pLicenseHTMLCtl = NULL;
  pAuthorHTMLCtl = NULL;
  m_blicensePageSet = false;

  Create(parent, id, caption, pos, size, style);
}

bool about::Create(wxWindow* parent, wxWindowID id, const wxString& caption,
                   const wxPoint& pos, const wxSize& size, long style) {
  m_parent = parent;
#ifdef __WXOSX__
  style |= wxSTAY_ON_TOP;
#endif

  SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, id, caption, pos, size, style);
  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*qFont);

  m_displaySize = g_Platform->getDisplaySize();
  CreateControls();
  Populate();

  RecalculateSize();

  return TRUE;
}

void about::SetColorScheme(void) {
  DimeControl(this);
  wxColor bg = GetBackgroundColour();
  if (pAboutHTMLCtl) pAboutHTMLCtl->SetBackgroundColour(bg);
  if (pLicenseHTMLCtl) pLicenseHTMLCtl->SetBackgroundColour(bg);
  if (pAuthorHTMLCtl) pAuthorHTMLCtl->SetBackgroundColour(bg);

  // This looks like non-sense, but is needed for __WXGTK__
  // to get colours to propagate down the control's family tree.
  SetBackgroundColour(bg);

#ifdef __WXQT__
  // wxQT has some trouble clearing the background of HTML window...
  wxBitmap tbm(GetSize().x, GetSize().y, -1);
  wxMemoryDC tdc(tbm);
  tdc.SetBackground(bg);
  tdc.Clear();
  if (pAboutHTMLCtl) pAboutHTMLCtl->SetBackgroundImage(tbm);
  if (pLicenseHTMLCtl) pLicenseHTMLCtl->SetBackgroundImage(tbm);
  if (pAuthorHTMLCtl) pAuthorHTMLCtl->SetBackgroundImage(tbm);
#endif
}

void about::Populate(void) {
  wxColor bg = GetBackgroundColour();
  wxColor fg = wxColour(0, 0, 0);

  // The HTML Header
  wxString aboutText = wxString::Format(
      _T( "<html><body bgcolor=#%02x%02x%02x><font color=#%02x%02x%02x>" ),
      bg.Red(), bg.Blue(), bg.Green(), fg.Red(), fg.Blue(), fg.Green());

  wxFont* dFont = FontMgr::Get().GetFont(_("Dialog"));

  // Do weird font size calculation
  int points = dFont->GetPointSize();
#ifndef __WXOSX__
  ++points;
#endif
  int sizes[7];
  for (int i = -2; i < 5; i++) {
    sizes[i + 2] = points + i + (i > 0 ? i : 0);
  }
  wxString face = dFont->GetFaceName();
  pAboutHTMLCtl->SetFonts(face, face, sizes);

  if (wxFONTSTYLE_ITALIC == dFont->GetStyle()) aboutText.Append(_T("<i>"));

#ifdef __OCPN__ANDROID__
  wxString msg;
  msg.Printf(_T(" [%d]"), androidGetVersionCode());
  wxString OpenCPNVersionAndroid =
      _T("OpenCPN for Android Version ") + androidGetVersionName() + msg;

  aboutText.Append(AboutText + OpenCPNVersionAndroid + OpenCPNInfoAlt);
#else
  aboutText.Append(AboutText + OpenCPNVersion + OpenCPNInfo);
#endif

  // Show where the log file is going to be placed
  wxString log_string = _T("Logfile location: ") + g_Platform->GetLogFileName();
  log_string.Replace(_T("/"),
                     _T("/ "));  // allow line breaks, in a cheap way...

  aboutText.Append(log_string);

  // Show where the config file is going to be placed
  wxString config_string =
      _T("<br><br>Config file location: ") + g_Platform->GetConfigFileName();
  config_string.Replace(_T("/"),
                        _T("/ "));  // allow line breaks, in a cheap way...
  aboutText.Append(config_string);

  if (wxFONTSTYLE_ITALIC == dFont->GetStyle()) aboutText.Append(_T("</i>"));

  // The HTML Footer
  aboutText.Append(_T("</font></body></html>"));

  pAboutHTMLCtl->SetPage(aboutText);

  /// Authors page
  // The HTML Header
  wxString authorText = wxString::Format(
      _T( "<html><body bgcolor=#%02x%02x%02x><font color=#%02x%02x%02x>" ),
      bg.Red(), bg.Blue(), bg.Green(), fg.Red(), fg.Blue(), fg.Green());

  pAuthorHTMLCtl->SetFonts(face, face, sizes);

  wxString authorFixText = AuthorText;
  authorFixText.Replace(_T("\n"), _T("<br>"));
  authorText.Append(authorFixText);

  // The HTML Footer
  authorText.Append(_T("</font></body></html>"));

  pAuthorHTMLCtl->SetPage(authorFixText);

  /// License page
  // Deferred....
#if 0
    // The HTML Header
    wxString licenseText =
    wxString::Format(
        _T( "<html><body bgcolor=#%02x%02x%02x><font color=#%02x%02x%02x>" ),
            bg.Red(), bg.Blue(), bg.Green(), fg.Red(), fg.Blue(), fg.Green() );

    pLicenseHTMLCtl->SetFonts( face, face, sizes );

    wxTextFile license_filea( m_DataLocn + _T("license.txt") );
    if ( license_filea.Open() ) {
        for ( wxString str = license_filea.GetFirstLine(); !license_filea.Eof() ; str = license_filea.GetNextLine() )
            licenseText.Append( str + _T("<br>") );
        license_filea.Close();
    } else {
        wxLogMessage( _T("Could not open License file: ") + m_DataLocn );
    }

    wxString suppLicense = g_Platform->GetSupplementalLicenseString();

    wxStringTokenizer st(suppLicense, _T("\n"), wxTOKEN_DEFAULT);
    while( st.HasMoreTokens() )
    {
        wxString s1 = st.GetNextToken();
        licenseText.Append( s1 + _T("<br>") );
    }

        // The HTML Footer
    licenseText.Append( _T("</font></body></html>") );

    pLicenseHTMLCtl->SetPage( licenseText );
#endif

  SetColorScheme();
}

void about::RecalculateSize(void) {
  //  Make an estimate of the dialog size, without scrollbars showing

  wxSize esize;
  esize.x = GetCharWidth() * 110;
  esize.y = GetCharHeight() * 44;

  wxSize dsize = GetParent()->GetSize();
  esize.y = wxMin(esize.y, dsize.y - (0 * GetCharHeight()));
  esize.x = wxMin(esize.x, dsize.x - (0 * GetCharHeight()));
  SetClientSize(esize);

  wxSize fsize = GetSize();
  fsize.y = wxMin(fsize.y, dsize.y - (0 * GetCharHeight()));
  fsize.x = wxMin(fsize.x, dsize.x - (0 * GetCharHeight()));

  SetSize(fsize);

  Centre();
}

void about::CreateControls(void) {
  //  Set the nominal vertical size of the embedded controls

  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);
  wxStaticText* pST1 =
      new wxStaticText(this, -1, _("The Open Source Chart Plotter/Navigator"),
                       wxDefaultPosition, wxSize(-1, 50 /* 500, 30 */),
                       wxALIGN_CENTRE /* | wxALIGN_CENTER_VERTICAL */);

  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));

  wxFont* headerFont = FontMgr::Get().FindOrCreateFont(
      14, wxFONTFAMILY_DEFAULT, qFont->GetStyle(), wxFONTWEIGHT_BOLD, false,
      qFont->GetFaceName());
  pST1->SetFont(*headerFont);
  mainSizer->Add(pST1, 0, wxALL | wxEXPAND, 8);

  bool orient = m_displaySize.x < m_displaySize.y;
#ifndef __OCPN__ANDROID__
  wxBoxSizer* buttonSizer = new wxBoxSizer(orient ? wxVERTICAL : wxHORIZONTAL);
  mainSizer->Add(buttonSizer, 0, wxALL, 0);

  wxButton* donateButton = new wxBitmapButton(
      this, ID_DONATE, g_StyleManager->GetCurrentStyle()->GetIcon(_T("donate")),
      wxDefaultPosition, wxDefaultSize, 0);

  buttonSizer->Add(
      new wxButton(this, ID_COPYLOG, _("Copy Log File to Clipboard")), 1,
      wxALL | wxEXPAND, 3);
  buttonSizer->Add(
      new wxButton(this, ID_COPYINI, _("Copy Settings File to Clipboard")), 1,
      wxALL | wxEXPAND, 3);
  buttonSizer->Add(
      donateButton, 1,
      wxALL |
          (buttonSizer->GetOrientation() == wxHORIZONTAL ? wxALIGN_RIGHT : 0),
      3);
#endif

  //  Main Notebook
  pNotebook = new wxNotebook(this, ID_NOTEBOOK_HELP, wxDefaultPosition,
                             wxSize(-1, -1), wxNB_TOP);

  pNotebook->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
                     wxNotebookEventHandler(about::OnNBPageChange), NULL, this);

  pNotebook->InheritAttributes();
  mainSizer->Add(pNotebook, 1,
                 (orient ? wxALIGN_CENTER_VERTICAL : 0) | wxEXPAND | wxALL, 5);

  //  About Panel
  itemPanelAbout = new wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize,
                               wxSUNKEN_BORDER | wxTAB_TRAVERSAL);
  itemPanelAbout->InheritAttributes();
  pNotebook->AddPage(itemPanelAbout, _("About"), TRUE /* Default page */);

  pAboutHTMLCtl =
      new wxHtmlWindow(itemPanelAbout, wxID_ANY, wxDefaultPosition,
                       wxDefaultSize, wxHW_SCROLLBAR_AUTO | wxHW_NO_SELECTION);
  pAboutHTMLCtl->SetBorders(5);
  wxBoxSizer* aboutSizer = new wxBoxSizer(wxVERTICAL);
  aboutSizer->Add(pAboutHTMLCtl, 1,
                  wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL, 5);
  itemPanelAbout->SetSizer(aboutSizer);

  //  Authors Panel

  itemPanelAuthors =
      new wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize,
                  wxSUNKEN_BORDER | wxTAB_TRAVERSAL);
  itemPanelAuthors->InheritAttributes();
  pNotebook->AddPage(itemPanelAuthors, _("Authors"));

  pAuthorHTMLCtl =
      new wxHtmlWindow(itemPanelAuthors, wxID_ANY, wxDefaultPosition,
                       wxDefaultSize, wxHW_SCROLLBAR_AUTO | wxHW_NO_SELECTION);
  pAuthorHTMLCtl->SetBorders(5);
  wxBoxSizer* authorSizer = new wxBoxSizer(wxVERTICAL);
  authorSizer->Add(pAuthorHTMLCtl, 1,
                   wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL, 5);
  itemPanelAuthors->SetSizer(authorSizer);

  //  License Panel
  itemPanelLicense =
      new wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize,
                  wxSUNKEN_BORDER | wxTAB_TRAVERSAL);
  itemPanelLicense->InheritAttributes();
  pNotebook->AddPage(itemPanelLicense, _("License"));

  pLicenseHTMLCtl =
      new wxHtmlWindow(itemPanelLicense, wxID_ANY, wxDefaultPosition,
                       wxDefaultSize, wxHW_SCROLLBAR_AUTO | wxHW_NO_SELECTION);
  pLicenseHTMLCtl->SetBorders(5);
  wxBoxSizer* licenseSizer = new wxBoxSizer(wxVERTICAL);
  licenseSizer->Add(pLicenseHTMLCtl, 1,
                    wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL, 5);
  itemPanelLicense->SetSizer(licenseSizer);

  //  Help Panel
  itemPanelTips = new wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize,
                              wxSUNKEN_BORDER | wxTAB_TRAVERSAL);
  itemPanelTips->InheritAttributes();
  pNotebook->AddPage(itemPanelTips, _("Help"));

  wxBoxSizer* helpSizer = new wxBoxSizer(wxVERTICAL);
  itemPanelTips->SetSizer(helpSizer);

  //  Close Button
  wxButton* closeButton = new wxButton(this, xID_OK, _("Close"),
                                       wxDefaultPosition, wxDefaultSize, 0);
  closeButton->SetDefault();
  closeButton->InheritAttributes();
  mainSizer->Add(closeButton, 0, wxALIGN_RIGHT | wxALL, 5);
}

void about::OnNBPageChange(wxNotebookEvent& event) {
  unsigned int i = event.GetSelection();

  if (i == 3) {
    g_Platform->LaunchLocalHelp();
    pNotebook->ChangeSelection(0);
  }

  else if ((i == 2) && !m_blicensePageSet) {  // license

    wxColor bg = GetBackgroundColour();
    wxColor fg = wxColour(0, 0, 0);
    wxFont* dFont = FontMgr::Get().GetFont(_("Dialog"));

    // Do weird font size calculation
    int points = dFont->GetPointSize();
#ifndef __WXOSX__
    ++points;
#endif
    int sizes[7];
    for (int i = -2; i < 5; i++) {
      sizes[i + 2] = points + i + (i > 0 ? i : 0);
    }
    wxString face = dFont->GetFaceName();

    /// License page
    g_Platform->ShowBusySpinner();

    // The HTML Header
    wxString licenseText = wxString::Format(
        _T( "<html><body bgcolor=#%02x%02x%02x><font color=#%02x%02x%02x>" ),
        bg.Red(), bg.Blue(), bg.Green(), fg.Red(), fg.Blue(), fg.Green());

    pLicenseHTMLCtl->SetFonts(face, face, sizes);

    wxTextFile license_filea(m_DataLocn + _T("license.txt"));
    if (license_filea.Open()) {
      for (wxString str = license_filea.GetFirstLine(); !license_filea.Eof();
           str = license_filea.GetNextLine())
        licenseText.Append(str + _T("<br>"));
      license_filea.Close();
    } else {
      wxLogMessage(_T("Could not open License file: ") + m_DataLocn);
    }

    wxString suppLicense = g_Platform->GetSupplementalLicenseString();

    wxStringTokenizer st(suppLicense, _T("\n"), wxTOKEN_DEFAULT);
    while (st.HasMoreTokens()) {
      wxString s1 = st.GetNextToken();
      licenseText.Append(s1 + _T("<br>"));
    }

    // The HTML Footer
    licenseText.Append(_T("</font></body></html>"));

    pLicenseHTMLCtl->SetPage(licenseText);

    g_Platform->HideBusySpinner();

    SetColorScheme();
    m_blicensePageSet = true;
  }
}

void about::OnXidOkClick(wxCommandEvent& event) { Close(); }

void about::OnClose(wxCloseEvent& event) {
#ifdef __WXGTK__
  gFrame->Raise();
#endif
  Destroy();
  g_pAboutDlgLegacy = NULL;
}

void about::OnDonateClick(wxCommandEvent& event) {
  wxLaunchDefaultBrowser(
      _T("https://sourceforge.net/donate/index.php?group_id=180842"));
}

void about::OnCopyClick(wxCommandEvent& event) {
  wxString filename = event.GetId() == ID_COPYLOG
                          ? g_Platform->GetLogFileName()
                          : g_Platform->GetConfigFileName();

  wxFFile file(filename);

  if (!file.IsOpened()) {
    wxLogMessage(_T("Failed to open file for Copy to Clipboard."));
    return;
  }

  wxString fileContent;
  char buf[1024];
  while (!file.Eof()) {
    int c = file.Read(&buf, 1024);
    if (c) fileContent += wxString(buf, wxConvUTF8, c);
  }

  file.Close();
  int length = fileContent.Length();

  if (event.GetId() == ID_COPYLOG) {
    wxString lastLogs = fileContent;
    int pos = lastLogs.Find(_T("________"));
    while (pos != wxNOT_FOUND && lastLogs.Length() > 65000) {
      lastLogs = lastLogs.Right(lastLogs.Length() - pos - 8);
      pos = lastLogs.Find(_T("________"));
    }
    fileContent = lastLogs;
  }

  ::wxBeginBusyCursor();
  if (wxTheClipboard->Open()) {
    if (!wxTheClipboard->SetData(new wxTextDataObject(fileContent)))
      wxLogMessage(_T("wxTheClipboard->Open() failed."));
    wxTheClipboard->Close();
  } else {
    wxLogMessage(_T("wxTheClipboard->Open() failed."));
  }
  ::wxEndBusyCursor();
}
