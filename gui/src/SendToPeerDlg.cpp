/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 */
#include <memory>

#include <wx/statline.h>

#include <curl/curl.h>

#include "model/cmdline.h"
#include "model/config_vars.h"
#include "model/mdns_cache.h"
#include "model/mdns_query.h"
#include "model/ocpn_utils.h"
#include "model/peer_client.h"
#include "model/route.h"
#include "model/route_point.h"

#include "gui_lib.h"
#include "OCPNPlatform.h"
#include "ocpn_frame.h"
#include "peer_client_dlg.h"
#include "route_gui.h"
#include "route_point_gui.h"

#include "SendToPeerDlg.h"
#include "ocpn_plugin.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

#define TIMER_AUTOSCAN 94522
#define TIMER_SCANTICK 94523

extern OCPNPlatform* g_Platform;

static PeerDlgResult ConfirmWriteDlg() {
  std::string msg(_("Objects exists on server. OK to overwrite?"));
  long style = wxYES | wxNO | wxNO_DEFAULT | wxICON_QUESTION;
  OCPNMessageDialog dlg(NULL, msg, _("OpenCPN Info"), style);
  int reply = dlg.ShowModal();
  return reply == wxID_YES ? PeerDlgResult::Ok : PeerDlgResult::Cancel;
}

static PeerDlgResult RunStatusDlg(PeerDlg kind, int status) {
  switch (kind) {
    case PeerDlg::InvalidHttpResponse: {
      std::stringstream ss;
      if (status >= 0) {
        ss << _("Server HTTP response is :") << status;
      } else {
        ss << _("Curl transfer error: ")
           << curl_easy_strerror(static_cast<CURLcode>(-status));
      }
      OCPNMessageDialog dlg(NULL, ss.str(), _("OpenCPN Info"),
                            wxICON_ERROR | wxOK | wxCANCEL);
      int r = dlg.ShowModal();
      return r == wxID_OK ? PeerDlgResult::Ok : PeerDlgResult::Cancel;
    }
    case PeerDlg::ErrorReturn: {
      std::stringstream ss;
      ss << _("Server internal error response:") << status;
      OCPNMessageDialog dlg(NULL, ss.str(), _("OpenCPN Info"),
                            wxICON_ERROR | wxOK | wxCANCEL);
      int r = dlg.ShowModal();
      return r == wxID_OK ? PeerDlgResult::Ok : PeerDlgResult::Cancel;
    }
    case PeerDlg::TransferOk: {
      std::stringstream ss;
      std::string msg(_("Transfer successfully completed"));
      OCPNMessageDialog dlg(NULL, msg, _("OpenCPN Info"),
                            wxICON_INFORMATION | wxOK);
      dlg.ShowModal();
      return PeerDlgResult::Ok;
    }
    case PeerDlg::JsonParseError: {
      std::string msg(_("Cannot parse server reply"));
      OCPNMessageDialog dlg(NULL, msg, _("OpenCPN Info"),
                            wxICON_ERROR | wxOK | wxCANCEL);
      int r = dlg.ShowModal();
      return r == wxID_OK ? PeerDlgResult::Ok : PeerDlgResult::Cancel;
    }
    case PeerDlg::BadPincode: {
      std::string msg(_("Pincode not accepted"));
      OCPNMessageDialog dlg(NULL, msg, _("OpenCPN Info"),
                            wxICON_ERROR | wxOK | wxCANCEL);
      int r = dlg.ShowModal();
      return r == wxID_OK ? PeerDlgResult::Ok : PeerDlgResult::Cancel;
    }
    case PeerDlg::ActivateUnsupported: {
      std::string msg(_("Server does not support activation"));
      OCPNMessageDialog dlg(NULL, msg, _("OpenCPN Info"),
                            wxICON_ERROR | wxOK | wxCANCEL);

      int r = dlg.ShowModal();
      return r == wxID_OK ? PeerDlgResult::Ok : PeerDlgResult::Cancel;
    }
    case PeerDlg::PinConfirm:
      assert(false && "Illegal PinConfirm result dialog");
  }
  return PeerDlgResult::Cancel;  // For the compiler, not reached
}

std::pair<PeerDlgResult, std::string> RunPincodeDlg() {
  PinConfirmDlg dlg(wxTheApp->GetTopWindow(), wxID_ANY,
                    _("OpenCPN Server Message"), "", wxDefaultPosition,
                    wxDefaultSize, SYMBOL_PCD_STYLE);

  static const char* const msg =
      _("A server pin is needed.\n"
        "Please enter PIN number from server to pair with this device");

  dlg.SetMessage(msg);
  dlg.SetPincodeText("");
  if (dlg.ShowModal() == wxID_OK) {
    auto pin = dlg.GetPincodeText().Trim().Trim(false);
    return {PeerDlgResult::HasPincode, pin.ToStdString()};
  }
  return {PeerDlgResult::Cancel, ""};
}

