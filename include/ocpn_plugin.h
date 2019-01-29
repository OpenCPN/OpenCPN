/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  PlugIn Object Definition/API
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 **************************************************************************/

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#ifndef DECL_EXP
#ifdef __WXMSW__
#  define DECL_EXP     __declspec(dllexport)
#else
# ifdef __GNUC__
# define DECL_EXP       __attribute__((visibility("default")))
# endif
#endif
#endif

#ifdef __WXMSW__
#ifdef MAKING_PLUGIN
#  define DECL_IMP     __declspec(dllimport)
#endif
#endif    

#include <wx/xml/xml.h>

#ifdef ocpnUSE_SVG
#include "wxSVG/svg.h"
#endif // ocpnUSE_SVG

#include <memory>
#include <vector>

class wxGLContext;

//    This is the most modern API Version number
//    It is expected that the API will remain downward compatible, meaning that
//    PlugIns conforming to API Version less then the most modern will also
//    be correctly supported.
#define API_VERSION_MAJOR           1
#define API_VERSION_MINOR           16

//    Fwd Definitions
class       wxFileConfig;
class       wxNotebook;
class       wxFont;
class       wxAuiManager;
class       wxScrolledWindow;
class       wxGLCanvas;

//---------------------------------------------------------------------------------------------------------
//
//    Bitfield PlugIn Capabilites flag definition
//
//---------------------------------------------------------------------------------------------------------
#define     WANTS_OVERLAY_CALLBACK                    0x00000001
#define     WANTS_CURSOR_LATLON                       0x00000002
#define     WANTS_TOOLBAR_CALLBACK                    0x00000004
#define     INSTALLS_TOOLBAR_TOOL                     0x00000008
#define     WANTS_CONFIG                              0x00000010
#define     INSTALLS_TOOLBOX_PAGE                     0x00000020
#define     INSTALLS_CONTEXTMENU_ITEMS                0x00000040
#define     WANTS_NMEA_SENTENCES                      0x00000080
#define     WANTS_NMEA_EVENTS                         0x00000100
#define     WANTS_AIS_SENTENCES                       0x00000200
#define     USES_AUI_MANAGER                          0x00000400
#define     WANTS_PREFERENCES                         0x00000800
#define     INSTALLS_PLUGIN_CHART                     0x00001000
#define     WANTS_ONPAINT_VIEWPORT                    0x00002000
#define     WANTS_PLUGIN_MESSAGING                    0x00004000
#define     WANTS_OPENGL_OVERLAY_CALLBACK             0x00008000
#define     WANTS_DYNAMIC_OPENGL_OVERLAY_CALLBACK     0x00010000
#define     WANTS_LATE_INIT                           0x00020000
#define     INSTALLS_PLUGIN_CHART_GL                  0x00040000
#define     WANTS_MOUSE_EVENTS                        0x00080000
#define     WANTS_VECTOR_CHART_OBJECT_INFO            0x00100000
#define     WANTS_KEYBOARD_EVENTS                     0x00200000

//----------------------------------------------------------------------------------------------------------
//    Some PlugIn API interface object class definitions
//----------------------------------------------------------------------------------------------------------
enum PI_ColorScheme
{
      PI_GLOBAL_COLOR_SCHEME_RGB,
      PI_GLOBAL_COLOR_SCHEME_DAY,
      PI_GLOBAL_COLOR_SCHEME_DUSK,
      PI_GLOBAL_COLOR_SCHEME_NIGHT,
      PI_N_COLOR_SCHEMES
};

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

class PlugIn_Position_Fix_Ex
{
      public:
            double Lat;
            double Lon;
            double Cog;
            double Sog;
            double Var;            // Variation, typically from RMC message
            double Hdm;
            double Hdt;
            time_t FixTime;
            int    nSats;
};

//    Describe AIS Alarm state
enum plugin_ais_alarm_type
{
      PI_AIS_NO_ALARM = 0,
      PI_AIS_ALARM_SET,
      PI_AIS_ALARM_ACKNOWLEDGED

};


class PlugIn_AIS_Target
{
      public:

            int                       MMSI;
            int                       Class;
            int                       NavStatus;
            double                    SOG;
            double                    COG;
            double                    HDG;
            double                    Lon;
            double                    Lat;
            int                       ROTAIS;
            char                      CallSign[8];                // includes terminator
            char                      ShipName[21];
            unsigned char             ShipType;
            int                       IMO;

            double                    Range_NM;
            double                    Brg;

    //      Per target collision parameters
            bool                      bCPA_Valid;
            double                    TCPA;                     // Minutes
            double                    CPA;                      // Nautical Miles

            plugin_ais_alarm_type     alarm_state;
};


//    ChartType constants
typedef enum ChartTypeEnumPI
{
      PI_CHART_TYPE_UNKNOWN = 0,
      PI_CHART_TYPE_DUMMY,
      PI_CHART_TYPE_DONTCARE,
      PI_CHART_TYPE_KAP,
      PI_CHART_TYPE_GEO,
      PI_CHART_TYPE_S57,
      PI_CHART_TYPE_CM93,
      PI_CHART_TYPE_CM93COMP,
      PI_CHART_TYPE_PLUGIN
}_ChartTypeEnumPI;

//    ChartFamily constants
typedef enum ChartFamilyEnumPI
{
      PI_CHART_FAMILY_UNKNOWN = 0,
      PI_CHART_FAMILY_RASTER,
      PI_CHART_FAMILY_VECTOR,
      PI_CHART_FAMILY_DONTCARE
}_ChartFamilyEnumPI;

//          Depth unit type enum
typedef enum ChartDepthUnitTypePI
{
      PI_DEPTH_UNIT_UNKNOWN,
      PI_DEPTH_UNIT_FEET,
      PI_DEPTH_UNIT_METERS,
      PI_DEPTH_UNIT_FATHOMS
}_ChartDepthUnitTypePI;

//          Projection type enum
typedef enum OcpnProjTypePI
{
      PI_PROJECTION_UNKNOWN,
      PI_PROJECTION_MERCATOR,
      PI_PROJECTION_TRANSVERSE_MERCATOR,
      PI_PROJECTION_POLYCONIC,

      PI_PROJECTION_ORTHOGRAPHIC,
      PI_PROJECTION_POLAR,
      PI_PROJECTION_STEREOGRAPHIC,
      PI_PROJECTION_GNOMONIC,
      PI_PROJECTION_EQUIRECTANGULAR
}_OcpnProjTypePI;

typedef struct _ExtentPI{
      double SLAT;
      double WLON;
      double NLAT;
      double ELON;
}ExtentPI;

