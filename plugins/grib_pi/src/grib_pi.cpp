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
#endif //precompiled headers


//#include <wx/treectrl.h>
#include <wx/fileconf.h>
#include "grib_pi.h"
#include "grib.h"
//#include <wx/event.h>

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

}

grib_pi::~grib_pi(void)
{
      delete _img_grib_pi;
      delete _img_grib;
}

int grib_pi::Init(void)
{
//      printf("grib_pi Init()\n");
      AddLocaleCatalog( _T("opencpn-grib_pi") );

      // Set some default private member parameters
      m_grib_dialog_x = 0;
      m_grib_dialog_y = 0;
      m_grib_dialog_sx = 200;
      m_grib_dialog_sy = 200;
      m_pGribDialog = NULL;
      m_pGRIBOverlayFactory = NULL;
      m_bShowGrib = false;

      ::wxDisplaySize(&m_display_width, &m_display_height);

      //    Get a pointer to the opencpn configuration object
      m_pconfig = GetOCPNConfigObject();

      //    And load the configuration items
      LoadConfig();

      // Get a pointer to the opencpn display canvas, to use as a parent for the GRIB dialog
      m_parent_window = GetOCPNCanvasWindow();

      int m_height = GetChartbarHeight();
      //    This PlugIn needs a toolbar icon, so request its insertion if enabled locally
      if(m_bGRIBShowIcon)
            m_leftclick_tool_id  = InsertPlugInTool(_T(""), _img_grib, _img_grib, wxITEM_CHECK,
                  _("Grib"), _T(""), NULL,
                   GRIB_TOOL_POSITION, 0, this);

      // Create the drawing factory
      m_pGRIBOverlayFactory = new GRIBOverlayFactory( m_bGRIBUseHiDef );
      m_pGRIBOverlayFactory->SetTimeZone( m_bTimeZone );
//      wxMenuItem *pmi = new wxMenuItem(NULL, -1, _("PlugIn Item"));
//      int miid = AddCanvasContextMenuItem(pmi, (PlugInCallBackFunction )&s_ContextMenuCallback );
//      SetCanvasContextMenuItemViz(miid, true);

      return (WANTS_OVERLAY_CALLBACK |
           WANTS_OPENGL_OVERLAY_CALLBACK |
           WANTS_CURSOR_LATLON       |
           WANTS_TOOLBAR_CALLBACK    |
           INSTALLS_TOOLBAR_TOOL     |
           WANTS_CONFIG              |
           WANTS_PREFERENCES
            );
}

bool grib_pi::DeInit(void)
{
//      printf("grib_pi DeInit()\n");
      if(m_pGribDialog)
            m_pGribDialog->Close();

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
      return _("GRIB");
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
      // If the config somehow says NOT to show the icon, override it so the user gets good feedback
      if(!m_bGRIBShowIcon)
      {
            m_bGRIBShowIcon = true;

            m_leftclick_tool_id  = InsertPlugInTool(_T(""), _img_grib, _img_grib, wxITEM_NORMAL,
                  _("Grib"), _T(""), NULL,
                   GRIB_TOOL_POSITION, 0, this);
      }
}


int grib_pi::GetToolbarToolCount(void)
{
      return 1;
}

