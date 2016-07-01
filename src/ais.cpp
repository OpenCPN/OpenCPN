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

#include "cutil.h"
#include "FontMgr.h"
#include "dychart.h"
#include "ais.h"
#include "chart1.h"
#include "navutil.h"        // for Select
#include "georef.h"
#include "pluginmanager.h"  // for PlugInManager
#include "styles.h"
#include "datastream.h"
#include "Select.h"
#include "AIS_Decoder.h"
#include "AIS_Target_Data.h"
#include "AISTargetAlertDialog.h"
#include "AISTargetQueryDialog.h"
#include "wx28compat.h"

extern  int             s_dns_test_flag;
extern  Select          *pSelectAIS;
extern  double          gLat, gLon, gSog, gCog;
extern ChartCanvas      *cc1;
extern MyFrame          *gFrame;
extern MyConfig         *pConfig;

int                      g_ais_cog_predictor_width;
extern AIS_Decoder              *g_pAIS;
extern AISTargetAlertDialog      *g_pais_alert_dialog_active;
extern AISTargetQueryDialog      *g_pais_query_dialog_active;

//    AIS Global configuration
extern bool             g_bShowAIS;
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
extern bool             g_bShowCOG;
extern double           g_ShowCOG_Mins;
extern bool             g_bShowTracks;
extern double           g_ShowTracks_Mins;
extern bool             g_bHideMoored;
extern double           g_ShowMoored_Kts;
extern bool             g_bAISShowTracks;
extern bool             g_bShowAreaNotices;
extern bool             g_bDrawAISSize;
extern bool             g_bShowAISName;
extern int              g_Show_Target_Name_Scale;

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
extern bool             g_bShowScaled;
extern bool             g_bAllowShowScaled;

int                     g_ShowScaled_Num;
int                     ImportanceSwitchPoint = 100;
int                     g_ScaledNumWeightSOG;
int                     g_ScaledNumWeightCPA;
int                     g_ScaledNumWeightTCPA;
int                     g_ScaledNumWeightRange;
int                     g_ScaledNumWeightSizeOfT;
int                     g_ScaledSizeMinimal;


extern bool             bGPSValid;
extern ArrayOfMMSIProperties   g_MMSI_Props_Array;

extern bool             g_bresponsive;
extern float            g_ChartScaleFactorExp;

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
        _("Power-driven vessel towing astern (regional use)"),
        _("Power-driven vessel pushing ahead or towing alongside (regional use)"),
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
    
    //  And remove any leading spaces to properly sort and display
    field.Trim(false);
    
    return field;
}

static wxPoint transrot( wxPoint pt, float sin_theta, float cos_theta, wxPoint offset=wxPoint(0,0) )
{
    wxPoint ret;
    float px = (float) ( pt.x * sin_theta ) + (float) ( pt.y * cos_theta );
    float py = (float) ( pt.y * sin_theta ) - (float) ( pt.x * cos_theta );
    ret.x = (int) wxRound( px );
    ret.y = (int) wxRound( py );
    ret.x += offset.x;
    ret.y += offset.y;

    return ret;
}

static void transrot_pts( int n, wxPoint *pt, float sin_theta, float cos_theta, wxPoint offset=wxPoint(0,0) )
{
    for(int i=0; i<n; i++)
        pt[i] = transrot(pt[i], sin_theta, cos_theta, offset);
}

void AISDrawAreaNotices( ocpnDC& dc )
{
    if( !g_pAIS || !g_bShowAIS || !g_bShowAreaNotices ) return;

    wxDateTime now = wxDateTime::Now();
    now.MakeGMT();

    bool b_pens_set = false;
    wxPen pen_save;
    wxBrush brush_save;
    wxColour yellow;
    wxColour green;
    wxPen pen;
    wxBrush *yellow_brush = wxTheBrushList->FindOrCreateBrush( wxColour(0,0,0), wxBRUSHSTYLE_TRANSPARENT );
    wxBrush *green_brush  = wxTheBrushList->FindOrCreateBrush( wxColour(0,0,0), wxBRUSHSTYLE_TRANSPARENT );;
    wxBrush *brush;

    AIS_Target_Hash *current_targets = g_pAIS->GetAreaNoticeSourcesList();

    float vp_scale = cc1->GetVPScale();

    for( AIS_Target_Hash::iterator target = current_targets->begin();
            target != current_targets->end(); ++target ) {
        AIS_Target_Data *target_data = target->second;
        if( !target_data->area_notices.empty() ) {
            if( !b_pens_set ) {
                pen_save = dc.GetPen();
                brush_save = dc.GetBrush();

                yellow = GetGlobalColor( _T ( "YELO1" ) );
                yellow.Set( yellow.Red(), yellow.Green(), yellow.Blue(), 64 );

                green = GetGlobalColor( _T ( "GREEN4" ) );
                green.Set( green.Red(), green.Green(), green.Blue(), 64 );

                pen.SetColour( yellow );
                pen.SetWidth( 2 );

                yellow_brush = wxTheBrushList->FindOrCreateBrush( yellow, wxBRUSHSTYLE_CROSSDIAG_HATCH );
                green_brush = wxTheBrushList->FindOrCreateBrush( green, wxBRUSHSTYLE_TRANSPARENT );
                brush = yellow_brush;

                b_pens_set = true;
            }

            for( AIS_Area_Notice_Hash::iterator ani = target_data->area_notices.begin();
                    ani != target_data->area_notices.end(); ++ani ) {
                Ais8_001_22& area_notice = ani->second;

                if( area_notice.expiry_time > now ) {
                    std::vector<wxPoint> points;
                    bool draw_polygon = false;

                    switch( area_notice.notice_type ) {
                    case 0:
                        pen.SetColour( green );
                        brush = green_brush;
                        break;
                    case 1:
                        pen.SetColour( yellow );
                        brush = yellow_brush;
                        break;
                    default:
                        pen.SetColour( yellow );
                        brush = yellow_brush;
                    }
                    dc.SetPen( pen );
                    dc.SetBrush( *brush );

                    for( Ais8_001_22_SubAreaList::iterator sa = area_notice.sub_areas.begin();
                            sa != area_notice.sub_areas.end(); ++sa ) {
                        switch( sa->shape ) {
                        case AIS8_001_22_SHAPE_CIRCLE: {
                            wxPoint target_point;
                            cc1->GetCanvasPointPix( sa->latitude, sa->longitude, &target_point );
                            points.push_back( target_point );
                            if( sa->radius_m > 0.0 )
                                dc.DrawCircle( target_point, sa->radius_m * vp_scale );
                            break;
                        }
                        case AIS8_001_22_SHAPE_POLYGON:
                            draw_polygon = true;
                        case AIS8_001_22_SHAPE_POLYLINE: {
                            double lat = sa->latitude;
                            double lon = sa->longitude;
                            for( int i = 0; i < 4; ++i ) {
                                ll_gc_ll( lat, lon, sa->angles[i], sa->dists_m[i] / 1852.0,
                                          &lat, &lon );
                                wxPoint target_point;
                                cc1->GetCanvasPointPix( lat, lon, &target_point );
                                points.push_back( target_point );
                            }
                        }
                        }
                    }
                    if( draw_polygon )
                        dc.DrawPolygon( points.size(), &points.front() );
                }
            }
        }
    }

    if( b_pens_set ) {
        dc.SetPen( pen_save );
        dc.SetBrush( brush_save );
    }

}

