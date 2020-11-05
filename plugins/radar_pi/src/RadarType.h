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

#if !defined(DEFINE_RADAR)
#ifndef _RADARTYPE_H_
#define _RADARTYPE_H_

#include "RadarInfo.h"
#include "pi_common.h"

#include "garminhd/GarminHDControl.h"
#include "garminhd/GarminHDControlsDialog.h"
#include "garminhd/GarminHDReceive.h"

#include "garminxhd/GarminxHDControl.h"
#include "garminxhd/GarminxHDControlsDialog.h"
#include "garminxhd/GarminxHDReceive.h"

#include "navico/NavicoControl.h"
#include "navico/NavicoControlsDialog.h"
#include "navico/NavicoReceive.h"

#include "emulator/EmulatorControl.h"
#include "emulator/EmulatorControlsDialog.h"
#include "emulator/EmulatorReceive.h"

#endif /* _RADARTYPE_H_ */

#define DEFINE_RADAR(t, x, s, l, a, b, c, d)
#define INITIALIZE_RADAR
#endif

#if !defined(DEFINE_RANGE_METRIC)
#define DEFINE_RANGE_METRIC(t, x)
#endif

#if !defined(DEFINE_RANGE_MIXED)
#define DEFINE_RANGE_MIXED(t, x)
#endif

#if !defined(DEFINE_RANGE_NAUTIC)
#define DEFINE_RANGE_NAUTIC(t, x)
#endif

#ifndef SPOKES_MAX
#define SPOKES_MAX 0
#endif

#ifndef SPOKE_LEN_MAX
#define SPOKE_LEN_MAX 0
#endif

#ifndef RO_SINGLE
#define RO_SINGLE (0)
#define RO_PRIMARY (1)
#define RO_SECONDARY (2)
#endif

#include "garminhd/garminhdtype.h"
#include "garminxhd/garminxhdtype.h"

#include "navico/br24type.h"
#include "navico/br3gtype.h"
#include "navico/br4gatype.h"
#include "navico/br4gbtype.h"

#include "navico/haloatype.h"
#include "navico/halobtype.h"

// TODO: Add Garmin etc.

#include "emulator/emulatortype.h"

#undef DEFINE_RADAR  // Prepare for next inclusion
#undef INITIALIZE_RADAR
#undef DEFINE_RANGE_METRIC
#undef DEFINE_RANGE_MIXED
#undef DEFINE_RANGE_NAUTIC
