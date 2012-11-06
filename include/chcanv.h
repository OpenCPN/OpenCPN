/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Canvas
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
 ***************************************************************************
 *
 */

#ifndef __CHCANV_H__
#define __CHCANV_H__

#include "bbox.h"

#include <wx/datetime.h>
#include <wx/treectrl.h>
#include "wx/dirctrl.h"
#include <wx/sound.h>
#include <wx/grid.h>
#include <wx/wxhtml.h>

#include <wx/glcanvas.h>

#include "chart1.h"                 // for enum types
#include "ocpndc.h"
#include "gshhs.h"
#include "undo.h"

//    Useful static routines
void ShowAISTargetQueryDialog(wxWindow *parent, int mmsi);

//--------------------------------------------------------
//    Screen Brightness Control Support Routines
//
//--------------------------------------------------------

int InitScreenBrightness(void);
int RestoreScreenBrightness(void);
int SetScreenBrightness(int brightness);

//Central dimmer...
void DimeControl(wxWindow* ctrl);
void DimeControl(wxWindow* ctrl, wxColour col, wxColour col1, wxColour back_color,wxColour text_color,wxColour uitext, wxColour udkrd, wxColour gridline);

//    Set up the preferred quilt type
#define QUILT_TYPE_2

//----------------------------------------------------------------------------
//    Forward Declarations
//----------------------------------------------------------------------------
      class Route;
      class TCWin;
      class RoutePoint;
      class SelectItem;
      class wxBoundingBox;
      class ocpnBitmap;
      class WVSChart;
      class MyFrame;
      class ChartBaseBSB;
      class ChartBase;
      class AIS_Target_Data;
      class IDX_entry;
      class S57ObjectTree;
      class S57ObjectDesc;
      class RolloverWin;
      class Quilt;
      class PixelCache;
      class ChInfoWin;
      class glChartCanvas;

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#define     RESCALE_TIMER     1
#define     PAN_TIMER         2
#define     CURTRACK_TIMER    3
#define     ROT_TIMER         4
#define     RTELEGPU_TIMER    5
#define     TCWININF_TIMER    6
#define     ROLLOVER_TIMER    7
#define     ZOOM_TIMER        8
#define     PANKEY_TIMER      9
#define     DBLCLICK_TIMER   10

enum                                //  specify the render behaviour of SetViewPoint()
{
    CURRENT_RENDER,                 // use the current render type
    FORCE_SUBSAMPLE                 // force sub-sampled render, with re-render timer
};

//          Cursor region enumerator
enum
{
      CENTER,
      MID_RIGHT,
      MID_LEFT,
      MID_TOP,
      MID_BOT,
};

typedef enum ownship_state_t
{
      SHIP_NORMAL        = 0,
      SHIP_LOWACCURACY,
      SHIP_INVALID
}_ownship_state_t;

enum {
      ID_S57QUERYTREECTRL =            10000,
      ID_AISDIALOGOK
};

class emboss_data
{
      public:
            emboss_data(){ pmap = NULL; gltexind = 0; }
            ~emboss_data(){ free(pmap); }

            int         *pmap;
            int         width;
            int         height;

            GLuint      gltexind;
      private:
};

//----------------------------------------------------------------------------
// ChartCanvas
//----------------------------------------------------------------------------
class ChartCanvas: public wxWindow
{
     friend class glChartCanvas;
public:
      ChartCanvas(wxFrame *frame);
      ~ChartCanvas();

      //    Methods
      void OnKeyDown(wxKeyEvent &event);
      void OnKeyUp(wxKeyEvent &event);
      void OnPaint(wxPaintEvent& event);
      void PaintCleanup();
      void Scroll(int dx, int dy);
      void CanvasPopupMenu(int x, int y, int seltype);

      void PopupMenuHandler(wxCommandEvent& event);

      virtual bool SetCursor(const wxCursor &c);
      virtual void Refresh( bool eraseBackground = true,
                            const wxRect *rect = (const wxRect *) NULL );
      virtual void Update();

      void LostMouseCapture(wxMouseCaptureLostEvent& event);

      void CancelMouseRoute();

      void Do_Pankeys(wxTimerEvent& event);

