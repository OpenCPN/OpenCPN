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

#include <wx/app.h>
#include <wx/tokenzr.h>

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#include "qdebug.h"
#endif

#include "priority_gui.h"
#include "ocpn_app.h"
#include "comm_bridge.h"
#include "ocpn_frame.h"

extern MyFrame *gFrame;

wxDECLARE_APP(MyApp);

class PriorityEntry : public wxTreeItemData {
public:
  PriorityEntry(int category, int index){ m_category = category, m_index=index; }
  ~PriorityEntry() {};

  int m_category, m_index;
};




PriorityDlg::PriorityDlg(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, _("Adjust Comm Priorities"), wxDefaultPosition,
               wxSize(480, 420), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{

  m_selIndex = 0;
  m_selmap_index = 0;
  m_selID = 0;

  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  wxBoxSizer* secondSizer = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(secondSizer, 1, wxEXPAND, 5);

  wxStaticBox* pclbBox = new wxStaticBox(this, wxID_ANY, _("Priority List"));
  wxStaticBoxSizer* stcSizer = new wxStaticBoxSizer(pclbBox, wxVERTICAL);
  secondSizer->Add(stcSizer, 1, wxALL | wxEXPAND, 5);

  m_prioTree = new wxTreeCtrl(this,wxID_ANY, wxDefaultPosition,
                                       wxDefaultSize);
  stcSizer->Add(m_prioTree, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer* btnEntrySizer = new wxBoxSizer(wxVERTICAL);
  secondSizer->Add(btnEntrySizer, 0, wxALL | wxEXPAND, 5);
  btnMoveUp = new wxButton(this, wxID_ANY, _("Move Up"));
  btnMoveDown = new wxButton(this, wxID_ANY, _("Move Down"));
  btnMoveUp->Disable();
  btnMoveDown->Disable();

  btnEntrySizer->Add(btnMoveUp, 0, wxALL, 5);
  btnEntrySizer->Add(btnMoveDown, 0, wxALL, 5);

  btnEntrySizer->AddSpacer(15);

  btnRefresh = new wxButton(this, wxID_ANY, _("Refresh"));
  btnClear = new wxButton(this, wxID_ANY, _("Clear All"));

  btnEntrySizer->Add(btnRefresh, 0, wxALL, 5);
  btnEntrySizer->Add(btnClear, 0, wxALL, 5);

#ifndef __WXMSW__
  //FIXME (dave)    gtk crashes on second call to tree->DeleteAllItems()...Why?
  btnRefresh->Hide();
  btnClear->Hide();
#endif

  wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
  wxButton* btnOK = new wxButton(this, wxID_OK);
  wxButton* btnCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
  btnSizer->AddButton(btnOK);
  btnSizer->AddButton(btnCancel);
  btnSizer->Realize();
  mainSizer->Add(btnSizer, 0, wxALL | wxEXPAND, 5);

  // Connect Events
  btnMoveUp->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                   wxCommandEventHandler(PriorityDlg::OnMoveUpClick), NULL,
                   this);
  btnMoveDown->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(PriorityDlg::OnMoveDownClick), NULL,
                     this);

  btnRefresh->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(PriorityDlg::OnRefreshClick), NULL,
                     this);

  btnClear->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(PriorityDlg::OnClearClick), NULL,
                     this);

  m_prioTree->Connect(wxEVT_TREE_SEL_CHANGED,
                           wxCommandEventHandler(PriorityDlg::OnItemSelected),
                           NULL, this);

  // Get the current status
  MyApp& app = wxGetApp();
  m_map = app.m_comm_bridge.GetPriorityMaps();

  Populate();

  int n_lines = wxMax(m_prioTree->GetCount(), 15);

  stcSizer->SetMinSize(m_maxStringLength * GetCharWidth() * 15 / 10,
                       wxMin(gFrame->GetSize().y * 3 /4 , n_lines * GetCharHeight()));


  Layout();
  mainSizer->Fit(this);
  Centre();
}


