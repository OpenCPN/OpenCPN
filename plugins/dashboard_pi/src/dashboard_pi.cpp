/******************************************************************************
 * $Id: dashboard_pi.cpp, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  DashBoard Plugin
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


#include "dashboard_pi.h"

#ifndef DECL_EXP
#ifdef __WXMSW__
#  define DECL_EXP     __declspec(dllexport)
#else
#  define DECL_EXP
#endif
#endif


// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new dashboard_pi(ppimgr);
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
      ID_DBWMENU_POSITION,
      ID_DBWMENU_SOG,
      ID_DBWMENU_SPEEDOMETER,
      ID_DBWMENU_COG,
      ID_DBWMENU_COMPASS,
      ID_DBWMENU_WIND,
      ID_DBWMENU_HDT,
      ID_DBWMENU_STW,
      ID_DBWMENU_DEPTH,
      ID_DBWMENU_RUDDERANGLE,
      ID_DBWMENU_GPS
};// DashboardWindow menu items;


//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

int dashboard_pi::Init(void)
{
//      printf("dashboard_pi Init()\n");

      m_pdashboard_window = NULL;

      // Get a pointer to the opencpn display canvas, to use as a parent for windows created
      m_parent_window = GetOCPNCanvasWindow();

      // Create the Context Menu Items

      //    In order to avoid an ASSERT on msw debug builds,
      //    we need to create a dummy menu to act as a surrogate parent of the created MenuItems
      //    The Items will be re-parented when added to the real context meenu
      wxMenu dummy_menu;

      wxMenuItem *pmi = new wxMenuItem(&dummy_menu, -1, _("Show PlugIn DashboardWindow"));
      m_show_id = AddCanvasContextMenuItem(pmi, this );
      SetCanvasContextMenuItemViz(m_show_id, true);

      wxMenuItem *pmih = new wxMenuItem(&dummy_menu, -1, _("Hide PlugIn DashboardWindow"));
      m_hide_id = AddCanvasContextMenuItem(pmih, this );
      SetCanvasContextMenuItemViz(m_hide_id, false);

      return (
           INSTALLS_CONTEXTMENU_ITEMS     |
           WANTS_NMEA_SENTENCES |
           WANTS_NMEA_EVENTS
            );
      /* read config
       * wxFileConfig * config = GetOCPNConfigObject();
                        wxString config_section = ( _T ( "/PlugIns/Dashboard" ) );
                        config_section += pic->m_common_name;
                        config->SetPath ( config_section );
                        config->Read ( _T ( "bEnabled" ), &pic->m_bEnabled );
       */
}

bool dashboard_pi::DeInit(void)
{
//      printf("dashboard_pi DeInit()\n");
      if(m_pdashboard_window)
      {
            m_pdashboard_window->Close();
            m_pdashboard_window->Destroy();
      }
      
      return true;
}

int dashboard_pi::GetAPIVersionMajor()
{
      return API_VERSION_MAJOR;
}

int dashboard_pi::GetAPIVersionMinor()
{
      return API_VERSION_MINOR;
}

int dashboard_pi::GetPlugInVersionMajor()
{
      return PLUGIN_VERSION_MAJOR;
}

int dashboard_pi::GetPlugInVersionMinor()
{
      return PLUGIN_VERSION_MINOR;
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
Provides navigation instrument display from NMEA source.\n");

}

void dashboard_pi::SetNMEASentence(wxString &sentence)
{
//      printf("dashboard_pi::SetNMEASentence\n");
      if(m_pdashboard_window)
      {
            m_pdashboard_window->SetSentence(sentence);
      }
}

void dashboard_pi::SetPositionFix(PlugIn_Position_Fix &pfix)
{
      if(m_pdashboard_window)
      {
            m_pdashboard_window->SetPosition(pfix);
      }
}

void dashboard_pi::OnContextMenuItemCallback(int id)
{
      wxLogMessage(_T("dashboard_pi OnContextMenuCallBack()"));
     ::wxBell();


      if(NULL == m_pdashboard_window)
      {
            m_pdashboard_window = new DashboardWindow(m_parent_window, wxID_ANY);
            m_pdashboard_window->Show();

            SetCanvasContextMenuItemViz(m_hide_id, true);
            SetCanvasContextMenuItemViz(m_show_id, false);
      }
      else
      {
            m_pdashboard_window->Close();
            m_pdashboard_window->Destroy();
            m_pdashboard_window = NULL;

            SetCanvasContextMenuItemViz(m_hide_id, false);
            SetCanvasContextMenuItemViz(m_show_id, true);
      }      

}