      bool SetViewPoint(double lat, double lon, double scale_ppm, double skew, double rotation, bool b_adjust = true);
      bool SetVPScale(double sc);
      bool SetViewPoint ( double lat, double lon);
      void ReloadVP ( bool b_adjust = true );
      void LoadVP ( ViewPort &vp, bool b_adjust = true );
      void SetVPRotation(double angle){ VPoint.rotation = angle; }
      double GetVPRotation(void) { return GetVP().rotation; }
      double GetVPSkew(void) { return GetVP().skew; }
      void ClearbFollow(void);

      void GetCanvasPointPix(double rlat, double rlon, wxPoint *r);
      void GetCanvasPixPoint(int x, int y, double &lat, double &lon);
      void WarpPointerDeferred(int x, int y);
      void UpdateShips();
      void UpdateAIS();
      void UpdateAlerts();                          // pjotrc 2010.02.22

      void SetQuiltMode(bool b_quilt);
      bool GetQuiltMode(void);
      ArrayOfInts GetQuiltIndexArray(void);
      bool IsQuiltDelta(void);
      void SetQuiltChartHiLiteIndex(int dbIndex);
      int GetQuiltReferenceChartIndex(void);

      int GetNextContextMenuId();

      void SetColorScheme(ColorScheme cs);
      ColorScheme GetColorScheme(){ return m_cs;}

      wxString FormatDistanceAdaptive( double distance );

      //    Accessors
      int GetCanvasWidth(){ return m_canvas_width;}
      int GetCanvasHeight(){ return m_canvas_height;}
      float GetVPScale(){return GetVP().view_scale_ppm;}
      float GetVPChartScale(){return GetVP().chart_scale;}
      double GetCanvasScaleFactor(){return m_canvas_scale_factor;}
      double GetCanvasTrueScale(){return m_true_scale_ppm;}
      double GetAbsoluteMinScalePpm(){ return m_absolute_min_scale_ppm; }
      ViewPort &GetVP();
      ChartBase* GetChartAtCursor();
      ChartBase* GetOverlayChartAtCursor();

      glChartCanvas *GetglCanvas(){ return m_glcc; }
      GSHHSChart* GetWorldBackgroundChart() { return pWorldBackgroundChart; }

      void  SetbTCUpdate(bool f){ m_bTCupdate = f;}
      bool  GetbTCUpdate(){ return m_bTCupdate;}
      void  SetbShowCurrent(bool f){ m_bShowCurrent = f;}
      bool  GetbShowCurrent(){ return m_bShowCurrent;}
      void  SetbShowTide(bool f){ m_bShowTide = f;}
      bool  GetbShowTide(){ return m_bShowTide;}
      double GetPixPerMM(){ return m_pix_per_mm;}

      void SetOwnShipState(ownship_state_t state){ m_ownship_state = state;}
      void GetCursorLatLon(double *lat, double *lon);

      bool ZoomCanvasIn(double factor);
      bool ZoomCanvasOut(double factor);
      bool DoZoomCanvasIn(double factor);
      bool DoZoomCanvasOut(double factor);

      bool PanCanvas(int dx, int dy);

      void ShowAISTargetList(void);

      void ShowGoToPosition(void);

      ChartBase *GetLargestScaleQuiltChart();
      ChartBase *GetFirstQuiltChart();
      ChartBase *GetNextQuiltChart();
      int GetQuiltChartCount();
      void InvalidateAllQuiltPatchs(void);
      void SetQuiltRefChart(int dbIndex);
      ArrayOfInts GetQuiltCandidatedbIndexArray(bool flag1 = true, bool flag2 = true);
      ArrayOfInts GetQuiltExtendedStackdbIndexArray();
      ArrayOfInts GetQuiltEclipsedStackdbIndexArray();
      int GetQuiltRefChartdbIndex(void);
      void InvalidateQuilt(void);
      double GetQuiltMaxErrorFactor();
      bool IsChartQuiltableRef(int db_index);
      bool IsChartLargeEnoughToRender( ChartBase* chart, ViewPort& vp );
      int GetCanvasChartNativeScale();
      int FindClosestCanvasChartdbIndex(int scale);
      void UpdateCanvasOnGroupChange(void);


