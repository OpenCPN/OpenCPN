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
bool b_inCompressAllCharts = false;
bool g_bAdvanceRouteWaypointOnArrivalOnly = false;
bool g_b_assume_azerty = false;
bool g_bChartBarEx = false;
bool g_bcompression_wait = false;
bool g_bDeferredInitDone = false;
bool g_bhide_context_menus = false;
bool g_bhide_depth_units = false;
bool g_bhide_overzoom_flag = false;
bool g_b_EnableVBO = false;
bool g_b_legacy_input_filter_behaviour = false;
bool g_b_overzoom_x = true;
bool g_bquiting = false;
bool g_bTempShowMenuBar = false;
bool g_CanvasHideNotificationIcon = false;
bool g_FlushNavobjChanges = false;
bool g_PrintingInProgress = false;

double AnchorPointMinDist;
double g_androidDPmm = 0.0;
double g_ChartNotRenderScaleFactor = 1.0;
double gCog_gt = 0;
double g_defaultBoatSpeedUserUnit = 0;
double g_scaler = 0;
double vLat = 0;
double vLon = 0;
double g_current_monitor_dip_px_ratio = 1.0;

size_t g_current_monitor = 0;

float g_compass_scalefactor = 0;
float g_toolbar_scalefactor = 0;

int g_click_stop = 0;
int g_mipmap_max_level = 4;
int g_sticky_chart = 0;
int g_tile_size = 0;
int g_uncompressed_tile_size = 0;

long g_tex_mem_used = 0;
wxDateTime gTimeSource;

wxColour g_colourOwnshipRangeRingsColour;
wxColour g_colourTrackLineColour;

wxString g_SENCPrefix;