void grib_pi::ShowPreferencesDialog( wxWindow* parent )
{
     GribPreferencesDialog *Pref_Dialog = new GribPreferencesDialog( parent, wxID_ANY,
         m_bGRIBUseHiDef, m_bCopyFirstCumRec, m_bCopyMissWaveRec, m_bSpeedUnit, m_bPressUnit, m_bTimeZone, m_bMailAdresse );
        

     if( Pref_Dialog->ShowModal() == wxID_OK ) {
         int updatelevel = 0;
         if( m_bSpeedUnit != Pref_Dialog->GetSpeedUnit() ) {
             m_bSpeedUnit = Pref_Dialog->GetSpeedUnit();
             updatelevel = 1;
         }
         if( m_bPressUnit != Pref_Dialog->GetPressUnit() ) {
             m_bPressUnit = Pref_Dialog->GetPressUnit();
             updatelevel = 1;
         }
      if( m_bTimeZone != Pref_Dialog->GetTimeZone() ) {
           m_bTimeZone = Pref_Dialog->GetTimeZone();
           m_pGRIBOverlayFactory->SetTimeZone( m_bTimeZone );
           updatelevel = 2;
      }
         if( m_bCopyFirstCumRec != Pref_Dialog->GetCopyFirstCumRec() 
                || m_bCopyMissWaveRec != Pref_Dialog->GetCopyMissWaveRec() ) {
             m_bCopyFirstCumRec = Pref_Dialog->GetCopyFirstCumRec();
             m_bCopyMissWaveRec = Pref_Dialog->GetCopyMissWaveRec();
             updatelevel = 3;
         }
         if(m_pGribDialog ) {
             switch( updatelevel ) {
             case 0:
                 break;                                          
             case 3:
                 //rebuild current activefile with new parameters and rebuil data list with current index
                 if( m_pGribDialog->HasValidData() ) {            
                     m_pGribDialog->CreateActiveFileFromName( m_pGribDialog->m_bGRIBActiveFile->GetFileName() );  
                     m_pGribDialog->PopulateComboDataList( m_pGribDialog->GetActiveForecastIndex() );
                     m_pGribDialog->DisplayDataGRS();
                 }
                 break;
             case 2 :
                 //only rebuild  data list with current index and new timezone
                 if( m_pGribDialog->HasValidData() ) {            
                    m_pGribDialog->PopulateComboDataList( m_pGribDialog->GetActiveForecastIndex() );
                    m_pGribDialog->DisplayDataGRS();
                 }
                 break;
             case 1:
                 //only update panel display with new speed unit
                 m_pGribDialog->UpdateTrackingControls();         
             }
         }
         m_bGRIBUseHiDef= Pref_Dialog->GetGRIBUseHiDef();

         SaveConfig();
     }
}

void grib_pi::OnToolbarToolCallback(int id)
{
//      printf("grib_pi ToolCallBack()\n");
//     ::wxBell();

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

      //Eventually create the GRIB dialog
      if(NULL == m_pGribDialog)
      {
           CreateGribDialog( 0, wxT(""), true );                               //create new dialog with 'zero' index ( younger file ) and
      }                                                                  //best forecast

      //    Toggle dialog?
      if(m_bShowGrib) {
          m_pGribDialog->Show();
          m_pGribDialog->DisplayDataGRS();   
      } else 
          m_pGribDialog->Close();

      // Toggle is handled by the toolbar but we must keep plugin manager b_toggle updated
      // to actual status to ensure correct status upon toolbar rebuild
      SetToolbarItemState( m_leftclick_tool_id, m_bShowGrib );


}

void grib_pi::OnGribDialogClose()
{
      SetToolbarItemState( m_leftclick_tool_id, false );

      m_pGribDialog->Hide();
      m_pGribDialog->SetGribRecordSet( NULL );          //clear the screen

      m_bShowGrib = false;

//      if(m_pGRIBOverlayFactory)
//            m_pGRIBOverlayFactory->Reset();

      SaveConfig();
}

bool grib_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
      if(m_pGribDialog) m_pGribDialog->SetViewPort( vp );
      if(m_bShowGrib && m_pGRIBOverlayFactory)
      {
            if(m_pGRIBOverlayFactory->IsReadyToRender())
            {
                m_pGRIBOverlayFactory->RenderGribOverlay ( dc, vp );
                return true;
            }
            else
                  return false;
      }
      else
            return false;
}

bool grib_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
      if(m_pGribDialog) m_pGribDialog->SetViewPort( vp );
      if(m_bShowGrib && m_pGRIBOverlayFactory)
      {
            if(m_pGRIBOverlayFactory->IsReadyToRender())
            {
                m_pGRIBOverlayFactory->RenderGLGribOverlay ( pcontext, vp );
                return true;
            }
            else
                  return false;
      }
      else
            return false;

}




void grib_pi::SetCursorLatLon(double lat, double lon)
{
      if(m_pGribDialog)
      {
            m_pGribDialog->SetCursorLatLon(lat, lon);
      }
}


