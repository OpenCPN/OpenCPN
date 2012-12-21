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
 */

#include "wx/wx.h"
#include "wx/tokenzr.h"
#include "wx/datetime.h"
#include "wx/sound.h"
#include <wx/wfstream.h>
#include <wx/imaglist.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "dychart.h"

#include "ais.h"
#include "chart1.h"
#include "navutil.h"        // for Select
#include "georef.h"
#include "pluginmanager.h"  // for PlugInManager
#include "styles.h"
#include "datastream.h"

extern  int             s_dns_test_flag;
extern  Select          *pSelectAIS;
extern  double          gLat, gLon, gSog, gCog;
extern FontMgr          *pFontMgr;
extern ChartCanvas      *cc1;
extern MyFrame          *gFrame;
extern MyConfig         *pConfig;
extern bool             g_bopengl;

//    AIS Global configuration
extern bool             g_bCPAMax;
extern double           g_CPAMax_NM;
extern bool             g_bCPAWarn;
extern double           g_CPAWarn_NM;
extern bool             g_bTCPA_Max;
extern double           g_TCPA_Max;
extern bool             g_bMarkLost;
extern double           g_MarkLost_Mins;
extern bool             g_bRemoveLost;
extern double           g_RemoveLost_Mins;
extern bool             g_bAISShowTracks;
extern double           g_AISShowTracks_Mins;
extern bool             g_bShowMoored;
extern double           g_ShowMoored_Kts;
extern bool             g_bShowAllCPA;

extern bool             g_bGPSAISMux;
extern ColorScheme      global_color_scheme;

extern bool             g_bAIS_CPA_Alert;
extern bool             g_bAIS_CPA_Alert_Audio;
extern bool             g_bAIS_CPA_Alert_Suppress_Moored;

extern AISTargetAlertDialog    *g_pais_alert_dialog_active;
extern int              g_ais_alert_dialog_x, g_ais_alert_dialog_y;
extern int              g_ais_alert_dialog_sx, g_ais_alert_dialog_sy;
extern wxString         g_sAIS_Alert_Sound_File;

extern AISTargetListDialog    *g_pAISTargetList;
extern int              g_AisTargetList_range;
extern wxString         g_AisTargetList_perspective;
extern int              g_AisTargetList_sortColumn;
extern bool             g_bAisTargetList_sortReverse;
extern wxString         g_AisTargetList_column_spec;
extern int              g_AisTargetList_count;

extern bool             g_bAISRolloverShowClass;
extern bool             g_bAISRolloverShowCOG;
extern bool             g_bAISRolloverShowCPA;

extern bool             g_bAIS_ACK_Timeout;
extern double           g_AckTimeout_Mins;

extern bool             bGPSValid;

extern PlugInManager    *g_pi_manager;
extern TTYWindow        *g_NMEALogWindow;
extern ocpnStyle::StyleManager* g_StyleManager;

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(AISTargetTrackList);

wxString ais_status[] = {
      _("Underway"),
      _("At Anchor"),
      _("Not Under Command"),
      _("Restricted Manoeuvrability"),
      _("Constrained by draught"),
      _("Moored"),
      _("Aground"),
      _("Engaged in Fishing"),
      _("Under way sailing"),
      _("High Speed Craft"),
      _("Wing In Ground Effect"),
      _("Reserved 11"),
      _("Reserved 12"),
      _("Reserved 13"),
      _("Reserved 14"),
      _("Undefined"),
      _("Virtual"),
      _("Virtual (On Position)"),
      _("Virtual (Off Position)"),
      _("Real"),
      _("Real (On Position)"),
      _("Real(Off Position)")
};

wxString ais_type[] = {
      _("Vessel Fishing"),             //30        0
      _("Vessel Towing"),              //31        1
      _("Vessel Towing, Long"),        //32        2
      _("Vessel Dredging"),            //33        3
      _("Vessel Diving"),              //34        4
      _("Military Vessel"),            //35        5
      _("Sailing Vessel"),             //36        6
      _("Pleasure craft"),             //37        7
      _("High Speed Craft"),           //4x        8
      _("Pilot Vessel"),               //50        9
      _("Search and Rescue Vessel"),   //51        10
      _("Tug"),                        //52        11
      _("Port Tender"),                //53        12
      _("Pollution Control Vessel"),   //54        13
      _("Law Enforcement Vessel"),     //55        14
      _("Medical Transport"),          //58        15
      _("Passenger Ship"),             //6x        16
      _("Cargo Ship"),                 //7x        17
      _("Tanker"),                     //8x        18
      _("Unknown"),                    //          19

      _("Unspecified"),                //00        20
      _("Reference Point"),            //01        21
      _("RACON"),                      //02        22
      _("Fixed Structure"),            //03        23
      _("Spare"),                      //04        24
      _("Light"),                      //05        25
      _("Light w/Sectors"),            //06        26
      _("Leading Light Front"),        //07        27
      _("Leading Light Rear"),         //08        28
      _("Cardinal N Beacon"),          //09        29
      _("Cardinal E Beacon"),          //10        30
      _("Cardinal S Beacon"),          //11        31
      _("Cardinal W Beacon"),          //12        32
      _("Beacon, Port Hand"),          //13        33
      _("Beacon, Starboard Hand"),     //14        34
      _("Beacon, Preferred Channel Port Hand"),         //15        35
      _("Beacon, Preferred Channel Starboard Hand"),    //16        36
      _("Beacon, Isolated Danger"),    //17        37
      _("Beacon, Safe Water"),         //18        38
      _("Beacon, Special Mark"),       //19        39
      _("Cardinal Mark N"),            //20        40
      _("Cardinal Mark E"),            //21        41
      _("Cardinal Mark S"),            //22        42
      _("Cardinal Mark W"),            //23        43
      _("Port Hand Mark"),             //24        44
      _("Starboard Hand Mark"),        //25        45
      _("Preferred Channel Port Hand"),      //26        46
      _("Preferred Channel Starboard Hand"), //27        47
      _("Isolated Danger"),            //28        48
      _("Safe Water"),                 //29        49
      _("Special Mark"),               //30        50
      _("Light Vessel/Rig"),           //31        51
      _("GpsGate Buddy"),              //xx           52
      _("Position Report"),            //xx        53
      _("Distress")                    //xx        54
};

wxString short_ais_type[] = {
      _("F/V"),                  //30        0
      _("Tow"),                  //31        1
      _("Long Tow"),             //32        2
      _("Dredge"),               //33        3
      _("D/V"),                  //34        4
      _("Mil/V"),                //35        5
      _("S/V"),                  //36        6
      _("Yat"),                  //37        7
      _("HSC"),                  //4x        8
      _("P/V"),                  //50        9
      _("SAR/V"),                //51        10
      _("Tug"),                  //52        11
      _("Tender"),               //53        12
      _("PC/V"),                 //54        13
      _("LE/V"),                 //55        14
      _("Med/V"),                //58        15
      _("Pass/V"),               //6x        16
      _("M/V"),                  //7x        17
      _("M/T"),                  //8x        18
      _("?"),                    //          19

    _("AtoN"),                 //00        20
    _("Ref. Pt"),              //01        21
    _("RACON"),                //02        22
    _("Fix.Struct."),          //03        23
    _("?"),                    //04        24
    _("Lt"),                   //05        25
    _("Lt sect."),             //06        26
    _("Ldg Lt Front"),         //07        27
    _("Ldg Lt Rear"),          //08        28
    _("Card. N"),              //09        29
    _("Card. E"),              //10        30
    _("Card. S"),              //11        31
    _("Card. W"),              //12        32
    _("Port"),                 //13        33
    _("Stbd"),                 //14        34
    _("Pref. Chnl"),           //15        35
    _("Pref. Chnl"),           //16        36
    _("Isol. Dngr"),           //17        37
    _("Safe Water"),           //18        38
    _("Special"),              //19        39
    _("Card. N"),              //20        40
    _("Card. E"),              //21        41
    _("Card. S"),              //22        42
    _("Card. W"),              //23        43
    _("Port Hand"),            //24        44
    _("Stbd Hand"),            //25        45
    _("Pref. Chnl"),           //26        46
    _("Pref. Chnl"),           //27        47
    _("Isol. Dngr"),           //28        48
    _("Safe Water"),           //29        49
    _("Special"),              //30        50
    _("LtV/Rig"),              //31        51
    _("Buddy"),                //xx        52
    _("DSC"),                  //xx        53
    _("Distress")              //xx        54
};

wxString ais8_001_22_notice_names[] = { // 128] = {
    _("Caution Area: Marine mammals habitat (implies whales NOT observed)"), // 0 - WARNING: extra text by Kurt
    _("Caution Area: Marine mammals in area - reduce speed"), // 1
    _("Caution Area: Marine mammals in area - stay clear"), // 2
    _("Caution Area: Marine mammals in area - report sightings"), // 3
    _("Caution Area: Protected habitat - reduce speed"), // 4
    _("Caution Area: Protected habitat - stay clear"), // 5
    _("Caution Area: Protected habitat - no fishing or anchoring"), // 6
    _("Caution Area: Derelicts (drifting objects)"), // 7
    _("Caution Area: Traffic congestion"), // 8
    _("Caution Area: Marine event"), // 9
    _("Caution Area: Divers down"), // 10
    _("Caution Area: Swim area"), // 11
    _("Caution Area: Dredge operations"), // 12
    _("Caution Area: Survey operations"), // 13
    _("Caution Area: Underwater operation"), // 14
    _("Caution Area: Seaplane operations"), // 15
    _("Caution Area: Fishery - nets in water"), // 16
    _("Caution Area: Cluster of fishing vessels"), // 17
    _("Caution Area: Fairway closed"), // 18
    _("Caution Area: Harbour closed"), // 19
    _("Caution Area: Risk (define in Associated text field)"), // 20
    _("Caution Area: Underwater vehicle operation"), // 21
    _("(reserved for future use)"), // 22
    _("Environmental Caution Area: Storm front (line squall)"), // 23
    _("Environmental Caution Area: Hazardous sea ice"), // 24
    _("Environmental Caution Area: Storm warning (storm cell or line of storms)"), // 25
    _("Environmental Caution Area: High wind"), // 26
    _("Environmental Caution Area: High waves"), // 27
    _("Environmental Caution Area: Restricted visibility (fog, rain, etc.)"), // 28
    _("Environmental Caution Area: Strong currents"), // 29
    _("Environmental Caution Area: Heavy icing"), // 30
    _("(reserved for future use)"), // 31
    _("Restricted Area: Fishing prohibited"), // 32
    _("Restricted Area: No anchoring."), // 33
    _("Restricted Area: Entry approval required prior to transit"), // 34
    _("Restricted Area: Entry prohibited"), // 35
    _("Restricted Area: Active military OPAREA"), // 36
    _("Restricted Area: Firing - danger area."), // 37
    _("Restricted Area: Drifting Mines"), // 38
    _("(reserved for future use)"), // 39
    _("Anchorage Area: Anchorage open"), // 40
    _("Anchorage Area: Anchorage closed"), // 41
    _("Anchorage Area: Anchoring prohibited"), // 42
    _("Anchorage Area: Deep draft anchorage"), // 43
    _("Anchorage Area: Shallow draft anchorage"), // 44
    _("Anchorage Area: Vessel transfer operations"), // 45
    _("(reserved for future use)"), // 46
    _("(reserved for future use)"), // 47
    _("(reserved for future use)"), // 48
    _("(reserved for future use)"), // 49
    _("(reserved for future use)"), // 50
    _("(reserved for future use)"), // 51
    _("(reserved for future use)"), // 52
    _("(reserved for future use)"), // 53
    _("(reserved for future use)"), // 54
    _("(reserved for future use)"), // 55
    _("Security Alert - Level 1"), // 56
    _("Security Alert - Level 2"), // 57
    _("Security Alert - Level 3"), // 58
    _("(reserved for future use)"), // 59
    _("(reserved for future use)"), // 60
    _("(reserved for future use)"), // 61
    _("(reserved for future use)"), // 62
    _("(reserved for future use)"), // 63
    _("Distress Area: Vessel disabled and adrift"), // 64
    _("Distress Area: Vessel sinking"), // 65
    _("Distress Area: Vessel abandoning ship"), // 66
    _("Distress Area: Vessel requests medical assistance"), // 67
    _("Distress Area: Vessel flooding"), // 68
    _("Distress Area: Vessel fire/explosion"), // 69
    _("Distress Area: Vessel grounding"), // 70
    _("Distress Area: Vessel collision"), // 71
    _("Distress Area: Vessel listing/capsizing"), // 72
    _("Distress Area: Vessel under assault"), // 73
    _("Distress Area: Person overboard"), // 74
    _("Distress Area: SAR area"), // 75
    _("Distress Area: Pollution response area"), // 76
    _("(reserved for future use)"), // 77
    _("(reserved for future use)"), // 78
    _("(reserved for future use)"), // 79
    _("Instruction: Contact VTS at this point/juncture"), // 80
    _("Instruction: Contact Port Administration at this point/juncture"), // 81
    _("Instruction: Do not proceed beyond this point/juncture"), // 82
    _("Instruction: Await instructions prior to proceeding beyond this point/juncture"), // 83
    _("Proceed to this location - await instructions"), // 84
    _("Clearance granted - proceed to berth"), // 85
    _("(reserved for future use)"), // 86
    _("(reserved for future use)"), // 87
    _("Information: Pilot boarding position"), // 88
    _("Information: Icebreaker waiting area"), // 89
    _("Information: Places of refuge"), // 90
    _("Information: Position of icebreakers"), // 91
    _("Information: Location of response units"), // 92
    _("VTS active target"), // 93
    _("Rouge or suspicious vessel"), // 94
    _("Vessel requesting non-distress assistance"), // 95
    _("Chart Feature: Sunken vessel"), // 96
    _("Chart Feature: Submerged object"), // 97
    _("Chart Feature: Semi-submerged object"), // 98
    _("Chart Feature: Shoal area"), // 99
    _("Chart Feature: Shoal area due north"), // 100
    _("Chart Feature: Shoal area due east"), // 101
    _("Chart Feature: Shoal area due south"), // 102
    _("Chart Feature: Shoal area due west"), // 103
    _("Chart Feature: Channel obstruction"), // 104
    _("Chart Feature: Reduced vertical clearance"), // 105
    _("Chart Feature: Bridge closed"), // 106
    _("Chart Feature: Bridge partially open"), // 107
    _("Chart Feature: Bridge fully open"), // 108
    _("(reserved for future use)"), // 109
    _("(reserved for future use)"), // 110
    _("(reserved for future use)"), // 111
    _("Report from ship: Icing info"), // 112
    _("(reserved for future use)"), // 113
    _("Report from ship: Miscellaneous information - define in Associated text field"), // 114
    _("(reserved for future use)"), // 115
    _("(reserved for future use)"), // 116
    _("(reserved for future use)"), // 117
    _("(reserved for future use)"), // 118
    _("(reserved for future use)"), // 119
    _("Route: Recommended route"), // 120
    _("Route: Alternative route"), // 121
    _("Route: Recommended route through ice"), // 122
    _("(reserved for future use)"), // 123
    _("(reserved for future use)"), // 124
    _("Other - Define in associated text field"), // 125
    _("Cancellation - cancel area as identified by Message Linkage ID"), // 126
    _("Undefined (default)") //, // 127
};

enum {
    tlNAME = 0,
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
};// AISTargetListCtrl Columns;


//    Define and declare a hasmap for ERI Ship type strings, keyed by their UN Codes.
 WX_DECLARE_HASH_MAP( int, wxString, wxIntegerHash, wxIntegerEqual, ERIShipTypeHash );

 ERIShipTypeHash        g_ERI_hash;


//#define AIS_DEBUG  1

wxString trimAISField( char *data )
{
    //  Clip any unused characters (@) from data

    wxString field = wxString::From8BitData( data );
    while( field.Right( 1 ) == '@' || field.Right( 1 ) == ' ' )
        field.RemoveLast();
    return field;
}

//---------------------------------------------------------------------------------
//
//  AIS_Target_Data Implementation
//
//---------------------------------------------------------------------------------
AIS_Target_Data::AIS_Target_Data()
{
    strncpy(ShipName, "Unknown             ", 21);
    strncpy(CallSign, "       ", 8);
    strncpy(Destination, "                    ", 21);
    ShipNameExtension[0] = 0;
    b_show_AIS_CPA = false;             

    SOG = 555.;
    COG = 666.;
    HDG = 511.;
    ROTAIS = -128;
    Lat = 0.;
    Lon = 0.;

    wxDateTime now = wxDateTime::Now();
    now.MakeGMT();
    PositionReportTicks = now.GetTicks();       // Default is my idea of NOW
    StaticReportTicks = now.GetTicks();
    b_lost = false;

    IMO = 0;
    MID = 555;
    MMSI = 666;
    NavStatus = UNDEFINED;
    SyncState = 888;
    SlotTO = 999;
    ShipType = 19;    // "Unknown"

    CPA = 100;     // Large values avoid false alarms
    TCPA = 100;

    Range_NM = -1.;
    Brg = -1.;

    DimA = DimB = DimC = DimD = 0;;

    ETA_Mo = 0;
    ETA_Day = 0;
    ETA_Hr = 24;
    ETA_Min = 60;

    Draft = 0.;

    RecentPeriod = 0;

    m_utc_hour = 0;
    m_utc_min = 0;
    m_utc_sec = 0;

    Class = AIS_CLASS_A;      // default
    n_alarm_state = AIS_NO_ALARM;
    b_suppress_audio = false;
    b_positionDoubtful = false;
    b_positionOnceValid = false;
    b_nameValid = false;

    Euro_Length = 0;            // Extensions for European Inland AIS
    Euro_Beam = 0;
    Euro_Draft = 0;
    strncpy(Euro_VIN, "       ", 8);
    UN_shiptype = 0;

    b_isEuroInland = false;
    b_blue_paddle = false;

    b_OwnShip = false;
    b_in_ack_timeout = false;

    m_ptrack = new AISTargetTrackList;
    b_active = false;
    blue_paddle = 0;
    bCPA_Valid = false;
    ROTIND = 0;
}