//    PlugInChartBase::Init()  init_flags constants
#define      PI_FULL_INIT        0
#define      PI_HEADER_ONLY      1
#define      PI_THUMB_ONLY       2


// ----------------------------------------------------------------------------
// PlugInChartBase
//  This class is the base class for Plug-able chart types
// ----------------------------------------------------------------------------

class DECL_EXP PlugInChartBase : public wxObject
{
      public:
            //    These methods Must be overriden in any derived class
            PlugInChartBase();
            virtual ~PlugInChartBase();

            virtual wxString GetFileSearchMask(void);

            virtual int Init( const wxString& full_path, int init_flags );
            virtual void SetColorScheme(int cs, bool bApplyImmediate);

            virtual double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);
            virtual double GetNormalScaleMax(double canvas_scale_factor, int canvas_width);
            virtual double GetNearestPreferredScalePPM(double target_scale_ppm);

            virtual bool GetChartExtent(ExtentPI *pext);

            virtual wxBitmap &RenderRegionView(const PlugIn_ViewPort& VPoint, const wxRegion &Region);

            virtual bool AdjustVP(PlugIn_ViewPort &vp_last, PlugIn_ViewPort &vp_proposed);

            virtual void GetValidCanvasRegion(const PlugIn_ViewPort& VPoint, wxRegion *pValidRegion);

            virtual int GetCOVREntries(){ return  0; }
            virtual int GetCOVRTablePoints(int iTable) { return 0; }
            virtual int  GetCOVRTablenPoints(int iTable){ return 0; }
            virtual float *GetCOVRTableHead(int iTable){ return (float *)NULL; }

            virtual wxBitmap *GetThumbnail(int tnx, int tny, int cs);

//    Accessors, need not be overridden in derived class if the member variables are maintained
            virtual wxString GetFullPath() const            { return m_FullPath;}
            virtual ChartTypeEnumPI GetChartType()          { return m_ChartType;}
            virtual ChartFamilyEnumPI GetChartFamily()      { return m_ChartFamily;}
            virtual OcpnProjTypePI GetChartProjection()     { return m_projection;}
            virtual wxString GetName()                      { return m_Name;}
            virtual wxString GetDescription()               { return m_Description;}
            virtual wxString GetID()                        { return m_ID;}
            virtual wxString GetSE()                        { return m_SE;}
            virtual wxString GetDepthUnits()                { return m_DepthUnits;}
            virtual wxString GetSoundingsDatum()            { return m_SoundingsDatum;}
            virtual wxString GetDatumString()               { return m_datum_str;}
            virtual wxString GetExtraInfo()                 { return m_ExtraInfo; }
            virtual wxString GetPubDate()                   { return m_PubYear;}
            virtual double GetChartErrorFactor()            { return m_Chart_Error_Factor;}
            virtual ChartDepthUnitTypePI GetDepthUnitId()   { return m_depth_unit_id;}
            virtual bool IsReadyToRender()                  { return m_bReadyToRender;}
            virtual int GetNativeScale()                    { return m_Chart_Scale; };
            virtual double GetChartSkew()                   { return m_Chart_Skew; }
            virtual wxDateTime GetEditionDate(void)         { return m_EdDate;}

//    Methods pertaining to CHART_FAMILY_RASTER type PlugIn charts only
            virtual void ComputeSourceRectangle(const PlugIn_ViewPort &vp, wxRect *pSourceRect);
            virtual double GetRasterScaleFactor();
            virtual bool GetChartBits( wxRect& source, unsigned char *pPix, int sub_samp );
            virtual int GetSize_X();
            virtual int GetSize_Y();
            virtual void latlong_to_chartpix(double lat, double lon, double &pixx, double &pixy);
            virtual void chartpix_to_latlong(double pixx, double pixy, double *plat, double *plon);
            
      protected:
            ChartTypeEnumPI     m_ChartType;
            ChartFamilyEnumPI   m_ChartFamily;

            wxString            m_FullPath;
            OcpnProjTypePI      m_projection;
            int                 m_Chart_Scale;
            double              m_Chart_Skew;

            wxDateTime          m_EdDate;
            bool                m_bReadyToRender;

            wxString          m_Name;
            wxString          m_Description;
            wxString          m_ID;
            wxString          m_SE;
            wxString          m_SoundingsDatum;
            wxString          m_datum_str;
            wxString          m_PubYear;
            wxString          m_DepthUnits;
            wxString          m_ExtraInfo;

            ChartDepthUnitTypePI  m_depth_unit_id;

            double            m_Chart_Error_Factor;


};





//    Declare an array of PlugIn_AIS_Targets
WX_DEFINE_ARRAY_PTR(PlugIn_AIS_Target *, ArrayOfPlugIn_AIS_Targets);



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
      virtual ~opencpn_plugin();

      //    Public API to the PlugIn class

      //    This group of methods is required, and will be called by the opencpn host
      //    opencpn PlugIns must implement this group
      virtual int Init(void);               // Return the PlugIn Capabilites flag
      virtual bool DeInit(void);

      virtual int GetAPIVersionMajor();
      virtual int GetAPIVersionMinor();
      virtual int GetPlugInVersionMajor();
      virtual int GetPlugInVersionMinor();
      virtual wxBitmap *GetPlugInBitmap();

      //    These three methods should produce valid, meaningful strings always
      //    ---EVEN IF--- the PlugIn has not (yet) been initialized.
      //    They are used by the PlugInManager GUI
      virtual wxString GetCommonName();
      virtual wxString GetShortDescription();
      virtual wxString GetLongDescription();

      //    This group is optional.
      //    PlugIns may override any of these methods as required

      virtual void SetDefaults(void);     //This will be called upon enabling a PlugIn via the user Dialog
                                          //It gives a chance to setup any default options and behavior

      virtual int GetToolbarToolCount(void);

      virtual int GetToolboxPanelCount(void);
      virtual void SetupToolboxPanel(int page_sel, wxNotebook* pnotebook);
      virtual void OnCloseToolboxPanel(int page_sel, int ok_apply_cancel);

      virtual void ShowPreferencesDialog( wxWindow* parent );

      virtual bool RenderOverlay(wxMemoryDC *pmdc, PlugIn_ViewPort *vp);
      virtual void SetCursorLatLon(double lat, double lon);
      virtual void SetCurrentViewPort(PlugIn_ViewPort &vp);

      virtual void SetPositionFix(PlugIn_Position_Fix &pfix);
      virtual void SetNMEASentence(wxString &sentence);
      virtual void SetAISSentence(wxString &sentence);

      virtual void ProcessParentResize(int x, int y);
      virtual void SetColorScheme(PI_ColorScheme cs);

      virtual void OnToolbarToolCallback(int id);
      virtual void OnContextMenuItemCallback(int id);

      virtual void UpdateAuiStatus(void);

      virtual wxArrayString GetDynamicChartClassNameArray(void);
 };


 // the types of the class factories used to create PlugIn instances
 typedef opencpn_plugin* create_t(void*);
 typedef void destroy_t(opencpn_plugin*);

 class DECL_EXP opencpn_plugin_16 : public opencpn_plugin
 {
       public:
             opencpn_plugin_16(void *pmgr);
             virtual ~opencpn_plugin_16();

             virtual bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);

             virtual void SetPluginMessage(wxString &message_id, wxString &message_body);

 };

