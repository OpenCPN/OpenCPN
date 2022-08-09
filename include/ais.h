/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  AIS Decoder Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#ifndef __AIS_H__
#define __AIS_H__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include <wx/datetime.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/wxhtml.h>

#include <vector>

#include "wx/sound.h"

#include "dychart.h"
#include "gui_lib.h"
#include "navutil.h"
#include "OCPN_Sound.h"
#include "AIS_Bitstring.h"
#include "AISTargetListDialog.h"

//  FWD definitions
class ChartCanvas;

//    Constants
#ifndef PI
#define PI 3.1415926535897931160E0 /* pi */
#endif

#define TIMER_AIS_MSEC 998
#define TIMER_AIS_AUDIO_MSEC 2000

#define AIS_TARGETDATA_MAX_CANVAS 6

enum {
  tlTRK = 0,
  tlNAME,
  tlCALL,
  tlMMSI,
  tlCLASS,
  tlTYPE,
  tlNAVSTATUS,
  tlBRG,
  tlRNG,
  tlCOG,
  tlSOG,
  tlCPA,
  tlTCPA
};  // AISTargetListCtrl Columns;

typedef enum AIS_Error {
  AIS_NoError = 0,
  AIS_Partial,
  AIS_NMEAVDX_TOO_LONG,
  AIS_NMEAVDX_CHECKSUM_BAD,
  AIS_NMEAVDX_BAD,
  AIS_NO_SERIAL,
  AIS_NO_TCP,
  AIS_GENERIC_ERROR,
  AIS_INCOMPLETE_MULTIPART
} _AIS_Error;

//      Describe NavStatus variable
typedef enum ais_nav_status {
  UNDERWAY_USING_ENGINE = 0,
  AT_ANCHOR,
  NOT_UNDER_COMMAND,
  RESTRICTED_MANOEUVRABILITY,
  CONSTRAINED_BY_DRAFT,
  MOORED,
  AGROUND,
  FISHING,
  UNDERWAY_SAILING,
  HSC,
  WIG,
  RESERVED_11,
  RESERVED_12,
  RESERVED_13,
  RESERVED_14,
  UNDEFINED,
  ATON_VIRTUAL,
  ATON_VIRTUAL_ONPOSITION,
  ATON_VIRTUAL_OFFPOSITION,
  ATON_REAL,
  ATON_REAL_ONPOSITION,
  ATON_REAL_OFFPOSITION

} _ais_nav_status;

//      Describe Transponder Class
typedef enum ais_transponder_class {
  AIS_CLASS_A = 0,
  AIS_CLASS_B,
  AIS_ATON,        // Aid to Navigation   pjotrc 2010/02/01
  AIS_BASE,        // Base station
  AIS_GPSG_BUDDY,  // GpsGate Buddy object
  AIS_DSC,         // DSC target
  AIS_SART,        // SART
  AIS_ARPA,        // ARPA radar target
  AIS_APRS         // APRS position report
} _ais_transponder_class;

//    Describe AIS Alert state
typedef enum ais_alert_type {
  AIS_NO_ALERT = 0,
  AIS_ALERT_SET,
  AIS_ALERT_NO_DIALOG_SET

} _ais_alarm_type;

class AISTargetTrackPoint {
public:
  double m_lat;
  double m_lon;
  time_t m_time;
};

// IMO Circ. 289 Area Notices, based on libais
const size_t AIS8_001_22_NUM_NAMES = 128;
const size_t AIS8_001_22_SUBAREA_SIZE = 87;

extern wxString ais8_001_22_notice_names[];

enum Ais8_001_22_AreaShapeEnum {
  AIS8_001_22_SHAPE_ERROR = -1,
  AIS8_001_22_SHAPE_CIRCLE = 0,  // OR Point
  AIS8_001_22_SHAPE_RECT = 1,
  AIS8_001_22_SHAPE_SECTOR = 2,
  AIS8_001_22_SHAPE_POLYLINE = 3,
  AIS8_001_22_SHAPE_POLYGON = 4,
  AIS8_001_22_SHAPE_TEXT = 5,
  AIS8_001_22_SHAPE_RESERVED_6 = 6,
  AIS8_001_22_SHAPE_RESERVED_7 = 7
};

struct Ais8_001_22_SubArea {
  int shape;
  float longitude, latitude;
  int radius_m;
  int e_dim_m;  // East dimension in meters
  int n_dim_m;
  int orient_deg;  // Orientation in degrees from true north
  int left_bound_deg;
  int right_bound_deg;
  float angles[4];
  float dists_m[4];
  wxString text;
};

// WX_DECLARE_LIST(Ais8_001_22_SubArea, Ais8_001_22_SubAreaList);
typedef std::vector<Ais8_001_22_SubArea> Ais8_001_22_SubAreaList;

struct Ais8_001_22 {
  int link_id;      // 10 bit id to match up text blocks
  int notice_type;  // area_type / Notice Description
  int month;        // These are in UTC
  int day;          // UTC!
  int hour;         // UTC!
  int minute;
  int duration_minutes;  // Time from the start until the notice expires
  wxDateTime start_time;
  wxDateTime expiry_time;
  Ais8_001_22_SubAreaList sub_areas;
};

//---------------------------------------------------------------------------------
//
//  AIS_Decoder Helpers
//
//---------------------------------------------------------------------------------
WX_DEFINE_SORTED_ARRAY(AIS_Target_Data *, ArrayOfAISTarget);

//      Implement the AISTargetList as a wxHashMap

wxString trimAISField(char *data);
wxString ais_get_status(int index);
wxString ais_get_type(int index);
wxString ais_get_short_type(int index);

void AISDrawAreaNotices(ocpnDC &dc, ViewPort &vp, ChartCanvas *cp);
void AISDraw(ocpnDC &dc, ViewPort &vp, ChartCanvas *cp);
bool AnyAISTargetsOnscreen(ChartCanvas *cc, ViewPort &vp);

WX_DECLARE_HASH_MAP(int, wxString, wxIntegerHash, wxIntegerEqual,
                    AIS_Target_Name_Hash);

#endif
