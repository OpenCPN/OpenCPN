
/***************************************************************************
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
 **************************************************************************/

#ifndef __CHCANV_H__
#define __CHCANV_H__

#include "bbox.h"

#include <wx/datetime.h>
#include <wx/treectrl.h>
#include "wx/dirctrl.h"
#include <wx/sound.h>
#include <wx/grid.h>
#include <wx/wxhtml.h>

#include "chart1.h"                 // for enum types
#include "ocpndc.h"
#include "undo.h"

#include "ocpCursor.h"
#include "GoToPositionDialog.h"
#include "CM93DSlide.h"
#include "RolloverWin.h"
#include "timers.h"
#include "emboss_data.h"
#include "S57Sector.h"

class wxGLContext;
class GSHHSChart;
class IDX_entry;

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
      class S57ObjectTree;
      class S57ObjectDesc;
      class RolloverWin;
      class Quilt;
      class PixelCache;
      class ChInfoWin;
      class glChartCanvas;
      class CanvasMenuHandler;

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
      void OnKeyChar(wxKeyEvent &event);
      void OnPaint(wxPaintEvent& event);
      void PaintCleanup();
      void Scroll(int dx, int dy);

      bool MouseEventOverlayWindows( wxMouseEvent& event );
      bool MouseEventChartBar( wxMouseEvent& event );
      bool MouseEventSetup( wxMouseEvent& event, bool b_handle_dclick = true );
      bool MouseEventProcessObjects( wxMouseEvent& event );
      bool MouseEventProcessCanvas( wxMouseEvent& event );
      void SetCanvasCursor( wxMouseEvent& event );
      
      void PopupMenuHandler(wxCommandEvent& event);

      bool SetUserOwnship();
          
      void EnablePaint(bool b_enable);
      virtual bool SetCursor(const wxCursor &c);
      virtual void Refresh( bool eraseBackground = true,
                            const wxRect *rect = (const wxRect *) NULL );
      virtual void Update();

      void LostMouseCapture(wxMouseCaptureLostEvent& event);
      
      void CancelMouseRoute();
      void SetDisplaySizeMM( double size );
      
      bool SetVPScale(double sc, bool b_refresh = true);
      bool SetVPProjection(int projection);
      bool SetViewPoint ( double lat, double lon);
      bool SetViewPointByCorners( double latSW, double lonSW, double latNE, double lonNE );
      bool SetViewPoint(double lat, double lon, double scale_ppm, double skew, double rotation,
                        int projection = 0, bool b_adjust = true, bool b_refresh = true);
      void ReloadVP ( bool b_adjust = true );
      void LoadVP ( ViewPort &vp, bool b_adjust = true );

      void SetVPRotation(double angle){ VPoint.rotation = angle; }
      double GetVPRotation(void) { return GetVP().rotation; }
      double GetVPSkew(void) { return GetVP().skew; }
      double GetVPTilt(void) { return GetVP().tilt; }
      void ClearbFollow(void);

      void GetDoubleCanvasPointPix(double rlat, double rlon, wxPoint2DDouble *r);
      void GetDoubleCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, wxPoint2DDouble *r );
      bool GetCanvasPointPix( double rlat, double rlon, wxPoint *r );
      bool GetCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, wxPoint *r );
      
      void GetCanvasPixPoint(double x, double y, double &lat, double &lon);
      void WarpPointerDeferred(int x, int y);
      void UpdateShips();
      void UpdateAIS();
      void UpdateAlerts();                          // pjotrc 2010.02.22

      wxBitmap &GetTideBitmap(){ return m_cTideBitmap; }
      
      void UnlockQuilt();
      void SetQuiltMode(bool b_quilt);
      bool GetQuiltMode(void);
      ArrayOfInts GetQuiltIndexArray(void);
      bool IsQuiltDelta(void);
      void SetQuiltChartHiLiteIndex(int dbIndex);
      int GetQuiltReferenceChartIndex(void);
      double GetBestStartScale(int dbi_hint, const ViewPort &vp);
      
      int GetNextContextMenuId();

      TCWin *getTCWin(){ return pCwin; }
      
      bool StartTimedMovement( bool stoptimer=true );
      void DoTimedMovement( );
      void DoMovement( long dt );
      void StopMovement( );

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
      void SetVP(ViewPort &);
      ChartBase* GetChartAtCursor();
      ChartBase* GetOverlayChartAtCursor();

      bool isRouteEditing( void ){ return m_bRouteEditing && m_pRoutePointEditTarget; }
      bool isMarkEditing( void ){ return m_bMarkEditing && m_pRoutePointEditTarget; }
      
      GSHHSChart* GetWorldBackgroundChart() { return pWorldBackgroundChart; }

      void  SetbTCUpdate(bool f){ m_bTCupdate = f;}
      bool  GetbTCUpdate(){ return m_bTCupdate;}
      void  SetbShowCurrent(bool f){ m_bShowCurrent = f;}
      bool  GetbShowCurrent(){ return m_bShowCurrent;}
      void  SetbShowTide(bool f){ m_bShowTide = f;}
      bool  GetbShowTide(){ return m_bShowTide;}
      double GetPixPerMM(){ return m_pix_per_mm;}

      void SetOwnShipState(ownship_state_t state){ m_ownship_state = state;}
      void SetCursorStatus( double cursor_lat, double cursor_lon );
      void GetCursorLatLon(double *lat, double *lon);

      bool PanCanvas(double dx, double dy);
      void StopAutoPan(void);

      void ZoomCanvas(double factor, bool can_zoom_to_cursor=true, bool stoptimer=true );
      void DoZoomCanvas(double factor,  bool can_zoom_to_cursor = true);

      void RotateCanvas( double dir );
      void DoRotateCanvas( double rotation );
      void DoTiltCanvas( double tilt );

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
      int AdjustQuiltRefChart( void );
 
      void ShowObjectQueryWindow( int x, int y, float zlat, float zlon);
      void ShowMarkPropertiesDialog( RoutePoint* markPoint );
      void ShowRoutePropertiesDialog(wxString title, Route* selected);
      void ShowTrackPropertiesDialog( Track* selected );
      void DrawTCWindow(int x, int y, void *pIDX);
      
      
      wxColour GetFogColor(){ return m_fog_color; }      
      
      void ShowChartInfoWindow(int x, int dbIndex);
      void HideChartInfoWindow(void);
    
      void StartMeasureRoute();
      void CancelMeasureRoute();
      void DropMarker(bool atOwnShip = true);

      //Todo build more accessors
      bool        m_bFollow;
      wxCursor    *pCursorPencil;
      wxCursor    *pCursorArrow;
      wxCursor    *pCursorCross;
      wxCursor    *pPlugIn_Cursor;
      TCWin       *pCwin;
      wxBitmap    *pscratch_bm;
      bool        m_brepaint_piano;
      double      m_cursor_lon, m_cursor_lat;
      Undo        *undo;
      wxPoint     r_rband;
      double      m_prev_rlat;
      double      m_prev_rlon;
      RoutePoint  *m_prev_pMousePoint;
      Quilt       *m_pQuilt;
      
      void RemovePointFromRoute( RoutePoint* point, Route* route );

      void DrawBlinkObjects( void );

      void StartRoute(void);
      void FinishRoute(void);
      
      void InvalidateGL();
      