AIS_Target_Data::~AIS_Target_Data()
{
      delete m_ptrack;
}

wxString FormatTimeAdaptive( int seconds ) {
    int s = seconds % 60;
    int m = seconds / 60;
    if( seconds < 100 )
        return wxString::Format( _T("%3ds"), seconds );
    else if( seconds < 3600 ) {
        int m = seconds / 60;
        int s = seconds % 60;
        return wxString::Format( _T("%2dmin %02ds"), m, s );
    }
    int h = seconds / 3600;
    m -= h* 60;
    return wxString::Format( _T("%2dh %02dmin"), h, m );
}

wxString AIS_Target_Data::BuildQueryResult( void )
{
    wxString html;
    wxDateTime now = wxDateTime::Now();

    wxString tableStart = _T("\n<table border=0 cellpadding=1 cellspacing=0>\n");
    wxString tableEnd = _T("</table>\n\n");
    wxString rowStart = _T("<tr><td><font size=-2>");
    wxString rowStartH = _T("<tr><td nowrap>");
    wxString rowSeparator = _T("</font></td><td></td><td><b>");
    wxString rowSeparatorH = _T("</td><td></td><td>");
    wxString colSeparator = _T("<td></td>");
    wxString rowEnd = _T("</b></td></tr>\n");
    wxString vertSpacer = _T("<tr><td></td></tr><tr><td></td></tr><tr><td></td></tr>\n\n");

    wxString IMOstr, MMSIstr, ClassStr;

    html << tableStart << _T("<tr><td nowrap colspan=2>");
    if( ( Class != AIS_BASE ) && ( Class != AIS_SART ) ) {
        if( b_nameValid ) {
            wxString shipName = trimAISField( ShipName );
            wxString intlName;
            if( shipName == _T("Unknown") ) intlName = wxGetTranslation( shipName );
            else
                intlName = shipName;
            html << _T("<font size=+2><i><b>") << intlName ;
            if( strlen( ShipNameExtension ) ) html << wxString( ShipNameExtension, wxConvUTF8 );
            html << _T("</b></i></font>&nbsp;&nbsp;<b>");
        }
    }

    if( ( Class != AIS_ATON ) && ( Class != AIS_BASE ) && ( Class != AIS_GPSG_BUDDY )
            && ( Class != AIS_SART ) ) {
        html << trimAISField( CallSign ) << _T("</b>") << rowEnd;

        if( Class != AIS_CLASS_B ) {
            if( IMO > 0 ) IMOstr = wxString::Format( _T("%08d"), abs( IMO ) );
        }
    }
    else html << _T("</b>") << rowEnd;

    html << vertSpacer;

    if( Class != AIS_GPSG_BUDDY ) {
        MMSIstr = wxString::Format( _T("%09d"), abs( MMSI ) );
    }
    ClassStr = wxGetTranslation( Get_class_string( false ) );

    if( IMOstr.Length() )
        html << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
            << rowStart <<_("MMSI") << _T("</font></td><td>&nbsp;</td><td><font size=-2>")
            << _("Class") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>")
            << _("IMO") << _T("</font></td></tr>")
            << rowStartH << _T("<b>") << MMSIstr << _T("</b></td><td>&nbsp;</td><td><b>")
            << ClassStr << _T("</b></td><td>&nbsp;</td><td align=right><b>")
            << IMOstr << rowEnd << _T("</table></td></tr>")
            << vertSpacer;
    else
        html << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
            << rowStart <<_("MMSI") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>")
            << _("Class") << _T("</font></td></tr>")
            << rowStartH << _T("<b>") << MMSIstr << _T("</b></td><td>&nbsp;</td><td align=right><b>")
            << ClassStr << rowEnd << _T("</table></td></tr>")
            << vertSpacer;

    wxString navStatStr;
    if( ( Class != AIS_BASE ) && ( Class != AIS_CLASS_B ) && ( Class != AIS_SART ) ) {
        if( ( NavStatus <= 21  ) && ( NavStatus >= 0 ) )
            navStatStr = wxGetTranslation( ais_status[NavStatus] );
    } else if( Class == AIS_SART ) {
        if( NavStatus == RESERVED_14 ) navStatStr = _("Active");
        else if( NavStatus == UNDEFINED ) navStatStr = _("Testing");
    }

    wxString sub_type;
    if( Class == AIS_SART ) {
        int mmsi_start = MMSI / 1000000;
        switch( mmsi_start ){
            case 970:
//                        sub_type = _T("SART");
                break;
            case 972:
                sub_type = _T("MOB");
                break;
            case 974:
                sub_type = _T("EPIRB");
                break;
            default:
                sub_type = _("Unknown");
                break;
        }
    }

    wxString AISTypeStr, UNTypeStr, sizeString;
    if( ( Class != AIS_BASE ) && ( Class != AIS_SART ) ) {

        //      Ship type
        AISTypeStr = wxGetTranslation( Get_vessel_type_string() );

        if( b_isEuroInland && UN_shiptype ) {
            ERIShipTypeHash::iterator it = g_ERI_hash.find( UN_shiptype );
            wxString type;
            if( it == g_ERI_hash.end() ) type = _("Undefined");
            else
                type = it->second;

            UNTypeStr = wxGetTranslation( type );
        }

        if( Class == AIS_SART ) {
            if( MSG_14_text.Len() ) {
                html << rowStart << _("Safety Broadcast Message") << rowEnd
                    << rowStartH << _T("<b>") << MSG_14_text << rowEnd;
            }
        }

       //  Dimensions

        if( NavStatus != ATON_VIRTUAL ) {
            if( ( Class == AIS_CLASS_B ) || ( Class == AIS_ATON ) ) {
                sizeString = wxString::Format( _T("%dm x %dm"), ( DimA + DimB ), ( DimC + DimD ) );
            } else {
                if( ( DimA + DimB + DimC + DimD ) == 0 ) {
                    if( b_isEuroInland ) {
                        if( Euro_Length == 0.0 ) {
                            if( Euro_Draft > 0.01 ) {
                                sizeString << wxString::Format( _T("---m x ---m x %4.1fm"), Euro_Draft );
                            } else {
                                sizeString << _T("---m x ---m x ---m");
                            }
                        } else {
                            if( Euro_Draft > 0.01 ) {
                                sizeString
                                        << wxString::Format( _T("%5.1fm x %4.1fm x %4.1fm"), Euro_Length,
                                                Euro_Beam, Euro_Draft );
                            } else {
                                sizeString
                                        << wxString::Format( _T("%5.1fm x %4.1fm x ---m\n\n"), Euro_Length,
                                                Euro_Beam );
                            }
                        }
                    } else {
                        if( Draft > 0.01 ) {
                            sizeString << wxString::Format( _T("---m x ---m x %4.1fm"), Draft );
                        } else {
                            sizeString << _T("---m x ---m x ---m");
                        }
                    }
                } else if( Draft < 0.01 ) {
                    sizeString
                            << wxString::Format( _T("%dm x %dm x ---m"), ( DimA + DimB ), ( DimC + DimD ) );
                } else {
                    sizeString
                            << wxString::Format( _T("%dm x %dm x %4.1fm"), ( DimA + DimB ), ( DimC + DimD ),
                                    Draft );
                }
            }
        }
    }

    if( ( Class != AIS_ATON ) && ( Class != AIS_BASE ) ) {
        html << _T("<tr><td colspan=2>") << _T("<b>") << AISTypeStr;
        if( sub_type.Length() ) html << _T(" (") << sub_type << _T(")");
        html << _T(", ") << navStatStr;
        if( UNTypeStr.Length() ) html << _T(" (UN Type ") << UNTypeStr << _T(")");
        html << rowEnd << _T("<tr><td colspan=2>") << _T("<b>") << sizeString << rowEnd;
    }

    if( b_positionOnceValid ) {
        wxString posTypeStr;
        if( b_positionDoubtful ) posTypeStr << _(" (Last Known)");

        now.MakeGMT();
        int target_age = now.GetTicks() - PositionReportTicks;

        html << vertSpacer
             << rowStart << _("Position") << posTypeStr << _T("</font></td><td align=right><font size=-2>")
             << _("Report Age") << _T("</font></td></tr>")

             << rowStartH << _T("<b>") << toSDMM( 1, Lat ) << _T("</b></td><td align=right><b>")
             << FormatTimeAdaptive( target_age ) << rowEnd
             << rowStartH << _T("<b>") << toSDMM( 2, Lon ) << rowEnd;
    }

    wxString courseStr, sogStr, hdgStr, rotStr, rngStr, brgStr, destStr, etaStr;

    if( Class == AIS_GPSG_BUDDY ) {
        html << vertSpacer << rowStart << _("Report as of") << rowEnd
             << rowStartH << wxString::Format( _T("<b>%d:%d UTC "), m_utc_hour, m_utc_min )
             << rowEnd;
    } else {
        if( Class == AIS_CLASS_A ) {
            html << vertSpacer << rowStart << _("Destination")
                 << _T("</font></td><td align=right><font size=-2>")
                 << _("ETA") << _T("</font></td></tr>\n")
                 << rowStartH << _T("<b>") << trimAISField( Destination )
                 << _T("</b></td><td nowrap align=right><b>");

            if( ( ETA_Mo ) && ( ETA_Hr < 24 ) ) {
                int yearOffset = 0;
                if( now.GetMonth() > ( ETA_Mo - 1 ) ) yearOffset = 1;
                wxDateTime eta( ETA_Day, wxDateTime::Month( ETA_Mo - 1 ),
                        now.GetYear() + yearOffset, ETA_Hr, ETA_Min );
                html << eta.Format( _T("%b %d %H:%M") );
            }
            else html << _("Unavailable");
            html << rowEnd;
        }

        if( ( Class == AIS_CLASS_A ) || ( Class == AIS_CLASS_B ) ) {
            int crs = wxRound( COG );
            if( crs < 360 ) courseStr = wxString::Format( _T("%03d&deg;"), crs );
            else if( COG == 360.0 ) courseStr = _T("---");
            else if( crs == 360 ) courseStr = _T("0&deg;");

            if( SOG <= 102.2 )
                sogStr = wxString::Format( _T("%5.2f Kts"), SOG );
            else
                sogStr = _("---");

            if( (int) HDG != 511 )
                hdgStr = wxString::Format( _T("%03d&deg;"), (int) HDG );
            else
                hdgStr = _T("---");


            if( ROTAIS != -128 ) {
                if( ROTAIS == 127 ) rotStr << _T("> 5&deg;/30s ") << _("Right");
                else if( ROTAIS == -127 ) rotStr << _T("> 5&deg;/30s ") << _("Left");
                else {
                    if( ROTIND > 0 ) rotStr << wxString::Format( _T("%3d&deg;/Min "), ROTIND ) << _("Right");
                    else if( ROTIND < 0 ) rotStr << wxString::Format( _T("%3d&deg;/Min "), -ROTIND ) << _("Left");
                    else rotStr = _T("0");
                }
            }
        }
    }

    if( b_positionOnceValid && bGPSValid && ( Range_NM >= 0. ) )
        rngStr = cc1->FormatDistanceAdaptive( Range_NM );
    else
        rngStr = _("---");

    int brg = (int) wxRound( Brg );
    if( Brg > 359.5 ) brg = 0;
    if( b_positionOnceValid && bGPSValid && ( Brg >= 0. ) && ( Range_NM > 0. ) && ( fabs( Lat ) < 85. ) )
        brgStr = wxString::Format( _T("%03d&deg;"), brg );
    else
        brgStr = _("---");

    wxString turnRateHdr; // Blank if ATON or BASE
    if( ( Class != AIS_ATON ) && ( Class != AIS_BASE ) ) {
        html << vertSpacer << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
            << rowStart <<_("Speed") << _T("</font></td><td>&nbsp;</td><td><font size=-2>")
            << _("Course") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>")
            << _("Heading") << _T("</font></td></tr>")
            << rowStartH << _T("<b>") << sogStr << _T("</b></td><td>&nbsp;</td><td><b>")
            << courseStr << _T("</b></td><td>&nbsp;</td><td align=right><b>")
            << hdgStr << rowEnd << _T("</table></td></tr>")
            << vertSpacer;
        turnRateHdr = _("Turn Rate");
    }
    html << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
        << rowStart <<_("Range") << _T("</font></td><td>&nbsp;</td><td><font size=-2>")
        << _("Bearing") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>")
        << turnRateHdr << _T("</font></td></tr>")
        << rowStartH << _T("<b>") << rngStr << _T("</b></td><td>&nbsp;</td><td><b>")
        << brgStr << _T("</b></td><td>&nbsp;</td><td align=right><b>")
        << rotStr << rowEnd << _T("</table></td></tr>")
        << vertSpacer;

    wxString tcpaStr;
    if( bCPA_Valid ) tcpaStr << _(" </b>in<b> ") << FormatTimeAdaptive( (int)(TCPA*60.) );

    if( bCPA_Valid ) {
        html<< vertSpacer << rowStart << _("CPA") << rowEnd
            << rowStartH << _T("<b>") << cc1->FormatDistanceAdaptive( CPA )
            << tcpaStr << rowEnd;
    }

    if( Class != AIS_BASE ) {
        if( blue_paddle == 1 ) {
            html << rowStart << _("Inland Blue Flag") << rowEnd
                 << rowStartH << _T("<b>") << _("Clear") << rowEnd;
        } else if( blue_paddle == 2 ) {
            html << rowStart << _("Inland Blue Flag") << rowEnd
                 << rowStartH << _T("<b>") << _("Set") << rowEnd;
        }
    }

    html << _T("</table>");
    return html;
}

wxString AIS_Target_Data::GetRolloverString( void )
{
    wxString result;
    wxString t;
    if( b_nameValid ) {
        result.Append( _T("\"") );
        wxString uret = trimAISField( ShipName );
        wxString ret;
        if( uret == _T("Unknown") ) ret = wxGetTranslation( uret );
        else
            ret = uret;

        result.Append( ret );
        if( strlen( ShipNameExtension ) ) result.Append(
                wxString( ShipNameExtension, wxConvUTF8 ) );

        result.Append( _T("\" ") );
    }
    if( Class != AIS_GPSG_BUDDY ) {
        t.Printf( _T("%09d"), abs( MMSI ) );
        result.Append( t );
    }
    t = trimAISField( CallSign );
    if( t.Len() ) {
        result.Append( _T(" (") );
        result.Append( t );
        result.Append( _T(")") );
    }
    if( g_bAISRolloverShowClass || ( Class == AIS_SART ) ) {
        if( result.Len() ) result.Append( _T("\n") );
        result.Append( _T("[") );
        result.Append( wxGetTranslation( Get_class_string( false ) ) );
        result.Append( _T("] ") );
        if( ( Class != AIS_ATON ) && ( Class != AIS_BASE ) ) {
            if( Class == AIS_SART ) {
                int mmsi_start = MMSI / 1000000;
                switch( mmsi_start ){
                    case 970:
                        break;
                    case 972:
                        result += _T("MOB");
                        break;
                    case 974:
                        result += _T("EPIRB");
                        break;
                    default:
                        result += _("Unknown");
                        break;
                }
            }

            if( Class != AIS_SART ) result.Append(
                    wxGetTranslation( Get_vessel_type_string( false ) ) );

            if( ( Class != AIS_CLASS_B ) && ( Class != AIS_SART ) ) {
                if( ( NavStatus <= 15 ) && ( NavStatus >= 0 ) ) {
                    result.Append( _T(" (") );
                    result.Append( wxGetTranslation( ais_status[NavStatus] ) );
                    result.Append( _T(")") );
                }
            } else if( Class == AIS_SART ) {
                result.Append( _T(" (") );
                if( NavStatus == RESERVED_14 ) result.Append( _("Active") );
                else if( NavStatus == UNDEFINED ) result.Append( _("Testing") );

                result.Append( _T(")") );
            }

        }
    }

    if( g_bAISRolloverShowCOG && ( SOG <= 102.2 )
            && ( ( Class != AIS_ATON ) && ( Class != AIS_BASE ) ) ) {
        if( result.Len() ) result << _T("\n");
        if( SOG < 10.0 ) result << wxString::Format( _T("SOG %.2f "), SOG ) << _("Kts") << _T(" ");
        else
            result << wxString::Format( _T("SOG %.1f "), SOG ) << _("Kts") << _T(" ");

        int crs = wxRound( COG );
        if( b_positionOnceValid ) {
            if( crs < 360 ) result
                    << wxString::Format( wxString( " COG %03d°", wxConvUTF8 ), crs );
            else if( COG == 360.0 ) result << _(" COG Unavailable");
            else if( crs == 360 ) result << wxString( " COG 000°", wxConvUTF8 );
        } else
            result << _(" COG Unavailable");
    }

    if( g_bAISRolloverShowCPA && bCPA_Valid ) {
        if( result.Len() ) result << _T("\n");
        result << _("CPA") << _T(" ") << cc1->FormatDistanceAdaptive( CPA )
        << _T(" ") << _("in") << _T(" ")
        << wxString::Format( _T("%.0f"), TCPA ) << _T(" ") << _("min");
    }
    return result;
}

wxString AIS_Target_Data::Get_vessel_type_string( bool b_short )
{
    int i = 19;
    if( Class == AIS_ATON ) {
        i = ShipType + 20;
    } else
        switch( ShipType ){
            case 30:
                i = 0;
                break;
            case 31:
                i = 1;
                break;
            case 32:
                i = 2;
                break;
            case 33:
                i = 3;
                break;
            case 34:
                i = 4;
                break;
            case 35:
                i = 5;
                break;
            case 36:
                i = 6;
                break;
            case 37:
                i = 7;
                break;
            case 50:
                i = 9;
                break;
            case 51:
                i = 10;
                break;
            case 52:
                i = 11;
                break;
            case 53:
                i = 12;
                break;
            case 54:
                i = 13;
                break;
            case 55:
                i = 14;
                break;
            case 58:
                i = 15;
                break;
            default:
                i = 19;
                break;
        }

    if( ( Class == AIS_CLASS_B ) || ( Class == AIS_CLASS_A ) ) {
        if( ( ShipType >= 40 ) && ( ShipType < 50 ) ) i = 8;

        if( ( ShipType >= 60 ) && ( ShipType < 70 ) ) i = 16;

        if( ( ShipType >= 70 ) && ( ShipType < 80 ) ) i = 17;

        if( ( ShipType >= 80 ) && ( ShipType < 90 ) ) i = 18;
    } else if( Class == AIS_GPSG_BUDDY ) i = 52;
    else if( Class == AIS_DSC ) i = ( ShipType == 12 ) ? 54 : 53;  // 12 is distress

    if( !b_short ) return ais_type[i];
    else
        return short_ais_type[i];
}

