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

#include "Kalman.h"

PLUGIN_BEGIN_NAMESPACE

int main() {
  int ret = 0;
  KalmanFilter *filter = new KalmanFilter(2048);
  Polar pol, expected;
  LocalPosition x_local;

  Matrix<double, 2, 2> m = {0.f, 0.f, 0.f, 0.f};

  if (m(0, 0) != 0.f || m(0, 1) != 0.f || m(1, 0) != 0.f || m(1, 1) != 0.f) {
    cout << "INFO: M(0,0)=" << m(0, 0) << " M(0,1)=" << m(0, 1) << " M(1,0)=" << m(1, 0) << " M(1,1)=" << m(1, 1) << "\n";
    cout << "ERROR: Matrix is not initialized to zero\n";
    ret = 1;
  }

  m = ZeroMatrix2;
  if (m(0, 0) != 0.f || m(0, 1) != 0.f || m(1, 0) != 0.f || m(1, 1) != 0.f) {
    cout << "INFO: M(0,0)=" << m(0, 0) << " M(0,1)=" << m(0, 1) << " M(1,0)=" << m(1, 0) << " M(1,1)=" << m(1, 1) << "\n";
    cout << "ERROR: Matrix is not initialized to zero\n";
    ret = 1;
  }

  m = m.Init(1.f);

  if (m(0, 0) != 1.f || m(0, 1) != 1.f || m(1, 0) != 1.f || m(1, 1) != 1.f) {
    cout << "INFO: M(0,0)=" << m(0, 0) << " M(0,1)=" << m(0, 1) << " M(1,0)=" << m(1, 0) << " M(1,1)=" << m(1, 1) << "\n";
    cout << "ERROR: Matrix is not initialized to ones\n";
    ret = 1;
  }

  m = m.Identity();

  if (m(0, 0) != 1.f || m(0, 1) != 0.f || m(1, 0) != 0.f || m(1, 1) != 1.f) {
    cout << "INFO: M(0,0)=" << m(0, 0) << " M(0,1)=" << m(0, 1) << " M(1,0)=" << m(1, 0) << " M(1,1)=" << m(1, 1) << "\n";
    cout << "ERROR: Matrix is not initialized to identity\n";
    ret = 1;
  }

  pol.angle = 0;
  pol.r = 1000;
  pol.time = 1000;

  x_local.pos.lat = 50;
  x_local.pos.lon = -5;
  x_local.dlat_dt = 5;
  x_local.dlon_dt = 2;
  x_local.sd_speed_m_s = 0.2;

  expected.angle = 10;
  expected.r = 1050;
  expected.time = 6000;

  filter->SetMeasurement(&pol, &x_local, &expected, 512. / 4000.);        // pol is measured position in polar coordinates
  filter->Predict(&x_local, (expected.time - pol.time).GetLo() / 1000.);  // x_local is new estimated local position of the target

  cout << "INFO: The predicted location is: lat=" << x_local.pos.lat << " lon=" << x_local.pos.lon << "\n";
  cout << "INFO: Delta lat=" << x_local.dlat_dt << " Delta lon=" << x_local.dlon_dt << "\n";
  cout << "INFO: StdDev speed=" << x_local.sd_speed_m_s << "\n";

#define ASSERT_VALUE(name, actual, predicted)                                                             \
  if (fabs(actual - predicted) > 0.001) {                                                                 \
    cout << "ERROR: Predicted " name " is not expected value " << predicted << " but " << actual << "\n"; \
    ret = 1;                                                                                              \
  }

  ASSERT_VALUE("lat", x_local.pos.lat, 69.1754);
  ASSERT_VALUE("lon", x_local.pos.lon, 5);
  ASSERT_VALUE("stddev", x_local.sd_speed_m_s, 2.03224);

  if (ret == 0) {
    cout << "INFO: TEST PASSED\n";
  } else {
    cout << "ERROR: TEST FAILED\n";
  }
  exit(ret);
}

PLUGIN_END_NAMESPACE

int main() { RadarPlugin::main(); }