class DECL_EXP opencpn_plugin_17 : public opencpn_plugin
{
       public:
             opencpn_plugin_17(void *pmgr);
             virtual ~opencpn_plugin_17();

             virtual bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
             virtual bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);

             virtual void SetPluginMessage(wxString &message_id, wxString &message_body);

};

class DECL_EXP opencpn_plugin_18 : public opencpn_plugin
{
      public:
            opencpn_plugin_18(void *pmgr);
            virtual ~opencpn_plugin_18();

            virtual bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
            virtual bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);
            virtual void SetPluginMessage(wxString &message_id, wxString &message_body);
            virtual void SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix);

};

class DECL_EXP opencpn_plugin_19 : public opencpn_plugin_18
{
      public:
            opencpn_plugin_19(void *pmgr);
            virtual ~opencpn_plugin_19();

            virtual void OnSetupOptions(void);
};

class DECL_EXP opencpn_plugin_110 : public opencpn_plugin_19
{
    public:
            opencpn_plugin_110(void *pmgr);
            virtual ~opencpn_plugin_110();

            virtual void LateInit(void); // If WANTS_LATE_INIT is returned by Init()
};

class DECL_EXP opencpn_plugin_111 : public opencpn_plugin_110
{
public:
    opencpn_plugin_111(void *pmgr);
    virtual ~opencpn_plugin_111();
    
};

class DECL_EXP opencpn_plugin_112 : public opencpn_plugin_111
{
public:
    opencpn_plugin_112(void *pmgr);
    virtual ~opencpn_plugin_112();
    
    virtual bool MouseEventHook( wxMouseEvent &event );
    virtual void SendVectorChartObjectInfo(wxString &chart, wxString &feature, wxString &objname, double lat, double lon, double scale, int nativescale);
    
};

class DECL_EXP opencpn_plugin_113 : public opencpn_plugin_112
{
public:
    opencpn_plugin_113(void *pmgr);
    virtual ~opencpn_plugin_113();
    
    virtual bool KeyboardEventHook( wxKeyEvent &event );
    virtual void OnToolbarToolDownCallback(int id);
    virtual void OnToolbarToolUpCallback(int id);
};

class DECL_EXP opencpn_plugin_114 : public opencpn_plugin_113
{
public:
  opencpn_plugin_114(void *pmgr);
  virtual ~opencpn_plugin_114();

};

class DECL_EXP opencpn_plugin_115 : public opencpn_plugin_114
{
public:
    opencpn_plugin_115(void *pmgr);
    virtual ~opencpn_plugin_115();

};

class DECL_EXP opencpn_plugin_116 : public opencpn_plugin_115
{
public:
    opencpn_plugin_116(void *pmgr);
    virtual ~opencpn_plugin_116();
    virtual bool RenderGLOverlayMultiCanvas(wxGLContext *pcontext, PlugIn_ViewPort *vp, int canvasIndex);
    virtual bool RenderOverlayMultiCanvas(wxDC &dc, PlugIn_ViewPort *vp, int canvasIndex);
    virtual void PrepareContextMenu( int canvasIndex);

};

//------------------------------------------------------------------
//      Route and Waypoint PlugIn support
//
//------------------------------------------------------------------

class DECL_EXP Plugin_Hyperlink
{
public:
    wxString DescrText;
    wxString Link;
    wxString Type;
};

WX_DECLARE_LIST(Plugin_Hyperlink, Plugin_HyperlinkList);



class DECL_EXP PlugIn_Waypoint
{
public:
    PlugIn_Waypoint();
    PlugIn_Waypoint(double lat, double lon,
                    const wxString& icon_ident, const wxString& wp_name,
                    const wxString& GUID = _T("") );
    ~PlugIn_Waypoint();

    double             m_lat;
    double             m_lon;

    wxString          m_GUID;

    wxString          m_MarkName;
    wxString          m_MarkDescription;
    wxDateTime        m_CreateTime;
	bool			  m_IsVisible;

    wxString          m_IconName;

    Plugin_HyperlinkList *m_HyperlinkList;

};

WX_DECLARE_LIST(PlugIn_Waypoint, Plugin_WaypointList);

class DECL_EXP PlugIn_Route
{
public:
    PlugIn_Route(void);
    ~PlugIn_Route(void);

    wxString    m_NameString;
    wxString    m_StartString;
    wxString    m_EndString;
    wxString    m_GUID;

    Plugin_WaypointList     *pWaypointList;
};

class DECL_EXP PlugIn_Track
{
public:
    PlugIn_Track(void);
    ~PlugIn_Track(void);

    wxString    m_NameString;
    wxString    m_StartString;
    wxString    m_EndString;
    wxString    m_GUID;

    Plugin_WaypointList     *pWaypointList;
};



//----------------------------------------------------------------------------------------------------------
//    The PlugIn CallBack API Definition
//
//    The API back up to the PlugIn Manager
//    PlugIns may call these static functions as necessary for system services
//
//----------------------------------------------------------------------------------------------------------


extern "C"  DECL_EXP int InsertPlugInTool(wxString label, wxBitmap *bitmap, wxBitmap *bmpRollover, wxItemKind kind,
                                          wxString shortHelp, wxString longHelp, wxObject *clientData, int position,
                                          int tool_sel, opencpn_plugin *pplugin);
extern "C"  DECL_EXP void RemovePlugInTool(int tool_id);
extern "C"  DECL_EXP void SetToolbarToolViz(int item, bool viz);      // Temporarily change toolbar tool viz
extern "C"  DECL_EXP void SetToolbarItemState(int item, bool toggle);
extern "C"  DECL_EXP void SetToolbarToolBitmaps(int item, wxBitmap *bitmap, wxBitmap *bmpRollover);

extern "C"  DECL_EXP int InsertPlugInToolSVG(wxString label, wxString SVGfile, wxString SVGfileRollover, wxString SVGfileToggled,
                                          wxItemKind kind, wxString shortHelp, wxString longHelp,
                                          wxObject *clientData, int position, int tool_sel, opencpn_plugin *pplugin);