static void TargetFrame( ocpnDC &dc, wxPen pen, int x, int y, int radius )
{
    //    Constants?
    int gap2 = 2 * radius / 6;

    wxPen pen_save = dc.GetPen();

    dc.SetPen( pen );

    dc.DrawLine( x - radius, y + gap2, x - radius, y + radius );
    dc.DrawLine( x - radius, y + radius, x - gap2, y + radius );
    dc.DrawLine( x + gap2, y + radius, x + radius, y + radius );
    dc.DrawLine( x + radius, y + radius, x + radius, y + gap2 );
    dc.DrawLine( x + radius, y - gap2, x + radius, y - radius );
    dc.DrawLine( x + radius, y - radius, x + gap2, y - radius );
    dc.DrawLine( x - gap2, y - radius, x - radius, y - radius );
    dc.DrawLine( x - radius, y - radius, x - radius, y - gap2 );

    dc.SetPen( pen_save );
}

static void Draw_Triangle_PointUp( ocpnDC &dc, int x, int y, int base)
	{
		dc.DrawLine( x - base, y , x + base, y  );
		dc.DrawLine( x + base, y , x , y - base );
		dc.DrawLine( x , y - base, x - base, y );
	}
static void Draw_Triangle_PointDown( ocpnDC &dc, int x, int y, int base)
	{
		dc.DrawLine( x , y , x + base, y - base );
		dc.DrawLine( x + base, y - base , x - base, y - base );
		dc.DrawLine( x - base, y - base, x , y );
	}

static void AtoN_Diamond( ocpnDC &dc, int x, int y, int radius, AIS_Target_Data* td )
{    
	//    Constants?
    wxPen pen_save = dc.GetPen();

    wxPen aton_DrawPen;
    wxPen aton_WhiteBorderPen;
    wxBrush aton_Brush;
    
    int rad1a = radius / 2; //size off topmarks of AtoN
    int rad2a = radius / 4;
    int rad3a = rad1a - 1;// slightly smaller size off topmarks to look better for the eye
    
    //Set the Pen for what is needed
    if( ( td->NavStatus == ATON_VIRTUAL_OFFPOSITION ) || ( td->NavStatus == ATON_REAL_OFFPOSITION ) )
        aton_DrawPen = wxPen( GetGlobalColor( _T ( "URED" ) ), 2 );
    else
        aton_DrawPen = wxPen( GetGlobalColor( _T ( "UBLCK" ) ), 2 );

    bool b_virt = ( td->NavStatus == ATON_VIRTUAL )
          | ( td->NavStatus == ATON_VIRTUAL_ONPOSITION )
          | ( td->NavStatus == ATON_VIRTUAL_OFFPOSITION );

    if( b_virt ) 
        aton_DrawPen.SetStyle(wxPENSTYLE_SHORT_DASH );
    
    aton_WhiteBorderPen = wxPen(GetGlobalColor( _T ( "UWHIT" ) ), aton_DrawPen.GetWidth()+2 );
   

    //Draw Base Diamond. First with Thick White pen then custom pen io to get a white border around the line.
    wxPoint diamond[5];
                diamond[0] = wxPoint(  radius, 0 );
                diamond[1] = wxPoint(  0, -radius );
                diamond[2] = wxPoint( -radius, 0 );
                diamond[3] = wxPoint(  0, radius );
                diamond[4] = wxPoint(  radius, 0 );
            dc.SetPen( aton_WhiteBorderPen );
            dc.DrawLines( 5, diamond, x, y );
            dc.SetPen( aton_DrawPen );
            dc.DrawLines( 5, diamond, x, y );

    aton_DrawPen = wxPen( GetGlobalColor( _T ( "UBLCK" ) ), 1 ); // Change drawing pen to Solid and width 1
    aton_WhiteBorderPen = wxPen(GetGlobalColor( _T ( "UWHIT" ) ), aton_DrawPen.GetWidth()+2 );

    // draw cross inside
    wxPoint cross[5];
                cross[0] = wxPoint(  -rad2a, 0 );
                cross[1] = wxPoint(  rad2a, 0 );
                cross[2] = wxPoint( 0, 0 );
                cross[3] = wxPoint(  0, rad2a );
                cross[4] = wxPoint(  0, -rad2a );
            dc.SetPen( aton_WhiteBorderPen );
            dc.DrawLines( 5, cross, x, y );
            dc.SetPen( aton_DrawPen );
            dc.DrawLines( 5, cross, x, y );
            
    wxPoint TriPointUp[4]; //Declare triangles here for multiple use
                TriPointUp[0] = wxPoint(  -rad1a, 0 );
                TriPointUp[1] = wxPoint(  rad1a, 0 );
                TriPointUp[2] = wxPoint(  0, -rad1a );
                TriPointUp[3] = wxPoint(  -rad1a, 0 );
            
    wxPoint TriPointDown[4]; //Declare triangles here for multiple use
    TriPointDown[0] = wxPoint(  -rad1a, -rad1a );
    TriPointDown[1] = wxPoint(  rad1a, -rad1a );
                TriPointDown[2] = wxPoint(  0, 0 );
                TriPointDown[3] = wxPoint(  -rad1a, -rad1a );

    wxPoint CircleOpen[16]; // Workaround to draw transparent circles
                CircleOpen[0] = wxPoint(  -1, 5 );
                CircleOpen[1] = wxPoint(  1, 5 );
                CircleOpen[2] = wxPoint(  3, 4 );
                CircleOpen[3] = wxPoint(  4, 3);
                CircleOpen[4] = wxPoint(  5, 1 );
                CircleOpen[5] = wxPoint(  5,-1  );
                CircleOpen[6] = wxPoint(  4,-3  );
                CircleOpen[7] = wxPoint(  3,-4  );
                CircleOpen[8] = wxPoint(  1,-5  );
                CircleOpen[9] = wxPoint( -1,-5 );
                CircleOpen[10] = wxPoint( -3,-4 );
                CircleOpen[11] = wxPoint( -4,-3 );
                CircleOpen[12] = wxPoint( -5,-1 );
                CircleOpen[13] = wxPoint( -4,3 );
                CircleOpen[14] = wxPoint( -3,4 );
                CircleOpen[15] = wxPoint( -1,5 );
                
    switch (td->ShipType ) {
        case 9 :
        case 20://Card. N
            dc.SetPen( aton_WhiteBorderPen );
            dc.DrawLines( 4, TriPointUp, x, y - radius -1);
            dc.DrawLines( 4, TriPointUp, x, y - radius -rad1a-3);
            dc.SetPen( aton_DrawPen );
            dc.DrawLines( 4, TriPointUp, x, y - radius -1);
            dc.DrawLines( 4, TriPointUp, x, y - radius -rad1a-3);
            break; 
        case 10:
        case 21: //Card E
            dc.SetPen( aton_WhiteBorderPen );
            dc.DrawLines( 4, TriPointDown, x, y - radius -1);
            dc.DrawLines( 4, TriPointUp, x, y - radius -rad1a-3);
            dc.SetPen( aton_DrawPen );
            dc.DrawLines( 4, TriPointDown, x, y - radius -1);
            dc.DrawLines( 4, TriPointUp, x, y - radius -rad1a-3);
            break;
        case 11:
        case 22: //Card S
            dc.SetPen( aton_WhiteBorderPen );
            dc.DrawLines( 4, TriPointDown, x, y - radius -1);
            dc.DrawLines( 4, TriPointDown, x, y - radius -rad1a-3);
            dc.SetPen( aton_DrawPen );
            dc.DrawLines( 4, TriPointDown, x, y - radius -1);
            dc.DrawLines( 4, TriPointDown, x, y - radius -rad1a-3);
            break;
        case 12:
        case 23: //Card W
            dc.SetPen( aton_WhiteBorderPen );
            dc.DrawLines( 4, TriPointUp, x, y - radius -1 );
            dc.DrawLines( 4, TriPointDown, x, y - radius -rad1a-3);
            dc.SetPen( aton_DrawPen );
            dc.DrawLines( 4, TriPointUp, x, y - radius -1);
            dc.DrawLines( 4, TriPointDown, x, y - radius -rad1a-3);
            break;
        case 13: //PortHand Beacon IALA-A
        case 24: { //StarboardHand Beacon IALA-B
           wxPoint aRect[5]; //Square topmark
                aRect[0] = wxPoint(  -rad3a, 0 );
                aRect[1] = wxPoint(  -rad3a, -rad3a-rad3a );
                aRect[2] = wxPoint(  rad3a, -rad3a-rad3a );
                aRect[3] = wxPoint(  rad3a, 0 );
                aRect[4] = wxPoint(  -rad3a, 0 );
            dc.SetPen( aton_WhiteBorderPen );
            dc.DrawLines( 5, aRect, x, y - radius-1 );
            dc.SetPen( aton_DrawPen );
            dc.DrawLines( 5, aRect , x, y - radius-1 );
            }
            break;
        case 14: //StarboardHand Beacon IALA-A
        case 25: //PortHand Beacon IALA-B 
            dc.SetPen( aton_WhiteBorderPen );
            dc.DrawLines( 4, TriPointUp, x, y - radius );
            dc.SetPen( aton_DrawPen );
            dc.DrawLines( 4, TriPointUp, x, y - radius);
            break;
        case 17: 
        case 28: //Isolated danger
            dc.SetPen( aton_WhiteBorderPen );
            dc.DrawLines( 16, CircleOpen, x, y - radius -5);
            dc.SetPen( aton_DrawPen );
            dc.DrawLines( 16, CircleOpen, x, y - radius -5);
            dc.SetPen( aton_WhiteBorderPen );
            dc.DrawLines( 16, CircleOpen, x, y - radius -16);
            dc.SetPen( aton_DrawPen );
            dc.DrawLines( 16, CircleOpen, x, y - radius -16);
            break;
        case 18: 
        case 29: //Safe water
            dc.SetPen( aton_WhiteBorderPen );
            dc.DrawLines( 16, CircleOpen, x, y - radius -5);
            dc.SetPen( aton_DrawPen );
            dc.DrawLines( 16, CircleOpen, x, y - radius -5);
            break;
        case 19: 
        case 30:{ //Special Mark
            cross[0] = wxPoint( -rad2a, -rad2a ); //reuse of cross array
            cross[1] = wxPoint( rad2a, rad2a);
            cross[2] = wxPoint( 0, 0 );
            cross[3] = wxPoint( -rad2a, rad2a );
            cross[4] = wxPoint( rad2a, -rad2a );
            dc.SetPen( aton_WhiteBorderPen );
            dc.DrawLines( 5, cross, x, y - radius-rad3a);
            dc.SetPen( aton_DrawPen );
            dc.DrawLines( 5, cross, x, y - radius-rad3a);
            }
            break;
        default:
        break;    
    }
    dc.SetPen( pen_save );
}


