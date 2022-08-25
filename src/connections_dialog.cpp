/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
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
 */

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "wx/tokenzr.h"
#include <wx/regex.h>

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#include "qdebug.h"
#endif

#include "connections_dialog.h"
#include "NMEALogWindow.h"
#include "OCPNPlatform.h"
#include "ocpn_plugin.h"    // FIXME for GetOCPNScaledFont_PlugIn
#include "options.h"
#include "udev_rule_mgr.h"
#include "comm_drv_factory.h"
#include "gui_lib.h"
#include "nmea0183.h"

extern bool g_bMagneticAPB;
extern bool g_bGarminHostUpload;
extern wxString g_GPS_Ident;
extern bool g_bfilter_cogsog;
extern int g_COGFilterSec;
extern int g_SOGFilterSec;
extern int g_NMEAAPBPrecision;
extern wxString g_TalkerIdText;
extern wxArrayOfConnPrm* g_pConnectionParams;
extern OCPNPlatform* g_Platform;

wxString StringArrayToString(wxArrayString arr) {
  wxString ret = wxEmptyString;
  for (size_t i = 0; i < arr.Count(); i++) {
    if (i > 0) ret.Append(_T(","));
    ret.Append(arr[i]);
  }
  return ret;
}

//------------------------------------------------------------------------------
//          ConnectionsDialog Implementation
//------------------------------------------------------------------------------

// Define constructors
ConnectionsDialog::ConnectionsDialog() {}

ConnectionsDialog::ConnectionsDialog(wxScrolledWindow *container, options *parent){
  m_container = container;
  m_parent = parent;

  Init();
}



ConnectionsDialog::~ConnectionsDialog() {
}

//FIXME (dave)  need to call this after page is setup
void ConnectionsDialog::SetInitialSettings(void) {
  m_TalkerIdText->SetValue(g_TalkerIdText.MakeUpper());

  m_cbNMEADebug->SetValue(false);
  if (NMEALogWindow::Get().GetTTYWindow()) {
    if (NMEALogWindow::Get().GetTTYWindow()->IsShown()) {
      m_cbNMEADebug->SetValue(true);
    }
  }

  //FIXME must do this
//     if (m_pSerialArray) {
//     m_comboPort->Clear();
//     for (size_t i = 0; i < m_pSerialArray->Count(); i++) {
//       m_comboPort->Append(m_pSerialArray->Item(i));
//     }
//   }

  //  On some platforms, the global connections list may be changed outside of
  //  the options dialog. Pick up any changes here, and re-populate the dialog
  //  list.
//   FillSourceList();
//
//   //  Reset the touch flag...
//   connectionsaved = true;
//
//   SetSelectedConnectionPanel(nullptr);


}

void ConnectionsDialog::RecalculateSize(void) {
#if 0
//FIXME  Implement this
  if (!g_bresponsive) {
    m_nCharWidthMax = GetSize().x / GetCharWidth();
    return;
  }

  wxSize esize;
  esize.x = GetCharWidth() * 110;
  esize.y = GetCharHeight() * 40;

  wxSize dsize = GetParent()->GetSize();  // GetClientSize();
  esize.y = wxMin(esize.y, dsize.y - 0 /*(2 * GetCharHeight())*/);
  esize.x = wxMin(esize.x, dsize.x - 0 /*(2 * GetCharHeight())*/);
  SetSize(esize);

  wxSize fsize = GetSize();
  wxSize canvas_size = GetParent()->GetSize();
  wxPoint screen_pos = GetParent()->GetScreenPosition();
  int xp = (canvas_size.x - fsize.x) / 2;
  int yp = (canvas_size.y - fsize.y) / 2;
  Move(screen_pos.x + xp, screen_pos.y + yp);

  m_nCharWidthMax = GetSize().x / GetCharWidth();
#endif
}

