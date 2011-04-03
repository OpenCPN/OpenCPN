/******************************************************************************
 * $Id: logbook_pi.cpp, v1.0 2011/03/18 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Logbook Plugin
 * Author:   Jean-Eudes Onfray
 *
 ***************************************************************************
 *   Copyright (C) 2011 by Jean-Eudes Onfray   *
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
#include "logbook_pi.h"
#include "icons.h"

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return (opencpn_plugin *)new logbook_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}

//---------------------------------------------------------------------------------------------------------
//
//    Logbook PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

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


logbook_pi::logbook_pi(void *ppimgr)
      : opencpn_plugin(ppimgr), wxTimer(this)
{
      // Create the PlugIn icons
      initialize_images();
}

int logbook_pi::Init(void)
{
      mPriPosition = 99;
      mPriCOGSOG = 99;
      mPriHeadingT = 99; // True heading
      mPriHeadingM = 99; // Magnetic heading
      mPriVar = 99;
      mPriDateTime = 99;
      mPriWindR = 99; // Relative wind
      mPriWindT = 99; // True wind
      mPriDepth = 99;

      AddLocaleCatalog( _T("opencpn-logbook_pi") );

      //    Get a pointer to the opencpn configuration object
      m_pconfig = GetOCPNConfigObject();

      //    And load the configuration items
      LoadConfig();

      ApplyConfig();

      return (
           WANTS_PREFERENCES         |
           WANTS_CONFIG              |
           WANTS_NMEA_SENTENCES      |
           WANTS_NMEA_EVENTS
            );
}

bool logbook_pi::DeInit(void)
{
      Stop();
      return true;
}

int logbook_pi::GetAPIVersionMajor()
{
      return MY_API_VERSION_MAJOR;
}

int logbook_pi::GetAPIVersionMinor()
{
      return MY_API_VERSION_MINOR;
}

int logbook_pi::GetPlugInVersionMajor()
{
      return PLUGIN_VERSION_MAJOR;
}

int logbook_pi::GetPlugInVersionMinor()
{
      return PLUGIN_VERSION_MINOR;
}

wxBitmap *logbook_pi::GetPlugInBitmap()
{
      return _img_logbook_pi;
}

wxString logbook_pi::GetCommonName()
{
      return _("Logbook");
}


wxString logbook_pi::GetShortDescription()
{
      return _("Logbook plugin for OpenCPN");
}

wxString logbook_pi::GetLongDescription()
{
      return _("Log user defined navigation data at regular interval.");

}

void logbook_pi::SetNMEASentence(wxString &sentence)
{
      m_NMEA0183 << sentence;

      if (m_NMEA0183.PreParse())
      {
            if (m_NMEA0183.LastSentenceIDReceived == _T("DBT"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (mPriDepth >= 1)
                        {
                              mPriDepth = 1;

                              /*
                              double m_NMEA0183.Dbt.DepthFeet;
                              double m_NMEA0183.Dbt.DepthMeters;
                              double m_NMEA0183.Dbt.DepthFathoms;
                              */
                              mDepth.SetValue(m_NMEA0183.Dbt.DepthMeters);
                        }
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("DPT"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (mPriDepth >= 2)
                        {
                              mPriDepth = 2;

                              /*
                              double m_NMEA0183.Dpt.DepthMeters
                              double m_NMEA0183.Dpt.OffsetFromTransducerMeters
                              */
                              mDepth.SetValue(m_NMEA0183.Dpt.DepthMeters);
                        }
                  }
            }
