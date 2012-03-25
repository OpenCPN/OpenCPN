/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  AIS Decoder Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   bdbcat@yahoo.com   *
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
 * 2010/02/10 23:00 pjotrc
 * - add pleasure craft type
 *
 * 2010/02/07 12:30 pjotrc
 * - fix NMEA checksum
 * - fix unitialized: ROTAIS, IMO, ShipType
 * - change annot. of s/v
 *
 * 2010/02/01 17:30 pjotrc
 * - implement AIS AtoN
 *
 * 2010/01/31 16:17:00 pjotrc
 * - change condition for skipping moored/anchored targets
 *
 *
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
#include "nmea.h"           // for AIS Muxed data stream
#include "navutil.h"        // for Select
#include "georef.h"
#include "pluginmanager.h"  // for PlugInManager
#include "bitmaps/icons.h" // for sorting icons

extern AISTargetQueryDialog    *g_pais_query_dialog_active;
extern int              g_ais_query_dialog_x, g_ais_query_dialog_y;

extern  OCP_AIS_Thread  *pAIS_Thread;
extern  wxString        *pAISDataSource;
extern  int             s_dns_test_flag;
extern  Select          *pSelectAIS;
extern  double          gLat, gLon, gSog, gCog;
extern  bool            g_bGPSAISMux;
extern FontMgr          *pFontMgr;
extern ChartCanvas      *cc1;
extern MyFrame          *gFrame;
extern MyConfig         *pConfig;

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
extern bool             g_bShowCOG;
extern double           g_ShowCOG_Mins;
extern bool             g_bAISShowTracks;
extern double           g_AISShowTracks_Mins;
extern bool             g_bShowMoored;
extern double           g_ShowMoored_Kts;

extern bool             g_bGPSAISMux;
extern ColorScheme      global_color_scheme;

extern bool             g_bAIS_CPA_Alert;
extern bool             g_bAIS_CPA_Alert_Audio;
extern bool             g_bAIS_CPA_Alert_Suppress_Moored;

extern AISTargetAlertDialog    *g_pais_alert_dialog_active;
extern int              g_ais_alert_dialog_x, g_ais_alert_dialog_y;
extern int              g_ais_alert_dialog_sx, g_ais_alert_dialog_sy;
extern wxString         g_sAIS_Alert_Sound_File;

extern int              g_nNMEADebug;
extern int              g_total_NMEAerror_messages;
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

//    A static structure storing generic position data
//    Used to communicate  AIVDO events to main application loop
static      GenericPosDat     AISPositionData;
extern ComPortManager   *g_pCommMan;



#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)

#endif


#include <wx/listimpl.cpp>
WX_DEFINE_LIST(AISTargetTrackList);

// the first string in this list produces a 6 digit MMSI... BUGBUG

char test_str[24][79] = {
"!AIVDM,1,1,,A,100Rsb@P1PJREovFCnS7n?vt08Ag,0*13**",
"!AIVDM,1,1,,A,15Mnh`PP0jIcl78Csm`hCgvB2D00,0*29**",
"!AIVDM,2,1,1,A,55Mnh`P00001L@7S3GP=Dl8E8h4pB2222222220P0`A6357d07PT851F,0*75**",
"!AIVDM,2,2,1,A,0Dp0jE6H8888880,2*40**",
"!AIVDM,1,1,,A,15Mnh`PP0rIce3VCr8nPWgvt24B0,0*10**",
"!AIVDM,1,1,,A,15Mnh`PP0pIcfJ<Cs0lPFwwR24Bt,0*6B**",
"!AIVDM,1,1,,A,15Mnh`PP0pIcfKBCs23P6gvB2D00,0*7E**",
"!AIVDM,1,1,,A,15Mnh`PP0nIcfL8Cs3DPLwvt28AV,0*5C**",
"!AIVDM,1,1,,A,15Mnh`PP0mIcfSlCs7w@KOwT2L00,0*4E**",
"!AIVDM,1,1,,A,15Mnh`PP0lIcjQRCsOK1LwvB2D00,0*6A**",
"!AIVDM,1,1,,A,15Mnh`PP0mIcj`NCsPNADOvt2@AQ,0*3E**",
"!AIVDM,1,1,,A,15Mnh`PP0mIcjfhCsQLAHOwT24H0,0*2C**",
"!AIVDM,1,1,,A,15Mnh`PP0mIcjlNCsRCiBwvB2@4s,0*4A**",
"!AIVDM,1,1,,A,15Mnh`PP0nIck5@CsUBi2gvB2<00,0*42**",
"!AIVDM,1,1,,A,15Mnh`PP0nIck:PCsVO0uOvt28AQ,0*47**",
"!AIVDM,1,1,,A,15Mnh`PP0oIck?:CsWQi0?wR2L00,0*19**",
"!AIVDM,1,1,,A,15Mnh`PP0oIckRVCscni3gvB24H@,0*29**",
"!AIVDM,1,1,,A,15Mnh`PP0pIck`VCse0i8wvt20Rg,0*48**",
"!AIVDM,1,1,,A,15Mnh`PP0nIckelCsf0hrgwR2D00,0*57**",
"!AIVDM,1,1,,A,15Mnh`PP0mIckiHCsfn@kOvB20S4,0*39**",
"!AIVDM,1,1,,A,15Mnh`PP0nIckmtCsh<0h?vr2HA6,0*05**",
"!AIVDM,1,1,,A,15Mnh`PP0nIckqdCsiC@iOwR2@LP,0*34**",
"!AIVDM,1,1,,A,15Mnh`PP0nIckttCsjEhdgvB2H4m,0*75**",
"!AIVDM,1,1,,A,15Mnh`PP0mIcl0jCskPhiwvr2D00,0*47**",


};


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
      _("Active"),
      _("Testing"),
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
      _("Sailing Vessel"),             //36        6     // pjotrc 2010.02.07
      _("Pleasure craft"),             //37        7     // pjotrc 2010.02.10
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

      _("Unspecified"), 		   //type 0	20  // pjotrc 2010.02.01
      _("Reference Point"),		   //01        21
      _("RACON"),     	               //02        22
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
      _("GpsGate Buddy"),		   //xx	   52
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
      _("Yat"),                  //37        7    // pjotrc 2010.02.10
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

	_("AtoN"),                 //00        20   // pjotrc 2010.02.01
	_("Ref. Pt"),		   //01        21
	_("RACON"),     	         //02        22
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
      _("Buddy"),			   //xx	   52
      _("DSC"),			   //xx        53
      _("Distress")		   //xx        54
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
      tlSOG
};// AISTargetListCtrl Columns;


//    Define and declare a hasmap for ERI Ship type strings, keyed by their UN Codes.
 WX_DECLARE_HASH_MAP( int, wxString, wxIntegerHash, wxIntegerEqual, ERIShipTypeHash );

 ERIShipTypeHash        g_ERI_hash;


//#define AIS_DEBUG  1

wxString trimAISField(char *data)
{
      //  Clip any unused characters (@) from data
/*
      wxString field;
      while((*data) && (*data != '@'))
            field.Append(*data++);
*/
      wxString field = wxString::From8BitData(data);
      while (field.Right(1)=='@' || field.Right(1)==' ')
            field.RemoveLast();
      return field;
}

//------------------------------------------------------------------------------
//    AIS Event Implementation
//------------------------------------------------------------------------------
DEFINE_EVENT_TYPE(wxEVT_OCPN_AIS)


OCPN_AISEvent::OCPN_AISEvent( wxEventType commandType, int id )
      :wxEvent(id, commandType)
{
}


OCPN_AISEvent::~OCPN_AISEvent( )
{
}

wxEvent* OCPN_AISEvent::Clone() const
{
      OCPN_AISEvent *newevent=new OCPN_AISEvent(*this);
      newevent->m_NMEAstring=this->m_NMEAstring.c_str();  // this enforces a deep copy of the string data
      return newevent;
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

    SOG = 555.;
    COG = 666.;
    HDG = 511.;
    ROTAIS = -128;                     // pjotrc 2010.02.07
    Lat = 0.;
    Lon = 0.;

    wxDateTime now = wxDateTime::Now();
    now.MakeGMT();
    PositionReportTicks = now.GetTicks();       // Default is my idea of NOW
    StaticReportTicks = now.GetTicks();
    b_lost = false;

    IMO = 0;                       // pjotrc 2010.02.07
    MID = 555;
    MMSI = 666;
    NavStatus = 15;
    SyncState = 888;
    SlotTO = 999;
    ShipType = 19;	// "Unknown"        // pjotrc 2010.02.10

    CPA = 100;                // Large values avoid false alarms
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
}

AIS_Target_Data::~AIS_Target_Data()
{
      delete m_ptrack;
}

