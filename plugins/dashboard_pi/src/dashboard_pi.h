/******************************************************************************
 * $Id: dashboard_pi.h, v1.0 2010/08/05 SethDart Exp $
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

#ifndef _DASHBOARDPI_H_
#define _DASHBOARDPI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#define     PLUGIN_VERSION_MAJOR    1
#define     PLUGIN_VERSION_MINOR    0

#include <wx/minifram.h>
#include "../../../include/ocpn_plugin.h"

#include "nmea0183/nmea0183.h"
#include "instrument.h"
#include "speedometer.h"
#include "compass.h"
#include "wind.h"
#include "rudder_angle.h"
#include "gps.h"

wxString toSDMM ( int NEflag, double a );

class DashboardWindow;
class DashboardInstrument_Single;
class DashboardInstrument_Double;

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------


class dashboard_pi : public opencpn_plugin
{
public:
      dashboard_pi(void *ppimgr):opencpn_plugin(ppimgr){}

//    The required PlugIn Methods
      int Init(void);
      bool DeInit(void);

      int GetAPIVersionMajor();
      int GetAPIVersionMinor();
      int GetPlugInVersionMajor();
      int GetPlugInVersionMinor();

      wxString GetCommonName();
      wxString GetShortDescription();
      wxString GetLongDescription();

//    The optional method overrides

      void SetNMEASentence(wxString &sentence);
      void SetPositionFix(PlugIn_Position_Fix &pfix);
      void OnContextMenuItemCallback(int id);

private:
      wxWindow         *m_parent_window;

      DashboardWindow       *m_pdashboard_window;
      int               m_show_id;
      int               m_hide_id;

};

enum
{
      ID_DASHBOARD_WINDOW
};

class DashboardWindow : public wxMiniFrame
{
public:
      DashboardWindow(wxWindow *pparent, wxWindowID id);
      ~DashboardWindow();

      void SetSentence(wxString &sentence);
      void SetPosition(PlugIn_Position_Fix &pfix);
      void OnFocus(wxFocusEvent& event);
      void OnContextMenu(wxContextMenuEvent& event);
      void ToggleInstrumentVisibility(wxCommandEvent& event);
      /*TODO: OnKeyPress pass event to main window or disable focus*/

private:
      NMEA0183             m_NMEA0183;                 // Used to parse NMEA Sentences
      short                mPriPosition, mPriCOGSOG, mPriHeading, mPriVar, mPriDateTime, mPriApWind, mPriDepth;
      double               mVar;
      // FFU
      double               mSatsInView;
      double               mHdm;
      wxDateTime           mUTCDateTime;

      wxBoxSizer* itemBoxSizer;
      DashboardInstrument_Double*      m_pDBIPosition;
      DashboardInstrument_Single*      m_pDBISog;
      DashboardInstrument_Speedometer* m_pDBISpeedometer;
      DashboardInstrument_Single*      m_pDBICog;
      DashboardInstrument_Compass*     m_pDBICompass;
      DashboardInstrument_Wind*        m_pDBIWind;
      DashboardInstrument_Single*      m_pDBIHdt;
      DashboardInstrument_Single*      m_pDBIStw;
      DashboardInstrument_Single*      m_pDBIDepth;
      DashboardInstrument_RudderAngle* m_pDBIRudderAngle;
      DashboardInstrument_GPS*         m_pDBIGPS;

DECLARE_EVENT_TABLE()
};

#endif