//----------------------------------------------------------------
//
//    Dashboard Window Implementation
//
//----------------------------------------------------------------

BEGIN_EVENT_TABLE(DashboardWindow, wxMiniFrame)
      EVT_SET_FOCUS( DashboardWindow::OnFocus )
      EVT_CONTEXT_MENU( DashboardWindow::OnContextMenu )
      EVT_MENU( ID_DBWMENU_POSITION, DashboardWindow::ToggleInstrumentVisibility )
      EVT_MENU( ID_DBWMENU_SOG, DashboardWindow::ToggleInstrumentVisibility )
      EVT_MENU( ID_DBWMENU_SPEEDOMETER, DashboardWindow::ToggleInstrumentVisibility )
      EVT_MENU( ID_DBWMENU_COG, DashboardWindow::ToggleInstrumentVisibility )
      EVT_MENU( ID_DBWMENU_COMPASS, DashboardWindow::ToggleInstrumentVisibility )
      EVT_MENU( ID_DBWMENU_WIND, DashboardWindow::ToggleInstrumentVisibility )
      EVT_MENU( ID_DBWMENU_HDT, DashboardWindow::ToggleInstrumentVisibility )
      EVT_MENU( ID_DBWMENU_STW, DashboardWindow::ToggleInstrumentVisibility )
      EVT_MENU( ID_DBWMENU_DEPTH, DashboardWindow::ToggleInstrumentVisibility )
      EVT_MENU( ID_DBWMENU_RUDDERANGLE, DashboardWindow::ToggleInstrumentVisibility )
      EVT_MENU( ID_DBWMENU_GPS, DashboardWindow::ToggleInstrumentVisibility )
END_EVENT_TABLE()

DashboardWindow::DashboardWindow(wxWindow *pparent, wxWindowID id)
      :wxMiniFrame(pparent, id, _T("DashBoard"), wxDefaultPosition, wxDefaultSize,
             wxCAPTION|wxRESIZE_BORDER|wxFRAME_FLOAT_ON_PARENT|wxFRAME_NO_TASKBAR)
{
      mVar = 0;
      mPriPosition = 99;
      mPriCOGSOG = 99;
      mPriHeading = 99;
      mPriVar = 99;
      mPriDateTime = 99;
      mPriApWind = 99;
      mPriDepth = 99;

      int w = 0; int h = 0;
      pparent->GetSize(&w, &h);
      int myW = 200; int myH = 300;
      SetSize(w-myW, 200, myW, myH);

      wxColour cl;
      GetGlobalColor(_T("UIBDR"), &cl);
      SetBackgroundColour(cl);

      itemBoxSizer = new wxBoxSizer(wxVERTICAL);
      SetSizer(itemBoxSizer);

      m_pDBIPosition = new DashboardInstrument_Double(this, id, _("Position"));
      itemBoxSizer->Add(m_pDBIPosition, 0, wxALL|wxEXPAND, 0);
      m_pDBISog = new DashboardInstrument_Single(this, id, _("SOG"));
      itemBoxSizer->Add(m_pDBISog, 0, wxALL|wxEXPAND, 0);
      m_pDBISpeedometer = new DashboardInstrument_Speedometer(this, id, _("Speedometer"), 0, 12);
      m_pDBISpeedometer->SetOptionLabel(1, DIAL_LABEL_HORIZONTAL);
      //m_pDBISpeedometer->SetOptionMarker(0.1, DIAL_MARKER_SIMPLE, 5);
      m_pDBISpeedometer->SetOptionMarker(0.5, DIAL_MARKER_SIMPLE, 2);
      m_pDBISpeedometer->SetOptionExtraValue(_T("STW: %2.2f Kts"), DIAL_POSITION_BOTTOMLEFT);
      itemBoxSizer->Add(m_pDBISpeedometer, 0, wxALL|wxEXPAND, 0);
      m_pDBICog = new DashboardInstrument_Single(this, id, _("COG"));
      itemBoxSizer->Add(m_pDBICog, 0, wxALL|wxEXPAND, 0);
      m_pDBICompass = new DashboardInstrument_Compass(this, id, _("Compass"));
      m_pDBICompass->SetOptionMarker(5, DIAL_MARKER_SIMPLE, 2);
      m_pDBICompass->SetOptionLabel(30, DIAL_LABEL_ROTATED);
      itemBoxSizer->Add(m_pDBICompass, 0, wxALL|wxEXPAND, 0);
      m_pDBIWind = new DashboardInstrument_Wind(this, id, _("Wind"));
      itemBoxSizer->Add(m_pDBIWind, 0, wxALL|wxEXPAND, 0);
      m_pDBIHdt = new DashboardInstrument_Single(this, id, _("Heading"));
      itemBoxSizer->Add(m_pDBIHdt, 0, wxALL|wxEXPAND, 0);
      m_pDBIStw = new DashboardInstrument_Single(this, id, _("Speed through water"));
      itemBoxSizer->Add(m_pDBIStw, 0, wxALL|wxEXPAND, 0);
      m_pDBIDepth = new DashboardInstrument_Single(this, id, _("Depth"));
      itemBoxSizer->Add(m_pDBIDepth, 0, wxALL|wxEXPAND, 0);
      m_pDBIRudderAngle = new DashboardInstrument_RudderAngle(this, id, _("Rudder angle"));
      itemBoxSizer->Add(m_pDBIRudderAngle, 0, wxALL|wxEXPAND, 0);
      m_pDBIGPS = new DashboardInstrument_GPS(this, id, _("GPS status"));
      itemBoxSizer->Add(m_pDBIGPS, 0, wxALL|wxEXPAND, 0);
      //m_pDBI->SetOptionMarker(5, DIAL_MARKER_REDGREEN, 2);

      itemBoxSizer->Hide(m_pDBISog);
      itemBoxSizer->Hide(m_pDBICog);
      itemBoxSizer->Hide(m_pDBIWind);
      itemBoxSizer->Hide(m_pDBIHdt);
      itemBoxSizer->Hide(m_pDBIStw);
      itemBoxSizer->Hide(m_pDBIDepth);
      itemBoxSizer->Hide(m_pDBIRudderAngle);
      //TODO: uncomment this//itemBoxSizer->Hide(m_pDBIGPS);
      itemBoxSizer->Fit(this);
}