#ifdef ocpnUSE_GL
      glChartCanvas *GetglCanvas(){ return m_glcc; }
#endif      

      void JaggyCircle(ocpnDC &dc, wxPen pen, int x, int y, int radius);
      
      bool CheckEdgePan( int x, int y, bool bdragging, int margin, int delta );

      Route       *m_pMouseRoute;
      bool        m_bMeasure_Active;
      wxString    m_active_upload_port;
      bool        m_bAppendingRoute;
      int         m_nMeasureState;
      Route       *m_pMeasureRoute;
      MyFrame     *parent_frame;
      wxString    FindValidUploadPort();
      CanvasMenuHandler  *m_canvasMenu;
      
private:
      bool IsTempMenuBarEnabled();
      bool InvokeCanvasMenu(int x, int y, int seltype);
    
      ViewPort    VPoint;
      void        PositionConsole(void);
      
      wxColour PredColor();
      wxColour ShipColor();

      void ComputeShipScaleFactor(float icon_hdt,
                                  int ownShipWidth, int ownShipLength, 
                                  wxPoint &lShipMidPoint,
                                  wxPoint &GpsOffsetPixels, wxPoint lGPSPoint,
                                  float &scale_factor_x, float &scale_factor_y);

      void ShipDrawLargeScale( ocpnDC& dc, wxPoint lShipMidPoint );
      void ShipIndicatorsDraw( ocpnDC& dc, float lpp,
                               wxPoint GPSOffsetPixels,
                               wxPoint lGPSPoint, wxPoint lHeadPoint,
                                      float img_height, float cog_rad,
                               wxPoint lPredPoint, bool b_render_hdt,
          wxPoint lShipMidPoint);
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
	  bool		  m_bRoutePoinDragging;
      bool        m_bIsInRadius;
      bool        m_bMayToggleMenuBar;

      RoutePoint  *m_pRoutePointEditTarget;
      RoutePoint  *m_lastRoutePointEditTarget;
      SelectItem  *m_pFoundPoint;
      bool        m_bChartDragging;
      Route       *m_pSelectedRoute;
      Track       *m_pSelectedTrack;
      wxArrayPtrVoid *m_pEditRouteArray;
      RoutePoint  *m_pFoundRoutePoint;

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

      wxBitmap    *pThumbDIBShow;
      wxBitmap    *pThumbShowing;

      bool        bShowingCurrent;
      bool        bShowingTide;

      double       m_canvas_scale_factor;    // converter....
                                             // useage....
                                             // true_chart_scale_on_display = m_canvas_scale_factor / pixels_per_meter of displayed chart
                                             // also may be considered as the "pixels-per-meter" of the canvas on-screen
      double      m_pix_per_mm;     // pixels per millimeter on the screen
      double      m_display_size_mm;
      
      double      m_absolute_min_scale_ppm;

      bool singleClickEventIsValid;
      wxMouseEvent singleClickEvent;

      std::vector<s57Sector_t> extendedSectorLegs;
      wxFont m_overzoomFont;
      int m_overzoomTextWidth;
      int m_overzoomTextHeight;

      //    Methods
      void OnActivate(wxActivateEvent& event);
      void OnSize(wxSizeEvent& event);
      void MouseTimedEvent(wxTimerEvent& event);
      void MouseEvent(wxMouseEvent& event);
      void ShipDraw(ocpnDC& dc);
      void DrawArrow(ocpnDC& dc, int x, int y, double rot_angle, double scale);
      void OnRolloverPopupTimerEvent ( wxTimerEvent& event );
      void FindRoutePointsAtCursor( float selectRadius, bool setBeingEdited );

      void RotateTimerEvent(wxTimerEvent& event);
      void PanTimerEvent(wxTimerEvent& event);
      void MovementTimerEvent(wxTimerEvent& );
      void MovementStopTimerEvent( wxTimerEvent& );
      void OnCursorTrackTimerEvent(wxTimerEvent& event);

      void DrawAllTracksInBBox( ocpnDC& dc, LLBBox& BltBBox );
      void DrawAllRoutesInBBox(ocpnDC& dc, LLBBox& BltBBox );
      void DrawAllWaypointsInBBox(ocpnDC& dc, LLBBox& BltBBox );
      void DrawAnchorWatchPoints( ocpnDC& dc );
      double GetAnchorWatchRadiusPixels(RoutePoint *pAnchorWatchPoint);

      void DrawAllTidesInBBox(ocpnDC& dc, LLBBox& BBox);
      void DrawAllCurrentsInBBox(ocpnDC& dc, LLBBox& BBox);
      void RebuildTideSelectList( LLBBox& BBox );
      void RebuildCurrentSelectList( LLBBox& BBox );
      

      void RenderAllChartOutlines(ocpnDC &dc, ViewPort& vp);
      void RenderChartOutline(ocpnDC &dc, int dbIndex, ViewPort& vp);
      void RenderRouteLegs ( ocpnDC &dc );

      void AlertDraw(ocpnDC& dc);                // pjotrc 2010.02.22

      void GridDraw(ocpnDC& dc); // Display lat/lon Grid in chart display
      void ScaleBarDraw( ocpnDC& dc );

      void DrawOverlayObjects ( ocpnDC &dc, const wxRegion& ru );

      emboss_data *EmbossDepthScale();
      emboss_data *CreateEmbossMapData(wxFont &font, int width, int height, const wxString &str, ColorScheme cs);
      void CreateDepthUnitEmbossMaps(ColorScheme cs);
      wxBitmap CreateDimBitmap(wxBitmap &Bitmap, double factor);

      void CreateOZEmbossMapData(ColorScheme cs);
      emboss_data *EmbossOverzoomIndicator ( ocpnDC &dc);
      void SetOverzoomFont();

