/***************************************************************************
 *
 * Project:  OpenCPN
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

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include <wx/tokenzr.h>
#include <wx/intl.h>

#include <wx/statline.h>
#include "conn_params_panel.h"

#include "ocpn_plugin.h"
#include "options.h"
#include "color_handler.h"

#if !wxUSE_XLOCALE && wxCHECK_VERSION(3, 0, 0)
#define wxAtoi(arg) atoi(arg)
#endif


/** A wxStaticText bold label with correct width, see #2538 */
class ConnBoldLabel: public wxStaticText {
public:
  ConnBoldLabel(wxWindow* parent, const wxString& label)
      : wxStaticText(parent, wxID_ANY, "") {
    font = parent->GetFont();
    font.MakeBold();
    SetFont(font);
    SetLabel(label);
    Connect(wxEVT_LEFT_DOWN,
            wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
            parent);
  }

  void SetLabel(const wxString& label) {
    wxStaticText::SetLabel(label);
    dc.SetFont(font);
    auto size = dc.GetTextExtent(label).Scale(1.1, 1.1);
    SetMinSize(size);
  }

private:
  wxScreenDC dc;
  wxFont font;
};


extern "C" bool GetGlobalColor(wxString colorName, wxColour *pcolour);

BEGIN_EVENT_TABLE(ConnectionParamsPanel, wxPanel)
EVT_PAINT(ConnectionParamsPanel::OnPaint)
EVT_ERASE_BACKGROUND(ConnectionParamsPanel::OnEraseBackground)
END_EVENT_TABLE()

ConnectionParamsPanel::ConnectionParamsPanel(
    wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
    ConnectionParams *p_itemConnectionParams, ConnectionsDialog *pContainer)
    : wxPanel(parent, id, pos, size, wxBORDER_NONE) {
  m_pContainer = pContainer;
  m_pConnectionParams = p_itemConnectionParams;
  m_bSelected = false;

  wxFont *dFont = GetOCPNScaledFont_PlugIn(_("Dialog"));
  SetFont(*dFont);

  int refHeight = GetCharHeight();

  //  This controls the basic heght when later added to a vertical sizer
  // SetMinSize(wxSize(-1, 6 * refHeight));

  Connect(wxEVT_LEFT_DOWN,
          wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL, this);
  CreateControls();
}

ConnectionParamsPanel::~ConnectionParamsPanel() {
  if (m_pConnectionParams) m_pConnectionParams->m_optionsPanel = nullptr;
}

void ConnectionParamsPanel::OnSelected(wxMouseEvent &event) {
  if (!m_bSelected) {
    SetSelected(true);
    m_pContainer->SetSelectedConnectionPanel(this);
  } else {
    SetSelected(false);
    m_pContainer->SetSelectedConnectionPanel(NULL);
  }
}

void ConnectionParamsPanel::SetSelected(bool selected) {
  m_bSelected = selected;
  wxColour colour;
  int refHeight = GetCharHeight();

  if (selected) {
    m_boxColour = GetDialogColor(DLG_HIGHLIGHT);
  } else {
    m_boxColour = GetDialogColor(DLG_BACKGROUND);
  }

  wxWindowList kids = GetChildren();
  for (unsigned int i = 0; i < kids.GetCount(); i++) {
    wxWindowListNode *node = kids.Item(i);
    wxWindow *win = node->GetData();
    win->SetBackgroundColour(m_boxColour);
  }

  GetSizer()->Layout();
  Refresh(true);
}

void ConnectionParamsPanel::OnEnableCBClick(wxCommandEvent &event) {
  if (m_pContainer) {
    m_pContainer->EnableConnection(m_pConnectionParams, event.IsChecked());
  }
}