extern "C"  DECL_EXP void SetToolbarToolBitmapsSVG(int item, wxString SVGfile,
                                                   wxString SVGfileRollover,
                                                   wxString SVGfileToggled );

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

extern "C"  DECL_EXP ArrayOfPlugIn_AIS_Targets *GetAISTargetArray(void);

extern "C"  DECL_EXP wxAuiManager *GetFrameAuiManager(void);

extern "C"  DECL_EXP bool AddLocaleCatalog( wxString catalog );

extern "C"  DECL_EXP void PushNMEABuffer( wxString str );

extern  DECL_EXP wxXmlDocument GetChartDatabaseEntryXML(int dbIndex, bool b_getGeom);

extern  DECL_EXP bool UpdateChartDBInplace(wxArrayString dir_array,
                          bool b_force_update,
                          bool b_ProgressDialog);
extern  DECL_EXP wxArrayString GetChartDBDirArrayString();

extern "C"  DECL_EXP void SendPluginMessage( wxString message_id, wxString message_body );

extern "C"  DECL_EXP void DimeWindow(wxWindow *);

extern "C"  DECL_EXP void JumpToPosition(double lat, double lon, double scale);

/* API 1.9  adds some common cartographic functions to avoid unnecessary code duplication */
/* Study the original OpenCPN source (georef.c) for functional definitions  */

extern "C" DECL_EXP void PositionBearingDistanceMercator_Plugin(double lat, double lon, double brg, double dist, double *dlat, double *dlon);
extern "C" DECL_EXP void DistanceBearingMercator_Plugin(double lat0, double lon0, double lat1, double lon1, double *brg, double *dist);
extern "C" DECL_EXP double DistGreatCircle_Plugin(double slat, double slon, double dlat, double dlon);

extern "C" DECL_EXP void toTM_Plugin(float lat, float lon, float lat0, float lon0, double *x, double *y);
extern "C" DECL_EXP void fromTM_Plugin(double x, double y, double lat0, double lon0, double *lat, double *lon);
extern "C" DECL_EXP void toSM_Plugin(double lat, double lon, double lat0, double lon0, double *x, double *y);
extern "C" DECL_EXP void fromSM_Plugin(double x, double y, double lat0, double lon0, double *lat, double *lon);
extern "C" DECL_EXP void toSM_ECC_Plugin(double lat, double lon, double lat0, double lon0, double *x, double *y);
extern "C" DECL_EXP void fromSM_ECC_Plugin(double x, double y, double lat0, double lon0, double *lat, double *lon);

extern "C" DECL_EXP bool DecodeSingleVDOMessage( const wxString& str, PlugIn_Position_Fix_Ex *pos, wxString *acc );
extern "C" DECL_EXP int GetChartbarHeight( void );
extern "C" DECL_EXP bool GetActiveRoutepointGPX( char *buffer, unsigned int buffer_length );



/* API 1.9 */
typedef enum OptionsParentPI
{
      PI_OPTIONS_PARENT_DISPLAY,
      PI_OPTIONS_PARENT_CONNECTIONS,
      PI_OPTIONS_PARENT_CHARTS,
      PI_OPTIONS_PARENT_SHIPS,
      PI_OPTIONS_PARENT_UI,
      PI_OPTIONS_PARENT_PLUGINS
}_OptionsParentPI;
extern DECL_EXP wxScrolledWindow *AddOptionsPage( OptionsParentPI parent, wxString title );
extern DECL_EXP bool DeleteOptionsPage( wxScrolledWindow* page );


/* API 1.10  */

/* API 1.10  adds some common functions to avoid unnecessary code duplication */
/* Study the original OpenCPN source for functional definitions  */
extern "C" DECL_EXP double toUsrDistance_Plugin( double nm_distance, int unit = -1 );
extern "C" DECL_EXP double fromUsrDistance_Plugin( double usr_distance, int unit = -1 );
extern "C" DECL_EXP double toUsrSpeed_Plugin( double kts_speed, int unit = -1 );
extern "C" DECL_EXP double fromUsrSpeed_Plugin( double usr_speed, int unit = -1 );
extern DECL_EXP wxString getUsrDistanceUnit_Plugin( int unit = -1 );
extern DECL_EXP wxString getUsrSpeedUnit_Plugin( int unit = -1 );
extern DECL_EXP wxString GetNewGUID();
extern "C" DECL_EXP bool PlugIn_GSHHS_CrossesLand(double lat1, double lon1, double lat2, double lon2);
/**
 * Start playing a sound file asynchronously. Supported formats depends
 * on sound backend.
 */
extern DECL_EXP void PlugInPlaySound( wxString &sound_file );




// API 1.10 Route and Waypoint Support
extern DECL_EXP wxBitmap *FindSystemWaypointIcon( wxString& icon_name );
extern DECL_EXP bool AddCustomWaypointIcon( wxBitmap *pimage, wxString key, wxString description );

extern DECL_EXP bool AddSingleWaypoint( PlugIn_Waypoint *pwaypoint, bool b_permanent = true);
extern DECL_EXP bool DeleteSingleWaypoint( wxString &GUID );
extern DECL_EXP bool UpdateSingleWaypoint( PlugIn_Waypoint *pwaypoint );

extern DECL_EXP bool AddPlugInRoute( PlugIn_Route *proute, bool b_permanent = true );
extern DECL_EXP bool DeletePlugInRoute( wxString& GUID );
extern DECL_EXP bool UpdatePlugInRoute ( PlugIn_Route *proute );

extern DECL_EXP bool AddPlugInTrack( PlugIn_Track *ptrack, bool b_permanent = true );
extern DECL_EXP bool DeletePlugInTrack( wxString& GUID );
extern DECL_EXP bool UpdatePlugInTrack ( PlugIn_Track *ptrack );

/* API 1.11  */

/* API 1.11  adds some more common functions to avoid unnecessary code duplication */
wxColour DECL_EXP GetBaseGlobalColor(wxString colorName);
int DECL_EXP OCPNMessageBox_PlugIn(wxWindow *parent,
                          const wxString& message,
                          const wxString& caption = _T("Message"),
                          int style = wxOK, int x = -1, int y = -1);

extern DECL_EXP wxString toSDMM_PlugIn(int NEflag, double a, bool hi_precision = true);

extern "C"  DECL_EXP wxString *GetpPrivateApplicationDataLocation();
extern  DECL_EXP wxString GetOCPN_ExePath( void );
extern "C"  DECL_EXP wxString *GetpPlugInLocation();
extern  DECL_EXP wxString GetPlugInPath(opencpn_plugin *pplugin);

