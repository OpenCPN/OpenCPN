/***************************************************************************
 *
 * Project:  OpenCPN
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
 */

#ifndef _AIS_TARGET_DATA_H__
#define _AIS_TARGET_DATA_H__

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>

#include <wx/datetime.h>
#include <wx/intl.h>
#include <wx/string.h>

#include "model/meteo_points.h"
#include "model/navutil_base.h"

#define SHIP_NAME_LEN 35
#define DESTINATION_LEN 21
#define CALL_SIGN_LEN 8
#define EURO_VIN_LEN 9

#define AIS_TARGETDATA_MAX_CANVAS 6

void make_hash_ERI(int key, const wxString& description);
void clear_hash_ERI(void);

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
  AIS_APRS,        // APRS position report
  AIS_METEO        // Meteorological and Hydrographic data
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

struct AisTargetCallbacks {
  std::function<double(double)> get_mag;
  AisTargetCallbacks(): get_mag([](double a) { return toMagnetic(a); }) {}
};


class AisTargetData {
friend class AisTargetDataMaker;

public:
  AisTargetData(AisTargetCallbacks callbacks);
  ~AisTargetData();

  wxString BuildQueryResult(void);
  wxString GetRolloverString(void);
  wxString Get_vessel_type_string(bool b_short = false);
  wxString Get_class_string(bool b_short = false);
  wxString GetFullName(void);
  wxString GetCountryCode(bool b_CntryLongStr);
  wxString GetNatureofDistress(int dscnature);
  void Toggle_AIS_CPA(void);
  void ToggleShowTrack(void);
  void CloneFrom(AisTargetData* q);
  bool IsValidMID(int);

  int MID;
  int MMSI;
  ais_transponder_class Class;
  int NavStatus;
  int SyncState;
  int SlotTO;
  double SOG;
  double COG;
  double HDG;
  double Lon;
  double Lat;
  int ROTAIS;
  int ROTIND;
  char CallSign[CALL_SIGN_LEN];  // includes terminator
  char ShipName[SHIP_NAME_LEN];
  char ShipNameExtension[15];
  unsigned char ShipType;
  int IMO;

  int DimA;
  int DimB;
  int DimC;
  int DimD;

  double Euro_Length;  // Extensions for European Inland AIS
  double Euro_Beam;
  double Euro_Draft;
  char Euro_VIN[EURO_VIN_LEN];  // includes terminator
  int UN_shiptype;
  bool b_isEuroInland;
  bool b_hasInlandDac;  // intermediate storage for EU Inland. SignalK
  bool b_blue_paddle;
  int blue_paddle;

  int ETA_Mo;
  int ETA_Day;
  int ETA_Hr;
  int ETA_Min;

  double Draft;

  char Destination[DESTINATION_LEN];

  time_t PositionReportTicks;
  time_t LastPositionReportTicks;
  time_t StaticReportTicks;

  int RecentPeriod;
  bool b_active;
  bool b_lost;
  bool b_removed;
  ais_alert_type n_alert_state;
  bool b_suppress_audio;
  bool b_positionDoubtful;
  bool b_positionOnceValid;
  bool b_nameValid;
  bool b_isFollower;
  bool b_isDSCtarget; // DSC flag to a possible simultaneous AIS target
  int  m_dscNature;
  int  m_dscTXmmsi;   // MMSI for the DSC relay issuer
  long dsc_NatureOfDistress;

    // MMSI Properties
  bool b_NoTrack;
  bool b_OwnShip;
  bool b_PersistTrack; // For AIS target query
  bool b_mPropPersistTrack; // For mmsi_prop

  int m_utc_hour;
  int m_utc_min;
  int m_utc_sec;
  wxString m_date_string;

  wxDateTime m_ack_time;
  bool b_in_ack_timeout;

  double Range_NM;
  double Brg;

  wxString MSG_14_text;

   // Per target collision parameters
  bool bCPA_Valid;
  double TCPA;  // Minutes
  double CPA;   // Nautical Miles
  bool b_show_AIS_CPA;  // TR 2012.06.28: Show AIS-CPA
  bool b_show_track;

  AisMeteoData met_data;
  std::vector<AISTargetTrackPoint> m_ptrack;

  std::unordered_map<int, Ais8_001_22> area_notices;
  bool b_SarAircraftPosnReport;
  int altitude;  // Metres, from special position report(9)
  bool b_nameFromCache;
  float importance;
  short last_scale[AIS_TARGETDATA_MAX_CANVAS];  // where
                                                // AIS_TARGETDATA_MAX_CANVAS is
                                                // the max number of chartcanvas

private:
  AisTargetCallbacks m_callbacks;
};

/**
 * Singleton factory. Unless SetCallbacks() is invoked GetTargetData()
 * returns an object with default, dummy callbacks.
 */

class AisTargetDataMaker {
public:
  static AisTargetDataMaker& GetInstance() {
    static AisTargetDataMaker instance;
    return instance;
  }

  AisTargetDataMaker(const AisTargetDataMaker&) = delete;
  AisTargetDataMaker& operator=(const AisTargetDataMaker&) = delete;


  std::shared_ptr<AisTargetData> GetTargetData() { return std::make_shared<AisTargetData>(m_callbacks); }
  void SetCallbacks(AisTargetCallbacks callbacks) { m_callbacks = callbacks; }

private:
  AisTargetDataMaker() : m_callbacks(AisTargetCallbacks()) {}
  AisTargetCallbacks m_callbacks;
};


wxString trimAISField(char *data);
wxString ais_get_status(int index);

wxString ais_get_type(int index);
wxString ais_get_short_type(int index);

#endif