wxString AIS_Target_Data::Get_class_string( bool b_short )
{
    switch( Class ){
        case AIS_CLASS_A:
            return _("A");
        case AIS_CLASS_B:
            return _("B");
        case AIS_ATON:
            return b_short ? _("AtoN") : _("Aid to Navigation");
        case AIS_BASE:
            return b_short ? _("Base") : _("Base Station");
        case AIS_GPSG_BUDDY:
            return b_short ? _("Buddy") : _("GPSGate Buddy");
        case AIS_DSC:
            return b_short ? _("DSC") : _("DSC Position Report");
        case AIS_SART:
            return b_short ? _("SART") : _("SART");

        default:
            return b_short ? _("Unk") : _("Unknown");
    }
}

void AIS_Target_Data::Toggle_AIS_CPA(void)
{
    b_show_AIS_CPA = !b_show_AIS_CPA ? true : false;
}

//---------------------------------------------------------------------------------
//
//  AIS_Decoder Helpers
//
//---------------------------------------------------------------------------------
AIS_Bitstring::AIS_Bitstring( const char *str )
{
    byte_length = strlen( str );

    for( int i = 0; i < byte_length; i++ ) {
        bitbytes[i] = to_6bit( str[i] );
    }
}

int AIS_Bitstring::GetBitCount()
{
    return byte_length * 6;
}


//  Convert printable characters to IEC 6 bit representation
//  according to rules in IEC AIS Specification
unsigned char AIS_Bitstring::to_6bit(const char c)
{
    if(c < 0x30)
        return (unsigned char)-1;
    if(c > 0x77)
        return (unsigned char)-1;
    if((0x57 < c) && (c < 0x60))
        return (unsigned char)-1;

    unsigned char cp = c;
    cp += 0x28;

    if(cp > 0x80)
        cp += 0x20;
    else
        cp += 0x28;

    return (unsigned char)(cp & 0x3f);
}


int AIS_Bitstring::GetInt(int sp, int len, bool signed_flag)
{
    int acc = 0;
    int s0p = sp-1;                          // to zero base

    int cp, cx, c0, cs;


    for(int i=0 ; i<len ; i++)
    {
        acc  = acc << 1;
        cp = (s0p + i) / 6;
        cx = bitbytes[cp];        // what if cp >= byte_length?
        cs = 5 - ((s0p + i) % 6);
        c0 = (cx >> (5 - ((s0p + i) % 6))) & 1;
        if(i == 0 && signed_flag && c0) // if signed value and first bit is 1, pad with 1's
            acc = ~acc;
        acc |= c0;
    }

    return acc;

}

int AIS_Bitstring::GetStr(int sp, int bit_len, char *dest, int max_len)
{
    //char temp_str[85];
    char *temp_str = new char[max_len + 1];

    char acc = 0;
    int s0p = sp-1;                          // to zero base

    int k=0;
    int cp, cx, c0, cs;

    int i = 0;
    while(i < bit_len && k < max_len)
    {
         acc=0;
         for(int j=0 ; j<6 ; j++)
         {
            acc  = acc << 1;
            cp = (s0p + i) / 6;
            cx = bitbytes[cp];        // what if cp >= byte_length?
            cs = 5 - ((s0p + i) % 6);
            c0 = (cx >> (5 - ((s0p + i) % 6))) & 1;
            acc |= c0;

            i++;
         }
         temp_str[k] = (char)(acc & 0x3f);

         if(acc < 32)
             temp_str[k] += 0x40;
         k++;

    }

    temp_str[k] = 0;

    int copy_len = wxMin((int)strlen(temp_str), max_len);
    strncpy(dest, temp_str, copy_len);

    delete [] temp_str;

    return copy_len;
}

//---------------------------------------------------------------------------------
//
//  AIS_Decoder Implementation
//
//---------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(AIS_Decoder, wxEvtHandler)
    EVT_TIMER(TIMER_AIS1, AIS_Decoder::OnTimerAIS)
    EVT_TIMER(TIMER_AISAUDIO, AIS_Decoder::OnTimerAISAudio)
END_EVENT_TABLE()

static int n_msgs;
static int n_msg1;
static int n_msg5;
static int n_msg24;
static int n_newname;
static bool b_firstrx;
static int first_rx_ticks;
static int rx_ticks;

AIS_Decoder::AIS_Decoder( wxFrame *parent )
{
    AISTargetList = new AIS_Target_Hash;
    AIS_AreaNotice_Sources = new AIS_Target_Hash;
    BuildERIShipTypeHash();

    g_pais_alert_dialog_active = NULL;
    m_bAIS_Audio_Alert_On = false;

    m_n_targets = 0;

    m_parent_frame = parent;
    
    TimerAIS.SetOwner(this, TIMER_AIS1);
    TimerAIS.Start(TIMER_AIS_MSEC,wxTIMER_CONTINUOUS);
    
    //  Create/connect a dynamic event handler slot for wxEVT_OCPN_DATASTREAM(s)
    Connect(wxEVT_OCPN_DATASTREAM, (wxObjectEventFunction)(wxEventFunction)&AIS_Decoder::OnEvtAIS); 
}

AIS_Decoder::~AIS_Decoder( void )
{
    AIS_Target_Hash::iterator it;
    AIS_Target_Hash *current_targets = GetTargetList();

    for( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it ) {
        AIS_Target_Data *td = it->second;

        delete td;
    }

    delete current_targets;

#ifdef AIS_DEBUG
    printf("First message[1, 2] ticks: %d  Last Message [1,2]ticks %d  Difference:  %d\n", first_rx_ticks, rx_ticks, rx_ticks - first_rx_ticks);
#endif
}

#define MAKEHASHERI(key, description) g_ERI_hash[key] = description;

void AIS_Decoder::BuildERIShipTypeHash(void)
{
      MAKEHASHERI(8000, _("Vessel, type unknown"))
      MAKEHASHERI(8150, _("Freightbarge"))
      MAKEHASHERI(8160, _("Tankbarge"))
      MAKEHASHERI(8163, _("Tankbarge, dry cargo as if liquid (e.g. cement)"))
      MAKEHASHERI(8450, _("Service vessel, police patrol, port service"))
      MAKEHASHERI(8430, _("Pushboat, single"))
      MAKEHASHERI(8510, _("Object, not otherwise specified"))
      MAKEHASHERI(8470, _("Object, towed, not otherwise specified"))
      MAKEHASHERI(8490, _("Bunkership"))
      MAKEHASHERI(8010, _("Motor freighter"))
      MAKEHASHERI(8020, _("Motor tanker"))
      MAKEHASHERI(8021, _("Motor tanker, liquid cargo, type N"))
      MAKEHASHERI(8022, _("Motor tanker, liquid cargo, type C"))
      MAKEHASHERI(8023, _("Motor tanker, dry cargo as if liquid (e.g. cement)"))
      MAKEHASHERI(8030, _("Container vessel"))
      MAKEHASHERI(8040, _("Gas tanker"))
      MAKEHASHERI(8050, _("Motor freighter, tug"))
      MAKEHASHERI(8060, _("Motor tanker, tug"))
      MAKEHASHERI(8070, _("Motor freighter with one or more ships alongside"))
      MAKEHASHERI(8080, _("Motor freighter with tanker"))
      MAKEHASHERI(8090, _("Motor freighter pushing one or more freighters"))
      MAKEHASHERI(8100, _("Motor freighter pushing at least one tank-ship"))
      MAKEHASHERI(8110, _("Tug, freighter"))
      MAKEHASHERI(8120, _("Tug, tanker"))
      MAKEHASHERI(8130, _("Tug freighter, coupled"))
      MAKEHASHERI(8140, _("Tug, freighter/tanker, coupled"))
      MAKEHASHERI(8161, _("Tankbarge, liquid cargo, type N"))
      MAKEHASHERI(8162, _("Tankbarge, liquid cargo, type C"))
      MAKEHASHERI(8170, _("Freightbarge with containers"))
      MAKEHASHERI(8180, _("Tankbarge, gas"))
      MAKEHASHERI(8210, _("Pushtow, one cargo barge"))
      MAKEHASHERI(8220, _("Pushtow, two cargo barges"))
      MAKEHASHERI(8230, _("Pushtow, three cargo barges"))
      MAKEHASHERI(8240, _("Pushtow, four cargo barges"))
      MAKEHASHERI(8250, _("Pushtow, five cargo barges"))
      MAKEHASHERI(8260, _("Pushtow, six cargo barges"))
      MAKEHASHERI(8270, _("Pushtow, seven cargo barges"))
      MAKEHASHERI(8280, _("Pushtow, eight cargo barges"))
      MAKEHASHERI(8290, _("Pushtow, nine or more barges"))
      MAKEHASHERI(8310, _("Pushtow, one tank/gas barge"))
      MAKEHASHERI(8320, _("Pushtow, two barges at least one tanker or gas barge"))
      MAKEHASHERI(8330, _("Pushtow, three barges at least one tanker or gas barge"))
      MAKEHASHERI(8340, _("Pushtow, four barges at least one tanker or gas barge"))
      MAKEHASHERI(8350, _("Pushtow, five barges at least one tanker or gas barge"))
      MAKEHASHERI(8360, _("Pushtow, six barges at least one tanker or gas barge"))
      MAKEHASHERI(8370, _("Pushtow, seven barges at least one tanker or gas barge"))
      MAKEHASHERI(8380, _("Pushtow, eight barges at least one tanker or gas barge"))
      MAKEHASHERI(8390, _("Pushtow, nine or more barges at least one tanker or gas barge"))
      MAKEHASHERI(8400, _("Tug, single"))
      MAKEHASHERI(8410, _("Tug, one or more tows"))
      MAKEHASHERI(8420, _("Tug, assisting a vessel or linked combination"))
      MAKEHASHERI(8430, _("Pushboat, single"))
      MAKEHASHERI(8440, _("Passenger ship, ferry, cruise ship, red cross ship"))
      MAKEHASHERI(8441, _("Ferry"))
      MAKEHASHERI(8442, _("Red cross ship"))
      MAKEHASHERI(8443, _("Cruise ship"))
      MAKEHASHERI(8444, _("Passenger ship without accomodation"))
      MAKEHASHERI(8460, _("Vessel, work maintainance craft, floating derrick, cable-ship, buoy-ship, dredge"))
      MAKEHASHERI(8480, _("Fishing boat"))
      MAKEHASHERI(8500, _("Barge, tanker, chemical"))
      MAKEHASHERI(1500, _("General cargo Vessel maritime"))
      MAKEHASHERI(1510, _("Unit carrier maritime"))
      MAKEHASHERI(1520, _("Bulk carrier maritime"))
      MAKEHASHERI(1530, _("Tanker"))
      MAKEHASHERI(1540, _("Liquified gas tanker"))
      MAKEHASHERI(1850, _("Pleasure craft, longer than 20 metres"))
      MAKEHASHERI(1900, _("Fast ship"))
      MAKEHASHERI(1910, _("Hydrofoil"))
}

//----------------------------------------------------------------------------------
//     Handle events from AIS DataStream
//----------------------------------------------------------------------------------
void AIS_Decoder::OnEvtAIS( OCPN_DataStreamEvent& event )
{
    wxString message = wxString(event.GetNMEAString().c_str(), wxConvUTF8);
    
    int nr = 0;
    if( !message.IsEmpty() ) 
    {
        if( message.Mid( 3, 3 ).IsSameAs( _T("VDM") ) ||
            message.Mid( 3, 3 ).IsSameAs( _T("VDO") ) ||
            message.Mid( 1, 5 ).IsSameAs( _T("FRPOS") ) ||
            message.Mid( 1, 2 ).IsSameAs( _T("CD") ) )
        {
                nr = Decode( message );
                gFrame->TouchAISActive();
        }         
    }
}

//----------------------------------------------------------------------------------
//      Decode a single AIVDO sentence to a Generic Position Report
//----------------------------------------------------------------------------------
AIS_Error AIS_Decoder::DecodeSingleVDO( const wxString& str, GenericPosDatEx *pos )
{
    //  Make some simple tests for validity
    if( str.Len() > 100 )
        return AIS_NMEAVDX_TOO_LONG;
    
    if( !NMEACheckSumOK( str ) )
        return AIS_NMEAVDX_CHECKSUM_BAD;

    if( !pos ) 
        return AIS_GENERIC_ERROR;
    
    //  We only process AIVDO messages
    if( !str.Mid( 1, 5 ).IsSameAs( _T("AIVDO") ) ) 
        return AIS_GENERIC_ERROR;

    //  Use a tokenizer to pull out the first 4 fields
    wxStringTokenizer tkz( str, _T(",") );
        
    wxString token;
    token = tkz.GetNextToken();         // !xxVDx
        
    token = tkz.GetNextToken();
    int nsentences = atoi( token.mb_str() );
        
    token = tkz.GetNextToken();
    int isentence = atoi( token.mb_str() );
        
    token = tkz.GetNextToken();         // skip 2 fields
    token = tkz.GetNextToken();
        
    wxString string_to_parse = tkz.GetNextToken();    // tha actual data
        
      // We only parse the first part of one part sentences
    if( ( 1 != nsentences ) || ( 1 != isentence ) )
        return AIS_GENERIC_ERROR;
        
    //  Create the bit accessible string
    AIS_Bitstring strbit( string_to_parse.mb_str() );
    
    AIS_Target_Data *pTargetData = new AIS_Target_Data;

    bool bdecode_result = Parse_VDXBitstring( &strbit, pTargetData );
    
    if(bdecode_result) {
        switch(pTargetData->MID)
        {
            case 1:
            case 2:
            case 3:
            case 18:
            {
                pos->kLat = pTargetData->Lat;
                pos->kLon = pTargetData->Lon;
                
                if(pTargetData->COG == 360.0)
                    pos->kCog = NAN;
                else
                    pos->kCog = pTargetData->COG;
                
                
                if(pTargetData->SOG > 102.2)
                    pos->kSog = NAN;
                else
                    pos->kSog = pTargetData->SOG;
                
                if((int)pTargetData->HDG == 511)
                    pos->kHdt = NAN;
                else
                    pos->kHdt = pTargetData->HDG;
                
                //  VDO messages do not contain variation or magnetic heading
                pos->kVar = NAN;
                pos->kHdm = NAN;
                    
            }
            default:
                break;
        }
        
        return AIS_NoError;
    }
    else
        return AIS_GENERIC_ERROR;
}


