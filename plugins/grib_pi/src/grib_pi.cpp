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
  #ifdef ocpnUSE_GL
      #include <wx/glcanvas.h>
  #endif
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

extern int   m_DialogStyle;

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
    :opencpn_plugin_112(ppimgr)
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
      m_CtrlBarxy = wxPoint( 0 ,0 );
      m_CursorDataxy = wxPoint( 0, 0 );

      m_pGribCtrlBar = NULL;
      m_pGRIBOverlayFactory = NULL;

      ::wxDisplaySize(&m_display_width, &m_display_height);

      m_DialogStyleChanged = false;

      //    Get a pointer to the opencpn configuration object
      m_pconfig = GetOCPNConfigObject();

      //    And load the configuration items
      LoadConfig();

      // Get a pointer to the opencpn display canvas, to use as a parent for the GRIB dialog
      m_parent_window = GetOCPNCanvasWindow();

//      int m_height = GetChartbarHeight();
      //    This PlugIn needs a CtrlBar icon, so request its insertion if enabled locally
      if(m_bGRIBShowIcon)
          m_leftclick_tool_id = InsertPlugInTool(_T(""), _img_grib, _img_grib, wxITEM_CHECK,
                                                 _("Grib"), _T(""), NULL,
                                                 GRIB_TOOL_POSITION, 0, this);

      m_bInitIsOK = QualifyCtrlBarPosition( m_CtrlBarxy, m_CtrlBar_Sizexy );

      return (WANTS_OVERLAY_CALLBACK |
              WANTS_OPENGL_OVERLAY_CALLBACK |
              WANTS_CURSOR_LATLON       |
              WANTS_TOOLBAR_CALLBACK    |
              INSTALLS_TOOLBAR_TOOL     |
              WANTS_CONFIG              |
              WANTS_PREFERENCES         |
              WANTS_PLUGIN_MESSAGING    |
              WANTS_MOUSE_EVENTS
            );
}

bool grib_pi::DeInit(void)
{
    if(m_pGribCtrlBar) {
        m_pGribCtrlBar->Close();
        delete m_pGribCtrlBar;
        m_pGribCtrlBar = NULL;
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
Provides basic GRIB file overlay capabilities for several GRIB file types\n\
and a request function to get GRIB files by eMail.\n\n\
Supported GRIB data include:\n\
- wind direction and speed (at 10 m)\n\
- wind gust\n\
- surface pressure\n\
- rainfall\n\
- cloud cover\n\
- significant wave height and direction\n\
- air surface temperature (at 2 m)\n\
- sea surface temperature\n\
- surface current direction and speed\n\
- Convective Available Potential Energy (CAPE)\n\
- wind, altitude, temperature and relative humidity at 300, 500, 700, 850 hPa." );
}

\
void grib_pi::SetDefaults(void)
{
}


int grib_pi::GetToolBarToolCount(void)
{
      return 1;
}

bool grib_pi::MouseEventHook( wxMouseEvent &event )
{
    if( (m_pGribCtrlBar && m_pGribCtrlBar->pReq_Dialog) )
        return m_pGribCtrlBar->pReq_Dialog->MouseEventHook( event );
    return false;
}

void grib_pi::ShowPreferencesDialog( wxWindow* parent )
{
    GribPreferencesDialog *Pref = new GribPreferencesDialog(parent);

    DimeWindow( Pref );                                     //aplly global colours scheme
    SetDialogFont( Pref );                                  //Apply global font

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

         if(m_pGribCtrlBar ) {
             switch( updatelevel ) {
             case 0:
                 break;
             case 3:
                 //rebuild current activefile with new parameters and rebuil data list with current index
                 m_pGribCtrlBar->CreateActiveFileFromName( m_pGribCtrlBar->m_bGRIBActiveFile->GetFileName() );
                 m_pGribCtrlBar->PopulateComboDataList();
                 m_pGribCtrlBar->TimelineChanged();
                 break;
             case 2 :
                 //only rebuild  data list with current index and new timezone
                 m_pGribCtrlBar->PopulateComboDataList();
                 m_pGribCtrlBar->TimelineChanged();
                 break;
             case 1:
                 //only re-compute the best forecast
                 m_pGribCtrlBar->ComputeBestForecastForNow();
                 break;
             }
         }

         SaveConfig();
     }
}

bool grib_pi::QualifyCtrlBarPosition( wxPoint position, wxSize size )
{   // Make sure drag bar (title bar) or grabber always screen
    bool b_reset_pos = false;
#ifdef __WXMSW__
    //  Support MultiMonitor setups which an allow negative window positions.
    //  If the requested window does not intersect any installed monitor,
    //  then default to simple primary monitor positioning.
    RECT frame_title_rect;
    frame_title_rect.left =  position.x;
    frame_title_rect.top =    position.y;
    frame_title_rect.right =  position.x + size.x;
    frame_title_rect.bottom = m_DialogStyle == ATTACHED_HAS_CAPTION ? position.y + 30 : position.y + size.y;


    if(NULL == MonitorFromRect(&frame_title_rect, MONITOR_DEFAULTTONULL))
        b_reset_pos = true;
#else
    wxRect window_title_rect;                    // conservative estimate
    window_title_rect.x = position.x;
    window_title_rect.y = position.y;
    window_title_rect.width = size.x;
    window_title_rect.height = m_DialogStyle == ATTACHED_HAS_CAPTION ? 30 : size.y;

    wxRect ClientRect = wxGetClientDisplayRect();
    if(!ClientRect.Intersects(window_title_rect))
        b_reset_pos = true;

#endif
    return !b_reset_pos;
}

