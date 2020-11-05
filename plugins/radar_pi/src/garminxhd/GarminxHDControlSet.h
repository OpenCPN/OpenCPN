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

#include "SoftwareControlSet.h"

#ifndef AUTO_LOW_HIGH_NAMES
#define AUTO_LOW_HIGH_NAMES \
  { _("Low"), _("High") }
#endif

#ifndef AUTO_CALM_MEDIUM_ROUGH_NAMES
#define AUTO_CALM_MEDIUM_ROUGH_NAMES \
  { _("Calm"), _("Medium"), _("Rough") }
#endif

#ifndef SLOW_FAST_NAMES
#define SLOW_FAST_NAMES \
  { _("Slow"), _("Fast") }
#endif

HAVE_CONTROL(CT_BEARING_ALIGNMENT, CTD_AUTO_NO, CTD_DEF_ZERO, -180, +180, CTD_STEP_1, CTD_NUMERIC)
HAVE_CONTROL(CT_NO_TRANSMIT_START, CTD_AUTO_NO, CTD_DEF_OFF, -180, +180, CTD_STEP_1, CTD_NUMERIC)
HAVE_CONTROL(CT_NO_TRANSMIT_END, CTD_AUTO_NO, CTD_DEF_OFF, -180, +180, CTD_STEP_1, CTD_NUMERIC)
HAVE_CONTROL(CT_GAIN, AUTO_LOW_HIGH_NAMES, 50, CTD_MIN_ZERO, CTD_MAX_100, CTD_STEP_1, CTD_PERCENTAGE)
HAVE_CONTROL(CT_INTERFERENCE_REJECTION, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 1, CTD_STEP_1, OFF_ON_NAMES)
HAVE_CONTROL(CT_RAIN, CTD_AUTO_NO, CTD_DEF_OFF, CTD_MIN_ZERO, CTD_MAX_100, CTD_STEP_1, CTD_PERCENTAGE)
HAVE_CONTROL(CT_RANGE, CTD_AUTO_YES, 1000, CTD_MIN_ZERO, 0, CTD_STEP_1, CTD_NUMERIC)
HAVE_CONTROL(CT_SCAN_SPEED, CTD_AUTO_NO, CTD_DEF_ZERO, CTD_MIN_ZERO, 1, CTD_STEP_1, SLOW_FAST_NAMES)
HAVE_CONTROL(CT_SEA, AUTO_CALM_MEDIUM_ROUGH_NAMES, CTD_DEF_OFF, CTD_MIN_ZERO, CTD_MAX_100, CTD_STEP_1, CTD_PERCENTAGE)
HAVE_CONTROL(CT_TIMED_IDLE, CTD_AUTO_NO, CTD_DEF_OFF, 1, 99, CTD_STEP_1, CTD_MINUTES)
HAVE_CONTROL(CT_TIMED_RUN, CTD_AUTO_NO, 1, 1, 99, CTD_STEP_1, CTD_MINUTES)
