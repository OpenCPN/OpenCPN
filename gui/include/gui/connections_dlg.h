
/***************************************************************************
 *   Copyright (C) 2025 Alec Leamas                                        *
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
 * The ConnectionsDlg class
 */
#ifndef CONNECT_NEW_DLG__
#define CONNECT_NEW_DLG__

#include <vector>

#include <wx/panel.h>
#include <wx/scrolwin.h>

#include "model/conn_params.h"
#include "observable_evtvar.h"
#include "color_types.h"

/**
 * Options | Connections tab implementaion, basically a UI
 * for maintaining the list of connections.
 */
class ConnectionsDlg : public wxPanel {
public:
  /**
   * Create a new ConnectionsDlg instance.
   * @param parent  Containing window
   * @param connections  List of existing connections.
   */
  ConnectionsDlg(wxWindow* parent,
                 const std::vector<ConnectionParams*>& connections);

  /**
   * Make dialog's settings the active ones, usually by updating globals.
   */
  void ApplySettings();

  /**
   * Restore dialog settings from currently used values, usually globals.
   */
  void CancelSettings();

  /**
   * Resize the connections tab
   * @param size New size available for the dialog i.e., GetClientSize()
   *   of containing window.
   */
  void OnResize(const wxSize& size);

  void SetColorScheme(ColorScheme cs);

private:
  void DoApply(wxWindow* root);
  void DoCancel(wxWindow* root);

  const std::vector<ConnectionParams*>& m_connections;

  /** Notified without data when a new connection is added. */
  EventVar m_evt_add_connection;
};

#endif  //  CONNECT_NEW_DLG__
