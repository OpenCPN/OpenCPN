/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 */


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
  #include <wx/glcanvas.h>
#endif //precompiled headers

#include <wx/fileconf.h>
#include <wx/stdpaths.h>

#include "grib_pi.h"

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new grib_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}

//---------------------------------------------------------------------------------------------------------
//
//    Grib PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

#include "icons.h"


//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

grib_pi::grib_pi(void *ppimgr)
      :opencpn_plugin_17(ppimgr)
{
      // Create the PlugIn icons
      initialize_images();
      m_pLastTimelineSet = NULL;
      m_bShowGrib = false;
}

grib_pi::~grib_pi(void)
{
      delete _img_grib_pi;
      delete _img_grib;
      delete m_pLastTimelineSet;
}

int grib_pi::Init(void)
{
      AddLocaleCatalog( _T("opencpn-grib_pi") );

      // Set some default private member parameters
      m_grib_dialog_x = 0;
      m_grib_dialog_y = 0;
      m_grib_dialog_sx = 200;
      m_grib_dialog_sy = 400;
      m_pGribDialog = NULL;
      m_pGRIBOverlayFactory = NULL;

      ::wxDisplaySize(&m_display_width, &m_display_height);

      //    Get a pointer to the opencpn configuration object
      m_pconfig = GetOCPNConfigObject();

      //    And load the configuration items
      LoadConfig();

      // Get a pointer to the opencpn display canvas, to use as a parent for the GRIB dialog
      m_parent_window = GetOCPNCanvasWindow();

//      int m_height = GetChartbarHeight();
      //    This PlugIn needs a toolbar icon, so request its insertion if enabled locally
      if(m_bGRIBShowIcon)
          m_leftclick_tool_id = InsertPlugInTool(_T(""), _img_grib, _img_grib, wxITEM_CHECK,
                                                 _("Grib"), _T(""), NULL,
                                                 GRIB_TOOL_POSITION, 0, this);

      return (WANTS_OVERLAY_CALLBACK |
              WANTS_OPENGL_OVERLAY_CALLBACK |
              WANTS_CURSOR_LATLON       |
              WANTS_TOOLBAR_CALLBACK    |
              INSTALLS_TOOLBAR_TOOL     |
              WANTS_CONFIG              |
              WANTS_PREFERENCES         |
              WANTS_PLUGIN_MESSAGING
            );
}

bool grib_pi::DeInit(void)
{
    if(m_pGribDialog) {
        m_pGribDialog->Close();
        delete m_pGribDialog;
        m_pGribDialog = NULL;
    }

    delete m_pGRIBOverlayFactory;
    m_pGRIBOverlayFactory = NULL;

    return true;
}

int grib_pi::GetAPIVersionMajor()
{
      return MY_API_VERSION_MAJOR;
}

int grib_pi::GetAPIVersionMinor()
{
      return MY_API_VERSION_MINOR;
}

int grib_pi::GetPlugInVersionMajor()
{
      return PLUGIN_VERSION_MAJOR;
}

int grib_pi::GetPlugInVersionMinor()
{
      return PLUGIN_VERSION_MINOR;
}

wxBitmap *grib_pi::GetPlugInBitmap()
{
      return _img_grib_pi;
}

wxString grib_pi::GetCommonName()
{
      return _T("GRIB");
}


wxString grib_pi::GetShortDescription()
{
      return _("GRIB PlugIn for OpenCPN");
}


wxString grib_pi::GetLongDescription()
{
      return _("GRIB PlugIn for OpenCPN\n\
Provides basic GRIB file overlay capabilities for several GRIB file types.\n\n\
Supported GRIB file types include:\n\
- wind direction and speed\n\
- significant wave height\n\
- sea surface temperature\n\
- surface current direction and speed.");

}


void grib_pi::SetDefaults(void)
{
}


int grib_pi::GetToolbarToolCount(void)
{
      return 1;
}

