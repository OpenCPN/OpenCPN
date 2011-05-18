/******************************************************************************
 * $Id: grib_pi.h,v 1.8 2010/06/21 01:54:37 bdbcat Exp $
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

#ifndef _GRIBPI_H_
#define _GRIBPI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#define     PLUGIN_VERSION_MAJOR    1
#define     PLUGIN_VERSION_MINOR    1

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    5

#include "../../../include/ocpn_plugin.h"

#include "grib.h"

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

#define GRIB_TOOL_POSITION    -1          // Request default positioning of toolbar tool

class grib_pi : public opencpn_plugin
{
public:
      grib_pi(void *ppimgr);

//    The required PlugIn Methods
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

//    The required override PlugIn Methods
      bool RenderOverlay(wxMemoryDC *pmdc, PlugIn_ViewPort *vp);
      void SetCursorLatLon(double lat, double lon);


      void SetDefaults(void);

      int GetToolbarToolCount(void);
      
      void ShowPreferencesDialog( wxWindow* parent );

      void OnToolbarToolCallback(int id);
      

// Other public methods

      void SetGribDir(wxString grib_dir){ m_grib_dir = grib_dir;};
      void SetGribDialogX    (int x){ m_grib_dialog_x = x;};
      void SetGribDialogY    (int x){ m_grib_dialog_y = x;}
      void SetGribDialogSizeX(int x){ m_grib_dialog_sx = x;}
      void SetGribDialogSizeY(int x){ m_grib_dialog_sy = x;}

      void OnGribDialogClose();
      GRIBOverlayFactory *GetGRIBOverlayFactory(){ return m_pGRIBOverlayFactory; }

private:
      bool LoadConfig(void);
      bool SaveConfig(void);

      wxFileConfig     *m_pconfig;
      wxWindow         *m_parent_window;

      GRIBUIDialog     *m_pGribDialog;
      GRIBOverlayFactory *m_pGRIBOverlayFactory;

      int              m_display_width, m_display_height;
      int              m_leftclick_tool_id;


      int              m_grib_dialog_x, m_grib_dialog_y;
      int              m_grib_dialog_sx, m_grib_dialog_sy;
      wxString         m_grib_dir;

      bool              m_bGRIBUseHiDef;
      bool              m_bGRIBShowIcon;


      //    Controls added to Preferences panel
      wxCheckBox              *m_pGRIBShowIcon;
      wxCheckBox              *m_pGRIBUseHiDef;

};

#endif



