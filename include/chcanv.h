/******************************************************************************
 * $Id: chcanv.h,v 1.52 2010/06/21 01:54:16 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Chart Canvas
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
 *
 */




#ifndef __CHCANV_H__
#define __CHCANV_H__

#include "bbox.h"

#include <wx/datetime.h>
#include <wx/treectrl.h>

#include "chart1.h"                 // for enum types


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

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#define     RESCALE_TIMER     1
#define     PAN_TIMER         2
#define     CURTRACK_TIMER    3
#define     ROT_TIMER         4
#define     RTELEGPU_TIMER    5




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
            emboss_data(){ pmap = NULL; }
            ~emboss_data(){ free(pmap); }

            int         *pmap;
            int         width;
            int         height;
      private:
};

//----------------------------------------------------------------------------
// ChartCanvas
//----------------------------------------------------------------------------
class ChartCanvas: public wxWindow
{
public:
      ChartCanvas(wxFrame *frame);
      ~ChartCanvas();

      //    Methods
      void OnChar(wxKeyEvent &event);
      void OnPaint(wxPaintEvent& event);
      void Scroll(int dx, int dy);
      void CanvasPopupMenu(int x, int y, int seltype);
      void DoCanvasPopupMenu ( int x, int y, wxMenu *pMenu );

      void PopupMenuHandler(wxCommandEvent& event);
      void SetMyCursor(wxCursor *c);
      void LostMouseCapture(wxMouseCaptureLostEvent& event);

      void CancelMouseRoute();

      bool Do_Hotkeys(wxKeyEvent &event);

      bool SetViewPoint(double lat, double lon, double scale_ppm, double skew, double rotation);
      void SetVPScale(double sc);
      bool SetViewPoint ( double lat, double lon);
      void ReloadVP ( void );
      void SetVPRotation(double angle){ VPoint.rotation = angle; }
      double GetVPRotation(void) { return VPoint.rotation; }
      double GetVPSkew(void) { return VPoint.skew; }
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

      int GetNextContextMenuId();

      void SetColorScheme(ColorScheme cs);

      //    Accessors
      int GetCanvasWidth(){ return m_canvas_width;}
      int GetCanvasHeight(){ return m_canvas_height;}
      float GetVPScale(){return VPoint.view_scale_ppm;}
      float GetVPChartScale(){return VPoint.chart_scale;}
      double GetCanvasScaleFactor(){return m_canvas_scale_factor;}


      void  SetbTCUpdate(bool f){ m_bTCupdate = f;}
      bool  GetbTCUpdate(){ return m_bTCupdate;}
      void  SetbShowCurrent(bool f){ m_bShowCurrent = f;}
      bool  GetbShowCurrent(){ return m_bShowCurrent;}
      void  SetbShowTide(bool f){ m_bShowTide = f;}
      bool  GetbShowTide(){ return m_bShowTide;}
      double GetPixPerMM(){ return m_pix_per_mm;}

      void SetOwnShipState(ownship_state_t state){ m_ownship_state = state;}
      void GetCursorLatLon(double *lat, double *lon);

      bool ZoomCanvasIn(double lat = 0., double lon = 0.);
      bool ZoomCanvasOut(double lat = 0., double lon = 0.);
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

      void ShowChartInfoWindow(int x, int y, int dbIndex);
      void HideChartInfoWindow(void);

      //Todo build more accessors
      bool        m_bFollow;
      wxCursor    *pCursorPencil;
      TCWin       *pCwin;
      ViewPort    VPoint;
      wxBitmap    *pscratch_bm;
      double      m_cursor_lon, m_cursor_lat;


private:
      void        PositionConsole(void);

      ChInfoWin   *m_pCIWin;

      bool        m_bShowCurrent;
      bool        m_bShowTide;
      int         cursor_region;
      bool        m_bTCupdate;

      wxRect      bbRect;

      wxPoint     r_rband;
      wxPoint     LastShipPoint;
      wxPoint     LastPredPoint;
      bool        m_bDrawingRoute;
      bool        m_bRouteEditing;
      bool        m_bMarkEditing;
      RoutePoint  *m_pRoutePointEditTarget;
      SelectItem  *m_pFoundPoint;
      bool        m_bChartDragging;