      void ShowChartInfoWindow(int x, int y, int dbIndex);
      void HideChartInfoWindow(void);
      void CancelMeasureRoute();

      //Todo build more accessors
      bool        m_bFollow;
      wxCursor    *pCursorPencil;
	  wxCursor    *pCursorArrow;
	  wxCursor    *pCursorCross;
      TCWin       *pCwin;
      wxBitmap    *pscratch_bm;
      double      m_cursor_lon, m_cursor_lat;
      Undo        *undo;
      wxPoint     r_rband;
      double      m_prev_rlat;
      double      m_prev_rlon;
      RoutePoint  *m_prev_pMousePoint;

      bool PurgeGLCanvasChartCache(ChartBase *pc);

      void RemovePointFromRoute( RoutePoint* point, Route* route );


private:
      ViewPort    VPoint;
      void        PositionConsole(void);
      void        FinishRoute(void);

      ChInfoWin   *m_pCIWin;

      bool        m_bShowCurrent;
      bool        m_bShowTide;
      int         cursor_region;
      bool        m_bTCupdate;

      wxRect      bbRect;

      wxPoint     LastShipPoint;
      wxPoint     LastPredPoint;
      bool        m_bDrawingRoute;
      bool        m_bRouteEditing;
      bool        m_bMarkEditing;
      RoutePoint  *m_pRoutePointEditTarget;
      SelectItem  *m_pFoundPoint;
      bool        m_bChartDragging;

      Route       *m_pMouseRoute;
      Route       *m_pSelectedRoute;
      Route       *m_pSelectedTrack;
      wxArrayPtrVoid *m_pEditRouteArray;
      RoutePoint  *m_pFoundRoutePoint;
      RoutePoint  *m_pFoundRoutePointSecond;

      int         m_FoundAIS_MMSI;

      wxCursor    *pCursorLeft;
      wxCursor    *pCursorRight;
      wxCursor    *pCursorUp;
      wxCursor    *pCursorDown;

      wxCursor    *pCursorUpLeft;
      wxCursor    *pCursorUpRight;
      wxCursor    *pCursorDownLeft;
      wxCursor    *pCursorDownRight;

      int         popx, popy;
      bool        m_bAppendingRoute;

      wxBitmap    *pThumbDIBShow;
      wxBitmap    *pThumbShowing;

      bool        bShowingCurrent;
      bool        bShowingTide;

      double       m_canvas_scale_factor;    // converter....
                                             // useage....
                                             // true_chart_scale_on_display = m_canvas_scale_factor / pixels_per_meter of displayed chart
                                             // also may be considered as the "pixels-per-meter" of the canvas on-screen
      double      m_absolute_min_scale_ppm;

      int m_panx, m_pany, m_panspeed, m_modkeys;
      bool m_bmouse_key_mod;

      bool singleClickEventIsValid;
      wxMouseEvent singleClickEvent;

      std::vector<s57Sector_t> extendedSectorLegs;

      //    Methods
      void OnActivate(wxActivateEvent& event);
      void OnSize(wxSizeEvent& event);
      void MouseTimedEvent(wxTimerEvent& event);
      void MouseEvent(wxMouseEvent& event);
      void ShipDraw(ocpnDC& dc);
      void DrawArrow(ocpnDC& dc, int x, int y, double rot_angle, double scale);
      void OnRouteLegPopupTimerEvent ( wxTimerEvent& event );
      void FindRoutePointsAtCursor( float selectRadius, bool setBeingEdited );

      void RotateTimerEvent(wxTimerEvent& event);
      void PanTimerEvent(wxTimerEvent& event);
      bool CheckEdgePan(int x, int y, bool bdragging);
      void OnCursorTrackTimerEvent(wxTimerEvent& event);
      void OnZoomTimerEvent(wxTimerEvent& event);

      void DrawAllRoutesInBBox(ocpnDC& dc, LLBBox& BltBBox, const wxRegion& clipregion);
      void DrawAllWaypointsInBBox(ocpnDC& dc, LLBBox& BltBBox, const wxRegion& clipregion, bool bDrawMarksOnly);
      double GetAnchorWatchRadiusPixels(RoutePoint *pAnchorWatchPoint);