void grib_pi::ShowPreferencesDialog( wxWindow* parent )
{
    GribPreferencesDialog *Pref = new GribPreferencesDialog(parent);

    Pref->m_cbUseHiDef->SetValue(m_bGRIBUseHiDef);
    Pref->m_cbUseGradualColors->SetValue(m_bGRIBUseGradualColors);
    Pref->m_cbCopyFirstCumulativeRecord->SetValue(m_bCopyFirstCumRec);
    Pref->m_cbCopyMissingWaveRecord->SetValue(m_bCopyMissWaveRec);
    Pref->m_rbTimeFormat->SetSelection( m_bTimeZone );
    Pref->m_rbLoadOptions->SetSelection( m_bLoadLastOpenFile );
    Pref->m_rbStartOptions->SetSelection( m_bStartOptions );

     if( Pref->ShowModal() == wxID_OK ) {
         m_bGRIBUseHiDef= Pref->m_cbUseHiDef->GetValue();
         m_bGRIBUseGradualColors= Pref->m_cbUseGradualColors->GetValue();
         m_bLoadLastOpenFile= Pref->m_rbLoadOptions->GetSelection();
          if( m_pGRIBOverlayFactory )
              m_pGRIBOverlayFactory->SetSettings( m_bGRIBUseHiDef, m_bGRIBUseGradualColors );

         int updatelevel = 0;

         if( m_bStartOptions != Pref->m_rbStartOptions->GetSelection() ) {
             m_bStartOptions = Pref->m_rbStartOptions->GetSelection();
             updatelevel = 1;
         }

         if( m_bTimeZone != Pref->m_rbTimeFormat->GetSelection() ) {
             m_bTimeZone = Pref->m_rbTimeFormat->GetSelection();
             if( m_pGRIBOverlayFactory )
                m_pGRIBOverlayFactory->SetTimeZone( m_bTimeZone );
             updatelevel = 2;
         }

         bool copyrec = Pref->m_cbCopyFirstCumulativeRecord->GetValue();
         bool copywave = Pref->m_cbCopyMissingWaveRecord->GetValue();
         if( m_bCopyFirstCumRec != copyrec || m_bCopyMissWaveRec != copywave ) {
             m_bCopyFirstCumRec = copyrec;
             m_bCopyMissWaveRec = copywave;
             updatelevel = 3;
         }

         if(m_pGribDialog ) {
             switch( updatelevel ) {
             case 0:
                 break;
             case 3:
                 //rebuild current activefile with new parameters and rebuil data list with current index
                 m_pGribDialog->CreateActiveFileFromName( m_pGribDialog->m_bGRIBActiveFile->GetFileName() );
                 m_pGribDialog->PopulateComboDataList();
                 m_pGribDialog->TimelineChanged();
                 break;
             case 2 :
                 //only rebuild  data list with current index and new timezone
                 m_pGribDialog->PopulateComboDataList();
                 m_pGribDialog->TimelineChanged();
                 break;
             case 1:
                 //only re-compute the best forecast
                 m_pGribDialog->ComputeBestForecastForNow();
                 break;
             }
         }

         SaveConfig();
     }
}

void grib_pi::OnToolbarToolCallback(int id)
{
    if(!m_pGribDialog)
    {
        m_pGribDialog = new GRIBUIDialog(m_parent_window, this);
        wxPoint p = wxPoint(m_grib_dialog_x, m_grib_dialog_y);
        m_pGribDialog->Move(0,0);        // workaround for gtk autocentre dialog behavior
        m_pGribDialog->Move(p);
        wxMenu* dummy = new wxMenu(_T("Plugin"));
        wxMenuItem* table = new wxMenuItem( dummy, wxID_ANY, wxString( _("Weather table") ), wxEmptyString, wxITEM_NORMAL );
        m_MenuItem = AddCanvasContextMenuItem(table, this);
        SetCanvasContextMenuItemViz(m_MenuItem, false);

        // Create the drawing factory
        m_pGRIBOverlayFactory = new GRIBOverlayFactory( *m_pGribDialog );
        m_pGRIBOverlayFactory->SetTimeZone( m_bTimeZone );
        m_pGRIBOverlayFactory->SetParentSize( m_display_width, m_display_height);
        m_pGRIBOverlayFactory->SetSettings( m_bGRIBUseHiDef, m_bGRIBUseGradualColors );

        m_pGribDialog->OpenFile( m_bLoadLastOpenFile == 0 );
    }

      // Qualify the GRIB dialog position
            bool b_reset_pos = false;

#ifdef __WXMSW__
        //  Support MultiMonitor setups which an allow negative window positions.
        //  If the requested window does not intersect any installed monitor,
        //  then default to simple primary monitor positioning.
            RECT frame_title_rect;
            frame_title_rect.left =   m_grib_dialog_x;
            frame_title_rect.top =    m_grib_dialog_y;
            frame_title_rect.right =  m_grib_dialog_x + m_grib_dialog_sx;
            frame_title_rect.bottom = m_grib_dialog_y + 30;


            if(NULL == MonitorFromRect(&frame_title_rect, MONITOR_DEFAULTTONULL))
                  b_reset_pos = true;
#else
       //    Make sure drag bar (title bar) of window on Client Area of screen, with a little slop...
            wxRect window_title_rect;                    // conservative estimate
            window_title_rect.x = m_grib_dialog_x;
            window_title_rect.y = m_grib_dialog_y;
            window_title_rect.width = m_grib_dialog_sx;
            window_title_rect.height = 30;

            wxRect ClientRect = wxGetClientDisplayRect();
            ClientRect.Deflate(60, 60);      // Prevent the new window from being too close to the edge
            if(!ClientRect.Intersects(window_title_rect))
                  b_reset_pos = true;

#endif

            if(b_reset_pos)
            {
                  m_grib_dialog_x = 20;
                  m_grib_dialog_y = 170;
                  m_grib_dialog_sx = 300;
                  m_grib_dialog_sy = 540;
            }

      //Toggle GRIB overlay display
      m_bShowGrib = !m_bShowGrib;

      //    Toggle dialog?
      if(m_bShowGrib) {
          m_pGribDialog->Show();
          if( m_pGribDialog->m_bGRIBActiveFile ) {
              if( m_pGribDialog->m_bGRIBActiveFile->IsOK() ) {
                  ArrayOfGribRecordSets *rsa = m_pGribDialog->m_bGRIBActiveFile->GetRecordSetArrayPtr();
                  if(rsa->GetCount() > 1) SetCanvasContextMenuItemViz( m_MenuItem, true);
              }
          }
      } else {
          SetCanvasContextMenuItemViz( m_MenuItem, false);
          m_pGribDialog->Hide();
          if(m_pGribDialog->pReq_Dialog) m_pGribDialog->pReq_Dialog->Hide();
          }

      // Toggle is handled by the toolbar but we must keep plugin manager b_toggle updated
      // to actual status to ensure correct status upon toolbar rebuild
      SetToolbarItemState( m_leftclick_tool_id, m_bShowGrib );
/*
      wxPoint p = m_pGribDialog->GetPosition();
      m_pGribDialog->Move(0,0);        // workaround for gtk autocentre dialog behavior
      m_pGribDialog->Move(p);
*/
      RequestRefresh(m_parent_window); // refresh mainn window
}

