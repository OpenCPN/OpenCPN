/***************************************************************************
 * $Id: dashboard_pi.cpp, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
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


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

// xw 2.8
#include <wx/filename.h>

#include <typeinfo>
#include "dashboard_pi.h"
#include "icons.h"
#include "wxJSON/jsonreader.h"
#include "wxJSON/jsonwriter.h"

wxFont *g_pFontTitle;
wxFont *g_pFontData;
wxFont *g_pFontLabel;
wxFont *g_pFontSmall;
int g_iDashSpeedMax;
int g_iDashSpeedUnit;
int g_iDashDepthUnit;
int g_iDashDistanceUnit;
int g_iDashWindSpeedUnit;

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif


// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi( void *ppimgr )
{
    return (opencpn_plugin *) new dashboard_pi( ppimgr );
}

extern "C" DECL_EXP void destroy_pi( opencpn_plugin* p )
{
    delete p;
}

//---------------------------------------------------------------------------------------------------------
//
//    Dashboard PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------
// !!! WARNING !!!
// do not change the order, add new instruments at the end, before ID_DBP_LAST_ENTRY!
// otherwise, for users with an existing opencpn.ini file, their instruments are changing !
enum {
    ID_DBP_I_POS, ID_DBP_I_SOG, ID_DBP_D_SOG, ID_DBP_I_COG, ID_DBP_D_COG, ID_DBP_I_STW,
    ID_DBP_I_HDT, ID_DBP_D_AW, ID_DBP_D_AWA, ID_DBP_I_AWS, ID_DBP_D_AWS, ID_DBP_D_TW,
    ID_DBP_I_DPT, ID_DBP_D_DPT, ID_DBP_I_TMP, ID_DBP_I_VMG, ID_DBP_D_VMG, ID_DBP_I_RSA,
    ID_DBP_D_RSA, ID_DBP_I_SAT, ID_DBP_D_GPS, ID_DBP_I_PTR, ID_DBP_I_CLK, ID_DBP_I_SUN,
    ID_DBP_D_MON, ID_DBP_I_ATMP, ID_DBP_I_AWA, ID_DBP_I_TWA, ID_DBP_I_TWD, ID_DBP_I_TWS,
    ID_DBP_D_TWD, ID_DBP_I_HDM, ID_DBP_D_HDT, ID_DBP_D_WDH, ID_DBP_I_VLW1, ID_DBP_I_VLW2, ID_DBP_D_MDA, ID_DBP_I_MDA,ID_DBP_D_BPH, ID_DBP_I_FOS,
	ID_DBP_M_COG, ID_DBP_I_PITCH, ID_DBP_I_HEEL, ID_DBP_D_AWA_TWA,
    ID_DBP_LAST_ENTRY //this has a reference in one of the routines; defining a "LAST_ENTRY" and setting the reference to it, is one codeline less to change (and find) when adding new instruments :-)
};

bool IsObsolete( int id ) {
    switch( id ) {
        case ID_DBP_D_AWA: return true;
        default: return false;
    }
}

wxString getInstrumentCaption( unsigned int id )
{
    switch( id ){
        case ID_DBP_I_POS:
            return _("Position");
        case ID_DBP_I_SOG:
            return _("SOG");
        case ID_DBP_D_SOG:
            return _("Speedometer");
        case ID_DBP_I_COG:
            return _("COG");
        case ID_DBP_M_COG:
            return _("Mag COG");
        case ID_DBP_D_COG:
            return _("GPS Compass");
        case ID_DBP_D_HDT:
            return _("True Compass");
        case ID_DBP_I_STW:
            return _("STW");
        case ID_DBP_I_HDT:
            return _("True HDG");
        case ID_DBP_I_HDM:
            return _("Mag HDG");
        case ID_DBP_D_AW:
        case ID_DBP_D_AWA:
            return _("App. Wind Angle & Speed");
		case ID_DBP_D_AWA_TWA:
			return _("App & True Wind Angle");
        case ID_DBP_I_AWS:
            return _("App. Wind Speed");
        case ID_DBP_D_AWS:
            return _("App. Wind Speed");
        case ID_DBP_D_TW:
            return _("True Wind Angle & Speed");
        case ID_DBP_I_DPT:
            return _("Depth");
        case ID_DBP_D_DPT:
            return _("Depth");
	case ID_DBP_D_MDA:
            return _("Barometric pressure");
        case ID_DBP_I_MDA:
            return _("Barometric pressure");
        case ID_DBP_I_TMP:
            return _("Water Temp.");
        case ID_DBP_I_ATMP:
            return _("Air Temp.");
        case ID_DBP_I_AWA:
            return _("App. Wind Angle");
        case ID_DBP_I_TWA:
            return _("True Wind Angle");
        case ID_DBP_I_TWD:
            return _("True Wind Direction");
        case ID_DBP_I_TWS:
            return _("True Wind Speed");
        case ID_DBP_D_TWD:
            return _("True Wind Dir. & Speed");
        case ID_DBP_I_VMG:
            return _("VMG");
        case ID_DBP_D_VMG:
            return _("VMG");
        case ID_DBP_I_RSA:
            return _("Rudder Angle");
        case ID_DBP_D_RSA:
            return _("Rudder Angle");
        case ID_DBP_I_SAT:
            return _("GPS in View");
        case ID_DBP_D_GPS:
            return _("GPS Status");
        case ID_DBP_I_PTR:
            return _("Cursor");
        case ID_DBP_I_CLK:
            return _("Clock");
        case ID_DBP_I_SUN:
            return _("Sunrise/Sunset");
        case ID_DBP_D_MON:
            return _("Moon phase");
        case ID_DBP_D_WDH:
            return _("Wind history");
        case ID_DBP_D_BPH:
            return  _("Barometric history");
        case ID_DBP_I_VLW1:
            return _("Trip Log");
        case ID_DBP_I_VLW2:
            return _("Sum Log");
        case ID_DBP_I_FOS:
            return _("From Ownship");
		case ID_DBP_I_PITCH:
			return _("Pitch");
		case ID_DBP_I_HEEL:
			return _("Heel");
    }
    return _T("");
}

void getListItemForInstrument( wxListItem &item, unsigned int id )
{
    item.SetData( id );
    item.SetText( getInstrumentCaption( id ) );
    switch( id ){
        case ID_DBP_I_POS:
        case ID_DBP_I_SOG:
        case ID_DBP_I_COG:
        case ID_DBP_M_COG:
        case ID_DBP_I_STW:
        case ID_DBP_I_HDT:
        case ID_DBP_I_HDM:
        case ID_DBP_I_AWS:
        case ID_DBP_I_DPT:
    	case ID_DBP_I_MDA:
        case ID_DBP_I_TMP:
        case ID_DBP_I_ATMP:
        case ID_DBP_I_TWA:
        case ID_DBP_I_TWD:
        case ID_DBP_I_TWS:
        case ID_DBP_I_AWA:
        case ID_DBP_I_VMG:
        case ID_DBP_I_RSA:
        case ID_DBP_I_SAT:
        case ID_DBP_I_PTR:
        case ID_DBP_I_CLK:
        case ID_DBP_I_SUN:
        case ID_DBP_I_VLW1:
        case ID_DBP_I_VLW2:
        case ID_DBP_I_FOS:
		case ID_DBP_I_PITCH:
		case ID_DBP_I_HEEL:
            item.SetImage( 0 );
            break;
        case ID_DBP_D_SOG:
        case ID_DBP_D_COG:
        case ID_DBP_D_AW:
        case ID_DBP_D_AWA:
        case ID_DBP_D_AWS:
        case ID_DBP_D_TW:
		case ID_DBP_D_AWA_TWA:
        case ID_DBP_D_TWD:
        case ID_DBP_D_DPT:
    	case ID_DBP_D_MDA:
        case ID_DBP_D_VMG:
        case ID_DBP_D_RSA:
        case ID_DBP_D_GPS:
        case ID_DBP_D_HDT:
        case ID_DBP_D_MON:
        case ID_DBP_D_WDH:
        case ID_DBP_D_BPH:
            item.SetImage( 1 );
            break;
    }
}

/*  These two function were taken from gpxdocument.cpp */
int GetRandomNumber(int range_min, int range_max)
{
      long u = (long)wxRound(((double)rand() / ((double)(RAND_MAX) + 1) * (range_max - range_min)) + range_min);
      return (int)u;
}

// RFC4122 version 4 compliant random UUIDs generator.
wxString GetUUID(void)
{
      wxString str;
      struct {
      int time_low;
      int time_mid;
      int time_hi_and_version;
      int clock_seq_hi_and_rsv;
      int clock_seq_low;
      int node_hi;
      int node_low;
      } uuid;

      uuid.time_low = GetRandomNumber(0, 2147483647);//FIXME: the max should be set to something like MAXINT32, but it doesn't compile un gcc...
      uuid.time_mid = GetRandomNumber(0, 65535);
      uuid.time_hi_and_version = GetRandomNumber(0, 65535);
      uuid.clock_seq_hi_and_rsv = GetRandomNumber(0, 255);
      uuid.clock_seq_low = GetRandomNumber(0, 255);
      uuid.node_hi = GetRandomNumber(0, 65535);
      uuid.node_low = GetRandomNumber(0, 2147483647);

      /* Set the two most significant bits (bits 6 and 7) of the
      * clock_seq_hi_and_rsv to zero and one, respectively. */
      uuid.clock_seq_hi_and_rsv = (uuid.clock_seq_hi_and_rsv & 0x3F) | 0x80;

      /* Set the four most significant bits (bits 12 through 15) of the
      * time_hi_and_version field to 4 */
      uuid.time_hi_and_version = (uuid.time_hi_and_version & 0x0fff) | 0x4000;

      str.Printf(_T("%08x-%04x-%04x-%02x%02x-%04x%08x"),
      uuid.time_low,
      uuid.time_mid,
      uuid.time_hi_and_version,
      uuid.clock_seq_hi_and_rsv,
      uuid.clock_seq_low,
      uuid.node_hi,
      uuid.node_low);

      return str;
}

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

dashboard_pi::dashboard_pi( void *ppimgr ) :
        wxTimer( this ), opencpn_plugin_16( ppimgr )
{
    // Create the PlugIn icons
    initialize_images();
}

dashboard_pi::~dashboard_pi( void )
{
    delete _img_dashboard_pi;
    delete _img_dashboard;
    delete _img_dial;
    delete _img_instrument;
    delete _img_minus;
    delete _img_plus;
}

