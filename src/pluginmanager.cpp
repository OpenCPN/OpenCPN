/******************************************************************************
 *
 *
 * Project:  OpenCPN
 * Purpose:  PlugIn Manager Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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
 *
 */
#include <wx/wx.h>
#include <wx/dir.h>

#include "pluginmanager.h"
#include "navutil.h"

extern MyConfig        *pConfig;

//-----------------------------------------------------------------------------------------------------
//
//          The PlugIn Manager Implementation
//
//-----------------------------------------------------------------------------------------------------
PlugInManager *s_ppim;

PlugInManager::PlugInManager(MyFrame *parent)
{
      pParent = parent;
      s_ppim = this;

      MyFrame *pFrame = GetParentFrame();
      if(pFrame)
      {
            m_plugin_menu_item_id_next = pFrame->GetCanvasWindow()->GetNextContextMenuId();
            m_plugin_tool_id_next = pFrame->GetNextToolbarToolId();
      }
}

PlugInManager::~PlugInManager()
{
}


bool PlugInManager::LoadAllPlugIns(wxString &shared_data_prefix)
{
      wxString plugin_location = shared_data_prefix + _T("plugins");
      wxDir pi_dir(plugin_location);

#ifdef __WXMSW__
      wxString pispec = _T("*_pi.dll");
#else
      wxString pispec = _T("*_pi.so");
#endif

      if(pi_dir.IsOpened())
      {
            wxString plugin_file;
            bool b_more =pi_dir.GetFirst(&plugin_file, pispec);
            while(b_more)
            {
                  wxString file_name = plugin_location + _T("/") + plugin_file;
                  PlugInContainer *pic = LoadPlugIn(file_name);
                  if(pic)
                  {
                        plugin_array.Add(pic);
                        pic->m_cap_flag = pic->m_pplugin->Init();
                  }

                  b_more =pi_dir.GetNext(&plugin_file);
            }
            return true;
      }
      else
            return false;
}

bool PlugInManager::UnLoadAllPlugIns()
{
      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            wxString msg(_("PlugInManager: UnLoading PlugIn: "));
            msg += pic->m_plugin_file;
            wxLogMessage(msg);

            pic->m_pplugin->DeInit();
            pic->m_destroy_fn(pic->m_pplugin);

            delete pic->m_plibrary;            // This will unload the PlugIn

            delete pic;
      }
      return true;
}


PlugInContainer *PlugInManager::LoadPlugIn(wxString plugin_file)
{
      wxString msg(_("PlugInManager: Loading PlugIn: "));
      msg += plugin_file;
      wxLogMessage(msg);

      PlugInContainer *pic = new PlugInContainer;
      pic->m_plugin_file = plugin_file;

   // load the library
      wxDynamicLibrary *plugin = new wxDynamicLibrary(plugin_file);
      pic->m_plibrary = plugin;     // Save a pointer to the wxDynamicLibrary for later deletion

      if(!plugin->IsLoaded())
      {
            wxString msg(_("   PlugInManager: Cannot load library: "));
            msg += plugin_file;
            wxLogMessage(msg);
            return NULL;
      }


    // load the factory symbols
      create_t* create_plugin = (create_t*)plugin->GetSymbol(_T("create_pi"));
      if (NULL == create_plugin)
      {
            wxString msg(_("   PlugInManager: Cannot load symbol create_pi: "));
            msg += plugin_file;
            wxLogMessage(msg);
            return NULL;
      }

      destroy_t* destroy_plugin = (destroy_t*) plugin->GetSymbol(_T("destroy_pi"));
      pic->m_destroy_fn = destroy_plugin;
      if (NULL == destroy_plugin) {
            wxString msg(_("   PlugInManager: Cannot load symbol destroy_pi: "));
            msg += plugin_file;
            wxLogMessage(msg);
            return NULL;
      }

    // create an instance of the plugin class
      opencpn_plugin* plug_in = create_plugin(this);

      pic->m_pplugin = plug_in;

      return pic;

}


bool PlugInManager::RenderAllCanvasOverlayPlugIns( wxMemoryDC *pmdc, ViewPort *vp)
{
      //    Create a PlugIn Viewport
      PlugIn_ViewPort pivp;
      pivp.clat =                   vp->clat;                   // center point
      pivp.clon =                   vp->clon;
      pivp.view_scale_ppm =         vp->view_scale_ppm;
      pivp.skew =                   vp->skew;
      pivp.rotation =               vp->rotation;
      pivp.chart_scale =            vp->chart_scale;
      pivp.pix_width =              vp->pix_width;
      pivp.pix_height =             vp->pix_height;
      pivp.rv_rect =                vp->rv_rect;
      pivp.b_quilt =                vp->b_quilt;
      pivp.m_projection_type =      vp->m_projection_type;

      pivp.lat_min =                vp->vpBBox.GetMinY();
      pivp.lat_max =                vp->vpBBox.GetMaxY();
      pivp.lon_min =                vp->vpBBox.GetMinX();
      pivp.lon_max =                vp->vpBBox.GetMaxX();

      pivp.bValid =                 vp->bValid;                 // This VP is valid

      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            pic->m_pplugin->RenderOverlay(pmdc, &pivp);
      }

      return true;
}

