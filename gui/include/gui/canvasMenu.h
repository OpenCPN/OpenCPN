/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  CanvasMenuHandler
 * Author:   David Register
 *
 ***************************************************************************
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#ifndef __CANVASMENU_H__
#define __CANVASMENU_H__

#include "bbox.h"

#include <wx/datetime.h>
#include <wx/treectrl.h>
#include <wx/dirctrl.h>
#include <wx/sound.h>
#include <wx/grid.h>
#include <wx/wxhtml.h>

#include "ocpndc.h"
#include "undo.h"

#include "ocpCursor.h"
#include "S57QueryDialog.h"
#include "GoToPositionDialog.h"
#include "DetailSlider.h"
#include "RolloverWin.h"
#include "AISTargetQueryDialog.h"
#include "timers.h"
#include "emboss_data.h"

class wxGLContext;
class GSHHSChart;
class IDX_entry;

//----------------------------------------------------------------------------
//    Forward Declarations
//----------------------------------------------------------------------------
class Route;
class TCWin;
class RoutePoint;
class SelectItem;
class BoundingBox;
class ocpnBitmap;
class WVSChart;
class MyFrame;
class ChartBaseBSB;
class ChartBase;
class AisTargetData;
class S57ObjectTree;
class S57ObjectDesc;
class RolloverWin;
class Quilt;
class PixelCache;
class ChInfoWin;
class glChartCanvas;
class Track;

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
