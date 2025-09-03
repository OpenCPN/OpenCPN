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

extern bool g_bTempShowMenuBar;
extern bool AnchorAlertOn2;
extern bool AnchorAlertOn1;
extern bool g_FlushNavobjChanges;
extern bool bDBUpdateInProgress;
extern bool g_bNavAidRadarRingsShown;
extern bool g_b_overzoom_x;  ///< Allow high overzoom
extern bool g_bquiting;
extern bool g_bopengl;
extern bool g_b_assume_azerty;
extern bool g_bDeferredInitDone;
extern bool g_CanvasHideNotificationIcon;
extern bool g_bhide_context_menus;
extern bool g_bhide_depth_units;
extern bool g_bhide_overzoom_flag;
extern bool g_bChartBarEx;
extern bool g_PrintingInProgress;

extern int g_click_stop;
extern int g_sticky_chart;

extern double AnchorPointMinDist;
extern double vLat; /**< Virtual lat from chcanv popup */
extern double vLon; /**< Virtual lon from chcanv popup */
extern double g_ChartNotRenderScaleFactor;
extern double g_defaultBoatSpeedUserUnit;
extern double gCog_gt;

extern float g_compass_scalefactor;
extern float g_toolbar_scalefactor;

extern wxColour g_colourOwnshipRangeRingsColour;
extern wxDateTime gTimeSource;
#endif  // GUI_VARS_H__
