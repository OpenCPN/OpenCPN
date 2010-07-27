/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  PlugIn Object Definition/API
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
#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#ifndef DECL_EXP
#ifdef __WXMSW__
#  define DECL_EXP     __declspec(dllexport)
#else
#  define DECL_EXP
#endif
#endif


#define API_VERSION_MAJOR           1
#define API_VERSION_MINOR           0

//    Fwd Definitions
class       wxFileConfig;
class       wxNotebook;
class       wxFont;


//---------------------------------------------------------------------------------------------------------
//
//    Bitfield PlugIn Capabilites flag definition
//
//---------------------------------------------------------------------------------------------------------
#define     WANTS_OVERLAY_CALLBACK              0x00000001
#define     WANTS_CURSOR_LATLON                 0x00000002
#define     WANTS_TOOLBAR_CALLBACK              0x00000004
#define     INSTALLS_TOOLBAR_TOOL               0x00000008
#define     WANTS_CONFIG                        0x00000010
#define     INSTALLS_TOOLBOX_PAGE               0x00000020
#define     INSTALLS_CONTEXTMENU_ITEMS          0x00000040
#define     WANTS_NMEA_SENTENCES                0x00000080
#define     WANTS_NMEA_EVENTS                   0x00000100
#define     WANTS_AIS_SENTENCES                 0x00000200

//----------------------------------------------------------------------------------------------------------
//    Some PlugIn API interface object class definitions
//----------------------------------------------------------------------------------------------------------
class PlugIn_ViewPort
{
      public:
            double   clat;                   // center point
            double   clon;
            double   view_scale_ppm;
            double   skew;
            double   rotation;

            float    chart_scale;            // conventional chart displayed scale

            int      pix_width;
            int      pix_height;
            wxRect   rv_rect;
            bool     b_quilt;
            int      m_projection_type;

            double   lat_min, lat_max, lon_min, lon_max;

            bool     bValid;                 // This VP is valid
};

class PlugIn_Position_Fix
{
   public:
      double Lat;
      double Lon;
      double Cog;
      double Sog;
      double Var;            // Variation, typically from RMC message
      time_t FixTime;
      int    nSats;
};

//----------------------------------------------------------------------------------------------------------
//    The Generic PlugIn Interface Class Definition
//
//    This is a virtual class.
//    opencpn PlugIns must derive from this class.
//    There are two types of methods in this class
//    a. Required...must be overridden and implemented by PlugIns
//    b. Optional..may be overridden by PlugIns

//    PlugIns must implement optional method overrides consistent with their
//     declared capabilities flag as returned by Init().
//----------------------------------------------------------------------------------------------------------
class DECL_EXP opencpn_plugin
{

public:
      opencpn_plugin(void *pmgr) {}
      virtual ~opencpn_plugin() {}

      //    Public API to the PlugIn class

      //    This group of methods is required, and will be called by the opencpn host
      //    opencpn PlugIns must implement this group
      virtual int Init(void);               // Return the PlugIn Capabilites flag
      virtual bool DeInit(void);

      virtual int GetAPIVersionMajor();
      virtual int GetAPIVersionMinor();
      virtual int GetPlugInVersionMajor();
      virtual int GetPlugInVersionMinor();

      virtual wxString GetShortDescription();
      virtual wxString GetLongDescription();

      //    This group is optional.
      //    PlugIns may override any of these methods as required

      virtual int GetToolbarToolCount(void);

      virtual int GetToolboxPanelCount(void);
      virtual void SetupToolboxPanel(int page_sel, wxNotebook* pnotebook);
      virtual void OnCloseToolboxPanel(int page_sel, int ok_apply_cancel);

      virtual bool RenderOverlay(wxMemoryDC *pmdc, PlugIn_ViewPort *vp);
      virtual void SetCursorLatLon(double lat, double lon);

      virtual void SetPositionFix(PlugIn_Position_Fix &pfix);
      virtual void SetNMEASentence(wxString &sentence);

      virtual void OnToolbarToolCallback(int id);
      virtual void OnContextMenuItemCallback(int id);

 };


 // the types of the class factories used to create PlugIn instances
 typedef opencpn_plugin* create_t(void*);
 typedef void destroy_t(opencpn_plugin*);



//----------------------------------------------------------------------------------------------------------
//    The PlugIn CallBack API Definition
//
//    The API back up to the PlugIn Manager
//    PlugIns may call these static functions as necessary for system services
//
//----------------------------------------------------------------------------------------------------------


extern "C"  DECL_EXP int InsertPlugInTool(wxChar *label, wxBitmap *bitmap, wxBitmap *bmpDisabled, wxItemKind kind,
                                          wxChar *shortHelp, wxChar *longHelp, wxObject *clientData, int position,
                                          int tool_sel, opencpn_plugin *pplugin);
extern "C"  DECL_EXP void RemovePlugInTool(int tool_id);
extern "C"  DECL_EXP void SetToolbarToolViz(int item, bool viz);      // Temporarily change toolbar tool viz

extern "C"  DECL_EXP  int AddCanvasContextMenuItem(wxMenuItem *pitem, opencpn_plugin *pplugin );
extern "C"  DECL_EXP void RemoveCanvasContextMenuItem(int item);      // Fully remove this item
extern "C"  DECL_EXP void SetCanvasContextMenuItemViz(int item, bool viz);      // Temporarily change context menu ptions
extern "C"  DECL_EXP void SetCanvasContextMenuItemGrey(int item, bool grey);


extern "C"  DECL_EXP wxFileConfig *GetOCPNConfigObject(void);

extern "C"  DECL_EXP void RequestRefresh(wxWindow *);
extern "C"  DECL_EXP bool GetGlobalColor(wxString colorName, wxColour *pcolour);

extern "C"  DECL_EXP void GetCanvasPixLL(PlugIn_ViewPort *vp, wxPoint *pp, double lat, double lon);
extern "C"  DECL_EXP void GetCanvasLLPix( PlugIn_ViewPort *vp, wxPoint p, double *plat, double *plon);

extern "C"  DECL_EXP wxWindow *GetOCPNCanvasWindow();

extern "C"  DECL_EXP wxFont *OCPNGetFont(wxString TextElement, int default_size);

extern "C"  DECL_EXP wxString *GetpSharedDataLocation();

#endif            // _PLUGIN_H_

