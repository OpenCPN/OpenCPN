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

#ifndef _KALMAN_H_
#define _KALMAN_H_

#include "Matrix.h"
#include "RadarInfo.h"

PLUGIN_BEGIN_NAMESPACE

#define NOISE \
  (0.015)                                               // Allowed covariance of target speed in lat and lon
                                                        // critical for the performance of target tracking
                                                        // lower value makes target go straight
                                                        // higher values allow target to make curves
#define CONVERT ((((1. / 1852.) / 1852.) / 60.) / 60.)  // converts meters ^ 2 to degrees ^ 2

class Polar {
 public:
  int angle;
  int r;
  wxLongLong time;  // wxGetUTCTimeMillis
};

class LocalPosition {
 public:
  GeoPosition pos;
  double dlat_dt;       // latitude  of speed vector, m/s
  double dlon_dt;       // longitude of speed vector, m/s
  double sd_speed_m_s;  // standard deviation of the speed, m/s
};

static Matrix<double, 4, 2> ZeroMatrix42;
static Matrix<double, 2, 4> ZeroMatrix24;
static Matrix<double, 4> ZeroMatrix4;
static Matrix<double, 2> ZeroMatrix2;

class KalmanFilter {
 public:
  KalmanFilter(size_t spokes);
  ~KalmanFilter();
  void SetMeasurement(Polar* p, LocalPosition* x, Polar* expected, double scale);
  void Predict(LocalPosition* x, double delta_time);  // measured position and expected position
  void ResetFilter();
  void Update_P();

  Matrix<double, 4> A;
  Matrix<double, 4> AT;
  Matrix<double, 4, 2> W;
  Matrix<double, 2, 4> WT;
  Matrix<double, 2, 4> H;
  Matrix<double, 4, 2> HT;
  Matrix<double, 4> P;
  Matrix<double, 2> Q;
  Matrix<double, 2> R;
  Matrix<double, 4, 2> K;
  Matrix<double, 4> I;

 private:
  size_t m_spokes;
};

class GPSKalmanFilter {
 public:
  GPSKalmanFilter();
  ~GPSKalmanFilter();
  void SetMeasurement(ExtendedPosition* gps, ExtendedPosition* updated);
  void Predict(ExtendedPosition* old, ExtendedPosition* updated);
  void Update_P();

  Matrix<double, 4> A;
  Matrix<double, 4> AT;
  Matrix<double, 4, 2> W;
  Matrix<double, 2, 4> WT;
  Matrix<double, 2, 4> H;
  Matrix<double, 4, 2> HT;
  Matrix<double, 4> P;
  Matrix<double, 2> Q;
  Matrix<double, 2> R;
  Matrix<double, 4, 2> K;
  Matrix<double, 4> I;
};

PLUGIN_END_NAMESPACE
#endif