extern "C"  DECL_EXP int AddChartToDBInPlace( wxString &full_path, bool b_RefreshCanvas );
extern "C"  DECL_EXP int RemoveChartFromDBInPlace( wxString &full_path );
extern  DECL_EXP wxString GetLocaleCanonicalName();

//  API 1.11 adds access to S52 Presentation library
//Types

//      A flag field that defines the object capabilities passed by a chart to the S52 PLIB

#define PLIB_CAPS_LINE_VBO              1
#define PLIB_CAPS_LINE_BUFFER           1 << 1
#define PLIB_CAPS_SINGLEGEO_BUFFER      1 << 2
#define PLIB_CAPS_OBJSEGLIST            1 << 3
#define PLIB_CAPS_OBJCATMUTATE          1 << 4


class PI_S57Obj;

WX_DECLARE_LIST(PI_S57Obj, ListOfPI_S57Obj);

// ----------------------------------------------------------------------------
// PlugInChartBaseGL
//  Derived from PlugInChartBase, add OpenGL Vector chart support
// ----------------------------------------------------------------------------

class DECL_EXP PlugInChartBaseGL : public PlugInChartBase
{
public:
    PlugInChartBaseGL();
    virtual ~PlugInChartBaseGL();

    virtual int RenderRegionViewOnGL( const wxGLContext &glc, const PlugIn_ViewPort& VPoint,
                                      const wxRegion &Region, bool b_use_stencil );

    virtual ListOfPI_S57Obj *GetObjRuleListAtLatLon(float lat, float lon, float select_radius, PlugIn_ViewPort *VPoint);
    virtual wxString CreateObjDescriptions( ListOfPI_S57Obj* obj_list );

    virtual int GetNoCOVREntries();
    virtual int GetNoCOVRTablePoints(int iTable);
    virtual int  GetNoCOVRTablenPoints(int iTable);
    virtual float *GetNoCOVRTableHead(int iTable);
    
};


// ----------------------------------------------------------------------------
// PlugInChartBaseExtended
//  Derived from PlugInChartBase, add extended chart support methods
// ----------------------------------------------------------------------------

class DECL_EXP PlugInChartBaseExtended : public PlugInChartBase
{
public:
    PlugInChartBaseExtended();
    virtual ~PlugInChartBaseExtended();
    
    virtual int RenderRegionViewOnGL( const wxGLContext &glc, const PlugIn_ViewPort& VPoint,
                                      const wxRegion &Region, bool b_use_stencil );
    
    virtual wxBitmap &RenderRegionViewOnDCNoText(  const PlugIn_ViewPort& VPoint, const wxRegion &Region);
    virtual bool RenderRegionViewOnDCTextOnly( wxMemoryDC &dc, const PlugIn_ViewPort& VPoint, const wxRegion &Region);
    
    virtual int RenderRegionViewOnGLNoText( const wxGLContext &glc, const PlugIn_ViewPort& VPoint,
                                            const wxRegion &Region, bool b_use_stencil );

    virtual int RenderRegionViewOnGLTextOnly( const wxGLContext &glc, const PlugIn_ViewPort& VPoint,
                                              const wxRegion &Region, bool b_use_stencil );
    
    virtual ListOfPI_S57Obj *GetObjRuleListAtLatLon(float lat, float lon, float select_radius, PlugIn_ViewPort *VPoint);
    virtual wxString CreateObjDescriptions( ListOfPI_S57Obj* obj_list );
    
    virtual int GetNoCOVREntries();
    virtual int GetNoCOVRTablePoints(int iTable);
    virtual int  GetNoCOVRTablenPoints(int iTable);
    virtual float *GetNoCOVRTableHead(int iTable);
    
    virtual void ClearPLIBTextList();
    
};






class wxArrayOfS57attVal;

// name of the addressed look up table set (fifth letter)
typedef enum _PI_LUPname{
    PI_SIMPLIFIED                             = 'L', // points
    PI_PAPER_CHART                            = 'R', // points
    PI_LINES                                  = 'S', // lines
    PI_PLAIN_BOUNDARIES                       = 'N', // areas
    PI_SYMBOLIZED_BOUNDARIES                  = 'O', // areas
    PI_LUPNAME_NUM                            = 5
}PI_LUPname;

// display category type
typedef enum _PI_DisCat{
    PI_DISPLAYBASE          = 'D',            //
    PI_STANDARD             = 'S',            //
    PI_OTHER                = 'O',            // O for OTHER
    PI_MARINERS_STANDARD    = 'M',            // Mariner specified
    PI_MARINERS_OTHER,                        // value not defined
    PI_DISP_CAT_NUM,                          // value not defined
}PI_DisCat;

// Display Priority
typedef enum _PI_DisPrio{
    PI_PRIO_NODATA          = '0',                  // no data fill area pattern
    PI_PRIO_GROUP1          = '1',                  // S57 group 1 filled areas
    PI_PRIO_AREA_1          = '2',                  // superimposed areas
    PI_PRIO_AREA_2          = '3',                  // superimposed areas also water features
    PI_PRIO_SYMB_POINT      = '4',                  // point symbol also land features
    PI_PRIO_SYMB_LINE       = '5',                  // line symbol also restricted areas
    PI_PRIO_SYMB_AREA       = '6',                  // area symbol also traffic areas
    PI_PRIO_ROUTEING        = '7',                  // routeing lines
    PI_PRIO_HAZARDS         = '8',                  // hazards
    PI_PRIO_MARINERS        = '9',                  // VRM, EBL, own ship
    PI_PRIO_NUM             = 10                    // number of priority levels

}PI_DisPrio;

typedef enum PI_InitReturn
{
    PI_INIT_OK = 0,
    PI_INIT_FAIL_RETRY,        // Init failed, retry suggested
    PI_INIT_FAIL_REMOVE,       // Init failed, suggest remove from further use
    PI_INIT_FAIL_NOERROR       // Init failed, request no explicit error message
}_PI_InitReturn;

class PI_line_segment_element
{
public:
    size_t              vbo_offset;
    size_t              n_points;
    int                 priority;
    float               lat_max;                // segment bounding box
    float               lat_min;
    float               lon_max;
    float               lon_min;
    int                 type;
    void                *private0;
   
    PI_line_segment_element *next;
};


class DECL_EXP PI_S57Obj
{
public:

      //  Public Methods
      PI_S57Obj();
      ~PI_S57Obj();

public:
      // Instance Data
      char                    FeatureName[8];
      int                     Primitive_type;