void ConnectionsDialog::Init(){

  // Setup some inital values
  m_buttonScanBT = 0;
  m_stBTPairs = 0;
  m_choiceBTDataSources = 0;

  //Create the UI
  int group_item_spacing = 2;

  wxBoxSizer* bSizer4 = new wxBoxSizer(wxVERTICAL);
  m_container->SetSizer(bSizer4);
  m_container->SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizerOuterContainer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer* sbSizerGeneral;
  sbSizerGeneral = new wxStaticBoxSizer(
      new wxStaticBox(m_container, wxID_ANY, _("General")), wxVERTICAL);

  wxBoxSizer* bSizer151;
  bSizer151 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer161;
  bSizer161 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer171;
  bSizer171 = new wxBoxSizer(wxHORIZONTAL);

  m_cbFilterSogCog = new wxCheckBox(m_container, wxID_ANY,
                                    _("Filter NMEA Course and Speed data"),
                                    wxDefaultPosition, wxDefaultSize, 0);
  m_cbFilterSogCog->SetValue(g_bfilter_cogsog);
  bSizer171->Add(m_cbFilterSogCog, 0, wxALL, 5);

  m_stFilterSec =
      new wxStaticText(m_container, wxID_ANY, _("Filter period (sec)"),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_stFilterSec->Wrap(-1);

  int nspace = 5;
#ifdef __WXGTK__
  nspace = 9;
#endif
  bSizer171->Add(m_stFilterSec, 0, wxALL, nspace);

  m_tFilterSec = new wxTextCtrl(m_container, wxID_ANY, wxEmptyString,
                                wxDefaultPosition, wxDefaultSize, 0);
  wxString sfilt;
  sfilt.Printf(_T("%d"), g_COGFilterSec);
  m_tFilterSec->SetValue(sfilt);
  bSizer171->Add(m_tFilterSec, 0, wxALL, 4);
  bSizer161->Add(bSizer171, 1, wxEXPAND, 5);

  int cb_space = 2;
  m_cbNMEADebug =
      new wxCheckBox(m_container, wxID_ANY, _("Show NMEA Debug Window"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbNMEADebug->SetValue(NMEALogWindow::Get().Active());
  bSizer161->Add(m_cbNMEADebug, 0, wxALL, cb_space);

  m_cbFurunoGP3X =
      new wxCheckBox(m_container, wxID_ANY, _("Format uploads for Furuno GP3X"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbFurunoGP3X->SetValue(g_GPS_Ident == _T( "FurunoGP3X" ));
  bSizer161->Add(m_cbFurunoGP3X, 0, wxALL, cb_space);

  m_cbGarminUploadHost = new wxCheckBox(
      m_container, wxID_ANY, _("Use Garmin GRMN (Host) mode for uploads"),
      wxDefaultPosition, wxDefaultSize, 0);
  m_cbGarminUploadHost->SetValue(g_bGarminHostUpload);
  bSizer161->Add(m_cbGarminUploadHost, 0, wxALL, cb_space);

  m_cbAPBMagnetic =
      new wxCheckBox(m_container, wxID_ANY,
                     _("Use magnetic bearings in output sentence ECAPB"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbAPBMagnetic->SetValue(g_bMagneticAPB);
  bSizer161->Add(m_cbAPBMagnetic, 0, wxALL, cb_space);

  bSizer151->Add(bSizer161, 1, wxEXPAND, 5);
  sbSizerGeneral->Add(bSizer151, 1, wxEXPAND, 5);
  bSizerOuterContainer->Add(sbSizerGeneral, 0, wxALL | wxEXPAND, 5);

#if 1
  //  Connections listbox, etc
  wxStaticBoxSizer* sbSizerLB = new wxStaticBoxSizer(
      new wxStaticBox(m_container, wxID_ANY, _("Data Connections")),
      wxVERTICAL);

  /*
    wxBoxSizer* bSizer17;
    bSizer17 = new wxBoxSizer(wxVERTICAL);

    m_lcSources = new wxListCtrl(m_pNMEAForm, wxID_ANY, wxDefaultPosition,
                                 wxSize(-1, 150), wxLC_REPORT |
    wxLC_SINGLE_SEL); bSizer17->Add(m_lcSources, 1, wxALL | wxEXPAND, 5);
  */

  wxPanel* cPanel =
      new wxPanel(m_container, wxID_ANY, wxDefaultPosition,
                  wxDLG_UNIT(m_parent, wxSize(-1, -1)), wxBG_STYLE_ERASE);
  sbSizerLB->Add(cPanel, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer* boxSizercPanel = new wxBoxSizer(wxVERTICAL);
  cPanel->SetSizer(boxSizercPanel);

  m_scrollWinConnections = new wxScrolledWindow(
      cPanel, wxID_ANY, wxDefaultPosition, wxDLG_UNIT(m_parent, wxSize(-1, 80)),
      wxBORDER_RAISED | wxVSCROLL | wxBG_STYLE_ERASE);
  m_scrollWinConnections->SetScrollRate(5, 5);
  boxSizercPanel->Add(m_scrollWinConnections, 0, wxALL | wxEXPAND, 5);

  boxSizerConnections = new wxBoxSizer(wxVERTICAL);
  m_scrollWinConnections->SetSizer(boxSizerConnections);

  bSizerOuterContainer->Add(sbSizerLB, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer18;
  bSizer18 = new wxBoxSizer(wxHORIZONTAL);
  sbSizerLB->Add(bSizer18, 1, wxEXPAND, 5);

  m_buttonAdd = new wxButton(m_container, wxID_ANY, _("Add Connection"),
                             wxDefaultPosition, wxDefaultSize, 0);
  bSizer18->Add(m_buttonAdd, 0, wxALL, 5);

  m_buttonRemove = new wxButton(m_container, wxID_ANY, _("Remove Connection"),
                                wxDefaultPosition, wxDefaultSize, 0);
  m_buttonRemove->Enable(FALSE);
  bSizer18->Add(m_buttonRemove, 0, wxALL, 5);

  //   wxBoxSizer* bSizer19 = new wxBoxSizer(wxHORIZONTAL);
  //   sbSizerLB->Add(bSizer19, 1, wxEXPAND, 5);
  //
  wxFont* dFont = GetOCPNScaledFont_PlugIn(_("Dialog"));
  double font_size = dFont->GetPointSize() * 17 / 16;
  wxFont* bFont = wxTheFontList->FindOrCreateFont(
      font_size, dFont->GetFamily(), dFont->GetStyle(), wxFONTWEIGHT_BOLD);
  //
  //   m_stEditCon = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Edit Selected
  //   Connection")); m_stEditCon->SetFont(*bFont); bSizer19->Add(m_stEditCon,
  //   0, wxALL | wxEXPAND | wxALIGN_CENTER_HORIZONTAL, 5);
  //

  //  Connections Properties
  m_sbConnEdit =
      new wxStaticBox(m_container, wxID_ANY, _("Edit Selected Connection"));
  m_sbConnEdit->SetFont(*bFont);

  sbSizerConnectionProps = new wxStaticBoxSizer(m_sbConnEdit, wxVERTICAL);

  wxBoxSizer* bSizer15;
  bSizer15 = new wxBoxSizer(wxHORIZONTAL);

  sbSizerConnectionProps->Add(bSizer15, 0, wxTOP | wxEXPAND, 5);

  m_rbTypeSerial =
      new wxRadioButton(m_container, wxID_ANY, _("Serial"), wxDefaultPosition,
                        wxDefaultSize, wxRB_GROUP);
  m_rbTypeSerial->SetValue(TRUE);
  bSizer15->Add(m_rbTypeSerial, 0, wxALL, 5);

  m_rbTypeNet = new wxRadioButton(m_container, wxID_ANY, _("Network"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  bSizer15->Add(m_rbTypeNet, 0, wxALL, 5);

  if (OCPNPlatform::hasInternalGPS()) {
    m_rbTypeInternalGPS =
        new wxRadioButton(m_container, wxID_ANY, _("Built-in GPS"),
                          wxDefaultPosition, wxDefaultSize, 0);
    bSizer15->Add(m_rbTypeInternalGPS, 0, wxALL, 5);
  } else
    m_rbTypeInternalGPS = NULL;

  // has built-in Bluetooth
  if (OCPNPlatform::hasInternalBT()) {
    m_rbTypeInternalBT =
        new wxRadioButton(m_container, wxID_ANY, _("Built-in Bluetooth SPP"),
                          wxDefaultPosition, wxDefaultSize, 0);
    bSizer15->Add(m_rbTypeInternalBT, 0, wxALL, 5);

    m_buttonScanBT = new wxButton(m_container, wxID_ANY, _("BT Scan"),
                                  wxDefaultPosition, wxDefaultSize);
    m_buttonScanBT->Hide();

    wxBoxSizer* bSizer15a = new wxBoxSizer(wxHORIZONTAL);
    sbSizerConnectionProps->Add(bSizer15a, 0, wxEXPAND, 5);

    bSizer15a->Add(m_buttonScanBT, 0, wxALL, 5);

    m_stBTPairs =
        new wxStaticText(m_container, wxID_ANY, _("Bluetooth Data Sources"),
                         wxDefaultPosition, wxDefaultSize, 0);
    m_stBTPairs->Wrap(-1);
    m_stBTPairs->Hide();
    bSizer15a->Add(m_stBTPairs, 0, wxALL, 5);

    wxArrayString mt;
    mt.Add(_T( "unscanned" ));
    m_choiceBTDataSources = new wxChoice(m_container, wxID_ANY,
                                         wxDefaultPosition, wxDefaultSize, mt);

#if 0
        m_BTscan_results.Clear();
        m_BTscan_results.Add(_T("None"));

        m_BTscan_results = g_Platform->getBluetoothScanResults();
        m_choiceBTDataSources->Clear();
        m_choiceBTDataSources->Append(m_BTscan_results[0]);  // scan status

        unsigned int i=1;
        while( (i+1) < m_BTscan_results.GetCount()){
            wxString item1 = m_BTscan_results[i] + _T(";");
            wxString item2 = m_BTscan_results.Item(i+1);
            m_choiceBTDataSources->Append(item1 + item2);

            i += 2;
    }

    if( m_BTscan_results.GetCount() > 1){
        m_choiceBTDataSources->SetSelection( 1 );
    }
#endif

    m_choiceBTDataSources->Hide();
    bSizer15a->Add(m_choiceBTDataSources, 1, wxEXPAND | wxTOP, 5);

    // FIXME Move to Connect section
//     m_buttonScanBT->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
//                             wxCommandEventHandler(options::OnScanBTClick), NULL,
//                             this);
  } else
    m_rbTypeInternalBT = NULL;

  gSizerNetProps = new wxGridSizer(0, 2, 0, 0);

  m_stNetProto = new wxStaticText(m_container, wxID_ANY, _("Protocol"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  m_stNetProto->Wrap(-1);
  gSizerNetProps->Add(m_stNetProto, 0, wxALL, 5);

  wxBoxSizer* bSizer16;
  bSizer16 = new wxBoxSizer(wxHORIZONTAL);

  m_rbNetProtoTCP =
      new wxRadioButton(m_container, wxID_ANY, _("TCP"), wxDefaultPosition,
                        wxDefaultSize, wxRB_GROUP);
  m_rbNetProtoTCP->Enable(TRUE);

  bSizer16->Add(m_rbNetProtoTCP, 0, wxALL, 5);

  m_rbNetProtoUDP = new wxRadioButton(m_container, wxID_ANY, _("UDP"),
                                      wxDefaultPosition, wxDefaultSize, 0);
  m_rbNetProtoUDP->Enable(TRUE);

  bSizer16->Add(m_rbNetProtoUDP, 0, wxALL, 5);

  m_rbNetProtoGPSD = new wxRadioButton(m_container, wxID_ANY, _("GPSD"),
                                       wxDefaultPosition, wxDefaultSize, 0);
  m_rbNetProtoGPSD->SetValue(TRUE);
  bSizer16->Add(m_rbNetProtoGPSD, 0, wxALL, 5);

  m_rbNetProtoSignalK = new wxRadioButton(m_container, wxID_ANY, _("Signal K"),
                                          wxDefaultPosition, wxDefaultSize, 0);
  m_rbNetProtoSignalK->Enable(TRUE);
  bSizer16->Add(m_rbNetProtoSignalK, 0, wxALL, 5);

  gSizerNetProps->Add(bSizer16, 1, wxEXPAND, 5);

  m_stNetAddr = new wxStaticText(m_container, wxID_ANY, _("Address"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_stNetAddr->Wrap(-1);
  gSizerNetProps->Add(m_stNetAddr, 0, wxALL, 5);

  m_tNetAddress = new wxTextCtrl(m_container, wxID_ANY, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize, 0);
  gSizerNetProps->Add(m_tNetAddress, 0, wxEXPAND | wxTOP, 5);

  m_stNetPort = new wxStaticText(m_container, wxID_ANY, _("DataPort"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_stNetPort->Wrap(-1);
  gSizerNetProps->Add(m_stNetPort, 0, wxALL, 5);

  m_tNetPort = new wxTextCtrl(m_container, wxID_ANY, wxEmptyString,
                              wxDefaultPosition, wxDefaultSize, 0);
  gSizerNetProps->Add(m_tNetPort, 1, wxEXPAND | wxTOP, 5);

  //  User Comments
  m_stNetComment = new wxStaticText(m_container, wxID_ANY, _("User Comment"),
                                    wxDefaultPosition, wxDefaultSize, 0);
  m_stNetComment->Wrap(-1);
  gSizerNetProps->Add(m_stNetComment, 0, wxALL, 5);

  m_tNetComment = new wxTextCtrl(m_container, wxID_ANY, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize, 0);
  gSizerNetProps->Add(m_tNetComment, 1, wxEXPAND | wxTOP, 5);

  sbSizerConnectionProps->Add(gSizerNetProps, 0, wxEXPAND, 5);

  gSizerSerProps = new wxGridSizer(0, 1, 0, 0);

  wxFlexGridSizer* fgSizer1;
  fgSizer1 = new wxFlexGridSizer(0, 4, 0, 0);
  fgSizer1->SetFlexibleDirection(wxBOTH);
  fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_stSerPort = new wxStaticText(m_container, wxID_ANY, _("DataPort"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_stSerPort->Wrap(-1);
  fgSizer1->Add(m_stSerPort, 0, wxALL, 5);

  m_comboPort = new wxComboBox(m_container, wxID_ANY, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
  fgSizer1->Add(m_comboPort, 0, wxEXPAND | wxTOP, 5);

  m_stSerBaudrate = new wxStaticText(m_container, wxID_ANY, _("Baudrate"),
                                     wxDefaultPosition, wxDefaultSize, 0);
  m_stSerBaudrate->Wrap(-1);
  fgSizer1->Add(m_stSerBaudrate, 0, wxALL, 5);

  wxString m_choiceBaudRateChoices[] = {
      _("150"),    _("300"),    _("600"),    _("1200"),  _("2400"),
      _("4800"),   _("9600"),   _("19200"),  _("38400"), _("57600"),
      _("115200"), _("230400"), _("460800"), _("921600")};
  int m_choiceBaudRateNChoices =
      sizeof(m_choiceBaudRateChoices) / sizeof(wxString);
  m_choiceBaudRate =
      new wxChoice(m_container, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choiceBaudRateNChoices, m_choiceBaudRateChoices, 0);
  m_choiceBaudRate->SetSelection(0);
  fgSizer1->Add(m_choiceBaudRate, 1, wxEXPAND | wxTOP, 5);

  m_stSerProtocol = new wxStaticText(m_container, wxID_ANY, _("Protocol"),
                                     wxDefaultPosition, wxDefaultSize, 0);
  m_stSerProtocol->Wrap(-1);
  fgSizer1->Add(m_stSerProtocol, 0, wxALL, 5);

  wxString m_choiceSerialProtocolChoices[] = {_("NMEA 0183"), _("NMEA 2000")};
  int m_choiceSerialProtocolNChoices =
      sizeof(m_choiceSerialProtocolChoices) / sizeof(wxString);
  m_choiceSerialProtocol = new wxChoice(
      m_container, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      m_choiceSerialProtocolNChoices, m_choiceSerialProtocolChoices, 0);
  m_choiceSerialProtocol->SetSelection(0);
  m_choiceSerialProtocol->Enable(TRUE);
  fgSizer1->Add(m_choiceSerialProtocol, 1, wxEXPAND | wxTOP, 5);

  m_stPriority = new wxStaticText(m_container, wxID_ANY, _("Priority"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  m_stPriority->Wrap(-1);
  fgSizer1->Add(m_stPriority, 0, wxALL, 5);

  wxString m_choicePriorityChoices[] = {_("0"), _("1"), _("2"), _("3"), _("4"),
                                        _("5"), _("6"), _("7"), _("8"), _("9")};
  int m_choicePriorityNChoices =
      sizeof(m_choicePriorityChoices) / sizeof(wxString);
  m_choicePriority =
      new wxChoice(m_container, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choicePriorityNChoices, m_choicePriorityChoices, 0);
  m_choicePriority->SetSelection(9);
  fgSizer1->Add(m_choicePriority, 0, wxEXPAND | wxTOP, 5);

  gSizerSerProps->Add(fgSizer1, 0, wxEXPAND, 5);

  wxFlexGridSizer* fgSizer5;
  fgSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizer5->SetFlexibleDirection(wxBOTH);
  fgSizer5->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  //  User Comments
  m_stSerialComment = new wxStaticText(m_container, wxID_ANY, _("User Comment"),
                                       wxDefaultPosition, wxDefaultSize, 0);
  m_stSerialComment->Wrap(-1);
  fgSizer5->Add(m_stSerialComment, 0, wxALL, 5);

  m_tSerialComment = new wxTextCtrl(m_container, wxID_ANY, wxEmptyString,
                                    wxDefaultPosition, wxDefaultSize, 0);
  fgSizer5->Add(m_tSerialComment, 1, wxEXPAND | wxTOP, 5);

  m_cbCheckCRC = new wxCheckBox(m_container, wxID_ANY, _("Control checksum"),
                                wxDefaultPosition, wxDefaultSize, 0);
  m_cbCheckCRC->SetValue(TRUE);
  m_cbCheckCRC->SetToolTip(
      _("If checked, only the sentences with a valid checksum are passed "
        "through"));
  fgSizer5->Add(m_cbCheckCRC, 0, wxALL, 5);

  m_cbGarminHost = new wxCheckBox(m_container, wxID_ANY,
                                  _("Use Garmin (GRMN) mode for input"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  m_cbGarminHost->SetValue(FALSE);
  fgSizer5->Add(m_cbGarminHost, 0, wxALL, 5);
#ifndef USE_GARMINHOST
  m_cbGarminHost->Hide();
#endif

  m_cbInput =
      new wxCheckBox(m_container, wxID_ANY, _("Receive Input on this Port"),
                     wxDefaultPosition, wxDefaultSize, 0);
  fgSizer5->Add(m_cbInput, 0, wxALL, 5);

  m_cbOutput =
      new wxCheckBox(m_container, wxID_ANY,
                     wxString::Format(_T("%s (%s)"), _("Output on this port"),
                                      _("as autopilot or NMEA repeater")),
                     wxDefaultPosition, wxDefaultSize, 0);
  fgSizer5->Add(m_cbOutput, 0, wxALL, 5);

  m_stTalkerIdText = new wxStaticText(
      m_container, wxID_ANY,
      wxString::Format(_T("%s (%s)"), _("Talker ID"), _("blank = default ID")),
      wxDefaultPosition, wxDefaultSize, 0);
  m_stTalkerIdText->Wrap(-1);
  fgSizer5->Add(m_stTalkerIdText, 0, wxALL, 5);

  // FIXME Verify "-1" ID is OK
  m_TalkerIdText = new wxTextCtrl(m_container, -1, _T( "" ),
                                  wxDefaultPosition, wxSize(50, -1), 0);
  m_TalkerIdText->SetMaxLength(2);
  fgSizer5->Add(m_TalkerIdText, 0, wxALIGN_LEFT | wxALL, group_item_spacing);

  m_stPrecision =
      new wxStaticText(m_container, wxID_ANY, _("APB bearing precision"),
                       wxDefaultPosition, wxDefaultSize, 0);

  m_stPrecision->Wrap(-1);
  fgSizer5->Add(m_stPrecision, 0, wxALL, 5);

  wxString m_choicePrecisionChoices[] = {_("x"), _("x.x"), _("x.xx"),
                                         _("x.xxx"), _("x.xxxx")};
  int m_choicePrecisionNChoices =
      sizeof(m_choicePrecisionChoices) / sizeof(wxString);
  m_choicePrecision =
      new wxChoice(m_container, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choicePrecisionNChoices, m_choicePrecisionChoices, 0);
  m_choicePrecision->SetSelection(g_NMEAAPBPrecision);
  fgSizer5->Add(m_choicePrecision, 0, wxALL, 5);

  // signalK discovery enable
  m_cbCheckSKDiscover =
      new wxCheckBox(m_container, wxID_ANY, _("Automatic server discovery"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbCheckSKDiscover->SetValue(TRUE);
  m_cbCheckSKDiscover->SetToolTip(
      _("If checked, signal K server will be discovered automatically"));
  fgSizer5->Add(m_cbCheckSKDiscover, 0, wxALL, 5);

  // signal K "Discover now" button
  m_ButtonSKDiscover = new wxButton(m_container, wxID_ANY, _("Discover now..."),
                                    wxDefaultPosition, wxDefaultSize, 0);
  m_ButtonSKDiscover->Hide();
  fgSizer5->Add(m_ButtonSKDiscover, 0, wxALL, 5);

  // signalK Server Status
  m_StaticTextSKServerStatus = new wxStaticText(
      m_container, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0);
  fgSizer5->Add(m_StaticTextSKServerStatus, 0, wxALL, 5);

  sbSizerConnectionProps->Add(gSizerSerProps, 0, wxEXPAND, 5);
  sbSizerConnectionProps->Add(fgSizer5, 0, wxEXPAND, 5);

  sbSizerInFilter = new wxStaticBoxSizer(
      new wxStaticBox(m_container, wxID_ANY, _("Input filtering")), wxVERTICAL);

  wxBoxSizer* bSizer9;
  bSizer9 = new wxBoxSizer(wxHORIZONTAL);

  m_rbIAccept =
      new wxRadioButton(m_container, wxID_ANY, _("Accept only sentences"),
                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  bSizer9->Add(m_rbIAccept, 0, wxALL, 5);

  m_rbIIgnore = new wxRadioButton(m_container, wxID_ANY, _("Ignore sentences"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  bSizer9->Add(m_rbIIgnore, 0, wxALL, 5);

  sbSizerInFilter->Add(bSizer9, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxHORIZONTAL);

  m_tcInputStc =
      new wxTextCtrl(m_container, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_READONLY);
  bSizer11->Add(m_tcInputStc, 1, wxALL | wxEXPAND, 5);

  m_btnInputStcList =
      new wxButton(m_container, wxID_ANY, _T("..."), wxDefaultPosition,
                   wxDefaultSize, wxBU_EXACTFIT);
  bSizer11->Add(m_btnInputStcList, 0, wxALL, 5);

  sbSizerInFilter->Add(bSizer11, 0, wxEXPAND, 5);

  sbSizerConnectionProps->Add(sbSizerInFilter, 0, wxEXPAND, 5);

  sbSizerOutFilter = new wxStaticBoxSizer(
      new wxStaticBox(m_container, wxID_ANY, _("Output filtering")),
      wxVERTICAL);

  wxBoxSizer* bSizer10;
  bSizer10 = new wxBoxSizer(wxHORIZONTAL);

  m_rbOAccept =
      new wxRadioButton(m_container, wxID_ANY, _("Transmit sentences"),
                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  bSizer10->Add(m_rbOAccept, 0, wxALL, 5);

  m_rbOIgnore = new wxRadioButton(m_container, wxID_ANY, _("Drop sentences"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  bSizer10->Add(m_rbOIgnore, 0, wxALL, 5);

  sbSizerOutFilter->Add(bSizer10, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer12;
  bSizer12 = new wxBoxSizer(wxHORIZONTAL);

  m_tcOutputStc =
      new wxTextCtrl(m_container, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_READONLY);
  bSizer12->Add(m_tcOutputStc, 1, wxALL | wxEXPAND, 5);

  m_btnOutputStcList =
      new wxButton(m_container, wxID_ANY, _T("..."), wxDefaultPosition,
                   wxDefaultSize, wxBU_EXACTFIT);
  bSizer12->Add(m_btnOutputStcList, 0, wxALL, 5);

  sbSizerOutFilter->Add(bSizer12, 0, wxEXPAND, 5);
  sbSizerConnectionProps->Add(sbSizerOutFilter, 0, wxEXPAND, 5);

  bSizerOuterContainer->Add(sbSizerConnectionProps, 1, wxALL | wxEXPAND, 5);
#endif
  bSizer4->Add(bSizerOuterContainer, 1, wxEXPAND, 5);

#if 1
  m_buttonAdd->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(ConnectionsDialog::OnAddDatasourceClick),
                       NULL, this);
  m_buttonRemove->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ConnectionsDialog::OnRemoveDatasourceClick), NULL, this);

  m_rbTypeSerial->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                          wxCommandEventHandler(ConnectionsDialog::OnTypeSerialSelected),
                          NULL, this);
  m_rbTypeNet->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(ConnectionsDialog::OnTypeNetSelected), NULL,
                       this);

  if (m_rbTypeInternalGPS)
    m_rbTypeInternalGPS->Connect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(ConnectionsDialog::OnTypeGPSSelected), NULL, this);
  if (m_rbTypeInternalBT)
    m_rbTypeInternalBT->Connect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(ConnectionsDialog::OnTypeBTSelected), NULL, this);

  m_rbNetProtoTCP->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(ConnectionsDialog::OnNetProtocolSelected), NULL, this);
  m_rbNetProtoUDP->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(ConnectionsDialog::OnNetProtocolSelected), NULL, this);
  m_rbNetProtoGPSD->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(ConnectionsDialog::OnNetProtocolSelected), NULL, this);
  m_rbNetProtoSignalK->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(ConnectionsDialog::OnNetProtocolSelected), NULL, this);
  m_tNetAddress->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                         wxCommandEventHandler(ConnectionsDialog::OnConnValChange), NULL,
                         this);
  m_tNetPort->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                      wxCommandEventHandler(ConnectionsDialog::OnConnValChange), NULL,
                      this);
  m_comboPort->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED,
                       wxCommandEventHandler(ConnectionsDialog::OnConnValChange), NULL,
                       this);
  m_comboPort->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                       wxCommandEventHandler(ConnectionsDialog::OnConnValChange), NULL,
                       this);
  m_choiceBaudRate->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
                            wxCommandEventHandler(ConnectionsDialog::OnBaudrateChoice),
                            NULL, this);
  m_choiceSerialProtocol->Connect(
      wxEVT_COMMAND_CHOICE_SELECTED,
      wxCommandEventHandler(ConnectionsDialog::OnProtocolChoice), NULL, this);
  m_choicePriority->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
                            wxCommandEventHandler(ConnectionsDialog::OnConnValChange),
                            NULL, this);
  m_cbCheckCRC->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                        wxCommandEventHandler(ConnectionsDialog::OnCrcCheck), NULL, this);
  m_cbGarminHost->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                          wxCommandEventHandler(ConnectionsDialog::OnUploadFormatChange),
                          NULL, this);
  m_cbGarminUploadHost->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionsDialog::OnUploadFormatChange), NULL, this);
  m_cbFurunoGP3X->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                          wxCommandEventHandler(ConnectionsDialog::OnUploadFormatChange),
                          NULL, this);
  m_cbCheckSKDiscover->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                               wxCommandEventHandler(ConnectionsDialog::OnConnValChange),
                               NULL, this);
  m_ButtonSKDiscover->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                              wxCommandEventHandler(ConnectionsDialog::OnDiscoverButton),
                              NULL, this);

  m_rbIAccept->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(ConnectionsDialog::OnRbAcceptInput), NULL,
                       this);
  m_rbIIgnore->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(ConnectionsDialog::OnRbIgnoreInput), NULL,
                       this);
  m_tcInputStc->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                        wxCommandEventHandler(ConnectionsDialog::OnConnValChange), NULL,
                        this);
  m_btnInputStcList->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                             wxCommandEventHandler(ConnectionsDialog::OnBtnIStcs), NULL,
                             this);
  m_cbInput->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                     wxCommandEventHandler(ConnectionsDialog::OnCbInput), NULL, this);
  m_cbOutput->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                      wxCommandEventHandler(ConnectionsDialog::OnCbOutput), NULL, this);
  m_rbOAccept->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(ConnectionsDialog::OnRbOutput), NULL, this);
  m_rbOIgnore->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(ConnectionsDialog::OnRbOutput), NULL, this);
  m_tcOutputStc->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                         wxCommandEventHandler(ConnectionsDialog::OnConnValChange), NULL,
                         this);
  m_btnOutputStcList->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                              wxCommandEventHandler(ConnectionsDialog::OnBtnOStcs), NULL,
                              this);
  m_cbCheckCRC->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                        wxCommandEventHandler(ConnectionsDialog::OnConnValChange), NULL,
                        this);

  m_cbNMEADebug->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionsDialog::OnShowGpsWindowCheckboxClick), NULL, this);
  m_cbFilterSogCog->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                            wxCommandEventHandler(ConnectionsDialog::OnValChange), NULL,
                            this);
  m_tFilterSec->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                        wxCommandEventHandler(ConnectionsDialog::OnValChange), NULL,
                        this);
  m_cbAPBMagnetic->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                           wxCommandEventHandler(ConnectionsDialog::OnValChange), NULL,
                           this);

  m_tNetComment->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                         wxCommandEventHandler(ConnectionsDialog::OnConnValChange), NULL,
                         this);
  m_tSerialComment->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                            wxCommandEventHandler(ConnectionsDialog::OnConnValChange),
                            NULL, this);