      Route       *m_pMouseRoute;
      double      m_prev_rlat;
      double      m_prev_rlon;
      RoutePoint  *m_prev_pMousePoint;
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
      wxCursor    *pCursorArrow;

      wxCursor    *pPriorCursor;

      int         popx, popy;
      bool        m_bAppendingRoute;

      wxBitmap    *pThumbDIBShow;
      wxBitmap    *pThumbShowing;

      bool        bShowingCurrent;
      bool        bShowingTide;

      double       m_canvas_scale_factor;    // converter....
                                             // useage....
                                             // true_chart_scale_on_display   = m_canvas_scale_factor / pixels_per_meter of displayed chart
                                             // also may be considered as the "pixels-per-meter" of the canvas on-screen

      //    Methods
      void OnActivate(wxActivateEvent& event);
      void OnSize(wxSizeEvent& event);
      void MouseEvent(wxMouseEvent& event);
      void ShipDraw(wxDC& dc);
      void DrawArrow(wxDC& dc, int x, int y, float rot_angle, float scale);
      void OnRouteLegPopupTimerEvent ( wxTimerEvent& event );

      void RotateTimerEvent(wxTimerEvent& event);
      void PanTimerEvent(wxTimerEvent& event);
      bool CheckEdgePan(int x, int y);
      void OnCursorTrackTimerEvent(wxTimerEvent& event);

      void DrawAllRoutesInBBox(wxDC& dc, LLBBox& BltBBox, const wxRegion& clipregion);
      void DrawAllWaypointsInBBox(wxDC& dc, LLBBox& BltBBox, const wxRegion& clipregion, bool bDrawMarksOnly);
      double GetAnchorWatchRadiusPixels(RoutePoint *pAnchorWatchPoint);

      void DrawAllTidesInBBox(wxDC& dc, LLBBox& BBox, bool bRebuildSelList,
                        bool bdraw_mono = false);
      void DrawAllCurrentsInBBox(wxDC& dc, LLBBox& BBox, double skew_angle,
                           bool bRebuildSelList, bool bforce_redraw_currents, bool bdraw_mono = false);
      void DrawTCWindow(int x, int y, void *pIDX);
      void RenderChartOutline(wxDC *pdc, int dbIndex, ViewPort& vp, bool bdraw_mono = false);
      void RenderAllChartOutlines(wxDC *pdc, ViewPort& vp, bool bdraw_mono = false);
      wxBitmap *DrawTCCBitmap( wxDC *pbackground_dc, bool bAddNewSelpoints = true);

      void AISDraw(wxDC& dc);
      void AISDrawTarget (AIS_Target_Data *td, wxDC& dc );

      void AlertDraw(wxDC& dc);                // pjotrc 2010.02.22

      void TargetFrame(wxDC &dc, wxPen pen, int x, int y, int radius);   // pjotrc 2010.02.01
      void AtoN_Diamond(wxDC &dc, wxPen pen, int x, int y, int radius);  // pjotrc 2010.02.01
      void Base_Square(wxDC &dc, wxPen pen, int x, int y, int radius);

      void GridDraw(wxDC& dc); // Display lat/lon Grid in chart display
      void ScaleBarDraw( wxDC& dc, int x_origin, int y_origin );

      void EmbossDepthScale(wxMemoryDC *psource_dc, wxMemoryDC *pdest_dc, int emboss_ident);
      emboss_data *CreateEmbossMapData(wxFont &font, int width, int height, const wxChar *str, ColorScheme cs);
      void CreateDepthUnitEmbossMaps(ColorScheme cs);
      wxBitmap CreateDimBitmap(wxBitmap &Bitmap, double factor);

      void CreateOZEmbossMapData(ColorScheme cs);
      void EmbossOverzoomIndicator ( wxMemoryDC *temp_dc, wxMemoryDC *scratch_dc);

      void CreateCM93OffsetEmbossMapData(ColorScheme cs);
      void EmbossCM93Offset ( wxMemoryDC *temp_dc, wxMemoryDC *scratch_dc);