      void DrawAllTidesInBBox(ocpnDC& dc, LLBBox& BBox, bool bRebuildSelList, bool bforce_redraw_tides,
                        bool bdraw_mono = false);
      void DrawAllCurrentsInBBox(ocpnDC& dc, LLBBox& BBox,
                           bool bRebuildSelList, bool bforce_redraw_currents, bool bdraw_mono = false);
      void DrawTCWindow(int x, int y, void *pIDX);

      void RenderAllChartOutlines(ocpnDC &dc, ViewPort& vp);
      void RenderChartOutline(ocpnDC &dc, int dbIndex, ViewPort& vp);
      void RenderRouteLegs ( ocpnDC &dc );

      wxBitmap *DrawTCCBitmap( wxDC *pbackground_dc, bool bAddNewSelpoints = true);

      void AISDraw(ocpnDC& dc);
      void AISDrawAreaNotices (ocpnDC& dc );
      void AISDrawTarget (AIS_Target_Data *td, ocpnDC& dc );


      void AlertDraw(ocpnDC& dc);                // pjotrc 2010.02.22

      void TargetFrame(ocpnDC &dc, wxPen pen, int x, int y, int radius);   // pjotrc 2010.02.01
      void AtoN_Diamond(ocpnDC &dc, wxPen pen, int x, int y, int radius, bool b_virtual);  // pjotrc 2010.02.01
      void Base_Square(ocpnDC &dc, wxPen pen, int x, int y, int radius);
      void SART_Render(ocpnDC &dc, wxPen pen, int x, int y, int radius);

      void GridDraw(ocpnDC& dc); // Display lat/lon Grid in chart display
      void ScaleBarDraw( ocpnDC& dc );

      void DrawOverlayObjects ( ocpnDC &dc, const wxRegion& ru );

      void EmbossDepthScale(ocpnDC &dc );
      emboss_data *CreateEmbossMapData(wxFont &font, int width, int height, const wxChar *str, ColorScheme cs);
      void CreateDepthUnitEmbossMaps(ColorScheme cs);
      wxBitmap CreateDimBitmap(wxBitmap &Bitmap, double factor);

      void CreateOZEmbossMapData(ColorScheme cs);
      void EmbossOverzoomIndicator ( ocpnDC &dc);

//      void CreateCM93OffsetEmbossMapData(ColorScheme cs);
//      void EmbossCM93Offset ( wxMemoryDC *pdc);

      void EmbossCanvas ( ocpnDC &dc, emboss_data *pemboss, int x, int y);

      void JaggyCircle(ocpnDC &dc, wxPen pen, int x, int y, int radius);
      void ShowObjectQueryWindow( int x, int y, float zlat, float zlon);
      void ShowMarkPropertiesDialog( RoutePoint* markPoint );
      void ShowRoutePropertiesDialog( wxString title, Route* selected );

      //    Data
      int         m_canvas_width, m_canvas_height;

      int         xr_margin;                          // chart scroll margins, control cursor, etc.
      int         xl_margin;
      int         yt_margin;
      int         yb_margin;

      MyFrame     *parent_frame;

      wxPoint     last_drag;

      wxMemoryDC  *pmemdc;

      int         warp_x, warp_y;
      bool        warp_flag;


      float       current_draw_scaler;


      wxTimer     *pPanTimer;       // This timer used for auto panning on route creation and edit
      wxTimer     *pCurTrackTimer;  // This timer used to update the status window on mouse idle
      wxTimer     *pRotDefTimer;    // This timer used to control rotaion rendering on mouse moves
      wxTimer     *pPanKeyTimer;    // This timer used to update pan key actions
      wxTimer     *m_DoubleClickTimer;

      wxTimer     m_MouseWheelTimer;
      wxTimer     m_RouteLegPopupTimer;

      int         m_mouse_wheel_oneshot;
      int         m_last_wheel_dir;

      int         m_curtrack_timer_msec;
      int         m_routeleg_popup_timer_msec;

      GSHHSChart  *pWorldBackgroundChart;