void ConnectionParamsPanel::CreateControls(void) {
  int metric = GetCharHeight();

  wxFont *dFont = GetOCPNScaledFont_PlugIn(_("Dialog"));
  double font_size = dFont->GetPointSize() * 17 / 16;
  wxFont *bFont = wxTheFontList->FindOrCreateFont(
      font_size, dFont->GetFamily(), dFont->GetStyle(), wxFONTWEIGHT_BOLD);

  wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  mainSizer->AddSpacer(metric);

  wxBoxSizer *panelSizer = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(panelSizer, 0, wxLEFT, 5);

  mainSizer->AddSpacer(metric);

  // Enable cbox
  wxBoxSizer *enableSizer = new wxBoxSizer(wxVERTICAL);
  panelSizer->Add(enableSizer, 1);

  m_cbEnable = new wxCheckBox(this, wxID_ANY, _("Enable"));
  m_cbEnable->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionParamsPanel::OnEnableCBClick), NULL,
      this);
  m_cbEnable->SetValue(m_pConnectionParams->bEnabled);

  enableSizer->Add(m_cbEnable, 1, wxLEFT | wxEXPAND, metric);

  //  Parms
  wxBoxSizer *parmSizer = new wxBoxSizer(wxVERTICAL);
  panelSizer->Add(parmSizer, 5);

  if (m_pConnectionParams->Type == SERIAL) {
    wxFlexGridSizer *serialGrid = new wxFlexGridSizer(2, 7, 0, metric / 2);
    serialGrid->SetFlexibleDirection(wxHORIZONTAL);
    parmSizer->Add(serialGrid, 0, wxALIGN_LEFT);

    wxString ioDir = m_pConnectionParams->GetIOTypeValueStr();

    wxStaticText *t1 = new wxStaticText(this, wxID_ANY, _("Type"));
    serialGrid->Add(t1, 0, wxALIGN_CENTER_HORIZONTAL);
    t1->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t3 = new wxStaticText(this, wxID_ANY, _T(""));
    serialGrid->Add(t3, 0, wxALIGN_CENTER_HORIZONTAL);
    t3->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t5 = new wxStaticText(this, wxID_ANY, _("Direction"));
    serialGrid->Add(t5, 0, wxALIGN_CENTER_HORIZONTAL);
    t5->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t11 = new wxStaticText(this, wxID_ANY, _("Protocol"));
    serialGrid->Add(t11, 0, wxALIGN_CENTER_HORIZONTAL);
    t11->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t13 = new wxStaticText(this, wxID_ANY, _("Serial Port"));
    serialGrid->Add(t13, 0, wxALIGN_CENTER_HORIZONTAL);
    t13->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t15 = new wxStaticText(this, wxID_ANY, _("Baudrate"));
    serialGrid->Add(t15, 0, wxALIGN_CENTER_HORIZONTAL);
    t15->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t17 = new wxStaticText(this, wxID_ANY, _("List position"));
    serialGrid->Add(t17, 0, wxALIGN_CENTER_HORIZONTAL);
    t17->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    // line 2
    t2 = new ConnBoldLabel(this, _("Serial"));
    serialGrid->Add(t2, 0, wxALIGN_CENTER_HORIZONTAL);

    t4 = new ConnBoldLabel(this, "");
    serialGrid->Add(t4, 0, wxALIGN_CENTER_HORIZONTAL);

    t6 = new ConnBoldLabel(this, ioDir);
    serialGrid->Add(t6, 0, wxALIGN_CENTER_HORIZONTAL);

    wxString proto;
    switch (m_pConnectionParams->Protocol) {
      case PROTO_NMEA0183:
        proto = "NMEA 0183";
        break;
      case PROTO_NMEA2000:
        proto = "NMEA 2000";
        break;
      default:
        proto = _("Undefined");
        break;
    }

    t12 = new ConnBoldLabel(this, proto);
    serialGrid->Add(t12, 0, wxALIGN_CENTER_HORIZONTAL);

    t14 = new ConnBoldLabel(this, m_pConnectionParams->Port);
    serialGrid->Add(t14, 0, wxALIGN_CENTER_HORIZONTAL);

    auto baudRate = wxString::Format("%d", m_pConnectionParams->Baudrate);
    t16 = new ConnBoldLabel(this, baudRate);
    serialGrid->Add(t16, 0, wxALIGN_CENTER_HORIZONTAL);

    auto priority = wxString::Format("%d", m_pConnectionParams->Priority);
    t18 = new ConnBoldLabel(this, priority);
    serialGrid->Add(t18, 0, wxALIGN_CENTER_HORIZONTAL);

    wxStaticLine *line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
                                          wxDefaultSize, wxLI_HORIZONTAL);
    parmSizer->Add(line, 0, wxEXPAND);
    line->Connect(wxEVT_LEFT_DOWN,
                  wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                  this);

    t21 = new wxStaticText(this, wxID_ANY,
                           _("Comment: ") + m_pConnectionParams->UserComment);
    parmSizer->Add(t21, 0);
    t21->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

  }

  else if (m_pConnectionParams->Type == NETWORK) {
    wxString ioDir = m_pConnectionParams->GetIOTypeValueStr();

    wxFlexGridSizer *netGrid = new wxFlexGridSizer(2, 7, 0, metric / 2);
    netGrid->SetFlexibleDirection(wxHORIZONTAL);
    parmSizer->Add(netGrid, 0, wxALIGN_LEFT);

    wxStaticText *t1 = new wxStaticText(this, wxID_ANY, _("Type"));
    netGrid->Add(t1, 0, wxALIGN_CENTER_HORIZONTAL);
    t1->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t3 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t3, 0, wxALIGN_CENTER_HORIZONTAL);
    t3->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t5 = new wxStaticText(this, wxID_ANY, _("Direction"));
    netGrid->Add(t5, 0, wxALIGN_CENTER_HORIZONTAL);
    t5->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t11 = new wxStaticText(this, wxID_ANY, _("Protocol"));
    netGrid->Add(t11, 0, wxALIGN_CENTER_HORIZONTAL);
    t11->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t13 = new wxStaticText(this, wxID_ANY, _("Network Address"));
    netGrid->Add(t13, 0, wxALIGN_CENTER_HORIZONTAL);
    t13->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t15 = new wxStaticText(this, wxID_ANY, _("Network Port"));
    netGrid->Add(t15, 0, wxALIGN_CENTER_HORIZONTAL);
    t15->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t17 = new wxStaticText(this, wxID_ANY, _("List position"));
    netGrid->Add(t17, 0, wxALIGN_CENTER_HORIZONTAL);
    t17->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    // line 2
    t2 = new wxStaticText(this, wxID_ANY, _("Network"));
    t2->SetFont(*bFont);
    netGrid->Add(t2, 0, wxALIGN_CENTER_HORIZONTAL);
    t2->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    t4 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t4, 0, wxALIGN_CENTER_HORIZONTAL);
    t4->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    t6 = new wxStaticText(this, wxID_ANY, ioDir);
    t6->SetFont(*bFont);
    netGrid->Add(t6, 0, wxALIGN_CENTER_HORIZONTAL);
    t6->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxString proto;
    switch (m_pConnectionParams->NetProtocol) {
      case UDP:
        proto = _T("UDP");
        if (m_pConnectionParams->Protocol == PROTO_NMEA0183)
          proto << " N0183";
        else if (m_pConnectionParams->Protocol == PROTO_NMEA2000)
          proto << " N2000";
        break;
      case TCP:
        proto = _T("TCP");
        if (m_pConnectionParams->Protocol == PROTO_NMEA0183)
          proto << " N0183";
        else if (m_pConnectionParams->Protocol == PROTO_NMEA2000)
          proto << " N2000";
        break;
      case GPSD:
        proto = _T("GPSD");
        break;
      case SIGNALK:
        proto = _T("Signal K");
        break;
      default:
        proto = _("Undefined");
        break;
    }

    t12 = new wxStaticText(this, wxID_ANY, proto);
    t12->SetFont(*bFont);
    netGrid->Add(t12, 0, wxALIGN_CENTER_HORIZONTAL);
    t12->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxString address = m_pConnectionParams->NetworkAddress;
    t14 = new wxStaticText(this, wxID_ANY, address);
    t14->SetFont(*bFont);
    netGrid->Add(t14, 0, wxALIGN_CENTER_HORIZONTAL);
    t14->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxString port;
    port.Printf(_T("%d"), m_pConnectionParams->NetworkPort);
    t16 = new wxStaticText(this, wxID_ANY, port);
    t16->SetFont(*bFont);
    netGrid->Add(t16, 0, wxALIGN_CENTER_HORIZONTAL);
    t16->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxString priority;
    priority.Printf(_T("%d"), m_pConnectionParams->Priority);
    t18 = new wxStaticText(this, wxID_ANY, priority);
    t18->SetFont(*bFont);
    netGrid->Add(t18, 0, wxALIGN_CENTER_HORIZONTAL);
    t18->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticLine *line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
                                          wxDefaultSize, wxLI_HORIZONTAL);
    parmSizer->Add(line, 0, wxEXPAND);
    line->Connect(wxEVT_LEFT_DOWN,
                  wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                  this);

    t21 = new wxStaticText(this, wxID_ANY,
                           _("Comment: ") + m_pConnectionParams->UserComment);
    parmSizer->Add(t21, 0);
    t21->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);
  }

  else if (m_pConnectionParams->Type == INTERNAL_GPS) {
    wxString ioDir = m_pConnectionParams->GetIOTypeValueStr();

    wxFlexGridSizer *netGrid = new wxFlexGridSizer(2, 7, 0, metric / 2);
    netGrid->SetFlexibleDirection(wxHORIZONTAL);
    parmSizer->Add(netGrid, 0, wxALIGN_LEFT);

    wxStaticText *t1 = new wxStaticText(this, wxID_ANY, _("Type"));
    netGrid->Add(t1, 0, wxALIGN_CENTER_HORIZONTAL);
    t1->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t3 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t3, 0, wxALIGN_CENTER_HORIZONTAL);
    t3->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t5 = new wxStaticText(this, wxID_ANY, _("Direction"));
    netGrid->Add(t5, 0, wxALIGN_CENTER_HORIZONTAL);
    t5->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t11 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t11, 0, wxALIGN_CENTER_HORIZONTAL);
    t11->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t13 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t13, 0, wxALIGN_CENTER_HORIZONTAL);
    t13->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t15 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t15, 0, wxALIGN_CENTER_HORIZONTAL);
    t15->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t17 = new wxStaticText(this, wxID_ANY, _("List position"));
    netGrid->Add(t17, 0, wxALIGN_CENTER_HORIZONTAL);
    t17->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    // line 2
    t2 = new wxStaticText(this, wxID_ANY, _("Built-in GPS"));
    t2->SetFont(*bFont);
    netGrid->Add(t2, 0, wxALIGN_CENTER_HORIZONTAL);
    t2->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    t4 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t4, 0, wxALIGN_CENTER_HORIZONTAL);
    t4->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    t6 = new wxStaticText(this, wxID_ANY, ioDir);
    t6->SetFont(*bFont);
    netGrid->Add(t6, 0, wxALIGN_CENTER_HORIZONTAL);
    t6->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxString proto = _T("");

    t12 = new wxStaticText(this, wxID_ANY, proto);
    t12->SetFont(*bFont);
    netGrid->Add(t12, 0, wxALIGN_CENTER_HORIZONTAL);
    t12->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxString address;
    t14 = new wxStaticText(this, wxID_ANY, address);
    t14->SetFont(*bFont);
    netGrid->Add(t14, 0, wxALIGN_CENTER_HORIZONTAL);
    t14->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxString port;
    t16 = new wxStaticText(this, wxID_ANY, port);
    t16->SetFont(*bFont);
    netGrid->Add(t16, 0, wxALIGN_CENTER_HORIZONTAL);
    t16->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxString priority;
    priority.Printf(_T("%d"), m_pConnectionParams->Priority);
    t18 = new wxStaticText(this, wxID_ANY, priority);
    t18->SetFont(*bFont);
    netGrid->Add(t18, 0, wxALIGN_CENTER_HORIZONTAL);
    t18->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticLine *line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
                                          wxDefaultSize, wxLI_HORIZONTAL);
    parmSizer->Add(line, 0, wxEXPAND);
    line->Connect(wxEVT_LEFT_DOWN,
                  wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                  this);

    t21 = new wxStaticText(this, wxID_ANY,
                           _("Comment: ") + m_pConnectionParams->UserComment);
    parmSizer->Add(t21, 0);
    t21->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

  } else if (m_pConnectionParams->Type == INTERNAL_BT) {
    wxString ioDir = m_pConnectionParams->GetIOTypeValueStr();

    wxFlexGridSizer *netGrid = new wxFlexGridSizer(2, 7, 0, metric / 2);
    netGrid->SetFlexibleDirection(wxHORIZONTAL);
    parmSizer->Add(netGrid, 0, wxALIGN_LEFT);

    wxStaticText *t1 = new wxStaticText(this, wxID_ANY, _("Type"));
    netGrid->Add(t1, 0, wxALIGN_CENTER_HORIZONTAL);
    t1->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t3 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t3, 0, wxALIGN_CENTER_HORIZONTAL);
    t3->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t5 = new wxStaticText(this, wxID_ANY, _("Direction"));
    netGrid->Add(t5, 0, wxALIGN_CENTER_HORIZONTAL);
    t5->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t11 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t11, 0, wxALIGN_CENTER_HORIZONTAL);
    t11->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t13 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t13, 0, wxALIGN_CENTER_HORIZONTAL);
    t13->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t15 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t15, 0, wxALIGN_CENTER_HORIZONTAL);
    t15->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t17 = new wxStaticText(this, wxID_ANY, _("List position"));
    netGrid->Add(t17, 0, wxALIGN_CENTER_HORIZONTAL);
    t17->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    // line 2
    t2 = new wxStaticText(this, wxID_ANY, _("Built-in Bluetooth"));
    t2->SetFont(*bFont);
    netGrid->Add(t2, 0, wxALIGN_CENTER_HORIZONTAL);
    t2->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    t4 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t4, 0, wxALIGN_CENTER_HORIZONTAL);
    t4->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    t6 = new wxStaticText(this, wxID_ANY, ioDir);
    t6->SetFont(*bFont);
    netGrid->Add(t6, 0, wxALIGN_CENTER_HORIZONTAL);
    t6->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxString proto = _T("");

    t12 = new wxStaticText(this, wxID_ANY, proto);
    t12->SetFont(*bFont);
    netGrid->Add(t12, 0, wxALIGN_CENTER_HORIZONTAL);
    t12->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxString address;
    t14 = new wxStaticText(this, wxID_ANY, address);
    t14->SetFont(*bFont);
    netGrid->Add(t14, 0, wxALIGN_CENTER_HORIZONTAL);
    t14->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxString port;
    t16 = new wxStaticText(this, wxID_ANY, port);
    t16->SetFont(*bFont);
    netGrid->Add(t16, 0, wxALIGN_CENTER_HORIZONTAL);
    t16->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxString priority;
    priority.Printf(_T("%d"), m_pConnectionParams->Priority);
    t18 = new wxStaticText(this, wxID_ANY, priority);
    t18->SetFont(*bFont);
    netGrid->Add(t18, 0, wxALIGN_CENTER_HORIZONTAL);
    t18->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticLine *line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
                                          wxDefaultSize, wxLI_HORIZONTAL);
    parmSizer->Add(line, 0, wxEXPAND);
    line->Connect(wxEVT_LEFT_DOWN,
                  wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                  this);

    t21 = new wxStaticText(this, wxID_ANY,
                           _("Comment: ") + m_pConnectionParams->UserComment);
    parmSizer->Add(t21, 0);
    t21->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);
  } else if (m_pConnectionParams->Type == SOCKETCAN) {
    wxFlexGridSizer *netGrid = new wxFlexGridSizer(2, 7, 0, metric / 2);
    netGrid->SetFlexibleDirection(wxHORIZONTAL);
    parmSizer->Add(netGrid, 0, wxALIGN_LEFT);

    wxStaticText *t1 = new wxStaticText(this, wxID_ANY, _("Type"));
    netGrid->Add(t1, 0, wxALIGN_CENTER_HORIZONTAL);
    t1->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t3 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t3, 0, wxALIGN_CENTER_HORIZONTAL);
    t3->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t5 = new wxStaticText(this, wxID_ANY, _("Driver"));
    netGrid->Add(t5, 0, wxALIGN_CENTER_HORIZONTAL);
    t5->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxStaticText *t11 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t11, 0, wxALIGN_CENTER_HORIZONTAL);
    t11->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t13 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t13, 0, wxALIGN_CENTER_HORIZONTAL);
    t13->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t15 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t15, 0, wxALIGN_CENTER_HORIZONTAL);
    t15->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticText *t17 = new wxStaticText(this, wxID_ANY, "");
    netGrid->Add(t17, 0, wxALIGN_CENTER_HORIZONTAL);
    t17->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    // line 2
    t2 = new wxStaticText(this, wxID_ANY, "socketCan");
    t2->SetFont(*bFont);
    netGrid->Add(t2, 0, wxALIGN_CENTER_HORIZONTAL);
    t2->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    t4 = new wxStaticText(this, wxID_ANY, _T(""));
    netGrid->Add(t4, 0, wxALIGN_CENTER_HORIZONTAL);
    t4->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    t6 = new wxStaticText(this, wxID_ANY, m_pConnectionParams->socketCAN_port);
    t6->SetFont(*bFont);
    netGrid->Add(t6, 0, wxALIGN_CENTER_HORIZONTAL);
    t6->Connect(wxEVT_LEFT_DOWN,
                wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                this);

    wxString proto = _T("");

    t12 = new wxStaticText(this, wxID_ANY, proto);
    t12->SetFont(*bFont);
    netGrid->Add(t12, 0, wxALIGN_CENTER_HORIZONTAL);
    t12->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxString address;
    t14 = new wxStaticText(this, wxID_ANY, address);
    t14->SetFont(*bFont);
    netGrid->Add(t14, 0, wxALIGN_CENTER_HORIZONTAL);
    t14->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxString port;
    t16 = new wxStaticText(this, wxID_ANY, port);
    t16->SetFont(*bFont);
    netGrid->Add(t16, 0, wxALIGN_CENTER_HORIZONTAL);
    t16->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxString priority;
    t18 = new wxStaticText(this, wxID_ANY, "");
    t18->SetFont(*bFont);
    netGrid->Add(t18, 0, wxALIGN_CENTER_HORIZONTAL);
    t18->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);

    wxStaticLine *line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition,
                                          wxDefaultSize, wxLI_HORIZONTAL);
    parmSizer->Add(line, 0, wxEXPAND);
    line->Connect(wxEVT_LEFT_DOWN,
                  wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                  this);

    t21 = new wxStaticText(this, wxID_ANY,
                           _("Comment: ") + m_pConnectionParams->UserComment);
    parmSizer->Add(t21, 0);
    t21->Connect(wxEVT_LEFT_DOWN,
                 wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
                 this);
  }
}