void PriorityDlg::AddLeaves(const std::vector<std::string> &map_list,
                            size_t map_index, std::string map_name,
                            wxTreeItemId leaf_parent){
  if(map_list.size() < (size_t)map_index)
    return;

  // Get the current Priority container for this branch
  MyApp& app = wxGetApp();
  PriorityContainer pc = app.m_comm_bridge.GetPriorityContainer(map_name);

  wxString priority_string(map_list[map_index].c_str());
  wxStringTokenizer tk(priority_string, "|");
  size_t index = 0;
  while (tk.HasMoreTokens()) {
    wxString item_string = tk.GetNextToken();

    // Record the maximum display string length, for use in dialog sizing.
    m_maxStringLength = wxMax(m_maxStringLength, item_string.Length());

    PriorityEntry *pe = new PriorityEntry(map_index, index);
    wxTreeItemId id_tk = m_prioTree->AppendItem(leaf_parent, item_string, -1, -1, pe);

    //  Set bold text on item currently active (usually 0)
    if ( (size_t)(pc.active_priority) == index)
      m_prioTree->SetItemBold(id_tk);

    if ((map_index == m_selmap_index) && (index == m_selIndex))
      m_selID = id_tk;
    index++;
  }
}


void PriorityDlg::Populate() {

  m_prioTree->Unselect();
  m_prioTree->DeleteAllItems();
  m_maxStringLength = 15;   // default width calculation

//  wxTreeItemId* rootData = new wxDirItemData(_T("Dummy"), _T("Dummy"), TRUE);
  wxTreeItemId m_rootId = m_prioTree->AddRoot(_("Priorities"), -1, -1, NULL);
  m_prioTree->SetItemHasChildren(m_rootId);

  wxTreeItemId id_position = m_prioTree->AppendItem(m_rootId, _("Position"), -1, -1, NULL);
  m_prioTree->SetItemHasChildren(id_position);
  AddLeaves(m_map, 0, "position", id_position);

  wxTreeItemId id_velocity = m_prioTree->AppendItem(m_rootId, _("Speed/Course"), -1, -1, NULL);
  m_prioTree->SetItemHasChildren(id_velocity);
  AddLeaves(m_map, 1, "velocity", id_velocity);

  wxTreeItemId id_heading = m_prioTree->AppendItem(m_rootId, _("Heading"), -1, -1, NULL);
  m_prioTree->SetItemHasChildren(id_heading);
  AddLeaves(m_map, 2, "heading", id_heading);

  wxTreeItemId id_magvar = m_prioTree->AppendItem(m_rootId, _("Mag Variation"), -1, -1, NULL);
  m_prioTree->SetItemHasChildren(id_magvar);
  AddLeaves(m_map, 3, "variation", id_magvar);

  wxTreeItemId id_sats = m_prioTree->AppendItem(m_rootId, _("Satellites"), -1, -1, NULL);
  m_prioTree->SetItemHasChildren(id_sats);
  AddLeaves(m_map, 4, "satellites", id_sats);

  m_prioTree->ExpandAll();

  if(m_selID)
    m_prioTree->SelectItem(m_selID);

#if 0
  wxString dirname;
  int nDir = dir_array.GetCount();
  for (int i = 0; i < nDir; i++) {
    wxString dirname = dir_array[i];
    if (!dirname.IsEmpty()) {
      wxDirItemData* dir_item = new wxDirItemData(dirname, dirname, TRUE);
      wxTreeItemId id = ptc->AppendItem(m_rootId, dirname, 0, -1, dir_item);

      // wxWidgets bug workaraound (Ticket #10085)
      ptc->SetItemText(id, dirname);
      if (pFont) ptc->SetItemFont(id, *pFont);

        // On MacOS, use the default system dialog color, to honor Dark mode.
#ifndef __WXOSX__
      ptc->SetItemTextColour(id, col);
#endif
      ptc->SetItemHasChildren(id);
    }
  }
#endif

}


void PriorityDlg::OnItemSelected(wxCommandEvent& event){
  btnMoveUp->Disable();
  btnMoveDown->Disable();

  wxTreeItemId id = m_prioTree->GetSelection();
  PriorityEntry *pe = (PriorityEntry *)m_prioTree->GetItemData(id);
  if (!pe)
    return;

  m_selIndex = pe->m_index;
  m_selmap_index = pe->m_category;

  if (pe->m_index > 0){
    btnMoveUp->Enable();
  }

  wxTreeItemId id_parent = m_prioTree->GetItemParent(id);

  // count siblings
  int n_sibs = 0;
  wxTreeItemIdValue cookie;
  wxTreeItemId ch = m_prioTree->GetFirstChild(id_parent, cookie);
  while (ch.IsOk()) {
    n_sibs++;
    ch = m_prioTree->GetNextChild(id_parent, cookie);
  }

  if (pe->m_index < n_sibs-1)
    btnMoveDown->Enable();
}

void PriorityDlg::OnMoveUpClick(wxCommandEvent& event) {
  ProcessMove(m_prioTree->GetSelection(), -1);
}