//      void CreateCM93OffsetEmbossMapData(ColorScheme cs);
//      void EmbossCM93Offset ( wxMemoryDC *pdc);

      void DrawEmboss ( ocpnDC &dc, emboss_data *pemboss );

 
      void ShowBrightnessLevelTimedPopup( int brightness, int min, int max );
      
      //    Data
      int         m_canvas_width, m_canvas_height;

      int         xr_margin;                          // chart scroll margins, control cursor, etc.
      int         xl_margin;
      int         yt_margin;
      int         yb_margin;


      wxPoint     last_drag;

      wxMemoryDC  *pmemdc;

      int         warp_x, warp_y;
      bool        warp_flag;


      float       current_draw_scaler;


      wxTimer     *pPanTimer;       // This timer used for auto panning on route creation and edit
      wxTimer     *pMovementTimer;       // This timer used for smooth movement in non-opengl mode
      wxTimer     *pMovementStopTimer; // This timer used to stop movement if a keyup event is lost
      wxTimer     *pCurTrackTimer;  // This timer used to update the status window on mouse idle
      wxTimer     *pRotDefTimer;    // This timer used to control rotaion rendering on mouse moves
      wxTimer     *m_DoubleClickTimer;

      wxTimer     m_RolloverPopupTimer;

      int         m_wheelzoom_stop_oneshot;
      int         m_last_wheel_dir;
      wxStopWatch m_wheelstopwatch;
      double      m_zoom_target;
      
      int         m_curtrack_timer_msec;
      int         m_rollover_popup_timer_msec;

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



      double      m_true_scale_ppm;

      ownship_state_t   m_ownship_state;

      ColorScheme m_cs;

      wxBitmap    m_bmTideDay;
      wxBitmap    m_bmTideDusk;
      wxBitmap    m_bmTideNight;
      wxBitmap    m_bmCurrentDay;
      wxBitmap    m_bmCurrentDusk;
      wxBitmap    m_bmCurrentNight;
      wxBitmap    m_cTideBitmap;
      wxBitmap    m_cCurrentBitmap;
      
      RolloverWin *m_pRouteRolloverWin;
      RolloverWin *m_pAISRolloverWin;
      
      TimedPopupWin *m_pBrightPopup;
      
      wxImage     m_os_image_red_day;
      wxImage     m_os_image_red_dusk;
      wxImage     m_os_image_red_night;
      wxImage     m_os_image_grey_day;
      wxImage     m_os_image_grey_dusk;
      wxImage     m_os_image_grey_night;
      wxImage     m_os_image_yellow_day;
      wxImage     m_os_image_yellow_dusk;
      wxImage     m_os_image_yellow_night;
      
      wxImage     *m_pos_image_red;
      wxImage     *m_pos_image_grey;
      wxImage     *m_pos_image_yellow;
      
      wxImage     *m_pos_image_user;
      wxImage     *m_pos_image_user_grey;
      wxImage     *m_pos_image_user_yellow;
      
      wxImage     *m_pos_image_user_day;
      wxImage     *m_pos_image_user_dusk;
      wxImage     *m_pos_image_user_night;
      wxImage     *m_pos_image_user_grey_day;
      wxImage     *m_pos_image_user_grey_dusk;
      wxImage     *m_pos_image_user_grey_night;
      wxImage     *m_pos_image_user_yellow_day;
      wxImage     *m_pos_image_user_yellow_dusk;
      wxImage     *m_pos_image_user_yellow_night;
      
      wxImage     m_ship_pix_image;             //cached ship draw image for high overzoom
      int         m_cur_ship_pix;
      bool        m_cur_ship_pix_isgrey;
      ColorScheme m_ship_cs;


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

      bool        m_bzooming, m_bzooming_to_cursor;
      IDX_entry   *m_pIDXCandidate;

//#ifdef ocpnUSE_GL
      glChartCanvas *m_glcc;
      wxGLContext   *m_pGLcontext;
//#endif

      //Smooth movement member variables
      int         m_panx, m_pany, m_modkeys;
      double      m_panspeed;
      bool        m_bmouse_key_mod;
      double      m_zoom_factor, m_rotation_speed;
      int         m_mustmove;


      wxDateTime m_last_movement_time;

      bool        m_b_paint_enable;
      
      int         m_AISRollover_MMSI;
      bool        m_bsectors_shown;
      bool        m_bedge_pan;
      double      m_displayed_scale_factor;
      
      wxColour    m_fog_color;      
      bool        m_disable_edge_pan;
      wxFont      *m_pgridFont;
      
      
DECLARE_EVENT_TABLE()
};

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif


#endif
