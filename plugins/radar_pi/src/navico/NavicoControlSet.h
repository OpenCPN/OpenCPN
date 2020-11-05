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
// This defines the capabilities of *all* navico radars, e.g. the common parts
// Ranges are not included because those are radar dependent.
//

#include "SoftwareControlSet.h"

//
// Make an entry for all control types. Specify all supported (or all?) controls.
//
// Macro:   HAVE_CONTROL (if it has this)
// Field 1: Control identifier
// Field 2: 0 = No auto, n = maximum auto value
//          For instance, Navico radars have multiple values for Auto Sea State.
// Field 3: Default value for initial setting (most values are stored in settings)
// Field 4: Minimal value
// Field 5: Maximum value
// Field 6: Step (for instance, min 0 / max 100 / step 10 gives 11 possible values)
// Field 7: Names
//
// Note that for range the min, max, step values are ignored, see TODO.

#ifndef OFF_LOW_MEDIUM_HIGH_NAMES
#define OFF_LOW_MEDIUM_HIGH_NAMES \
  { _("Off"), _("Low"), _("Medium"), _("High") }
#endif
#ifndef OFF_LOW_HIGH_NAMES
#define OFF_LOW_HIGH_NAMES \
  { _("Off"), _("Low"), _("High") }
#endif
#ifndef SLOW_FAST_NAMES
#define SLOW_FAST_NAMES \
  { _("Slow"), _("Fast") }
#endif
#ifndef NAVICO_SEA_AUTO_NAMES
#define NAVICO_SEA_AUTO_NAMES \
  { _("Harbour"), _("Offshore") }
#endif
#ifndef NAVICO_HALO_SEA_AUTO_NAMES
#define NAVICO_HALO_SEA_AUTO_NAMES \
  { _("Harbour"), _("Offshore"), _("Weather"), _("Bird") }
#endif
#ifndef DOPPLER_NAMES
#define DOPPLER_NAMES \
  { _("Off"), _("Normal"), _("Approaching") }
#endif

HAVE_CONTROL(CT_ANTENNA_HEIGHT, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 50, CTD_STEP_1, CTD_NUMERIC)
HAVE_CONTROL(CT_BEARING_ALIGNMENT, CTD_AUTO_NO, CTD_DEF_ZERO, -180, +180, CTD_STEP_1, CTD_NUMERIC)
HAVE_CONTROL(CT_GAIN, CTD_AUTO_YES, 50, CTD_MIN_ZERO, CTD_MAX_100, CTD_STEP_1, CTD_PERCENTAGE)
HAVE_CONTROL(CT_INTERFERENCE_REJECTION, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 3, CTD_STEP_1, OFF_LOW_MEDIUM_HIGH_NAMES)
HAVE_CONTROL(CT_LOCAL_INTERFERENCE_REJECTION, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 3, CTD_STEP_1, OFF_LOW_MEDIUM_HIGH_NAMES)
HAVE_CONTROL(CT_RAIN, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, CTD_MAX_100, CTD_STEP_1, CTD_PERCENTAGE)
HAVE_CONTROL(CT_RANGE, CTD_AUTO_YES, 1000, CTD_MIN_ZERO, 0, CTD_STEP_1, CTD_NUMERIC)
if (radarType >= RT_HaloA) {
  HAVE_CONTROL(CT_SCAN_SPEED, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 3, CTD_STEP_1, OFF_LOW_MEDIUM_HIGH_NAMES)
} else {
  HAVE_CONTROL(CT_SCAN_SPEED, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 1, CTD_STEP_1, SLOW_FAST_NAMES)
}
if (radarType >= RT_HaloA) {
  HAVE_CONTROL(CT_SEA, NAVICO_HALO_SEA_AUTO_NAMES, CTD_DEF_ZERO, CTD_MIN_ZERO, CTD_MAX_100, CTD_STEP_1, CTD_PERCENTAGE)
} else {
  HAVE_CONTROL(CT_SEA, NAVICO_SEA_AUTO_NAMES, CTD_DEF_ZERO, CTD_MIN_ZERO, CTD_MAX_100, CTD_STEP_1, CTD_PERCENTAGE)
}
HAVE_CONTROL(CT_SIDE_LOBE_SUPPRESSION, CTD_AUTO_YES, CTD_DEF_ZERO, CTD_MIN_ZERO, CTD_MAX_100, CTD_STEP_1, CTD_PERCENTAGE)
HAVE_CONTROL(CT_TARGET_BOOST, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 2, CTD_STEP_1, OFF_LOW_HIGH_NAMES)
HAVE_CONTROL(CT_TARGET_EXPANSION, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 1, CTD_STEP_1, OFF_ON_NAMES)
if (radarType != RT_BR24) {
  HAVE_CONTROL(CT_NOISE_REJECTION, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 2, CTD_STEP_1, OFF_LOW_HIGH_NAMES)
}
if (radarType != RT_BR24 && radarType != RT_3G) {
  HAVE_CONTROL(CT_TARGET_SEPARATION, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 3, CTD_STEP_1, OFF_LOW_MEDIUM_HIGH_NAMES)
}
if (radarType >= RT_HaloA) {
  HAVE_CONTROL(CT_DOPPLER, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 2, CTD_STEP_1, DOPPLER_NAMES)
}
HAVE_CONTROL(CT_TIMED_IDLE, CTD_AUTO_NO, CTD_DEF_OFF, 1, 99, CTD_STEP_1, CTD_MINUTES)
HAVE_CONTROL(CT_TIMED_RUN, CTD_AUTO_NO, 1, 1, 99, CTD_STEP_1, CTD_MINUTES)
