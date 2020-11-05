/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

//
// This defines the capabilities of the software controls.

#ifndef CTD_AUTO_NO
#define CTD_AUTO_NO \
  { wxT("") }
#endif
#ifndef CTD_AUTO_YES
#define CTD_AUTO_YES \
  { _("Auto") }
#endif
#ifndef CTD_AUTO_OFF
#define CTD_AUTO_OFF \
  { _("Off") }
#endif

#ifndef CTD_DEF_OFF
#define CTD_DEF_OFF -100000
#endif
#ifndef CTD_DEF_ZERO
#define CTD_DEF_ZERO 0
#endif
#ifndef CTD_MIN_ZERO
#define CTD_MIN_ZERO 0
#endif
#ifndef CTD_MAX_100
#define CTD_MAX_100 100
#endif
#ifndef CTD_STEP_1
#define CTD_STEP_1 1
#endif
#ifndef CTD_STEP_10
#define CTD_STEP_10 10
#endif

// Standard values for unit, when names == 1 long it is the unit
#ifndef CTD_NUMERIC
#define CTD_NUMERIC \
  { wxT("") }
#endif
#ifndef CTD_PERCENTAGE
#define CTD_PERCENTAGE \
  { wxT("%") }
#endif
#ifndef CTD_MINUTES
#define CTD_MINUTES \
  { _("min") }
#endif

//
// Make an entry for all control types. Specify all supported (or all?) controls.
//
// Macro:   Either HAVE_CONTROL (if it has this) or SKIP_CONTROL (if it doesn't have it)
// Field 1: Control identifier
// Field 2: 0 = No auto, n = maximum auto value
//          For instance, Navico radars have multiple values for Auto Sea State.
// Field 3: Default value for initial setting (most values are stored in settings)
// Field 4: Minimal value
// Field 5: Maximum value
// Field 6: Step (for instance, min 0 / max 100 / step 10 gives 11 possible values)
// Field 7: Names
//
// The following are controls that are software only. These should not be varied
// per radar type.

// Note that due to how the C++ preprocessor works, { } must be defined in a nested define

#ifndef ORIENTATION_NAMES
#define ORIENTATION_NAMES \
  { _("Head up"), _("Head up (Stabilized)"), _("North up"), _("Course up"), _("Unknown") }
#endif
// Orientation HEAD_UP is available if there is no heading or dev mode is switched on
// Other orientations are available if there is a heading
#define ORIENTATION_HEAD_UP \
  (0)  // Unstabilized heading (as if without compass)                                     // Available if no compass or in dev mode
#define ORIENTATION_STABILIZED_UP (1)  // Stabilized heading (averaged over a few seconds)
#define ORIENTATION_NORTH_UP (2)       // North up
#define ORIENTATION_COG_UP (3)         // Averaged GPS COG up (same way as OpenCPN)
#define ORIENTATION_NUMBER (4)

#ifndef VIEW_NAMES
#define VIEW_NAMES \
  { _("Dragged View"), _("Center View"), _("Forward View"), _("Aft View") }
#endif
#define DRAGGED_VIEW (0)
#define CENTER_VIEW (1)
#define FORWARD_VIEW (2)
#define BACKWARD_VIEW (3)

#ifndef TARGET_TRAIL_NAMES
#define TARGET_TRAIL_NAMES \
  { _("15 sec"), _("30 sec"), _("1 min"), _("3 min"), _("5 min"), _("10 min"), _("Continuous") }
#endif
#ifndef TRAIL_MOTION_NAMES
#define TRAIL_MOTION_NAMES \
  { _("Relative"), _("True") }
#endif
#define TARGET_MOTION_RELATIVE (0)
#define TARGET_MOTION_TRUE (1)
#ifndef OFF_ON_NAMES
#define OFF_ON_NAMES \
  { _("Off"), _("On") }
#endif
#ifndef TARGET_ON_PPI_NAMES
#define TARGET_ON_PPI_NAMES \
  { _("Off"), _("On") }
#endif

HAVE_CONTROL(CT_ANTENNA_FORWARD, CTD_AUTO_NO, CTD_DEF_ZERO, -500, +500, CTD_STEP_1, CTD_NUMERIC)
HAVE_CONTROL(CT_ANTENNA_STARBOARD, CTD_AUTO_NO, CTD_DEF_ZERO, -100, +100, CTD_STEP_1, CTD_NUMERIC)
HAVE_CONTROL(CT_MAIN_BANG_SIZE, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, CTD_MAX_100, CTD_STEP_1, CTD_NUMERIC)
HAVE_CONTROL(CT_ORIENTATION, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, ORIENTATION_NUMBER - 1, CTD_STEP_1, ORIENTATION_NAMES)
HAVE_CONTROL(CT_CENTER_VIEW, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, ORIENTATION_NUMBER - 1, CTD_STEP_1, VIEW_NAMES)
HAVE_CONTROL(CT_OVERLAY_CANVAS, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 1, CTD_STEP_1, OFF_ON_NAMES)
HAVE_CONTROL(CT_REFRESHRATE, CTD_AUTO_NO, 1, 1, 5, CTD_STEP_1, CTD_NUMERIC)
HAVE_CONTROL(CT_TRANSPARENCY, CTD_AUTO_NO, 5, MIN_OVERLAY_TRANSPARENCY, MAX_OVERLAY_TRANSPARENCY, CTD_STEP_10, CTD_PERCENTAGE)
HAVE_CONTROL(CT_TARGET_TRAILS, CTD_AUTO_NO, CTD_DEF_OFF, CTD_MIN_ZERO, 6, CTD_STEP_1, TARGET_TRAIL_NAMES)
HAVE_CONTROL(CT_TRAILS_MOTION, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 1, CTD_STEP_1, TRAIL_MOTION_NAMES)
HAVE_CONTROL(CT_TARGET_ON_PPI, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 1, CTD_STEP_1, TARGET_ON_PPI_NAMES)
