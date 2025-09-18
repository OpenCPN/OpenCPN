/***************************************************************************
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
 *\file
 *
 * Implement canvas_config.h -- canvas configuration
 */

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include "gl_headers.h"

#include "canvas_config.h"
#include "s52s57.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#ifndef PI
#define PI 3.1415926535897931160E0 /* pi */
#endif

//------------------------------------------------------------------------------
// canvasConfig Implementation
//------------------------------------------------------------------------------

canvasConfig::canvasConfig(int index) {
  configIndex = index;
  canvas = NULL;
  GroupID = 0;
  iLat = 0.;
  iLon = 0.;
  iScale = .0003;  // decent initial value
  iRotation = 0.;
}

canvasConfig::~canvasConfig() {}

void canvasConfig::Reset() {
  bFollow = false;
  bShowTides = false;
  bShowCurrents = false;
  bCourseUp = false;
  bHeadUp = false;
  bLookahead = false;
  bShowAIS = true;
  bAttenAIS = false;
  bQuilt = true;
  nENCDisplayCategory = (int)(enum _DisCat)OTHER;
  bShowENCDataQuality = 0;
  bShowENCBuoyLabels = 0;
  bShowENCLightDescriptions = 1;
}

void canvasConfig::LoadFromLegacyConfig(wxConfigBase *conf) {
  if (!conf) return;

  bFollow = false;
  bShowAIS = true;

  // S52 stuff
  conf->SetPath("/Settings/GlobalState");
  conf->Read("bShowS57Text", &bShowENCText, 1);
  conf->Read("bShowLightDescription", &bShowENCLightDescriptions, 1);
  conf->Read("nDisplayCategory", &nENCDisplayCategory, (enum _DisCat)OTHER);
  conf->Read("bShowSoundg", &bShowENCDepths, 1);
  conf->Read("bShowAtonText", &bShowENCBuoyLabels, 0);
  bShowENCLights = true;
  bShowENCVisibleSectorLights = false;
  bShowENCAnchorInfo = false;
  bShowENCDataQuality = false;

  conf->SetPath("/Settings/AIS");
  conf->Read("bShowScaledTargets", &bAttenAIS, 0);

  conf->SetPath("/Settings");
  conf->Read("ShowTide", &bShowTides, 0);
  conf->Read("ShowCurrent", &bShowCurrents, 0);
  conf->Read("CourseUpMode", &bCourseUp, 0);
  conf->Read("HeadUpMode", &bHeadUp, 0);
  conf->Read("LookAheadMode", &bLookahead, 0);

  conf->Read("ShowGrid", &bShowGrid, 0);
  conf->Read("ShowChartOutlines", &bShowOutlines, 1);
  conf->Read("ShowDepthUnits", &bShowDepthUnits, 1);
  conf->Read("ChartQuilting", &bQuilt, 1);

  conf->Read("ActiveChartGroup", &GroupID, 0);
  conf->Read("InitialdBIndex", &DBindex, -1);

  conf->SetPath("/Settings/GlobalState");
  wxString st;
  double st_view_scale, st_rotation;
  if (conf->Read(wxString("VPScale"), &st)) {
    sscanf(st.mb_str(wxConvUTF8), "%lf", &st_view_scale);
    //    Sanity check the scale
    st_view_scale = fmax(st_view_scale, .001 / 32);
    st_view_scale = fmin(st_view_scale, 4);
    iScale = st_view_scale;
  }

  if (conf->Read(wxString("VPRotation"), &st)) {
    sscanf(st.mb_str(wxConvUTF8), "%lf", &st_rotation);
    //    Sanity check the rotation
    st_rotation = fmin(st_rotation, 360);
    st_rotation = fmax(st_rotation, 0);
    iRotation = st_rotation * PI / 180.;
  }

  wxString sll;
  double lat, lon;
  if (conf->Read("VPLatLon", &sll)) {
    sscanf(sll.mb_str(wxConvUTF8), "%lf,%lf", &lat, &lon);

    //    Sanity check the lat/lon...both have to be reasonable.
    if (fabs(lon) < 360.) {
      while (lon < -180.) lon += 360.;

      while (lon > 180.) lon -= 360.;

      iLon = lon;
    }

    if (fabs(lat) < 90.0) iLat = lat;
  }
}
