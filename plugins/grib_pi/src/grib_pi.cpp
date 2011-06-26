/******************************************************************************
 * $Id: grib_pi.cpp,v 1.8 2010/06/21 01:54:37 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin
 * Author:   David Register
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


#include <wx/treectrl.h>
#include <wx/fileconf.h>


#include "grib_pi.h"
#include "grib.h"


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
      :opencpn_plugin(ppimgr)
{
      // Create the PlugIn icons
      initialize_images();

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

      ::wxDisplaySize(&m_display_width, &m_display_height);

      //    Get a pointer to the opencpn configuration object
      m_pconfig = GetOCPNConfigObject();

      //    And load the configuration items
      LoadConfig();

      // Get a pointer to the opencpn display canvas, to use as a parent for the GRIB dialog
      m_parent_window = GetOCPNCanvasWindow();

      //    This PlugIn needs a toolbar icon, so request its insertion if enabled locally
      if(m_bGRIBShowIcon)
            m_leftclick_tool_id  = InsertPlugInTool(_T(""), _img_grib, _img_grib, wxITEM_NORMAL,
                  _("Grib"), _T(""), NULL,
                   GRIB_TOOL_POSITION, 0, this);

      // Create the drawing factory
      m_pGRIBOverlayFactory = new GRIBOverlayFactory;

//      wxMenuItem *pmi = new wxMenuItem(NULL, -1, _("PlugIn Item"));
//      int miid = AddCanvasContextMenuItem(pmi, (PlugInCallBackFunction )&s_ContextMenuCallback );
//      SetCanvasContextMenuItemViz(miid, true);

      return (WANTS_OVERLAY_CALLBACK |
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
      wxDialog *dialog = new wxDialog( parent, wxID_ANY, _("GRIB Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE );

    //      Build GRIB. Page for Toolbox
    int border_size = 4;

    wxBoxSizer* itemBoxSizerGRIBPanel = new wxBoxSizer(wxVERTICAL);
    dialog->SetSizer(itemBoxSizerGRIBPanel);

    //  Grib toolbox icon checkbox
    wxStaticBox* itemStaticBoxSizerGRIBStatic = new wxStaticBox(dialog, wxID_ANY, _("GRIB"));
    wxStaticBoxSizer* itemStaticBoxSizerGRIB = new wxStaticBoxSizer(itemStaticBoxSizerGRIBStatic, wxVERTICAL);
    itemBoxSizerGRIBPanel->Add(itemStaticBoxSizerGRIB, 0, wxGROW|wxALL, border_size);

    m_pGRIBShowIcon = new wxCheckBox( dialog, -1, _("Show GRIB icon"), wxDefaultPosition, wxSize(-1, -1), 0 );
    itemStaticBoxSizerGRIB->Add(m_pGRIBShowIcon, 1, wxALIGN_LEFT|wxALL, border_size);

    m_pGRIBUseHiDef = new wxCheckBox( dialog, -1, _("Use High Definition Graphics"));
    itemStaticBoxSizerGRIB->Add(m_pGRIBUseHiDef, 1, wxALIGN_LEFT|wxALL, border_size);

    m_pGRIBShowIcon->SetValue(m_bGRIBShowIcon);
    m_pGRIBUseHiDef->SetValue(m_bGRIBUseHiDef);

      wxStdDialogButtonSizer* DialogButtonSizer = dialog->CreateStdDialogButtonSizer(wxOK|wxCANCEL);
      itemBoxSizerGRIBPanel->Add(DialogButtonSizer, 0, wxALIGN_RIGHT|wxALL, 5);

      dialog->Fit();

      if(dialog->ShowModal() == wxID_OK)
      {
            //    Show Icon changed value?
            if(m_bGRIBShowIcon != m_pGRIBShowIcon->GetValue())
            {
                  m_bGRIBShowIcon= m_pGRIBShowIcon->GetValue();

                  if(m_bGRIBShowIcon)
                        m_leftclick_tool_id  = InsertPlugInTool(_T(""), _img_grib, _img_grib, wxITEM_NORMAL,
                              _("Grib"), _T(""), NULL, GRIB_TOOL_POSITION,
                              0, this);
                  else
                        RemovePlugInTool(m_leftclick_tool_id);
            }

            m_bGRIBUseHiDef= m_pGRIBUseHiDef->GetValue();

            SaveConfig();
      }

}

void grib_pi::OnToolbarToolCallback(int id)
{
//      printf("grib_pi ToolCallBack()\n");
//     ::wxBell();

      // show the GRIB dialog
      if(NULL == m_pGribDialog)
      {
            m_pGribDialog = new GRIBUIDialog();
            m_pGribDialog->Create ( m_parent_window, this, -1, _("GRIB Display Control"), m_grib_dir,
                               wxPoint( m_grib_dialog_x, m_grib_dialog_y), wxSize( m_grib_dialog_sx, m_grib_dialog_sy));
      }

      m_pGribDialog->Show();                        // Show modeless, so it stays on the screen

}





void grib_pi::OnGribDialogClose()
{
      m_pGribDialog = NULL;
      if(m_pGRIBOverlayFactory)
            m_pGRIBOverlayFactory->Reset();
      SaveConfig();
}

bool grib_pi::RenderOverlay(wxMemoryDC *pmdc, PlugIn_ViewPort *vp)
{
      if(m_pGribDialog && m_pGRIBOverlayFactory)
      {
            if(m_pGRIBOverlayFactory->IsReadyToRender())
            {
                  m_pGRIBOverlayFactory->RenderGribOverlay ( pmdc, vp );
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
            pConf->SetPath ( _T( "/Settings" ) );
            pConf->Read ( _T( "GRIBUseHiDef" ),  &m_bGRIBUseHiDef, 0 );
            pConf->Read ( _T( "ShowGRIBIcon" ),  &m_bGRIBShowIcon, 1 );


            m_grib_dialog_sx = pConf->Read ( _T ( "GRIBDialogSizeX" ), 300L );
            m_grib_dialog_sy = pConf->Read ( _T ( "GRIBDialogSizeY" ), 540L );
            m_grib_dialog_x =  pConf->Read ( _T ( "GRIBDialogPosX" ), 20L );
            m_grib_dialog_y =  pConf->Read ( _T ( "GRIBDialogPosY" ), 170L );

//            if((m_grib_dialog_x < 0) || (m_grib_dialog_x > m_display_width))
//                  m_grib_dialog_x = 5;
//            if((m_grib_dialog_y < 0) || (m_grib_dialog_y > m_display_height))
//                  m_grib_dialog_y = 5;

            pConf->SetPath ( _T ( "/Directories" ) );
            pConf->Read ( _T ( "GRIBDirectory" ), &m_grib_dir );

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
            pConf->SetPath ( _T ( "/Settings" ) );
            pConf->Write ( _T ( "GRIBUseHiDef" ), m_bGRIBUseHiDef );
            pConf->Write ( _T ( "ShowGRIBIcon" ), m_bGRIBShowIcon );

            pConf->Write ( _T ( "GRIBDialogSizeX" ),  m_grib_dialog_sx );
            pConf->Write ( _T ( "GRIBDialogSizeY" ),  m_grib_dialog_sy );
            pConf->Write ( _T ( "GRIBDialogPosX" ),   m_grib_dialog_x );
            pConf->Write ( _T ( "GRIBDialogPosY" ),   m_grib_dialog_y );

            pConf->SetPath ( _T ( "/Directories" ) );
            pConf->Write ( _T ( "GRIBDirectory" ), m_grib_dir );

            return true;
      }
      else
            return false;
}