wxString AIS_Target_Data::BuildQueryResult( void )
{

      wxString line;
      wxString result;
      wxDateTime now = wxDateTime::Now();

      if((Class != AIS_BASE) && (Class != AIS_SART) )
      {
            line.Printf(_("Name:  "));
            if(b_nameValid)
            {
                  wxString uret = trimAISField(ShipName);
                  wxString ret;
                  if(uret == _T("Unknown"))
                        ret = wxGetTranslation(uret);
                  else
                        ret = uret;

                  line.Append( ret );

                  if(strlen(ShipNameExtension))
                        line.Append(wxString(ShipNameExtension, wxConvUTF8));

            }
            line.Append(_T("\n\n"));
            result.Append(line);
      }

      if(Class != AIS_GPSG_BUDDY)
      {
            line.Printf(_T("MMSI:                 %09d\n"), abs(MMSI));
            result.Append(line);
      }
      if((Class != AIS_ATON) && (Class != AIS_BASE) && (Class != AIS_GPSG_BUDDY) && (Class != AIS_SART))
      {
            line.Printf(_("CallSign:             "));
            line.Append( trimAISField(CallSign) );
            line.Append(_T("\n"));
            result.Append(line);

            if(Class != AIS_CLASS_B)
            {
                  if(IMO > 0)
                        line.Printf(_("IMO:                 %8d\n"), IMO);
                  else
                        line.Printf(_("IMO:\n"));
                  result.Append(line);
            }
      }

      line.Printf(_("Class:                "));
      line.Append( wxGetTranslation(Get_class_string(false) ));
      line.Append(_T("\n"));
      result.Append(line);

      if( (Class != AIS_BASE) && (Class != AIS_CLASS_B) )
      {
            line.Printf(_("Navigational Status:  "));
            if((NavStatus <= 21) && (NavStatus >= 0))
                  line.Append(  wxGetTranslation(ais_status[NavStatus]) );
            line.Append(_T("\n"));
            result.Append(line);
      }

      if( Class == AIS_SART )
      {
            line.Printf(_("Type:                 "));
            int mmsi_start =  MMSI / 1000000;
            wxString sub_type;
            switch(mmsi_start)
            {
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

            if(sub_type.Len())
            {
                  line.Append(sub_type);
                  line.Append(_T("\n"));
                  result.Append(line);
            }

            result.Append(_T("\n"));
      }


      if( (Class != AIS_BASE) && (Class != AIS_SART) )
      {

      //      Ship type
            line.Printf(_("Type:                 "));
            line.Append(  wxGetTranslation(Get_vessel_type_string() ));
            line.Append(_T("\n"));
            result.Append(line);

            if(b_isEuroInland && UN_shiptype)
            {

                  line.Printf(_("UN Ship Type:         "));
                  ERIShipTypeHash::iterator it = g_ERI_hash.find(UN_shiptype);
                  wxString type;
                  if(it == g_ERI_hash.end())
                        type =_("Undefined");
                  else
                        type = it->second;

                  wxString type_t;
                  type_t =  wxGetTranslation(type);
                  if(type_t.Len() < 20)
                        line.Append(type_t);
                  else
                  {
                        line.Append(_T("\n  "));
                        line.Append(type_t);
                  }

                  line.Append(_T("\n"));
                  result.Append(line);
            }


      //  Dimensions

            if((DimA + DimB + DimC + DimD) == 0)
            {
                  if(b_isEuroInland)
                  {
                        if(Euro_Length == 0.0)
                        {
                              if(Euro_Draft > 0.01)
                                    line.Printf(_("Size:                 ---m x ---m x %4.1fm\n\n"),  Euro_Draft);
                              else
                                    line.Printf(_("Size:                 ---m x ---m x ---m\n\n"));
                        }
                        else
                        {
                              if(Euro_Draft > 0.01)
                                    line.Printf(_("Size:               %5.1fm x %4.1fm x %4.1fm\n\n"), Euro_Length, Euro_Beam, Euro_Draft);
                              else
                                    line.Printf(_("Size:               %5.1fm x %4.1fm x ---m\n\n"), Euro_Length, Euro_Beam);
                        }
                  }
                  else
                  {
                        if(Draft > 0.01)
                              line.Printf(_("Size:                 ---m x ---m x %4.1fm\n\n"),  Draft);
                        else
                              line.Printf(_("Size:                 ---m x ---m x ---m\n\n"));
                  }
            }
            else if(Draft < 0.01)
                  line.Printf(_("Size:                 %dm x %dm x ---m\n\n"), (DimA + DimB), (DimC + DimD));
            else
                  line.Printf(_("Size:                 %dm x %dm x %4.1fm\n\n"), (DimA + DimB), (DimC + DimD), Draft);

            if((Class == AIS_CLASS_B) || (Class == AIS_ATON))
                  line.Printf(_("Size:                 %dm x %dm\n\n"), (DimA + DimB), (DimC + DimD));

            if (NavStatus != ATON_VIRTUAL)
                  result.Append(line);

      }

      if (Class == AIS_GPSG_BUDDY) {
            line.Printf(_("Report as of: %d:%d UTC "), m_utc_hour, m_utc_min);
            line.Append(_T("\n"));
            result.Append(line);
      } else {
      if (Class == AIS_CLASS_A )
      {
            line.Printf(_("Destination:          "));
            line.Append( trimAISField(Destination) );
            line.Append(_T("\n"));
            result.Append(line);
                         //  ETA
            if((ETA_Mo) && (ETA_Hr < 24))
            {
                  wxDateTime eta(ETA_Day, wxDateTime::Month(ETA_Mo-1), now.GetYear(), ETA_Hr, ETA_Min);
                  line.Printf(_("ETA:                  "));
                  line.Append( eta.Format(_T("%b %d")));
                  line.Append(_T(" "));
                  line.Append( eta.FormatISOTime());
                  line.Append(_T("\n"));
            }
            else
            {
                  line.Printf(_("ETA:"));
                  line.Append(_T("\n"));
            }

            result.Append(line);
            result.Append(_T("\n"));
      }

      if ((Class == AIS_CLASS_A) || (Class == AIS_CLASS_B))
      {
            int crs = wxRound(COG);
            if(crs < 360)
                  line.Printf(_("Course:                 %03d Deg.\n"), crs);
            else if(COG == 360.0)
                  line.Printf(_("Course:                 Unavailable\n"));
            else if(crs == 360)
                  line.Printf(_("Course:                 000 Deg.\n"));
            result.Append(line);

            if(SOG <= 102.2)
                  line.Printf(_("Speed:                %5.2f Kts.\n"), SOG);
            else
                  line.Printf(_("Speed:                  Unavailable\n"));
            result.Append(line);

            if((int)HDG != 511)
            {
                  line.Printf(_("Heading:                %03d Deg.\n"), (int)HDG);
                  result.Append(line);
            }

            if(ROTAIS != -128)
            {
                  if(ROTAIS == 127)
                        line.Printf(_("Rate Of Turn:       > 5 Deg./30 s. Right\n"));
                  else if(ROTAIS == -127)
                        line.Printf(_("Rate Of Turn:       > 5 Deg./30 s. Left\n"));
                  else
                  {
                        if(ROTIND > 0)
                              line.Printf(_("Rate Of Turn            %3d Deg./Min. Right\n"), ROTIND);
                        else if(ROTIND < 0)
                              line.Printf(_("Rate Of Turn            %3d Deg./Min. Left\n"), abs(ROTIND));
                        else
                              line.Printf(_("Rate Of Turn            0\n"));
                  }

                  result.Append(line);
                  }
            }
      }

      if(b_positionOnceValid && bGPSValid && (Range_NM >= 0.))
                  line.Printf(_("Range:                %5.1f NM\n"), Range_NM);
      else
            line.Printf(_("Range:                  Unavailable\n"));
      result.Append(line);

      int brg = (int)wxRound(Brg);
      if(Brg > 359.5)
            brg = 0;
      if(b_positionOnceValid && bGPSValid && (Brg >= 0.) && (Range_NM > 0.))
            line.Printf(_("Bearing:                %03d Deg.\n"), brg);
      else
            line.Printf(_("Bearing:                Unavailable\n"));
      result.Append(line);

      if(Class != AIS_BASE)
      {
            if(blue_paddle == 1)
            {
                  line.Printf(_("Inland Blue Flag        Clear\n"));
                  result.Append(line);
            }
            else if(blue_paddle == 2)
            {
                  line.Printf(_("Inland Blue Flag        Set\n"));
                  result.Append(line);
            }
      }


      line.Printf(_("Position"));
      if(b_positionOnceValid)
      {
            if(b_positionDoubtful)
                  line << _(" (Last Known):  ");
            else
                  line << _(":               ");

            wxString pos_st;
            pos_st += toSDMM(1, Lat);
            pos_st <<_T("\n                        ");
            pos_st += toSDMM(2, Lon);
            pos_st << _T("\n\n");
            line << pos_st;
      }
      else
            line << _(":               Unavailable\n\n\n");
      result.Append(line);


      wxDateTime rt(PositionReportTicks);
      line.Printf(_("Last Position Report Time:  "));
      line << rt.FormatISOTime();
      line << _T(" ");
      line << _("UTC");
      line << _T("\n");
      result.Append(line);


      now.MakeGMT();
      int target_age = now.GetTicks() - PositionReportTicks;


      line.Printf(_("Position Report Age:       %3d Sec.\n\n"), target_age);
      result.Append(line);

 //     line.Printf(_("Recent Report Period:   %3d Sec.\n\n"), RecentPeriod);
 //     result.Append(line);

      double hours = floor(TCPA / 60.);
      double mins = TCPA - (hours * 60);

      if(bCPA_Valid)
            line.Printf(_("TCPA:                 %02d:%02d Hr:Min\n"), (int)hours, (int)mins);
      else
            line.Printf(_("TCPA:  \n"));
      result.Append(line);

      if(bCPA_Valid)
            line.Printf(_("CPA:                 %6.1f NM\n"), CPA);
      else
            line.Printf(_("CPA:       \n"));
      result.Append(line);
/*
      //    Count lines and characters
      wxString max_line;
      int nl = 0;
      unsigned int max_len = 0;
      int max_pix = 0;

      if(pn_nl)
      {
            unsigned int i = 0;
            wxString rline;

            while(i < result.Len())
            {
                  while(result.GetChar(i) != '\n')
                        rline.Append(result.GetChar(i++));

                  if(rline.Len() > max_len)
                  {
                        max_line = rline;
                        max_len = rline.Len();
                  }

                  if(pFont && ppix_size)              // measure this line exactly
                  {
                        int w, h;
                        pdc->GetTextExtent(rline, &w, &h, NULL, NULL, pFont);
                        if(w > max_pix)
                              max_pix = w;
                  }


                  i++;              // skip nl
                  nl++;
                  rline.Clear();
            }

            *pn_nl = nl;
            if(pn_cmax)
                  *pn_cmax = max_len;
      }


      //    Measurement requested?
      if(pFont && ppix_size)
      {
            int w, h;
            pdc->GetTextExtent(max_line, &w, &h, NULL, NULL, pFont);

            ppix_size->x = max_pix;       // x comes from above
            ppix_size->y = h * nl;        // y is the same for all
      }

*/
      return result;
}

wxString AIS_Target_Data::GetRolloverString( void )
{
      wxString result;
      wxString t;
      if(b_nameValid)
      {
            result.Append(_T("\""));
            wxString uret = trimAISField(ShipName);
            wxString ret;
            if(uret == _T("Unknown"))
                  ret = wxGetTranslation(uret);
            else
                  ret = uret;

            result.Append(ret);
            if(strlen(ShipNameExtension))
                  result.Append(wxString(ShipNameExtension, wxConvUTF8));

            result.Append(_T("\" "));
      }
      if (Class != AIS_GPSG_BUDDY)
      {
            t.Printf(_T("%09d"), abs(MMSI));
            result.Append(t);
      }
      t = trimAISField(CallSign);
      if (t.Len())
      {
            result.Append(_T(" ("));
            result.Append(t);
            result.Append(_T(")"));
      }
      if (g_bAISRolloverShowClass || (Class == AIS_SART) )
      {
            if (result.Len())
                  result.Append(_T("\n"));
            result.Append(_T("["));
            result.Append(wxGetTranslation(Get_class_string(false)));
            result.Append(_T("] "));
            if((Class != AIS_ATON) && (Class != AIS_BASE))
            {
                  if(Class == AIS_SART)
                  {
                        int mmsi_start =  MMSI / 1000000;
                        switch(mmsi_start)
                        {
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


                  if(Class != AIS_SART)
                        result.Append(wxGetTranslation(Get_vessel_type_string(false)));

                  if(Class != AIS_CLASS_B)
                  {
                        if((NavStatus <= 15) && (NavStatus >= 0) )
                        {
                              result.Append(_T(" ("));
                              result.Append(wxGetTranslation(ais_status[NavStatus]));
                              result.Append(_T(")"));
                        }
                  }
            }
      }
      if(g_bAISRolloverShowCOG && (SOG <= 102.2) && ((Class != AIS_ATON) && (Class != AIS_BASE)))
      {
            if (result.Len())
                  result.Append(_T("\n"));
            t.Printf(_T("SOG: %.2fKts"), SOG); result.Append(t);
            result.Append(_T(" "));

            int crs = wxRound(COG);
            if(b_positionOnceValid)
            {
                  if(crs < 360)
                        t.Printf(_(" COG: %03d Deg."), crs);
                  else if(COG == 360.0)
                        t.Printf(_(" COG: Unavailable"));
                  else if(crs == 360)
                        t.Printf(_(" COG: 000 Deg."));
            }
            else
                  t.Printf(_(" COG: Unavailable"));
            result.Append(t);

      }
      if (g_bAISRolloverShowCPA && bCPA_Valid)
      {
            if (result.Len())
                  result.Append(_T("\n"));
            result.Append(_("CPA"));
            result.Append(_T(" "));
            t.Printf(_T("%.2fNm"), CPA); result.Append(t);
            result.Append(_T(" "));
            t.Printf(_T("%.0fMins"), TCPA); result.Append(t);
      }
      return result;
}

wxString AIS_Target_Data::Get_vessel_type_string(bool b_short)
{
      int i=19;                            // pjotrc 2010.02.10 renumber
      if (Class == AIS_ATON)               // pjotrc 2010.02.01
	{
		i = ShipType + 20 ;
	}
	else
	switch(ShipType)
      {
            case 30:
                  i=0; break;
            case 31:
                  i=1; break;
            case 32:
                  i=2; break;
            case 33:
                  i=3; break;
            case 34:
                  i=4; break;
            case 35:
                  i=5; break;
            case 36:
                  i=6; break;
            case 37:
                  i=7; break;
            case 50:
                  i=9; break;
            case 51:
                  i=10; break;
            case 52:
                  i=11; break;
            case 53:
                  i=12; break;
            case 54:
                  i=13; break;
            case 55:
                  i=14; break;
            case 58:
                  i=15; break;
            default:
                  i=19; break;
      }

      if ((Class == AIS_CLASS_B) || (Class == AIS_CLASS_A) )    // pjotrc 2010.02.07
	{
      if((ShipType >= 40) && (ShipType < 50))
            i=8;

      if((ShipType >= 60) && (ShipType < 70))
            i=16;

      if((ShipType >= 70) && (ShipType < 80))
            i=17;

      if((ShipType >= 80) && (ShipType < 90))
            i=18;
	}
      else if (Class == AIS_GPSG_BUDDY)
            i = 52;
      else if (Class == AIS_DSC)
            i = (ShipType==12)? 54 : 53;  // 12 is distress

      if(!b_short)
            return ais_type[i];
      else
            return short_ais_type[i];
}

wxString AIS_Target_Data::Get_class_string(bool b_short)
{
      switch (Class)
      {
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


//---------------------------------------------------------------------------------
//
//  AIS_Decoder Helpers
//
//---------------------------------------------------------------------------------
AIS_Bitstring::AIS_Bitstring(const char *str)
{
    byte_length = strlen(str);

    for(int i=0 ; i<byte_length ; i++)
    {
        bitbytes[i] = to_6bit(str[i]);
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


int AIS_Bitstring::GetInt(int sp, int len)
{
    int acc = 0;
    int s0p = sp-1;                          // to zero base

    int cp, cx, c0, cs;


    for(int i=0 ; i<len ; i++)
    {
        acc  = acc << 1;
        cp = (s0p + i) / 6;
        cx = bitbytes[cp];		// what if cp >= byte_length?
        cs = 5 - ((s0p + i) % 6);
        c0 = (cx >> (5 - ((s0p + i) % 6))) & 1;
        acc |= c0;
    }

    return acc;

}

bool AIS_Bitstring::GetStr(int sp, int len, char *dest, int max_len)
{
    char temp_str[85];

    char acc = 0;
    int s0p = sp-1;                          // to zero base

    int k=0;
    int cp, cx, c0, cs;

    int i = 0;
    while(i < len)
    {
         acc=0;
         for(int j=0 ; j<6 ; j++)
         {
            acc  = acc << 1;
            cp = (s0p + i) / 6;
            cx = bitbytes[cp];		// what if cp >= byte_length?
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

    return true;
}




//---------------------------------------------------------------------------------
//
//  AIS_Decoder Implementation
//
//---------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(AIS_Decoder, wxEvtHandler)

//  EVT_SOCKET(AIS_SOCKET_ID, AIS_Decoder::OnSocketEvent)
  EVT_TIMER(TIMER_AIS1, AIS_Decoder::OnTimerAIS)
  EVT_TIMER(TIMER_AISAUDIO, AIS_Decoder::OnTimerAISAudio)
//  EVT_COMMAND(ID_AIS_WINDOW, EVT_AIS, AIS_Decoder::OnEvtAIS)

  END_EVENT_TABLE()

static int n_msgs;
static int n_msg1;
static int n_msg5;
static int n_msg24;
static int n_newname;
static bool b_firstrx;
static int first_rx_ticks;
static int rx_ticks;



AIS_Decoder::AIS_Decoder(int handler_id, wxFrame *pParent, const wxString& AISDataSource, wxMutex *pGPSMutex)

{
      m_handler_id = handler_id;

      AISTargetList = new AIS_Target_Hash;

      BuildERIShipTypeHash();

      m_pShareGPSMutex = pGPSMutex;

      m_parent_frame = pParent;

      m_pMainEventHandler = pParent->GetEventHandler();

      g_pais_alert_dialog_active = NULL;
      m_bAIS_Audio_Alert_On = false;

      m_n_targets = 0;

      OpenDataSource(pParent, AISDataSource);

      //  Create/connect a dynamic event handler slot for OCPN_AISEvent(s) coming from AIS thread
      Connect(wxEVT_OCPN_AIS, (wxObjectEventFunction)(wxEventFunction)&AIS_Decoder::OnEvtAIS);

}

AIS_Decoder::~AIS_Decoder(void)
{
      if(pAIS_Thread)
      {
            wxLogMessage(_T("Stopping AIS Secondary Thread"));

            m_Thread_run_flag = 0;
            int tsec = 5;
            while((m_Thread_run_flag >= 0) && (tsec--))
            {
                  wxSleep(1);
            }

            wxString msg;
            if(m_Thread_run_flag < 0)
                  msg.Printf(_T("Stopped in %d sec."), 5 - tsec);
            else
                  msg.Printf(_T("Not Stopped after 5 sec."));
            wxLogMessage(msg);

            pAIS_Thread = NULL;
      }


    AIS_Target_Hash::iterator it;
    AIS_Target_Hash *current_targets = GetTargetList();

    for( it = (*current_targets).begin(); it != (*current_targets).end(); ++it )
    {
          AIS_Target_Data *td = it->second;

          delete td;
    }

    delete current_targets;

#ifndef OCPN_NO_SOCKETS
    //    Kill off the TCP/IP Socket if alive
    if(m_sock)
    {
          m_sock->Notify(FALSE);
          m_sock->Destroy();
          TimerAIS.Stop();
    }
#endif

    wxDateTime now = wxDateTime::Now();
    now.MakeGMT();

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
//     Handle events from AIS Serial Port RX thread
//----------------------------------------------------------------------------------
void AIS_Decoder::OnEvtAIS(OCPN_AISEvent& event)
{
    switch(event.GetExtraLong())
    {
        case EVT_AIS_PARSE_RX:
        {
//              wxDateTime now = wxDateTime::Now();
//              printf("AIS Event at %ld\n", now.GetTicks());

            wxString message = event.GetNMEAString();

            int nr = 0;
            if(!message.IsEmpty())
            {
                  if(g_NMEALogWindow)
                  {
                        wxDateTime now = wxDateTime::Now();
                        wxString ss = now.FormatISOTime();
                        ss.Append(_T("  "));
                        ss.Append(message);
                        g_NMEALogWindow->Add(ss);
                        g_NMEALogWindow->Refresh(false);
                  }


                  if( message.Mid(3,3).IsSameAs(_T("VDM")) || message.Mid(3,3).IsSameAs(_T("VDO")) || message.Mid(1,5).IsSameAs(_T("FRPOS")) || message.Mid(1,2).IsSameAs(_T("CD")) )
                  {
                        nr = Decode(message);
                        if(message.Mid(3,3).IsSameAs(_T("VDO")))
                        {
                              //    This is an ownship message, presumably from a transponder
                              //    Simulate an ownship GPS position report upstream for message IDs 1,2,3

                              if(m_pLatestTargetData && (nr == AIS_NoError) && g_bGPSAISMux && !m_pLatestTargetData->b_positionDoubtful && m_pLatestTargetData->b_positionOnceValid)
                              {
                                    switch(m_pLatestTargetData->MID)
                                    {
                                          case 1:
                                          case 2:
                                          case 3:
                                          {
                                                AISPositionData.kLat = m_pLatestTargetData->Lat;
                                                AISPositionData.kLon = m_pLatestTargetData->Lon;

                                                if(m_pLatestTargetData->COG == 360.0)
                                                      AISPositionData.kCog = NAN;
                                                else
                                                      AISPositionData.kCog = m_pLatestTargetData->COG;


                                                if(m_pLatestTargetData->SOG > 102.2)
                                                      AISPositionData.kSog = NAN;
                                                else
                                                      AISPositionData.kSog = m_pLatestTargetData->SOG;

                                                wxCommandEvent event( EVT_NMEA,  m_handler_id );
                                                event.SetEventObject( (wxObject *)this );
                                                event.SetExtraLong(EVT_NMEA_DIRECT);
                                                event.SetClientData(&AISPositionData);
                                                m_pMainEventHandler->AddPendingEvent(event);
                                                break;
                                          }
                                          default:
                                                break;
                                    }
                              }
                        }
                        g_pi_manager->SendAISSentenceToAllPlugIns(message);
                  }
                  else
                  {
                        if(g_bGPSAISMux)
                              Parse_And_Send_Posn(message);
                  }
            }
            break;
        }       //case
    }           // switch
}

void AIS_Decoder::ThreadMessage(const wxString &msg)
{

    //    Signal the main program thread
      wxCommandEvent event( EVT_THREADMSG);
      event.SetEventObject( (wxObject *)this );
      event.SetString(msg);
      m_pMainEventHandler->AddPendingEvent(event);

}


void AIS_Decoder::Parse_And_Send_Posn(wxString &message)
{
      if( g_nNMEADebug && (g_total_NMEAerror_messages < g_nNMEADebug) )
      {
            g_total_NMEAerror_messages++;
            wxString msg(_T("AIS.NMEA Sentence received..."));
            msg.Append(message);
            ThreadMessage(msg);
      }

      OCPN_NMEAEvent Nevent(wxEVT_OCPN_NMEA, 0);
      Nevent.SetNMEAString(message);
      m_pMainEventHandler->AddPendingEvent(Nevent);

      return;

}








//----------------------------------------------------------------------------------
//      Decode NMEA VDM/VDO/FRPOS/DSCDSE sentence to AIS Target(s)
//----------------------------------------------------------------------------------
AIS_Error AIS_Decoder::Decode(const wxString& str)
{
    AIS_Error ret;
    wxString string_to_parse;

    double gpsg_lat, gpsg_lon, gpsg_mins, gpsg_degs;
    double  gpsg_cog, gpsg_sog, gpsg_utc_time;
    int gpsg_utc_hour;
    int gpsg_utc_min;
    int gpsg_utc_sec;
    char gpsg_name_str[21];

    double dsc_lat, dsc_lon, dsc_mins, dsc_degs, dsc_tmp, dsc_addr;
    double dse_tmp, dse_lat, dse_lon, dse_addr;
    long dsc_fmt, dsc_quadrant;

    int gpsg_mmsi = 0;
    int dsc_mmsi = 0;
    int dse_mmsi = 0;
    int mmsi = 0;

    //  Make some simple tests for validity

//    int nlen = str.Len();

    if(str.Len() > 100)
        return AIS_NMEAVDX_TOO_LONG;

    if(!NMEACheckSumOK(str))
    {
//          printf("Checksum error at n_msgs:%d\n", n_msgs);

            if( g_nNMEADebug && (g_total_NMEAerror_messages < g_nNMEADebug) )   // pjotrc 2010.02.07
            {
                  g_total_NMEAerror_messages++;
                  wxString msg(_T("   AIS checksum bad, continuing..."));
                  msg.Append(str);
                  ThreadMessage(msg);
            }
            else
            return AIS_NMEAVDX_CHECKSUM_BAD;
    }
    if (str.Mid(1,2).IsSameAs(_T("CD")) )
    {
          // parse a DSC Position message			$CDDSx,.....

          //  Use a tokenizer to pull out the first 9 fields
          if( g_nNMEADebug && (g_total_NMEAerror_messages < g_nNMEADebug) )
          {
                g_total_NMEAerror_messages++;
                wxString msg(_T("CDDSx Sentence received..."));
                ThreadMessage(msg);
          }

          wxString string(str);
          wxStringTokenizer tkz(string, _T(",*"));

          wxString token;
          token = tkz.GetNextToken();         // !$CDDS

          if (str.Mid(3,3).IsSameAs(_T("DSC")) ){
                token = tkz.GetNextToken();         // format specifier (02-area,12-distress,16-allships,20-individual,...)
                token.ToLong(&dsc_fmt);

                token = tkz.GetNextToken();         // address i.e. mmsi*10 for received msg, or area spec
                token.ToDouble(&dsc_addr);
                dsc_mmsi = 0 - (int) (dsc_addr/10);				// as per NMEA 0183 3.01

                token = tkz.GetNextToken();         // category
                token = tkz.GetNextToken();         // nature of distress or telecommand1
                token = tkz.GetNextToken();         // comm type or telecommand2

                token = tkz.GetNextToken();         // position or channel/freq
                token.ToDouble(&dsc_tmp);

                token = tkz.GetNextToken();         // time or tel. no.
                token = tkz.GetNextToken();         // mmsi of ship in distress
                token = tkz.GetNextToken();         // nature of distress
                token = tkz.GetNextToken();         // acknowledgement
                token = tkz.GetNextToken();         // expansion indicator

                dsc_quadrant = (int) dsc_tmp / 1000000000.0;

                dsc_lat = (int) (dsc_tmp / 100000.0);
                dsc_lon = dsc_tmp - dsc_lat*100000.0;
                dsc_lat = dsc_lat - dsc_quadrant*10000;
                dsc_degs = (int) (dsc_lat / 100.0);
                dsc_mins = dsc_lat - dsc_degs*100.0;
                dsc_lat = dsc_degs + dsc_mins/60.0;

                dsc_degs = (int) (dsc_lon / 100.0);
                dsc_mins = dsc_lon - dsc_degs*100.0;
                dsc_lon = dsc_degs + dsc_mins/60.0;
                switch (dsc_quadrant)
                {
                  case 0: break;							// NE
                  case 1: dsc_lon = 0-dsc_lon; break;		// NW
                  case 3: dsc_lon = 0-dsc_lon;			// SW
                  case 2: dsc_lat = 0-dsc_lat;			// SE
                  default: break;
                }
                if (dsc_fmt != 02)
                      mmsi = (int) dsc_mmsi;
          }
          else if (str.Mid(3,3).IsSameAs(_T("DSE")) ) {

                token = tkz.GetNextToken();         // total number of sentences

                token = tkz.GetNextToken();         // sentence number

                token = tkz.GetNextToken();         // query/rely flag

                token = tkz.GetNextToken();         // vessel MMSI
                token.ToDouble(&dse_addr);
                dse_mmsi = 0 - (int) (dse_addr/10);				// as per NMEA 0183 3.01

                token = tkz.GetNextToken();         // code field

                token = tkz.GetNextToken();         // data field - position - 2*4 digits latlon .mins
                token.ToDouble(&dse_tmp);
                dse_lat = (int) (dse_tmp / 10000.0);
                dse_lon = (int) (dse_tmp - dse_lat*10000.0);
                dse_lat = dse_lat / 600000.0;
                dse_lon = dse_lon / 600000.0;

                mmsi = (int) dse_mmsi;
          }
      }
      else if (str.Mid(1,5).IsSameAs(_T("FRPOS")) )
      {
      // parse a GpsGate Position message			$FRPOS,.....

      //  Use a tokenizer to pull out the first 9 fields
            if( g_nNMEADebug && (g_total_NMEAerror_messages < g_nNMEADebug) )
            {
                  g_total_NMEAerror_messages++;
                  wxString msg(_T("AIS.FRPOS Sentence received..."));
                  ThreadMessage(msg);
            }

            wxString string(str);
            wxStringTokenizer tkz(string, _T(",*"));

            wxString token;
            token = tkz.GetNextToken();         // !$FRPOS

            token = tkz.GetNextToken();			//	latitude DDMM.MMMM
            token.ToDouble(&gpsg_lat);
            gpsg_degs = (int) (gpsg_lat / 100.0);
            gpsg_mins = gpsg_lat - gpsg_degs * 100.0;
            gpsg_lat = gpsg_degs + gpsg_mins/60.0;

            token = tkz.GetNextToken();			//  hemisphere N or S
            if (token.Mid(1,1).Contains(_T("Ss")))
                  gpsg_lat = 0 - gpsg_lat;

            token = tkz.GetNextToken();			// longitude DDDMM.MMMM
            token.ToDouble(&gpsg_lon);
            gpsg_degs = (int)(gpsg_lon / 100.0);
            gpsg_mins = gpsg_lon - gpsg_degs * 100.0;
            gpsg_lon = gpsg_degs + gpsg_mins/60.0;

            token = tkz.GetNextToken();			// hemisphere E or W
            if (token.Mid(1,1).Contains(_T("Ww")))
                  gpsg_lon = 0 - gpsg_lon;

            token = tkz.GetNextToken();			//	altitude AA.a
            //    token.toDouble(&gpsg_alt);

            token = tkz.GetNextToken();			//  speed over ground SSS.SS knots
            token.ToDouble(&gpsg_sog);

            token = tkz.GetNextToken();			//  heading over ground HHH.hh degrees
            token.ToDouble(&gpsg_cog);

            token = tkz.GetNextToken();			// date DDMMYY

            token = tkz.GetNextToken();			// time UTC hhmmss.dd
            token.ToDouble(&gpsg_utc_time);
            gpsg_utc_hour = (int) (gpsg_utc_time / 10000.0);
            gpsg_utc_min = (int)(gpsg_utc_time/100.0) - gpsg_utc_hour*100;
            gpsg_utc_sec = (int) gpsg_utc_time - gpsg_utc_hour*10000 - gpsg_utc_min*100;

            // now comes the name, followed by in * and NMEA checksum

            token = tkz.GetNextToken();
            int i, len, hash = 0;
            len = wxMin(wxStrlen(token),20);
            strncpy(gpsg_name_str,token.mb_str(),len);
            gpsg_name_str[len] = 0;
            for (i = 0; i<len; i++) {
		            hash = hash * 10;
		            hash += (int)(token[i]);
		            while (hash >= 100000)
                              hash = hash / 100000;
            }
            gpsg_mmsi = 199000000 + hash;  // 199 is INMARSAT-A MID, should not occur ever in AIS stream
            mmsi = gpsg_mmsi;
    }
    else if(!str.Mid(3,2).IsSameAs(_T("VD")))
    {
          return AIS_NMEAVDX_BAD;
    }

    //  OK, looks like the sentence is OK

    //  Use a tokenizer to pull out the first 4 fields
    wxString string(str);
    wxStringTokenizer tkz(string, _T(","));

    wxString token;
    token = tkz.GetNextToken();         // !xxVDx

    token = tkz.GetNextToken();
    nsentences = atoi(token.mb_str());

    token = tkz.GetNextToken();
    isentence = atoi(token.mb_str());

    token = tkz.GetNextToken();
    long lsequence_id = 0;
    token.ToLong(&lsequence_id);

    token = tkz.GetNextToken();
    long lchannel;
    token.ToLong(&lchannel);

    //  Now, some decisions

    string_to_parse.Clear();

    //  Simple case first
    //  First and only part of a one-part sentence
    if((1 == nsentences) && (1 == isentence))
    {
        string_to_parse = tkz.GetNextToken();         // the encapsulated data
    }

    else if(nsentences > 1)
    {
        if(1 == isentence)
        {
            sentence_accumulator = tkz.GetNextToken();         // the encapsulated data
        }

        else
        {
            sentence_accumulator += tkz.GetNextToken();
        }

        if(isentence == nsentences)
        {
            string_to_parse = sentence_accumulator;
        }
     }


     if ( mmsi || (!string_to_parse.IsEmpty() && (string_to_parse.Len() < AIS_MAX_MESSAGE_LEN)) )
     {

        //  Create the bit accessible string
        AIS_Bitstring strbit(string_to_parse.mb_str());

        //  Extract the MMSI
        if (!mmsi)
              mmsi = strbit.GetInt(9, 30);
        long mmsi_long = mmsi;

        //  Here is some debug code to capture/filter to on MMSI number
//        if(mmsi != 244670456)
//             return AIS_NoError;

//        int message_ID = strbit.GetInt(1, 6);

        AIS_Target_Data *pTargetData;
        AIS_Target_Data *pStaleTarget = NULL;
        bool bnewtarget = false;

        //  Search the current AISTargetList for an MMSI match
        AIS_Target_Hash::iterator it = AISTargetList->find( mmsi );
        if(it == AISTargetList->end())                  // not found
        {
              pTargetData = new AIS_Target_Data;
              bnewtarget = true;
              m_n_targets++;
        }
        else
        {
              pTargetData = (*AISTargetList)[mmsi];          // find current entry
              pStaleTarget = pTargetData;                   // save a pointer to stale data
        }

        //  Grab the stale targets's last report time
        int last_report_ticks;

        wxDateTime now = wxDateTime::Now();
        now.MakeGMT();

        if(pStaleTarget)
              last_report_ticks = pStaleTarget->PositionReportTicks;
        else
              last_report_ticks = now.GetTicks();

        // Delete the stale AIS Target selectable point if not a CDDSE
        if(pStaleTarget && !dse_mmsi)
            pSelectAIS->DeleteSelectablePoint((void *)mmsi_long, SELTYPE_AISTARGET);

        bool bhad_name = false;
        if(pStaleTarget)
            bhad_name =  pStaleTarget->b_nameValid;

        bool bdecode_result = false; // for CDDSE assume target is there
            if (dse_mmsi)
                  bdecode_result = true;

            if (dse_mmsi && !pTargetData->b_nameValid && pTargetData->b_positionOnceValid && ((now.GetTicks() - pTargetData->PositionReportTicks)) < 20) {  // ignore stray CDDSE sentences
	            pTargetData->Lat = pTargetData->Lat +((pTargetData->Lat)>=0 ? dse_lat : -dse_lat);
	            pTargetData->Lon = pTargetData->Lon +((pTargetData->Lon)>=0 ? dse_lon : -dse_lon);
	            pTargetData->b_nameValid = true;
	            }
            else
            if (dsc_mmsi) {
	            pTargetData->PositionReportTicks = now.GetTicks();
                  pTargetData->StaticReportTicks = now.GetTicks();

	            pTargetData->MMSI = mmsi;
	            pTargetData->NavStatus = 0; // underway
	            pTargetData->Lat = dsc_lat;
	            pTargetData->Lon = dsc_lon;
	            pTargetData->b_positionOnceValid = true;	// need to cheat here, since nothing would be shown
	            pTargetData->COG = 0;
	            pTargetData->SOG = 0;
	            pTargetData->ShipType = dsc_fmt; // DSC report
	            pTargetData->Class = AIS_DSC;
	            if (dsc_fmt==12) strncpy(pTargetData->ShipName, "DISTRESS            ", 21);
		            else strncpy(pTargetData->ShipName, "POSITION REPORT     ", 21);
	            pTargetData->b_nameValid = false;       // continue cheating, because position maybe incomplete
	            pTargetData->b_active = true;
                  pTargetData->b_lost = false;

	            bdecode_result = true;
	            }
            else if (gpsg_mmsi) {
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
			strncpy(pTargetData->ShipName, gpsg_name_str, strlen(gpsg_name_str)+1);
			pTargetData->b_nameValid = true;
			pTargetData->b_active = true;
                  pTargetData->b_lost = false;

                  bdecode_result = true;
			}
            else
                  bdecode_result = Parse_VDXBitstring(&strbit, pTargetData);            // Parse the new data

            if( g_nNMEADebug && (g_total_NMEAerror_messages < g_nNMEADebug) ) // debug pjotrc
            {
                  g_total_NMEAerror_messages++;
                  wxString msg(_T("   AIS Target data..."));
	            wxString item;
                  item.Printf(_T("MMSI: %d LAT %10.5f LON %10.5f"), pTargetData->MMSI, pTargetData->Lat, pTargetData->Lon);
                  msg.Append(item);
                  ThreadMessage(msg);
            }

        //  pTargetData is valid, either new or existing. Continue processing

        m_pLatestTargetData = pTargetData;

        if(str.Mid(3,3).IsSameAs(_T("VDO")))
              pTargetData->b_OwnShip = true;

        if((bdecode_result) && (pTargetData->b_nameValid) && (pStaleTarget))
           if(!bhad_name)
                    n_newname++;

        //  If the message was decoded correctly
        //  Update the AIS Target information
        if(bdecode_result)
        {
              (*AISTargetList)[mmsi] = pTargetData;            // update the hash table entry

              //     Update the most recent report period
              if (!dse_mmsi)
                    pTargetData->RecentPeriod = pTargetData->PositionReportTicks - last_report_ticks;

              //  If this is not an ownship message, update the AIS Target in the Selectable list, and update the CPA info
              if(!pTargetData->b_OwnShip)
              {
                    if(pTargetData->b_positionOnceValid)
                    {
                          SelectItem *pSel = pSelectAIS->AddSelectablePoint(pTargetData->Lat, pTargetData->Lon, (void *)mmsi_long, SELTYPE_AISTARGET);
                          pSel->SetUserData(mmsi);
                    }

            //    Calculate CPA info for this target immediately
                    UpdateOneCPA(pTargetData);

            //    Update this target's track
                    if(g_bAISShowTracks)
                        UpdateOneTrack(pTargetData);
              }
        }
        else
        {
//             printf("Unrecognised AIS message ID: %d\n", pTargetData->MID);
             if(bnewtarget)
             {
                    delete pTargetData;                                       // this target is not going to be used
                    m_n_targets--;
             }
        }


        ret = AIS_NoError;

        if( g_nNMEADebug && (g_total_NMEAerror_messages < g_nNMEADebug) ) // debug pjotrc
        {
                  g_total_NMEAerror_messages++;
                  wxString msg(_T("   AIS sentence decoded..."));
                  wxString item;
                  item.Printf(_T("MMSI: %09d"), pTargetData->MMSI);
                  msg.Append(item);
                  ThreadMessage(msg);
        }


    }
    else
        ret = AIS_Partial;                // accumulating parts of a multi-sentence message

 //           if( g_nNMEADebug && (g_total_NMEAerror_messages < g_nNMEADebug) ) // debug pjotrc
 //           {
 //                 g_total_NMEAerror_messages++;
 //                 wxString msg(_T("   AIS partial..."));
 //                 ThreadMessage(msg);
 //           }



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
bool AIS_Decoder::Parse_VDXBitstring(AIS_Bitstring *bstr, AIS_Target_Data *ptd)
{
    bool parse_result = false;
    bool b_posn_report = false;

    wxDateTime now = wxDateTime::Now();
    now.MakeGMT(true);                    // no DST
    if(now.IsDST())
          now.Subtract(wxTimeSpan(1,0,0,0));
//    int utc_hour = now.GetHour();
//    int utc_min = now.GetMinute();
//    int utc_sec = now.GetSecond();
 //   int utc_day = now.GetDay();
 //   wxDateTime::Month utc_month = now.GetMonth();
 //   int utc_year = now.GetYear();


    int message_ID = bstr->GetInt(1, 6);        // Parse on message ID

    ptd->MID = message_ID;

    ptd->MMSI = bstr->GetInt(9, 30);            // MMSI is always in the same spot in the bitstream


    switch (message_ID)
    {
    case 1:                                 // Position Report
    case 2:
    case 3:
        {
            n_msg1++;

            ptd->NavStatus = bstr->GetInt(39, 4);
            ptd->SOG = 0.1 * (bstr->GetInt(51, 10));

            int lon = bstr->GetInt(62, 28);
            if(lon & 0x08000000)                    // negative?
                lon |= 0xf0000000;
            double lon_tentative = lon / 600000.;

            int lat = bstr->GetInt(90, 27);
            if(lat & 0x04000000)                    // negative?
                lat |= 0xf8000000;
            double lat_tentative = lat / 600000.;

            if((lon_tentative <= 180.) && (lat_tentative <= 90.))   // Ship does not report Lat or Lon "unavailable"
            {
                  ptd->Lon = lon_tentative;
                  ptd->Lat = lat_tentative;
                  ptd->b_positionDoubtful = false;
                  ptd->b_positionOnceValid = true;          // Got the position at least once
                  ptd->PositionReportTicks = now.GetTicks();
            }
            else
                  ptd->b_positionDoubtful = true;

            //    decode balance of message....
            ptd->COG = 0.1 * (bstr->GetInt(117, 12));
            ptd->HDG = 1.0 * (bstr->GetInt(129, 9));


            ptd->ROTAIS = bstr->GetInt(43, 8);
            double rot_dir = 1.0;

            if(ptd->ROTAIS == 128)
                  ptd->ROTAIS = -128;                     // not available codes as -128
            else if((ptd->ROTAIS & 0x80) == 0x80)
            {
                  ptd->ROTAIS = ptd->ROTAIS - 256;       // convert to twos complement
                  rot_dir = -1.0;
            }

            ptd->ROTIND = wxRound(rot_dir * pow((((double)ptd->ROTAIS) / 4.733), 2));      // Convert to indicated ROT

            ptd->m_utc_sec = bstr->GetInt(138, 6);

            if((1 == message_ID) || (2 == message_ID))      // decode SOTDMA per 7.6.7.2.2
            {
                  ptd->SyncState = bstr->GetInt(151,2);
                  ptd->SlotTO = bstr->GetInt(153,2);
                  if((ptd->SlotTO == 1) && (ptd->SyncState == 0)) // UTCDirect follows
                  {
                        ptd->m_utc_hour = bstr->GetInt(155, 5);

                        ptd->m_utc_min = bstr->GetInt(160,7);

                        if((ptd->m_utc_hour < 24) && (ptd->m_utc_min <60) && (ptd->m_utc_sec < 60))
                        {
                              wxDateTime rx_time(ptd->m_utc_hour, ptd->m_utc_min, ptd->m_utc_sec);
                              rx_ticks = rx_time.GetTicks();
                              if(!b_firstrx)
                              {
                                    first_rx_ticks = rx_ticks;
                                    b_firstrx = true;
                                    }
                        }
//                    else
//                          printf("hr:min:sec:   %d:%d:%d\n", ptd->m_utc_hour, ptd->m_utc_min, ptd->m_utc_sec);

                  }
            }

            //    Capture Euro Inland special passing arrangement signal ("stbd-stbd")
            ptd->blue_paddle = bstr->GetInt(144, 2);
            ptd->b_blue_paddle = (ptd->blue_paddle == 2);             // paddle is set

            ptd->Class = AIS_CLASS_A;

            //    Check for SART and friends by looking at first two digits of MMSI
            int mmsi_start =  ptd->MMSI / 10000000;

            if( mmsi_start == 97)
                  ptd->Class = AIS_SART;

            parse_result = true;                // so far so good
            b_posn_report = true;

            break;
        }

          case 18:
          {
                ptd->SOG = 0.1 * (bstr->GetInt(47, 10));

                int lon = bstr->GetInt(58, 28);
                if(lon & 0x08000000)                    // negative?
                      lon |= 0xf0000000;
                double lon_tentative = lon / 600000.;

                int lat = bstr->GetInt(86, 27);
                if(lat & 0x04000000)                    // negative?
                      lat |= 0xf8000000;
                double lat_tentative = lat / 600000.;

                if((lon_tentative <= 180.) && (lat_tentative <= 90.))   // Ship does not report Lat or Lon "unavailable"
                {
                      ptd->Lon = lon_tentative;
                      ptd->Lat = lat_tentative;
                      ptd->b_positionDoubtful = false;
                      ptd->b_positionOnceValid = true;          // Got the position at least once
                      ptd->PositionReportTicks = now.GetTicks();
                }
                else
                      ptd->b_positionDoubtful = true;

                ptd->COG = 0.1 * (bstr->GetInt(113, 12));
                ptd->HDG = 1.0 * (bstr->GetInt(125, 9));

                ptd->m_utc_sec = bstr->GetInt(134, 6);

                ptd->Class = AIS_CLASS_B;

                parse_result = true;                // so far so good
                b_posn_report = true;

                break;
          }

    case 5:
        {
              n_msg5++;
              ptd->Class = AIS_CLASS_A;


//          Get the AIS Version indicator
//          0 = station compliant with Recommendation ITU-R M.1371-1
//          1 = station compliant with Recommendation ITU-R M.1371-3
//          2-3 = station compliant with future editions
            int AIS_version_indicator = bstr->GetInt(39, 2);
            if(AIS_version_indicator < 2)
            {
                  ptd->IMO = bstr->GetInt(41, 30);

                  bstr->GetStr(71,42, &ptd->CallSign[0], 7);
                  bstr->GetStr(113,120, &ptd->ShipName[0], 20);
                  ptd->b_nameValid = true;
//                  printf("%d %d %s\n", n_msgs, ptd->MMSI, ptd->ShipName);

                  ptd->ShipType = (unsigned char)bstr->GetInt(233,8);

                  ptd->DimA = bstr->GetInt(241, 9);
                  ptd->DimB = bstr->GetInt(250, 9);
                  ptd->DimC = bstr->GetInt(259, 6);
                  ptd->DimD = bstr->GetInt(265, 6);

//                int epfd = bstr->GetInt(271, 4);

                  ptd->ETA_Mo =  bstr->GetInt(275, 4);
                  ptd->ETA_Day = bstr->GetInt(279, 5);
                  ptd->ETA_Hr =  bstr->GetInt(284, 5);
                  ptd->ETA_Min = bstr->GetInt(289, 6);

                  ptd->Draft = (double)(bstr->GetInt(295, 8)) / 10.0;

                  bstr->GetStr(303,120, &ptd->Destination[0], 20);

                  ptd->StaticReportTicks = now.GetTicks();

                  parse_result = true;
            }

            break;
        }

     case 24:
         {
               ptd->Class = AIS_CLASS_B;

               int part_number = bstr->GetInt(39, 2);
               if(0 == part_number)
               {
                     bstr->GetStr(41,120, &ptd->ShipName[0], 20);
//                     printf("%d %d %s\n", n_msgs, ptd->MMSI, ptd->ShipName);
                     ptd->b_nameValid = true;
                     parse_result = true;
                     n_msg24++;
               }
               else if(1 == part_number)
               {
                     ptd->ShipType = (unsigned char)bstr->GetInt(41,8);
                     bstr->GetStr(91,42, &ptd->CallSign[0], 7);

                     ptd->DimA = bstr->GetInt(133, 9);
                     ptd->DimB = bstr->GetInt(142, 9);
                     ptd->DimC = bstr->GetInt(151, 6);
                     ptd->DimD = bstr->GetInt(157, 6);
                     parse_result = true;
               }


               break;
         }
     case 4:                                    // base station
          {
                ptd->Class = AIS_BASE;

//                ptd->m_utc_year = bstr->GetInt(39, 14);
//                ptd->m_utc_month =  bstr->GetInt(53,  4);
//                ptd->m_utc_day =    bstr->GetInt(57,  5);
                ptd->m_utc_hour =   bstr->GetInt(62,  5);
                ptd->m_utc_min    = bstr->GetInt(67,  6);
                ptd->m_utc_sec    = bstr->GetInt(73,  6);
                //                              (79,  1);
                int lon = bstr->GetInt(80, 28);
                if(lon & 0x08000000)                    // negative?
                      lon |= 0xf0000000;
                double lon_tentative = lon / 600000.;

                int lat = bstr->GetInt(108, 27);
                if(lat & 0x04000000)                    // negative?
                      lat |= 0xf8000000;
                double lat_tentative = lat / 600000.;

                if((lon_tentative <= 180.) && (lat_tentative <= 90.))   // Ship does not report Lat or Lon "unavailable"
                {
                      ptd->Lon = lon_tentative;
                      ptd->Lat = lat_tentative;
                      ptd->b_positionDoubtful = false;
                      ptd->b_positionOnceValid = true;          // Got the position at least once
                      ptd->PositionReportTicks = now.GetTicks();
                }
                else
                      ptd->b_positionDoubtful = true;

                ptd->COG = -1.;
                ptd->HDG = 511;
                ptd->SOG = -1.;

                parse_result = true;
                b_posn_report = true;

                break;
          }
     case 9:                                    // Special Position Report (Standard SAR Aircraft Position Report)
          {
                break;
          }
     case 21:                                    // Test Message (Aid to Navigation)   pjotrc 2010.02.01
          {
//
// The following is a patch to impersonate an AtoN as Ship      // pjotrc 2010.02.01
//
//                  ptd->NavStatus = MOORED;
                  ptd->ShipType = (unsigned char)bstr->GetInt(39,5);
                  ptd->IMO = 0;
                  ptd->SOG = 0;
                  ptd->HDG = 0;
                  ptd->COG = 0;
                  ptd->ROTAIS = -128;                 // i.e. not available
                  ptd->DimA = bstr->GetInt(220, 9);
                  ptd->DimB = bstr->GetInt(229, 9);
                  ptd->DimC = bstr->GetInt(238, 6);
                  ptd->DimD = bstr->GetInt(244, 6);
                  ptd->Draft = 0;

                  ptd->m_utc_sec = bstr->GetInt(254, 6);

                  int offpos = bstr->GetInt(260,1); // off position flag
                  int virt = bstr->GetInt(270,1); // virtual flag

                  if (virt)
                        ptd->NavStatus = ATON_VIRTUAL;
                  else
                        ptd->NavStatus = ATON_REAL;
                  if (ptd->m_utc_sec<=59 /*&& !virt*/)
                  {
                         ptd->NavStatus += 1;
                         if (offpos) ptd->NavStatus += 1;
                  }


                  bstr->GetStr(44,120, &ptd->ShipName[0], 20); // short name only, extension wont fit in Ship structure

                  if(bstr->GetBitCount() > 276)
                  {
                        int nx = ((bstr->GetBitCount() - 272) / 6) * 6;
                        bstr->GetStr(273,nx, &ptd->ShipNameExtension[0], 20);
                        ptd->ShipNameExtension[20] = 0;
                  }

                ptd->b_nameValid = true;

                parse_result = true;                // so far so good

                ptd->Class = AIS_ATON;

                int lon = bstr->GetInt(165, 28);

                if(lon & 0x08000000)                    // negative?
                      lon |= 0xf0000000;
                double lon_tentative = lon / 600000.;

                int lat = bstr->GetInt(193, 27);

                if(lat & 0x04000000)                    // negative?
                      lat |= 0xf8000000;
                double lat_tentative = lat / 600000.;

                if((lon_tentative <= 180.) && (lat_tentative <= 90.))   // Ship does not report Lat or Lon "unavailable"
                {
                      ptd->Lon = lon_tentative;
                      ptd->Lat = lat_tentative;
                      ptd->b_positionDoubtful = false;
                      ptd->b_positionOnceValid = true;          // Got the position at least once
                      ptd->PositionReportTicks = now.GetTicks();
                }
                else
                      ptd->b_positionDoubtful = true;

                if( g_nNMEADebug && (g_total_NMEAerror_messages < g_nNMEADebug) )
                {
                      g_total_NMEAerror_messages++;
                      wxString msg(_T("   AIS type 21...(MMSI, lon, lat:"));
                      wxString item;
                      item.Printf(_T("%09d, %10.5f, %10.5f"), ptd->MMSI, ptd->Lon, ptd->Lat);
                      msg.Append(item);
                      msg.Append(_T(" ) "));
                      ThreadMessage(msg);
                }

                b_posn_report = true;
                break;
          }
     case 8:                                    // Binary Broadcast
          {
                int dac = bstr->GetInt(41, 10);
                int fi = bstr->GetInt(51, 6);
                if(dac == 200)                  // European inland
                {
                      if(fi == 10)              // "Inland ship static and voyage related data"
                      {
                            ptd->b_isEuroInland = true;

                            bstr->GetStr(57, 48, &ptd->Euro_VIN[0], 8);
                            ptd->Euro_Length = ((double)bstr->GetInt(105, 13))/10.0;
                            ptd->Euro_Beam = ((double)bstr->GetInt(118, 10))/10.0;
                            ptd->UN_shiptype = bstr->GetInt(128, 14);
//                            int hazcargo = bstr->GetInt(142, 3);
                            ptd->Euro_Draft = ((double)bstr->GetInt(145, 11))/100.0;
//                            int load = bstr->GetInt(156, 2);
//                            int qualspd = bstr->GetInt(158, 1);
//                            int qualcrs = bstr->GetInt(159, 1);
//                            int qualhead = bstr->GetInt(160, 1);
//                            spare = bstr->GetInt(161, 8);
                            parse_result = true;

                      }
                }
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
     default:
          {
                break;
          }

    }

    if(b_posn_report)
          ptd->b_lost = false;

    if(true == parse_result)
    {
          //      Revalidate the target under some conditions
          if(!ptd->b_active && !ptd->b_positionDoubtful && b_posn_report)
                      ptd->b_active = true;
    }

    return parse_result;
}



bool AIS_Decoder::NMEACheckSumOK(const wxString& str_in)
{

   unsigned char checksum_value = 0;
   int sentence_hex_sum;

   char str_ascii[AIS_MAX_MESSAGE_LEN + 1];
   strncpy(str_ascii, str_in.mb_str(), AIS_MAX_MESSAGE_LEN);
   str_ascii[AIS_MAX_MESSAGE_LEN] = '\0';

   int string_length = strlen(str_ascii);

   int payload_length = 0;                       // pjotrc 2010.02.07
   while ((payload_length<string_length) && (str_ascii[payload_length]!='*')) // look for '*'
	payload_length++;

   if (payload_length == string_length) return false; // '*' not found at all, no checksum

   int index = 1; // Skip over the $ at the begining of the sentence

   while( index < payload_length )                 // pjotrc 2010.02.07
   {                                               // pjotrc 2010.02.07
         checksum_value ^= str_ascii[ index ];
         index++;
   }

   if(string_length > 4)
   {
        char scanstr[3];
        scanstr[0] = str_ascii[payload_length+1];       // pjotrc 2010.02.07
        scanstr[1] = str_ascii[payload_length+2];       // pjotrc 2010.02.07
        scanstr[2] = 0;
        sscanf(scanstr, "%2x", &sentence_hex_sum);

        if(sentence_hex_sum == checksum_value)
            return true;
   }

   return false;
}

void AIS_Decoder::UpdateAllCPA(void)
{
      //    Iterate thru all the targets
      AIS_Target_Hash::iterator it;
      AIS_Target_Hash *current_targets = GetTargetList();

      for( it = (*current_targets).begin(); it != (*current_targets).end(); ++it )
      {
            AIS_Target_Data *td = it->second;

            if(NULL != td)
                  UpdateOneCPA(td);
      }
}

void AIS_Decoder::UpdateAllTracks(void)
{
           //    Iterate thru all the targets
      AIS_Target_Hash::iterator it;
      AIS_Target_Hash *current_targets = GetTargetList();

      for( it = (*current_targets).begin(); it != (*current_targets).end(); ++it )
      {
            AIS_Target_Data *td = it->second;

            if(NULL != td)
                 UpdateOneTrack(td);
      }
}

void AIS_Decoder::UpdateOneTrack(AIS_Target_Data *ptarget)
{
      if(!ptarget->b_positionOnceValid)
            return;

      //    Add the newest point
      AISTargetTrackPoint *ptrackpoint = new AISTargetTrackPoint;
      ptrackpoint->m_lat = ptarget->Lat;
      ptrackpoint->m_lon = ptarget->Lon;
      ptrackpoint->m_time = wxDateTime::Now().GetTicks();

      ptarget->m_ptrack->Append(ptrackpoint);

      //    Walk the list, removing any track points that are older than the stipulated time

      time_t test_time = wxDateTime::Now().GetTicks() - (time_t)(g_AISShowTracks_Mins * 60);

      wxAISTargetTrackListNode *node = ptarget->m_ptrack->GetFirst();
      while(node)
      {
            AISTargetTrackPoint *ptrack_point = node->GetData();

            if(ptrack_point->m_time < test_time)
            {
                  if(ptarget->m_ptrack->DeleteObject(ptrack_point))
                  {
                        delete ptrack_point;
                        node = ptarget->m_ptrack->GetFirst();                // restart the list
                  }
            }
            else
                  node = node->GetNext();
      }

}




void AIS_Decoder::UpdateAllAlarms(void)
{
      m_bGeneralAlert    = false;                // no alerts yet


           //    Iterate thru all the targets
      AIS_Target_Hash::iterator it;
      AIS_Target_Hash *current_targets = GetTargetList();

      for( it = (*current_targets).begin(); it != (*current_targets).end(); ++it )
      {
            AIS_Target_Data *td = it->second;

            if(NULL != td)
            {
                  //  Maintain General Alert
                  if(!m_bGeneralAlert)
                  {
                  //    Quick check on basic condition
                        if((td->CPA < g_CPAWarn_NM) && (td->TCPA > 0))
                              m_bGeneralAlert = true;

                  //    Some options can suppress general alerts
                        if(g_bAIS_CPA_Alert_Suppress_Moored && (td->SOG <= g_ShowMoored_Kts))
                              m_bGeneralAlert = false;

                  //    Skip distant targets if requested
                        if((g_bCPAMax) && ( td->Range_NM > g_CPAMax_NM))
                              m_bGeneralAlert = false;

                  //    Skip if TCPA is too long
                        if((g_bTCPA_Max) && (td->TCPA > g_TCPA_Max))
                              m_bGeneralAlert = false;
                  }

                  ais_alarm_type this_alarm = AIS_NO_ALARM;
                  if(g_bCPAWarn && td->b_active && td->b_positionOnceValid)
                  {
                        //      Skip anchored/moored(interpreted as low speed) targets if requested
                        if((!g_bShowMoored) && (td->SOG <= g_ShowMoored_Kts))        // dsr
                        {
                              td->n_alarm_state = AIS_NO_ALARM;
                              continue;
                        }

                        //    No Alert on moored(interpreted as low speed) targets if so requested
                        if(g_bAIS_CPA_Alert_Suppress_Moored && (td->SOG <= g_ShowMoored_Kts))                 // dsr
                        {

                              td->n_alarm_state = AIS_NO_ALARM;
                              continue;
                        }


                        //    Skip distant targets if requested
                        if(g_bCPAMax)
                        {
                              if( td->Range_NM > g_CPAMax_NM)
                              {
                                    td->n_alarm_state = AIS_NO_ALARM;
                                    continue;
                              }
                        }

                        if((td->CPA < g_CPAWarn_NM) && (td->TCPA > 0))
                        {
                              if(g_bTCPA_Max)
                              {
                                    if(td->TCPA < g_TCPA_Max)
                                          this_alarm = AIS_ALARM_SET;
                              }
                        else
                              this_alarm = AIS_ALARM_SET;
                        }
                  }

                  //    Maintain the timer for in_ack flag
                  if(g_bAIS_ACK_Timeout)
                  {
                        if(td->b_in_ack_timeout)
                        {
                              wxTimeSpan delta = wxDateTime::Now() - td->m_ack_time;
                              if(delta.GetMinutes() > g_AckTimeout_Mins)
                                   td->b_in_ack_timeout = false;
                        }
                  }
                  else
                        td->b_in_ack_timeout = false;

                  td->n_alarm_state = this_alarm;

            }
      }
}


void AIS_Decoder::UpdateOneCPA(AIS_Target_Data *ptarget)
{
      ptarget->Range_NM = -1.;            // Defaults
      ptarget->Brg = -1.;

      if(!ptarget->b_positionOnceValid || !bGPSValid)
      {
            ptarget->bCPA_Valid = false;
            return;
      }

      //    Compute the current Range/Brg to the target
      double brg, dist;
      DistanceBearingMercator(ptarget->Lat, ptarget->Lon, gLat, gLon, &brg, &dist);
      ptarget->Range_NM = dist;
      ptarget->Brg = brg;

      if(dist <= 1e-5)
            ptarget->Brg = -1.0;             // Brg is undefined if Range == 0.


      //    There can be no collision between ownship and itself....
      //    This can happen if AIVDO messages are received, and there is another source of ownship position, like NMEA GLL
      //    The two positions are always temporally out of sync, and one will always be exactly in front of the other one.
      if(ptarget->b_OwnShip)
      {
            ptarget->CPA = 100;
            ptarget->TCPA = -100;
            ptarget->bCPA_Valid = false;
            return;
      }

      double cpa_calc_ownship_cog = gCog;
      double cpa_calc_target_cog = ptarget->COG;

//    Ownship is not reporting valid SOG, so no way to calculate CPA
      if(wxIsNaN(gSog) || (gSog > 102.2))
      {
            ptarget->bCPA_Valid = false;
            return;
      }

//    Ownship is maybe anchored and not reporting COG
      if( wxIsNaN(gCog) || gCog == 360.0 )
      {
            if(gSog < .01)
                  cpa_calc_ownship_cog = 0.;          // substitute value
                                                      // for the case where SOG ~= 0, and COG is unknown.
            else
            {
                  ptarget->bCPA_Valid = false;
                  return;
            }
      }

//    Target is maybe anchored and not reporting COG
      if(ptarget->COG == 360.0)
      {
            if(ptarget->SOG >102.2)
            {
                  ptarget->bCPA_Valid = false;
                  return;
            }
            else if(ptarget->SOG < .01)
                  cpa_calc_target_cog = 0.;           // substitute value
                                                      // for the case where SOG ~= 0, and COG is unknown.
            else
            {
                  ptarget->bCPA_Valid = false;
                  return;
            }
      }


      //    Express the SOGs as meters per hour
      double v0 = gSog         * 1852.;
      double v1 = ptarget->SOG * 1852.;

      if((v0 < 1e-6) && (v1 < 1e-6))
      {
            ptarget->TCPA = 0.;
            ptarget->CPA = 0.;

            ptarget->bCPA_Valid = false;
      }
      else
      {
            //    Calculate the TCPA first

            //    Working on a Reduced Lat/Lon orthogonal plotting sheet....
            //    Get easting/northing to target,  in meters

            double east1 = (ptarget->Lon - gLon) * 60 * 1852;
            double north1 = (ptarget->Lat - gLat) * 60 * 1852;

            double east = east1 * (cos(gLat * PI / 180));;
            double north = north1;

            //    Convert COGs trigonometry to standard unit circle
            double cosa = cos((90. - cpa_calc_ownship_cog) * PI / 180.);
            double sina = sin((90. - cpa_calc_ownship_cog) * PI / 180.);
            double cosb = cos((90. - cpa_calc_target_cog) * PI / 180.);
            double sinb = sin((90. - cpa_calc_target_cog) * PI / 180.);


            //    These will be useful
            double fc = (v0 * cosa) - (v1 * cosb);
            double fs = (v0 * sina) - (v1 * sinb);

            double d = (fc * fc) + (fs * fs);
            double tcpa;

            // the tracks are almost parallel
            if (fabs(d) < 1e-6)
                  tcpa = 0.;
            else
            //    Here is the equation for t, which will be in hours
                  tcpa = ((fc * east) + (fs * north)) / d;

            //    Convert to minutes
            ptarget->TCPA = tcpa * 60.;

            //    Calculate CPA
            //    Using TCPA, predict ownship and target positions

            double OwnshipLatCPA, OwnshipLonCPA, TargetLatCPA, TargetLonCPA;

            ll_gc_ll(gLat,         gLon,         cpa_calc_ownship_cog, gSog * tcpa, &OwnshipLatCPA, &OwnshipLonCPA);
            ll_gc_ll(ptarget->Lat, ptarget->Lon, cpa_calc_target_cog, ptarget->SOG * tcpa, &TargetLatCPA,  &TargetLonCPA);

            //   And compute the distance
            ptarget->CPA = DistGreatCircle(OwnshipLatCPA, OwnshipLonCPA, TargetLatCPA, TargetLonCPA);

            ptarget->bCPA_Valid = true;

            if(ptarget->TCPA  < 0)
                  ptarget->bCPA_Valid = false;
      }
}




//------------------------------------------------------------------------------------
//
//      AIS Data Source Support
//
//------------------------------------------------------------------------------------


AIS_Error AIS_Decoder::OpenDataSource(wxFrame *pParent, const wxString& AISDataSource)
{
      pAIS_Thread = NULL;
      m_OK = false;

      TimerAIS.SetOwner(this, TIMER_AIS1);
      TimerAIS.Stop();

      m_data_source_string = AISDataSource;

//      Create and manage AIS data stream source

//    Decide upon source
      wxString msg(_T("AIS Data Source is...."));
      msg.Append(m_data_source_string);
      wxLogMessage(msg);

#ifndef OCPN_NO_SOCKETS
      m_sock = NULL;

//      Data Source is private TCP/IP Server
      if(m_data_source_string.Contains(_T("TCP/IP")))
      {
            wxString AIS_data_ip;
            AIS_data_ip = m_data_source_string.Mid(7);         // extract the IP

// Create the socket
            m_sock = new wxSocketClient();

// Setup the event handler and subscribe to most events
            m_sock->SetEventHandler(*this, AIS_SOCKET_ID);

            m_sock->SetNotify(wxSOCKET_CONNECTION_FLAG |
                    wxSOCKET_INPUT_FLAG |
                    wxSOCKET_LOST_FLAG);
            m_sock->Notify(TRUE);

            m_busy = FALSE;


//    Build the target address

//    n.b. Win98
//    wxIPV4address::Hostname() uses sockets function gethostbyname() for address resolution
//    Implications...Either name target must exist in c:\windows\hosts, or
//                            a DNS server must be active on the network.
//    If neither true, then wxIPV4address::Hostname() will block (forever?)....
//
//    Workaround....
//    Use a thread to try the name lookup, in case it hangs

            DNSTestThread *ptest_thread = NULL;
            ptest_thread = new DNSTestThread(AIS_data_ip);

            ptest_thread->Run();                      // Run the thread from ::Entry()


//    Sleep and loop for N seconds
#define SLEEP_TEST_SEC  2

            for(int is=0 ; is<SLEEP_TEST_SEC * 10 ; is++)
            {
                  wxMilliSleep(100);
                  if(s_dns_test_flag)
                        break;
            }

            if(!s_dns_test_flag)
            {

                  wxString msg(AIS_data_ip);
                  msg.Prepend(_("Could not resolve TCP/IP host '"));
                  msg.Append(_("'\n Suggestion: Try 'xxx.xxx.xxx.xxx' notation"));
                  OCPNMessageDialog md(pParent, msg, _("OpenCPN Message"), wxICON_ERROR );
                  md.ShowModal();

                  m_sock->Notify(FALSE);
                  m_sock->Destroy();
                  m_sock = NULL;

                  return AIS_NO_TCP;
            }


            //      Resolved the name, somehow, so Connect() the socket
            addr.Hostname(AIS_data_ip);
            addr.Service(3047/*GPSD_PORT_NUMBER*/);
            m_sock->Connect(addr, FALSE);       // Non-blocking connect
            m_OK = true;
      }

#endif

//    AIS Data Source is specified serial port

      if(m_data_source_string.Contains(_T("Serial")))
      {
          wxString comx;
//          comx =  m_pdata_source_string->Mid(7);        // either "COM1" style or "/dev/ttyS0" style
          comx =  m_data_source_string.AfterFirst(':');      // strip "Serial:"

#ifdef __WXMSW__
          wxString mcomx = comx;
          comx.Prepend(_T("\\\\.\\"));                  // Required for access to Serial Ports greater than COM9

//  As a quick check, verify that the specified port is available
            HANDLE m_hSerialComm = CreateFile(comx.c_str(),       // Port Name
                                             GENERIC_READ,
                                             0,
                                             NULL,
                                             OPEN_EXISTING,
                                             0,
                                             NULL);

            if(m_hSerialComm == INVALID_HANDLE_VALUE)
            {
                  wxString msg(mcomx);
                  msg.Prepend(_("  Could not open AIS serial port '"));
                  msg.Append(_("'\nSuggestion: Try closing other applications."));
                  OCPNMessageDialog md(pParent, msg, _("OpenCPN Message"), wxICON_ERROR );
                  md.ShowModal();

                  return AIS_NO_SERIAL;
            }

            else
                  CloseHandle(m_hSerialComm);

//    Kick off the AIS RX thread
            //    Kick off the NMEA RX thread
//            m_pSecondary_Thread = new OCP_NMEA_Thread(this, frame, pMutex, m_pPortMutex, comx, g_pCommMan, strBaudRate);
//            m_Thread_run_flag = 1;
//            pAIS_Thread->Run();

            pAIS_Thread = new OCP_AIS_Thread(this, comx);
            m_Thread_run_flag = 1;
            pAIS_Thread->Run();
#endif


#ifdef __POSIX__
//    Kick off the NMEA RX thread
            pAIS_Thread = new OCP_AIS_Thread(this, comx);
            m_Thread_run_flag = 1;
            pAIS_Thread->Run();
#endif

            m_OK = true;
      }

      if(m_OK)
          TimerAIS.Start(TIMER_AIS_MSEC,wxTIMER_CONTINUOUS);

      return AIS_NoError;

}




void AIS_Decoder::GetSource(wxString& source)
{
      source = m_data_source_string;
}




void AIS_Decoder::Pause(void)
{
      TimerAIS.Stop();

#ifndef OCPN_NO_SOCKETS
      if(m_sock)
            m_sock->Notify(FALSE);
#endif
}

void AIS_Decoder::UnPause(void)
{
    TimerAIS.Start(TIMER_AIS_MSEC,wxTIMER_CONTINUOUS);

#ifndef OCPN_NO_SOCKETS
    if(m_sock)
            m_sock->Notify(TRUE);
#endif
}



void AIS_Decoder::OnSocketEvent(wxSocketEvent& event)
{
#ifndef OCPN_NO_SOCKETS

#define RD_BUF_SIZE    200

    int nBytes;
    char *bp;
    char buf[RD_BUF_SIZE + 1];
    int char_count;


  switch(event.GetSocketEvent())
  {
      case wxSOCKET_INPUT :                     // from  Daemon
            m_sock->SetFlags(wxSOCKET_NOWAIT);


//          Read the reply, one character at a time, looking for 0x0a (lf)
            bp = buf;
            char_count = 0;

            while (char_count < RD_BUF_SIZE)
            {
                m_sock->Read(bp, 1);
                nBytes = m_sock->LastCount();
                if(*bp == 0x0a)
                {
                    bp++;
                    break;
                }

                bp++;
                char_count++;
            }

            *bp = 0;                        // end string

//          Validate the string

            if(!strncmp((const char *)buf, "!AIVDM", 6))
            {
//                  Decode(buf);

//    Signal the main program thread

//                    wxCommandEvent event( EVT_AIS, ID_AIS_WINDOW );
//                    event.SetEventObject( (wxObject *)this );
//                    event.SetExtraLong(EVT_AIS_DIRECT);
//                    m_pParentEventHandler->AddPendingEvent(event);
            }



    case wxSOCKET_LOST       :
    case wxSOCKET_CONNECTION :
    default                  :
          break;
  }
#endif
}

void AIS_Decoder::OnTimerAISAudio(wxTimerEvent& event)
{
      if(g_bAIS_CPA_Alert_Audio && m_bAIS_Audio_Alert_On)
      {
            m_AIS_Sound.Create(g_sAIS_Alert_Sound_File);
            if(m_AIS_Sound.IsOk())
                  m_AIS_Sound.Play();
      }
      m_AIS_Audio_Alert_Timer.Start(TIMER_AIS_AUDIO_MSEC,wxTIMER_CONTINUOUS);
}


void AIS_Decoder::OnTimerAIS(wxTimerEvent& event)
{
      TimerAIS.Stop();

#if 0
      /// testing for orphans in the Select list
      SelectItem *pFindSel;

//    Iterate on the list
      wxSelectableItemListNode *node = pSelectAIS->GetSelectList()->GetFirst();

      int n_sel = 0;
      while(node)
      {
            pFindSel = node->GetData();
            if(pFindSel->m_seltype == SELTYPE_AISTARGET)
            {
                  int mmsi = (int) pFindSel->m_Data4;

                  AIS_Target_Data *tdp = Get_Target_Data_From_MMSI(mmsi);
                  if(tdp == NULL)
                        int yyp = 6;

                  if(tdp->SOG > 200.)
                        int yyu = 4;
            }

            n_sel++;

            node = node->GetNext();
      }

      if(n_sel > m_n_targets)
            int yyr = 3;

      if(n_sel > GetTargetList()->size())
            int yyl = 4;
#endif


      ///testing


      //    Scrub the target hash list
      //    removing any targets older than stipulated age


      wxDateTime now = wxDateTime::Now();
      now.MakeGMT();

      AIS_Target_Hash::iterator it;
      AIS_Target_Hash *current_targets = GetTargetList();

      it = (*current_targets).begin();
      while( it != (*current_targets).end())
      {
          bool b_new_it = false;

          AIS_Target_Data *td = it->second;

          if(NULL == td)                        // This should never happen, but I saw it once....
          {
                current_targets->erase(it);
                break;                          // leave the loop
          }

          int target_posn_age = now.GetTicks() - td->PositionReportTicks;
          int target_static_age = now.GetTicks() - td->StaticReportTicks;

          //      Mark lost targets if specified
          if(g_bMarkLost)
          {
                if((target_posn_age > g_MarkLost_Mins * 60) && (td->Class != AIS_GPSG_BUDDY))
                        td->b_active = false;
          }

          //      Remove lost targets if specified
          double removelost_Mins = fmax(g_RemoveLost_Mins,g_MarkLost_Mins);
          if(g_bRemoveLost)
          {
                if ((target_posn_age > removelost_Mins * 60) && (td->Class != AIS_GPSG_BUDDY))
                {
                      //      So mark the target as lost, with unknown position, and make it not selectable
                      td->b_lost = true;
                      td->b_positionOnceValid = false;
                      td->COG = 360.0;
                      td->SOG = 103.0;
                      td->HDG = 511.0;
                      td->ROTAIS = -128;

                      long mmsi_long = td->MMSI;
                      pSelectAIS->DeleteSelectablePoint((void *)mmsi_long, SELTYPE_AISTARGET);

                      //      If we have not seen a static report in 3 times the removal spec,
                      //      then remove the target from all lists.
                      if(target_static_age > removelost_Mins * 60 * 3)
                      {
                        current_targets->erase(it);
                        delete td;

                      //      Reset the iterator on item erase.
                        it = (*current_targets).begin();
                        b_new_it = true;
                      }
                }
          }

          if(!b_new_it)
                ++it;
      }


//--------------TEST DATA strings
#if 0
      char str[82];
      if(1)
      {
          if(itime++ > 2)
          {
              itime = 0;
              strcpy(str, test_str[istr]);
              istr++;
              if(istr > 23)
                  istr = 0;
              Decode(str);
          }
      }
#endif

      UpdateAllCPA();

      UpdateAllAlarms();

      //    Update the general suppression flag
      m_bSuppressed = false;
      if(g_bAIS_CPA_Alert_Suppress_Moored ||
         !g_bShowMoored)
            m_bSuppressed = true;


      if(g_bAIS_CPA_Alert)
      {

            m_bAIS_Audio_Alert_On = false;            // default, may be set on

            //    Process any Alarms

            //    If the AIS Alert Dialog is not currently shown....

            //    Show the Alert dialog
            //    Which of multiple targets?
            //    search the list for any targets with alarms, selecting the target with shortest TCPA

            if(NULL == g_pais_alert_dialog_active)
            {
                  double tcpa_min = 1e6;             // really long
                  AIS_Target_Data *palarm_target = NULL;

                  for( it = (*current_targets).begin(); it != (*current_targets).end(); ++it )
                  {
                        AIS_Target_Data *td = it->second;
                        if(td)
                        {
                              if(td->b_active)
                              {
                                    if((AIS_ALARM_SET == td->n_alarm_state) && !td->b_in_ack_timeout)
                                    {
                                          if(td->TCPA < tcpa_min)
                                          {
                                                tcpa_min = td->TCPA;
                                                palarm_target = td;
                                          }
                                    }
                              }
                        }
                  }

                  if(palarm_target)
                  {
                  //    Show the alert

                        AISTargetAlertDialog *pAISAlertDialog = new AISTargetAlertDialog();
                        pAISAlertDialog->Create ( palarm_target->MMSI, m_parent_frame, this, -1, _("AIS Alert"),
                                                wxPoint( g_ais_alert_dialog_x, g_ais_alert_dialog_y),
                                                wxSize( g_ais_alert_dialog_sx, g_ais_alert_dialog_sy));

                        g_pais_alert_dialog_active = pAISAlertDialog;
                        pAISAlertDialog->Show();                        // Show modeless, so it stays on the screen


                        //    Audio alert if requested
                        m_bAIS_Audio_Alert_On = true;             // always on when alert is first shown
                  }
            }

            //    The AIS Alert dialog is already shown.  If the  dialog MMSI number is still alerted, update the dialog
            //    otherwise, destroy the dialog
            else
            {
                  AIS_Target_Data *palert_target = Get_Target_Data_From_MMSI(g_pais_alert_dialog_active->Get_Dialog_MMSI());

                  if(palert_target)
                  {
                        if((AIS_ALARM_SET == palert_target->n_alarm_state) && !palert_target->b_in_ack_timeout)
                        {
                              g_pais_alert_dialog_active->UpdateText();
                        }
                        else
                        {
                              g_pais_alert_dialog_active->Close();
                              m_bAIS_Audio_Alert_On = false;
                        }

                        if(true == palert_target->b_suppress_audio)
                              m_bAIS_Audio_Alert_On = false;
                        else
                              m_bAIS_Audio_Alert_On = true;
                  }
                  else
                  {                                                     // this should not happen, however...
                        g_pais_alert_dialog_active->Close();
                        m_bAIS_Audio_Alert_On = false;
                  }

            }
      }

      //    At this point, the audio flag is set

      //    Honor the global flag
      if(!g_bAIS_CPA_Alert_Audio)
            m_bAIS_Audio_Alert_On = false;


      if(m_bAIS_Audio_Alert_On)
      {
            if(!m_AIS_Audio_Alert_Timer.IsRunning())
            {
                  m_AIS_Audio_Alert_Timer.SetOwner(this, TIMER_AISAUDIO);
                  m_AIS_Audio_Alert_Timer.Start(TIMER_AIS_AUDIO_MSEC);

                  m_AIS_Sound.Create(g_sAIS_Alert_Sound_File);
                  if(m_AIS_Sound.IsOk())
                        m_AIS_Sound.Play();
            }
      }
      else
            m_AIS_Audio_Alert_Timer.Stop();


      TimerAIS.Start(TIMER_AIS_MSEC,wxTIMER_CONTINUOUS);
}


AIS_Target_Data *AIS_Decoder::Get_Target_Data_From_MMSI(int mmsi)
{
      if(AISTargetList->find( mmsi ) == AISTargetList->end())     // if entry does not exist....
            return NULL;
      else
            return (*AISTargetList)[mmsi];          // find current entry

/*
      //    Search the active target list for a matching MMSI
      AIS_Target_Hash::iterator it;
      AIS_Target_Hash *current_targets =GetTargetList();

      AIS_Target_Data *td_found = NULL;
      for( it = (*current_targets).begin(); it != (*current_targets).end(); ++it )
      {
            AIS_Target_Data *td = it->second;
            if(mmsi == td->MMSI)
            {
                  td_found = td;
                  break;
            }
      }
      return td_found;
*/
}




//-------------------------------------------------------------------------------------------------------------
//
//    AIS Serial Input Thread
//
//    This thread manages reading the AIS data stream from the declared serial port
//
//-------------------------------------------------------------------------------------------------------------

//          Inter-thread communication event implementation
//DEFINE_EVENT_TYPE(EVT_AIS)



//-------------------------------------------------------------------------------------------------------------
//    OCP_AIS_Thread Implementation
//-------------------------------------------------------------------------------------------------------------

//    ctor

OCP_AIS_Thread::OCP_AIS_Thread(AIS_Decoder *pParent, const wxString& PortName)
{

      m_pParentEventHandler = pParent;

      m_pPortName = new wxString(PortName);

      rx_buffer = new char[RX_BUFFER_SIZE + 1];
      temp_buf = new char[RX_BUFFER_SIZE + 1];

      put_ptr = rx_buffer;
      tak_ptr = rx_buffer;
      nl_count = 0;

      Create();
}

OCP_AIS_Thread::~OCP_AIS_Thread(void)
{
      delete m_pPortName;
      delete rx_buffer;
      delete temp_buf;

}

void OCP_AIS_Thread::OnExit(void)
{
    //  Mark the global status as dead, gone
    pAIS_Thread = NULL;
}

bool OCP_AIS_Thread::HandleRead(char *buf, int character_count)
{
    // Copy the characters into circular buffer

    char *bp = buf;
    int ichar = character_count;
    while(ichar)
    {
     // keep track of how many nl in the buffer
        if(0x0a == *bp)
            nl_count++;

        *put_ptr++ = *bp++;
        if((put_ptr - rx_buffer) > RX_BUFFER_SIZE)
            put_ptr = rx_buffer;

        ichar--;
    }

///    wxString msg;
///    msg.Printf(_T("(put_ptr-tak_ptr): %d      character_count: %d        nl_count: %d"), (int)(put_ptr-tak_ptr), character_count, nl_count);
///    wxLogMessage(msg);

    if(nl_count < 0)                // "This will never happen..."
        nl_count = 0;

    // If there are any nl's in the buffer
    // There is at least one sentences in the buffer,
    // plus maybe a partial
    // Try hard to send multiple sentences

    if(nl_count)
    {
        bool bmore_data = true;

        while((bmore_data) && (nl_count))
        {
            if(put_ptr != tak_ptr)
            {
                //copy the buffer, looking for nl
                char *tptr;
                char *ptmpbuf;

                tptr = tak_ptr;
                ptmpbuf = temp_buf;

                while((*tptr != 0x0a) && (tptr != put_ptr))
                {
                    *ptmpbuf++ = *tptr++;

                    if((tptr - rx_buffer) > RX_BUFFER_SIZE)
                        tptr = rx_buffer;
                }

                if((*tptr == 0x0a) && (tptr != put_ptr))                // well formed sentence
                {
                    *ptmpbuf++ = *tptr++;
                    if((tptr - rx_buffer) > RX_BUFFER_SIZE)
                        tptr = rx_buffer;

                    *ptmpbuf = 0;

                    tak_ptr = tptr;

    //    Signal the parent program thread

                    OCPN_AISEvent event(wxEVT_OCPN_AIS , ID_AIS_WINDOW );
                    event.SetEventObject( (wxObject *)this );
                    event.SetExtraLong(EVT_AIS_PARSE_RX);
                    event.SetNMEAString(wxString(temp_buf,  wxConvUTF8));
                    m_pParentEventHandler->AddPendingEvent(event);

///     msg.Printf(_T("         removing: %d,   (put_ptr-tak_ptr): %d,   nl_count:%d"), strlen(temp_buf), (int)(put_ptr-tak_ptr), nl_count);
///     wxLogMessage(msg);

                    nl_count--;
                }   // if good sentence
                else                                 // partial sentence
                {
                    bmore_data = false;
///                    msg.Printf(_T("         partial...   nl_count:%d"), nl_count);
///                    wxLogMessage(msg);
                }

            }   // if data
            else
            {
                bmore_data = false;                  // no more data in buffer
///                msg.Printf(_T("         no data...   nl_count:%d"), nl_count);
///                wxLogMessage(msg);
            }

        }       // while
    }       // if nl

    return true;
}


//      Sadly, the thread itself must implement the underlying OS serial port
//      in a very machine specific way....

#ifdef __POSIX__
//    Entry Point
void *OCP_AIS_Thread::Entry()
{
#if 0
                                     // testing direct file read of AIS data stream
      char buf[100];
      wxFile f("/home/dsr/Desktop/Downloads/SIITECHBIG.TXT");
                           if(f.IsOpened())
                           int yyo = 5;

                           wxFileInputStream stream(f);

                           if(stream.IsOk())
                           int yyp = 5;

                           while(!stream.Eof())
               {
                           stream.Read(buf, 40);
                           HandleRead(buf, 40);                   // Read completed successfully.
}

                           return 0;
#endif
          // Allocate the termios data structures

      pttyset = (termios *)calloc(sizeof (termios), 1);
      pttyset_old = (termios *)malloc(sizeof (termios));

    // Open the requested port.
    //   using O_NDELAY to force ignore of DCD (carrier detect) MODEM line
    if ((m_ais_fd = open(m_pPortName->fn_str(), O_RDWR|O_NDELAY|O_NOCTTY)) < 0)
    {
        wxString msg(_T("AIS input device open failed: "));
        msg.Append(*m_pPortName);
        wxLogMessage(msg);
        return 0;
    }

    /*
     A special test for a user defined FIFO
     To use this method, do the following:
     a.  Create a fifo            $mkfifo /tmp/aisfifo
     b.  netcat into the fifo     $nc {ip} {port} > /tmp/aisfifo
                     sample {ip} {port} could be  nc 82.182.117.51 6401 > /tmp/aisfifo
     c.  hand edit opencpn.conf and make AIS data source like this:
          [Settings/AISPort]
          Port=Serial:/tmp/aisfifo

    This also works if you have an ascii ais log
    for which you can simply $cat ais.log > /tmp/aisfifo
    */
    if(m_pPortName->Contains(_T("fifo")))
          goto port_ready;


      tcsetattr(m_ais_fd, TCSANOW, pttyset);

    if (isatty(m_ais_fd) == 0)
    {
        wxString msg(_T("AIS tty input device isatty() failed, retrying open()...  "));
        msg.Append(*m_pPortName);
        wxLogMessage(msg);

           close(m_ais_fd);
         if ((m_ais_fd = open(m_pPortName->fn_str(), O_RDWR|O_NDELAY|O_NOCTTY)) < 0)
           {
               wxString msg(_T("AIS tty input device open failed on retry, aborting.  "));
               msg.Append(*m_pPortName);
               wxLogMessage(msg);
               close(m_ais_fd);
               return(0);
           }

           if (isatty(m_ais_fd) == 0)
           {
               wxString msg(_T("AIS tty input device isatty() failed on retry, aborting.  "));
               msg.Append(*m_pPortName);
               wxLogMessage(msg);
               close(m_ais_fd);
               return(0);
           }
    }

    if (1/*isatty(m_ais_fd) != 0*/)
    {

      /* Save original terminal parameters */
      if (tcgetattr(m_ais_fd,pttyset_old) != 0)
      {
          wxString msg(_T("AIS tty input device getattr() failed, retrying...  "));
          msg.Append(*m_pPortName);
          wxLogMessage(msg);

        close(m_ais_fd);
        if ((m_ais_fd = open(m_pPortName->fn_str(), O_RDWR|O_NDELAY|O_NOCTTY)) < 0)
          {
              wxString msg(_T("AIS tty input device open failed on retry, aborting.  "));
              msg.Append(*m_pPortName);
              wxLogMessage(msg);
              return 0;
          }

        if (tcgetattr(m_ais_fd,pttyset_old) != 0)
          {
              wxString msg(_T("AIS tty input device getattr failed on retry, aborting.  "));
              msg.Append(*m_pPortName);
              wxLogMessage(msg);
              return 0;
          }
      }

      memcpy(pttyset, pttyset_old, sizeof(termios));

      //  Build the new parms off the old

      // Set blocking/timeout behaviour
      memset(pttyset->c_cc,0,sizeof(pttyset->c_cc));

      pttyset->c_cc[VTIME] = 11;                        // 1.1 sec timeout
      pttyset->c_cc[VEOF]  = 4;                         // EOF Character

      /*
      * No Flow Control
      */
      pttyset->c_cflag &= ~(PARENB | PARODD | CRTSCTS);
      pttyset->c_cflag |= CREAD | CLOCAL;
      pttyset->c_iflag = pttyset->c_oflag = pttyset->c_lflag = (tcflag_t) 0;

      int stopbits = 1;
      char parity = 'N';
      pttyset->c_iflag &=~ (PARMRK | INPCK);
      pttyset->c_cflag &=~ (CSIZE | CSTOPB | PARENB | PARODD);
      pttyset->c_cflag |= (stopbits==2 ? CS7|CSTOPB : CS8);
      switch (parity)
      {
          case 'E':
              pttyset->c_iflag |= INPCK;
              pttyset->c_cflag |= PARENB;
              break;
          case 'O':
              pttyset->c_iflag |= INPCK;
              pttyset->c_cflag |= PARENB | PARODD;
              break;
      }
      pttyset->c_cflag &=~ CSIZE;
      pttyset->c_cflag |= (CSIZE & (stopbits==2 ? CS7 : CS8));

      cfsetispeed(pttyset, B38400);
      cfsetospeed(pttyset, B38400);

      if (tcsetattr(m_ais_fd, TCSANOW, pttyset) != 0)
      {
          wxString msg(_T("AIS tty input device setattr() failed  "));
          msg.Append(*m_pPortName);
          wxLogMessage(msg);
          return 0;
      }

      (void)tcflush(m_ais_fd, TCIOFLUSH);
    }


port_ready:

    bool not_done = true;
    char next_byte = 0;
    ssize_t newdata = 0;

//    The main loop
//    printf("starting\n");


    while((not_done) && (m_pParentEventHandler->m_Thread_run_flag > 0))
    {
        if(TestDestroy())
        {
            not_done = false;                               // smooth exit
        }

//#define oldway 1
#ifdef oldway
//    Blocking, timeout protected read of one character at a time
//    Timeout value is set by c_cc[VTIME]
//    Storing incoming characters in circular buffer
//    And watching for new line character
//     On new line character, send notification to parent


        newdata = read(m_ais_fd, &next_byte, 1);            // blocking read of one char
                                                            // return (-1) if no data available, timeout
#else
//      Kernel I/O multiplexing provides a cheap way to wait for chars

        fd_set rfds;
        struct timeval tv;
        int retval;

        //      m_ais_fd to see when it has input.
        FD_ZERO(&rfds);
        FD_SET(m_ais_fd, &rfds);
        // Wait up to 1 second
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        newdata = 0;
        next_byte = 0;

//      wait for a read available on m_ais_fd, we don't care about write or exceptions
        retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);

        if(FD_ISSET(m_ais_fd, &rfds) && (retval != -1) && retval)
            newdata = read(m_ais_fd, &next_byte, 1);            // blocking read of one char
                                                                // bound to succeed
#endif

        if(newdata > 0)
            HandleRead(&next_byte, 1);

    }           // the big while


//          Close the port cleanly
// this is the clean way to do it
//    pttyset_old->c_cflag |= HUPCL;
//    (void)tcsetattr(m_ais_fd,TCSANOW,pttyset_old);
    close(m_ais_fd);

    free (pttyset);
    free (pttyset_old);

    m_pParentEventHandler->m_Thread_run_flag = -1;

    return 0;

}


#endif          //__POSIX__


#ifdef __WXMSW__
//    Entry Point
void *OCP_AIS_Thread::Entry()
{


      bool not_done;

      DWORD dwRead;
      BOOL fWaitingOnRead = FALSE;
      OVERLAPPED osReader = {0};
#define READ_BUF_SIZE 1000
#define READ_LEN_REQUEST 40
      char buf[READ_BUF_SIZE];

#define READ_TIMEOUT      500      // milliseconds

      DWORD dwRes;
      DWORD dwError;

#if 0                                     // testing direct file read of AIS data stream
      wxFile f("C:\\SIITECH.TXT");

      wxFileInputStream stream(f);

      if(stream.IsOk())
            int yyp = 5;

      while(!stream.Eof())
      {
            stream.Read(buf, 40);
            HandleRead(buf, 40);                   // Read completed successfully.
      }

      return 0;
#endif

//    Set up the serial port

      m_hSerialComm = CreateFile(m_pPortName->fn_str(),      // Port Name
                                             GENERIC_READ,              // Desired Access
                                             0,                         // Shared Mode
                                             NULL,                      // Security
                                             OPEN_EXISTING,             // Creation Disposition
                                             FILE_FLAG_OVERLAPPED,
                                             NULL);                     //  Overlapped

      if(m_hSerialComm == INVALID_HANDLE_VALUE)
      {
            error = ::GetLastError();
            goto fail_point;
      }


      if(!SetupComm(m_hSerialComm, 1024, 1024))
            goto fail_point;

      DCB dcbConfig;

      if(GetCommState(m_hSerialComm, &dcbConfig))           // Configuring Serial Port Settings
      {
            dcbConfig.BaudRate = 38400;
            dcbConfig.ByteSize = 8;
            dcbConfig.Parity = NOPARITY;
            dcbConfig.StopBits = ONESTOPBIT;
            dcbConfig.fBinary = TRUE;
            dcbConfig.fParity = TRUE;
      }

      else
            goto fail_point;

      if(!SetCommState(m_hSerialComm, &dcbConfig))
            goto fail_point;

      COMMTIMEOUTS commTimeout;

      TimeOutInSec = 2;

      if(GetCommTimeouts(m_hSerialComm, &commTimeout)) // Configuring Read & Write Time Outs
      {
            commTimeout.ReadIntervalTimeout = 1000*TimeOutInSec;
            commTimeout.ReadTotalTimeoutConstant = 1000*TimeOutInSec;
            commTimeout.ReadTotalTimeoutMultiplier = 0;
            commTimeout.WriteTotalTimeoutConstant = 1000*TimeOutInSec;
            commTimeout.WriteTotalTimeoutMultiplier = 0;
      }

      else
            goto fail_point;

      if(!SetCommTimeouts(m_hSerialComm, &commTimeout))
            goto fail_point;


//    Set up event specification

      if(!SetCommMask(m_hSerialComm, EV_RXCHAR)) // Setting Event Type
            goto fail_point;


// Create the overlapped event. Must be closed before exiting
// to avoid a handle leak.
      osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
      if (osReader.hEvent == NULL)
            goto fail_point;                                     // Error creating overlapped event; abort.

///
///
//    The main loop
      not_done = true;

      while((not_done) && (m_pParentEventHandler->m_Thread_run_flag > 0))
      {
            if(TestDestroy())
                  not_done = false;                                     // smooth exit

            //    Was port closed due to error condition?
            while(!m_hSerialComm)
            {
                  if(TestDestroy())
                        goto fail_point;                               // smooth exit

                  int fd;
                  if ((fd = g_pCommMan->OpenComPort(*m_pPortName, 38400)) > 0)
                  {
                        m_hSerialComm = (HANDLE)fd;
                  }
                  else
                  {
                        m_hSerialComm = NULL;
                        wxThread::Sleep(2000);                        // stall for a bit
                  }

            }

            if (!fWaitingOnRead)
            {
                        // Issue read operation.
                if (!ReadFile(m_hSerialComm, buf, READ_LEN_REQUEST, &dwRead, &osReader))
                {
                      dwError = GetLastError();
                      if (dwError != ERROR_IO_PENDING)     // retry port on all unknown errors
                      {
                            g_pCommMan->CloseComPort((int)m_hSerialComm);
                            m_hSerialComm = NULL;
                            dwRead = 0;
                            fWaitingOnRead = FALSE;
                      }
                      else                              // read delayed
                            fWaitingOnRead = TRUE;
                }
                else
                {      // read completed immediately
                      HandleRead(buf, dwRead);                         // read completed immediately
                      fWaitingOnRead = FALSE;
                }
            }
/*
                {
                    dwError = GetLastError();

                    if (dwError == ERROR_IO_PENDING)                    // read not delayed?
                          fWaitingOnRead = TRUE;

                    else if(dwError == ERROR_OPERATION_ABORTED)
                          fWaitingOnRead = TRUE;
                    else
                          ClearCommError(m_hSerialComm, NULL, NULL); //goto fail_point;  // Error in communications.
                }
                else
                      HandleRead(buf, dwRead);                         // read completed immediately
            }
*/

                if (fWaitingOnRead)
                {
                  //    Loop forever, checking for thread exit request
                      while(fWaitingOnRead)
                      {
                            if(TestDestroy())
                                  goto fail_point;                               // smooth exit


                            dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
                            switch(dwRes)
                            {
                                  case WAIT_OBJECT_0:
                                        if (!GetOverlappedResult(m_hSerialComm, &osReader, &dwRead, FALSE))
                                        {
                                              g_pCommMan->CloseComPort((int)m_hSerialComm);
                                              m_hSerialComm = NULL;
                                              dwRead = 0;
                                              fWaitingOnRead = FALSE;
                                        }
                                        else
                                        {
                                              if(dwRead)
                                                    HandleRead(buf, dwRead);                   // Read completed successfully.
                                              fWaitingOnRead = FALSE;
                                        }
                                        break;


                                  default:
                                        break;
                            }     // switch
                      }           // while
                }                 // if

      }                 // big while


fail_point:

      CloseHandle(m_hSerialComm);

      if (osReader.hEvent)
            CloseHandle(osReader.hEvent);

      m_pParentEventHandler->m_Thread_run_flag = -1;

      return 0;
}

#endif            //__WXMSW__


//---------------------------------------------------------------------------------------
//          AISTargetAlertDialog Implementation
//---------------------------------------------------------------------------------------
IMPLEMENT_CLASS ( AISTargetAlertDialog, wxDialog )


// AISTargetAlertDialog event table definition

            BEGIN_EVENT_TABLE ( AISTargetAlertDialog, wxDialog )

            EVT_CLOSE(AISTargetAlertDialog::OnClose)
            EVT_BUTTON( ID_ACKNOWLEDGE, AISTargetAlertDialog::OnIdAckClick )
            EVT_BUTTON( ID_SILENCE, AISTargetAlertDialog::OnIdSilenceClick )
            EVT_MOVE( AISTargetAlertDialog::OnMove )
            EVT_SIZE( AISTargetAlertDialog::OnSize )

            END_EVENT_TABLE()


AISTargetAlertDialog::AISTargetAlertDialog( )
{
      Init();
}

AISTargetAlertDialog::~AISTargetAlertDialog( )
{
}


void AISTargetAlertDialog::Init( )
{
      m_target_mmsi = 0;
      m_pparent = NULL;

}


bool AISTargetAlertDialog::Create ( int target_mmsi,
                                    wxWindow *parent, AIS_Decoder *pdecoder,
                                    wxWindowID id, const wxString& caption,
                                    const wxPoint& pos, const wxSize& size, long style )
{

        //    As a display optimization....
        //    if current color scheme is other than DAY,
        //    Then create the dialog ..WITHOUT.. borders and title bar.
        //    This way, any window decorations set by external themes, etc
        //    will not detract from night-vision

      long wstyle = wxDEFAULT_FRAME_STYLE;
      if (( global_color_scheme != GLOBAL_COLOR_SCHEME_DAY ) && ( global_color_scheme != GLOBAL_COLOR_SCHEME_RGB ))
            wstyle |= ( wxNO_BORDER );

      wxSize size_min = size;
      size_min.IncTo(wxSize(500,600));
      if ( !wxDialog::Create ( parent, id, caption, pos, size_min, wstyle ) )
            return false;

      m_target_mmsi = target_mmsi;
      m_pparent = parent;
      m_pdecoder = pdecoder;

      wxFont *dFont = pFontMgr->GetFont(_("AISTargetAlert"), 12);
      int font_size = wxMax(8, dFont->GetPointSize());
      wxString face;
#ifdef __WXGTK__
      face = _T("Monospace");
#endif
      wxFont *fp_font = wxTheFontList->FindOrCreateFont(font_size,
                  wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, dFont->GetWeight(), false, face);

      SetFont ( *fp_font );

      wxColour back_color = GetGlobalColor ( _T ( "UIBDR" ) );
      SetBackgroundColour ( back_color );

      wxColour text_color = GetGlobalColor ( _T ( "UINFF" ) );          // or UINFD
      SetForegroundColour ( text_color );
//      SetForegroundColour(pFontMgr->GetFontColor(_T("AISTargetAlert")));

      CreateControls();

      if(CanSetTransparent())
            SetTransparent(192);

      // This fits the dialog to the minimum size dictated by
// the sizers
      GetSizer()->Fit ( this );

// This ensures that the dialog cannot be sized smaller
// than the minimum size
      GetSizer()->SetSizeHints ( this );

      return true;
}




void AISTargetAlertDialog::CreateControls()
{

// A top-level sizer
      wxBoxSizer* topSizer = new wxBoxSizer ( wxVERTICAL );
      SetSizer ( topSizer );

// A second box sizer to give more space around the controls
      wxBoxSizer* boxSizer = new wxBoxSizer ( wxVERTICAL );
      topSizer->Add ( boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 2 );

// Here is the query result

      m_pAlertTextCtl = new AISInfoWin ( this );
      m_pAlertTextCtl->SetHPad(8);
      m_pAlertTextCtl->SetVPad(2);


      wxColour back_color =GetGlobalColor ( _T ( "UIBCK" ) );
      m_pAlertTextCtl->SetBackgroundColour ( back_color );

      wxColour text_color = GetGlobalColor ( _T ( "UINFF" ) );          // or UINFD
      m_pAlertTextCtl->SetForegroundColour ( text_color );
//      m_pAlertTextCtl->SetForegroundColour(pFontMgr->GetFontColor(_T("AISTargetAlert")));


//      wxString alert_text;
//      int n_nl;
//      wxSize pix_size;

      if(GetAlertText())
      {
            m_pAlertTextCtl->AppendText ( m_alert_text );

            wxSize osize = m_pAlertTextCtl->GetOptimumSize();

            m_pAlertTextCtl->SetSize(osize);
            boxSizer->SetMinSize(osize);
            boxSizer->FitInside(m_pAlertTextCtl);
      }
      boxSizer->Add ( m_pAlertTextCtl, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 1 );



// A horizontal box sizer to contain Ack
      wxBoxSizer* AckBox = new wxBoxSizer ( wxHORIZONTAL );
      boxSizer->Add ( AckBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

//    Button color
      wxColour button_color = GetGlobalColor ( _T ( "UIBCK" ) );;

// The Silence button
      wxButton* silence = new wxButton ( this, ID_SILENCE, _( "&Silence Alert" ),
                                    wxDefaultPosition, wxDefaultSize, 0 );
      AckBox->Add ( silence, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
      silence->SetBackgroundColour ( button_color );

// The Ack button
      wxButton* ack = new wxButton ( this, ID_ACKNOWLEDGE, _( "&Acknowledge" ),
                                     wxDefaultPosition, wxDefaultSize, 0 );
      AckBox->Add ( ack, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
      ack->SetBackgroundColour ( button_color );

}

bool AISTargetAlertDialog::GetAlertText()
{
      //    Search the parent AIS_Decoder's target list for specified mmsi

      if(m_pdecoder)
      {
            AIS_Target_Data *td_found = m_pdecoder->Get_Target_Data_From_MMSI(Get_Dialog_MMSI());

            if(td_found)
            {
                  m_alert_text = td_found->BuildQueryResult();
                  return true;
            }
            else
                  return false;
      }
      else
            return false;
}

void AISTargetAlertDialog::UpdateText()
{
      if(GetAlertText())
      {
            m_pAlertTextCtl->Clear();
            m_pAlertTextCtl->AppendText ( m_alert_text );
      }
      if(CanSetTransparent())
            SetTransparent(192);

      m_pAlertTextCtl->SetInsertionPoint(0);
}


void AISTargetAlertDialog::OnClose(wxCloseEvent& event)
{
      Destroy();
      g_pais_alert_dialog_active = NULL;
}


void AISTargetAlertDialog::OnIdAckClick( wxCommandEvent& event )
{
      //    Acknowledge the Alert, and dismiss the dialog
      if(m_pdecoder)
      {
            AIS_Target_Data *td = m_pdecoder->Get_Target_Data_From_MMSI(Get_Dialog_MMSI());
            if(td)
            {
                  if(AIS_ALARM_SET == td->n_alarm_state)
                  {
//                        td->n_alarm_state = AIS_ALARM_ACKNOWLEDGED;
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
      if(m_pdecoder)
      {
            AIS_Target_Data *td = m_pdecoder->Get_Target_Data_From_MMSI(Get_Dialog_MMSI());
            if(td)
                  td->b_suppress_audio = true;
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
class OCPNListCtrl: public wxListCtrl
{
public:
      OCPNListCtrl(AISTargetListDialog* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style);
      ~OCPNListCtrl();

      wxString OnGetItemText(long item, long column) const;
      int OnGetItemColumnImage(long item, long column) const;

      wxString GetTargetColumnData(AIS_Target_Data *pAISTarget, long column) const;

      AISTargetListDialog     *m_parent;


};


OCPNListCtrl::OCPNListCtrl(AISTargetListDialog* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style):
            wxListCtrl(parent, id, pos, size, style)
{
      m_parent = parent;
}

OCPNListCtrl::~OCPNListCtrl()
{
            g_AisTargetList_column_spec.Clear();
            for(int i=0 ; i < tlSOG + 1 ; i++)
            {
                  wxListItem item;
                  GetColumn(i, item);
                  wxString sitem;
                  sitem.Printf(_T("%d;"), item.m_width);
                  g_AisTargetList_column_spec += sitem;
            }
}


wxString  OCPNListCtrl::OnGetItemText(long item, long column) const
{
      wxString ret;

      if(m_parent->m_pListCtrlAISTargets)
      {
            AIS_Target_Data *pAISTarget = m_parent->GetpTarget(item);
            if(pAISTarget)
                  ret = GetTargetColumnData(pAISTarget, column);
      }

      return ret;
}


int OCPNListCtrl::OnGetItemColumnImage(long item, long column) const
{
      return -1;
}

wxString OCPNListCtrl::GetTargetColumnData(AIS_Target_Data *pAISTarget, long column) const
{
      wxString ret;

      if(pAISTarget)
      {
            switch (column)
            {
                  case tlNAME:
                        if( (pAISTarget->Class == AIS_BASE) || (pAISTarget->Class == AIS_SART))
                              ret =  _("-");
                        else
                        {
                              wxString uret = trimAISField(pAISTarget->ShipName);
                              if(uret == _T("Unknown"))
                                    ret = wxGetTranslation(uret);
                              else
                                    ret = uret;

                              if(strlen(pAISTarget->ShipNameExtension))
                                    ret.Append(wxString(pAISTarget->ShipNameExtension, wxConvUTF8));
                        }
                        break;

                  case tlCALL:
                        ret = trimAISField(pAISTarget->CallSign);
                        break;

                  case tlMMSI:
                        if (pAISTarget->Class != AIS_GPSG_BUDDY)
                              ret.Printf(_T("%09d"), abs(pAISTarget->MMSI));
				else
                              ret.Printf(_T("   nil   "));
                        break;

                  case tlCLASS:
                        ret = wxGetTranslation(pAISTarget->Get_class_string(true));
                        break;

                  case tlTYPE:
                        if((pAISTarget->Class == AIS_BASE) || (pAISTarget->Class == AIS_SART))
                              ret =  _("-");
                        else
                              ret = wxGetTranslation(pAISTarget->Get_vessel_type_string(false));
                        break;

                  case tlNAVSTATUS:
                  {
                        if((pAISTarget->NavStatus <= 20) && (pAISTarget->NavStatus >= 0))
                              ret =  wxGetTranslation(ais_status[pAISTarget->NavStatus]);
                        else
                              ret = _("-");
                        if( (pAISTarget->Class == AIS_ATON)||(pAISTarget->Class == AIS_BASE)||(pAISTarget->Class == AIS_CLASS_B))
                              ret =  _("-");
                        break;
                  }

                  case tlBRG:
                  {
                        if(pAISTarget->b_positionOnceValid && bGPSValid && (pAISTarget->Brg >= 0.))
                        {
                              int brg = (int)wxRound(pAISTarget->Brg);
                              if(pAISTarget->Brg > 359.5)
                                    brg = 0;

                              ret.Printf(_T("%03d"), brg);
                        }
                        else
                             ret = _("-");
                        break;
                  }

                  case tlCOG:
                  {
                        if( (pAISTarget->COG >= 360.0) || (pAISTarget->Class == AIS_ATON) || (pAISTarget->Class == AIS_BASE))
                              ret =  _("-");
                        else
                        {
                              int crs = wxRound( pAISTarget->COG);
                              if(crs == 360)
                                    ret.Printf(_T("  000"));
                              else
                                    ret.Printf(_T("  %03d"), crs);
                        }
                        break;
                  }

                  case tlSOG:
                  {
                        if( (pAISTarget->SOG > 100.) || (pAISTarget->Class == AIS_ATON) || (pAISTarget->Class == AIS_BASE))
                              ret = _("-");
                        else
                              ret.Printf(_T("%5.1f"), pAISTarget->SOG);
                        break;
                  }

                  case tlRNG:
                  {
                        if(pAISTarget->b_positionOnceValid && bGPSValid && (pAISTarget->Range_NM >= 0.))
                              ret.Printf(_T("%5.2f"), pAISTarget->Range_NM);
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
//WX_DEFINE_SORTED_ARRAY(AIS_Target_Data *,ArrayOfAISTarget);


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
      if(g_AisTargetList_count > 1000)
            return 0;

      AIS_Target_Data *t1 = pAISTarget1;
      AIS_Target_Data *t2 = pAISTarget2;

      switch (g_AisTargetList_sortColumn)
      {
            case tlNAME:
                  s1 =  trimAISField(t1->ShipName);
                  if( (t1->Class == AIS_BASE) || (t1->Class == AIS_SART))
                        s1 =  _T("-");

                  s2 =  trimAISField(t2->ShipName);
                  if( (t2->Class == AIS_BASE) || (t2->Class == AIS_SART))
                        s2 =  _T("-");
                  break;

            case tlCALL:
                  s1 = trimAISField(t1->CallSign);
                  s2 = trimAISField(t2->CallSign);
                  break;

            case tlMMSI:
                  n1 = t1->MMSI;
                  n2 = t2->MMSI;
                  b_cmptype_num = true;
                  break;

            case tlCLASS:
                  s1 = t1->Get_class_string(true);
                  s2 = t2->Get_class_string(true);
                  break;

            case tlTYPE:
                  s1 = t1->Get_vessel_type_string(false);
                  if( (t1->Class == AIS_BASE) || (t1->Class == AIS_SART))
                        s1 =  _T("-");

                  s2 = t2->Get_vessel_type_string(false);
                  if( (t1->Class == AIS_BASE) || (t1->Class == AIS_SART))
                        s2 =  _T("-");
                  break;

            case tlNAVSTATUS:
            {
                  if((t1->NavStatus <= 15) && (t1->NavStatus >= 0))
                        s1 =  ais_status[t1->NavStatus];
                  else
                        s1 = _("-");

                  if( (t1->Class == AIS_BASE))
                        s1 =  _T("-");

                  if((t2->NavStatus <= 15) && (t2->NavStatus >= 0))
                        s2 =  ais_status[t2->NavStatus];
                  else
                        s2 = _("-");

                  if( (t2->Class == AIS_BASE))
                        s2 =  _T("-");

                  break;
            }

            case tlBRG:
            {
                  int brg1 = wxRound( t1->Brg);
                  if(brg1 == 360)
                        n1=0.;
                  else
                        n1=brg1;

                  int brg2 = wxRound( t2->Brg);
                  if(brg2 == 360)
                        n2=0.;
                  else
                        n2=brg2;

                  b_cmptype_num = true;
                  break;
            }

            case tlCOG:
            {
                  if( (t1->COG >= 360.0) || (t1->Class == AIS_ATON) || (t1->Class == AIS_BASE))
                        n1=-1.0;
                  else
                  {
                        int crs = wxRound( t1->COG);
                        if(crs == 360)
                              n1=0.;
                        else
                              n1=crs;
                  }


                  if( (t2->COG >= 360.0) || (t2->Class == AIS_ATON) || (t2->Class == AIS_BASE))
                        n2=-1.0;
                  else
                  {
                        int crs = wxRound( t2->COG);
                        if(crs == 360)
                              n2=0.;
                        else
                              n2=crs;
                  }

                  b_cmptype_num = true;
                  break;
            }

            case tlSOG:
            {
                  if( (t1->SOG > 100.) || (t1->Class == AIS_ATON) || (t1->Class == AIS_BASE))
                        n1 = -1.0;
                  else
                        n1 = t1->SOG;

                  if( (t2->SOG > 100.) || (t2->Class == AIS_ATON) || (t2->Class == AIS_BASE))
                        n2 = -1.0;
                  else
                        n2 = t2->SOG;

                  b_cmptype_num = true;
                  break;
            }

            case tlRNG:
            {
                  n1 = t1->Range_NM;
                  n2 = t2->Range_NM;
                  b_cmptype_num = true;
                  break;
            }

            default:
                  break;
      }

      if(!b_cmptype_num)
      {
            if (g_bAisTargetList_sortReverse)
                  return s2.Cmp(s1);
            return s1.Cmp(s2);
      }
      else
      {
            //    If numeric sort values are equal, secondary sort is on Range_NM
            if (g_bAisTargetList_sortReverse)
            {
                  if(n2 > n1)
                        return 1;
                  else if(n2 < n1)
                        return -1;
                  else return (t1->Range_NM > t2->Range_NM); //0;
            }
            else
            {
                  if(n2 > n1)
                        return -1;
                  else if(n2 < n1)
                        return 1;
                  else return (t1->Range_NM > t2->Range_NM); //0;
            }
      }
}

AIS_Decoder *g_p_sort_decoder;

int ArrayItemCompareMMSI( int MMSI1, int MMSI2 )
{

      if(g_p_sort_decoder)
      {
            AIS_Target_Data *pAISTarget1 = g_p_sort_decoder->Get_Target_Data_From_MMSI(MMSI1);
            AIS_Target_Data *pAISTarget2 = g_p_sort_decoder->Get_Target_Data_From_MMSI(MMSI2);

            if(pAISTarget1 && pAISTarget2)
                  return ItemCompare( pAISTarget1, pAISTarget2 );
            else
                  return 0;
      }
      else
            return 0;
}


IMPLEMENT_CLASS ( AISTargetListDialog, wxPanel )

            BEGIN_EVENT_TABLE(AISTargetListDialog, wxPanel)
            EVT_CLOSE(AISTargetListDialog::OnClose)

            END_EVENT_TABLE()

AISTargetListDialog::AISTargetListDialog( wxWindow *parent, wxAuiManager *auimgr, AIS_Decoder *pdecoder)
      :wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 780, 250 ), wxBORDER_NONE )
{
      m_pparent = parent;
      m_pAuiManager = auimgr;
      m_pdecoder = pdecoder;

      g_p_sort_decoder = pdecoder;
      m_pMMSI_array = new ArrayOfMMSI(ArrayItemCompareMMSI); //ArrayOfAISTarget(ArrayItemCompare);

// A top-level sizer
      wxBoxSizer* topSizer = new wxBoxSizer ( wxHORIZONTAL );
      SetSizer( topSizer );


      //  Parse the global column width string as read from config file
      wxStringTokenizer tkz(g_AisTargetList_column_spec, _T(";"));
      wxString s_width = tkz.GetNextToken();
      int width;
      long lwidth;


      m_pListCtrlAISTargets = new OCPNListCtrl(this, ID_AIS_TARGET_LIST, wxDefaultPosition, wxDefaultSize,
                                               wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_HRULES|wxLC_VRULES|wxBORDER_SUNKEN|wxLC_VIRTUAL );
      wxImageList *imglist = new wxImageList( 16, 16, true, 2 );
      imglist->Add(*_img_sort_asc);
      imglist->Add(*_img_sort_desc);
      m_pListCtrlAISTargets->AssignImageList( imglist, wxIMAGE_LIST_SMALL );
      m_pListCtrlAISTargets->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( AISTargetListDialog::OnTargetSelected ), NULL, this );
      m_pListCtrlAISTargets->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( AISTargetListDialog::OnTargetSelected ), NULL, this );
      m_pListCtrlAISTargets->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler( AISTargetListDialog::OnTargetDefaultAction ), NULL, this );
      m_pListCtrlAISTargets->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( AISTargetListDialog::OnTargetListColumnClicked ), NULL, this );

      width = 105;
      if(s_width.ToLong(&lwidth)){ width = wxMax(20, lwidth);  width = wxMin(width, 250);}
      m_pListCtrlAISTargets->InsertColumn( tlNAME, _("Name"), wxLIST_FORMAT_LEFT, width );
      s_width = tkz.GetNextToken();

      width = 55;
      if(s_width.ToLong(&lwidth)){ width = wxMax(20, lwidth);  width = wxMin(width, 250);}
      m_pListCtrlAISTargets->InsertColumn( tlCALL, _("Call"), wxLIST_FORMAT_LEFT, width);
      s_width = tkz.GetNextToken();

      width = 80;
      if(s_width.ToLong(&lwidth)){ width = wxMax(20, lwidth);  width = wxMin(width, 250);}
      m_pListCtrlAISTargets->InsertColumn( tlMMSI, _("MMSI"), wxLIST_FORMAT_LEFT, width );
      s_width = tkz.GetNextToken();

      width = 55;
      if(s_width.ToLong(&lwidth)){ width = wxMax(20, lwidth);  width = wxMin(width, 250);}
      m_pListCtrlAISTargets->InsertColumn( tlCLASS, _("Class"), wxLIST_FORMAT_CENTER, width );
      s_width = tkz.GetNextToken();

      width = 80;
      if(s_width.ToLong(&lwidth)){ width = wxMax(20, lwidth);  width = wxMin(width, 250);}
      m_pListCtrlAISTargets->InsertColumn( tlTYPE, _("Type"), wxLIST_FORMAT_LEFT, width );
      s_width = tkz.GetNextToken();

      width = 90;
      if(s_width.ToLong(&lwidth)){ width = wxMax(20, lwidth);  width = wxMin(width, 250);}
      m_pListCtrlAISTargets->InsertColumn( tlNAVSTATUS, _("Nav Status"), wxLIST_FORMAT_LEFT, width );
      s_width = tkz.GetNextToken();

      width = 45;
      if(s_width.ToLong(&lwidth)){ width = wxMax(20, lwidth);  width = wxMin(width, 250);}
      m_pListCtrlAISTargets->InsertColumn( tlBRG, _("Brg"), wxLIST_FORMAT_RIGHT, width );
      s_width = tkz.GetNextToken();

      width = 62;
      if(s_width.ToLong(&lwidth)){ width = wxMax(20, lwidth);  width = wxMin(width, 250);}
      m_pListCtrlAISTargets->InsertColumn( tlRNG, _("Range"), wxLIST_FORMAT_RIGHT, width );
      s_width = tkz.GetNextToken();

      width = 50;
      if(s_width.ToLong(&lwidth)){ width = wxMax(20, lwidth);  width = wxMin(width, 250);}
      m_pListCtrlAISTargets->InsertColumn( tlCOG, _("CoG"), wxLIST_FORMAT_RIGHT, width );
      s_width = tkz.GetNextToken();

      width = 50;
      if(s_width.ToLong(&lwidth)){ width = wxMax(20, lwidth);  width = wxMin(width, 250);}
      m_pListCtrlAISTargets->InsertColumn( tlSOG, _("SoG"), wxLIST_FORMAT_RIGHT, width );

      wxListItem item;
      item.SetMask(wxLIST_MASK_IMAGE);
      item.SetImage( g_bAisTargetList_sortReverse ? 1 : 0 );
      g_AisTargetList_sortColumn = wxMax(g_AisTargetList_sortColumn, 0);
      m_pListCtrlAISTargets->SetColumn( g_AisTargetList_sortColumn, item );

      topSizer->Add( m_pListCtrlAISTargets, 1, wxEXPAND|wxALL, 0 );

      wxBoxSizer* boxSizer02 = new wxBoxSizer( wxVERTICAL );
      boxSizer02->AddSpacer( 22 );
      //wxBitmapButton* button01 = new wxBitmapButton( this, wxID_ANY, *_img_ais_info, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
      m_pButtonInfo = new wxButton( this, wxID_ANY, _("Target info"), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
      m_pButtonInfo->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( AISTargetListDialog::OnTargetQuery ), NULL, this );
      boxSizer02->Add( m_pButtonInfo, 0, wxALL, 0 );
      boxSizer02->AddSpacer( 5 );
      //wxBitmapButton* button02 = new wxBitmapButton( this, wxID_ANY, *_img_ais_zoom, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
      m_pButtonJumpTo = new wxButton( this, wxID_ANY, _("Jump To"), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
      m_pButtonJumpTo->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( AISTargetListDialog::OnTargetScrollTo ), NULL, this );
      boxSizer02->Add( m_pButtonJumpTo, 0, wxALL, 0 );
      boxSizer02->AddSpacer( 10 );

      m_pStaticTextRange = new wxStaticText( this, wxID_ANY, _("Limit range: NM"), wxDefaultPosition, wxDefaultSize, 0 );
      boxSizer02->Add( m_pStaticTextRange, 0, wxALL, 0 );
      boxSizer02->AddSpacer( 2 );
      m_pSpinCtrlRange = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50, -1 ), wxSP_ARROW_KEYS, 1, 20000, g_AisTargetList_range );
      m_pSpinCtrlRange->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler( AISTargetListDialog::OnLimitRange ), NULL, this );
      m_pSpinCtrlRange->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( AISTargetListDialog::OnLimitRange ), NULL, this );
      boxSizer02->Add( m_pSpinCtrlRange, 0, wxEXPAND|wxALL, 0 );
      topSizer->Add( boxSizer02, 0, wxEXPAND|wxALL, 2 );

      boxSizer02->AddSpacer( 10 );
      m_pStaticTextCount = new wxStaticText( this, wxID_ANY, _("Target Count"), wxDefaultPosition, wxDefaultSize, 0 );
      boxSizer02->Add( m_pStaticTextCount, 0, wxALL, 0 );

      boxSizer02->AddSpacer( 2 );
      m_pTextTargetCount = new wxTextCtrl(this, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
      boxSizer02->Add( m_pTextTargetCount, 0, wxALL, 0 );

      topSizer->Layout();

      //    This is silly, but seems to be required for __WXMSW__ build
      //    If not done, the SECOND invocation of AISTargetList fails to expand the list to the full wxSizer size....
      SetSize(GetSize().x, GetSize().y-1);

      SetColorScheme();
      UpdateButtons();


      if(m_pAuiManager)
      {
            wxAuiPaneInfo pane = wxAuiPaneInfo().Name(_T("AISTargetList")).Caption(_("AIS target list")).CaptionVisible(true).DestroyOnClose(true).Float().FloatingPosition( 50, 200 ).TopDockable(false).BottomDockable(true).LeftDockable(false).RightDockable(false).Show(true);
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

            if(NULL == MonitorFromRect(&frame_title_rect, MONITOR_DEFAULTTONULL))
                  b_reset_pos = true;
#else

            //    Make sure drag bar (title bar) of window intersects wxClient Area of screen, with a little slop...
            wxRect window_title_rect;                    // conservative estimate
            window_title_rect.x = pane.floating_pos.x;
            window_title_rect.y = pane.floating_pos.y;
            window_title_rect.width = pane.floating_size.x;
            window_title_rect.height = 30;

            wxRect ClientRect = wxGetClientDisplayRect();
            ClientRect.Deflate(60, 60);                     // Prevent the new window from being too close to the edge
            if(!ClientRect.Intersects(window_title_rect))
                  b_reset_pos = true;

#endif

            if(b_reset_pos)
                  pane.FloatingPosition(50,200);

            //    If the list got accidentally dropped on top of the chart bar, move it away....
            if(pane.IsDocked() && (pane.dock_row == 0))
            {
                  pane.Float();
                  pane.Row(1);
                  pane.Position(0);

                  g_AisTargetList_perspective = m_pAuiManager->SavePaneInfo( pane);
                  pConfig->UpdateSettings();
            }

            m_pAuiManager->AddPane( this, pane );
            m_pAuiManager->Update();

            m_pAuiManager->Connect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( AISTargetListDialog::OnPaneClose ), NULL, this );
      }
}


AISTargetListDialog::~AISTargetListDialog( )
{
     Disconnect_decoder();
     g_pAISTargetList = NULL;
}

void AISTargetListDialog::OnClose(wxCloseEvent &event)
{
      Disconnect_decoder();
}

void AISTargetListDialog::Disconnect_decoder()
{
      m_pdecoder = NULL;
}


void AISTargetListDialog::SetColorScheme()
{
      //    This needs to be done, but column headings are not affected,
      //    which makes the list unreadable at night....

      wxColour cl = GetGlobalColor( _T("DILG1") );
      SetBackgroundColour(cl);
      m_pListCtrlAISTargets->SetBackgroundColour( cl );
      m_pButtonInfo->SetBackgroundColour(cl);
      m_pButtonJumpTo->SetBackgroundColour(cl);
      m_pSpinCtrlRange->SetBackgroundColour( cl );
/* Doesn't work
      wxListItem item;
      item.SetMask( wxLIST_MASK_FORMAT );
      item.SetBackgroundColour( cl );
      for ( int i = 0; i < m_pListCtrlAISTargets->GetColumnCount(); i++ )
            m_pListCtrlAISTargets->SetColumn( i, item );
*/

      cl = GetGlobalColor( _T( "UINFD" ) );          // or UINFF
      SetForegroundColour( cl );
      m_pListCtrlAISTargets->SetForegroundColour( cl );
      m_pStaticTextRange->SetForegroundColour( cl );
      m_pSpinCtrlRange->SetForegroundColour( cl );

}

void AISTargetListDialog::OnPaneClose( wxAuiManagerEvent& event )
{
      if (event.pane->name == _T("AISTargetList"))
      {
            g_AisTargetList_perspective = m_pAuiManager->SavePaneInfo( *event.pane );
            //event.Veto();
      }
      event.Skip();
}

void AISTargetListDialog::UpdateButtons()
{
      long item = -1;
      item = m_pListCtrlAISTargets->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      bool enable = (item != -1);

      m_pButtonInfo->Enable(enable);

      if(m_pdecoder && item != -1)
      {
            AIS_Target_Data *pAISTargetSel =
                        m_pdecoder->Get_Target_Data_From_MMSI(m_pMMSI_array->Item(item));
            if(pAISTargetSel && (!pAISTargetSel->b_positionOnceValid))
                  enable = false;
      }
      m_pButtonJumpTo->Enable(enable);
}

void AISTargetListDialog::OnTargetSelected( wxListEvent &event )
{
      UpdateButtons();
}

void AISTargetListDialog::DoTargetQuery( int mmsi )
{
      ShowAISTargetQueryDialog(m_pparent, mmsi);
}

/*
** When an item is activated in AIS TArget List then opens the AIS Target Query Dialog
*/
void AISTargetListDialog::OnTargetDefaultAction( wxListEvent& event )
{
// Flav: 8/10/2010 changed the way to get the data (less function calls)
      long mmsi_no;

      if ((mmsi_no = event.GetData()))
            DoTargetQuery( mmsi_no );
}

void AISTargetListDialog::OnTargetQuery( wxCommandEvent& event )
{
      long selItemID = -1;
      selItemID = m_pListCtrlAISTargets->GetNextItem( selItemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
      if (selItemID == -1)
            return;

      if(m_pdecoder)
      {
            AIS_Target_Data *pAISTarget =
                        m_pdecoder->Get_Target_Data_From_MMSI(m_pMMSI_array->Item(selItemID));
            if(pAISTarget)
                  DoTargetQuery( pAISTarget->MMSI );
      }
}

void AISTargetListDialog::OnTargetListColumnClicked( wxListEvent &event )
{
      int key = event.GetColumn();
      wxListItem item;
      item.SetMask(wxLIST_MASK_IMAGE);
      if ( key == g_AisTargetList_sortColumn )
            g_bAisTargetList_sortReverse = !g_bAisTargetList_sortReverse;
      else
      {
            item.SetImage( -1 );
            m_pListCtrlAISTargets->SetColumn( g_AisTargetList_sortColumn, item );
            g_bAisTargetList_sortReverse = false;
            g_AisTargetList_sortColumn = key;
      }
      item.SetImage( g_bAisTargetList_sortReverse ? 1 : 0 );
      if(g_AisTargetList_sortColumn >= 0)
      {
            m_pListCtrlAISTargets->SetColumn( g_AisTargetList_sortColumn, item );
            UpdateAISTargetList();
      }
}

void AISTargetListDialog::OnTargetScrollTo( wxCommandEvent& event )
{
      long selItemID = -1;
      selItemID = m_pListCtrlAISTargets->GetNextItem( selItemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
      if (selItemID == -1)
            return;

      AIS_Target_Data *pAISTarget = NULL;
      if(m_pdecoder)
            pAISTarget = m_pdecoder->Get_Target_Data_From_MMSI(m_pMMSI_array->Item(selItemID));

      if(pAISTarget)
            gFrame->JumpToPosition(pAISTarget->Lat, pAISTarget->Lon, cc1->GetVPScale());
}

void AISTargetListDialog::OnLimitRange( wxCommandEvent& event )
{
      g_AisTargetList_range = m_pSpinCtrlRange->GetValue();
      UpdateAISTargetList();
}

AIS_Target_Data *AISTargetListDialog::GetpTarget(unsigned int list_item)
{
      return m_pdecoder->Get_Target_Data_From_MMSI(m_pMMSI_array->Item(list_item));
}

void AISTargetListDialog::UpdateAISTargetList(void)
{
      if(m_pdecoder)
      {

            int sb_position = m_pListCtrlAISTargets->GetScrollPos(wxVERTICAL);

            //    Capture the MMSI of the curently selected list item
            long selItemID = -1;
            selItemID = m_pListCtrlAISTargets->GetNextItem(selItemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

            int selMMSI = -1;
            if (selItemID != -1)
                  selMMSI = m_pMMSI_array->Item(selItemID);

            AIS_Target_Hash::iterator it;
            AIS_Target_Hash *current_targets = m_pdecoder->GetTargetList();
            wxListItem item;

            int index = 0;
            m_pMMSI_array->Clear();

            for( it = (*current_targets).begin(); it != (*current_targets).end(); ++it, ++index )
            {
                  AIS_Target_Data *pAISTarget = it->second;
                  item.SetId(index);

                  if ( NULL != pAISTarget)
                  {
                        if((pAISTarget->b_positionOnceValid) && (pAISTarget->Range_NM <= g_AisTargetList_range) )
                              m_pMMSI_array->Add(pAISTarget->MMSI);
                        else if(!pAISTarget->b_positionOnceValid)
                              m_pMMSI_array->Add(pAISTarget->MMSI);
                  }
            }

            m_pListCtrlAISTargets->SetItemCount(m_pMMSI_array->GetCount());

            g_AisTargetList_count = m_pMMSI_array->GetCount();

            m_pListCtrlAISTargets->SetScrollPos(wxVERTICAL, sb_position, false);

            //    Restore selected item
            long item_sel = 0;
            if ((selItemID != -1) && (selMMSI != -1))
            {
                  for(unsigned int i=0 ; i < m_pMMSI_array->GetCount() ; i++)
                  {
                        if(m_pMMSI_array->Item(i) == selMMSI)
                        {
                              item_sel = i;
                              break;
                        }
                  }
            }

            if(m_pMMSI_array->GetCount())
                  m_pListCtrlAISTargets->SetItemState(item_sel, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
            else
                  m_pListCtrlAISTargets->DeleteAllItems();

            wxString count;
            count.Printf(_T("%d"), m_pMMSI_array->GetCount());
            m_pTextTargetCount->ChangeValue(count);

#ifdef __WXMSW__
            m_pListCtrlAISTargets->Refresh(false);
#endif
     }

}