int dashboard_pi::Init( void )
{
    AddLocaleCatalog( _T("opencpn-dashboard_pi") );

    mVar = NAN;
    mPriPosition = 99;
    mPriCOGSOG = 99;
    mPriHeadingT = 99; // True heading
    mPriHeadingM = 99; // Magnetic heading
    mPriVar = 99;
    mPriDateTime = 99;
    mPriAWA = 99; // Relative wind
    mPriTWA = 99; // True wind
    mPriDepth = 99;
    m_config_version = -1;
    mHDx_Watchdog = 2;
    mHDT_Watchdog = 2;
    mGPS_Watchdog = 2;
    mVar_Watchdog = 2;

    g_pFontTitle = new wxFont( 10, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL );
    g_pFontData = new wxFont( 14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    g_pFontLabel = new wxFont( 8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    g_pFontSmall = new wxFont( 8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );

    m_pauimgr = GetFrameAuiManager();
    m_pauimgr->Connect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( dashboard_pi::OnPaneClose ),
            NULL, this );

    //    Get a pointer to the opencpn configuration object
    m_pconfig = GetOCPNConfigObject();

    //    And load the configuration items
    LoadConfig();

    //    This PlugIn needs a toolbar icon
//    m_toolbar_item_id = InsertPlugInTool( _T(""), _img_dashboard, _img_dashboard, wxITEM_CHECK,
//            _("Dashboard"), _T(""), NULL, DASHBOARD_TOOL_POSITION, 0, this );
    
    wxString shareLocn =*GetpSharedDataLocation() +
                _T("plugins") + wxFileName::GetPathSeparator() +
                _T("dashboard_pi") + wxFileName::GetPathSeparator()
                +_T("data") + wxFileName::GetPathSeparator();
    
     wxString normalIcon = shareLocn + _T("Dashboard.svg");
     wxString toggledIcon = shareLocn + _T("Dashboard_toggled.svg");
     wxString rolloverIcon = shareLocn + _T("Dashboard_rollover.svg");
     
     //  For journeyman styles, we prefer the built-in raster icons which match the rest of the toolbar.
     if(GetActiveStyleName().Lower() != _T("traditional")){
         normalIcon = _T("");
         toggledIcon = _T("");
         rolloverIcon = _T("");
     }
         
      m_toolbar_item_id = InsertPlugInToolSVG( _T(""), normalIcon, rolloverIcon, toggledIcon, wxITEM_CHECK,
             _("Dashboard"), _T(""), NULL, DASHBOARD_TOOL_POSITION, 0, this );
    
    
    ApplyConfig();

    //  If we loaded a version 1 config setup, convert now to version 2
    if(m_config_version == 1) {
        SaveConfig();
    }

    Start( 1000, wxTIMER_CONTINUOUS );

    return ( WANTS_CURSOR_LATLON | WANTS_TOOLBAR_CALLBACK | INSTALLS_TOOLBAR_TOOL
            | WANTS_PREFERENCES | WANTS_CONFIG | WANTS_NMEA_SENTENCES | WANTS_NMEA_EVENTS
            | USES_AUI_MANAGER | WANTS_PLUGIN_MESSAGING );
}

bool dashboard_pi::DeInit( void )
{
    SaveConfig();
    if( IsRunning() ) // Timer started?
    Stop(); // Stop timer

    for( size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
        DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item( i )->m_pDashboardWindow;
        if( dashboard_window ) {
            m_pauimgr->DetachPane( dashboard_window );
            dashboard_window->Close();
            dashboard_window->Destroy();
            m_ArrayOfDashboardWindow.Item( i )->m_pDashboardWindow = NULL;
        }
    }

    for( size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
        DashboardWindowContainer *pdwc = m_ArrayOfDashboardWindow.Item( i );
        delete pdwc;
    }

    delete g_pFontTitle;
    delete g_pFontData;
    delete g_pFontLabel;
    delete g_pFontSmall;

    return true;
}

void dashboard_pi::Notify()
{
    SendUtcTimeToAllInstruments( mUTCDateTime );
    for( size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
        DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item( i )->m_pDashboardWindow;
        if( dashboard_window ) dashboard_window->Refresh();
    }
    //  Manage the watchdogs
    mHDx_Watchdog--;
    if( mHDx_Watchdog <= 0 ) {
        mHdm = NAN;
        SendSentenceToAllInstruments( OCPN_DBP_STC_HDM, mHdm, _T("\u00B0") );
    }

    mHDT_Watchdog--;
    if( mHDT_Watchdog <= 0 ) {
        SendSentenceToAllInstruments( OCPN_DBP_STC_HDT, NAN, _T("\u00B0T") );
    }

    mVar_Watchdog--;
    if( mVar_Watchdog <= 0 ) {
        mVar = NAN;
        mPriVar = 99;
        SendSentenceToAllInstruments( OCPN_DBP_STC_HMV, NAN, _T("\u00B0T") );
    }

    mGPS_Watchdog--;
    if( mGPS_Watchdog <= 0 ) {
        SAT_INFO sats[4];
        for(int i=0 ; i < 4 ; i++) {
            sats[i].SatNumber = 0;
            sats[i].SignalToNoiseRatio = 0;
        }
        SendSatInfoToAllInstruments( 0, 1, sats );
        SendSatInfoToAllInstruments( 0, 2, sats );
        SendSatInfoToAllInstruments( 0, 3, sats );

        mSatsInView = 0;
        SendSentenceToAllInstruments( OCPN_DBP_STC_SAT, 0, _T("") );
    }
}

int dashboard_pi::GetAPIVersionMajor()
{
    return MY_API_VERSION_MAJOR;
}

int dashboard_pi::GetAPIVersionMinor()
{
    return MY_API_VERSION_MINOR;
}

int dashboard_pi::GetPlugInVersionMajor()
{
    return PLUGIN_VERSION_MAJOR;
}

int dashboard_pi::GetPlugInVersionMinor()
{
    return PLUGIN_VERSION_MINOR;
}

wxBitmap *dashboard_pi::GetPlugInBitmap()
{
    return _img_dashboard_pi;
}

wxString dashboard_pi::GetCommonName()
{
    return _("Dashboard");
}

wxString dashboard_pi::GetShortDescription()
{
    return _("Dashboard PlugIn for OpenCPN");
}

wxString dashboard_pi::GetLongDescription()
{
    return _("Dashboard PlugIn for OpenCPN\n\
Provides navigation instrument display from NMEA source.");

}

void dashboard_pi::SendSentenceToAllInstruments( int st, double value, wxString unit )
{
    for( size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
        DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item( i )->m_pDashboardWindow;
        if( dashboard_window ) dashboard_window->SendSentenceToAllInstruments( st, value, unit );
    }
}

void dashboard_pi::SendUtcTimeToAllInstruments( wxDateTime value )
{
    for( size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
        DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item( i )->m_pDashboardWindow;
        if( dashboard_window ) dashboard_window->SendUtcTimeToAllInstruments( value );
    }
}

void dashboard_pi::SendSatInfoToAllInstruments( int cnt, int seq, SAT_INFO sats[4] )
{
    for( size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
        DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item( i )->m_pDashboardWindow;
        if( dashboard_window ) dashboard_window->SendSatInfoToAllInstruments( cnt, seq, sats );
    }
}

void dashboard_pi::SetNMEASentence( wxString &sentence )
{
    m_NMEA0183 << sentence;

    if( m_NMEA0183.PreParse() ) {
        if( m_NMEA0183.LastSentenceIDReceived == _T("DBT") ) {
            if( m_NMEA0183.Parse() ) {
                if( mPriDepth >= 2 ) {
                    mPriDepth = 2;

                    /*
                     double m_NMEA0183.Dbt.DepthFeet;
                     double m_NMEA0183.Dbt.DepthMeters;
                     double m_NMEA0183.Dbt.DepthFathoms;
                     */
                    double depth = 999.;
                    if( m_NMEA0183.Dbt.DepthMeters != 999. ) depth = m_NMEA0183.Dbt.DepthMeters;
                    else if( m_NMEA0183.Dbt.DepthFeet != 999. ) depth = m_NMEA0183.Dbt.DepthFeet
                            * 0.3048;
                    else if( m_NMEA0183.Dbt.DepthFathoms != 999. ) depth =
                            m_NMEA0183.Dbt.DepthFathoms * 1.82880;
                    SendSentenceToAllInstruments( OCPN_DBP_STC_DPT, toUsrDistance_Plugin( depth / 1852.0, g_iDashDepthUnit ), getUsrDistanceUnit_Plugin( g_iDashDepthUnit ) );
                }
            }
        }

        else if( m_NMEA0183.LastSentenceIDReceived == _T("DPT") ) {
            if( m_NMEA0183.Parse() ) {
                if( mPriDepth >= 1 ) {
                    mPriDepth = 1;

                    /*
                     double m_NMEA0183.Dpt.DepthMeters
                     double m_NMEA0183.Dpt.OffsetFromTransducerMeters
                     */
                    double depth = 999.;
                    if( m_NMEA0183.Dpt.DepthMeters != 999. ) depth = m_NMEA0183.Dpt.DepthMeters;
                    if( m_NMEA0183.Dpt.OffsetFromTransducerMeters != 999. ) depth += m_NMEA0183.Dpt.OffsetFromTransducerMeters;
                    SendSentenceToAllInstruments( OCPN_DBP_STC_DPT, toUsrDistance_Plugin( depth / 1852.0, g_iDashDepthUnit ), getUsrDistanceUnit_Plugin( g_iDashDepthUnit ) );
                }
            }
        }
// TODO: GBS - GPS Satellite fault detection
        else if( m_NMEA0183.LastSentenceIDReceived == _T("GGA") ) {
            if( m_NMEA0183.Parse() ) {
                if( m_NMEA0183.Gga.GPSQuality > 0 ) {
                    if( mPriPosition >= 3 ) {
                        mPriPosition = 3;
                        double lat, lon;
                        float llt = m_NMEA0183.Gga.Position.Latitude.Latitude;
                        int lat_deg_int = (int) ( llt / 100 );
                        float lat_deg = lat_deg_int;
                        float lat_min = llt - ( lat_deg * 100 );
                        lat = lat_deg + ( lat_min / 60. );
                        if( m_NMEA0183.Gga.Position.Latitude.Northing == South ) lat = -lat;
                        SendSentenceToAllInstruments( OCPN_DBP_STC_LAT, lat, _T("SDMM") );

                        float lln = m_NMEA0183.Gga.Position.Longitude.Longitude;
                        int lon_deg_int = (int) ( lln / 100 );
                        float lon_deg = lon_deg_int;
                        float lon_min = lln - ( lon_deg * 100 );
                        lon = lon_deg + ( lon_min / 60. );
                        if( m_NMEA0183.Gga.Position.Longitude.Easting == West ) lon = -lon;
                        SendSentenceToAllInstruments( OCPN_DBP_STC_LON, lon, _T("SDMM") );
                    }

                    if( mPriDateTime >= 4 ) {
                        // Not in use, we need the date too.
                        //mPriDateTime = 4;
                        //mUTCDateTime.ParseFormat( m_NMEA0183.Gga.UTCTime.c_str(), _T("%H%M%S") );
                    }

                    mSatsInView = m_NMEA0183.Gga.NumberOfSatellitesInUse;
                }
            }
        }

        else if( m_NMEA0183.LastSentenceIDReceived == _T("GLL") ) {
            if( m_NMEA0183.Parse() ) {
                if( m_NMEA0183.Gll.IsDataValid == NTrue ) {
                    if( mPriPosition >= 2 ) {
                        mPriPosition = 2;
                        double lat, lon;
                        float llt = m_NMEA0183.Gll.Position.Latitude.Latitude;
                        int lat_deg_int = (int) ( llt / 100 );
                        float lat_deg = lat_deg_int;
                        float lat_min = llt - ( lat_deg * 100 );
                        lat = lat_deg + ( lat_min / 60. );
                        if( m_NMEA0183.Gll.Position.Latitude.Northing == South ) lat = -lat;
                        SendSentenceToAllInstruments( OCPN_DBP_STC_LAT, lat, _T("SDMM") );

                        float lln = m_NMEA0183.Gll.Position.Longitude.Longitude;
                        int lon_deg_int = (int) ( lln / 100 );
                        float lon_deg = lon_deg_int;
                        float lon_min = lln - ( lon_deg * 100 );
                        lon = lon_deg + ( lon_min / 60. );
                        if( m_NMEA0183.Gll.Position.Longitude.Easting == West ) lon = -lon;
                        SendSentenceToAllInstruments( OCPN_DBP_STC_LON, lon, _T("SDMM") );
                    }

                    if( mPriDateTime >= 5 ) {
                        // Not in use, we need the date too.
                        //mPriDateTime = 5;
                        //mUTCDateTime.ParseFormat( m_NMEA0183.Gll.UTCTime.c_str(), _T("%H%M%S") );
                    }
                }
            }
        }

        else if( m_NMEA0183.LastSentenceIDReceived == _T("GSV") ) {
            if( m_NMEA0183.Parse() ) {
                mSatsInView = m_NMEA0183.Gsv.SatsInView;
                // m_NMEA0183.Gsv.NumberOfMessages;
                SendSentenceToAllInstruments( OCPN_DBP_STC_SAT, m_NMEA0183.Gsv.SatsInView, _T("") );
                SendSatInfoToAllInstruments( m_NMEA0183.Gsv.SatsInView,
                        m_NMEA0183.Gsv.MessageNumber, m_NMEA0183.Gsv.SatInfo );

                mGPS_Watchdog = gps_watchdog_timeout_ticks;
            }
        }

        else if( m_NMEA0183.LastSentenceIDReceived == _T("HDG") ) {
            if( m_NMEA0183.Parse() ) {
                if( mPriVar >= 2 ) {
                    if( !wxIsNaN( m_NMEA0183.Hdg.MagneticVariationDegrees ) ){
                        mPriVar = 2;
                        if( m_NMEA0183.Hdg.MagneticVariationDirection == East )
                            mVar =  m_NMEA0183.Hdg.MagneticVariationDegrees;
                        else if( m_NMEA0183.Hdg.MagneticVariationDirection == West )
                            mVar = -m_NMEA0183.Hdg.MagneticVariationDegrees;
                        SendSentenceToAllInstruments( OCPN_DBP_STC_HMV, mVar, _T("\u00B0") );
                    }

                }
                if( mPriHeadingM >= 1 ) {
                    mPriHeadingM = 1;
                    mHdm = m_NMEA0183.Hdg.MagneticSensorHeadingDegrees;
                    SendSentenceToAllInstruments( OCPN_DBP_STC_HDM, mHdm, _T("\u00B0") );
                }
                if( !wxIsNaN(m_NMEA0183.Hdg.MagneticSensorHeadingDegrees) )
                       mHDx_Watchdog = gps_watchdog_timeout_ticks;

                //      If Variation is available, no higher priority HDT is available,
                //      then calculate and propagate calculated HDT
                if( !wxIsNaN(m_NMEA0183.Hdg.MagneticSensorHeadingDegrees) ) {
                    if( !wxIsNaN( mVar )  && (mPriHeadingT > 3) ){
                        mPriHeadingT = 4;
                        SendSentenceToAllInstruments(OCPN_DBP_STC_HDT, mHdm + mVar, _T("\u00B0"));
                        mHDT_Watchdog = gps_watchdog_timeout_ticks;
                    }
                }
            }
        }

        else if( m_NMEA0183.LastSentenceIDReceived == _T("HDM") ) {
            if( m_NMEA0183.Parse() ) {
                if( mPriHeadingM >= 2 ) {
                    mPriHeadingM = 2;
                    mHdm = m_NMEA0183.Hdm.DegreesMagnetic;
                    SendSentenceToAllInstruments( OCPN_DBP_STC_HDM, mHdm, _T("\u00B0M") );
                }
                if( !wxIsNaN(m_NMEA0183.Hdm.DegreesMagnetic) )
                    mHDx_Watchdog = gps_watchdog_timeout_ticks;

                //      If Variation is available, no higher priority HDT is available,
                //      then calculate and propagate calculated HDT
                if( !wxIsNaN(m_NMEA0183.Hdm.DegreesMagnetic) ) {
                    if( !wxIsNaN( mVar )  && (mPriHeadingT > 2) ){
                        mPriHeadingT = 3;
                        SendSentenceToAllInstruments(OCPN_DBP_STC_HDT, mHdm + mVar, _T("\u00B0"));
                        mHDT_Watchdog = gps_watchdog_timeout_ticks;
                    }
                }

            }
        }

        else if( m_NMEA0183.LastSentenceIDReceived == _T("HDT") ) {
            if( m_NMEA0183.Parse() ) {
                if( mPriHeadingT >= 1 ) {
                    mPriHeadingT = 1;
                    if( m_NMEA0183.Hdt.DegreesTrue < 999. ) {
                        SendSentenceToAllInstruments( OCPN_DBP_STC_HDT, m_NMEA0183.Hdt.DegreesTrue,
                                _T("\u00B0T") );
                    }
                }
                if( !wxIsNaN(m_NMEA0183.Hdt.DegreesTrue) )
                    mHDT_Watchdog = gps_watchdog_timeout_ticks;

            }
        } else if( m_NMEA0183.LastSentenceIDReceived == _T("MTA") ) {  //Air temperature
            if( m_NMEA0183.Parse() ) {
                /*
                 double   m_NMEA0183.Mta.Temperature;
                 wxString m_NMEA0183.Mta.UnitOfMeasurement;
                 */
                SendSentenceToAllInstruments( OCPN_DBP_STC_ATMP, m_NMEA0183.Mta.Temperature,
                        m_NMEA0183.Mta.UnitOfMeasurement );
            }
        } else if( m_NMEA0183.LastSentenceIDReceived == _T("MDA") ) {  //Barometric pressure
            if( m_NMEA0183.Parse() ) {
                // TODO make posibilyti to select between Bar or InchHg
                /*

                 double   m_NMEA0183.Mda.Pressure;

                 wxString m_NMEA0183.Mda.UnitOfMeasurement;

                 */

                if( m_NMEA0183.Mda.Pressure > .8 && m_NMEA0183.Mda.Pressure < 1.1 ) {
                    SendSentenceToAllInstruments( OCPN_DBP_STC_MDA, m_NMEA0183.Mda.Pressure *1000,
                           _T("hPa") ); //Convert to hpa befor sending to instruments.
                }

            }

        }
        else if( m_NMEA0183.LastSentenceIDReceived == _T("MTW") ) {
            if( m_NMEA0183.Parse() ) {
                /*
                 double   m_NMEA0183.Mtw.Temperature;
                 wxString m_NMEA0183.Mtw.UnitOfMeasurement;
                 */
                SendSentenceToAllInstruments( OCPN_DBP_STC_TMP, m_NMEA0183.Mtw.Temperature,
                        m_NMEA0183.Mtw.UnitOfMeasurement );
            }

        }
        else if( m_NMEA0183.LastSentenceIDReceived == _T("VLW") ) {
            if( m_NMEA0183.Parse() ) {
                /*
                 double   m_NMEA0183.Vlw.TotalMileage;
                 double   m_NMEA0183.Vlw.TripMileage;
                                  */
                SendSentenceToAllInstruments( OCPN_DBP_STC_VLW1, toUsrDistance_Plugin( m_NMEA0183.Vlw.TripMileage, g_iDashDistanceUnit ),
                        getUsrDistanceUnit_Plugin( g_iDashDistanceUnit ) );

                SendSentenceToAllInstruments( OCPN_DBP_STC_VLW2, toUsrDistance_Plugin( m_NMEA0183.Vlw.TotalMileage, g_iDashDistanceUnit ),
                        getUsrDistanceUnit_Plugin( g_iDashDistanceUnit ) );
            }

        }
        // NMEA 0183 standard Wind Direction and Speed, with respect to north.
        else if( m_NMEA0183.LastSentenceIDReceived == _T("MWD") ) {
            if( m_NMEA0183.Parse() ) {
                // Option for True vs Magnetic
                wxString windunit;
                if( m_NMEA0183.Mwd.WindAngleTrue < 999. ) { //if WindAngleTrue is available, use it ...
                    SendSentenceToAllInstruments( OCPN_DBP_STC_TWD, m_NMEA0183.Mwd.WindAngleTrue,
                            _T("\u00B0T") );
                } else if( m_NMEA0183.Mwd.WindAngleMagnetic < 999. ) { //otherwise try WindAngleMagnetic ...
                    SendSentenceToAllInstruments( OCPN_DBP_STC_TWD, m_NMEA0183.Mwd.WindAngleMagnetic,
                            _T("\u00B0M") );
                }

                SendSentenceToAllInstruments( OCPN_DBP_STC_TWS, toUsrSpeed_Plugin( m_NMEA0183.Mwd.WindSpeedKnots, g_iDashWindSpeedUnit ),
                                              getUsrSpeedUnit_Plugin( g_iDashWindSpeedUnit ) );
                SendSentenceToAllInstruments( OCPN_DBP_STC_TWS2, toUsrSpeed_Plugin( m_NMEA0183.Mwd.WindSpeedKnots, g_iDashWindSpeedUnit ),
                        getUsrSpeedUnit_Plugin( g_iDashWindSpeedUnit ) );
                //m_NMEA0183.Mwd.WindSpeedms
            }
        }
        // NMEA 0183 standard Wind Speed and Angle, in relation to the vessel's bow/centerline.
        else if( m_NMEA0183.LastSentenceIDReceived == _T("MWV") ) {
            if( m_NMEA0183.Parse() ) {
                if( m_NMEA0183.Mwv.IsDataValid == NTrue ) {
                    //MWV windspeed has different units. Form it to knots to fit "toUsrSpeed_Plugin()"
                    double m_wSpeedFactor = 1.0; //knots ("N")
                    if (m_NMEA0183.Mwv.WindSpeedUnits == _T("K") ) m_wSpeedFactor = 0.53995 ; //km/h > knots
                    if (m_NMEA0183.Mwv.WindSpeedUnits == _T("M") ) m_wSpeedFactor = 1.94384 ; //m/s > knots

                    if( m_NMEA0183.Mwv.Reference == _T("R") ) // Relative (apparent wind)
                    {
                        if( mPriAWA >= 1 ) {
                            mPriAWA = 1;
							wxString m_awaunit;
							double m_awaangle;
							if (m_NMEA0183.Mwv.WindAngle >180) {
								m_awaunit = _T("\u00B0L");
								m_awaangle = 180.0 - (m_NMEA0183.Mwv.WindAngle - 180.0);
							}
							else {
								m_awaunit = _T("\u00B0R");
								m_awaangle = m_NMEA0183.Mwv.WindAngle;
							}
                            SendSentenceToAllInstruments( OCPN_DBP_STC_AWA,
								m_awaangle, m_awaunit);
                            SendSentenceToAllInstruments( OCPN_DBP_STC_AWS,
                                    toUsrSpeed_Plugin( m_NMEA0183.Mwv.WindSpeed * m_wSpeedFactor, g_iDashWindSpeedUnit ),
                                    getUsrSpeedUnit_Plugin( g_iDashWindSpeedUnit ) );
                        }
                    } else if( m_NMEA0183.Mwv.Reference == _T("T") ) // Theoretical (aka True)
                    {
                        if( mPriTWA >= 1 ) {
                            mPriTWA = 1;
							wxString m_twaunit;
							double m_twaangle;
							if (m_NMEA0183.Mwv.WindAngle >180) {
								m_twaunit = _T("\u00B0L");
								m_twaangle = 180.0 - (m_NMEA0183.Mwv.WindAngle - 180.0);
							}
							else {
								m_twaunit = _T("\u00B0R");
								m_twaangle = m_NMEA0183.Mwv.WindAngle;
							}
                            SendSentenceToAllInstruments( OCPN_DBP_STC_TWA,
								m_twaangle, m_twaunit);
                            SendSentenceToAllInstruments( OCPN_DBP_STC_TWS,
                                    toUsrSpeed_Plugin( m_NMEA0183.Mwv.WindSpeed * m_wSpeedFactor, g_iDashWindSpeedUnit ),
                                    getUsrSpeedUnit_Plugin( g_iDashWindSpeedUnit ) );
                            SendSentenceToAllInstruments( OCPN_DBP_STC_TWS2,
                                    toUsrSpeed_Plugin( m_NMEA0183.Mwv.WindSpeed * m_wSpeedFactor, g_iDashWindSpeedUnit ),
                                    getUsrSpeedUnit_Plugin( g_iDashWindSpeedUnit ) );
                        }
                    }
                }
            }
        }

        else if( m_NMEA0183.LastSentenceIDReceived == _T("RMC") ) {
            if( m_NMEA0183.Parse() ) {
                if( m_NMEA0183.Rmc.IsDataValid == NTrue ) {
                    if( mPriPosition >= 4 ) {
                        mPriPosition = 4;
                        double lat, lon;
                        float llt = m_NMEA0183.Rmc.Position.Latitude.Latitude;
                        int lat_deg_int = (int) ( llt / 100 );
                        float lat_deg = lat_deg_int;
                        float lat_min = llt - ( lat_deg * 100 );
                        lat = lat_deg + ( lat_min / 60. );
                        if( m_NMEA0183.Rmc.Position.Latitude.Northing == South ) lat = -lat;
                        SendSentenceToAllInstruments( OCPN_DBP_STC_LAT, lat, _T("SDMM") );

                        float lln = m_NMEA0183.Rmc.Position.Longitude.Longitude;
                        int lon_deg_int = (int) ( lln / 100 );
                        float lon_deg = lon_deg_int;
                        float lon_min = lln - ( lon_deg * 100 );
                        lon = lon_deg + ( lon_min / 60. );
                        if( m_NMEA0183.Rmc.Position.Longitude.Easting == West ) lon = -lon;
                        SendSentenceToAllInstruments( OCPN_DBP_STC_LON, lon, _T("SDMM") );
                    }

                    if( mPriCOGSOG >= 3 ) {
                        mPriCOGSOG = 3;
                        if( m_NMEA0183.Rmc.SpeedOverGroundKnots < 999. ) {
                            SendSentenceToAllInstruments( OCPN_DBP_STC_SOG,
                                    toUsrSpeed_Plugin( m_NMEA0183.Rmc.SpeedOverGroundKnots, g_iDashSpeedUnit ), getUsrSpeedUnit_Plugin( g_iDashSpeedUnit ) );
                        } else {
                            //->SetData(_T("---"));
                        }
                        if( m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue < 999. ) {
                            SendSentenceToAllInstruments( OCPN_DBP_STC_COG,
                                    m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue, _T("\u00B0") );
                        } else {
                            //->SetData(_T("---"));
                        }
                        if( m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue < 999. && m_NMEA0183.Rmc.MagneticVariation < 999.) {
                            double dMagneticCOG;
                            if (m_NMEA0183.Rmc.MagneticVariationDirection == East) {
                                dMagneticCOG = m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue - m_NMEA0183.Rmc.MagneticVariation;
                                if ( dMagneticCOG < 0.0 ) dMagneticCOG = 360.0 + dMagneticCOG;
                            }
                            else {
                                dMagneticCOG = m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue + m_NMEA0183.Rmc.MagneticVariation;
                                if ( dMagneticCOG > 360.0 ) dMagneticCOG = dMagneticCOG - 360.0;
                            }
                            SendSentenceToAllInstruments( OCPN_DBP_STC_MCOG,
                                    dMagneticCOG, _T("\u00B0M") );
                        } else {
                            //->SetData(_T("---"));
                        }
                    }

                    if( mPriVar >= 3 ) {
                        if( !wxIsNaN( m_NMEA0183.Rmc.MagneticVariation ) ){
                            mPriVar = 3;
                            if( m_NMEA0183.Rmc.MagneticVariationDirection == East )
                                mVar = m_NMEA0183.Rmc.MagneticVariation;
                            else if( m_NMEA0183.Rmc.MagneticVariationDirection == West )
                                mVar = -m_NMEA0183.Rmc.MagneticVariation;
                            mVar_Watchdog = gps_watchdog_timeout_ticks;

                            SendSentenceToAllInstruments( OCPN_DBP_STC_HMV, mVar, _T("\u00B0") );
                        }
                    }

                    if( mPriDateTime >= 3 ) {
                        mPriDateTime = 3;
                        wxString dt = m_NMEA0183.Rmc.Date + m_NMEA0183.Rmc.UTCTime;
                        mUTCDateTime.ParseFormat( dt.c_str(), _T("%d%m%y%H%M%S") );
                    }
                }
            }
        }

        else if( m_NMEA0183.LastSentenceIDReceived == _T("RSA") ) {
            if( m_NMEA0183.Parse() ) {
                if( m_NMEA0183.Rsa.IsStarboardDataValid == NTrue ) {
                    SendSentenceToAllInstruments( OCPN_DBP_STC_RSA, m_NMEA0183.Rsa.Starboard,
                            _T("\u00B0") );
                } else if( m_NMEA0183.Rsa.IsPortDataValid == NTrue ) {
                    SendSentenceToAllInstruments( OCPN_DBP_STC_RSA, -m_NMEA0183.Rsa.Port,
                            _T("\u00B0") );
                }
            }
        }

        else if( m_NMEA0183.LastSentenceIDReceived == _T("VHW") ) {
            if( m_NMEA0183.Parse() ) {
                if( mPriHeadingT >= 2 ) {
                    if( m_NMEA0183.Vhw.DegreesTrue < 999. ) {
                        mPriHeadingT = 2;
                        SendSentenceToAllInstruments( OCPN_DBP_STC_HDT, m_NMEA0183.Vhw.DegreesTrue,
                                _T("\u00B0T") );
                    }
                }
                if( mPriHeadingM >= 3 ) {
                    mPriHeadingM = 3;
                    SendSentenceToAllInstruments( OCPN_DBP_STC_HDM, m_NMEA0183.Vhw.DegreesMagnetic,
                            _T("\u00B0M") );
                }
                if( m_NMEA0183.Vhw.Knots < 999. ) {
                    SendSentenceToAllInstruments( OCPN_DBP_STC_STW, toUsrSpeed_Plugin( m_NMEA0183.Vhw.Knots, g_iDashSpeedUnit ),
                            getUsrSpeedUnit_Plugin( g_iDashSpeedUnit ) );
                }

                if( !wxIsNaN(m_NMEA0183.Vhw.DegreesMagnetic) )
                    mHDx_Watchdog = gps_watchdog_timeout_ticks;
                if( !wxIsNaN(m_NMEA0183.Vhw.DegreesTrue) )
                    mHDT_Watchdog = gps_watchdog_timeout_ticks;

            }
        }

        else if( m_NMEA0183.LastSentenceIDReceived == _T("VTG") ) {
            if( m_NMEA0183.Parse() ) {
                if( mPriCOGSOG >= 2 ) {
                    mPriCOGSOG = 2;
                    //    Special check for unintialized values, as opposed to zero values
                    if( m_NMEA0183.Vtg.SpeedKnots < 999. ) {
                        SendSentenceToAllInstruments( OCPN_DBP_STC_SOG, toUsrSpeed_Plugin( m_NMEA0183.Vtg.SpeedKnots, g_iDashSpeedUnit ),
                                getUsrSpeedUnit_Plugin( g_iDashSpeedUnit ) );
                    } else {
                        //->SetData(_T("---"));
                    }
                    // Vtg.SpeedKilometersPerHour;
                    if( m_NMEA0183.Vtg.TrackDegreesTrue < 999. ) {
                        SendSentenceToAllInstruments( OCPN_DBP_STC_COG,
                                m_NMEA0183.Vtg.TrackDegreesTrue, _T("\u00B0") );
                    } else {
                        //->SetData(_T("---"));
                    }
                }

                /*
                 m_NMEA0183.Vtg.TrackDegreesMagnetic;
                 */
            }
        }
        /* NMEA 0183 Relative (Apparent) Wind Speed and Angle. Wind angle in relation
         * to the vessel's heading, and wind speed measured relative to the moving vessel. */
        else if( m_NMEA0183.LastSentenceIDReceived == _T("VWR") ) {
            if( m_NMEA0183.Parse() ) {
                if( mPriAWA >= 2 ) {
                    mPriAWA = 2;

                    wxString awaunit;
                    awaunit = m_NMEA0183.Vwr.DirectionOfWind == Left ? _T("\u00B0L") : _T("\u00B0R");
                    SendSentenceToAllInstruments( OCPN_DBP_STC_AWA,
                            m_NMEA0183.Vwr.WindDirectionMagnitude, awaunit );
                    SendSentenceToAllInstruments( OCPN_DBP_STC_AWS, toUsrSpeed_Plugin( m_NMEA0183.Vwr.WindSpeedKnots, g_iDashWindSpeedUnit ),
                            getUsrSpeedUnit_Plugin( g_iDashWindSpeedUnit ) );
                    /*
                     double m_NMEA0183.Vwr.WindSpeedms;
                     double m_NMEA0183.Vwr.WindSpeedKmh;
                     */
                }
            }
        }
        /* NMEA 0183 True wind angle in relation to the vessel's heading, and true wind
         * speed referenced to the water. True wind is the vector sum of the Relative
         * (apparent) wind vector and the vessel's velocity vector relative to the water along
         * the heading line of the vessel. It represents the wind at the vessel if it were
         * stationary relative to the water and heading in the same direction. */
        else if( m_NMEA0183.LastSentenceIDReceived == _T("VWT") ) {
            if( m_NMEA0183.Parse() ) {
                if( mPriTWA >= 2 ) {
                    mPriTWA = 2;
                    wxString vwtunit;
                    vwtunit = m_NMEA0183.Vwt.DirectionOfWind == Left ? _T("\u00B0L") : _T("\u00B0R");
                    SendSentenceToAllInstruments( OCPN_DBP_STC_TWA,
                            m_NMEA0183.Vwt.WindDirectionMagnitude, vwtunit );
                    SendSentenceToAllInstruments( OCPN_DBP_STC_TWS, toUsrSpeed_Plugin( m_NMEA0183.Vwt.WindSpeedKnots, g_iDashWindSpeedUnit ),
                            getUsrSpeedUnit_Plugin( g_iDashWindSpeedUnit ) );
                    /*
                     double           m_NMEA0183.Vwt.WindSpeedms;
                     double           m_NMEA0183.Vwt.WindSpeedKmh;
                     */
                }
            }
        }

        else if (m_NMEA0183.LastSentenceIDReceived == _T("XDR")) { //Transducer measurement
             /* XDR Transducer types
              * AngularDisplacementTransducer = 'A',
              * TemperatureTransducer = 'C',
              * LinearDisplacementTransducer = 'D',
              * FrequencyTransducer = 'F',
              * HumidityTransducer = 'H',
              * ForceTransducer = 'N',
              * PressureTransducer = 'P',
              * FlowRateTransducer = 'R',
              * TachometerTransducer = 'T',
              * VolumeTransducer = 'V'
             */

            if (m_NMEA0183.Parse()) { 
                wxString xdrunit;
                double xdrdata;
		for (int i = 0; i<m_NMEA0183.Xdr.TransducerCnt; i++){
                     xdrdata = m_NMEA0183.Xdr.TransducerInfo[i].MeasurementData;
                     // XDR Airtemp
                     if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerType == _T("C")){
                         SendSentenceToAllInstruments(OCPN_DBP_STC_ATMP, xdrdata , m_NMEA0183.Xdr.TransducerInfo[i].UnitOfMeasurement);
                     }
                     // XDR Pressure
                     if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerType == _T("P")){
                         if (m_NMEA0183.Xdr.TransducerInfo[i].UnitOfMeasurement == _T("B")){
  			     xdrdata *= 1000;
                             SendSentenceToAllInstruments(OCPN_DBP_STC_MDA, xdrdata , _T("mBar") );
                         }
		     }
                     // XDR Pitch (=Nose up/down)
                     if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerType == _T("D")) {
                         if (m_NMEA0183.Xdr.TransducerInfo[i].MeasurementData > 0){
                             xdrunit = _T("\u00B0 Nose up");
                         }
                         else if (m_NMEA0183.Xdr.TransducerInfo[i].MeasurementData < 0) {
                             xdrunit = _T("\u00B0 Nose down");
                             xdrdata *= -1;
                         }
                         else {
                             xdrunit = _T("\u00B0");
                         }
                         SendSentenceToAllInstruments(OCPN_DBP_STC_PITCH, xdrdata, xdrunit);
                     }
                     // XDR Heel
                     if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerType == _T("A")) {
                         if (m_NMEA0183.Xdr.TransducerInfo[i].MeasurementData > 0) {
                             xdrunit = _T("\u00B0 to Starboard");
		         }
                         else if (m_NMEA0183.Xdr.TransducerInfo[i].MeasurementData < 0) {
                             xdrunit = _T("\u00B0 to Port");
                             xdrdata *= -1;
                         }
                         else {
                            xdrunit = _T("\u00B0");
			 }
                         SendSentenceToAllInstruments(OCPN_DBP_STC_HEEL, xdrdata, xdrunit);
                     } 
		     //Nasa style water temp
                     if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerName == _T("ENV_WATER_T")){
                         SendSentenceToAllInstruments(OCPN_DBP_STC_TMP, m_NMEA0183.Xdr.TransducerInfo[i].MeasurementData,m_NMEA0183.Xdr.TransducerInfo[i].UnitOfMeasurement);
                     }
                }
            }
        }
        else if (m_NMEA0183.LastSentenceIDReceived == _T("ZDA")) {
           if( m_NMEA0183.Parse() ) {
                if( mPriDateTime >= 2 ) {
                    mPriDateTime = 2;

                    /*
                     wxString m_NMEA0183.Zda.UTCTime;
                     int      m_NMEA0183.Zda.Day;
                     int      m_NMEA0183.Zda.Month;
                     int      m_NMEA0183.Zda.Year;
                     int      m_NMEA0183.Zda.LocalHourDeviation;
                     int      m_NMEA0183.Zda.LocalMinutesDeviation;
                     */
                    wxString dt;
                    dt.Printf( _T("%4d%02d%02d"), m_NMEA0183.Zda.Year, m_NMEA0183.Zda.Month,
                            m_NMEA0183.Zda.Day );
                    dt.Append( m_NMEA0183.Zda.UTCTime );
                    mUTCDateTime.ParseFormat( dt.c_str(), _T("%Y%m%d%H%M%S") );
                }
            }
        }
    }
        //      Process an AIVDO message
    else if( sentence.Mid( 1, 5 ).IsSameAs( _T("AIVDO") ) ) {
        PlugIn_Position_Fix_Ex gpd;
        if( DecodeSingleVDOMessage(sentence, &gpd, &m_VDO_accumulator) ) {

            if( !wxIsNaN(gpd.Lat) )
                SendSentenceToAllInstruments( OCPN_DBP_STC_LAT, gpd.Lat, _T("SDMM") );

            if( !wxIsNaN(gpd.Lon) )
                SendSentenceToAllInstruments( OCPN_DBP_STC_LON, gpd.Lon, _T("SDMM") );

            SendSentenceToAllInstruments( OCPN_DBP_STC_SOG, toUsrSpeed_Plugin( gpd.Sog, g_iDashSpeedUnit ), getUsrSpeedUnit_Plugin( g_iDashSpeedUnit ) );
            SendSentenceToAllInstruments( OCPN_DBP_STC_COG, gpd.Cog, _T("\u00B0") );
            if( !wxIsNaN(gpd.Hdt) ) {
                SendSentenceToAllInstruments( OCPN_DBP_STC_HDT, gpd.Hdt, _T("\u00B0T") );
                mHDT_Watchdog = gps_watchdog_timeout_ticks;
            }
        }
    }
}