static void Base_Square( ocpnDC &dc, wxPen pen, int x, int y, int radius )
{
    //    Constants?
    int gap2 = 2 * radius / 6;
    int pen_width = pen.GetWidth();

    wxPen pen_save = dc.GetPen();

    dc.SetPen( pen );   // draw square

    dc.DrawLine( x - radius, y - radius, x - radius, y + radius );
    dc.DrawLine( x - radius, y + radius, x + radius, y + radius );
    dc.DrawLine( x + radius, y + radius, x + radius, y - radius );
    dc.DrawLine( x + radius, y - radius, x - radius, y - radius );

    if( pen_width > 1 ) {
        pen_width -= 1;
        pen.SetWidth( pen_width );
    }    // draw cross inside

    dc.DrawLine( x - gap2, y, x + gap2, y );
    dc.DrawLine( x, y - gap2, x, y + gap2 );

    dc.SetPen( pen_save );
}

static void SART_Render( ocpnDC &dc, wxPen pen, int x, int y, int radius )
{
    //    Constants
    int gap = ( radius * 12 ) / 10;
    int pen_width = pen.GetWidth();

    wxPen pen_save = dc.GetPen();

    dc.SetPen( pen );

    wxBrush brush_save = dc.GetBrush();
    wxBrush *ppBrush = wxTheBrushList->FindOrCreateBrush( wxColour( 0, 0, 0 ), wxBRUSHSTYLE_TRANSPARENT );
    dc.SetBrush( *ppBrush );

    dc.DrawCircle( x, y, radius );

    if( pen_width > 1 ) {
        pen_width -= 1;
        pen.SetWidth( pen_width );
    }    // draw cross inside

    dc.DrawLine( x - gap, y - gap, x + gap, y + gap );
    dc.DrawLine( x - gap, y + gap, x + gap, y - gap );

    dc.SetBrush( brush_save );
    dc.SetPen( pen_save );
}

// spherical coordinates is sufficient for visually plotting with relatively small
// distances and about 6x faster than ll_gc_ll
static void spherical_ll_gc_ll(float lat, float lon, float brg, float dist, float *dlat, float *dlon)
{
    float angr = brg/180*M_PI;
    float latr = lat*M_PI/180;
    float D = dist/3443; // earth radius in nm
    float sD = sinf(D), cD = cosf(D);
    float sy = sinf(latr), cy = cosf(latr);
    float sa = sinf(angr), ca = cosf(angr);

    *dlon = lon + asinf(sa*sD/cy) * 180/M_PI;
    *dlat = asinf(sy*cD + cy*sD*ca) * 180/M_PI;
}

