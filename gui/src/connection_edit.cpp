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

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include "config.h"

#include <wx/tokenzr.h>
#include <wx/regex.h>

#if defined(__linux__) && !defined(__ANDROID__)
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <serial/serial.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "dnet.h"
#endif

#ifdef __ANDROID__
#include "androidUTIL.h"
#include "qdebug.h"
#endif

#include "model/comm_drv_factory.h"
#include "model/config_vars.h"
#include "model/ser_ports.h"
#include "model/sys_events.h"

#include "connection_edit.h"
#include "conn_params_panel.h"
#include "gui_lib.h"
#include "nmea0183.h"
#include "NMEALogWindow.h"
#include "OCPNPlatform.h"
#include "ocpn_plugin.h"  // FIXME for GetOCPNScaledFont_PlugIn
#include "options.h"
#include "priority_gui.h"
#include "udev_rule_mgr.h"


extern bool g_bfilter_cogsog;
extern int g_COGFilterSec;
extern int g_SOGFilterSec;

extern OCPNPlatform* g_Platform;

static wxString StringArrayToString(wxArrayString arr) {
  wxString ret = wxEmptyString;
  for (size_t i = 0; i < arr.Count(); i++) {
    if (i > 0) ret.Append(",");
    ret.Append(arr[i]);
  }
  return ret;
}

// Check available SocketCAN interfaces

#if defined(__linux__) && !defined(__ANDROID__)
static intf_t   *intf;
std::vector<std::string> can_if_candidates;
static int print_intf(const struct intf_entry *entry, void *arg)
{
  std::string iface = entry->intf_name;
  if (entry->intf_type == 1 && iface.find("can") != std::string::npos) {
    can_if_candidates.push_back(entry->intf_name);
  }
  return 0;
}
#endif

static wxArrayString GetAvailableSocketCANInterfaces() {
  wxArrayString rv;

#if defined(__linux__) && !defined(__ANDROID__)
  struct intf_entry *entry;

  can_if_candidates.clear();

  if ((intf = intf_open()) == NULL) {
    wxLogWarning("Error opening interface list");
  }

  if (intf_loop(intf, print_intf, NULL) < 0) {
    wxLogWarning("Error looping over interface list");
  }
  intf_close(intf);

  for (const auto & iface : can_if_candidates) {
    int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) {
      continue;
    }

    // Get the interface index
    struct ifreq if_request;
    strcpy(if_request.ifr_name, iface.c_str());
    if (ioctl(sock, SIOCGIFINDEX, &if_request) < 0) {
      continue;
    }

    // Check if interface is UP
    struct sockaddr_can can_address;
    can_address.can_family = AF_CAN;
    can_address.can_ifindex = if_request.ifr_ifindex;
    if (ioctl(sock, SIOCGIFFLAGS, &if_request) < 0) {
      continue;
    }
    if (if_request.ifr_flags & IFF_UP) {
      rv.Add(iface);
    } else {
      continue;
    }
  }
#endif
  return rv;
}

static void LoadSerialPorts(wxComboBox* box) {
  box->Clear();
  wxArrayString* ports = EnumerateSerialPorts();
  for (size_t i = 0; i < ports->GetCount(); i++) box->Append((*ports)[i]);
  delete ports;
}

//------------------------------------------------------------------------------
//          ConnectionEditDialog Implementation
//------------------------------------------------------------------------------

//BEGIN_EVENT_TABLE(ConnectionEditDialog, wxDialog)
//EVT_TIMER(ID_BT_SCANTIMER, ConnectionEditDialog::onBTScanTimer)
//END_EVENT_TABLE()

// Define constructors
ConnectionEditDialog::ConnectionEditDialog() {}

ConnectionEditDialog::ConnectionEditDialog( options *parent, ConnectionsDialog *client)
    : wxDialog(parent, wxID_ANY, _("Connection Edit"), wxDefaultPosition,
               wxSize(280, 420))
{
  m_parent = parent;

  Init();
}

ConnectionEditDialog::~ConnectionEditDialog() {}


void ConnectionEditDialog::SetInitialSettings(void) {
  LoadSerialPorts(m_comboPort);
}

void ConnectionEditDialog::Init() {

  MORE = "<span foreground=\'blue\'>";
  MORE += _("More");
  MORE += "...</span>";
  LESS = "<span foreground=\'blue\'>";
  LESS += _("Less");
  LESS += "...</span>";

  //  For small displays, skip the "More" text.
  // if (g_Platform->getDisplaySize().x < 80 * GetCharWidth()) MORE = "";


  // Setup some inital values
  m_buttonScanBT = 0;
  m_stBTPairs = 0;
  m_choiceBTDataSources = 0;

  m_BTScanTimer.SetOwner(this, ID_BT_SCANTIMER);
  m_BTscanning = 0;

  // Create the UI

  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *boxSizer02 = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(boxSizer02, 1, wxEXPAND | wxALL, 2);

  m_scrolledwin = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxSize(-1, -1),
                            wxTAB_TRAVERSAL | wxVSCROLL);
  m_scrolledwin->SetScrollRate(0, 5);
  boxSizer02->Add(m_scrolledwin, 1, wxALL | wxEXPAND, 3);

  auto boxSizerSWin = new wxBoxSizer(wxVERTICAL);
  m_scrolledwin->SetSizer(boxSizerSWin);


  wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
  wxButton* btnOK = new wxButton(this, wxID_OK);
  wxButton* btnCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
  btnSizer->AddButton(btnOK);
  btnSizer->AddButton(btnCancel);
  btnSizer->Realize();
  mainSizer->Add(btnSizer, 0, wxALL | wxEXPAND, 5);

  SetSizer(mainSizer);

  int group_item_spacing = 2;

#if 0
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
  sfilt.Printf("%d", g_COGFilterSec);
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
  m_cbFurunoGP3X->SetValue(g_GPS_Ident == "FurunoGP3X");
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

  m_ButtonPriorityDialog = new wxButton(m_container, wxID_ANY,
                                        _("Adjust communication priorities..."),
                                        wxDefaultPosition, wxDefaultSize, 0);
  bSizer161->Add(m_ButtonPriorityDialog, 0, wxALL, cb_space);

  bSizer151->Add(bSizer161, 1, wxEXPAND, 5);
  sbSizerGeneral->Add(bSizer151, 1, wxEXPAND, 5);
  bSizerOuterContainer->Add(sbSizerGeneral, 0, wxALL | wxEXPAND, 5);
#endif