#if 0
//   m_lcSources->Connect(wxEVT_LEFT_DOWN,
//                        wxMouseEventHandler(options::OnConnectionToggleEnableMouse),
//                        NULL, this);
// #if wxCHECK_VERSION(2, 9, 0)
//   m_lcSources->Connect(wxEVT_LIST_ITEM_ACTIVATED,
//                        wxListEventHandler(options::OnConnectionToggleEnable),
//                        NULL, this);
// #endif

  wxString columns[] = {_("Enable"),   _("Type"),       _("DataPort"),
                        _("Priority"), _("Parameters"), _("Connection"),
                        _("Filters")};
  for (int i = 0; i < 7; ++i) {
    wxListItem col;
    col.SetId(i);
    col.SetText(columns[i]);
//    m_lcSources->InsertColumn(i, col);
  }

  //  Build the image list
  wxImageList* imglist = new wxImageList(16, 16, TRUE, 1);
  wxBitmap unchecked_bmp(16, 16), checked_bmp(16, 16);
  wxMemoryDC renderer_dc;

  // Unchecked
  renderer_dc.SelectObject(unchecked_bmp);
  renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(
      GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
  renderer_dc.Clear();
  wxRendererNative::Get().DrawCheckBox(this, renderer_dc, wxRect(0, 0, 16, 16),
                                       0);

  // Checked
  renderer_dc.SelectObject(checked_bmp);
  renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(
      GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
  renderer_dc.Clear();
  wxRendererNative::Get().DrawCheckBox(this, renderer_dc, wxRect(0, 0, 16, 16),
                                       wxCONTROL_CHECKED);

  // Deselect the renderer Object
  renderer_dc.SelectObject(wxNullBitmap);

  imglist->Add(unchecked_bmp);
  imglist->Add(checked_bmp);
//  m_lcSources->AssignImageList(imglist, wxIMAGE_LIST_SMALL);

//  m_lcSources->Refresh();
#endif
#endif
  FillSourceList();

  ShowNMEACommon(true);
  ShowNMEASerial(true);
  ShowNMEANet(true);
  connectionsaved = TRUE;
}

void ConnectionsDialog::SetSelectedConnectionPanel(ConnectionParamsPanel* panel) {
  //  Only one panel can be selected at any time
  //  Clear any selections

  if (mSelectedConnection && mSelectedConnection->m_optionsPanel)
    mSelectedConnection->m_optionsPanel->SetSelected(false);

  if (panel) {
    mSelectedConnection = panel->m_pConnectionParams;
    panel->SetSelected(true);
    SetConnectionParams(mSelectedConnection);
    m_buttonRemove->Enable();
    m_buttonRemove->Show();
    m_buttonAdd->Disable();
    m_sbConnEdit->SetLabel(_("Edit Selected Connection"));

  } else {
    mSelectedConnection = NULL;
    m_buttonRemove->Disable();
    m_buttonAdd->Enable();
    m_buttonAdd->Show();
    m_sbConnEdit->SetLabel(_T(""));
    ClearNMEAForm();
  }
}

void ConnectionsDialog::EnableConnection(ConnectionParams* conn, bool value) {
  if (conn) {
    conn->bEnabled = value;
    conn->b_IsSetup = FALSE;  // trigger a rebuild/takedown of the connection
    m_connection_enabled = conn->bEnabled;
  }
}

#if 1
void ConnectionsDialog::OnValChange(wxCommandEvent& event) { event.Skip(); }

void ConnectionsDialog::OnScanBTClick(wxCommandEvent& event) {
  if (m_BTscanning)
    StopBTScan();
  else {
    m_btNoChangeCounter = 0;
    m_btlastResultCount = 0;

    m_BTScanTimer.Start(1000, wxTIMER_CONTINUOUS);
    g_Platform->startBluetoothScan();
    m_BTscanning = 1;
    if (m_buttonScanBT) {
      m_buttonScanBT->SetLabel(_("Stop Scan"));
    }
  }
}

void ConnectionsDialog::onBTScanTimer(wxTimerEvent& event) {
  if (m_BTscanning) {
    m_BTscanning++;

    m_BTscan_results = g_Platform->getBluetoothScanResults();

    m_choiceBTDataSources->Clear();
    m_choiceBTDataSources->Append(m_BTscan_results[0]);  // scan status

    unsigned int i = 1;
    while ((i + 1) < m_BTscan_results.GetCount()) {
      wxString item1 = m_BTscan_results[i] + _T(";");
      wxString item2 = m_BTscan_results.Item(i + 1);
      m_choiceBTDataSources->Append(item1 + item2);

      i += 2;
    }

    if (m_BTscan_results.GetCount() > 1) {
      m_choiceBTDataSources->SetSelection(1);
    }

    //  Watch for changes.  When no changes occur after n seconds, stop the scan
    if (m_btNoChangeCounter > 5) StopBTScan();

    if ((int)m_BTscan_results.GetCount() == m_btlastResultCount)
      m_btNoChangeCounter++;
    else
      m_btNoChangeCounter = 0;

    m_btlastResultCount = m_BTscan_results.GetCount();

    // Absolute fallback
    if (m_BTscanning >= 15) {
      StopBTScan();
    }
  } else {
  }
  return;
}

void ConnectionsDialog::StopBTScan(void) {
  m_BTScanTimer.Stop();

  g_Platform->stopBluetoothScan();

  m_BTscanning = 0;

  if (m_buttonScanBT) {
    m_buttonScanBT->SetLabel(_("BT Scan"));
    m_buttonScanBT->Enable();
  }
}

void ConnectionsDialog::OnConnValChange(wxCommandEvent& event) {
  connectionsaved = FALSE;
  event.Skip();
}

void ConnectionsDialog::OnTypeSerialSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToSerial();
}

