/**************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * %Chart canvas configuration state
 */

#ifndef __CANVCONFIG_H__
#define __CANVCONFIG_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/config.h"
#include "wx/wxprec.h"

#include "chcanv.h"

class ChartCanvas;  // Circular

/**
 * Encapsulates persistent canvas configuration. Stores various settings for a
 * chart canvas, including position, scale, rotation, and display options for
 * different chart elements.
 */
class canvasConfig {
public:
  canvasConfig();
  canvasConfig(int index);
  ~canvasConfig();

  /**
   * Resets all configuration options to default values.
   *
   * Initializes follow mode, tides, currents, orientation modes, and AIS
   * settings.
   */
  void Reset();
  void LoadFromLegacyConfig(wxConfigBase *conf);

  int configIndex;
  ChartCanvas *canvas;  //!< Pointer to associated chart canvas.
  double iLat;          //!< Latitude of the center of the chart, in degrees.
  double iLon;          //!< Longitude of the center of the chart, in degrees.
  double iScale;        //!< Initial chart scale factor.
  double iRotation;     //!< Initial rotation angle in radians.
  int DBindex;
  int GroupID;
  bool bFollow;          //!< Enable vessel following mode.
  bool bQuilt;           //!< Enable chart quilting.
  bool bShowTides;       //!< Display tide information.
  bool bShowCurrents;    //!< Display current information.
  wxSize canvasSize;     //!< Canvas dimensions.
  bool bShowGrid;        //!< Display coordinate grid.
  bool bShowOutlines;    //!< Display chart outlines.
  bool bShowDepthUnits;  //!< Display depth unit indicators.
  bool bCourseUp;        //!< Orient display to course up.
  bool bHeadUp;          //!< Orient display to heading up.
  bool bLookahead;       //!< Enable lookahead mode.
  bool bShowAIS;         //!< Display AIS targets.
  bool bAttenAIS;        //!< Enable AIS target attenuation.
  // ENC options
  bool bShowENCText;  //!< Display ENC text elements.
  int nENCDisplayCategory;
  bool bShowENCDepths;
  bool bShowENCBuoyLabels;
  bool bShowENCLightDescriptions;
  bool bShowENCLights;
  bool bShowENCVisibleSectorLights;
  bool bShowENCAnchorInfo;
  bool bShowENCDataQuality;
};

WX_DEFINE_ARRAY_PTR(canvasConfig *, arrayofCanvasConfigPtr);

#endif
