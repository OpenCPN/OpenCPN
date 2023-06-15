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

#include "config_vars.h"
#include "mDNS_query.h"
#include "OCPNPlatform.h"
#include "peer_client.h"
#include "route_gui.h"
#include "route.h"
#include "route_point_gui.h"
#include "route_point.h"
#include "SendToPeerDlg.h"
#include "ocpn_plugin.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

#define TIMER_AUTOSCAN  94522
#define TIMER_SCANTICK  94523
#define TIMER_TRANSFER  94524

extern OCPNPlatform* g_Platform;
extern std::vector<std::shared_ptr<ocpn_DNS_record_t>> g_DNS_cache;
extern wxDateTime g_DNS_cache_time;
extern bool g_bportable;
extern int navobj_transfer_progress;

IMPLEMENT_DYNAMIC_CLASS(SendToPeerDlg, wxDialog)

BEGIN_EVENT_TABLE(SendToPeerDlg, wxDialog)
  EVT_BUTTON(ID_STP_CANCEL, SendToPeerDlg::OnCancelClick)
  EVT_BUTTON(ID_STP_OK, SendToPeerDlg::OnSendClick)
  EVT_BUTTON(ID_STP_SCAN, SendToPeerDlg::OnScanClick)
  EVT_TIMER(TIMER_AUTOSCAN, SendToPeerDlg::OnTimerAutoscan)
  EVT_TIMER(TIMER_SCANTICK, SendToPeerDlg::OnTimerScanTick)
  EVT_TIMER(TIMER_TRANSFER, SendToPeerDlg::OnTimerTransferTick)
END_EVENT_TABLE()

SendToPeerDlg::SendToPeerDlg() {
  m_PeerListBox = NULL;
  m_pgauge = NULL;
  m_SendButton = NULL;
  m_CancelButton = NULL;
  premtext = NULL;
  m_scanTime = 5;  //default, seconds
  m_bScanOnCreate = false;
#ifdef __ANDROID__
  androidDisableRotation();
#endif

}

SendToPeerDlg::SendToPeerDlg(wxWindow* parent, wxWindowID id,
                           const wxString& caption, const wxString& hint,
                           const wxPoint& pos, const wxSize& size, long style) {
#ifdef __ANDROID__
  androidDisableRotation();
#endif
  Create(parent, id, caption, hint, pos, size, style);
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

  wxFont *pF = OCPNGetFont(_T("Dialog"), 0);
  SetFont(*pF);

  wxDialog::Create(parent, id, caption, pos, size, style);

  CreateControls(hint);
  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();

  if (m_bScanOnCreate){
    m_autoScanTimer.SetOwner(this, TIMER_AUTOSCAN);
    m_autoScanTimer.Start(500, wxTIMER_ONE_SHOT);
  }

  m_ScanTickTimer.SetOwner(this, TIMER_SCANTICK);
  m_TransferTimer.SetOwner(this, TIMER_TRANSFER);

  return TRUE;
}

void SendToPeerDlg::CreateControls(const wxString& hint) {
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
  for (unsigned int i=0; i < g_DNS_cache.size(); i++){
    wxString item(g_DNS_cache[i]->hostname.c_str());

    //skip "self"
    if (!g_hostname.IsSameAs(item.BeforeFirst('.'))) {
      item += " {";
      item += g_DNS_cache[i]->ip.c_str();
      item += "}";
      m_PeerListBox->Append(item);
    }
  }

  if (m_PeerListBox->GetCount())
    m_PeerListBox->SetSelection(0);

  comm_box_sizer->Add(m_PeerListBox, 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer3, 0, wxEXPAND | wxALL, 5);

  m_RescanButton = new wxButton(itemDialog1, ID_STP_SCAN, _("Scan again"),
                                wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer3->Add(m_RescanButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_pgauge = new wxGauge(itemDialog1, -1, m_scanTime * 2,
                          wxDefaultPosition, wxSize(-1, GetCharHeight()));
  itemBoxSizer3->Add(m_pgauge, 0, wxEXPAND | wxALL, 20);

  //    Add a reminder text box
  itemBoxSizer2->AddSpacer(30);

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
}

void SendToPeerDlg::SetMessage(wxString msg) {
  if (premtext) {
    premtext->SetLabel(msg);
    premtext->Refresh(true);
  }
}

void SendToPeerDlg::OnSendClick(wxCommandEvent& event) {
  if (m_RouteList.empty() && m_TrackList.empty() && m_RoutePointList.empty())
    Close();

  //    Get the selected peer information
  wxString peer_ip = m_PeerListBox->GetValue();
  wxString server_name = peer_ip.BeforeFirst('{').Trim();
  int tail = peer_ip.Find('{');
  if (tail != wxNOT_FOUND)
    peer_ip = peer_ip.Mid(tail+1);
  peer_ip = peer_ip.BeforeFirst('}');
  peer_ip += ":";

  // Is the destination a portable?  Detect by string inspection.
  wxString p = "Portable";
  if (p.IsSameAs(server_name.BeforeFirst('-')))
    peer_ip += "8444";
  else
    peer_ip += "8443";

  std::string server_address("https://");
  server_address += peer_ip.ToStdString();


   //g_uploadConnection = src;  // save for persistence


  //    And send it out
  m_pgauge->SetRange(100);
  m_pgauge->SetValue(0);
  m_TransferTimer.Start(50);
  m_pgauge->Show();
  if (!m_RouteList.empty() || !m_RoutePointList.empty() || !m_TrackList.empty())
  {
    int return_code = SendNavobjects(server_address, server_name.ToStdString(), m_RouteList, m_RoutePointList, m_TrackList, true);
  }
  m_TransferTimer.Stop();
  m_pgauge->Hide();
  Close();
}

void SendToPeerDlg::OnScanClick(wxCommandEvent& event) {
  DoScan();
}

void SendToPeerDlg::OnTimerAutoscan(wxTimerEvent &event) {
  DoScan();
}

void SendToPeerDlg::OnTimerScanTick(wxTimerEvent &event) {
  m_tick--;
  if(m_pgauge) {
    int v = m_pgauge->GetValue();
    if( v + 1 <= m_pgauge->GetRange())
      m_pgauge->SetValue(v+1);
  }

  if (m_tick == 0){
    // Housekeeping
   m_ScanTickTimer.Stop();
   g_Platform->HideBusySpinner();
   m_RescanButton->Enable();
   m_SendButton->Enable();
   m_SendButton->SetDefault();
   m_pgauge->Hide();
   m_bScanOnCreate = false;

      // Clear the combo box
   m_PeerListBox->Clear();

   //    Fill in the wxComboBox with all detected peers
   for (unsigned int i=0; i < g_DNS_cache.size(); i++){
     wxString item(g_DNS_cache[i]->hostname.c_str());

    //skip "self"
     if (!g_hostname.IsSameAs(item.BeforeFirst('.'))) {
       item += " {";
       item += g_DNS_cache[i]->ip.c_str();
       item += "}";
       m_PeerListBox->Append(item);
     }
   }
   if (m_PeerListBox->GetCount())
    m_PeerListBox->SetSelection(0);

   g_DNS_cache_time = wxDateTime::Now();
  }
}

void SendToPeerDlg::OnTimerTransferTick(wxTimerEvent &event) {
  m_pgauge->SetValue(navobj_transfer_progress);
  event.Skip();
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

void SendToPeerDlg::OnCancelClick(wxCommandEvent& event) {
  g_Platform->HideBusySpinner();
  Close();
}
