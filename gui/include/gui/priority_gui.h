/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Input priorities management dialog
 */

#ifndef _PRIORITY_DIALOG_H
#define _PRIORITY_DIALOG_H

#include <string>
#include <vector>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/font.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/timer.h>
#include <wx/treectrl.h>

class PriorityDlg : public wxDialog {
public:
  PriorityDlg(wxWindow *parent);
  virtual ~PriorityDlg();

private:
  void OnMoveUpClick(wxCommandEvent &event);
  void OnMoveDownClick(wxCommandEvent &event);
  void OnRefreshClick(wxCommandEvent &event);
  void OnClearClick(wxCommandEvent &event);
  void OnItemSelected(wxCommandEvent &event);

  void ProcessMove(wxTreeItemId, int dir);

  void Populate();
  void AddLeaves(const std::vector<std::string> &map_list, size_t map_index,
                 std::string map_name, wxTreeItemId leaf_parent);
  void AdjustSatPriority();
  void AdjustCOGSOGPriority();

  wxButton *btnMoveUp, *btnMoveDown;
  wxButton *btnRefresh, *btnClear;
  wxTreeCtrl *m_prioTree;
  std::vector<std::string> m_map;

  size_t m_selmap_index;
  size_t m_selIndex;

  int m_maxStringLength;
  wxString m_max_string;
  wxFont *m_pF;
};

#endif  //_PRIORITY_DIALOG_H
