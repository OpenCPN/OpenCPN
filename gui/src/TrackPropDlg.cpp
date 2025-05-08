/**************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Track Properties Dialog
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 **************************************************************************/
#include "config.h"

#include "model/georef.h"
#include "model/navobj_db.h"
#include "model/navutil_base.h"
#include "model/own_ship.h"
#include "model/plugin_comm.h"
#include "model/route.h"
#include "model/routeman.h"
#include "model/select.h"
#include "model/track.h"

#include "chcanv.h"
#include "displays.h"
#include "gui_lib.h"
#include "navutil.h"
#include "ocpn_frame.h"
#include "OCPNPlatform.h"
#include "pluginmanager.h"
#include "print_dialog.h"
#include "routemanagerdialog.h"
#include "routeman_gui.h"
#include "track_printout.h"
#include "TrackPropDlg.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

#define UTCINPUT 0  //!< Format date/time in UTC.
#define LTINPUT \
  1  //!< Format date/time using timezone configured in the operating system.
#define LMTINPUT \
  2  //!< Format date/time using Local Mean Time (LMT) at a given point.
/** Format date/time according to global OpenCPN settings. */
#define GLOBAL_SETTINGS_INPUT 3

#define INPUT_FORMAT 1
#define DISPLAY_FORMAT 2
#define TIMESTAMP_FORMAT 3

extern std::vector<Track*> g_TrackList;
extern ActiveTrack* g_pActiveTrack;
extern Routeman* g_pRouteMan;
extern RouteManagerDialog* pRouteManagerDialog;
extern MyConfig* pConfig;
extern MyFrame* gFrame;

///////////////////////////////////////////////////////////////////////////
bool TrackPropDlg::instanceFlag = false;
TrackPropDlg* TrackPropDlg::single = NULL;
TrackPropDlg* TrackPropDlg::getInstance(wxWindow* parent, wxWindowID id,
                                        const wxString& title,
                                        const wxPoint& pos, const wxSize& size,
                                        long style) {
  if (!instanceFlag) {
    single = new TrackPropDlg(parent, id, title, pos, size, style);
    instanceFlag = true;
    return single;
  } else {
    return single;
  }
}

TrackPropDlg::TrackPropDlg(wxWindow* parent, wxWindowID id,
                           const wxString& title, const wxPoint& pos,
                           const wxSize& size, long style)
    : DIALOG_PARENT(parent, id, title, pos, size, style) {
  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*qFont);

  SetWindowStyleFlag(style);

  m_scrolledWindowLinks = NULL;
  m_tDescription = NULL;

  m_bcompact = false;

#ifdef __ANDROID__
  m_bcompact = true;
  CreateControlsCompact();
#else
  CreateControls();
#endif

  RecalculateSize();

  // Centre( );

  // Connect Events
  m_sdbBtmBtnsSizerCancel->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(TrackPropDlg::OnCancelBtnClick), NULL, this);
  m_sdbBtmBtnsSizerOK->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(TrackPropDlg::OnOKBtnClick), NULL, this);
  if (m_sdbBtmBtnsSizerPrint)
    m_sdbBtmBtnsSizerPrint->Connect(
        wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(TrackPropDlg::OnPrintBtnClick), NULL, this);
  m_sdbBtmBtnsSizerSplit->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(TrackPropDlg::OnSplitBtnClick), NULL, this);
  m_sdbBtmBtnsSizerExtend->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(TrackPropDlg::OnExtendBtnClick), NULL, this);
  m_sdbBtmBtnsSizerToRoute->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(TrackPropDlg::OnToRouteBtnClick), NULL, this);
  if (m_sdbBtmBtnsSizerExport)
    m_sdbBtmBtnsSizerExport->Connect(
        wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(TrackPropDlg::OnExportBtnClick), NULL, this);
  m_lcPoints->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
                      wxListEventHandler(TrackPropDlg::OnTrackPropListClick),
                      NULL, this);
  Connect(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
          wxListEventHandler(TrackPropDlg::OnTrackPropRightClick), NULL, this);
  Connect(wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(TrackPropDlg::OnTrackPropMenuSelected), NULL,
          this);

#ifdef __WXOSX__
  Connect(wxEVT_ACTIVATE, wxActivateEventHandler(TrackPropDlg::OnActivate),
          NULL, this);
#endif

  if (!m_bcompact) {
    m_buttonAddLink->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                             wxCommandEventHandler(TrackPropDlg::OnAddLink),
                             NULL, this);
    m_toggleBtnEdit->Connect(
        wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
        wxCommandEventHandler(TrackPropDlg::OnEditLinkToggle), NULL, this);
  }

  if (m_rbShowTimeUTC)
    m_rbShowTimeUTC->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                             wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ),
                             NULL, this);
  if (m_rbShowTimePC)
    m_rbShowTimePC->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                            wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ),
                            NULL, this);
  if (m_rbShowTimeLocal)
    m_rbShowTimeLocal->Connect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ), NULL, this);

  if (m_rbShowTimeGlobalSettings)
    m_rbShowTimeGlobalSettings->Connect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ), NULL, this);

  m_pMyLinkList = NULL;
}

TrackPropDlg::~TrackPropDlg() {
  // Disconnect Events
  m_sdbBtmBtnsSizerCancel->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(TrackPropDlg::OnCancelBtnClick), NULL, this);
  m_sdbBtmBtnsSizerOK->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(TrackPropDlg::OnOKBtnClick), NULL, this);
  if (m_sdbBtmBtnsSizerPrint)
    m_sdbBtmBtnsSizerPrint->Disconnect(
        wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(TrackPropDlg::OnPrintBtnClick), NULL, this);
  m_sdbBtmBtnsSizerSplit->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(TrackPropDlg::OnSplitBtnClick), NULL, this);
  m_sdbBtmBtnsSizerExtend->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(TrackPropDlg::OnExtendBtnClick), NULL, this);
  m_sdbBtmBtnsSizerToRoute->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(TrackPropDlg::OnToRouteBtnClick), NULL, this);
  if (m_sdbBtmBtnsSizerExport)
    m_sdbBtmBtnsSizerExport->Disconnect(
        wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(TrackPropDlg::OnExportBtnClick), NULL, this);
  m_lcPoints->Disconnect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
                         wxListEventHandler(TrackPropDlg::OnTrackPropListClick),
                         NULL, this);
  Disconnect(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
             wxListEventHandler(TrackPropDlg::OnTrackPropRightClick), NULL,
             this);
  Disconnect(wxEVT_COMMAND_MENU_SELECTED,
             wxCommandEventHandler(TrackPropDlg::OnTrackPropMenuSelected), NULL,
             this);

  Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
             wxCommandEventHandler(TrackPropDlg::OnDeleteLink));
  Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
             wxCommandEventHandler(TrackPropDlg::OnEditLink));
  Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
             wxCommandEventHandler(TrackPropDlg::OnAddLink));

  if (!m_bcompact) {
    m_buttonAddLink->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
                                wxCommandEventHandler(TrackPropDlg::OnAddLink),
                                NULL, this);
    m_toggleBtnEdit->Disconnect(
        wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
        wxCommandEventHandler(TrackPropDlg::OnEditLinkToggle), NULL, this);
  }

  if (m_rbShowTimeUTC)
    m_rbShowTimeUTC->Disconnect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ), NULL, this);
  if (m_rbShowTimePC)
    m_rbShowTimePC->Disconnect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ), NULL, this);
  if (m_rbShowTimeLocal)
    m_rbShowTimeLocal->Disconnect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ), NULL, this);
  if (m_rbShowTimeGlobalSettings)
    m_rbShowTimeGlobalSettings->Disconnect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ), NULL, this);

  instanceFlag = false;
}

static wxString getDatetimeTimezoneSelector(int selection) {
  switch (selection) {
    case UTCINPUT:
      return "UTC";
    case LTINPUT:
      return "Local Time";
    case LMTINPUT:
      return "LMT";
    case GLOBAL_SETTINGS_INPUT:
    default:
      return wxEmptyString;
  }
}

void TrackPropDlg::OnActivate(wxActivateEvent& event) {
  auto pWin = dynamic_cast<DIALOG_PARENT*>(event.GetEventObject());
  long int style = pWin->GetWindowStyle();
  if (event.GetActive())
    pWin->SetWindowStyle(style | wxSTAY_ON_TOP);
  else
    pWin->SetWindowStyle(style ^ wxSTAY_ON_TOP);
}

void TrackPropDlg::RecalculateSize(void) {
  //  Make an estimate of the dialog size, without scrollbars showing

  wxSize esize;
  esize.x = GetCharWidth() * 110;
  esize.y = GetCharHeight() * 40;

  wxSize dsize = GetParent()->GetClientSize();
  esize.y = wxMin(esize.y, dsize.y - (2 * GetCharHeight()));
  esize.x = wxMin(esize.x, dsize.x - (2 * GetCharHeight()));
  SetClientSize(esize);

  wxSize fsize = GetSize();
  fsize.y = wxMin(fsize.y, dsize.y - (2 * GetCharHeight()));
  fsize.x = wxMin(fsize.x, dsize.x - (2 * GetCharHeight()));
  SetSize(fsize);

  if (m_bcompact) {
    int sy = GetCharHeight() * m_lcPoints->GetItemCount();
    sy = wxMax(sy, 250);
    sy = wxMin(sy, 500);
    m_lcPoints->SetSize(wxSize(GetClientSize().x - 40, sy));

    if (m_lcPoints->GetItemCount()) Layout();
  }

  Centre();
}