#if 0
  //  Connections listbox, etc
  wxStaticBoxSizer* sbSizerLB = new wxStaticBoxSizer(
      new wxStaticBox(m_container, wxID_ANY, _("Data Connections")),
      wxVERTICAL);

  wxPanel* cPanel =
      new wxPanel(m_container, wxID_ANY, wxDefaultPosition,
                  wxSize(-1, -1), wxBG_STYLE_ERASE);
  sbSizerLB->Add(cPanel, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer* boxSizercPanel = new wxBoxSizer(wxVERTICAL);
  cPanel->SetSizer(boxSizercPanel);

#ifdef __ANDROID__
  m_scrollWinConnections =
      new wxPanel(cPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                  wxBORDER_RAISED | wxBG_STYLE_ERASE);
#else
  m_scrollWinConnections = new wxScrolledWindow(
      cPanel, wxID_ANY, wxDefaultPosition, wxSize(-1, 80),
      wxBORDER_RAISED | wxVSCROLL | wxBG_STYLE_ERASE);
  m_scrollWinConnections->SetScrollRate(5, 5);
#endif

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

#endif
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
      new wxStaticBox(m_scrolledwin, wxID_ANY, _("Edit Selected Connection"));
  m_sbConnEdit->SetFont(*bFont);

  sbSizerConnectionProps = new wxStaticBoxSizer(m_sbConnEdit, wxVERTICAL);
  m_scrolledwin->GetSizer()->Add(sbSizerConnectionProps, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer* bSizer15;
  bSizer15 = new wxBoxSizer(wxHORIZONTAL);

  sbSizerConnectionProps->Add(bSizer15, 0, wxTOP | wxEXPAND, 5);
//  bSizer4->Add(bSizerOuterContainer, 1, wxEXPAND, 5);

  m_rbTypeSerial =
      new wxRadioButton(m_scrolledwin, wxID_ANY, _("Serial"), wxDefaultPosition,
                        wxDefaultSize, wxRB_GROUP);
  m_rbTypeSerial->SetValue(TRUE);
  bSizer15->Add(m_rbTypeSerial, 0, wxALL, 5);

  m_rbTypeNet = new wxRadioButton(m_scrolledwin, wxID_ANY, _("Network"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  bSizer15->Add(m_rbTypeNet, 0, wxALL, 5);

  m_rbTypeCAN = new wxRadioButton(m_scrolledwin, wxID_ANY, "socketCAN",
                                  wxDefaultPosition, wxDefaultSize, 0);
 #if defined(__linux__) && !defined(__ANDROID__) && !defined(__WXOSX__)
  bSizer15->Add(m_rbTypeCAN, 0, wxALL, 5);
 #else
  m_rbTypeCAN->Hide();
 #endif

   if (OCPNPlatform::hasInternalGPS()) {
 
    m_rbTypeInternalGPS =
        new wxRadioButton(m_scrolledwin, wxID_ANY, _("Built-in GPS"),
                          wxDefaultPosition, wxDefaultSize, 0);
    bSizer15->Add(m_rbTypeInternalGPS, 0, wxALL, 5);
  } else
    m_rbTypeInternalGPS = NULL;

  // has built-in Bluetooth
   if (OCPNPlatform::hasInternalBT()) {
 
    m_rbTypeInternalBT =
        new wxRadioButton(m_scrolledwin, wxID_ANY, _("Built-in Bluetooth SPP"),
                          wxDefaultPosition, wxDefaultSize, 0);
    bSizer15->Add(m_rbTypeInternalBT, 0, wxALL, 5);




    m_buttonScanBT = new wxButton(m_scrolledwin, wxID_ANY, _("BT Scan"),
                                  wxDefaultPosition, wxDefaultSize);
    m_buttonScanBT->Hide();

    //     wxBoxSizer* bSizer15a = new wxBoxSizer(wxHORIZONTAL);
    //     sbSizerConnectionProps->Add(bSizer15a, 0, wxEXPAND, 5);

    sbSizerConnectionProps->Add(m_buttonScanBT, 0, wxALL, 5);

    m_stBTPairs =
        new wxStaticText(m_scrolledwin, wxID_ANY, _("Bluetooth Data Sources"),
                         wxDefaultPosition, wxDefaultSize, 0);
    m_stBTPairs->Wrap(-1);
    m_stBTPairs->Hide();
    sbSizerConnectionProps->Add(m_stBTPairs, 0, wxALL, 5);

    wxArrayString mt;
    mt.Add("unscanned");
    
    int ref_size = m_scrolledwin->GetCharWidth();
    m_choiceBTDataSources =
        new wxChoice(m_scrolledwin, wxID_ANY, wxDefaultPosition,
                      wxSize(40 * ref_size, 2 * ref_size), mt);
    //m_choiceBTDataSources->Bind(wxEVT_MOUSEWHEEL, &ConnectionEditDialog::OnWheelChoice, this);
    m_choiceBTDataSources->SetSelection(0);
    m_choiceBTDataSources->Hide();
    sbSizerConnectionProps->Add(m_choiceBTDataSources, 1, wxEXPAND | wxTOP,
                                5);

  } else
    m_rbTypeInternalBT = NULL;

  gSizerNetProps = new wxFlexGridSizer(0, 4, 0, 0);


  sbSizerConnectionProps->Add(gSizerNetProps, 0, wxEXPAND, 5);

  m_stNetProto = new wxStaticText(m_scrolledwin, wxID_ANY, _("Network Protocol"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  m_stNetProto->Wrap(-1);
  gSizerNetProps->Add(m_stNetProto, 0, wxALL, 5);

  wxBoxSizer* bSizer16;
  bSizer16 = new wxBoxSizer(wxHORIZONTAL);

  m_rbNetProtoTCP =
      new wxRadioButton(m_scrolledwin, wxID_ANY, _("TCP"), wxDefaultPosition,
                        wxDefaultSize, wxRB_GROUP);
  m_rbNetProtoTCP->Enable(TRUE);
  m_rbNetProtoTCP->SetValue(TRUE);

  bSizer16->Add(m_rbNetProtoTCP, 0, wxALL, 5);

  m_rbNetProtoUDP = new wxRadioButton(m_scrolledwin, wxID_ANY, _("UDP"),
                                      wxDefaultPosition, wxDefaultSize, 0);
  m_rbNetProtoUDP->Enable(TRUE);

  bSizer16->Add(m_rbNetProtoUDP, 0, wxALL, 5);

  m_rbNetProtoGPSD = new wxRadioButton(m_scrolledwin, wxID_ANY, _("GPSD"),
                                       wxDefaultPosition, wxDefaultSize, 0);
  m_rbNetProtoGPSD->Enable(TRUE);
  bSizer16->Add(m_rbNetProtoGPSD, 0, wxALL, 5);

  m_rbNetProtoSignalK = new wxRadioButton(m_scrolledwin, wxID_ANY, _("Signal K"),
                                          wxDefaultPosition, wxDefaultSize, 0);
  m_rbNetProtoSignalK->Enable(TRUE);
  bSizer16->Add(m_rbNetProtoSignalK, 0, wxALL, 5);


  gSizerNetProps->Add(bSizer16, 1, wxEXPAND, 5);
  gSizerNetProps->AddSpacer(1);
  gSizerNetProps->AddSpacer(1);


  m_stNetDataProtocol =
      new wxStaticText(m_scrolledwin, wxID_ANY, _("Data Protocol"),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_stNetDataProtocol->Wrap(-1);

  gSizerNetProps->Add(m_stNetDataProtocol, 0, wxALL, 5);

                          

  wxString m_choiceNetProtocolChoices[] = {_("NMEA 0183"), _("NMEA 2000")};
  int m_choiceNetProtocolNChoices =
      sizeof(m_choiceNetProtocolChoices) / sizeof(wxString);
  m_choiceNetDataProtocol =
      new wxChoice(m_scrolledwin, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choiceNetProtocolNChoices, m_choiceNetProtocolChoices, 0);
  //m_choiceNetDataProtocol->Bind(wxEVT_MOUSEWHEEL, &ConnectionEditDialog::OnWheelChoice, this);
  m_choiceNetDataProtocol->SetSelection(0);
  m_choiceNetDataProtocol->Enable(TRUE);


  gSizerNetProps->Add(m_choiceNetDataProtocol, 1, wxEXPAND | wxTOP, 5);
  gSizerNetProps->AddSpacer(1);
  gSizerNetProps->AddSpacer(1);

    m_stNetAddr = new wxStaticText(m_scrolledwin, wxID_ANY, _("Address"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_stNetAddr->Wrap(-1);
  int column1width = 16 * m_stNetProto->GetCharWidth();
  m_stNetAddr->SetMinSize(wxSize(column1width, -1));
  gSizerNetProps->Add(m_stNetAddr, 0, wxALL, 5);

  m_tNetAddress = new wxTextCtrl(m_scrolledwin, wxID_ANY, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize, 0);
  int column2width = 40 * m_scrolledwin->GetCharWidth();
  m_tNetAddress->SetMaxSize(wxSize(column2width, -1));
  m_tNetAddress->SetMinSize(wxSize(column2width, -1));

  gSizerNetProps->Add(m_tNetAddress, 0, wxEXPAND | wxTOP, 5);
  gSizerNetProps->AddSpacer(1);
  gSizerNetProps->AddSpacer(1);

  m_stNetPort = new wxStaticText(m_scrolledwin, wxID_ANY, _("DataPort"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_stNetPort->Wrap(-1);
  gSizerNetProps->Add(m_stNetPort, 0, wxALL, 5);

  m_tNetPort = new wxTextCtrl(m_scrolledwin, wxID_ANY, wxEmptyString,
                              wxDefaultPosition, wxDefaultSize, 0);
    gSizerNetProps->Add(m_tNetPort, 1, wxEXPAND | wxTOP, 5);
  gSizerNetProps->AddSpacer(1);
    gSizerNetProps->AddSpacer(1);
  

 
  gSizerCanProps = new wxGridSizer(0, 1, 0, 0);

  wxFlexGridSizer* fgSizer1C;
  fgSizer1C = new wxFlexGridSizer(0, 2, 0, 0);
  // fgSizer1C->SetFlexibleDirection(wxBOTH);
  // fgSizer1C->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);


  m_stCANSource = new wxStaticText(m_scrolledwin, wxID_ANY, _("socketCAN Source"),
                                   wxDefaultPosition, wxDefaultSize, 0);
  m_stCANSource->Wrap(-1);
  m_stCANSource->SetMinSize(wxSize(column1width, -1));
  fgSizer1C->Add(m_stCANSource, 0, wxALL, 5);

  wxArrayString choices = GetAvailableSocketCANInterfaces();
  m_choiceCANSource = new wxChoice(m_scrolledwin, wxID_ANY, wxDefaultPosition,
                                   wxDefaultSize, choices);

  m_choiceCANSource->SetSelection(0);
  m_choiceCANSource->Enable(TRUE);
  m_choiceCANSource->SetMaxSize(wxSize(column2width, -1));
  m_choiceCANSource->SetMinSize(wxSize(column2width, -1));
  fgSizer1C->Add(m_choiceCANSource, 1, wxEXPAND | wxTOP, 5);

  gSizerCanProps->Add(fgSizer1C, 0, wxEXPAND, 5);

  sbSizerConnectionProps->Add(gSizerCanProps, 0, wxEXPAND, 5);

  gSizerSerProps = new wxGridSizer(0, 1, 0, 0);
  sbSizerConnectionProps->Add(gSizerSerProps, 0, wxEXPAND, 5);

  wxFlexGridSizer* fgSizer1;
  fgSizer1 = new wxFlexGridSizer(0, 4, 0, 0);
  fgSizer1->SetFlexibleDirection(wxBOTH);
  fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_stSerPort =
      new wxStaticText(m_scrolledwin, wxID_ANY, _("Data port"),
                       wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
  m_stSerPort->SetMinSize(wxSize(column1width, -1));
  m_stSerPort->Wrap(-1);
 
  fgSizer1->Add(m_stSerPort, 0, wxALL, 5);

  m_comboPort = new wxComboBox(m_scrolledwin, wxID_ANY, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize, 0, NULL, 0);

  m_comboPort->SetMaxSize(wxSize(column2width, -1));
  m_comboPort->SetMinSize(wxSize(column2width, -1));

  fgSizer1->Add(m_comboPort, 0, wxEXPAND | wxTOP, 5);

  m_stSerBaudrate = new wxStaticText(m_scrolledwin, wxID_ANY, _("Baudrate"),
                                     wxDefaultPosition, wxDefaultSize, 0);
  m_stSerBaudrate->Wrap(-1);
  fgSizer1->AddSpacer(1);
  fgSizer1->AddSpacer(1);
  fgSizer1->Add(m_stSerBaudrate, 0, wxALL, 5);

  wxString m_choiceBaudRateChoices[] = {
      _("150"),    _("300"),    _("600"),    _("1200"),  _("2400"),
      _("4800"),   _("9600"),   _("19200"),  _("38400"), _("57600"),
      _("115200"), _("230400"), _("460800"), _("921600")};
  int m_choiceBaudRateNChoices =
      sizeof(m_choiceBaudRateChoices) / sizeof(wxString);
  m_choiceBaudRate =
      new wxChoice(m_scrolledwin, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choiceBaudRateNChoices, m_choiceBaudRateChoices, 0);
  //m_choiceBaudRate->Bind(wxEVT_MOUSEWHEEL, &ConnectionEditDialog::OnWheelChoice, this);

  m_choiceBaudRate->SetSelection(0);

  fgSizer1->Add(m_choiceBaudRate, 1, wxEXPAND | wxTOP, 5);
  fgSizer1->AddSpacer(1);
  fgSizer1->AddSpacer(1);

  m_stSerProtocol = new wxStaticText(m_scrolledwin, wxID_ANY, _("Protocol"),
                                     wxDefaultPosition, wxDefaultSize, 0);
  m_stSerProtocol->Wrap(-1);
  fgSizer1->Add(m_stSerProtocol, 0, wxALL, 5);

  wxString m_choiceSerialProtocolChoices[] = {_("NMEA 0183"), _("NMEA 2000")};
  int m_choiceSerialProtocolNChoices =
      sizeof(m_choiceSerialProtocolChoices) / sizeof(wxString);
  m_choiceSerialProtocol = new wxChoice(
      m_scrolledwin, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      m_choiceSerialProtocolNChoices, m_choiceSerialProtocolChoices, 0);
  //m_choiceSerialProtocol->Bind(wxEVT_MOUSEWHEEL, &ConnectionEditDialog::OnWheelChoice, this);

  m_choiceSerialProtocol->SetSelection(0);
  m_choiceSerialProtocol->Enable(TRUE);
  fgSizer1->Add(m_choiceSerialProtocol, 1, wxEXPAND | wxTOP, 5);



  gSizerSerProps->Add(fgSizer1, 0, wxEXPAND, 5);

  //  User Comments
  
  wxFlexGridSizer* commentSizer = new wxFlexGridSizer(0,2,0,0);
  // sbSizerConnectionProps->Add(commentSizer, 0, wxEXPAND, 5);

    //  Net User Comments
  m_stNetComment = new wxStaticText(m_scrolledwin, wxID_ANY, _("User Comment"),
                                    wxDefaultPosition, wxDefaultSize, 0);
  m_stNetComment->Wrap(-1);
  m_stNetComment->SetMinSize(wxSize(column1width, -1));
  commentSizer->Add(m_stNetComment, 0, wxALL, 5);

  m_tNetComment = new wxTextCtrl(m_scrolledwin, wxID_ANY, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_tNetComment->SetMaxSize(wxSize(column2width, -1));
  m_tNetComment->SetMinSize(wxSize(column2width, -1));

  commentSizer->Add(m_tNetComment, 1, wxEXPAND | wxTOP, 5);


     //  Serial User Comments
  m_stSerialComment = new wxStaticText(m_scrolledwin, wxID_ANY, _("User Comment"),
                                       wxDefaultPosition, wxDefaultSize, 0);
  m_stSerialComment->Wrap(-1);
  m_stSerialComment->SetMinSize(wxSize(column1width, -1));
  commentSizer->Add(m_stSerialComment, 0, wxALL, 5);

  m_tSerialComment = new wxTextCtrl(m_scrolledwin, wxID_ANY, wxEmptyString,
                                    wxDefaultPosition, wxDefaultSize, 0);
  m_tSerialComment->SetMaxSize(wxSize(column2width, -1));
  m_tSerialComment->SetMinSize(wxSize(column2width, -1));

  commentSizer->Add(m_tSerialComment, 1, wxTOP, 5);

  m_stPriority = new wxStaticText(m_scrolledwin, wxID_ANY, _("List position"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  m_stPriority->Wrap(-1);
  commentSizer->Add(m_stPriority, 0, wxALL, 5);

  wxString m_choicePriorityChoices[] = {_("0"), _("1"), _("2"), _("3"), _("4"),
                                        _("5"), _("6"), _("7"), _("8"), _("9")};
  int m_choicePriorityNChoices =
      sizeof(m_choicePriorityChoices) / sizeof(wxString);
  m_choicePriority =
      new wxChoice(m_scrolledwin, wxID_ANY, wxDefaultPosition,
                   wxSize(8 * m_parent->GetCharWidth(), -1),
                   m_choicePriorityNChoices, m_choicePriorityChoices, 0);
  // m_choicePriority->Bind(wxEVT_MOUSEWHEEL, &ConnectionsDialog::OnWheelChoice,
  // this);

  m_choicePriority->SetSelection(9);
  commentSizer->Add(m_choicePriority, 0, wxEXPAND | wxTOP, 5);

  sbSizerConnectionProps->Add(commentSizer, 0, wxEXPAND, 5);

  wxFlexGridSizer* fgSizer5;
  fgSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizer5->SetFlexibleDirection(wxBOTH);
  fgSizer5->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
  sbSizerConnectionProps->Add(fgSizer5, 0, wxEXPAND, 5);

   
  m_cbInput =
      new wxCheckBox(m_scrolledwin, wxID_ANY, _("Receive Input on this Port"),
                     wxDefaultPosition, wxDefaultSize, 0);
  fgSizer5->Add(m_cbInput, 0, wxALL, 2);
  fgSizer5->AddSpacer(1);

  m_cbOutput =
      new wxCheckBox(m_scrolledwin, wxID_ANY,
                     wxString::Format("%s (%s)", _("Output on this port"),
                                      _("as autopilot or NMEA repeater")),
                     wxDefaultPosition, wxDefaultSize, 0);
  fgSizer5->Add(m_cbOutput, 0, wxALL, 2);
  fgSizer5->AddSpacer(1);

    // Authentication token
  
  m_stAuthToken = new wxStaticText(m_scrolledwin, wxID_ANY, _("Auth Token"),
                                   wxDefaultPosition, wxDefaultSize, 0);
  m_stAuthToken->SetMinSize(wxSize(column1width, -1));
  m_stAuthToken->Wrap(-1);
  fgSizer5->Add(m_stAuthToken, 0, wxALL, 5);
  m_stAuthToken->Hide();

  m_tAuthToken = new wxTextCtrl(m_scrolledwin, wxID_ANY, wxEmptyString,
                                wxDefaultPosition, wxDefaultSize, 0);
  m_tAuthToken->SetMinSize(wxSize(column2width, -1));
  fgSizer5->Add(m_tAuthToken, 1, wxEXPAND | wxTOP, 5);

  m_tAuthToken->Hide();


  m_cbCheckCRC = new wxCheckBox(m_scrolledwin, wxID_ANY, _("Control checksum"),
                                wxDefaultPosition, wxDefaultSize, 0);
  m_cbCheckCRC->SetValue(TRUE);
  m_cbCheckCRC->SetToolTip(
      _("If checked, only the sentences with a valid checksum are passed "
        "through"));
  fgSizer5->Add(m_cbCheckCRC, 0, wxALL, 2);
  fgSizer5->AddSpacer(1);

  m_cbGarminHost = new wxCheckBox(m_scrolledwin, wxID_ANY,
                                  _("Use Garmin (GRMN) mode for input"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  m_cbGarminHost->SetValue(FALSE);
  fgSizer5->Add(m_cbGarminHost, 0, wxALL, 2);
  fgSizer5->AddSpacer(1);

#ifndef USE_GARMINHOST
  m_cbGarminHost->Hide();
#endif



  m_cbMultiCast =
    new wxCheckBox(m_scrolledwin, wxID_ANY, _(" UDP Multicast"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_cbMultiCast->SetValue(FALSE);
  // m_cbMultiCast->SetToolTip(_("Advanced Use Only. Enable UDP Multicast."));

   fgSizer5->Add(m_cbMultiCast, 0, wxALL, 2);
   fgSizer5->AddSpacer(1);

  m_stPrecision =
      new wxStaticText(m_scrolledwin, wxID_ANY, _("APB bearing precision"),
                       wxDefaultPosition, wxDefaultSize, 0);

  m_stPrecision->Wrap(-1);
  fgSizer5->Add(m_stPrecision, 0, wxALL, 2);

  wxString m_choicePrecisionChoices[] = {_("x"), _("x.x"), _("x.xx"),
                                         _("x.xxx"), _("x.xxxx")};
  int m_choicePrecisionNChoices =
      sizeof(m_choicePrecisionChoices) / sizeof(wxString);
  m_choicePrecision =
      new wxChoice(m_scrolledwin, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choicePrecisionNChoices, m_choicePrecisionChoices, 0);
  //m_choicePrecision->Bind(wxEVT_MOUSEWHEEL, &ConnectionEditDialog::OnWheelChoice, this);

  m_choicePrecision->SetSelection(g_NMEAAPBPrecision);
  fgSizer5->Add(m_choicePrecision, 0, wxALL, 2);

  // signalK discovery enable
  m_cbCheckSKDiscover =
      new wxCheckBox(m_scrolledwin, wxID_ANY, _("Automatic server discovery"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbCheckSKDiscover->SetValue(TRUE);
  m_cbCheckSKDiscover->SetToolTip(
      _("If checked, signal K server will be discovered automatically"));
  
  fgSizer5->Add(m_cbCheckSKDiscover, 0, wxALL, 2);

  // signal K "Discover now" button
  m_ButtonSKDiscover = new wxButton(m_scrolledwin, wxID_ANY, _("Discover now..."),
                                    wxDefaultPosition, wxDefaultSize, 0);
  m_ButtonSKDiscover->Hide();
  fgSizer5->Add(m_ButtonSKDiscover, 0, wxALL, 2);

  // signalK Server Status
  m_StaticTextSKServerStatus = new wxStaticText(
      m_scrolledwin, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0);
  fgSizer5->Add(m_StaticTextSKServerStatus, 0, wxALL, 2);


  sbSizerInFilter = new wxStaticBoxSizer(
      new wxStaticBox(m_scrolledwin, wxID_ANY, _("Input filtering")), wxVERTICAL);
  sbSizerConnectionProps->Add(sbSizerInFilter, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer9;
  bSizer9 = new wxBoxSizer(wxHORIZONTAL);

  m_rbIAccept =
      new wxRadioButton(m_scrolledwin, wxID_ANY, _("Accept only sentences"),
                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  bSizer9->Add(m_rbIAccept, 0, wxALL, 5);

  m_rbIIgnore = new wxRadioButton(m_scrolledwin, wxID_ANY, _("Ignore sentences"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  bSizer9->Add(m_rbIIgnore, 0, wxALL, 5);

  sbSizerInFilter->Add(bSizer9, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxHORIZONTAL);
  sbSizerInFilter->Add(bSizer11, 0, wxEXPAND, 5);

  m_tcInputStc =
      new wxTextCtrl(m_scrolledwin, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_READONLY);
  bSizer11->Add(m_tcInputStc, 1, wxALL | wxEXPAND, 5);

  m_btnInputStcList =
      new wxButton(m_scrolledwin, wxID_ANY, "...", wxDefaultPosition,
                   wxDefaultSize, wxBU_EXACTFIT);
  bSizer11->Add(m_btnInputStcList, 0, wxALL, 5);

  bSizer11->AddSpacer(GetCharWidth() * 5);



  sbSizerOutFilter = new wxStaticBoxSizer(
      new wxStaticBox(m_scrolledwin, wxID_ANY, _("Output filtering")),
      wxVERTICAL);
  sbSizerConnectionProps->Add(sbSizerOutFilter, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer10;
  bSizer10 = new wxBoxSizer(wxHORIZONTAL);

  m_rbOAccept =
      new wxRadioButton(m_scrolledwin, wxID_ANY, _("Transmit sentences"),
                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  bSizer10->Add(m_rbOAccept, 0, wxALL, 5);

  m_rbOIgnore = new wxRadioButton(m_scrolledwin, wxID_ANY, _("Drop sentences"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  bSizer10->Add(m_rbOIgnore, 0, wxALL, 5);

  sbSizerOutFilter->Add(bSizer10, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer12;
  bSizer12 = new wxBoxSizer(wxHORIZONTAL);
  sbSizerOutFilter->Add(bSizer12, 0, wxEXPAND, 5);

  m_tcOutputStc =
      new wxTextCtrl(m_scrolledwin, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_READONLY);
  bSizer12->Add(m_tcOutputStc, 1, wxALL | wxEXPAND, 5);

  m_btnOutputStcList =
      new wxButton(m_scrolledwin, wxID_ANY, "...", wxDefaultPosition,
                   wxDefaultSize, wxBU_EXACTFIT);
  bSizer12->Add(m_btnOutputStcList, 0, wxALL, 5);

  bSizer12->AddSpacer(GetCharWidth() * 5);

 
  sbSizerConnectionProps->AddSpacer(20);

  m_more = new wxStaticText(m_scrolledwin, wxID_ANY, "4 chars",
                            wxDefaultPosition,
                            wxDefaultSize, wxALIGN_LEFT);
  m_more->SetLabelMarkup(MORE);
  sbSizerConnectionProps->Add(m_more,wxSizerFlags());

  ConnectControls();

  SetInitialSettings();

  ShowTypeCommon();

  ShowNMEACommon(true);
  ShowNMEASerial(true);
  ShowNMEANet(false);
  ShowNMEACAN(false);
  connectionsaved = TRUE;

  GetSizer()->Fit(m_scrolledwin);

  new_device_listener.Init(SystemEvents::GetInstance().evt_dev_change,
                           [&](ObservedEvt&) { LoadSerialPorts(m_comboPort); });


}


//void ConnectionEditDialog::OnWheelChoice(wxMouseEvent& event) {
//  return;
//}

void ConnectionEditDialog::SetSelectedConnectionPanel(
    ConnectionParamsPanel* panel) {
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
    m_sbConnEdit->SetLabel("");
    ClearNMEAForm();
  }

  //  Scroll the panel to allow the user to see more of the NMEA parameter
  //  settings area
  //wxPoint buttonPosition = m_buttonAdd->GetPosition();
  //this->Scroll(-1, buttonPosition.y / m_parent->GetScrollRate());
}

void ConnectionEditDialog::SetPropsLabel(wxString label) {
  m_sbConnEdit->SetLabel(label);
}

void ConnectionEditDialog::EnableConnection(ConnectionParams* conn, bool value) {
  if (conn) {
    conn->bEnabled = value;
    conn->b_IsSetup = FALSE;  // trigger a rebuild/takedown of the connection
    m_connection_enabled = conn->bEnabled;
  }
}

#if 1
void ConnectionEditDialog::OnValChange(wxCommandEvent& event) { event.Skip(); }

void ConnectionEditDialog::OnScanBTClick(wxCommandEvent& event) {
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

void ConnectionEditDialog::onBTScanTimer(wxTimerEvent& event) {
  if (m_BTscanning) {
    m_BTscanning++;

    m_BTscan_results = g_Platform->getBluetoothScanResults();

    m_choiceBTDataSources->Clear();
    m_choiceBTDataSources->Append(m_BTscan_results[0]);  // scan status

    unsigned int i = 1;
    while ((i + 1) < m_BTscan_results.GetCount()) {
      wxString item1 = m_BTscan_results[i] + ";";
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

void ConnectionEditDialog::StopBTScan(void) {
  m_BTScanTimer.Stop();

  g_Platform->stopBluetoothScan();

  m_BTscanning = 0;

  if (m_buttonScanBT) {
    m_buttonScanBT->SetLabel(_("BT Scan"));
    m_buttonScanBT->Enable();
  }
}

void ConnectionEditDialog::OnConnValChange(wxCommandEvent& event) {
  connectionsaved = FALSE;
  event.Skip();
}

void ConnectionEditDialog::OnTypeSerialSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToSerial();
}

void ConnectionEditDialog::OnTypeNetSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToNet();
}

void ConnectionEditDialog::OnTypeCANSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToCAN();
}

void ConnectionEditDialog::OnTypeGPSSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToGPS();
}

void ConnectionEditDialog::OnTypeBTSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToBT();
}

void ConnectionEditDialog::OnUploadFormatChange(wxCommandEvent& event) {
  if (event.GetEventObject() == m_cbGarminUploadHost && event.IsChecked())
    m_cbFurunoGP3X->SetValue(FALSE);
  else if (event.GetEventObject() == m_cbFurunoGP3X && event.IsChecked())
    m_cbGarminUploadHost->SetValue(FALSE);

  OnConnValChange(event);
  event.Skip();
}
#endif

void ConnectionEditDialog::ShowTypeCommon(bool visible) {
  m_rbTypeSerial->Show(visible);
  m_rbTypeNet->Show(visible);
#if defined(__linux__) && !defined(__ANDROID__) && !defined(__WXOSX__)
  m_rbTypeCAN->Show(visible);
#endif
  if (m_rbTypeInternalGPS) m_rbTypeInternalGPS->Show(visible);
  if (m_rbTypeInternalBT) m_rbTypeInternalBT->Show(visible);
}

void ConnectionEditDialog::ShowNMEACommon(bool visible) {
  bool advanced = m_advanced;
  m_cbInput->Show(visible);
  m_cbOutput->Show(visible);
  m_stPrecision->Show(visible && advanced);
  m_choicePrecision->Show(visible && advanced);
  m_stPrecision->Show(visible && advanced);
  m_cbCheckCRC->Show(visible && advanced);
  m_stAuthToken->Show(visible && advanced);
  m_tAuthToken->Show(visible && advanced);
  if (visible) {
    const bool bout_enable = (m_cbOutput->IsChecked() && advanced);
    m_stPrecision->Enable(bout_enable);
    m_choicePrecision->Enable(bout_enable);
  } else {
    sbSizerOutFilter->SetDimension(0, 0, 0, 0);
    sbSizerInFilter->SetDimension(0, 0, 0, 0);
    sbSizerConnectionProps->SetDimension(0, 0, 0, 0);
    m_sbConnEdit->SetLabel("");
  }

  m_cbCheckSKDiscover->Hide();  // Provisional
  m_ButtonSKDiscover->Hide();

  const bool bin_enable = (m_cbInput->IsChecked() && advanced);
  ShowInFilter(visible && bin_enable);
  const bool bout_enable = (m_cbOutput->IsChecked() && advanced);
  ShowOutFilter(visible && bout_enable);

  m_bNMEAParams_shown = visible;
}

void ConnectionEditDialog::ShowNMEANet(bool visible) {
  bool advanced = m_advanced;
  m_stNetAddr->Show(visible);
  m_tNetAddress->Show(visible);
  m_stNetDataProtocol->Show(visible);
  m_stNetPort->Show(visible);
  m_choiceNetDataProtocol->Show(visible);
  m_tNetPort->Show(visible);
  m_stNetProto->Show(visible);
  m_rbNetProtoSignalK->Show(visible);
  m_rbNetProtoGPSD->Show(visible);
  m_rbNetProtoTCP->Show(visible);
  m_rbNetProtoUDP->Show(visible);
  m_stNetComment->Show(visible);
  m_tNetComment->Show(visible);
  m_stAuthToken->Show(visible);
  m_tAuthToken->Show(visible);
  m_cbGarminHost->Hide();
}

void ConnectionEditDialog::ShowNMEASerial(bool visible) {
  bool advanced = m_advanced;
  m_stSerBaudrate->Show(visible);
  m_choiceBaudRate->Show(visible);
  m_stSerPort->Show(visible);
  m_comboPort->Show(visible);
  m_stSerProtocol->Show(visible);
  m_choiceSerialProtocol->Show(visible);
  m_cbGarminHost->Show(visible && advanced);
  m_stSerialComment->Show(visible);
  m_tSerialComment->Show(visible);
}

void ConnectionEditDialog::ShowNMEAGPS(bool visible) {
  m_cbCheckSKDiscover->Hide();
  m_ButtonSKDiscover->Hide();
  m_stAuthToken->Hide();
  m_tAuthToken->Hide();
  m_cbOutput->Hide();
 
}

void ConnectionEditDialog::ShowNMEACAN(bool visible) {
  m_stCANSource->Show(visible);
  m_choiceCANSource->Show(visible);
  
}

void ConnectionEditDialog::ShowNMEABT(bool visible) {
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
  m_cbCheckSKDiscover->Hide();
  m_stAuthToken->Hide();
  m_tAuthToken->Hide();
  m_cbCheckSKDiscover->Hide();    // Provisional
  m_ButtonSKDiscover->Hide();
  m_tcOutputStc->Show(visible);
  m_btnOutputStcList->Show(visible);
  m_cbOutput->Show(visible);
}

void ConnectionEditDialog::SetNMEAFormToSerial(void) {
  bool advanced = m_advanced;
  ShowNMEACommon(TRUE);
  ShowNMEANet(FALSE);
  ShowNMEAGPS(FALSE);
  ShowNMEABT(FALSE);
  ShowNMEASerial(TRUE);
  ShowNMEACAN(FALSE);
  SetDSFormRWStates();
  LayoutDialog();
}

void ConnectionEditDialog::SetNMEAFormToNet(void) {
  bool advanced = m_advanced;
  ShowNMEACommon(TRUE);
  ShowNMEANet(TRUE);
  ShowNMEAGPS(FALSE);
  ShowNMEABT(FALSE);
  ShowNMEASerial(FALSE);
  ShowNMEACAN(FALSE);
  SetUDPNetAddressVisiblity();
  SetDSFormRWStates();

  LayoutDialog();
}

void ConnectionEditDialog::SetNMEAFormToCAN(void) {
  bool advanced = m_advanced;
  ShowNMEACommon(FALSE);
  ShowNMEANet(FALSE);
  ShowNMEAGPS(FALSE);
  ShowNMEABT(FALSE);
  ShowNMEASerial(FALSE);
  ShowNMEACAN(TRUE);
  sbSizerInFilter->Show(false);
  sbSizerOutFilter->Show(false);
  SetDSFormRWStates();

  LayoutDialog();
}

void ConnectionEditDialog::SetNMEAFormToGPS(void) {
  ShowNMEACommon(TRUE);
  ShowNMEANet(FALSE);
  ShowNMEAGPS(TRUE);
  ShowNMEABT(FALSE);
  ShowNMEASerial(FALSE);
  ShowNMEACAN(FALSE);

  //m_container->FitInside();
  // Fit();
  SetDSFormRWStates();
  LayoutDialog();
}

void ConnectionEditDialog::SetNMEAFormToBT(void) {
  m_rbNetProtoUDP->SetValue(true);
  ShowNMEACommon(TRUE);
  ShowNMEANet(FALSE);
  ShowNMEAGPS(FALSE);
  ShowNMEABT(TRUE);
  ShowNMEASerial(FALSE);
  ShowNMEACAN(FALSE);

  //m_container->FitInside();
  // Fit();
  SetDSFormRWStates();
  LayoutDialog();
}

void ConnectionEditDialog::ClearNMEAForm(void) {
  ShowNMEACommon(FALSE);
  ShowNMEANet(FALSE);
  ShowNMEAGPS(FALSE);
  ShowNMEABT(FALSE);
  ShowNMEASerial(FALSE);
  ShowNMEACAN(FALSE);

  //m_container->FitInside();
  // Fit();
}

void ConnectionEditDialog::SetDSFormOptionVizStates(void) {
  bool advanced = m_advanced;
  m_more->Show(true);
  m_cbInput->Show();
  m_cbOutput->Show();
  m_cbCheckCRC->Show(advanced);
  m_stPrecision->Show(true);
  m_choicePrecision->Show(true);

  ShowInFilter(advanced);
  ShowOutFilter(advanced);
  // Discovery hidden until it works.
  // m_cbCheckSKDiscover->Show();
  m_stAuthToken->Show(advanced);
  m_tAuthToken->Show(advanced);
  // m_ButtonSKDiscover->Show();
  m_StaticTextSKServerStatus->Show(advanced);

  if (m_rbTypeSerial->GetValue()) {
    m_cbMultiCast->Hide();
    m_cbCheckSKDiscover->Hide();
    m_stAuthToken->Hide();
    m_tAuthToken->Hide();
    m_ButtonSKDiscover->Hide();
    m_StaticTextSKServerStatus->Hide();
    bool n0183ctlenabled =
        (DataProtocol)m_choiceSerialProtocol->GetSelection() ==
        DataProtocol::PROTO_NMEA0183;
    if (!n0183ctlenabled) {
      m_cbInput->Hide();
      m_cbOutput->Hide();
      ShowOutFilter(false);
      ShowInFilter(false);
      m_stPrecision->Hide();
      m_choicePrecision->Hide();
      m_cbCheckCRC->Hide();
      m_stNetDataProtocol->Hide();
      m_choiceNetDataProtocol->Hide();
    }
    else {
      m_cbInput->Show();
      m_cbInput->Enable();

      ShowInFilter(m_cbInput->IsChecked() && advanced);
      ShowOutFilter(m_cbOutput->IsChecked() && advanced);

      m_stPrecision->Show(m_cbOutput->IsChecked() && advanced);
      m_choicePrecision->Show(m_cbOutput->IsChecked() && advanced);

      m_cbGarminHost->Show(m_cbInput->IsChecked() && advanced);
    }
  }

  if (m_rbTypeInternalGPS && m_rbTypeInternalGPS->GetValue()) {
    m_cbMultiCast->Hide();
    m_cbCheckSKDiscover->Hide();
    m_stAuthToken->Hide();
    m_tAuthToken->Hide();
    m_ButtonSKDiscover->Hide();
    m_StaticTextSKServerStatus->Hide();
    m_cbOutput->Hide();
    m_cbInput->Hide();
    ShowOutFilter( false );
    ShowInFilter(false);
    m_stPrecision->Hide();
    m_choicePrecision->Hide();
    m_cbCheckCRC->Hide();
    m_cbGarminHost->Hide();
    m_more->Hide();
    
  }

  if (m_rbTypeInternalBT && m_rbTypeInternalBT->GetValue()) {
    m_cbMultiCast->Hide();
    m_cbCheckSKDiscover->Hide();
    m_stAuthToken->Hide();
    m_tAuthToken->Hide();
    m_ButtonSKDiscover->Hide();
    m_StaticTextSKServerStatus->Hide();

    ShowInFilter(m_cbInput->IsChecked() && advanced);
    ShowOutFilter(m_cbOutput->IsChecked() && advanced);

    m_stPrecision->Show(m_cbOutput->IsChecked() && advanced);
    m_choicePrecision->Show(m_cbOutput->IsChecked() && advanced);
  }

  if (m_rbTypeCAN->GetValue()) {
    m_cbMultiCast->Hide();
    m_cbCheckSKDiscover->Hide();
    m_stAuthToken->Hide();
    m_tAuthToken->Hide();
    m_ButtonSKDiscover->Hide();
    m_StaticTextSKServerStatus->Hide();
    m_cbGarminHost->Hide();
    m_cbInput->Hide();
    m_cbOutput->Hide();

    ShowInFilter(false);
    ShowOutFilter(false);

    m_stPrecision->Hide();
    m_choicePrecision->Hide();
    m_cbCheckCRC->Hide();

    m_stNetDataProtocol->Hide();
    m_choiceNetDataProtocol->Hide();
    m_more->Hide();
  }

  if (m_rbTypeNet->GetValue()) {
    if (m_rbNetProtoGPSD->GetValue()) {
      m_cbMultiCast->Hide();
      m_cbCheckSKDiscover->Hide();
      m_cbInput->Hide();
      m_cbOutput->Hide();
      ShowInFilter(false);
      ShowOutFilter(false);
      m_cbCheckCRC->Hide();
      m_stPrecision->Hide();
      m_choicePrecision->Hide();
      m_ButtonSKDiscover->Hide();
      m_StaticTextSKServerStatus->Hide();
      m_stAuthToken->Hide();
      m_tAuthToken->Hide();
      m_stNetDataProtocol->Hide();
      m_choiceNetDataProtocol->Hide();
      m_cbGarminHost->Hide();
      m_more->Hide();


    } else if (m_rbNetProtoSignalK->GetValue()) {
      m_cbMultiCast->Hide();
      m_cbInput->Hide();
      m_cbOutput->Hide();
      ShowInFilter(false);
      ShowOutFilter(false);
      m_cbCheckCRC->Hide();
      m_stPrecision->Hide();
      m_choicePrecision->Hide();
      m_stNetDataProtocol->Hide();
      m_choiceNetDataProtocol->Hide();
      m_cbGarminHost->Hide();

    } else {    // tcp or udp
      m_stAuthToken->Hide();
      m_tAuthToken->Hide();
      m_cbCheckSKDiscover->Hide();
      m_ButtonSKDiscover->Hide();
      m_StaticTextSKServerStatus->Hide();
      m_stNetDataProtocol->Show();
      m_choiceNetDataProtocol->Show();
      m_cbGarminHost->Hide();

 //     if (m_rbNetProtoUDP->GetValue()) m_cbMultiCast->Show(advanced);

      if ((DataProtocol)m_choiceNetDataProtocol->GetSelection() ==
          DataProtocol::PROTO_NMEA2000) {
        m_cbCheckCRC->Hide();
        m_stPrecision->Hide();
        m_choicePrecision->Hide();

        ShowInFilter(false);
        ShowOutFilter(false);
        if (m_rbNetProtoTCP->GetValue()) m_more->Hide();
      }
      if ((DataProtocol)m_choiceNetDataProtocol->GetSelection() ==
          DataProtocol::PROTO_NMEA0183) {
        

        m_stPrecision->Show(advanced);
        m_choicePrecision->Show(advanced);
        m_stPrecision->Enable(m_cbOutput->IsChecked() && advanced);
        m_choicePrecision->Enable(m_cbOutput->IsChecked() && advanced);

        ShowInFilter(m_cbInput->IsChecked() && advanced);
        ShowOutFilter(m_cbOutput->IsChecked() && advanced);
      }

    }
  }
}

void ConnectionEditDialog::SetDSFormRWStates(void) {
  if (m_rbTypeSerial->GetValue()) {
    m_cbInput->Enable(FALSE);
    m_cbOutput->Enable(TRUE);
    ShowInFilter();
    ShowOutFilter(m_cbOutput->IsChecked());
  } else if (m_rbNetProtoGPSD->GetValue()) {
    if (m_tNetPort->GetValue() == wxEmptyString)
      m_tNetPort->SetValue(DEFAULT_GPSD_PORT);
    m_cbInput->SetValue(TRUE);
    m_cbInput->Enable(FALSE);
//    m_cbOutput->SetValue(FALSE);
    m_cbOutput->Enable(FALSE);
    m_rbOAccept->Enable(FALSE);
    m_rbOIgnore->Enable(FALSE);
    m_btnOutputStcList->Enable(FALSE);
  } else if (m_rbNetProtoSignalK->GetValue()) {
    if (m_tNetPort->GetValue() == wxEmptyString)
      m_tNetPort->SetValue(DEFAULT_SIGNALK_PORT);
    m_cbInput->SetValue(TRUE);
    m_cbInput->Enable(FALSE);
//   m_cbOutput->SetValue(FALSE);
    m_cbOutput->Enable(FALSE);
    m_rbOAccept->Enable(FALSE);
    m_rbOIgnore->Enable(FALSE);
    UpdateDiscoverStatus(wxEmptyString);
  } else {
    if (m_tNetPort->GetValue() == wxEmptyString) {
      if (m_rbNetProtoTCP->GetValue()) {
        m_tNetPort->SetValue(DEFAULT_TCP_PORT);
      } else {
        m_tNetPort->SetValue(DEFAULT_UDP_PORT);
      }
    }
    m_cbInput->Enable(TRUE);
    m_cbOutput->Enable(TRUE);
    m_rbOAccept->Enable(TRUE);
    m_rbOIgnore->Enable(TRUE);
    m_btnOutputStcList->Enable(TRUE);
  }

  SetDSFormOptionVizStates();
  //m_container->FitInside();
}

void ConnectionEditDialog::ShowInFilter(bool bshow){
  sbSizerInFilter->GetStaticBox()->Show(bshow);
  m_rbIAccept->Show(bshow);
  m_rbIIgnore->Show(bshow);
  m_tcInputStc->Show(bshow);
  m_btnInputStcList->Show(bshow);
}

void ConnectionEditDialog::ShowOutFilter(bool bshow){
  sbSizerOutFilter->GetStaticBox()->Show(bshow);
  m_rbOAccept->Show(bshow);
  m_rbOIgnore->Show(bshow);
  m_tcOutputStc->Show(bshow);
  m_btnOutputStcList->Show(bshow);
}

void ConnectionEditDialog::PreloadControls(ConnectionParams* cp) {
  SetConnectionParams(cp);
}

void ConnectionEditDialog::SetConnectionParams(ConnectionParams* cp) {
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
      m_choiceBaudRate->FindString(wxString::Format("%d", cp->Baudrate)));
  m_choiceSerialProtocol->Select(cp->Protocol);  // TODO
  m_choicePriority->Select(
      m_choicePriority->FindString(wxString::Format("%d", cp->Priority)));
  m_tNetAddress->SetValue(cp->NetworkAddress);

  m_choiceNetDataProtocol->Select(cp->Protocol);  // TODO

  m_cbMultiCast->SetValue(IsAddressMultiCast(m_tNetAddress->GetValue()));

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
    SetNMEAFormForSerialProtocol();
  } else if (cp->Type == NETWORK) {
    m_rbTypeNet->SetValue(TRUE);
    SetNMEAFormToNet();
  } else if (cp->Type == SOCKETCAN) {
    m_rbTypeCAN->SetValue(TRUE);
    SetNMEAFormToCAN();

  } else if (cp->Type == INTERNAL_GPS) {
    if (m_rbTypeInternalGPS) m_rbTypeInternalGPS->SetValue(TRUE);
    SetNMEAFormToGPS();
  } else if (cp->Type == INTERNAL_BT) {
    if (m_rbTypeInternalBT) m_rbTypeInternalBT->SetValue(TRUE);
    SetNMEAFormToBT();

    // Preset the source selector
    wxString bts = cp->NetworkAddress + ";" + cp->GetPortStr();
    m_choiceBTDataSources->Clear();
    m_choiceBTDataSources->Append(bts);
    m_choiceBTDataSources->SetSelection(0);
  } else
    ClearNMEAForm();

  if (cp->Type == SERIAL)
    m_tSerialComment->SetValue(cp->UserComment);
  else if (cp->Type == NETWORK)
    m_tNetComment->SetValue(cp->UserComment);

  m_tAuthToken->SetValue(cp->AuthToken);

  m_connection_enabled = cp->bEnabled;

  // Reset touch flag
  connectionsaved = true;
}

void ConnectionEditDialog::SetUDPNetAddressVisiblity(void) {
  if (m_rbNetProtoUDP->GetValue() && !m_cbMultiCast->IsChecked() &&
      !m_cbOutput->IsChecked()) {
//    m_stNetAddr->Show(FALSE);
//    m_tNetAddress->Show(FALSE);
    m_tNetAddress->Enable(TRUE);
  } else {
    m_stNetAddr->Show(TRUE);
    m_tNetAddress->Show(TRUE);
    m_tNetAddress->Enable(TRUE);
  }
  if (!m_rbNetProtoUDP->GetValue()) {
    m_stNetAddr->Show(TRUE);
    m_tNetAddress->Show(TRUE);
    m_tNetAddress->Enable(TRUE);
  }
  if (m_rbNetProtoUDP->GetValue() && m_advanced) {
   // m_cbMultiCast->Show();
  } else
    m_cbMultiCast->Hide();
}

void ConnectionEditDialog::SetDefaultConnectionParams(void) {
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
  m_choiceBaudRate->Select(m_choiceBaudRate->FindString("4800"));
  //    m_choiceSerialProtocol->Select( cp->Protocol ); // TODO
  m_choicePriority->Select(m_choicePriority->FindString("1"));

  m_tNetAddress->SetValue(DEFAULT_IP_ADDRESS);

  m_tNetComment->SetValue(wxEmptyString);
  m_tSerialComment->SetValue(wxEmptyString);
  m_tAuthToken->SetValue(wxEmptyString);

  bool bserial = TRUE;
#ifdef __WXGTK__
  bserial = FALSE;
#endif

#ifdef __WXOSX__
  bserial = FALSE;
#endif

#ifdef __ANDROID__
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
  m_rbTypeCAN->SetValue(FALSE);
#endif

  m_connection_enabled = TRUE;

  // Reset touch flag
  connectionsaved = false;
}

bool ConnectionEditDialog::SortSourceList(void) {
  if (TheConnectionParams()->Count() < 2) return false;

  std::vector<int> ivec;
  for (size_t i = 0; i < TheConnectionParams()->Count(); i++) ivec.push_back(i);

  bool did_sort = false;
  bool did_swap = true;
  while (did_swap) {
    did_swap = false;
    for (size_t j = 1; j < ivec.size(); j++) {
      ConnectionParams* c1 = TheConnectionParams()->Item(ivec[j]);
      ConnectionParams* c2 = TheConnectionParams()->Item(ivec[j - 1]);

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
          TheConnectionParams()->Item(ivec[i])->m_optionsPanel;
      boxSizerConnections->Add(pPanel, 0, wxEXPAND | wxALL, 0);
    }
  }

  return did_sort;
}

void ConnectionEditDialog::LayoutDialog() {
  gSizerNetProps->Layout();
  gSizerSerProps->Layout();
  m_scrolledwin->Layout();
  m_scrolledwin->Fit();
  GetSizer()->Layout();
}

void ConnectionEditDialog::UpdateSourceList(bool bResort) {
  for (size_t i = 0; i < TheConnectionParams()->Count(); i++) {
    ConnectionParams* cp = TheConnectionParams()->Item(i);
    ConnectionParamsPanel* panel = cp->m_optionsPanel;
    if (panel) panel->Update(TheConnectionParams()->Item(i));
  }

  if (bResort) {
    SortSourceList();
  }

  m_scrollWinConnections->Layout();
}

#if 0
void ConnectionEditDialog::OnAddDatasourceClick(wxCommandEvent& event) {
  //  Unselect all panels
  for (size_t i = 0; i < TheConnectionParams()->Count(); i++)
    TheConnectionParams()->Item(i)->m_optionsPanel->SetSelected(false);

  connectionsaved = FALSE;
  SetDefaultConnectionParams();

  m_sbConnEdit->SetLabel(_("Configure new connection"));

  m_buttonRemove->Hide();  // Disable();
  m_buttonAdd->Hide();     // Disable();

  m_rbTypeSerial->Show(true);
  m_rbTypeNet->Show(true);
#if defined(__linux__) && !defined(__ANDROID__) && !defined(__WXOSX__)
  m_rbTypeCAN->Show(true);
#endif

  // Default is always "serial"
  m_rbTypeSerial->SetValue(TRUE);
  SetDSFormRWStates();
  SetNMEAFormToSerial();


  //  Scroll the panel to allow the user to see more of the NMEA parameter
  //  settings area
  wxPoint buttonPosition = m_buttonAdd->GetPosition();
  //m_container->Scroll(-1, buttonPosition.y / m_parent->GetScrollRate());
}
#endif

void ConnectionEditDialog::OnSelectDatasource(wxListEvent& event) {
  SetConnectionParams(TheConnectionParams()->Item(event.GetData()));
  m_buttonRemove->Enable();
  m_buttonRemove->Show();
  event.Skip();
}

void ConnectionEditDialog::OnDiscoverButton(wxCommandEvent& event) {
#if 0  // FIXME (dave)
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

void ConnectionEditDialog::UpdateDiscoverStatus(wxString stat) {
  m_StaticTextSKServerStatus->SetLabel(stat);
}

void ConnectionEditDialog::OnBtnIStcs(wxCommandEvent& event) {
  const ListType type = m_rbIAccept->GetValue() ? WHITELIST : BLACKLIST;
  const wxArrayString list =
      wxStringTokenize(m_tcInputStc->GetValue(), ",");
  SentenceListDlg dlg(m_parent, FILTER_INPUT, type, list);

  if (dlg.ShowModal() == wxID_OK) m_tcInputStc->SetValue(dlg.GetSentences());
}

void ConnectionEditDialog::OnBtnOStcs(wxCommandEvent& event) {
  const ListType type = m_rbOAccept->GetValue() ? WHITELIST : BLACKLIST;
  const wxArrayString list =
      wxStringTokenize(m_tcOutputStc->GetValue(), ",");
  SentenceListDlg dlg(m_parent, FILTER_OUTPUT, type, list);

  if (dlg.ShowModal() == wxID_OK) m_tcOutputStc->SetValue(dlg.GetSentences());
}

void ConnectionEditDialog::OnNetProtocolSelected(wxCommandEvent& event) {
  if (m_rbNetProtoGPSD->GetValue()) {
    if (IsDefaultPort(m_tNetPort->GetValue())) {
      m_tNetPort->SetValue(DEFAULT_GPSD_PORT);
    }
    m_tNetAddress->SetValue(DEFAULT_IP_ADDRESS);
  } else if (m_rbNetProtoUDP->GetValue()) {
    if (IsDefaultPort(m_tNetPort->GetValue())) {
      m_tNetPort->SetValue(DEFAULT_UDP_PORT);
    }
    m_tNetAddress->SetValue(DEFAULT_IP_ADDRESS);
    if (m_cbInput->GetValue() && !m_cbMultiCast->GetValue() && m_rbNetProtoUDP->GetValue())
      m_tNetAddress->SetValue(DEFAULT_IP_ADDRESS);


  } else if (m_rbNetProtoSignalK->GetValue()) {
    if (IsDefaultPort(m_tNetPort->GetValue())) {
      m_tNetPort->SetValue(DEFAULT_SIGNALK_PORT);
    }
    m_tNetAddress->SetValue(DEFAULT_IP_ADDRESS);
  } else if (m_rbNetProtoTCP->GetValue()) {
    if (IsDefaultPort(m_tNetPort->GetValue())) {
      m_tNetPort->SetValue(DEFAULT_TCP_PORT);
    }
    m_tNetAddress->SetValue(DEFAULT_IP_ADDRESS);
  }


  SetUDPNetAddressVisiblity();
  SetDSFormRWStates();
  LayoutDialog();
  OnConnValChange(event);
}

void ConnectionEditDialog::OnRbAcceptInput(wxCommandEvent& event) {
  OnConnValChange(event);
}
void ConnectionEditDialog::OnRbIgnoreInput(wxCommandEvent& event) {
  OnConnValChange(event);
}

void ConnectionEditDialog::OnRbOutput(wxCommandEvent& event) {
  OnConnValChange(event);
}

void ConnectionEditDialog::OnCbInput(wxCommandEvent& event) {
  const bool checked = m_cbInput->IsChecked();
  ShowInFilter(checked);

  SetDSFormRWStates();
  LayoutDialog();
  if (m_rbTypeNet->GetValue()) SetUDPNetAddressVisiblity();
  OnConnValChange(event);
}

void ConnectionEditDialog::OnCbOutput(wxCommandEvent& event) {
  OnConnValChange(event);
  const bool checked = m_cbOutput->IsChecked();
  m_stPrecision->Enable(checked);
  m_choicePrecision->Enable(checked);
  ShowOutFilter(checked);

  if (!m_cbMultiCast->IsChecked() && m_rbNetProtoUDP->GetValue()) {
    if (checked) {
      m_tNetAddress->SetValue(DEFAULT_IP_ADDRESS); // IP address for output
    } else {
      m_tNetAddress->SetValue(DEFAULT_IP_ADDRESS); // IP address for input
    }
  }


  if (m_rbTypeNet->GetValue()) SetUDPNetAddressVisiblity();
  SetDSFormRWStates();
  LayoutDialog();
}

void ConnectionEditDialog::OnCbMultiCast(wxCommandEvent& event) {
  const bool checked = m_cbMultiCast->IsChecked();
  if (checked) {
    if (!IsAddressMultiCast(m_tNetAddress->GetValue())) {
      m_tNetAddress->SetValue("224.0.2.21");
    }
  } else if (m_cbOutput->IsChecked()) {
      m_tNetAddress->SetValue(DEFAULT_IP_ADDRESS); // IP address for output
    } else {
      m_tNetAddress->SetValue(DEFAULT_IP_ADDRESS);  // IP address for input
    }


  SetUDPNetAddressVisiblity();
  LayoutDialog();
}

void ConnectionEditDialog::OnClickMore(wxMouseEvent& event) {
  // m_cbAdvanced->SetValue(!m_cbAdvanced->IsChecked());
  m_advanced = !m_advanced;
  m_more->SetLabelMarkup(m_advanced ? LESS : MORE);
  if (m_rbTypeNet->GetValue())
      SetNMEAFormForNetProtocol();
  else
      SetNMEAFormForSerialProtocol();
  LayoutDialog();
}

void ConnectionEditDialog::OnCbAdvanced(wxCommandEvent& event) {
  if (m_rbTypeNet->GetValue())
      SetNMEAFormForNetProtocol();
  else
      SetNMEAFormForSerialProtocol();
  LayoutDialog();
}

void ConnectionEditDialog::OnShowGpsWindowCheckboxClick(wxCommandEvent& event) {
  if (!m_cbNMEADebug->GetValue()) {
    NMEALogWindow::Get().DestroyWindow();
  } else {
    NMEALogWindow::Get().Create((wxWindow*)(m_parent->pParent), 35);

    // Try to ensure that the log window is a least a little bit visible
    wxRect logRect(
        NMEALogWindow::Get().GetPosX(), NMEALogWindow::Get().GetPosY(),
        NMEALogWindow::Get().GetSizeW(), NMEALogWindow::Get().GetSizeH());

#if 0
    if (m_container->GetRect().Contains(logRect)) {
      NMEALogWindow::Get().SetPos(
          m_container->GetRect().x / 2,
          (m_container->GetRect().y +
           (m_container->GetRect().height - logRect.height) / 2));
      NMEALogWindow::Get().Move();
    }
#endif
    m_parent->Raise();
  }
}
void ConnectionEditDialog::SetNMEAFormForSerialProtocol() {
  bool n0183ctlenabled = (DataProtocol)m_choiceSerialProtocol->GetSelection() ==
                         DataProtocol::PROTO_NMEA0183;
  bool advanced = m_advanced;
  ShowNMEACommon(n0183ctlenabled && advanced);
  m_cbGarminHost->Show(n0183ctlenabled && advanced);
  m_stPriority->Show(true);
  m_choicePriority->Show(true);

  SetDSFormRWStates();
  LayoutDialog();
}

void ConnectionEditDialog::SetNMEAFormForNetProtocol() {
  bool n0183ctlenabled = (DataProtocol)m_choiceNetDataProtocol->GetSelection() ==
                         DataProtocol::PROTO_NMEA0183;
  bool advanced = m_advanced;
  ShowNMEACommon(n0183ctlenabled && advanced);
  m_cbGarminHost->Show(n0183ctlenabled && advanced);
  m_stPriority->Show(true);
  m_choicePriority->Show(true);

  SetDSFormRWStates();

  LayoutDialog();
}

void ConnectionEditDialog::OnProtocolChoice(wxCommandEvent& event) {
  if (m_rbTypeNet->GetValue())
    SetNMEAFormForNetProtocol();
  else
    SetNMEAFormForSerialProtocol();

  OnConnValChange(event);
}

void ConnectionEditDialog::ApplySettings() {
#if 0
  g_bfilter_cogsog = m_cbFilterSogCog->GetValue();

  long filter_val = 1;
  m_tFilterSec->GetValue().ToLong(&filter_val);
  g_COGFilterSec =
      wxMin(static_cast<int>(filter_val),
            60 /*MAX_COGSOG_FILTER_SECONDS*/);  // FIXME (dave)  should be
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
  DataProtocol lastDataProtocol = PROTO_NMEA0183;

  if (mSelectedConnection) {
    ConnectionParams* cpo = mSelectedConnection;
    lastAddr = cpo->NetworkAddress;
    lastPort = cpo->NetworkPort;
    lastNetProtocol = cpo->NetProtocol;
    lastDataProtocol = cpo->Protocol;
  }

  if (!connectionsaved) {
    size_t nCurrentPanelCount = TheConnectionParams()->GetCount();
    ConnectionParams* cp = NULL;
    int old_priority = -1;
    {
      if (mSelectedConnection) {
        cp = mSelectedConnection;
        old_priority = cp->Priority;
        UpdateConnectionParamsFromControls(cp);
        cp->b_IsSetup = false;
        // cp->bEnabled = false;
        // if (cp->m_optionsPanel)
        //   cp->m_optionsPanel->SetEnableCheckbox(false);

        // delete TheConnectionParams()->Item(itemIndex)->m_optionsPanel;
        // old_priority = TheConnectionParams()->Item(itemIndex)->Priority;
        // TheConnectionParams()->RemoveAt(itemIndex);
        // TheConnectionParams()->Insert(cp, itemIndex);
        // mSelectedConnection = cp;
        // cp->m_optionsPanel->SetSelected( true );
      } else {
        cp = CreateConnectionParamsFromSelectedItem();
        if (cp) TheConnectionParams()->Add(cp);
      }

      //  Record the previous parameters, if any
      if (cp) {
        cp->LastNetProtocol = lastNetProtocol;
        cp->LastNetworkAddress = lastAddr;
        cp->LastNetworkPort = lastPort;
        cp->LastDataProtocol = lastDataProtocol;
      }

      if (TheConnectionParams()->GetCount() != nCurrentPanelCount)
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
  for (size_t i = 0; i < TheConnectionParams()->Count(); i++) {
    ConnectionParams* cp = TheConnectionParams()->Item(i);

    if (cp->b_IsSetup) continue;

    // Connection is new, or edited, or disabled

    // Terminate and remove any existing driver, if present in registry
    StopAndRemoveCommDriver(cp->GetStrippedDSPort(), cp->GetCommProtocol());

    // Stop and remove  "previous" port, in case other params have changed.
    StopAndRemoveCommDriver(cp->GetLastDSPort(), cp->GetLastCommProtocol());

    // Internal BlueTooth driver stacks commonly need a time delay to purge
    // their buffers, etc. before restating with new parameters...
    if (cp->Type == INTERNAL_BT) wxSleep(1);

    // Connection has been disabled
    if (!cp->bEnabled) continue;

    // Make any new or re-enabled drivers
    MakeCommDriver(cp);
    cp->b_IsSetup = TRUE;
  }

  g_bGarminHostUpload = m_cbGarminUploadHost->GetValue();
  g_GPS_Ident =
      m_cbFurunoGP3X->GetValue() ? "FurunoGP3X" : "Generic";
#endif
}

ConnectionParams* ConnectionEditDialog::CreateConnectionParamsFromSelectedItem() {
#if 0
  // FIXME (dave)  How could this happen?
  // if (!m_bNMEAParams_shown) return NULL;

  //  Special encoding for deleted connection
  if (m_rbTypeSerial->GetValue() && m_comboPort->GetValue() ==  "Deleted")
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
      m_tNetPort->SetValue("10110");  // reset to default
    }
    if (m_tNetAddress->GetValue() == wxEmptyString) {
      m_tNetAddress->SetValue("0.0.0.0");
    }
  } else if (m_rbTypeCAN->GetValue()) {
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
#endif
  return NULL;
}

ConnectionParams* ConnectionEditDialog::GetParamsFromControls() {
  ConnectionParams* pConnectionParams = new ConnectionParams();
  UpdateConnectionParamsFromControls(pConnectionParams);
  return pConnectionParams;
}


ConnectionParams* ConnectionEditDialog::UpdateConnectionParamsFromControls(
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
  else if (m_rbTypeCAN && m_rbTypeCAN->GetValue())
    pConnectionParams->Type = SOCKETCAN;

  if (m_rbTypeNet->GetValue()) {
    //  Save the existing addr/port to allow closing of existing port
    pConnectionParams->LastNetworkAddress = pConnectionParams->NetworkAddress;
    pConnectionParams->LastNetworkPort = pConnectionParams->NetworkPort;
    pConnectionParams->LastNetProtocol = pConnectionParams->NetProtocol;
    pConnectionParams->LastDataProtocol = pConnectionParams->Protocol;

    pConnectionParams->NetworkAddress =
        m_tNetAddress->GetValue().Trim(false).Trim(true);
    pConnectionParams->NetworkPort =
        wxAtoi(m_tNetPort->GetValue().Trim(false).Trim(true));
    if (m_rbNetProtoTCP->GetValue()) {
      pConnectionParams->NetProtocol = TCP;
      pConnectionParams->Protocol =
          (DataProtocol)m_choiceNetDataProtocol->GetSelection();
    } else if (m_rbNetProtoUDP->GetValue()) {
      pConnectionParams->NetProtocol = UDP;
      pConnectionParams->Protocol =
          (DataProtocol)m_choiceNetDataProtocol->GetSelection();
    } else if (m_rbNetProtoGPSD->GetValue())
      pConnectionParams->NetProtocol = GPSD;
    else if (m_rbNetProtoSignalK->GetValue())
      pConnectionParams->NetProtocol = SIGNALK;
    else
      pConnectionParams->NetProtocol = PROTO_UNDEFINED;
  }

  if (m_rbTypeSerial->GetValue())
    pConnectionParams->Protocol =
        (DataProtocol)m_choiceSerialProtocol->GetSelection();
  else if (m_rbTypeNet->GetValue())
    pConnectionParams->Protocol =
        (DataProtocol)m_choiceNetDataProtocol->GetSelection();

  pConnectionParams->Baudrate = wxAtoi(m_choiceBaudRate->GetStringSelection());
  pConnectionParams->Priority = wxAtoi(m_choicePriority->GetStringSelection());
  pConnectionParams->ChecksumCheck = m_cbCheckCRC->GetValue();
  pConnectionParams->AutoSKDiscover = m_cbCheckSKDiscover->GetValue();
  pConnectionParams->Garmin = m_cbGarminHost->GetValue();
  pConnectionParams->InputSentenceList =
      wxStringTokenize(m_tcInputStc->GetValue(), ",");
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
      wxStringTokenize(m_tcOutputStc->GetValue(), ",");
  if (m_rbOAccept->GetValue())
    pConnectionParams->OutputSentenceListType = WHITELIST;
  else
    pConnectionParams->OutputSentenceListType = BLACKLIST;
  pConnectionParams->Port = m_comboPort->GetValue().BeforeFirst(' ');
#if defined(__linux__) && !defined(__ANDROID__)
  if (pConnectionParams->Type == SERIAL)
    CheckSerialAccess(m_parent, pConnectionParams->Port.ToStdString());
#endif

  if (m_rbTypeCAN && m_rbTypeCAN->GetValue())
    pConnectionParams->Protocol = PROTO_NMEA2000;

  pConnectionParams->bEnabled = m_connection_enabled;
  pConnectionParams->b_IsSetup = FALSE;

  if (pConnectionParams->Type == INTERNAL_GPS) {
    pConnectionParams->NetworkAddress = "";
    pConnectionParams->NetworkPort = 0;
    pConnectionParams->NetProtocol = PROTO_UNDEFINED;
    pConnectionParams->Baudrate = 0;
  }

  if (pConnectionParams->Type == INTERNAL_BT) {
    wxString parms = m_choiceBTDataSources->GetStringSelection();
    wxStringTokenizer tkz(parms, ";");
    wxString name = tkz.GetNextToken();
    wxString mac = tkz.GetNextToken();

    pConnectionParams->NetworkAddress = name;
    pConnectionParams->Port = mac;
    pConnectionParams->NetworkPort = 0;
    pConnectionParams->NetProtocol = PROTO_UNDEFINED;
    pConnectionParams->Baudrate = 0;
    //        pConnectionParams->SetAuxParameterStr(m_choiceBTDataSources->GetStringSelection());
  }

  if (pConnectionParams->Type == SOCKETCAN) {
    pConnectionParams->NetworkAddress = "";
    pConnectionParams->NetworkPort = 0;
    pConnectionParams->NetProtocol = PROTO_UNDEFINED;
    pConnectionParams->Baudrate = 0;
    pConnectionParams->socketCAN_port =
        m_choiceCANSource->GetString(m_choiceCANSource->GetSelection());
  }

  if (pConnectionParams->Type == SERIAL) {
    pConnectionParams->UserComment = m_tSerialComment->GetValue();
  } else if (pConnectionParams->Type == NETWORK) {
    pConnectionParams->UserComment = m_tNetComment->GetValue();
  }
  pConnectionParams->AuthToken = m_tAuthToken->GetValue();

  return pConnectionParams;
}

void ConnectionEditDialog::OnPriorityDialog(wxCommandEvent& event) {
  PriorityDlg* pdlg = new PriorityDlg(m_parent);
  pdlg->ShowModal();
  delete pdlg;
}
void ConnectionEditDialog::ConnectControls(){

// Connect controls
  m_rbTypeSerial->Connect(
    wxEVT_COMMAND_RADIOBUTTON_SELECTED,
    wxCommandEventHandler(ConnectionEditDialog::OnTypeSerialSelected), NULL,
    this);
  m_rbTypeNet->Connect(
    wxEVT_COMMAND_RADIOBUTTON_SELECTED,
    wxCommandEventHandler(ConnectionEditDialog::OnTypeNetSelected), NULL, this);

  m_rbTypeCAN->Connect(
    wxEVT_COMMAND_RADIOBUTTON_SELECTED,
    wxCommandEventHandler(ConnectionEditDialog::OnTypeCANSelected), NULL, this);

  if (m_rbTypeInternalGPS)
    m_rbTypeInternalGPS->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(ConnectionEditDialog::OnTypeGPSSelected), NULL,
      this);
  if (m_rbTypeInternalBT)
    m_rbTypeInternalBT->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(ConnectionEditDialog::OnTypeBTSelected), NULL, this);

// Network connection
  m_rbNetProtoTCP->Connect(
    wxEVT_COMMAND_RADIOBUTTON_SELECTED,
    wxCommandEventHandler(ConnectionEditDialog::OnNetProtocolSelected), NULL,
    this);
  m_rbNetProtoUDP->Connect(
    wxEVT_COMMAND_RADIOBUTTON_SELECTED,
    wxCommandEventHandler(ConnectionEditDialog::OnNetProtocolSelected), NULL,
    this);
  m_rbNetProtoGPSD->Connect(
    wxEVT_COMMAND_RADIOBUTTON_SELECTED,
    wxCommandEventHandler(ConnectionEditDialog::OnNetProtocolSelected), NULL,
    this);
  m_rbNetProtoSignalK->Connect(
    wxEVT_COMMAND_RADIOBUTTON_SELECTED,
    wxCommandEventHandler(ConnectionEditDialog::OnNetProtocolSelected), NULL,
    this);
  m_choiceNetDataProtocol->Connect(
    wxEVT_COMMAND_CHOICE_SELECTED,
    wxCommandEventHandler(ConnectionEditDialog::OnProtocolChoice), NULL, this);
  m_choiceSerialProtocol->Connect(
    wxEVT_COMMAND_CHOICE_SELECTED,
    wxCommandEventHandler(ConnectionEditDialog::OnProtocolChoice), NULL, this);
  m_cbMultiCast->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                      wxCommandEventHandler(ConnectionEditDialog::OnCbMultiCast),
                      NULL, this);
  // m_cbAdvanced->Connect(
  //    wxEVT_COMMAND_CHECKBOX_CLICKED,
  //    wxCommandEventHandler(ConnectionEditDialog::OnCbAdvanced), NULL, this);
  m_more->Bind(wxEVT_LEFT_DOWN, &ConnectionEditDialog::OnClickMore, this);

// input/output control
  m_cbInput->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                   wxCommandEventHandler(ConnectionEditDialog::OnCbInput), NULL,
                   this);
  m_cbOutput->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                    wxCommandEventHandler(ConnectionEditDialog::OnCbOutput),
                    NULL, this);

// Input filtering
  //m_rbIAccept->Connect(
  //    wxEVT_COMMAND_RADIOBUTTON_SELECTED,
  //    wxCommandEventHandler(ConnectionEditDialog::OnRbAcceptInput), NULL, this);
  //m_rbIIgnore->Connect(
  //    wxEVT_COMMAND_RADIOBUTTON_SELECTED,
  //    wxCommandEventHandler(ConnectionEditDialog::OnRbIgnoreInput), NULL, this);
  //m_tcInputStc->Connect(
  //    wxEVT_COMMAND_TEXT_UPDATED,
  //    wxCommandEventHandler(ConnectionEditDialog::OnConnValChange), NULL, this);
  m_btnInputStcList->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnBtnIStcs), NULL, this);

// output filtering
  m_btnOutputStcList->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnBtnOStcs), NULL, this);


#if 0
    m_tNetAddress->Connect(
      wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(ConnectionEditDialog::OnConnValChange), NULL, this);
  m_tNetPort->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                      wxCommandEventHandler(ConnectionEditDialog::OnConnValChange),
                      NULL, this);
  m_comboPort->Connect(
      wxEVT_COMMAND_COMBOBOX_SELECTED,
      wxCommandEventHandler(ConnectionEditDialog::OnConnValChange), NULL, this);
  m_comboPort->Connect(
      wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(ConnectionEditDialog::OnConnValChange), NULL, this);
  m_choiceBaudRate->Connect(
      wxEVT_COMMAND_CHOICE_SELECTED,
      wxCommandEventHandler(ConnectionEditDialog::OnBaudrateChoice), NULL, this);
  m_choicePriority->Connect(
      wxEVT_COMMAND_CHOICE_SELECTED,
      wxCommandEventHandler(ConnectionEditDialog::OnConnValChange), NULL, this);
  m_choiceCANSource->Connect(
      wxEVT_COMMAND_CHOICE_SELECTED,
      wxCommandEventHandler(ConnectionEditDialog::OnConnValChange), NULL, this);
  m_cbCheckCRC->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                        wxCommandEventHandler(ConnectionEditDialog::OnCrcCheck),
                        NULL, this);
  m_cbGarminHost->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnUploadFormatChange), NULL,
      this);
  m_cbGarminUploadHost->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnUploadFormatChange), NULL,
      this);
  m_cbFurunoGP3X->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnUploadFormatChange), NULL,
      this);
  m_cbCheckSKDiscover->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnConnValChange), NULL, this);
  m_ButtonSKDiscover->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnDiscoverButton), NULL, this);

    m_rbOAccept->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(ConnectionEditDialog::OnRbOutput),
                       NULL, this);
  m_rbOIgnore->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(ConnectionEditDialog::OnRbOutput),
                       NULL, this);
  m_tcOutputStc->Connect(
      wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(ConnectionEditDialog::OnConnValChange), NULL, this);
    m_cbCheckCRC->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnConnValChange), NULL, this);

  m_cbNMEADebug->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnShowGpsWindowCheckboxClick),
      NULL, this);
  m_cbFilterSogCog->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnValChange), NULL, this);
  m_tFilterSec->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                        wxCommandEventHandler(ConnectionEditDialog::OnValChange),
                        NULL, this);
  m_cbAPBMagnetic->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnValChange), NULL, this);

  m_ButtonPriorityDialog->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ConnectionEditDialog::OnPriorityDialog), NULL, this);

  m_tNetComment->Connect(
      wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(ConnectionEditDialog::OnConnValChange), NULL, this);
  m_tSerialComment->Connect(
      wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(ConnectionEditDialog::OnConnValChange), NULL, this);
  m_tAuthToken->Connect(
      wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(ConnectionEditDialog::OnConnValChange), NULL, this);

  if (m_buttonScanBT)
    m_buttonScanBT->Connect(
        wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(ConnectionEditDialog::OnScanBTClick), NULL, this);