DashboardWindow::~DashboardWindow()
{
}

void DashboardWindow::SetSentence(wxString &sentence)
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
                              m_pDBIDepth->SetData(wxString::Format(_("%5.1fm"), m_NMEA0183.Dbt.DepthMeters));
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
                              m_pDBIDepth->SetData(wxString::Format(_("%5.1fm"), m_NMEA0183.Dpt.DepthMeters));
                        }
                  }
            }

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

                                    float lln = m_NMEA0183.Gga.Position.Longitude.Longitude;
                                    int lon_deg_int = (int)(lln / 100);
                                    float lon_deg = lon_deg_int;
                                    float lon_min = lln - (lon_deg * 100);
                                    lon = lon_deg + (lon_min/60.);
                                    if(m_NMEA0183.Gga.Position.Longitude.Easting == West)
                                          lon = -lon;
                                    m_pDBIPosition->SetData(toSDMM(1, lat), toSDMM(2, lon));
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

                                    float lln = m_NMEA0183.Gll.Position.Longitude.Longitude;
                                    int lon_deg_int = (int)(lln / 100);
                                    float lon_deg = lon_deg_int;
                                    float lon_min = lln - (lon_deg * 100);
                                    lon = lon_deg + (lon_min/60.);
                                    if(m_NMEA0183.Gll.Position.Longitude.Easting == West)
                                          lon = -lon;
                                    m_pDBIPosition->SetData(toSDMM(1, lat), toSDMM(2, lon));
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
                        m_pDBIGPS->SetSatInfo(m_NMEA0183.Gsv.SatsInView, m_NMEA0183.Gsv.MessageNumber, m_NMEA0183.Gsv.SatInfo);
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
                              m_pDBIHdt->SetData(wxString::Format(_("%5.0f Deg"), mHdm + mVar));
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
                              m_pDBIHdt->SetData(wxString::Format(_("%5.0f Deg"), mHdm + mVar));
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("HDT"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if (mPriHeading >= 1) {
                              mPriHeading = 1;
                              m_pDBIHdt->SetData(wxString::Format(_("%5.0f Deg"), m_NMEA0183.Hdt.DegreesTrue));
                        }
                  }
            }

            else if(m_NMEA0183.LastSentenceIDReceived == _T("MTW"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        //WaterTemp=0

                        /*
                        double   m_NMEA0183.Mtw.Temperature;
                        wxString m_NMEA0183.Mtw.UnitOfMeasurement;
                        */
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
                                    m_pDBIWind->SetMainValue(m_NMEA0183.Mwv.WindAngle);
                                    //  3) Wind Speed
                                    m_pDBIWind->SetExtraValue(m_NMEA0183.Mwv.WindSpeed);
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

                                    float lln = m_NMEA0183.Rmc.Position.Longitude.Longitude;
                                    int lon_deg_int = (int)(lln / 100);
                                    float lon_deg = lon_deg_int;
                                    float lon_min = lln - (lon_deg * 100);
                                    lon = lon_deg + (lon_min/60.);
                                    if(m_NMEA0183.Rmc.Position.Longitude.Easting == West)
                                          lon = -lon;
                                    m_pDBIPosition->SetData(toSDMM(1, lat), toSDMM(2, lon));
                              }

                              if (mPriCOGSOG >= 3) {
                                    mPriCOGSOG = 3;
                                    if(m_NMEA0183.Rmc.SpeedOverGroundKnots < 999.)
                                    {
                                          m_pDBISog->SetData(wxString::Format(_("%5.2f Kts"), m_NMEA0183.Rmc.SpeedOverGroundKnots));
                                          m_pDBISpeedometer->SetMainValue(m_NMEA0183.Rmc.SpeedOverGroundKnots);
                                    }
                                    else
                                    {
                                          m_pDBISog->SetData(_T("---"));
                                    }
                                    if(m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue < 999.)
                                    {
                                          m_pDBICog->SetData(wxString::Format(_("%5.0f Deg"), m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue));
                                          m_pDBICompass->SetMainValue(m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue);
                                    }
                                    else
                                    {
                                          m_pDBICog->SetData(_T("---"));
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
                              m_pDBIRudderAngle->SetMainValue(m_NMEA0183.Rsa.Starboard);
                        }
                        else if (m_NMEA0183.Rsa.IsPortDataValid == NTrue)
                        {
                              m_pDBIRudderAngle->SetMainValue(- m_NMEA0183.Rsa.Port);
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
                              m_pDBIStw->SetData(wxString::Format(_("%5.2f Kts"), m_NMEA0183.Vhw.Knots));
                              m_pDBISpeedometer->SetExtraValue(m_NMEA0183.Vhw.Knots);
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
                                    m_pDBISog->SetData(wxString::Format(_("%5.2f Kts"), m_NMEA0183.Vtg.SpeedKnots));
                                    m_pDBISpeedometer->SetMainValue(m_NMEA0183.Vtg.SpeedKnots);
                              }
                              else
                              {
                                    m_pDBISog->SetData(_T("---"));
                              }
                              // Vtg.SpeedKilometersPerHour;
                              if(m_NMEA0183.Vtg.TrackDegreesTrue < 999.)
                              {
                                    m_pDBICog->SetData(wxString::Format(_("%5.0f Deg"), m_NMEA0183.Vtg.TrackDegreesTrue));
                                    m_pDBICompass->SetMainValue(m_NMEA0183.Vtg.TrackDegreesTrue);
                              }
                              else
                              {
                                    m_pDBICog->SetData(_T("---"));
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

                              m_pDBIWind->SetMainValue(m_NMEA0183.Vwr.DirectionOfWind==Left ? 360-m_NMEA0183.Vwr.WindDirectionMagnitude : m_NMEA0183.Vwr.WindDirectionMagnitude);
                              m_pDBIWind->SetExtraValue(m_NMEA0183.Vwr.WindSpeedKnots);
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
void DashboardWindow::SetPosition(PlugIn_Position_Fix &pfix)
{
      if (mPriPosition >= 1)
      {
            mPriPosition = 1;
            m_pDBIPosition->SetData(toSDMM(1, pfix.Lat), toSDMM(2, pfix.Lon));
      }
      if (mPriCOGSOG >= 1)
      {
            mPriCOGSOG = 1;
            m_pDBISog->SetData(wxString::Format(_("%5.2f Kts"), pfix.Sog));
            m_pDBISpeedometer->SetMainValue(pfix.Sog);
            m_pDBICog->SetData(wxString::Format(_("%5.0f Deg"), pfix.Cog));
            m_pDBICompass->SetMainValue(pfix.Cog);
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

void DashboardWindow::OnFocus(wxFocusEvent& event)
{
      wxWindow* parent = GetParent();
      parent->SetFocus();
      event.Skip();
}

void DashboardWindow::OnContextMenu(wxContextMenuEvent& event)
{
      wxMenu menu;
      wxMenuItem *item;
      item = menu.AppendCheckItem(ID_DBWMENU_POSITION, _T("Position"));
      item->Check(m_pDBIPosition->IsShown());
      item = menu.AppendCheckItem(ID_DBWMENU_SOG, _T("SOG"));
      item->Check(m_pDBISog->IsShown());
      item = menu.AppendCheckItem(ID_DBWMENU_SPEEDOMETER, _T("Speedometer"));
      item->Check(m_pDBISpeedometer->IsShown());
      item = menu.AppendCheckItem(ID_DBWMENU_COG, _T("COG"));
      item->Check(m_pDBICog->IsShown());
      item = menu.AppendCheckItem(ID_DBWMENU_COMPASS, _T("Compass"));
      item->Check(m_pDBICompass->IsShown());
      item = menu.AppendCheckItem(ID_DBWMENU_WIND, _T("Wind"));
      item->Check(m_pDBIWind->IsShown());
      item = menu.AppendCheckItem(ID_DBWMENU_HDT, _T("Heading"));
      item->Check(m_pDBIHdt->IsShown());
      item = menu.AppendCheckItem(ID_DBWMENU_STW, _T("Speed through water"));
      item->Check(m_pDBIStw->IsShown());
      item = menu.AppendCheckItem(ID_DBWMENU_DEPTH, _T("Depth"));
      item->Check(m_pDBIDepth->IsShown());
      item = menu.AppendCheckItem(ID_DBWMENU_RUDDERANGLE, _T("Rudder angle"));
      item->Check(m_pDBIRudderAngle->IsShown());
      item = menu.AppendCheckItem(ID_DBWMENU_GPS, _T("GPS status"));
      item->Check(m_pDBIGPS->IsShown());
      PopupMenu(&menu);
}

void DashboardWindow::ToggleInstrumentVisibility(wxCommandEvent& event)
{
      switch(event.GetId())
      {
      case ID_DBWMENU_POSITION:
            itemBoxSizer->Show(m_pDBIPosition, event.IsChecked());
            break;
      case ID_DBWMENU_SOG:
            itemBoxSizer->Show(m_pDBISog, event.IsChecked());
            break;
      case ID_DBWMENU_SPEEDOMETER:
            itemBoxSizer->Show(m_pDBISpeedometer, event.IsChecked());
            break;
      case ID_DBWMENU_COG:
            itemBoxSizer->Show(m_pDBICog, event.IsChecked());
            break;
      case ID_DBWMENU_COMPASS:
            itemBoxSizer->Show(m_pDBICompass, event.IsChecked());
            break;
      case ID_DBWMENU_WIND:
            itemBoxSizer->Show(m_pDBIWind, event.IsChecked());
            break;
      case ID_DBWMENU_HDT:
            itemBoxSizer->Show(m_pDBIHdt, event.IsChecked());
            break;
      case ID_DBWMENU_STW:
            itemBoxSizer->Show(m_pDBIStw, event.IsChecked());
            break;
      case ID_DBWMENU_DEPTH:
            itemBoxSizer->Show(m_pDBIDepth, event.IsChecked());
            break;
      case ID_DBWMENU_RUDDERANGLE:
            itemBoxSizer->Show(m_pDBIRudderAngle, event.IsChecked());
            break;
      case ID_DBWMENU_GPS:
            itemBoxSizer->Show(m_pDBIGPS, event.IsChecked());
            break;
      }
      itemBoxSizer->Layout();
      itemBoxSizer->Fit(this);
}

/**************************************************************************/
/*          Some assorted utilities                                       */
/**************************************************************************/

wxString toSDMM ( int NEflag, double a )
{
      short neg = 0;
      int d;
      long m;

      if ( a < 0.0 )
      {
            a = -a;
            neg = 1;
      }
      d = ( int ) a;
      m = ( long ) ( ( a - ( double ) d ) * 60000.0 );

      if ( neg )
            d = -d;

      wxString s;

      if ( !NEflag )
            s.Printf ( _T ( "%d %02ld.%03ld'" ), d, m / 1000, m % 1000 );
      else
      {
            if ( NEflag == 1 )
            {
                  char c = 'N';

                  if ( neg )
                  {
                        d = -d;
                        c = 'S';
                  }

                  s.Printf ( _T ( "%03d %02ld.%03ld %c" ), d, m / 1000, ( m % 1000 ), c );
            }
            else if ( NEflag == 2 )
            {
                  char c = 'E';

                  if ( neg )
                  {
                        d = -d;
                        c = 'W';
                  }
                  s.Printf ( _T ( "%03d %02ld.%03ld %c" ), d, m / 1000, ( m % 1000 ), c );
            }
      }
      return s;
}