static void addColumns(wxListCtrl* lctrl, int dx) {
  lctrl->InsertColumn(0, _("Leg"), wxLIST_FORMAT_LEFT, dx * 6);
  lctrl->InsertColumn(1, _("Distance"), wxLIST_FORMAT_LEFT, dx * 11);
  lctrl->InsertColumn(2, _("Bearing"), wxLIST_FORMAT_LEFT, dx * 8);
  // Width of lat/lon may be up to 15 characters: 'DDD° MM.MMMM' W'.
  lctrl->InsertColumn(3, _("Latitude"), wxLIST_FORMAT_LEFT, dx * 15);
  lctrl->InsertColumn(4, _("Longitude"), wxLIST_FORMAT_LEFT, dx * 15);
  // Width of timestamp may  be be up to 26 characters: 'MM/DD/YYYY HH:MM:SS PM
  // UTC'.
  lctrl->InsertColumn(5, _("Timestamp"), wxLIST_FORMAT_LEFT, dx * 26);
  lctrl->InsertColumn(6, _("Speed"), wxLIST_FORMAT_CENTER, dx * 8);

  lctrl->SetMinSize(wxSize(-1, 50));
}

void TrackPropDlg::CreateControlsCompact() {
  wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
  SetSizer(itemBoxSizer1);

  itemDialog1 = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition,
                                     wxSize(-1, -1), wxVSCROLL);
  itemDialog1->SetScrollRate(0, 1);

#ifdef __ANDROID__
  //  Set Dialog Font by custom crafted Qt Stylesheet.
  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));

  wxString wqs = getFontQtStylesheet(qFont);
  wxCharBuffer sbuf = wqs.ToUTF8();
  QString qsb = QString(sbuf.data());

  QString qsbq = getQtStyleSheet();  // basic scrollbars, etc

  itemDialog1->GetHandle()->setStyleSheet(qsb +
                                          qsbq);  // Concatenated style sheets

