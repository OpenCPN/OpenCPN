/******************************************************************************
 * $Id:  $
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   Johan van der Sman
 *
 ***************************************************************************
 *   Copyright (C) 2015 Johan van der Sman                                 *
 *   johan.sman@gmail.com                                                  *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */
#include <wx/wxprec.h>

#ifndef  WX_PRECOMP
  #include <wx/wx.h>
#endif //precompiled headers

#include <wx/fileconf.h>
#include "aisradar_pi.h"

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr) {
    return new aisradar_pi(ppimgr);
}


extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) {
    delete p;
}


//---------------------------------------------------------------------------------------------------------
//
//    Radar PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------
#include "my_icons.h"

//#include "icons.h"

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

aisradar_pi::aisradar_pi(void *ppimgr) 
: opencpn_plugin_19(ppimgr), 
    m_pconfig(0), 
    m_parent_window(0),
    m_pRadarFrame(0),
    AisTargets(0),
    m_display_width(0), 
    m_display_height(0),
    m_leftclick_tool_id(0),
    m_radar_frame_x(0), 
    m_radar_frame_y(0),
    m_radar_frame_sx(0), 
    m_radar_frame_sy(0),
    m_radar_range(0),
    m_lat(0.0),
    m_lon(0.0),
    m_cog(0.0),
    m_sog(0.0),
    m_sats(0),
    m_radar_show_icon(true),
    m_radar_use_ais(true),
    m_radar_north_up(false),
    m_pRadarShowIcon(0),	
    m_pRadarUseAis(0)
{
    initialize_my_images();
}


aisradar_pi::~aisradar_pi() {
    if ( AisTargets ) {
        WX_CLEAR_ARRAY(*AisTargets);     
        delete AisTargets;
        AisTargets=0;
    }
}


int aisradar_pi::Init(void) {
    AddLocaleCatalog( _T("opencpn-aisradar_pi") );
    m_radar_frame_x = m_radar_frame_y = 0;
    m_radar_frame_sx = m_radar_frame_sy = 200;
    m_pRadarFrame = 0;
    m_lat= m_lon=0.;
    m_cog= m_sog=0.;
    m_sats=0;
    ::wxDisplaySize(&m_display_width, &m_display_height);
    m_pconfig = GetOCPNConfigObject();
    LoadConfig();
    if (AisTargets) {  // Init may be called more than once, check for cleanup
        WX_CLEAR_ARRAY(*AisTargets);     
        delete AisTargets;
	}
	AisTargets = GetAISTargetArray();
    m_parent_window = GetOCPNCanvasWindow();
    if(m_radar_show_icon) {
        m_leftclick_tool_id  = InsertPlugInTool(_T(""), 
            _img_radar, 
            _img_radar, 
            wxITEM_NORMAL, 
			wxString::Format(_T("AIS Radar View %d.%d"), 
                PLUGIN_VERSION_MAJOR, 
                PLUGIN_VERSION_MINOR
            ), 
			_T(""), 
            0,
            RADAR_TOOL_POSITION, 
            0,
            this
        );
    }
    return (WANTS_TOOLBAR_CALLBACK | INSTALLS_TOOLBAR_TOOL |
         WANTS_CONFIG | WANTS_PREFERENCES | WANTS_AIS_SENTENCES  |
         WANTS_NMEA_EVENTS | WANTS_PLUGIN_MESSAGING | USES_AUI_MANAGER
    );
}


bool aisradar_pi::DeInit(void) {
    if(m_pRadarFrame) {
        m_pRadarFrame->Close();
    }
    return true;
}


int aisradar_pi::GetAPIVersionMajor() {
    return MY_API_VERSION_MAJOR;
}


int aisradar_pi::GetAPIVersionMinor() {
    return MY_API_VERSION_MINOR;
}


int aisradar_pi::GetPlugInVersionMajor() {
    return PLUGIN_VERSION_MAJOR;
}


int aisradar_pi::GetPlugInVersionMinor() {
    return PLUGIN_VERSION_MINOR;
}


wxBitmap *aisradar_pi::GetPlugInBitmap() {
    return _img_aisradar_pi;
}


wxString aisradar_pi::GetCommonName() {
    return _("AIS Radar view");
}


wxString aisradar_pi::GetShortDescription() {
    return _("AIS Radar view PlugIn");
}


wxString aisradar_pi::GetLongDescription() {
    return _("Radar PlugIn for OpenCPN\nShows AIS targets in a radar style view \n\n");
}