/** Dig out server name and ip address from textbox value. */
static void ParsePeer(const wxString& ui_value, PeerData& peer_data) {
  wxString server_name = ui_value.BeforeFirst('{').Trim();
  wxString peer_ip = ui_value;
  int tail = ui_value.Find('{');
  if (tail != wxNOT_FOUND) peer_ip = peer_ip.Mid(tail + 1);
  peer_ip = peer_ip.BeforeFirst('}') + ":";
  // Is the destination a portable?  Detect by string inspection.
  peer_ip += server_name.BeforeFirst('-') == "Portable" ? "8444" : "8443";
  peer_data.server_name = server_name.ToStdString();
  peer_data.dest_ip_address = peer_ip.ToStdString();
}

BEGIN_EVENT_TABLE(SendToPeerDlg, wxDialog)
EVT_BUTTON(ID_STP_CANCEL, SendToPeerDlg::OnCancelClick)
EVT_BUTTON(ID_STP_OK, SendToPeerDlg::OnSendClick)
EVT_BUTTON(ID_STP_SCAN, SendToPeerDlg::OnScanClick)
EVT_TIMER(TIMER_AUTOSCAN, SendToPeerDlg::OnTimerAutoscan)
EVT_TIMER(TIMER_SCANTICK, SendToPeerDlg::OnTimerScanTick)
END_EVENT_TABLE()

SendToPeerDlg::SendToPeerDlg() {
  m_PeerListBox = NULL;
  m_pgauge = NULL;
  m_SendButton = NULL;
  m_CancelButton = NULL;
  premtext = NULL;
  m_scanTime = 5;  // default, seconds
  m_bScanOnCreate = false;

  // Get our own local ipv4 address, for filtering
  std::vector<std::string> ipv4_addrs = get_local_ipv4_addresses();
  if (ipv4_addrs.size()) m_ownipAddr = ipv4_addrs[0];

#ifdef __ANDROID__
  androidDisableRotation();
#endif
}

SendToPeerDlg::~SendToPeerDlg() {
  delete m_PeerListBox;
  delete m_pgauge;
  delete m_SendButton;
  delete m_CancelButton;
#ifdef __ANDROID__
  androidEnableRotation();
#endif
}

bool SendToPeerDlg::Create(wxWindow* parent, wxWindowID id,
                           const wxString& caption, const wxString& hint,
                           const wxPoint& pos, const wxSize& size, long style) {
  SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
  wxFont* pF = OCPNGetFont(_("Dialog"));
  SetFont(*pF);

  wxDialog::Create(parent, id, caption, pos, size, style);

  CreateControls(hint);
  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();

  if (m_bScanOnCreate) {
    m_autoScanTimer.SetOwner(this, TIMER_AUTOSCAN);
    m_autoScanTimer.Start(500, wxTIMER_ONE_SHOT);
  }
  m_ScanTickTimer.SetOwner(this, TIMER_SCANTICK);

  auto action = [&](ObservedEvt& evt) { m_pgauge->SetValue(evt.GetInt()); };
  progress_listener.Init(progress, action);
#ifdef __ANDROID__
  androidDisableRotation();
#endif
  return true;
}

bool SendToPeerDlg::EnableActivateChkbox() {
  return m_RouteList.size() == 1 && m_RoutePointList.empty() &&
         m_TrackList.empty();
}