void dashboard_pi::SetPositionFix( PlugIn_Position_Fix &pfix )
{
    if( mPriPosition >= 1 ) {
        mPriPosition = 1;
        SendSentenceToAllInstruments( OCPN_DBP_STC_LAT, pfix.Lat, _T("SDMM") );
        SendSentenceToAllInstruments( OCPN_DBP_STC_LON, pfix.Lon, _T("SDMM") );
    }
    if( mPriCOGSOG >= 1 ) {
        double dMagneticCOG;
        mPriCOGSOG = 1;
        SendSentenceToAllInstruments( OCPN_DBP_STC_SOG, toUsrSpeed_Plugin( pfix.Sog, g_iDashSpeedUnit ), getUsrSpeedUnit_Plugin( g_iDashSpeedUnit ) );
        SendSentenceToAllInstruments( OCPN_DBP_STC_COG, pfix.Cog, _T("\u00B0") );
        dMagneticCOG = pfix.Cog - pfix.Var;
        if ( dMagneticCOG < 0.0 ) dMagneticCOG = 360.0 + dMagneticCOG;
        if ( dMagneticCOG > 360.0 ) dMagneticCOG = dMagneticCOG - 360.0;
        SendSentenceToAllInstruments( OCPN_DBP_STC_MCOG, dMagneticCOG , _T("\u00B0M") );
    }
    if( mPriVar >= 1 ) {
        if( !wxIsNaN( pfix.Var ) ){
            mPriVar = 1;
            mVar = pfix.Var;
            mVar_Watchdog = gps_watchdog_timeout_ticks;

            SendSentenceToAllInstruments( OCPN_DBP_STC_HMV, pfix.Var, _T("\u00B0") );
        }
    }
    if( mPriDateTime >= 6 ) { //We prefer the GPS datetime
        mPriDateTime = 6;
        mUTCDateTime.Set( pfix.FixTime );
        mUTCDateTime = mUTCDateTime.ToUTC();
    }
    mSatsInView = pfix.nSats;
//    SendSentenceToAllInstruments( OCPN_DBP_STC_SAT, mSatsInView, _T("") );

}