      char                    *att_array;
      wxArrayOfS57attVal      *attVal;
      int                     n_attr;

      int                     iOBJL;
      int                     Index;

      double                  x;                      // for POINT
      double                  y;
      double                  z;
      int                     npt;                    // number of points as needed by arrays
      void                    *geoPt;                 // for LINE & AREA not described by PolyTessGeo
      double                  *geoPtz;                // an array[3] for MultiPoint, SM with Z, i.e. depth
      double                  *geoPtMulti;            // an array[2] for MultiPoint, lat/lon to make bbox
                                                      // of decomposed points

      void                    *pPolyTessGeo;

      double                  m_lat;                  // The lat/lon of the object's "reference" point
      double                  m_lon;

      double                  chart_ref_lat;
      double                  chart_ref_lon;

      double                  lat_min;
      double                  lat_max;
      double                  lon_min;
      double                  lon_max;

      int                     Scamin;                 // SCAMIN attribute decoded during load

      bool                    bIsClone;
      int                     nRef;                   // Reference counter, to signal OK for deletion

      bool                    bIsAton;                // This object is an aid-to-navigation
      bool                    bIsAssociable;          // This object is DRGARE or DEPARE

      int                     m_n_lsindex;
      int                     *m_lsindex_array;
      int                     m_n_edge_max_points;
      void                    *m_chart_context;

      PI_DisCat               m_DisplayCat;

      void *                  S52_Context;
      PI_S57Obj               *child;           // child list, used only for MultiPoint Soundings

      PI_S57Obj               *next;            //  List linkage

                                                      // This transform converts from object geometry
                                                      // to SM coordinates.
      double                  x_rate;                 // These auxiliary transform coefficients are
      double                  y_rate;                 // to be used in GetPointPix() and friends
      double                  x_origin;               // on a per-object basis if necessary
      double                  y_origin;
      
      int auxParm0;                                   // some per-object auxiliary parameters, used for OpenGL
      int auxParm1;
      int auxParm2;
      int auxParm3;

      PI_line_segment_element *m_ls_list;
      bool                    m_bcategory_mutable;
      int                     m_DPRI;
};



wxString DECL_EXP PI_GetPLIBColorScheme();
int DECL_EXP PI_GetPLIBDepthUnitInt();
int DECL_EXP PI_GetPLIBSymbolStyle();
int DECL_EXP PI_GetPLIBBoundaryStyle();
int DECL_EXP PI_GetPLIBStateHash();
double DECL_EXP PI_GetPLIBMarinerSafetyContour();
bool DECL_EXP PI_GetObjectRenderBox( PI_S57Obj *pObj, double *lat_min, double *lat_max, double *lon_min, double *lon_max);
void DECL_EXP PI_UpdateContext(PI_S57Obj *pObj);

bool DECL_EXP PI_PLIBObjectRenderCheck( PI_S57Obj *pObj, PlugIn_ViewPort *vp );
PI_LUPname DECL_EXP PI_GetObjectLUPName( PI_S57Obj *pObj );
PI_DisPrio DECL_EXP PI_GetObjectDisplayPriority( PI_S57Obj *pObj );
PI_DisCat DECL_EXP PI_GetObjectDisplayCategory( PI_S57Obj *pObj );
void DECL_EXP PI_PLIBSetLineFeaturePriority( PI_S57Obj *pObj, int prio );
void DECL_EXP PI_PLIBPrepareForNewRender(void);
void DECL_EXP PI_PLIBFreeContext( void *pContext );
void DECL_EXP PI_PLIBSetRenderCaps( unsigned int flags );

bool DECL_EXP PI_PLIBSetContext( PI_S57Obj *pObj );

int DECL_EXP PI_PLIBRenderObjectToDC( wxDC *pdc, PI_S57Obj *pObj, PlugIn_ViewPort *vp );
int DECL_EXP PI_PLIBRenderAreaToDC( wxDC *pdc, PI_S57Obj *pObj, PlugIn_ViewPort *vp, wxRect rect, unsigned char *pixbuf );


int DECL_EXP PI_PLIBRenderAreaToGL( const wxGLContext &glcc, PI_S57Obj *pObj,
                                    PlugIn_ViewPort *vp, wxRect &render_rect );

int DECL_EXP PI_PLIBRenderObjectToGL( const wxGLContext &glcc, PI_S57Obj *pObj,
                                    PlugIn_ViewPort *vp, wxRect &render_rect );

/* API 1.11 OpenGL Display List and vertex buffer object routines

   Effectively these two routines cancel each other so all
   of the translation, scaling and rotation can be done by opengl.

   Display lists need only be built infrequently, but used in each frame
   greatly accelerates the speed of rendering.  This avoids costly calculations,
   and also allows the vertexes to be stored in graphics memory.

   static int dl = 0;
   glPushMatrix();
   PlugInMultMatrixViewport(current_viewport);
   if(dl)
      glCallList(dl);
   else {
      dl = glGenLists(1);
      PlugInViewPort norm_viewport = current_viewport;
      NormalizeViewPort(norm_viewport);
      glNewList(dl, GL_COMPILE_AND_EXECUTE);
      ... // use norm_viewport with GetCanvasLLPix here
      glEndList();
   }      
   glPopMatrix();
   ... // use current_viewport with GetCanvasLLPix again
*/

extern DECL_EXP bool PlugInHasNormalizedViewPort ( PlugIn_ViewPort *vp );
extern DECL_EXP void PlugInMultMatrixViewport ( PlugIn_ViewPort *vp, float lat=0, float lon=0 );
extern DECL_EXP void PlugInNormalizeViewport ( PlugIn_ViewPort *vp, float lat=0, float lon=0 );

class wxPoint2DDouble;
extern "C"  DECL_EXP void GetDoubleCanvasPixLL(PlugIn_ViewPort *vp, wxPoint2DDouble *pp, double lat, double lon);

/* API 1.13  */
/* API 1.13  adds some more common functions to avoid unnecessary code duplication */


extern DECL_EXP double fromDMM_Plugin( wxString sdms );
extern DECL_EXP void SetCanvasRotation(double rotation);
extern DECL_EXP void SetCanvasProjection(int projection);
extern DECL_EXP bool GetSingleWaypoint( wxString GUID, PlugIn_Waypoint *pwaypoint );
extern DECL_EXP bool CheckEdgePan_PlugIn( int x, int y, bool dragging, int margin, int delta );
extern DECL_EXP wxBitmap GetIcon_PlugIn(const wxString & name);
extern DECL_EXP void SetCursor_PlugIn( wxCursor *pPlugin_Cursor = NULL );
extern DECL_EXP wxFont *GetOCPNScaledFont_PlugIn(wxString TextElement, int default_size = 0);
extern DECL_EXP wxFont GetOCPNGUIScaledFont_PlugIn(wxString item);
extern DECL_EXP double GetOCPNGUIToolScaleFactor_PlugIn(int GUIScaledFactor);
extern DECL_EXP double GetOCPNGUIToolScaleFactor_PlugIn();
extern DECL_EXP float  GetOCPNChartScaleFactor_Plugin();
extern DECL_EXP wxColour GetFontColour_PlugIn(wxString TextElement);

