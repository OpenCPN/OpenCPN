/***************************************************************************
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
#include "Select.h"

extern  int             s_dns_test_flag;
extern  Select          *pSelectAIS;
extern  double          gLat, gLon, gSog, gCog;
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
extern ocpnStyle::StyleManager* g_StyleManager;

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(AISTargetTrackList);

wxString ais_get_status(int index)
{
	static const wxString ais_status[] = {
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

	return ais_status[index];
}

wxString ais_get_type(int index)
{
	static const wxString ais_type[] = {
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
        _("GpsGate Buddy"),              //xx        52
        _("Position Report"),            //xx        53
        _("Distress"),                   //xx        54
        _("ARPA radar target"),          //xx        55
        _("APRS Position Report")        //xx        56
	};

	return ais_type[index];
}

wxString ais_get_short_type(int index)
{
    static const wxString short_ais_type[] = {
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
        _("Distress"),             //xx        54
        _("ARPA"),                 //xx        55
        _("APRS")                  //xx        56
	};

	return short_ais_type[index];
}

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
    _("Rogue or suspicious vessel"), // 94
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

wxString trimAISField( char *data )
{
    //  Clip any unused characters (@) from data

    wxString field = wxString::From8BitData( data );
    while( field.Right( 1 ) == '@' || field.Right( 1 ) == ' ' )
        field.RemoveLast();
    return field;
}

