/**************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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
 * Canvas context (right click) menu handler
 */

#ifndef __CANVASMENU_H__
#define __CANVASMENU_H__

#include "gl_headers.h"

#include <wx/glcanvas.h>
#include <wx/treectrl.h>

#include "model/route.h"
#include "model/route_point.h"

#include "chcanv.h"
#include "ocp_cursor.h"

class ChartCanvas;  // circular

//----------------------------------------------------------------------------
//    Forward Declarations
//----------------------------------------------------------------------------

/**
 * Handles context menu events for the chart canvas. Manages the creation and
 * handling of context menus that appear when right-clicking on the chart
 * canvas.
 */
class CanvasMenuHandler : public wxEvtHandler {
public:
  CanvasMenuHandler(ChartCanvas *parentCanvas, Route *selectedRoute,
                    Track *selectedTrack, RoutePoint *selectedPoint,
                    int selectedAIS_MMSI, void *selectedTCIndex,
                    wxWindow *nmea_log);

  ~CanvasMenuHandler();

  void CanvasPopupMenu(int x, int y, int seltype);
  void PopupMenuHandler(wxCommandEvent &event);
  static int GetNextContextMenuId();
  void PrepareMenuItem(wxMenuItem *item);
  void MenuPrepend1(wxMenu *menu, int id, wxString label);
  void MenuAppend1(wxMenu *menu, int id, wxString label);
  void SetMenuItemFont1(wxMenuItem *item);

  static wxFont m_scaledFont;

private:
  void AddPluginContextMenuItems(wxMenu *contextMenu, wxMenu *menuRoute,
                                 wxMenu *menuTrack, wxMenu *menuWaypoint,
                                 wxMenu *menuAIS);

  int popx, popy;
  ChartCanvas *parent;
  Route *m_pSelectedRoute;
  Track *m_pSelectedTrack;
  RoutePoint *m_pFoundRoutePoint;
  Route *m_pHead;  // for split function
  Route *m_pTail;
  int m_SelectedIdx;
  int m_FoundAIS_MMSI;
  void *m_pIDXCandidate;
  double m_DIPFactor;
  wxWindow *m_nmea_log;
};

#endif