void grib_pi::OnGribDialogClose()
{
    m_bShowGrib = false;
    SetToolbarItemState( m_leftclick_tool_id, m_bShowGrib );

    m_pGribDialog->Hide();
    if(m_pGribDialog->pReq_Dialog) m_pGribDialog->pReq_Dialog->Hide();

    SaveConfig();

    RequestRefresh(m_parent_window); // refresh mainn window

}

bool grib_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    if(!m_pGribDialog ||
       !m_pGribDialog->IsShown() ||
       !m_pGRIBOverlayFactory)
        return false;

    m_pGribDialog->SetViewPort( vp );
    m_pGRIBOverlayFactory->RenderGribOverlay ( dc, vp );
    return true;
}

bool grib_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    if(!m_pGribDialog ||
       !m_pGribDialog->IsShown() ||
       !m_pGRIBOverlayFactory)
        return false;

    m_pGribDialog->SetViewPort( vp );
    m_pGRIBOverlayFactory->RenderGLGribOverlay ( pcontext, vp );
    return true;
}

void grib_pi::SetCursorLatLon(double lat, double lon)
{
    if(m_pGribDialog)
        m_pGribDialog->SetCursorLatLon(lat, lon);
}

void grib_pi::OnContextMenuItemCallback(int id)
{
    if(!m_pGribDialog->m_bGRIBActiveFile) return;
    m_pGribDialog->ContextMenuItemCallback(id);
}

void grib_pi::SetPluginMessage(wxString &message_id, wxString &message_body)
{
    if(message_id == _T("GRIB_VERSION_REQUEST"))
    {
        wxJSONValue v;
        v[_T("GribVersionMinor")] = GetAPIVersionMinor();
        v[_T("GribVersionMajor")] = GetAPIVersionMajor();

        wxJSONWriter w;
        wxString out;
        w.Write(v, out);
        SendPluginMessage(wxString(_T("GRIB_VERSION")), out);
    }
    if(message_id == _T("GRIB_TIMELINE_REQUEST"))
    {
        SendTimelineMessage(m_pGribDialog ? m_pGribDialog->TimelineTime() : wxDateTime::Now());
    }
    if(message_id == _T("GRIB_TIMELINE_RECORD_REQUEST"))
    {
        wxJSONReader r;
        wxJSONValue v;
        r.Parse(message_body, &v);
        wxDateTime time(v[_T("Day")].AsInt(),
                        (wxDateTime::Month)v[_T("Month")].AsInt(),
                        v[_T("Year")].AsInt(),
                        v[_T("Hour")].AsInt(),
                        v[_T("Minute")].AsInt(),
                        v[_T("Second")].AsInt());

        GribTimelineRecordSet *set = m_pGribDialog ? m_pGribDialog->GetTimeLineRecordSet(time) : NULL;

        char ptr[64];
        snprintf(ptr, sizeof ptr, "%p", set);

        v[_T("TimelineSetPtr")] = wxString::From8BitData(ptr);

        wxJSONWriter w;
        wxString out;
        w.Write(v, out);
        SendPluginMessage(wxString(_T("GRIB_TIMELINE_RECORD")), out);
        delete m_pLastTimelineSet;
        m_pLastTimelineSet = set;
    }
}