//----------------------------------------------------------------------------------
//      Decode NMEA VDM/VDO/FRPOS/DSCDSE sentence to AIS Target(s)
//----------------------------------------------------------------------------------
AIS_Error AIS_Decoder::Decode( const wxString& str )
{
    AIS_Error ret;
    wxString string_to_parse;

    double gpsg_lat, gpsg_lon, gpsg_mins, gpsg_degs;
    double gpsg_cog, gpsg_sog, gpsg_utc_time;
    int gpsg_utc_hour = 0;
    int gpsg_utc_min = 0;
    int gpsg_utc_sec = 0;
    char gpsg_name_str[21];

    double dsc_lat = 0.;
    double dsc_lon = 0.;
    double dsc_mins, dsc_degs, dsc_tmp, dsc_addr;
    double dse_tmp, dse_addr;
    double dse_lat = 0.;
    double dse_lon = 0;
    long dsc_fmt, dsc_quadrant;

    int gpsg_mmsi = 0;
    int dsc_mmsi = 0;
    int dse_mmsi = 0;
    int mmsi = 0;

    //  Make some simple tests for validity

    if( str.Len() > 100 ) return AIS_NMEAVDX_TOO_LONG;

    if( !NMEACheckSumOK( str ) ) {
            return AIS_NMEAVDX_CHECKSUM_BAD;
    }
    if( str.Mid( 1, 2 ).IsSameAs( _T("CD") ) ) {
        // parse a DSC Position message            $CDDSx,.....
        //  Use a tokenizer to pull out the first 9 fields
        wxString string( str );
        wxStringTokenizer tkz( string, _T(",*") );

        wxString token;
        token = tkz.GetNextToken();         // !$CDDS

        if( str.Mid( 3, 3 ).IsSameAs( _T("DSC") ) ) {
            token = tkz.GetNextToken(); // format specifier (02-area,12-distress,16-allships,20-individual,...)
            token.ToLong( &dsc_fmt );

            token = tkz.GetNextToken();       // address i.e. mmsi*10 for received msg, or area spec
            token.ToDouble( &dsc_addr );
            dsc_mmsi = 0 - (int) ( dsc_addr / 10 ); // as per NMEA 0183 3.01

            token = tkz.GetNextToken();         // category
            token = tkz.GetNextToken();         // nature of distress or telecommand1
            token = tkz.GetNextToken();         // comm type or telecommand2

            token = tkz.GetNextToken();         // position or channel/freq
            token.ToDouble( &dsc_tmp );

            token = tkz.GetNextToken();         // time or tel. no.
            token = tkz.GetNextToken();         // mmsi of ship in distress
            token = tkz.GetNextToken();         // nature of distress
            token = tkz.GetNextToken();         // acknowledgement
            token = tkz.GetNextToken();         // expansion indicator

            dsc_quadrant = (int) dsc_tmp / 1000000000.0;

            dsc_lat = (int) ( dsc_tmp / 100000.0 );
            dsc_lon = dsc_tmp - dsc_lat * 100000.0;
            dsc_lat = dsc_lat - dsc_quadrant * 10000;
            dsc_degs = (int) ( dsc_lat / 100.0 );
            dsc_mins = dsc_lat - dsc_degs * 100.0;
            dsc_lat = dsc_degs + dsc_mins / 60.0;

            dsc_degs = (int) ( dsc_lon / 100.0 );
            dsc_mins = dsc_lon - dsc_degs * 100.0;
            dsc_lon = dsc_degs + dsc_mins / 60.0;
            switch( dsc_quadrant ) {
                case 0: break;                                             // NE
                case 1: dsc_lon = -dsc_lon; break;                         // NW
                case 2: dsc_lat = -dsc_lat; break;                         // SE
                case 3: dsc_lon = -dsc_lon; dsc_lat = -dsc_lat; break;     // SW
                default: break;
            }
            if( dsc_fmt != 02 ) mmsi = (int) dsc_mmsi;
        } else if( str.Mid( 3, 3 ).IsSameAs( _T("DSE") ) ) {

            token = tkz.GetNextToken();         // total number of sentences
            token = tkz.GetNextToken();         // sentence number
            token = tkz.GetNextToken();         // query/rely flag
            token = tkz.GetNextToken();         // vessel MMSI
            token.ToDouble( &dse_addr );
            dse_mmsi = 0 - (int) ( dse_addr / 10 ); // as per NMEA 0183 3.01

            token = tkz.GetNextToken();         // code field
            token = tkz.GetNextToken();         // data field - position - 2*4 digits latlon .mins
            token.ToDouble( &dse_tmp );
            dse_lat = (int) ( dse_tmp / 10000.0 );
            dse_lon = (int) ( dse_tmp - dse_lat * 10000.0 );
            dse_lat = dse_lat / 600000.0;
            dse_lon = dse_lon / 600000.0;

            mmsi = (int) dse_mmsi;
        }
    } else if( str.Mid( 1, 5 ).IsSameAs( _T("FRPOS") ) ) {
        // parse a GpsGate Position message            $FRPOS,.....

        //  Use a tokenizer to pull out the first 9 fields
        wxString string( str );
        wxStringTokenizer tkz( string, _T(",*") );

        wxString token;
        token = tkz.GetNextToken();         // !$FRPOS

        token = tkz.GetNextToken();            //    latitude DDMM.MMMM
        token.ToDouble( &gpsg_lat );
        gpsg_degs = (int) ( gpsg_lat / 100.0 );
        gpsg_mins = gpsg_lat - gpsg_degs * 100.0;
        gpsg_lat = gpsg_degs + gpsg_mins / 60.0;

        token = tkz.GetNextToken();            //  hemisphere N or S
        if( token.Mid( 1, 1 ).Contains( _T("Ss") ) ) gpsg_lat = 0 - gpsg_lat;

        token = tkz.GetNextToken();            // longitude DDDMM.MMMM
        token.ToDouble( &gpsg_lon );
        gpsg_degs = (int) ( gpsg_lon / 100.0 );
        gpsg_mins = gpsg_lon - gpsg_degs * 100.0;
        gpsg_lon = gpsg_degs + gpsg_mins / 60.0;

        token = tkz.GetNextToken();            // hemisphere E or W
        if( token.Mid( 1, 1 ).Contains( _T("Ww") ) ) gpsg_lon = 0 - gpsg_lon;

        token = tkz.GetNextToken();            //    altitude AA.a
        //    token.toDouble(&gpsg_alt);

        token = tkz.GetNextToken();            //  speed over ground SSS.SS knots
        token.ToDouble( &gpsg_sog );

        token = tkz.GetNextToken();            //  heading over ground HHH.hh degrees
        token.ToDouble( &gpsg_cog );

        token = tkz.GetNextToken();            // date DDMMYY

        token = tkz.GetNextToken();            // time UTC hhmmss.dd
        token.ToDouble( &gpsg_utc_time );
        gpsg_utc_hour = (int) ( gpsg_utc_time / 10000.0 );
        gpsg_utc_min = (int) ( gpsg_utc_time / 100.0 ) - gpsg_utc_hour * 100;
        gpsg_utc_sec = (int) gpsg_utc_time - gpsg_utc_hour * 10000 - gpsg_utc_min * 100;

        // now comes the name, followed by in * and NMEA checksum

        token = tkz.GetNextToken();
        int i, len, hash = 0;
        len = wxMin(wxStrlen(token),20);
        strncpy( gpsg_name_str, token.mb_str(), len );
        gpsg_name_str[len] = 0;
        for( i = 0; i < len; i++ ) {
            hash = hash * 10;
            hash += (int) ( token[i] );
            while( hash >= 100000 )
                hash = hash / 100000;
        }
        gpsg_mmsi = 199000000 + hash;  // 199 is INMARSAT-A MID, should not occur ever in AIS stream
        mmsi = gpsg_mmsi;
    } else if( !str.Mid( 3, 2 ).IsSameAs( _T("VD") ) ) {
        return AIS_NMEAVDX_BAD;
    }

    //  OK, looks like the sentence is OK

    //  Use a tokenizer to pull out the first 4 fields
    wxString string( str );
    wxStringTokenizer tkz( string, _T(",") );

    wxString token;
    token = tkz.GetNextToken();         // !xxVDx

    token = tkz.GetNextToken();
    nsentences = atoi( token.mb_str() );

    token = tkz.GetNextToken();
    isentence = atoi( token.mb_str() );

    token = tkz.GetNextToken();
    long lsequence_id = 0;
    token.ToLong( &lsequence_id );

    token = tkz.GetNextToken();
    long lchannel;
    token.ToLong( &lchannel );

    //  Now, some decisions

    string_to_parse.Clear();

    //  Simple case first
    //  First and only part of a one-part sentence
    if( ( 1 == nsentences ) && ( 1 == isentence ) ) {
        string_to_parse = tkz.GetNextToken();         // the encapsulated data
    }

    else if( nsentences > 1 ) {
        if( 1 == isentence ) {
            sentence_accumulator = tkz.GetNextToken();         // the encapsulated data
        }

        else {
            sentence_accumulator += tkz.GetNextToken();
        }

        if( isentence == nsentences ) {
            string_to_parse = sentence_accumulator;
        }
    }

    if( mmsi
            || ( !string_to_parse.IsEmpty() && ( string_to_parse.Len() < AIS_MAX_MESSAGE_LEN ) ) ) {

        //  Create the bit accessible string
        AIS_Bitstring strbit( string_to_parse.mb_str() );

        //  Extract the MMSI
        if( !mmsi ) mmsi = strbit.GetInt( 9, 30 );
        long mmsi_long = mmsi;

        AIS_Target_Data *pTargetData;
        AIS_Target_Data *pStaleTarget = NULL;
        bool bnewtarget = false;

        //  Search the current AISTargetList for an MMSI match
        AIS_Target_Hash::iterator it = AISTargetList->find( mmsi );
        if( it == AISTargetList->end() )                  // not found
                {
            pTargetData = new AIS_Target_Data;
            bnewtarget = true;
            m_n_targets++;
        } else {
            pTargetData = ( *AISTargetList )[mmsi];          // find current entry
            pStaleTarget = pTargetData;                   // save a pointer to stale data
        }

        //  Grab the stale targets's last report time
        int last_report_ticks;

        wxDateTime now = wxDateTime::Now();
        now.MakeGMT();

        if( pStaleTarget ) last_report_ticks = pStaleTarget->PositionReportTicks;
        else
            last_report_ticks = now.GetTicks();

        // Delete the stale AIS Target selectable point if not a CDDSE
        if( pStaleTarget && !dse_mmsi ) pSelectAIS->DeleteSelectablePoint( (void *) mmsi_long,
                SELTYPE_AISTARGET );

        bool bhad_name = false;
        if( pStaleTarget ) bhad_name = pStaleTarget->b_nameValid;

        bool bdecode_result = false; // for CDDSE assume target is there
        if( dse_mmsi ) bdecode_result = true;

        if( dse_mmsi && !pTargetData->b_nameValid && pTargetData->b_positionOnceValid
                && ( ( now.GetTicks() - pTargetData->PositionReportTicks ) ) < 20 ) { // ignore stray CDDSE sentences
            pTargetData->Lat = pTargetData->Lat
                    + ( ( pTargetData->Lat ) >= 0 ? dse_lat : -dse_lat );
            pTargetData->Lon = pTargetData->Lon
                    + ( ( pTargetData->Lon ) >= 0 ? dse_lon : -dse_lon );
            pTargetData->b_nameValid = true;
        } else if( dsc_mmsi ) {
            pTargetData->PositionReportTicks = now.GetTicks();
            pTargetData->StaticReportTicks = now.GetTicks();

            pTargetData->MMSI = mmsi;
            pTargetData->NavStatus = 0; // underway
            pTargetData->Lat = dsc_lat;
            pTargetData->Lon = dsc_lon;
            pTargetData->b_positionOnceValid = true; // need to cheat here, since nothing would be shown
            pTargetData->COG = 0;
            pTargetData->SOG = 0;
            pTargetData->ShipType = dsc_fmt; // DSC report
            pTargetData->Class = AIS_DSC;
            if( dsc_fmt == 12 ) strncpy( pTargetData->ShipName, "DISTRESS            ", 21 );
            else
                strncpy( pTargetData->ShipName, "POSITION REPORT     ", 21 );
            pTargetData->b_nameValid = false; // continue cheating, because position maybe incomplete
            pTargetData->b_active = true;
            pTargetData->b_lost = false;

            bdecode_result = true;
        } else if( gpsg_mmsi ) {
            pTargetData->PositionReportTicks = now.GetTicks();
            pTargetData->StaticReportTicks = now.GetTicks();
            pTargetData->m_utc_hour = gpsg_utc_hour;
            pTargetData->m_utc_min = gpsg_utc_min;
            pTargetData->m_utc_sec = gpsg_utc_sec;
            pTargetData->MMSI = gpsg_mmsi;
            pTargetData->NavStatus = 0; // underway
            pTargetData->Lat = gpsg_lat;
            pTargetData->Lon = gpsg_lon;
            pTargetData->b_positionOnceValid = true;
            pTargetData->COG = gpsg_cog;
            pTargetData->SOG = gpsg_sog;
            pTargetData->ShipType = 52; // buddy
            pTargetData->Class = AIS_GPSG_BUDDY;
            strncpy( pTargetData->ShipName, gpsg_name_str, strlen( gpsg_name_str ) + 1 );
            pTargetData->b_nameValid = true;
            pTargetData->b_active = true;
            pTargetData->b_lost = false;

            bdecode_result = true;
        } else
            bdecode_result = Parse_VDXBitstring( &strbit, pTargetData );       // Parse the new data

        //  pTargetData is valid, either new or existing. Continue processing

        m_pLatestTargetData = pTargetData;

        if( str.Mid( 3, 3 ).IsSameAs( _T("VDO") ) ) pTargetData->b_OwnShip = true;

        if( ( bdecode_result ) && ( pTargetData->b_nameValid ) && ( pStaleTarget ) ) if( !bhad_name ) n_newname++;

        //  If the message was decoded correctly
        //  Update the AIS Target information
        if( bdecode_result ) {
            ( *AISTargetList )[mmsi] = pTargetData;            // update the hash table entry

            if( !pTargetData->area_notices.empty() ) {
                AIS_Target_Hash::iterator it = AIS_AreaNotice_Sources->find( mmsi );
                if( it == AIS_AreaNotice_Sources->end() )
                    ( *AIS_AreaNotice_Sources ) [mmsi] = pTargetData;
            }

            //     Update the most recent report period
            if( !dse_mmsi ) pTargetData->RecentPeriod = pTargetData->PositionReportTicks
                    - last_report_ticks;

            //  If this is not an ownship message, update the AIS Target in the Selectable list, and update the CPA info
            if( !pTargetData->b_OwnShip ) {
                if( pTargetData->b_positionOnceValid ) {
                    SelectItem *pSel = pSelectAIS->AddSelectablePoint( pTargetData->Lat,
                            pTargetData->Lon, (void *) mmsi_long, SELTYPE_AISTARGET );
                    pSel->SetUserData( mmsi );
                }

                //    Calculate CPA info for this target immediately
                UpdateOneCPA( pTargetData );

                //    Update this target's track
                if( g_bAISShowTracks ) UpdateOneTrack( pTargetData );
            }
        } else {
//             printf("Unrecognised AIS message ID: %d\n", pTargetData->MID);
            if( bnewtarget ) {
                delete pTargetData;                           // this target is not going to be used
                m_n_targets--;
            }
        }

        ret = AIS_NoError;

    } else
        ret = AIS_Partial;                // accumulating parts of a multi-sentence message
    n_msgs++;
#ifdef AIS_DEBUG
    if((n_msgs % 10000) == 0)
    printf("n_msgs %10d m_n_targets: %6d  n_msg1: %10d  n_msg5+24: %10d  n_new5: %10d \n", n_msgs, n_targets, n_msg1, n_msg5 + n_msg24, n_newname);
#endif

    return ret;
}

