/**************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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
 * OCPN_AUIManager
 */

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/aui/aui.h>

class OCPN_AUIManager;  // forward

extern OCPN_AUIManager* g_pauimgr;  ///< Global instance

class OCPN_AUIManager : public wxAuiManager {
public:
  OCPN_AUIManager(wxWindow* managed_wnd = NULL,
                  unsigned int flags = wxAUI_MGR_DEFAULT);
  ~OCPN_AUIManager();

  void OnMotionx(wxMouseEvent& event);
  bool DoEndResizeAction(wxMouseEvent& event);
  void OnLeftUp(wxMouseEvent& event);
  bool ProcessDockResult(wxAuiPaneInfo& target, const wxAuiPaneInfo& new_pos);

  wxAuiDockInfo* FindDock(wxAuiPaneInfo& pane);
  void SetDockSize(wxAuiDockInfo* d, int size);

  wxRect m_0actionHintRect;

  DECLARE_EVENT_TABLE()
};