#endif
  itemBoxSizer1->Add(itemDialog1, 1, wxEXPAND | wxALL, 0);

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticText* itemStaticText4 = new wxStaticText(
      itemDialog1, wxID_STATIC, _("Name"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer2->Add(itemStaticText4, 0,
                     wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5);

  m_tName = new wxTextCtrl(itemDialog1, wxID_ANY, _T(""), wxDefaultPosition,
                           wxSize(400, -1), 0);
  itemBoxSizer2->Add(m_tName, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM, 5);

  wxStaticText* itemStaticText7 =
      new wxStaticText(itemDialog1, wxID_STATIC, _("Depart From"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer2->Add(itemStaticText7, 0,
                     wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT,
                     5);

  m_tFrom = new wxTextCtrl(itemDialog1, wxID_ANY, _T(""), wxDefaultPosition,
                           wxSize(-1, -1), 0);
  itemBoxSizer2->Add(m_tFrom, 0,
                     wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL |
                         wxLEFT | wxRIGHT | wxBOTTOM,
                     5);

  wxStaticText* itemStaticText8 =
      new wxStaticText(itemDialog1, wxID_STATIC, _("Destination"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer2->Add(itemStaticText8, 0,
                     wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT,
                     5);

  m_tTo = new wxTextCtrl(itemDialog1, wxID_ANY, _T(""), wxDefaultPosition,
                         wxSize(-1, -1), 0);
  itemBoxSizer2->Add(m_tTo, 0,
                     wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL |
                         wxLEFT | wxRIGHT | wxBOTTOM,
                     5);

  m_cbShow = new wxCheckBox(itemDialog1, wxID_ANY, _("Show on chart"),
                            wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer2->Add(m_cbShow, 0, wxALL, 5);

  wxFlexGridSizer* itemFlexGridSizer6a = new wxFlexGridSizer(4, 2, 0, 0);
  itemFlexGridSizer6a->AddGrowableCol(1, 0);

  itemBoxSizer2->Add(itemFlexGridSizer6a, 0, wxEXPAND | wxALIGN_LEFT | wxALL,
                     5);

  wxStaticText* itemStaticText11 =
      new wxStaticText(itemDialog1, wxID_STATIC, _("Total distance"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer6a->Add(
      itemStaticText11, 0,
      wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP, 5);

  m_tTotDistance =
      new wxTextCtrl(itemDialog1, wxID_ANY, _T(""), wxDefaultPosition,
                     wxSize(-1, -1), wxTE_READONLY);
  itemFlexGridSizer6a->Add(m_tTotDistance, 0,
                           wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL |
                               wxLEFT | wxRIGHT | wxBOTTOM,
                           5);

  m_stAvgSpeed = new wxStaticText(itemDialog1, wxID_STATIC, _("Avg. speed"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer6a->Add(
      m_stAvgSpeed, 0,
      wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP, 5);

  m_tAvgSpeed = new wxTextCtrl(itemDialog1, wxID_ANY, _T(""), wxDefaultPosition,
                               wxSize(150, -1), wxTE_PROCESS_ENTER);
  itemFlexGridSizer6a->Add(
      m_tAvgSpeed, 0,
      wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5);

  m_stTimeEnroute =
      new wxStaticText(itemDialog1, wxID_STATIC, _("Time Enroute"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer6a->Add(
      m_stTimeEnroute, 0,
      wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP, 5);

  m_tTimeEnroute =
      new wxTextCtrl(itemDialog1, wxID_ANY, _T(""), wxDefaultPosition,
                     wxSize(-1, -1), wxTE_READONLY);
  itemFlexGridSizer6a->Add(m_tTimeEnroute, 0,
                           wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL |
                               wxLEFT | wxRIGHT | wxBOTTOM,
                           5);

  /*
      m_StartTimeLabel = new wxStaticText( itemDialog1, wxID_STATIC,
     _("Departure Time"), wxDefaultPosition, wxDefaultSize, 0 );
      itemFlexGridSizer6a->Add( m_StartTimeLabel, 0,
              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
              5 );

      m_StartTimeCtl = new wxTextCtrl( itemDialog1, ID_STARTTIMECTL, _T(""),
     wxDefaultPosition, wxSize( -1, -1 ), wxTE_PROCESS_ENTER );
      itemFlexGridSizer6a->Add( m_StartTimeCtl, 0,
              wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT |
     wxRIGHT | wxBOTTOM, 5 );
  */

  wxString pDispTimeZone[] = {_("UTC"), _("Local Time"), _("LMT@Location"),
                              _("Honor Global Settings")};

  wxStaticText* itemStaticText12b =
      new wxStaticText(itemDialog1, wxID_STATIC, _("Time shown as"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer2->Add(itemStaticText12b, 0, wxEXPAND | wxALL, 5);

  m_rbShowTimeUTC =
      new wxRadioButton(itemDialog1, wxID_ANY, _("UTC"), wxDefaultPosition,
                        wxDefaultSize, wxRB_GROUP);
  itemBoxSizer2->Add(
      m_rbShowTimeUTC, 0,
      wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5);

  m_rbShowTimePC = new wxRadioButton(itemDialog1, wxID_ANY, _("Local Time"));
  itemBoxSizer2->Add(
      m_rbShowTimePC, 0,
      wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5);

  m_rbShowTimeLocal =
      new wxRadioButton(itemDialog1, wxID_ANY, _("LMT@Location"));
  itemBoxSizer2->Add(
      m_rbShowTimeLocal, 0,
      wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5);

  m_rbShowTimeGlobalSettings =
      new wxRadioButton(itemDialog1, wxID_ANY, _("Honor Global Settings"));
  itemBoxSizer2->Add(
      m_rbShowTimeGlobalSettings, 0,
      wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5);

  wxFlexGridSizer* itemFlexGridSizer6b = new wxFlexGridSizer(3, 2, 0, 0);
  itemBoxSizer2->Add(itemFlexGridSizer6b, 0, wxEXPAND | wxALIGN_LEFT | wxALL,
                     5);

  wxStaticText* m_staticText1 =
      new wxStaticText(itemDialog1, wxID_ANY, _("Color") + _T(":"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer6b->Add(m_staticText1, 0, wxALIGN_CENTER_VERTICAL | wxALL,
                           5);

  wxString m_chColorChoices[] = {
      _("Default color"), _("Black"),     _("Dark Red"),     _("Dark Green"),
      _("Dark Yellow"),   _("Dark Blue"), _("Dark Magenta"), _("Dark Cyan"),
      _("Light Gray"),    _("Dark Gray"), _("Red"),          _("Green"),
      _("Yellow"),        _("Blue"),      _("Magenta"),      _("Cyan"),
      _("White")};
  int m_chColorNChoices = sizeof(m_chColorChoices) / sizeof(wxString);
  m_cColor =
      new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(250, -1),
                   m_chColorNChoices, m_chColorChoices, 0);
  m_cColor->SetSelection(0);
  itemFlexGridSizer6b->Add(m_cColor, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxStaticText* staticTextStyle =
      new wxStaticText(itemDialog1, wxID_ANY, _("Style") + _T(":"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer6b->Add(staticTextStyle, 0, wxALIGN_CENTER_VERTICAL | wxALL,
                           5);

  wxString m_chStyleChoices[] = {_("Default"), _("Solid"), _("Dot"),
                                 _("Long dash"), _("Short dash")};
  int m_chStyleNChoices = sizeof(m_chStyleChoices) / sizeof(wxString);
  m_cStyle =
      new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_chStyleNChoices, m_chStyleChoices, 0);
  m_cStyle->SetSelection(0);
  itemFlexGridSizer6b->Add(m_cStyle, 0,
                           wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 5);

#ifdef ocpnUSE_GLES  // linestipple is emulated poorly
  staticTextStyle->Hide();
  m_cStyle->Hide();
#endif

  m_stWidth = new wxStaticText(itemDialog1, wxID_ANY, _("Width") + _T(":"),
                               wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer6b->Add(m_stWidth, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxString m_chWidthChoices[] = {_("Default"),  _("1 pixel"),  _("2 pixels"),
                                 _("3 pixels"), _("4 pixels"), _("5 pixels"),
                                 _("6 pixels"), _("7 pixels"), _("8 pixels"),
                                 _("9 pixels"), _("10 pixels")};
  int m_chWidthNChoices = sizeof(m_chWidthChoices) / sizeof(wxString);
  m_cWidth =
      new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(150, -1),
                   m_chWidthNChoices, m_chWidthChoices, 0);
  m_cWidth->SetSelection(0);
  itemFlexGridSizer6b->Add(m_cWidth, 0,
                           wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxStaticBox* itemStaticBoxSizer14Static =
      new wxStaticBox(itemDialog1, wxID_ANY, _("Waypoints"));
  wxStaticBoxSizer* m_pListSizer =
      new wxStaticBoxSizer(itemStaticBoxSizer14Static, wxVERTICAL);
  itemBoxSizer2->Add(m_pListSizer, 1, wxEXPAND | wxALL, 1);

  wxScrolledWindow* itemlistWin =
      new wxScrolledWindow(itemDialog1, wxID_ANY, wxDefaultPosition,
                           wxDefaultSize, wxHSCROLL | wxVSCROLL);
  itemlistWin->SetScrollRate(2, 2);

  m_pListSizer->Add(itemlistWin, 0, wxEXPAND | wxALL, 6);

  long flags = wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_EDIT_LABELS;
#ifndef __WXQT__  // Does not support Virtual list boxes....
  flags |= wxLC_VIRTUAL;
#endif

  m_lcPoints = new OCPNTrackListCtrl(itemlistWin, wxID_ANY, wxDefaultPosition,
                                     wxSize(100, 500), flags);
  addColumns(m_lcPoints, GetCharWidth());

  // sbSizerPoints->Add( m_lcPoints, 1, wxALL|wxEXPAND, 5 );

#ifdef __ANDROID__
  m_lcPoints->GetHandle()->setStyleSheet(getQtStyleSheet());
#endif

  //  Buttons, etc...

  wxBoxSizer* itemBoxSizerBottom = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer1->Add(itemBoxSizerBottom, 0, wxALIGN_LEFT | wxALL | wxEXPAND, 5);

  int n_col = 3;

  wxFlexGridSizer* itemBoxSizerAux = new wxFlexGridSizer(0, n_col, 0, 0);
  itemBoxSizerAux->SetFlexibleDirection(wxBOTH);
  itemBoxSizerAux->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  itemBoxSizerBottom->Add(itemBoxSizerAux, 1, wxALIGN_LEFT | wxALL, 5);

#ifndef __ANDROID__

  m_sdbBtmBtnsSizerPrint = new wxButton(this, wxID_ANY, _("Print"),
                                        wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizerAux->Add(m_sdbBtmBtnsSizerPrint, 0,
                       wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_sdbBtmBtnsSizerPrint->Enable(true);
#else
  m_sdbBtmBtnsSizerPrint = NULL;
#endif

  m_sdbBtmBtnsSizerSplit = new wxButton(this, wxID_ANY, _("Split"),
                                        wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizerAux->Add(m_sdbBtmBtnsSizerSplit, 0,
                       wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_sdbBtmBtnsSizerSplit->Enable(false);

  m_sdbBtmBtnsSizerExtend = new wxButton(this, wxID_ANY, _("Extend"),
                                         wxDefaultPosition, wxDefaultSize, 0);
  m_sdbBtmBtnsSizerExtend->SetToolTip(
      _("Extends this track by connecting it to another track.\n"
        "Disabled when the track is active, the track is in a layer, or no "
        "suitable track to connect to exists.\n"
        "A suitable track is one that is visible, is different from this "
        "track, and has its last point's timestamp earlier than or equal to "
        "this track's first point's timestamp."));
  itemBoxSizerAux->Add(m_sdbBtmBtnsSizerExtend, 0,
                       wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_sdbBtmBtnsSizerToRoute = new wxButton(this, wxID_ANY, _("To route"),
                                          wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizerAux->Add(m_sdbBtmBtnsSizerToRoute, 0,
                       wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_sdbBtmBtnsSizerExport = NULL;
  //      m_sdbBtmBtnsSizerExport = new wxButton( this, wxID_ANY, _("Export"),
  //      wxDefaultPosition, wxDefaultSize, 0 ); itemBoxSizerAux->Add(
  //      m_sdbBtmBtnsSizerExport, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL |
  //      wxALL, 5 );

  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizerBottom->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_sdbBtmBtnsSizerCancel = new wxButton(this, wxID_CANCEL, _("Cancel"),
                                         wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_sdbBtmBtnsSizerCancel, 0,
                      wxALIGN_RIGHT | wxALIGN_BOTTOM | wxALL, 5);

  m_sdbBtmBtnsSizerOK =
      new wxButton(this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_sdbBtmBtnsSizerOK, 0,
                      wxALIGN_RIGHT | wxALIGN_BOTTOM | wxALL, 5);
  m_sdbBtmBtnsSizerOK->SetDefault();

  int char_size = GetCharWidth();
  // Set the maximum size of the entire  dialog
  int width, height;
  width = g_monitor_info[g_current_monitor].width;
  height = g_monitor_info[g_current_monitor].height;
  SetSizeHints(-1, -1, width - 100, height - 100);
}

void TrackPropDlg::CreateControls(void) {
  wxBoxSizer* bSizerMain;
  bSizerMain = new wxBoxSizer(wxVERTICAL);
  SetSizer(bSizerMain);

  m_notebook1 =
      new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);

  bSizerMain->Add(m_notebook1, 1, wxEXPAND | wxALL, 5);

  wxPanel* m_panel0 =
      new wxPanel(m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize);
  m_notebook1->AddPage(m_panel0, _("Basic"), true);

  wxBoxSizer* bSizer0 = new wxBoxSizer(wxVERTICAL);
  m_panel0->SetSizer(bSizer0);

  wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
  bSizer0->Add(bSizer1, 1, wxEXPAND | wxALL, 5);

  m_panelBasic = new wxScrolledWindow(m_panel0, wxID_ANY, wxDefaultPosition,
                                      wxSize(-1, -1),
                                      wxHSCROLL | wxVSCROLL | wxTAB_TRAVERSAL);

#ifdef __ANDROID__
  //  Set Dialog Font by custom crafted Qt Stylesheet.
  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));

  wxString wqs = getFontQtStylesheet(qFont);
  wxCharBuffer sbuf = wqs.ToUTF8();
  QString qsb = QString(sbuf.data());

  QString qsbq = getQtStyleSheet();  // basic scrollbars, etc

  m_panelBasic->GetHandle()->setStyleSheet(qsb +
                                           qsbq);  // Concatenated style sheets

#endif

  // #ifdef __ANDROID__
  //     m_panelBasic->GetHandle()->setStyleSheet( getQtStyleSheet());
  // #endif

  wxBoxSizer* bSizerBasic = new wxBoxSizer(wxVERTICAL);
  m_panelBasic->SetSizer(bSizerBasic);

  bSizer1->Add(m_panelBasic, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer* bSizerName;
  bSizerName = new wxBoxSizer(wxHORIZONTAL);
  bSizerBasic->Add(bSizerName, 0, wxALL | wxEXPAND, 5);

  m_stName = new wxStaticText(m_panelBasic, wxID_ANY, _("Name"),
                              wxDefaultPosition, wxDefaultSize, 0);
  // m_stName->Wrap( -1 );
  bSizerName->Add(m_stName, 0, wxALL, 5);

  m_tName = new wxTextCtrl(m_panelBasic, wxID_ANY, wxEmptyString,
                           wxDefaultPosition, wxDefaultSize, 0);
  bSizerName->Add(m_tName, 1, 0, 5);

  wxBoxSizer* bSizerFromTo;
  bSizerFromTo = new wxBoxSizer(wxHORIZONTAL);
  bSizerBasic->Add(bSizerFromTo, 0, wxALL | wxEXPAND, 5);

  m_stFrom = new wxStaticText(m_panelBasic, wxID_ANY, _("From"),
                              wxDefaultPosition, wxDefaultSize, 0);
  // m_stFrom->Wrap( -1 );
  bSizerFromTo->Add(m_stFrom, 0, wxALL, 5);

  m_tFrom = new wxTextCtrl(m_panelBasic, wxID_ANY, wxEmptyString,
                           wxDefaultPosition, wxDefaultSize, 0);
  bSizerFromTo->Add(m_tFrom, 1, 0, 5);

  m_stTo = new wxStaticText(m_panelBasic, wxID_ANY, _("To"), wxDefaultPosition,
                            wxDefaultSize, 0);
  // m_stTo->Wrap( -1 );
  bSizerFromTo->Add(m_stTo, 0, wxALL, 5);

  m_tTo = new wxTextCtrl(m_panelBasic, wxID_ANY, wxEmptyString,
                         wxDefaultPosition, wxDefaultSize, 0);
  bSizerFromTo->Add(m_tTo, 1, 0, 5);

  wxStaticBoxSizer* sbSizerParams;
  sbSizerParams = new wxStaticBoxSizer(
      new wxStaticBox(m_panelBasic, wxID_ANY, _("Display parameters")),
      wxHORIZONTAL);
  bSizerBasic->Add(sbSizerParams, 0, wxALL | wxEXPAND, 5);

  m_cbShow = new wxCheckBox(m_panelBasic, wxID_ANY, _("Show on chart"),
                            wxDefaultPosition, wxDefaultSize, 0);
  sbSizerParams->Add(m_cbShow, 0, wxALL, 5);

  m_stColor = new wxStaticText(m_panelBasic, wxID_ANY, _("Color"),
                               wxDefaultPosition, wxDefaultSize, 0);
  // m_stColor->Wrap( -1 );
  sbSizerParams->Add(m_stColor, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxString m_cColorChoices[] = {
      _("Default color"), _("Black"),     _("Dark Red"),     _("Dark Green"),
      _("Dark Yellow"),   _("Dark Blue"), _("Dark Magenta"), _("Dark Cyan"),
      _("Light Gray"),    _("Dark Gray"), _("Red"),          _("Green"),
      _("Yellow"),        _("Blue"),      _("Magenta"),      _("Cyan"),
      _("White")};
  int m_cColorNChoices = sizeof(m_cColorChoices) / sizeof(wxString);
  m_cColor = new wxChoice(m_panelBasic, wxID_ANY, wxDefaultPosition,
                          wxDefaultSize, m_cColorNChoices, m_cColorChoices, 0);
  m_cColor->SetSelection(0);
  sbSizerParams->Add(m_cColor, 1, 0, 5);

  m_stStyle = new wxStaticText(m_panelBasic, wxID_ANY, _("Style"),
                               wxDefaultPosition, wxDefaultSize, 0);
  // m_stStyle->Wrap( -1 );
  sbSizerParams->Add(m_stStyle, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxString m_cStyleChoices[] = {_("Default"), _("Solid"), _("Dot"),
                                _("Long dash"), _("Short dash")};
  int m_cStyleNChoices = sizeof(m_cStyleChoices) / sizeof(wxString);
  m_cStyle = new wxChoice(m_panelBasic, wxID_ANY, wxDefaultPosition,
                          wxDefaultSize, m_cStyleNChoices, m_cStyleChoices, 0);
  m_cStyle->SetSelection(0);
  sbSizerParams->Add(m_cStyle, 1, 0, 5);

  m_stWidth = new wxStaticText(m_panelBasic, wxID_ANY, _("Width"),
                               wxDefaultPosition, wxDefaultSize, 0);
  // m_stWidth->Wrap( -1 );
  sbSizerParams->Add(m_stWidth, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxString m_cWidthChoices[] = {_("Default"),  _("1 pixel"),  _("2 pixels"),
                                _("3 pixels"), _("4 pixels"), _("5 pixels"),
                                _("6 pixels"), _("7 pixels"), _("8 pixels"),
                                _("9 pixels"), _("10 pixels")};
  int m_cWidthNChoices = sizeof(m_cWidthChoices) / sizeof(wxString);
  m_cWidth = new wxChoice(m_panelBasic, wxID_ANY, wxDefaultPosition,
                          wxDefaultSize, m_cWidthNChoices, m_cWidthChoices, 0);
  m_cWidth->SetSelection(0);
  sbSizerParams->Add(m_cWidth, 1, 0, 5);

  wxStaticBoxSizer* sbSizerStats;
  sbSizerStats = new wxStaticBoxSizer(
      new wxStaticBox(m_panelBasic, wxID_ANY, _("Statistics")), wxVERTICAL);
  bSizerBasic->Add(sbSizerStats, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer* bSizerStats;
  bSizerStats = new wxBoxSizer(wxHORIZONTAL);

  m_stTotDistance =
      new wxStaticText(m_panelBasic, wxID_ANY, _("Total distance"),
                       wxDefaultPosition, wxDefaultSize, 0);
  // m_stTotDistance->Wrap( -1 );
  bSizerStats->Add(m_stTotDistance, 0, wxALL, 5);

  m_tTotDistance =
      new wxTextCtrl(m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_READONLY);
  bSizerStats->Add(m_tTotDistance, 1, 0, 5);

  m_stAvgSpeed = new wxStaticText(m_panelBasic, wxID_ANY, _("Avg. speed"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  // m_stAvgSpeed->Wrap( -1 );
  bSizerStats->Add(m_stAvgSpeed, 0, wxALL, 5);

  m_tAvgSpeed = new wxTextCtrl(m_panelBasic, wxID_ANY, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  bSizerStats->Add(m_tAvgSpeed, 1, 0, 5);

  m_stTimeEnroute = new wxStaticText(m_panelBasic, wxID_ANY, _("Time enroute"),
                                     wxDefaultPosition, wxDefaultSize, 0);
  // m_stTimeEnroute->Wrap( -1 );
  bSizerStats->Add(m_stTimeEnroute, 0, wxALL, 5);

  m_tTimeEnroute =
      new wxTextCtrl(m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_READONLY);
  bSizerStats->Add(m_tTimeEnroute, 2, 0, 5);

  sbSizerStats->Add(bSizerStats, 0, wxEXPAND, 5);

  wxStaticBoxSizer* sbSizerPoints;
  sbSizerPoints = new wxStaticBoxSizer(
      new wxStaticBox(m_panel0, wxID_ANY, _("Recorded points")), wxVERTICAL);
  bSizer1->Add(sbSizerPoints, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer* bSizerShowTime;
  bSizerShowTime = new wxBoxSizer(wxHORIZONTAL);

  m_stShowTime = new wxStaticText(m_panel0, wxID_ANY, _("Time shown as"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  // m_stShowTime->Wrap( -1 );
  bSizerShowTime->Add(m_stShowTime, 0, wxALL, 5);

  m_rbShowTimeUTC = new wxRadioButton(m_panel0, wxID_ANY, _("UTC"),
                                      wxDefaultPosition, wxDefaultSize, 0);
  bSizerShowTime->Add(m_rbShowTimeUTC, 0,
                      wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT,
                      5);

  m_rbShowTimePC = new wxRadioButton(m_panel0, wxID_ANY, _("Local Time"),
                                     wxDefaultPosition, wxDefaultSize, 0);
  bSizerShowTime->Add(m_rbShowTimePC, 0,
                      wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT,
                      5);

  m_rbShowTimeLocal =
      new wxRadioButton(m_panel0, wxID_ANY, _("LMT@Track Start"),
                        wxDefaultPosition, wxDefaultSize, 0);
  bSizerShowTime->Add(m_rbShowTimeLocal, 0,
                      wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT,
                      5);

  m_rbShowTimeGlobalSettings =
      new wxRadioButton(m_panel0, wxID_ANY, _("Honor Global Settings"),
                        wxDefaultPosition, wxDefaultSize, 0);
  bSizerShowTime->Add(m_rbShowTimeGlobalSettings, 0,
                      wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT,
                      5);

  m_rbShowTimeGlobalSettings->SetValue(true);

  sbSizerPoints->Add(bSizerShowTime, 0, wxEXPAND, 5);

  long flags = wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_EDIT_LABELS;
#ifndef __WXQT__  // Does not support Virtual list boxes....
  flags |= wxLC_VIRTUAL;
#endif

  m_lcPoints = new OCPNTrackListCtrl(m_panel0, wxID_ANY, wxDefaultPosition,
                                     wxDefaultSize, flags);
  addColumns(m_lcPoints, GetCharWidth());

  sbSizerPoints->Add(m_lcPoints, 1, wxALL | wxEXPAND, 5);

#ifdef __ANDROID__
  m_lcPoints->GetHandle()->setStyleSheet(getQtStyleSheet());
#endif

  //       m_lcPoints = new OCPNTrackListCtrl( m_panel0, wxID_ANY,
  //       wxDefaultPosition, wxDefaultSize,
  //                                           wxLC_REPORT | wxLC_HRULES |
  //                                           wxLC_VRULES | wxLC_EDIT_LABELS |
  //                                           wxLC_VIRTUAL );
  //
  //       m_lcPoints->Hide();
  //       m_rbShowTimeUTC = NULL;
  //       m_rbShowTimePC = NULL;
  //       m_rbShowTimeLocal = NULL;

  m_panelAdvanced = new wxScrolledWindow(
      m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

  wxBoxSizer* bSizerAdvanced;
  bSizerAdvanced = new wxBoxSizer(wxVERTICAL);

  m_stDescription =
      new wxStaticText(m_panelAdvanced, wxID_ANY, _("Description"),
                       wxDefaultPosition, wxDefaultSize, 0);
  // m_stDescription->Wrap( -1 );
  bSizerAdvanced->Add(m_stDescription, 0, wxALL, 5);

  m_tDescription =
      new wxTextCtrl(m_panelAdvanced, wxID_ANY, wxEmptyString,
                     wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
  bSizerAdvanced->Add(m_tDescription, 1, wxALL | wxEXPAND, 5);

  sbSizerLinks = new wxStaticBoxSizer(
      new wxStaticBox(m_panelAdvanced, wxID_ANY, _("Links")), wxVERTICAL);

  m_scrolledWindowLinks =
      new wxScrolledWindow(m_panelAdvanced, wxID_ANY, wxDefaultPosition,
                           wxDefaultSize, wxHSCROLL | wxVSCROLL);
  m_scrolledWindowLinks->SetScrollRate(5, 5);
  bSizerLinks = new wxBoxSizer(wxVERTICAL);

  m_hyperlink1 =
      new wxHyperlinkCtrl(m_scrolledWindowLinks, wxID_ANY, _("wxFB Website"),
                          wxT("http://www.wxformbuilder.org"),
                          wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
  m_menuLink = new wxMenu();
  m_menuItemEdit =
      new wxMenuItem(m_menuLink, ID_TRK_MENU_EDIT, wxString(_("Edit")),
                     wxEmptyString, wxITEM_NORMAL);
  m_menuLink->Append(m_menuItemEdit);

  m_menuItemAdd =
      new wxMenuItem(m_menuLink, ID_TRK_MENU_ADD, wxString(_("Add new")),
                     wxEmptyString, wxITEM_NORMAL);
  m_menuLink->Append(m_menuItemAdd);

  m_menuItemDelete =
      new wxMenuItem(m_menuLink, ID_TRK_MENU_DELETE, wxString(_("Delete")),
                     wxEmptyString, wxITEM_NORMAL);
  m_menuLink->Append(m_menuItemDelete);

  m_hyperlink1->Connect(
      wxEVT_RIGHT_DOWN,
      wxMouseEventHandler(TrackPropDlg::m_hyperlink1OnContextMenu), NULL, this);

  bSizerLinks->Add(m_hyperlink1, 0, wxALL, 5);

  m_scrolledWindowLinks->SetSizer(bSizerLinks);
  m_scrolledWindowLinks->Layout();
  bSizerLinks->Fit(m_scrolledWindowLinks);
  sbSizerLinks->Add(m_scrolledWindowLinks, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer* bSizerLinkBtns;
  bSizerLinkBtns = new wxBoxSizer(wxHORIZONTAL);

  m_buttonAddLink =
      new wxButton(m_panelAdvanced, wxID_ANY, _("Add"), wxDefaultPosition,
                   wxDefaultSize, wxBU_EXACTFIT);
  bSizerLinkBtns->Add(m_buttonAddLink, 0, wxALL, 5);

  m_toggleBtnEdit = new wxToggleButton(m_panelAdvanced, wxID_ANY, _("Edit"),
                                       wxDefaultPosition, wxDefaultSize, 0);
  bSizerLinkBtns->Add(m_toggleBtnEdit, 0, wxALL, 5);

  m_staticTextEditEnabled = new wxStaticText(
      m_panelAdvanced, wxID_ANY, _("Links are opened in the default browser."),
      wxDefaultPosition, wxDefaultSize, 0);
  // m_staticTextEditEnabled->Wrap( -1 );
  bSizerLinkBtns->Add(m_staticTextEditEnabled, 0,
                      wxALIGN_CENTER_VERTICAL | wxALL, 5);

  sbSizerLinks->Add(bSizerLinkBtns, 0, wxEXPAND, 5);

  bSizerAdvanced->Add(sbSizerLinks, 1, wxEXPAND, 5);

  m_panelAdvanced->SetSizer(bSizerAdvanced);
  m_notebook1->AddPage(m_panelAdvanced, _("Advanced"), false);

  //  Buttons, etc...

  wxBoxSizer* itemBoxSizerBottom = new wxBoxSizer(wxVERTICAL);
  bSizerMain->Add(itemBoxSizerBottom, 0, wxALIGN_LEFT | wxALL | wxEXPAND, 5);

  // wxBoxSizer* itemBoxSizerAux = new wxBoxSizer( wxHORIZONTAL );

  int n_col = 5;

  wxFlexGridSizer* itemBoxSizerAux = new wxFlexGridSizer(0, n_col, 0, 0);
  itemBoxSizerAux->SetFlexibleDirection(wxBOTH);
  itemBoxSizerAux->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  itemBoxSizerBottom->Add(itemBoxSizerAux, 1, wxALIGN_LEFT | wxALL, 5);

#ifndef __ANDROID__

  m_sdbBtmBtnsSizerPrint = new wxButton(this, wxID_ANY, _("Print"),
                                        wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizerAux->Add(m_sdbBtmBtnsSizerPrint, 0,
                       wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_sdbBtmBtnsSizerPrint->Enable(true);
#else
  m_sdbBtmBtnsSizerPrint = NULL;
#endif

  m_sdbBtmBtnsSizerSplit = new wxButton(this, wxID_ANY, _("Split"),
                                        wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizerAux->Add(m_sdbBtmBtnsSizerSplit, 0,
                       wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_sdbBtmBtnsSizerSplit->Enable(false);

  m_sdbBtmBtnsSizerExtend = new wxButton(this, wxID_ANY, _("Extend track"),
                                         wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizerAux->Add(m_sdbBtmBtnsSizerExtend, 0,
                       wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_sdbBtmBtnsSizerToRoute = new wxButton(this, wxID_ANY, _("To route"),
                                          wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizerAux->Add(m_sdbBtmBtnsSizerToRoute, 0,
                       wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_sdbBtmBtnsSizerExport = new wxButton(this, wxID_ANY, _("Export"),
                                         wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizerAux->Add(m_sdbBtmBtnsSizerExport, 0,
                       wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizerBottom->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_sdbBtmBtnsSizerCancel = new wxButton(this, wxID_CANCEL, _("Cancel"),
                                         wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_sdbBtmBtnsSizerCancel, 0, wxALIGN_BOTTOM | wxALL, 5);

  m_sdbBtmBtnsSizerOK =
      new wxButton(this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_sdbBtmBtnsSizerOK, 0, wxALIGN_BOTTOM | wxALL, 5);
  m_sdbBtmBtnsSizerOK->SetDefault();

  // Make it look nice and add the needed non-standard buttons
  int w1, w2, h;
  ((wxWindowBase*)m_stName)->GetSize(&w1, &h);
  ((wxWindowBase*)m_stFrom)->GetSize(&w2, &h);
  ((wxWindowBase*)m_stName)->SetMinSize(wxSize(wxMax(w1, w2), h));
  ((wxWindowBase*)m_stFrom)->SetMinSize(wxSize(wxMax(w1, w2), h));

  m_panelBasic->SetScrollRate(5, 5);
  m_panelAdvanced->SetScrollRate(5, 5);
}

void TrackPropDlg::SetTrackAndUpdate(Track* pt) {
  m_pTrack = pt;

  m_lcPoints->DeleteAllItems();

  if (m_pMyLinkList) delete m_pMyLinkList;
  m_pMyLinkList = new HyperlinkList();

  int NbrOfLinks = m_pTrack->m_TrackHyperlinkList->GetCount();
  if (NbrOfLinks > 0) {
    wxHyperlinkListNode* linknode = m_pTrack->m_TrackHyperlinkList->GetFirst();
    while (linknode) {
      Hyperlink* link = linknode->GetData();

      Hyperlink* h = new Hyperlink();
      h->DescrText = link->DescrText;
      h->Link = link->Link;
      h->LType = link->LType;

      m_pMyLinkList->Append(h);

      linknode = linknode->GetNext();
    }
  }

  InitializeList();
  UpdateProperties();

  RecalculateSize();
}

void TrackPropDlg::InitializeList() {
  if (NULL == m_pTrack) return;

  m_lcPoints->m_pTrack = m_pTrack;

  if (m_pTrack->GetnPoints()) {
    TrackPoint* prp = m_pTrack->GetPoint(0);
    if (prp)
      m_lcPoints->m_LMT_Offset = long((prp->m_lon) * 3600. / 15.);  // estimated
    else
      m_lcPoints->m_LMT_Offset = 0;
  }

  if (m_lcPoints->IsVirtual())
    m_lcPoints->SetItemCount(m_pTrack->GetnPoints());

  else {
    for (int in = 0; in < m_pTrack->GetnPoints(); in++) {
      wxListItem item;
      item.SetId(in);

      m_lcPoints->InsertItem(item);
      for (int j = 0; j < 7; j++) {
        item.SetColumn(j);
        item.SetText(m_lcPoints->OnGetItemText(in, j));
        m_lcPoints->SetItem(item);
      }
    }
  }
}

bool TrackPropDlg::UpdateProperties() {
  if (NULL == m_pTrack) return false;

  ::wxBeginBusyCursor();

  if (m_scrolledWindowLinks) {
    wxWindowList kids = m_scrolledWindowLinks->GetChildren();
    for (unsigned int i = 0; i < kids.GetCount(); i++) {
      wxWindowListNode* node = kids.Item(i);
      wxWindow* win = node->GetData();

      auto link_win = dynamic_cast<wxHyperlinkCtrl*>(win);
      if (link_win) {
        link_win->Disconnect(
            wxEVT_COMMAND_HYPERLINK,
            wxHyperlinkEventHandler(TrackPropDlg::OnHyperLinkClick));
        link_win->Disconnect(
            wxEVT_RIGHT_DOWN,
            wxMouseEventHandler(TrackPropDlg::m_hyperlinkContextMenu));
        win->Destroy();
      }
    }
    ///        m_scrolledWindowLinks->DestroyChildren();
    int NbrOfLinks = m_pTrack->m_TrackHyperlinkList->GetCount();
    HyperlinkList* hyperlinklist = m_pTrack->m_TrackHyperlinkList;
    //            int len = 0;
    if (NbrOfLinks > 0) {
      wxHyperlinkListNode* linknode = hyperlinklist->GetFirst();
      while (linknode) {
        Hyperlink* link = linknode->GetData();
        wxString Link = link->Link;
        wxString Descr = link->DescrText;

        wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl(
            m_scrolledWindowLinks, wxID_ANY, Descr, Link, wxDefaultPosition,
            wxDefaultSize, wxHL_DEFAULT_STYLE);
        ctrl->Connect(wxEVT_COMMAND_HYPERLINK,
                      wxHyperlinkEventHandler(TrackPropDlg::OnHyperLinkClick),
                      NULL, this);
        if (!m_pTrack->m_bIsInLayer)
          ctrl->Connect(
              wxEVT_RIGHT_DOWN,
              wxMouseEventHandler(TrackPropDlg::m_hyperlinkContextMenu), NULL,
              this);

        bSizerLinks->Add(ctrl, 0, wxALL, 5);

        linknode = linknode->GetNext();
      }
    }
    bSizerLinks->Fit(m_scrolledWindowLinks);
  }

  m_tName->SetValue(m_pTrack->GetName());
  m_tFrom->SetValue(m_pTrack->m_TrackStartString);
  m_tTo->SetValue(m_pTrack->m_TrackEndString);
  if (m_tDescription) m_tDescription->SetValue(m_pTrack->m_TrackDescription);

  m_tTotDistance->SetValue(_T(""));
  m_tTimeEnroute->SetValue(_T(""));

  m_sdbBtmBtnsSizerSplit->Enable(false);
  m_sdbBtmBtnsSizerExtend->Enable(false);

  // Calculate AVG speed if we are showing a track and total time
  TrackPoint* last_point = m_pTrack->GetLastPoint();
  TrackPoint* first_point = m_pTrack->GetPoint(0);
  double trackLength = m_pTrack->Length();
  double total_seconds = 0.;

  wxString speed(_T("--"));

  if (last_point && first_point) {
    if (last_point->GetCreateTime().IsValid() &&
        first_point->GetCreateTime().IsValid()) {
      total_seconds = last_point->GetCreateTime()
                          .Subtract(first_point->GetCreateTime())
                          .GetSeconds()
                          .ToDouble();
      if (total_seconds != 0.) {
        m_avgspeed = trackLength / total_seconds * 3600;
      } else {
        m_avgspeed = 0;
      }
      speed.Printf(_T("%5.2f"), toUsrSpeed(m_avgspeed));
    }
  }

  m_tAvgSpeed->SetValue(speed);

  //  Total length
  wxString slen;
  slen.Printf(wxT("%5.2f ") + getUsrDistanceUnit(), toUsrDistance(trackLength));

  m_tTotDistance->SetValue(slen);

  //  Time
  wxString time_form;
  wxTimeSpan time(0, 0, (int)total_seconds, 0);
  // TODO  Construct a readable time string, e.g. "xx Days, 15:34"
  if (total_seconds > 3600. * 24.)
    time_form = time.Format("%H:%M");
  else if (total_seconds > 0.)
    time_form = time.Format("%H:%M");
  else
    time_form = _T("--");
  m_tTimeEnroute->SetValue(time_form);

  m_cbShow->SetValue(m_pTrack->IsVisible());

  if (m_pTrack->m_Colour == wxEmptyString)
    m_cColor->Select(0);
  else {
    for (unsigned int i = 0; i < sizeof(::GpxxColorNames) / sizeof(wxString);
         i++) {
      if (m_pTrack->m_Colour == ::GpxxColorNames[i]) {
        m_cColor->Select(i + 1);
        break;
      }
    }
  }

  for (unsigned int i = 0; i < sizeof(::StyleValues) / sizeof(int); i++) {
    if (m_pTrack->m_style == ::StyleValues[i]) {
      m_cStyle->Select(i);
      break;
    }
  }

  for (unsigned int i = 0; i < sizeof(::WidthValues) / sizeof(int); i++) {
    if (m_pTrack->m_width == ::WidthValues[i]) {
      m_cWidth->Select(i);
      break;
    }
  }

  if (m_pTrack->m_bIsInLayer) {
    m_tName->SetEditable(false);
    m_tFrom->SetEditable(false);
    m_tTo->SetEditable(false);
    if (m_tDescription) m_tDescription->SetEditable(false);
    m_cbShow->Enable(false);
    m_cColor->Enable(false);
    m_cStyle->Enable(false);
    m_cWidth->Enable(false);
    m_sdbBtmBtnsSizerExtend->Enable(false);
    m_sdbBtmBtnsSizerSplit->Enable(false);
    SetTitle(wxString::Format(_T("%s, %s: %d"), _("Track properties"),
                              _T("Layer"), m_pTrack->m_LayerID));
  } else {
    m_tName->SetEditable(true);
    m_tFrom->SetEditable(true);
    m_tTo->SetEditable(true);
    if (m_tDescription) m_tDescription->SetEditable(true);
    m_cbShow->Enable(true);
    m_cColor->Enable(true);
    m_cStyle->Enable(true);
    m_cWidth->Enable(true);

    m_sdbBtmBtnsSizerExtend->Enable(IsThisTrackExtendable());
    // m_sdbBtmBtnsSizerSplit->Enable( false );
    SetTitle(_("Track properties"));
  }

  ::wxEndBusyCursor();

  return true;
}

bool TrackPropDlg::IsThisTrackExtendable() {
  m_pExtendTrack = NULL;
  m_pExtendPoint = NULL;
  if (m_pTrack == g_pActiveTrack || m_pTrack->m_bIsInLayer) {
    return false;
  }

  TrackPoint* pLastPoint = m_pTrack->GetPoint(0);
  if (!pLastPoint->GetCreateTime().IsValid()) {
    return false;
  }

  for (Track* ptrack : g_TrackList) {
    if (ptrack->IsVisible() && (ptrack->m_GUID != m_pTrack->m_GUID)) {
      TrackPoint* track_node = ptrack->GetLastPoint();
      if (track_node) {
        if (track_node->GetCreateTime().IsValid()) {
          if (track_node->GetCreateTime() <= pLastPoint->GetCreateTime()) {
            if (!m_pExtendPoint ||
                track_node->GetCreateTime() > m_pExtendPoint->GetCreateTime()) {
              m_pExtendPoint = track_node;
              m_pExtendTrack = ptrack;
            }
          }
        }
      }
    }
  }
  if (m_pExtendTrack) {
    return (!m_pExtendTrack->m_bIsInLayer);
  } else {
    return false;
  }
}

void TrackPropDlg::OnExtendBtnClick(wxCommandEvent& event) {
  TrackPoint* pFirstPoint = m_pTrack->GetPoint(0);

  if (IsThisTrackExtendable()) {
    int begin = 0;
    if (pFirstPoint->GetCreateTime() == m_pExtendPoint->GetCreateTime()) {
      begin = 1;
    }
    pSelect->DeleteAllSelectableTrackSegments(m_pExtendTrack);
    m_pExtendTrack->Clone(m_pTrack, begin, m_pTrack->GetnPoints(), _("_plus"));
    pSelect->AddAllSelectableTrackSegments(m_pExtendTrack);
    pSelect->DeleteAllSelectableTrackSegments(m_pTrack);
    RoutemanGui(*g_pRouteMan).DeleteTrack(m_pTrack);

    SetTrackAndUpdate(m_pExtendTrack);
    UpdateProperties();

    if (pRouteManagerDialog && pRouteManagerDialog->IsShown()) {
      pRouteManagerDialog->UpdateTrkListCtrl();
    }
  }
}

void TrackPropDlg::OnSplitBtnClick(wxCommandEvent& event) {
  m_sdbBtmBtnsSizerSplit->Enable(false);

  if (m_pTrack->m_bIsInLayer) {
    return;
  }

  if ((m_nSelected > 1) && (m_nSelected < m_pTrack->GetnPoints())) {
    Track* pHead = new Track();
    Track* pTail = new Track();
    pHead->Clone(m_pTrack, 0, m_nSelected - 1, _("_A"));
    pTail->Clone(m_pTrack, m_nSelected - 1, m_pTrack->GetnPoints(), _("_B"));

    g_TrackList.push_back(pHead);
    NavObj_dB::GetInstance().InsertTrack(pHead);

    g_TrackList.push_back(pTail);
    NavObj_dB::GetInstance().InsertTrack(pTail);

    NavObj_dB::GetInstance().DeleteTrack(m_pTrack);

    pSelect->DeleteAllSelectableTrackSegments(m_pTrack);
    RoutemanGui(*g_pRouteMan).DeleteTrack(m_pTrack);
    pSelect->AddAllSelectableTrackSegments(pTail);
    pSelect->AddAllSelectableTrackSegments(pHead);

    SetTrackAndUpdate(pTail);
    UpdateProperties();

    if (pRouteManagerDialog && pRouteManagerDialog->IsShown()) {
      pRouteManagerDialog->UpdateTrkListCtrl();
    }
  }
}

void TrackPropDlg::OnTrackPropCopyTxtClick(wxCommandEvent& event) {
  wxString tab("\t", wxConvUTF8);
  wxString eol("\n", wxConvUTF8);
  wxString csvString;

  csvString << this->GetTitle() << eol << _("Name") << tab
            << m_pTrack->GetName() << eol << _("Depart From") << tab
            << m_pTrack->m_TrackStartString << eol << _("Destination") << tab
            << m_pTrack->m_TrackEndString << eol << _("Total distance") << tab
            << m_tTotDistance->GetValue() << eol << _("Speed") << tab
            << m_tAvgSpeed->GetValue() << eol
            << _("Departure Time") + _T(" ") + _("(m/d/y h:m)") << tab
            << m_pTrack->GetPoint(1)->GetCreateTime().Format() << eol
            << _("Time enroute") << tab << m_tTimeEnroute->GetValue() << eol
            << eol;

  int noCols;
  int noRows;
  noCols = m_lcPoints->GetColumnCount();
  noRows = m_lcPoints->GetItemCount();
  wxListItem item;
  item.SetMask(wxLIST_MASK_TEXT);

  for (int i = 0; i < noCols; i++) {
    m_lcPoints->GetColumn(i, item);
    csvString << item.GetText() << tab;
  }
  csvString << eol;

  for (int j = 0; j < noRows; j++) {
    item.SetId(j);
    for (int i = 0; i < noCols; i++) {
      item.SetColumn(i);
      m_lcPoints->GetItem(item);
      csvString << item.GetText() << tab;
    }
    csvString << eol;
  }

  if (wxTheClipboard->Open()) {
    wxTextDataObject* data = new wxTextDataObject;
    data->SetText(csvString);
    wxTheClipboard->SetData(data);
    wxTheClipboard->Close();
  }
}

void TrackPropDlg::OnPrintBtnClick(wxCommandEvent& event) {
  static std::set<int> s_options;  // keep selected options
  TrackPrintDialog dlg(this, s_options);
  int result = dlg.ShowModal();

  if (result == wxID_OK) {
    dlg.GetSelected(s_options);
    TrackPrintout printout(m_pTrack, m_lcPoints, s_options);
    auto& printer = PrintDialog::GetInstance();
    printer.Initialize(wxPORTRAIT);
    printer.EnablePageNumbers(true);
    printer.Print(this, &printout);
  }
}

void TrackPropDlg::OnTrackPropRightClick(wxListEvent& event) {
  wxMenu menu;

  wxMenuItem* copyItem =
      menu.Append(ID_RCLK_MENU_COPY_TEXT, _("&Copy all as text"));

  PopupMenu(&menu);
}

void TrackPropDlg::OnTrackPropListClick(wxListEvent& event) {
  long itemno = -1;
  m_nSelected = 0;

  int selected_no;
  itemno =
      m_lcPoints->GetNextItem(itemno, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (itemno == -1)
    selected_no = 0;
  else
    selected_no = itemno;

  m_pTrack->m_HighlightedTrackPoint = -1;

  if (itemno >= 0) {
    TrackPoint* prp = m_pTrack->GetPoint(itemno);
    if (prp) {
      m_pTrack->m_HighlightedTrackPoint = itemno;  // highlight the trackpoint

      if (!(m_pTrack->m_bIsInLayer) && !(m_pTrack == g_pActiveTrack)) {
        m_nSelected = selected_no + 1;
        m_sdbBtmBtnsSizerSplit->Enable(true);
      }
      if (gFrame->GetFocusCanvas()) {
        gFrame->JumpToPosition(gFrame->GetFocusCanvas(), prp->m_lat, prp->m_lon,
                               gFrame->GetFocusCanvas()->GetVPScale());
      }
#ifdef __WXMSW__
      if (m_lcPoints) m_lcPoints->SetFocus();
#endif
    }
  }
  if (selected_no == 0 || selected_no == m_pTrack->GetnPoints() - 1)
    m_sdbBtmBtnsSizerSplit->Enable(false);
}

void TrackPropDlg::OnTrackPropMenuSelected(wxCommandEvent& event) {
  switch (event.GetId()) {
    case ID_RCLK_MENU_COPY_TEXT:
      OnTrackPropCopyTxtClick(event);
      break;
  }
}

void TrackPropDlg::OnToRouteBtnClick(wxCommandEvent& event) {
  pRouteManagerDialog->TrackToRoute(m_pTrack);
  if (NULL != pRouteManagerDialog && pRouteManagerDialog->IsVisible())
    pRouteManagerDialog->UpdateRouteListCtrl();
}

void TrackPropDlg::OnExportBtnClick(wxCommandEvent& event) {
  wxString suggested_name = _("track");
  std::vector<Track*> list = {m_pTrack};
  if (m_pTrack->GetName() != wxEmptyString)
    suggested_name = m_pTrack->GetName();
  ExportGPXTracks(this, &list, suggested_name);
}

void TrackPropDlg::m_hyperlinkContextMenu(wxMouseEvent& event) {
  m_pEditedLink = (wxHyperlinkCtrl*)event.GetEventObject();
  Connect(
      wxEVT_COMMAND_MENU_SELECTED,
      (wxObjectEventFunction)(wxEventFunction)&TrackPropDlg::PopupMenuHandler);
  m_scrolledWindowLinks->PopupMenu(
      m_menuLink, m_pEditedLink->GetPosition().x + event.GetPosition().x,
      m_pEditedLink->GetPosition().y + event.GetPosition().y);
}

void TrackPropDlg::PopupMenuHandler(wxCommandEvent& event) {
  switch (event.GetId()) {
    case ID_TRK_MENU_ADD:
      OnAddLink(event);
      break;
    case ID_TRK_MENU_EDIT:
      OnEditLink(event);
      break;
    case ID_TRK_MENU_DELETE:
      OnDeleteLink(event);
      break;
    default:
      break;
  }
}

void TrackPropDlg::OnDeleteLink(wxCommandEvent& event) {
  wxHyperlinkListNode* nodeToDelete = NULL;
  wxString findurl = m_pEditedLink->GetURL();
  wxString findlabel = m_pEditedLink->GetLabel();

  wxWindowList kids = m_scrolledWindowLinks->GetChildren();
  for (unsigned int i = 0; i < kids.GetCount(); i++) {
    wxWindowListNode* node = kids.Item(i);
    wxWindow* win = node->GetData();

    auto link_win = dynamic_cast<wxHyperlinkCtrl*>(win);
    if (link_win) {
      link_win->Disconnect(
          wxEVT_COMMAND_HYPERLINK,
          wxHyperlinkEventHandler(TrackPropDlg::OnHyperLinkClick));
      link_win->Disconnect(
          wxEVT_RIGHT_DOWN,
          wxMouseEventHandler(TrackPropDlg::m_hyperlinkContextMenu));
      win->Destroy();
    }
  }

  ///    m_scrolledWindowLinks->DestroyChildren();
  int NbrOfLinks = m_pTrack->m_TrackHyperlinkList->GetCount();
  HyperlinkList* hyperlinklist = m_pTrack->m_TrackHyperlinkList;
  //      int len = 0;
  if (NbrOfLinks > 0) {
    wxHyperlinkListNode* linknode = hyperlinklist->GetFirst();
    while (linknode) {
      Hyperlink* link = linknode->GetData();
      wxString Link = link->Link;
      wxString Descr = link->DescrText;
      if (Link == findurl &&
          (Descr == findlabel || (Link == findlabel && Descr == wxEmptyString)))
        nodeToDelete = linknode;
      else {
        wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl(
            m_scrolledWindowLinks, wxID_ANY, Descr, Link, wxDefaultPosition,
            wxDefaultSize, wxHL_DEFAULT_STYLE);
        ctrl->Connect(wxEVT_COMMAND_HYPERLINK,
                      wxHyperlinkEventHandler(TrackPropDlg::OnHyperLinkClick),
                      NULL, this);
        ctrl->Connect(wxEVT_RIGHT_DOWN,
                      wxMouseEventHandler(TrackPropDlg::m_hyperlinkContextMenu),
                      NULL, this);

        bSizerLinks->Add(ctrl, 0, wxALL, 5);
      }
      linknode = linknode->GetNext();
    }
  }
  if (nodeToDelete) hyperlinklist->DeleteNode(nodeToDelete);
  m_scrolledWindowLinks->InvalidateBestSize();
  m_scrolledWindowLinks->Layout();
  sbSizerLinks->Layout();
  // event.Skip();
}

void TrackPropDlg::OnEditLink(wxCommandEvent& event) {
  wxString findurl = m_pEditedLink->GetURL();
  wxString findlabel = m_pEditedLink->GetLabel();

  LinkPropImpl* LinkPropDlg = new LinkPropImpl(this);
  LinkPropDlg->m_textCtrlLinkDescription->SetValue(findlabel);
  LinkPropDlg->m_textCtrlLinkUrl->SetValue(findurl);
  DimeControl(LinkPropDlg);
  LinkPropDlg->ShowWindowModalThenDo([this, LinkPropDlg, findurl,
                                      findlabel](int retcode) {
    if (retcode == wxID_OK) {
      int NbrOfLinks = m_pTrack->m_TrackHyperlinkList->GetCount();
      HyperlinkList* hyperlinklist = m_pTrack->m_TrackHyperlinkList;
      //            int len = 0;
      if (NbrOfLinks > 0) {
        wxHyperlinkListNode* linknode = hyperlinklist->GetFirst();
        while (linknode) {
          Hyperlink* link = linknode->GetData();
          wxString Link = link->Link;
          wxString Descr = link->DescrText;
          if (Link == findurl &&
              (Descr == findlabel ||
               (Link == findlabel && Descr == wxEmptyString))) {
            link->Link = LinkPropDlg->m_textCtrlLinkUrl->GetValue();
            link->DescrText =
                LinkPropDlg->m_textCtrlLinkDescription->GetValue();
            wxHyperlinkCtrl* h =
                (wxHyperlinkCtrl*)m_scrolledWindowLinks->FindWindowByLabel(
                    findlabel);
            if (h) {
              h->SetLabel(LinkPropDlg->m_textCtrlLinkDescription->GetValue());
              h->SetURL(LinkPropDlg->m_textCtrlLinkUrl->GetValue());
            }
          }
          linknode = linknode->GetNext();
        }
      }

      m_scrolledWindowLinks->InvalidateBestSize();
      m_scrolledWindowLinks->Layout();
      sbSizerLinks->Layout();
    }
  });
  // event.Skip();
}

void TrackPropDlg::OnAddLink(wxCommandEvent& event) {
  LinkPropImpl* LinkPropDlg = new LinkPropImpl(this);
  LinkPropDlg->m_textCtrlLinkDescription->SetValue(wxEmptyString);
  LinkPropDlg->m_textCtrlLinkUrl->SetValue(wxEmptyString);
  DimeControl(LinkPropDlg);
  LinkPropDlg->ShowWindowModalThenDo([this, LinkPropDlg](int retcode) {
    if (retcode == wxID_OK) {
      wxString desc = LinkPropDlg->m_textCtrlLinkDescription->GetValue();
      if (desc == wxEmptyString)
        desc = LinkPropDlg->m_textCtrlLinkUrl->GetValue();
      wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl(
          m_scrolledWindowLinks, wxID_ANY, desc,
          LinkPropDlg->m_textCtrlLinkUrl->GetValue(), wxDefaultPosition,
          wxDefaultSize, wxHL_DEFAULT_STYLE);
      ctrl->Connect(wxEVT_COMMAND_HYPERLINK,
                    wxHyperlinkEventHandler(TrackPropDlg::OnHyperLinkClick),
                    NULL, this);
      ctrl->Connect(wxEVT_RIGHT_DOWN,
                    wxMouseEventHandler(TrackPropDlg::m_hyperlinkContextMenu),
                    NULL, this);

      bSizerLinks->Add(ctrl, 0, wxALL, 5);
      bSizerLinks->Fit(m_scrolledWindowLinks);
      // this->Fit();

      Hyperlink* h = new Hyperlink();
      h->DescrText = LinkPropDlg->m_textCtrlLinkDescription->GetValue();
      h->Link = LinkPropDlg->m_textCtrlLinkUrl->GetValue();
      h->LType = wxEmptyString;
      m_pTrack->m_TrackHyperlinkList->Append(h);
    }
  });
  //    sbSizerLinks->Layout();

  m_scrolledWindowLinks->InvalidateBestSize();
  m_scrolledWindowLinks->Layout();
  sbSizerLinks->Layout();

  // event.Skip();
}

void TrackPropDlg::OnEditLinkToggle(wxCommandEvent& event) {
  if (m_toggleBtnEdit->GetValue())
    m_staticTextEditEnabled->SetLabel(_("Links are opened for editing."));
  else
    m_staticTextEditEnabled->SetLabel(
        _("Links are opened in the default browser."));
  event.Skip();
}

void TrackPropDlg::OnHyperLinkClick(wxHyperlinkEvent& event) {
  if (m_toggleBtnEdit->GetValue()) {
    m_pEditedLink = (wxHyperlinkCtrl*)event.GetEventObject();
    OnEditLink(event);
    event.Skip(false);
    return;
  }
  //    Windows has trouble handling local file URLs with embedded anchor
  //    points, e.g file://testfile.html#point1 The trouble is with the
  //    wxLaunchDefaultBrowser with verb "open" Workaround is to probe the
  //    registry to get the default browser, and open directly
  //
  //    But, we will do this only if the URL contains the anchor point charater
  //    '#' What a hack......

#ifdef __WXMSW__

  wxString cc = event.GetURL();
  if (cc.Find(_T("#")) != wxNOT_FOUND) {
    wxRegKey RegKey(
        wxString(_T("HKEY_CLASSES_ROOT\\HTTP\\shell\\open\\command")));
    if (RegKey.Exists()) {
      wxString command_line;
      RegKey.QueryValue(wxString(_T("")), command_line);

      //  Remove "
      command_line.Replace(wxString(_T("\"")), wxString(_T("")));

      //  Strip arguments
      int l = command_line.Find(_T(".exe"));
      if (wxNOT_FOUND == l) l = command_line.Find(_T(".EXE"));

      if (wxNOT_FOUND != l) {
        wxString cl = command_line.Mid(0, l + 4);
        cl += _T(" ");
        cc.Prepend(_T("\""));
        cc.Append(_T("\""));
        cl += cc;
        wxExecute(cl);  // Async, so Fire and Forget...
      }
    }
  } else
    event.Skip();
#else
  wxString url = event.GetURL();
  url.Replace(_T(" "), _T("%20"));
  ::wxLaunchDefaultBrowser(url);
//    event.Skip();
#endif
}

void TrackPropDlg::OnShowTimeTZ(wxCommandEvent& event) {
  if (m_rbShowTimeUTC && m_rbShowTimeUTC->GetValue())
    m_lcPoints->m_tz_selection = UTCINPUT;
  else if (m_rbShowTimePC && m_rbShowTimePC->GetValue())
    m_lcPoints->m_tz_selection = LTINPUT;
  else if (m_rbShowTimeLocal && m_rbShowTimeLocal->GetValue())
    m_lcPoints->m_tz_selection = LMTINPUT;
  else if (m_rbShowTimeGlobalSettings && m_rbShowTimeGlobalSettings->GetValue())
    m_lcPoints->m_tz_selection = GLOBAL_SETTINGS_INPUT;
  else
    throw std::logic_error("Unexpected time zone selection");
  m_lcPoints->DeleteAllItems();
  InitializeList();
}

bool TrackPropDlg::SaveChanges(void) {
  if (m_pTrack && !m_pTrack->m_bIsInLayer) {
    //  Get User input Text Fields
    m_pTrack->SetName(m_tName->GetValue());
    m_pTrack->m_TrackStartString = m_tFrom->GetValue();
    m_pTrack->m_TrackEndString = m_tTo->GetValue();
    if (m_tDescription)
      m_pTrack->m_TrackDescription = m_tDescription->GetValue();
    m_pTrack->SetVisible(m_cbShow->GetValue());
    if (m_cColor->GetSelection() == 0)
      m_pTrack->m_Colour = wxEmptyString;
    else
      m_pTrack->m_Colour = ::GpxxColorNames[m_cColor->GetSelection() - 1];
    m_pTrack->m_style = (wxPenStyle)::StyleValues[m_cStyle->GetSelection()];
    m_pTrack->m_width = ::WidthValues[m_cWidth->GetSelection()];

    NavObj_dB::GetInstance().UpdateDBTrackAttributes(m_pTrack);

    pConfig->UpdateSettings();
  }

  if (m_pTrack && m_pTrack->IsRunning()) {
    wxJSONValue v;
    v[_T("Changed")] = true;
    v[_T("Name")] = m_pTrack->GetName();
    v[_T("GUID")] = m_pTrack->m_GUID;
    wxString msg_id(_T("OCPN_TRK_ACTIVATED"));
    SendJSONMessageToAllPlugins(msg_id, v);
  }

  return true;
}

void TrackPropDlg::OnOKBtnClick(wxCommandEvent& event) {
  //    Look in the track list to be sure the track is still available
  //    (May have been deleted by RouteManagerDialog...)

  bool b_found_track = std::find(g_TrackList.begin(), g_TrackList.end(),
                                 m_pTrack) != g_TrackList.end();

  if (b_found_track) {
    SaveChanges();  // write changes to globals and update config
    m_pTrack->ClearHighlights();
  }

  m_bStartNow = false;

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateTrkListCtrl();

  Hide();
  gFrame->InvalidateAllGL();
  gFrame->RefreshAllCanvas(false);

  event.Skip();
}

void TrackPropDlg::OnCancelBtnClick(wxCommandEvent& event) {
  bool b_found_track = std::find(g_TrackList.begin(), g_TrackList.end(),
                                 m_pTrack) != g_TrackList.end();

  if (b_found_track) m_pTrack->ClearHighlights();

  Hide();
  gFrame->InvalidateAllGL();
  gFrame->RefreshAllCanvas(false);

  event.Skip();
}

//--------------------------------------------------------------------------------------
//          OCPNTrackListCtrl Implementation
//---------------------------------------------------------------------------------------

OCPNTrackListCtrl::OCPNTrackListCtrl(wxWindow* parent, wxWindowID id,
                                     const wxPoint& pos, const wxSize& size,
                                     long style)
    : wxListCtrl(parent, id, pos, size, style) {
  m_parent = parent;
  m_tz_selection = GLOBAL_SETTINGS_INPUT;
  m_LMT_Offset = 0;
}

OCPNTrackListCtrl::~OCPNTrackListCtrl() {}

double OCPNTrackListCtrl::getStartPointLongitude() const {
  if (m_pTrack->GetnPoints()) {
    TrackPoint* prp = m_pTrack->GetPoint(0);
    if (prp) return prp->m_lon;
  }
  return NAN;
}

wxString OCPNTrackListCtrl::OnGetItemText(long item, long column) const {
  wxString ret;

  if (item < 0 || item >= m_pTrack->GetnPoints()) return wxEmptyString;

  TrackPoint* this_point = m_pTrack->GetPoint(item);
  TrackPoint* prev_point = item > 0 ? m_pTrack->GetPoint(item - 1) : NULL;

  if (!this_point) return wxEmptyString;

  double gt_brg, gt_leg_dist;
  double slat, slon;
  if (item == 0) {
    slat = gLat;
    slon = gLon;
  } else {
    slat = prev_point->m_lat;
    slon = prev_point->m_lon;
  }

  switch (column) {
    case 0:
      if (item == 0)
        ret = _T("---");
      else
        ret.Printf(_T("%ld"), item);
      break;

    case 1:
      DistanceBearingMercator(this_point->m_lat, this_point->m_lon, slat, slon,
                              &gt_brg, &gt_leg_dist);

      ret.Printf(_T("%6.2f ") + getUsrDistanceUnit(),
                 toUsrDistance(gt_leg_dist));
      break;

    case 2:
      DistanceBearingMercator(this_point->m_lat, this_point->m_lon, slat, slon,
                              &gt_brg, &gt_leg_dist);
      ret.Printf("%03.0f %cT", gt_brg, 0x00B0);
      break;

    case 3:
      ret = toSDMM(1, this_point->m_lat, 1);
      break;

    case 4:
      ret = toSDMM(2, this_point->m_lon, 1);
      break;

    case 5: {
      wxDateTime timestamp = this_point->GetCreateTime();
      if (timestamp.IsValid()) {
        DateTimeFormatOptions opts =
            DateTimeFormatOptions()
                .SetTimezone(getDatetimeTimezoneSelector(m_tz_selection))
                .SetLongitude(getStartPointLongitude());
        ret = ocpn::toUsrDateTimeFormat(timestamp.FromUTC(), opts);
      } else
        ret = _T("----");
    } break;

    case 6:
      if ((item > 0) && this_point->GetCreateTime().IsValid() &&
          prev_point->GetCreateTime().IsValid()) {
        DistanceBearingMercator(this_point->m_lat, this_point->m_lon, slat,
                                slon, &gt_brg, &gt_leg_dist);
        double speed = 0.;
        double seconds = this_point->GetCreateTime()
                             .Subtract(prev_point->GetCreateTime())
                             .GetSeconds()
                             .ToDouble();

        if (seconds > 0.) speed = gt_leg_dist / seconds * 3600;

        ret.Printf(_T("%5.2f"), toUsrSpeed(speed));
      } else
        ret = _("--");
      break;

    default:
      break;
  }

  return ret;
}

int OCPNTrackListCtrl::OnGetItemColumnImage(long item, long column) const {
  return -1;
}
