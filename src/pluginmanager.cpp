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
#include <wx/filename.h>
#include <wx/aui/aui.h>

#include "pluginmanager.h"
#include "navutil.h"
#include "ais.h"

extern MyConfig        *pConfig;
extern FontMgr         *pFontMgr;
extern wxString        g_SData_Locn;
extern AIS_Decoder     *g_pAIS;
extern wxAuiManager    *g_pauimgr;

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
      m_plugin_location = shared_data_prefix + _T("plugins");
      wxDir pi_dir(m_plugin_location);

      wxString msg(_("PlugInManager searching for PlugIns in location "));
      msg += m_plugin_location;
      wxLogMessage(msg);

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
                  wxString file_name = m_plugin_location + _T("/") + plugin_file;

                  PlugInContainer *pic = LoadPlugIn(file_name);
                  if(pic)
                  {
    //    Verify the PlugIn API Version

                        int api_major = pic->m_pplugin->GetAPIVersionMajor();
                        int api_minor = pic->m_pplugin->GetAPIVersionMinor();
                        int ver = (api_major * 100) + api_minor;
                        bool bver_ok = false;
                        switch(ver)
                        {
                              case 101:                                 // TODO add more valid API versions to this case as necessary
                                    bver_ok = true;
                                    break;
                              default:
                                    break;
                        }

                        if(bver_ok)
                        {
                              plugin_array.Add(pic);

                              pic->m_api_version = ver;                 // record the PlugIn's API version

                              //    The common name is available without initialization and startup of the PlugIn
                              pic->m_common_name = pic->m_pplugin->GetCommonName();

                              //    Check the config file to see if this PlugIn is user-enabled
                              wxString config_section = ( _T ( "/PlugIns/" ) );
                              config_section += pic->m_common_name;
                              pConfig->SetPath ( config_section );
                              pConfig->Read ( _T ( "bEnabled" ), &pic->m_bEnabled );

                              if(pic->m_bEnabled)
                              {
                                    pic->m_cap_flag = pic->m_pplugin->Init();
                                    pic->m_bInitState = true;
                              }

                              pic->m_short_description = pic->m_pplugin->GetShortDescription();
                              pic->m_long_description = pic->m_pplugin->GetLongDescription();
                        }
                        else
                        {
                              wxString msg;
                              msg.Printf(_("PlugInManager: Unloading PlugIn with invalid API version %d.%d: "), api_major, api_minor );
                              msg += pic->m_plugin_file;
                              wxLogMessage(msg);

                              delete pic->m_plibrary;            // This will unload the PlugIn
                              delete pic;
                        }
                  }

                  b_more =pi_dir.GetNext(&plugin_file);
            }
            return true;
      }
      else
            return false;
}

bool PlugInManager::UpdatePlugIns()
{
      bool bret = false;

      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);

            if(pic->m_bEnabled && !pic->m_bInitState)
            {
                  wxString msg(_("PlugInManager: Initializing PlugIn: "));
                  msg += pic->m_plugin_file;
                  wxLogMessage(msg);

                  pic->m_cap_flag = pic->m_pplugin->Init();
                  pic->m_pplugin->SetDefaults();
                  pic->m_bInitState = true;
                  pic->m_short_description = pic->m_pplugin->GetShortDescription();
                  pic->m_long_description = pic->m_pplugin->GetLongDescription();
                  bret = true;
            }
            else if(!pic->m_bEnabled && pic->m_bInitState)
            {
                  wxString msg(_("PlugInManager: Deactivating PlugIn: "));
                  msg += pic->m_plugin_file;
                  wxLogMessage(msg);

                  pic->m_pplugin->DeInit();

                  //    Deactivate (Remove) any ToolbarTools added by this PlugIn
                  for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
                  {
                        PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);
                        {
                              if(pttc->m_pplugin == pic->m_pplugin)
                              {
                                    m_PlugInToolbarTools.Remove(pttc);
                                    delete pttc;
                              }
                        }
                  }

                 //    Deactivate (Remove) any ContextMenu items addded by this PlugIn
                  for(unsigned int i=0; i < m_PlugInMenuItems.GetCount(); i++)
                  {
                        PlugInMenuItemContainer *pimis = m_PlugInMenuItems.Item(i);
                        {
                              if(pimis->m_pplugin == pic->m_pplugin)
                              {
                                    m_PlugInMenuItems.Remove(pimis);
                                    delete pimis;
                              }
                        }
                  }

                  pic->m_bInitState = false;
                  bret = true;
            }
      }

      return bret;
}