static void AISDrawTarget( AIS_Target_Data *td, ocpnDC& dc )
{
    //      Target data must be valid
    if( NULL == td ) return;

    //    Target is lost due to position report time-out, but still in Target List
    if( td->b_lost ) return;
    
    float scale_factor = 1.0;
//    if(g_bresponsive){
        scale_factor =  g_ChartScaleFactorExp;
//    }
    
    //      Skip anchored/moored (interpreted as low speed) targets if requested
    //      unless the target is NUC or AtoN, in which case it is always displayed.
    if( ( g_bHideMoored ) && ( td->SOG <= g_ShowMoored_Kts )
            && ( td->NavStatus != NOT_UNDER_COMMAND )
            && ( ( td->Class == AIS_CLASS_A ) || ( td->Class == AIS_CLASS_B ) ) ) return;

    //      Target data position must have been valid once
    if( !td->b_positionOnceValid ) return;

    // And we never draw ownship
    if( td->b_OwnShip ) return;

    //    If target's speed is unavailable, use zero for further calculations
    float target_sog = td->SOG;
    if( (td->SOG > 102.2) && !td->b_SarAircraftPosnReport )
        target_sog = 0.;

    int drawit = 0;
    wxPoint TargetPoint, PredPoint;

    //   Always draw alert targets, even if they are off the screen
    if( td->n_alert_state == AIS_ALERT_SET ) drawit++;
    else
    //    Is target in Vpoint?
    if( cc1->GetVP().GetBBox().Contains( td->Lat,  td->Lon ))
        drawit++;                       // yep
    else
    //  If AIS tracks are shown, is the first point of the track on-screen?
    if( 1/*g_bAISShowTracks*/ && td->b_show_track ) {
        wxAISTargetTrackListNode *node = td->m_ptrack->GetFirst();
        if( node ) {
            AISTargetTrackPoint *ptrack_point = node->GetData();
            if( cc1->GetVP().GetBBox().Contains( ptrack_point->m_lat,  ptrack_point->m_lon ) )
                drawit++;
        }
    }

    //    Calculate AIS target Position Predictor, using global static variable for length of vector

    float pred_lat, pred_lon;
    spherical_ll_gc_ll( td->Lat, td->Lon, td->COG, target_sog * g_ShowCOG_Mins / 60., &pred_lat, &pred_lon );

    //    Is predicted point in the VPoint?
    if( cc1->GetVP().GetBBox().Contains( pred_lat,  pred_lon ) )
        drawit++;                     // yep
    else {
        LLBBox box;
        box.SetFromSegment(td->Lon, td->Lat, pred_lon, pred_lat);
    // And one more test to catch the case where target COG line crosses the screen,
    // but the target itself and its pred point are both off-screen
        if( !cc1->GetVP().GetBBox().IntersectOut(box))
            drawit++;
    }

    //    Do the draw if conditions indicate
    if( !drawit )
        return;

    cc1->GetCanvasPointPix( td->Lat, td->Lon, &TargetPoint );
    cc1->GetCanvasPointPix( pred_lat, pred_lon, &PredPoint );

    float theta;
    //    If the target reported a valid HDG, then use it for icon
    if( (int) ( td->HDG ) != 511 ) {
        theta = ( ( td->HDG - 90 ) * PI / 180. ) + cc1->GetVP().rotation;
    } else {
        // question: why can we not compute similar to above using COG instead of HDG?
        //  Calculate the relative angle for this chart orientation
        //    Use a 100 pixel vector to calculate angle
        float angle_distance_nm = ( 100. / cc1->GetVP().view_scale_ppm ) / 1852.;
        float angle_lat, angle_lon;
        spherical_ll_gc_ll( td->Lat, td->Lon, td->COG, angle_distance_nm, &angle_lat, &angle_lon );

        wxPoint AnglePoint;
        cc1->GetCanvasPointPix( angle_lat, angle_lon, &AnglePoint );

        if( abs( AnglePoint.x - TargetPoint.x ) > 0 ) {
            if( target_sog > g_ShowMoored_Kts )
                theta = atan2f(
                    (double) ( AnglePoint.y - TargetPoint.y ),
                    (double) ( AnglePoint.x - TargetPoint.x ) );
            else
                theta = (float)-PI / 2.;
        } else {
            if( AnglePoint.y > TargetPoint.y )
                theta = (float)PI / 2.;             // valid COG 180
            else
                theta = (float)-PI / 2.;            //  valid COG 000 or speed is too low to resolve course
        }
    }

    // only need to compute this once;
    float sin_theta = sinf( theta ), cos_theta = cosf( theta );

    wxDash dash_long[2];
    dash_long[0] = (int) ( 1.0 * cc1->GetPixPerMM() );  // Long dash  <---------+
    dash_long[1] = (int) ( 0.5 * cc1->GetPixPerMM() );  // Short gap            |

    int targetscale = 100;
    if ( g_bAllowShowScaled && g_bShowScaled ){
            double temp_importance, So, Tcpa, Cpa, Rang, Siz = 0.; //calc the importance of target
            So = g_ScaledNumWeightSOG/12 * td->SOG; //0 - 12 knts gives 0 - g_ScaledNumWeightSOG weight
            if (So > g_ScaledNumWeightSOG) So = g_ScaledNumWeightSOG; 
                       
            if (td->bCPA_Valid){
                Cpa=g_ScaledNumWeightCPA - g_ScaledNumWeightCPA/4 * td->CPA;
                //if TCPA is positief (target is coming closer), make weight of CPA bigger
                if (td->TCPA > .0) Cpa = Cpa + Cpa * g_ScaledNumWeightTCPA/100;
                if ( Cpa < .0 ) Cpa = .0; //if CPA is > 4
            }
            else Cpa = .0;
            
            Rang = g_ScaledNumWeightRange / 10 * td->Range_NM;
            if ( Rang > g_ScaledNumWeightRange ) Rang = g_ScaledNumWeightRange;
            Rang = g_ScaledNumWeightRange - Rang;
                                                   
            Siz = g_ScaledNumWeightSizeOfT/30*( td->DimA + td->DimB);
            if ( Siz > g_ScaledNumWeightSizeOfT ) Siz = g_ScaledNumWeightSizeOfT;
            temp_importance = So + Cpa + Rang + Siz;
            
            td->importance=(int)temp_importance;
            int calc_scale = 0;
            if ( td->importance  > ImportanceSwitchPoint ) calc_scale=100; else calc_scale = 50; //50% is minium scale for target
            //with one tick per second targets gan slink from 100 to g_ScaledSizeMinimal% in 25 seconds
            if ( td->importance  < ImportanceSwitchPoint ) targetscale = td->last_scale -2;
            //growing from g_ScaledSizeMinimal till 100% goes faster in 10 seconds
            if ( td->importance  > ImportanceSwitchPoint ) targetscale = td->last_scale +5; 
            if ( targetscale > 100 ) targetscale = 100;
            if ( targetscale < 50 ) targetscale = 50;//g_ScaledSizeMinimal;
            td->last_scale = targetscale;            
        }//if (g_bShowScaled
    
    //  Draw the icon rotated to the COG
    wxPoint ais_real_size[6];
    bool bcan_draw_size = true;
    if (g_bDrawAISSize)
    {
        if (td->DimA + td->DimB == 0 || td->DimC + td->DimD == 0)
        {
            bcan_draw_size = false;
        }
        else
        {
            double ref_lat, ref_lon;
            ll_gc_ll( td->Lat, td->Lon, 0, 100. / 1852., &ref_lat, &ref_lon );
            wxPoint2DDouble b_point = cc1->GetVP().GetDoublePixFromLL( td->Lat, td->Lon );
            wxPoint2DDouble r_point = cc1->GetVP().GetDoublePixFromLL( ref_lat, ref_lon );
            double ppm = r_point.GetDistance(b_point) / 100.;
            double offwid = (td->DimC + td->DimD) * ppm * 0.25;
            double offlen = (td->DimA + td->DimB) * ppm * 0.15;
            ais_real_size[0].x = -td->DimD * ppm;
            ais_real_size[0].y = -td->DimB * ppm;
            ais_real_size[1].x = -td->DimD * ppm;
            ais_real_size[1].y = td->DimA * ppm - offlen;
            ais_real_size[2].x = -td->DimD * ppm + offwid;
            ais_real_size[2].y = td->DimA * ppm;
            ais_real_size[3].x = td->DimC * ppm - offwid;
            ais_real_size[3].y = td->DimA * ppm;
            ais_real_size[4].x = td->DimC * ppm;
            ais_real_size[4].y = td->DimA * ppm - offlen;
            ais_real_size[5].x = td->DimC * ppm;
            ais_real_size[5].y = -td->DimB * ppm;

            if (ais_real_size[4].x - ais_real_size[0].x < 16 || ais_real_size[2].y - ais_real_size[0].y < 30)
                bcan_draw_size = false; //drawing too small does not make sense
            else {
                bcan_draw_size = true;
                transrot_pts(6, ais_real_size, sin_theta, cos_theta);
            }
        }
    }
    
    // to speed up we only calculate scale when not max or minimal 
    wxPoint ais_quad_icon[4]={ wxPoint(-4, -3),  wxPoint(0, 12),  wxPoint(4, -3), wxPoint(0, -3) };
    if (targetscale == 100){
        ais_quad_icon[0] = wxPoint(-8, -6);
        ais_quad_icon[1] = wxPoint( 0, 24);
        ais_quad_icon[2] = wxPoint( 8, -6);
        ais_quad_icon[3] = wxPoint( 0, -6);
    }
    else if ( targetscale != 50) {
            ais_quad_icon[0] = wxPoint((int)-8*targetscale/100, (int)-6*targetscale/100);
            ais_quad_icon[1] = wxPoint( 0, (int)24*targetscale/100);
            ais_quad_icon[2] = wxPoint( (int)8*targetscale/100, (int)-6*targetscale/100);
            ais_quad_icon[3] = wxPoint( 0, (int)-6*targetscale/100);
    }

    //   If this is an AIS Class B target, so symbolize it differently
    if( td->Class == AIS_CLASS_B ) ais_quad_icon[3].y = 0;
    else if( td->Class == AIS_GPSG_BUDDY ) {
        ais_quad_icon[0] = wxPoint(-5, -12);
        ais_quad_icon[1] = wxPoint(-3,  12);
        ais_quad_icon[2] = wxPoint( 3,  12);
        ais_quad_icon[3] = wxPoint( 5, -12);
    }
    else if( td->Class == AIS_DSC ) {
        ais_quad_icon[0].y = 0;
        ais_quad_icon[1].y = 8;
        ais_quad_icon[2].y = 0;
        ais_quad_icon[3].y = -8;
    }
    else if( td->Class == AIS_APRS ) {
        ais_quad_icon[0] = wxPoint(-8, -8);
        ais_quad_icon[1] = wxPoint(-8,  8);
        ais_quad_icon[2] = wxPoint( 8,  8);
        ais_quad_icon[3] = wxPoint( 8, -8);
    }

    transrot_pts(4, ais_quad_icon, sin_theta, cos_theta);

    wxColour UBLCK = GetGlobalColor( _T ( "UBLCK" ));
    dc.SetPen( wxPen( UBLCK ) );

    // Default color is green
    wxColour UINFG = GetGlobalColor( _T ( "UINFG" ));
    wxBrush target_brush = wxBrush( UINFG );

    // Euro Inland targets render slightly differently
    if( td->b_isEuroInland )
        target_brush = wxBrush( GetGlobalColor( _T ( "TEAL1" ) ) );

    // Target name comes from cache
    if( td->b_nameFromCache )
        target_brush = wxBrush( GetGlobalColor( _T ( "GREEN5" ) ) );
            
    //and....
    wxColour URED = GetGlobalColor( _T ( "URED" ));
    if( !td->b_nameValid )
        target_brush = wxBrush( GetGlobalColor( _T ( "CHYLW" ) ) );
    if( ( td->Class == AIS_DSC ) && ( td->ShipType == 12 ) )                    // distress
        target_brush = wxBrush( URED );
    if( td->b_SarAircraftPosnReport )
        target_brush = wxBrush( UINFG );

    if( ( td->n_alert_state == AIS_ALERT_SET ) && ( td->bCPA_Valid ) )
        target_brush = wxBrush( URED );

    if( td->b_positionDoubtful ) target_brush = wxBrush( GetGlobalColor( _T ( "UINFF" ) ) );

    //    Check for alarms here, maintained by AIS class timer tick
    if( ((td->n_alert_state == AIS_ALERT_SET) && (td->bCPA_Valid)) || (td->b_show_AIS_CPA && (td->bCPA_Valid))) {
        //  Calculate the point of CPA for target
        double tcpa_lat, tcpa_lon;
        ll_gc_ll( td->Lat, td->Lon, td->COG, target_sog * td->TCPA / 60., &tcpa_lat, &tcpa_lon );
        wxPoint tCPAPoint;
        wxPoint TPoint = TargetPoint;
        cc1->GetCanvasPointPix( tcpa_lat, tcpa_lon, &tCPAPoint );

        //  Draw the intercept line from target
        ClipResult res = cohen_sutherland_line_clip_i( &TPoint.x, &TPoint.y, &tCPAPoint.x,
                                                       &tCPAPoint.y, 0, cc1->GetVP().pix_width, 0, cc1->GetVP().pix_height );

        if( res != Invisible ) {
            wxPen ppPen2( URED, 2, wxPENSTYLE_USER_DASH );
            ppPen2.SetDashes( 2, dash_long );
            dc.SetPen( ppPen2 );

            dc.StrokeLine( TPoint.x, TPoint.y, tCPAPoint.x, tCPAPoint.y );
        }

        //  Calculate the point of CPA for ownship
        double ocpa_lat, ocpa_lon;

        //  Detect and handle the case where ownship COG is undefined....
        if( wxIsNaN(gCog) || wxIsNaN( gSog ) ) {
            ocpa_lat = gLat;
            ocpa_lon = gLon;
        }
        else {
            ll_gc_ll( gLat, gLon, gCog, gSog * td->TCPA / 60., &ocpa_lat, &ocpa_lon );
        }

        wxPoint oCPAPoint;

        cc1->GetCanvasPointPix( ocpa_lat, ocpa_lon, &oCPAPoint );
        cc1->GetCanvasPointPix( tcpa_lat, tcpa_lon, &tCPAPoint );

        //        Save a copy of these unclipped points
        wxPoint oCPAPoint_unclipped = oCPAPoint;
        wxPoint tCPAPoint_unclipped = tCPAPoint;

        //  Draw a line from target CPA point to ownship CPA point
        ClipResult ores = cohen_sutherland_line_clip_i( &tCPAPoint.x, &tCPAPoint.y,
                                                        &oCPAPoint.x, &oCPAPoint.y, 0, cc1->GetVP().pix_width, 0, cc1->GetVP().pix_height );

        if( ores != Invisible ) {
            wxColour yellow = GetGlobalColor( _T ( "YELO1" ) );
            dc.SetPen( wxPen( yellow, 4 ) );
            dc.StrokeLine( tCPAPoint.x, tCPAPoint.y, oCPAPoint.x, oCPAPoint.y );

            wxPen ppPen2( URED, 2, wxPENSTYLE_USER_DASH );
            ppPen2.SetDashes( 2, dash_long );
            dc.SetPen( ppPen2 );
            dc.StrokeLine( tCPAPoint.x, tCPAPoint.y, oCPAPoint.x, oCPAPoint.y );

            //        Draw little circles at the ends of the CPA alert line
            wxBrush br( GetGlobalColor( _T ( "BLUE3" ) ) );
            dc.SetBrush( br );
            dc.SetPen( wxPen( UBLCK ) );

            //  Using the true ends, not the clipped ends
            dc.StrokeCircle( tCPAPoint_unclipped.x, tCPAPoint_unclipped.y, 5 );
            dc.StrokeCircle( oCPAPoint_unclipped.x, oCPAPoint_unclipped.y, 5 );
        }

        // Draw the intercept line from ownship
        wxPoint oShipPoint;
        cc1->GetCanvasPointPix ( gLat, gLon, &oShipPoint );
        oCPAPoint = oCPAPoint_unclipped;    // recover the unclipped point

        ClipResult ownres = cohen_sutherland_line_clip_i ( &oShipPoint.x, &oShipPoint.y,
                                                           &oCPAPoint.x, &oCPAPoint.y,
                                                           0, cc1->GetVP().pix_width, 0, cc1->GetVP().pix_height );

        if ( ownres != Invisible ) {
            wxPen ppPen2 ( URED, 2, wxPENSTYLE_USER_DASH );
            ppPen2.SetDashes( 2, dash_long );
            dc.SetPen(ppPen2);

            dc.StrokeLine ( oShipPoint.x, oShipPoint.y, oCPAPoint.x, oCPAPoint.y );
        } //TR : till here

        dc.SetPen( wxPen( UBLCK ) );
        dc.SetBrush( wxBrush( URED ) );
    }

    //  Highlight the AIS target symbol if an alert dialog is currently open for it
    if( g_pais_alert_dialog_active && g_pais_alert_dialog_active->IsShown() ) {
        if( g_pais_alert_dialog_active->Get_Dialog_MMSI() == td->MMSI )
            cc1->JaggyCircle( dc, wxPen( URED , 2 ), TargetPoint.x, TargetPoint.y, 100 );
    }

    //  Highlight the AIS target symbol if a query dialog is currently open for it
    if( g_pais_query_dialog_active && g_pais_query_dialog_active->IsShown() ) {
        if( g_pais_query_dialog_active->GetMMSI() == td->MMSI )
            TargetFrame( dc, wxPen( UBLCK , 2 ), TargetPoint.x, TargetPoint.y, 25 );
    }
    
    //       Render the COG line if the speed is greater than moored speed defined by ais options dialog
    if( ( g_bShowCOG ) && ( target_sog > g_ShowMoored_Kts ) && td->b_active ) {
        int pixx = TargetPoint.x;
        int pixy = TargetPoint.y;
        int pixx1 = PredPoint.x;
        int pixy1 = PredPoint.y;

        //  Don't draw the COG line  and predictor point if zoomed far out.... or if target lost/inactive
        float l = sqrtf( powf( (float) ( PredPoint.x - TargetPoint.x ), 2 )
                         + powf( (float) ( PredPoint.y - TargetPoint.y ), 2 ) );

        if( l > 24 ) {
            ClipResult res = cohen_sutherland_line_clip_i( &pixx, &pixy, &pixx1, &pixy1, 0,
                                                           cc1->GetVP().pix_width, 0, cc1->GetVP().pix_height );

            if( res != Invisible ) {
                    //    Draw a wider coloured line
                    if (targetscale >= 75){
                        wxPen wide_pen( target_brush.GetColour(), g_ais_cog_predictor_width );
                        dc.SetPen( wide_pen );
                        dc.StrokeLine( pixx, pixy, pixx1, pixy1 );
                    }

                    if( g_ais_cog_predictor_width > 1 ) {
                        //    Draw a 1 pixel wide black line
                        wxPen narrow_pen( UBLCK, 1 );
                        dc.SetPen( narrow_pen );
                        dc.StrokeLine( pixx, pixy, pixx1, pixy1 );
                    }

#ifdef ocpnUSE_GL

/*
                    // opengl optimized version, looks not as nice...
                    
                    wxColour c = target_brush.GetColour();
                    glColor3ub(c.Red(), c.Green(), c.Blue());
                    float dx = pixx1 - pixx, dy = pixy1 - pixy;
                    float m = (g_ais_cog_predictor_width + 1) / 2 / sqrtf(dx*dx + dy*dy);
                    float tx = dy * m, ty = dx * m;
                    glBegin(GL_TRIANGLE_STRIP);
                    glVertex2f(pixx+tx,  pixy+ty);
                    glVertex2f(pixx1+tx, pixy1+ty);
                    glVertex2f(pixx-tx,  pixy-ty);
                    glVertex2f(pixx1-tx, pixy1-ty);
                    glEnd();

                    if( g_ais_cog_predictor_width > 1 ) {
                        //    Draw a 1 pixel wide black line
                        glLineWidth( 1 );
                        glColor3ub(0, 0, 0);
                        glEnable( GL_BLEND );
                        glEnable( GL_LINE_SMOOTH );
                        glBegin(GL_LINES);
                        glVertex2i(pixx, pixy);
                        glVertex2i(pixx1, pixy1);
                        glEnd();
                        glDisable( GL_LINE_SMOOTH );
                        glDisable( GL_BLEND );
                        glColor3ub(c.Red(), c.Green(), c.Blue());
                    }
*/
#endif
                    if(dc.GetDC()) {      
                        dc.SetBrush( target_brush );
                        if (targetscale >= 75)
                            dc.StrokeCircle( PredPoint.x, PredPoint.y, 5 );
                        else
                            dc.StrokeCircle( PredPoint.x, PredPoint.y, 2 );
                    } else {
#ifdef ocpnUSE_GL
                        
                    // draw circle
                        float points[] = {0.0f, 5.0f, 2.5f, 4.330127f, 4.330127f, 2.5f, 5.0f,
                                      0, 4.330127f, -2.5f, 2.5f, -4.330127f, 0, -5.1f,
                                      -2.5f, -4.330127f, -4.330127f, -2.5f, -5.0f, 0,
                                      -4.330127f, 2.5f, -2.5f, 4.330127f, 0, 5.0f};
                        if (targetscale <= 75){
                            for (unsigned int i = 0; i<(sizeof points) / (sizeof *points); i++ )
                               points[i] =  points[i]/2;
                       }
                                      
                        wxColour c = target_brush.GetColour();
                        glColor3ub(c.Red(), c.Green(), c.Blue());
                                      
                        glBegin(GL_TRIANGLE_FAN);
                        for(unsigned int i=0; i<(sizeof points) / (sizeof *points); i+=2)
                            glVertex2i(pixx1 + points[i], pixy1 + points[i+1]);
                        glEnd();

                        glColor3ub(0, 0, 0);
                        glLineWidth( 1 );
                        glBegin(GL_LINE_LOOP);
                        for(unsigned int i=0; i<(sizeof points) / (sizeof *points); i+=2)
                            glVertex2i(pixx1 + points[i], pixy1 + points[i+1]);
                        glEnd();
#endif                    
                    }
            }

            //      Draw RateOfTurn Vector
            if( ( td->ROTAIS != 0 ) && ( td->ROTAIS != -128 ) && (!g_bShowScaled) ) {
                float nv = 10;
                float theta2 = theta;
                if( td->ROTAIS > 0 ) theta2 += (float)PI / 2.;
                else
                    theta2 -= (float)PI / 2.;

                int xrot = (int) round ( pixx1 + ( nv * cosf ( theta2 ) ) );
                int yrot = (int) round ( pixy1 + ( nv * sinf ( theta2 ) ) );
                dc.StrokeLine( pixx1, pixy1, xrot, yrot );
            }
        }
    }

    //        Actually Draw the target
    if( td->Class == AIS_ARPA ) {
        wxPen target_pen( UBLCK, 2 );

        dc.SetPen( target_pen );
        dc.SetBrush( target_brush );

        dc.StrokeCircle( TargetPoint.x, TargetPoint.y, 9 );
        dc.StrokeCircle( TargetPoint.x, TargetPoint.y, 1 );
        //        Draw the inactive cross-out line
        if( !td->b_active ) {
            dc.SetPen( wxPen( UBLCK, 2 ) );
            dc.StrokeLine( TargetPoint.x - 14, TargetPoint.y, TargetPoint.x + 14, TargetPoint.y );
            dc.SetPen( wxPen( UBLCK, 1 ) );
        }
    } else if( td->Class == AIS_ATON ) {                   // Aid to Navigation
        AtoN_Diamond( dc, TargetPoint.x, TargetPoint.y, 12, td );
    } else if( td->Class == AIS_BASE ) {                      // Base Station
        Base_Square( dc, wxPen( UBLCK , 2 ), TargetPoint.x, TargetPoint.y, 8 );
    } else if( td->Class == AIS_SART ) {                      // SART Target
        if( td->NavStatus == 14 )       // active
            SART_Render( dc, wxPen( URED , 2 ), TargetPoint.x, TargetPoint.y, 8 );
        else
            SART_Render( dc, wxPen( GetGlobalColor( _T ( "UGREN" ) ), 2 ),
                         TargetPoint.x, TargetPoint.y, 8 );

    } else if(td->b_SarAircraftPosnReport) {
        wxPoint SarIcon[10];
        wxPoint SarRot[10];

        SarIcon[0] = wxPoint(0, 12);
        SarIcon[1] = wxPoint(4, 2);
        SarIcon[2] = wxPoint(16, -2);
        SarIcon[3] = wxPoint(16, -8);
        SarIcon[4] = wxPoint(4, -8);
        SarIcon[5] = wxPoint(3, -16);
        SarIcon[6] = wxPoint(10, -18);
        SarIcon[7] = wxPoint(10, -22);
        SarIcon[8] = wxPoint(0, -22);


        // Draw icon as two halves

        //  First half

        for( int i = 0; i < 9; i++ )
            SarRot[i] = SarIcon[i];
        transrot_pts(9, SarRot, sin_theta, cos_theta);

        wxPen tri_pen( target_brush.GetColour(), 1 );
        dc.SetPen( tri_pen );
        dc.SetBrush( target_brush );

        int mappings[7][3] = {{0, 1, 4}, {1, 2, 3}, {1, 3, 4}, {0, 4, 5}, {0, 5, 8}, {5, 6, 7}, {5, 7, 8}};
        for(int i=0; i<7; i++) {
            wxPoint ais_tri_icon[3];
            for(int j=0; j<3; j++)
                ais_tri_icon[j] = SarRot[mappings[i][j]];
            dc.StrokePolygon( 3, ais_tri_icon, TargetPoint.x, TargetPoint.y );
        }

        wxPen target_outline_pen( UBLCK, 2 );
        dc.SetPen( target_outline_pen );
        dc.SetBrush( wxBrush( UBLCK, wxBRUSHSTYLE_TRANSPARENT ) );
        dc.StrokePolygon( 9, SarRot, TargetPoint.x, TargetPoint.y );

        // second half

        for( int i = 0; i < 9; i++ )
            SarRot[i] = wxPoint(-SarIcon[i].x, SarIcon[i].y); // mirror the icon (x -> -x)

        transrot_pts(9, SarRot, sin_theta, cos_theta);

        dc.SetPen( tri_pen );
        dc.SetBrush( target_brush );

        for(int i=0; i<7; i++) {
            wxPoint ais_tri_icon[3];
            for(int j=0; j<3; j++)
                ais_tri_icon[j] = SarRot[mappings[i][j]];
            dc.StrokePolygon( 3, ais_tri_icon, TargetPoint.x, TargetPoint.y );
        }

        dc.SetPen( target_outline_pen );
        dc.SetBrush( wxBrush( UBLCK, wxBRUSHSTYLE_TRANSPARENT ) );
        dc.StrokePolygon( 9, SarRot, TargetPoint.x, TargetPoint.y );

    } else {         // ship class A or B or a Buddy or DSC
        wxPen target_pen( UBLCK, 1 );

        dc.SetBrush( target_brush );

        dc.SetPen( target_pen );
        if(dc.GetDC()) {
            dc.StrokePolygon( 4, ais_quad_icon, TargetPoint.x, TargetPoint.y, scale_factor );
        } else {
#ifdef ocpnUSE_GL
            wxColour c = target_brush.GetColour();
            glColor3ub(c.Red(), c.Green(), c.Blue());
            glBegin(GL_TRIANGLE_FAN);

            glVertex2i(ais_quad_icon[3].x + TargetPoint.x, ais_quad_icon[3].y  + TargetPoint.y);
            glVertex2i(ais_quad_icon[0].x + TargetPoint.x, ais_quad_icon[0].y  + TargetPoint.y);
            glVertex2i(ais_quad_icon[1].x + TargetPoint.x, ais_quad_icon[1].y  + TargetPoint.y);
            glVertex2i(ais_quad_icon[2].x + TargetPoint.x, ais_quad_icon[2].y  + TargetPoint.y);
            
            glEnd();

            glLineWidth(1);
            glColor3ub(0,0,0);
            glBegin(GL_LINE_LOOP);
            for(int i=0; i<4; i++)
                glVertex2i(ais_quad_icon[i].x + TargetPoint.x, ais_quad_icon[i].y  + TargetPoint.y);
            glEnd();
#endif
        }

        if (g_bDrawAISSize && bcan_draw_size)
        {
            dc.SetBrush( wxBrush( UBLCK, wxBRUSHSTYLE_TRANSPARENT ) );
            dc.StrokePolygon( 6, ais_real_size, TargetPoint.x, TargetPoint.y, 1.0 );
        }

        dc.SetBrush( wxBrush( GetGlobalColor( _T ( "SHIPS" ) ) ) );
        int navstatus = td->NavStatus;

        // HSC usually have correct ShipType but navstatus == 0...
        if( ( ( td->ShipType >= 40 ) && ( td->ShipType < 50 ) )
            && navstatus == UNDERWAY_USING_ENGINE ) navstatus = HSC;
    
        if(targetscale > 90){
            switch( navstatus ) {
            case MOORED:
            case AT_ANCHOR: {
                dc.StrokeCircle( TargetPoint.x, TargetPoint.y, 4 );
                break;
            }
            case RESTRICTED_MANOEUVRABILITY: {
                wxPoint diamond[4];
                diamond[0] = wxPoint(  4, 0 );
                diamond[1] = wxPoint(  0, -6 );
                diamond[2] = wxPoint( -4, 0 );
                diamond[3] = wxPoint(  0, 6 );
                dc.StrokePolygon( 4, diamond, TargetPoint.x, TargetPoint.y-11 );
                dc.StrokeCircle( TargetPoint.x, TargetPoint.y, 4 );
                dc.StrokeCircle( TargetPoint.x, TargetPoint.y-22, 4 );
                break;
                break;
            }
            case CONSTRAINED_BY_DRAFT: {
                wxPoint can[4] = {wxPoint(-3, 0), wxPoint(3, 0), wxPoint(3, -16), wxPoint(-3, -16)};
                dc.StrokePolygon( 4, can, TargetPoint.x, TargetPoint.y );
                break;
            }
            case NOT_UNDER_COMMAND: {
                dc.StrokeCircle( TargetPoint.x, TargetPoint.y, 4 );
                dc.StrokeCircle( TargetPoint.x, TargetPoint.y-9, 4 );
                break;
            }
            case FISHING: {
                wxPoint tri[3];
                tri[0] = wxPoint( -4, 0 );
                tri[1] = wxPoint(  4, 0 );
                tri[2] = wxPoint(  0, -9 );
                dc.StrokePolygon( 3, tri, TargetPoint.x, TargetPoint.y );
                tri[0] = wxPoint(  0, -9 );
                tri[1] = wxPoint(  4, -18 );
                tri[2] = wxPoint( -4, -18 );
                dc.StrokePolygon( 3, tri, TargetPoint.x, TargetPoint.y );
                break;
            }
            case AGROUND: {
                dc.StrokeCircle( TargetPoint.x, TargetPoint.y, 4 );
                dc.StrokeCircle( TargetPoint.x, TargetPoint.y-9, 4 );
                dc.StrokeCircle( TargetPoint.x, TargetPoint.y-18, 4 );
                break;
            }
            case HSC:
            case WIG: {
                dc.SetBrush( target_brush );

                wxPoint arrow1[3] = {wxPoint( -4, 20 ), wxPoint(  0, 27 ), wxPoint(  4, 20 )};
                transrot_pts(3, arrow1, sin_theta, cos_theta, TargetPoint);
                dc.StrokePolygon( 3, arrow1 );

                wxPoint arrow2[3] = {wxPoint( -4, 27 ), wxPoint(  0, 34 ), wxPoint(  4, 27 )};
                transrot_pts(3, arrow2, sin_theta, cos_theta, TargetPoint);
                dc.StrokePolygon( 3, arrow2 );
                break;
            }
            }
        }//end if (targetscale > 75)

        //        Draw the inactive cross-out line
        if( !td->b_active ) {
            wxPoint p1 = transrot( wxPoint( (int)-14*targetscale/100, 0 ), sin_theta, cos_theta, TargetPoint );
            wxPoint p2 = transrot( wxPoint( (int)14*targetscale/100, 0 ),  sin_theta, cos_theta, TargetPoint );

            dc.SetPen( wxPen( UBLCK, 2 ) );
            dc.StrokeLine( p1.x, p1.y, p2.x, p2.y );
        }

        //    European Inland AIS define a "stbd-stbd" meeting sign, a blue paddle.
        //    Symbolize it if set by most recent message
        if( td->b_blue_paddle ) {
            wxPoint ais_flag_icon[4];
            ais_flag_icon[0] = wxPoint((int)-8*targetscale/100, (int)-6*targetscale/100);
            ais_flag_icon[1] = wxPoint( (int)-2*targetscale/100, (int)18*targetscale/100);
            ais_flag_icon[2] = wxPoint( (int)-2*targetscale/100, 0);
            ais_flag_icon[3] = wxPoint( (int)-2*targetscale/100, (int)-6*targetscale/100);
            
            transrot_pts(4, ais_flag_icon, sin_theta, cos_theta, TargetPoint);

            int penWidth = 2;
            if(targetscale < 100)
                penWidth = 1;
            dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UINFB" ) ) ) );
            dc.SetPen( wxPen( GetGlobalColor( _T ( "CHWHT" ) ), penWidth ) );
            dc.StrokePolygon( 4, ais_flag_icon);
        }
    }

    if ( (g_bShowAISName) && (targetscale > 75) ) {
        int true_scale_display = (int) (floor( cc1->GetVP().chart_scale / 100. ) * 100);
        if( true_scale_display < g_Show_Target_Name_Scale ) { // from which scale to display name

            wxString tgt_name = td->GetFullName();
            tgt_name = tgt_name.substr( 0, tgt_name.find( _T ( "Unknown" ), 0) );

            if ( tgt_name != wxEmptyString ) {
                dc.SetFont( *FontMgr::Get().GetFont( _( "AIS Target Name" ), 12 ) );
                dc.SetTextForeground( FontMgr::Get().GetFontColor( _( "AIS Target Name" ) ) );

                int w, h;
                dc.GetTextExtent( tgt_name, &w, &h );

                if ( ( td->COG > 90 ) && ( td->COG < 180 ) )
                    dc.DrawText( tgt_name, TargetPoint.x+10, TargetPoint.y-h );
                else
                    dc.DrawText( tgt_name, TargetPoint.x+10, TargetPoint.y+0.5*h );

            } //If name do not empty
        } // if scale
    }

    //  Draw tracks if enabled
    //  Check the Special MMSI Properties array
    bool b_noshow = false;
    bool b_forceshow = false;
    for(unsigned int i=0 ; i < g_MMSI_Props_Array.GetCount() ; i++){
        if(td->MMSI == g_MMSI_Props_Array.Item(i)->MMSI ){
            MMSIProperties *props = g_MMSI_Props_Array.Item(i);
            if( TRACKTYPE_NEVER == props->TrackType){
                b_noshow = true;
                break;
            }
            else if( TRACKTYPE_ALWAYS == props->TrackType){
                b_forceshow = true;
                break;
            }
            else
                break;
        }
    }

    if( (!b_noshow && td->b_show_track) || b_forceshow ) {
        wxColour c = GetGlobalColor( _T ( "CHMGD" ) );
        if(dc.GetDC()) {
            dc.SetPen( wxPen( c, 2 ) );
        } else {
#ifdef ocpnUSE_GL
            glLineWidth(2);
            glColor3ub(c.Red(), c.Green(), c.Blue());
            glBegin(GL_LINE_STRIP);
#endif
        }

        //  create vector of x-y points
        int TrackLength = td->m_ptrack->GetCount();
        if (TrackLength > 1) {
            int TrackPointCount;
            wxPoint *TrackPoints = new wxPoint[TrackLength];
            wxAISTargetTrackListNode *node = td->m_ptrack->GetFirst();
            for (TrackPointCount = 0; node && (TrackPointCount < TrackLength); TrackPointCount++) {
                AISTargetTrackPoint *ptrack_point = node->GetData();
                cc1->GetCanvasPointPix(ptrack_point->m_lat, ptrack_point->m_lon, &TrackPoints[TrackPointCount]);
                node = node->GetNext();
            }
            TrackLength = TrackPointCount;
            if ( dc.GetDC() && (TrackLength > 1) )
                dc.StrokeLines(TrackPointCount, TrackPoints);
#ifdef ocpnUSE_GL
            else
                for (TrackPointCount = 0; TrackPointCount < TrackLength; TrackPointCount++)
                    glVertex2i(TrackPoints[TrackPointCount].x, TrackPoints[TrackPointCount].y);
#endif
            delete[] TrackPoints;
        }

#ifdef ocpnUSE_GL
        if(!dc.GetDC())
            glEnd();
#endif
    }           // Draw tracks
}

