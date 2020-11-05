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
 *   Copyright (C) 2013-2016 by Douwe Fokkkema             df@percussion.nl*
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

#ifndef _RADAR_MARPA_H_
#define _RADAR_MARPA_H_

//#include "pi_common.h"

//#include "radar_pi.h"
#include "Kalman.h"
#include "Matrix.h"
#include "RadarInfo.h"

PLUGIN_BEGIN_NAMESPACE

//    Forward definitions
class KalmanFilter;

#define MAX_NUMBER_OF_TARGETS (100)
#define TARGET_SEARCH_RADIUS1 (2)   // radius of target search area for pass 1 (on top of the size of the blob)
#define TARGET_SEARCH_RADIUS2 (15)  // radius of target search area for pass 1
#define SCAN_MARGIN (150)           // number of lines that a next scan of the target may have moved
#define SCAN_MARGIN2 (1000)         // if target is refreshed after this time you will be shure it is the next sweep
#define MAX_CONTOUR_LENGTH (601)    // defines maximal size of target contour in pixels
#define MAX_TARGET_DIAMETER (200)   // target will be set lost if diameter in pixels is larger than this value
#define MAX_LOST_COUNT (3)          // number of sweeps that target can be missed before it is set to lost

#define FOR_DELETION (-2)  // status of a duplicate target used to delete a target
#define LOST (-1)
#define ACQUIRE0 (0)  // 0 under acquisition, first seen, no contour yet
#define ACQUIRE1 (1)  // 1 under acquisition, contour found, first position FOUND
#define ACQUIRE2 (2)  // 2 under acquisition, speed and course taken
#define ACQUIRE3 \
  (3)  // 3 under acquisition, speed and course verified, next time active
       //    >=4  active

#define Q_NUM (4)  // status Q to OCPN at target status
#define T_NUM (6)  // status T to OCPN at target status
#define TARGET_SPEED_DIV_SDEV 2.
#define STATUS_TO_OCPN (5)            // First status to be send to OCPN
#define START_UP_SPEED (0.5)          // maximum allowed speed (m/sec) for new target, real format with .
#define DISTANCE_BETWEEN_TARGETS (4)  // minimum separation between targets

typedef int target_status;
enum OCPN_target_status {
  Q,  // acquiring
  T,  // active
  L   // lost
};

enum TargetProcessStatus { UNKNOWN, NOT_FOUND_IN_PASS1 };
enum PassN { PASS1, PASS2 };

class ArpaTarget {
  friend class RadarArpa;  // Allow RadarArpa access to private members

 public:
  ArpaTarget(radar_pi* pi, RadarInfo* ri);
  ArpaTarget();
  ~ArpaTarget();

  int GetContour(Polar* p);
  void set(radar_pi* pi, RadarInfo* ri);
  bool FindNearestContour(Polar* pol, int dist);
  bool FindContourFromInside(Polar* p);
  bool GetTarget(Polar* pol, int dist);
  void RefreshTarget(int dist);
  void PassARPAtoOCPN(Polar* p, OCPN_target_status s);
  void SetStatusLost();
  void ResetPixels();
  bool Pix(int ang, int rad);
  bool MultiPix(int ang, int rad);

 private:
  RadarInfo* m_ri;
  radar_pi* m_pi;
  KalmanFilter* m_kalman;
  int m_target_id;
  target_status m_status;
  // radar position at time of last target fix, the polars in the contour refer to this origin
  GeoPosition m_radar_pos;
  ExtendedPosition m_position;  // holds actual position of target
  double m_speed_kn;            // Average speed of target. TODO: Merge with m_position.speed?
  wxLongLong m_refresh;         // time of last refresh
  double m_course;
  int m_stationary;  // number of sweeps target was stationary
  int m_lost_count;
  bool m_check_for_duplicate;
  TargetProcessStatus m_pass1_result;
  PassN m_pass_nr;
  Polar m_contour[MAX_CONTOUR_LENGTH + 1];  // contour of target, only valid immediately after finding it
  int m_contour_length;
  Polar m_max_angle, m_min_angle, m_max_r, m_min_r;  // charasterictics of contour
  Polar m_expected;
  bool m_automatic;  // True for ARPA, false for MARPA.

  ExtendedPosition Polar2Pos(Polar pol, ExtendedPosition own_ship);
  Polar Pos2Polar(ExtendedPosition p, ExtendedPosition own_ship);
};

class RadarArpa {
 public:
  RadarArpa(radar_pi* pi, RadarInfo* ri);
  ~RadarArpa();
  void DrawArpaTargetsOverlay(double scale, double arpa_rotate);
  void DrawArpaTargetsPanel(double scale, double arpa_rotate);
  void RefreshArpaTargets();
  int AcquireNewARPATarget(Polar pol, int status);
  void AcquireNewMARPATarget(ExtendedPosition p);
  void DeleteTarget(ExtendedPosition p);
  bool MultiPix(int ang, int rad);
  void DeleteAllTargets();
  void CleanUpLostTargets();
  void RadarLost() {
    DeleteAllTargets();  // Let ARPA targets disappear
  }
  void ClearContours();
  int GetTargetCount() { return m_number_of_targets; }

 private:
  int m_number_of_targets;
  ArpaTarget* m_targets[MAX_NUMBER_OF_TARGETS];

  radar_pi* m_pi;
  RadarInfo* m_ri;

  void AcquireOrDeleteMarpaTarget(ExtendedPosition p, int status);
  void CalculateCentroid(ArpaTarget* t);
  void DrawContour(ArpaTarget* t);
  bool Pix(int ang, int rad);
};

PLUGIN_END_NAMESPACE

#endif