//----------------------------------------------------------------------------
//      Parse a NMEA VDM/VDO Bitstring
//----------------------------------------------------------------------------
bool AIS_Decoder::Parse_VDXBitstring( AIS_Bitstring *bstr, AIS_Target_Data *ptd )
{
    bool parse_result = false;
    bool b_posn_report = false;

    wxDateTime now = wxDateTime::Now();
    now.MakeGMT( true );                    // no DST
    if( now.IsDST() ) now.Subtract( wxTimeSpan( 1, 0, 0, 0 ) );
    int message_ID = bstr->GetInt( 1, 6 );        // Parse on message ID
    ptd->MID = message_ID;
    ptd->MMSI = bstr->GetInt( 9, 30 );           // MMSI is always in the same spot in the bitstream

    switch( message_ID ){
        case 1:                                 // Position Report
        case 2:
        case 3: {
            n_msg1++;

            ptd->NavStatus = bstr->GetInt( 39, 4 );
            ptd->SOG = 0.1 * ( bstr->GetInt( 51, 10 ) );

            int lon = bstr->GetInt( 62, 28 );
            if( lon & 0x08000000 )                    // negative?
            lon |= 0xf0000000;
            double lon_tentative = lon / 600000.;

            int lat = bstr->GetInt( 90, 27 );
            if( lat & 0x04000000 )                    // negative?
            lat |= 0xf8000000;
            double lat_tentative = lat / 600000.;

            if( ( lon_tentative <= 180. ) && ( lat_tentative <= 90. ) ) // Ship does not report Lat or Lon "unavailable"
                    {
                ptd->Lon = lon_tentative;
                ptd->Lat = lat_tentative;
                ptd->b_positionDoubtful = false;
                ptd->b_positionOnceValid = true;          // Got the position at least once
                ptd->PositionReportTicks = now.GetTicks();
            } else
                ptd->b_positionDoubtful = true;

            //    decode balance of message....
            ptd->COG = 0.1 * ( bstr->GetInt( 117, 12 ) );
            ptd->HDG = 1.0 * ( bstr->GetInt( 129, 9 ) );

            ptd->ROTAIS = bstr->GetInt( 43, 8 );
            double rot_dir = 1.0;

            if( ptd->ROTAIS == 128 ) ptd->ROTAIS = -128;              // not available codes as -128
            else if( ( ptd->ROTAIS & 0x80 ) == 0x80 ) {
                ptd->ROTAIS = ptd->ROTAIS - 256;       // convert to twos complement
                rot_dir = -1.0;
            }

            ptd->ROTIND = wxRound( rot_dir * pow( ( ( (double) ptd->ROTAIS ) / 4.733 ), 2 ) ); // Convert to indicated ROT

            ptd->m_utc_sec = bstr->GetInt( 138, 6 );

            if( ( 1 == message_ID ) || ( 2 == message_ID ) )      // decode SOTDMA per 7.6.7.2.2
                    {
                ptd->SyncState = bstr->GetInt( 151, 2 );
                ptd->SlotTO = bstr->GetInt( 153, 2 );
                if( ( ptd->SlotTO == 1 ) && ( ptd->SyncState == 0 ) ) // UTCDirect follows
                        {
                    ptd->m_utc_hour = bstr->GetInt( 155, 5 );

                    ptd->m_utc_min = bstr->GetInt( 160, 7 );

                    if( ( ptd->m_utc_hour < 24 ) && ( ptd->m_utc_min < 60 )
                            && ( ptd->m_utc_sec < 60 ) ) {
                        wxDateTime rx_time( ptd->m_utc_hour, ptd->m_utc_min, ptd->m_utc_sec );
                        rx_ticks = rx_time.GetTicks();
                        if( !b_firstrx ) {
                            first_rx_ticks = rx_ticks;
                            b_firstrx = true;
                        }
                    }
                }
            }

            //    Capture Euro Inland special passing arrangement signal ("stbd-stbd")
            ptd->blue_paddle = bstr->GetInt( 144, 2 );
            ptd->b_blue_paddle = ( ptd->blue_paddle == 2 );             // paddle is set

            ptd->Class = AIS_CLASS_A;

            //    Check for SART and friends by looking at first two digits of MMSI
            int mmsi_start = ptd->MMSI / 10000000;

            if( mmsi_start == 97 ) {
                ptd->Class = AIS_SART;
                ptd->StaticReportTicks = now.GetTicks(); // won't get a static report, so fake it here

                //    On receipt of Msg 3, force any existing SART target out of acknowledge mode
                //    by adjusting its ack_time to yesterday
                //    This will cause any previously "Acknowledged" SART to re-alert.
                ptd->m_ack_time = wxDateTime::Now() - wxTimeSpan::Day();
            }

            parse_result = true;                // so far so good
            b_posn_report = true;

            break;
        }

        case 18: {
            ptd->NavStatus = UNDEFINED;         // Class B targets have no status.  Enforce this...
            
            ptd->SOG = 0.1 * ( bstr->GetInt( 47, 10 ) );

            int lon = bstr->GetInt( 58, 28 );
            if( lon & 0x08000000 )                    // negative?
            lon |= 0xf0000000;
            double lon_tentative = lon / 600000.;

            int lat = bstr->GetInt( 86, 27 );
            if( lat & 0x04000000 )                    // negative?
            lat |= 0xf8000000;
            double lat_tentative = lat / 600000.;

            if( ( lon_tentative <= 180. ) && ( lat_tentative <= 90. ) ) // Ship does not report Lat or Lon "unavailable"
                    {
                ptd->Lon = lon_tentative;
                ptd->Lat = lat_tentative;
                ptd->b_positionDoubtful = false;
                ptd->b_positionOnceValid = true;          // Got the position at least once
                ptd->PositionReportTicks = now.GetTicks();
            } else
                ptd->b_positionDoubtful = true;

            ptd->COG = 0.1 * ( bstr->GetInt( 113, 12 ) );
            ptd->HDG = 1.0 * ( bstr->GetInt( 125, 9 ) );

            ptd->m_utc_sec = bstr->GetInt( 134, 6 );

            ptd->Class = AIS_CLASS_B;

            parse_result = true;                // so far so good
            b_posn_report = true;

            break;
        }

        case 5: {
            n_msg5++;
            ptd->Class = AIS_CLASS_A;

//          Get the AIS Version indicator
//          0 = station compliant with Recommendation ITU-R M.1371-1
//          1 = station compliant with Recommendation ITU-R M.1371-3
//          2-3 = station compliant with future editions
            int AIS_version_indicator = bstr->GetInt( 39, 2 );
            if( AIS_version_indicator < 2 ) {
                ptd->IMO = bstr->GetInt( 41, 30 );

                bstr->GetStr( 71, 42, &ptd->CallSign[0], 7 );
                bstr->GetStr( 113, 120, &ptd->ShipName[0], 20 );
                ptd->b_nameValid = true;

                ptd->ShipType = (unsigned char) bstr->GetInt( 233, 8 );

                ptd->DimA = bstr->GetInt( 241, 9 );
                ptd->DimB = bstr->GetInt( 250, 9 );
                ptd->DimC = bstr->GetInt( 259, 6 );
                ptd->DimD = bstr->GetInt( 265, 6 );

                ptd->ETA_Mo = bstr->GetInt( 275, 4 );
                ptd->ETA_Day = bstr->GetInt( 279, 5 );
                ptd->ETA_Hr = bstr->GetInt( 284, 5 );
                ptd->ETA_Min = bstr->GetInt( 289, 6 );

                ptd->Draft = (double) ( bstr->GetInt( 295, 8 ) ) / 10.0;

                bstr->GetStr( 303, 120, &ptd->Destination[0], 20 );

                ptd->StaticReportTicks = now.GetTicks();

                parse_result = true;
            }

            break;
        }

        case 24: {
            int part_number = bstr->GetInt( 39, 2 );
            if( 0 == part_number ) {
                bstr->GetStr( 41, 120, &ptd->ShipName[0], 20 );
                ptd->b_nameValid = true;
                parse_result = true;
                n_msg24++;
            } else if( 1 == part_number ) {
                ptd->ShipType = (unsigned char) bstr->GetInt( 41, 8 );
                bstr->GetStr( 91, 42, &ptd->CallSign[0], 7 );

                ptd->DimA = bstr->GetInt( 133, 9 );
                ptd->DimB = bstr->GetInt( 142, 9 );
                ptd->DimC = bstr->GetInt( 151, 6 );
                ptd->DimD = bstr->GetInt( 157, 6 );
                parse_result = true;
            }
            break;
        }
        case 4:                                    // base station
        {
            ptd->Class = AIS_BASE;

            ptd->m_utc_hour = bstr->GetInt( 62, 5 );
            ptd->m_utc_min = bstr->GetInt( 67, 6 );
            ptd->m_utc_sec = bstr->GetInt( 73, 6 );
            //                              (79,  1);
            int lon = bstr->GetInt( 80, 28 );
            if( lon & 0x08000000 )                    // negative?
            lon |= 0xf0000000;
            double lon_tentative = lon / 600000.;

            int lat = bstr->GetInt( 108, 27 );
            if( lat & 0x04000000 )                    // negative?
            lat |= 0xf8000000;
            double lat_tentative = lat / 600000.;

            if( ( lon_tentative <= 180. ) && ( lat_tentative <= 90. ) ) // Ship does not report Lat or Lon "unavailable"
                    {
                ptd->Lon = lon_tentative;
                ptd->Lat = lat_tentative;
                ptd->b_positionDoubtful = false;
                ptd->b_positionOnceValid = true;          // Got the position at least once
                ptd->PositionReportTicks = now.GetTicks();
            } else
                ptd->b_positionDoubtful = true;

            ptd->COG = -1.;
            ptd->HDG = 511;
            ptd->SOG = -1.;

            parse_result = true;
            b_posn_report = true;

            break;
        }
        case 9:                   // Special Position Report (Standard SAR Aircraft Position Report)
        {
            break;
        }
        case 21:                                    // Test Message (Aid to Navigation)
        {
            ptd->ShipType = (unsigned char) bstr->GetInt( 39, 5 );
            ptd->IMO = 0;
            ptd->SOG = 0;
            ptd->HDG = 0;
            ptd->COG = 0;
            ptd->ROTAIS = -128;                 // i.e. not available
            ptd->DimA = bstr->GetInt( 220, 9 );
            ptd->DimB = bstr->GetInt( 229, 9 );
            ptd->DimC = bstr->GetInt( 238, 6 );
            ptd->DimD = bstr->GetInt( 244, 6 );
            ptd->Draft = 0;

            ptd->m_utc_sec = bstr->GetInt( 254, 6 );

            int offpos = bstr->GetInt( 260, 1 ); // off position flag
            int virt = bstr->GetInt( 270, 1 ); // virtual flag

            if( virt ) ptd->NavStatus = ATON_VIRTUAL;
            else
                ptd->NavStatus = ATON_REAL;
            if( ptd->m_utc_sec <= 59 /*&& !virt*/) {
                ptd->NavStatus += 1;
                if( offpos ) ptd->NavStatus += 1;
            }

            bstr->GetStr( 44, 120, &ptd->ShipName[0], 20 ); // short name only, extension wont fit in Ship structure

            if( bstr->GetBitCount() > 276 ) {
                int nx = ( ( bstr->GetBitCount() - 272 ) / 6 ) * 6;
                bstr->GetStr( 273, nx, &ptd->ShipNameExtension[0], 14 );
                ptd->ShipNameExtension[14] = 0;
            } else {
                ptd->ShipNameExtension[0] = 0;
            }

            ptd->b_nameValid = true;

            parse_result = true;                // so far so good

            ptd->Class = AIS_ATON;

            int lon = bstr->GetInt( 165, 28 );

            if( lon & 0x08000000 )                    // negative?
            lon |= 0xf0000000;
            double lon_tentative = lon / 600000.;

            int lat = bstr->GetInt( 193, 27 );

            if( lat & 0x04000000 )                    // negative?
            lat |= 0xf8000000;
            double lat_tentative = lat / 600000.;

            if( ( lon_tentative <= 180. ) && ( lat_tentative <= 90. ) ) // Ship does not report Lat or Lon "unavailable"
                    {
                ptd->Lon = lon_tentative;
                ptd->Lat = lat_tentative;
                ptd->b_positionDoubtful = false;
                ptd->b_positionOnceValid = true;          // Got the position at least once
                ptd->PositionReportTicks = now.GetTicks();
            } else
                ptd->b_positionDoubtful = true;

            b_posn_report = true;
            break;
        }
        case 8:                                    // Binary Broadcast
        {
            int dac = bstr->GetInt( 41, 10 );
            int fi = bstr->GetInt( 51, 6 );
            if( dac == 200 )                  // European inland
                    {
                if( fi == 10 )              // "Inland ship static and voyage related data"
                        {
                    ptd->b_isEuroInland = true;

                    bstr->GetStr( 57, 48, &ptd->Euro_VIN[0], 8 );
                    ptd->Euro_Length = ( (double) bstr->GetInt( 105, 13 ) ) / 10.0;
                    ptd->Euro_Beam = ( (double) bstr->GetInt( 118, 10 ) ) / 10.0;
                    ptd->UN_shiptype = bstr->GetInt( 128, 14 );
                    ptd->Euro_Draft = ( (double) bstr->GetInt( 145, 11 ) ) / 100.0;
                    parse_result = true;
                }
            }
            if( dac == 1 )                     // IMO
                    {
                if( fi == 22 )                 // Area Notice
                        {
                    if( bstr->GetBitCount() >= 111 ) {
                        Ais8_001_22 an;
                        an.link_id = bstr->GetInt( 57, 10 );
                        an.notice_type = bstr->GetInt( 67, 7 );
                        an.month = bstr->GetInt( 74, 4 );
                        an.day = bstr->GetInt( 78, 5 );
                        an.hour = bstr->GetInt( 83, 5 );
                        an.minute = bstr->GetInt( 88, 6 );
                        an.duration_minutes = bstr->GetInt( 94, 18 );

                        wxDateTime now = wxDateTime::Now();
                        now.MakeGMT();

                        an.start_time.Set( an.day, wxDateTime::Month( an.month - 1 ), now.GetYear(),
                                an.hour, an.minute );

                        // msg is not supposed to be transmitted more than a day before it comes into effect,
                        // so a start_time less than a day or two away might indicate a month rollover
                        if( an.start_time > now + wxTimeSpan::Hours( 48 ) ) an.start_time.Set(
                                an.day, wxDateTime::Month( an.month - 1 ), now.GetYear() - 1,
                                an.hour, an.minute );

                        an.expiry_time = an.start_time + wxTimeSpan::Minutes( an.duration_minutes );

                        // msg is not supposed to be transmitted beyond expiration, so taking into account a
                        // fudge factor for clock issues, assume an expiry date in the past indicates incorrect year
                        if( an.expiry_time < now - wxTimeSpan::Hours( 24 ) ) {
                            an.start_time.Set( an.day, wxDateTime::Month( an.month - 1 ),
                                    now.GetYear() + 1, an.hour, an.minute );
                            an.expiry_time = an.start_time
                                    + wxTimeSpan::Minutes( an.duration_minutes );
                        }

                        int subarea_count = ( bstr->GetBitCount() - 111 ) / 87;
                        for( int i = 0; i < subarea_count; ++i ) {
                            int base = 111 + i * 87;
                            Ais8_001_22_SubArea sa;
                            sa.shape = bstr->GetInt( base + 1, 3 );
                            int scale_factor = 1;
                            if( sa.shape == AIS8_001_22_SHAPE_TEXT ) {
                                char t[15];
                                t[14] = 0;
                                bstr->GetStr( base + 4, 84, t, 14 );
                                sa.text = wxString( t, wxConvUTF8 );
                            } else {
                                int scale_multipliers[4] = { 1, 10, 100, 1000 };
                                scale_factor = scale_multipliers[bstr->GetInt( base + 4, 2 )];
                                switch( sa.shape ){
                                    case AIS8_001_22_SHAPE_CIRCLE:
                                    case AIS8_001_22_SHAPE_SECTOR:
                                        sa.radius_m = bstr->GetInt( base + 58, 12 ) * scale_factor;
                                    case AIS8_001_22_SHAPE_RECT:
                                        sa.longitude = bstr->GetInt( base + 6, 25, true ) / 60000.0;
                                        sa.latitude = bstr->GetInt( base + 31, 24, true ) / 60000.0;
                                        break;
                                    case AIS8_001_22_SHAPE_POLYLINE:
                                    case AIS8_001_22_SHAPE_POLYGON:
                                        for( int i = 0; i < 4; ++i ) {
                                            sa.angles[i] = bstr->GetInt( base + 6 + i * 20, 10 )
                                                    * 0.5;
                                            sa.dists_m[i] = bstr->GetInt( base + 16 + i * 20, 10 )
                                                    * scale_factor;
                                        }
                                }
                                if( sa.shape == AIS8_001_22_SHAPE_RECT ) {
                                    sa.e_dim_m = bstr->GetInt( base + 58, 8 ) * scale_factor;
                                    sa.n_dim_m = bstr->GetInt( base + 66, 8 ) * scale_factor;
                                    sa.orient_deg = bstr->GetInt( base + 74, 9 );
                                }
                                if( sa.shape == AIS8_001_22_SHAPE_SECTOR ) {
                                    sa.left_bound_deg = bstr->GetInt( 70, 9 );
                                    sa.right_bound_deg = bstr->GetInt( 79, 9 );
                                }
                            }
                            an.sub_areas.push_back( sa );
                        }
                        ptd->area_notices[an.link_id] = an;
                        parse_result = true;
                    }
                }
            }
            break;
        }
        case 14:                                    // Safety Related Broadcast
        {
            //  Always capture the MSG_14 text
            char msg_14_text[968];
            if( bstr->GetBitCount() > 40 ) {
                int nx = ( ( bstr->GetBitCount() - 40 ) / 6 ) * 6;
                int nd = bstr->GetStr( 41, nx, msg_14_text, 968 );
                nd = wxMax(0, nd);
                nd = wxMin(nd, 967);
                msg_14_text[nd] = 0;
                ptd->MSG_14_text = wxString( msg_14_text, wxConvUTF8 );
            }
            parse_result = true;                // so far so good

            break;
        }

        case 6:                                    // Addressed Binary Message
        {
            break;
        }
        case 7:                                    // Binary Ack
        {
            break;
        }
        default: {
            break;
        }

    }

    if( b_posn_report ) ptd->b_lost = false;

    if( true == parse_result ) {
        //      Revalidate the target under some conditions
        if( !ptd->b_active && !ptd->b_positionDoubtful && b_posn_report ) ptd->b_active = true;
    }

    return parse_result;
}

bool AIS_Decoder::NMEACheckSumOK( const wxString& str_in )
{

    unsigned char checksum_value = 0;
    int sentence_hex_sum;

    char str_ascii[AIS_MAX_MESSAGE_LEN + 1];
    strncpy( str_ascii, str_in.mb_str(), AIS_MAX_MESSAGE_LEN );
    str_ascii[AIS_MAX_MESSAGE_LEN] = '\0';

    int string_length = strlen( str_ascii );

    int payload_length = 0;
    while( ( payload_length < string_length ) && ( str_ascii[payload_length] != '*' ) ) // look for '*'
        payload_length++;

    if( payload_length == string_length ) return false; // '*' not found at all, no checksum

    int index = 1; // Skip over the $ at the begining of the sentence

    while( index < payload_length ) {
        checksum_value ^= str_ascii[index];
        index++;
    }

    if( string_length > 4 ) {
        char scanstr[3];
        scanstr[0] = str_ascii[payload_length + 1];
        scanstr[1] = str_ascii[payload_length + 2];
        scanstr[2] = 0;
        sscanf( scanstr, "%2x", &sentence_hex_sum );

        if( sentence_hex_sum == checksum_value ) return true;
    }

    return false;
}

void AIS_Decoder::UpdateAllCPA( void )
{
    //    Iterate thru all the targets
    AIS_Target_Hash::iterator it;
    AIS_Target_Hash *current_targets = GetTargetList();

    for( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it ) {
        AIS_Target_Data *td = it->second;

        if( NULL != td ) UpdateOneCPA( td );
    }
}

void AIS_Decoder::UpdateAllTracks( void )
{
    //    Iterate thru all the targets
    AIS_Target_Hash::iterator it;
    AIS_Target_Hash *current_targets = GetTargetList();

    for( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it ) {
        AIS_Target_Data *td = it->second;

        if( NULL != td ) UpdateOneTrack( td );
    }
}

void AIS_Decoder::UpdateOneTrack( AIS_Target_Data *ptarget )
{
    if( !ptarget->b_positionOnceValid ) return;

    //    Add the newest point
    AISTargetTrackPoint *ptrackpoint = new AISTargetTrackPoint;
    ptrackpoint->m_lat = ptarget->Lat;
    ptrackpoint->m_lon = ptarget->Lon;
    ptrackpoint->m_time = wxDateTime::Now().GetTicks();

    ptarget->m_ptrack->Append( ptrackpoint );

    //    Walk the list, removing any track points that are older than the stipulated time

    time_t test_time = wxDateTime::Now().GetTicks() - (time_t) ( g_AISShowTracks_Mins * 60 );

    wxAISTargetTrackListNode *node = ptarget->m_ptrack->GetFirst();
    while( node ) {
        AISTargetTrackPoint *ptrack_point = node->GetData();

        if( ptrack_point->m_time < test_time ) {
            if( ptarget->m_ptrack->DeleteObject( ptrack_point ) ) {
                delete ptrack_point;
                node = ptarget->m_ptrack->GetFirst();                // restart the list
            }
        } else
            node = node->GetNext();
    }
}

void AIS_Decoder::UpdateAllAlarms( void )
{
    m_bGeneralAlert = false;                // no alerts yet

    //    Iterate thru all the targets
    AIS_Target_Hash::iterator it;
    AIS_Target_Hash *current_targets = GetTargetList();

    for( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it ) {
        AIS_Target_Data *td = it->second;

        if( NULL != td ) {
            //  Maintain General Alert
            if( !m_bGeneralAlert ) {
                //    Quick check on basic condition
                if( ( td->CPA < g_CPAWarn_NM ) && ( td->TCPA > 0 ) && ( td->Class != AIS_ATON ) )
                    m_bGeneralAlert = true;

                //    Some options can suppress general alerts
                if( g_bAIS_CPA_Alert_Suppress_Moored && ( td->SOG <= g_ShowMoored_Kts ) )
                    m_bGeneralAlert = false;

                //    Skip distant targets if requested
                if( ( g_bCPAMax ) && ( td->Range_NM > g_CPAMax_NM ) )
                    m_bGeneralAlert = false;

                //    Skip if TCPA is too long
                if( ( g_bTCPA_Max ) && ( td->TCPA > g_TCPA_Max ) )
                    m_bGeneralAlert = false;

                //  SART targets always alert
                if( td->Class == AIS_SART )
                    m_bGeneralAlert = true;

            }

            ais_alarm_type this_alarm = AIS_NO_ALARM;
            if( g_bCPAWarn && td->b_active && td->b_positionOnceValid ) {
                //      Skip anchored/moored(interpreted as low speed) targets if requested
                if( ( !g_bShowMoored ) && ( td->SOG <= g_ShowMoored_Kts ) )        // dsr
                        {
                    td->n_alarm_state = AIS_NO_ALARM;
                    continue;
                }

                //    No Alert on moored(interpreted as low speed) targets if so requested
                if( g_bAIS_CPA_Alert_Suppress_Moored && ( td->SOG <= g_ShowMoored_Kts ) )     // dsr
                        {

                    td->n_alarm_state = AIS_NO_ALARM;
                    continue;
                }

                //    Skip distant targets if requested
                if( g_bCPAMax ) {
                    if( td->Range_NM > g_CPAMax_NM ) {
                        td->n_alarm_state = AIS_NO_ALARM;
                        continue;
                    }
                }

                if( ( td->CPA < g_CPAWarn_NM ) && ( td->TCPA > 0 ) && ( td->Class != AIS_ATON ) ) {
                    if( g_bTCPA_Max ) {
                        if( td->TCPA < g_TCPA_Max ) this_alarm = AIS_ALARM_SET;
                    } else
                        this_alarm = AIS_ALARM_SET;
                }
            }

            //  SART targets always alert
            if( td->Class == AIS_SART ) this_alarm = AIS_ALARM_SET;

            //    Maintain the timer for in_ack flag
            //  SART targets always maintain ack timeout

            if( g_bAIS_ACK_Timeout || ( td->Class == AIS_SART ) ) {
                if( td->b_in_ack_timeout ) {
                    wxTimeSpan delta = wxDateTime::Now() - td->m_ack_time;
                    if( delta.GetMinutes() > g_AckTimeout_Mins ) td->b_in_ack_timeout = false;
                }
            } else
                td->b_in_ack_timeout = false;

            td->n_alarm_state = this_alarm;

        }
    }
}