void AISDraw( ocpnDC& dc )
{
    if( !g_pAIS ) return;

    // Toggling AIS display on and off
    if( !g_bShowAIS )
        return;//
    //wxArrayInt importancearray; 
    //      Iterate over the AIS Target Hashmap
    AIS_Target_Hash::iterator it;

    AIS_Target_Hash *current_targets = g_pAIS->GetTargetList();
    // init an aray for scaling calculations
    int NoOfElements=(int)g_ShowScaled_Num;
    int* p_Array = NULL;   // Pointer to int, initialize to nothing.
    p_Array = new int[NoOfElements];  // Allocate n ints and save ptr in p_Array.
    for (int i=0; i < NoOfElements; i++) {
        p_Array[i] = 0;}    // Initialize all elements to zero.
    int low=0;
    int temp;
    
    //    Draw all targets in three pass loop, sorted on SOG, GPSGate & DSC on top
    //    This way, fast targets are not obscured by slow/stationary targets
    for( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it ) {
        AIS_Target_Data *td = it->second;
        if( ( td->SOG < g_ShowMoored_Kts )
                && !( ( td->Class == AIS_GPSG_BUDDY ) || ( td->Class == AIS_DSC ) ) ) 
        {
            AISDrawTarget( td, dc );
            if( td->importance > low )
            {
                temp = low; low = 999999;
                for (int i=0; i < NoOfElements; i++) 
                {
                    if ( p_Array[i] == temp ) { p_Array[i] = td->importance; temp=-1; }
                    if ( p_Array[i] < low ) low = p_Array[i];
                }
            }
        }
        
    }
    
    for( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it ) {
        AIS_Target_Data *td = it->second;
        if( ( td->SOG >= g_ShowMoored_Kts )
                && !( ( td->Class == AIS_GPSG_BUDDY ) || ( td->Class == AIS_DSC ) ) )
        {
            AISDrawTarget( td, dc ); // yes this is a doubling of code;(
            if( td->importance > 0 )
            AISDrawTarget( td, dc );
            if( td->importance > low )
            {
                temp = low; low = 999999;
                for (int i=0; i < NoOfElements; i++) 
                {
                    if ( p_Array[i] == temp ) { p_Array[i] = td->importance; temp=-1; }
                    if ( p_Array[i] < low ) low = p_Array[i];
                }
            }
        }           
    }

    for( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it ) {
        AIS_Target_Data *td = it->second;
        if( ( td->Class == AIS_GPSG_BUDDY ) || ( td->Class == AIS_DSC ) ) AISDrawTarget( td, dc );
    }
    ImportanceSwitchPoint = low;
    delete [] p_Array;  // When done, free memory pointed to by p_Array.
    p_Array = NULL; 
}

bool AnyAISTargetsOnscreen( ViewPort &vp )
{
    if( !g_pAIS )
        return false;
    
    if( !g_bShowAIS )
        return false;//
        
    //      Iterate over the AIS Target Hashmap
    AIS_Target_Hash::iterator it;
    AIS_Target_Hash *current_targets = g_pAIS->GetTargetList();
    
    for( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it ) {
        AIS_Target_Data *td = it->second;
        if( vp.GetBBox().Contains( td->Lat,  td->Lon ) )
            return true;                       // yep
    }
    
    return false;
}
