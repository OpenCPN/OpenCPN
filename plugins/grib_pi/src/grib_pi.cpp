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

#include <wx/treectrl.h>
#include <wx/fileconf.h>

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

}

grib_pi::~grib_pi(void)
{
      delete _img_grib_pi;
      delete _img_grib;
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

      //    This PlugIn needs a toolbar icon, so request its insertion if enabled locally
      m_leftclick_tool_id  = InsertPlugInTool(_T(""), _img_grib, _img_grib, wxITEM_CHECK,
                                              _("Grib"), _T(""), NULL,
                                              GRIB_TOOL_POSITION, 0, this);

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
}


int grib_pi::GetToolbarToolCount(void)
{
      return 1;
}

void grib_pi::ShowPreferencesDialog( wxWindow* parent )
{
      wxDialog *dialog = new wxDialog( parent, wxID_ANY, _("GRIB Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE );

    //      Build GRIB. Page for Toolbox
    int border_size = 4;

    wxBoxSizer* itemBoxSizerGRIBPanel = new wxBoxSizer(wxVERTICAL);
    dialog->SetSizer(itemBoxSizerGRIBPanel);

    //  Grib toolbox icon checkbox
    wxStaticBox* itemStaticBoxSizerGRIBStatic = new wxStaticBox(dialog, wxID_ANY, _("GRIB"));
    wxStaticBoxSizer* itemStaticBoxSizerGRIB = new wxStaticBoxSizer(itemStaticBoxSizerGRIBStatic, wxVERTICAL);
    itemBoxSizerGRIBPanel->Add(itemStaticBoxSizerGRIB, 0, wxGROW|wxALL, border_size);

    m_pGRIBUseHiDef = new wxCheckBox( dialog, -1, _("Use High Definition Graphics"));
    itemStaticBoxSizerGRIB->Add(m_pGRIBUseHiDef, 1, wxALIGN_LEFT|wxALL, border_size);

    m_pGRIBUseGradualColors = new wxCheckBox( dialog, -1, _("Use gradual colors blended (wave height etc) instead of step at set heights."));
    itemStaticBoxSizerGRIB->Add(m_pGRIBUseGradualColors, 1, wxALIGN_LEFT|wxALL, border_size);


    m_pGRIBUseHiDef->SetValue(m_bGRIBUseHiDef);
    m_pGRIBUseGradualColors->SetValue(m_bGRIBUseGradualColors);

    wxStdDialogButtonSizer* DialogButtonSizer = dialog->CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    itemBoxSizerGRIBPanel->Add(DialogButtonSizer, 0, wxALIGN_RIGHT|wxALL, 5);

    dialog->Fit();

    if(dialog->ShowModal() == wxID_OK)
    {
        m_bGRIBUseHiDef= m_pGRIBUseHiDef->GetValue();
        m_bGRIBUseGradualColors= m_pGRIBUseGradualColors->GetValue();
        if(m_pGRIBOverlayFactory)
            m_pGRIBOverlayFactory->SetSettings( m_bGRIBUseHiDef, m_bGRIBUseGradualColors );
        SaveConfig();
    }
}

void grib_pi::OnToolbarToolCallback(int id)
{
    if(!m_pGribDialog)
    {
        m_pGribDialog = new GRIBUIDialog(m_parent_window, this);
        m_pGribDialog->Move(wxPoint(m_grib_dialog_x, m_grib_dialog_y));

        // Create the drawing factory
        m_pGRIBOverlayFactory = new GRIBOverlayFactory( *m_pGribDialog );
        m_pGRIBOverlayFactory->SetSettings( m_bGRIBUseHiDef, m_bGRIBUseGradualColors );
    }

    m_pGribDialog->Show(!m_pGribDialog->IsShown());

    // Toggle is handled by the toolbar but we must keep plugin manager b_toggle updated
    // to actual status to ensure correct status upon toolbar rebuild
    SetToolbarItemState( m_leftclick_tool_id, m_pGribDialog->IsShown() );

    wxPoint p = m_pGribDialog->GetPosition();
    m_pGribDialog->Move(0,0);        // workaround for gtk autocentre dialog behavior
    m_pGribDialog->Move(p);
}

void grib_pi::OnGribDialogClose()
{
    SetToolbarItemState( m_leftclick_tool_id, false );

    if(m_pGribDialog)
        m_pGribDialog->Show(false);

    SaveConfig();
}

bool grib_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    if(!m_pGribDialog ||
       !m_pGribDialog->IsShown() ||
       !m_pGRIBOverlayFactory)
        return false;

    m_pGRIBOverlayFactory->RenderGribOverlay ( dc, vp );
    return true;
}

bool grib_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    if(!m_pGribDialog ||
       !m_pGribDialog->IsShown() ||
       !m_pGRIBOverlayFactory)
        return false;

    m_pGRIBOverlayFactory->RenderGLGribOverlay ( pcontext, vp );
    return true;
}

void grib_pi::SetCursorLatLon(double lat, double lon)
{
    if(m_pGribDialog)
        m_pGribDialog->SetCursorLatLon(lat, lon);
}

bool grib_pi::LoadConfig(void)
{
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

    if(!pConf)
        return false;

    pConf->SetPath ( _T( "/Settings" ) );
    pConf->Read ( _T( "GRIBUseHiDef" ),  &m_bGRIBUseHiDef, 0 );
    pConf->Read ( _T( "GRIBUseGradualColors" ),     &m_bGRIBUseGradualColors, 0 );

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

    pConf->SetPath ( _T ( "/Settings" ) );
    pConf->Write ( _T ( "GRIBUseHiDef" ), m_bGRIBUseHiDef );
    pConf->Write ( _T ( "GRIBUseGradualColors" ),    m_bGRIBUseGradualColors );

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
