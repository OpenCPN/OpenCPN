/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2024 by David S. Register                               *
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

#include <wx/statline.h>

#include "model/comm_drv_factory.h"
#include "model/config_vars.h"
#include "model/sys_events.h"

#include "connections_dialog.h"
#include "conn_params_panel.h"
#include "NMEALogWindow.h"
#include "OCPNPlatform.h"
#include "options.h"
#include "priority_gui.h"
#include "connection_edit.h"

extern bool g_bfilter_cogsog;
extern int g_COGFilterSec;
extern int g_SOGFilterSec;


//------------------------------------------------------------------------------
//          ConnectionsDialog Implementation
//------------------------------------------------------------------------------

// Define constructors
ConnectionsDialog::ConnectionsDialog() {}

ConnectionsDialog::ConnectionsDialog(wxScrolledWindow* container,
                                     options* parent) {
  m_container = container;
  m_parent = parent;

  Init();
}

ConnectionsDialog::~ConnectionsDialog() {}

void ConnectionsDialog::SetInitialSettings(void) {

  m_cbNMEADebug->SetValue(false);
  if (NMEALogWindow::Get().GetTTYWindow()) {
    if (NMEALogWindow::Get().GetTTYWindow()->IsShown()) {
      m_cbNMEADebug->SetValue(true);
    }
  }

  FillSourceList();

  //  Reset the touch flag...
  SetSelectedConnectionPanel(nullptr);
}


void ::ConnectionsDialog::OnSize(wxSizeEvent &ev){
  if (m_sbSizerLB)
    m_sbSizerLB->Layout();
}

void ConnectionsDialog::Init() {
  // Setup some initial values
  mSelectedConnection = NULL;

  // Create the UI

  //  Catch SizeEvents to force a layout honoring proportions
  m_container->Connect(wxEVT_SIZE, wxSizeEventHandler(ConnectionsDialog::OnSize), NULL, this);

  wxBoxSizer* bSizer4 = new wxBoxSizer(wxVERTICAL);
  m_container->SetSizer(bSizer4);
  m_container->SetVirtualSize(m_container->GetParent()->GetSize());
  // Do not allow wxScrollWindow m_container to scroll
  m_container->SetScrollRate(0, 0);

  m_bSizerOuterContainer = new wxBoxSizer(wxVERTICAL);
  bSizer4->Add(m_bSizerOuterContainer, 1, wxEXPAND, 5);

  wxStaticBoxSizer* sbSizerGeneral;
  sbSizerGeneral = new wxStaticBoxSizer(
      new wxStaticBox(m_container, wxID_ANY, _("General")), wxVERTICAL);
  m_bSizerOuterContainer->Add(sbSizerGeneral, 0, wxRIGHT | wxEXPAND, 5);

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
  bSizer161->Add(bSizer171, 0, wxEXPAND, 5);

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

  bSizer151->Add(bSizer161, 0, wxEXPAND, 5);
  sbSizerGeneral->Add(bSizer151, 0, wxEXPAND, 5);

  //  Connections listbox, etc
  m_sbSizerLB = new wxStaticBoxSizer(
      new wxStaticBox(m_container, wxID_ANY, _("Data Connections")),
      wxVERTICAL);
  m_bSizerOuterContainer->Add(m_sbSizerLB, 1, wxEXPAND | wxRIGHT, 5);

  // Add/Remove/Edit Buttons
  wxBoxSizer* bSizer18;
  bSizer18 = new wxBoxSizer(wxHORIZONTAL);
  m_sbSizerLB->Add(bSizer18, 0, wxEXPAND, 5);

  m_buttonAdd = new wxButton(m_container, wxID_ANY, _("Add Connection"),
                             wxDefaultPosition, wxDefaultSize, 0);
  bSizer18->Add(m_buttonAdd, 0, wxALL, 5);

  m_buttonEdit = new wxButton(m_container, wxID_ANY, _("Edit Connection"),
                              wxDefaultPosition, wxDefaultSize, 0);
  m_buttonEdit->Enable(FALSE);
  bSizer18->Add(m_buttonEdit, 0, wxALL, 5);

  m_buttonRemove = new wxButton(m_container, wxID_ANY, _("Remove Connection"),
                                wxDefaultPosition, wxDefaultSize, 0);
  m_buttonRemove->Enable(FALSE);
  bSizer18->Add(m_buttonRemove, 0, wxALL, 5);

  wxStaticLine *staticline5 = new wxStaticLine(m_container, wxID_ANY, wxDefaultPosition,
                       wxDefaultSize, wxLI_HORIZONTAL);
  m_sbSizerLB->Add(staticline5, 0, wxEXPAND | wxALL, 5);


  //m_cPanel =
  //    new wxPanel(m_container, wxID_ANY, wxDefaultPosition,
  //                wxDLG_UNIT(m_parent, wxSize(-1, -1)));
  //sbSizerLB->Add(m_cPanel, 0, wxALL | wxEXPAND, 0);


  //wxBoxSizer* boxSizercPanel = new wxBoxSizer(wxVERTICAL);
  //m_cPanel->SetSizer(boxSizercPanel);

#ifdef __ANDROID__
  m_scrollWinConnections =
      new wxPanel(m_container, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                  wxBORDER_RAISED | wxBG_STYLE_ERASE);
#else
  m_scrollWinConnections = new wxScrolledWindow(
      m_container, wxID_ANY, wxDefaultPosition, wxDLG_UNIT(m_parent, wxSize(-1, -1)),
      wxBORDER_RAISED | wxVSCROLL | wxBG_STYLE_ERASE);
  m_scrollWinConnections->SetScrollRate(5, 5);
#endif

  m_sbSizerLB->Add(m_scrollWinConnections, 1, wxRIGHT | wxEXPAND, 5);

  boxSizerConnections = new wxBoxSizer(wxVERTICAL);
  m_scrollWinConnections->SetSizer(boxSizerConnections);

  // Connect controls
  m_buttonAdd->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ConnectionsDialog::OnAddDatasourceClick), NULL,
      this);
  m_buttonEdit->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ConnectionsDialog::OnEditDatasourceClick), NULL,
      this);
  m_buttonRemove->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ConnectionsDialog::OnRemoveDatasourceClick), NULL,
      this);

  m_cbNMEADebug->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(ConnectionsDialog::OnShowGpsWindowCheckboxClick),
      NULL, this);
  //m_cbFilterSogCog->Connect(
  //    wxEVT_COMMAND_CHECKBOX_CLICKED,
  //    wxCommandEventHandler(ConnectionsDialog::OnValChange), NULL, this);
  //m_tFilterSec->Connect(wxEVT_COMMAND_TEXT_UPDATED,
  //                      wxCommandEventHandler(ConnectionsDialog::OnValChange),
  //                      NULL, this);
  //m_cbAPBMagnetic->Connect(
  //    wxEVT_COMMAND_CHECKBOX_CLICKED,
  //    wxCommandEventHandler(ConnectionsDialog::OnValChange), NULL, this);

  m_ButtonPriorityDialog->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ConnectionsDialog::OnPriorityDialog), NULL, this);

}