void PlugInManager::SendCursorLatLonToAllPlugIns( double lat, double lon)
{
      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic->m_cap_flag & WANTS_CURSOR_LATLON)
                  pic->m_pplugin->SetCursorLatLon(lat, lon);
      }
}

void PlugInManager::AddAllPlugInToolboxPanels( wxNotebook *pnotebook)
{
      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE)
            {
                  pic->m_pplugin->SetupToolboxPanel(0, pnotebook);
            }
      }
}

void PlugInManager::CloseAllPlugInPanels( int ok_apply_cancel)
{
      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE)
            {
                  pic->m_pplugin->OnCloseToolboxPanel(0, ok_apply_cancel);
            }
      }

}

int PlugInManager::AddCanvasContextMenuItem(wxMenuItem *pitem, opencpn_plugin *pplugin )
{
      PlugInMenuItemContainer *pmic = new PlugInMenuItemContainer;
      pmic->pmenu_item = pitem;
      pmic->m_pplugin = pplugin;
      pmic->id = m_plugin_menu_item_id_next;
      pmic->b_viz = true;
      pmic->b_grey = false;

      m_PlugInMenuItems.Add(pmic);

      m_plugin_menu_item_id_next++;

      return pmic->id;
}




void PlugInManager::RemoveCanvasContextMenuItem(int item)
{
      for(unsigned int i=0; i < m_PlugInMenuItems.GetCount(); i++)
      {
            PlugInMenuItemContainer *pimis = m_PlugInMenuItems.Item(i);
            {
                  if(pimis->id == item)
                  {
                        m_PlugInMenuItems.Remove(pimis);
                        delete pimis;
                        break;
                  }
            }
      }
}

void PlugInManager::SetCanvasContextMenuItemViz(int item, bool viz)
{
      for(unsigned int i=0; i < m_PlugInMenuItems.GetCount(); i++)
      {
            PlugInMenuItemContainer *pimis = m_PlugInMenuItems.Item(i);
            {
                  if(pimis->id == item)
                  {
                        pimis->b_viz = viz;
                        break;
                  }
            }
      }
}

void PlugInManager::SetCanvasContextMenuItemGrey(int item, bool grey)
{
      for(unsigned int i=0; i < m_PlugInMenuItems.GetCount(); i++)
      {
            PlugInMenuItemContainer *pimis = m_PlugInMenuItems.Item(i);
            {
                  if(pimis->id == item)
                  {
                        pimis->b_grey = grey;
                        break;
                  }
            }
      }
}

void PlugInManager::SendNMEASentenceToAllPlugIns(wxString &sentence)
{
      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic->m_cap_flag & WANTS_NMEA_SENTENCES)
                  pic->m_pplugin->SetNMEASentence(sentence);
      }
}


void PlugInManager::SendPositionFixToAllPlugIns(GenericPosDat *ppos)
{
      PlugIn_Position_Fix pfix;
      pfix.Lat = ppos->kLat;
      pfix.Lon = ppos->kLon;
      pfix.Cog = ppos->kCog;
      pfix.Sog = ppos->kSog;
      pfix.Var = ppos->kVar;
      pfix.FixTime = ppos->FixTime;
      pfix.nSats = ppos->nSats;

      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic->m_cap_flag & WANTS_NMEA_EVENTS)
                  pic->m_pplugin->SetPositionFix(pfix);
      }
}



int PlugInManager::AddToolbarTool(wxChar *label, wxBitmap *bitmap, wxBitmap *bmpDisabled, wxItemKind kind,
                                      wxChar *shortHelp, wxChar *longHelp, wxObject *clientData, int position,
                                      int tool_sel, opencpn_plugin *pplugin )
{
      PlugInToolbarToolContainer *pttc = new PlugInToolbarToolContainer;
      pttc->label = label;
      pttc->bitmap = bitmap;
      pttc->bmpDisabled = bmpDisabled;
      pttc->kind = kind;
      pttc->shortHelp = shortHelp;
      pttc->longHelp = longHelp;
      pttc->clientData = clientData;
      pttc->position = position;
      pttc->m_pplugin = pplugin;
      pttc->tool_sel = tool_sel;
      pttc->b_viz = true;
      pttc->id = m_plugin_tool_id_next;



      m_PlugInToolbarTools.Add(pttc);

      m_plugin_tool_id_next++;

      pParent->RequestNewToolbar();

      return pttc->id;
}