bool PlugInManager::UpdateConfig()
{
      pConfig->SetPath(_T("/"));
      if(pConfig->HasGroup( _T ( "PlugIns" )))
               pConfig->DeleteGroup( _T ( "PlugIns" ) );


      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);

            wxString config_section = ( _T ( "/PlugIns/" ) );
            config_section += pic->m_common_name;
            pConfig->SetPath ( config_section );
            pConfig->Write ( _T ( "bEnabled" ), pic->m_bEnabled );
      }

      return true;
}

bool PlugInManager::UnLoadAllPlugIns()
{
      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            wxString msg(_("PlugInManager: UnLoading PlugIn: "));
            msg += pic->m_plugin_file;
            wxLogMessage(msg);

            pic->m_destroy_fn(pic->m_pplugin);

            delete pic->m_plibrary;            // This will unload the PlugIn

            pic->m_bInitState = false;

            delete pic;
      }
      return true;
}

bool PlugInManager::DeactivateAllPlugIns()
{
      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            wxString msg(_("PlugInManager: Deactivating PlugIn: "));
            msg += pic->m_plugin_file;
            wxLogMessage(msg);

            if(pic->m_bEnabled && pic->m_bInitState)
            {
                  pic->m_pplugin->DeInit();
                  pic->m_bInitState = false;
            }

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
            if(pic->m_bEnabled && pic->m_bInitState)
                  pic->m_pplugin->RenderOverlay(pmdc, &pivp);
      }

      return true;
}

void PlugInManager::SendCursorLatLonToAllPlugIns( double lat, double lon)
{
      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic->m_bEnabled && pic->m_bInitState)
            {
                  if(pic->m_cap_flag & WANTS_CURSOR_LATLON)
                        pic->m_pplugin->SetCursorLatLon(lat, lon);
            }
      }
}

void PlugInManager::AddAllPlugInToolboxPanels( wxNotebook *pnotebook)
{
      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic->m_bEnabled && pic->m_bInitState)
            {
                  if(pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE)
                  {
                      pic->m_pplugin->SetupToolboxPanel(0, pnotebook);
                      pic->m_bToolboxPanel = true;
                  }
            }
      }
}

void PlugInManager::CloseAllPlugInPanels( int ok_apply_cancel)
{
      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic->m_bEnabled && pic->m_bInitState)
            {
                  if((pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE) && ( pic->m_bToolboxPanel))
                  {
                       pic->m_pplugin->OnCloseToolboxPanel(0, ok_apply_cancel);
                       pic->m_bToolboxPanel = false;
                  }
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
            if(pic->m_bEnabled && pic->m_bInitState)
            {
                  if(pic->m_cap_flag & WANTS_NMEA_SENTENCES)
                        pic->m_pplugin->SetNMEASentence(sentence);
            }
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
            if(pic->m_bEnabled && pic->m_bInitState)
            {
                  if(pic->m_cap_flag & WANTS_NMEA_EVENTS)
                        pic->m_pplugin->SetPositionFix(pfix);
            }
      }
}

void PlugInManager::SendResizeEventToAllPlugIns(int x, int y)
{
      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic->m_bEnabled && pic->m_bInitState)
                  pic->m_pplugin->ProcessParentResize(x, y);
      }
}

void PlugInManager::SetColorSchemeForAllPlugIns(ColorScheme cs)
{
      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic->m_bEnabled && pic->m_bInitState)
                  pic->m_pplugin->SetColorScheme((PI_ColorScheme)cs);
      }
}

void PlugInManager::NotifyAuiPlugIns(void)
{
      for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
      {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic->m_bEnabled && pic->m_bInitState && (pic->m_cap_flag & USES_AUI_MANAGER))
                  pic->m_pplugin->UpdateAuiStatus();
      }
}