void ConnectionsDialog::SetSelectedConnectionPanel(
    ConnectionParamsPanel* panel) {
  //  Only one panel can be selected at any time
  //  Clear any selections

  if (mSelectedConnection && mSelectedConnection->m_optionsPanel)
    mSelectedConnection->m_optionsPanel->SetSelected(false);

  if (panel) {
    mSelectedConnection = panel->m_pConnectionParams;
    panel->SetSelected(true);
    m_buttonRemove->Enable();
    m_buttonRemove->Show();
    m_buttonEdit->Enable();
    m_buttonEdit->Show();
    m_buttonAdd->Disable();

  } else {
    mSelectedConnection = NULL;
    m_buttonRemove->Disable();
    m_buttonAdd->Enable();
    m_buttonAdd->Show();
    m_buttonEdit->Disable();
  }

  m_container->FitInside();
  //  Scroll the panel to allow the user to see more of the NMEA parameter
  //  settings area
  wxPoint buttonPosition = m_buttonAdd->GetPosition();
  m_container->Scroll(-1, buttonPosition.y / m_parent->GetScrollRate());
}

void ConnectionsDialog::EnableConnection(ConnectionParams* conn, bool value) {
  if (conn) {
    conn->bEnabled = value;
    // trigger a rebuild/takedown of the connection on "APPLY"
    conn->b_IsSetup = FALSE;  // trigger a rebuild/takedown of the connection
  }
}

bool ConnectionsDialog::SortSourceList(void) {
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

      if (c1->Priority < c2->Priority) {
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
      boxSizerConnections->Add(pPanel, 0, wxEXPAND | wxRIGHT, 10);
    }
  }

  return did_sort;
}

void ConnectionsDialog::FillSourceList(void) {
  m_buttonRemove->Enable(FALSE);

  // Add new panels as necessary
  for (size_t i = 0; i < TheConnectionParams()->Count(); i++) {
    ConnectionParams *cp = TheConnectionParams()->Item(i);
    if (!cp->m_optionsPanel) {
      ConnectionParamsPanel* pPanel = new ConnectionParamsPanel(
          m_scrollWinConnections, wxID_ANY, wxDefaultPosition, wxDefaultSize,
          cp, this);
      pPanel->SetSelected(false);
      boxSizerConnections->Add(pPanel, 0, wxEXPAND | wxRIGHT, 10);
      cp->m_optionsPanel = pPanel;
    } else {
      cp->m_optionsPanel->Update(cp);
    }
  }
  SortSourceList();

  mSelectedConnection = NULL;
  mSelectedConnection = NULL;
  m_buttonAdd->Enable(true);
  m_buttonAdd->Show();
}