void aisradar_pi::SetDefaults(void) {
    if(!m_radar_show_icon) {
        m_radar_show_icon = true;
        m_leftclick_tool_id  = InsertPlugInTool(_T(""), _img_radar, _img_radar, wxITEM_NORMAL,
              _("AIS Radar"), _T(""), 0,
               RADAR_TOOL_POSITION, 0, this
        );
    }
}


int aisradar_pi::GetToolbarToolCount(void) {
    return 1;
}


void aisradar_pi::ShowPreferencesDialog( wxWindow* parent ) {
    wxDialog *dialog = new wxDialog( parent, wxID_ANY, _("Radar Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE );
    int border_size = 4;

    wxColor     cl;
    GetGlobalColor(_T("DILG1"), &cl);
    dialog->SetBackgroundColour(cl);

    wxBoxSizer* PanelSizer = new wxBoxSizer(wxVERTICAL);
    dialog->SetSizer(PanelSizer);

    //  Radar toolbox icon checkbox
    wxStaticBox* RadarBox = new wxStaticBox(dialog, wxID_ANY, _("Radar"));
    wxStaticBoxSizer* RadarBoxSizer = new wxStaticBoxSizer(RadarBox, wxVERTICAL);
    PanelSizer->Add(RadarBoxSizer, 0, wxGROW|wxALL, border_size);

    m_pRadarShowIcon = new wxCheckBox( dialog, -1, _("Show RADAR icon:"), wxDefaultPosition, wxSize(-1, -1), 0 );
    RadarBoxSizer->Add(m_pRadarShowIcon, 1, wxALIGN_LEFT|wxALL, border_size);
    m_pRadarShowIcon->SetValue(m_radar_show_icon);

    m_pRadarUseAis = new wxCheckBox( dialog, -1, _("Use AIS as radar source:"), wxDefaultPosition, wxSize(-1, -1), 0 );
    RadarBoxSizer->Add(m_pRadarUseAis, 2, wxALIGN_LEFT|wxALL, border_size);
    m_pRadarUseAis->SetValue(m_radar_use_ais);

    wxStdDialogButtonSizer* DialogButtonSizer = dialog->CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    PanelSizer->Add(DialogButtonSizer, 0, wxALIGN_RIGHT|wxALL, 5);
    dialog->Fit();

    if(dialog->ShowModal() == wxID_OK)       {
         //    Show Icon changed value?
         if(m_radar_show_icon != m_pRadarShowIcon->GetValue()) {
              m_radar_show_icon= m_pRadarShowIcon->GetValue();
              if(m_radar_show_icon) {
                  m_leftclick_tool_id  = InsertPlugInTool(_T(""), _img_radar, _img_radar, wxITEM_NORMAL,
                      _("Radar"), _T(""), 0, RADAR_TOOL_POSITION,
                      0, this);
              } else {
                   RemovePlugInTool(m_leftclick_tool_id);
              }
         }
         m_radar_use_ais    = m_pRadarUseAis->GetValue();
         SaveConfig();
    }
}


void aisradar_pi::OnToolbarToolCallback(int id) {
   ::wxBell();
    if(!m_pRadarFrame) {
        m_pRadarFrame = new RadarFrame();
        m_pRadarFrame->Create ( m_parent_window, 
            this, 
            -1,
			wxString::Format(_T("AIS Radar View %d.%d"), 
                PLUGIN_VERSION_MAJOR, 
                PLUGIN_VERSION_MINOR
            ),
            wxPoint( m_radar_frame_x, m_radar_frame_y), 
            wxSize( m_radar_frame_sx, m_radar_frame_sy)
        );
        m_pRadarFrame->Show();
    } else {
        m_pRadarFrame->Close(true);
    }
}


void aisradar_pi::SetAISSentence(wxString &sentence) {
    // Ignore the AIS message itself. It is merely used as an event
    // to retrieve the AIS targets from the mainprogram
    // The targets are already updated to reflect the current message
    // So we re-use that information
    if (m_radar_use_ais) {
        GetAisTargets();
    }
    if ( m_pRadarFrame ) {
        m_pRadarFrame->Refresh();
    }
}


void aisradar_pi::SetPositionFix(PlugIn_Position_Fix &pfix) {
    // Check if our position changed
    // If so, update view otherwise not, 
    if ( m_lat != pfix.Lat || m_lon != pfix.Lon || m_cog != pfix.Cog || m_sog != pfix.Sog ) {
        m_lat  = pfix.Lat;
        m_lon  = pfix.Lon;
		if (pfix.Cog>=0.0) {
            m_cog  = pfix.Cog;
		}
		if (pfix.Sog>=0.0) {
            m_sog  = pfix.Sog;
		}
        m_sats = pfix.nSats;
        if ( m_pRadarFrame ) {
            m_pRadarFrame->Refresh();
        }
    }
}


void aisradar_pi::SetPluginMessage(wxString &message_id, wxString &message_body) {
// Parse message with radar targets
// Format:
//    {
//        "RadarTargets" :
//        {
//            "Source"      : "BR24_pi",
//          "Orientation" : "North",
//            "TTL"         : 30,
//            "Targets" : 
//            [
//                { "Brg" : 180, Range : 1.2 },
//                { "Brg" : 359, Range : 8.3 },
//                { "Brg" : 34, Range : 0.9 },
//                { "Brg" : 13, Range : 0.4 },
//                { "Brg" : 57, Range : 0.7 }
//            ]
//        }
//    }
//  TODO: implement parse routine    
}


void aisradar_pi::OnRadarFrameClose() {
    m_pRadarFrame = 0;
    SaveConfig();
}


bool aisradar_pi::ShowMoored(void) {
    bool Result=true;
    m_pconfig->SetPath ( _T( "/Settings/AIS" ) );
    m_pconfig->Read ( _T( "bShowMooredTargets" ),  &Result, 1 );
    return Result;
}


double aisradar_pi::GetMooredSpeed(void) {
    double Result=0.;
    m_pconfig->SetPath ( _T( "/Settings/AIS" ) );
    m_pconfig->Read ( _T( "MooredTargetMaxSpeedKnots" ),  &Result, 0.0 );
    return Result;
}


bool aisradar_pi::ShowCogArrows(void) {
    bool Result=true;
    m_pconfig->SetPath ( _T( "/Settings/AIS" ) );
    m_pconfig->Read ( _T("bShowCOGArrows"), &Result, 1);
    return Result;
}


int aisradar_pi::GetCogArrowMinutes(void) {
    int Result=6;
    m_pconfig->SetPath ( _T( "/Settings/AIS" ) );
    m_pconfig->Read ( _T("CogArrowMinutes"), &Result, 6);
    return Result;
}


void aisradar_pi::SetColorScheme(PI_ColorScheme cs) {
    // Colours changed, pass the event on to the radarframe
    if ( m_pRadarFrame ) {
        m_pRadarFrame->SetColourScheme(cs);
        DimeWindow(m_pRadarFrame);
    }
}


ArrayOfPlugIn_AIS_Targets  *aisradar_pi::GetAisTargets() {
    if ( AisTargets ) {
        WX_CLEAR_ARRAY(*AisTargets);     
        delete AisTargets;
    }
    AisTargets = GetAISTargetArray();
    return AisTargets;
}


bool aisradar_pi::LoadConfig(void) {
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;
    if(pConf) {
        pConf->SetPath ( _T( "/Settings" ) );
        pConf->Read ( _T( "ShowRADARIcon" ),  &m_radar_show_icon, 1 );
        pConf->Read ( _T( "UseAisRadar" ),  &m_radar_use_ais, 1 );
        pConf->Read ( _T( "NorthUp" ),  &m_radar_north_up, 0 );
        m_radar_frame_sx  = pConf->Read ( _T ( "RADARDialogSizeX" ),   300L );
        m_radar_frame_sy  = pConf->Read ( _T ( "RADARDialogSizeY" ),   300L );
        m_radar_frame_x   = pConf->Read ( _T ( "RADARDialogPosX" ),     50L );
        m_radar_frame_y   = pConf->Read ( _T ( "RADARDialogPosY" ),    170L );
        m_radar_range     = pConf->Read ( _T ( "RADARRange" ),           4L );
        return true;
    } else {
        return false;
    }
}


bool aisradar_pi::SaveConfig(void) {
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;
    if(pConf) {
        pConf->SetPath ( _T ( "/Settings" ) );
        pConf->Write   ( _T ( "ShowRADARIcon" ),      m_radar_show_icon  );
        pConf->Write   ( _T ( "UseAisRadar" ),        m_radar_use_ais    );
        pConf->Write   ( _T ( "NorthUp" ),            m_radar_north_up   );
        pConf->Write   ( _T ( "RADARDialogSizeX" ),   m_radar_frame_sx   );
        pConf->Write   ( _T ( "RADARDialogSizeY" ),   m_radar_frame_sy   );
        pConf->Write   ( _T ( "RADARDialogPosX" ),    m_radar_frame_x    );
        pConf->Write   ( _T ( "RADARDialogPosY" ),    m_radar_frame_y    );
        pConf->Write   ( _T ( "RADARRange" ),         m_radar_range      );
        return true;
    } else {
        return false;
    }
}