void grib_pi::MoveDialog( wxDialog *dialog, wxPoint position, wxPoint dfault )
{
    wxPoint p = position;
    if( m_DialogStyle != ATTACHED_HAS_CAPTION ) {
        if( !QualifyCtrlBarPosition(p, dialog->GetSize()) )
            p = dfault;
    }
#ifdef __WXGTK__
    dialog->Move(0, 0);
#endif
    dialog->Move(p);
}

void grib_pi::OnToolbarToolCallback(int id)
{
    if( !m_bInitIsOK ) {
        wxMessageDialog mes( m_parent_window, _("The Gribs Dialogs are probably too wide for the screen size\nIn this case, please try a smaller Font size"),
                _("Warning!"), wxOK);
            mes.ShowModal();
        m_bInitIsOK = true;
    }

    bool starting = false;
    if(!m_pGribCtrlBar)
    {
        starting = true;
        long style = m_DialogStyle == ATTACHED_HAS_CAPTION ? wxCAPTION|wxCLOSE_BOX|wxSYSTEM_MENU : wxBORDER_NONE|wxSYSTEM_MENU;
        m_pGribCtrlBar = new GRIBUICtrlBar(m_parent_window, wxID_ANY, wxEmptyString, wxDefaultPosition,
                wxDefaultSize, style, this);
        wxMenu* dummy = new wxMenu(_T("Plugin"));
        wxMenuItem* table = new wxMenuItem( dummy, wxID_ANY, wxString( _("Weather table") ), wxEmptyString, wxITEM_NORMAL );
#ifdef __WXMSW__
        wxFont *qFont = OCPNGetFont(_("Menu"), 10);
        table->SetFont(*qFont);
#endif
        m_MenuItem = AddCanvasContextMenuItem(table, this);
        SetCanvasContextMenuItemViz(m_MenuItem, false);

        // Create the drawing factory
        m_pGRIBOverlayFactory = new GRIBOverlayFactory( *m_pGribCtrlBar );
        m_pGRIBOverlayFactory->SetTimeZone( m_bTimeZone );
        m_pGRIBOverlayFactory->SetParentSize( m_display_width, m_display_height);
        m_pGRIBOverlayFactory->SetSettings( m_bGRIBUseHiDef, m_bGRIBUseGradualColors );

        m_pGribCtrlBar->OpenFile( m_bLoadLastOpenFile == 0 );

    }

    //Toggle GRIB overlay display
    m_bShowGrib = !m_bShowGrib;

    //    Toggle dialog?
    if(m_bShowGrib) {
        if( m_pGribCtrlBar->GetFont() != *OCPNGetFont(_("Dialog"), 10) || starting) {
            SetDialogFont( m_pGribCtrlBar );
            m_pGribCtrlBar->SetDialogsStyleSizePosition( true );
        } else {
            MoveDialog( m_pGribCtrlBar, GetCtrlBarXY(), wxPoint( 20, 60) );
            if( m_DialogStyle >> 1 == SEPARATED ) {
                MoveDialog( m_pGribCtrlBar->GetCDataDialog(), GetCursorDataXY(), wxPoint( 20, 170));
                m_pGribCtrlBar->GetCDataDialog()->Show( m_pGribCtrlBar->m_CDataIsShown );
                }
        }
        m_pGribCtrlBar->Show();
        if( m_pGribCtrlBar->m_bGRIBActiveFile ) {
            if( m_pGribCtrlBar->m_bGRIBActiveFile->IsOK() ) {
                ArrayOfGribRecordSets *rsa = m_pGribCtrlBar->m_bGRIBActiveFile->GetRecordSetArrayPtr();
                if(rsa->GetCount() > 1) SetCanvasContextMenuItemViz( m_MenuItem, true);
            }
        }
        // Toggle is handled by the CtrlBar but we must keep plugin manager b_toggle updated
        // to actual status to ensure correct status upon CtrlBar rebuild
        SetToolbarItemState( m_leftclick_tool_id, m_bShowGrib );
        RequestRefresh(m_parent_window); // refresh main window
    } else
       m_pGribCtrlBar->Close();
}

void grib_pi::OnGribCtrlBarClose()
{
    m_bShowGrib = false;
    SetToolbarItemState( m_leftclick_tool_id, m_bShowGrib );

    m_pGribCtrlBar->Hide();

    SaveConfig();

    SetCanvasContextMenuItemViz(m_MenuItem, false);

    RequestRefresh(m_parent_window); // refresh main window

    if( m_DialogStyleChanged ) {
        delete m_pGribCtrlBar;
        m_pGribCtrlBar = NULL;
        m_DialogStyleChanged = false;
    }
}

