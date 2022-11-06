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

#include "OCPNPlatform.h"
#include "route_gui.h"
#include "route.h"
#include "route_point_gui.h"
#include "route_point.h"
#include "SendToPeerDlg.h"
#include "mDNS_query.h"
#include "peer_client.h"

extern OCPNPlatform* g_Platform;
extern std::vector<std::shared_ptr<ocpn_DNS_record_t>> g_DNS_cache;
extern wxString g_hostname;

IMPLEMENT_DYNAMIC_CLASS(SendToPeerDlg, wxDialog)

BEGIN_EVENT_TABLE(SendToPeerDlg, wxDialog)
  EVT_BUTTON(ID_STP_CANCEL, SendToPeerDlg::OnCancelClick)
  EVT_BUTTON(ID_STP_OK, SendToPeerDlg::OnSendClick)
END_EVENT_TABLE()

SendToPeerDlg::SendToPeerDlg() {
  m_PeerListBox = NULL;
  m_pgauge = NULL;
  m_SendButton = NULL;
  m_CancelButton = NULL;
  m_pRoute = NULL;
  m_pRoutePoint = NULL;
  premtext = NULL;
}

SendToPeerDlg::SendToPeerDlg(wxWindow* parent, wxWindowID id,
                           const wxString& caption, const wxString& hint,
                           const wxPoint& pos, const wxSize& size, long style) {
  Create(parent, id, caption, hint, pos, size, style);
}

SendToPeerDlg::~SendToPeerDlg() {
  delete m_PeerListBox;
  delete m_pgauge;
  delete m_SendButton;
  delete m_CancelButton;
}

bool SendToPeerDlg::Create(wxWindow* parent, wxWindowID id,
                          const wxString& caption, const wxString& hint,
                          const wxPoint& pos, const wxSize& size, long style) {
  SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, id, caption, pos, size, style);

  CreateControls(hint);
  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();

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
    if(item.StartsWith(g_hostname))
      continue;
    item += " {";
    item += g_DNS_cache[i]->ip.c_str();
    item += "}";
    m_PeerListBox->Append(item);
  }


#if 0
  //    Make the proper initial selection
  if (!g_uploadConnection.IsEmpty()) {
    if (g_uploadConnection.Lower().StartsWith("tcp") ||
        g_uploadConnection.Lower().StartsWith("udp")) {
      bool b_connExists = false;
      for (unsigned int i = 0; i < netconns.GetCount(); i++) {
        if (g_uploadConnection.IsSameAs(netconns[i])) {
          b_connExists = true;
          break;
        }
      }
      if (b_connExists) m_PeerListBox->SetValue(g_uploadConnection);
    } else
      m_PeerListBox->SetValue(g_uploadConnection);
  } else
#endif

  m_PeerListBox->SetSelection(0);

  comm_box_sizer->Add(m_PeerListBox, 0, wxEXPAND | wxALL, 5);

  //    Add a reminder text box
  itemBoxSizer2->AddSpacer(20);

#if 0
  premtext = new wxStaticText(
      this, -1, _("Prepare GPS for Route/Waypoint upload and press Send..."));
  itemBoxSizer2->Add(premtext, 0, wxEXPAND | wxALL, 10);

  //    Create a progress gauge
  wxStaticBox* prog_box = new wxStaticBox(this, wxID_ANY, _("Progress..."));

  wxStaticBoxSizer* prog_box_sizer = new wxStaticBoxSizer(prog_box, wxVERTICAL);
  itemBoxSizer2->Add(prog_box_sizer, 0, wxEXPAND | wxALL, 5);

  m_pgauge = new wxGauge(this, -1, 100);
  prog_box_sizer->Add(m_pgauge, 0, wxEXPAND | wxALL, 5);
#endif

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
  if (!m_pRoute)
    Close();

  //    Get the selected peer information
  wxString peer_ip = m_PeerListBox->GetValue();
  wxString server_name = peer_ip.BeforeFirst('{').Trim();

  int tail = peer_ip.Find('{');
  if (tail != wxNOT_FOUND)
    peer_ip = peer_ip.Mid(tail+1);
  peer_ip = peer_ip.BeforeFirst('}');
  peer_ip += ":";
  peer_ip += "8443";

  std::string server_address("https://");
  server_address += peer_ip.ToStdString();


   //g_uploadConnection = src;  // save for persistence


  //    And send it out
  int return_code = SendRoute(server_address, server_name.ToStdString(), m_pRoute, true);

  //if (m_pRoutePoint) RoutePointGui(*m_pRoutePoint).SendToGPS(destPort, this);

  //    Show( false );
  //    event.Skip();
  Close();
}

void SendToPeerDlg::OnCancelClick(wxCommandEvent& event) {
  //    Show( false );
  //    event.Skip();
  Close();
}