void dashboard_pi::SetCursorLatLon( double lat, double lon )
{
    SendSentenceToAllInstruments( OCPN_DBP_STC_PLA, lat, _T("SDMM") );
    SendSentenceToAllInstruments( OCPN_DBP_STC_PLO, lon, _T("SDMM") );
}

void dashboard_pi::SetPluginMessage(wxString &message_id, wxString &message_body)
{
    if(message_id == _T("WMM_VARIATION_BOAT"))
    {

        // construct the JSON root object
        wxJSONValue  root;
        // construct a JSON parser
        wxJSONReader reader;

        // now read the JSON text and store it in the 'root' structure
        // check for errors before retreiving values...
        int numErrors = reader.Parse( message_body, &root );
        if ( numErrors > 0 )  {
            //              const wxArrayString& errors = reader.GetErrors();
            return;
        }

        // get the DECL value from the JSON message
        wxString decl = root[_T("Decl")].AsString();
        double decl_val;
        decl.ToDouble(&decl_val);


        if( mPriVar >= 4 ) {
            mPriVar = 4;
            mVar = decl_val;
            mVar_Watchdog = gps_watchdog_timeout_ticks;
            SendSentenceToAllInstruments( OCPN_DBP_STC_HMV, mVar, _T("\u00B0") );
        }
    }
}

int dashboard_pi::GetToolbarToolCount( void )
{
    return 1;
}

void dashboard_pi::ShowPreferencesDialog( wxWindow* parent )
{
    DashboardPreferencesDialog *dialog = new DashboardPreferencesDialog( parent, wxID_ANY,
            m_ArrayOfDashboardWindow );

    if( dialog->ShowModal() == wxID_OK ) {
        delete g_pFontTitle;
        g_pFontTitle = new wxFont( dialog->m_pFontPickerTitle->GetSelectedFont() );
        delete g_pFontData;
        g_pFontData = new wxFont( dialog->m_pFontPickerData->GetSelectedFont() );
        delete g_pFontLabel;
        g_pFontLabel = new wxFont( dialog->m_pFontPickerLabel->GetSelectedFont() );
        delete g_pFontSmall;
        g_pFontSmall = new wxFont( dialog->m_pFontPickerSmall->GetSelectedFont() );

        // OnClose should handle that for us normally but it doesn't seems to do so
        // We must save changes first
        dialog->SaveDashboardConfig();
        m_ArrayOfDashboardWindow.Clear();
        m_ArrayOfDashboardWindow = dialog->m_Config;

        ApplyConfig();
        SaveConfig();
        SetToolbarItemState( m_toolbar_item_id, GetDashboardWindowShownCount() != 0 );
    }
    dialog->Destroy();
}

void dashboard_pi::SetColorScheme( PI_ColorScheme cs )
{
    for( size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
        DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item( i )->m_pDashboardWindow;
        if( dashboard_window ) dashboard_window->SetColorScheme( cs );
    }
}

int dashboard_pi::GetDashboardWindowShownCount()
{
    int cnt = 0;

    for( size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
        DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item( i )->m_pDashboardWindow;
        if( dashboard_window ) {
            wxAuiPaneInfo &pane = m_pauimgr->GetPane( dashboard_window );
            if( pane.IsOk() && pane.IsShown() ) cnt++;
        }
    }
    return cnt;
}

void dashboard_pi::OnPaneClose( wxAuiManagerEvent& event )
{
    // if name is unique, we should use it
    DashboardWindow *dashboard_window = (DashboardWindow *) event.pane->window;
    int cnt = 0;
    for( size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
        DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item( i );
        DashboardWindow *d_w = cont->m_pDashboardWindow;
        if( d_w ) {
            // we must not count this one because it is being closed
            if( dashboard_window != d_w ) {
                wxAuiPaneInfo &pane = m_pauimgr->GetPane( d_w );
                if( pane.IsOk() && pane.IsShown() ) cnt++;
            } else {
                cont->m_bIsVisible = false;
            }
        }
    }
    SetToolbarItemState( m_toolbar_item_id, cnt != 0 );

    event.Skip();
}

void dashboard_pi::OnToolbarToolCallback( int id )
{
    int cnt = GetDashboardWindowShownCount();

    bool b_anyviz = false;
    for( size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
        DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item( i );
        if( cont->m_bIsVisible ) {
            b_anyviz = true;
            break;
        }
    }

    for( size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
        DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item( i );
        DashboardWindow *dashboard_window = cont->m_pDashboardWindow;
        if( dashboard_window ) {
            wxAuiPaneInfo &pane = m_pauimgr->GetPane( dashboard_window );
            if( pane.IsOk() ) {
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

                if( b_reset_pos ) pane.FloatingPosition( 50, 50 );

                if( cnt == 0 )
                    if( b_anyviz )
                        pane.Show( cont->m_bIsVisible );
                    else {
                       cont->m_bIsVisible = cont->m_bPersVisible;
                       pane.Show( cont->m_bIsVisible );
                    }
                else
                    pane.Show( false );
            }

            //  This patch fixes a bug in wxAUIManager
            //  FS#548
            // Dropping a DashBoard Window right on top on the (supposedly fixed) chart bar window
            // causes a resize of the chart bar, and the Dashboard window assumes some of its properties
            // The Dashboard window is no longer grabbable...
            // Workaround:  detect this case, and force the pane to be on a different Row.
            // so that the display is corrected by toggling the dashboard off and back on.
            if( ( pane.dock_direction == wxAUI_DOCK_BOTTOM ) && pane.IsDocked() ) pane.Row( 2 );
        }
    }
    // Toggle is handled by the toolbar but we must keep plugin manager b_toggle updated
    // to actual status to ensure right status upon toolbar rebuild
    SetToolbarItemState( m_toolbar_item_id, GetDashboardWindowShownCount() != 0/*cnt==0*/);
    m_pauimgr->Update();
}