void ConnectionsDialog::OnTypeNetSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToNet();
}

void ConnectionsDialog::OnTypeGPSSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToGPS();
}

void ConnectionsDialog::OnTypeBTSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToBT();
}

void ConnectionsDialog::OnUploadFormatChange(wxCommandEvent& event) {
  if (event.GetEventObject() == m_cbGarminUploadHost && event.IsChecked())
    m_cbFurunoGP3X->SetValue(FALSE);
  else if (event.GetEventObject() == m_cbFurunoGP3X && event.IsChecked())
    m_cbGarminUploadHost->SetValue(FALSE);

  OnConnValChange(event);
  event.Skip();
}
#endif

void ConnectionsDialog::ShowNMEACommon(bool visible) {
  m_rbTypeSerial->Show(visible);
  m_rbTypeNet->Show(visible);
  if (m_rbTypeInternalGPS) m_rbTypeInternalGPS->Show(visible);
  if (m_rbTypeInternalBT) m_rbTypeInternalBT->Show(visible);
  m_rbIAccept->Show(visible);
  m_rbIIgnore->Show(visible);
  m_rbOAccept->Show(visible);
  m_rbOIgnore->Show(visible);
  m_tcInputStc->Show(visible);
  m_btnInputStcList->Show(visible);
  m_tcOutputStc->Show(visible);
  m_btnOutputStcList->Show(visible);
  m_cbInput->Show(visible);
  m_cbOutput->Show(visible);
  m_stPrecision->Show(visible);
  m_choicePrecision->Show(visible);
  m_choicePriority->Show(visible);
  m_stPriority->Show(visible);
  m_stPrecision->Show(visible);
  m_stTalkerIdText->Show(visible);
  m_TalkerIdText->Show(visible);
  m_cbCheckCRC->Show(visible);
  m_cbCheckSKDiscover->Show(visible);
  m_ButtonSKDiscover->Show(visible);
  if (visible) {
    const bool output = m_cbOutput->IsChecked();
    m_stPrecision->Enable(output);
    m_choicePrecision->Enable(output);
    m_stTalkerIdText->Enable(output);
    m_TalkerIdText->Enable(output);
  } else {
    sbSizerOutFilter->SetDimension(0, 0, 0, 0);
    sbSizerInFilter->SetDimension(0, 0, 0, 0);
    sbSizerConnectionProps->SetDimension(0, 0, 0, 0);
    m_sbConnEdit->SetLabel(_T(""));
  }
  m_bNMEAParams_shown = visible;
}