bool grib_pi::LoadConfig(void)
{
      wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

      if(pConf)
      {
            pConf->SetPath ( _T( "/PlugIns/GRIB" ) );
            pConf->Read ( _T( "ShowGRIBIcon" ), &m_bGRIBShowIcon, 1 );
            pConf->Read ( _T( "GRIBUseHiDef" ), &m_bGRIBUseHiDef, 0 );
            pConf->Read ( _T( "GRIBSpeedUnit" ), &m_bSpeedUnit, 2 );
            pConf->Read ( _T( "GRIBPressureUnit" ), &m_bPressUnit, 1 );
            pConf->Read ( _T( "GRIBTimeZone" ), &m_bTimeZone, 1 );
            pConf->Read ( _T( "CopyFirstCumulativeRecord" ), &m_bCopyFirstCumRec, 1 );
            pConf->Read ( _T( "CopyMissingWaveRecord" ), &m_bCopyMissWaveRec, 1 );
            pConf->Read ( _T( "GRIBdataConfig" ), &m_grib_DataConfig, _T( "0XXXXXXXXXX" ) );
            pConf->Read ( _T( "MailRequestConfig" ), &m_grib_RequestConfig, _T( "00220XX....." ) );
            pConf->Read ( _T( "MailRequestAdesse" ), &m_bMailAdresse, _T("query@saildocs.com") );


            //if GriDataConfig has been corrupted , take the standard one to fix a crash
            if( m_grib_DataConfig.Len() != wxString (_T( "0XXXXXXXXXX" ) ).Len() )
                m_grib_DataConfig = _T( "0XXXXXXXXXX" );

            m_grib_dialog_sx = pConf->Read ( _T ( "GRIBDialogSizeX" ), 300L );
            m_grib_dialog_sy = pConf->Read ( _T ( "GRIBDialogSizeY" ), 540L );
            m_grib_dialog_x =  pConf->Read ( _T ( "GRIBDialogPosX" ), 20L );
            m_grib_dialog_y =  pConf->Read ( _T ( "GRIBDialogPosY" ), 170L );

//            if((m_grib_dialog_x < 0) || (m_grib_dialog_x > m_display_width))
//                  m_grib_dialog_x = 5;
//            if((m_grib_dialog_y < 0) || (m_grib_dialog_y > m_display_height))
//                  m_grib_dialog_y = 5;

            wxStandardPaths spath;
            pConf->SetPath ( _T ( "/Directories" ) );
            pConf->Read ( _T ( "GRIBDirectory" ), &m_grib_dir , spath.GetDocumentsDir() );

            return true;
      }
      else
            return false;
}

bool grib_pi::SaveConfig(void)
{
      wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

      if(pConf)
      {
            pConf->SetPath ( _T( "/PlugIns/GRIB" ) );
            pConf->Write ( _T ( "ShowGRIBIcon" ), m_bGRIBShowIcon );
            pConf->Write ( _T ( "GRIBUseHiDef" ), m_bGRIBUseHiDef );
            pConf->Write ( _T ( "GRIBSpeedUnit" ), m_bSpeedUnit );
            pConf->Write ( _T ( "GRIBPressureUnit" ), m_bPressUnit );
            pConf->Write ( _T ( "GRIBTimeZone" ), m_bTimeZone );
            pConf->Write ( _T ( "CopyFirstCumulativeRecord" ), m_bCopyFirstCumRec );
            pConf->Write ( _T ( "CopyMissingWaveRecord" ), m_bCopyMissWaveRec );
            pConf->Write ( _T ( "GRIBdataConfig" ), m_grib_DataConfig );
            pConf->Write ( _T ( "MailRequestConfig" ), m_grib_RequestConfig );
            pConf->Write ( _T( "MailRequestAdesse" ), m_bMailAdresse );

            pConf->Write ( _T ( "GRIBDialogSizeX" ), m_grib_dialog_sx );
            pConf->Write ( _T ( "GRIBDialogSizeY" ), m_grib_dialog_sy );
            pConf->Write ( _T ( "GRIBDialogPosX" ), m_grib_dialog_x );
            pConf->Write ( _T ( "GRIBDialogPosY" ), m_grib_dialog_y );

            pConf->SetPath ( _T ( "/Directories" ) );
            pConf->Write ( _T ( "GRIBDirectory" ), m_grib_dir );

            return true;
      }
      else
            return false;
}

void grib_pi::SetColorScheme(PI_ColorScheme cs)
{

      DimeWindow(m_pGribDialog);

}

void grib_pi::CreateGribDialog( int index, wxString filename, bool newfile )
{
    if( m_pGribDialog ) m_pGribDialog->Destroy();
 
    m_pGribDialog = new GRIBUIDialog();

    m_pGribDialog->Create ( m_parent_window, this, -1, _("GRIB Display Control"), wxPoint( m_grib_dialog_x, m_grib_dialog_y ),
        wxSize( m_grib_dialog_sx, m_grib_dialog_sy) );

    if( newfile ) {
    //find younger file , build data list and get best forecast for "now
    m_pGribDialog->GetFirstrFileInDirectory();               
    }
    else {
        //only recreate the same file and rebuilt data list with current index
        m_pGribDialog->CreateActiveFileFromName( filename );      
        m_pGribDialog->PopulateComboDataList( index );
    }
    m_pGribDialog->DisplayDataGRS();                                        
    m_pGribDialog->Show();
}