void SendToPeerDlg::CreateControls(const wxString&) {
  SendToPeerDlg* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  //      Create the ScrollBox list of available com ports in a labeled static
  //      box
  wxStaticBox* comm_box =
      new wxStaticBox(this, wxID_ANY, _("Detected OpenCPN peer instances"));

  wxStaticBoxSizer* comm_box_sizer = new wxStaticBoxSizer(comm_box, wxVERTICAL);
  itemBoxSizer2->Add(comm_box_sizer, 0, wxEXPAND | wxALL, 5);

  m_PeerListBox = new wxComboBox(this, ID_STP_CHOICE_PEER);

  //    Fill in the wxComboBox with all detected peers
  for (auto& entry : MdnsCache::GetInstance().GetCache()) {
    wxString item(entry.hostname.c_str());

    // skip "self"
    if (!g_hostname.IsSameAs(item.BeforeFirst('.')) ||
        (m_ownipAddr != entry.ip)) {
      item += " {";
      item += entry.ip.c_str();
      item += "}";
      m_PeerListBox->Append(item);
    }
  }

  if (m_PeerListBox->GetCount()) m_PeerListBox->SetSelection(0);
  m_PeerListBox->Bind(wxEVT_TEXT, [&](wxCommandEvent&) {
    m_SendButton->Enable(m_PeerListBox->GetValue() != "");
  });
  m_PeerListBox->Enable(!m_bScanOnCreate);
  comm_box_sizer->Add(m_PeerListBox, 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxEXPAND | wxALL, 5);

  m_RescanButton = new wxButton(itemDialog1, ID_STP_SCAN, _("Scan again"),
                                wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer3->Add(m_RescanButton, 0, wxALL, 5);

  m_pgauge = new wxGauge(itemDialog1, -1, m_scanTime * 2, wxDefaultPosition,
                         wxSize(-1, GetCharHeight()));
  itemBoxSizer3->Add(m_pgauge, 0, wxEXPAND | wxALL, 20);

  itemBoxSizer2->AddSpacer(30);
  itemBoxSizer2->Add(new wxStaticLine(this), wxSizerFlags(0).Expand());
  m_activate_chkbox =
      new wxCheckBox(this, wxID_ANY, _("Activate after transfer"),
                     wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
  itemBoxSizer2->Add(m_activate_chkbox, 0, wxALIGN_RIGHT | wxALL, 10);
  if (!EnableActivateChkbox()) m_activate_chkbox->Disable();

  //    OK/Cancel/etc.
  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(itemDialog1, ID_STP_CANCEL, _("Cancel"),
                                wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_SendButton = new wxButton(itemDialog1, ID_STP_OK, _("Send"),
                              wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_SendButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_SendButton->SetDefault();
  m_SendButton->Enable(!m_PeerListBox->IsListEmpty());
}

void SendToPeerDlg::SetMessage(wxString msg) {
  if (premtext) {
    premtext->SetLabel(msg);
    premtext->Refresh(true);
  }
}

void SendToPeerDlg::OnSendClick(wxCommandEvent&) {
  if (m_RouteList.empty() && m_TrackList.empty() && m_RoutePointList.empty()) {
    Close();
    return;
  }
  // Set up transfer data
  PeerData peer_data(progress);
  ParsePeer(m_PeerListBox->GetValue(), peer_data);
  auto addr_port = ocpn::split(peer_data.dest_ip_address, ":");
  if (addr_port.size() == 1) addr_port.push_back("8443");
  MdnsCache::GetInstance().Add(addr_port[0], addr_port[1]);
  peer_data.routes = m_RouteList;
  peer_data.tracks = m_TrackList;
  peer_data.routepoints = m_RoutePointList;
  peer_data.run_status_dlg = RunStatusDlg;
  peer_data.run_pincode_dlg = RunPincodeDlg;
  peer_data.activate = m_activate_chkbox->GetValue();

  // And send it out
  m_pgauge->SetRange(100);
  m_pgauge->SetValue(0);
  m_pgauge->Show();

  GetApiVersion(peer_data);
  if (peer_data.api_version < SemanticVersion(5, 9)) {
    SendNavobjects(peer_data);
  } else {
    bool is_writable = CheckNavObjects(peer_data);
    if (is_writable || ConfirmWriteDlg() == PeerDlgResult::Ok) {
      peer_data.overwrite = true;
      SendNavobjects(peer_data);
    }
  }
  m_pgauge->Hide();
  Close();
}

void SendToPeerDlg::OnScanClick(wxCommandEvent&) { DoScan(); }

void SendToPeerDlg::OnTimerAutoscan(wxTimerEvent&) { DoScan(); }

void SendToPeerDlg::OnTimerScanTick(wxTimerEvent&) {
  m_tick--;
  if (m_pgauge) {
    int v = m_pgauge->GetValue();
    if (v + 1 <= m_pgauge->GetRange()) m_pgauge->SetValue(v + 1);
  }

  if (m_tick == 0) {
    // Housekeeping
    m_ScanTickTimer.Stop();
    g_Platform->HideBusySpinner();
    m_RescanButton->Enable();
    m_SendButton->SetDefault();
    m_pgauge->Hide();
    m_PeerListBox->Enable(true);
    m_bScanOnCreate = false;

    m_PeerListBox->Clear();

    //    Fill in the wxComboBox with all detected peers besides own host
    using namespace ocpn;
    for (const MdnsCache::Entry& e : MdnsCache::GetInstance().GetCache()) {
      if (g_hostname != split(e.hostname, ".")[0] || m_ownipAddr != e.ip) {
        m_PeerListBox->Append(e.hostname + " {" + e.ip.c_str() + "}");
      }
    }
    if (m_PeerListBox->GetCount()) m_PeerListBox->SetSelection(0);
    m_SendButton->Enable(m_PeerListBox->GetCount() > 0);
  }
}

void SendToPeerDlg::DoScan() {
  m_RescanButton->Disable();
  m_SendButton->Disable();
  g_Platform->ShowBusySpinner();
  m_pgauge->SetRange(m_scanTime);
  m_pgauge->SetValue(0);
  m_pgauge->Show();

  FindAllOCPNServers(m_scanTime);

  m_tick = m_scanTime * 2;
  m_ScanTickTimer.Start(500, wxTIMER_CONTINUOUS);
}

void SendToPeerDlg::OnCancelClick(wxCommandEvent&) {
  g_Platform->HideBusySpinner();
  Close();
}