// TODO: GBS - GPS Satellite fault detection
            else if (m_NMEA0183.LastSentenceIDReceived == _T("GGA"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (m_NMEA0183.Gga.GPSQuality > 0)
                        {
                              if (mPriPosition >= 3) {
                                    mPriPosition = 3;
                                    double lat, lon;
                                    float llt = m_NMEA0183.Gga.Position.Latitude.Latitude;
                                    int lat_deg_int = (int)(llt / 100);
                                    float lat_deg = lat_deg_int;
                                    float lat_min = llt - (lat_deg * 100);
                                    lat = lat_deg + (lat_min/60.);
                                    if (m_NMEA0183.Gga.Position.Latitude.Northing == South)
                                          lat = -lat;
                                    mLat.SetValue(lat);

                                    float lln = m_NMEA0183.Gga.Position.Longitude.Longitude;
                                    int lon_deg_int = (int)(lln / 100);
                                    float lon_deg = lon_deg_int;
                                    float lon_min = lln - (lon_deg * 100);
                                    lon = lon_deg + (lon_min/60.);
                                    if (m_NMEA0183.Gga.Position.Longitude.Easting == West)
                                          lon = -lon;
                                    mLon.SetValue(lon);
                              }

                              if (mPriDateTime >= 4) {
                                    mPriDateTime = 4;
                                    //mUTCDateTime.ParseFormat(m_NMEA0183.Gga.UTCTime.c_str(), _T("%H%M%S"));
                              }

                              //m_NMEA0183.Gga.NumberOfSatellitesInUse;
                        }
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("GLL"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (m_NMEA0183.Gll.IsDataValid == NTrue)
                        {
                              if (mPriPosition >= 2) {
                                    mPriPosition = 2;
                                    double lat, lon;
                                    float llt = m_NMEA0183.Gll.Position.Latitude.Latitude;
                                    int lat_deg_int = (int)(llt / 100);
                                    float lat_deg = lat_deg_int;
                                    float lat_min = llt - (lat_deg * 100);
                                    lat = lat_deg + (lat_min/60.);
                                    if (m_NMEA0183.Gll.Position.Latitude.Northing == South)
                                          lat = -lat;
                                    mLat.SetValue(lat);

                                    float lln = m_NMEA0183.Gll.Position.Longitude.Longitude;
                                    int lon_deg_int = (int)(lln / 100);
                                    float lon_deg = lon_deg_int;
                                    float lon_min = lln - (lon_deg * 100);
                                    lon = lon_deg + (lon_min/60.);
                                    if (m_NMEA0183.Gll.Position.Longitude.Easting == West)
                                          lon = -lon;
                                    mLon.SetValue(lon);
                              }

                              if (mPriDateTime >= 5)
                              {
                                    mPriDateTime = 5;
                                    //mUTCDateTime.ParseFormat(m_NMEA0183.Gll.UTCTime.c_str(), _T("%H%M%S"));
                              }
                        }
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("HDG"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (mPriVar >= 2) {
                              mPriVar = 2;
                              if (m_NMEA0183.Hdg.MagneticVariationDirection == East)
                                    mVar.SetValue(m_NMEA0183.Hdg.MagneticVariationDegrees);
                              else if (m_NMEA0183.Hdg.MagneticVariationDirection == West)
                                    mVar.SetValue(-m_NMEA0183.Hdg.MagneticVariationDegrees);
                        }
                        if (mPriHeadingM >= 1) {
                              mPriHeadingM = 1;
                              mHeadingM.SetValue(m_NMEA0183.Hdg.MagneticSensorHeadingDegrees);
                        }
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("HDM"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (mPriHeadingM >= 2) {
                              mPriHeadingM = 2;
                              mHeadingM.SetValue(m_NMEA0183.Hdm.DegreesMagnetic);
                        }
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("HDT"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (mPriHeadingT >= 1) {
                              mPriHeadingT = 1;
                              if (m_NMEA0183.Hdt.DegreesTrue < 999.)
                              {
                                    mHeadingT.SetValue(m_NMEA0183.Hdt.DegreesTrue);
                              }
                        }
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("MTW"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        /*
                        double   m_NMEA0183.Mtw.Temperature;
                        wxString m_NMEA0183.Mtw.UnitOfMeasurement;
                        */
                        mTemp.SetValue(m_NMEA0183.Mtw.Temperature);
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("MWD"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (mPriWindT >= 3)
                        {
                              mPriWindT = 3;

                              // Option for True vs Magnetic
                              mTWA.SetValue(m_NMEA0183.Mwd.WindAngleTrue);
                              //m_NMEA0183.Mwd.WindAngleMagnetic
                              mTWS.SetValue(m_NMEA0183.Mwd.WindSpeedKnots);
                              //m_NMEA0183.Mwd.WindSpeedms
                        }
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("MWV"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (m_NMEA0183.Mwv.IsDataValid == NTrue)
                        {
                              if (m_NMEA0183.Mwv.Reference == _T("R")) // Relative (apparent wind)
                              {
                                    if (mPriWindR >= 1)
                                    {
                                          mPriWindR = 1;

                                          mAWA.SetValue(m_NMEA0183.Mwv.WindAngle);
                                          //  4) Wind Speed Units, K/M/N
                                          mAWS.SetValue(m_NMEA0183.Mwv.WindSpeed);
                                    }
                              }
                              else if (m_NMEA0183.Mwv.Reference == _T("T")) // Theoretical (aka True)
                              {
                                    if (mPriWindT >= 1)
                                    {
                                          mPriWindT = 1;

                                          mTWA.SetValue(m_NMEA0183.Mwv.WindAngle);
                                          //  4) Wind Speed Units, K/M/N
                                          mTWS.SetValue(m_NMEA0183.Mwv.WindSpeed);
                                    }
                              }
                        }
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("RMC"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (m_NMEA0183.Rmc.IsDataValid == NTrue)
                        {
                              if (mPriPosition >= 4) {
                                    mPriPosition = 4;
                                    double lat, lon;
                                    float llt = m_NMEA0183.Rmc.Position.Latitude.Latitude;
                                    int lat_deg_int = (int)(llt / 100);
                                    float lat_deg = lat_deg_int;
                                    float lat_min = llt - (lat_deg * 100);
                                    lat = lat_deg + (lat_min/60.);
                                    if (m_NMEA0183.Rmc.Position.Latitude.Northing == South)
                                          lat = -lat;
                                    mLat.SetValue(lat);

                                    float lln = m_NMEA0183.Rmc.Position.Longitude.Longitude;
                                    int lon_deg_int = (int)(lln / 100);
                                    float lon_deg = lon_deg_int;
                                    float lon_min = lln - (lon_deg * 100);
                                    lon = lon_deg + (lon_min/60.);
                                    if (m_NMEA0183.Rmc.Position.Longitude.Easting == West)
                                          lon = -lon;
                                    mLon.SetValue(lon);
                              }

                              if (mPriCOGSOG >= 3) {
                                    mPriCOGSOG = 3;
                                    if (m_NMEA0183.Rmc.SpeedOverGroundKnots < 999.)
                                    {
                                          mSOG.SetValue(m_NMEA0183.Rmc.SpeedOverGroundKnots);
                                    }
                                    if (m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue < 999.)
                                    {
                                          mCOG.SetValue(m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue);
                                    }
                              }

                              if (mPriVar >= 3) {
                                    mPriVar = 3;
                                    if (m_NMEA0183.Rmc.MagneticVariationDirection == East)
                                          mVar.SetValue(m_NMEA0183.Rmc.MagneticVariation);
                                    else if (m_NMEA0183.Rmc.MagneticVariationDirection == West)
                                          mVar.SetValue(-m_NMEA0183.Rmc.MagneticVariation);
                              }

                              if (mPriDateTime >= 3)
                              {
                                    mPriDateTime = 3;
                                    wxString dt = m_NMEA0183.Rmc.UTCTime;
                                    dt.Append(m_NMEA0183.Rmc.Date);
                                    //mUTCDateTime.ParseFormat(dt.c_str(), _T("%d%m%y%H%M%S"));
                              }
                        }
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("VHW"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (mPriHeadingT >= 2) {
                              mPriHeadingT = 2;
                              if (m_NMEA0183.Vhw.DegreesTrue < 999.)
                              {
                                    mHeadingT.SetValue(m_NMEA0183.Vhw.DegreesTrue);
                              }
                        }
                        if (mPriHeadingM >= 3) {
                              mPriHeadingM = 3;
                              mHeadingM.SetValue(m_NMEA0183.Vhw.DegreesMagnetic);
                        }
                        if (m_NMEA0183.Vhw.Knots < 999.)
                        {
                              mSTW.SetValue(m_NMEA0183.Vhw.Knots);
                        }
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("VTG"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (mPriCOGSOG >= 2) {
                              mPriCOGSOG = 2;
                              //    Special check for unintialized values, as opposed to zero values
                              if (m_NMEA0183.Vtg.SpeedKnots < 999.)
                              {
                                    mSOG.SetValue(m_NMEA0183.Vtg.SpeedKnots);
                              }
                              // Vtg.SpeedKilometersPerHour;
                              if (m_NMEA0183.Vtg.TrackDegreesTrue < 999.)
                              {
                                    mCOG.SetValue(m_NMEA0183.Vtg.TrackDegreesTrue);
                              }
                        }

                        /*
                        m_NMEA0183.Vtg.TrackDegreesMagnetic;
                        */
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("VWR"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (mPriWindR >= 2)
                        {
                              mPriWindR = 2;

                              mAWA.SetValue(m_NMEA0183.Vwr.DirectionOfWind==Left ? 360-m_NMEA0183.Vwr.WindDirectionMagnitude : m_NMEA0183.Vwr.WindDirectionMagnitude);
                              mAWS.SetValue(m_NMEA0183.Vwr.WindSpeedKnots);
                              /*
                              double           m_NMEA0183.Vwr.WindSpeedms;
                              double           m_NMEA0183.Vwr.WindSpeedKmh;
                              */
                        }
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("VWT"))
            {
                  if (m_NMEA0183.Parse())
                  {
                        if (mPriWindT >= 2)
                        {
                              mPriWindT = 2;

/*
Calculated wind angle relative to the vessel, 0 to 180o, left/right L/R of vessel heading
*/
                              mTWA.SetValue(m_NMEA0183.Vwt.DirectionOfWind==Left ? 360-m_NMEA0183.Vwt.WindDirectionMagnitude : m_NMEA0183.Vwr.WindDirectionMagnitude);
                              mTWS.SetValue(m_NMEA0183.Vwt.WindSpeedKnots);
                              /*
                              double           m_NMEA0183.Vwt.WindSpeedms;
                              double           m_NMEA0183.Vwt.WindSpeedKmh;
                              */
                        }
                  }
            }

            else if (m_NMEA0183.LastSentenceIDReceived == _T("ZDA"))
            {
                  if (m_NMEA0183.Parse())
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
                              //mUTCDateTime.ParseFormat(dt.c_str(), _T("%Y%m%d%H%M%S"));
                        }
                  }
            }
      }
}

void logbook_pi::ShowPreferencesDialog( wxWindow* parent )
{
      LogbookPreferencesDialog *dialog = new LogbookPreferencesDialog( parent, wxID_ANY, m_filename, m_interval );

      if (dialog->ShowModal() == wxID_OK)
      {
            // OnClose should handle that for us normally but it doesn't seems to do so
            // We must save changes first
            dialog->SaveLogbookConfig();

            m_filename = dialog->m_filename;
            m_interval = dialog->m_interval;
            SaveConfig();
            ApplyConfig();
      }
      dialog->Destroy();
}

bool logbook_pi::LoadConfig(void)
{
      wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

      if (pConf)
      {
            pConf->SetPath( _T("/PlugIns/Logbook") );

            pConf->Read( _T("Filename"), &m_filename, wxEmptyString );
            pConf->Read( _T("Interval"), &m_interval, -1 );

            return true;
      }
      else
            return false;
}

bool logbook_pi::SaveConfig(void)
{
      wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

      if (pConf)
      {
            pConf->SetPath( _T("/PlugIns/Logbook") );

            pConf->Write( _T("Filename"), m_filename );
            pConf->Write( _T("Interval"), m_interval );

            return true;
      }
      else
            return false;
}

void logbook_pi::WriteLogEntry( wxString entry )
{
      wxTextFile file(m_filename);
      file.Open();
      file.AddLine(entry);
      file.Write();
}

void logbook_pi::ApplyConfig(void)
{
      if ( m_filename != wxEmptyString && m_interval != -1 )
      {
            // Transform minutes interval into milliseconds
            if (! Start( m_interval*60000, wxTIMER_CONTINUOUS ) )
                  wxLogMessage(_T("logbook_pi: Timer start failed!"));
      }
      //TODO: Write header?
      WriteLogEntry(_T("Date;Lat;Lon;COG;SOG;HDM;HDT;STW;AWA;AWS;TWA;TWS;Depth;Temp"));
}

void logbook_pi::Notify()
{
      //TODO: Write fields
      wxDateTime now = wxDateTime::Now();
      wxString s = now.Format(_T("%c"), wxDateTime::UTC)+_T(";");
      if (mLat.IsValid())
            s += toSDMM(1, mLat.GetValue(OCPN_LBI_MAIN, true));
      s += _T(";");
      if (mLon.IsValid())
            s += toSDMM(2, mLon.GetValue(OCPN_LBI_MAIN, true));
      s += _T(";")+mCOG.GetFormattedValue(OCPN_LBI_MAIN, _T("%3.0f Deg"), true)+
            _T(";")+mSOG.GetFormattedValue(OCPN_LBI_MAIN, _T("%4.2f Kts"), true)+
            _T(";")+mHeadingM.GetFormattedValue(OCPN_LBI_MAIN, _T("%3.0f Deg"), true)+
            _T(";")+mHeadingT.GetFormattedValue(OCPN_LBI_MAIN, _T("%3.0f Deg"), true)+
            _T(";")+mSTW.GetFormattedValue(OCPN_LBI_MAIN, _T("%4.2f Kts"), true)+
            _T(";")+mAWA.GetFormattedValue(OCPN_LBI_MAIN, _T("%3.0f Deg"), true)+
            _T(";")+mAWS.GetFormattedValue(OCPN_LBI_MAIN, _T("%4.2f Kts"), true)+
            _T(";")+mTWA.GetFormattedValue(OCPN_LBI_MAIN, _T("%3.0f Deg"), true)+
            _T(";")+mTWS.GetFormattedValue(OCPN_LBI_MAIN, _T("%4.2f Kts"), true)+
            _T(";")+mDepth.GetFormattedValue(OCPN_LBI_MAIN, _T("%3.1f m"), true)+
            _T(";")+mTemp.GetFormattedValue(OCPN_LBI_MAIN, _T("%2.0f C"), true);

      WriteLogEntry(s);
}

/* LogbookItem
 *
 */

LogbookItem::LogbookItem()
{
      Reset();
}

void LogbookItem::SetValue( double value )
{
      if ( m_lastReceived == wxInvalidDateTime )
      {
            m_min = value;
            m_max = value;
            m_avg = value;
      }
      else
      {
            if (value < m_min) m_min = value;
            if (value > m_max) m_max = value;
            m_avg = ((m_count * m_avg) + value) / (m_count + 1);
      }
      m_value = value;
      m_count++;
      m_lastReceived.SetToCurrent();
}

void LogbookItem::Reset()
{
      m_lastReceived = wxInvalidDateTime;
      m_value = LOGBOOK_EMPTY_VALUE;
      m_min = LOGBOOK_EMPTY_VALUE;
      m_max = LOGBOOK_EMPTY_VALUE;
      m_avg = LOGBOOK_EMPTY_VALUE;
      m_count = 0;
}

bool LogbookItem::IsValid()
{
      return ( m_lastReceived != wxInvalidDateTime && m_lastReceived.IsEqualUpTo(wxDateTime::Now(), wxTimeSpan::Minutes(DATA_VALIDITY)) );
}

double LogbookItem::GetValue(short which, bool reset=false)
{
      if (! IsValid()) return LOGBOOK_EMPTY_VALUE;

      double ret = LOGBOOK_EMPTY_VALUE;
      switch (which)
      {
      case OCPN_LBI_MAIN:
            ret = m_value;
      break;
      case OCPN_LBI_MIN:
            ret = m_min;
      break;
      case OCPN_LBI_MAX:
            ret = m_max;
      break;
      case OCPN_LBI_AVG:
            ret = m_avg;
      break;
      }
      if (reset) Reset();
      return ret;

}

wxString LogbookItem::GetFormattedValue(short which, wxString format, bool reset=false)
{
      if (! IsValid()) return wxEmptyString;
      return wxString::Format(format, GetValue(which, reset));
}

/* LogbookPreferencesDialog
 *
 */

LogbookPreferencesDialog::LogbookPreferencesDialog( wxWindow *parent, wxWindowID id, wxString filename, int interval )
      :wxDialog( parent, id, _("Logbook preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE )
{
      Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( LogbookPreferencesDialog::OnCloseDialog ), NULL, this );

      wxBoxSizer* itemBoxSizerMainPanel = new wxBoxSizer(wxVERTICAL);
      SetSizer(itemBoxSizerMainPanel);

      wxStaticBox* itemStaticBox = new wxStaticBox( this, wxID_ANY, _("Preferences") );
      wxStaticBoxSizer* itemStaticBoxSizer = new wxStaticBoxSizer( itemStaticBox, wxHORIZONTAL );
      itemBoxSizerMainPanel->Add( itemStaticBoxSizer, 0, wxEXPAND|wxALL, 2 );

      wxFlexGridSizer *itemFlexGridSizer = new wxFlexGridSizer(2);
      itemFlexGridSizer->AddGrowableCol(1);
      itemStaticBoxSizer->Add( itemFlexGridSizer, 0, wxGROW|wxALL, 2 );

      wxStaticText* itemStaticText01 = new wxStaticText( this, wxID_ANY, _("Interval:"), wxDefaultPosition, wxDefaultSize, 0 );
      itemFlexGridSizer->Add( itemStaticText01, 0, wxEXPAND|wxALL, 2 );
      m_interval = interval;
      m_pInterval = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 60, interval );
      itemFlexGridSizer->Add( m_pInterval, 1, wxALIGN_LEFT|wxALL, 2 );

      wxStaticText* itemStaticText02 = new wxStaticText( this, wxID_ANY, _("Filename:"), wxDefaultPosition, wxDefaultSize, 0 );
      itemFlexGridSizer->Add( itemStaticText02, 0, wxEXPAND|wxALL, 2 );
      m_filename = filename;
      m_pFilename = new wxFilePickerCtrl( this, wxID_ANY, filename, _("Select a file"), _T("*.csv"), wxDefaultPosition, wxDefaultSize, wxFLP_SAVE|wxFLP_OVERWRITE_PROMPT|wxFLP_USE_TEXTCTRL );
      itemFlexGridSizer->Add( m_pFilename, 1, wxALIGN_LEFT|wxALL, 2 );

      wxStdDialogButtonSizer* DialogButtonSizer = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
      itemBoxSizerMainPanel->Add(DialogButtonSizer, 0, wxALIGN_RIGHT|wxALL, 5);

      //SetMinSize(wxSize(454, -1));
      Fit();
}

void LogbookPreferencesDialog::OnCloseDialog(wxCloseEvent& event)
{
      SaveLogbookConfig();
      event.Skip();
}

void LogbookPreferencesDialog::SaveLogbookConfig()
{
      m_filename = m_pFilename->GetPath();
      m_interval = m_pInterval->GetValue();
}