      void EmbossCanvas ( wxMemoryDC *psource_dc, wxMemoryDC *pdest_dc, emboss_data *pemboss, int x, int y);

      void JaggyCircle(wxDC &dc, wxPen pen, int x, int y, int radius);


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

      wxTimer     m_MouseWheelTimer;
      wxTimer     m_RouteLegPopupTimer;

      int         m_mouse_wheel_oneshot;
      int         m_last_wheel_dir;

      int         m_curtrack_timer_msec;
      int         m_routeleg_popup_timer_msec;

      WVSChart    *pwvs_chart;

      ChartBaseBSB *pCBSB;
      wxBitmap    *pss_overlay_bmp;
      wxMask      *pss_overlay_mask;

      wxRect      ship_draw_rect;
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

      wxImage     m_os_image_green_day;
      wxImage     m_os_image_green_dusk;
      wxImage     m_os_image_green_night;
      wxImage     m_os_image_red_day;
      wxImage     m_os_image_red_dusk;
      wxImage     m_os_image_red_night;
      wxImage     m_os_image_grey_day;
      wxImage     m_os_image_grey_dusk;
      wxImage     m_os_image_grey_night;

      wxImage     *m_pos_image_green;
      wxImage     *m_pos_image_red;
      wxImage     *m_pos_image_grey;

      wxImage     *m_pos_image_user;

      wxImage     m_ship_pix_image;             //cached ship draw image for high overzoom
      int         m_cur_ship_pix;
      bool        m_cur_ship_pix_isgrey;

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
      void OKEvent(wxCommandEvent& event);
      void NXEvent(wxCommandEvent& event);
      void PREvent(wxCommandEvent& event);
      void OnCloseWindow(wxCloseEvent& event);

      void Resize(void);

      void RePosition(void);


private:
      int         m_plot_type;

      IDX_entry   *pIDX;
      wxButton    *OK_button;
      wxButton    *NX_button;
      wxButton    *PR_button;

      int         im;
      int         ib;
      int         it;
      int         val_off;


      float       tcv[24];
      bool        btc_valid;
      ChartCanvas *pParent;
      int         m_corr_mins;
      wxString    m_stz;
      int         m_t_graphday_00_at_station;
      wxDateTime  m_graphday;
      int         m_plot_y_offset;

      SplineList  m_sList;

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
//    s57QueryDialog Specification
//----------------------------------------------------------------------------------------------------------
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
      /// Initialise our variables
      void Init();

      /// Creation
      bool Create( wxWindow* parent,
            wxWindowID id = wxID_ANY,
            const wxString& caption = _("Object Query"),
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

      void SetColorScheme(ColorScheme cs);

      /// Creates the controls and sizers
      void CreateControls();

      void SetText(wxString &text_string);
      void SetObjectTree(void **pOD, int n_items);

      void SetSelectedItem(wxTreeItemId item_id);                  // a "notification" from Tree control


      wxString format_attributes(wxString &attr, int lcol, int rcol);

      //    Overrides
      void OnPaint ( wxPaintEvent& event );


      //    Data
      wxString          QueryResult;
      wxTextCtrl        *m_pQueryTextCtl;
      S57ObjectTree     *m_pTree;
      wxTreeItemId      m_root_id;

      int               m_n_items;
      void              **m_ppOD;

      wxTreeItemId      *m_id_array;                              // an array of wxTreeItemIDs
      int               m_char_width;

};


class AISInfoWin;

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
      AISInfoWin        *m_pQueryTextCtl;
      ColorScheme       m_colorscheme;
      wxBoxSizer        *m_pboxSizer;
      int               m_nl;
      wxButton          *m_okButton;
};


//----------------------------------------------------------------------------------------------------------
//    S57 Object Query Tree Control Specification
//----------------------------------------------------------------------------------------------------------
class S57ObjectTree: public wxTreeCtrl
{
      DECLARE_CLASS( S57ObjectTree )
      DECLARE_EVENT_TABLE()
public:
      /// Constructors
      S57ObjectTree( );
      S57ObjectTree( S57QueryDialog* parent, wxWindowID id = wxID_ANY,
                                        const wxPoint& pos = wxDefaultPosition,
                                        const wxSize& size = wxDefaultSize,
                                        long style = wxTR_HAS_BUTTONS );