bool grib_pi::LoadConfig(void)
{
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

    if(!pConf)
        return false;

    pConf->SetPath ( _T( "/PlugIns/GRIB" ) );
    pConf->Read ( _T( "LoadLastOpenFile" ), &m_bLoadLastOpenFile, 0 );
    pConf->Read ( _T("OpenFileOption" ), &m_bStartOptions, 1 );
    pConf->Read ( _T( "GRIBUseHiDef" ),  &m_bGRIBUseHiDef, 0 );
    pConf->Read ( _T( "GRIBUseGradualColors" ),     &m_bGRIBUseGradualColors, 0 );

    pConf->Read ( _T( "ShowGRIBIcon" ), &m_bGRIBShowIcon, 1 );
    pConf->Read ( _T( "GRIBTimeZone" ), &m_bTimeZone, 1 );
    pConf->Read ( _T( "CopyFirstCumulativeRecord" ), &m_bCopyFirstCumRec, 1 );
    pConf->Read ( _T( "CopyMissingWaveRecord" ), &m_bCopyMissWaveRec, 1 );

    m_grib_dialog_sx = pConf->Read ( _T ( "GRIBDialogSizeX" ), 300L );
    m_grib_dialog_sy = pConf->Read ( _T ( "GRIBDialogSizeY" ), 540L );
    m_grib_dialog_x =  pConf->Read ( _T ( "GRIBDialogPosX" ), 20L );
    m_grib_dialog_y =  pConf->Read ( _T ( "GRIBDialogPosY" ), 170L );

    return true;
}

bool grib_pi::SaveConfig(void)
{
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

    if(!pConf)
        return false;

    pConf->SetPath ( _T( "/PlugIns/GRIB" ) );

    pConf->Write ( _T ( "LoadLastOpenFile" ), m_bLoadLastOpenFile );
    pConf->Write ( _T ( "OpenFileOption" ), m_bStartOptions );
    pConf->Write ( _T ( "ShowGRIBIcon" ), m_bGRIBShowIcon );
    pConf->Write ( _T ( "GRIBUseHiDef" ), m_bGRIBUseHiDef );
    pConf->Write ( _T ( "GRIBUseGradualColors" ),    m_bGRIBUseGradualColors );
    pConf->Write ( _T ( "GRIBTimeZone" ), m_bTimeZone );
    pConf->Write ( _T ( "CopyFirstCumulativeRecord" ), m_bCopyFirstCumRec );
    pConf->Write ( _T ( "CopyMissingWaveRecord" ), m_bCopyMissWaveRec );

    pConf->Write ( _T ( "GRIBDialogSizeX" ),  m_grib_dialog_sx );
    pConf->Write ( _T ( "GRIBDialogSizeY" ),  m_grib_dialog_sy );
    pConf->Write ( _T ( "GRIBDialogPosX" ),   m_grib_dialog_x );
    pConf->Write ( _T ( "GRIBDialogPosY" ),   m_grib_dialog_y );

    return true;
}

void grib_pi::SetColorScheme(PI_ColorScheme cs)
{
    DimeWindow(m_pGribDialog);
}

void grib_pi::SendTimelineMessage(wxDateTime time)
{
    if(!m_pGribDialog)
        return;

    wxJSONValue v;
    v[_T("Day")] = time.GetDay();
    v[_T("Month")] = time.GetMonth();
    v[_T("Year")] = time.GetYear();
    v[_T("Hour")] = time.GetHour();
    v[_T("Minute")] = time.GetMinute();
    v[_T("Second")] = time.GetSecond();

    wxJSONWriter w;
    wxString out;
    w.Write(v, out);
    SendPluginMessage(wxString(_T("GRIB_TIMELINE")), out);
}

//----------------------------------------------------------------------------------------------------------
//          Prefrence dialog Implementation
//----------------------------------------------------------------------------------------------------------
void GribPreferencesDialog::OnStartOptionChange( wxCommandEvent& event )
{
    if(m_rbStartOptions->GetSelection() == 2) {
        wxMessageDialog mes(this, _("You have chosen to authorize interpolation.\nDon't forget that data displayed at current time will not be real but interpolated!"),
                _("Warning!"), wxOK);
            mes.ShowModal();
        }
}
