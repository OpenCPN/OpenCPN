/**************************************************************************
 *   Copyright (C) 2021 Alec Leamas                                        *
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

/**
 * \file
 * Implement gui_vars.h
 */

#include <wx/colour.h>
#include <wx/datetime.h>

#include "model/gui_vars.h"

bool AnchorAlertOn1 = false;
bool AnchorAlertOn2 = false;
bool bDBUpdateInProgress = false;
bool b_inCloseWindow = false;
bool b_inCompressAllCharts = false;
bool bVelocityValid = false;
bool g_bAdvanceRouteWaypointOnArrivalOnly = false;
bool g_b_assume_azerty = false;
bool g_bChartBarEx = false;
bool g_bcompression_wait = false;
bool g_bCruising = false;
bool g_bDeferredInitDone = false;
bool g_bDeferredStartTrack = false;
bool g_b_EnableVBO = false;
bool g_bFirstRun = false;
bool g_bhide_context_menus = false;
bool g_bhide_depth_units = false;
bool g_bhide_overzoom_flag = false;
bool g_bhide_route_console = false;
bool g_b_legacy_input_filter_behaviour = false;
bool g_bmasterToolbarFull = true;
bool g_b_overzoom_x = true;
bool g_bPauseTest = false;
bool g_bquiting = false;
bool g_bSleep = false;
bool g_bTempShowMenuBar = false;
bool g_bTrackActive = false;
bool g_bTransparentToolbarInOpenGLOK = false;
bool g_bUpgradeInProcess = false;
bool g_CanvasHideNotificationIcon = false;
bool g_disable_main_toolbar = false;
bool g_FlushNavobjChanges = false;
bool g_PrintingInProgress = false;
bool g_bShowTimeline = true;

double AnchorPointMinDist;
double g_androidDPmm = 0.0;
double g_ChartNotRenderScaleFactor = 1.0;
double gCog_gt = 0;
double g_current_monitor_dip_px_ratio = 1.0;
double g_defaultBoatSpeedUserUnit = 0;
double g_scaler = 0;
double vLat = 0;
double vLon = 0;

size_t g_current_monitor = 0;

float g_compass_scalefactor = 0;
float g_toolbar_scalefactor = 0;

int g_ChartUpdatePeriod = 0;
int g_click_stop = 0;
int g_FlushNavobjChangesTimeout = 0;
int g_lastMemTick = -1;
int g_memUsed = 0;
int g_mipmap_max_level = 4;
int gpIDXn = 0;
int g_sticky_chart = 0;
int g_tick = 0;
int g_tile_size = 0;
int g_uncompressed_tile_size = 0;
int quitflag = 0;
;
int options_lastPage = 0;
int g_NeedDBUpdate = 0;

long g_tex_mem_used = 0;

unsigned gs_plib_flags = 0;

wxDateTime g_loglast_time;
wxDateTime g_start_time;
wxDateTime gTimeSource;

wxColour g_colourOwnshipRangeRingsColour;
wxColour g_colourTrackLineColour;

wxString ChartListFileName;
wxString gDefaultWorldMapLocation;
wxString g_SENCPrefix;
wxString g_csv_locn;

wxString* pInit_Chart_Dir;