void dashboard_pi::UpdateAuiStatus( void )
{
    //    This method is called after the PlugIn is initialized
    //    and the frame has done its initial layout, possibly from a saved wxAuiManager "Perspective"
    //    It is a chance for the PlugIn to syncronize itself internally with the state of any Panes that
    //    were added to the frame in the PlugIn ctor.

    for( size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
        DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item( i );
        wxAuiPaneInfo &pane = m_pauimgr->GetPane( cont->m_pDashboardWindow );
        // Initialize visible state as perspective is loaded now
        cont->m_bIsVisible = ( pane.IsOk() && pane.IsShown() );
    }

    //    We use this callback here to keep the context menu selection in sync with the window state

    SetToolbarItemState( m_toolbar_item_id, GetDashboardWindowShownCount() != 0 );
}

bool dashboard_pi::LoadConfig( void )
{
    wxFileConfig *pConf = (wxFileConfig *) m_pconfig;

    if( pConf ) {
        pConf->SetPath( _T("/PlugIns/Dashboard") );

        wxString version;
        pConf->Read( _T("Version"), &version, wxEmptyString );
        wxString config;
        pConf->Read( _T("FontTitle"), &config, wxEmptyString );
        if( !config.IsEmpty() ) g_pFontTitle->SetNativeFontInfo( config );
        pConf->Read( _T("FontData"), &config, wxEmptyString );
        if( !config.IsEmpty() ) g_pFontData->SetNativeFontInfo( config );
        pConf->Read( _T("FontLabel"), &config, wxEmptyString );
        if( !config.IsEmpty() ) g_pFontLabel->SetNativeFontInfo( config );
        pConf->Read( _T("FontSmall"), &config, wxEmptyString );
        if( !config.IsEmpty() ) g_pFontSmall->SetNativeFontInfo( config );

        pConf->Read( _T("SpeedometerMax"), &g_iDashSpeedMax, 12 );
        pConf->Read( _T("SpeedUnit"), &g_iDashSpeedUnit, 0 );

        pConf->Read( _T("DepthUnit"), &g_iDashDepthUnit, 3 );
        g_iDashDepthUnit = wxMax(g_iDashDepthUnit, 3);

        pConf->Read( _T("DistanceUnit"), &g_iDashDistanceUnit, 0 );
        pConf->Read( _T("WindSpeedUnit"), &g_iDashWindSpeedUnit, 0 );


        int d_cnt;
        pConf->Read( _T("DashboardCount"), &d_cnt, -1 );
        // TODO: Memory leak? We should destroy everything first
        m_ArrayOfDashboardWindow.Clear();
        if( version.IsEmpty() && d_cnt == -1 ) {
            m_config_version = 1;
            // Let's load version 1 or default settings.
            int i_cnt;
            pConf->Read( _T("InstrumentCount"), &i_cnt, -1 );
            wxArrayInt ar;
            if( i_cnt != -1 ) {
                for( int i = 0; i < i_cnt; i++ ) {
                    int id;
                    pConf->Read( wxString::Format( _T("Instrument%d"), i + 1 ), &id, -1 );
                    if( id != -1 ) ar.Add( id );
                }
            } else {
                // This is the default instrument list
                ar.Add( ID_DBP_I_POS );
                ar.Add( ID_DBP_D_COG );
                ar.Add( ID_DBP_D_GPS );
            }

            DashboardWindowContainer *cont = new DashboardWindowContainer( NULL, GetUUID(), _("Dashboard"), _T("V"), ar );
            cont->m_bPersVisible = true;
            m_ArrayOfDashboardWindow.Add(cont);
            
        } else {
            // Version 2
            m_config_version = 2;
            bool b_onePersisted = false;
            for( int i = 0; i < d_cnt; i++ ) {
                pConf->SetPath( wxString::Format( _T("/PlugIns/Dashboard/Dashboard%d"), i + 1 ) );
                wxString name;
                pConf->Read( _T("Name"), &name, GetUUID() );
                wxString caption;
                pConf->Read( _T("Caption"), &caption, _("Dashboard") );
                wxString orient;
                pConf->Read( _T("Orientation"), &orient, _T("V") );
                int i_cnt;
                pConf->Read( _T("InstrumentCount"), &i_cnt, -1 );
                bool b_persist;
                pConf->Read( _T("Persistence"), &b_persist, 1 );
                
                wxArrayInt ar;
                for( int i = 0; i < i_cnt; i++ ) {
                    int id;
                    pConf->Read( wxString::Format( _T("Instrument%d"), i + 1 ), &id, -1 );
                    if( id != -1 ) ar.Add( id );
                }
// TODO: Do not add if GetCount == 0

                DashboardWindowContainer *cont = new DashboardWindowContainer( NULL, name, caption, orient, ar );
                cont->m_bPersVisible = b_persist;

                if(b_persist)
                    b_onePersisted = true;
                
                m_ArrayOfDashboardWindow.Add(cont);

            }
            
            // Make sure at least one dashboard is scheduled to be visible
            if( m_ArrayOfDashboardWindow.Count() && !b_onePersisted){
                DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item(0);
                if(cont)
                    cont->m_bPersVisible = true;
            }
                
        }

        return true;
    } else
        return false;
}

bool dashboard_pi::SaveConfig( void )
{
    wxFileConfig *pConf = (wxFileConfig *) m_pconfig;

    if( pConf ) {
        pConf->SetPath( _T("/PlugIns/Dashboard") );
        pConf->Write( _T("Version"), _T("2") );
        pConf->Write( _T("FontTitle"), g_pFontTitle->GetNativeFontInfoDesc() );
        pConf->Write( _T("FontData"), g_pFontData->GetNativeFontInfoDesc() );
        pConf->Write( _T("FontLabel"), g_pFontLabel->GetNativeFontInfoDesc() );
        pConf->Write( _T("FontSmall"), g_pFontSmall->GetNativeFontInfoDesc() );

        pConf->Write( _T("SpeedometerMax"), g_iDashSpeedMax );
        pConf->Write( _T("SpeedUnit"), g_iDashSpeedUnit );
        pConf->Write( _T("DepthUnit"), g_iDashDepthUnit );
        pConf->Write( _T("DistanceUnit"), g_iDashDistanceUnit );
        pConf->Write( _T("WindSpeedUnit"), g_iDashWindSpeedUnit );

        pConf->Write( _T("DashboardCount" ), (int) m_ArrayOfDashboardWindow.GetCount() );
        for( unsigned int i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
            DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item( i );
            pConf->SetPath( wxString::Format( _T("/PlugIns/Dashboard/Dashboard%d"), i + 1 ) );
            pConf->Write( _T("Name"), cont->m_sName );
            pConf->Write( _T("Caption"), cont->m_sCaption );
            pConf->Write( _T("Orientation"), cont->m_sOrientation );
            pConf->Write( _T("Persistence"), cont->m_bPersVisible );
            
            pConf->Write( _T("InstrumentCount"), (int) cont->m_aInstrumentList.GetCount() );
            for( unsigned int j = 0; j < cont->m_aInstrumentList.GetCount(); j++ )
                pConf->Write( wxString::Format( _T("Instrument%d"), j + 1 ),
                        cont->m_aInstrumentList.Item( j ) );
        }

        return true;
    } else
        return false;
}

void dashboard_pi::ApplyConfig( void )
{
    // Reverse order to handle deletes
    for( size_t i = m_ArrayOfDashboardWindow.GetCount(); i > 0; i-- ) {
        DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item( i - 1 );
        int orient = ( cont->m_sOrientation == _T("V") ? wxVERTICAL : wxHORIZONTAL );
        if( cont->m_bIsDeleted ) {
            if( cont->m_pDashboardWindow ) {
                m_pauimgr->DetachPane( cont->m_pDashboardWindow );
                cont->m_pDashboardWindow->Close();
                cont->m_pDashboardWindow->Destroy();
                cont->m_pDashboardWindow = NULL;
            }
            m_ArrayOfDashboardWindow.Remove( cont );
            delete cont;

        } else if( !cont->m_pDashboardWindow ) {
            // A new dashboard is created
            cont->m_pDashboardWindow = new DashboardWindow( GetOCPNCanvasWindow(), wxID_ANY,
                    m_pauimgr, this, orient, cont );
            cont->m_pDashboardWindow->SetInstrumentList( cont->m_aInstrumentList );
            bool vertical = orient == wxVERTICAL;
            wxSize sz = cont->m_pDashboardWindow->GetMinSize();
// Mac has a little trouble with initial Layout() sizing...
#ifdef __WXOSX__
            if(sz.x == 0)
                sz.IncTo( wxSize( 160, 388) );
#endif
                wxAuiPaneInfo p = wxAuiPaneInfo().Name( cont->m_sName ).Caption( cont->m_sCaption ).CaptionVisible( false ).TopDockable(
                    !vertical ).BottomDockable( !vertical ).LeftDockable( vertical ).RightDockable( vertical ).MinSize(
                        sz ).BestSize( sz ).FloatingSize( sz ).FloatingPosition( 100, 100 ).Float().Show( cont->m_bIsVisible ).Gripper(false) ;
                        
            m_pauimgr->AddPane( cont->m_pDashboardWindow, p);
                //wxAuiPaneInfo().Name( cont->m_sName ).Caption( cont->m_sCaption ).CaptionVisible( false ).TopDockable(
               // !vertical ).BottomDockable( !vertical ).LeftDockable( vertical ).RightDockable( vertical ).MinSize(
               // sz ).BestSize( sz ).FloatingSize( sz ).FloatingPosition( 100, 100 ).Float().Show( cont->m_bIsVisible ) );
        } else {
            wxAuiPaneInfo& pane = m_pauimgr->GetPane( cont->m_pDashboardWindow );
            pane.Caption( cont->m_sCaption ).Show( cont->m_bIsVisible );
            if( !cont->m_pDashboardWindow->isInstrumentListEqual( cont->m_aInstrumentList ) ) {
                cont->m_pDashboardWindow->SetInstrumentList( cont->m_aInstrumentList );
                wxSize sz = cont->m_pDashboardWindow->GetMinSize();
                pane.MinSize( sz ).BestSize( sz ).FloatingSize( sz );
            }
            if( cont->m_pDashboardWindow->GetSizerOrientation() != orient ) {
                cont->m_pDashboardWindow->ChangePaneOrientation( orient, false );
            }
        }
    }
    m_pauimgr->Update();
}

void dashboard_pi::PopulateContextMenu( wxMenu* menu )
{
    for( size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++ ) {
        DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item( i );
        wxMenuItem* item = menu->AppendCheckItem( i+1, cont->m_sCaption );
        item->Check( cont->m_bIsVisible );
    }
}

void dashboard_pi::ShowDashboard( size_t id, bool visible )
{
    if ( id < m_ArrayOfDashboardWindow.GetCount() ) {
        DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item( id );
        m_pauimgr->GetPane( cont->m_pDashboardWindow ).Show( visible );
        cont->m_bIsVisible = visible;
        cont->m_bPersVisible = visible;
        m_pauimgr->Update();
    }
}

/* DashboardPreferencesDialog
 *
 */