void ConnectionsDialog::ShowNMEANet(bool visible) {
  m_stNetAddr->Show(visible);
  m_tNetAddress->Show(visible);
  m_stNetPort->Show(visible);
  m_tNetPort->Show(visible);
  m_stNetProto->Show(visible);
  m_rbNetProtoSignalK->Show(visible);
  m_rbNetProtoGPSD->Show(visible);
  m_rbNetProtoTCP->Show(visible);
  m_rbNetProtoUDP->Show(visible);
  m_stNetComment->Show(visible);
  m_tNetComment->Show(visible);
}

void ConnectionsDialog::ShowNMEASerial(bool visible) {
  m_stSerBaudrate->Show(visible);
  m_choiceBaudRate->Show(visible);
  m_stSerPort->Show(visible);
  m_comboPort->Show(visible);
  m_stSerProtocol->Show(visible);
  m_choiceSerialProtocol->Show(visible);
  m_cbGarminHost->Show(visible);
  m_stSerialComment->Show(visible);
  m_tSerialComment->Show(visible);
}

void ConnectionsDialog::ShowNMEAGPS(bool visible) {}

void ConnectionsDialog::ShowNMEABT(bool visible) {
  if (visible) {
    if (m_buttonScanBT) m_buttonScanBT->Show();
    if (m_stBTPairs) m_stBTPairs->Show();
    if (m_choiceBTDataSources) {
      if (m_choiceBTDataSources->GetCount() > 1)
        m_choiceBTDataSources->SetSelection(1);
      m_choiceBTDataSources->Show();
    }
  } else {
    if (m_buttonScanBT) m_buttonScanBT->Hide();
    if (m_stBTPairs) m_stBTPairs->Hide();
    if (m_choiceBTDataSources) m_choiceBTDataSources->Hide();
  }
  m_tcOutputStc->Show(visible);
  m_btnOutputStcList->Show(visible);
  m_cbOutput->Show(visible);
}

void ConnectionsDialog::SetNMEAFormToSerial(void) {
  ShowNMEACommon(TRUE);
  ShowNMEANet(FALSE);
  ShowNMEAGPS(FALSE);
  ShowNMEABT(FALSE);
  ShowNMEASerial(TRUE);

  m_container->FitInside();
  // Fit();
  RecalculateSize();
  SetDSFormRWStates();
}

void ConnectionsDialog::SetNMEAFormToNet(void) {
  ShowNMEACommon(TRUE);
  ShowNMEANet(TRUE);
  ShowNMEAGPS(FALSE);
  ShowNMEABT(FALSE);
  ShowNMEASerial(FALSE);
  m_container->FitInside();
  // Fit();
  RecalculateSize();
  SetDSFormRWStates();
}

void ConnectionsDialog::SetNMEAFormToGPS(void) {
  ShowNMEACommon(TRUE);
  ShowNMEANet(FALSE);
  ShowNMEAGPS(TRUE);
  ShowNMEABT(FALSE);
  ShowNMEASerial(FALSE);
  m_container->FitInside();
  // Fit();
  RecalculateSize();
  SetDSFormRWStates();
}

void ConnectionsDialog::SetNMEAFormToBT(void) {
  m_rbNetProtoUDP->SetValue(true);
  ShowNMEACommon(TRUE);
  ShowNMEANet(FALSE);
  ShowNMEAGPS(FALSE);
  ShowNMEABT(TRUE);
  ShowNMEASerial(FALSE);
  m_container->FitInside();
  // Fit();
  RecalculateSize();
  SetDSFormRWStates();
}

void ConnectionsDialog::ClearNMEAForm(void) {
  ShowNMEACommon(FALSE);
  ShowNMEANet(FALSE);
  ShowNMEAGPS(FALSE);
  ShowNMEABT(FALSE);
  ShowNMEASerial(FALSE);
  m_container->FitInside();
  // Fit();
  RecalculateSize();
}


void ConnectionsDialog::SetDSFormOptionVizStates(void) {
  m_cbInput->Show();
  m_cbOutput->Show();
  m_cbCheckCRC->Show();
  m_stPrecision->Show();
  m_choicePrecision->Show();
  m_stTalkerIdText->Show();
  m_TalkerIdText->Show();
  sbSizerInFilter->GetStaticBox()->Show();
  m_rbIAccept->Show();
  m_rbIIgnore->Show();
  sbSizerOutFilter->GetStaticBox()->Show();
  m_rbOAccept->Show();
  m_rbOIgnore->Show();
  m_tcInputStc->Show();
  m_btnInputStcList->Show();
  m_tcOutputStc->Show();
  m_btnOutputStcList->Show();
  m_cbCheckSKDiscover->Show();
  m_ButtonSKDiscover->Show();
  m_StaticTextSKServerStatus->Show();

  if (m_rbTypeSerial->GetValue()) {
    m_cbCheckSKDiscover->Hide();
    m_ButtonSKDiscover->Hide();
    m_StaticTextSKServerStatus->Hide();

  } else if (m_rbNetProtoGPSD->GetValue()) {
    m_cbCheckSKDiscover->Hide();
    m_cbInput->Hide();
    m_cbOutput->Hide();
    sbSizerOutFilter->GetStaticBox()->Hide();
    m_rbOAccept->Hide();
    m_rbOIgnore->Hide();
    m_tcOutputStc->Hide();
    m_btnOutputStcList->Hide();
    m_stPrecision->Hide();
    m_choicePrecision->Hide();
    m_stTalkerIdText->Hide();
    m_TalkerIdText->Hide();
    m_cbCheckSKDiscover->Hide();
    m_ButtonSKDiscover->Hide();
    m_StaticTextSKServerStatus->Hide();

  } else if (m_rbNetProtoSignalK->GetValue()) {
    // here
    m_cbInput->Hide();
    m_cbOutput->Hide();
    m_cbCheckCRC->Hide();
    m_stPrecision->Hide();
    m_choicePrecision->Hide();
    m_stTalkerIdText->Hide();
    m_TalkerIdText->Hide();
    sbSizerInFilter->GetStaticBox()->Hide();
    m_rbIAccept->Hide();
    m_rbIIgnore->Hide();
    sbSizerOutFilter->GetStaticBox()->Hide();
    m_rbOAccept->Hide();
    m_rbOIgnore->Hide();
    m_tcInputStc->Hide();
    m_btnInputStcList->Hide();
    m_tcOutputStc->Hide();
    m_btnOutputStcList->Hide();

  } else {
    m_cbCheckSKDiscover->Hide();
    m_ButtonSKDiscover->Hide();
    m_StaticTextSKServerStatus->Hide();
  }
}

void ConnectionsDialog::SetDSFormRWStates(void) {
  if (m_rbTypeSerial->GetValue()) {
    m_cbInput->Enable(FALSE);
    m_cbOutput->Enable(TRUE);
    m_rbOAccept->Enable(TRUE);
    m_rbOIgnore->Enable(TRUE);
    m_btnOutputStcList->Enable(TRUE);
  } else if (m_rbNetProtoGPSD->GetValue()) {
    if (m_tNetPort->GetValue() == wxEmptyString)
      m_tNetPort->SetValue(_T("2947"));
    m_cbInput->SetValue(TRUE);
    m_cbInput->Enable(FALSE);
    m_cbOutput->SetValue(FALSE);
    m_cbOutput->Enable(FALSE);
    m_rbOAccept->Enable(FALSE);
    m_rbOIgnore->Enable(FALSE);
    m_btnOutputStcList->Enable(FALSE);
  } else if (m_rbNetProtoSignalK->GetValue()) {
    if (m_tNetPort->GetValue() == wxEmptyString)
      m_tNetPort->SetValue(_T("3000"));
    m_cbInput->SetValue(TRUE);
    m_cbInput->Enable(FALSE);
    m_cbOutput->SetValue(FALSE);
    m_cbOutput->Enable(FALSE);
    m_rbOAccept->Enable(FALSE);
    m_rbOIgnore->Enable(FALSE);
    UpdateDiscoverStatus(wxEmptyString);

  } else {
    if (m_tNetPort->GetValue() == wxEmptyString)
      m_tNetPort->SetValue(_T("10110"));
    m_cbInput->Enable(TRUE);
    m_cbOutput->Enable(TRUE);
    m_rbOAccept->Enable(TRUE);
    m_rbOIgnore->Enable(TRUE);
    m_btnOutputStcList->Enable(TRUE);
  }

  SetDSFormOptionVizStates();
  m_container->FitInside();
}

void ConnectionsDialog::SetConnectionParams(ConnectionParams* cp) {
  if (wxNOT_FOUND == m_comboPort->FindString(cp->Port))
    m_comboPort->Append(cp->Port);

  m_comboPort->Select(m_comboPort->FindString(cp->Port));

  m_cbCheckCRC->SetValue(cp->ChecksumCheck);
  m_cbGarminHost->SetValue(cp->Garmin);
  m_cbInput->SetValue(cp->IOSelect != DS_TYPE_OUTPUT);
  m_cbOutput->SetValue(cp->IOSelect != DS_TYPE_INPUT);
  m_cbCheckSKDiscover->SetValue(cp->AutoSKDiscover);

  if (cp->InputSentenceListType == WHITELIST)
    m_rbIAccept->SetValue(TRUE);
  else
    m_rbIIgnore->SetValue(TRUE);
  if (cp->OutputSentenceListType == WHITELIST)
    m_rbOAccept->SetValue(TRUE);
  else
    m_rbOIgnore->SetValue(TRUE);
  m_tcInputStc->SetValue(StringArrayToString(cp->InputSentenceList));
  m_tcOutputStc->SetValue(StringArrayToString(cp->OutputSentenceList));
  m_choiceBaudRate->Select(
      m_choiceBaudRate->FindString(wxString::Format(_T( "%d" ), cp->Baudrate)));
  m_choiceSerialProtocol->Select(cp->Protocol);  // TODO
  m_choicePriority->Select(
      m_choicePriority->FindString(wxString::Format(_T( "%d" ), cp->Priority)));
  m_tNetAddress->SetValue(cp->NetworkAddress);

  if (cp->NetworkPort == 0)
    m_tNetPort->SetValue(wxEmptyString);
  else
    m_tNetPort->SetValue(wxString::Format(wxT("%i"), cp->NetworkPort));

  if (cp->NetProtocol == TCP)
    m_rbNetProtoTCP->SetValue(TRUE);
  else if (cp->NetProtocol == UDP)
    m_rbNetProtoUDP->SetValue(TRUE);
  else if (cp->NetProtocol == GPSD)
    m_rbNetProtoGPSD->SetValue(TRUE);
  else if (cp->NetProtocol == SIGNALK)
    m_rbNetProtoSignalK->SetValue(TRUE);
  else
    m_rbNetProtoGPSD->SetValue(TRUE);

  if (cp->Type == SERIAL) {
    m_rbTypeSerial->SetValue(TRUE);
    SetNMEAFormToSerial();
  } else if (cp->Type == NETWORK) {
    m_rbTypeNet->SetValue(TRUE);
    SetNMEAFormToNet();
  } else if (cp->Type == INTERNAL_GPS) {
    if (m_rbTypeInternalGPS) m_rbTypeInternalGPS->SetValue(TRUE);
    SetNMEAFormToGPS();
  } else if (cp->Type == INTERNAL_BT) {
    if (m_rbTypeInternalBT) m_rbTypeInternalBT->SetValue(TRUE);
    SetNMEAFormToBT();

    // Preset the source selector
    wxString bts = cp->NetworkAddress + _T(";") + cp->GetPortStr();
    m_choiceBTDataSources->Clear();
    m_choiceBTDataSources->Append(bts);
    m_choiceBTDataSources->SetSelection(0);
  } else
    ClearNMEAForm();

  if (cp->Type == SERIAL)
    m_tSerialComment->SetValue(cp->UserComment);
  else if (cp->Type == NETWORK)
    m_tNetComment->SetValue(cp->UserComment);

  m_connection_enabled = cp->bEnabled;

  // Reset touch flag
  connectionsaved = true;
}


