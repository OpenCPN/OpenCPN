/******************************************************************************
 * $Id: dashboard_pi.cpp, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <typeinfo>
#include "dashboard_pi.h"
#include "icons.h"

wxFont *g_pFontTitle;
wxFont *g_pFontData;
wxFont *g_pFontLabel;
wxFont *g_pFontSmall;

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return (opencpn_plugin *)new dashboard_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}




//---------------------------------------------------------------------------------------------------------
//
//    Dashboard PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

enum
{
      ID_DBP_I_POS,
      ID_DBP_I_SOG,
      ID_DBP_D_SOG,
      ID_DBP_I_COG,
      ID_DBP_D_COG,
      ID_DBP_I_STW,
      ID_DBP_I_HDG,
      ID_DBP_D_AW,
      ID_DBP_D_AWA,
      ID_DBP_I_AWS,
      ID_DBP_D_AWS,
      ID_DBP_D_TW,
      ID_DBP_I_DPT,
      ID_DBP_D_DPT,
      ID_DBP_I_TMP,
      ID_DBP_I_VMG,
      ID_DBP_D_VMG,
      ID_DBP_I_RSA,
      ID_DBP_D_RSA,
      ID_DBP_I_SAT,
      ID_DBP_D_GPS,
      ID_DBP_I_PTR,
      ID_DBP_I_CLK,
      ID_DBP_I_SUN,
      ID_DBP_I_MON
};

wxString getInstrumentCaption(unsigned int id)
{
      switch (id)
      {
      case ID_DBP_I_POS:
            return _("Position");
      case ID_DBP_I_SOG:
            return _("SOG");
      case ID_DBP_D_SOG:
            return _("Speedometer");
      case ID_DBP_I_COG:
            return _("COG");
      case ID_DBP_D_COG:
            return _("Compass");
      case ID_DBP_I_STW:
            return _("STW");
      case ID_DBP_I_HDG:
            return _("HDG");
      case ID_DBP_D_AW:
            return _("Apparent wind");
      case ID_DBP_D_AWA:
            return _("Wind angle");
      case ID_DBP_I_AWS:
            return _("Wind speed");
      case ID_DBP_D_AWS:
            return _("Wind speed");
      case ID_DBP_D_TW:
            return _("True wind");
      case ID_DBP_I_DPT:
            return _("Depth");
      case ID_DBP_D_DPT:
            return _("Depth");
      case ID_DBP_I_TMP:
            return _("Temp");
      case ID_DBP_I_VMG:
            return _("VMG");
      case ID_DBP_D_VMG:
            return _("VMG");
      case ID_DBP_I_RSA:
            return _("Rudder angle");
      case ID_DBP_D_RSA:
            return _("Rudder angle");
      case ID_DBP_I_SAT:
            return _("GPS in view");
      case ID_DBP_D_GPS:
            return _("GPS status");
      case ID_DBP_I_PTR:
            return _("Cursor");
      case ID_DBP_I_CLK:
            return _("Clock");
      case ID_DBP_I_SUN:
            return _("Sunrise/Sunset");
      case ID_DBP_I_MON:
            return _("Moon phase");
      }
      return _T("");
}

void getListItemForInstrument(wxListItem &item, unsigned int id)
{
      item.SetData(id);
      item.SetText(getInstrumentCaption(id));
      switch (id)
      {
      case ID_DBP_I_POS:
      case ID_DBP_I_SOG:
      case ID_DBP_I_COG:
      case ID_DBP_I_STW:
      case ID_DBP_I_HDG:
      case ID_DBP_I_AWS:
      case ID_DBP_I_DPT:
      case ID_DBP_I_TMP:
      case ID_DBP_I_VMG:
      case ID_DBP_I_RSA:
      case ID_DBP_I_SAT:
      case ID_DBP_I_PTR:
      case ID_DBP_I_CLK:
      case ID_DBP_I_SUN:
      case ID_DBP_I_MON:
            item.SetImage(0);
            break;
      case ID_DBP_D_SOG:
      case ID_DBP_D_COG:
      case ID_DBP_D_AW:
      case ID_DBP_D_AWA:
      case ID_DBP_D_AWS:
      case ID_DBP_D_TW:
      case ID_DBP_D_DPT:
      case ID_DBP_D_VMG:
      case ID_DBP_D_RSA:
      case ID_DBP_D_GPS:
            item.SetImage(1);
            break;
      }
}

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

dashboard_pi::dashboard_pi(void *ppimgr)
      :opencpn_plugin(ppimgr)
{
      // Create the PlugIn icons
      initialize_images();

}

int dashboard_pi::Init(void)
{
      AddLocaleCatalog( _T("opencpn-dashboard_pi") );

      mVar = 0;
      mPriPosition = 99;
      mPriCOGSOG = 99;
      mPriHeadingT = 99; // True heading
      mPriHeadingM = 99; // Magnetic heading
      mPriVar = 99;
      mPriDateTime = 99;
      mPriWindR = 99; // Relative wind
      mPriWindT = 99; // True wind
      mPriDepth = 99;

      g_pFontTitle = new wxFont( 10, wxFONTFAMILY_ROMAN, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL );
      g_pFontData = new wxFont( 14, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
      g_pFontLabel = new wxFont( 9, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
      g_pFontSmall = new wxFont( 8, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );

      m_pauimgr = GetFrameAuiManager();
      m_pauimgr->Connect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( dashboard_pi::OnPaneClose ), NULL, this );

      //    Get a pointer to the opencpn configuration object
      m_pconfig = GetOCPNConfigObject();

      //    And load the configuration items
      LoadConfig();

      //    This PlugIn needs a toolbar icon
      m_toolbar_item_id  = InsertPlugInTool(_T(""), _img_dashboard, _img_dashboard, wxITEM_CHECK,
            _("Dashboard"), _T(""), NULL, DASHBOARD_TOOL_POSITION, 0, this);

      ApplyConfig();
      
      return (
           WANTS_CURSOR_LATLON       |
           WANTS_TOOLBAR_CALLBACK    |
           INSTALLS_TOOLBAR_TOOL     |
           WANTS_PREFERENCES         |
           WANTS_CONFIG              |
           WANTS_NMEA_SENTENCES      |
           WANTS_NMEA_EVENTS         |
           USES_AUI_MANAGER
            );
}

bool dashboard_pi::DeInit(void)
{
      for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++)
      {
            DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
            if ( dashboard_window )
            {
                  m_pauimgr->DetachPane(dashboard_window);
                  dashboard_window->Close();
                  dashboard_window->Destroy();
                  m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow = NULL;
            }
      }

      delete g_pFontTitle;
      delete g_pFontData;
      delete g_pFontLabel;
      delete g_pFontSmall;
      
      return true;
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

void dashboard_pi::SendSentenceToAllInstruments(int st, double value, wxString unit)
{
      for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++)
      {
            DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
            if ( dashboard_window )
                  dashboard_window->SendSentenceToAllInstruments( st, value, unit );
      }
}

void dashboard_pi::SendUtcTimeToAllInstruments(int st, wxDateTime value)
{
      for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++)
      {
            DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
            if ( dashboard_window )
                  dashboard_window->SendUtcTimeToAllInstruments( st, value );
      }
}


void dashboard_pi::SendSatInfoToAllInstruments(int cnt, int seq, SAT_INFO sats[4])
{
      for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++)
      {
            DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
            if ( dashboard_window )
                  dashboard_window->SendSatInfoToAllInstruments( cnt, seq, sats );
      }
}

void dashboard_pi::SetNMEASentence(wxString &sentence)
{
      m_NMEA0183 << sentence;

      if(m_NMEA0183.PreParse())
      {
            SendUtcTimeToAllInstruments(OCPN_DBP_STC_CLK | OCPN_DBP_STC_MON, mUTCDateTime);
            if(m_NMEA0183.LastSentenceIDReceived == _T("DBT"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (mPriDepth >= 1)
                        {
                              mPriDepth = 1;

                              /*
                              double m_NMEA0183.Dbt.DepthFeet;
                              double m_NMEA0183.Dbt.DepthMeters;
                              double m_NMEA0183.Dbt.DepthFathoms;
                              */
                              SendSentenceToAllInstruments(OCPN_DBP_STC_DPT, m_NMEA0183.Dbt.DepthMeters, _T("m"));
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("DPT"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (mPriDepth >= 2)
                        {
                              mPriDepth = 2;

                              /*
                              double m_NMEA0183.Dpt.DepthMeters
                              double m_NMEA0183.Dpt.OffsetFromTransducerMeters
                              */
                              SendSentenceToAllInstruments(OCPN_DBP_STC_DPT, m_NMEA0183.Dpt.DepthMeters, _T("m"));
                        }
                  }
            }
