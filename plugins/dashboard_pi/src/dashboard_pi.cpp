/******************************************************************************
 * $Id: dashboard.cpp,v 1.8 2010/06/21 01:54:37 bdbcat Exp $
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
           WANTS_NMEA_SENTENCES
            );
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


wxString dashboard_pi::GetShortDescription()
{
      return _("Dashboard PlugIn for OpenCPN");
}

wxString dashboard_pi::GetLongDescription()
{
      return _("Dashboard PlugIn for OpenCPN\n\
Demonstrates PlugIn processing of NMEA messages.");

}

void dashboard_pi::SetNMEASentence(wxString &sentence)
{
//      printf("dashboard_pi::SetNMEASentence\n");
      if(m_pdashboard_window)
      {
            m_pdashboard_window->SetSentence(sentence);
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

DashboardWindow::DashboardWindow(wxWindow *pparent, wxWindowID id)
      :wxMiniFrame(pparent, id, _T("DashBoard"), wxDefaultPosition, wxDefaultSize,
             wxCAPTION|wxRESIZE_BORDER|wxFRAME_FLOAT_ON_PARENT|wxFRAME_NO_TASKBAR)
{
      int w = 0; int h = 0;
      pparent->GetSize(&w, &h);
      int myW = 200; int myH = 300;
      SetSize(w-myW, 200, myW, myH);
      SetMinSize(wxSize(myW, myH));

      wxColour cl;
      GetGlobalColor(_T("UIBDR"), &cl);
      SetBackgroundColour(cl);

      wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
      SetSizer(itemBoxSizer1);

      m_pDBILat = new DashboardInstrument(this, id, _("Lat"));
      itemBoxSizer1->Add(m_pDBILat, 0, wxALL|wxEXPAND, 0);
      m_pDBILon = new DashboardInstrument(this, id, _("Lon"));
      itemBoxSizer1->Add(m_pDBILon, 0, wxALL|wxEXPAND, 0);
      m_pDBISog = new DashboardInstrument(this, id, _("SOG"));
      itemBoxSizer1->Add(m_pDBISog, 0, wxALL|wxEXPAND, 0);
      m_pDBICog = new DashboardInstrument(this, id, _("COG"));
      itemBoxSizer1->Add(m_pDBICog, 0, wxALL|wxEXPAND, 0);
}

DashboardWindow::~DashboardWindow()
{
}

void DashboardWindow::SetSentence(wxString &sentence)
{
      m_NMEA0183 << sentence;

      if(m_NMEA0183.PreParse())
      {
            if(m_NMEA0183.LastSentenceIDReceived == _T("RMC"))
            {
                  if(m_NMEA0183.Parse())
                  {
                        if(m_NMEA0183.Rmc.IsDataValid == NTrue)
                        {
                              double val;
                              float llt = m_NMEA0183.Rmc.Position.Latitude.Latitude;
                              int lat_deg_int = (int)(llt / 100);
                              float lat_deg = lat_deg_int;
                              float lat_min = llt - (lat_deg * 100);
                              val = lat_deg + (lat_min/60.);
                              if(m_NMEA0183.Rmc.Position.Latitude.Northing == South)
                                    val = -val;
                              m_pDBILat->SetData(toSDMM(1, val));

                              float lln = m_NMEA0183.Rmc.Position.Longitude.Longitude;
                              int lon_deg_int = (int)(lln / 100);
                              float lon_deg = lon_deg_int;
                              float lon_min = lln - (lon_deg * 100);
                              val = lon_deg + (lon_min/60.);
                              if(m_NMEA0183.Rmc.Position.Longitude.Easting == West)
                                    val = -val;
                              m_pDBILon->SetData(toSDMM(2, val));

                              m_pDBISog->SetData(wxString::Format(_("%5.2f Kts"), m_NMEA0183.Rmc.SpeedOverGroundKnots));
                              m_pDBICog->SetData(wxString::Format(_("%5.0f Deg"), m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue));

/*
                              if(m_NMEA0183.Rmc.MagneticVariationDirection == East)
                                    mVar =  m_NMEA0183.Rmc.MagneticVariation;
                              else if(m_NMEA0183.Rmc.MagneticVariationDirection == West)
                                    mVar = -m_NMEA0183.Rmc.MagneticVariation;
*/

                        }
                  }
            }
      }
}

//----------------------------------------------------------------
//
//    Dashboard Instrument Implementation
//
//----------------------------------------------------------------

BEGIN_EVENT_TABLE(DashboardInstrument, wxWindow)
  EVT_PAINT ( DashboardInstrument::OnPaint )
END_EVENT_TABLE()

DashboardInstrument::DashboardInstrument(wxWindow *pparent, wxWindowID id, wxString title)
      :wxWindow(pparent, id, wxDefaultPosition, wxDefaultSize,
             wxBORDER_NONE, _T("OpenCPN PlugIn"))
{
      m_label = title;
      m_data = _T("");

      wxColour cl;
      GetGlobalColor(_T("UIBDR"), &cl);
      SetBackgroundColour(cl);    //UINFF
//      SetTransparent(0);
      SetMinSize(wxSize(200, 50));
}

DashboardInstrument::~DashboardInstrument()
{
}

void DashboardInstrument::OnPaint(wxPaintEvent& event)
{
      wxPaintDC dc ( this );

//      dc.SetBackground(wxBrush(GetGlobalColor(_T("UIBDR")), wxSOLID));    //UINFF
//      dc.Clear();

      dc.SetFont(*OCPNGetFont(_T("DashBoard Label"), 9));
//      dc.SetTextForeground(pFontMgr->GetFontColor(_T("DashBoard Label")));
      wxColour cl;
      GetGlobalColor(_T("BLUE2"), &cl);
      dc.SetTextForeground(cl);
      dc.DrawText(m_label, 5, 5);

      dc.SetFont(*OCPNGetFont(_T("DashBoard Data"), 16));
      //dc.SetTextForeground(pFontMgr->GetFontColor(_T("DashBoard Data")));
      wxColour cd;
      GetGlobalColor(_T("BLUE1"), &cd);
      dc.SetTextForeground(cd);

      dc.DrawText(m_data, 30, 20);
}

void DashboardInstrument::SetData(wxString data)
{
      m_data = data;
      Refresh(false);
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