void ConnectionsDialog::SetDefaultConnectionParams(void) {
  if (m_comboPort && !m_comboPort->IsListEmpty()) {
    m_comboPort->Select(0);
    m_comboPort->SetValue(wxEmptyString);  // These two broke it
  }
  m_cbCheckCRC->SetValue(TRUE);
  m_cbGarminHost->SetValue(FALSE);
  m_cbInput->SetValue(TRUE);
  m_cbOutput->SetValue(FALSE);
  m_rbIAccept->SetValue(TRUE);
  m_rbOAccept->SetValue(TRUE);
  m_tcInputStc->SetValue(wxEmptyString);
  m_tcOutputStc->SetValue(wxEmptyString);
  m_choiceBaudRate->Select(m_choiceBaudRate->FindString(_T( "4800" )));
  //    m_choiceSerialProtocol->Select( cp->Protocol ); // TODO
  m_choicePriority->Select(m_choicePriority->FindString(_T( "1" )));

  m_tNetAddress->SetValue(_T("0.0.0.0"));

  m_tNetComment->SetValue(wxEmptyString);
  m_tSerialComment->SetValue(wxEmptyString);

  bool bserial = TRUE;
#ifdef __WXGTK__
  bserial = FALSE;
#endif

#ifdef __WXOSX__
  bserial = FALSE;
#endif

#ifdef __OCPN__ANDROID__
  if (m_rbTypeInternalGPS) {
    m_rbTypeInternalGPS->SetValue(true);
    SetNMEAFormToGPS();
  } else {
    m_rbTypeNet->SetValue(true);
    SetNMEAFormToNet();
  }

#else
  m_rbTypeSerial->SetValue(bserial);
  m_rbTypeNet->SetValue(!bserial);
  bserial ? SetNMEAFormToSerial() : SetNMEAFormToNet();
#endif

  m_connection_enabled = TRUE;

  // Reset touch flag
  connectionsaved = false;
}

bool ConnectionsDialog::SortSourceList(void) {
  if (g_pConnectionParams->Count() < 2) return false;

  std::vector<int> ivec;
  for (size_t i = 0; i < g_pConnectionParams->Count(); i++) ivec.push_back(i);

  bool did_sort = false;
  bool did_swap = true;
  while (did_swap) {
    did_swap = false;
    for (size_t j = 1; j < ivec.size(); j++) {
      ConnectionParams* c1 = g_pConnectionParams->Item(ivec[j]);
      ConnectionParams* c2 = g_pConnectionParams->Item(ivec[j - 1]);

      if (c1->Priority > c2->Priority) {
        int t = ivec[j - 1];
        ivec[j - 1] = ivec[j];
        ivec[j] = t;
        did_swap = true;
        did_sort = true;
      }
    }
  }

  // if(did_sort)
  {
    boxSizerConnections = new wxBoxSizer(wxVERTICAL);
    m_scrollWinConnections->SetSizer(boxSizerConnections);

    for (size_t i = 0; i < ivec.size(); i++) {
      ConnectionParamsPanel* pPanel =
          g_pConnectionParams->Item(ivec[i])->m_optionsPanel;
      boxSizerConnections->Add(pPanel, 0, wxEXPAND | wxALL, 0);
    }
  }

  return did_sort;
}

void ConnectionsDialog::FillSourceList(void) {
  m_buttonRemove->Enable(FALSE);

  // Add new panels as necessary
  for (size_t i = 0; i < g_pConnectionParams->Count(); i++) {
    if (!g_pConnectionParams->Item(i)->m_optionsPanel) {
      ConnectionParamsPanel* pPanel = new ConnectionParamsPanel(
          m_scrollWinConnections, wxID_ANY, wxDefaultPosition, wxDefaultSize,
          g_pConnectionParams->Item(i), this);
      pPanel->SetSelected(false);
      boxSizerConnections->Add(pPanel, 0, wxEXPAND | wxALL, 0);
      g_pConnectionParams->Item(i)->m_optionsPanel = pPanel;
    } else {
      g_pConnectionParams->Item(i)->m_optionsPanel->Update(
          g_pConnectionParams->Item(i));
    }
  }
  SortSourceList();
  m_scrollWinConnections->Layout();

  mSelectedConnection = NULL;
  m_buttonAdd->Enable(true);
  m_buttonAdd->Show();
}

void ConnectionsDialog::UpdateSourceList(bool bResort) {
  for (size_t i = 0; i < g_pConnectionParams->Count(); i++) {
    ConnectionParams* cp = g_pConnectionParams->Item(i);
    ConnectionParamsPanel* panel = cp->m_optionsPanel;
    if (panel) panel->Update(g_pConnectionParams->Item(i));
  }

  if (bResort) {
    SortSourceList();
  }

  m_scrollWinConnections->Layout();
}

void ConnectionsDialog::OnAddDatasourceClick(wxCommandEvent& event) {
  //  Unselect all panels
  for (size_t i = 0; i < g_pConnectionParams->Count(); i++)
    g_pConnectionParams->Item(i)->m_optionsPanel->SetSelected(false);

  connectionsaved = FALSE;
  SetDefaultConnectionParams();

  m_sbConnEdit->SetLabel(_("Configure new connection"));

  m_buttonRemove->Hide();  // Disable();
  m_buttonAdd->Hide();     // Disable();

  SetDSFormRWStates();

  RecalculateSize();

  //  Scroll the panel to allow the user to see more of the NMEA parameter
  //  settings area
  wxPoint buttonPosition = m_buttonAdd->GetPosition();
  m_container->Scroll(-1, buttonPosition.y / m_parent->GetScrollRate());
}

void ConnectionsDialog::OnRemoveDatasourceClick(wxCommandEvent& event) {
  if (mSelectedConnection) {
    // Find the index
    int index = -1;
    ConnectionParams* cp = NULL;
    for (size_t i = 0; i < g_pConnectionParams->Count(); i++) {
      cp = g_pConnectionParams->Item(i);
      if (mSelectedConnection == cp) {
        index = i;
        break;
      }
    }

    if ((index >= 0) && (cp)) {
      delete g_pConnectionParams->Item(index)->m_optionsPanel;
      g_pConnectionParams->RemoveAt(index);

      //FIXME (dave)
      //DataStream* pds_existing = g_pMUX->FindStream(cp->GetDSPort());
      //if (pds_existing) g_pMUX->StopAndRemoveStream(pds_existing);
      // delete mSelectedConnection->m_optionsPanel;
      mSelectedConnection = NULL;
    }
  }

  //  Mark connection deleted
  m_rbTypeSerial->SetValue(TRUE);
  m_comboPort->SetValue(_T( "Deleted" ));

  FillSourceList();
  ShowNMEACommon(FALSE);
  ShowNMEANet(FALSE);
  ShowNMEASerial(FALSE);
}

void ConnectionsDialog::OnSelectDatasource(wxListEvent& event) {
  SetConnectionParams(g_pConnectionParams->Item(event.GetData()));
  m_buttonRemove->Enable();
  m_buttonRemove->Show();
  event.Skip();
}

void ConnectionsDialog::OnDiscoverButton(wxCommandEvent& event) {
#if 0 //FIXME (dave)
  wxString ip;
  int port;
  std::string serviceIdent =
      std::string("_signalk-ws._tcp.local.");  // Works for node.js server

  g_Platform->ShowBusySpinner();

  if (SignalKDataStream::DiscoverSKServer(serviceIdent, ip, port,
                                          1))  // 1 second scan
  {
    m_tNetAddress->SetValue(ip);
    m_tNetPort->SetValue(wxString::Format(wxT("%i"), port));
    UpdateDiscoverStatus(_("Signal K server available."));
  } else {
    UpdateDiscoverStatus(_("Signal K server not found."));
  }
  g_Platform->HideBusySpinner();
#endif
  event.Skip();
}

void ConnectionsDialog::UpdateDiscoverStatus(wxString stat) {
  m_StaticTextSKServerStatus->SetLabel(stat);
}

void ConnectionsDialog::OnBtnIStcs(wxCommandEvent& event) {
  const ListType type = m_rbIAccept->GetValue() ? WHITELIST : BLACKLIST;
  const wxArrayString list =
      wxStringTokenize(m_tcInputStc->GetValue(), _T( "," ));
  SentenceListDlg dlg(m_parent, FILTER_INPUT, type, list);

  if (dlg.ShowModal() == wxID_OK) m_tcInputStc->SetValue(dlg.GetSentences());
}

void ConnectionsDialog::OnBtnOStcs(wxCommandEvent& event) {
  const ListType type = m_rbOAccept->GetValue() ? WHITELIST : BLACKLIST;
  const wxArrayString list =
      wxStringTokenize(m_tcOutputStc->GetValue(), _T( "," ));
  SentenceListDlg dlg(m_parent, FILTER_OUTPUT, type, list);

  if (dlg.ShowModal() == wxID_OK) m_tcOutputStc->SetValue(dlg.GetSentences());
}