int PlugInManager::AddToolbarTool(wxString label, wxBitmap *bitmap, wxBitmap *bmpDisabled, wxItemKind kind,
                                      wxString shortHelp, wxString longHelp, wxObject *clientData, int position,
                                      int tool_sel, opencpn_plugin *pplugin )
{
      PlugInToolbarToolContainer *pttc = new PlugInToolbarToolContainer;
      pttc->label = label;

      pttc->bitmap_day = bitmap;
      pttc->bitmap_dusk = BuildDimmedToolBitmap(bitmap, 128);
      pttc->bitmap_night = BuildDimmedToolBitmap(bitmap, 32);

      pttc->bmpDisabled = bmpDisabled;
      pttc->kind = kind;
      pttc->shortHelp = shortHelp;
      pttc->longHelp = longHelp;
      pttc->clientData = clientData;
      pttc->position = position;
      pttc->m_pplugin = pplugin;
      pttc->tool_sel = tool_sel;
      pttc->b_viz = true;
      pttc->b_toggle = false;
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

void PlugInManager::SetToolbarItemState(int item, bool toggle)
{
      for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
      {
            PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);
            {
                  if(pttc->id == item)
                  {
                        pttc->b_toggle = toggle;
                        pParent->SetToolbarItemState(item, toggle);
                        break;
                  }
            }
      }
}

wxString PlugInManager::GetLastError()
{
      return m_last_error_string;
}

wxBitmap *PlugInManager::BuildDimmedToolBitmap(wxBitmap *pbmp_normal, unsigned char dim_ratio)
{
      wxImage img_dup = pbmp_normal->ConvertToImage();

      if(dim_ratio < 200)
      {
              //  Create a dimmed version of the image/bitmap
            int gimg_width = img_dup.GetWidth();
            int gimg_height = img_dup.GetHeight();

            double factor = (double)(dim_ratio) / 256.0;

            for(int iy=0 ; iy < gimg_height ; iy++)
            {
                  for(int ix=0 ; ix < gimg_width ; ix++)
                  {
                        if(!img_dup.IsTransparent(ix, iy))
                        {
                              wxImage::RGBValue rgb(img_dup.GetRed(ix, iy), img_dup.GetGreen(ix, iy), img_dup.GetBlue(ix, iy));
                              wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
                              hsv.value = hsv.value * factor;
                              wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
                              img_dup.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);
                        }
                  }
            }
      }

        //  Make a bitmap
      wxBitmap *ptoolBarBitmap;

#ifdef __WXMSW__
      wxBitmap tbmp(img_dup.GetWidth(),img_dup.GetHeight(),-1);
      wxMemoryDC dwxdc;
      dwxdc.SelectObject(tbmp);

      ptoolBarBitmap = new wxBitmap(img_dup, (wxDC &)dwxdc);
#else
      ptoolBarBitmap = new wxBitmap(img_dup);
#endif

        // store it
      return ptoolBarBitmap;
}


//----------------------------------------------------------------------------------------------------------
//    The PlugIn CallBack API Implementation
//    The definitions of this API are found in ocpn_plugin.h
//----------------------------------------------------------------------------------------------------------