void PriorityDlg::OnMoveDownClick(wxCommandEvent& event) {
  ProcessMove(m_prioTree->GetSelection(), 1);
}

void PriorityDlg::ProcessMove(wxTreeItemId id, int dir){

  // Get the extra data
  PriorityEntry *pe = (PriorityEntry *)m_prioTree->GetItemData(id);
  if (!pe)
    return;
  if(pe->m_category >4)
    return;

  // Get the selected category string from the map
  wxString priority_string = wxString(m_map[pe->m_category].c_str());

  // Build an array
  wxString prio_array[16];     // enough, plus

  wxStringTokenizer tk(priority_string, "|");
  int index = 0;
  while (tk.HasMoreTokens() && index < 16) {
    prio_array[index] = tk.GetNextToken();
    index++;
  }
  int max_index = index;

  // perform the action
  if (dir == -1){     // Move UP
    if (pe->m_index > 0){
      // swap entries in array
      wxString s_above = prio_array[pe->m_index - 1];
      wxString s_move = prio_array[pe->m_index];
      prio_array[pe->m_index - 1] = s_move;
      prio_array[pe->m_index] = s_above;
      m_selIndex--;
    }
  }
  else{               // Move DOWN
    if (pe->m_index < max_index){
      // swap entries in array
      wxString s_below = prio_array[pe->m_index + 1];
      wxString s_move = prio_array[pe->m_index];
      prio_array[pe->m_index + 1] = s_move;
      prio_array[pe->m_index] = s_below;
      m_selIndex++;
    }
  }

  // create the new string
  wxString prio_mod;
  for (int i=0 ; i < 16 ; i++){
    if (prio_array[i].Length()){
      prio_mod += prio_array[i];
      prio_mod += wxString("|");
    }
  }

  // update the string in the map
  std::string s_upd(prio_mod.c_str());
  m_map[pe->m_category] = s_upd;

  AdjustSatPriority();

  // Update the priority mechanism
  MyApp& app = wxGetApp();
  app.m_comm_bridge.UpdateAndApplyMaps(m_map);

  // And reload the tree GUI
  m_map = app.m_comm_bridge.GetPriorityMaps();
  Populate();
}

void PriorityDlg::OnRefreshClick(wxCommandEvent& event) {
  // Reload the tree GUI
  MyApp& app = wxGetApp();
  m_map = app.m_comm_bridge.GetPriorityMaps();
  Populate();
}

void PriorityDlg::OnClearClick(wxCommandEvent& event) {
  m_map[0].clear();
  m_map[1].clear();
  m_map[2].clear();
  m_map[3].clear();
  m_map[4].clear();

  // Update the priority mechanism
  MyApp& app = wxGetApp();
  app.m_comm_bridge.UpdateAndApplyMaps(m_map);

  // And reload the tree GUI
  m_map = app.m_comm_bridge.GetPriorityMaps();
  Populate();

}

void PriorityDlg::AdjustSatPriority() {

  // Get an array of available sat sources
  std::string sat_prio = m_map[4];
  wxArrayString sat_sources;
  wxString sat_priority_string(sat_prio.c_str());
  wxStringTokenizer tks(sat_priority_string, "|");
  while (tks.HasMoreTokens()) {
    wxString item_string = tks.GetNextToken();
    sat_sources.Add(item_string);
  }

  // Step thru the POS priority map
  std::string pos_prio = m_map[0];
  wxString pos_priority_string(pos_prio.c_str());
  wxStringTokenizer tk(pos_priority_string, "|");
  wxArrayString new_sat_prio;
  while (tk.HasMoreTokens()) {
    wxString item_string = tk.GetNextToken();
    wxString pos_channel = item_string.BeforeFirst(';');

    // search the sat sources array for a match
    // if found, add to proposed new priority array
    for (size_t i = 0 ; i < sat_sources.GetCount(); i++){
      if (pos_channel.IsSameAs(sat_sources[i].BeforeFirst(';'))){
        new_sat_prio.Add(sat_sources[i]);
        // Mark this source as "used"
        sat_sources[i] = "USED";
        break;
      }
      else {      // no match, what to do? //FIXME (dave)
        int yyp = 4;
      }
    }
  }
    //  Create a new sat priority string from new_sat_prio array
  wxString proposed_sat_prio;
  for (size_t i = 0 ; i < new_sat_prio.GetCount(); i++){
    proposed_sat_prio += new_sat_prio[i];
    proposed_sat_prio += wxString("|");
  }

  // Update the maps with the new sat priority string
  m_map[4] = proposed_sat_prio.ToStdString();
}