void ConnectionsDialog::OnNetProtocolSelected(wxCommandEvent& event) {
  if (m_rbNetProtoGPSD->GetValue()) {
    if (m_tNetPort->GetValue().IsEmpty()) m_tNetPort->SetValue(_T( "2947" ));
  } else if (m_rbNetProtoUDP->GetValue()) {
    if (m_tNetPort->GetValue().IsEmpty()) m_tNetPort->SetValue(_T( "10110" ));
    if (m_tNetAddress->GetValue().IsEmpty())
      m_tNetAddress->SetValue(_T( "0.0.0.0" ));
  } else if (m_rbNetProtoSignalK->GetValue()) {
    if (m_tNetPort->GetValue().IsEmpty()) m_tNetPort->SetValue(_T( "8375" ));
  } else if (m_rbNetProtoTCP->GetValue()) {
    if (m_tNetPort->GetValue().IsEmpty()) m_tNetPort->SetValue(_T( "10110" ));
  }

  SetDSFormRWStates();
  OnConnValChange(event);
}
void ConnectionsDialog::OnRbAcceptInput(wxCommandEvent& event) { OnConnValChange(event); }
void ConnectionsDialog::OnRbIgnoreInput(wxCommandEvent& event) { OnConnValChange(event); }

void ConnectionsDialog::OnRbOutput(wxCommandEvent& event) { OnConnValChange(event); }

void ConnectionsDialog::OnCbInput(wxCommandEvent& event) { OnConnValChange(event); }

void ConnectionsDialog::OnCbOutput(wxCommandEvent& event) {
  OnConnValChange(event);
  const bool checked = m_cbOutput->IsChecked();
  m_stPrecision->Enable(checked);
  m_choicePrecision->Enable(checked);
  m_stTalkerIdText->Enable(checked);
  m_TalkerIdText->Enable(checked);
}

void ConnectionsDialog::OnShowGpsWindowCheckboxClick(wxCommandEvent& event) {
  if (!m_cbNMEADebug->GetValue()) {
    NMEALogWindow::Get().DestroyWindow();
  } else {
    NMEALogWindow::Get().Create((wxWindow *)(m_parent->pParent), 35);

    // Try to ensure that the log window is a least a little bit visible
    wxRect logRect(
        NMEALogWindow::Get().GetPosX(), NMEALogWindow::Get().GetPosY(),
        NMEALogWindow::Get().GetSizeW(), NMEALogWindow::Get().GetSizeH());

    if (m_container->GetRect().Contains(logRect)) {
      NMEALogWindow::Get().SetPos(
          m_container->GetRect().x / 2,
          (m_container->GetRect().y + (m_container->GetRect().height - logRect.height) / 2));
      NMEALogWindow::Get().Move();
    }

    m_parent->Raise();
  }
}

void ConnectionsDialog::ApplySettings(){

  g_bfilter_cogsog = m_cbFilterSogCog->GetValue();

  long filter_val = 1;
  m_tFilterSec->GetValue().ToLong(&filter_val);
  g_COGFilterSec =
      wxMin(static_cast<int>(filter_val), 60/*MAX_COGSOG_FILTER_SECONDS*/);  //FIXME (dave)  should be
  g_COGFilterSec = wxMax(g_COGFilterSec, 1);
  g_SOGFilterSec = g_COGFilterSec;

  g_bMagneticAPB = m_cbAPBMagnetic->GetValue();
  g_NMEAAPBPrecision = m_choicePrecision->GetCurrentSelection();

  // NMEA Source
  //  If the stream selected exists, capture some of its existing parameters
  //  to facility identification and allow stop and restart of the stream
  wxString lastAddr;
  int lastPort = 0;
  NetworkProtocol lastNetProtocol = PROTO_UNDEFINED;

  if (mSelectedConnection) {
    ConnectionParams* cpo = mSelectedConnection;
    lastAddr = cpo->NetworkAddress;
    lastPort = cpo->NetworkPort;
    lastNetProtocol = cpo->NetProtocol;
  }

  if (!connectionsaved) {
    size_t nCurrentPanelCount = g_pConnectionParams->GetCount();
    ConnectionParams* cp = NULL;
    int old_priority = -1;
    {
      if (mSelectedConnection) {
        cp = mSelectedConnection;
        old_priority = cp->Priority;
        UpdateConnectionParamsFromSelectedItem(cp);
        cp->b_IsSetup = false;

        // delete g_pConnectionParams->Item(itemIndex)->m_optionsPanel;
        // old_priority = g_pConnectionParams->Item(itemIndex)->Priority;
        // g_pConnectionParams->RemoveAt(itemIndex);
        // g_pConnectionParams->Insert(cp, itemIndex);
        // mSelectedConnection = cp;
        // cp->m_optionsPanel->SetSelected( true );
      } else {
        cp = CreateConnectionParamsFromSelectedItem();
        if (cp) g_pConnectionParams->Add(cp);
      }

      //  Record the previous parameters, if any
      if (cp) {
        cp->LastNetProtocol = lastNetProtocol;
        cp->LastNetworkAddress = lastAddr;
        cp->LastNetworkPort = lastPort;
      }

      if (g_pConnectionParams->GetCount() != nCurrentPanelCount)
        FillSourceList();
      else if (old_priority >= 0) {
        if (old_priority != cp->Priority)  // need resort
          UpdateSourceList(true);
        else
          UpdateSourceList(false);
      }

      connectionsaved = TRUE;
    }
    //     else {
    //       ::wxEndBusyCursor();
    //       if (m_bNMEAParams_shown) event.SetInt(wxID_STOP);
    //     }

    SetSelectedConnectionPanel(nullptr);
  }

  // Recreate datastreams that are new, or have been edited
  for (size_t i = 0; i < g_pConnectionParams->Count(); i++) {
    ConnectionParams* cp = g_pConnectionParams->Item(i);

    if (cp->b_IsSetup) continue;

    // Connection is new, or edited, or disabled

    // Terminate and remove any existing driver, if present in registry
    StopAndRemoveCommDriver(cp->GetStrippedDSPort());

#if 0  //FIXME
    //  Try to stop any previous stream to avoid orphans
    DataStream* pds_existing = g_pMUX->FindStream(cp->GetLastDSPort());
    if (pds_existing) g_pMUX->StopAndRemoveStream(pds_existing);

    //  This for Bluetooth, which has strange parameters
    if (cp->Type == INTERNAL_BT) {
      pds_existing = g_pMUX->FindStream(cp->GetPortStr());
      if (pds_existing) g_pMUX->StopAndRemoveStream(pds_existing);
    }

    // Internal BlueTooth driver stacks commonly need a time delay to purge
    // their buffers, etc. before restating with new parameters...
    if (cp->Type == INTERNAL_BT) wxSleep(1);
#endif

    //Connection has been disabled
    if (!cp->bEnabled) continue;

    //Make any new or re-enabled drivers
    MakeCommDriver(cp);
    cp->b_IsSetup = TRUE;
  }

  g_bGarminHostUpload = m_cbGarminUploadHost->GetValue();
  g_GPS_Ident =
      m_cbFurunoGP3X->GetValue() ? _T( "FurunoGP3X" ) : _T( "Generic" );
}

ConnectionParams* ConnectionsDialog::CreateConnectionParamsFromSelectedItem(void) {
  if (!m_bNMEAParams_shown) return NULL;

  //  Special encoding for deleted connection
  if (m_rbTypeSerial->GetValue() && m_comboPort->GetValue() == _T("Deleted" ))
    return NULL;

  //  We check some values here for consistency.
  //  If necessary, set defaults so user will see some result, however wrong...

  //  DataStreams should be Input, Output, or Both
  if (!(m_cbInput->GetValue() || m_cbOutput->GetValue())) {
    m_cbInput->SetValue(true);
  }

  if (m_rbTypeSerial->GetValue() && m_comboPort->GetValue() == wxEmptyString) {
    m_comboPort->Select(0);
  }
  //  TCP, GPSD and UDP require port field to be set.
  //  TCP clients, GPSD and UDP output sockets require an address
  else if (m_rbTypeNet->GetValue()) {
    if (wxAtoi(m_tNetPort->GetValue()) == 0) {
      m_tNetPort->SetValue(_T("2947"));  // reset to default
    }
    if (m_tNetAddress->GetValue() == wxEmptyString) {
      m_tNetAddress->SetValue(_T("0.0.0.0"));
    }
  }

  ConnectionParams* pConnectionParams = new ConnectionParams();

  UpdateConnectionParamsFromSelectedItem(pConnectionParams);

  ConnectionParamsPanel* pPanel = new ConnectionParamsPanel(
      m_scrollWinConnections, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      pConnectionParams, this);
  pPanel->SetSelected(false);
  boxSizerConnections->Add(pPanel, 0, wxEXPAND | wxALL, 0);
  pConnectionParams->m_optionsPanel = pPanel;

  return pConnectionParams;
}