      ChartBaseBSB *pCBSB;
      wxBitmap    *pss_overlay_bmp;
      wxMask      *pss_overlay_mask;

      wxRect      ship_draw_rect;
      wxRect      ship_draw_last_rect;
      wxRect      ais_draw_rect;
      wxRect      alert_draw_rect;          // pjotrc 2010.02.22

      wxBitmap    *proute_bm;          // a bitmap and dc used to calculate route bounding box
      wxMemoryDC  m_dc_route;         // seen in mouse->edit->route


      emboss_data *m_pEM_Feet;                // maps for depth unit emboss pattern
      emboss_data *m_pEM_Meters;
      emboss_data *m_pEM_Fathoms;

      emboss_data *m_pEM_OverZoom;
//      emboss_data *m_pEM_CM93Offset;	// Flav


      double      m_pix_per_mm;     // pixels per millimeter on the screen

      double      m_true_scale_ppm;

      ownship_state_t   m_ownship_state;

      ColorScheme m_cs;
      bool        m_bMeasure_Active;
      int         m_nMeasureState;
      Route       *m_pMeasureRoute;

      wxBitmap    m_bmTideDay;
      wxBitmap    m_bmTideDusk;
      wxBitmap    m_bmTideNight;
      wxBitmap    m_bmCurrentDay;
      wxBitmap    m_bmCurrentDusk;
      wxBitmap    m_bmCurrentNight;

      RolloverWin *m_pRolloverWin;
      RolloverWin *m_pAISRolloverWin;

      wxImage     m_os_image_red_day;
      wxImage     m_os_image_red_dusk;
      wxImage     m_os_image_red_night;
      wxImage     m_os_image_grey_day;
      wxImage     m_os_image_grey_dusk;
      wxImage     m_os_image_grey_night;

      wxImage     *m_pos_image_red;
      wxImage     *m_pos_image_grey;

      wxImage     *m_pos_image_user;
      wxImage     *m_pos_image_user_grey;

      wxImage     *m_pos_image_user_day;
      wxImage     *m_pos_image_user_dusk;
      wxImage     *m_pos_image_user_night;
      wxImage     *m_pos_image_user_grey_day;
      wxImage     *m_pos_image_user_grey_dusk;
      wxImage     *m_pos_image_user_grey_night;

      wxImage     m_ship_pix_image;             //cached ship draw image for high overzoom
      int         m_cur_ship_pix;
      bool        m_cur_ship_pix_isgrey;
      ColorScheme m_ship_cs;

      Quilt       *m_pQuilt;

      ViewPort    m_cache_vp;
      wxBitmap    *m_prot_bm;
      wxPoint     m_roffset;

      bool        m_b_rot_hidef;

      SelectItem  *m_pRolloverRouteSeg;

      double      m_wheel_lat, m_wheel_lon;
      int         m_wheel_x,m_wheel_y;

      ViewPort    m_bm_cache_vp;
      wxBitmap    m_working_bm;           // Used to build quilt in OnPaint()
      wxBitmap    m_cached_chart_bm;      // A cached copy of the fully drawn quilt

      bool        m_bbrightdir;
      int         m_brightmod;

      bool        m_bzooming;
      IDX_entry   *m_pIDXCandidate;

      glChartCanvas *m_glcc;
      wxGLContext   *m_pGLcontext;

      //Smooth zoom member variables
      wxTimer     m_zoom_timer;
      bool        m_bzooming_in;
      bool        m_bzooming_out;
      int         m_zoomt;                // zoom timer constant, msec
      double      m_zoom_target_factor;
      double      m_zoom_current_factor;

      wxSound     m_anchorwatch_sound;

DECLARE_EVENT_TABLE()
};

class glTextureDescriptor;          // Defined/implemented in chcanv.cpp

WX_DECLARE_OBJARRAY(glTextureDescriptor, ArrayOfTexDescriptors);

WX_DECLARE_HASH_MAP( int, glTextureDescriptor*, wxIntegerHash, wxIntegerEqual, ChartTextureHashType );
WX_DECLARE_HASH_MAP( void*, ChartTextureHashType*, wxPointerHash, wxPointerEqual, ChartPointerHashType );

