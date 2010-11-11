/******************************************************************************
 * $Id: dashboard_pi.cpp, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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
      ID_DBP_D_GPS
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

      m_pdashboard_window = NULL;

}

int dashboard_pi::Init(void)
{
      mVar = 0;
      mPriPosition = 99;
      mPriCOGSOG = 99;
      mPriHeading = 99;
      mPriVar = 99;
      mPriDateTime = 99;
      mPriApWind = 99;
      mPriDepth = 99;

      m_pauimgr = GetFrameAuiManager();

      //    Get a pointer to the opencpn configuration object
      m_pconfig = GetOCPNConfigObject();

      //    And load the configuration items
      LoadConfig();

      //    This PlugIn needs a toolbar icon
      m_toolbar_item_id  = InsertPlugInTool(_(""), _img_dashboard, _img_dashboard, wxITEM_CHECK,
            _("Dashboard"), _(""), NULL, DASHBOARD_TOOL_POSITION, 0, this);

      m_pdashboard_window = new DashboardWindow(GetOCPNCanvasWindow(), wxID_ANY, m_pauimgr, m_toolbar_item_id);
      m_pauimgr->AddPane(m_pdashboard_window, wxAuiPaneInfo().Name(_T("Dashboard")).Caption(_("Dashboard")).CaptionVisible(true).Float().FloatingPosition(0,0).TopDockable(false).BottomDockable(false).Show(false));
      m_pauimgr->Update();
      ApplyConfig();
      
      return (
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
      if(m_pdashboard_window)
      {
            m_pauimgr->DetachPane(m_pdashboard_window);
            m_pdashboard_window->Close();
            m_pdashboard_window->Destroy();
      }
      
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

void dashboard_pi::SetNMEASentence(wxString &sentence)
{
      if(m_pdashboard_window)
      {
            m_NMEA0183 << sentence;

            if(m_NMEA0183.PreParse())
            {
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
                                    m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_DPT, m_NMEA0183.Dbt.DepthMeters, _T("m"));
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
                                    m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_DPT, m_NMEA0183.Dpt.DepthMeters, _T("m"));
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
                                          m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, lat, _T("SDMM"));

                                          float lln = m_NMEA0183.Gga.Position.Longitude.Longitude;
                                          int lon_deg_int = (int)(lln / 100);
                                          float lon_deg = lon_deg_int;
                                          float lon_min = lln - (lon_deg * 100);
                                          lon = lon_deg + (lon_min/60.);
                                          if(m_NMEA0183.Gga.Position.Longitude.Easting == West)
                                                lon = -lon;
                                          m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_LON, lon, _T("SDMM"));
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
                                          m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, lat, _T("SDMM"));

                                          float lln = m_NMEA0183.Gll.Position.Longitude.Longitude;
                                          int lon_deg_int = (int)(lln / 100);
                                          float lon_deg = lon_deg_int;
                                          float lon_min = lln - (lon_deg * 100);
                                          lon = lon_deg + (lon_min/60.);
                                          if(m_NMEA0183.Gll.Position.Longitude.Easting == West)
                                                lon = -lon;
                                          m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_LON, lon, _T("SDMM"));
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
                              m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_SAT, m_NMEA0183.Gsv.SatsInView, _T(""));
                              m_pdashboard_window->SendSatInfoToAllInstruments(m_NMEA0183.Gsv.SatsInView, m_NMEA0183.Gsv.MessageNumber, m_NMEA0183.Gsv.SatInfo);
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
                              }
                              if (mPriHeading >= 3) {
                                    mPriHeading = 3;
                                    mHdm = m_NMEA0183.Hdg.MagneticSensorHeadingDegrees;
                                    m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_HDG, mHdm + mVar, _T("Deg"));
                              }
                        }
                  }

                  else if(m_NMEA0183.LastSentenceIDReceived == _T("HDM"))
                  {
                        if(m_NMEA0183.Parse())
                        {
                              if (mPriHeading >= 2) {
                                    mPriHeading = 2;
                                    mHdm = m_NMEA0183.Hdm.DegreesMagnetic;
                                    m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_HDG, mHdm + mVar, _T("Deg"));
                              }
                        }
                  }

                  else if(m_NMEA0183.LastSentenceIDReceived == _T("HDT"))
                  {
                        if(m_NMEA0183.Parse())
                        {
                              if (mPriHeading >= 1) {
                                    mPriHeading = 1;
                                    m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_HDG, m_NMEA0183.Hdt.DegreesTrue, _T("Deg"));
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
                              m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_TMP, m_NMEA0183.Mtw.Temperature, m_NMEA0183.Mtw.UnitOfMeasurement);
                        }
                  }
                  else if(m_NMEA0183.LastSentenceIDReceived == _T("MWV"))
                  {
                        if(m_NMEA0183.Parse())
                        {
                              if (mPriApWind >= 1)
                              {
                                    mPriApWind = 1;

                                    if (m_NMEA0183.Mwv.IsDataValid == NTrue)
                                    {
                                          //  1) Wind Angle, 0 to 360 degrees
                                          m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_AWA, m_NMEA0183.Mwv.WindAngle, _T("Deg"));
                                          //  3) Wind Speed
                                          m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_AWS, m_NMEA0183.Mwv.WindSpeed, m_NMEA0183.Mwv.WindSpeedUnits);
                                       /* TODO: handle Reference+Unit
                                        * //  2) Reference, R = Relative, T = True
                                        * wxString         m_NMEA0183.Mwv.Reference;
                                        * //  4) Wind Speed Units, K/M/N
                                        * wxString         m_NMEA0183.Mwv.WindSpeedUnits;
                                        */
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
                                          m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, lat, _T("SDMM"));

                                          float lln = m_NMEA0183.Rmc.Position.Longitude.Longitude;
                                          int lon_deg_int = (int)(lln / 100);
                                          float lon_deg = lon_deg_int;
                                          float lon_min = lln - (lon_deg * 100);
                                          lon = lon_deg + (lon_min/60.);
                                          if(m_NMEA0183.Rmc.Position.Longitude.Easting == West)
                                                lon = -lon;
                                          m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_LON, lon, _T("SDMM"));
                                    }

                                    if (mPriCOGSOG >= 3) {
                                          mPriCOGSOG = 3;
                                          if(m_NMEA0183.Rmc.SpeedOverGroundKnots < 999.)
                                          {
                                                m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_SOG, m_NMEA0183.Rmc.SpeedOverGroundKnots, _T("Kts"));
                                          }
                                          else
                                          {
                                                //->SetData(_T("---"));
                                          }
                                          if(m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue < 999.)
                                          {
                                                m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_COG, m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue, _T("Deg"));
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
                                    m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_RSA, m_NMEA0183.Rsa.Starboard, _T("Deg"));
                              }
                              else if (m_NMEA0183.Rsa.IsPortDataValid == NTrue)
                              {
                                    m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_RSA, -m_NMEA0183.Rsa.Port, _T("Deg"));
                              }
                        }
                  }

                  else if(m_NMEA0183.LastSentenceIDReceived == _T("VHW"))
                  {
                        if(m_NMEA0183.Parse())
                        {
                              /* Water heading!
                               *   m_NMEA0183.Vhw.DegreesTrue;
                               *   m_NMEA0183.Vhw.DegreesMagnetic;
                               */
                              if(m_NMEA0183.Vhw.Knots < 999.)
                              {
                                    m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_STW, m_NMEA0183.Vhw.Knots, _T("Kts"));
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
                                          m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_SOG, m_NMEA0183.Vtg.SpeedKnots, _T("Kts"));
                                    }
                                    else
                                    {
                                          //->SetData(_T("---"));
                                    }
                                    // Vtg.SpeedKilometersPerHour;
                                    if(m_NMEA0183.Vtg.TrackDegreesTrue < 999.)
                                    {
                                          m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_COG, m_NMEA0183.Vtg.TrackDegreesTrue, _T("Deg"));
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
                              if (mPriApWind >= 2)
                              {
                                    mPriApWind = 2;

                                    m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_AWA, m_NMEA0183.Vwr.DirectionOfWind==Left ? 360-m_NMEA0183.Vwr.WindDirectionMagnitude : m_NMEA0183.Vwr.WindDirectionMagnitude, _T("Deg"));
                                    m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_AWS, m_NMEA0183.Vwr.WindSpeedKnots, _T("Kts"));
                                    /*
                                    double           m_NMEA0183.Vwr.WindSpeedms;
                                    double           m_NMEA0183.Vwr.WindSpeedKmh;
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
}

void dashboard_pi::SetPositionFix(PlugIn_Position_Fix &pfix)
{
      if(m_pdashboard_window)
      {
            if (mPriPosition >= 1)
            {
                  mPriPosition = 1;
                  m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, pfix.Lat, _T("SDMM"));
                  m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_LON, pfix.Lon, _T("SDMM"));
            }
            if (mPriCOGSOG >= 1)
            {
                  mPriCOGSOG = 1;
                  m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_SOG, pfix.Sog, _T("Kts"));
                  m_pdashboard_window->SendSentenceToAllInstruments(OCPN_DBP_STC_COG, pfix.Cog, _T("Deg"));
            }
            if (mPriVar >= 1)
            {
                  mPriVar = 1;
                  mVar = pfix.Var;
            }
            if (mPriDateTime >= 1)
            {
                  mPriDateTime = 1;
                  mUTCDateTime.Set(pfix.FixTime);
            }
            mSatsInView = pfix.nSats;
      }
}

int dashboard_pi::GetToolbarToolCount(void)
{
      return 1;
}

void dashboard_pi::ShowPreferencesDialog( wxWindow* parent )
{
      wxDialog *dialog = new wxDialog( parent, wxID_ANY, _("Dashboard preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE );
      //      Build Dashboard Page for Toolbox
      int border_size = 4;

      wxBoxSizer* itemBoxSizerMainPanel = new wxBoxSizer(wxVERTICAL);
      dialog->SetSizer(itemBoxSizerMainPanel);

      //  Grib toolbox icon checkbox
      wxStaticBox* itemStaticBox01 = new wxStaticBox( dialog, wxID_ANY, _("General") );
      wxStaticBoxSizer* itemStaticBoxSizer01 = new wxStaticBoxSizer(itemStaticBox01, wxVERTICAL);
      itemBoxSizerMainPanel->Add(itemStaticBoxSizer01, 0, wxEXPAND|wxALL, border_size);

      wxStaticBox* itemStaticBox02 = new wxStaticBox( dialog, wxID_ANY, _("Dashboard") );
      wxStaticBoxSizer* itemStaticBoxSizer02 = new wxStaticBoxSizer(itemStaticBox02, wxHORIZONTAL);
      itemBoxSizerMainPanel->Add(itemStaticBoxSizer02, 0, wxEXPAND|wxALL, border_size);
      wxFlexGridSizer *itemFlexGridSizer = new wxFlexGridSizer(2);
      itemFlexGridSizer->AddGrowableCol(1);
      itemStaticBoxSizer02->Add(itemFlexGridSizer, 1, wxEXPAND|wxALL, 0);

      wxStaticText* itemStaticText02 = new wxStaticText( dialog, wxID_ANY, _("Instrument width:"), wxDefaultPosition, wxDefaultSize, 0 );
      itemFlexGridSizer->Add(itemStaticText02, 0, wxEXPAND|wxALL, border_size);
      m_pInstrumentWidth = new wxTextCtrl( dialog, wxID_ANY, wxString::Format(_T("%d"), m_iInstrumentWidth), wxDefaultPosition, wxDefaultSize );
      itemFlexGridSizer->Add(m_pInstrumentWidth, 0, wxALIGN_RIGHT|wxALL, border_size);
      // velocity range
      // rudder range

      wxBoxSizer* itemBoxSizer03 = new wxBoxSizer(wxHORIZONTAL);
      itemBoxSizerMainPanel->Add(itemBoxSizer03, 0, wxEXPAND|wxALL, 0);

      wxImageList *imglist = new wxImageList(20, 20, true, 2);
      imglist->Add(*_img_instrument);
      imglist->Add(*_img_dial);

      wxStaticBox* itemStaticBox03 = new wxStaticBox( dialog, wxID_ANY, _("Instruments") );
      wxStaticBoxSizer* itemStaticBoxSizer03 = new wxStaticBoxSizer(itemStaticBox03, wxHORIZONTAL);
      itemBoxSizer03->Add(itemStaticBoxSizer03, 1, wxEXPAND|wxALL, border_size);

      m_pListCtrlInstruments = new wxListCtrl( dialog, wxID_ANY, wxDefaultPosition, wxSize(-1, 200), wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL );
      itemStaticBoxSizer03->Add(m_pListCtrlInstruments, 1, wxEXPAND|wxALL, border_size);
      m_pListCtrlInstruments->AssignImageList(imglist, wxIMAGE_LIST_SMALL);
      m_pListCtrlInstruments->InsertColumn(0, _("Instruments"));
      m_pListCtrlInstruments->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
            wxListEventHandler(dashboard_pi::OnInstrumentSelected), NULL, this);
      m_pListCtrlInstruments->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED,
            wxListEventHandler(dashboard_pi::OnInstrumentSelected), NULL, this);

      wxBoxSizer* itemBoxSizer04 = new wxBoxSizer(wxVERTICAL);
      itemStaticBoxSizer03->Add(itemBoxSizer04, 0, wxALIGN_TOP|wxALL, border_size);
      m_pButtonAdd = new wxButton( dialog, wxID_ANY, _("Add"), wxDefaultPosition, wxSize(20, -1) );
      itemBoxSizer04->Add(m_pButtonAdd, 0, wxEXPAND|wxALL, border_size);
      m_pButtonAdd->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(dashboard_pi::OnInstrumentAdd), NULL, this);
      m_pButtonEdit = new wxButton( dialog, wxID_ANY, _("Edit"), wxDefaultPosition, wxDefaultSize );
      itemBoxSizer04->Add(m_pButtonEdit, 0, wxEXPAND|wxALL, border_size);
      m_pButtonEdit->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(dashboard_pi::OnInstrumentEdit), NULL, this);
      m_pButtonDelete = new wxButton( dialog, wxID_ANY, _("Delete"), wxDefaultPosition, wxSize(20, -1) );
      itemBoxSizer04->Add(m_pButtonDelete, 0, wxEXPAND|wxALL, border_size);
      m_pButtonDelete->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(dashboard_pi::OnInstrumentDelete), NULL, this);
      itemBoxSizer04->AddSpacer(10);
      m_pButtonUp = new wxButton( dialog, wxID_ANY, _("Up"), wxDefaultPosition, wxDefaultSize );
      itemBoxSizer04->Add(m_pButtonUp, 0, wxEXPAND|wxALL, border_size);
      m_pButtonUp->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(dashboard_pi::OnInstrumentUp), NULL, this);
      m_pButtonDown = new wxButton( dialog, wxID_ANY, _("Down"), wxDefaultPosition, wxDefaultSize );
      itemBoxSizer04->Add(m_pButtonDown, 0, wxEXPAND|wxALL, border_size);
      m_pButtonDown->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(dashboard_pi::OnInstrumentDown), NULL, this);
      wxStdDialogButtonSizer* DialogButtonSizer = dialog->CreateStdDialogButtonSizer(wxOK|wxCANCEL);
      itemBoxSizerMainPanel->Add(DialogButtonSizer, 0, wxALIGN_RIGHT|wxALL, 5);

      for (size_t i = 0; i < m_aInstrumentList.GetCount(); i++)
      {
            wxListItem item;
            getListItemForInstrument(item, m_aInstrumentList.Item(i));
            item.SetId(m_pListCtrlInstruments->GetItemCount());
            m_pListCtrlInstruments->InsertItem(item);
      }

      m_pListCtrlInstruments->SetColumnWidth(0, wxLIST_AUTOSIZE);

      UpdateButtonsState();
      dialog->SetMinSize(wxSize(350, -1));
      dialog->Fit();

      if(dialog->ShowModal() == wxID_OK)
      {
            m_iInstrumentWidth = wxAtoi(m_pInstrumentWidth->GetValue());
            m_iInstrumentCount = m_pListCtrlInstruments->GetItemCount();
            m_aInstrumentList.Clear();
            for (int i = 0; i < m_pListCtrlInstruments->GetItemCount(); i++)
                  m_aInstrumentList.Add((int)m_pListCtrlInstruments->GetItemData(i));

            //RemovePlugInTool(m_leftclick_tool_id);

            SaveConfig();
            ApplyConfig();
      }
}
void dashboard_pi::SetColorScheme(PI_ColorScheme cs)
{
      if(m_pdashboard_window)
            m_pdashboard_window->SetColorScheme(cs);
}

void dashboard_pi::OnInstrumentSelected(wxListEvent& event)
{
      UpdateButtonsState();
}

void dashboard_pi::UpdateButtonsState()
{
      long item = -1;
      item = m_pListCtrlInstruments->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      bool enable = (item != -1);

      m_pButtonDelete->Enable(enable);
      m_pButtonEdit->Enable(false); // TODO: Properties
      m_pButtonUp->Enable(item > 0);
      m_pButtonDown->Enable(item != -1 && item < m_pListCtrlInstruments->GetItemCount()-1);
}

void dashboard_pi::OnInstrumentAdd(wxCommandEvent& event)
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

void dashboard_pi::OnInstrumentDelete(wxCommandEvent& event)
{
      long itemID = -1;
      itemID = m_pListCtrlInstruments->GetNextItem(itemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

      m_pListCtrlInstruments->DeleteItem(itemID);
      UpdateButtonsState();
}

void dashboard_pi::OnInstrumentEdit(wxCommandEvent& event)
{
// TODO: Instument options
}

void dashboard_pi::OnInstrumentUp(wxCommandEvent& event)
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

void dashboard_pi::OnInstrumentDown(wxCommandEvent& event)
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

void dashboard_pi::OnToolbarToolCallback(int id)
{
      if(NULL == m_pdashboard_window)
            return;

      wxAuiPaneInfo &pane = m_pauimgr->GetPane(m_pdashboard_window);
      if(!pane.IsOk())
            return;

      pane.Show(!pane.IsShown());
      // Toggle is handled by the toolbar but we must keep plugin manager b_toggle updated
      // to actual status to ensure right status upon toolbar rebuild
      SetToolbarItemState(m_toolbar_item_id, pane.IsShown());
      m_pauimgr->Update();
}

void dashboard_pi::UpdateAuiStatus(void)
{
      //    This method is called after the PlugIn is initialized
      //    and the frame has done its initial layout, possibly from a saved wxAuiManager "Perspective"
      //    It is a chance for the PlugIn to syncronize itself internally with the state of any Panes that
      //    were added to the frame in the PlugIn ctor.

      //    We use this callback here to keep the context menu selection in sync with the window state

  
      wxAuiPaneInfo &pane = m_pauimgr->GetPane(m_pdashboard_window);
      if(!pane.IsOk())
            return;

      SetToolbarItemState(m_toolbar_item_id, pane.IsShown());
}

bool dashboard_pi::LoadConfig(void)
{
      wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

      if(pConf)
      {
            pConf->SetPath( _T("/PlugIns/Dashboard") );
            pConf->Read( _T("InstrumentWidth"), &m_iInstrumentWidth, 150 );
            int cnt;
            pConf->Read( _T("InstrumentCount"), &cnt, -1 );
            m_aInstrumentList.Clear();
            if (cnt == -1)
            {
                  // This is the default instrument list
                  m_aInstrumentList.Add(ID_DBP_I_POS);
                  m_aInstrumentList.Add(ID_DBP_D_COG);
                  m_aInstrumentList.Add(ID_DBP_D_GPS);
                  m_iInstrumentCount = m_aInstrumentList.GetCount();
            }
            else
            {
                  m_iInstrumentCount = cnt;
                  for (int i = 0; i < cnt; i++)
                  {
                        int id;
                        pConf->Read( wxString::Format(_T("Instrument%d"), i+1), &id, -1 );
                        if (id == -1)
                              m_iInstrumentCount--;
                        else
                              m_aInstrumentList.Add(id);
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
            pConf->SetPath( _T( "/PlugIns/Dashboard" ) );
            pConf->Write( _T( "InstrumentWidth" ), m_iInstrumentWidth );
            pConf->Write( _T( "InstrumentCount" ), m_iInstrumentCount );
            //pConf->Write( _T( "InstrumentCount" ), m_aInstrumentList.GetCount() );
            for (size_t i = 0; i < m_aInstrumentList.GetCount(); i++)
                  pConf->Write( wxString::Format(_T("Instrument%d"), i+1), m_aInstrumentList.Item(i));

            return true;
      }
      else
            return false;
}

void dashboard_pi::ApplyConfig(void)
{
      if(m_pdashboard_window)
      {
            m_pdashboard_window->SetInstrumentList(m_aInstrumentList);
            m_pdashboard_window->SetInstrumentWidth(m_iInstrumentWidth);
      }
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

      for (unsigned int i = ID_DBP_I_POS; i <= ID_DBP_D_GPS; i++)
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
DashboardWindow::DashboardWindow(wxWindow *pparent, wxWindowID id, wxAuiManager *auimgr, int tbitem)
      :wxWindow(pparent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, _T("Dashboard"))
{
      m_pauimgr = auimgr;
      m_toolbar_item_id = tbitem;

      wxColour cl;
      GetGlobalColor(_T("DILG1"), &cl);
      SetBackgroundColour(cl);

//wx2.9      itemBoxSizer = new wxWrapSizer(wxVERTICAL);
      itemBoxSizer = new wxBoxSizer(wxVERTICAL);
      SetSizer(itemBoxSizer);

      Connect(this->GetId(), wxEVT_SIZE, wxSizeEventHandler(DashboardWindow::OnSize));
      Connect(this->GetId(), wxEVT_SHOW, wxShowEventHandler(DashboardWindow::OnShow));
}

void DashboardWindow::SetColorScheme(PI_ColorScheme cs)
{
      wxColour cl;
      GetGlobalColor(_T("DILG1"), &cl);
      SetBackgroundColour(cl);

      Refresh(false);
}

void DashboardWindow::OnSize(wxSizeEvent& evt)
{

/* Disabled until I get Horizontal docking working right
 *
      wxAuiPaneInfo &pi = m_pauimgr->GetPane(this);

      if (pi.IsDocked())
      {
            if ((pi.dock_direction == wxAUI_DOCK_TOP || pi.dock_direction == wxAUI_DOCK_BOTTOM) && itemBoxSizer->GetOrientation() != wxHORIZONTAL)
            {
                  itemBoxSizer->SetOrientation(wxHORIZONTAL);
                  itemBoxSizer->Layout();
                  //itemBoxSizer->Fit(this);
                  //pi.BestSize(GetMinSize());
            }
            else if ((pi.dock_direction == wxAUI_DOCK_LEFT || pi.dock_direction == wxAUI_DOCK_RIGHT) && itemBoxSizer->GetOrientation() != wxVERTICAL)
            {
                  itemBoxSizer->SetOrientation(wxVERTICAL);
                  itemBoxSizer->Layout();
                  //itemBoxSizer->Fit(this);
                  //pi.BestSize(GetMinSize());
            }
      }
      else if (pi.IsFloating() && itemBoxSizer->GetOrientation() != wxVERTICAL)
      {
            itemBoxSizer->SetOrientation(wxVERTICAL);
            itemBoxSizer->Layout();
            //itemBoxSizer->Fit(this);
            //pi.BestSize(GetMinSize());
            //pi.FloatingSize(GetMinSize());
      }
 */
}