void ConnectionsDialog::UpdateSourceList(bool bResort) {
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

void ConnectionsDialog::OnAddDatasourceClick(wxCommandEvent& event) {
  //  Unselect all panels
  for (size_t i = 0; i < TheConnectionParams()->Count(); i++)
    TheConnectionParams()->Item(i)->m_optionsPanel->SetSelected(false);

  ConnectionEditDialog dialog(m_parent, this);
  dialog.SetSize(m_parent->GetSize());  // fill the entire "settings" dialog space
  dialog.SetPropsLabel(_("Configure new connection"));
  dialog.SetDefaultConnectionParams();

  auto rv = dialog.ShowModal();
  if (rv == wxID_OK) {
    ConnectionParams* cp = dialog.GetParamsFromControls();
    if (cp) {
      cp->b_IsSetup = false;  // Trigger new stream
      TheConnectionParams()->Add(cp);
      FillSourceList();
    }
    UpdateDatastreams();
    m_sbSizerLB->Layout();
  }
}

void ConnectionsDialog::OnRemoveDatasourceClick(wxCommandEvent& event) {
  if (mSelectedConnection) {
    // Find the index
    int index = -1;
    ConnectionParams* cp = NULL;
    for (size_t i = 0; i < TheConnectionParams()->Count(); i++) {
      cp = TheConnectionParams()->Item(i);
      if (mSelectedConnection == cp) {
        index = i;
        break;
      }
    }

    if ((index >= 0) && (cp)) {
      delete TheConnectionParams()->Item(index)->m_optionsPanel;
      TheConnectionParams()->RemoveAt(index);
      StopAndRemoveCommDriver(cp->GetStrippedDSPort(), cp->GetCommProtocol());
      mSelectedConnection = NULL;

      FillSourceList();
    }
    m_buttonEdit->Disable();
    m_buttonRemove->Disable();

    m_sbSizerLB->Layout();
  }
}

void ConnectionsDialog::OnEditDatasourceClick(wxCommandEvent& event) {
  if (mSelectedConnection) {
    // Find the index
    int index = -1;
    ConnectionParams* cp = NULL;
    for (size_t i = 0; i < TheConnectionParams()->Count(); i++) {
      cp = TheConnectionParams()->Item(i);
      if (mSelectedConnection == cp) {
        index = i;
        break;
      }
    }

    if ((index >= 0) && (cp)) {
      ConnectionEditDialog dialog(m_parent, this);
      dialog.SetSize(m_parent->GetSize());  // fill the entire "settings" dialog space
      dialog.SetPropsLabel(_("Edit Selected Connection"));
      // Preload the dialog contents
      dialog.PreloadControls(cp);

      auto rv = dialog.ShowModal();
      if (rv == wxID_OK){
        ConnectionParams* cp_edited = dialog.GetParamsFromControls();
        delete cp->m_optionsPanel;
        StopAndRemoveCommDriver(cp->GetStrippedDSPort(), cp->GetCommProtocol());

        TheConnectionParams()->Item(index) = cp_edited;
        cp_edited->b_IsSetup = false;  // Trigger new stream
        FillSourceList();
        UpdateDatastreams();
        m_sbSizerLB->Layout();
      }
    }
  }
}



void ConnectionsDialog::OnShowGpsWindowCheckboxClick(wxCommandEvent& event) {
  if (!m_cbNMEADebug->GetValue()) {
    NMEALogWindow::Get().DestroyWindow();
  } else {
    NMEALogWindow::Get().Create((wxWindow*)(m_parent->pParent), 35);

    // Try to ensure that the log window is a least a little bit visible
    wxRect logRect(
        NMEALogWindow::Get().GetPosX(), NMEALogWindow::Get().GetPosY(),
        NMEALogWindow::Get().GetSizeW(), NMEALogWindow::Get().GetSizeH());

    if (m_container->GetRect().Contains(logRect)) {
      NMEALogWindow::Get().SetPos(
          m_container->GetRect().x / 2,
          (m_container->GetRect().y +
           (m_container->GetRect().height - logRect.height) / 2));
      NMEALogWindow::Get().Move();
    }

    m_parent->Raise();
  }
}

void ConnectionsDialog::ApplySettings() {
  g_bfilter_cogsog = m_cbFilterSogCog->GetValue();

  long filter_val = 1;
  m_tFilterSec->GetValue().ToLong(&filter_val);
  g_COGFilterSec =
      wxMin(static_cast<int>(filter_val),
            60 /*MAX_COGSOG_FILTER_SECONDS*/);  // FIXME (dave)  should be
  g_COGFilterSec = wxMax(g_COGFilterSec, 1);
  g_SOGFilterSec = g_COGFilterSec;

  g_bMagneticAPB = m_cbAPBMagnetic->GetValue();
  //g_NMEAAPBPrecision = m_choicePrecision->GetCurrentSelection();

  g_bGarminHostUpload = m_cbGarminUploadHost->GetValue();
  g_GPS_Ident = m_cbFurunoGP3X->GetValue() ? "FurunoGP3X" : "Generic";

  UpdateDatastreams();
}

void ConnectionsDialog::UpdateDatastreams() {
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
}


void ConnectionsDialog::OnPriorityDialog(wxCommandEvent& event) {
  PriorityDlg* pdlg = new PriorityDlg(m_parent);
  pdlg->ShowModal();
  delete pdlg;
}


