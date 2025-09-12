/**************************************************************************
 *   Copyright (C) 2022 Alec Leamas                                        *
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
 ***************************************************************************/

/**
 * \file
 * Miscellaneous globals primarely used by gui layer.
 */

#ifndef GUI_VARS_H__
#define GUI_VARS_H__

#include <wx/colour.h>
#include <wx/string.h>

extern bool AnchorAlertOn1;
extern bool AnchorAlertOn2;
extern bool bDBUpdateInProgress;
extern bool b_inCompressAllCharts;
extern bool g_bAdvanceRouteWaypointOnArrivalOnly;
extern bool g_b_assume_azerty;
extern bool g_bChartBarEx;
extern bool g_bcompression_wait;
extern bool g_bDeferredInitDone;
extern bool g_bhide_context_menus;
extern bool g_bhide_depth_units;
extern bool g_bhide_overzoom_flag;
extern bool g_b_EnableVBO;
extern bool g_b_legacy_input_filter_behaviour;
extern bool g_bopengl;
extern bool g_b_overzoom_x;  ///< Allow high overzoom
extern bool g_bquiting;
extern bool g_bTempShowMenuBar;
extern bool g_btenhertz;
extern bool g_CanvasHideNotificationIcon;
extern bool g_FlushNavobjChanges;
extern bool g_PrintingInProgress;

extern int g_click_stop;
extern int g_sticky_chart;
extern int g_tile_size;
extern int g_uncompressed_tile_size;

extern double AnchorPointMinDist;
extern double g_androidDPmm;  ///< Only used used by __ANDROID__
extern double g_ChartNotRenderScaleFactor;
extern double gCog_gt;
extern double g_defaultBoatSpeedUserUnit;
extern double g_scaler;
extern double vLat; /**< Virtual lat from chcanv popup */
extern double vLon; /**< Virtual lon from chcanv popup */

extern float g_compass_scalefactor;
extern float g_toolbar_scalefactor;

/**
 * ratio to convert between DIP and physical pixels. Important while using
 * OpenGL canvas on macOS with Retina displays where this ratio is 2.0
 */
extern double g_current_monitor_dip_px_ratio;

extern long g_tex_mem_used;

/** Current monitor displaying main application frame */
extern size_t g_current_monitor;

extern wxColour g_colourOwnshipRangeRingsColour;
extern wxColour g_colourTrackLineColour;

extern wxDateTime gTimeSource;

extern wxString g_SENCPrefix;

#endif  // GUI_VARS_H__