      ~S57ObjectTree( );

      /// Initialise our variables
      void Init();

      //  Override events
      void OnItemExpanding( wxTreeEvent& event);
      void OnItemSelectChange( wxTreeEvent& event);

      //    Data
      S57QueryDialog    *m_parent;

};

class MyTreeItemData : public wxTreeItemData
{
public:
      MyTreeItemData(S57ObjectDesc *pOD){ m_pOD = pOD; }

      S57ObjectDesc     *m_pOD;
};



//----------------------------------------------------------------------------
// Generic Rollover Window
//----------------------------------------------------------------------------
class RolloverWin: public wxWindow
{
      public:
            RolloverWin(wxWindow *parent);
            ~RolloverWin();

            void OnPaint(wxPaintEvent& event);

            void SetColorScheme(ColorScheme cs);
            void SetString(wxString &s){ m_string = s; }
            void SetPosition(wxPoint pt){ m_position = pt; }
            void SetBitmap(void);
            void SetBestPosition(int x, int y, int off_x, int off_y, wxSize parent_size);


      private:

            wxString          m_string;
            wxSize            m_size;
            wxPoint           m_position;
            wxBitmap          *m_pbm;

            DECLARE_EVENT_TABLE()
};


//------------------------------------------------------------------------------
//    AISInfoWin Specification
//------------------------------------------------------------------------------

class AISInfoWin : public wxWindow
{
      public:
            AISInfoWin ( wxWindow *parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                         long style = 0, const wxString& name = _T(""));

            ~AISInfoWin(void);

            void OnPaint(wxPaintEvent& event);
            void AppendText(wxString &text);
            void Clear(void){}
            void SetInsertionPoint(int pt){}
            wxSize GetOptimumSize(int *pn_nl = NULL, int *pn_cmax = NULL);
            void SetHPad(int d){ m_offsetx = d; }
            void SetVPad(int d){ m_offsety = d; }

            wxString    m_text;
            int         m_maxtl;

            int         m_offsetx, m_offsety;

            DECLARE_EVENT_TABLE()
};

//------------------------------------------------------------------------------
//    CM93 Detail Slider Specification
//------------------------------------------------------------------------------

class CM93DSlide : public wxDialog
{
      public:
            CM93DSlide ( wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue,
                         const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& title = _T(""));

            ~CM93DSlide(void);

            void Init(void);
            bool Create( wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue,
                                     const wxPoint& pos, const wxSize& size, long style, const wxString& title);

            void OnCancelClick( wxCommandEvent& event );
            void OnMove( wxMoveEvent& event );
            void OnChangeValue( wxScrollEvent& event);
            void OnClose(wxCloseEvent& event);


            wxSlider          *m_pCM93DetailSlider;
            wxWindow          *m_pparent;

            DECLARE_EVENT_TABLE()
};


//----------------------------------------------------------------------------
// ChartInfo Rollover Window
//----------------------------------------------------------------------------
class ChInfoWin: public wxWindow
{
      public:
            ChInfoWin(wxWindow *parent);
            ~ChInfoWin();

            void SetString(wxString &s){ m_string = s; }
            void SetPosition(wxPoint pt){ m_position = pt; }
            void SetWinSize(wxSize sz){ m_size = sz; }
            void SetBitmap(void);


      private:

            wxString          m_string;
            wxSize            m_size;
            wxPoint           m_position;
            wxTextCtrl        *m_pInfoTextCtl;

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
#define SYMBOL_GOTOPOS_TITLE _("Jump To Position")
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

      /// Should we show tooltips?
            static bool ShowToolTips();

            wxTextCtrl*   m_MarkLatCtl;
            wxTextCtrl*   m_MarkLonCtl;
            wxButton*     m_CancelButton;
            wxButton*     m_OKButton;

            double        m_lat_save;
            double        m_lon_save;
};




#endif