#endif
}


bool ConnectionEditDialog::IsAddressMultiCast(wxString ip) {
  wxArrayString bytes = wxSplit(ip, '.');
  if (bytes.size() != 4) {
    return false;
  }
  unsigned long ipNum = (wxAtoi(bytes[0]) << 24) + (wxAtoi(bytes[1]) << 16) +
                        (wxAtoi(bytes[2]) << 8) + wxAtoi(bytes[3]);
  unsigned long multicastStart = (224 << 24);
  unsigned long multicastEnd = (239 << 24) + (255 << 16) + (255 << 8) + 255;
  return ipNum >= multicastStart && ipNum <= multicastEnd;
}



bool ConnectionEditDialog::IsDefaultPort(wxString address) {
    return  (address == DEFAULT_TCP_PORT) || (address == DEFAULT_UDP_PORT) || (address == DEFAULT_SIGNALK_PORT) ||
           (address == DEFAULT_GPSD_PORT);
}

bool ConnectionEditDialog::IsAddressBroadcast(wxString ip) {
  wxArrayString bytes = wxSplit(ip, '.');
  if (bytes.size() != 4) {
    std::cerr << "Invalid IP format." << std::endl;
    return false;
  }
  return wxAtoi(bytes[3]) == 255;
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
    wxString s;
    m_sentences.Add(g_TalkerIdText + wxString("RMB"));
    m_sentences.Add(g_TalkerIdText + wxString("RMC"));
    m_sentences.Add(g_TalkerIdText + wxString("APB"));
    m_sentences.Add(g_TalkerIdText + wxString("XTE"));
  }
  m_sentences.Add("AIVDM");
  m_sentences.Add("AIVDO");
  m_sentences.Add("FRPOS");
  m_sentences.Add("CD");
  m_clbSentences->Clear();
  m_clbSentences->InsertItems(m_sentences, 0);

  wxArrayString new_strings;

  if (list.Count() == 0) {
    for (size_t i = 0; i < m_clbSentences->GetCount(); ++i)
      m_clbSentences->Check(i, m_type == WHITELIST);
  } else {
    for (size_t i = 0; i < list.Count(); ++i) {
      int item = m_clbSentences->FindString(list[i]);
      if (item != wxNOT_FOUND)
        m_clbSentences->Check(item);
      else
        new_strings.Add(list[i]);
    }
    if(new_strings.GetCount())
      m_clbSentences->InsertItems(new_strings, m_clbSentences->GetCount());
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
#ifdef __ANDROID__
  androidDisableRotation();
#endif

  wxTextEntryDialog textdlg(
      this,
      _("Enter the NMEA sentence (2, 3 or 5 characters)\n  or a valid REGEX "
        "expression (6 characters or longer)"),
      _("Enter the NMEA sentence"));

  textdlg.SetTextValidator(wxFILTER_ASCII);
  int result = textdlg.ShowModal();

#ifdef __ANDROID__
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