void AIS_Decoder::UpdateOneCPA( AIS_Target_Data *ptarget )
{
    ptarget->Range_NM = -1.;            // Defaults
    ptarget->Brg = -1.;

    if( !ptarget->b_positionOnceValid || !bGPSValid ) {
        ptarget->bCPA_Valid = false;
        return;
    }

    //    Compute the current Range/Brg to the target
    double brg, dist;
    DistanceBearingMercator( ptarget->Lat, ptarget->Lon, gLat, gLon, &brg, &dist );
    ptarget->Range_NM = dist;
    ptarget->Brg = brg;

    if( dist <= 1e-5 ) ptarget->Brg = -1.0;             // Brg is undefined if Range == 0.

    //    There can be no collision between ownship and itself....
    //    This can happen if AIVDO messages are received, and there is another source of ownship position, like NMEA GLL
    //    The two positions are always temporally out of sync, and one will always be exactly in front of the other one.
    if( ptarget->b_OwnShip ) {
        ptarget->CPA = 100;
        ptarget->TCPA = -100;
        ptarget->bCPA_Valid = false;
        return;
    }

    double cpa_calc_ownship_cog = gCog;
    double cpa_calc_target_cog = ptarget->COG;

//    Ownship is not reporting valid SOG, so no way to calculate CPA
    if( wxIsNaN(gSog) || ( gSog > 102.2 ) ) {
        ptarget->bCPA_Valid = false;
        return;
    }

//    Ownship is maybe anchored and not reporting COG
    if( wxIsNaN(gCog) || gCog == 360.0 ) {
        if( gSog < .01 ) cpa_calc_ownship_cog = 0.;          // substitute value
                                                             // for the case where SOG ~= 0, and COG is unknown.
        else {
            ptarget->bCPA_Valid = false;
            return;
        }
    }

//    Target is maybe anchored and not reporting COG
    if( ptarget->COG == 360.0 ) {
        if( ptarget->SOG > 102.2 ) {
            ptarget->bCPA_Valid = false;
            return;
        } else if( ptarget->SOG < .01 ) cpa_calc_target_cog = 0.;           // substitute value
                                                                            // for the case where SOG ~= 0, and COG is unknown.
        else {
            ptarget->bCPA_Valid = false;
            return;
        }
    }

    //    Express the SOGs as meters per hour
    double v0 = gSog * 1852.;
    double v1 = ptarget->SOG * 1852.;

    if( ( v0 < 1e-6 ) && ( v1 < 1e-6 ) ) {
        ptarget->TCPA = 0.;
        ptarget->CPA = 0.;

        ptarget->bCPA_Valid = false;
    } else {
        //    Calculate the TCPA first

        //    Working on a Reduced Lat/Lon orthogonal plotting sheet....
        //    Get easting/northing to target,  in meters

        double east1 = ( ptarget->Lon - gLon ) * 60 * 1852;
        double north1 = ( ptarget->Lat - gLat ) * 60 * 1852;

        double east = east1 * ( cos( gLat * PI / 180 ) );
        ;
        double north = north1;

        //    Convert COGs trigonometry to standard unit circle
        double cosa = cos( ( 90. - cpa_calc_ownship_cog ) * PI / 180. );
        double sina = sin( ( 90. - cpa_calc_ownship_cog ) * PI / 180. );
        double cosb = cos( ( 90. - cpa_calc_target_cog ) * PI / 180. );
        double sinb = sin( ( 90. - cpa_calc_target_cog ) * PI / 180. );

        //    These will be useful
        double fc = ( v0 * cosa ) - ( v1 * cosb );
        double fs = ( v0 * sina ) - ( v1 * sinb );

        double d = ( fc * fc ) + ( fs * fs );
        double tcpa;

        // the tracks are almost parallel
        if( fabs( d ) < 1e-6 ) tcpa = 0.;
        else
            //    Here is the equation for t, which will be in hours
            tcpa = ( ( fc * east ) + ( fs * north ) ) / d;

        //    Convert to minutes
        ptarget->TCPA = tcpa * 60.;

        //    Calculate CPA
        //    Using TCPA, predict ownship and target positions

        double OwnshipLatCPA, OwnshipLonCPA, TargetLatCPA, TargetLonCPA;

        ll_gc_ll( gLat, gLon, cpa_calc_ownship_cog, gSog * tcpa, &OwnshipLatCPA, &OwnshipLonCPA );
        ll_gc_ll( ptarget->Lat, ptarget->Lon, cpa_calc_target_cog, ptarget->SOG * tcpa,
                &TargetLatCPA, &TargetLonCPA );

        //   And compute the distance
        ptarget->CPA = DistGreatCircle( OwnshipLatCPA, OwnshipLonCPA, TargetLatCPA, TargetLonCPA );

        ptarget->bCPA_Valid = true;

        if( ptarget->TCPA < 0 ) ptarget->bCPA_Valid = false;
    }
}

void AIS_Decoder::OnTimerAISAudio( wxTimerEvent& event )
{
    if( g_bAIS_CPA_Alert_Audio && m_bAIS_Audio_Alert_On ) {
        m_AIS_Sound.Create( g_sAIS_Alert_Sound_File );
        if( m_AIS_Sound.IsOk() ) m_AIS_Sound.Play();
    }
    m_AIS_Audio_Alert_Timer.Start( TIMER_AIS_AUDIO_MSEC, wxTIMER_CONTINUOUS );
}

void AIS_Decoder::OnTimerAIS( wxTimerEvent& event )
{
    TimerAIS.Stop();

    //    Scrub the target hash list
    //    removing any targets older than stipulated age

    wxDateTime now = wxDateTime::Now();
    now.MakeGMT();

    AIS_Target_Hash::iterator it;
    AIS_Target_Hash *current_targets = GetTargetList();

    it = ( *current_targets ).begin();
    while( it != ( *current_targets ).end() ) {
        bool b_new_it = false;

        AIS_Target_Data *td = it->second;

        if( NULL == td )                        // This should never happen, but I saw it once....
                {
            current_targets->erase( it );
            break;                          // leave the loop
        }

        int target_posn_age = now.GetTicks() - td->PositionReportTicks;
        int target_static_age = now.GetTicks() - td->StaticReportTicks;

        //      Mark lost targets if specified
        if( g_bMarkLost ) {
            if( ( target_posn_age > g_MarkLost_Mins * 60 ) && ( td->Class != AIS_GPSG_BUDDY ) ) td->b_active =
                    false;
        }

        //      Remove lost targets if specified
        double removelost_Mins = fmax(g_RemoveLost_Mins,g_MarkLost_Mins);

        if( td->Class == AIS_SART ) removelost_Mins = 18.0;

        if( g_bRemoveLost ) {
            if( ( target_posn_age > removelost_Mins * 60 ) && ( td->Class != AIS_GPSG_BUDDY ) ) {
                //      So mark the target as lost, with unknown position, and make it not selectable
                td->b_lost = true;
                td->b_positionOnceValid = false;
                td->COG = 360.0;
                td->SOG = 103.0;
                td->HDG = 511.0;
                td->ROTAIS = -128;

                long mmsi_long = td->MMSI;
                pSelectAIS->DeleteSelectablePoint( (void *) mmsi_long, SELTYPE_AISTARGET );

                //      If we have not seen a static report in 3 times the removal spec,
                //      then remove the target from all lists.
                if( target_static_age > removelost_Mins * 60 * 3 ) {
                    current_targets->erase( it );
                    delete td;

                    //      Reset the iterator on item erase.
                    it = ( *current_targets ).begin();
                    b_new_it = true;
                }
            }
        }

        if( !b_new_it ) ++it;
    }

    UpdateAllCPA();
    UpdateAllAlarms();

    //    Update the general suppression flag
    m_bSuppressed = false;
    if( g_bAIS_CPA_Alert_Suppress_Moored || !g_bShowMoored ) m_bSuppressed = true;

    m_bAIS_Audio_Alert_On = false;            // default, may be set on

    //    Process any Alarms

    //    If the AIS Alert Dialog is not currently shown....

    //    Show the Alert dialog
    //    Which of multiple targets?
    //    Give priority to SART targets, and among them the shortest range
    //    Otherwise,
    //    search the list for any targets with CPA alarms, selecting the target with shortest TCPA

    if( NULL == g_pais_alert_dialog_active ) {
        double tcpa_min = 1e6;             // really long
        double sart_range = 1e6;
        AIS_Target_Data *palarm_target_cpa = NULL;
        AIS_Target_Data *palarm_target_sart = NULL;

        for( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it ) {
            AIS_Target_Data *td = it->second;
            if( td ) {
                if( td->Class != AIS_SART ) {

                    if( g_bAIS_CPA_Alert && td->b_active ) {
                        if( ( AIS_ALARM_SET == td->n_alarm_state ) && !td->b_in_ack_timeout ) {
                            if( td->TCPA < tcpa_min ) {
                                tcpa_min = td->TCPA;
                                palarm_target_cpa = td;
                            }
                        }
                    }
                } else {
                    if( td->b_active ) {
                        if( ( AIS_ALARM_SET == td->n_alarm_state ) && !td->b_in_ack_timeout ) {
                            if( td->Range_NM < sart_range ) {
                                tcpa_min = sart_range;
                                palarm_target_sart = td;
                            }
                        }
                    }
                }
            }
        }

        AIS_Target_Data *palarm_target = palarm_target_cpa;

        if( palarm_target_sart ) palarm_target = palarm_target_sart;

        if( palarm_target ) {
            //    Show the alert

            bool b_jumpto = palarm_target->Class == AIS_SART;

            AISTargetAlertDialog *pAISAlertDialog = new AISTargetAlertDialog();
            pAISAlertDialog->Create( palarm_target->MMSI, m_parent_frame, this, b_jumpto, -1,
                    _("AIS Alert"), wxPoint( g_ais_alert_dialog_x, g_ais_alert_dialog_y ),
                    wxSize( g_ais_alert_dialog_sx, g_ais_alert_dialog_sy ) );

            g_pais_alert_dialog_active = pAISAlertDialog;
            pAISAlertDialog->Show();                     // Show modeless, so it stays on the screen

            //    Audio alert if requested
            m_bAIS_Audio_Alert_On = true;             // always on when alert is first shown
        }
    }

    //    The AIS Alert dialog is already shown.  If the  dialog MMSI number is still alerted, update the dialog
    //    otherwise, destroy the dialog
    else {
        AIS_Target_Data *palert_target = Get_Target_Data_From_MMSI(
                g_pais_alert_dialog_active->Get_Dialog_MMSI() );

        if( palert_target ) {
            if( ( ( AIS_ALARM_SET == palert_target->n_alarm_state )
                    && !palert_target->b_in_ack_timeout )
                    || ( palert_target->Class == AIS_SART ) ) {
                g_pais_alert_dialog_active->UpdateText();
            } else {
                g_pais_alert_dialog_active->Close();
                m_bAIS_Audio_Alert_On = false;
            }

            if( true == palert_target->b_suppress_audio ) m_bAIS_Audio_Alert_On = false;
            else
                m_bAIS_Audio_Alert_On = true;
        } else {                                               // this should not happen, however...
            g_pais_alert_dialog_active->Close();
            m_bAIS_Audio_Alert_On = false;
        }

    }

    //    At this point, the audio flag is set

    //    Honor the global flag
    if( !g_bAIS_CPA_Alert_Audio ) m_bAIS_Audio_Alert_On = false;

    if( m_bAIS_Audio_Alert_On ) {
        if( !m_AIS_Audio_Alert_Timer.IsRunning() ) {
            m_AIS_Audio_Alert_Timer.SetOwner( this, TIMER_AISAUDIO );
            m_AIS_Audio_Alert_Timer.Start( TIMER_AIS_AUDIO_MSEC );

            m_AIS_Sound.Create( g_sAIS_Alert_Sound_File );
            if( m_AIS_Sound.IsOk() ) m_AIS_Sound.Play();
        }
    } else
        m_AIS_Audio_Alert_Timer.Stop();

    TimerAIS.Start( TIMER_AIS_MSEC, wxTIMER_CONTINUOUS );
}

AIS_Target_Data *AIS_Decoder::Get_Target_Data_From_MMSI( int mmsi )
{
    if( AISTargetList->find( mmsi ) == AISTargetList->end() )     // if entry does not exist....
    return NULL;
    else
        return ( *AISTargetList )[mmsi];          // find current entry
}

//---------------------------------------------------------------------------------------
//          AISTargetAlertDialog Implementation
//---------------------------------------------------------------------------------------
IMPLEMENT_CLASS ( AISTargetAlertDialog, wxDialog )

BEGIN_EVENT_TABLE ( AISTargetAlertDialog, wxDialog )
    EVT_CLOSE(AISTargetAlertDialog::OnClose)
    EVT_BUTTON( ID_ACKNOWLEDGE, AISTargetAlertDialog::OnIdAckClick )
    EVT_BUTTON( ID_SILENCE, AISTargetAlertDialog::OnIdSilenceClick )
    EVT_BUTTON( ID_JUMPTO, AISTargetAlertDialog::OnIdJumptoClick )
    EVT_MOVE( AISTargetAlertDialog::OnMove )
    EVT_SIZE( AISTargetAlertDialog::OnSize )
END_EVENT_TABLE()

AISTargetAlertDialog::AISTargetAlertDialog()
{
    Init();
}

AISTargetAlertDialog::~AISTargetAlertDialog()
{
}

void AISTargetAlertDialog::Init()
{
    m_target_mmsi = 0;
    m_pparent = NULL;
}

bool AISTargetAlertDialog::Create( int target_mmsi, wxWindow *parent, AIS_Decoder *pdecoder,
        bool b_jumpto, wxWindowID id, const wxString& caption, const wxPoint& pos,
        const wxSize& size, long style )
{
    //    As a display optimization....
    //    if current color scheme is other than DAY,
    //    Then create the dialog ..WITHOUT.. borders and title bar.
    //    This way, any window decorations set by external themes, etc
    //    will not detract from night-vision

    long wstyle = wxDEFAULT_FRAME_STYLE;
    if( ( global_color_scheme != GLOBAL_COLOR_SCHEME_DAY )
            && ( global_color_scheme != GLOBAL_COLOR_SCHEME_RGB ) ) wstyle |= ( wxNO_BORDER );

    m_bjumpto = b_jumpto;

    wxSize size_min = size;
    size_min.IncTo( wxSize( 500, 600 ) );
    if( !wxDialog::Create( parent, id, caption, pos, size_min, wstyle ) ) return false;

    m_target_mmsi = target_mmsi;
    m_pparent = parent;
    m_pdecoder = pdecoder;

    wxFont *dFont = pFontMgr->GetFont( _("AISTargetAlert"), 12 );
    int font_size = wxMax(8, dFont->GetPointSize());
    wxString face = dFont->GetFaceName();
#ifdef __WXGTK__
    face = _T("Monospace");
#endif
    wxFont *fp_font = wxTheFontList->FindOrCreateFont( font_size, wxFONTFAMILY_MODERN,
            wxFONTSTYLE_NORMAL, dFont->GetWeight(), false, face );

    SetFont( *fp_font );

    CreateControls();
    if( !g_bopengl && CanSetTransparent() ) SetTransparent( 192 );
    DimeControl( this );

    return true;
}