extern DECL_EXP double GetCanvasTilt();
extern DECL_EXP void SetCanvasTilt(double tilt);

/**
 * Start playing a sound file asynchronously. Supported formats depends
 * on sound backend. The deviceIx is only used on platforms using the
 * portaudio sound backend where -1 indicates the default device.
 */
extern DECL_EXP bool PlugInPlaySoundEx( wxString &sound_file, int deviceIndex=-1 );
extern DECL_EXP void AddChartDirectory( wxString &path );
extern DECL_EXP void ForceChartDBUpdate();

extern  DECL_EXP wxString GetWritableDocumentsDir( void );
extern  DECL_EXP wxDialog *GetActiveOptionsDialog();
extern  DECL_EXP wxArrayString GetWaypointGUIDArray( void );
extern  DECL_EXP wxArrayString GetIconNameArray(void);

extern  DECL_EXP bool AddPersistentFontKey(wxString TextElement);
extern  DECL_EXP wxString GetActiveStyleName();

extern  DECL_EXP wxBitmap GetBitmapFromSVGFile(wxString filename, unsigned int width, unsigned int height);
extern  DECL_EXP bool IsTouchInterface_PlugIn(void);

/*  Platform optimized File/Dir selector dialogs */
extern  DECL_EXP int PlatformDirSelectorDialog( wxWindow *parent, wxString *file_spec, wxString Title, wxString initDir);

extern  DECL_EXP int PlatformFileSelectorDialog( wxWindow *parent, wxString *file_spec, wxString Title, wxString initDir,
                                wxString suggestedName, wxString wildcard);


/*  OpenCPN HTTP File Download PlugIn Interface   */

/*   Various method Return Codes, etc          */
typedef enum _OCPN_DLStatus{
    OCPN_DL_UNKNOWN         =-1,
    OCPN_DL_NO_ERROR        = 0,
    OCPN_DL_FAILED          = 1,
    OCPN_DL_ABORTED         = 2,
    OCPN_DL_USER_TIMEOUT    = 4,
    OCPN_DL_STARTED         = 8
}OCPN_DLStatus;


typedef enum _OCPN_DLCondition{
    OCPN_DL_EVENT_TYPE_UNKNOWN      = -1,
    OCPN_DL_EVENT_TYPE_START        = 80,
    OCPN_DL_EVENT_TYPE_PROGRESS     = 81,
    OCPN_DL_EVENT_TYPE_END          = 82
}OCPN_DLCondition;

//      Style definitions for Synchronous file download modal dialogs, if desired.
//      Abstracted from wxCURL package
enum OCPN_DLDialogStyle
{
    OCPN_DLDS_ELAPSED_TIME = 0x0001,       //!< The dialog shows the elapsed time.
    OCPN_DLDS_ESTIMATED_TIME = 0x0002,     //!< The dialog shows the estimated total time.
    OCPN_DLDS_REMAINING_TIME = 0x0004,     //!< The dialog shows the remaining time.
    OCPN_DLDS_SPEED = 0x0008,              //!< The dialog shows the transfer speed.
    OCPN_DLDS_SIZE = 0x0010,               //!< The dialog shows the size of the resource to download/upload.
    OCPN_DLDS_URL = 0x0020,                //!< The dialog shows the URL involved in the transfer.
    
    // styles related to the use of wxCurlConnectionSettingsDialog:
    
    OCPN_DLDS_CONN_SETTINGS_AUTH = 0x0040,  //!< The dialog allows the user to change the authentication settings.
    OCPN_DLDS_CONN_SETTINGS_PORT = 0x0080,  //!< The dialog allows the user to change the port for the transfer.
    OCPN_DLDS_CONN_SETTINGS_PROXY = 0x0100, //!< The dialog allows the user to change the proxy settings.
    
    OCPN_DLDS_CONN_SETTINGS_ALL = OCPN_DLDS_CONN_SETTINGS_AUTH|OCPN_DLDS_CONN_SETTINGS_PORT|OCPN_DLDS_CONN_SETTINGS_PROXY,
    
    OCPN_DLDS_SHOW_ALL =OCPN_DLDS_ELAPSED_TIME|OCPN_DLDS_ESTIMATED_TIME|OCPN_DLDS_REMAINING_TIME|
    OCPN_DLDS_SPEED|OCPN_DLDS_SIZE|OCPN_DLDS_URL|OCPN_DLDS_CONN_SETTINGS_ALL,
    
    OCPN_DLDS_CAN_ABORT = 0x0200,          //!< The transfer can be aborted by the user.
    OCPN_DLDS_CAN_START = 0x0400,          //!< The transfer won't start automatically. The user needs to start it.
    OCPN_DLDS_CAN_PAUSE = 0x0800,          //!< The transfer can be paused.
    
    OCPN_DLDS_AUTO_CLOSE = 0x1000,         //!< The dialog auto closes when transfer is complete.
    
    // by default all available features are enabled:
    OCPN_DLDS_DEFAULT_STYLE = OCPN_DLDS_CAN_START|OCPN_DLDS_CAN_PAUSE|OCPN_DLDS_CAN_ABORT|OCPN_DLDS_SHOW_ALL|OCPN_DLDS_AUTO_CLOSE
};

#define ONLINE_CHECK_RETRY 30 // Recheck the Internet connection availability every ONLINE_CHECK_RETRY s

/*   Synchronous (Blocking) download of a single file  */

extern DECL_EXP _OCPN_DLStatus OCPN_downloadFile( const wxString& url, const wxString &outputFile, 
                                       const wxString &title, const wxString &message, 
                                       const wxBitmap& bitmap,
                                       wxWindow *parent, long style, int timeout_secs);


/*   Asynchronous (Background) download of a single file  */

extern DECL_EXP _OCPN_DLStatus OCPN_downloadFileBackground( const wxString& url, const wxString &outputFile,
                                                  wxEvtHandler *handler, long *handle);

extern DECL_EXP void OCPN_cancelDownloadFileBackground( long handle );

/*   Synchronous (Blocking) HTTP POST operation for small amounts of data */

