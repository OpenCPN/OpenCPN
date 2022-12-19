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
 */

#ifndef _PRIORITY_DIALOG_H
#define _PRIORITY_DIALOG_H

#include <vector>
#include <wx/checkbox.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/timer.h>
#include <wx/treectrl.h>

class PriorityDlg : public wxDialog {
public:
  PriorityDlg(wxWindow *parent);

private:
  void OnMoveUpClick(wxCommandEvent& event);
  void OnMoveDownClick(wxCommandEvent& event);
  void OnItemSelected(wxCommandEvent& event);

  void ProcessMove(wxTreeItemId, int dir);

  void Populate();
  void AddLeaves(const std::vector<std::string> &map_list,
                            size_t map_index,
                            wxTreeItemId leaf_parent);

  wxButton *btnMoveUp, *btnMoveDown;
  wxTreeCtrl *m_prioTree;
  std::vector<std::string> m_map;

  size_t m_selmap_index;
  size_t m_selIndex;
  wxTreeItemId m_selID;

  int m_maxStringLength;
};


#endif    //_PRIORITY_DIALOG_H
