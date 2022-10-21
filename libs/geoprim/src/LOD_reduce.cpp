/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Extern C Linked Utilities
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
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <vector>

#include "vector2D.h"

void DouglasPeucker(double *PointList, int fp, int lp, double epsilon,
                    std::vector<int> *keep) {
  // Find the point with the maximum distance
  double dmax = 0;
  int index = 0;

  vector2D va(PointList[2 * fp] - PointList[2 * lp],
              PointList[2 * fp + 1] - PointList[2 * lp + 1]);

  double da = va.x * va.x + va.y * va.y;
  for (int i = fp + 1; i < lp; ++i) {
    vector2D vb(PointList[2 * i] - PointList[2 * fp],
                PointList[2 * i + 1] - PointList[2 * fp + 1]);

    double dab = va.x * vb.x + va.y * vb.y;
    double db = vb.x * vb.x + vb.y * vb.y;
    double d = da - dab * dab / db;
    if (d > dmax) {
      index = i;
      dmax = d;
    }
  }
  // If max distance is greater than epsilon, recursively simplify
  if (dmax > epsilon * epsilon) {
    keep->push_back(index);

    // Recursive call
    DouglasPeucker(PointList, fp, index, epsilon, keep);
    DouglasPeucker(PointList, index, lp, epsilon, keep);
  }
}

void DouglasPeuckerF(float *PointList, int fp, int lp, double epsilon,
                     std::vector<int> *keep) {
  // Find the point with the maximum distance
  double dmax = 0;
  int index = 0;

  vector2D va(PointList[2 * fp] - PointList[2 * lp],
              PointList[2 * fp + 1] - PointList[2 * lp + 1]);

  double da = va.x * va.x + va.y * va.y;
  for (int i = fp + 1; i < lp; ++i) {
    vector2D vb(PointList[2 * i] - PointList[2 * fp],
                PointList[2 * i + 1] - PointList[2 * fp + 1]);

    double dab = va.x * vb.x + va.y * vb.y;
    double db = vb.x * vb.x + vb.y * vb.y;
    double d = da - dab * dab / db;
    if (d > dmax) {
      index = i;
      dmax = d;
    }
  }
  // If max distance is greater than epsilon, recursively simplify
  if (dmax > epsilon * epsilon) {
    keep->push_back(index);

    // Recursive call
    DouglasPeuckerF(PointList, fp, index, epsilon, keep);
    DouglasPeuckerF(PointList, index, lp, epsilon, keep);
  }
}

void DouglasPeuckerFI(float *PointList, int fp, int lp, double epsilon,
                      std::vector<bool> &keep) {
  keep[fp] = true;
  keep[lp] = true;

  // Find the point with the maximum distance
  double dmax = 0;
  int maxdistIndex = -1;

  vector2D va(PointList[2 * fp] - PointList[2 * lp],
              PointList[2 * fp + 1] - PointList[2 * lp + 1]);

  double da = va.x * va.x + va.y * va.y;
  for (int i = fp + 1; i < lp; ++i) {
    vector2D vb(PointList[2 * i] - PointList[2 * fp],
                PointList[2 * i + 1] - PointList[2 * fp + 1]);

    double dab = va.x * vb.x + va.y * vb.y;
    double db = vb.x * vb.x + vb.y * vb.y;
    double d = da - dab * dab / db;
    if (d > dmax) {
      maxdistIndex = i;
      dmax = d;
    }
  }
  // If max distance is greater than epsilon, recursively simplify
  if (dmax > epsilon * epsilon) {
    // Recursive call
    DouglasPeuckerFI(PointList, fp, maxdistIndex, epsilon, keep);
    DouglasPeuckerFI(PointList, maxdistIndex, lp, epsilon, keep);
  }
}

void DouglasPeuckerDI(double *PointList, int fp, int lp, double epsilon,
                      std::vector<bool> &keep) {
  keep[fp] = true;
  keep[lp] = true;

  // Find the point with the maximum distance
  double dmax = 0;
  int maxdistIndex = -1;

  vector2D va(PointList[3 * fp] - PointList[3 * lp],
              PointList[3 * fp + 1] - PointList[3 * lp + 1]);
  double da = sqrt(va.x * va.x + va.y * va.y);

  double y2 = PointList[3 * lp + 1];
  double y1 = PointList[3 * fp + 1];
  double x2 = PointList[3 * lp];
  double x1 = PointList[3 * fp];

  for (int i = fp + 1; i < lp; ++i) {
    // Distance from point to line.
    // ref.  https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
    double d = (y2 - y1) * PointList[3 * i] - (x2 - x1) * PointList[3 * i + 1] +
               (x2 * y1) - (y2 * x1);

    d = fabs(d / da);

    if (d > dmax) {
      maxdistIndex = i;
      dmax = d;
    }
  }
  // If max distance is greater than epsilon, recursively simplify
  if (dmax > epsilon) {
    // Recursive call
    DouglasPeuckerDI(PointList, fp, maxdistIndex, epsilon, keep);
    DouglasPeuckerDI(PointList, maxdistIndex, lp, epsilon, keep);
  }
}

void DouglasPeuckerM(double *PointList, int fp, int lp, double epsilon,
                     std::vector<int> *keep) {
  // Find the point with the maximum distance
  int index = 0;
  double lmax = 0;

  vector2D va(PointList[2 * fp] - PointList[2 * lp],
              PointList[2 * fp + 1] - PointList[2 * lp + 1]);

  for (int i = fp + 1; i < lp; ++i) {
    vector2D vb(PointList[2 * i] - PointList[2 * fp],
                PointList[2 * i + 1] - PointList[2 * fp + 1]);

    vector2D vn;
    double l = vGetLengthOfNormal(&vb, &va, &vn);
    if (l > lmax) {
      index = i;
      lmax = l;
    }
  }
  // If max distance is greater than epsilon, recursively simplify
  if (lmax > epsilon) {
    keep->push_back(index);

    // Recursive call
    DouglasPeuckerM(PointList, fp, index, epsilon, keep);
    DouglasPeuckerM(PointList, index, lp, epsilon, keep);
  }
}