void PlugInManager::RemoveToolbarTool(int tool_id)
{
      for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
      {
            PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);
            {
                  if(pttc->id == tool_id)
                  {
                        m_PlugInToolbarTools.Remove(pttc);
                        delete pttc;
                        break;
                  }
            }
      }

      pParent->RequestNewToolbar();
}

void PlugInManager::SetToolbarToolViz(int item, bool viz)
{
      for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
      {
            PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);
            {
                  if(pttc->id == item)
                  {
                        pttc->b_viz = viz;
                        break;
                  }
            }
      }
}


wxString PlugInManager::GetLastError()
{
      return m_last_error_string;
}




//----------------------------------------------------------------------------------------------------------
//    The PlugIn CallBack API Implementation
//    The definitions of this API are found in ocpn_plugin.h
//----------------------------------------------------------------------------------------------------------


int InsertPlugInTool(wxChar *label, wxBitmap *bitmap, wxBitmap *bmpDisabled, wxItemKind kind,
                                          wxChar *shortHelp, wxChar *longHelp, wxObject *clientData, int position,
                                          int tool_sel, opencpn_plugin *pplugin)
{
      if(s_ppim)
            return s_ppim->AddToolbarTool(label, bitmap, bmpDisabled, kind,
                   shortHelp, longHelp, clientData, position,
                   tool_sel, pplugin );
      else
            return -1;
}


void  RemovePlugInTool(int tool_id)
{
      if(s_ppim)
            s_ppim->RemoveToolbarTool(tool_id);
}

void SetToolbarToolViz(int item, bool viz)
{
      if(s_ppim)
            s_ppim->SetToolbarToolViz(item, viz);
}


int AddCanvasContextMenuItem(wxMenuItem *pitem, opencpn_plugin *pplugin )
{
      if(s_ppim)
            return s_ppim->AddCanvasContextMenuItem(pitem, pplugin );
      else
            return -1;
}


void SetCanvasContextMenuItemViz(int item, bool viz)
{
      if(s_ppim)
            s_ppim->SetCanvasContextMenuItemViz(item, viz);
}

void SetCanvasContextMenuItemGrey(int item, bool grey)
{
      if(s_ppim)
            s_ppim->SetCanvasContextMenuItemGrey(item, grey);
}


void RemoveCanvasContextMenuItem(int item)
{
      if(s_ppim)
            s_ppim->RemoveCanvasContextMenuItem(item);
}




wxFileConfig *GetOCPNConfigObject(void)
{
      if(s_ppim)
            return pConfig;         // return the global application config object
      else
            return NULL;
}

wxWindow *GetOCPNCanvasWindow()
{
      wxWindow *pret = NULL;
      if(s_ppim)
      {
            MyFrame *pFrame = s_ppim->GetParentFrame();
            pret = (wxWindow *)pFrame->GetCanvasWindow();
      }
      return pret;
}


void RequestRefresh(wxWindow *win)
{
      if(win)
            win->Refresh();
}

void GetCanvasPixLL(PlugIn_ViewPort *vp, wxPoint *pp, double lat, double lon)
{
      //    Make enough of an application viewport to run its method....
      ViewPort ocpn_vp;
      ocpn_vp.clat = vp->clat;
      ocpn_vp.clon = vp->clon;
      ocpn_vp.m_projection_type = vp->m_projection_type;
      ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
      ocpn_vp.skew = vp->skew;
      ocpn_vp.rotation = vp->rotation;
      ocpn_vp.pix_width = vp->pix_width;
      ocpn_vp.pix_height = vp->pix_height;

      wxPoint ret = ocpn_vp.GetMercatorPixFromLL(lat, lon);
      pp->x = ret.x;
      pp->y = ret.y;
}

void GetCanvasLLPix( PlugIn_ViewPort *vp, wxPoint p, double *plat, double *plon)
{
            //    Make enough of an application viewport to run its method....
      ViewPort ocpn_vp;
      ocpn_vp.clat = vp->clat;
      ocpn_vp.clon = vp->clon;
      ocpn_vp.m_projection_type = vp->m_projection_type;
      ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
      ocpn_vp.skew = vp->skew;
      ocpn_vp.rotation = vp->rotation;
      ocpn_vp.pix_width = vp->pix_width;
      ocpn_vp.pix_height = vp->pix_height;

      return ocpn_vp.GetMercatorLLFromPix( p, plat, plon);
}

bool GetGlobalColor(wxString colorName, wxColour *pcolour)
{
      wxColour c = GetGlobalColor(colorName);
      *pcolour = c;

      return true;
}


