/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  XyGrib model definitions
 * Author:   Ronan Demoment
 *
 ***************************************************************************
 *   Copyright (C) 2024 by R. Demoment                                     *
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
 *
 */

#ifndef __XYGRIBMODELDEF_H__
#define __XYGRIBMODELDEF_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

typedef struct {
  wxString name;
  int nbRes;
  wxString *resolution;
  wxString *reqName;
  int nbInter;
  int *interval;
  int maxDays;
  bool wind;
  bool windGust;
  bool pressure;
  bool altPressure;
  bool airTemp;
  bool CAPE;
  bool reflectivity;
  bool cloudCover;
  bool rainfall;
} AtmModelDef_t;

typedef struct {
  wxString name;
  wxString reqName;
  bool significantHeight;
  bool windWaves;
} WaveModelDef_t;

extern AtmModelDef_t *xygribAtmModelList[];
extern WaveModelDef_t *xygribWaveModelList[];

#endif