void AISTargetAlertDialog::CreateControls()
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( topSizer );

    m_pAlertTextCtl = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
            wxHW_SCROLLBAR_AUTO );
    m_pAlertTextCtl->SetBorders( 5 );

    topSizer->Add( m_pAlertTextCtl, 1, wxALIGN_CENTER_HORIZONTAL | wxALL | wxEXPAND, 5 );

    // A horizontal box sizer to contain Ack
    wxBoxSizer* AckBox = new wxBoxSizer( wxHORIZONTAL );
    topSizer->Add( AckBox, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    // The Silence button
    wxButton* silence = new wxButton( this, ID_SILENCE, _( "&Silence Alert" ), wxDefaultPosition,
            wxDefaultSize, 0 );
    AckBox->Add( silence, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    // The Ack button
    wxButton* ack = new wxButton( this, ID_ACKNOWLEDGE, _( "&Acknowledge" ), wxDefaultPosition,
            wxDefaultSize, 0 );
    AckBox->Add( ack, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    if( m_bjumpto ) {
        wxButton* jumpto = new wxButton( this, ID_JUMPTO, _( "&Jump To" ), wxDefaultPosition,
                wxDefaultSize, 0 );
        AckBox->Add( jumpto, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
    }

    UpdateText();
}

bool AISTargetAlertDialog::GetAlertText()
{
    //    Search the parent AIS_Decoder's target list for specified mmsi
    if( m_pdecoder ) {
        AIS_Target_Data *td_found = m_pdecoder->Get_Target_Data_From_MMSI( Get_Dialog_MMSI() );

        if( td_found ) {
            m_alert_text = td_found->BuildQueryResult();
            return true;
        } else
            return false;
    } else
        return false;
}

void AISTargetAlertDialog::UpdateText()
{
    if( GetAlertText() ) {
        wxColor bg = GetBackgroundColour();
        m_pAlertTextCtl->SetBackgroundColour( bg );

        wxFont *dFont = pFontMgr->GetFont( _("AISTargetQuery"), 12 );
        wxString face = dFont->GetFaceName();
        int sizes[7];
        for( int i = -2; i < 5; i++ ) {
            sizes[i + 2] = dFont->GetPointSize() + i + ( i > 0 ? i : 0 );
        }

        wxString html;
        html.Printf( _T("<html><body bgcolor=#%02x%02x%02x><center>"), bg.Red(), bg.Blue(),
                bg.Green() );

        html << m_alert_text;
        html << _T("</center></font></body></html>");

        m_pAlertTextCtl->SetFonts( face, face, sizes );
        m_pAlertTextCtl->SetPage( html );

        // Try to create a min size that works across font sizes.
        wxSize sz;
        if( !IsShown() ) {
            sz = m_pAlertTextCtl->GetVirtualSize();
            sz.x = 300;
            m_pAlertTextCtl->SetSize( sz );
        }
        m_pAlertTextCtl->Layout();
        wxSize ir( m_pAlertTextCtl->GetInternalRepresentation()->GetWidth(),
                m_pAlertTextCtl->GetInternalRepresentation()->GetHeight() );
        sz.x = wxMax( m_pAlertTextCtl->GetSize().x, ir.x );
        sz.y = wxMax( m_pAlertTextCtl->GetSize().y, ir.y );
        m_pAlertTextCtl->SetMinSize( sz );
        Fit();
        sz -= wxSize( 200, 200 );
        m_pAlertTextCtl->SetMinSize( sz );
    }

    DimeControl( this );
    if( !g_bopengl && CanSetTransparent() ) SetTransparent( 192 );
}

void AISTargetAlertDialog::OnClose( wxCloseEvent& event )
{
    //    Acknowledge any existing Alert, and dismiss the dialog
    if( m_pdecoder ) {
        AIS_Target_Data *td = m_pdecoder->Get_Target_Data_From_MMSI( Get_Dialog_MMSI() );
        if( td ) {
            if( AIS_ALARM_SET == td->n_alarm_state ) {
                td->m_ack_time = wxDateTime::Now();
                td->b_in_ack_timeout = true;
            }
        }
    }

    Destroy();
    g_pais_alert_dialog_active = NULL;
}

void AISTargetAlertDialog::OnIdAckClick( wxCommandEvent& event )
{
    //    Acknowledge the Alert, and dismiss the dialog
    if( m_pdecoder ) {
        AIS_Target_Data *td = m_pdecoder->Get_Target_Data_From_MMSI( Get_Dialog_MMSI() );
        if( td ) {
            if( AIS_ALARM_SET == td->n_alarm_state ) {
                td->m_ack_time = wxDateTime::Now();
                td->b_in_ack_timeout = true;
            }
        }
    }
    Destroy();
    g_pais_alert_dialog_active = NULL;
}

void AISTargetAlertDialog::OnIdSilenceClick( wxCommandEvent& event )
{
    //    Set the suppress audio flag
    if( m_pdecoder ) {
        AIS_Target_Data *td = m_pdecoder->Get_Target_Data_From_MMSI( Get_Dialog_MMSI() );
        if( td ) td->b_suppress_audio = true;
    }
}

void AISTargetAlertDialog::OnIdJumptoClick( wxCommandEvent& event )
{
    if( m_pdecoder ) {
        AIS_Target_Data *td = m_pdecoder->Get_Target_Data_From_MMSI( Get_Dialog_MMSI() );
        if( td ) gFrame->JumpToPosition( td->Lat, td->Lon, cc1->GetVPScale() );
    }
}

void AISTargetAlertDialog::OnMove( wxMoveEvent& event )
{
    //    Record the dialog position
    wxPoint p = event.GetPosition();
    g_ais_alert_dialog_x = p.x;
    g_ais_alert_dialog_y = p.y;

    event.Skip();
}

void AISTargetAlertDialog::OnSize( wxSizeEvent& event )
{
    //    Record the dialog size
    wxSize p = event.GetSize();
    g_ais_alert_dialog_sx = p.x;
    g_ais_alert_dialog_sy = p.y;

    event.Skip();
}

class AISTargetListDialog;
//---------------------------------------------------------------------------------------
//          OCPNListCtrl Definition
//---------------------------------------------------------------------------------------
class OCPNListCtrl: public wxListCtrl {
public:
    OCPNListCtrl( AISTargetListDialog* parent, wxWindowID id, const wxPoint& pos,
            const wxSize& size, long style );
    ~OCPNListCtrl();

    wxString OnGetItemText( long item, long column ) const;
    int OnGetItemColumnImage( long item, long column ) const;

    wxString GetTargetColumnData( AIS_Target_Data *pAISTarget, long column ) const;

    AISTargetListDialog *m_parent;

};

OCPNListCtrl::OCPNListCtrl( AISTargetListDialog* parent, wxWindowID id, const wxPoint& pos,
        const wxSize& size, long style ) :
        wxListCtrl( parent, id, pos, size, style )
{
    m_parent = parent;
}

OCPNListCtrl::~OCPNListCtrl()
{
    g_AisTargetList_column_spec.Clear();
    for( int i = 0; i < tlSOG + 1; i++ ) {
        wxListItem item;
        GetColumn( i, item );
        wxString sitem;
        sitem.Printf( _T("%d;"), item.m_width );
        g_AisTargetList_column_spec += sitem;
    }
}

wxString OCPNListCtrl::OnGetItemText( long item, long column ) const
{
    wxString ret;

    if( m_parent->m_pListCtrlAISTargets ) {
        AIS_Target_Data *pAISTarget = m_parent->GetpTarget( item );
        if( pAISTarget ) ret = GetTargetColumnData( pAISTarget, column );
    }

    return ret;
}

int OCPNListCtrl::OnGetItemColumnImage( long item, long column ) const
{
    return -1;
}

wxString OCPNListCtrl::GetTargetColumnData( AIS_Target_Data *pAISTarget, long column ) const
{
    wxString ret;

    if( pAISTarget ) {
        switch( column ){
            case tlNAME:
                if( ( pAISTarget->Class == AIS_BASE ) || ( pAISTarget->Class == AIS_SART ) ) ret =
                        _("-");
                else {
                    wxString uret = trimAISField( pAISTarget->ShipName );
                    if( uret == _T("Unknown") ) ret = wxGetTranslation( uret );
                    else
                        ret = uret;

                    if( strlen( pAISTarget->ShipNameExtension ) ) ret.Append(
                            wxString( pAISTarget->ShipNameExtension, wxConvUTF8 ) );
                }
                break;

            case tlCALL:
                ret = trimAISField( pAISTarget->CallSign );
                break;

            case tlMMSI:
                if( pAISTarget->Class != AIS_GPSG_BUDDY ) ret.Printf( _T("%09d"),
                        abs( pAISTarget->MMSI ) );
                else
                    ret.Printf( _T("   nil   ") );
                break;

            case tlCLASS:
                ret = wxGetTranslation( pAISTarget->Get_class_string( true ) );
                break;

            case tlTYPE:
                if( ( pAISTarget->Class == AIS_BASE ) || ( pAISTarget->Class == AIS_SART ) ) ret =
                        _("-");
                else
                    ret = wxGetTranslation( pAISTarget->Get_vessel_type_string( false ) );
                break;

            case tlNAVSTATUS: {
                if( pAISTarget->Class == AIS_SART ) {
                    if( pAISTarget->NavStatus == RESERVED_14 ) ret = _("Active");
                    else if( pAISTarget->NavStatus == UNDEFINED ) ret = _("Testing");
                } else {

                    if( ( pAISTarget->NavStatus <= 20 ) && ( pAISTarget->NavStatus >= 0 ) ) ret =
                            wxGetTranslation( ais_status[pAISTarget->NavStatus] );
                    else
                        ret = _("-");
                }

                if( ( pAISTarget->Class == AIS_ATON ) || ( pAISTarget->Class == AIS_BASE )
                        || ( pAISTarget->Class == AIS_CLASS_B ) ) ret = _("-");
                break;
            }

            case tlBRG: {
                if( pAISTarget->b_positionOnceValid && bGPSValid && ( pAISTarget->Brg >= 0. )
                        && ( fabs( pAISTarget->Lat ) < 85. ) ) {
                    int brg = (int) wxRound( pAISTarget->Brg );
                    if( pAISTarget->Brg > 359.5 ) brg = 0;

                    ret.Printf( _T("%03d"), brg );
                } else
                    ret = _("-");
                break;
            }

            case tlCOG: {
                if( ( pAISTarget->COG >= 360.0 ) || ( pAISTarget->Class == AIS_ATON )
                        || ( pAISTarget->Class == AIS_BASE ) ) ret = _("-");
                else {
                    int crs = wxRound( pAISTarget->COG );
                    if( crs == 360 ) ret.Printf( _T("  000") );
                    else
                        ret.Printf( _T("  %03d"), crs );
                }
                break;
            }

            case tlSOG: {
                if( ( pAISTarget->SOG > 100. ) || ( pAISTarget->Class == AIS_ATON )
                        || ( pAISTarget->Class == AIS_BASE ) ) ret = _("-");
                else
                    ret.Printf( _T("%5.1f"), pAISTarget->SOG );
                break;
            }
            case tlCPA:
            {
                if( ( !pAISTarget->bCPA_Valid ) || ( pAISTarget->Class == AIS_ATON )
                        || ( pAISTarget->Class == AIS_BASE ) ) ret = _("-");
                else
                    ret.Printf( _T("%5.2f"), pAISTarget->CPA );
                break;
            }
            case tlTCPA:
            {
                if( ( !pAISTarget->bCPA_Valid ) || ( pAISTarget->Class == AIS_ATON )
                        || ( pAISTarget->Class == AIS_BASE ) ) ret = _("-");
                else
                    ret.Printf( _T("%5.0f"), pAISTarget->TCPA );
                break;
            }
            case tlRNG: {
                if( pAISTarget->b_positionOnceValid && bGPSValid && ( pAISTarget->Range_NM >= 0. ) ) ret.Printf(
                        _T("%5.2f"), pAISTarget->Range_NM );
                else
                    ret = _("-");
                break;
            }

            default:
                break;
        }

    }

    return ret;
}

#include <wx/arrimpl.cpp>

//---------------------------------------------------------------------------------------
//          AISTargetListDialog Implementation
//---------------------------------------------------------------------------------------

int ItemCompare( AIS_Target_Data *pAISTarget1, AIS_Target_Data *pAISTarget2 )
{
    wxString s1, s2;
    double n1 = 0.;
    double n2 = 0.;
    bool b_cmptype_num = false;

    //    Don't sort if target list count is too large
    if( g_AisTargetList_count > 1000 ) return 0;

    AIS_Target_Data *t1 = pAISTarget1;
    AIS_Target_Data *t2 = pAISTarget2;

    if( t1->Class == AIS_SART ) return -1;
    if( t2->Class == AIS_SART ) return 1;

    switch( g_AisTargetList_sortColumn ){
        case tlNAME:
            s1 = trimAISField( t1->ShipName );
            if( ( t1->Class == AIS_BASE ) || ( t1->Class == AIS_SART ) ) s1 = _T("-");

            s2 = trimAISField( t2->ShipName );
            if( ( t2->Class == AIS_BASE ) || ( t2->Class == AIS_SART ) ) s2 = _T("-");
            break;

        case tlCALL:
            s1 = trimAISField( t1->CallSign );
            s2 = trimAISField( t2->CallSign );
            break;

        case tlMMSI:
            n1 = t1->MMSI;
            n2 = t2->MMSI;
            b_cmptype_num = true;
            break;

        case tlCLASS:
            s1 = t1->Get_class_string( true );
            s2 = t2->Get_class_string( true );
            break;

        case tlTYPE:
            s1 = t1->Get_vessel_type_string( false );
            if( ( t1->Class == AIS_BASE ) || ( t1->Class == AIS_SART ) ) s1 = _T("-");

            s2 = t2->Get_vessel_type_string( false );
            if( ( t1->Class == AIS_BASE ) || ( t1->Class == AIS_SART ) ) s2 = _T("-");
            break;

        case tlNAVSTATUS: {
            if( ( t1->NavStatus <= 15 ) && ( t1->NavStatus >= 0 ) ) {
                if( t1->Class == AIS_SART ) {
                    if( t1->NavStatus == RESERVED_14 ) s1 = _("Active");
                    else if( t1->NavStatus == UNDEFINED ) s1 = _("Testing");
                } else
                    s1 = ais_status[t1->NavStatus];
            } else
                s1 = _("-");

            if( ( t1->Class == AIS_ATON ) || ( t1->Class == AIS_BASE )
                || ( t1->Class == AIS_CLASS_B ) ) s1 = _T("-");
 
            if( ( t2->NavStatus <= 15 ) && ( t2->NavStatus >= 0 ) ) {
                if( t2->Class == AIS_SART ) {
                    if( t2->NavStatus == RESERVED_14 ) s2 = _("Active");
                    else if( t2->NavStatus == UNDEFINED ) s2 = _("Testing");
                } else
                    s2 = ais_status[t2->NavStatus];
            } else
                s2 = _("-");

            if( ( t2->Class == AIS_ATON ) || ( t2->Class == AIS_BASE )
                || ( t2->Class == AIS_CLASS_B ) ) s2 = _T("-");

            break;
        }

        case tlBRG: {
            int brg1 = wxRound( t1->Brg );
            if( brg1 == 360 ) n1 = 0.;
            else
                n1 = brg1;

            int brg2 = wxRound( t2->Brg );
            if( brg2 == 360 ) n2 = 0.;
            else
                n2 = brg2;

            b_cmptype_num = true;
            break;
        }

        case tlCOG: {
            if( ( t1->COG >= 360.0 ) || ( t1->Class == AIS_ATON ) || ( t1->Class == AIS_BASE ) ) n1 =
                    -1.0;
            else {
                int crs = wxRound( t1->COG );
                if( crs == 360 ) n1 = 0.;
                else
                    n1 = crs;
            }

            if( ( t2->COG >= 360.0 ) || ( t2->Class == AIS_ATON ) || ( t2->Class == AIS_BASE ) ) n2 =
                    -1.0;
            else {
                int crs = wxRound( t2->COG );
                if( crs == 360 ) n2 = 0.;
                else
                    n2 = crs;
            }

            b_cmptype_num = true;
            break;
        }

        case tlSOG: {
            if( ( t1->SOG > 100. ) || ( t1->Class == AIS_ATON ) || ( t1->Class == AIS_BASE ) ) n1 =
                    -1.0;
            else
                n1 = t1->SOG;

            if( ( t2->SOG > 100. ) || ( t2->Class == AIS_ATON ) || ( t2->Class == AIS_BASE ) ) n2 =
                    -1.0;
            else
                n2 = t2->SOG;

            b_cmptype_num = true;
            break;
        }
        case tlCPA:
        {
            if( ( !t1->bCPA_Valid ) || ( t1->Class == AIS_ATON ) || ( t1->Class == AIS_BASE ) ) n1 =
                    99999.0;
            else
                n1 = t1->CPA;

            if( ( !t2->bCPA_Valid ) || ( t2->Class == AIS_ATON ) || ( t2->Class == AIS_BASE ) ) n2 =
                    99999.0;
            else
                n2 = t2->CPA;

            b_cmptype_num = true;
            break;
        }
        case tlTCPA:
        {
            if( ( !t1->bCPA_Valid ) || ( t1->Class == AIS_ATON ) || ( t1->Class == AIS_BASE ) ) n1 =
                    99999.0;
            else
                n1 = t1->TCPA;

            if( ( !t2->bCPA_Valid ) || ( t2->Class == AIS_ATON ) || ( t2->Class == AIS_BASE ) ) n2 =
                    99999.0;
            else
                n2 = t2->TCPA;

            b_cmptype_num = true;
            break;
        }
        case tlRNG: {
            n1 = t1->Range_NM;
            n2 = t2->Range_NM;
            b_cmptype_num = true;
            break;
        }

        default:
            break;
    }

    if( !b_cmptype_num ) {
        if( g_bAisTargetList_sortReverse ) return s2.Cmp( s1 );
        return s1.Cmp( s2 );
    } else {
        //    If numeric sort values are equal, secondary sort is on Range_NM
        if( g_bAisTargetList_sortReverse ) {
            if( n2 > n1 ) return 1;
            else if( n2 < n1 ) return -1;
            else
                return ( t1->Range_NM > t2->Range_NM ); //0;
        } else {
            if( n2 > n1 ) return -1;
            else if( n2 < n1 ) return 1;
            else
                return ( t1->Range_NM > t2->Range_NM ); //0;
        }
    }
}

AIS_Decoder *g_p_sort_decoder;

int ArrayItemCompareMMSI( int MMSI1, int MMSI2 )
{

    if( g_p_sort_decoder ) {
        AIS_Target_Data *pAISTarget1 = g_p_sort_decoder->Get_Target_Data_From_MMSI( MMSI1 );
        AIS_Target_Data *pAISTarget2 = g_p_sort_decoder->Get_Target_Data_From_MMSI( MMSI2 );

        if( pAISTarget1 && pAISTarget2 ) return ItemCompare( pAISTarget1, pAISTarget2 );
        else
            return 0;
    } else
        return 0;
}
IMPLEMENT_CLASS ( AISTargetListDialog, wxPanel )

BEGIN_EVENT_TABLE(AISTargetListDialog, wxPanel)
    EVT_CLOSE(AISTargetListDialog::OnClose)
END_EVENT_TABLE()

AISTargetListDialog::AISTargetListDialog( wxWindow *parent, wxAuiManager *auimgr,
        AIS_Decoder *pdecoder ) :
        wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 780, 250 ), wxBORDER_NONE )
{
    m_pparent = parent;
    m_pAuiManager = auimgr;
    m_pdecoder = pdecoder;

    g_p_sort_decoder = pdecoder;
    m_pMMSI_array = new ArrayOfMMSI( ArrayItemCompareMMSI );

    wxBoxSizer* topSizer = new wxBoxSizer( wxHORIZONTAL );
    SetSizer( topSizer );

    //  Parse the global column width string as read from config file
    wxStringTokenizer tkz( g_AisTargetList_column_spec, _T(";") );
    wxString s_width = tkz.GetNextToken();
    int width;
    long lwidth;

    m_pListCtrlAISTargets = new OCPNListCtrl( this, ID_AIS_TARGET_LIST, wxDefaultPosition,
            wxDefaultSize,
            wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxBORDER_SUNKEN
                    | wxLC_VIRTUAL );
    wxImageList *imglist = new wxImageList( 16, 16, true, 2 );

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    imglist->Add( style->GetIcon( _T("sort_asc") ) );
    imglist->Add( style->GetIcon( _T("sort_desc") ) );

    m_pListCtrlAISTargets->AssignImageList( imglist, wxIMAGE_LIST_SMALL );
    m_pListCtrlAISTargets->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED,
            wxListEventHandler( AISTargetListDialog::OnTargetSelected ), NULL, this );
    m_pListCtrlAISTargets->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,
            wxListEventHandler( AISTargetListDialog::OnTargetSelected ), NULL, this );
    m_pListCtrlAISTargets->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
            wxListEventHandler( AISTargetListDialog::OnTargetDefaultAction ), NULL, this );
    m_pListCtrlAISTargets->Connect( wxEVT_COMMAND_LIST_COL_CLICK,
            wxListEventHandler( AISTargetListDialog::OnTargetListColumnClicked ), NULL, this );

    width = 105;
    if( s_width.ToLong( &lwidth ) ) {
        width = wxMax(20, lwidth);
        width = wxMin(width, 250);
    }
    m_pListCtrlAISTargets->InsertColumn( tlNAME, _("Name"), wxLIST_FORMAT_LEFT, width );
    s_width = tkz.GetNextToken();

    width = 55;
    if( s_width.ToLong( &lwidth ) ) {
        width = wxMax(20, lwidth);
        width = wxMin(width, 250);
    }
    m_pListCtrlAISTargets->InsertColumn( tlCALL, _("Call"), wxLIST_FORMAT_LEFT, width );
    s_width = tkz.GetNextToken();

    width = 80;
    if( s_width.ToLong( &lwidth ) ) {
        width = wxMax(20, lwidth);
        width = wxMin(width, 250);
    }
    m_pListCtrlAISTargets->InsertColumn( tlMMSI, _("MMSI"), wxLIST_FORMAT_LEFT, width );
    s_width = tkz.GetNextToken();

    width = 55;
    if( s_width.ToLong( &lwidth ) ) {
        width = wxMax(20, lwidth);
        width = wxMin(width, 250);
    }
    m_pListCtrlAISTargets->InsertColumn( tlCLASS, _("Class"), wxLIST_FORMAT_CENTER, width );
    s_width = tkz.GetNextToken();

    width = 80;
    if( s_width.ToLong( &lwidth ) ) {
        width = wxMax(20, lwidth);
        width = wxMin(width, 250);
    }
    m_pListCtrlAISTargets->InsertColumn( tlTYPE, _("Type"), wxLIST_FORMAT_LEFT, width );
    s_width = tkz.GetNextToken();

    width = 90;
    if( s_width.ToLong( &lwidth ) ) {
        width = wxMax(20, lwidth);
        width = wxMin(width, 250);
    }
    m_pListCtrlAISTargets->InsertColumn( tlNAVSTATUS, _("Nav Status"), wxLIST_FORMAT_LEFT, width );
    s_width = tkz.GetNextToken();

    width = 45;
    if( s_width.ToLong( &lwidth ) ) {
        width = wxMax(20, lwidth);
        width = wxMin(width, 250);
    }
    m_pListCtrlAISTargets->InsertColumn( tlBRG, _("Brg"), wxLIST_FORMAT_RIGHT, width );
    s_width = tkz.GetNextToken();

    width = 62;
    if( s_width.ToLong( &lwidth ) ) {
        width = wxMax(20, lwidth);
        width = wxMin(width, 250);
    }
    m_pListCtrlAISTargets->InsertColumn( tlRNG, _("Range"), wxLIST_FORMAT_RIGHT, width );
    s_width = tkz.GetNextToken();

    width = 50;
    if( s_width.ToLong( &lwidth ) ) {
        width = wxMax(20, lwidth);
        width = wxMin(width, 250);
    }
    m_pListCtrlAISTargets->InsertColumn( tlCOG, _("CoG"), wxLIST_FORMAT_RIGHT, width );
    s_width = tkz.GetNextToken();

    width = 50;
    if( s_width.ToLong( &lwidth ) ) {
        width = wxMax(20, lwidth);
        width = wxMin(width, 250);
    }
    m_pListCtrlAISTargets->InsertColumn( tlSOG, _("SoG"), wxLIST_FORMAT_RIGHT, width );

    width = 55;
    if( s_width.ToLong( &lwidth ) ) {
        width = wxMax(20, lwidth);
        width = wxMin(width, 250);
    }
    m_pListCtrlAISTargets->InsertColumn( tlCPA, _("CPA"), wxLIST_FORMAT_RIGHT, width );

    width = 65;
    if( s_width.ToLong( &lwidth ) ) {
        width = wxMax(20, lwidth);
        width = wxMin(width, 250);
    }
    m_pListCtrlAISTargets->InsertColumn( tlTCPA, _("TCPA"), wxLIST_FORMAT_RIGHT, width );
    wxListItem item;
    item.SetMask( wxLIST_MASK_IMAGE );
    item.SetImage( g_bAisTargetList_sortReverse ? 1 : 0 );
    g_AisTargetList_sortColumn = wxMax(g_AisTargetList_sortColumn, 0);
    m_pListCtrlAISTargets->SetColumn( g_AisTargetList_sortColumn, item );

    topSizer->Add( m_pListCtrlAISTargets, 1, wxEXPAND | wxALL, 0 );

    wxBoxSizer* boxSizer02 = new wxBoxSizer( wxVERTICAL );
    boxSizer02->AddSpacer( 22 );

    m_pButtonInfo = new wxButton( this, wxID_ANY, _("Target info"), wxDefaultPosition,
            wxDefaultSize, wxBU_AUTODRAW );
    m_pButtonInfo->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( AISTargetListDialog::OnTargetQuery ), NULL, this );
    boxSizer02->Add( m_pButtonInfo, 0, wxALL, 0 );
    boxSizer02->AddSpacer( 5 );

    m_pButtonJumpTo = new wxButton( this, wxID_ANY, _("Center View"), wxDefaultPosition,
            wxDefaultSize, wxBU_AUTODRAW );
    m_pButtonJumpTo->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( AISTargetListDialog::OnTargetScrollTo ), NULL, this );
    boxSizer02->Add( m_pButtonJumpTo, 0, wxALL, 0 );
    boxSizer02->AddSpacer( 10 );

    m_pStaticTextRange = new wxStaticText( this, wxID_ANY, _("Limit range: NM"), wxDefaultPosition,
            wxDefaultSize, 0 );
    boxSizer02->Add( m_pStaticTextRange, 0, wxALL, 0 );
    boxSizer02->AddSpacer( 2 );
    m_pSpinCtrlRange = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition,
            wxSize( 50, -1 ), wxSP_ARROW_KEYS, 1, 20000, g_AisTargetList_range );
    m_pSpinCtrlRange->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED,
            wxCommandEventHandler( AISTargetListDialog::OnLimitRange ), NULL, this );
    m_pSpinCtrlRange->Connect( wxEVT_COMMAND_TEXT_UPDATED,
            wxCommandEventHandler( AISTargetListDialog::OnLimitRange ), NULL, this );
    boxSizer02->Add( m_pSpinCtrlRange, 0, wxEXPAND | wxALL, 0 );
    topSizer->Add( boxSizer02, 0, wxEXPAND | wxALL, 2 );

    boxSizer02->AddSpacer( 10 );
    m_pStaticTextCount = new wxStaticText( this, wxID_ANY, _("Target Count"), wxDefaultPosition,
            wxDefaultSize, 0 );
    boxSizer02->Add( m_pStaticTextCount, 0, wxALL, 0 );

    boxSizer02->AddSpacer( 2 );
    m_pTextTargetCount = new wxTextCtrl( this, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize,
            wxTE_READONLY );
    boxSizer02->Add( m_pTextTargetCount, 0, wxALL, 0 );

    topSizer->Layout();

    //    This is silly, but seems to be required for __WXMSW__ build
    //    If not done, the SECOND invocation of AISTargetList fails to expand the list to the full wxSizer size....
    SetSize( GetSize().x, GetSize().y - 1 );

    SetColorScheme();
    UpdateButtons();

    if( m_pAuiManager ) {
        wxAuiPaneInfo pane =
                wxAuiPaneInfo().Name( _T("AISTargetList") ).Caption( _("AIS target list") ).CaptionVisible(
                        true ).DestroyOnClose( true ).Float().FloatingPosition( 50, 200 ).TopDockable(
                        false ).BottomDockable( true ).LeftDockable( false ).RightDockable( false ).Show(
                        true );
        m_pAuiManager->LoadPaneInfo( g_AisTargetList_perspective, pane );

        bool b_reset_pos = false;

#ifdef __WXMSW__
        //  Support MultiMonitor setups which an allow negative window positions.
        //  If the requested window title bar does not intersect any installed monitor,
        //  then default to simple primary monitor positioning.
        RECT frame_title_rect;
        frame_title_rect.left = pane.floating_pos.x;
        frame_title_rect.top = pane.floating_pos.y;
        frame_title_rect.right = pane.floating_pos.x + pane.floating_size.x;
        frame_title_rect.bottom = pane.floating_pos.y + 30;

        if( NULL == MonitorFromRect( &frame_title_rect, MONITOR_DEFAULTTONULL ) ) b_reset_pos =
                true;
#else

        //    Make sure drag bar (title bar) of window intersects wxClient Area of screen, with a little slop...
        wxRect window_title_rect;// conservative estimate
        window_title_rect.x = pane.floating_pos.x;
        window_title_rect.y = pane.floating_pos.y;
        window_title_rect.width = pane.floating_size.x;
        window_title_rect.height = 30;

        wxRect ClientRect = wxGetClientDisplayRect();
        ClientRect.Deflate(60, 60);// Prevent the new window from being too close to the edge
        if(!ClientRect.Intersects(window_title_rect))
        b_reset_pos = true;

#endif

        if( b_reset_pos ) pane.FloatingPosition( 50, 200 );

        //    If the list got accidentally dropped on top of the chart bar, move it away....
        if( pane.IsDocked() && ( pane.dock_row == 0 ) ) {
            pane.Float();
            pane.Row( 1 );
            pane.Position( 0 );

            g_AisTargetList_perspective = m_pAuiManager->SavePaneInfo( pane );
            pConfig->UpdateSettings();
        }

        m_pAuiManager->AddPane( this, pane );
        m_pAuiManager->Update();

        m_pAuiManager->Connect( wxEVT_AUI_PANE_CLOSE,
                wxAuiManagerEventHandler( AISTargetListDialog::OnPaneClose ), NULL, this );
    }
}

