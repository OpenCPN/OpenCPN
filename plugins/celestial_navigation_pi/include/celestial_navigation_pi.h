/******************************************************************************
 * $Id: celestial_navigation_pi.h,v 1.8 2010/06/21 01:54:37 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Celestial Navigation Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2010 by Sean D'Epagnier                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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

#ifndef _CELESTIAL_NAVIGATIONPI_H_
#define _CELESTIAL_NAVIGATIONPI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#define     PLUGIN_VERSION_MAJOR    1
#define     PLUGIN_VERSION_MINOR    0

#include "../../../include/ocpn_plugin.h"

#include "celestialnavigationdialog.h"

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------


class celestial_navigation_pi : public opencpn_plugin
{
public:
      celestial_navigation_pi(void *ppimgr):opencpn_plugin(ppimgr){}

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

      void OnContextMenuItemCallback(int id);

      bool RenderOverlay(wxMemoryDC *pmdc, PlugIn_ViewPort *vp);
      void SetCursorLatLon(double lat, double lon);

private:
      wxWindow         *m_parent_window;

      CelestialNavigationDialog       *m_pcelestial_navigation_dialog;
      int               m_show_id;
      int               m_hide_id;
};

extern double celestial_navigation_pi_CursorLat();
extern double celestial_navigation_pi_CursorLon();

#endif
