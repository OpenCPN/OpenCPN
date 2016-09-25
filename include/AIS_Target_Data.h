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

#ifndef __AIS_TARGET_DATA_H__
#define __AIS_TARGET_DATA_H__

#include <wx/string.h>
#include <wx/datetime.h>

#include "ais.h"

void make_hash_ERI(int key, const wxString & description);
void clear_hash_ERI( void );

class AIS_Target_Data
{
public:

    AIS_Target_Data();
    ~AIS_Target_Data();

    wxString BuildQueryResult(void);
    wxString GetRolloverString(void);
    wxString Get_vessel_type_string(bool b_short = false);
    wxString Get_class_string(bool b_short = false);
    wxString GetFullName( void );
    wxString GetCountryCode(bool b_CntryLongStr);
    void Toggle_AIS_CPA(void);
    void ToggleShowTrack(void);
    void CloneFrom( AIS_Target_Data* q );
    

    int                       MID;
    int                       MMSI;
    ais_transponder_class     Class;
    int                       NavStatus;
    int                       SyncState;
    int                       SlotTO;
    double                    SOG;
    double                    COG;
    double                    HDG;
    double                    Lon;
    double                    Lat;
    int                       ROTAIS;
    int                       ROTIND;
    char                      CallSign[8];                // includes terminator
    char                      ShipName[21];
    char                      ShipNameExtension[15];
    unsigned char             ShipType;
    int                       IMO;

    int                       DimA;
    int                       DimB;
    int                       DimC;
    int                       DimD;

    double                    Euro_Length;            // Extensions for European Inland AIS
    double                    Euro_Beam;
    double                    Euro_Draft;
    char                      Euro_VIN[8];
    int                       UN_shiptype;
    bool                      b_isEuroInland;
    bool                      b_blue_paddle;
    int                       blue_paddle;

    int                       ETA_Mo;
    int                       ETA_Day;
    int                       ETA_Hr;
    int                       ETA_Min;

    double                    Draft;

    char                      Destination[21];

    time_t                    PositionReportTicks;
    time_t                    StaticReportTicks;

    int                       RecentPeriod;
    bool                      b_active;
    bool                      b_lost;
    ais_alert_type            n_alert_state;
    bool                      b_suppress_audio;
    bool                      b_positionDoubtful;
    bool                      b_positionOnceValid;
    bool                      b_nameValid;

    //                     MMSI Properties
    bool                      b_NoTrack;
    bool                      b_OwnShip;
    bool                      b_PersistTrack;

    int                       m_utc_hour;
    int                       m_utc_min;
    int                       m_utc_sec;
    wxString                  m_date_string;
    
    wxDateTime                m_ack_time;
    bool                      b_in_ack_timeout;

    double                    Range_NM;
    double                    Brg;

    wxString                  MSG_14_text;

    //      Per target collision parameters
    bool                      bCPA_Valid;
    double                    TCPA;                     // Minutes
    double                    CPA;                      // Nautical Miles

    bool                      b_show_AIS_CPA;           //TR 2012.06.28: Show AIS-CPA
    
    bool                      b_show_track;

    AISTargetTrackList        *m_ptrack;

    AIS_Area_Notice_Hash     area_notices;
    bool                     b_SarAircraftPosnReport;
    int                      altitude;                  // Metres, from special position report(9)
    bool                     b_nameFromCache;
    float                    importance;
    float                    last_scale;
    
};

#endif