void DashboardWindow::OnShow(wxShowEvent& event)
{
      // This work if we have only one Dashboard. TODO: Find a workaround when we have several.
      SetToolbarItemState(m_toolbar_item_id, IsShown());
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
*/
      m_ArrayOfInstrument.Clear();
      itemBoxSizer->Clear(true);
      for (size_t i = 0; i < list.GetCount(); i++)
      {
            int id = list.Item(i);
            DashboardInstrument *instrument;
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
                  instrument = new DashboardInstrument_Single(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_HDG, _T("%5.0f Deg"));
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
                  // TODO: unchecked
                  instrument = new DashboardInstrument_Compass(this, wxID_ANY, getInstrumentCaption(id), OCPN_DBP_STC_TWA);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionMarker(5, DIAL_MARKER_REDGREEN, 2);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionLabel(30, DIAL_LABEL_ROTATED);
                  ((DashboardInstrument_Dial *)instrument)->SetOptionExtraValue(OCPN_DBP_STC_TWS, _T("TWS: %2.2f Kts"), DIAL_POSITION_TOPLEFT);
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
            }
            m_ArrayOfInstrument.Add(new DashboardInstrumentContainer(id, instrument, instrument->GetCapacity()));
            itemBoxSizer->Add(instrument, 0, wxALL, 0);
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
      wxAuiPaneInfo &pi = m_pauimgr->GetPane(this);
      pi.MinSize(wxSize(width, width));
      pi.BestSize(itemBoxSizer->GetMinSize());
      pi.FloatingSize(itemBoxSizer->GetMinSize());
      m_pauimgr->Update();
      //Layout();
//      SetMinSize(GetMinSize());
//      SetMaxSize(GetSize());
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