ConnectionParams* ConnectionsDialog::UpdateConnectionParamsFromSelectedItem(
    ConnectionParams* pConnectionParams) {
  pConnectionParams->Valid = TRUE;
  if (m_rbTypeSerial->GetValue())
    pConnectionParams->Type = SERIAL;
  else if (m_rbTypeNet->GetValue())
    pConnectionParams->Type = NETWORK;
  else if (m_rbTypeInternalGPS && m_rbTypeInternalGPS->GetValue())
    pConnectionParams->Type = INTERNAL_GPS;
  else if (m_rbTypeInternalBT && m_rbTypeInternalBT->GetValue())
    pConnectionParams->Type = INTERNAL_BT;

  //  Save the existing addr/port to allow closing of existing port
  pConnectionParams->LastNetworkAddress = pConnectionParams->NetworkAddress;
  pConnectionParams->LastNetworkPort = pConnectionParams->NetworkPort;
  pConnectionParams->LastNetProtocol = pConnectionParams->NetProtocol;

  pConnectionParams->NetworkAddress = m_tNetAddress->GetValue();
  pConnectionParams->NetworkPort = wxAtoi(m_tNetPort->GetValue());
  if (m_rbNetProtoTCP->GetValue())
    pConnectionParams->NetProtocol = TCP;
  else if (m_rbNetProtoUDP->GetValue())
    pConnectionParams->NetProtocol = UDP;
  else if (m_rbNetProtoGPSD->GetValue())
    pConnectionParams->NetProtocol = GPSD;
  else if (m_rbNetProtoSignalK->GetValue())
    pConnectionParams->NetProtocol = SIGNALK;
  else
    pConnectionParams->NetProtocol = PROTO_UNDEFINED;

  pConnectionParams->Baudrate = wxAtoi(m_choiceBaudRate->GetStringSelection());
  pConnectionParams->Protocol = (DataProtocol)m_choiceSerialProtocol->GetSelection();
  pConnectionParams->Priority = wxAtoi(m_choicePriority->GetStringSelection());
  pConnectionParams->ChecksumCheck = m_cbCheckCRC->GetValue();
  pConnectionParams->AutoSKDiscover = m_cbCheckSKDiscover->GetValue();
  pConnectionParams->Garmin = m_cbGarminHost->GetValue();
  pConnectionParams->InputSentenceList =
      wxStringTokenize(m_tcInputStc->GetValue(), _T(","));
  if (m_rbIAccept->GetValue())
    pConnectionParams->InputSentenceListType = WHITELIST;
  else
    pConnectionParams->InputSentenceListType = BLACKLIST;
  if (m_cbInput->GetValue()) {
    if (m_cbOutput->GetValue()) {
      pConnectionParams->IOSelect = DS_TYPE_INPUT_OUTPUT;
    } else {
      pConnectionParams->IOSelect = DS_TYPE_INPUT;
    }
  } else
    pConnectionParams->IOSelect = DS_TYPE_OUTPUT;

  pConnectionParams->OutputSentenceList =
      wxStringTokenize(m_tcOutputStc->GetValue(), _T(","));
  if (m_rbOAccept->GetValue())
    pConnectionParams->OutputSentenceListType = WHITELIST;
  else
    pConnectionParams->OutputSentenceListType = BLACKLIST;
  pConnectionParams->Port = m_comboPort->GetValue().BeforeFirst(' ');
#if defined(__linux__) && !defined(__OCPN__ANDROID__)
  if (pConnectionParams->Type == SERIAL)
    CheckSerialAccess(m_parent, pConnectionParams->Port.ToStdString());
#endif

//   if ((pConnectionParams->Type != INTERNAL_GPS) &&
//       (pConnectionParams->Type != INTERNAL_BT))
//     CheckDeviceAccess(pConnectionParams->Port);

  pConnectionParams->bEnabled = m_connection_enabled;
  pConnectionParams->b_IsSetup = FALSE;

  if (pConnectionParams->Type == INTERNAL_GPS) {
    pConnectionParams->NetworkAddress = _T("");
    pConnectionParams->NetworkPort = 0;
    pConnectionParams->NetProtocol = PROTO_UNDEFINED;
    pConnectionParams->Baudrate = 0;
  }

  if (pConnectionParams->Type == INTERNAL_BT) {
    wxString parms = m_choiceBTDataSources->GetStringSelection();
    wxStringTokenizer tkz(parms, _T(";"));
    wxString name = tkz.GetNextToken();
    wxString mac = tkz.GetNextToken();

    pConnectionParams->NetworkAddress = name;
    pConnectionParams->Port = mac;
    pConnectionParams->NetworkPort = 0;
    pConnectionParams->NetProtocol = PROTO_UNDEFINED;
    pConnectionParams->Baudrate = 0;
    //        pConnectionParams->SetAuxParameterStr(m_choiceBTDataSources->GetStringSelection());
  }

  if (pConnectionParams->Type == SERIAL) {
    pConnectionParams->UserComment = m_tSerialComment->GetValue();
  } else if (pConnectionParams->Type == NETWORK) {
    pConnectionParams->UserComment = m_tNetComment->GetValue();
  }

  return pConnectionParams;
}

SentenceListDlg::SentenceListDlg(wxWindow* parent, FilterDirection dir,
                                 ListType type, const wxArrayString& list)
    : wxDialog(parent, wxID_ANY, _("Sentence Filter"), wxDefaultPosition,
               wxSize(280, 420)),
      m_type(type),
      m_dir(dir),
      m_sentences(NMEA0183().GetRecognizedArray()) {
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* secondSizer = new wxBoxSizer(wxHORIZONTAL);
  wxStaticBox* pclbBox = new wxStaticBox(this, wxID_ANY, GetBoxLabel());
  wxStaticBoxSizer* stcSizer = new wxStaticBoxSizer(pclbBox, wxVERTICAL);
  m_clbSentences = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition,
                                      wxDefaultSize, m_sentences);
  wxBoxSizer* btnEntrySizer = new wxBoxSizer(wxVERTICAL);
  wxButton* btnCheckAll = new wxButton(this, wxID_ANY, _("Select All"));
  wxButton* btnClearAll = new wxButton(this, wxID_ANY, _("Clear All"));
  wxButton* btnAdd = new wxButton(this, wxID_ANY, _("Add"));
  m_btnDel = new wxButton(this, wxID_ANY, _("Delete"));
  m_btnDel->Disable();
  wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
  wxButton* btnOK = new wxButton(this, wxID_OK);
  wxButton* btnCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));

  secondSizer->Add(stcSizer, 1, wxALL | wxEXPAND, 5);
  stcSizer->Add(m_clbSentences, 1, wxALL | wxEXPAND, 5);
  btnEntrySizer->Add(btnCheckAll, 0, wxALL, 5);
  btnEntrySizer->Add(btnClearAll, 0, wxALL, 5);
  btnEntrySizer->AddSpacer(1);
  btnEntrySizer->Add(btnAdd, 0, wxALL, 5);
  btnEntrySizer->Add(m_btnDel, 0, wxALL, 5);
  secondSizer->Add(btnEntrySizer, 0, wxALL | wxEXPAND, 5);
  mainSizer->Add(secondSizer, 1, wxEXPAND, 5);
  btnSizer->AddButton(btnOK);
  btnSizer->AddButton(btnCancel);
  btnSizer->Realize();
  mainSizer->Add(btnSizer, 0, wxALL | wxEXPAND, 5);

  SetSizer(mainSizer);
  mainSizer->SetSizeHints(this);
  Centre();

  // Connect Events
  btnAdd->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                  wxCommandEventHandler(SentenceListDlg::OnAddClick), NULL,
                  this);
  m_btnDel->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                    wxCommandEventHandler(SentenceListDlg::OnDeleteClick), NULL,
                    this);
  m_clbSentences->Connect(wxEVT_COMMAND_LISTBOX_SELECTED,
                          wxCommandEventHandler(SentenceListDlg::OnCLBSelect),
                          NULL, this);
  btnCheckAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(SentenceListDlg::OnCheckAllClick),
                       NULL, this);
  btnClearAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(SentenceListDlg::OnClearAllClick),
                       NULL, this);

  Populate(list);
}

wxString SentenceListDlg::GetBoxLabel(void) const {
  if (m_dir == FILTER_OUTPUT)
    return m_type == WHITELIST ? _("Transmit sentences") : _("Drop sentences");
  else
    return m_type == WHITELIST ? _("Accept only sentences")
                               : _("Ignore sentences");
}

void SentenceListDlg::Populate(const wxArrayString& list) {
  if (m_dir == FILTER_OUTPUT) {
    m_sentences.Add(_T("ECRMB"));
    m_sentences.Add(_T("ECRMC"));
    m_sentences.Add(_T("ECAPB"));
  }
  m_sentences.Add(_T("AIVDM"));
  m_sentences.Add(_T("AIVDO"));
  m_sentences.Add(_T("FRPOS"));
  m_sentences.Add(_T("CD"));
  m_clbSentences->Clear();
  m_clbSentences->InsertItems(m_sentences, 0);

  if (list.Count() == 0) {
    for (size_t i = 0; i < m_clbSentences->GetCount(); ++i)
      m_clbSentences->Check(i, m_type == WHITELIST);
  } else {
    m_clbSentences->InsertItems(list, m_sentences.GetCount());
    for (size_t i = 0; i < list.Count(); ++i) {
      int item = m_clbSentences->FindString(list[i]);
      if (item != wxNOT_FOUND) m_clbSentences->Check(item);
    }
  }
}

wxString SentenceListDlg::GetSentences(void) {
  wxArrayString retString;
  for (size_t i = 0; i < m_clbSentences->GetCount(); i++) {
    if (m_clbSentences->IsChecked(i))
      retString.Add(m_clbSentences->GetString(i));
  }
  return StringArrayToString(retString);
}

void SentenceListDlg::OnCLBSelect(wxCommandEvent& e) {
  // Only activate the "Delete" button if the selection is not in the standard
  // list
  m_btnDel->Enable(m_sentences.Index(e.GetString()) == wxNOT_FOUND);
}

void SentenceListDlg::OnAddClick(wxCommandEvent& event) {
#ifdef __OCPN__ANDROID__
  androidDisableRotation();
#endif

  wxTextEntryDialog textdlg(
      this,
      _("Enter the NMEA sentence (2, 3 or 5 characters)\n  or a valid REGEX "
        "expression (6 characters or longer)"),
      _("Enter the NMEA sentence"));

  textdlg.SetTextValidator(wxFILTER_ASCII);
  int result = textdlg.ShowModal();

#ifdef __OCPN__ANDROID__
  androidEnableRotation();
#endif

  if (result == wxID_CANCEL) return;
  wxString stc = textdlg.GetValue();

  if (stc.Length() == 2 || stc.Length() == 3 || stc.Length() == 5) {
    m_clbSentences->Append(stc);
    m_clbSentences->Check(m_clbSentences->FindString(stc));
    return;
  } else if (stc.Length() < 2) {
    OCPNMessageBox(
        this,
        _("An NMEA sentence is generally 3 characters long (like RMC, GGA etc.)\n \
          It can also have a two letter prefix identifying the source, or TALKER, of the message.\n \
          The whole sentences then looks like GPGGA or AITXT.\n \
          You may filter out all the sentences with certain TALKER prefix (like GP, AI etc.).\n \
          The filter also accepts Regular Expressions (REGEX) with 6 or more characters. \n\n"),
        _("OpenCPN Info"));
    return;
  }

  else {
    // Verify that a longer text entry is a valid RegEx
    wxRegEx r(stc);
    if (r.IsValid()) {
      m_clbSentences->Append(stc);
      m_clbSentences->Check(m_clbSentences->FindString(stc));
      return;
    } else {
      OCPNMessageBox(this, _("REGEX syntax error: \n") + stc,
                     _("OpenCPN Info"));
      return;
    }
  }
}

void SentenceListDlg::OnDeleteClick(wxCommandEvent& event) {
  m_clbSentences->Delete(m_clbSentences->GetSelection());
}

void SentenceListDlg::OnClearAllClick(wxCommandEvent& event) {
  for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
    m_clbSentences->Check(i, FALSE);
}

void SentenceListDlg::OnCheckAllClick(wxCommandEvent& event) {
  for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
    m_clbSentences->Check(i, TRUE);
}