bool grib_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    if(!m_pGribCtrlBar ||
       !m_pGribCtrlBar->IsShown() ||
       !m_pGRIBOverlayFactory)
        return false;

    m_pGribCtrlBar->SetViewPort( vp );
    m_pGRIBOverlayFactory->RenderGribOverlay ( dc, vp );
    if( m_pGribCtrlBar->pReq_Dialog )
        m_pGribCtrlBar->pReq_Dialog->RenderZoneOverlay( dc );
    return true;
}

bool grib_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    if(!m_pGribCtrlBar ||
       !m_pGribCtrlBar->IsShown() ||
       !m_pGRIBOverlayFactory)
        return false;

    m_pGribCtrlBar->SetViewPort( vp );
    m_pGRIBOverlayFactory->RenderGLGribOverlay ( pcontext, vp );
    if( m_pGribCtrlBar->pReq_Dialog )
        m_pGribCtrlBar->pReq_Dialog->RenderGlZoneOverlay();
    return true;
}

void grib_pi::SetCursorLatLon(double lat, double lon)
{
    if(m_pGribCtrlBar && m_pGribCtrlBar->IsShown())
        m_pGribCtrlBar->SetCursorLatLon(lat, lon);
}

void grib_pi::OnContextMenuItemCallback(int id)
{
    if(!m_pGribCtrlBar->m_bGRIBActiveFile) return;
    m_pGribCtrlBar->ContextMenuItemCallback(id);
}

void grib_pi::SetDialogFont( wxWindow *dialog, wxFont *font)
{
    dialog->SetFont( *font );
    wxWindowList list = dialog->GetChildren();
    wxWindowListNode *node = list.GetFirst();
    for( size_t i = 0; i < list.GetCount(); i++ ) {
        wxWindow *win = node->GetData();
        win->SetFont( *font );
        node = node->GetNext();
    }
    dialog->Fit();
    dialog->Refresh();
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
        SendTimelineMessage(m_pGribCtrlBar ? m_pGribCtrlBar->TimelineTime() : wxDateTime::Now());
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

        if(!m_pGribCtrlBar)
            OnToolbarToolCallback(0);

        GribTimelineRecordSet *set = m_pGribCtrlBar ? m_pGribCtrlBar->GetTimeLineRecordSet(time) : NULL;

        char ptr[64];
        snprintf(ptr, sizeof ptr, "%p", set);

        v[_T("GribVersionMajor")] = PLUGIN_VERSION_MAJOR;
        v[_T("GribVersionMinor")] = PLUGIN_VERSION_MINOR;
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

    m_CtrlBar_Sizexy.x = pConf->Read ( _T ( "GRIBCtrlBarSizeX" ), 1400L );
    m_CtrlBar_Sizexy.y = pConf->Read ( _T ( "GRIBCtrlBarSizeY" ), 800L );
    m_CtrlBarxy.x =  pConf->Read ( _T ( "GRIBCtrlBarPosX" ), 20L );
    m_CtrlBarxy.y =  pConf->Read ( _T ( "GRIBCtrlBarPosY" ), 60L );
    m_CursorDataxy.x =  pConf->Read ( _T ( "GRIBCursorDataPosX" ),20L );
    m_CursorDataxy.y =  pConf->Read ( _T ( "GRIBCursorDataPosY" ), 170L );

    pConf->Read( _T ( "GribCursorDataDisplayStyle" ), &m_DialogStyle, 0 );
    if( m_DialogStyle > 3 ) m_DialogStyle = 0;         //ensure validity of the .conf value
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

    pConf->Write ( _T ( "GRIBCtrlBarSizeX" ), m_CtrlBar_Sizexy.x );
    pConf->Write ( _T ( "GRIBCtrlBarSizeY" ), m_CtrlBar_Sizexy.y );
    pConf->Write ( _T ( "GRIBCtrlBarPosX" ), m_CtrlBarxy.x );
    pConf->Write ( _T ( "GRIBCtrlBarPosY" ), m_CtrlBarxy.y );
    pConf->Write ( _T ( "GRIBCursorDataPosX" ), m_CursorDataxy.x );
    pConf->Write ( _T ( "GRIBCursorDataPosY" ), m_CursorDataxy.y );

    return true;
}

void grib_pi::SetColorScheme(PI_ColorScheme cs)
{
    DimeWindow(m_pGribCtrlBar);
    if( m_pGribCtrlBar ) {
        if( m_pGRIBOverlayFactory ) m_pGRIBOverlayFactory->ClearCachedLabel();
        if(m_pGribCtrlBar->pReq_Dialog) m_pGribCtrlBar->pReq_Dialog->Refresh();
        m_pGribCtrlBar->Refresh();
        //m_pGribDialog->SetDataBackGroundColor();
    }
}

void grib_pi::SendTimelineMessage(wxDateTime time)
{
    if(!m_pGribCtrlBar)
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
        wxMessageDialog mes(this, _("You have chosen to authorize interpolation.\nDon't forget that data displayed at current time will not be real but Recomputed and this can decrease accuracy!"),
                _("Warning!"), wxOK);
        mes.ShowModal();
    }
}