//-------------------------------------------------------------------------------------------------------
//Preferences Dialog implementation
//---------------------------------------------------------------------------------------------------------
GribPreferencesDialog::GribPreferencesDialog( wxWindow *pparent, wxWindowID id, bool init_HiDef,
    bool init_CumRec, bool init_WaveRec, int init_SpeedUnit, int init_PressUnit, int init_TimeFormat, wxString init_MailAdresse ):
    wxDialog( pparent, id, _("GRIB Preferences"), wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE )
{
    int border_size = 4;

    wxBoxSizer* itemBoxSizerGRIBPanel = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizerGRIBPanel);

    //  Grib Setting
    wxStaticBox* itemStaticBoxSizerGRIBStatic = new wxStaticBox(this, wxID_ANY, _("GRIB"));
    wxStaticBoxSizer* itemStaticBoxSizerGRIB = new wxStaticBoxSizer(itemStaticBoxSizerGRIBStatic, wxVERTICAL);
    itemBoxSizerGRIBPanel->Add(itemStaticBoxSizerGRIB, 0, wxGROW|wxALL, border_size);

    m_pGRIBUseHiDef = new wxCheckBox( this, -1, _("Use High Definition Graphics"));
    itemStaticBoxSizerGRIB->Add(m_pGRIBUseHiDef, 1, wxALIGN_LEFT|wxALL, border_size );

    m_pCopyFirstCumRec = new wxCheckBox( this, -1, _("Copy first Cumulative missing Record"));
    itemStaticBoxSizerGRIB->Add(m_pCopyFirstCumRec, 1, wxALIGN_LEFT|wxALL, border_size );

    m_pCopyMissWaveRec = new wxCheckBox( this, -1, _("Copy missing Wave Records"));
    itemStaticBoxSizerGRIB->Add(m_pCopyMissWaveRec, 1, wxALIGN_LEFT|wxALL, border_size );

    //Unit
    wxStaticBox* itemStaticBoxSizerUnitStatic = new wxStaticBox(this, wxID_ANY, _("Units"));
    wxStaticBoxSizer* itemStaticBoxSizerUnit = new wxStaticBoxSizer(itemStaticBoxSizerUnitStatic, wxVERTICAL);
    itemBoxSizerGRIBPanel->Add(itemStaticBoxSizerUnit, 0, wxGROW|wxALL, border_size);

    wxString unit_s[] = { _T("m/s"), _T("km/h"), _T("kts") };
    m_pSpeedUnit = new wxRadioBox( this, -1, _("Speed unit"), wxDefaultPosition, wxSize(-1, -1),
        3, unit_s, 1, wxRA_SPECIFY_ROWS );
    itemStaticBoxSizerUnit->Add(m_pSpeedUnit, 1, wxALIGN_LEFT|wxALL, border_size );

    wxString unit_p[] = { _T("mmHg"), _T("mBa") };
    m_pPressUnit = new wxRadioBox( this, -1, _("Pressure unit"), wxDefaultPosition, wxSize(-1, -1),
        2, unit_p, 1, wxRA_SPECIFY_ROWS );
    itemStaticBoxSizerUnit->Add(m_pPressUnit, 1, wxALIGN_LEFT|wxALL, border_size );

    wxString zone[] = { _("Local Time"), _("UTC"), };
    m_pTimeZone = new wxRadioBox( this, -1, _("Time Zone"), wxDefaultPosition, wxSize(-1, -1),
         2, zone, 1, wxRA_SPECIFY_ROWS );
    itemStaticBoxSizerUnit->Add(m_pTimeZone, 1, wxALIGN_LEFT|wxALL, border_size);

    m_pTimeZone->SetSelection( init_TimeFormat );
    m_pGRIBUseHiDef->SetValue( init_HiDef );
    m_pSpeedUnit->SetSelection( init_SpeedUnit );
    m_pPressUnit->SetSelection( init_PressUnit );
    m_pCopyFirstCumRec->SetValue( init_CumRec );
    m_pCopyMissWaveRec->SetValue( init_WaveRec );

    wxStdDialogButtonSizer* DialogButtonSizer = this->CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    itemBoxSizerGRIBPanel->Add(DialogButtonSizer, 0, wxALIGN_RIGHT|wxALL, 5);

    this->Fit();
}