/******************************************************************************
 * $Id: celestial_navigation_pi.cpp,v 1.8 2010/06/21 01:54:37 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  CELESTIAL_NAVIGATION Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2010 by Sean D'Epagnier   *
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


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers


#include "celestial_navigation_pi.h"
#include "sight.h"

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
    return new celestial_navigation_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}


//---------------------------------------------
//
//    Static Variables
//
//----------------------------------------------
SightList *pSightList;



//---------------------------------------------------------------------------------------------------------
//
//    Celestial_Navigation PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

int celestial_navigation_pi::Init(void)
{
      printf("celestial_navigation_pi Init()\n");

      m_pcelestial_navigation_dialog = NULL;

      // Get a pointer to the opencpn display canvas, to use as a parent for windows created
      m_parent_window = GetOCPNCanvasWindow();

      // Create the Context Menu Items

      //    In order to avoid an ASSERT on msw debug builds,
      //    we need to create a dummy menu to act as a surrogate parent of the created MenuItems
      //    The Items will be re-parented when added to the real context meenu
      wxMenu dummy_menu;

      wxMenuItem *pmi = new wxMenuItem(&dummy_menu, -1, _("Show PlugIn Celestial_NavigationWindow"));
      m_show_id = AddCanvasContextMenuItem(pmi, this );
      SetCanvasContextMenuItemViz(m_show_id, true);

      wxMenuItem *pmih = new wxMenuItem(&dummy_menu, -1, _("Hide PlugIn Celestial_NavigationWindow"));
      m_hide_id = AddCanvasContextMenuItem(pmih, this );
      SetCanvasContextMenuItemViz(m_hide_id, false);

      return (INSTALLS_CONTEXTMENU_ITEMS);
}

bool celestial_navigation_pi::DeInit(void)
{
//      printf("celestial_navigation_pi DeInit()\n");
      if(m_pcelestial_navigation_dialog)
      {
            m_pcelestial_navigation_dialog->Close();
            m_pcelestial_navigation_dialog->Destroy();
      }
      
      return true;
}

int celestial_navigation_pi::GetAPIVersionMajor()
{
      return API_VERSION_MAJOR;
}

int celestial_navigation_pi::GetAPIVersionMinor()
{
      return API_VERSION_MINOR;
}

int celestial_navigation_pi::GetPlugInVersionMajor()
{
      return PLUGIN_VERSION_MAJOR;
}

int celestial_navigation_pi::GetPlugInVersionMinor()
{
      return PLUGIN_VERSION_MINOR;
}


wxString celestial_navigation_pi::GetShortDescription()
{
      return _("Celestial_Navigation PlugIn for OpenCPN");
}

wxString celestial_navigation_pi::GetLongDescription()
{
      return _("Celestial_Navigation PlugIn for OpenCPN\n\
Celestial_Navigationnstrates PlugIn processing of NMEA messages.");

}


void celestial_navigation_pi::OnContextMenuItemCallback(int id)
{
      wxLogMessage(_T("celestial_navigation_pi OnContextMenuCallBack()"));
     ::wxBell();


      if(NULL == m_pcelestial_navigation_dialog)
      {
            m_pcelestial_navigation_dialog = new CelestialNavigationDialog(m_parent_window);


            SetCanvasContextMenuItemViz(m_hide_id, true);
            SetCanvasContextMenuItemViz(m_show_id, false);
      }
      else
      {
            m_pcelestial_navigation_dialog->Close();
            m_pcelestial_navigation_dialog->Destroy();
            m_pcelestial_navigation_dialog = NULL;

            SetCanvasContextMenuItemViz(m_hide_id, false);
            SetCanvasContextMenuItemViz(m_show_id, true);
      }      

}
