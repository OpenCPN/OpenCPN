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
 *
 */

#ifndef __LODREDUCE_H__
#define __LODREDUCE_H__

#include <vector>

void DouglasPeucker(double *PointList, int fp, int lp,
                               double epsilon, std::vector<int> *keep);
void DouglasPeuckerF(float *PointList, int fp, int lp,
                                double epsilon, std::vector<int> *keep);
void DouglasPeuckerM(double *PointList, int fp, int lp,
                                double epsilon, std::vector<int> *keep);
void DouglasPeuckerFI(float *PointList, int fp, int lp,
                                 double epsilon, std::vector<bool> &keep);
void DouglasPeuckerDI(double *PointList, int fp, int lp,
                                 double epsilon, std::vector<bool> &keep);


#endif    // guard
