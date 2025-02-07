/******************************************************************************
 *
 * Project: OpenCPN
 * Purpose: Position, course, speed, etc. obtained from instruments.
 *
 ***************************************************************************
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
 ***************************************************************************
 */

#ifndef OWN_SHIP_H__
#define OWN_SHIP_H__

extern double gLat;
extern double gLon;
extern double gCog;
extern double gSog;
extern double gHdt;
extern double gHdm;
extern double gVar;

// Indicate whether the Global Navigation Satellite System (GNSS) has a valid
// position. true  = OpenCPN has obtained a current position from a GNSS
// receiver. false = OpenCPN unable to determine position, or no GNSS receiver
// is connected. Note: the variable name uses the term "GPS" for historical
// reasons; modern GNSS receivers can acquire data from GPS and other satellite
// systems.
extern bool bGPSValid;

#endif  // OWN_SHIP_H__
