/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN private types and ENUMs
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 *
 *
 */

#ifndef _OCPNTYPES_H__
#define _OCPNTYPES_H__

#include <wx/colour.h>

//    ChartType constants
typedef enum ChartTypeEnum {
  CHART_TYPE_UNKNOWN = 0,
  CHART_TYPE_DUMMY,
  CHART_TYPE_DONTCARE,
  CHART_TYPE_KAP,
  CHART_TYPE_GEO,
  CHART_TYPE_S57,
  CHART_TYPE_CM93,
  CHART_TYPE_CM93COMP,
  CHART_TYPE_PLUGIN,
  CHART_TYPE_MBTILES
} _ChartTypeEnum;

//    ChartFamily constants
typedef enum ChartFamilyEnum {
  CHART_FAMILY_UNKNOWN = 0,
  CHART_FAMILY_RASTER,
  CHART_FAMILY_VECTOR,
  CHART_FAMILY_DONTCARE
} _ChartFamilyEnum;

//----------------------------------------------------------------------------
// ocpn Toolbar stuff
//----------------------------------------------------------------------------
class ChartBase;
class wxSocketEvent;
class ocpnToolBarSimple;

/**
 * A generic position and navigation data structure.
 *
 * This structure provides position and navigation data that may come from
 * various sources:
 * - GNSS receiver (primary source)
 * - Last known position (when GNSS signal is lost)
 * - User-defined position (when manually moved on map)
 * - Dead reckoning (calculated from last known position and movement)
 */
typedef struct {
  /**
   * Latitude in decimal degrees.
   * May represent last known position rather than current true position if:
   * - GNSS signal is lost
   * - Position has been manually set by user on map
   */
  double kLat;

  /**
   * Longitude in decimal degrees.
   * May represent last known position rather than current true position if:
   * - GNSS signal is lost
   * - Position has been manually set by user on map
   */
  double kLon;

  /** Course over ground in degrees */
  double kCog;

  /**
   * Speed over ground in knots.
   * May be NaN if speed cannot be determined.
   */
  double kSog;

  /**
   * Magnetic variation in degrees.
   * Typically sourced from NMEA RMC message.
   */
  double kVar;

  /**
   * Magnetic heading in degrees.
   * May be NaN if heading sensor data is not available.
   */
  double kHdm;

  /**
   * True heading in degrees.
   * May be NaN if true heading cannot be calculated (requires both magnetic
   * heading and variation).
   */
  double kHdt;

  /**
   * UTC time of fix.
   * - If GNSS available: Time from most recent GNSS message
   * - If GNSS watchdog expired: Current system time
   */
  time_t FixTime;

  /**
   * Number of satellites used in the fix.
   * Will be 0 if:
   * - GNSS watchdog has expired
   * - Position is not from GNSS
   */
  int nSats;
} GenericPosDatEx;

//    A collection of active leg Data structure
typedef struct {
  double Xte;  // Left side of the track -> negative XTE
  double Btw;
  double Dtw;
  wxString wp_name;  // Name of destination waypoint for active leg;
  bool arrival;
} ActiveLegDat;

#endif  //  _OCPNTYPES_H__
