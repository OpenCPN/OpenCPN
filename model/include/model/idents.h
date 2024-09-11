/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 * Author:   David Register
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
 **************************************************************************/

#ifndef _IDENTS_H__
#define _IDENTS_H__

#include <wx/event.h>
#include <wx/toolbar.h>

#define TIMER_GFRAME_1 999

#define ID_QUIT 101
#define ID_CM93ZOOMG 102

//    ToolBar Constants
const int ID_TOOLBAR = 500;

enum {
  // The following constants represent the toolbar items (some are also used in
  // menus). They MUST be in the SAME ORDER as on the toolbar and new items MUST
  // NOT be added amongst them, due to the way the toolbar button visibility is
  // saved and calculated.
  ID_ZOOMIN = 1550,
  ID_ZOOMOUT,
  ID_STKUP,
  ID_STKDN,
  ID_ROUTE,
  ID_FOLLOW,
  ID_SETTINGS,
  ID_AIS,
  ID_ENC_TEXT,
  ID_CURRENT,
  ID_TIDE,
  ID_PRINT,
  ID_ROUTEMANAGER,
  ID_TRACK,
  ID_COLSCHEME,
  ID_ABOUT,
  ID_MOB,
  ID_TBEXIT,
  ID_TBSTAT,
  ID_TBSTATBOX,
  ID_MASTERTOGGLE,

  ID_PLUGIN_BASE  // This MUST be the last item in the enum
};

static const long TOOLBAR_STYLE = wxTB_FLAT | wxTB_DOCKABLE | wxTB_TEXT;

enum {
  IDM_TOOLBAR_TOGGLETOOLBARSIZE = 200,
  IDM_TOOLBAR_TOGGLETOOLBARORIENT,
  IDM_TOOLBAR_TOGGLETOOLBARROWS,
  IDM_TOOLBAR_ENABLEPRINT,
  IDM_TOOLBAR_DELETEPRINT,
  IDM_TOOLBAR_INSERTPRINT,
  IDM_TOOLBAR_TOGGLEHELP,
  IDM_TOOLBAR_TOGGLE_TOOLBAR,
  IDM_TOOLBAR_TOGGLE_ANOTHER_TOOLBAR,
  IDM_TOOLBAR_CHANGE_TOOLTIP,
  IDM_TOOLBAR_SHOW_TEXT,
  IDM_TOOLBAR_SHOW_ICONS,
  IDM_TOOLBAR_SHOW_BOTH,

  ID_COMBO = 1000
};

// Menu item IDs for the main menu bar
enum {
  ID_MENU_ZOOM_IN = 2000,
  ID_MENU_ZOOM_OUT,
  ID_MENU_SCALE_IN,
  ID_MENU_SCALE_OUT,

  ID_MENU_NAV_FOLLOW,
  ID_MENU_NAV_TRACK,

  ID_MENU_CHART_NORTHUP,
  ID_MENU_CHART_COGUP,
  ID_MENU_CHART_HEADUP,
  ID_MENU_CHART_QUILTING,
  ID_MENU_CHART_OUTLINES,

  ID_MENU_UI_CHARTBAR,
  ID_MENU_UI_COLSCHEME,
  ID_MENU_UI_FULLSCREEN,

  ID_MENU_ENC_TEXT,
  ID_MENU_ENC_LIGHTS,
  ID_MENU_ENC_SOUNDINGS,
  ID_MENU_ENC_ANCHOR,
  ID_MENU_ENC_DATA_QUALITY,

  ID_MENU_SHOW_TIDES,
  ID_MENU_SHOW_CURRENTS,

  ID_MENU_TOOL_MEASURE,
  ID_MENU_ROUTE_MANAGER,
  ID_MENU_ROUTE_NEW,
  ID_MENU_MARK_BOAT,
  ID_MENU_MARK_CURSOR,
  ID_MENU_MARK_MOB,

  ID_MENU_AIS_TARGETS,
  ID_MENU_AIS_MOORED_TARGETS,
  ID_MENU_AIS_SCALED_TARGETS,
  ID_MENU_AIS_TRACKS,
  ID_MENU_AIS_CPADIALOG,
  ID_MENU_AIS_CPASOUND,
  ID_MENU_AIS_TARGETLIST,
  ID_MENU_AIS_CPAWARNING,

  ID_MENU_SETTINGS_BASIC,

  ID_MENU_OQUIT,

  ID_CMD_SELECT_CHART_TYPE,
  ID_CMD_SELECT_CHART_FAMILY,
  ID_CMD_INVALIDATE,
  ID_CMD_CLOSE_ALL_DIALOGS,

  ID_MENU_SHOW_NAVOBJECTS,
};

//      A global definition for window, timer and other ID's as needed.
enum {
  ID_NMEA_WINDOW = wxID_HIGHEST,
  ID_AIS_WINDOW,
  INIT_TIMER,
  FRAME_TIMER_1,
  FRAME_TIMER_2,
  TIMER_AIS1,
  TIMER_DSC,
  TIMER_AISAUDIO,
  AIS_SOCKET_Isa,
  FRAME_TIMER_DOG,
  FRAME_TC_TIMER,
  FRAME_COG_TIMER,
  MEMORY_FOOTPRINT_TIMER,
  BELLS_TIMER,
  ID_NMEA_THREADMSG,
  RESIZE_TIMER,
  TOOLBAR_ANIMATE_TIMER,
  RECAPTURE_TIMER,
  WATCHDOG_TIMER

};

enum { TIME_TYPE_UTC = 1, TIME_TYPE_LMT, TIME_TYPE_COMPUTER };

#define DS_SOCKET_ID 5001
#define DS_SERVERSOCKET_ID 5002
#define DS_ACTIVESERVERSOCKET_ID 5003

#endif  // _IDENTS_H__
