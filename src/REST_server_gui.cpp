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

// #include "conn_params.h"
// #include "OCPNPlatform.h"
// #include "route_gui.h"
// #include "route.h"
// #include "route_point_gui.h"
// #include "route_point.h"
// #include "ser_ports.h"

#include "REST_server_gui.h"
#include "FontMgr.h"

IMPLEMENT_DYNAMIC_CLASS(AcceptObjectDialog, wxDialog)

BEGIN_EVENT_TABLE(AcceptObjectDialog, wxDialog)
 EVT_BUTTON(ID_STG_CANCEL, AcceptObjectDialog::OnCancelClick)
 EVT_BUTTON(ID_STG_OK, AcceptObjectDialog::OnOKClick)
END_EVENT_TABLE()

AcceptObjectDialog::AcceptObjectDialog() {
  m_OKButton = NULL;
  m_CancelButton = NULL;
  premtext = NULL;
}

AcceptObjectDialog::AcceptObjectDialog(wxWindow* parent, wxWindowID id,
                           const wxString& caption, const wxString& hint,
                           const wxPoint& pos, const wxSize& size, long style) {
  wxFont* pif = FontMgr::Get().GetFont(_T("Dialog"));
  SetFont( *pif );
  Create(parent, id, caption, hint, pos, size, style);
}

AcceptObjectDialog::~AcceptObjectDialog() {
  delete m_OKButton;
  delete m_CancelButton;
}

bool AcceptObjectDialog::Create(wxWindow* parent, wxWindowID id,
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

void AcceptObjectDialog::CreateControls(const wxString& hint) {
  AcceptObjectDialog* itemDialog1 = this;

   wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
   SetSizer(itemBoxSizer2);

//
//   //      Create the ScrollBox list of available com ports in a labeled static
//   //      box
//   wxStaticBox* comm_box =
//       new wxStaticBox(this, wxID_ANY, _("GPS/Plotter Port"));
//
//   wxStaticBoxSizer* comm_box_sizer = new wxStaticBoxSizer(comm_box, wxVERTICAL);
//   itemBoxSizer2->Add(comm_box_sizer, 0, wxEXPAND | wxALL, 5);

#if 0
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
#endif

  // Add any defined Network connections supporting "output"
  wxArrayString netconns;
#if 0
  if (g_pConnectionParams) {
    for (size_t i = 0; i < g_pConnectionParams->Count(); i++) {
      ConnectionParams* cp = g_pConnectionParams->Item(i);
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
  }
#endif

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
      if (b_connExists) m_itemCommListBox->SetValue(g_uploadConnection);
    } else
      m_itemCommListBox->SetValue(g_uploadConnection);
  } else
#endif
//     m_itemCommListBox->SetSelection(0);
//
//   comm_box_sizer->Add(m_itemCommListBox, 0, wxEXPAND | wxALL, 5);

  //    Add a reminder text box
  itemBoxSizer2->AddSpacer(20);

  premtext = new wxStaticText( this, -1, "A loooooooooooooooooooooooooooooooooooooooooooooong line\n");
  itemBoxSizer2->Add(premtext, 0, wxEXPAND | wxALL, 10);

  //    Create a progress gauge
//   wxStaticBox* prog_box = new wxStaticBox(this, wxID_ANY, _("Progress..."));
//
//   wxStaticBoxSizer* prog_box_sizer = new wxStaticBoxSizer(prog_box, wxVERTICAL);
//   itemBoxSizer2->Add(prog_box_sizer, 0, wxEXPAND | wxALL, 5);
//
//   m_pgauge = new wxGauge(this, -1, 100);
//   prog_box_sizer->Add(m_pgauge, 0, wxEXPAND | wxALL, 5);

  m_pCheck1 = new wxCheckBox(this, ID_STG_CHECK1, m_checkbox1_msg);
  itemBoxSizer2->Add(m_pCheck1, 0, wxEXPAND | wxALL, 10);

  if(!m_checkbox1_msg.Length())
    m_pCheck1->Hide();


  //    OK/Cancel/etc.
  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(itemDialog1, ID_STG_CANCEL, _("Cancel"),
                                wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(itemDialog1, ID_STG_OK, "OK",
                              wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->SetDefault();
}

void AcceptObjectDialog::SetMessage(const wxString &msg) {
  if (premtext) {
    premtext->SetLabel(msg);
    premtext->Refresh(true);
  }
}

void AcceptObjectDialog::SetCheck1Message(const wxString &msg) {
  m_checkbox1_msg = msg;
  m_pCheck1->SetLabel(msg);
  m_pCheck1->Show();
  GetSizer()->Fit(this);
}

void AcceptObjectDialog::OnOKClick(wxCommandEvent& event) {
  EndModal(ID_STG_OK);
}

void AcceptObjectDialog::OnCancelClick(wxCommandEvent& event) {
  EndModal(ID_STG_CANCEL);
}