void ConnectionParamsPanel::Update(ConnectionParams *ConnectionParams) {
  m_pConnectionParams = ConnectionParams;

  wxString ioDir = m_pConnectionParams->GetIOTypeValueStr();
  wxString priority;
  priority.Printf(_T("%d"), m_pConnectionParams->Priority);

  if (m_pConnectionParams->Type == SERIAL) {
    wxString baudRate;
    baudRate.Printf(_T("%d"), m_pConnectionParams->Baudrate);

    wxString proto;
    switch (m_pConnectionParams->Protocol) {
      case PROTO_NMEA0183:
        proto = _T("NMEA 0183");
        break;
      case PROTO_NMEA2000:
        proto = _T("NMEA 2000");
        break;
      default:
        proto = _("Undefined");
        break;
    }

    t2->SetLabel(_("Serial"));
    t6->SetLabel(ioDir);
    t12->SetLabel(proto);
    t14->SetLabel(m_pConnectionParams->Port);
    t16->SetLabel(baudRate);
    t18->SetLabel(priority);

    t21->SetLabel(_("Comment: ") + m_pConnectionParams->UserComment);
  } else if (m_pConnectionParams->Type == NETWORK) {
    wxString proto;
    switch (m_pConnectionParams->NetProtocol) {
      case UDP:
        proto = _T("UDP");
        if (m_pConnectionParams->Protocol == PROTO_NMEA0183)
          proto << " N0183";
        else if (m_pConnectionParams->Protocol == PROTO_NMEA2000)
          proto << " N2000";
        break;
      case TCP:
        proto = _T("TCP");
        if (m_pConnectionParams->Protocol == PROTO_NMEA0183)
          proto << " N0183";
        else if (m_pConnectionParams->Protocol == PROTO_NMEA2000)
          proto << " N2000";
        break;
      case GPSD:
        proto = _T("GPSD");
        break;
      case SIGNALK:
        proto = _T("Signal K");
        break;
      default:
        proto = _("Undefined");
        break;
    }
    wxString port;
    port.Printf(_T("%d"), m_pConnectionParams->NetworkPort);

    t2->SetLabel(_("Network"));
    t6->SetLabel(ioDir);
    t12->SetLabel(proto);
    t14->SetLabel(m_pConnectionParams->NetworkAddress);
    t16->SetLabel(port);
    t18->SetLabel(priority);

    t21->SetLabel(_("Comment: ") + m_pConnectionParams->UserComment);
  } else if (m_pConnectionParams->Type == INTERNAL_GPS) {
    t21->SetLabel(_("Comment: ") + m_pConnectionParams->UserComment);
  }

  else if (m_pConnectionParams->Type == INTERNAL_BT) {
    t21->SetLabel(_("Comment: ") + m_pConnectionParams->UserComment);
  }

  else if (m_pConnectionParams->Type == SOCKETCAN) {
    t21->SetLabel(_("Comment: ") + m_pConnectionParams->UserComment);
    t6->SetLabel(m_pConnectionParams->socketCAN_port);
  }

  GetSizer()->Layout();
}