DashboardPreferencesDialog::DashboardPreferencesDialog( wxWindow *parent, wxWindowID id,
        wxArrayOfDashboard config ) :
        wxDialog( parent, id, _("Dashboard preferences"), wxDefaultPosition, wxDefaultSize,
                wxDEFAULT_DIALOG_STYLE )
{
    Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DashboardPreferencesDialog::OnCloseDialog ),
            NULL, this );

    // Copy original config
    m_Config = wxArrayOfDashboard( config );
    //      Build Dashboard Page for Toolbox
    int border_size = 2;

    wxBoxSizer* itemBoxSizerMainPanel = new wxBoxSizer( wxVERTICAL );
    SetSizer( itemBoxSizerMainPanel );

    wxNotebook *itemNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
            wxNB_TOP );
    itemBoxSizerMainPanel->Add( itemNotebook, 1,
            wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, border_size );

    wxPanel *itemPanelNotebook01 = new wxPanel( itemNotebook, wxID_ANY, wxDefaultPosition,
            wxDefaultSize, wxTAB_TRAVERSAL );
    wxFlexGridSizer *itemFlexGridSizer01 = new wxFlexGridSizer( 2 );
    itemFlexGridSizer01->AddGrowableCol( 1 );
    itemPanelNotebook01->SetSizer( itemFlexGridSizer01 );
    itemNotebook->AddPage( itemPanelNotebook01, _("Dashboard") );

    wxBoxSizer *itemBoxSizer01 = new wxBoxSizer( wxVERTICAL );
    itemFlexGridSizer01->Add( itemBoxSizer01, 1, wxEXPAND | wxTOP | wxLEFT, border_size );

    wxImageList *imglist1 = new wxImageList( 32, 32, true, 1 );
    imglist1->Add( *_img_dashboard_pi );

    m_pListCtrlDashboards = new wxListCtrl( itemPanelNotebook01, wxID_ANY, wxDefaultPosition,
            wxSize( 50, 200 ), wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL );
    m_pListCtrlDashboards->AssignImageList( imglist1, wxIMAGE_LIST_SMALL );
    m_pListCtrlDashboards->InsertColumn( 0, _T("") );
    m_pListCtrlDashboards->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED,
            wxListEventHandler(DashboardPreferencesDialog::OnDashboardSelected), NULL, this );
    m_pListCtrlDashboards->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,
            wxListEventHandler(DashboardPreferencesDialog::OnDashboardSelected), NULL, this );
    itemBoxSizer01->Add( m_pListCtrlDashboards, 1, wxEXPAND, 0 );

    wxBoxSizer *itemBoxSizer02 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer01->Add( itemBoxSizer02 );

    m_pButtonAddDashboard = new wxBitmapButton( itemPanelNotebook01, wxID_ANY, *_img_plus,
            wxDefaultPosition, wxDefaultSize );
    itemBoxSizer02->Add( m_pButtonAddDashboard, 0, wxALIGN_CENTER, 2 );
    m_pButtonAddDashboard->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnDashboardAdd), NULL, this );
    m_pButtonDeleteDashboard = new wxBitmapButton( itemPanelNotebook01, wxID_ANY, *_img_minus,
            wxDefaultPosition, wxDefaultSize );
    itemBoxSizer02->Add( m_pButtonDeleteDashboard, 0, wxALIGN_CENTER, 2 );
    m_pButtonDeleteDashboard->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnDashboardDelete), NULL, this );

    m_pPanelDashboard = new wxPanel( itemPanelNotebook01, wxID_ANY, wxDefaultPosition,
            wxDefaultSize, wxBORDER_SUNKEN );
    itemFlexGridSizer01->Add( m_pPanelDashboard, 1, wxEXPAND | wxTOP | wxRIGHT, border_size );

    wxBoxSizer* itemBoxSizer03 = new wxBoxSizer( wxVERTICAL );
    m_pPanelDashboard->SetSizer( itemBoxSizer03 );

    wxStaticBox* itemStaticBox02 = new wxStaticBox( m_pPanelDashboard, wxID_ANY, _("Dashboard") );
    wxStaticBoxSizer* itemStaticBoxSizer02 = new wxStaticBoxSizer( itemStaticBox02, wxHORIZONTAL );
    itemBoxSizer03->Add( itemStaticBoxSizer02, 0, wxEXPAND | wxALL, border_size );
    wxFlexGridSizer *itemFlexGridSizer = new wxFlexGridSizer( 2 );
    itemFlexGridSizer->AddGrowableCol( 1 );
    itemStaticBoxSizer02->Add( itemFlexGridSizer, 1, wxEXPAND | wxALL, 0 );

    m_pCheckBoxIsVisible = new wxCheckBox( m_pPanelDashboard, wxID_ANY, _("show this dashboard"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer->Add( m_pCheckBoxIsVisible, 0, wxEXPAND | wxALL, border_size );
    wxStaticText *itemDummy01 = new wxStaticText( m_pPanelDashboard, wxID_ANY, _T("") );
    itemFlexGridSizer->Add( itemDummy01, 0, wxEXPAND | wxALL, border_size );

    wxStaticText* itemStaticText01 = new wxStaticText( m_pPanelDashboard, wxID_ANY, _("Caption:"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer->Add( itemStaticText01, 0, wxEXPAND | wxALL, border_size );
    m_pTextCtrlCaption = new wxTextCtrl( m_pPanelDashboard, wxID_ANY, _T(""), wxDefaultPosition,
            wxDefaultSize );
    itemFlexGridSizer->Add( m_pTextCtrlCaption, 0, wxEXPAND | wxALL, border_size );

    wxStaticText* itemStaticText02 = new wxStaticText( m_pPanelDashboard, wxID_ANY,
            _("Orientation:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer->Add( itemStaticText02, 0, wxEXPAND | wxALL, border_size );
    m_pChoiceOrientation = new wxChoice( m_pPanelDashboard, wxID_ANY, wxDefaultPosition,
            wxSize( 120, -1 ) );
    m_pChoiceOrientation->Append( _("Vertical") );
    m_pChoiceOrientation->Append( _("Horizontal") );
    itemFlexGridSizer->Add( m_pChoiceOrientation, 0, wxALIGN_RIGHT | wxALL, border_size );

    wxImageList *imglist = new wxImageList( 20, 20, true, 2 );
    imglist->Add( *_img_instrument );
    imglist->Add( *_img_dial );

    wxStaticBox* itemStaticBox03 = new wxStaticBox( m_pPanelDashboard, wxID_ANY, _("Instruments") );
    wxStaticBoxSizer* itemStaticBoxSizer03 = new wxStaticBoxSizer( itemStaticBox03, wxHORIZONTAL );
    itemBoxSizer03->Add( itemStaticBoxSizer03, 1, wxEXPAND | wxALL, border_size );

    m_pListCtrlInstruments = new wxListCtrl( m_pPanelDashboard, wxID_ANY, wxDefaultPosition,
            wxSize( -1, 200 ), wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL );
    itemStaticBoxSizer03->Add( m_pListCtrlInstruments, 1, wxEXPAND | wxALL, border_size );
    m_pListCtrlInstruments->AssignImageList( imglist, wxIMAGE_LIST_SMALL );
    m_pListCtrlInstruments->InsertColumn( 0, _("Instruments") );
    m_pListCtrlInstruments->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED,
            wxListEventHandler(DashboardPreferencesDialog::OnInstrumentSelected), NULL, this );
    m_pListCtrlInstruments->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,
            wxListEventHandler(DashboardPreferencesDialog::OnInstrumentSelected), NULL, this );

    wxBoxSizer* itemBoxSizer04 = new wxBoxSizer( wxVERTICAL );
    itemStaticBoxSizer03->Add( itemBoxSizer04, 0, wxALIGN_TOP | wxALL, border_size );
    m_pButtonAdd = new wxButton( m_pPanelDashboard, wxID_ANY, _("Add"), wxDefaultPosition,
            wxSize( 20, -1 ) );
    itemBoxSizer04->Add( m_pButtonAdd, 0, wxEXPAND | wxALL, border_size );
    m_pButtonAdd->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentAdd), NULL, this );

/* TODO  Instrument Properties
    m_pButtonEdit = new wxButton( m_pPanelDashboard, wxID_ANY, _("Edit"), wxDefaultPosition,
            wxDefaultSize );
    itemBoxSizer04->Add( m_pButtonEdit, 0, wxEXPAND | wxALL, border_size );
    m_pButtonEdit->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentEdit), NULL, this );
*/
    m_pButtonDelete = new wxButton( m_pPanelDashboard, wxID_ANY, _("Delete"), wxDefaultPosition,
            wxSize( 20, -1 ) );
    itemBoxSizer04->Add( m_pButtonDelete, 0, wxEXPAND | wxALL, border_size );
    m_pButtonDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentDelete), NULL, this );
    itemBoxSizer04->AddSpacer( 10 );
    m_pButtonUp = new wxButton( m_pPanelDashboard, wxID_ANY, _("Up"), wxDefaultPosition,
            wxDefaultSize );
    itemBoxSizer04->Add( m_pButtonUp, 0, wxEXPAND | wxALL, border_size );
    m_pButtonUp->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentUp), NULL, this );
    m_pButtonDown = new wxButton( m_pPanelDashboard, wxID_ANY, _("Down"), wxDefaultPosition,
            wxDefaultSize );
    itemBoxSizer04->Add( m_pButtonDown, 0, wxEXPAND | wxALL, border_size );
    m_pButtonDown->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentDown), NULL, this );

    wxPanel *itemPanelNotebook02 = new wxPanel( itemNotebook, wxID_ANY, wxDefaultPosition,
            wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer05 = new wxBoxSizer( wxVERTICAL );
    itemPanelNotebook02->SetSizer( itemBoxSizer05 );
    itemNotebook->AddPage( itemPanelNotebook02, _("Appearance") );

    wxStaticBox* itemStaticBox01 = new wxStaticBox( itemPanelNotebook02, wxID_ANY, _("Fonts") );
    wxStaticBoxSizer* itemStaticBoxSizer01 = new wxStaticBoxSizer( itemStaticBox01, wxHORIZONTAL );
    itemBoxSizer05->Add( itemStaticBoxSizer01, 0, wxEXPAND | wxALL, border_size );
    wxFlexGridSizer *itemFlexGridSizer03 = new wxFlexGridSizer( 2 );
    itemFlexGridSizer03->AddGrowableCol( 1 );
    itemStaticBoxSizer01->Add( itemFlexGridSizer03, 1, wxEXPAND | wxALL, 0 );
    wxStaticText* itemStaticText04 = new wxStaticText( itemPanelNotebook02, wxID_ANY, _("Title:"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer03->Add( itemStaticText04, 0, wxEXPAND | wxALL, border_size );
    m_pFontPickerTitle = new wxFontPickerCtrl( itemPanelNotebook02, wxID_ANY, *g_pFontTitle,
            wxDefaultPosition, wxDefaultSize );
    itemFlexGridSizer03->Add( m_pFontPickerTitle, 0, wxALIGN_RIGHT | wxALL, 0 );
    wxStaticText* itemStaticText05 = new wxStaticText( itemPanelNotebook02, wxID_ANY, _("Data:"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer03->Add( itemStaticText05, 0, wxEXPAND | wxALL, border_size );
    m_pFontPickerData = new wxFontPickerCtrl( itemPanelNotebook02, wxID_ANY, *g_pFontData,
            wxDefaultPosition, wxDefaultSize );
    itemFlexGridSizer03->Add( m_pFontPickerData, 0, wxALIGN_RIGHT | wxALL, 0 );
    wxStaticText* itemStaticText06 = new wxStaticText( itemPanelNotebook02, wxID_ANY, _("Label:"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer03->Add( itemStaticText06, 0, wxEXPAND | wxALL, border_size );
    m_pFontPickerLabel = new wxFontPickerCtrl( itemPanelNotebook02, wxID_ANY, *g_pFontLabel,
            wxDefaultPosition, wxDefaultSize );
    itemFlexGridSizer03->Add( m_pFontPickerLabel, 0, wxALIGN_RIGHT | wxALL, 0 );
    wxStaticText* itemStaticText07 = new wxStaticText( itemPanelNotebook02, wxID_ANY, _("Small:"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer03->Add( itemStaticText07, 0, wxEXPAND | wxALL, border_size );
    m_pFontPickerSmall = new wxFontPickerCtrl( itemPanelNotebook02, wxID_ANY, *g_pFontSmall,
            wxDefaultPosition, wxDefaultSize );
    itemFlexGridSizer03->Add( m_pFontPickerSmall, 0, wxALIGN_RIGHT | wxALL, 0 );
//      wxColourPickerCtrl

    wxStaticBox* itemStaticBox04 = new wxStaticBox( itemPanelNotebook02, wxID_ANY, _("Units, Ranges, Formats") );
    wxStaticBoxSizer* itemStaticBoxSizer04 = new wxStaticBoxSizer( itemStaticBox04, wxHORIZONTAL );
    itemBoxSizer05->Add( itemStaticBoxSizer04, 0, wxEXPAND | wxALL, border_size );
    wxFlexGridSizer *itemFlexGridSizer04 = new wxFlexGridSizer( 2 );
    itemFlexGridSizer04->AddGrowableCol( 1 );
    itemStaticBoxSizer04->Add( itemFlexGridSizer04, 1, wxEXPAND | wxALL, 0 );
    wxStaticText* itemStaticText08 = new wxStaticText( itemPanelNotebook02, wxID_ANY, _("Speedometer max value:"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer04->Add( itemStaticText08, 0, wxEXPAND | wxALL, border_size );
    m_pSpinSpeedMax = new wxSpinCtrl( itemPanelNotebook02, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10, 100, g_iDashSpeedMax );
    itemFlexGridSizer04->Add( m_pSpinSpeedMax, 0, wxALIGN_RIGHT | wxALL, 0 );
    wxStaticText* itemStaticText09 = new wxStaticText( itemPanelNotebook02, wxID_ANY, _("Boat speed units:"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer04->Add( itemStaticText09, 0, wxEXPAND | wxALL, border_size );
    wxString m_SpeedUnitChoices[] = { _("Honor OpenCPN settings"), _("Kts"), _("mph"), _("km/h"), _("m/s") };
    int m_SpeedUnitNChoices = sizeof( m_SpeedUnitChoices ) / sizeof( wxString );
    m_pChoiceSpeedUnit = new wxChoice( itemPanelNotebook02, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_SpeedUnitNChoices, m_SpeedUnitChoices, 0 );
    m_pChoiceSpeedUnit->SetSelection( g_iDashSpeedUnit + 1 );
    itemFlexGridSizer04->Add( m_pChoiceSpeedUnit, 0, wxALIGN_RIGHT | wxALL, 0 );

    wxStaticText* itemStaticTextDepthU = new wxStaticText( itemPanelNotebook02, wxID_ANY, _("Depth units:"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer04->Add( itemStaticTextDepthU, 0, wxEXPAND | wxALL, border_size );
    wxString m_DepthUnitChoices[] = { _("Meters"), _("Feet"), _("Fathoms"), _("Centimeters"), _("Inches") };
    int m_DepthUnitNChoices = sizeof( m_DepthUnitChoices ) / sizeof( wxString );
    m_pChoiceDepthUnit = new wxChoice( itemPanelNotebook02, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_DepthUnitNChoices, m_DepthUnitChoices, 0 );
    m_pChoiceDepthUnit->SetSelection( g_iDashDepthUnit - 3);
    itemFlexGridSizer04->Add( m_pChoiceDepthUnit, 0, wxALIGN_RIGHT | wxALL, 0 );

    wxStaticText* itemStaticText0b = new wxStaticText( itemPanelNotebook02, wxID_ANY, _("Distance units:"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer04->Add( itemStaticText0b, 0, wxEXPAND | wxALL, border_size );
    wxString m_DistanceUnitChoices[] = { _("Honor OpenCPN settings"), _("Nautical miles"), _("Statute miles"), _("Kilometers"), _("Meters") };
    int m_DistanceUnitNChoices = sizeof( m_DistanceUnitChoices ) / sizeof( wxString );
    m_pChoiceDistanceUnit = new wxChoice( itemPanelNotebook02, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_DistanceUnitNChoices, m_DistanceUnitChoices, 0 );
    m_pChoiceDistanceUnit->SetSelection( g_iDashDistanceUnit + 1 );
    itemFlexGridSizer04->Add( m_pChoiceDistanceUnit, 0, wxALIGN_RIGHT | wxALL, 0 );

    wxStaticText* itemStaticText0a = new wxStaticText( itemPanelNotebook02, wxID_ANY, _("Wind speed units:"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer04->Add( itemStaticText0a, 0, wxEXPAND | wxALL, border_size );
    wxString m_WSpeedUnitChoices[] = { _("Kts"), _("mph"), _("km/h"), _("m/s") };
    int m_WSpeedUnitNChoices = sizeof( m_WSpeedUnitChoices ) / sizeof( wxString );
    m_pChoiceWindSpeedUnit = new wxChoice( itemPanelNotebook02, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_WSpeedUnitNChoices, m_WSpeedUnitChoices, 0 );
    m_pChoiceWindSpeedUnit->SetSelection( g_iDashWindSpeedUnit );
    itemFlexGridSizer04->Add( m_pChoiceWindSpeedUnit, 0, wxALIGN_RIGHT | wxALL, 0 );


    wxStdDialogButtonSizer* DialogButtonSizer = CreateStdDialogButtonSizer( wxOK | wxCANCEL );
    itemBoxSizerMainPanel->Add( DialogButtonSizer, 0, wxALIGN_RIGHT | wxALL, 5 );

    curSel = -1;
    for( size_t i = 0; i < m_Config.GetCount(); i++ ) {
        m_pListCtrlDashboards->InsertItem( i, 0 );
        // Using data to store m_Config index for managing deletes
        m_pListCtrlDashboards->SetItemData( i, i );
    }
    m_pListCtrlDashboards->SetColumnWidth( 0, wxLIST_AUTOSIZE );

    UpdateDashboardButtonsState();
    UpdateButtonsState();
    SetMinSize( wxSize( 450, -1 ) );
    Fit();
}

void DashboardPreferencesDialog::OnCloseDialog( wxCloseEvent& event )
{
    SaveDashboardConfig();
    event.Skip();
}

void DashboardPreferencesDialog::SaveDashboardConfig()
{
    g_iDashSpeedMax = m_pSpinSpeedMax->GetValue();
    g_iDashSpeedUnit = m_pChoiceSpeedUnit->GetSelection() - 1;
    g_iDashDepthUnit = m_pChoiceDepthUnit->GetSelection() + 3;
    g_iDashDistanceUnit = m_pChoiceDistanceUnit->GetSelection() - 1;
    g_iDashWindSpeedUnit = m_pChoiceWindSpeedUnit->GetSelection();
    if( curSel != -1 ) {
        DashboardWindowContainer *cont = m_Config.Item( curSel );
        cont->m_bIsVisible = m_pCheckBoxIsVisible->IsChecked();
        cont->m_sCaption = m_pTextCtrlCaption->GetValue();
        cont->m_sOrientation = m_pChoiceOrientation->GetSelection() == 0 ? _T("V") : _T("H");
        cont->m_aInstrumentList.Clear();
        for( int i = 0; i < m_pListCtrlInstruments->GetItemCount(); i++ )
            cont->m_aInstrumentList.Add( (int) m_pListCtrlInstruments->GetItemData( i ) );
    }
}

void DashboardPreferencesDialog::OnDashboardSelected( wxListEvent& event )
{
    // save changes
    SaveDashboardConfig();
    UpdateDashboardButtonsState();
}

void DashboardPreferencesDialog::UpdateDashboardButtonsState()
{
    long item = -1;
    item = m_pListCtrlDashboards->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    bool enable = ( item != -1 );

    //  Disable the Dashboard Delete button if the parent(Dashboard) of this dialog is selected.
    bool delete_enable = enable;
    if( item != -1 ) {
        int sel = m_pListCtrlDashboards->GetItemData( item );
        DashboardWindowContainer *cont = m_Config.Item( sel );
        DashboardWindow *dash_sel = cont->m_pDashboardWindow;
        if(dash_sel == GetParent())
            delete_enable = false;
    }
    m_pButtonDeleteDashboard->Enable( delete_enable );

    m_pPanelDashboard->Enable( enable );

    if( item != -1 ) {
        curSel = m_pListCtrlDashboards->GetItemData( item );
        DashboardWindowContainer *cont = m_Config.Item( curSel );
        m_pCheckBoxIsVisible->SetValue( cont->m_bIsVisible );
        m_pTextCtrlCaption->SetValue( cont->m_sCaption );
        m_pChoiceOrientation->SetSelection( cont->m_sOrientation == _T("V") ? 0 : 1 );
        m_pListCtrlInstruments->DeleteAllItems();
        for( size_t i = 0; i < cont->m_aInstrumentList.GetCount(); i++ ) {
            wxListItem item;
            getListItemForInstrument( item, cont->m_aInstrumentList.Item( i ) );
            item.SetId( m_pListCtrlInstruments->GetItemCount() );
            m_pListCtrlInstruments->InsertItem( item );
        }

        m_pListCtrlInstruments->SetColumnWidth( 0, wxLIST_AUTOSIZE );
    } else {
        curSel = -1;
        m_pCheckBoxIsVisible->SetValue( false );
        m_pTextCtrlCaption->SetValue( _T("") );
        m_pChoiceOrientation->SetSelection( 0 );
        m_pListCtrlInstruments->DeleteAllItems();
    }
//      UpdateButtonsState();
}

void DashboardPreferencesDialog::OnDashboardAdd( wxCommandEvent& event )
{
    int idx = m_pListCtrlDashboards->GetItemCount();
    m_pListCtrlDashboards->InsertItem( idx, 0 );
    // Data is index in m_Config
    m_pListCtrlDashboards->SetItemData( idx, m_Config.GetCount() );
    wxArrayInt ar;
    DashboardWindowContainer *dwc = new DashboardWindowContainer( NULL, GetUUID(), _("Dashboard"), _T("V"), ar );
    dwc->m_bIsVisible = true;
    m_Config.Add( dwc );
}

void DashboardPreferencesDialog::OnDashboardDelete( wxCommandEvent& event )
{
    long itemID = -1;
    itemID = m_pListCtrlDashboards->GetNextItem( itemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

    int idx = m_pListCtrlDashboards->GetItemData( itemID );
    m_pListCtrlDashboards->DeleteItem( itemID );
    m_Config.Item( idx )->m_bIsDeleted = true;
    UpdateDashboardButtonsState();
}

void DashboardPreferencesDialog::OnInstrumentSelected( wxListEvent& event )
{
    UpdateButtonsState();
}

void DashboardPreferencesDialog::UpdateButtonsState()
{
    long item = -1;
    item = m_pListCtrlInstruments->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    bool enable = ( item != -1 );

    m_pButtonDelete->Enable( enable );
//    m_pButtonEdit->Enable( false ); // TODO: Properties
    m_pButtonUp->Enable( item > 0 );
    m_pButtonDown->Enable( item != -1 && item < m_pListCtrlInstruments->GetItemCount() - 1 );
}

void DashboardPreferencesDialog::OnInstrumentAdd( wxCommandEvent& event )
{
    AddInstrumentDlg pdlg( (wxWindow *) event.GetEventObject(), wxID_ANY );

    if( pdlg.ShowModal() == wxID_OK ) {
        wxListItem item;
        getListItemForInstrument( item, pdlg.GetInstrumentAdded() );
        item.SetId( m_pListCtrlInstruments->GetItemCount() );
        m_pListCtrlInstruments->InsertItem( item );
        m_pListCtrlInstruments->SetColumnWidth( 0, wxLIST_AUTOSIZE );
        UpdateButtonsState();
    }
}

void DashboardPreferencesDialog::OnInstrumentDelete( wxCommandEvent& event )
{
    long itemID = -1;
    itemID = m_pListCtrlInstruments->GetNextItem( itemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

    m_pListCtrlInstruments->DeleteItem( itemID );
    UpdateButtonsState();
}

void DashboardPreferencesDialog::OnInstrumentEdit( wxCommandEvent& event )
{
// TODO: Instument options
}

void DashboardPreferencesDialog::OnInstrumentUp( wxCommandEvent& event )
{
    long itemID = -1;
    itemID = m_pListCtrlInstruments->GetNextItem( itemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

    wxListItem item;
    item.SetId( itemID );
    item.SetMask( wxLIST_MASK_TEXT | wxLIST_MASK_IMAGE | wxLIST_MASK_DATA );
    m_pListCtrlInstruments->GetItem( item );
    item.SetId( itemID - 1 );
    m_pListCtrlInstruments->DeleteItem( itemID );
    m_pListCtrlInstruments->InsertItem( item );
    m_pListCtrlInstruments->SetItemState( itemID - 1, wxLIST_STATE_SELECTED,
            wxLIST_STATE_SELECTED );
    UpdateButtonsState();
}

void DashboardPreferencesDialog::OnInstrumentDown( wxCommandEvent& event )
{
    long itemID = -1;
    itemID = m_pListCtrlInstruments->GetNextItem( itemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

    wxListItem item;
    item.SetId( itemID );
    item.SetMask( wxLIST_MASK_TEXT | wxLIST_MASK_IMAGE | wxLIST_MASK_DATA );
    m_pListCtrlInstruments->GetItem( item );
    item.SetId( itemID + 1 );
    m_pListCtrlInstruments->DeleteItem( itemID );
    m_pListCtrlInstruments->InsertItem( item );
    m_pListCtrlInstruments->SetItemState( itemID + 1, wxLIST_STATE_SELECTED,
            wxLIST_STATE_SELECTED );
    UpdateButtonsState();
}

//----------------------------------------------------------------
//
//    Add Instrument Dialog Implementation
//
//----------------------------------------------------------------

AddInstrumentDlg::AddInstrumentDlg( wxWindow *pparent, wxWindowID id ) :
        wxDialog( pparent, id, _("Add instrument"), wxDefaultPosition, wxDefaultSize,
                wxDEFAULT_DIALOG_STYLE )
{
    wxBoxSizer* itemBoxSizer01 = new wxBoxSizer( wxVERTICAL );
    SetSizer( itemBoxSizer01 );
    wxStaticText* itemStaticText01 = new wxStaticText( this, wxID_ANY,
            _("Select instrument to add:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer01->Add( itemStaticText01, 0, wxEXPAND | wxALL, 5 );

    wxImageList *imglist = new wxImageList( 20, 20, true, 2 );
    imglist->Add( *_img_instrument );
    imglist->Add( *_img_dial );

    m_pListCtrlInstruments = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxSize( 250, 180 ),
            wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING );
    itemBoxSizer01->Add( m_pListCtrlInstruments, 0, wxEXPAND | wxALL, 5 );
    m_pListCtrlInstruments->AssignImageList( imglist, wxIMAGE_LIST_SMALL );
    m_pListCtrlInstruments->InsertColumn( 0, _("Instruments") );
    wxStdDialogButtonSizer* DialogButtonSizer = CreateStdDialogButtonSizer( wxOK | wxCANCEL );
    itemBoxSizer01->Add( DialogButtonSizer, 0, wxALIGN_RIGHT | wxALL, 5 );

    for( unsigned int i = ID_DBP_I_POS; i < ID_DBP_LAST_ENTRY; i++ ) { //do not reference an instrument, but the last dummy entry in the list
        wxListItem item;
        if( IsObsolete( i ) ) continue;
        getListItemForInstrument( item, i );
        item.SetId( i );
        m_pListCtrlInstruments->InsertItem( item );
    }

    m_pListCtrlInstruments->SetColumnWidth( 0, wxLIST_AUTOSIZE );
    m_pListCtrlInstruments->SetItemState( 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    Fit();
}

unsigned int AddInstrumentDlg::GetInstrumentAdded()
{
    long itemID = -1;
    itemID = m_pListCtrlInstruments->GetNextItem( itemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

    return (int) m_pListCtrlInstruments->GetItemData( itemID );
}

//----------------------------------------------------------------
//
//    Dashboard Window Implementation
//
//----------------------------------------------------------------

// wxWS_EX_VALIDATE_RECURSIVELY required to push events to parents
DashboardWindow::DashboardWindow( wxWindow *pparent, wxWindowID id, wxAuiManager *auimgr,
        dashboard_pi* plugin, int orient, DashboardWindowContainer* mycont ) :
        wxWindow( pparent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE,
                _T("Dashboard") )
{
    m_pauimgr = auimgr;
    m_plugin = plugin;
    m_Container = mycont;

//wx2.9      itemBoxSizer = new wxWrapSizer( orient );
    itemBoxSizer = new wxBoxSizer( orient );
    SetSizer( itemBoxSizer );
    Connect( wxEVT_SIZE, wxSizeEventHandler( DashboardWindow::OnSize ), NULL, this );
    Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( DashboardWindow::OnContextMenu ), NULL,
            this );
    Connect( wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( DashboardWindow::OnContextMenuSelect ), NULL, this );
}

DashboardWindow::~DashboardWindow()
{
    for( size_t i = 0; i < m_ArrayOfInstrument.GetCount(); i++ ) {
        DashboardInstrumentContainer *pdic = m_ArrayOfInstrument.Item( i );
        delete pdic;
    }
}

void DashboardWindow::OnSize( wxSizeEvent& event )
{
    event.Skip();
    for( unsigned int i=0; i<m_ArrayOfInstrument.size(); i++ ) {
        DashboardInstrument* inst = m_ArrayOfInstrument.Item(i)->m_pInstrument;
        inst->SetMinSize( inst->GetSize( itemBoxSizer->GetOrientation(), GetClientSize() ) );
    }
    Layout();
    Refresh();
}

void DashboardWindow::OnContextMenu( wxContextMenuEvent& event )
{
    wxMenu* contextMenu = new wxMenu();

    wxMenuItem* btnVertical = contextMenu->AppendRadioItem( ID_DASH_VERTICAL, _("Vertical") );
    btnVertical->Check( itemBoxSizer->GetOrientation() == wxVERTICAL );
    wxMenuItem* btnHorizontal = contextMenu->AppendRadioItem( ID_DASH_HORIZONTAL, _("Horizontal") );
    btnHorizontal->Check( itemBoxSizer->GetOrientation() == wxHORIZONTAL );
    contextMenu->AppendSeparator();

    m_plugin->PopulateContextMenu( contextMenu );

    contextMenu->AppendSeparator();
    contextMenu->Append( ID_DASH_PREFS, _("Preferences...") );
    PopupMenu( contextMenu );
    delete contextMenu;
}

void DashboardWindow::OnContextMenuSelect( wxCommandEvent& event )
{
    if( event.GetId() < ID_DASH_PREFS ) { // Toggle dashboard visibility
        m_plugin->ShowDashboard( event.GetId()-1, event.IsChecked() );
        if( m_plugin )
            SetToolbarItemState( m_plugin->GetToolbarItemId(), m_plugin->GetDashboardWindowShownCount() != 0 );
    }

    switch( event.GetId() ){
        case ID_DASH_PREFS: {
            m_plugin->ShowPreferencesDialog( this );
            return; // Does it's own save.
        }
        case ID_DASH_VERTICAL: {
            ChangePaneOrientation( wxVERTICAL, true );
            m_Container->m_sOrientation = _T("V");
            break;
        }
        case ID_DASH_HORIZONTAL: {
            ChangePaneOrientation( wxHORIZONTAL, true );
            m_Container->m_sOrientation = _T("H");
            break;
        }
    }
    m_plugin->SaveConfig();
}

void DashboardWindow::SetColorScheme( PI_ColorScheme cs )
{
    DimeWindow( this );
    
    //  Improve appearance, especially in DUSK or NIGHT palette
    wxColour col;
    GetGlobalColor( _T("DASHL"), &col );
    SetBackgroundColour( col );
    
    Refresh( false );
}

void DashboardWindow::ChangePaneOrientation( int orient, bool updateAUImgr )
{
    m_pauimgr->DetachPane( this );
    SetSizerOrientation( orient );
    bool vertical = orient == wxVERTICAL;
    //wxSize sz = GetSize( orient, wxDefaultSize );
    wxSize sz = GetMinSize();
    // We must change Name to reset AUI perpective
    m_Container->m_sName = GetUUID();
    m_pauimgr->AddPane( this, wxAuiPaneInfo().Name( m_Container->m_sName ).Caption(
        m_Container->m_sCaption ).CaptionVisible( true ).TopDockable( !vertical ).BottomDockable(
        !vertical ).LeftDockable( vertical ).RightDockable( vertical ).MinSize( sz ).BestSize(
        sz ).FloatingSize( sz ).FloatingPosition( 100, 100 ).Float().Show( m_Container->m_bIsVisible ) );
    if ( updateAUImgr ) m_pauimgr->Update();
}

void DashboardWindow::SetSizerOrientation( int orient )
{
    itemBoxSizer->SetOrientation( orient );
    /* We must reset all MinSize to ensure we start with new default */
    wxWindowListNode* node = GetChildren().GetFirst();
    while(node) {
        node->GetData()->SetMinSize( wxDefaultSize );
        node = node->GetNext();
    }
    SetMinSize( wxDefaultSize );
    Fit();
    SetMinSize( itemBoxSizer->GetMinSize() );
}

int DashboardWindow::GetSizerOrientation()
{
    return itemBoxSizer->GetOrientation();
}

bool isArrayIntEqual( const wxArrayInt& l1, const wxArrayOfInstrument &l2 )
{
    if( l1.GetCount() != l2.GetCount() ) return false;

    for( size_t i = 0; i < l1.GetCount(); i++ )
        if( l1.Item( i ) != l2.Item( i )->m_ID ) return false;

    return true;
}

bool DashboardWindow::isInstrumentListEqual( const wxArrayInt& list )
{
    return isArrayIntEqual( list, m_ArrayOfInstrument );
}

void DashboardWindow::SetInstrumentList( wxArrayInt list )
{
    /* options
     ID_DBP_D_SOG: config max value, show STW optional
     ID_DBP_D_COG:  +SOG +HDG? +BRG?
     ID_DBP_D_AWS: config max value. Two arrows for AWS+TWS?
     ID_DBP_D_VMG: config max value
     ID_DBP_I_DPT: config unit (meter, feet, fathoms)
     ID_DBP_D_DPT: show temp optional
     // compass: use COG or HDG
     // velocity range
     // rudder range

     */
    m_ArrayOfInstrument.Clear();
    itemBoxSizer->Clear( true );
    for( size_t i = 0; i < list.GetCount(); i++ ) {
        int id = list.Item( i );
        DashboardInstrument *instrument = NULL;
        switch( id ){
            case ID_DBP_I_POS:
                instrument = new DashboardInstrument_Position( this, wxID_ANY,
                        getInstrumentCaption( id ) );
                break;
            case ID_DBP_I_SOG:
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_SOG, _T("%5.2f") );
                break;
            case ID_DBP_D_SOG:
                instrument = new DashboardInstrument_Speedometer( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_SOG, 0, g_iDashSpeedMax );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionLabel( g_iDashSpeedMax / 20 + 1,
                        DIAL_LABEL_HORIZONTAL );
                //(DashboardInstrument_Dial *)instrument->SetOptionMarker(0.1, DIAL_MARKER_SIMPLE, 5);
                ( (DashboardInstrument_Dial *) instrument )->SetOptionMarker( 0.5,
                        DIAL_MARKER_SIMPLE, 2 );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionExtraValue(
                        OCPN_DBP_STC_STW, _T("STW\n%.2f"), DIAL_POSITION_BOTTOMLEFT );
                break;
            case ID_DBP_I_COG:
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_COG, _T("%.0f") );
                break;
            case ID_DBP_M_COG:
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_MCOG, _T("%.0f") );
                break;
            case ID_DBP_D_COG:
                instrument = new DashboardInstrument_Compass( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_COG );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionMarker( 5,
                        DIAL_MARKER_SIMPLE, 2 );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionLabel( 30,
                        DIAL_LABEL_ROTATED );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionExtraValue(
                        OCPN_DBP_STC_SOG, _T("SOG\n%.2f"), DIAL_POSITION_BOTTOMLEFT );
                break;
            case ID_DBP_D_HDT:
                instrument = new DashboardInstrument_Compass( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_HDT );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionMarker( 5,
                        DIAL_MARKER_SIMPLE, 2 );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionLabel( 30,
                        DIAL_LABEL_ROTATED );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionExtraValue(
                        OCPN_DBP_STC_STW, _T("STW\n%.2f"), DIAL_POSITION_BOTTOMLEFT );
                break;
            case ID_DBP_I_STW:
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_STW, _T("%.2f") );
                break;
            case ID_DBP_I_HDT: //true heading
                // TODO: Option True or Magnetic
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_HDT, _T("%.0f") );
                break;
            case ID_DBP_I_HDM:  //magnetic heading
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_HDM, _T("%.0f") );
                break;
            case ID_DBP_D_AW:
            case ID_DBP_D_AWA:
                instrument = new DashboardInstrument_Wind( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_AWA );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionMainValue( _T("%.0f"),
                        DIAL_POSITION_BOTTOMLEFT );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionExtraValue(
                        OCPN_DBP_STC_AWS, _T("%.1f"), DIAL_POSITION_INSIDE );
                break;
            case ID_DBP_I_AWS:
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_AWS, _T("%.2f") );
                break;
            case ID_DBP_D_AWS:
                instrument = new DashboardInstrument_Speedometer( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_AWS, 0, 45 );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionLabel( 5,
                        DIAL_LABEL_HORIZONTAL );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionMarker( 1,
                        DIAL_MARKER_SIMPLE, 5 );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionMainValue( _T("A %.2f"),
                        DIAL_POSITION_BOTTOMLEFT );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionExtraValue(
                        OCPN_DBP_STC_TWS, _T("T %.1f"), DIAL_POSITION_BOTTOMRIGHT );
                break;
            case ID_DBP_D_TW: //True Wind angle +-180° on boat axis
                instrument = new DashboardInstrument_TrueWindAngle( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_TWA );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionMainValue( _T("%.0f"),
                        DIAL_POSITION_BOTTOMLEFT );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionExtraValue(
                        OCPN_DBP_STC_TWS, _T("%.1f"), DIAL_POSITION_INSIDE );
                break;
			case ID_DBP_D_AWA_TWA: //App/True Wind angle +-180° on boat axis
				instrument = new DashboardInstrument_AppTrueWindAngle(this, wxID_ANY,
					getInstrumentCaption(id), OCPN_DBP_STC_AWA | OCPN_DBP_STC_TWA);
				((DashboardInstrument_Dial *)instrument)->SetOptionMainValue(_T("%.0f"),
					DIAL_POSITION_NONE);
				((DashboardInstrument_Dial *)instrument)->SetOptionExtraValue(
					OCPN_DBP_STC_TWS | OCPN_DBP_STC_AWS, _T("%.1f"), DIAL_POSITION_NONE);
				break;
            case ID_DBP_D_TWD: //True Wind direction
                instrument = new DashboardInstrument_WindCompass( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_TWD );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionMainValue( _T("%.0f"),
                        DIAL_POSITION_BOTTOMLEFT );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionExtraValue(
                        OCPN_DBP_STC_TWS2, _T("%.1f"), DIAL_POSITION_INSIDE );
                break;
            case ID_DBP_I_DPT:
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_DPT, _T("%5.1f") );
                break;
            case ID_DBP_D_DPT:
                instrument = new DashboardInstrument_Depth( this, wxID_ANY,
                        getInstrumentCaption( id ) );
                break;
            case ID_DBP_I_TMP: //water temperature
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_TMP, _T("%2.1f") );
                break;
            case ID_DBP_I_MDA: //barometric pressure
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_MDA, _T("%5.3f") );
                break;
               case ID_DBP_D_MDA: //barometric pressure
                instrument = new DashboardInstrument_Speedometer( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_MDA, 940, 1040 );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionLabel( 10,
                        DIAL_LABEL_HORIZONTAL );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionMarker( 5,
                        DIAL_MARKER_SIMPLE, 1 );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionMainValue( _T("%5.3f"),
                        DIAL_POSITION_INSIDE );
                break;
            case ID_DBP_I_ATMP: //air temperature
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_ATMP, _T("%2.1f") );
                break;
            case ID_DBP_I_VLW1: // Trip Log
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_VLW1, _T("%2.1f") );
                break;

            case ID_DBP_I_VLW2: // Sum Log
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_VLW2, _T("%2.1f") );
                break;

            case ID_DBP_I_TWA: //true wind angle
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_TWA, _T("%5.0f") );
                break;
            case ID_DBP_I_TWD: //true wind direction
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_TWD, _T("%5.0f") );
                break;
            case ID_DBP_I_TWS: // true wind speed
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_TWS, _T("%2.2f") );
                break;
            case ID_DBP_I_AWA: //apparent wind angle
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_AWA, _T("%5.0f") );
                break;
            case ID_DBP_I_VMG:
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_VMG, _T("%5.2f") );
                break;
            case ID_DBP_D_VMG:
                instrument = new DashboardInstrument_Speedometer( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_VMG, 0, g_iDashSpeedMax );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionLabel( 1,
                        DIAL_LABEL_HORIZONTAL );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionMarker( 0.5,
                        DIAL_MARKER_SIMPLE, 2 );
                ( (DashboardInstrument_Dial *) instrument )->SetOptionExtraValue(
                        OCPN_DBP_STC_SOG, _T("SOG\n%.2f"), DIAL_POSITION_BOTTOMLEFT );
                break;
            case ID_DBP_I_RSA:
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_RSA, _T("%5.0f") );
                break;
            case ID_DBP_D_RSA:
                instrument = new DashboardInstrument_RudderAngle( this, wxID_ANY,
                        getInstrumentCaption( id ) );
                break;
            case ID_DBP_I_SAT:
                instrument = new DashboardInstrument_Single( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_SAT, _T("%5.0f") );
                break;
            case ID_DBP_D_GPS:
                instrument = new DashboardInstrument_GPS( this, wxID_ANY,
                        getInstrumentCaption( id ) );
                break;
            case ID_DBP_I_PTR:
                instrument = new DashboardInstrument_Position( this, wxID_ANY,
                        getInstrumentCaption( id ), OCPN_DBP_STC_PLA, OCPN_DBP_STC_PLO );
                break;
            case ID_DBP_I_CLK:
                instrument = new DashboardInstrument_Clock( this, wxID_ANY,
                        getInstrumentCaption( id ) );
                break;
            case ID_DBP_I_SUN:
                instrument = new DashboardInstrument_Sun( this, wxID_ANY,
                        getInstrumentCaption( id ) );
                break;
            case ID_DBP_D_MON:
                instrument = new DashboardInstrument_Moon( this, wxID_ANY,
                        getInstrumentCaption( id ) );
                break;
            case ID_DBP_D_WDH:
                instrument = new DashboardInstrument_WindDirHistory(this, wxID_ANY,
                        getInstrumentCaption( id ) );
                break;
            case ID_DBP_D_BPH:
                instrument = new DashboardInstrument_BaroHistory(this, wxID_ANY,
                        getInstrumentCaption( id ) );
                break;
            case ID_DBP_I_FOS:
                instrument = new DashboardInstrument_FromOwnship( this, wxID_ANY,
                        getInstrumentCaption( id ) );
                break;
			case ID_DBP_I_PITCH:
				instrument = new DashboardInstrument_Single(this, wxID_ANY,
					getInstrumentCaption(id), OCPN_DBP_STC_PITCH, _T("%2.1f"));
				break;
			case ID_DBP_I_HEEL:
				instrument = new DashboardInstrument_Single(this, wxID_ANY,
					getInstrumentCaption(id), OCPN_DBP_STC_HEEL, _T("%2.1f"));
		}
        if( instrument ) {
            instrument->instrumentTypeId = id;
            m_ArrayOfInstrument.Add(
                    new DashboardInstrumentContainer( id, instrument,
                            instrument->GetCapacity() ) );
            itemBoxSizer->Add( instrument, 0, wxEXPAND, 0 );
            if( itemBoxSizer->GetOrientation() == wxHORIZONTAL ) {
                itemBoxSizer->AddSpacer( 5 );
            }
        }
    }
    Fit();
    Layout();
    SetMinSize( itemBoxSizer->GetMinSize() );
}

void DashboardWindow::SendSentenceToAllInstruments( int st, double value, wxString unit )
{
    for( size_t i = 0; i < m_ArrayOfInstrument.GetCount(); i++ ) {
        if( m_ArrayOfInstrument.Item( i )->m_cap_flag & st ) m_ArrayOfInstrument.Item( i )->m_pInstrument->SetData(
                st, value, unit );
    }
}

void DashboardWindow::SendSatInfoToAllInstruments( int cnt, int seq, SAT_INFO sats[4] )
{
    for( size_t i = 0; i < m_ArrayOfInstrument.GetCount(); i++ ) {
        if( ( m_ArrayOfInstrument.Item( i )->m_cap_flag & OCPN_DBP_STC_GPS )
                && m_ArrayOfInstrument.Item( i )->m_pInstrument->IsKindOf(
                        CLASSINFO(DashboardInstrument_GPS)))
                        ((DashboardInstrument_GPS*)m_ArrayOfInstrument.Item(i)->m_pInstrument)->SetSatInfo(cnt, seq, sats);
                    }
                }

void DashboardWindow::SendUtcTimeToAllInstruments( wxDateTime value )
{
    for( size_t i = 0; i < m_ArrayOfInstrument.GetCount(); i++ ) {
        if( ( m_ArrayOfInstrument.Item( i )->m_cap_flag & OCPN_DBP_STC_CLK )
                && m_ArrayOfInstrument.Item( i )->m_pInstrument->IsKindOf( CLASSINFO( DashboardInstrument_Clock ) ) )
//                  || m_ArrayOfInstrument.Item( i )->m_pInstrument->IsKindOf( CLASSINFO( DashboardInstrument_Sun ) )
//                  || m_ArrayOfInstrument.Item( i )->m_pInstrument->IsKindOf( CLASSINFO( DashboardInstrument_Moon ) ) ) )
            ((DashboardInstrument_Clock*)m_ArrayOfInstrument.Item(i)->m_pInstrument)->SetUtcTime( value );
    }
}