//----------------------------------------------------------------------------
// glChartCanvas
//----------------------------------------------------------------------------

class glChartCanvas : public wxGLCanvas
{
public:

      glChartCanvas(wxWindow *parent);
      ~glChartCanvas();
      
      void SetContext(wxGLContext *pcontext) { m_pcontext = pcontext; }

      void OnPaint(wxPaintEvent& event);
      void OnEraseBG(wxEraseEvent& evt);
      void render();
      void OnActivate ( wxActivateEvent& event );
      void OnSize ( wxSizeEvent& event );
      void MouseEvent(wxMouseEvent& event);

      wxString GetRendererString(){ return m_renderer; }

      void Invalidate() { m_gl_cache_vp.Invalidate(); }
      void RenderRasterChartRegionGL(ChartBase *chart, ViewPort &vp, wxRegion &region);
      bool PurgeChartTextures(ChartBase *pc);
      void ClearAllRasterTextures(void);
      void DrawGLOverLayObjects(void);

protected:
      void RenderQuiltViewGL(ViewPort &vp, wxRegion Region, bool b_clear = true);
      void BuildFBO(void);
      void SetClipRegion(ViewPort &vp, wxRegion &region, bool b_clear);
      void ComputeRenderQuiltViewGLRegion( ViewPort &vp, wxRegion Region );

      wxGLContext       *m_pcontext;
      
      int m_cacheinvalid;
      int max_texture_dimension;

      unsigned char *m_data;
      int m_datasize;

      bool m_bsetup;

      wxString m_renderer;

      void GrowData(int size);

      ArrayOfTexDescriptors         m_tex_array;

      //    This is a hash table
      //    key is ChartBaseBSB pointer
      //    Value is ChartTextureHashType*
      ChartPointerHashType          m_chart_hash;

      ViewPort    m_gl_cache_vp;


      bool m_bGenMM;
      bool m_bGL_GEN_MM;
      int  m_ntex;
      int  m_tex_max_res;
      int  m_tex_max_res_initial;
      bool m_b_mem_crunch;

      //    For FBO(s)
      bool         m_b_useFBO;
      bool         m_b_useFBOStencil;
      GLuint       m_fb0;
      GLuint       m_depth_rb;

      GLenum       m_TEX_TYPE;
      GLuint       m_cache_tex;
      GLuint       m_blit_tex;
      int          m_cache_tex_x;
      int          m_cache_tex_y;
      wxRegion     m_gl_rendered_region;

DECLARE_EVENT_TABLE()
};


//----------------------------------------------------------------------------
//    Constants
//----------------------------------------------------------------------------
enum
{
      ID_TCWIN_NX,
      ID_TCWIN_PR
};

enum
{
      TIDE_PLOT,
      CURRENT_PLOT
};

//----------------------------------------------------------------------------
// TCWin
//----------------------------------------------------------------------------
WX_DECLARE_LIST(wxPoint, SplineList);           // for spline curve points

class TCWin: public wxDialog
{
public:
      TCWin(ChartCanvas *parent, int x, int y, void *pvIDX);
      ~TCWin();

      void OnSize(wxSizeEvent& event);
      void OnPaint(wxPaintEvent& event);
      void MouseEvent(wxMouseEvent& event);
	  void OnTCWinPopupTimerEvent(wxTimerEvent& event);
      void OKEvent(wxCommandEvent& event);
      void NXEvent(wxCommandEvent& event);
      void PREvent(wxCommandEvent& event);
      void OnCloseWindow(wxCloseEvent& event);

      void Resize(void);

      void RePosition(void);


private:
    wxTextCtrl  *m_ptextctrl;
    wxTimer	  m_TCWinPopupTimer;
    RolloverWin *m_pTCRolloverWin;
    int           curs_x;
    int           curs_y;
    int          m_plot_type;

      IDX_entry   *pIDX;
      wxButton    *OK_button;
      wxButton    *NX_button;
      wxButton    *PR_button;

      int         im;
      int         ib;
      int         it;
      int         val_off;
      wxRect    m_graph_rect;


