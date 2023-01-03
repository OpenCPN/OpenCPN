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

#include <wx/arrstr.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/dynarray.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/window.h>

#include "conn_params.h"
#include "OCPNPlatform.h"
#include "route_gui.h"
#include "route.h"
#include "route_point_gui.h"
#include "route_point.h"
#include "SendToGpsDlg.h"
#include "ser_ports.h"

extern OCPNPlatform* g_Platform;
extern wxString g_uploadConnection;

IMPLEMENT_DYNAMIC_CLASS(SendToGpsDlg, wxDialog)

BEGIN_EVENT_TABLE(SendToGpsDlg, wxDialog)
EVT_BUTTON(ID_STG_CANCEL, SendToGpsDlg::OnCancelClick)
    EVT_BUTTON(ID_STG_OK, SendToGpsDlg::OnSendClick) END_EVENT_TABLE()

        SendToGpsDlg::SendToGpsDlg() {
  m_itemCommListBox = NULL;
  m_pgauge = NULL;
  m_SendButton = NULL;
  m_CancelButton = NULL;
  m_pRoute = NULL;
  m_pRoutePoint = NULL;
  premtext = NULL;
}

SendToGpsDlg::SendToGpsDlg(wxWindow* parent, wxWindowID id,
                           const wxString& caption, const wxString& hint,
                           const wxPoint& pos, const wxSize& size, long style) {
  Create(parent, id, caption, hint, pos, size, style);
}

SendToGpsDlg::~SendToGpsDlg() {
  delete m_itemCommListBox;
  delete m_pgauge;
  delete m_SendButton;
  delete m_CancelButton;
}

bool SendToGpsDlg::Create(wxWindow* parent, wxWindowID id,
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

void SendToGpsDlg::CreateControls(const wxString& hint) {
  SendToGpsDlg* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  //      Create the ScrollBox list of available com ports in a labeled static
  //      box
  wxStaticBox* comm_box =
      new wxStaticBox(this, wxID_ANY, _("GPS/Plotter Port"));

  wxStaticBoxSizer* comm_box_sizer = new wxStaticBoxSizer(comm_box, wxVERTICAL);
  itemBoxSizer2->Add(comm_box_sizer, 0, wxEXPAND | wxALL, 5);

  wxArrayString* pSerialArray = EnumerateSerialPorts();

  m_itemCommListBox = new wxComboBox(this, ID_STG_CHOICE_COMM);

  //    Fill in the listbox with all detected serial ports
  for (unsigned int iPortIndex = 0; iPortIndex < pSerialArray->GetCount();
       iPortIndex++) {
    wxString full_port = pSerialArray->Item(iPortIndex);
    full_port.Prepend(_T("Serial:"));
    m_itemCommListBox->Append(full_port);
  }

  delete pSerialArray;

  // Add any defined Network connections supporting "output"
  wxArrayString netconns;
  for (size_t i = 0; i < TheConnectionParams()->Count(); i++) {
    ConnectionParams* cp = TheConnectionParams()->Item(i);
    wxString netident;

    if ((cp->IOSelect != DS_TYPE_INPUT) && cp->Type == NETWORK &&
        (cp->NetProtocol == TCP)) {
      netident << _T("TCP:") << cp->NetworkAddress << _T(":")
               << cp->NetworkPort;
      m_itemCommListBox->Append(netident);
      netconns.Add(netident);
    }
    if ((cp->IOSelect != DS_TYPE_INPUT) && cp->Type == NETWORK &&
        (cp->NetProtocol == UDP)) {
      netident << _T("UDP:") << cp->NetworkAddress << _T(":")
               << cp->NetworkPort;
      m_itemCommListBox->Append(netident);
      netconns.Add(netident);
    }
  }

  // Add Bluetooth, if the platform supports it natively
  if (g_Platform) {
    if (g_Platform->startBluetoothScan()) {
      wxSleep(2);
      wxArrayString btscanResults = g_Platform->getBluetoothScanResults();

      unsigned int i = 1;
      while ((i + 1) < btscanResults.GetCount()) {
        wxString item1 = btscanResults[i] + _T(";");
        wxString item2 = btscanResults.Item(i + 1);
        wxString port = item1 + item2;
        port.Prepend(_T("Bluetooth:"));
        m_itemCommListBox->Append(port);

        i += 2;
      }

      g_Platform->stopBluetoothScan();
    }
  }

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
      if (b_connExists) m_itemCommListBox->SetValue(g_uploadConnection);
    } else
      m_itemCommListBox->SetValue(g_uploadConnection);
  } else
    m_itemCommListBox->SetSelection(0);

  comm_box_sizer->Add(m_itemCommListBox, 0, wxEXPAND | wxALL, 5);

  //    Add a reminder text box
  itemBoxSizer2->AddSpacer(20);

  premtext = new wxStaticText(
      this, -1, _("Prepare GPS for Route/Waypoint upload and press Send..."));
  itemBoxSizer2->Add(premtext, 0, wxEXPAND | wxALL, 10);

  //    Create a progress gauge
  wxStaticBox* prog_box = new wxStaticBox(this, wxID_ANY, _("Progress..."));

  wxStaticBoxSizer* prog_box_sizer = new wxStaticBoxSizer(prog_box, wxVERTICAL);
  itemBoxSizer2->Add(prog_box_sizer, 0, wxEXPAND | wxALL, 5);

  m_pgauge = new wxGauge(this, -1, 100);
  prog_box_sizer->Add(m_pgauge, 0, wxEXPAND | wxALL, 5);

  //    OK/Cancel/etc.
  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(itemDialog1, ID_STG_CANCEL, _("Cancel"),
                                wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_SendButton = new wxButton(itemDialog1, ID_STG_OK, _("Send"),
                              wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_SendButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_SendButton->SetDefault();
}

void SendToGpsDlg::SetMessage(wxString msg) {
  if (premtext) {
    premtext->SetLabel(msg);
    premtext->Refresh(true);
  }
}

void SendToGpsDlg::OnSendClick(wxCommandEvent& event) {
  //    Get the selected comm port
  wxString src = m_itemCommListBox->GetValue();
  int tail = src.Find(" - ");
  if (tail != wxNOT_FOUND) {
    src = src.SubString(0, tail);
  }
  if (!src.Lower().StartsWith("tcp") && !src.Lower().StartsWith("udp") &&
      !src.Lower().StartsWith("serial") && !src.Lower().StartsWith("usb:") &&
      !src.Lower().StartsWith("bluetooth")) {
    src = src.Prepend("Serial:");
  }
  g_uploadConnection = src;  // save for persistence

  wxString destPort = src.BeforeFirst(' ');  // Serial:

  // For Bluetooth, we need the entire string
  if (src.Lower().Find(_T("Bluetooth")) != wxNOT_FOUND) destPort = src;

  //    And send it out
  if (m_pRoute) RouteGui(*m_pRoute).SendToGPS(destPort, true, this);
  if (m_pRoutePoint) RoutePointGui(*m_pRoutePoint).SendToGPS(destPort, this);

  //    Show( false );
  //    event.Skip();
  Close();
}

void SendToGpsDlg::OnCancelClick(wxCommandEvent& event) {
  //    Show( false );
  //    event.Skip();
  Close();
}