// TODO: GBS - GPS Satellite fault detection
            else if(m_NMEA0183.LastSentenceIDReceived == _T("GGA"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if(m_NMEA0183.Gga.GPSQuality > 0)
                        {
                              if (mPriPosition >= 3) {
                                    mPriPosition = 3;
                                    double lat, lon;
                                    float llt = m_NMEA0183.Gga.Position.Latitude.Latitude;
                                    int lat_deg_int = (int)(llt / 100);
                                    float lat_deg = lat_deg_int;
                                    float lat_min = llt - (lat_deg * 100);
                                    lat = lat_deg + (lat_min/60.);
                                    if(m_NMEA0183.Gga.Position.Latitude.Northing == South)
                                          lat = -lat;
                                    SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, lat, _T("SDMM"));

                                    float lln = m_NMEA0183.Gga.Position.Longitude.Longitude;
                                    int lon_deg_int = (int)(lln / 100);
                                    float lon_deg = lon_deg_int;
                                    float lon_min = lln - (lon_deg * 100);
                                    lon = lon_deg + (lon_min/60.);
                                    if(m_NMEA0183.Gga.Position.Longitude.Easting == West)
                                          lon = -lon;
                                    SendSentenceToAllInstruments(OCPN_DBP_STC_LON, lon, _T("SDMM"));
                              }

                              if (mPriDateTime >= 4) {
                                    mPriDateTime = 4;
                                    mUTCDateTime.ParseFormat(m_NMEA0183.Gga.UTCTime.c_str(), _T("%H%M%S"));
                              }

                              mSatsInView = m_NMEA0183.Gga.NumberOfSatellitesInUse;
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("GLL"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if(m_NMEA0183.Gll.IsDataValid == NTrue)
                        {
                              if (mPriPosition >= 2) {
                                    mPriPosition = 2;
                                    double lat, lon;
                                    float llt = m_NMEA0183.Gll.Position.Latitude.Latitude;
                                    int lat_deg_int = (int)(llt / 100);
                                    float lat_deg = lat_deg_int;
                                    float lat_min = llt - (lat_deg * 100);
                                    lat = lat_deg + (lat_min/60.);
                                    if(m_NMEA0183.Gll.Position.Latitude.Northing == South)
                                          lat = -lat;
                                    SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, lat, _T("SDMM"));

                                    float lln = m_NMEA0183.Gll.Position.Longitude.Longitude;
                                    int lon_deg_int = (int)(lln / 100);
                                    float lon_deg = lon_deg_int;
                                    float lon_min = lln - (lon_deg * 100);
                                    lon = lon_deg + (lon_min/60.);
                                    if(m_NMEA0183.Gll.Position.Longitude.Easting == West)
                                          lon = -lon;
                                    SendSentenceToAllInstruments(OCPN_DBP_STC_LON, lon, _T("SDMM"));
                              }

                              if (mPriDateTime >= 5)
                              {
                                    mPriDateTime = 5;
                                    mUTCDateTime.ParseFormat(m_NMEA0183.Gll.UTCTime.c_str(), _T("%H%M%S"));
                              }
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("GSV"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        mSatsInView = m_NMEA0183.Gsv.SatsInView;
                        // m_NMEA0183.Gsv.NumberOfMessages;
                        SendSentenceToAllInstruments(OCPN_DBP_STC_SAT, m_NMEA0183.Gsv.SatsInView, _T(""));
                        SendSatInfoToAllInstruments(m_NMEA0183.Gsv.SatsInView, m_NMEA0183.Gsv.MessageNumber, m_NMEA0183.Gsv.SatInfo);
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("HDG"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (mPriVar >= 2) {
                              mPriVar = 2;
                              if(m_NMEA0183.Hdg.MagneticVariationDirection == East)
                                    mVar = m_NMEA0183.Hdg.MagneticVariationDegrees;
                              else if(m_NMEA0183.Hdg.MagneticVariationDirection == West)
                                    mVar = -m_NMEA0183.Hdg.MagneticVariationDegrees;
                              SendSentenceToAllInstruments(OCPN_DBP_STC_HMV, mVar, _T("Deg"));
                        }
                        if (mPriHeadingM >= 1) {
                              mPriHeadingM = 1;
                              mHdm = m_NMEA0183.Hdg.MagneticSensorHeadingDegrees;
                              SendSentenceToAllInstruments(OCPN_DBP_STC_HDM, mHdm, _T("Deg"));
                              //SendSentenceToAllInstruments(OCPN_DBP_STC_HDT, mHdm + mVar, _T("Deg"));
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("HDM"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (mPriHeadingM >= 2) {
                              mPriHeadingM = 2;
                              mHdm = m_NMEA0183.Hdm.DegreesMagnetic;
                              SendSentenceToAllInstruments(OCPN_DBP_STC_HDM, mHdm, _T("Deg"));
                              //SendSentenceToAllInstruments(OCPN_DBP_STC_HDT, mHdm + mVar, _T("Deg"));
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("HDT"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (mPriHeadingT >= 1) {
                              mPriHeadingT = 1;
                              if (m_NMEA0183.Hdt.DegreesTrue < 999.)
                              {
                                    SendSentenceToAllInstruments(OCPN_DBP_STC_HDT, m_NMEA0183.Hdt.DegreesTrue, _T("Deg"));
                              }
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("MTW"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        /*
                        double   m_NMEA0183.Mtw.Temperature;
                        wxString m_NMEA0183.Mtw.UnitOfMeasurement;
                        */
                        SendSentenceToAllInstruments(OCPN_DBP_STC_TMP, m_NMEA0183.Mtw.Temperature, m_NMEA0183.Mtw.UnitOfMeasurement);
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("MWD"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (mPriWindT >= 3)
                        {
                              mPriWindT = 3;

                              // Option for True vs Magnetic
                              SendSentenceToAllInstruments(OCPN_DBP_STC_TWA, m_NMEA0183.Mwd.WindAngleTrue, _T("Deg"));
                              //m_NMEA0183.Mwd.WindAngleMagnetic
                              SendSentenceToAllInstruments(OCPN_DBP_STC_TWS, m_NMEA0183.Mwd.WindSpeedKnots, _("N"));
                              //m_NMEA0183.Mwd.WindSpeedms
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("MWV"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (m_NMEA0183.Mwv.IsDataValid == NTrue)
                        {
                              if (m_NMEA0183.Mwv.Reference == _T("R")) // Relative (apparent wind)
                              {
                                    if (mPriWindR >= 1)
                                    {
                                          mPriWindR = 1;

                                          SendSentenceToAllInstruments(OCPN_DBP_STC_AWA, m_NMEA0183.Mwv.WindAngle, _T("Deg"));
                                          //  4) Wind Speed Units, K/M/N
                                          SendSentenceToAllInstruments(OCPN_DBP_STC_AWS, m_NMEA0183.Mwv.WindSpeed, m_NMEA0183.Mwv.WindSpeedUnits);
                                    }
                              }
                              else if (m_NMEA0183.Mwv.Reference == _T("T")) // Theoretical (aka True)
                              {
                                    if (mPriWindT >= 1)
                                    {
                                          mPriWindT = 1;

                                          SendSentenceToAllInstruments(OCPN_DBP_STC_TWA, m_NMEA0183.Mwv.WindAngle, _T("Deg"));
                                          //  4) Wind Speed Units, K/M/N
                                          SendSentenceToAllInstruments(OCPN_DBP_STC_TWS, m_NMEA0183.Mwv.WindSpeed, m_NMEA0183.Mwv.WindSpeedUnits);
                                    }
                              }
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("RMC"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if(m_NMEA0183.Rmc.IsDataValid == NTrue)
                        {
                              if (mPriPosition >= 4) {
                                    mPriPosition = 4;
                                    double lat, lon;
                                    float llt = m_NMEA0183.Rmc.Position.Latitude.Latitude;
                                    int lat_deg_int = (int)(llt / 100);
                                    float lat_deg = lat_deg_int;
                                    float lat_min = llt - (lat_deg * 100);
                                    lat = lat_deg + (lat_min/60.);
                                    if(m_NMEA0183.Rmc.Position.Latitude.Northing == South)
                                          lat = -lat;
                                    SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, lat, _T("SDMM"));

                                    float lln = m_NMEA0183.Rmc.Position.Longitude.Longitude;
                                    int lon_deg_int = (int)(lln / 100);
                                    float lon_deg = lon_deg_int;
                                    float lon_min = lln - (lon_deg * 100);
                                    lon = lon_deg + (lon_min/60.);
                                    if(m_NMEA0183.Rmc.Position.Longitude.Easting == West)
                                          lon = -lon;
                                    SendSentenceToAllInstruments(OCPN_DBP_STC_LON, lon, _T("SDMM"));
                              }

                              if (mPriCOGSOG >= 3) {
                                    mPriCOGSOG = 3;
                                    if(m_NMEA0183.Rmc.SpeedOverGroundKnots < 999.)
                                    {
                                          SendSentenceToAllInstruments(OCPN_DBP_STC_SOG, m_NMEA0183.Rmc.SpeedOverGroundKnots, _T("Kts"));
                                    }
                                    else
                                    {
                                          //->SetData(_T("---"));
                                    }
                                    if(m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue < 999.)
                                    {
                                          SendSentenceToAllInstruments(OCPN_DBP_STC_COG, m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue, _T("Deg"));
                                    }
                                    else
                                    {
                                          //->SetData(_T("---"));
                                    }
                              }

                              if (mPriVar >= 3) {
                                    mPriVar = 3;
                                    if(m_NMEA0183.Rmc.MagneticVariationDirection == East)
                                          mVar =  m_NMEA0183.Rmc.MagneticVariation;
                                    else if(m_NMEA0183.Rmc.MagneticVariationDirection == West)
                                          mVar = -m_NMEA0183.Rmc.MagneticVariation;
                                    SendSentenceToAllInstruments(OCPN_DBP_STC_HMV, mVar, _T("Deg"));
                              }

                              if (mPriDateTime >= 3)
                              {
                                    mPriDateTime = 3;
                                    wxString dt = m_NMEA0183.Rmc.UTCTime;
                                    dt.Append(m_NMEA0183.Rmc.Date);
                                    mUTCDateTime.ParseFormat(dt.c_str(), _T("%d%m%y%H%M%S"));
                              }
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("RSA"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (m_NMEA0183.Rsa.IsStarboardDataValid == NTrue)
                        {
                              SendSentenceToAllInstruments(OCPN_DBP_STC_RSA, m_NMEA0183.Rsa.Starboard, _T("Deg"));
                        }
                        else if (m_NMEA0183.Rsa.IsPortDataValid == NTrue)
                        {
                              SendSentenceToAllInstruments(OCPN_DBP_STC_RSA, -m_NMEA0183.Rsa.Port, _T("Deg"));
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("VHW"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (mPriHeadingT >= 2) {
                              mPriHeadingT = 2;
                              if (m_NMEA0183.Vhw.DegreesTrue < 999.)
                              {
                                    SendSentenceToAllInstruments(OCPN_DBP_STC_HDT, m_NMEA0183.Vhw.DegreesTrue, _T("Deg"));
                              }
                        }
                        if (mPriHeadingM >= 3) {
                              mPriHeadingM = 3;
                              SendSentenceToAllInstruments(OCPN_DBP_STC_HDM, m_NMEA0183.Vhw.DegreesMagnetic, _T("Deg"));
                        }
                        if(m_NMEA0183.Vhw.Knots < 999.)
                        {
                              SendSentenceToAllInstruments(OCPN_DBP_STC_STW, m_NMEA0183.Vhw.Knots, _T("Kts"));
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("VTG"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (mPriCOGSOG >= 2) {
                              mPriCOGSOG = 2;
                              //    Special check for unintialized values, as opposed to zero values
                              if(m_NMEA0183.Vtg.SpeedKnots < 999.)
                              {
                                    SendSentenceToAllInstruments(OCPN_DBP_STC_SOG, m_NMEA0183.Vtg.SpeedKnots, _T("Kts"));
                              }
                              else
                              {
                                    //->SetData(_T("---"));
                              }
                              // Vtg.SpeedKilometersPerHour;
                              if(m_NMEA0183.Vtg.TrackDegreesTrue < 999.)
                              {
                                    SendSentenceToAllInstruments(OCPN_DBP_STC_COG, m_NMEA0183.Vtg.TrackDegreesTrue, _T("Deg"));
                              }
                              else
                              {
                                    //->SetData(_T("---"));
                              }
                        }

                        /*
                        m_NMEA0183.Vtg.TrackDegreesMagnetic;
                        */
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("VWR"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (mPriWindR >= 2)
                        {
                              mPriWindR = 2;

                              SendSentenceToAllInstruments(OCPN_DBP_STC_AWA, m_NMEA0183.Vwr.DirectionOfWind==Left ? 360-m_NMEA0183.Vwr.WindDirectionMagnitude : m_NMEA0183.Vwr.WindDirectionMagnitude, _T("Deg"));
                              SendSentenceToAllInstruments(OCPN_DBP_STC_AWS, m_NMEA0183.Vwr.WindSpeedKnots, _T("Kts"));
                              /*
                              double           m_NMEA0183.Vwr.WindSpeedms;
                              double           m_NMEA0183.Vwr.WindSpeedKmh;
                              */
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("VWT"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (mPriWindT >= 2)
                        {
                              mPriWindT = 2;

/*
Calculated wind angle relative to the vessel, 0 to 180o, left/right L/R of vessel heading
*/
                              SendSentenceToAllInstruments(OCPN_DBP_STC_TWA, m_NMEA0183.Vwt.DirectionOfWind==Left ? 360-m_NMEA0183.Vwt.WindDirectionMagnitude : m_NMEA0183.Vwr.WindDirectionMagnitude, _T("Deg"));
                              SendSentenceToAllInstruments(OCPN_DBP_STC_TWS, m_NMEA0183.Vwt.WindSpeedKnots, _T("Kts"));
                              /*
                              double           m_NMEA0183.Vwt.WindSpeedms;
                              double           m_NMEA0183.Vwt.WindSpeedKmh;
                              */
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("ZDA"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (mPriDateTime >= 2)
                        {
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
                              dt.Printf(_T("%4d%02d%02d"), m_NMEA0183.Zda.Year, m_NMEA0183.Zda.Month, m_NMEA0183.Zda.Day);
                              dt.Append(m_NMEA0183.Zda.UTCTime);
                              mUTCDateTime.ParseFormat(dt.c_str(), _T("%Y%m%d%H%M%S"));
                        }
                  }
            }
      }
}

void dashboard_pi::SetPositionFix(PlugIn_Position_Fix &pfix)
{
      if (mPriPosition >= 1)
      {
            mPriPosition = 1;
            SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, pfix.Lat, _T("SDMM"));
            SendSentenceToAllInstruments(OCPN_DBP_STC_LON, pfix.Lon, _T("SDMM"));
      }
      if (mPriCOGSOG >= 1)
      {
            mPriCOGSOG = 1;
            SendSentenceToAllInstruments(OCPN_DBP_STC_SOG, pfix.Sog, _T("Kts"));
            SendSentenceToAllInstruments(OCPN_DBP_STC_COG, pfix.Cog, _T("Deg"));
      }
      if (mPriVar >= 1)
      {
            mPriVar = 1;
            mVar = pfix.Var;
            SendSentenceToAllInstruments(OCPN_DBP_STC_HMV, pfix.Var, _T("Deg"));
      }
      if (mPriDateTime >= 1)
      {
            mPriDateTime = 1;
            mUTCDateTime.Set(pfix.FixTime);
      }
      mSatsInView = pfix.nSats;
}

void dashboard_pi::SetCursorLatLon(double lat, double lon)
{
            SendSentenceToAllInstruments(OCPN_DBP_STC_PLA, lat, _T("SDMM"));
            SendSentenceToAllInstruments(OCPN_DBP_STC_PLO, lon, _T("SDMM"));
}

int dashboard_pi::GetToolbarToolCount(void)
{
      return 1;
}

void dashboard_pi::ShowPreferencesDialog( wxWindow* parent )
{
      for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++)
      {
            DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item(i);
            wxAuiPaneInfo &pane = m_pauimgr->GetPane( cont->m_pDashboardWindow );
            cont->m_bIsVisible = ( pane.IsOk() && pane.IsShown() );
      }

      DashboardPreferencesDialog *dialog = new DashboardPreferencesDialog( parent, wxID_ANY, m_ArrayOfDashboardWindow );

      if(dialog->ShowModal() == wxID_OK)
      {
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
            SetToolbarItemState( m_toolbar_item_id, GetDashboardWindowShownCount()!=0 );
      }
      dialog->Destroy();
}

void dashboard_pi::SetColorScheme(PI_ColorScheme cs)
{
      for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++)
      {
            DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
            if ( dashboard_window )
                  dashboard_window->SetColorScheme(cs);
      }
}

int dashboard_pi::GetDashboardWindowShownCount()
{
      int cnt = 0;

      for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++)
      {
            DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
            if ( dashboard_window )
            {
                  wxAuiPaneInfo &pane = m_pauimgr->GetPane( dashboard_window );
                  if(pane.IsOk() && pane.IsShown())
                        cnt++;
            }
      }
      return cnt;
}

void dashboard_pi::OnPaneClose( wxAuiManagerEvent& event )
{
      // if name is unique, we should use it
      DashboardWindow *dashboard_window = (DashboardWindow *)event.pane->window;
      int cnt = 0;
      for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++)
      {
            DashboardWindow *d_w = m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
            if ( d_w )
            {
                  // we must not count this one because it is being closed
                  if ( dashboard_window != d_w )
                  {
                        wxAuiPaneInfo &pane = m_pauimgr->GetPane( d_w );
                        if(pane.IsOk() && pane.IsShown())
                              cnt++;
                  }
            }
      }
      SetToolbarItemState( m_toolbar_item_id, cnt!=0 );

      event.Skip();
}

void dashboard_pi::OnToolbarToolCallback(int id)
{
      int cnt = GetDashboardWindowShownCount();

      for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++)
      {
            DashboardWindow *dashboard_window = m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
            if ( dashboard_window )
            {
                  wxAuiPaneInfo &pane = m_pauimgr->GetPane( dashboard_window );
                  if (pane.IsOk())
                        pane.Show(cnt==0);

            //  This patch fixes a bug in wxAUIManager
            //  FS#548
            // Dropping a DashBoard Window right on top on the (supposedly fixed) chart bar window
            // causes a resize of the chart bar, and the Dashboard window assumes some of its properties
            // The Dashboard window is no longer grabbable...
            // Workaround:  detect this case, and force the pane to be on a different Row.
            // so that the display is corrected by toggling the dashboard off and back on.
            if((pane.dock_direction == wxAUI_DOCK_BOTTOM) && pane.IsDocked())
                  pane.Row(2);
            }
      }
      // Toggle is handled by the toolbar but we must keep plugin manager b_toggle updated
      // to actual status to ensure right status upon toolbar rebuild
      SetToolbarItemState( m_toolbar_item_id, GetDashboardWindowShownCount()!= 0/*cnt==0*/ );
      m_pauimgr->Update();
}

void dashboard_pi::UpdateAuiStatus(void)
{
      //    This method is called after the PlugIn is initialized
      //    and the frame has done its initial layout, possibly from a saved wxAuiManager "Perspective"
      //    It is a chance for the PlugIn to syncronize itself internally with the state of any Panes that
      //    were added to the frame in the PlugIn ctor.

      //    We use this callback here to keep the context menu selection in sync with the window state

      SetToolbarItemState( m_toolbar_item_id, GetDashboardWindowShownCount()!= 0 );
}

bool dashboard_pi::LoadConfig(void)
{
      wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

      if(pConf)
      {
            pConf->SetPath( _T("/PlugIns/Dashboard") );

            wxString config;
            pConf->Read( _T("FontTitle"), &config, wxEmptyString );
            if ( !config.IsEmpty() )
                  g_pFontTitle->SetNativeFontInfo( config );
            pConf->Read( _T("FontData"), &config, wxEmptyString );
            if ( !config.IsEmpty() )
                  g_pFontData->SetNativeFontInfo( config );
            pConf->Read( _T("FontLabel"), &config, wxEmptyString );
            if ( !config.IsEmpty() )
                  g_pFontLabel->SetNativeFontInfo( config );
            pConf->Read( _T("FontSmall"), &config, wxEmptyString );
            if ( !config.IsEmpty() )
                  g_pFontSmall->SetNativeFontInfo( config );

            int d_cnt;
            pConf->Read( _T("DashboardCount"), &d_cnt, -1 );
            // TODO: Memory leak? We should destroy everything first
            m_ArrayOfDashboardWindow.Clear();
            if (d_cnt == -1)
            {
                  // No v1.1 config found. Let's load v1.0 or default settings.
                  int width;
                  pConf->Read( _T("InstrumentWidth"), &width, 150 );
                  int i_cnt;
                  pConf->Read( _T("InstrumentCount"), &i_cnt, -1 );
                  wxArrayInt ar;
                  if ( i_cnt != -1 )
                  {
                        for (int i = 0; i < i_cnt; i++)
                        {
                              int id;
                              pConf->Read( wxString::Format(_T("Instrument%d"), i+1), &id, -1 );
                              if (id != -1)
                                    ar.Add(id);
                        }
                  }
                  else
                  {
                        // This is the default instrument list
                        ar.Add(ID_DBP_I_POS);
                        ar.Add(ID_DBP_D_COG);
                        ar.Add(ID_DBP_D_GPS);
                  }

                  m_ArrayOfDashboardWindow.Add(new DashboardWindowContainer( NULL, _("Dashboard"), _T("V"), width, ar ));
            }
            else
            {
                  for (int i = 0; i < d_cnt; i++)
                  {
                        pConf->SetPath( wxString::Format( _T("/PlugIns/Dashboard/Dashboard%d"), i+1 ) );
                        wxString caption;
                        pConf->Read( _T("Caption"), &caption, _("Dashboard") );
                        wxString orient;
                        pConf->Read( _T("Orientation"), &orient, _T("V") );
                        int width;
                        pConf->Read( _T("InstrumentWidth"), &width, 150 );
                        int i_cnt;
                        pConf->Read( _T("InstrumentCount"), &i_cnt, -1 );
                        wxArrayInt ar;
                        for (int i = 0; i < i_cnt; i++)
                        {
                              int id;
                              pConf->Read( wxString::Format(_T("Instrument%d"), i+1), &id, -1 );
                              if (id != -1)
                                    ar.Add(id);
                        }
// TODO: Do not add if GetCount == 0
                        m_ArrayOfDashboardWindow.Add(new DashboardWindowContainer( NULL, caption, orient, width, ar ));
                  }
            }

            return true;
      }
      else
            return false;
}

bool dashboard_pi::SaveConfig(void)
{
      wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

      if(pConf)
      {
            pConf->SetPath( _T("/PlugIns/Dashboard") );
            pConf->Write( _T("FontTitle"), g_pFontTitle->GetNativeFontInfoDesc() );
            pConf->Write( _T("FontData"), g_pFontData->GetNativeFontInfoDesc() );
            pConf->Write( _T("FontLabel"), g_pFontLabel->GetNativeFontInfoDesc() );
            pConf->Write( _T("FontSmall"), g_pFontSmall->GetNativeFontInfoDesc() );

            pConf->Write( _T("DashboardCount" ), (int)m_ArrayOfDashboardWindow.GetCount() );
            for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++)
            {
                  DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item(i);
                  pConf->SetPath( wxString::Format( _T("/PlugIns/Dashboard/Dashboard%d"), i+1 ) );
                  pConf->Write( _T("Caption"), cont->m_sCaption );
                  pConf->Write( _T("Orientation"), cont->m_sOrientation );
                  pConf->Write( _T("InstrumentWidth"), cont->m_iInstrumentWidth );
                  pConf->Write( _T("InstrumentCount"), (int)cont->m_aInstrumentList.GetCount() );
                  for (size_t j = 0; j < cont->m_aInstrumentList.GetCount(); j++)
                        pConf->Write( wxString::Format(_T("Instrument%d"), j+1), cont->m_aInstrumentList.Item(j));
            }

            return true;
      }
      else
            return false;
}

void dashboard_pi::ApplyConfig(void)
{
      // Reverse order to handle deletes
      for (size_t i = m_ArrayOfDashboardWindow.GetCount(); i > 0; i--)
      {
            DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item(i-1);
            if ( cont->m_bIsDeleted )
            {
                  if ( cont->m_pDashboardWindow )
                  {
                        m_pauimgr->DetachPane( cont->m_pDashboardWindow );
                        cont->m_pDashboardWindow->Close();
                        cont->m_pDashboardWindow->Destroy();
                        cont->m_pDashboardWindow = NULL;
                  }
                  m_ArrayOfDashboardWindow.Remove( cont );
                  delete cont;
            }
            else
            {
                  if (! cont->m_pDashboardWindow )
                  {
                        cont->m_pDashboardWindow = new DashboardWindow(GetOCPNCanvasWindow(), wxID_ANY, m_pauimgr);
                        // Name contains Orientation for perspective
                        wxAuiPaneInfo pane = wxAuiPaneInfo().Name(wxString::Format(_T("Dashboard%d"), i-1)+cont->m_sOrientation).Caption(cont->m_sCaption).CaptionVisible(true).Float().FloatingPosition(10,100).Show(cont->m_bIsVisible);
                        if (cont->m_sOrientation == _T("V"))
                        {
                              pane.TopDockable(false).BottomDockable(false).LeftDockable(true).RightDockable(true);
                              cont->m_pDashboardWindow->SetSizerOrientation( wxVERTICAL );
                        }
                        else if (cont->m_sOrientation == _T("H"))
                        {
                              pane.TopDockable(true).BottomDockable(true).LeftDockable(false).RightDockable(false);
                              cont->m_pDashboardWindow->SetSizerOrientation( wxHORIZONTAL );
                        }
                        pane.Row(1);
                        m_pauimgr->AddPane( cont->m_pDashboardWindow, pane );
                  }
                  else
                  {
                        wxAuiPaneInfo &pane = m_pauimgr->GetPane( cont->m_pDashboardWindow );
                        pane.Name(wxString::Format(_T("Dashboard%d"), i-1)+cont->m_sOrientation).Caption(cont->m_sCaption).Show(cont->m_bIsVisible);
                        if (cont->m_sOrientation == _T("V"))
                        {
                              pane.TopDockable(false).BottomDockable(false).LeftDockable(true).RightDockable(true);
                              cont->m_pDashboardWindow->SetSizerOrientation( wxVERTICAL );
                        }
                        else if (cont->m_sOrientation == _T("H"))
                        {
                              pane.TopDockable(true).BottomDockable(true).LeftDockable(false).RightDockable(false);
                              cont->m_pDashboardWindow->SetSizerOrientation( wxHORIZONTAL );
                        }
                        pane.Row(1);

                  }
// TODO: orientation may have changed. then we've a problem with dock constraints (AUI bug?)
                  cont->m_pDashboardWindow->SetInstrumentList(cont->m_aInstrumentList);
                  cont->m_pDashboardWindow->SetInstrumentWidth(cont->m_iInstrumentWidth);
            }
      }
      m_pauimgr->Update();
}

/* DashboardPreferencesDialog
 *
 */

DashboardPreferencesDialog::DashboardPreferencesDialog( wxWindow *parent, wxWindowID id, wxArrayOfDashboard config )
      :wxDialog( parent, id, _("Dashboard preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE )
{
      Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DashboardPreferencesDialog::OnCloseDialog ), NULL, this );

      // Copy original config
      m_Config = wxArrayOfDashboard(config);
      //      Build Dashboard Page for Toolbox
      int border_size = 2;

      wxBoxSizer* itemBoxSizerMainPanel = new wxBoxSizer(wxVERTICAL);
      SetSizer(itemBoxSizerMainPanel);

      wxNotebook *itemNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP );
      itemBoxSizerMainPanel->Add( itemNotebook, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, border_size );

      wxPanel *itemPanelNotebook01 = new wxPanel( itemNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
      wxFlexGridSizer *itemFlexGridSizer01 = new wxFlexGridSizer(2);
      itemFlexGridSizer01->AddGrowableCol(1);
      itemPanelNotebook01->SetSizer( itemFlexGridSizer01 );
      itemNotebook->AddPage( itemPanelNotebook01, _("Dashboard") );

      wxBoxSizer *itemBoxSizer01 = new wxBoxSizer( wxVERTICAL );
      itemFlexGridSizer01->Add( itemBoxSizer01, 1, wxEXPAND|wxTOP|wxLEFT, border_size );

      wxImageList *imglist1 = new wxImageList(32, 32, true, 1);
      imglist1->Add(*_img_dashboard_pi);

      m_pListCtrlDashboards = new wxListCtrl( itemPanelNotebook01, wxID_ANY, wxDefaultPosition, wxSize(50, 200), wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL );
      m_pListCtrlDashboards->AssignImageList(imglist1, wxIMAGE_LIST_SMALL);
      m_pListCtrlDashboards->InsertColumn(0, _T(""));
      m_pListCtrlDashboards->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
            wxListEventHandler(DashboardPreferencesDialog::OnDashboardSelected), NULL, this);
      m_pListCtrlDashboards->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED,
            wxListEventHandler(DashboardPreferencesDialog::OnDashboardSelected), NULL, this);
      itemBoxSizer01->Add( m_pListCtrlDashboards, 1, wxEXPAND, 0 );

      wxBoxSizer *itemBoxSizer02 = new wxBoxSizer( wxHORIZONTAL );
      itemBoxSizer01->Add( itemBoxSizer02 );

      m_pButtonAddDashboard = new wxBitmapButton( itemPanelNotebook01, wxID_ANY, *_img_plus, wxDefaultPosition, wxDefaultSize );
      itemBoxSizer02->Add( m_pButtonAddDashboard, 0, wxALIGN_CENTER, 2 );
      m_pButtonAddDashboard->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnDashboardAdd), NULL, this);
      m_pButtonDeleteDashboard = new wxBitmapButton( itemPanelNotebook01, wxID_ANY, *_img_minus, wxDefaultPosition, wxDefaultSize );
      itemBoxSizer02->Add( m_pButtonDeleteDashboard, 0, wxALIGN_CENTER, 2 );
      m_pButtonDeleteDashboard->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnDashboardDelete), NULL, this);

      m_pPanelDashboard = new wxPanel( itemPanelNotebook01, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN );
      itemFlexGridSizer01->Add( m_pPanelDashboard, 1, wxEXPAND|wxTOP|wxRIGHT, border_size );

      wxBoxSizer* itemBoxSizer03 = new wxBoxSizer(wxVERTICAL);
      m_pPanelDashboard->SetSizer( itemBoxSizer03 );

      wxStaticBox* itemStaticBox02 = new wxStaticBox( m_pPanelDashboard, wxID_ANY, _("Dashboard") );
      wxStaticBoxSizer* itemStaticBoxSizer02 = new wxStaticBoxSizer(itemStaticBox02, wxHORIZONTAL);
      itemBoxSizer03->Add(itemStaticBoxSizer02, 0, wxEXPAND|wxALL, border_size);
      wxFlexGridSizer *itemFlexGridSizer = new wxFlexGridSizer(2);
      itemFlexGridSizer->AddGrowableCol(1);
      itemStaticBoxSizer02->Add(itemFlexGridSizer, 1, wxEXPAND|wxALL, 0);

      m_pCheckBoxIsVisible = new wxCheckBox( m_pPanelDashboard, wxID_ANY, _("show this dashboard"), wxDefaultPosition, wxDefaultSize, 0 );
      itemFlexGridSizer->Add( m_pCheckBoxIsVisible, 0, wxEXPAND|wxALL, border_size );
      wxStaticText *itemDummy01 = new wxStaticText( m_pPanelDashboard, wxID_ANY, _T(""));
      itemFlexGridSizer->Add( itemDummy01, 0, wxEXPAND|wxALL, border_size );

      wxStaticText* itemStaticText01 = new wxStaticText( m_pPanelDashboard, wxID_ANY, _("Caption:"), wxDefaultPosition, wxDefaultSize, 0 );
      itemFlexGridSizer->Add( itemStaticText01, 0, wxEXPAND|wxALL, border_size );
      m_pTextCtrlCaption = new wxTextCtrl( m_pPanelDashboard, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize );
      itemFlexGridSizer->Add( m_pTextCtrlCaption, 0, wxEXPAND|wxALL, border_size );

      wxStaticText* itemStaticText02 = new wxStaticText( m_pPanelDashboard, wxID_ANY, _("Orientation:"), wxDefaultPosition, wxDefaultSize, 0 );
      itemFlexGridSizer->Add( itemStaticText02, 0, wxEXPAND|wxALL, border_size );
      m_pChoiceOrientation = new wxChoice( m_pPanelDashboard, wxID_ANY, wxDefaultPosition, wxSize( 120, -1) );
      m_pChoiceOrientation->Append( _("Vertical") );
      m_pChoiceOrientation->Append( _("Horizontal") );
      itemFlexGridSizer->Add( m_pChoiceOrientation, 0, wxALIGN_RIGHT|wxALL, border_size );

      wxStaticText* itemStaticText03 = new wxStaticText( m_pPanelDashboard, wxID_ANY, _("Instrument width:"), wxDefaultPosition, wxDefaultSize, 0 );
      itemFlexGridSizer->Add(itemStaticText03, 0, wxEXPAND|wxALL, border_size);
      m_pInstrumentWidth = new wxSpinCtrl( m_pPanelDashboard, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50, -1 ), wxSP_ARROW_KEYS, 50, 400 );
      itemFlexGridSizer->Add(m_pInstrumentWidth, 0, wxALIGN_RIGHT|wxALL, border_size);

      wxImageList *imglist = new wxImageList(20, 20, true, 2);
      imglist->Add(*_img_instrument);
      imglist->Add(*_img_dial);

      wxStaticBox* itemStaticBox03 = new wxStaticBox( m_pPanelDashboard, wxID_ANY, _("Instruments") );
      wxStaticBoxSizer* itemStaticBoxSizer03 = new wxStaticBoxSizer(itemStaticBox03, wxHORIZONTAL);
      itemBoxSizer03->Add(itemStaticBoxSizer03, 1, wxEXPAND|wxALL, border_size);

      m_pListCtrlInstruments = new wxListCtrl( m_pPanelDashboard, wxID_ANY, wxDefaultPosition, wxSize(-1, 200), wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL );
      itemStaticBoxSizer03->Add(m_pListCtrlInstruments, 1, wxEXPAND|wxALL, border_size);
      m_pListCtrlInstruments->AssignImageList(imglist, wxIMAGE_LIST_SMALL);
      m_pListCtrlInstruments->InsertColumn(0, _("Instruments"));
      m_pListCtrlInstruments->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
            wxListEventHandler(DashboardPreferencesDialog::OnInstrumentSelected), NULL, this);
      m_pListCtrlInstruments->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED,
            wxListEventHandler(DashboardPreferencesDialog::OnInstrumentSelected), NULL, this);

      wxBoxSizer* itemBoxSizer04 = new wxBoxSizer(wxVERTICAL);
      itemStaticBoxSizer03->Add(itemBoxSizer04, 0, wxALIGN_TOP|wxALL, border_size);
      m_pButtonAdd = new wxButton( m_pPanelDashboard, wxID_ANY, _("Add"), wxDefaultPosition, wxSize(20, -1) );
      itemBoxSizer04->Add(m_pButtonAdd, 0, wxEXPAND|wxALL, border_size);
      m_pButtonAdd->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentAdd), NULL, this);
      m_pButtonEdit = new wxButton( m_pPanelDashboard, wxID_ANY, _("Edit"), wxDefaultPosition, wxDefaultSize );
      itemBoxSizer04->Add(m_pButtonEdit, 0, wxEXPAND|wxALL, border_size);
      m_pButtonEdit->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentEdit), NULL, this);
      m_pButtonDelete = new wxButton( m_pPanelDashboard, wxID_ANY, _("Delete"), wxDefaultPosition, wxSize(20, -1) );
      itemBoxSizer04->Add(m_pButtonDelete, 0, wxEXPAND|wxALL, border_size);
      m_pButtonDelete->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentDelete), NULL, this);
      itemBoxSizer04->AddSpacer(10);
      m_pButtonUp = new wxButton( m_pPanelDashboard, wxID_ANY, _("Up"), wxDefaultPosition, wxDefaultSize );
      itemBoxSizer04->Add(m_pButtonUp, 0, wxEXPAND|wxALL, border_size);
      m_pButtonUp->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentUp), NULL, this);
      m_pButtonDown = new wxButton( m_pPanelDashboard, wxID_ANY, _("Down"), wxDefaultPosition, wxDefaultSize );
      itemBoxSizer04->Add(m_pButtonDown, 0, wxEXPAND|wxALL, border_size);
      m_pButtonDown->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentDown), NULL, this);

      wxPanel *itemPanelNotebook02 = new wxPanel( itemNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
      wxBoxSizer* itemBoxSizer05 = new wxBoxSizer(wxVERTICAL);
      itemPanelNotebook02->SetSizer( itemBoxSizer05 );
      itemNotebook->AddPage( itemPanelNotebook02, _("Appearance") );

      wxStaticBox* itemStaticBox01 = new wxStaticBox( itemPanelNotebook02, wxID_ANY, _("Fonts") );
      wxStaticBoxSizer* itemStaticBoxSizer01 = new wxStaticBoxSizer(itemStaticBox01, wxHORIZONTAL);
      itemBoxSizer05->Add( itemStaticBoxSizer01, 0, wxEXPAND|wxALL, border_size );
      wxFlexGridSizer *itemFlexGridSizer03 = new wxFlexGridSizer(2);
      itemFlexGridSizer03->AddGrowableCol(1);
      itemStaticBoxSizer01->Add(itemFlexGridSizer03, 1, wxEXPAND|wxALL, 0);
      wxStaticText* itemStaticText04 = new wxStaticText( itemPanelNotebook02, wxID_ANY, _("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
      itemFlexGridSizer03->Add(itemStaticText04, 0, wxEXPAND|wxALL, border_size);
      m_pFontPickerTitle = new wxFontPickerCtrl( itemPanelNotebook02, wxID_ANY, *g_pFontTitle, wxDefaultPosition, wxDefaultSize );
      itemFlexGridSizer03->Add(m_pFontPickerTitle, 0, wxALIGN_RIGHT|wxALL, 0);
      wxStaticText* itemStaticText05 = new wxStaticText( itemPanelNotebook02, wxID_ANY, _("Data:"), wxDefaultPosition, wxDefaultSize, 0 );
      itemFlexGridSizer03->Add(itemStaticText05, 0, wxEXPAND|wxALL, border_size);
      m_pFontPickerData = new wxFontPickerCtrl( itemPanelNotebook02, wxID_ANY, *g_pFontData, wxDefaultPosition, wxDefaultSize );
      itemFlexGridSizer03->Add(m_pFontPickerData, 0, wxALIGN_RIGHT|wxALL, 0);
      wxStaticText* itemStaticText06 = new wxStaticText( itemPanelNotebook02, wxID_ANY, _("Label:"), wxDefaultPosition, wxDefaultSize, 0 );
      itemFlexGridSizer03->Add(itemStaticText06, 0, wxEXPAND|wxALL, border_size);
      m_pFontPickerLabel = new wxFontPickerCtrl( itemPanelNotebook02, wxID_ANY, *g_pFontLabel, wxDefaultPosition, wxDefaultSize );
      itemFlexGridSizer03->Add(m_pFontPickerLabel, 0, wxALIGN_RIGHT|wxALL, 0);
      wxStaticText* itemStaticText07 = new wxStaticText( itemPanelNotebook02, wxID_ANY, _("Small:"), wxDefaultPosition, wxDefaultSize, 0 );
      itemFlexGridSizer03->Add(itemStaticText07, 0, wxEXPAND|wxALL, border_size);
      m_pFontPickerSmall = new wxFontPickerCtrl( itemPanelNotebook02, wxID_ANY, *g_pFontSmall, wxDefaultPosition, wxDefaultSize );
      itemFlexGridSizer03->Add(m_pFontPickerSmall, 0, wxALIGN_RIGHT|wxALL, 0);
//      wxColourPickerCtrl

      wxStdDialogButtonSizer* DialogButtonSizer = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
      itemBoxSizerMainPanel->Add(DialogButtonSizer, 0, wxALIGN_RIGHT|wxALL, 5);

      curSel = -1;
      for (size_t i = 0; i < m_Config.GetCount(); i++)
      {
            m_pListCtrlDashboards->InsertItem( i, 0 );
            // Using data to store m_Config index for managing deletes
            m_pListCtrlDashboards->SetItemData( i, i );
      }
      m_pListCtrlDashboards->SetColumnWidth(0, wxLIST_AUTOSIZE);

      UpdateDashboardButtonsState();
      UpdateButtonsState();
      SetMinSize(wxSize(450, -1));
      Fit();
}

void DashboardPreferencesDialog::OnCloseDialog(wxCloseEvent& event)
{
      SaveDashboardConfig();
      event.Skip();
}

void DashboardPreferencesDialog::SaveDashboardConfig()
{
      if ( curSel != -1 )
      {
            DashboardWindowContainer *cont = m_Config.Item(curSel);
            cont->m_bIsVisible = m_pCheckBoxIsVisible->IsChecked();
            cont->m_sCaption = m_pTextCtrlCaption->GetValue();
            cont->m_sOrientation = m_pChoiceOrientation->GetSelection() == 0 ? _T("V") : _T("H");
            cont->m_iInstrumentWidth = m_pInstrumentWidth->GetValue();
            cont->m_aInstrumentList.Clear();
            for (int i = 0; i < m_pListCtrlInstruments->GetItemCount(); i++)
                  cont->m_aInstrumentList.Add((int)m_pListCtrlInstruments->GetItemData(i));
      }
}

void DashboardPreferencesDialog::OnDashboardSelected(wxListEvent& event)
{
      // save changes
      SaveDashboardConfig();
      UpdateDashboardButtonsState();
}

void DashboardPreferencesDialog::UpdateDashboardButtonsState()
{
      long item = -1;
      item = m_pListCtrlDashboards->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      bool enable = (item != -1);

      m_pButtonDeleteDashboard->Enable(enable);
      m_pPanelDashboard->Enable(enable);

      if ( item != -1 )
      {
            curSel = m_pListCtrlDashboards->GetItemData( item );
            DashboardWindowContainer *cont = m_Config.Item( curSel );
            m_pCheckBoxIsVisible->SetValue( cont->m_bIsVisible );
            m_pTextCtrlCaption->SetValue( cont->m_sCaption );
            m_pChoiceOrientation->SetSelection( cont->m_sOrientation == _T("V") ? 0 : 1 );
            m_pInstrumentWidth->SetValue( cont->m_iInstrumentWidth );
            m_pListCtrlInstruments->DeleteAllItems();
            for (size_t i = 0; i < cont->m_aInstrumentList.GetCount(); i++)
            {
                  wxListItem item;
                  getListItemForInstrument(item, cont->m_aInstrumentList.Item(i));
                  item.SetId(m_pListCtrlInstruments->GetItemCount());
                  m_pListCtrlInstruments->InsertItem(item);
            }

            m_pListCtrlInstruments->SetColumnWidth(0, wxLIST_AUTOSIZE);
      }
      else
      {
            curSel = -1;
            m_pCheckBoxIsVisible->SetValue( false );
            m_pTextCtrlCaption->SetValue( _T("") );
            m_pChoiceOrientation->SetSelection( 0 );
            m_pInstrumentWidth->SetValue( 150 );
            m_pListCtrlInstruments->DeleteAllItems();
      }
//      UpdateButtonsState();
}

void DashboardPreferencesDialog::OnDashboardAdd(wxCommandEvent& event)
{
      int idx = m_pListCtrlDashboards->GetItemCount();
      m_pListCtrlDashboards->InsertItem( idx, 0 );
      // Data is index in m_Config
      m_pListCtrlDashboards->SetItemData( idx, m_Config.GetCount() );
      wxArrayInt ar;
      m_Config.Add(new DashboardWindowContainer( NULL, _("Dashboard"), _T("V"), 150, ar ));
}

void DashboardPreferencesDialog::OnDashboardDelete(wxCommandEvent& event)
{
      long itemID = -1;
      itemID = m_pListCtrlDashboards->GetNextItem( itemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

      int idx = m_pListCtrlDashboards->GetItemData( itemID );
      m_pListCtrlDashboards->DeleteItem( itemID );
      m_Config.Item( idx )->m_bIsDeleted = true;
      UpdateDashboardButtonsState();
}

void DashboardPreferencesDialog::OnInstrumentSelected(wxListEvent& event)
{
      UpdateButtonsState();
}

void DashboardPreferencesDialog::UpdateButtonsState()
{
      long item = -1;
      item = m_pListCtrlInstruments->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      bool enable = (item != -1);

      m_pButtonDelete->Enable(enable);
      m_pButtonEdit->Enable(false); // TODO: Properties
      m_pButtonUp->Enable(item > 0);
      m_pButtonDown->Enable(item != -1 && item < m_pListCtrlInstruments->GetItemCount()-1);
}

void DashboardPreferencesDialog::OnInstrumentAdd(wxCommandEvent& event)
{
      AddInstrumentDlg pdlg((wxWindow *)event.GetEventObject(), wxID_ANY);

      if (pdlg.ShowModal() == wxID_OK)
      {
            wxListItem item;
            getListItemForInstrument(item, pdlg.GetInstrumentAdded());
            item.SetId(m_pListCtrlInstruments->GetItemCount());
            m_pListCtrlInstruments->InsertItem(item);
            m_pListCtrlInstruments->SetColumnWidth(0, wxLIST_AUTOSIZE);
            UpdateButtonsState();
      }
}

void DashboardPreferencesDialog::OnInstrumentDelete(wxCommandEvent& event)
{
      long itemID = -1;
      itemID = m_pListCtrlInstruments->GetNextItem(itemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

      m_pListCtrlInstruments->DeleteItem(itemID);
      UpdateButtonsState();
}

void DashboardPreferencesDialog::OnInstrumentEdit(wxCommandEvent& event)
{
// TODO: Instument options
}

void DashboardPreferencesDialog::OnInstrumentUp(wxCommandEvent& event)
{
      long itemID = -1;
      itemID = m_pListCtrlInstruments->GetNextItem(itemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

      wxListItem item;
      item.SetId(itemID);
      item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_IMAGE | wxLIST_MASK_DATA);
      m_pListCtrlInstruments->GetItem(item);
      item.SetId(itemID-1);
      m_pListCtrlInstruments->DeleteItem(itemID);
      m_pListCtrlInstruments->InsertItem(item);
      m_pListCtrlInstruments->SetItemState(itemID-1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      UpdateButtonsState();
}

void DashboardPreferencesDialog::OnInstrumentDown(wxCommandEvent& event)
{
      long itemID = -1;
      itemID = m_pListCtrlInstruments->GetNextItem(itemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

      wxListItem item;
      item.SetId(itemID);
      item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_IMAGE | wxLIST_MASK_DATA);
      m_pListCtrlInstruments->GetItem(item);
      item.SetId(itemID+1);
      m_pListCtrlInstruments->DeleteItem(itemID);
      m_pListCtrlInstruments->InsertItem(item);
      m_pListCtrlInstruments->SetItemState(itemID+1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      UpdateButtonsState();
}

//----------------------------------------------------------------
//
//    Add Instrument Dialog Implementation
//
//----------------------------------------------------------------

AddInstrumentDlg::AddInstrumentDlg(wxWindow *pparent, wxWindowID id)
      :wxDialog(pparent, id, _("Add instrument"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
      wxBoxSizer* itemBoxSizer01 = new wxBoxSizer(wxVERTICAL);
      SetSizer(itemBoxSizer01);
      wxStaticText* itemStaticText01 = new wxStaticText( this, wxID_ANY, _("Select instrument to add:"), wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizer01->Add(itemStaticText01, 0, wxEXPAND|wxALL, 5);

      wxImageList *imglist = new wxImageList(20, 20, true, 2);
      imglist->Add(*_img_instrument);
      imglist->Add(*_img_dial);

      m_pListCtrlInstruments = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxSize(250, 80), wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL );
      itemBoxSizer01->Add(m_pListCtrlInstruments, 0, wxEXPAND|wxALL, 5);
      m_pListCtrlInstruments->AssignImageList(imglist, wxIMAGE_LIST_SMALL);
      m_pListCtrlInstruments->InsertColumn(0, _("Instruments"));
      wxStdDialogButtonSizer* DialogButtonSizer = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
      itemBoxSizer01->Add(DialogButtonSizer, 0, wxALIGN_RIGHT|wxALL, 5);

      for (unsigned int i = ID_DBP_I_POS; i <= ID_DBP_I_MON; i++)
      {
            wxListItem item;
            getListItemForInstrument(item, i);
            item.SetId(i);
            m_pListCtrlInstruments->InsertItem(item);
      }

      m_pListCtrlInstruments->SetColumnWidth(0, wxLIST_AUTOSIZE);
      m_pListCtrlInstruments->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      Fit();
}

unsigned int AddInstrumentDlg::GetInstrumentAdded()
{
      long itemID = -1;
      itemID = m_pListCtrlInstruments->GetNextItem(itemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

      return (int)m_pListCtrlInstruments->GetItemData(itemID);
}

//----------------------------------------------------------------
//
//    Dashboard Window Implementation
//
//----------------------------------------------------------------

// wxWS_EX_VALIDATE_RECURSIVELY required to push events to parents
DashboardWindow::DashboardWindow(wxWindow *pparent, wxWindowID id, wxAuiManager *auimgr)
      :wxWindow(pparent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, _T("Dashboard"))
{
      m_pauimgr = auimgr;

      wxColour cl;
      GetGlobalColor(_T("DILG1"), &cl);
      SetBackgroundColour(cl);

//wx2.9      itemBoxSizer = new wxWrapSizer(wxVERTICAL);
      itemBoxSizer = new wxBoxSizer(wxVERTICAL);
      SetSizer(itemBoxSizer);
}

void DashboardWindow::SetColorScheme(PI_ColorScheme cs)
{
      wxColour cl;
      GetGlobalColor(_T("DILG1"), &cl);
      SetBackgroundColour(cl);

      Refresh(false);
}

void DashboardWindow::SetSizerOrientation( int orient )
{
      itemBoxSizer->SetOrientation( orient );
      //itemBoxSizer->Layout();
}

bool isArrayIntEqual(const wxArrayInt& l1, const wxArrayOfInstrument &l2)
{
      if (l1.GetCount() != l2.GetCount())
            return false;

      for (size_t i = 0; i < l1.GetCount(); i++)
            if (l1.Item(i) != l2.Item(i)->m_ID)
                  return false;
      
      return true;
}

void DashboardWindow::SetInstrumentList(wxArrayInt list)
{
      if (isArrayIntEqual(list, m_ArrayOfInstrument))
            return;

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
      itemBoxSizer->Clear(true);
      for (size_t i = 0; i < list.GetCount(); i++)
      {
            int id = list.Item(i);
            DashboardInstrument *instrument = NULL;
            switch (id)
            {
            case ID_DBP_I_POS:
                  instrument = new DashboardInstrument_Position(this, wxID_ANY, getInstrumentCaption(id));
                  break;
            case ID_DBP_I_SOG:
                  instrument = new DashboardInstrument_Single(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_SOG, _T("%5.2f Kts"));
                  break;
            case ID_DBP_D_SOG:
                  instrument = new DashboardInstrument_Speedometer(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_SOG, 0, 12);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionLabel(1, DIAL_LABEL_HORIZONTAL);
                  //(DashboardInstrument_Dial *)instrument->SetOptionMarker(0.1, DIAL_MARKER_SIMPLE, 5);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionMarker(0.5, DIAL_MARKER_SIMPLE, 2);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionExtraValue(OCPN_DBP_STC_STW, _T("STW: %2.2f Kts"), DIAL_POSITION_BOTTOMLEFT);
                  break;
            case ID_DBP_I_COG:
                  instrument = new DashboardInstrument_Single(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_COG, _T("%5.0f Deg"));
                  break;
            case ID_DBP_D_COG:
                  instrument = new DashboardInstrument_Compass(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_COG);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionMarker(5, DIAL_MARKER_SIMPLE, 2);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionLabel(30, DIAL_LABEL_ROTATED);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionExtraValue(OCPN_DBP_STC_SOG, _T("SOG: %2.2f Kts"), DIAL_POSITION_BOTTOMLEFT);
                  break;
            case ID_DBP_I_STW:
                  instrument = new DashboardInstrument_Single(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_STW, _T("%5.2f Kts"));
                  break;
            case ID_DBP_I_HDG:
                  // TODO: Option True or Magnetic
                  instrument = new DashboardInstrument_Single(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_HDT, _T("%5.0f Deg"));
                  break;
            case ID_DBP_D_AW:
                  instrument = new DashboardInstrument_Wind(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_AWA);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionExtraValue(OCPN_DBP_STC_AWS, _T("%5.2f Kts"), DIAL_POSITION_INSIDE);
                  break;
            case ID_DBP_D_AWA:
                  instrument = new DashboardInstrument_Wind(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_AWA);
                  break;
            case ID_DBP_I_AWS:
                  instrument = new DashboardInstrument_Single(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_AWS, _T("%5.2f Kts"));
                  break;
            case ID_DBP_D_AWS:
                  instrument = new DashboardInstrument_Speedometer(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_AWS, 0, 45);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionLabel(5, DIAL_LABEL_HORIZONTAL);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionMarker(1, DIAL_MARKER_SIMPLE, 5);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionMainValue(_T("A: %5.2f Kts"), DIAL_POSITION_BOTTOMLEFT);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionExtraValue(OCPN_DBP_STC_TWS, _T("T: %5.2f Kts"), DIAL_POSITION_BOTTOMRIGHT);
                  break;
            case ID_DBP_D_TW:
                  instrument = new DashboardInstrument_WindCompass( this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_TWA );
                  ((DashboardInstrument_Dial *)instrument)->SetOptionMainValue(_T("%5.0f Deg"), DIAL_POSITION_TOPRIGHT);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionExtraValue(OCPN_DBP_STC_TWS, _T("%2.2f Kts"), DIAL_POSITION_TOPLEFT);
                  break;
            case ID_DBP_I_DPT:
                  instrument = new DashboardInstrument_Single(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_DPT, _T("%5.1fm"));
                  break;
            case ID_DBP_D_DPT:
                  instrument = new DashboardInstrument_Depth(this, wxID_ANY, getInstrumentCaption(id));
                  break;
            case ID_DBP_I_TMP:
                  instrument = new DashboardInstrument_Single(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_TMP, _T("%2.1f°"));
                  break;
            case ID_DBP_I_VMG:
                  instrument = new DashboardInstrument_Single(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_VMG, _T("%5.2f Kts"));
                  break;
            case ID_DBP_D_VMG:
                  instrument = new DashboardInstrument_Speedometer(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_VMG, 0, 12);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionLabel(1, DIAL_LABEL_HORIZONTAL);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionMarker(0.5, DIAL_MARKER_SIMPLE, 2);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionExtraValue(OCPN_DBP_STC_SOG, _T("SOG: %2.2f Kts"), DIAL_POSITION_BOTTOMLEFT);
                  break;
            case ID_DBP_I_RSA:
                  instrument = new DashboardInstrument_Single(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_RSA, _T("%5.0f Deg"));
                  break;
            case ID_DBP_D_RSA:
                  instrument = new DashboardInstrument_RudderAngle(this, wxID_ANY, getInstrumentCaption(id));
                  break;
            case ID_DBP_I_SAT:
                  instrument = new DashboardInstrument_Single(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_SAT, _T("%5.0f"));
                  break;
            case ID_DBP_D_GPS:
                  instrument = new DashboardInstrument_GPS(this, wxID_ANY, getInstrumentCaption(id));
                  break;
            case ID_DBP_I_PTR:
                  instrument = new DashboardInstrument_Position(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_PLA, OCPN_DBP_STC_PLO);
                  break;
            case ID_DBP_I_CLK:
                  instrument = new DashboardInstrument_Clock(this, wxID_ANY, getInstrumentCaption(id));
                  break;
            case ID_DBP_I_SUN:
                  instrument = new DashboardInstrument_Sun(this, wxID_ANY, getInstrumentCaption(id));
                  break;
            case ID_DBP_I_MON:
                  instrument = new DashboardInstrument_Moon(this, wxID_ANY, getInstrumentCaption(id));
                  break;
            }
            if(instrument)
            {
                   m_ArrayOfInstrument.Add(new DashboardInstrumentContainer(id, instrument, instrument->GetCapacity()));
                   itemBoxSizer->Add(instrument, 0, wxALL, 0);
            }
      }
}

void DashboardWindow::SetInstrumentWidth(int width)
{
      for (size_t i = 0; i < m_ArrayOfInstrument.GetCount(); i++)
      {
            m_ArrayOfInstrument.Item(i)->m_pInstrument->SetInstrumentWidth(width);
      }

      itemBoxSizer->Layout();
      itemBoxSizer->Fit(this);
      SetMinSize( itemBoxSizer->GetMinSize() );
      wxAuiPaneInfo &pi = m_pauimgr->GetPane(this);
      pi.MinSize(GetMinSize());
      //pi.MinSize(wxSize(width, width));
      pi.BestSize(GetMinSize());
      //pi.MaxSize(GetMinSize());
      pi.FloatingSize(GetMinSize());
      m_pauimgr->Update();
}

void DashboardWindow::SendSentenceToAllInstruments(int st, double value, wxString unit)
{
      for (size_t i = 0; i < m_ArrayOfInstrument.GetCount(); i++)
      {
            if (m_ArrayOfInstrument.Item(i)->m_cap_flag & st)
                  m_ArrayOfInstrument.Item(i)->m_pInstrument->SetData(st, value, unit);
      }
}

void DashboardWindow::SendSatInfoToAllInstruments(int cnt, int seq, SAT_INFO sats[4])
{
      for (size_t i = 0; i < m_ArrayOfInstrument.GetCount(); i++)
      {
            if ((m_ArrayOfInstrument.Item(i)->m_cap_flag & OCPN_DBP_STC_GPS) &&
                        m_ArrayOfInstrument.Item(i)->m_pInstrument->IsKindOf(CLASSINFO(DashboardInstrument_GPS)))
                  ((DashboardInstrument_GPS*)m_ArrayOfInstrument.Item(i)->m_pInstrument)->SetSatInfo(cnt, seq, sats);
      }
}

void DashboardWindow::SendUtcTimeToAllInstruments(int st, wxDateTime value)
{
      for (size_t i = 0; i < m_ArrayOfInstrument.GetCount(); i++)
      {
            if ((m_ArrayOfInstrument.Item(i)->m_cap_flag & OCPN_DBP_STC_MON) &&
                        m_ArrayOfInstrument.Item(i)->m_pInstrument->IsKindOf(CLASSINFO(DashboardInstrument_Moon)))
                  ((DashboardInstrument_Moon*)m_ArrayOfInstrument.Item(i)->m_pInstrument)->SetUtcTime(st, value);
            if ((m_ArrayOfInstrument.Item(i)->m_cap_flag & OCPN_DBP_STC_CLK) &&
                        (m_ArrayOfInstrument.Item(i)->m_pInstrument->IsKindOf(CLASSINFO(DashboardInstrument_Clock)) || m_ArrayOfInstrument.Item(i)->m_pInstrument->IsKindOf(CLASSINFO(DashboardInstrument_Sun))))
                  ((DashboardInstrument_Clock*)m_ArrayOfInstrument.Item(i)->m_pInstrument)->SetUtcTime(st, value);
      }
}