void ConnectionParamsPanel::OnEraseBackground(wxEraseEvent &event) {}

void ConnectionParamsPanel::OnPaint(wxPaintEvent &event) {
  int width, height;
  GetSize(&width, &height);
  wxPaintDC dc(this);

  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.SetBrush(wxBrush(GetBackgroundColour()));
  dc.DrawRectangle(GetVirtualSize());

  wxColour c;

  wxString nameString = m_pConnectionParams->Serialize();

  wxFont *dFont = GetOCPNScaledFont_PlugIn(_("Dialog"));

  if (m_bSelected) {
    dc.SetBrush(wxBrush(m_boxColour));

    GetGlobalColor(_T ( "UITX1" ), &c);
    dc.SetPen(wxPen(wxColor(0xCE, 0xD5, 0xD6), 3));

    dc.DrawRoundedRectangle(0, 0, width - 1, height - 1, height / 10);

    // Draw the thumbnail

    dc.SetTextForeground(wxColour(0, 0, 0));
  } else {
    dc.SetBrush(wxBrush(m_boxColour));

    GetGlobalColor(_T ( "UITX1" ), &c);
    dc.SetPen(wxPen(c, 1));

    int offset = height / 10;
    dc.DrawRectangle(offset, offset, width - (2 * offset),
                     height - (2 * offset));

    dc.SetTextForeground(wxColour(128, 128, 128));
  }
}