      float       tcv[26];
	  wxListBox  *m_tList ;
      bool        btc_valid;
      ChartCanvas *pParent;
      int         m_corr_mins;
      wxString    m_stz;
      int         m_t_graphday_00_at_station;
      wxDateTime  m_graphday;
      int         m_plot_y_offset;

      SplineList  m_sList;

      wxFont *pSFont;
      wxFont *pSMFont;
      wxFont *pMFont;
      wxFont *pLFont;

      wxPen *pblack_1;
      wxPen *pblack_2;
      wxPen *pblack_3;
      wxPen *pred_2;
      wxBrush *pltgray;
      wxBrush *pltgray2;


DECLARE_EVENT_TABLE()
};


//----------------------------------------------------------------------------------------------------------
//    ocpCursor Specification
//----------------------------------------------------------------------------------------------------------

class wxCursorRefData;

class ocpCursor : public wxCursor
{
      public:

            ocpCursor(const wxString& cursorName, long type, int hotSpotX=0, int hotSpotY=0);
            ocpCursor(const char **xpm_data, long type, int hotSpotX=0, int hotSpotY=0);
};


//----------------------------------------------------------------------------------------------------------
//    AISTargetQueryDialog Specification
//----------------------------------------------------------------------------------------------------------
class AISTargetQueryDialog: public wxDialog
{
DECLARE_CLASS( AISTargetQueryDialog )
DECLARE_EVENT_TABLE()
public:

      /// Constructors