AISTargetListDialog::~AISTargetListDialog()
{
    Disconnect_decoder();
    g_pAISTargetList = NULL;
}

void AISTargetListDialog::OnClose( wxCloseEvent &event )
{
    Disconnect_decoder();
}

void AISTargetListDialog::Disconnect_decoder()
{
    m_pdecoder = NULL;
}

void AISTargetListDialog::SetColorScheme()
{
    DimeControl( this );
}

void AISTargetListDialog::OnPaneClose( wxAuiManagerEvent& event )
{
    if( event.pane->name == _T("AISTargetList") ) {
        g_AisTargetList_perspective = m_pAuiManager->SavePaneInfo( *event.pane );
        //event.Veto();
    }
    event.Skip();
}

void AISTargetListDialog::UpdateButtons()
{
    long item = -1;
    item = m_pListCtrlAISTargets->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    bool enable = ( item != -1 );

    m_pButtonInfo->Enable( enable );

    if( m_pdecoder && item != -1 ) {
        AIS_Target_Data *pAISTargetSel = m_pdecoder->Get_Target_Data_From_MMSI(
                m_pMMSI_array->Item( item ) );
        if( pAISTargetSel && ( !pAISTargetSel->b_positionOnceValid ) ) enable = false;
    }
    m_pButtonJumpTo->Enable( enable );
}

void AISTargetListDialog::OnTargetSelected( wxListEvent &event )
{
    UpdateButtons();
}

void AISTargetListDialog::DoTargetQuery( int mmsi )
{
    ShowAISTargetQueryDialog( m_pparent, mmsi );
}

/*
 ** When an item is activated in AIS TArget List then opens the AIS Target Query Dialog
 */
void AISTargetListDialog::OnTargetDefaultAction( wxListEvent& event )
{
    long mmsi_no;
    if( ( mmsi_no = event.GetData() ) ) DoTargetQuery( mmsi_no );
}

void AISTargetListDialog::OnTargetQuery( wxCommandEvent& event )
{
    long selItemID = -1;
    selItemID = m_pListCtrlAISTargets->GetNextItem( selItemID, wxLIST_NEXT_ALL,
            wxLIST_STATE_SELECTED );
    if( selItemID == -1 ) return;

    if( m_pdecoder ) {
        AIS_Target_Data *pAISTarget = m_pdecoder->Get_Target_Data_From_MMSI(
                m_pMMSI_array->Item( selItemID ) );
        if( pAISTarget ) DoTargetQuery( pAISTarget->MMSI );
    }
}

void AISTargetListDialog::OnTargetListColumnClicked( wxListEvent &event )
{
    int key = event.GetColumn();
    wxListItem item;
    item.SetMask( wxLIST_MASK_IMAGE );
    if( key == g_AisTargetList_sortColumn ) g_bAisTargetList_sortReverse =
            !g_bAisTargetList_sortReverse;
    else {
        item.SetImage( -1 );
        m_pListCtrlAISTargets->SetColumn( g_AisTargetList_sortColumn, item );
        g_bAisTargetList_sortReverse = false;
        g_AisTargetList_sortColumn = key;
    }
    item.SetImage( g_bAisTargetList_sortReverse ? 1 : 0 );
    if( g_AisTargetList_sortColumn >= 0 ) {
        m_pListCtrlAISTargets->SetColumn( g_AisTargetList_sortColumn, item );
        UpdateAISTargetList();
    }
}

void AISTargetListDialog::OnTargetScrollTo( wxCommandEvent& event )
{
    long selItemID = -1;
    selItemID = m_pListCtrlAISTargets->GetNextItem( selItemID, wxLIST_NEXT_ALL,
            wxLIST_STATE_SELECTED );
    if( selItemID == -1 ) return;

    AIS_Target_Data *pAISTarget = NULL;
    if( m_pdecoder ) pAISTarget = m_pdecoder->Get_Target_Data_From_MMSI(
            m_pMMSI_array->Item( selItemID ) );

    if( pAISTarget ) gFrame->JumpToPosition( pAISTarget->Lat, pAISTarget->Lon, cc1->GetVPScale() );
}

void AISTargetListDialog::OnLimitRange( wxCommandEvent& event )
{
    g_AisTargetList_range = m_pSpinCtrlRange->GetValue();
    UpdateAISTargetList();
}

AIS_Target_Data *AISTargetListDialog::GetpTarget( unsigned int list_item )
{
    return m_pdecoder->Get_Target_Data_From_MMSI( m_pMMSI_array->Item( list_item ) );
}

void AISTargetListDialog::UpdateAISTargetList( void )
{
    if( m_pdecoder ) {
        int sb_position = m_pListCtrlAISTargets->GetScrollPos( wxVERTICAL );

        //    Capture the MMSI of the curently selected list item
        long selItemID = -1;
        selItemID = m_pListCtrlAISTargets->GetNextItem( selItemID, wxLIST_NEXT_ALL,
                wxLIST_STATE_SELECTED );

        int selMMSI = -1;
        if( selItemID != -1 ) selMMSI = m_pMMSI_array->Item( selItemID );

        AIS_Target_Hash::iterator it;
        AIS_Target_Hash *current_targets = m_pdecoder->GetTargetList();
        wxListItem item;

        int index = 0;
        m_pMMSI_array->Clear();

        for( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it, ++index ) {
            AIS_Target_Data *pAISTarget = it->second;
            item.SetId( index );

            if( NULL != pAISTarget ) {
                if( ( pAISTarget->b_positionOnceValid )
                        && ( pAISTarget->Range_NM <= g_AisTargetList_range ) ) m_pMMSI_array->Add(
                        pAISTarget->MMSI );
                else if( !pAISTarget->b_positionOnceValid ) m_pMMSI_array->Add( pAISTarget->MMSI );
            }
        }

        m_pListCtrlAISTargets->SetItemCount( m_pMMSI_array->GetCount() );

        g_AisTargetList_count = m_pMMSI_array->GetCount();

        m_pListCtrlAISTargets->SetScrollPos( wxVERTICAL, sb_position, false );

        //    Restore selected item
        long item_sel = 0;
        if( ( selItemID != -1 ) && ( selMMSI != -1 ) ) {
            for( unsigned int i = 0; i < m_pMMSI_array->GetCount(); i++ ) {
                if( m_pMMSI_array->Item( i ) == selMMSI ) {
                    item_sel = i;
                    break;
                }
            }
        }

        if( m_pMMSI_array->GetCount() ) m_pListCtrlAISTargets->SetItemState( item_sel,
                wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED );
        else
            m_pListCtrlAISTargets->DeleteAllItems();

        wxString count;
        count.Printf( _T("%d"), m_pMMSI_array->GetCount() );
        m_pTextTargetCount->ChangeValue( count );

#ifdef __WXMSW__
        m_pListCtrlAISTargets->Refresh( false );
#endif
    }
}
