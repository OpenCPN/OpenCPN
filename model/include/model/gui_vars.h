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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 ***************************************************************************/

/**
 * \file
 *
 * Miscellaneous globals primarely used by gui layer, not persisted in
 * configuration file.
 */

#ifndef GUI_VARS_H_
#define GUI_VARS_H_

#include <wx/colour.h>
#include <wx/string.h>

extern bool AnchorAlertOn1;
extern bool AnchorAlertOn2;
extern bool bDBUpdateInProgress;
extern bool b_inCloseWindow;
extern bool b_inCompressAllCharts;  // See extended docs below
extern bool bVelocityValid;
extern bool g_bAdvanceRouteWaypointOnArrivalOnly;
extern bool g_b_assume_azerty;
extern bool g_bChartBarEx;
extern bool g_bcompression_wait;
extern bool g_bCruising;
extern bool g_bDeferredInitDone;
extern bool g_bDeferredStartTrack;
extern bool g_b_EnableVBO;
extern bool g_bFirstRun;
extern bool g_bhide_context_menus;
extern bool g_bhide_depth_units;
extern bool g_bhide_overzoom_flag;
extern bool g_bhide_route_console;
extern bool g_b_legacy_input_filter_behaviour;
extern bool g_bmasterToolbarFull;
extern bool g_bopengl;
extern bool g_b_needFinish;
extern bool g_b_overzoom_x;  ///< Allow high overzoom
extern bool g_bPauseTest;
extern bool g_bquiting;
extern bool g_bSleep;
extern bool g_bTempShowMenuBar;
extern bool g_btenhertz;
extern bool g_bTrackActive;
extern bool g_bTransparentToolbarInOpenGLOK;
extern bool g_bUpgradeInProcess;
extern bool g_CanvasHideNotificationIcon;
extern bool g_disable_main_toolbar;
extern bool g_FlushNavobjChanges;
extern bool g_PrintingInProgress;

extern double AnchorPointMinDist;
extern double g_androidDPmm;  ///< Only used used by __ANDROID__
extern double g_ChartNotRenderScaleFactor;
extern double gCog_gt;
extern double g_current_monitor_dip_px_ratio;  //  See extended docs below
extern double g_defaultBoatSpeedUserUnit;
extern double g_scaler;
extern double vLat; /**< Virtual lat from chcanv popup */
extern double vLon; /**< Virtual lon from chcanv popup */

extern float g_compass_scalefactor;
extern float g_toolbar_scalefactor;

extern int g_canvas_context_menu_disable_mask;
extern int g_ChartUpdatePeriod;
extern int g_blinker_tick;
extern int g_click_stop;
extern int g_FlushNavobjChangesTimeout;
extern int g_lastMemTick;
extern int g_memUsed;
extern int g_mipmap_max_level;
extern int g_NeedDBUpdate;  // See extended docs below
extern int gpIDXn;          // Not used
extern int g_sticky_chart;
extern int g_tick;
extern int g_tile_size;
extern int g_uncompressed_tile_size;
extern int options_lastPage;
extern int quitflag;

extern long g_tex_mem_used;

extern size_t g_current_monitor;  // See extended docs below

extern unsigned gs_plib_flags;

extern wxColour g_colourOwnshipRangeRingsColour;
extern wxColour g_colourTrackLineColour;

extern wxDateTime g_loglast_time;
extern wxDateTime g_start_time;
extern wxDateTime gTimeSource;

extern wxString ChartListFileName;
extern wxString g_csv_locn;
extern wxString gDefaultWorldMapLocation;
extern wxString g_SENCPrefix;
extern wxString* pInit_Chart_Dir;

extern const wxString kUtfCheckMark;
extern const wxString kUtfCircledDivisionSlash;
extern const wxString kUtfFallingDiagonal;
extern const wxString kUtfIdenticalTo;
extern const wxString kUtfLeftArrow;
extern const wxString kUtfLeftRightArrow;
extern const wxString kUtfLeftwardsArrowToBar;
extern const wxString kUtfMultiplicationX;
extern const wxString kUtfRightArrow;

/**
 * \var b_inCompressAllCharts
 *
 * Flag to control adaptive UI scaling.
 *
 * When true, OpenCPN will automatically maximize the application window
 * if the pixel density suggests a touch-friendly device.
 *
 * This helps ensure better usability on mobile and tablet devices by
 * providing a full-screen interface optimized for touch interaction.
 *
 * @note For the most part, the use of this feature is conditionally compiled
 * for Android builds only.
 */

/**
 * \var g_current_monitor
 *
 * Current monitor displaying main application frame
 */

/**
 * \var g_current_monitor_dip_px_ratio
 *
 * ratio to convert between DIP and physical pixels. Important while using
 * OpenGL canvas on macOS with Retina displays where this ratio is 2.0
 */

/**
 * \var g_NeedDBUpdate
 *
 * 0 - No update needed,
 * 1 - Update needed because there is  no chart database, inform user
 * 2 - Start update right away
 */
#endif  // GUI_VARS_H_