      AISTargetQueryDialog( );
      AISTargetQueryDialog( wxWindow* parent,
            wxWindowID id = wxID_ANY,
            const wxString& caption = _("Object Query"),
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

      ~AISTargetQueryDialog( );
      /// Initialise our variables
      void Init();

      /// Creation
      bool Create( wxWindow* parent,
            wxWindowID id = wxID_ANY,
            const wxString& caption = _("Object Query"),
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

      void OnClose(wxCloseEvent& event);
      void OnIdOKClick( wxCommandEvent& event );
      void OnMove( wxMoveEvent& event );

      void CreateControls();

      void SetText(wxString &text_string);
      void SetColorScheme(ColorScheme cs);

      void UpdateText(void);
      void SetMMSI(int mmsi){ m_MMSI = mmsi; }
      int  GetMMSI(void){ return m_MMSI; }

      //    Data
      int               m_MMSI;
      wxHtmlWindow     *m_pQueryTextCtl;
      ColorScheme       m_colorscheme;
      wxBoxSizer        *m_pboxSizer;
      int               m_nl;
      wxButton          *m_okButton;
};


//----------------------------------------------------------------------------
// Generic Rollover Window
//----------------------------------------------------------------------------
class RolloverWin: public wxWindow {
public:
    RolloverWin( wxWindow *parent, int timeout = -1 );
    ~RolloverWin();

    void OnPaint( wxPaintEvent& event );

    void SetColorScheme( ColorScheme cs );
    void SetString( wxString &s ) { m_string = s; }
    void SetPosition( wxPoint pt ) { m_position = pt; }
    void SetBitmap( int rollover );
    wxBitmap* GetBitmap() { return m_pbm; }
    void SetBestPosition( int x, int y, int off_x, int off_y, int rollover, wxSize parent_size );
    void OnTimer( wxTimerEvent& event );
    void OnMouseEvent( wxMouseEvent& event );
    void SetMousePropogation( int level ) { m_mmouse_propogate = level; }
    bool IsActive() { return isActive; }
    void IsActive( bool state ) { isActive = state; }

private:
    wxString m_string;
    wxSize m_size;
    wxPoint m_position;
    wxBitmap *m_pbm;
    wxTimer m_timer_timeout;
    int m_timeout_sec;
    int m_mmouse_propogate;
    bool isActive;

DECLARE_EVENT_TABLE()
};

//------------------------------------------------------------------------------
//    CM93 Detail Slider Specification
//------------------------------------------------------------------------------

class CM93DSlide: public wxDialog {
public:
    CM93DSlide( wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue,
            const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
            long style = 0, const wxString& title = _T("") );

    ~CM93DSlide( void );

    void Init( void );
    bool Create( wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue,
            const wxPoint& pos, const wxSize& size, long style, const wxString& title );

    void OnCancelClick( wxCommandEvent& event );
    void OnMove( wxMoveEvent& event );
    void OnChangeValue( wxScrollEvent& event );
    void OnClose( wxCloseEvent& event );

    wxSlider *m_pCM93DetailSlider;
    wxWindow *m_pparent;

DECLARE_EVENT_TABLE()
};

//-------------------------------------------------------------------------------
//
//    Go To Position Dialog Implementation
//
//-------------------------------------------------------------------------------

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_GOTOPOS 8100
#define SYMBOL_GOTOPOS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_GOTOPOS_TITLE _("Center View")
#define SYMBOL_GOTOPOS_IDNAME ID_GOTOPOS
#define SYMBOL_GOTOPOS_SIZE wxSize(200, 300)
#define SYMBOL_GOTOPOS_POSITION wxDefaultPosition
#define ID_GOTOPOS_CANCEL 8101
#define ID_GOTOPOS_OK 8102


////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * GoToPositionDialog class declaration
 */

class GoToPositionDialog: public wxDialog
{
      DECLARE_DYNAMIC_CLASS( GoToPositionDialog )
      DECLARE_EVENT_TABLE()

      public:
    /// Constructors
            GoToPositionDialog( );
            GoToPositionDialog( wxWindow* parent, wxWindowID id = SYMBOL_GOTOPOS_IDNAME,
                                const wxString& caption = SYMBOL_GOTOPOS_TITLE,
                                const wxPoint& pos = SYMBOL_GOTOPOS_POSITION,
                                const wxSize& size = SYMBOL_GOTOPOS_SIZE,
                                long style = SYMBOL_GOTOPOS_STYLE );

            ~GoToPositionDialog();

    /// Creation
            bool Create( wxWindow* parent, wxWindowID id = SYMBOL_GOTOPOS_IDNAME,
                         const wxString& caption = SYMBOL_GOTOPOS_TITLE,
                         const wxPoint& pos = SYMBOL_GOTOPOS_POSITION,
                         const wxSize& size = SYMBOL_GOTOPOS_SIZE, long style = SYMBOL_GOTOPOS_STYLE );

            void SetColorScheme(ColorScheme cs);

            void CreateControls();

            void OnGoToPosCancelClick( wxCommandEvent& event );
            void OnGoToPosOkClick( wxCommandEvent& event );
            void OnPositionCtlUpdated( wxCommandEvent& event );
            void CheckPasteBufferForPosition();

      /// Should we show tooltips?
            static bool ShowToolTips();

            wxTextCtrl*   m_MarkLatCtl;
            wxTextCtrl*   m_MarkLonCtl;
            wxButton*     m_CancelButton;
            wxButton*     m_OKButton;

            double        m_lat_save;
            double        m_lon_save;
};


//----------------------------------------------------------------------------------------------------------
//    s57QueryDialog Specification
//----------------------------------------------------------------------------------------------------------
class wxHtmlWindow;

class S57QueryDialog: public wxDialog
{
      DECLARE_CLASS( S57QueryDialog )
                  DECLARE_EVENT_TABLE()
      public:

      /// Constructors

            S57QueryDialog( );
            S57QueryDialog( wxWindow* parent,
                            wxWindowID id = wxID_ANY,
                            const wxString& caption = _("Object Query"),
                                        const wxPoint& pos = wxDefaultPosition,
                                        const wxSize& size = wxDefaultSize,
                                        long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

            ~S57QueryDialog( );
            void Init();

            bool Create( wxWindow* parent,
                         wxWindowID id = wxID_ANY,
                         const wxString& caption = _("Object Query"),
                                     const wxPoint& pos = wxDefaultPosition,
                                     const wxSize& size = wxDefaultSize,
                                     long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

            void SetColorScheme(void);

            void CreateControls();
            void OnSize(wxSizeEvent& event);
            void OnClose(wxCloseEvent& event);

      //    Overrides
            void OnPaint ( wxPaintEvent& event );

            void SetHTMLPage(wxString& page);

      //    Data
            wxHtmlWindow      *m_phtml;

};




#endif
