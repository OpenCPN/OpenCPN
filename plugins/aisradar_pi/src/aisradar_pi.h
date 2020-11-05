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

#ifndef _RADARPI_H_
#define _RADARPI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "version.h"

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    9

#include "ocpn_plugin.h"
#include "aisradar.h"

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

#define RADAR_TOOL_POSITION    -1          // Request default positioning of toolbar tool

class aisradar_pi : public opencpn_plugin_19
{
public:
    aisradar_pi(void *ppimgr);
    ~aisradar_pi();

// The required PlugIn Methods
    int Init(void);
    bool DeInit(void);

    int GetAPIVersionMajor();
    int GetAPIVersionMinor();
    int GetPlugInVersionMajor();
    int GetPlugInVersionMinor();
    wxBitmap *GetPlugInBitmap();
    wxString GetCommonName();
    wxString GetShortDescription();
    wxString GetLongDescription();

    void SetDefaults(void);
    int  GetToolbarToolCount(void);
    void ShowPreferencesDialog( wxWindow* parent );
    void OnToolbarToolCallback(int id);
    void SetAISSentence(wxString &sentence);
    void SetPositionFix(PlugIn_Position_Fix &pfix);
    void SetPluginMessage(wxString &message_id, wxString &message_body);
    void SetColorScheme(PI_ColorScheme cs);
    
    // void OnPaneClose( wxAuiManagerEvent& event );
    // void UpdateAuiStatus(void);

// Other public methods
    void             SetRadarFrameX    (int x)  { m_radar_frame_x  = x;   }
    void             SetRadarFrameY    (int x)  { m_radar_frame_y  = x;   }
    void             SetRadarFrameSizeX(int x)  { m_radar_frame_sx = x;   }
    void             SetRadarFrameSizeY(int x)  { m_radar_frame_sy = x;   }
    void             SetRadarNorthUp   (bool x) { m_radar_north_up = x;   }
    void             SetRadarRange     (int x)  { m_radar_range    = x;   }
    bool             GetRadarNorthUp   (void)   { return m_radar_north_up;}
    int              GetRadarRange     (void)   { return m_radar_range;   }
    double           GetCog            (void)   { return m_cog;           }
    double           GetSog            (void)   { return m_sog;           }
    int              GetSats           (void)   { return m_sats;          }
    wxFileConfig    *GetConfig         (void)   { return m_pconfig;       }
    ArrayOfPlugIn_AIS_Targets  *GetAisTargets();
    void OnRadarFrameClose();
    bool             ShowMoored        (void);
    double           GetMooredSpeed    (void);
    bool             ShowCogArrows     (void);
    int              GetCogArrowMinutes(void);

private:
    bool LoadConfig(void);
    bool SaveConfig(void);

private:
    wxFileConfig     *m_pconfig;
    wxWindow         *m_parent_window;
    RadarFrame       *m_pRadarFrame;
    ArrayOfPlugIn_AIS_Targets *AisTargets;
    int               m_display_width, m_display_height;
    int               m_leftclick_tool_id;
    int               m_radar_frame_x, m_radar_frame_y;
    int               m_radar_frame_sx, m_radar_frame_sy;
    int               m_radar_range;
    double            m_lat;
    double            m_lon;
    double            m_cog;
    double            m_sog;
    int               m_sats;
    bool              m_radar_show_icon;
    bool              m_radar_use_ais;
    bool              m_radar_north_up;
    wxCheckBox       *m_pRadarShowIcon;
    wxCheckBox       *m_pRadarUseAis;

    
};

#endif
