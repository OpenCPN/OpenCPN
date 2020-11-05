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
// A list of all possible controls.
//
// This is included multiple times for various purposes
//

// Software
CONTROL_TYPE(CT_ANTENNA_FORWARD, "Antenna forward of GPS")
CONTROL_TYPE(CT_ANTENNA_STARBOARD, "Antenna starboard of GPS")
CONTROL_TYPE(CT_MAIN_BANG_SIZE, "Main bang size")
CONTROL_TYPE(CT_ORIENTATION, "Orientation")
CONTROL_TYPE(CT_CENTER_VIEW, "Look Around")
CONTROL_TYPE(CT_OVERLAY_CANVAS, "Overlay Canvas n")
CONTROL_TYPE(CT_TARGET_ON_PPI, "AIS/ARPA on PPI")
CONTROL_TYPE(CT_REFRESHRATE, "Refresh rate")
CONTROL_TYPE(CT_TARGET_TRAILS, "Target trails")
CONTROL_TYPE(CT_TIMED_IDLE, "Timed idle")
CONTROL_TYPE(CT_TIMED_RUN, "Timed run")
CONTROL_TYPE(CT_TRAILS_MOTION, "Target trails motion")

// Hardware
CONTROL_TYPE(CT_ANTENNA_HEIGHT, "Antenna height")
CONTROL_TYPE(CT_BEARING_ALIGNMENT, "Bearing alignment")
CONTROL_TYPE(CT_GAIN, "Gain")
CONTROL_TYPE(CT_INTERFERENCE_REJECTION, "Interference rejection")
CONTROL_TYPE(CT_LOCAL_INTERFERENCE_REJECTION, "Local interference rejection")
CONTROL_TYPE(CT_NOISE_REJECTION, "Noise rejection")
CONTROL_TYPE(CT_NO_TRANSMIT_END, "No Transmit end")
CONTROL_TYPE(CT_NO_TRANSMIT_START, "No Transmit start")
CONTROL_TYPE(CT_RAIN, "Rain clutter")
CONTROL_TYPE(CT_RANGE, "Range")
CONTROL_TYPE(CT_SCAN_SPEED, "Scan speed")
CONTROL_TYPE(CT_SEA, "Sea clutter")
CONTROL_TYPE(CT_FTC, "FTC")
CONTROL_TYPE(CT_SIDE_LOBE_SUPPRESSION, "Side lobe suppression")
CONTROL_TYPE(CT_TARGET_BOOST, "Target boost")
CONTROL_TYPE(CT_TARGET_EXPANSION, "Target expansion")
CONTROL_TYPE(CT_TARGET_SEPARATION, "Target separation")
CONTROL_TYPE(CT_TRANSPARENCY, "Transparency")
CONTROL_TYPE(CT_DOPPLER, "Doppler")