int InsertPlugInTool(wxString label, wxBitmap *bitmap, wxBitmap *bmpDisabled, wxItemKind kind,
                                          wxString shortHelp, wxString longHelp, wxObject *clientData, int position,
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

void SetToolbarItemState(int item, bool toggle)
{
      if(s_ppim)
            s_ppim->SetToolbarItemState(item, toggle);
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

wxFont *OCPNGetFont(wxString TextElement, int default_size)
{
      if(pFontMgr)
            return pFontMgr->GetFont(TextElement, default_size);
      else
            return NULL;
}

wxString *GetpSharedDataLocation(void)
{
      return &g_SData_Locn;
}


ArrayOfPlugIn_AIS_Targets *GetAISTargetArray(void)
{
      if ( !g_pAIS )
            return NULL;


      ArrayOfPlugIn_AIS_Targets *pret = new ArrayOfPlugIn_AIS_Targets;

      //      Iterate over the AIS Target Hashmap
      AIS_Target_Hash::iterator it;

      AIS_Target_Hash *current_targets = g_pAIS->GetTargetList();

      for ( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it )
      {
            AIS_Target_Data *td = it->second;
            PlugIn_AIS_Target *ptarget = Create_PI_AIS_Target(td);
            pret->Add(ptarget);
      }

//  Test one alarm target
#if 0
      AIS_Target_Data td;
      td.n_alarm_state = AIS_ALARM_SET;
      PlugIn_AIS_Target *ptarget = Create_PI_AIS_Target(&td);
      pret->Add(ptarget);
#endif
      return pret;
}


wxAuiManager *GetFrameAuiManager(void)
{
      return g_pauimgr;
}


//-----------------------------------------------------------------------------------------
//    The opencpn_plugin base class implementation
//-----------------------------------------------------------------------------------------

int opencpn_plugin::Init(void)
{  return 0; }


bool opencpn_plugin::DeInit(void)
{  return true; }

int opencpn_plugin::GetAPIVersionMajor()
{  return API_VERSION_MAJOR; }

int opencpn_plugin::GetAPIVersionMinor()
{  return API_VERSION_MINOR; }

int opencpn_plugin::GetPlugInVersionMajor()
{  return 1; }

int opencpn_plugin::GetPlugInVersionMinor()
{  return 0; }

wxString opencpn_plugin::GetCommonName()
{
      return _("BaseClassCommonName");
}

wxString opencpn_plugin::GetShortDescription()
{
      return _("OpenCPN PlugIn Base Class");
}

wxString opencpn_plugin::GetLongDescription()
{
      return _("OpenCPN PlugIn Base Class\n\
PlugInManager created this base class");
}



void opencpn_plugin::SetPositionFix(PlugIn_Position_Fix &pfix)
{}

void opencpn_plugin::SetNMEASentence(wxString &sentence)
{}

int opencpn_plugin::GetToolbarToolCount(void)
{  return 0; }

int opencpn_plugin::GetToolboxPanelCount(void)
{  return 0; }

void opencpn_plugin::SetupToolboxPanel(int page_sel, wxNotebook* pnotebook)
{}

void opencpn_plugin::OnCloseToolboxPanel(int page_sel, int ok_apply_cancel)
{}

void opencpn_plugin::OnToolbarToolCallback(int id)
{}

void opencpn_plugin::OnContextMenuItemCallback(int id)
{}

bool opencpn_plugin::RenderOverlay(wxMemoryDC *pmdc, PlugIn_ViewPort *vp)
{  return false; }

void opencpn_plugin::SetCursorLatLon(double lat, double lon)
{}

void opencpn_plugin::SetDefaults(void)
{}

void opencpn_plugin::ProcessParentResize(int x, int y)
{}

void opencpn_plugin::SetColorScheme(PI_ColorScheme cs)
{}

void opencpn_plugin::UpdateAuiStatus(void)
{}

//          Helper and interface classes

//-------------------------------------------------------------------------------
//    PlugIn_AIS_Target Implementation
//-------------------------------------------------------------------------------

PlugIn_AIS_Target *Create_PI_AIS_Target(AIS_Target_Data *ptarget)
{
      PlugIn_AIS_Target *pret = new PlugIn_AIS_Target;

      pret->MMSI =            ptarget->MMSI;
      pret->Class =           ptarget->Class;
      pret->NavStatus =       ptarget->NavStatus;
      pret->SOG =             ptarget->SOG;
      pret->COG =             ptarget->COG;
      pret->HDG =             ptarget->HDG;
      pret->Lon =             ptarget->Lon;
      pret->Lat =             ptarget->Lat;
      pret->ROTAIS =          ptarget->ROTAIS;
      pret->ShipType =        ptarget->ShipType;
      pret->IMO =             ptarget->IMO;

      pret->Range_NM =        ptarget->Range_NM;
      pret->Brg =             ptarget->Brg;

    //      Per target collision parameters
      pret->bCPA_Valid =      ptarget->bCPA_Valid;
      pret->TCPA =            ptarget->TCPA;                     // Minutes
      pret->CPA =             ptarget->CPA;                      // Nautical Miles

      pret->alarm_state =     (plugin_ais_alarm_type)ptarget->n_alarm_state;

      strncpy(pret->CallSign, ptarget->CallSign, sizeof(ptarget->CallSign));
      strncpy(pret->ShipName, ptarget->ShipName, sizeof(ptarget->ShipName));

      return pret;
}