extern DECL_EXP _OCPN_DLStatus OCPN_postDataHttp( const wxString& url, const wxString& parameters, wxString& result, int timeout_secs );

/*   Check whether connection to the Internet is working */

extern DECL_EXP bool OCPN_isOnline();

/*  Supporting  Event for Background downloading          */
/*  OCPN_downloadEvent Definition  */

/*  PlugIn should be ready/able to handle this event after initiating a background file transfer  
 * 
 * The event as received should be parsed primarily by the getDLEventCondition() method.
 * This will allow identification of download start, progress, and end states.
 * 
 * Other accessor methods contain status, byte counts, etc.
 * 
 * A PlugIn may safely destroy its EvtHandler after receipt of an OCPN_downloadEvent with 
 *     getDLEventCondition == OCPN_DL_EVENT_TYPE_END
 */

class DECL_EXP OCPN_downloadEvent: public wxEvent
{
public:
    OCPN_downloadEvent( wxEventType commandType = wxEVT_NULL, int id = 0 );
    ~OCPN_downloadEvent( );
    
    // accessors
    _OCPN_DLStatus getDLEventStatus(){ return m_stat; }
    OCPN_DLCondition getDLEventCondition(){ return m_condition; }
    
    void setDLEventStatus( _OCPN_DLStatus stat ){ m_stat = stat; }
    void setDLEventCondition( OCPN_DLCondition cond ){ m_condition = cond; }
    
    void setTotal( long bytes ){m_totalBytes = bytes; }
    void setTransferred( long bytes ){m_sofarBytes = bytes; }
    long getTotal(){ return m_totalBytes; }
    long getTransferred(){ return m_sofarBytes; }
    
    void setComplete(bool b_complete){ m_b_complete = b_complete; }
    bool getComplete(){ return m_b_complete; }
    
    
    // required for sending with wxPostEvent()
    wxEvent *Clone() const;
    
private:
    OCPN_DLStatus m_stat;
    OCPN_DLCondition m_condition;
    
    long m_totalBytes;
    long m_sofarBytes;
    bool m_b_complete;
};


//extern WXDLLIMPEXP_CORE const wxEventType wxEVT_DOWNLOAD_EVENT;

#ifdef MAKING_PLUGIN
extern   DECL_IMP wxEventType wxEVT_DOWNLOAD_EVENT;
#else
extern   DECL_EXP wxEventType wxEVT_DOWNLOAD_EVENT;
#endif

// API 1.14 Extra canvas Support

/* Allow drawing of objects onto other OpenGL canvases */
extern DECL_EXP void PlugInAISDrawGL( wxGLCanvas* glcanvas, const PlugIn_ViewPort& vp );
extern DECL_EXP bool PlugInSetFontColor(const wxString TextElement, const wxColour color);

// API 1.15
extern DECL_EXP double PlugInGetDisplaySizeMM();

// 
extern DECL_EXP wxFont* FindOrCreateFont_PlugIn( int point_size, wxFontFamily family, 
                    wxFontStyle style, wxFontWeight weight, bool underline = false,
                    const wxString &facename = wxEmptyString,
                    wxFontEncoding encoding = wxFONTENCODING_DEFAULT );

extern DECL_EXP int PlugInGetMinAvailableGshhgQuality();
extern DECL_EXP int PlugInGetMaxAvailableGshhgQuality();

extern DECL_EXP void PlugInHandleAutopilotRoute(bool enable);

// API 1.16
//
/**
 * Return the plugin data directory for a given directory name.
 *
 * On Linux, the returned data path is an existing directory ending in
 * "opencpn/plugins/<plugin_name>" where the last part is the plugin_name
 * argument. The prefix part is one of the directories listed in the
 * environment variable XDG_DATA_DIRS, by default
 * ~/.local/share:/usr/local/share:/usr/share.
 *
 * On other platforms, the returned value is GetSharedDataDir() +
 * "/opencpn/plugins/" + plugin_name (with native path separators)
 * if that path exists.
 *
 * Return "" if no existing directory is found.
 */
extern DECL_EXP wxString GetPluginDataDir(const char* plugin_name);

extern DECL_EXP bool ShuttingDown( void );

//  Support for MUI MultiCanvas model

extern DECL_EXP wxWindow* PluginGetFocusCanvas();
extern DECL_EXP wxWindow* PluginGetOverlayRenderCanvas();

extern "C"  DECL_EXP void CanvasJumpToPosition( wxWindow *canvas, double lat, double lon, double scale);
extern "C"  DECL_EXP  int AddCanvasMenuItem(wxMenuItem *pitem, opencpn_plugin *pplugin, const char *name = "");
extern "C"  DECL_EXP void RemoveCanvasMenuItem(int item, const char *name = "");      // Fully remove this item
extern "C"  DECL_EXP void SetCanvasMenuItemViz(int item, bool viz, const char *name = ""); // Temporarily change context menu options
extern "C"  DECL_EXP void SetCanvasMenuItemGrey(int item, bool grey, const char *name = "");

// Extract waypoints, routes and tracks
extern DECL_EXP wxString GetSelectedWaypointGUID_Plugin( );
extern DECL_EXP wxString GetSelectedRouteGUID_Plugin( );
extern DECL_EXP wxString GetSelectedTrackGUID_Plugin( );

extern DECL_EXP std::unique_ptr<PlugIn_Waypoint> GetWaypoint_Plugin( const wxString& ); // doublon with GetSingleWaypoint
extern DECL_EXP std::unique_ptr<PlugIn_Route> GetRoute_Plugin( const wxString& );
extern DECL_EXP std::unique_ptr<PlugIn_Track> GetTrack_Plugin( const wxString& );

extern DECL_EXP wxWindow* GetCanvasUnderMouse( );
extern DECL_EXP int GetCanvasIndexUnderMouse( );
//extern DECL_EXP std::vector<wxWindow *> GetCanvasArray();
extern DECL_EXP wxWindow *GetCanvasByIndex( int canvasIndex );
extern DECL_EXP int GetCanvasCount( );
extern DECL_EXP bool CheckMUIEdgePan_PlugIn( int x, int y, bool dragging, int margin, int delta, int canvasIndex );
extern DECL_EXP void SetMUICursor_PlugIn( wxCursor *pCursor, int canvasIndex );

enum SDDMFORMAT
{
    DEGREES_DECIMAL_MINUTES = 0,
    DECIMAL_DEGREES,
    DEGREES_MINUTES_SECONDS,
    END_SDDMFORMATS
};

extern DECL_EXP int GetLatLonFormat(void);

#endif //_PLUGIN_H_
