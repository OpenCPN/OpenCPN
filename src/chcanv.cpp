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


// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers
#include "wx/image.h"
#include <wx/graphics.h>
#include <wx/listbook.h>
#include <wx/clipbrd.h>
#include <wx/aui/aui.h>

#include "dychart.h"

#include <wx/listimpl.cpp>

#include "chcanv.h"

#include "geodesic.h"
#include "styles.h"
#include "routeman.h"
#include "statwin.h"
#include "navutil.h"
#include "kml.h"
#include "concanv.h"
#include "thumbwin.h"
#include "chartdb.h"
#include "chartimg.h"
#include "chart1.h"
#include "cutil.h"
#include "routeprop.h"
#include "tcmgr.h"
#include "routemanagerdialog.h"
#include "pluginmanager.h"
#include "ocpn_pixel.h"
#include "ocpndc.h"
#include "undo.h"
#include "toolbar.h"
#include "multiplexer.h"

#ifdef USE_S57
#include "cm93.h"                   // for chart outline draw
#include "s57chart.h"               // for ArrayOfS57Obj
#include "s52plib.h"
#endif

#include "ais.h"

#ifdef __MSVC__
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

#ifndef __WXMSW__
#include <signal.h>
#include <setjmp.h>


extern struct sigaction sa_all;
extern struct sigaction sa_all_old;

extern sigjmp_buf           env;                    // the context saved by sigsetjmp();
#endif

#include <vector>

//    Profiling support
//#include "/usr/include/valgrind/callgrind.h"

// ----------------------------------------------------------------------------
// Useful Prototypes
// ----------------------------------------------------------------------------
extern bool G_FloatPtInPolygon ( MyFlPoint *rgpts, int wnumpts, float x, float y ) ;
extern void catch_signals(int signo);
extern bool GetMemoryStatus(int *mem_total, int *mem_used);

extern ChartBase        *Current_Vector_Ch;
extern ChartBase        *Current_Ch;
extern double           g_ChartNotRenderScaleFactor;
extern double           gLat, gLon, gCog, gSog, gHdt;
extern double           vLat, vLon;
extern ChartDB          *ChartData;
extern bool             bDBUpdateInProgress;
extern ColorScheme      global_color_scheme;
extern bool             g_bHDTValid;
extern int              g_nbrightness;

extern ConsoleCanvas    *console;

extern RouteList        *pRouteList;
extern MyConfig         *pConfig;
extern Select           *pSelect;
extern Routeman         *g_pRouteMan;
extern ThumbWin         *pthumbwin;
extern TCMgr            *ptcmgr;
extern Select           *pSelectTC;
extern Select           *pSelectAIS;
extern WayPointman      *pWayPointMan;
extern MarkInfoImpl     *pMarkPropDialog;
extern RouteProp        *pRoutePropDialog;
extern MarkInfoImpl     *pMarkInfoDialog;
extern Track            *g_pActiveTrack;
extern bool             g_bConfirmObjectDelete;

extern IDX_entry        *gpIDX;
extern int               gpIDXn;

extern RoutePoint       *pAnchorWatchPoint1;
extern RoutePoint       *pAnchorWatchPoint2;
extern double           AnchorPointMinDist;
extern bool             AnchorAlertOn1;
extern bool             AnchorAlertOn2;
extern wxString         g_AW1GUID;
extern wxString         g_AW2GUID;
extern int              g_nAWDefault;
extern int              g_nAWMax;

extern ocpnFloatingToolbarDialog *g_FloatingToolbarDialog;
extern RouteManagerDialog *pRouteManagerDialog;
extern GoToPositionDialog *pGoToPositionDialog;
extern wxString GetLayerName(int id);

extern bool             bDrawCurrentValues;

#ifdef USE_S57
extern s52plib          *ps52plib;
extern CM93OffsetDialog  *g_pCM93OffsetDialog;
#endif

extern bool             bGPSValid;
extern bool             g_bShowOutlines;
extern bool             g_bShowDepthUnits;
extern AIS_Decoder      *g_pAIS;
extern FontMgr         *pFontMgr;

extern MyFrame          *gFrame;
extern StatWin          *stats;

//    AIS Global configuration
extern bool             g_bShowAIS;
extern bool             g_bCPAMax;
extern double           g_CPAMax_NM;
extern bool             g_bCPAWarn;
extern double           g_CPAWarn_NM;
extern bool             g_bTCPA_Max;
extern double           g_TCPA_Max;
extern bool             g_bMarkLost;
extern double           g_MarkLost_Mins;
extern bool             g_bRemoveLost;
extern double           g_RemoveLost_Mins;
extern bool             g_bShowCOG;
extern double           g_ShowCOG_Mins;
extern bool             g_bShowTracks;
extern double           g_ShowTracks_Mins;
extern bool             g_bShowMoored;
extern double           g_ShowMoored_Kts;
extern bool             g_bAISShowTracks;
extern bool             g_bShowAreaNotices;
extern bool             g_bDrawAISSize;

extern int              g_iNavAidRadarRingsNumberVisible;
extern float            g_fNavAidRadarRingsStep;
extern int              g_pNavAidRadarRingsStepUnits;
extern bool             g_bWayPointPreventDragging;
extern bool             g_bEnableZoomToCursor;

extern AISTargetAlertDialog    *g_pais_alert_dialog_active;
extern AISTargetQueryDialog    *g_pais_query_dialog_active;
extern int              g_ais_query_dialog_x, g_ais_query_dialog_y;

extern int              g_S57_dialog_sx, g_S57_dialog_sy;

extern CM93DSlide       *pCM93DetailSlider;
extern bool             g_bShowCM93DetailSlider;
extern int              g_cm93detail_dialog_x, g_cm93detail_dialog_y;
extern int              g_cm93_zoom_factor;

extern bool             g_b_overzoom_x;                      // Allow high overzoom
extern bool             g_bDisplayGrid;

extern bool             g_bUseGreenShip;

extern ChartCanvas      *cc1;

extern bool             g_bshow_overzoom_emboss;
extern int              g_OwnShipIconType;
extern double           g_n_ownship_length_meters;
extern double           g_n_ownship_beam_meters;
extern double           g_n_gps_antenna_offset_y;
extern double           g_n_gps_antenna_offset_x;
extern long             g_n_ownship_min_mm;

extern wxPlatformInfo   *g_pPlatform;

extern bool             g_bUseRaster;
extern bool             g_bUseVector;
extern bool             g_bUseCM93;

extern bool             g_bCourseUp;
extern double           g_COGAvg;               // only needed for debug....

extern int              g_click_stop;
extern double           g_ownship_predictor_minutes;

extern ArrayOfInts      g_quilt_noshow_index_array;
extern ChartStack       *pCurrentStack;
extern bool              g_bquiting;
extern AISTargetListDialog *g_pAISTargetList;
extern wxString         g_sAIS_Alert_Sound_File;

extern PlugInManager    *g_pi_manager;

extern wxAuiManager      *g_pauimgr;

extern bool             g_bskew_comp;
extern bool             g_bopengl;

extern bool             g_bFullScreenQuilt;
extern wxProgressDialog *s_ProgDialog;

extern bool             g_bsmoothpanzoom;

extern int              g_GPU_MemSize;
extern bool             g_b_useStencil;

bool                    g_bDebugOGL;

extern int              g_memCacheLimit;
extern bool             g_b_assume_azerty;

extern int              g_GroupIndex;
extern ChartGroupArray  *g_pGroupArray;
extern wxString         g_default_wp_icon;

extern int              g_current_arrow_scale;

extern S57QueryDialog   *g_pObjectQueryDialog;
extern ocpnStyle::StyleManager* g_StyleManager;
extern Multiplexer      *g_pMUX;
extern wxArrayOfConnPrm *g_pConnectionParams;

//  TODO why are these static?
static int mouse_x;
static int mouse_y;
static bool mouse_leftisdown;

int r_gamma_mult;
int g_gamma_mult;
int b_gamma_mult;
int gamma_state;
bool g_brightness_init;
int   last_brightness;

// "Curtain" mode parameters
wxDialog                *g_pcurtain;

#define MIN_BRIGHT 10
#define MAX_BRIGHT 100

//    Constants for right click menus
enum
{
    ID_DEF_MENU_MAX_DETAIL = 1,
    ID_DEF_MENU_SCALE_IN,
    ID_DEF_MENU_SCALE_OUT,
    ID_DEF_MENU_DROP_WP,
    ID_DEF_MENU_QUERY,
    ID_DEF_MENU_MOVE_BOAT_HERE,
    ID_DEF_MENU_GOTO_HERE,
    ID_DEF_MENU_GOTOPOSITION,

    ID_WP_MENU_GOTO,
    ID_WP_MENU_DELPOINT,
    ID_WP_MENU_PROPERTIES,
    ID_RT_MENU_ACTIVATE,
    ID_RT_MENU_DEACTIVATE,
    ID_RT_MENU_INSERT,
    ID_RT_MENU_APPEND,
    ID_RT_MENU_COPY,
    ID_TK_MENU_COPY,
    ID_WPT_MENU_COPY,
    ID_WPT_MENU_SENDTOGPS,
    ID_PASTE_WAYPOINT,
    ID_PASTE_ROUTE,
    ID_PASTE_TRACK,
    ID_RT_MENU_DELETE,
    ID_RT_MENU_REVERSE,
    ID_RT_MENU_DELPOINT,
    ID_RT_MENU_ACTPOINT,
    ID_RT_MENU_DEACTPOINT,
    ID_RT_MENU_ACTNXTPOINT,
    ID_RT_MENU_REMPOINT,
    ID_RT_MENU_PROPERTIES,
    ID_WP_MENU_SET_ANCHORWATCH,
    ID_WP_MENU_CLEAR_ANCHORWATCH,
    ID_DEF_MENU_AISTARGETLIST,

    ID_RC_MENU_SCALE_IN,
    ID_RC_MENU_SCALE_OUT,
    ID_RC_MENU_ZOOM_IN,
    ID_RC_MENU_ZOOM_OUT,
    ID_RC_MENU_FINISH,
    ID_DEF_MENU_AIS_QUERY,
    ID_DEF_MENU_AIS_CPA,
    ID_DEF_MENU_ACTIVATE_MEASURE,
    ID_DEF_MENU_DEACTIVATE_MEASURE,

    ID_UNDO,
    ID_REDO,

    ID_DEF_MENU_CM93OFFSET_DIALOG,

    ID_TK_MENU_PROPERTIES,
    ID_TK_MENU_DELETE,
    ID_WP_MENU_ADDITIONAL_INFO,

    ID_DEF_MENU_QUILTREMOVE,
    ID_DEF_MENU_COGUP,
    ID_DEF_MENU_NORTHUP,
    ID_DEF_MENU_TIDEINFO,
    ID_DEF_MENU_CURRENTINFO,

    ID_DEF_MENU_GROUPBASE,

    ID_DEF_MENU_LAST
};

//constants for hight and low tide search
enum
{
    FORWARD_ONE_HOUR_STEP    =3600,
    FORWARD_TEN_MINUTES_STEP =600,
    FORWARD_ONE_MINUTES_STEP =60,
    BACKWARD_ONE_HOUR_STEP    =-3600,
    BACKWARD_TEN_MINUTES_STEP =-600,
    BACKWARD_ONE_MINUTES_STEP =-60
};

//constants for rollovers fonts
enum
{
    AIS_ROLLOVER =1,
    LEG_ROLLOVER =2,
    TC_ROLLOVER  =3
};


PFNGLGENFRAMEBUFFERSEXTPROC         s_glGenFramebuffersEXT;
PFNGLGENRENDERBUFFERSEXTPROC        s_glGenRenderbuffersEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    s_glFramebufferTexture2DEXT;
PFNGLBINDFRAMEBUFFEREXTPROC         s_glBindFramebufferEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC s_glFramebufferRenderbufferEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC     s_glRenderbufferStorageEXT;
PFNGLBINDRENDERBUFFEREXTPROC        s_glBindRenderbufferEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  s_glCheckFramebufferStatusEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC      s_glDeleteFramebuffersEXT;
PFNGLDELETERENDERBUFFERSEXTPROC     s_glDeleteRenderbuffersEXT;

#ifdef __WXMSW__
HINSTANCE s_hGL_DLL;                   // Handle to DLL
#endif

static bool GetglEntryPoints( void )
{
#if defined(__WXMSW__)
    s_hGL_DLL = LoadLibrary( (LPCWSTR) "opengl32.dll" );
    if( NULL == s_hGL_DLL ) return false;

    s_glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) GetProcAddress( s_hGL_DLL,
                             "glGenFramebuffersEXT" );
    s_glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) GetProcAddress( s_hGL_DLL,
                              "glGenRenderbuffersEXT" );
    s_glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) GetProcAddress( s_hGL_DLL,
                                  "glFramebufferTexture2DEXT" );
    s_glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) GetProcAddress( s_hGL_DLL,
                             "glBindFramebufferEXT" );
    s_glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) GetProcAddress(
                                         s_hGL_DLL, "glFramebufferRenderbufferEXT" );
    s_glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) GetProcAddress( s_hGL_DLL,
                                 "glRenderbufferStorageEXT" );
    s_glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) GetProcAddress( s_hGL_DLL,
                              "glBindRenderbufferEXT" );
    s_glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) GetProcAddress( s_hGL_DLL,
                                    "glCheckFramebufferStatusEXT" );
    s_glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) GetProcAddress( s_hGL_DLL,
                                "glDeleteFramebuffersEXT" );
    s_glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC) GetProcAddress( s_hGL_DLL,
                                 "glDeleteRenderbuffersEXT" );

#elif defined(__WXMAC__)
    return false;

#else

    s_glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)glXGetProcAddress((const GLubyte *)"glGenFramebuffersEXT");
    s_glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)glXGetProcAddress((const GLubyte *)"glGenRenderbuffersEXT");
    s_glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)glXGetProcAddress((const GLubyte *)"glFramebufferTexture2DEXT");
    s_glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)glXGetProcAddress((const GLubyte *)"glBindFramebufferEXT");
    s_glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)glXGetProcAddress((const GLubyte *)"glFramebufferRenderbufferEXT");
    s_glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)glXGetProcAddress((const GLubyte *)"glRenderbufferStorageEXT");
    s_glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)glXGetProcAddress((const GLubyte *)"glBindRenderbufferEXT");
    s_glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)glXGetProcAddress((const GLubyte *)"glCheckFramebufferStatusEXT");
    s_glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)glXGetProcAddress((const GLubyte *)"glDeleteFramebuffersEXT");
    s_glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)glXGetProcAddress((const GLubyte *)"glDeleteRenderbuffersEXT");
#endif

    if( NULL == s_glGenFramebuffersEXT ) return false;
    if( NULL == s_glGenRenderbuffersEXT ) return false;
    if( NULL == s_glFramebufferTexture2DEXT ) return false;
    if( NULL == s_glBindFramebufferEXT ) return false;
    if( NULL == s_glFramebufferRenderbufferEXT ) return false;
    if( NULL == s_glRenderbufferStorageEXT ) return false;
    if( NULL == s_glBindRenderbufferEXT ) return false;
    if( NULL == s_glCheckFramebufferStatusEXT ) return false;
    if( NULL == s_glDeleteFramebuffersEXT ) return false;
    if( NULL == s_glDeleteRenderbuffersEXT ) return false;

    return true;
}
static GLboolean QueryExtension( const char *extName )
{
    /*
     ** Search for extName in the extensions string. Use of strstr()
     ** is not sufficient because extension names can be prefixes of
     ** other extension names. Could use strtok() but the constant
     ** string returned by glGetString might be in read-only memory.
     */
    char *p;
    char *end;
    int extNameLen;

    extNameLen = strlen( extName );

    p = (char *) glGetString( GL_EXTENSIONS );
    if( NULL == p ) {
        return GL_FALSE;
    }

    end = p + strlen( p );

    while( p < end ) {
        int n = strcspn( p, " " );
        if( ( extNameLen == n ) && ( strncmp( extName, p, n ) == 0 ) ) {
            return GL_TRUE;
        }
        p += ( n + 1 );
    }
    return GL_FALSE;
}

//----------------------------------------------------------------------------
// ChartInfo Rollover Window Definition
//----------------------------------------------------------------------------
class ChInfoWin: public wxWindow {
public:
    ChInfoWin( wxWindow *parent );
    ~ChInfoWin();

    void SetString( wxString &s )
    {
        m_string = s;
    }
    void SetPosition( wxPoint pt )
    {
        m_position = pt;
    }
    void SetWinSize( wxSize sz )
    {
        m_size = sz;
    }
    void SetBitmap( void );
    void FitToChars( int char_width, int char_height );
    wxSize GetWinSize( void )
    {
        return m_size;
    }
    void OnPaint( wxPaintEvent& event );
    void OnEraseBackground( wxEraseEvent& event );

private:

    wxString m_string;
    wxSize m_size;
    wxPoint m_position;
    wxStaticText *m_pInfoTextCtl;

    DECLARE_EVENT_TABLE()
};

//-----------------------------------------------------------------------
//
//    Chart Info Rollover window implementation
//
//-----------------------------------------------------------------------
BEGIN_EVENT_TABLE(ChInfoWin, wxWindow)
    EVT_PAINT ( ChInfoWin::OnPaint )
    EVT_ERASE_BACKGROUND(ChInfoWin::OnEraseBackground)
END_EVENT_TABLE()

// Define a constructor
ChInfoWin::ChInfoWin( wxWindow *parent ) :
    wxWindow( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER )
{
    int ststyle = wxALIGN_LEFT | wxST_NO_AUTORESIZE;
    m_pInfoTextCtl = new wxStaticText( this, -1, _T ( "" ), wxDefaultPosition, wxDefaultSize,
                                       ststyle );

    Hide();
}

ChInfoWin::~ChInfoWin()
{
    delete m_pInfoTextCtl;
}

void ChInfoWin::OnEraseBackground( wxEraseEvent& event )
{
}

void ChInfoWin::OnPaint( wxPaintEvent& event )
{
    int width, height;
    GetClientSize( &width, &height );
    wxPaintDC dc( this );

    dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UIBCK" ) ) ) );
    dc.SetPen( wxPen( GetGlobalColor( _T ( "UITX1" ) ) ) );
    dc.DrawRectangle( 0, 0, width, height );
}

void ChInfoWin::SetBitmap()
{
    SetBackgroundColour( GetGlobalColor( _T ( "UIBCK" ) ) );

    m_pInfoTextCtl->SetBackgroundColour( GetGlobalColor( _T ( "UIBCK" ) ) );
    m_pInfoTextCtl->SetForegroundColour( GetGlobalColor( _T ( "UITX1" ) ) );

    m_pInfoTextCtl->SetSize( 1, 1, m_size.x - 2, m_size.y - 2 );
    m_pInfoTextCtl->SetLabel( m_string );

    SetSize( m_position.x, m_position.y, m_size.x, m_size.y );
}

void ChInfoWin::FitToChars( int char_width, int char_height )
{
    wxSize size;

    size.x = GetCharWidth() * char_width;
    size.y = GetCharHeight() * ( char_height + 1 );
    SetWinSize( size );
}

//------------------------------------------------------------------------------
//    Quilt Candidate Definition
//------------------------------------------------------------------------------
class QuiltCandidate {
public:
    QuiltCandidate()
    {
        b_include = false;
        b_eclipsed = false;
    }

    int dbIndex;
    int ChartScale;
    bool b_include;
    bool b_eclipsed;
    wxRegion quilt_region;

};

WX_DEFINE_SORTED_ARRAY( QuiltCandidate *, ArrayOfSortedQuiltCandidates );

int CompareScales( QuiltCandidate *qc1, QuiltCandidate *qc2 )
{
    if( !ChartData ) return 0;

    const ChartTableEntry &cte1 = ChartData->GetChartTableEntry( qc1->dbIndex );
    const ChartTableEntry &cte2 = ChartData->GetChartTableEntry( qc2->dbIndex );

    if( cte1.GetScale() == cte2.GetScale() )          // same scales, so sort on dbIndex
        return qc1->dbIndex - qc2->dbIndex;
    else
        return cte1.GetScale() - cte2.GetScale();
}

//------------------------------------------------------------------------------
//    Patch Definition
//------------------------------------------------------------------------------
class QuiltPatch {
public:
    QuiltPatch()
    {
        b_Valid = false;
        b_eclipsed = false;
        b_overlay = false;
    }
    int dbIndex;
    wxRegion ActiveRegion;
    int ProjType;
    bool b_Valid;
    bool b_eclipsed;
    bool b_overlay;
    wxRegion quilt_region;
};

WX_DECLARE_LIST( QuiltPatch, PatchList );

//------------------------------------------------------------------------------
//    Quilt Definition
//------------------------------------------------------------------------------
class Quilt {
public:

    Quilt();
    ~Quilt();

    void SetQuiltParameters( double CanvasScaleFactor, int CanvasWidth )
    {
        m_canvas_scale_factor = CanvasScaleFactor;
        m_canvas_width = CanvasWidth;
    }

    bool BuildExtendedChartStackAndCandidateArray(bool b_fullscreen, int ref_db_index, ViewPort &vp_in);
    bool Compose( const ViewPort &vp );
    bool IsComposed() {
        return m_bcomposed;
    }
    ChartBase *GetFirstChart();
    ChartBase *GetNextChart();
    ChartBase *GetLargestScaleChart();
    ArrayOfInts GetQuiltIndexArray( void );
    bool IsQuiltDelta( ViewPort &vp );
    bool IsChartQuiltableRef( int db_index );
    ViewPort &GetQuiltVP() {
        return m_vp_quilt;
    }
    wxString GetQuiltDepthUnit() {
        return m_quilt_depth_unit;
    }
    void SetRenderedVP( ViewPort &vp ) {
        m_vp_rendered = vp;
    }
    bool HasOverlays( void ) {
        return m_bquilt_has_overlays;
    }

    int GetExtendedStackCount(void) {
        return m_extended_stack_array.GetCount();
    }

    int GetnCharts() {
        return m_PatchList.GetCount();
    }

    void ComputeRenderRegion( ViewPort &vp, wxRegion &chart_region );
    bool RenderQuiltRegionViewOnDC( wxMemoryDC &dc, ViewPort &vp, wxRegion &chart_region );
    bool IsVPBlittable( ViewPort &VPoint, int dx, int dy, bool b_allow_vector = false );
    ChartBase *GetChartAtPix( wxPoint p );
    ChartBase *GetOverlayChartAtPix( wxPoint p );
    int GetChartdbIndexAtPix( wxPoint p );
    void InvalidateAllQuiltPatchs( void );
    void Invalidate( void )
    {
        m_bcomposed = false;
        m_vp_quilt.Invalidate();
        m_zout_dbindex = -1;
    }
    void AdjustQuiltVP( ViewPort &vp_last, ViewPort &vp_proposed );

    wxRegion &GetFullQuiltRegion( void ) {
        return m_covered_region;
    }
    wxRegion &GetFullQuiltRenderedRegion( void ) {
        return m_rendered_region;
    }
    bool IsChartSmallestScale( int dbIndex );

    int AdjustRefOnZoomOut( double proposed_scale_onscreen );
    int AdjustRefOnZoomIn( double proposed_scale_onscreen );

    void SetHiliteIndex( int index ) {
        m_nHiLiteIndex = index;
    }
    void SetReferenceChart( int dbIndex ) {
        m_refchart_dbIndex = dbIndex;
    }
    int GetRefChartdbIndex( void ) {
        return m_refchart_dbIndex;
    }
    int GetQuiltProj( void )
    {
        return m_quilt_proj;
    }
    double GetMaxErrorFactor()
    {
        return m_max_error_factor;
    }
    double GetRefScale()
    {
        return m_reference_scale;
    }
    double GetRefNativeScale();

    ArrayOfInts GetCandidatedbIndexArray( bool from_ref_chart, bool exclude_user_hidden );
    ArrayOfInts GetExtendedStackIndexArray()
    {
        return m_extended_stack_array;
    }
    ArrayOfInts GetEclipsedStackIndexArray()
    {
        return m_eclipsed_stack_array;
    }

    unsigned long GetXStackHash() {
        return m_xa_hash;
    }

    bool IsBusy()
    {
        return m_bbusy;
    }
    QuiltPatch *GetCurrentPatch();
    bool IsChartInQuilt( ChartBase *pc );
    bool IsQuiltVector( void );
    wxRegion GetHiliteRegion( ViewPort &vp );

private:
    wxRegion GetChartQuiltRegion( const ChartTableEntry &cte, ViewPort &vp );
    void EmptyCandidateArray( void );
    void SubstituteClearDC( wxMemoryDC &dc, ViewPort &vp );
    int GetNewRefChart( void );

    wxRegion m_covered_region;
    wxRegion m_rendered_region;

    PatchList m_PatchList;
    wxBitmap *m_pBM;

    bool m_bcomposed;
    wxPatchListNode *cnode;
    bool m_bbusy;
    int m_quilt_proj;

    ArrayOfSortedQuiltCandidates *m_pcandidate_array;
    ArrayOfInts m_last_index_array;
    ArrayOfInts m_index_array;
    ArrayOfInts m_extended_stack_array;
    ArrayOfInts m_eclipsed_stack_array;

    ViewPort m_vp_quilt;
    ViewPort m_vp_rendered;          // last VP rendered

    int m_nHiLiteIndex;
    int m_refchart_dbIndex;
    int m_reference_scale;
    int m_reference_type;
    int m_reference_family;
    bool m_bneed_clear;
    wxRegion m_back_region;
    wxString m_quilt_depth_unit;
    double m_max_error_factor;
    double m_canvas_scale_factor;
    int m_canvas_width;
    bool m_bquilt_has_overlays;
    unsigned long m_xa_hash;
    int m_zout_dbindex;

};

WX_DEFINE_LIST( PatchList );

//------------------------------------------------------------------------------
//    Quilt Implementation
//------------------------------------------------------------------------------
Quilt::Quilt()
{
//      m_bEnableRaster = true;
//      m_bEnableVector = false;;
//      m_bEnableCM93 = false;

    m_reference_scale = 1;
    m_refchart_dbIndex = -1;
    m_reference_type = CHART_TYPE_UNKNOWN;
    m_reference_family = CHART_FAMILY_UNKNOWN;

    cnode = NULL;

    m_pBM = NULL;
    m_bcomposed = false;
    m_bbusy = false;

    m_pcandidate_array = new ArrayOfSortedQuiltCandidates( CompareScales );
    m_nHiLiteIndex = -1;

}

Quilt::~Quilt()
{
    m_PatchList.DeleteContents( true );
    m_PatchList.Clear();

    EmptyCandidateArray();
    delete m_pcandidate_array;

    m_extended_stack_array.Clear();

    delete m_pBM;
}

bool Quilt::IsVPBlittable( ViewPort &VPoint, int dx, int dy, bool b_allow_vector )
{
    bool ret_val = true;
    if( m_vp_rendered.IsValid() ) {
        wxPoint2DDouble p1 = VPoint.GetDoublePixFromLL( m_vp_rendered.clat, m_vp_rendered.clon );
        wxPoint2DDouble p2 = VPoint.GetDoublePixFromLL( VPoint.clat, VPoint.clon );
        double deltax = p2.m_x - p1.m_x;
        double deltay = p2.m_y - p1.m_y;

        ChartBase *pch = GetFirstChart();
        while( pch ) {
            if( pch->GetChartFamily() == CHART_FAMILY_RASTER ) {
                if( ( fabs( deltax - dx ) > 1e-2 ) || ( fabs( deltay - dy ) > 1e-2 ) ) {
                    ret_val = false;
                    break;
                }

            } else {
                if( !b_allow_vector ) {
                    ret_val = false;
                    break;
                } else if( ( fabs( deltax - dx ) > 1e-2 ) || ( fabs( deltay - dy ) > 1e-2 ) ) {
                    ret_val = false;
                    break;
                }

            }

            pch = GetNextChart();
        }
    } else
        ret_val = false;

    return ret_val;
}

bool Quilt::IsChartQuiltableRef( int db_index )
{
    if( db_index < 0 ) return false;

    //    Is the chart targeted by db_index useable as a quilt reference chart?
    const ChartTableEntry &ctei = ChartData->GetChartTableEntry( db_index );

    bool bproj_match = true;                  // Accept all projections

    double skew_norm = ctei.GetChartSkew();
    if( skew_norm > 180. ) skew_norm -= 360.;

    bool skew_match = fabs( skew_norm ) < 1.;  // Only un-skewed charts are acceptable for quilt

    //    In noshow array?
    bool b_noshow = false;
    for( unsigned int i = 0; i < g_quilt_noshow_index_array.GetCount(); i++ ) {
        if( g_quilt_noshow_index_array.Item( i ) == db_index )        // chart is in the noshow list
        {
            b_noshow = true;
            break;
        }
    }

    return ( bproj_match & skew_match & !b_noshow );
}

bool Quilt::IsChartInQuilt( ChartBase *pc )
{
    //    Iterate thru the quilt
    for( unsigned int ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
        QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
        if( ( pqc->b_include ) && ( !pqc->b_eclipsed ) ) {
            if( ChartData->OpenChartFromDB( pqc->dbIndex, FULL_INIT ) == pc ) return true;
        }
    }
    return false;
}

ArrayOfInts Quilt::GetCandidatedbIndexArray( bool from_ref_chart, bool exclude_user_hidden )
{
    ArrayOfInts ret;
    for( unsigned int ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
        QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
        if( from_ref_chart )                     // only add entries of smaller scale than ref scale
        {
            if( pqc->ChartScale >= m_reference_scale ) {
                // Search the no-show array
                if( exclude_user_hidden ) {
                    bool b_noshow = false;
                    for( unsigned int i = 0; i < g_quilt_noshow_index_array.GetCount(); i++ ) {
                        if( g_quilt_noshow_index_array.Item( i ) == pqc->dbIndex ) // chart is in the noshow list
                        {
                            b_noshow = true;
                            break;
                        }
                    }
                    if( !b_noshow ) ret.Add( pqc->dbIndex );
                } else {
                    ret.Add( pqc->dbIndex );
                }
            }
        } else
            ret.Add( pqc->dbIndex );

    }
    return ret;
}

QuiltPatch *Quilt::GetCurrentPatch()
{
    if( cnode ) return ( cnode->GetData() );
    else
        return NULL;
}

void Quilt::EmptyCandidateArray( void )
{
    for( unsigned int i = 0; i < m_pcandidate_array->GetCount(); i++ ) {
        delete m_pcandidate_array->Item( i );
    }

    m_pcandidate_array->Clear();

}

ChartBase *Quilt::GetFirstChart()
{
    if( !ChartData ) return NULL;

    if( !ChartData->IsValid() ) // This could happen during yield recursion from progress dialog during databse update
        return NULL;

    if( !m_bcomposed ) return NULL;

    if( m_bbusy ) return NULL;

    m_bbusy = true;
    ChartBase *pret = NULL;
    cnode = m_PatchList.GetFirst();
    while( cnode && !cnode->GetData()->b_Valid )
        cnode = cnode->GetNext();
    if( cnode && cnode->GetData()->b_Valid ) pret = ChartData->OpenChartFromDB(
                    cnode->GetData()->dbIndex, FULL_INIT );

    m_bbusy = false;
    return pret;
}

ChartBase *Quilt::GetNextChart()
{
    if( !ChartData ) return NULL;

    if( !ChartData->IsValid() ) return NULL;

    if( m_bbusy ) return NULL;

    m_bbusy = true;
    ChartBase *pret = NULL;
    if( cnode ) {
        cnode = cnode->GetNext();
        while( cnode && !cnode->GetData()->b_Valid )
            cnode = cnode->GetNext();
        if( cnode && cnode->GetData()->b_Valid ) pret = ChartData->OpenChartFromDB(
                        cnode->GetData()->dbIndex, FULL_INIT );
    }

    m_bbusy = false;
    return pret;
}

ChartBase *Quilt::GetLargestScaleChart()
{
    if( !ChartData ) return NULL;

    if( m_bbusy ) return NULL;

    m_bbusy = true;
    ChartBase *pret = NULL;
    cnode = m_PatchList.GetLast();
    if( cnode ) pret = ChartData->OpenChartFromDB( cnode->GetData()->dbIndex, FULL_INIT );

    m_bbusy = false;
    return pret;
}

wxRegion Quilt::GetChartQuiltRegion( const ChartTableEntry &cte, ViewPort &vp )
{
    wxRegion chart_region;
    wxRegion screen_region( vp.rv_rect );

    // Special case for charts which extend around the world, or near to it
    //  Mostly this means cm93....
    //  Take the whole screen, clipped at +/- 80 degrees lat
    if(fabs(cte.GetLonMax() - cte.GetLonMin()) > 180.) {
        int n_ply_entries = 4;
        float ply[8];
        ply[0] = 80.;
        ply[1] = vp.GetBBox().GetMinX();
        ply[2] = 80.;
        ply[3] = vp.GetBBox().GetMaxX();
        ply[4] = -80.;
        ply[5] = vp.GetBBox().GetMaxX();
        ply[6] = -80.;
        ply[7] = vp.GetBBox().GetMinX();


        wxRegion t_region = vp.GetVPRegionIntersect( screen_region, 4, &ply[0],
                                                     cte.GetScale() );
        return t_region;
    }


    //    If the chart has an aux ply table, use it for finer region precision
    int nAuxPlyEntries = cte.GetnAuxPlyEntries();
    if( nAuxPlyEntries >= 1 ) {
        for( int ip = 0; ip < nAuxPlyEntries; ip++ ) {
            float *pfp = cte.GetpAuxPlyTableEntry( ip );
            int nAuxPly = cte.GetAuxCntTableEntry( ip );

            wxRegion t_region = vp.GetVPRegionIntersect( screen_region, nAuxPly, pfp,
                                cte.GetScale() );
            if( !t_region.Empty() )
                chart_region.Union( t_region );
        }
    }

    else {
        int n_ply_entries = cte.GetnPlyEntries();
        float *pfp = cte.GetpPlyTable();

        if( n_ply_entries >= 3 ) // could happen with old database and some charts, e.g. SHOM 2381.kap
        {
            wxRegion t_region = vp.GetVPRegionIntersect( screen_region, n_ply_entries, pfp,
                                cte.GetScale() );
            if( !t_region.Empty() )
                chart_region.Union( t_region );

        } else
            chart_region = screen_region;
    }

    //  Remove the NoCovr regions
    int nNoCovrPlyEntries = cte.GetnNoCovrPlyEntries();
    if( nNoCovrPlyEntries ) {
        for( int ip = 0; ip < nNoCovrPlyEntries; ip++ ) {
            float *pfp = cte.GetpNoCovrPlyTableEntry( ip );
            int nNoCovrPly = cte.GetNoCovrCntTableEntry( ip );

            wxRegion t_region = vp.GetVPRegionIntersect( screen_region, nNoCovrPly, pfp,
                                                         cte.GetScale() );

            //  We do a test removal of the NoCovr region.
            //  If the result iz empty, it must be that the NoCovr region is
            //  the full extent M_COVR(CATCOV=2) feature found in NOAA ENCs.
            //  We ignore it.

            if(!t_region.IsEmpty()) {
                wxRegion test_region = chart_region;
                test_region.Subtract( t_region );

                if( !test_region.IsEmpty())
                    chart_region = test_region;
            }
        }
    }


    //    Another superbad hack....
    //    Super small scale raster charts like bluemarble.kap usually cross the prime meridian
    //    and Plypoints georef is problematic......
    //    So, force full screen coverage in the quilt
    if( (cte.GetScale() > 90000000) && (cte.GetChartFamily() == CHART_FAMILY_RASTER) )
        chart_region = screen_region;

    //    Clip the region to the current viewport
    chart_region.Intersect( vp.rv_rect );

    if( chart_region.IsOk() ) return chart_region;
    else
        return wxRegion( 0, 0, 100, 100 );
}

bool Quilt::IsQuiltVector( void )
{
    if( m_bbusy ) return false;

    m_bbusy = true;

    bool ret = false;

    wxPatchListNode *cnode = m_PatchList.GetFirst();
    while( cnode ) {
        if( cnode->GetData() ) {
            QuiltPatch *pqp = cnode->GetData();

            if( ( pqp->b_Valid ) && ( !pqp->b_eclipsed ) ) {
                const ChartTableEntry &ctei = ChartData->GetChartTableEntry( pqp->dbIndex );

                if( ctei.GetChartFamily() == CHART_FAMILY_VECTOR ) {
                    ret = true;
                    break;
                }

            }
        }
        cnode = cnode->GetNext();
    }

    m_bbusy = false;
    return ret;
}

int Quilt::GetChartdbIndexAtPix( wxPoint p )
{
    if( m_bbusy ) return -1;

    m_bbusy = true;

    int ret = -1;

    wxPatchListNode *cnode = m_PatchList.GetFirst();
    while( cnode ) {
        if( cnode->GetData()->ActiveRegion.Contains( p ) == wxInRegion ) {
            ret = cnode->GetData()->dbIndex;
            break;
        } else
            cnode = cnode->GetNext();
    }

    m_bbusy = false;
    return ret;
}

ChartBase *Quilt::GetChartAtPix( wxPoint p )
{
    if( m_bbusy ) return NULL;

    m_bbusy = true;

    //    The patchlist is organized from small to large scale.
    //    We generally will want the largest scale chart at this point, so
    //    walk the whole list.  The result will be the last one found, i.e. the largest scale chart.
    ChartBase *pret = NULL;
    wxPatchListNode *cnode = m_PatchList.GetFirst();
    while( cnode ) {
        QuiltPatch *pqp = cnode->GetData();
        if( !pqp->b_overlay && (pqp->ActiveRegion.Contains( p ) == wxInRegion) )
                pret = ChartData->OpenChartFromDB( pqp->dbIndex, FULL_INIT );
        cnode = cnode->GetNext();
    }

    m_bbusy = false;
    return pret;
}

ChartBase *Quilt::GetOverlayChartAtPix( wxPoint p )
{
    if( m_bbusy ) return NULL;

    m_bbusy = true;

    //    The patchlist is organized from small to large scale.
    //    We generally will want the largest scale chart at this point, so
    //    walk the whole list.  The result will be the last one found, i.e. the largest scale chart.
    ChartBase *pret = NULL;
    wxPatchListNode *cnode = m_PatchList.GetFirst();
    while( cnode ) {
        QuiltPatch *pqp = cnode->GetData();
        if( pqp->b_overlay && ( pqp->ActiveRegion.Contains( p ) == wxInRegion) )
                pret = ChartData->OpenChartFromDB( pqp->dbIndex, FULL_INIT );
        cnode = cnode->GetNext();
    }

    m_bbusy = false;
    return pret;
}


void Quilt::InvalidateAllQuiltPatchs( void )
{
    if( m_bbusy ) return;

    m_bbusy = true;
    m_bbusy = false;
    return;
}

ArrayOfInts Quilt::GetQuiltIndexArray( void )
{
    return m_index_array;

    ArrayOfInts ret;

    if( m_bbusy ) return ret;

    m_bbusy = true;

    wxPatchListNode *cnode = m_PatchList.GetFirst();
    while( cnode ) {
        ret.Add( cnode->GetData()->dbIndex );
        cnode = cnode->GetNext();
    }

    m_bbusy = false;

    return ret;
}

bool Quilt::IsQuiltDelta( ViewPort &vp )
{
    if( !m_vp_quilt.IsValid() || !m_bcomposed ) return true;

    if( m_vp_quilt.view_scale_ppm != vp.view_scale_ppm ) return true;

    //    Has the quilt shifted by more than one pixel in any direction?
    wxPoint cp_last, cp_this;

    cp_last = m_vp_quilt.GetPixFromLL( vp.clat, vp.clon );
    cp_this = vp.GetPixFromLL( vp.clat, vp.clon );

    return ( cp_last != cp_this );
}

void Quilt::AdjustQuiltVP( ViewPort &vp_last, ViewPort &vp_proposed )
{
    if( m_bbusy ) return;

//      ChartBase *pRefChart = GetLargestScaleChart();
    ChartBase *pRefChart = ChartData->OpenChartFromDB( m_refchart_dbIndex, FULL_INIT );

    if( pRefChart ) pRefChart->AdjustVP( vp_last, vp_proposed );
}

double Quilt::GetRefNativeScale()
{
    double ret_val = 1.0;
    if( ChartData ) {
        ChartBase *pc = ChartData->OpenChartFromDB( m_refchart_dbIndex, FULL_INIT );
        if( pc ) ret_val = pc->GetNativeScale();
    }

    return ret_val;
}

int Quilt::GetNewRefChart( void )
{
    //    Using the current quilt, select a useable reference chart
    //    Said chart will be in the extended (possibly full-screen) stack,
    //    And will have a scale equal to or just greater than the current quilt reference scale,
    //    And will match current quilt projection type, and
    //    will have Skew=0, so as to be fully quiltable
    int new_ref_dbIndex = m_refchart_dbIndex;
    unsigned int im = m_extended_stack_array.GetCount();
    if( im > 0 ) {
        for( unsigned int is = 0; is < im; is++ ) {
            const ChartTableEntry &m = ChartData->GetChartTableEntry( m_extended_stack_array.Item( is ) );
//                  if((m.GetScale() >= m_reference_scale) && (m_reference_type == m.GetChartType()))
            if( ( m.GetScale() >= m_reference_scale )
                    && ( m_reference_family == m.GetChartFamily() )
                    && ( m_quilt_proj == m.GetChartProjectionType() )
                    && ( m.GetChartSkew() == 0.0 ) ) {
                new_ref_dbIndex = m_extended_stack_array.Item( is );
                break;
            }
        }
    }
    return new_ref_dbIndex;
}

int Quilt::AdjustRefOnZoomOut( double proposed_scale_onscreen )
{
    //    If the reference chart is undefined, we really need to select one now.
    if( m_refchart_dbIndex < 0 ) {
        int new_ref_dbIndex = GetNewRefChart();
        SetReferenceChart( new_ref_dbIndex );
    }

    int new_db_index = m_refchart_dbIndex;

    unsigned int extended_array_count = m_extended_stack_array.GetCount();

    if( m_refchart_dbIndex >= 0 && ( extended_array_count > 0 ) ) {
        ChartBase *pc = ChartData->OpenChartFromDB( m_refchart_dbIndex, FULL_INIT );
        if( pc ) {
            int current_db_index = m_refchart_dbIndex;
            int current_family = m_reference_family;

            double max_ref_scale = pc->GetNormalScaleMax( m_canvas_scale_factor, m_canvas_width );

            if( proposed_scale_onscreen > max_ref_scale ) {
                m_zout_dbindex = -1;
                unsigned int target_stack_index = 0;
                int target_stack_index_check = m_extended_stack_array.Index( current_db_index ); // Lookup

                if( wxNOT_FOUND != target_stack_index_check ) target_stack_index =
                        target_stack_index_check;

                while( ( proposed_scale_onscreen > max_ref_scale )
                        && ( target_stack_index < ( extended_array_count - 1 ) ) ) {
                    target_stack_index++;
                    int test_db_index = m_extended_stack_array.Item( target_stack_index );

                    if( ( current_family == ChartData->GetDBChartFamily( test_db_index ) )
                            && IsChartQuiltableRef( test_db_index ) ) {
                        //    open the target, and check the min_scale
                        ChartBase *ptest_chart = ChartData->OpenChartFromDB( test_db_index,
                                                 FULL_INIT );
                        if( ptest_chart ) max_ref_scale = ptest_chart->GetNormalScaleMax(
                                                                  m_canvas_scale_factor, m_canvas_width );
                    }
                }

                bool b_ref_set = false;
                if( proposed_scale_onscreen > max_ref_scale) {          // could not find a useful chart

                    //  If cm93 is available, allow a one-time switch of chart family
                    //  and leave a bread crumb (m_zout_dbindex) to allow selecting this chart
                    //  and family on subsequent zoomin.
                    for( unsigned int ir = 0; ir < m_extended_stack_array.GetCount(); ir++ ) {
                        int i = m_extended_stack_array.Item(ir);        // chart index

                        if( ChartData->GetDBChartType( i ) == CHART_TYPE_CM93COMP )  {
                            target_stack_index = ir;
                            m_zout_dbindex = m_refchart_dbIndex;  //save for later
                            SetReferenceChart( i );
                            b_ref_set = true;
                            break;
                        }
                    }
                }

                if( !b_ref_set && (target_stack_index < extended_array_count) ) {
                    new_db_index = m_extended_stack_array.Item( target_stack_index );
                    if( ( current_family == ChartData->GetDBChartFamily( new_db_index ) )
                            && IsChartQuiltableRef( new_db_index ) )
                        SetReferenceChart( new_db_index );
                }
            }
        }
    }
    return m_refchart_dbIndex;
}

int Quilt::AdjustRefOnZoomIn( double proposed_scale_onscreen )
{
    //    If the reference chart is undefined, we really need to select one now.
    if( m_refchart_dbIndex < 0 ) {
        int new_ref_dbIndex = GetNewRefChart();
        SetReferenceChart( new_ref_dbIndex );
    }

    int new_db_index = m_refchart_dbIndex;
    int current_db_index = m_refchart_dbIndex;
    int current_family = m_reference_family;

    unsigned int extended_array_count = m_extended_stack_array.GetCount();

    if( m_refchart_dbIndex >= 0 && ( extended_array_count > 0 ) ) {
        ChartBase *pc = ChartData->OpenChartFromDB( m_refchart_dbIndex, FULL_INIT );
        if( pc ) {
            double min_ref_scale = pc->GetNormalScaleMin( m_canvas_scale_factor, false );

            //  If the current chart is cm93, and it became so due to a zout from another family,
            //  detect this case and allow switch to save chart index if the min/max scales comply
            if( ( pc->GetChartType() == CHART_TYPE_CM93COMP ) && (m_zout_dbindex >= 0) ) {
                min_ref_scale = proposed_scale_onscreen + 1;  // just to force the test below to pass
                current_family = ChartData->GetDBChartFamily( m_zout_dbindex );
            }

            if( proposed_scale_onscreen < min_ref_scale )  {

                unsigned int target_stack_index = 0;
                int target_stack_index_check = m_extended_stack_array.Index( current_db_index ); // Lookup

                if( wxNOT_FOUND != target_stack_index_check ) target_stack_index =
                        target_stack_index_check;

                while( ( proposed_scale_onscreen < min_ref_scale ) && ( target_stack_index > 0 ) ) {
                    target_stack_index--;
                    int test_db_index = m_extended_stack_array.Item( target_stack_index );

                    if( pCurrentStack->DoesStackContaindbIndex( test_db_index ) ) {
                        if( ( current_family == ChartData->GetDBChartFamily( test_db_index ) )
                                && IsChartQuiltableRef( test_db_index ) ) {

                            //    open the target, and check the min_scale
                            ChartBase *ptest_chart = ChartData->OpenChartFromDB( test_db_index,
                                                     FULL_INIT );
                            if( ptest_chart ) min_ref_scale = ptest_chart->GetNormalScaleMin(
                                                                      m_canvas_scale_factor, false );
                        }
                    }
                }

                if( target_stack_index >= 0 ) {
                    new_db_index = m_extended_stack_array.Item( target_stack_index );

                    //  The target chart min/max scales must comply with propsed chart scale on-screen
                    ChartBase *pcandidate = ChartData->OpenChartFromDB( new_db_index, FULL_INIT );
                    double test_max_ref_scale = 1e8;
                    double test_min_ref_scale = 0;
                    if(pcandidate) {
                        test_max_ref_scale = 1.01 * pcandidate->GetNormalScaleMax( m_canvas_scale_factor, cc1->GetCanvasWidth() );
                        test_min_ref_scale = 0.99 * pcandidate->GetNormalScaleMin( m_canvas_scale_factor, false );
                    }

                    if( ( current_family == ChartData->GetDBChartFamily( new_db_index ) )
                            && IsChartQuiltableRef( new_db_index )
                            && (proposed_scale_onscreen >= test_min_ref_scale)
                            && (proposed_scale_onscreen <= test_max_ref_scale) )

                            SetReferenceChart( new_db_index );
                } else {
                    int new_ref_dbIndex = GetNewRefChart();
                    SetReferenceChart( new_ref_dbIndex );
                }
            }
        }
    }
    return m_refchart_dbIndex;
}

bool Quilt::IsChartSmallestScale( int dbIndex )
{
    // find the smallest scale chart of the specified type on the extended stack array

    int specified_type = ChartData->GetDBChartType( dbIndex );
    int target_dbindex = -1;

    unsigned int target_stack_index = 0;
    if( m_extended_stack_array.GetCount() ) {
        while( ( target_stack_index <= ( m_extended_stack_array.GetCount() - 1 ) ) ) {
            int test_db_index = m_extended_stack_array.Item( target_stack_index );

            if( specified_type == ChartData->GetDBChartType( test_db_index ) ) target_dbindex =
                    test_db_index;

            target_stack_index++;
        }
    }
    return ( dbIndex == target_dbindex );
}

wxRegion Quilt::GetHiliteRegion( ViewPort &vp )
{
    wxRegion r;
    if( m_nHiLiteIndex >= 0 ) {
        // Walk the PatchList, looking for the target hilite index
        for( unsigned int i = 0; i < m_PatchList.GetCount(); i++ ) {
            wxPatchListNode *pcinode = m_PatchList.Item( i );
            QuiltPatch *piqp = pcinode->GetData();
            if( ( m_nHiLiteIndex == piqp->dbIndex ) && ( piqp->b_Valid ) ) // found it
            {
                r = piqp->ActiveRegion;
                break;
            }
        }

        // If not in the patchlist, look in the full chartbar
        if( r.IsEmpty() ) {
            for( unsigned int ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
                QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
                if( m_nHiLiteIndex == pqc->dbIndex ) {
                    const ChartTableEntry &cte = ChartData->GetChartTableEntry( m_nHiLiteIndex );
                    wxRegion chart_region = pqc->quilt_region;
                    if( !chart_region.Empty() ) {
                        // Do not highlite fully eclipsed charts
                        bool b_eclipsed = false;
                        for( unsigned int ir = 0; ir < m_eclipsed_stack_array.GetCount(); ir++ ) {
                            if( m_nHiLiteIndex == m_eclipsed_stack_array.Item( ir ) ) {
                                b_eclipsed = true;
                                break;
                            }
                        }

                        if( !b_eclipsed )
                            r = chart_region;
                        break;
                    }
                }
            }
        }
    }
    return r;
}

bool Quilt::BuildExtendedChartStackAndCandidateArray(bool b_fullscreen, int ref_db_index, ViewPort &vp_in)
{
    EmptyCandidateArray();
    m_extended_stack_array.Clear();

    int reference_scale;
    int reference_type = -1;
    int reference_family;
    int quilt_proj;

    if( ref_db_index >= 0 ) {
        const ChartTableEntry &cte_ref = ChartData->GetChartTableEntry( ref_db_index );
        reference_scale = cte_ref.GetScale();
        reference_type = cte_ref.GetChartType();
        quilt_proj = ChartData->GetDBChartProj( ref_db_index );
        reference_family = cte_ref.GetChartFamily();
    }

    bool b_need_resort = false;

    ViewPort vp_local = vp_in;          // non-const copy

    if( !pCurrentStack ) {
        pCurrentStack = new ChartStack;
        ChartData->BuildChartStack( pCurrentStack, vp_local.clat, vp_local.clon );
    }

    int n_charts = 0;
    if( pCurrentStack ) {
        n_charts = pCurrentStack->nEntry;

        //    Walk the current ChartStack...
        //    Building the quilt candidate array
        for( int ics = 0; ics < n_charts; ics++ ) {
            int i = pCurrentStack->GetDBIndex( ics );
            m_extended_stack_array.Add( i );
            //  A viable candidate?
            double chart_skew = ChartData->GetDBChartSkew( i );
            if( chart_skew > 180. ) chart_skew -= 360.;

            // only unskewed charts of the proper projection and type may be quilted....
            // and we avoid adding CM93 Composite until later
            if( ( reference_type == ChartData->GetDBChartType( i ) )
            && ( fabs( chart_skew ) < 1.0 )
            && ( ChartData->GetDBChartProj( i ) == quilt_proj )
            && ( ChartData->GetDBChartType( i ) != CHART_TYPE_CM93COMP ) ) {
                QuiltCandidate *qcnew = new QuiltCandidate;
                qcnew->dbIndex = i;
                qcnew->ChartScale = ChartData->GetDBChartScale( i );

                //      Calculate and store the quilt region on-screen with the candidate
                const ChartTableEntry &cte = ChartData->GetChartTableEntry( i );
                wxRegion chart_region = GetChartQuiltRegion( cte, vp_local );
                qcnew->quilt_region = chart_region;

                m_pcandidate_array->Add( qcnew );               // auto-sorted on scale

            }
        }
    }

    if( b_fullscreen ) {
        //    Search the entire database, potentially adding all charts
        //    which intersect the ViewPort in any way
        //    .AND. other requirements.
        //    Again, skipping cm93 for now
        int n_all_charts = ChartData->GetChartTableEntries();

        LLBBox viewbox = vp_local.GetBBox();
        int sure_index = -1;
        int sure_index_scale = 0;

        for( int i = 0; i < n_all_charts; i++ ) {
            //    We can eliminate some charts immediately
            //    Try to make these tests in some sensible order....

            if( reference_type != ChartData->GetDBChartType( i ) ) continue;

            if( ChartData->GetDBChartType( i ) == CHART_TYPE_CM93COMP ) continue;

            if( ( g_GroupIndex > 0 ) && ( !ChartData->IsChartInGroup( i, g_GroupIndex ) ) ) continue;

            wxBoundingBox chart_box;
            ChartData->GetDBBoundingBox( i, &chart_box );
            if( ( viewbox.Intersect( chart_box ) == _OUT ) ) continue;

            if( quilt_proj != ChartData->GetDBChartProj( i ) ) continue;

            double chart_skew = ChartData->GetDBChartSkew( i );
            if( chart_skew > 180. ) chart_skew -= 360.;
            if( fabs( chart_skew ) > 1.0 ) continue;

            //    Calculate zoom factor for this chart
            double chart_native_ppm;
            chart_native_ppm = m_canvas_scale_factor / ChartData->GetDBChartScale( i );
            double zoom_factor = vp_in.view_scale_ppm / chart_native_ppm;

            //  Try to guarantee that there is one chart added with scale larger than reference scale
            //    Take note here, and keep track of the smallest scale chart that is larger scale than reference....
            if( ChartData->GetDBChartScale( i ) < reference_scale ) {
                if( ChartData->GetDBChartScale( i ) > sure_index_scale ) {
                    sure_index = i;
                    sure_index_scale = ChartData->GetDBChartScale( i );
                }
            }

            //    At this point, the candidate is the right type, skew, and projection, and is on-screen somewhere....
            //    Now  add the candidate if its scale is smaller than the reference scale, or is not excessively underzoomed.

            if( ( ChartData->GetDBChartScale( i ) >= reference_scale ) || ( zoom_factor > .2 ) ) {
                bool b_add = true;

                const ChartTableEntry &cte = ChartData->GetChartTableEntry( i );
                wxRegion chart_region = GetChartQuiltRegion( cte, vp_local );

                //    Special case for S57 ENC
                //    Add the chart only if the chart's fractional area exceeds n%
                if( CHART_TYPE_S57 == reference_type ) {
                    //Get the fractional area of this chart
                    double chart_fractional_area = 0.;
                    double quilt_area = vp_local.pix_width * vp_local.pix_height;
                    if( !chart_region.Empty() ) {
                        wxRect rect_ch = chart_region.GetBox();
                        chart_fractional_area = ( rect_ch.GetWidth() * rect_ch.GetHeight() )
                                                / quilt_area;
                    } else
                        b_add = false;  // this chart has no overlap on screen
                    // probably because it has a concave outline
                    // i.e the bboxes overlap, but the actual coverage intersect is null.

                    if( chart_fractional_area < .20 ) {
                        b_add = false;
                    }
                }

                if( ref_db_index == i)
                    b_add = true;

                if( b_add ) {
                    // Check to see if this chart is already in the stack array
                    // by virtue of being under the Viewport center point....
                    bool b_exists = false;
                    for( unsigned int ir = 0; ir < m_extended_stack_array.GetCount(); ir++ ) {
                        if( i == m_extended_stack_array.Item( ir ) ) {
                            b_exists = true;
                            break;
                        }
                    }

                    if( !b_exists ) {
                        //      Check to be sure that this chart has not already been added
                        //    i.e. charts that have exactly the same file name and mod time
                        //    These charts can be in the database due to having the exact same chart in different directories,
                        //    as may be desired for some grouping schemes
                        bool b_noadd = false;
                        ChartTableEntry *pn = ChartData->GetpChartTableEntry( i );
                        for( unsigned int id = 0; id < m_extended_stack_array.GetCount() ; id++ ) {
                            if( m_extended_stack_array.Item( id ) != -1 ) {
                                ChartTableEntry *pm = ChartData->GetpChartTableEntry( m_extended_stack_array.Item( id ) );
                                if( pm->GetFileTime() && pn->GetFileTime()) {
                                    if( pm->GetFileTime() == pn->GetFileTime() ) {           // simple test
                                        if( pn->GetpFileName()->IsSameAs( *( pm->GetpFileName() ) ) )
                                            b_noadd = true;
                                    }
                                }
                            }
                        }

                        if(!b_noadd) {
                            m_extended_stack_array.Add( i );

                            QuiltCandidate *qcnew = new QuiltCandidate;
                            qcnew->dbIndex = i;
                            qcnew->ChartScale = ChartData->GetDBChartScale( i );
                            qcnew->quilt_region = chart_region;

                            m_pcandidate_array->Add( qcnew );               // auto-sorted on scale

                            b_need_resort = true;
                        }
                    }
                }
            }
        }               // for all charts

        //    Check to be sure that at least one chart was added that is larger scale than reference scale
        if( -1 != sure_index ) {
            // check to see if it is already in
            bool sure_exists = false;
            for( unsigned int ir = 0; ir < m_extended_stack_array.GetCount(); ir++ ) {
                if( sure_index == m_extended_stack_array.Item(ir) ) {
                    sure_exists = true;
                    break;
                }
            }

            //    If not already added, do so now
            if( !sure_exists ) {
                m_extended_stack_array.Add( sure_index );
                QuiltCandidate *qcnew = new QuiltCandidate;
                qcnew->dbIndex = sure_index;
                qcnew->ChartScale = ChartData->GetDBChartScale( sure_index );
                const ChartTableEntry &cte = ChartData->GetChartTableEntry( sure_index );
                qcnew->quilt_region = GetChartQuiltRegion( cte, vp_local );
                m_pcandidate_array->Add( qcnew );               // auto-sorted on scale

                b_need_resort = true;
            }
        }
    }   // fullscreen

    // Re sort the extended stack array on scale
    if( b_need_resort && m_extended_stack_array.GetCount() > 1 ) {
        int swap = 1;
        int ti;
        while( swap == 1 ) {
            swap = 0;
            for( unsigned int is = 0; is < m_extended_stack_array.GetCount() - 1; is++ ) {
                const ChartTableEntry &m = ChartData->GetChartTableEntry(
                                               m_extended_stack_array.Item( is ) );
                const ChartTableEntry &n = ChartData->GetChartTableEntry(
                                               m_extended_stack_array.Item( is + 1 ) );

                if( n.GetScale() < m.GetScale() ) {
                    ti = m_extended_stack_array.Item( is );
                    m_extended_stack_array.RemoveAt( is );
                    m_extended_stack_array.Insert( ti, is + 1 );
                    swap = 1;
                }
            }
        }
    }
    return true;
}


bool Quilt::Compose( const ViewPort &vp_in )
{
    if( !ChartData ) return false;
    if( m_bbusy ) return false;

    ChartData->UnLockCache();

    ViewPort vp_local = vp_in;                   // need a non-const copy

    //    Get Reference Chart parameters
    if( m_refchart_dbIndex >= 0 ) {
        const ChartTableEntry &cte_ref = ChartData->GetChartTableEntry( m_refchart_dbIndex );
        m_reference_scale = cte_ref.GetScale();
        m_reference_type = cte_ref.GetChartType();
        m_quilt_proj = ChartData->GetDBChartProj( m_refchart_dbIndex );
        m_reference_family = cte_ref.GetChartFamily();
    }

    //    Set up the vieport projection type
    vp_local.SetProjectionType( m_quilt_proj );

    bool bfull = vp_in.b_FullScreenQuilt;
    BuildExtendedChartStackAndCandidateArray(bfull, m_refchart_dbIndex, vp_local);

    //    It is possible that the reference chart is not really part of the visible quilt
    //    This can happen when the reference chart is panned
    //    off-screen in full screen quilt mode
    //    If this situation occurs, we need to immediately select a new reference chart
    //    And rebuild the Candidate Array
    //
    //    A special case occurs with cm93 composite chart set as the reference chart:
    //    It is not at this point a candidate, so won't be found by the search
    //    This case is indicated if the candidate count is zero.
    //    If so, do not invalidate the ref chart
    bool bf = false;
    for( unsigned int i = 0; i < m_pcandidate_array->GetCount(); i++ ) {
        QuiltCandidate *qc = m_pcandidate_array->Item( i );
        if( qc->dbIndex == m_refchart_dbIndex ) {
            bf = true;
            break;
        }
    }

    if( !bf && m_pcandidate_array->GetCount() ) {
        m_refchart_dbIndex = GetNewRefChart();
        BuildExtendedChartStackAndCandidateArray(bfull, m_refchart_dbIndex, vp_local);
    }


    //    Using Region logic, and starting from the largest scale chart
    //    figuratively "draw" charts until the ViewPort window is completely quilted over
    //    Add only those charts whose scale is smaller than the "reference scale"
    wxRegion vp_region( vp_local.rv_rect );
    unsigned int ir;

    //    As ChartdB data is always in rectilinear space, region calculations need to be done with no VP rotation
    double saved_vp_rotation = vp_local.rotation;                      // save a copy
    vp_local.SetRotationAngle( 0. );

    //    "Draw" the reference chart first, since it is special in that it controls the fine vpscale setting
    QuiltCandidate *pqc_ref = NULL;
    for( ir = 0; ir < m_pcandidate_array->GetCount(); ir++ )       // find ref chart entry
    {
        QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
        if( pqc->dbIndex == m_refchart_dbIndex ) {
            pqc_ref = pqc;
            break;
        }
    }

    if( pqc_ref ) {
        const ChartTableEntry &cte_ref = ChartData->GetChartTableEntry( m_refchart_dbIndex );

        wxRegion vpu_region( vp_local.rv_rect );

        wxRegion chart_region = pqc_ref->quilt_region; //GetChartQuiltRegion( cte_ref, vp_local );

        if( !chart_region.Empty() ) vpu_region.Intersect( chart_region );

        if( vpu_region.IsEmpty() ) pqc_ref->b_include = false;   // skip this chart, no true overlap
        else {
            pqc_ref->b_include = true;
            vp_region.Subtract( chart_region );          // adding this chart
        }
    }

    //    Now the rest of the candidates
    //    We always walk the entire list for s57 quilts, to pick up eclipsed overlays
    if( !vp_region.IsEmpty() || ( CHART_TYPE_S57 == m_reference_type ) ) {
        for( ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
            QuiltCandidate *pqc = m_pcandidate_array->Item( ir );

            if( pqc->dbIndex == m_refchart_dbIndex ) continue;               // already did this one

            const ChartTableEntry &cte = ChartData->GetChartTableEntry( pqc->dbIndex );


            if( cte.GetScale() >= m_reference_scale ) {
                //  If this chart appears in the no-show array, then simply include it, but
                //  don't subtract its region when determining the smaller scale charts to include.....
                bool b_in_noshow = false;
                for( unsigned int ins = 0; ins < g_quilt_noshow_index_array.GetCount(); ins++ ) {
                    if( g_quilt_noshow_index_array.Item( ins ) == pqc->dbIndex ) // chart is in the noshow list
                    {
                        b_in_noshow = true;
                        break;
                    }
                }

                if( !b_in_noshow ) {
                    //    Check intersection
                    wxRegion vpu_region( vp_local.rv_rect );

                    wxRegion chart_region = pqc->quilt_region;
                    if( !chart_region.Empty() )
                        vpu_region.Intersect( chart_region );

                    if( vpu_region.IsEmpty() )
                        pqc->b_include = false; // skip this chart, no true overlap
                    else {
                        pqc->b_include = true;
                        vp_region.Subtract( chart_region );          // adding this chart
                    }
                } else {
                    pqc->b_include = true;
                }

            } else {
                pqc->b_include = false;                       // skip this chart, scale is too large
            }

            /// Don't break early if the quilt is S57 ENC
            /// This will allow the overlay cells found in Euro(Austrian) IENC to be included
            if( CHART_TYPE_S57 != m_reference_type ) {
                if( vp_region.IsEmpty() )                   // normal stop condition, quilt is full
                    break;
            }
        }
    }

    //    Walk the candidate list again, marking "eclipsed" charts
    //    which at this point are the ones with b_include == false .AND. whose scale is strictly smaller than the ref scale
    //    Also, maintain the member list of same

    m_eclipsed_stack_array.Clear();

    for( ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
        QuiltCandidate *pqc = m_pcandidate_array->Item( ir );

        if( !pqc->b_include ) {
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( pqc->dbIndex );
            if( cte.GetScale() >= m_reference_scale ) {
                m_eclipsed_stack_array.Add( pqc->dbIndex );
                pqc->b_eclipsed = true;
            }
        }
    }

    //    Potentially add cm93 to the candidate array if the region is not yet fully covered
    if( ( m_quilt_proj == PROJECTION_MERCATOR ) && !vp_region.IsEmpty() ) {
        //    Check the remaining unpainted region.
        //    It may contain very small "slivers" of empty space, due to mixing of very small scale charts
        //    with the quilt.  If this is the case, do not waste time loading cm93....

        bool b_must_add_cm93 = false;
        wxRegionIterator updd( vp_region );
        while( updd ) {
            wxRect rect = updd.GetRect();
            if( ( rect.width > 2 ) && ( rect.height > 2 ) ) {
                b_must_add_cm93 = true;
                break;
            }
            updd++;
        }

        if( b_must_add_cm93 ) {
            for( int ics = 0; ics < pCurrentStack->nEntry; ics++ ) {
                int i = pCurrentStack->GetDBIndex( ics );
                if( CHART_TYPE_CM93COMP == ChartData->GetDBChartType( i ) ) {
                    QuiltCandidate *qcnew = new QuiltCandidate;
                    qcnew->dbIndex = i;
                    qcnew->ChartScale = ChartData->GetDBChartScale( i );

                    const ChartTableEntry &cte = ChartData->GetChartTableEntry( i );
                    wxRegion chart_region = GetChartQuiltRegion( cte, vp_local );
                    qcnew->quilt_region = chart_region;

                    m_pcandidate_array->Add( qcnew );
                }
            }
        }
    }

    //    Check the list...if no charts are visible due to all being smaller than reference_scale,
    //    then make sure the smallest scale chart which has any true region intersection is visible anyway
    //    Also enable any other charts which are the same scale as the first one added
    bool b_vis = false;
    for( unsigned int i = 0; i < m_pcandidate_array->GetCount(); i++ ) {
        QuiltCandidate *pqc = m_pcandidate_array->Item( i );
        if( pqc->b_include ) {
            b_vis = true;
            break;
        }
    }

    if( !b_vis && m_pcandidate_array->GetCount() ) {
        int add_scale = 0;

        for( int i = m_pcandidate_array->GetCount() - 1; i >= 0; i-- ) {
            QuiltCandidate *pqc = m_pcandidate_array->Item( i );
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( pqc->dbIndex );

            //    Don't add cm93 yet, it is always covering the quilt...
            if( cte.GetChartType() == CHART_TYPE_CM93COMP ) continue;

            //    Check intersection
            wxRegion vpck_region( vp_local.rv_rect );

            wxRegion chart_region = pqc->quilt_region; //GetChartQuiltRegion( cte, vp_local );
            if( !chart_region.Empty() ) vpck_region.Intersect( chart_region );

            if( !vpck_region.IsEmpty() ) {
                if( add_scale ) {
                    if( add_scale == cte.GetScale() ) pqc->b_include = true;
                    ;
                } else {
                    pqc->b_include = true;
                    add_scale = cte.GetScale();
                }
            }
        }
    }


    //    Finally, build a list of "patches" for the quilt.
    //    Smallest scale first, as this will be the natural drawing order

    m_PatchList.DeleteContents( true );
    m_PatchList.Clear();

    if( m_pcandidate_array->GetCount() ) {
        for( int i = m_pcandidate_array->GetCount() - 1; i >= 0; i-- ) {
            QuiltCandidate *pqc = m_pcandidate_array->Item( i );

            //    cm93 add has been deferred until here
            //    so that it would not displace possible raster or ENCs of larger scale
            const ChartTableEntry &m = ChartData->GetChartTableEntry( pqc->dbIndex );

            if( m.GetChartType() == CHART_TYPE_CM93COMP ) pqc->b_include = true; // force acceptance of this chart in quilt
            // would not be in candidate array if not elected

            if( pqc->b_include ) {
                QuiltPatch *pqp = new QuiltPatch;
                pqp->dbIndex = pqc->dbIndex;
                pqp->ProjType = m.GetChartProjectionType();
                pqp->quilt_region = pqc->quilt_region;
                pqp->b_Valid = true;

                m_PatchList.Append( pqp );
            }
        }
    }
    //    From here on out, the PatchList is usable...

#ifdef QUILT_TYPE_1
    //    Establish the quilt projection type
    m_quilt_proj = PROJECTION_MERCATOR;// default
    ChartBase *ppc = GetLargestScaleChart();
    if(ppc)
        m_quilt_proj = ppc->GetChartProjectionType();
#endif

    //    Walk the PatchList, marking any entries whose projection does not match the determined quilt projection
    for( unsigned int i = 0; i < m_PatchList.GetCount(); i++ ) {
        wxPatchListNode *pcinode = m_PatchList.Item( i );
        QuiltPatch *piqp = pcinode->GetData();
        if( ( piqp->ProjType != m_quilt_proj ) && ( piqp->ProjType != PROJECTION_UNKNOWN ) ) piqp->b_Valid =
                false;
    }

    //    Walk the PatchList, marking any entries which appear in the noshow array
    for( unsigned int i = 0; i < m_PatchList.GetCount(); i++ ) {
        wxPatchListNode *pcinode = m_PatchList.Item( i );
        QuiltPatch *piqp = pcinode->GetData();
        for( unsigned int ins = 0; ins < g_quilt_noshow_index_array.GetCount(); ins++ ) {
            if( g_quilt_noshow_index_array.Item( ins ) == piqp->dbIndex ) // chart is in the noshow list
            {
                piqp->b_Valid = false;
                break;
            }
        }
    }

    //    Generate the final render regions for the patches, one by one, smallest to largest scale
    wxRegion unrendered_region( vp_local.rv_rect );

    m_covered_region.Clear();

    for( unsigned int i = 0; i < m_PatchList.GetCount(); i++ ) {
        wxPatchListNode *pcinode = m_PatchList.Item( i );
        QuiltPatch *piqp = pcinode->GetData();

        if( !piqp->b_Valid )                         // skip invalid entries
            continue;

        const ChartTableEntry &ctei = ChartData->GetChartTableEntry( piqp->dbIndex );

        wxRegion vpr_region = unrendered_region;

        //    Start with the chart's full region coverage.
        vpr_region = piqp->quilt_region;


#if 1       // This clause went away with full-screen quilting
        // ...and came back with OpenGL....

        //fetch and subtract regions for all larger scale charts
        for( unsigned int k = i + 1; k < m_PatchList.GetCount(); k++ ) {
            wxPatchListNode *pnode = m_PatchList.Item( k );
            QuiltPatch *pqp = pnode->GetData();

            if( !pqp->b_Valid )                         // skip invalid entries
                continue;

/// In S57ENC quilts, do not subtract larger scale regions from smaller.
/// This does two things:
/// 1. This allows co-incident or overlayed chart regions to both be included
///    thus covering the case found in layered Euro(Austrian) IENC cells
/// 2. This make quilted S57 ENC renders much faster, as the larger scale charts are not rendered
///     until the canvas is zoomed sufficiently.

/// Above logic does not apply to cm93 composites
            if( ( CHART_TYPE_S57 != ctei.GetChartType() )/* && (CHART_TYPE_CM93COMP != ctei.GetChartType())*/) {

                if( !vpr_region.Empty() ) {
                    const ChartTableEntry &cte = ChartData->GetChartTableEntry( pqp->dbIndex );
                    wxRegion larger_scale_chart_region = pqp->quilt_region; //GetChartQuiltRegion( cte, vp_local );

                    vpr_region.Subtract( larger_scale_chart_region );
                }
            }

        }
#endif

        //    Whatever is left in the vpr region and has not been yet rendered must belong to the current target chart

        wxPatchListNode *pinode = m_PatchList.Item( i );
        QuiltPatch *pqpi = pinode->GetData();
        pqpi->ActiveRegion = vpr_region;

        //    Move the active region so that upper left is 0,0 in final render region
        pqpi->ActiveRegion.Offset( -vp_local.rv_rect.x, -vp_local.rv_rect.y );

        //    Could happen that a larger scale chart covers completely a smaller scale chart
        if( pqpi->ActiveRegion.IsEmpty() )
            pqpi->b_eclipsed = true;

        //    Update the next pass full region to remove the region just allocated
        if( !vpr_region.Empty() )
            unrendered_region.Subtract( vpr_region );

        //    Maintain the present full quilt coverage region
        if( !pqpi->ActiveRegion.IsEmpty() )
            m_covered_region.Union( pqpi->ActiveRegion );
    }

    //    Restore temporary VP Rotation
    vp_local.SetRotationAngle( saved_vp_rotation );

    //    Walk the list again, removing any entries marked as eclipsed....
    unsigned int il = 0;
    while( il < m_PatchList.GetCount() ) {
        wxPatchListNode *pcinode = m_PatchList.Item( il );
        QuiltPatch *piqp = pcinode->GetData();
        if( piqp->b_eclipsed ) {
            //    Make sure that this chart appears in the eclipsed list...
            //    This can happen when....
            bool b_noadd = false;
            for( unsigned int ir = 0; ir < m_eclipsed_stack_array.GetCount(); ir++ ) {
                if( piqp->dbIndex == m_eclipsed_stack_array.Item( ir ) ) {
                    b_noadd = true;
                    break;
                }
            }
            if( !b_noadd ) m_eclipsed_stack_array.Add( piqp->dbIndex );

            m_PatchList.DeleteNode( pcinode );
            il = 0;           // restart the list walk
        }

        else
            il++;
    }

    //    Mark the quilt to indicate need for background clear if the region is not fully covered
    m_bneed_clear = !unrendered_region.IsEmpty();
    m_back_region = unrendered_region;

    //    Finally, iterate thru the quilt and preload all of the required charts.
    //    For dynamic S57 SENC creation, this is where SENC creation happens first.....
    for( ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
        QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
        if( ( pqc->b_include ) && ( !pqc->b_eclipsed ) ) ChartData->OpenChartFromDB( pqc->dbIndex,
                    FULL_INIT );
    }

    //    Build and maintain the array of indexes in this quilt

    m_last_index_array = m_index_array;       //save the last one for delta checks

    m_index_array.Clear();

    //    The index array is to be built in reverse, largest scale first
    unsigned int kl = m_PatchList.GetCount();
    for( unsigned int k = 0; k < kl; k++ ) {
        wxPatchListNode *cnode = m_PatchList.Item( ( kl - k ) - 1 );
        m_index_array.Add( cnode->GetData()->dbIndex );
        cnode = cnode->GetNext();
    }

    //    Walk the patch list again, checking the depth units
    //    If they are all the same, then the value is usable

    m_quilt_depth_unit = _T("");
    ChartBase *pc = ChartData->OpenChartFromDB( m_refchart_dbIndex, FULL_INIT );
    if( pc ) {
        m_quilt_depth_unit = pc->GetDepthUnits();

#ifdef USE_S57
        if( pc->GetChartFamily() == CHART_FAMILY_VECTOR ) {
            int units = ps52plib->m_nDepthUnitDisplay;
            switch( units ) {
            case 0:
                m_quilt_depth_unit = _T("Feet");
                break;
            case 1:
                m_quilt_depth_unit = _T("Meters");
                break;
            case 2:
                m_quilt_depth_unit = _T("Fathoms");
                break;
            }
        }
#endif
    }

    for( unsigned int k = 0; k < m_PatchList.GetCount(); k++ ) {
        wxPatchListNode *pnode = m_PatchList.Item( k );
        QuiltPatch *pqp = pnode->GetData();

        if( !pqp->b_Valid )                         // skip invalid entries
            continue;

        ChartBase *pc = ChartData->OpenChartFromDB( pqp->dbIndex, FULL_INIT );
        if( pc ) {
            wxString du = pc->GetDepthUnits();
#ifdef USE_S57
            if( pc->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                int units = ps52plib->m_nDepthUnitDisplay;
                switch( units ) {
                case 0:
                    du = _T("Feet");
                    break;
                case 1:
                    du = _T("Meters");
                    break;
                case 2:
                    du = _T("Fathoms");
                    break;
                }
            }
#endif
            wxString dul = du.Lower();
            wxString ml = m_quilt_depth_unit.Lower();

            if( dul != ml ) {
                //    Try all the odd cases
                if( dul.StartsWith( _T("meters") ) && ml.StartsWith( _T("meters") ) ) continue;
                else if( dul.StartsWith( _T("metres") ) && ml.StartsWith( _T("metres") ) ) continue;
                else if( dul.StartsWith( _T("fathoms") ) && ml.StartsWith( _T("fathoms") ) ) continue;
                else if( dul.StartsWith( _T("met") ) && ml.StartsWith( _T("met") ) ) continue;

                //    They really are different
                m_quilt_depth_unit = _T("");
                break;
            }
        }
    }

    //    And try to prove that all required charts are in the cache
    //    If one is missing, try to load it
    //    If still missing, remove its patch from the quilt
    //    This will probably leave a "black hole" in the quilt...
    for( unsigned int k = 0; k < m_PatchList.GetCount(); k++ ) {
        wxPatchListNode *pnode = m_PatchList.Item( k );
        QuiltPatch *pqp = pnode->GetData();

        if( pqp->b_Valid ) {
            if( !ChartData->IsChartInCache( pqp->dbIndex ) ) {
                wxLogMessage( _T("   Quilt Compose cache miss...") );
                ChartData->OpenChartFromDB( pqp->dbIndex, FULL_INIT );
                if( !ChartData->IsChartInCache( pqp->dbIndex ) ) {
                    wxLogMessage( _T("    Oops, removing from quilt...") );
                    pqp->b_Valid = false;
                }
            }
        }
    }

    //    Make sure the reference chart is in the cache
    if( !ChartData->IsChartInCache( m_refchart_dbIndex ) ) ChartData->OpenChartFromDB(
            m_refchart_dbIndex, FULL_INIT );

    //    Walk the patch list again, checking the error factor
    //    Also, directly mark the patch to indicate if it should be treated as an overlay
    //    as seen in Austrian Inland series

    m_bquilt_has_overlays = false;
    m_max_error_factor = 0.;
    for( unsigned int k = 0; k < m_PatchList.GetCount(); k++ ) {
        wxPatchListNode *pnode = m_PatchList.Item( k );
        QuiltPatch *pqp = pnode->GetData();

        if( !pqp->b_Valid )                         // skip invalid entries
            continue;

        ChartBase *pc = ChartData->OpenChartFromDB( pqp->dbIndex, FULL_INIT );
        if( pc ) {
            m_max_error_factor = wxMax(m_max_error_factor, pc->GetChart_Error_Factor());

            if( pc->GetChartType() == CHART_TYPE_S57 ) {
                s57chart *ps57 = dynamic_cast<s57chart *>( pc );
                pqp->b_overlay = ( ps57->GetUsageChar() == 'L' || ps57->GetUsageChar() == 'A' );
                if( pqp->b_overlay )
                    m_bquilt_has_overlays = true;
            }
        }
    }

    m_bcomposed = true;

    m_vp_quilt = vp_in;                 // save the corresponding ViewPort locally

    ChartData->LockCache();

    //  Create and store a hash value representing the contents of the m_extended_stack_array
    unsigned long xa_hash = 5381;
    for(unsigned int im=0 ; im < m_extended_stack_array.GetCount() ; im++) {
        int dbindex = m_extended_stack_array.Item(im);
        xa_hash = ((xa_hash << 5) + xa_hash) + dbindex; /* hash * 33 + dbindex */
    }

    m_xa_hash = xa_hash;
    return true;
}


//      Compute and update the member quilt render region, considering all scale factors, group exclusions, etc.
void Quilt::ComputeRenderRegion( ViewPort &vp, wxRegion &chart_region )
{
    if( !m_bcomposed ) return;

    wxRegion rendered_region;

    if( GetnCharts() && !m_bbusy ) {

        wxRegion screen_region = chart_region;

        //  Walk the quilt, considering each chart from smallest scale to largest

        ChartBase *chart = GetFirstChart();

        while( chart ) {
            bool okToRender = cc1->IsChartLargeEnoughToRender( chart, vp );

            if( chart->GetChartProjectionType() != PROJECTION_MERCATOR && vp.b_MercatorProjectionOverride )
                okToRender = false;

            if( ! okToRender ) {
                chart = GetNextChart();
                continue;
            }
            QuiltPatch *pqp = GetCurrentPatch();
            if( pqp->b_Valid  ) {
                wxRegion get_region = pqp->ActiveRegion;

                if( !chart_region.IsEmpty() ) {

                    get_region.Intersect( chart_region );

                    if( !get_region.IsEmpty() ) {
                        rendered_region.Union(get_region);
                    }
                }
           }

            chart = GetNextChart();
        }
    }
    //  Record the region actually rendered
    m_rendered_region = rendered_region;
}



int g_render;

bool Quilt::RenderQuiltRegionViewOnDC( wxMemoryDC &dc, ViewPort &vp, wxRegion &chart_region )
{

#ifdef ocpnUSE_DIBSECTION
    ocpnMemDC tmp_dc;
#else
    wxMemoryDC tmp_dc;
#endif

    if( !m_bcomposed ) return false;

    wxRegion rendered_region;

//    double scale_onscreen = vp.view_scale_ppm;
//    double max_allowed_scale = 4. * cc1->GetAbsoluteMinScalePpm();

    if( GetnCharts() && !m_bbusy ) {

        wxRegion screen_region = chart_region;

        //  Walk the quilt, drawing each chart from smallest scale to largest
        //  Render the quilt's charts onto a temp dc
        //  and blit the active region rectangles to to target dc, one-by-one

        ChartBase *chart = GetFirstChart();
        int chartsDrawn = 0;

        if( !chart_region.IsEmpty() ) {
            while( chart ) {
                bool okToRender = cc1->IsChartLargeEnoughToRender( chart, vp );

                if( chart->GetChartProjectionType() != PROJECTION_MERCATOR && vp.b_MercatorProjectionOverride )
                    okToRender = false;

                if( ! okToRender ) {
                    chart = GetNextChart();
                    continue;
                }
                QuiltPatch *pqp = GetCurrentPatch();
                if( pqp->b_Valid  ) {
                    bool b_chart_rendered = false;
                    wxRegion get_region = pqp->ActiveRegion;

                    get_region.Intersect( chart_region );

                    if( !get_region.IsEmpty() ) {

                        if( !pqp->b_overlay ) {
                            b_chart_rendered = chart->RenderRegionViewOnDC( tmp_dc, vp, get_region );
                            if( chart->GetChartType() != CHART_TYPE_CM93COMP )
                                b_chart_rendered = true;
                            screen_region.Subtract( get_region );
                        }
                    }

                    wxRegionIterator upd( get_region );
                    while( upd ) {
                        wxRect rect = upd.GetRect();
                        dc.Blit( rect.x, rect.y, rect.width, rect.height, &tmp_dc, rect.x, rect.y,
                                wxCOPY, true );
                        upd++;
                    }

                    tmp_dc.SelectObject( wxNullBitmap );

                    if(b_chart_rendered)
                        rendered_region.Union(get_region);
                }

                chartsDrawn++;
                chart = GetNextChart();
            }
        }

        if( ! chartsDrawn ) cc1->GetVP().SetProjectionType( PROJECTION_MERCATOR );

        //    Render any Overlay patches for s57 charts(cells)
        if( m_bquilt_has_overlays && !chart_region.IsEmpty() ) {
            chart = GetFirstChart();
            while( chart ) {
                QuiltPatch *pqp = GetCurrentPatch();
                if( pqp->b_Valid ) {
                    if( pqp->b_overlay ) {
                        wxRegion get_region = pqp->ActiveRegion;
                        get_region.Intersect( chart_region );

                        if( !get_region.IsEmpty() ) {
                            s57chart *Chs57 = dynamic_cast<s57chart*>( chart );
                            Chs57->RenderOverlayRegionViewOnDC( tmp_dc, vp, get_region );

                            wxRegionIterator upd( get_region );
                            while( upd ) {
                                wxRect rect = upd.GetRect();
                                dc.Blit( rect.x, rect.y, rect.width, rect.height, &tmp_dc, rect.x,
                                      rect.y, wxCOPY, true );
                                upd++;
                            }
                            tmp_dc.SelectObject( wxNullBitmap );
                        }
                     }
                }

                chart = GetNextChart();
            }
        }

        //    Any part of the chart region that was not rendered in the loop needs to be cleared
        wxRegionIterator clrit( screen_region );
        while( clrit ) {
            wxRect rect = clrit.GetRect();
#ifdef __WXOSX__
            dc.SetPen(*wxBLACK_PEN);
            dc.SetBrush(*wxBLACK_BRUSH);
            dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);
#else
            dc.Blit( rect.x, rect.y, rect.width, rect.height, &dc, rect.x, rect.y, wxCLEAR );
#endif
            clrit++;
        }

        //    Highlighting....
        if( m_nHiLiteIndex >= 0 ) {
            wxRegion hiregion = GetHiliteRegion( vp );

            wxRect box = hiregion.GetBox();

            if( !box.IsEmpty() ) {
                //    Is scratch member bitmap OK?
                if( m_pBM ) {
                    if( ( m_pBM->GetWidth() != vp.rv_rect.width )
                            || ( m_pBM->GetHeight() != vp.rv_rect.height ) ) {
                        delete m_pBM;
                        m_pBM = NULL;
                    }
                }

                if( NULL == m_pBM ) m_pBM = new wxBitmap( vp.rv_rect.width, vp.rv_rect.height );

                //    Copy the entire quilt to my scratch bm
                wxMemoryDC q_dc;
                q_dc.SelectObject( *m_pBM );
                q_dc.Blit( 0, 0, vp.rv_rect.width, vp.rv_rect.height, &dc, 0, 0 );
                q_dc.SelectObject( wxNullBitmap );

                //    Create a "mask" bitmap from the chart's region
                //    WxGTK has an error in this method....Creates a color bitmap, not usable for mask creation
                //    So, I clone with correction
                wxBitmap hl_mask_bm( vp.rv_rect.width, vp.rv_rect.height, 1 );
                wxMemoryDC mdc;
                mdc.SelectObject( hl_mask_bm );
                mdc.SetBackground( *wxBLACK_BRUSH );
                mdc.Clear();
                mdc.SetClippingRegion( box );
                mdc.SetBackground( *wxWHITE_BRUSH );
                mdc.Clear();
                mdc.SelectObject( wxNullBitmap );

                if( hl_mask_bm.IsOk() ) {
                    wxMask *phl_mask = new wxMask( hl_mask_bm );
                    m_pBM->SetMask( phl_mask );
                    q_dc.SelectObject( *m_pBM );

                    // Create another mask, dc and bitmap for red-out
                    wxBitmap rbm( vp.rv_rect.width, vp.rv_rect.height );
                    wxMask *pr_mask = new wxMask( hl_mask_bm );
                    wxMemoryDC rdc;
                    rbm.SetMask( pr_mask );
                    rdc.SelectObject( rbm );
                    unsigned char hlcolor = 255;
                    switch( global_color_scheme ) {
                    case GLOBAL_COLOR_SCHEME_DAY:
                        hlcolor = 255;
                        break;
                    case GLOBAL_COLOR_SCHEME_DUSK:
                        hlcolor = 64;
                        break;
                    case GLOBAL_COLOR_SCHEME_NIGHT:
                        hlcolor = 16;
                        break;
                    default:
                        hlcolor = 255;
                        break;
                    }

                    rdc.SetBackground( wxBrush( wxColour( hlcolor, 0, 0 ) ) );
                    rdc.Clear();

                    wxRegionIterator upd ( hiregion );
                    while ( upd )
                    {
                        wxRect rect = upd.GetRect();
                        rdc.Blit( rect.x, rect.y, rect.width, rect.height, &q_dc, rect.x, rect.y, wxOR,
                                  true );
                        upd ++ ;
                    }

                    wxRegionIterator updq ( hiregion );
                    while ( updq )
                    {
                        wxRect rect = updq.GetRect();
                        q_dc.Blit( rect.x, rect.y, rect.width, rect.height, &rdc, rect.x, rect.y, wxCOPY,
                                   true );
                        updq ++ ;
                    }


                    q_dc.SelectObject( wxNullBitmap );
                    m_pBM->SetMask( NULL );

                    //    Select the scratch BM as the return dc contents
                    dc.SelectObject( *m_pBM );

                    //    Clear the rdc
                    rdc.SelectObject( wxNullBitmap );
                }
            }  // box not empty
        }     // m_nHiLiteIndex

        if( !dc.IsOk() )          // some error, probably bad charts, to be disabled on next compose
        {
            SubstituteClearDC( dc, vp );
        }

    } else {             // no charts yet, or busy....
        SubstituteClearDC( dc, vp );
    }

    //  Record the region actually rendered
    m_rendered_region = rendered_region;

    m_vp_rendered = vp;
    return true;
}

void Quilt::SubstituteClearDC( wxMemoryDC &dc, ViewPort &vp )
{
    if( m_pBM ) {
        if( ( m_pBM->GetWidth() != vp.rv_rect.width )
                || ( m_pBM->GetHeight() != vp.rv_rect.height ) ) {
            delete m_pBM;
            m_pBM = NULL;
        }
    }

    if( NULL == m_pBM ) {
        m_pBM = new wxBitmap( vp.rv_rect.width, vp.rv_rect.height );
    }

    dc.SelectObject( wxNullBitmap );
    dc.SelectObject( *m_pBM );
    dc.SetBackground( *wxBLACK_BRUSH );
    dc.Clear();
    m_covered_region.Clear();

}

//------------------------------------------------------------------------------
//    ViewPort Implementation
//------------------------------------------------------------------------------
ViewPort::ViewPort()
{
    bValid = false;
    skew = 0.;
    view_scale_ppm = 1;
    rotation = 0.;
    b_quilt = false;
    pix_height = pix_width = 0;
    b_MercatorProjectionOverride = false;
}

wxPoint ViewPort::GetPixFromLL( double lat, double lon ) const
{
    double easting, northing;
    double xlon = lon;

    /*  Make sure lon and lon0 are same phase */
    if( xlon * clon < 0. ) {
        if( xlon < 0. ) xlon += 360.;
        else
            xlon -= 360.;
    }

    if( fabs( xlon - clon ) > 180. ) {
        if( xlon > clon ) xlon -= 360.;
        else
            xlon += 360.;
    }

    if( PROJECTION_TRANSVERSE_MERCATOR == m_projection_type ) {
        //    We calculate northings as referenced to the equator
        //    And eastings as though the projection point is midscreen.

        double tmeasting, tmnorthing;
        double tmceasting, tmcnorthing;
        toTM( clat, clon, 0., clon, &tmceasting, &tmcnorthing );
        toTM( lat, xlon, 0., clon, &tmeasting, &tmnorthing );

//            tmeasting -= tmceasting;
//            tmnorthing -= tmcnorthing;

        northing = tmnorthing - tmcnorthing;
        easting = tmeasting - tmceasting;
    } else if( PROJECTION_POLYCONIC == m_projection_type ) {

        //    We calculate northings as referenced to the equator
        //    And eastings as though the projection point is midscreen.
        double pceasting, pcnorthing;
        toPOLY( clat, clon, 0., clon, &pceasting, &pcnorthing );

        double peasting, pnorthing;
        toPOLY( lat, xlon, 0., clon, &peasting, &pnorthing );

        easting = peasting;
        northing = pnorthing - pcnorthing;
    }

    else
        toSM( lat, xlon, clat, clon, &easting, &northing );

    if( !wxFinite(easting) || !wxFinite(northing) ) return wxPoint( 0, 0 );

    double epix = easting * view_scale_ppm;
    double npix = northing * view_scale_ppm;
    double dxr = epix;
    double dyr = npix;

    //    Apply VP Rotation
    if( g_bCourseUp ) {
        dxr = epix * cos( rotation ) + npix * sin( rotation );
        dyr = npix * cos( rotation ) - epix * sin( rotation );
    }
    wxPoint r;
    //    We definitely need a round() function here
    r.x = (int) wxRound( ( pix_width / 2 ) + dxr );
    r.y = (int) wxRound( ( pix_height / 2 ) - dyr );

    return r;
}

wxPoint2DDouble ViewPort::GetDoublePixFromLL( double lat, double lon )
{
    double easting, northing;
    double xlon = lon;

    /*  Make sure lon and lon0 are same phase */
    if( xlon * clon < 0. ) {
        if( xlon < 0. ) xlon += 360.;
        else
            xlon -= 360.;
    }

    if( fabs( xlon - clon ) > 180. ) {
        if( xlon > clon ) xlon -= 360.;
        else
            xlon += 360.;
    }

    if( PROJECTION_TRANSVERSE_MERCATOR == m_projection_type ) {
        //    We calculate northings as referenced to the equator
        //    And eastings as though the projection point is midscreen.

        double tmeasting, tmnorthing;
        double tmceasting, tmcnorthing;
        toTM( clat, clon, 0., clon, &tmceasting, &tmcnorthing );
        toTM( lat, xlon, 0., clon, &tmeasting, &tmnorthing );

        northing = tmnorthing - tmcnorthing;
        easting = tmeasting - tmceasting;
    } else if( PROJECTION_POLYCONIC == m_projection_type ) {

        //    We calculate northings as referenced to the equator
        //    And eastings as though the projection point is midscreen.
        double pceasting, pcnorthing;
        toPOLY( clat, clon, 0., clon, &pceasting, &pcnorthing );

        double peasting, pnorthing;
        toPOLY( lat, xlon, 0., clon, &peasting, &pnorthing );

        easting = peasting;
        northing = pnorthing - pcnorthing;
    }

    else
        toSM( lat, xlon, clat, clon, &easting, &northing );

    if( !wxFinite(easting) || !wxFinite(northing) ) return wxPoint( 0, 0 );

    double epix = easting * view_scale_ppm;
    double npix = northing * view_scale_ppm;
    double dxr = epix;
    double dyr = npix;

    //    Apply VP Rotation
    if( g_bCourseUp ) {
        dxr = epix * cos( rotation ) + npix * sin( rotation );
        dyr = npix * cos( rotation ) - epix * sin( rotation );
    }

    wxPoint2DDouble r;
    //    We definitely need a round() function here
    r.m_x = ( ( pix_width / 2 ) + dxr );
    r.m_y = ( ( pix_height / 2 ) - dyr );

    return r;
}

void ViewPort::GetLLFromPix( const wxPoint &p, double *lat, double *lon )
{
    int dx = p.x - ( pix_width / 2 );
    int dy = ( pix_height / 2 ) - p.y;

    double xpr = dx;
    double ypr = dy;

    //    Apply VP Rotation
    if( g_bCourseUp ) {
        xpr = ( dx * cos( rotation ) ) - ( dy * sin( rotation ) );
        ypr = ( dy * cos( rotation ) ) + ( dx * sin( rotation ) );
    }
    double d_east = xpr / view_scale_ppm;
    double d_north = ypr / view_scale_ppm;

    double slat, slon;
    if( PROJECTION_TRANSVERSE_MERCATOR == m_projection_type ) {
        double tmceasting, tmcnorthing;
        toTM( clat, clon, 0., clon, &tmceasting, &tmcnorthing );

        fromTM( d_east, d_north + tmcnorthing, 0., clon, &slat, &slon );
    } else if( PROJECTION_POLYCONIC == m_projection_type ) {
        double polyeasting, polynorthing;
        toPOLY( clat, clon, 0., clon, &polyeasting, &polynorthing );

        fromPOLY( d_east, d_north + polynorthing, 0., clon, &slat, &slon );
    }

    //TODO  This could be fromSM_ECC to better match some Raster charts
    //      However, it seems that cm93 (and S57) prefer no eccentricity correction
    //      Think about it....
    else
        fromSM( d_east, d_north, clat, clon, &slat, &slon );

    *lat = slat;

    if( slon < -180. ) slon += 360.;
    else if( slon > 180. ) slon -= 360.;
    *lon = slon;
}

wxRegion ViewPort::GetVPRegionIntersect( const wxRegion &Region, size_t n, float *llpoints,
        int chart_native_scale, wxPoint *ppoints )
{
    //  Calculate the intersection between a given wxRegion (Region) and a polygon specified by lat/lon points.

    //    If the viewpoint is highly overzoomed wrt to chart native scale, the polygon region may be huge.
    //    This can be very expensive, and lead to crashes on some platforms (gtk in particular)
    //    So, look for this case and handle appropriately with respect to the given Region

    if( chart_scale < chart_native_scale / 10 ) {
        //    Make a positive definite vp
        ViewPort vp_positive = *this;
        while( vp_positive.vpBBox.GetMinX() < 0 ) {
            vp_positive.clon += 360.;
            wxPoint2DDouble t( 360., 0. );
            vp_positive.vpBBox.Translate( t );
        }

        //    Scan the points one-by-one, so that we can get min/max to make a bbox
        float *pfp = llpoints;
        float lon_max = -10000.;
        float lon_min = 10000.;
        float lat_max = -10000.;
        float lat_min = 10000.;

        for( unsigned int ip = 0; ip < n; ip++ ) {
            lon_max = wxMax(lon_max, pfp[1]);
            lon_min = wxMin(lon_min, pfp[1]);
            lat_max = wxMax(lat_max, pfp[0]);
            lat_min = wxMin(lat_min, pfp[0]);

            pfp += 2;
        }

        wxBoundingBox chart_box( lon_min, lat_min, lon_max, lat_max );

        //    Case:  vpBBox is completely outside the chart box, or vice versa
        //    Return an empty region
        if( _OUT == chart_box.Intersect( (wxBoundingBox&) vp_positive.vpBBox ) ) {
            if( _OUT == chart_box.Intersect( (wxBoundingBox&) vpBBox ) ) {
                // try again with the chart translated 360
                wxPoint2DDouble rtw( 360., 0. );
                wxBoundingBox trans_box = chart_box;
                trans_box.Translate( rtw );

                if( _OUT == trans_box.Intersect( (wxBoundingBox&) vp_positive.vpBBox ) ) {
                    if( _OUT == trans_box.Intersect( (wxBoundingBox&) vpBBox ) ) {
                        return wxRegion();
                    }
                }
            }
        }

        //    Case:  vpBBox is completely inside the chart box
        //      Note that this test is not perfect, and will fail for some charts.
        //      The chart coverage may be  essentially triangular, and the viewport box
        //      may be in the "cut off" segment of the chart_box, and not actually
        //      exhibit any true overlap.  Results will be reported incorrectly.
        //      How to fix: maybe scrub the chart points and see if it is likely that
        //      a region may be safely built and intersection tested.

        if( _IN == chart_box.Intersect( (wxBoundingBox&) vp_positive.vpBBox ) ) {
            return Region;
        }

        if(_IN == chart_box.Intersect((wxBoundingBox&)vpBBox))
        {
            return Region;
        }

        //    The ViewPort and the chart region overlap in some way....
        //    Create the intersection of the two bboxes
        //    Boxes must be same phase
        while( chart_box.GetMinX() < 0 ) {
            wxPoint2DDouble t( 360., 0. );
            chart_box.Translate( t );
        }

        double cb_minlon = wxMax(chart_box.GetMinX(), vp_positive.vpBBox.GetMinX());
        double cb_maxlon = wxMin(chart_box.GetMaxX(), vp_positive.vpBBox.GetMaxX());
        double cb_minlat = wxMax(chart_box.GetMinY(), vp_positive.vpBBox.GetMinY());
        double cb_maxlat = wxMin(chart_box.GetMaxY(), vp_positive.vpBBox.GetMaxY());

        if( cb_maxlon < cb_minlon ) cb_maxlon += 360.;

        wxPoint p1 = GetPixFromLL( cb_maxlat, cb_minlon );  // upper left
        wxPoint p2 = GetPixFromLL( cb_minlat, cb_maxlon );   // lower right

        wxRegion r( p1, p2 );
        r.Intersect( Region );
        return r;
    }

    //    More "normal" case

    wxPoint *pp;

    //    Use the passed point buffer if available
    if( ppoints == NULL ) pp = new wxPoint[n];
    else
        pp = ppoints;

    float *pfp = llpoints;

    for( unsigned int ip = 0; ip < n; ip++ ) {
        wxPoint p = GetPixFromLL( pfp[0], pfp[1] );
        pp[ip] = p;
        pfp += 2;
    }

#ifdef __WXGTK__
    sigaction(SIGSEGV, NULL, &sa_all_old);             // save existing action for this signal

    struct sigaction temp;
    sigaction(SIGSEGV, NULL, &temp);// inspect existing action for this signal

    temp.sa_handler = catch_signals;// point to my handler
    sigemptyset(&temp.sa_mask);// make the blocking set
    // empty, so that all
    // other signals will be
    // unblocked during my handler
    temp.sa_flags = 0;
    sigaction(SIGSEGV, &temp, NULL);

    if(sigsetjmp(env, 1))//  Something in the below code block faulted....
    {
        sigaction(SIGSEGV, &sa_all_old, NULL);        // reset signal handler

        return Region;

    }

    else
    {
        wxRegion r = wxRegion(n, pp);
        if(NULL == ppoints)
            delete[] pp;

        sigaction(SIGSEGV, &sa_all_old, NULL);        // reset signal handler
        r.Intersect(Region);
        return r;
    }

#else
    wxRegion r = wxRegion( n, pp );

    if( NULL == ppoints ) delete[] pp;

    r.Intersect( Region );
    return r;

#endif
}

void ViewPort::SetBoxes( void )
{

    //  In the case where canvas rotation is applied, we need to define a larger "virtual" pixel window size to ensure that
    //  enough chart data is fatched and available to fill the rotated screen.
    rv_rect = wxRect( 0, 0, pix_width, pix_height );

    //  Specify the minimum required rectangle in unrotated screen space which will supply full screen data after specified rotation
    if( ( g_bskew_comp && ( fabs( skew ) > .001 ) ) || ( fabs( rotation ) > .001 ) ) {

        double rotator = rotation;
        rotator -= skew;

        int dy = wxRound(
                     fabs( pix_height * cos( rotator ) ) + fabs( pix_width * sin( rotator ) ) );
        int dx = wxRound(
                     fabs( pix_width * cos( rotator ) ) + fabs( pix_height * sin( rotator ) ) );

        //  It is important for MSW build that viewport pixel dimensions be multiples of 4.....
        if( dy % 4 ) dy += 4 - ( dy % 4 );
        if( dx % 4 ) dx += 4 - ( dx % 4 );

        //  Grow the source rectangle appropriately
        if( fabs( rotator ) > .001 ) rv_rect.Inflate( ( dx - pix_width ) / 2,
                    ( dy - pix_height ) / 2 );

    }

    //  Compute Viewport lat/lon reference points for co-ordinate hit testing

    //  This must be done in unrotated space with respect to full unrotated screen space calculated above
    double rotation_save = rotation;
    SetRotationAngle( 0. );

    double lat_ul, lat_ur, lat_lr, lat_ll;
    double lon_ul, lon_ur, lon_lr, lon_ll;

    GetLLFromPix( wxPoint( rv_rect.x, rv_rect.y ), &lat_ul, &lon_ul );
    GetLLFromPix( wxPoint( rv_rect.x + rv_rect.width, rv_rect.y ), &lat_ur, &lon_ur );
    GetLLFromPix( wxPoint( rv_rect.x + rv_rect.width, rv_rect.y + rv_rect.height ), &lat_lr,
                  &lon_lr );
    GetLLFromPix( wxPoint( rv_rect.x, rv_rect.y + rv_rect.height ), &lat_ll, &lon_ll );

    if( clon < 0. ) {
        if( ( lon_ul > 0. ) && ( lon_ur < 0. ) ) {
            lon_ul -= 360.;
            lon_ll -= 360.;
        }
    } else {
        if( ( lon_ul > 0. ) && ( lon_ur < 0. ) ) {
            lon_ur += 360.;
            lon_lr += 360.;
        }
    }

    if( lon_ur < lon_ul ) {
        lon_ur += 360.;
        lon_lr += 360.;
    }

    if( lon_ur > 360. ) {
        lon_ur -= 360.;
        lon_lr -= 360.;
        lon_ul -= 360.;
        lon_ll -= 360.;
    }

    double dlat_min = lat_ul;
    dlat_min = fmin ( dlat_min, lat_ur );
    dlat_min = fmin ( dlat_min, lat_lr );
    dlat_min = fmin ( dlat_min, lat_ll );

    double dlon_min = lon_ul;
    dlon_min = fmin ( dlon_min, lon_ur );
    dlon_min = fmin ( dlon_min, lon_lr );
    dlon_min = fmin ( dlon_min, lon_ll );

    double dlat_max = lat_ul;
    dlat_max = fmax ( dlat_max, lat_ur );
    dlat_max = fmax ( dlat_max, lat_lr );
    dlat_max = fmax ( dlat_max, lat_ll );

    double dlon_max = lon_ur;
    dlon_max = fmax ( dlon_max, lon_ul );
    dlon_max = fmax ( dlon_max, lon_lr );
    dlon_max = fmax ( dlon_max, lon_ll );

    //  Set the viewport lat/lon bounding box appropriately
    vpBBox.SetMin( dlon_min, dlat_min );
    vpBBox.SetMax( dlon_max, dlat_max );

    // Restore the rotation angle
    SetRotationAngle( rotation_save );
}
//------------------------------------------------------------------------------
//    ChartCanvas Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE ( ChartCanvas, wxWindow )
    EVT_PAINT ( ChartCanvas::OnPaint )
    EVT_ACTIVATE ( ChartCanvas::OnActivate )
    EVT_SIZE ( ChartCanvas::OnSize )
    EVT_MOUSE_EVENTS ( ChartCanvas::MouseEvent )
    EVT_TIMER ( DBLCLICK_TIMER, ChartCanvas::MouseTimedEvent )
    EVT_TIMER ( PAN_TIMER, ChartCanvas::PanTimerEvent )
    EVT_TIMER ( CURTRACK_TIMER, ChartCanvas::OnCursorTrackTimerEvent )
    EVT_TIMER ( ROT_TIMER, ChartCanvas::RotateTimerEvent )
    EVT_TIMER ( RTELEGPU_TIMER, ChartCanvas::OnRouteLegPopupTimerEvent )
    EVT_KEY_DOWN(ChartCanvas::OnKeyDown )
    EVT_KEY_UP(ChartCanvas::OnKeyUp )
    EVT_TIMER ( PANKEY_TIMER, ChartCanvas::Do_Pankeys )
    EVT_MOUSE_CAPTURE_LOST(ChartCanvas::LostMouseCapture )
    EVT_TIMER ( ZOOM_TIMER, ChartCanvas::OnZoomTimerEvent )

    EVT_MENU ( ID_DEF_MENU_MAX_DETAIL,         ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_SCALE_IN,           ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_SCALE_OUT,          ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_QUERY,              ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_DROP_WP,            ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_MOVE_BOAT_HERE,     ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_GOTO_HERE,          ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_GOTOPOSITION,       ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_COGUP,              ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_NORTHUP,            ChartCanvas::PopupMenuHandler )

    EVT_MENU ( ID_RT_MENU_ACTIVATE,     ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RT_MENU_DEACTIVATE,   ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RT_MENU_INSERT,       ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RT_MENU_APPEND,       ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RT_MENU_COPY,         ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_TK_MENU_COPY,         ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_WPT_MENU_COPY,        ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_WPT_MENU_SENDTOGPS,   ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_PASTE_WAYPOINT,       ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_PASTE_ROUTE,          ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_PASTE_TRACK,          ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RT_MENU_DELETE,       ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RT_MENU_REVERSE,      ChartCanvas::PopupMenuHandler )

    EVT_MENU ( ID_RT_MENU_DELPOINT,     ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RT_MENU_REMPOINT,     ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RT_MENU_ACTPOINT,     ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RT_MENU_DEACTPOINT,   ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RT_MENU_ACTNXTPOINT,  ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RT_MENU_PROPERTIES,   ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_WP_MENU_SET_ANCHORWATCH,    ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_WP_MENU_CLEAR_ANCHORWATCH,  ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_AISTARGETLIST,     ChartCanvas::PopupMenuHandler )

    EVT_MENU ( ID_RC_MENU_SCALE_IN,     ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RC_MENU_SCALE_OUT,    ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RC_MENU_ZOOM_IN,      ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RC_MENU_ZOOM_OUT,     ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_RC_MENU_FINISH,       ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_AIS_QUERY,   ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_AIS_CPA,     ChartCanvas::PopupMenuHandler )

    EVT_MENU ( ID_UNDO,                 ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_REDO,                 ChartCanvas::PopupMenuHandler )

    EVT_MENU ( ID_DEF_MENU_ACTIVATE_MEASURE,   ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_DEACTIVATE_MEASURE, ChartCanvas::PopupMenuHandler )

    EVT_MENU ( ID_DEF_MENU_CM93OFFSET_DIALOG,   ChartCanvas::PopupMenuHandler )

    EVT_MENU ( ID_WP_MENU_GOTO,               ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_WP_MENU_DELPOINT,           ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_WP_MENU_PROPERTIES,         ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_WP_MENU_ADDITIONAL_INFO,    ChartCanvas::PopupMenuHandler )

    EVT_MENU ( ID_TK_MENU_PROPERTIES,       ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_TK_MENU_DELETE,           ChartCanvas::PopupMenuHandler )

    EVT_MENU ( ID_DEF_MENU_QUILTREMOVE,     ChartCanvas::PopupMenuHandler )

    EVT_MENU ( ID_DEF_MENU_TIDEINFO,        ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_CURRENTINFO,     ChartCanvas::PopupMenuHandler )
    EVT_MENU ( ID_DEF_MENU_GROUPBASE,       ChartCanvas::PopupMenuHandler )
END_EVENT_TABLE()

// Define a constructor for my canvas
ChartCanvas::ChartCanvas ( wxFrame *frame ) :
    wxWindow ( frame, wxID_ANY,    wxPoint ( 20,20 ), wxSize ( 5,5 ), wxSIMPLE_BORDER )
{
    parent_frame = ( MyFrame * ) frame;       // save a pointer to parent

    SetBackgroundColour ( GetGlobalColor ( _T ( "NODTA" ) ) );
    SetBackgroundStyle ( wxBG_STYLE_CUSTOM );  // on WXMSW, this prevents flashing on color scheme change

    m_bDrawingRoute = false;
    m_bRouteEditing = false;
    m_bMarkEditing = false;
    m_bFollow = false;
    m_bTCupdate = false;
    m_bAppendingRoute = false;          // was true in MSW, why??
    pThumbDIBShow = NULL;
    m_bShowCurrent = false;
    m_bShowTide = false;
    bShowingCurrent = false;
    pCwin = NULL;
    warp_flag = false;
    m_bzooming = false;
    m_bmouse_key_mod = false;

    pss_overlay_bmp = NULL;
    pss_overlay_mask = NULL;
    m_bChartDragging = false;
    m_bMeasure_Active = false;
    m_pMeasureRoute = NULL;
    m_pRolloverWin = NULL;
    m_pAISRolloverWin = NULL;

    m_pCIWin = NULL;

    m_pSelectedRoute              = NULL;
    m_pSelectedTrack              = NULL;
    m_pRoutePointEditTarget       = NULL;
    m_pFoundPoint                 = NULL;
    m_pMouseRoute                 = NULL;
    m_prev_pMousePoint            = NULL;
    m_pEditRouteArray             = NULL;
    m_pFoundRoutePoint            = NULL;
    m_pFoundRoutePointSecond      = NULL;

    m_pRolloverRouteSeg           = NULL;

    m_bbrightdir = false;
    r_gamma_mult = 1;
    g_gamma_mult = 1;
    b_gamma_mult = 1;


    m_pos_image_user_day        = NULL;
    m_pos_image_user_dusk       = NULL;
    m_pos_image_user_night      = NULL;
    m_pos_image_user_grey_day   = NULL;
    m_pos_image_user_grey_dusk  = NULL;
    m_pos_image_user_grey_night = NULL;

    m_zoom_timer.SetOwner(this, ZOOM_TIMER);
    m_bzooming_in = false;;
    m_bzooming_out = false;;

    undo = new Undo;

    VPoint.Invalidate();

    m_glcc = new glChartCanvas(this);

#if wxCHECK_VERSION(2, 9, 0)
    m_pGLcontext = new wxGLContext(m_glcc);
    m_glcc->SetContext(m_pGLcontext);
#else
    m_pGLcontext = m_glcc->GetContext();
#endif


    singleClickEventIsValid = false;

//    Build the cursors

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

#if defined( __WXGTK__) || defined(__WXOSX__)

    wxImage ICursorLeft = style->GetIcon( _T("left") ).ConvertToImage();
    wxImage ICursorRight = style->GetIcon( _T("right") ).ConvertToImage();
    wxImage ICursorUp = style->GetIcon( _T("up") ).ConvertToImage();
    wxImage ICursorDown = style->GetIcon( _T("down") ).ConvertToImage();
    wxImage ICursorPencil = style->GetIcon( _T("pencil") ).ConvertToImage();
    wxImage ICursorCross = style->GetIcon( _T("cross") ).ConvertToImage();

#if wxCHECK_VERSION(2, 8, 12)
#else
    ICursorLeft.ConvertAlphaToMask(128);
    ICursorRight.ConvertAlphaToMask(128);
    ICursorUp.ConvertAlphaToMask(128);
    ICursorDown.ConvertAlphaToMask(128);
    ICursorPencil.ConvertAlphaToMask(10);
    ICursorCross.ConvertAlphaToMask(10);
#endif

    if ( ICursorLeft.Ok() )
    {
        ICursorLeft.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_X, 0 );
        ICursorLeft.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorLeft = new wxCursor ( ICursorLeft );
    }
    else
        pCursorLeft = new wxCursor ( wxCURSOR_ARROW );

    if ( ICursorRight.Ok() )
    {
        ICursorRight.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_X, 31 );
        ICursorRight.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorRight = new wxCursor ( ICursorRight );
    }
    else
        pCursorRight = new wxCursor ( wxCURSOR_ARROW );

    if ( ICursorUp.Ok() )
    {
        ICursorUp.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_X, 15 );
        ICursorUp.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0 );
        pCursorUp = new wxCursor ( ICursorUp );
    }
    else
        pCursorUp = new wxCursor ( wxCURSOR_ARROW );

    if ( ICursorDown.Ok() )
    {
        ICursorDown.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_X, 15 );
        ICursorDown.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 31 );
        pCursorDown = new wxCursor ( ICursorDown );
    }
    else
        pCursorDown = new wxCursor ( wxCURSOR_ARROW );

    if ( ICursorPencil.Ok() )
    {
        ICursorPencil.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_X, 0 );
        ICursorPencil.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
        pCursorPencil = new wxCursor ( ICursorPencil );
    }
    else
        pCursorPencil = new wxCursor ( wxCURSOR_ARROW );

    if ( ICursorCross.Ok() )
    {
        ICursorCross.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_X, 13 );
        ICursorCross.SetOption ( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 12);
        pCursorCross = new wxCursor ( ICursorCross );
    }
    else
        pCursorCross = new wxCursor ( wxCURSOR_ARROW );

#else

    wxImage ICursorLeft = style->GetIcon( _T("left") ).ConvertToImage();
    wxImage ICursorRight = style->GetIcon( _T("right") ).ConvertToImage();
    wxImage ICursorUp = style->GetIcon( _T("up") ).ConvertToImage();
    wxImage ICursorDown = style->GetIcon( _T("down") ).ConvertToImage();
    wxImage ICursorPencil = style->GetIcon( _T("pencil") ).ConvertToImage();
    wxImage ICursorCross = style->GetIcon( _T("cross") ).ConvertToImage();

    if( ICursorLeft.Ok() ) {
        ICursorLeft.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, 0 );
        ICursorLeft.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorLeft = new wxCursor( ICursorLeft );
    } else
        pCursorLeft = new wxCursor( wxCURSOR_ARROW );

    if( ICursorRight.Ok() ) {
        ICursorRight.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, 31 );
        ICursorRight.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorRight = new wxCursor( ICursorRight );
    } else
        pCursorRight = new wxCursor( wxCURSOR_ARROW );

    if( ICursorUp.Ok() ) {
        ICursorUp.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, 15 );
        ICursorUp.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0 );
        pCursorUp = new wxCursor( ICursorUp );
    } else
        pCursorUp = new wxCursor( wxCURSOR_ARROW );

    if( ICursorDown.Ok() ) {
        ICursorDown.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, 15 );
        ICursorDown.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 31 );
        pCursorDown = new wxCursor( ICursorDown );
    } else
        pCursorDown = new wxCursor( wxCURSOR_ARROW );

    if( ICursorPencil.Ok() ) {
        ICursorPencil.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, 0 );
        ICursorPencil.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorPencil = new wxCursor( ICursorPencil );
    } else
        pCursorPencil = new wxCursor( wxCURSOR_ARROW );

    if( ICursorCross.Ok() ) {
        ICursorCross.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_X, 13 );
        ICursorCross.SetOption( wxIMAGE_OPTION_CUR_HOTSPOT_Y, 12 );
        pCursorCross = new wxCursor( ICursorCross );
    } else
        pCursorCross = new wxCursor( wxCURSOR_ARROW );

#endif      // MSW, X11
    pCursorArrow = new wxCursor( wxCURSOR_ARROW );

    SetCursor( *pCursorArrow );

    pPanTimer = new wxTimer( this, PAN_TIMER );
    pPanTimer->Stop();

    pRotDefTimer = new wxTimer( this, ROT_TIMER );
    pRotDefTimer->Stop();

    m_DoubleClickTimer = new wxTimer( this, DBLCLICK_TIMER );
    m_DoubleClickTimer->Stop();

    pPanKeyTimer = new wxTimer( this, PANKEY_TIMER );
    pPanKeyTimer->Stop();
    m_panx = m_pany = 0;
    m_panspeed = 0;

    pCurTrackTimer = new wxTimer( this, CURTRACK_TIMER );
    pCurTrackTimer->Stop();
    m_curtrack_timer_msec = 10;

    m_MouseWheelTimer.SetOwner( this );

    m_RouteLegPopupTimer.SetOwner( this, RTELEGPU_TIMER );

    m_routeleg_popup_timer_msec = 20;

    m_b_rot_hidef = true;

//    Set up current arrow drawing factors
    int mmx, mmy;
    wxDisplaySizeMM( &mmx, &mmy );

    int sx, sy;
    wxDisplaySize( &sx, &sy );

    m_pix_per_mm = ( (double) sx ) / ( (double) mmx );

    int mm_per_knot = 10;
    current_draw_scaler = mm_per_knot * m_pix_per_mm * g_current_arrow_scale / 100.0;
    pscratch_bm = NULL;
    proute_bm = NULL;

    m_prot_bm = NULL;

// Set some benign initial values

    m_cs = GLOBAL_COLOR_SCHEME_DAY;
    VPoint.clat = 0;
    VPoint.clon = 0;
    VPoint.view_scale_ppm = 1;
    VPoint.Invalidate();

    m_canvas_scale_factor = 1.;

    m_canvas_width = 1000;

//    Create the default world chart
    pWorldBackgroundChart = new GSHHSChart;

//    Create the default depth unit emboss maps
    m_pEM_Feet = NULL;
    m_pEM_Meters = NULL;
    m_pEM_Fathoms = NULL;

    CreateDepthUnitEmbossMaps( GLOBAL_COLOR_SCHEME_DAY );

    m_pEM_OverZoom = NULL;
    CreateOZEmbossMapData( GLOBAL_COLOR_SCHEME_DAY );

//    Build icons for tide/current points
    m_bmTideDay = style->GetIcon( _T("tidesml") );

//    Dusk
    m_bmTideDusk = CreateDimBitmap( m_bmTideDay, .50 );

//    Night
    m_bmTideNight = CreateDimBitmap( m_bmTideDay, .20 );

//    Build Dusk/Night  ownship icons
    double factor_dusk = 0.5;
    double factor_night = 0.25;

    //Red
    m_os_image_red_day = style->GetIcon( _T("ship-red") ).ConvertToImage();

    int rimg_width = m_os_image_red_day.GetWidth();
    int rimg_height = m_os_image_red_day.GetHeight();

    m_os_image_red_dusk = m_os_image_red_day.Copy();
    m_os_image_red_night = m_os_image_red_day.Copy();

    for( int iy = 0; iy < rimg_height; iy++ ) {
        for( int ix = 0; ix < rimg_width; ix++ ) {
            if( !m_os_image_red_day.IsTransparent( ix, iy ) ) {
                wxImage::RGBValue rgb( m_os_image_red_day.GetRed( ix, iy ),
                                       m_os_image_red_day.GetGreen( ix, iy ),
                                       m_os_image_red_day.GetBlue( ix, iy ) );
                wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
                hsv.value = hsv.value * factor_dusk;
                wxImage::RGBValue nrgb = wxImage::HSVtoRGB( hsv );
                m_os_image_red_dusk.SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );

                hsv = wxImage::RGBtoHSV( rgb );
                hsv.value = hsv.value * factor_night;
                nrgb = wxImage::HSVtoRGB( hsv );
                m_os_image_red_night.SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );
            }
        }
    }

    //Grey
    m_os_image_grey_day = style->GetIcon( _T("ship-red") ).ConvertToImage().ConvertToGreyscale();

    int gimg_width = m_os_image_grey_day.GetWidth();
    int gimg_height = m_os_image_grey_day.GetHeight();

    m_os_image_grey_dusk = m_os_image_grey_day.Copy();
    m_os_image_grey_night = m_os_image_grey_day.Copy();

    for( int iy = 0; iy < gimg_height; iy++ ) {
        for( int ix = 0; ix < gimg_width; ix++ ) {
            if( !m_os_image_grey_day.IsTransparent( ix, iy ) ) {
                wxImage::RGBValue rgb( m_os_image_grey_day.GetRed( ix, iy ),
                                       m_os_image_grey_day.GetGreen( ix, iy ),
                                       m_os_image_grey_day.GetBlue( ix, iy ) );
                wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
                hsv.value = hsv.value * factor_dusk;
                wxImage::RGBValue nrgb = wxImage::HSVtoRGB( hsv );
                m_os_image_grey_dusk.SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );

                hsv = wxImage::RGBtoHSV( rgb );
                hsv.value = hsv.value * factor_night;
                nrgb = wxImage::HSVtoRGB( hsv );
                m_os_image_grey_night.SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );
            }
        }
    }

    //  Set initial pointers to ownship images
    m_pos_image_red = &m_os_image_red_day;

    //  Look for user defined ownship image
    //  This may be found in the shared data location along with other user defined icons.
    //  and will be called "ownship.xpm" or "ownship.png"
    if( pWayPointMan && pWayPointMan->DoesIconExist( _T("ownship") ) ) {
        wxBitmap *pbmp = pWayPointMan->GetIconBitmap( _T("ownship") );
        m_pos_image_user_day = new wxImage;
        *m_pos_image_user_day = pbmp->ConvertToImage();
        m_pos_image_user_day->InitAlpha();

        int gimg_width = m_pos_image_user_day->GetWidth();
        int gimg_height = m_pos_image_user_day->GetHeight();

        // Make dusk and night images
        m_pos_image_user_dusk = new wxImage;
        m_pos_image_user_night = new wxImage;

        *m_pos_image_user_dusk = m_pos_image_user_day->Copy();
        *m_pos_image_user_night = m_pos_image_user_day->Copy();

        for( int iy = 0; iy < gimg_height; iy++ ) {
            for( int ix = 0; ix < gimg_width; ix++ ) {
                if( !m_pos_image_user_day->IsTransparent( ix, iy ) ) {
                    wxImage::RGBValue rgb( m_pos_image_user_day->GetRed( ix, iy ),
                                           m_pos_image_user_day->GetGreen( ix, iy ),
                                           m_pos_image_user_day->GetBlue( ix, iy ) );
                    wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
                    hsv.value = hsv.value * factor_dusk;
                    wxImage::RGBValue nrgb = wxImage::HSVtoRGB( hsv );
                    m_pos_image_user_dusk->SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );

                    hsv = wxImage::RGBtoHSV( rgb );
                    hsv.value = hsv.value * factor_night;
                    nrgb = wxImage::HSVtoRGB( hsv );
                    m_pos_image_user_night->SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );
                }
            }
        }

        //  Make some alternate greyed out day/dusk/night images
        m_pos_image_user_grey_day = new wxImage;
        *m_pos_image_user_grey_day = m_pos_image_user_day->ConvertToGreyscale();

        m_pos_image_user_grey_dusk = new wxImage;
        m_pos_image_user_grey_night = new wxImage;

        *m_pos_image_user_grey_dusk = m_pos_image_user_grey_day->Copy();
        *m_pos_image_user_grey_night = m_pos_image_user_grey_day->Copy();

        for( int iy = 0; iy < gimg_height; iy++ ) {
            for( int ix = 0; ix < gimg_width; ix++ ) {
                if( !m_pos_image_user_grey_day->IsTransparent( ix, iy ) ) {
                    wxImage::RGBValue rgb( m_pos_image_user_grey_day->GetRed( ix, iy ),
                                           m_pos_image_user_grey_day->GetGreen( ix, iy ),
                                           m_pos_image_user_grey_day->GetBlue( ix, iy ) );
                    wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
                    hsv.value = hsv.value * factor_dusk;
                    wxImage::RGBValue nrgb = wxImage::HSVtoRGB( hsv );
                    m_pos_image_user_grey_dusk->SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );

                    hsv = wxImage::RGBtoHSV( rgb );
                    hsv.value = hsv.value * factor_night;
                    nrgb = wxImage::HSVtoRGB( hsv );
                    m_pos_image_user_grey_night->SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );
                }
            }
        }

    }

    m_pQuilt = new Quilt();
}

ChartCanvas::~ChartCanvas()
{

    delete pThumbDIBShow;

//    Delete Cursors
    delete pCursorLeft;
    delete pCursorRight;
    delete pCursorUp;
    delete pCursorDown;
    delete pCursorArrow;
    delete pCursorPencil;
    delete pCursorCross;

    delete pPanTimer;
    delete pCurTrackTimer;
    delete pRotDefTimer;
    delete pPanKeyTimer;
    delete m_DoubleClickTimer;

    delete m_pRolloverWin;
    delete m_pAISRolloverWin;

    delete m_pCIWin;

    delete pscratch_bm;

    m_dc_route.SelectObject( wxNullBitmap );
    delete proute_bm;

    delete pWorldBackgroundChart;
    delete pss_overlay_bmp;

    delete m_pEM_Feet;
    delete m_pEM_Meters;
    delete m_pEM_Fathoms;

    delete m_pEM_OverZoom;
//        delete m_pEM_CM93Offset;

    delete m_pQuilt;

    delete m_prot_bm;

    delete m_pos_image_user_day;
    delete m_pos_image_user_dusk;
    delete m_pos_image_user_night;
    delete m_pos_image_user_grey_day;
    delete m_pos_image_user_grey_dusk;
    delete m_pos_image_user_grey_night;
    delete undo;
    delete m_glcc;
}

int ChartCanvas::GetCanvasChartNativeScale()
{
    int ret = 1;
    if( !VPoint.b_quilt ) {
        if( Current_Ch ) ret = Current_Ch->GetNativeScale();
    } else
        ret = (int) m_pQuilt->GetRefNativeScale();

    return ret;

}

ChartBase* ChartCanvas::GetChartAtCursor() {
    ChartBase* target_chart;
    if( Current_Ch && ( Current_Ch->GetChartFamily() == CHART_FAMILY_VECTOR ) )
        target_chart = Current_Ch;
    else
        if( VPoint.b_quilt )
            target_chart = cc1->m_pQuilt->GetChartAtPix( wxPoint( mouse_x, mouse_y ) );
        else
            target_chart = NULL;
    return target_chart;
}

ChartBase* ChartCanvas::GetOverlayChartAtCursor() {
    ChartBase* target_chart;
    if( VPoint.b_quilt )
        target_chart = cc1->m_pQuilt->GetOverlayChartAtPix( wxPoint( mouse_x, mouse_y ) );
    else
        target_chart = NULL;
    return target_chart;
}

int ChartCanvas::FindClosestCanvasChartdbIndex( int scale )
{
    int new_dbIndex = -1;
    if( !VPoint.b_quilt ) {
        if( pCurrentStack ) {
            for( int i = 0; i < pCurrentStack->nEntry; i++ ) {
                int sc = ChartData->GetStackChartScale( pCurrentStack, i, NULL, 0 );
                if( sc >= scale ) {
                    new_dbIndex = pCurrentStack->GetDBIndex( i );
                    break;
                }
            }
        }
    } else {
        //    Using the current quilt, select a useable reference chart
        //    Said chart will be in the extended (possibly full-screen) stack,
        //    And will have a scale equal to or just greater than the stipulated value
        unsigned int im = m_pQuilt->GetExtendedStackIndexArray().GetCount();
        if( im > 0 ) {
            for( unsigned int is = 0; is < im; is++ ) {
                const ChartTableEntry &m = ChartData->GetChartTableEntry(
                                               m_pQuilt->GetExtendedStackIndexArray().Item( is ) );
                if( ( m.GetScale() >= scale )/* && (m_reference_family == m.GetChartFamily())*/) {
                    new_dbIndex = m_pQuilt->GetExtendedStackIndexArray().Item( is );
                    break;
                }
            }
        }
    }

    return new_dbIndex;
}

bool ChartCanvas::IsQuiltDelta()
{
    return m_pQuilt->IsQuiltDelta( VPoint );
}

ArrayOfInts ChartCanvas::GetQuiltIndexArray( void )
{
    return m_pQuilt->GetQuiltIndexArray();;
}

void ChartCanvas::SetQuiltMode( bool b_quilt )
{
    VPoint.b_quilt = b_quilt;
    VPoint.b_FullScreenQuilt = g_bFullScreenQuilt;
}

bool ChartCanvas::GetQuiltMode( void )
{
    return VPoint.b_quilt;
}

int ChartCanvas::GetQuiltReferenceChartIndex(void)
{
    return m_pQuilt->GetRefChartdbIndex();
}

void ChartCanvas::InvalidateAllQuiltPatchs( void )
{
    m_pQuilt->InvalidateAllQuiltPatchs();
}

ChartBase *ChartCanvas::GetLargestScaleQuiltChart()
{
    return m_pQuilt->GetLargestScaleChart();
}

ChartBase *ChartCanvas::GetFirstQuiltChart()
{
    return m_pQuilt->GetFirstChart();
}

ChartBase *ChartCanvas::GetNextQuiltChart()
{
    return m_pQuilt->GetNextChart();
}

int ChartCanvas::GetQuiltChartCount()
{
    return m_pQuilt->GetnCharts();
}

void ChartCanvas::SetQuiltChartHiLiteIndex( int dbIndex )
{
    m_pQuilt->SetHiliteIndex( dbIndex );
}

ArrayOfInts ChartCanvas::GetQuiltCandidatedbIndexArray( bool flag1, bool flag2 )
{
    return m_pQuilt->GetCandidatedbIndexArray( flag1, flag2 );
}

int ChartCanvas::GetQuiltRefChartdbIndex( void )
{
    return m_pQuilt->GetRefChartdbIndex();
}

ArrayOfInts ChartCanvas::GetQuiltExtendedStackdbIndexArray()
{
    return m_pQuilt->GetExtendedStackIndexArray();
}

ArrayOfInts ChartCanvas::GetQuiltEclipsedStackdbIndexArray()
{
    return m_pQuilt->GetEclipsedStackIndexArray();
}

void ChartCanvas::InvalidateQuilt( void )
{
    return m_pQuilt->Invalidate();
}

double ChartCanvas::GetQuiltMaxErrorFactor()
{
    return m_pQuilt->GetMaxErrorFactor();
}

bool ChartCanvas::IsChartQuiltableRef( int db_index )
{
    return m_pQuilt->IsChartQuiltableRef( db_index );
}

bool ChartCanvas::IsChartLargeEnoughToRender( ChartBase* chart, ViewPort& vp )
{
    double chartMaxScale = chart->GetNormalScaleMax( GetCanvasScaleFactor(), GetCanvasWidth() );
    return ( chartMaxScale*g_ChartNotRenderScaleFactor > vp.chart_scale );
}

void ChartCanvas::CancelMeasureRoute()
{
    m_bMeasure_Active = false;
    m_nMeasureState = 0;
    g_pRouteMan->DeleteRoute( m_pMeasureRoute );
    m_pMeasureRoute = NULL;
}

ViewPort &ChartCanvas::GetVP()
{
    return VPoint;
}

void ChartCanvas::OnKeyDown( wxKeyEvent &event )
{
    m_modkeys = event.GetModifiers();

    if( event.GetKeyCode() == WXK_CONTROL ) m_bmouse_key_mod = true;

    // HOTKEYS
    switch( event.GetKeyCode() ) {
    case WXK_LEFT:
        if( m_modkeys == wxMOD_CONTROL ) parent_frame->DoStackDown();
        else
            m_panx = -1;
        break;

    case WXK_UP:
        m_pany = -1;
        break;

    case WXK_RIGHT:
        if( m_modkeys == wxMOD_CONTROL ) parent_frame->DoStackUp();
        else
            m_panx = 1;
        break;

    case WXK_DOWN:
        m_pany = 1;
        break;

    case WXK_F2:
        parent_frame->TogglebFollow();
        break;

    case WXK_F3: {
        parent_frame->ToggleENCText();
        break;
    }
    case WXK_F4:
        if( !parent_frame->nRoute_State )   // no measure tool if currently creating route
        {
            if( m_bMeasure_Active ) {
                g_pRouteMan->DeleteRoute( m_pMeasureRoute );
                m_pMeasureRoute = NULL;
            }

            m_bMeasure_Active = true;
            m_nMeasureState = 1;
            SetCursor( *pCursorPencil );
            Refresh();
        }
        break;

    case WXK_F5:
        parent_frame->ToggleColorScheme();
        break;

    case WXK_F6: {
        int mod = m_modkeys & wxMOD_SHIFT;
        if( mod != m_brightmod ) {
            m_brightmod = mod;
            m_bbrightdir = !m_bbrightdir;
        }

        if( !m_bbrightdir ) {
            g_nbrightness -= 10;
            if( g_nbrightness <= MIN_BRIGHT ) {
                g_nbrightness = MIN_BRIGHT;
                m_bbrightdir = true;
            }
        } else {
            g_nbrightness += 10;
            if( g_nbrightness >= MAX_BRIGHT ) {
                g_nbrightness = MAX_BRIGHT;
                m_bbrightdir = false;
            }
        }

        SetScreenBrightness( g_nbrightness );

        gFrame->ShowBrightnessLevelTimedDialog( g_nbrightness / 10, 1, 10 );

        SetFocus();             // just in case the external program steals it....
        gFrame->Raise();        // And reactivate the application main

        break;
    }

    case WXK_F7:
        parent_frame->DoStackDown();
        break;

    case WXK_F8:
        parent_frame->DoStackUp();
        break;

    case WXK_F9: {
        parent_frame->ToggleQuiltMode();
        ReloadVP();
        break;
    }

    case WXK_F11:
        parent_frame->ToggleFullScreen();
        break;

    case WXK_F12: {
        parent_frame->ToggleChartOutlines();
        break;
    }

    //NUMERIC PAD
    case WXK_NUMPAD_ADD:              // '+' on NUM PAD
        if( m_modkeys == wxMOD_CONTROL ) ZoomCanvasIn( 1.1 );
        else
            ZoomCanvasIn( 2.0 );
        break;

    case WXK_NUMPAD_SUBTRACT:   // '-' on NUM PAD
        if( m_modkeys == wxMOD_CONTROL ) ZoomCanvasOut( 1.1 );
        else
            ZoomCanvasOut( 2.0 );
        break;

    default:
        break;

    }

    if( event.GetKeyCode() < 128 )            //ascii
    {
        char key_char = (char) event.GetKeyCode();
        if( m_modkeys == wxMOD_CONTROL ) key_char -= 64;

        //      Handle both QWERTY and AZERTY keyboard separately for a few control codes
        if( !g_b_assume_azerty ) {
            switch( key_char ) {
            case '+':
            case '+' - 64:
            case '=':
            case '=' - 64:          // Ctrl =
                if( ( m_modkeys == wxMOD_CONTROL ) ) ZoomCanvasIn( 1.1 );
                else
                    ZoomCanvasIn( 2.0 );
                break;

            case '-':
            case '-' - 64:          // Ctrl -
            case '_':
                if( ( m_modkeys == wxMOD_CONTROL ) ) ZoomCanvasOut( 1.1 );
                else
                    ZoomCanvasOut( 2.0 );
                break;
            }
        } else {
            switch( key_char ) {
            case 43:
            case -21:
                if( ( m_modkeys == wxMOD_CONTROL ) ) ZoomCanvasIn( 1.1 );
                else
                    ZoomCanvasIn( 2.0 );
                break;

            case 54:                     // '-'  alpha/num pad
            case 56:                     // '_'  alpha/num pad
            case -10:                     // Ctrl '-'  alpha/num pad
            case -8:                     // Ctrl '_' alpha/num pad
                if( ( m_modkeys == wxMOD_CONTROL ) ) ZoomCanvasOut( 1.1 );
                else
                    ZoomCanvasOut( 2.0 );
                break;
            }
        }

        switch( key_char ) {
        case 'A':
            parent_frame->ToggleAnchor();
            break;

        case 'D': {
                int x,y;
                event.GetPosition( &x, &y );
                bool cm93IsAvailable = ( Current_Ch && ( Current_Ch->GetChartType() == CHART_TYPE_CM93COMP ) );
                if( VPoint.b_quilt ) {
                    ChartBase *pChartTest = m_pQuilt->GetChartAtPix( wxPoint( x, y ) );
                    if( pChartTest ) {
                        if( pChartTest->GetChartType() == CHART_TYPE_CM93 ) cm93IsAvailable = true;
                        if( pChartTest->GetChartType() == CHART_TYPE_CM93COMP ) cm93IsAvailable = true;
                    }
                }

                if( cm93IsAvailable ) {
                    if( !pCM93DetailSlider ) {
                        pCM93DetailSlider = new CM93DSlide( this, -1, 0,
                                -CM93_ZOOM_FACTOR_MAX_RANGE, CM93_ZOOM_FACTOR_MAX_RANGE,
                                wxPoint( g_cm93detail_dialog_x, g_cm93detail_dialog_y ),
                                wxDefaultSize, wxSIMPLE_BORDER, _("CM93 Detail Level") );
                    }
                    pCM93DetailSlider->Show( !pCM93DetailSlider->IsShown() );
                }
                break;
            }

       case 'L':
            parent_frame->ToggleLights();
            break;

        case 'O':
            parent_frame->ToggleChartOutlines();
            break;

        case 'R':
            parent_frame->ToggleRocks();
            break;

        case 'S':
            parent_frame->ToggleSoundings();
            break;

        case 'T':
            parent_frame->ToggleENCText();
            break;

        case 1:                      // Ctrl A
            parent_frame->TogglebFollow();
            break;

        case 2:                      // Ctrl B
            if( stats ) {
                if( stats->IsShown() )
                    stats->Hide();
                else {
                    stats->Move(0,0);
                    stats->RePosition();
                    stats->Show();
                    gFrame->Raise();
                }
            }
            break;

        case 13:             // Ctrl M // Drop Marker at cursor // Enter // Drop Marker at boat;
        {
            double lat, lon;
            if( m_modkeys == wxMOD_CONTROL ) {
                lat = m_cursor_lat;
                lon = m_cursor_lon;
            } else {
                lat = gLat;
                lon = gLon;
            }
            RoutePoint *pWP = new RoutePoint( lat, lon, g_default_wp_icon, wxEmptyString,
                                              GPX_EMPTY_STRING );
            pWP->m_bIsolatedMark = true;                      // This is an isolated mark
            pSelect->AddSelectableRoutePoint( lat, lon, pWP );
            pConfig->AddNewWayPoint( pWP, -1 );    // use auto next num

            if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateWptListCtrl();
            undo->BeforeUndoableAction( Undo_CreateWaypoint, pWP, Undo_HasParent, NULL );
            undo->AfterUndoableAction( NULL );
            Refresh( false );
            break;
        }

        case 32:             // Space                      //    Drop Marker at boat's position;
        {
            RoutePoint *pWP = new RoutePoint( gLat, gLon, g_default_wp_icon, wxEmptyString,
                                              GPX_EMPTY_STRING );
            pWP->m_bIsolatedMark = true;                      // This is an isolated mark
            pSelect->AddSelectableRoutePoint( gLat, gLon, pWP );
            pConfig->AddNewWayPoint( pWP, -1 );    // use auto next num

            if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateWptListCtrl();
            undo->BeforeUndoableAction( Undo_CreateWaypoint, pWP, Undo_HasParent, NULL );
            undo->AfterUndoableAction( NULL );
            Refresh( false );
            break;
        }

        case -32:                     // Ctrl Space            //    Drop MOB
        {
            if( m_modkeys == wxMOD_CONTROL ) parent_frame->ActivateMOB();

            break;
        }

        case 17:                       // Ctrl Q
            parent_frame->Close();
            return;

        case 18:                       // Ctrl R
            gFrame->nRoute_State = 1;
            cc1->SetCursor( *cc1->pCursorPencil );
            return;

        case 20:                       // Ctrl T
            if( NULL == pGoToPositionDialog ) // There is one global instance of the Go To Position Dialog
                pGoToPositionDialog = new GoToPositionDialog( this );
            pGoToPositionDialog->Show();
            break;

        case 25:                       // Ctrl Y
            if( undo->AnythingToRedo() ) {
                undo->RedoNextAction();
                Refresh( false );
            }
            break;

        case 26:                       // Ctrl Z
            if( undo->AnythingToUndo() ) {
                undo->UndoLastAction();
                Refresh( false );
            }
            break;

        case 27:
            // Generic break
            if( m_bMeasure_Active ) {
                m_bMeasure_Active = false;
                m_nMeasureState = 0;
                g_pRouteMan->DeleteRoute( m_pMeasureRoute );
                m_pMeasureRoute = NULL;
                gFrame->SurfaceToolbar();
                Refresh( false );
            }

            if( parent_frame->nRoute_State )         // creating route?
            {
                FinishRoute();
                gFrame->SurfaceToolbar();
                Refresh( false );
            }

            break;

        case 7:                       // Ctrl G
            switch( gamma_state ) {
            case ( 0 ):
                r_gamma_mult = 0;
                g_gamma_mult = 1;
                b_gamma_mult = 0;
                gamma_state = 1;
                break;
            case ( 1 ):
                r_gamma_mult = 1;
                g_gamma_mult = 0;
                b_gamma_mult = 0;
                gamma_state = 2;
                break;
            case ( 2 ):
                r_gamma_mult = 1;
                g_gamma_mult = 1;
                b_gamma_mult = 1;
                gamma_state = 0;
                break;
            }
            SetScreenBrightness( g_nbrightness );

            break;

        default:
            return;

        }           // switch
    }

    if( !pPanKeyTimer->IsRunning() && ( m_panx || m_pany ) ) pPanKeyTimer->Start( 1,
                wxTIMER_ONE_SHOT );

    event.Skip();
}

void ChartCanvas::OnKeyUp( wxKeyEvent &event )
{
    switch( event.GetKeyCode() ) {
    case WXK_LEFT:
    case WXK_RIGHT:
        m_panx = 0;
        m_panspeed = 0;
        break;

    case WXK_UP:
    case WXK_DOWN:
        m_pany = 0;
        m_panspeed = 0;
        break;

    case WXK_CONTROL:
        m_modkeys = wxMOD_NONE;          //Clear Ctrl key
        m_bmouse_key_mod = false;

        break;

    }
    event.Skip();
}

void ChartCanvas::Do_Pankeys( wxTimerEvent& event )
{
    if( !( m_panx || m_pany ) ) return;

    const int slowpan = 2, maxpan = 100;
    int repeat = 100;

    if( m_modkeys == wxMOD_ALT ) m_panspeed = slowpan;
    else if( g_bsmoothpanzoom ) {
        /* accelerate panning */
        m_panspeed += 2;
        if( m_panspeed > maxpan ) m_panspeed = maxpan;

        repeat = 5;
    } else
        m_panspeed = maxpan;

    PanCanvas( m_panspeed * m_panx, m_panspeed * m_pany );
    pPanKeyTimer->Start( repeat, wxTIMER_ONE_SHOT );
}

void ChartCanvas::SetColorScheme( ColorScheme cs )
{
    //    Setup ownship image pointers
    switch( cs ) {
    case GLOBAL_COLOR_SCHEME_DAY:
        m_pos_image_red = &m_os_image_red_day;
        m_pos_image_grey = &m_os_image_grey_day;
        m_pos_image_user = m_pos_image_user_day;
        m_pos_image_user_grey = m_pos_image_user_grey_day;
        break;
    case GLOBAL_COLOR_SCHEME_DUSK:
        m_pos_image_red = &m_os_image_red_dusk;
        m_pos_image_grey = &m_os_image_grey_dusk;
        m_pos_image_user = m_pos_image_user_dusk;
        m_pos_image_user_grey = m_pos_image_user_grey_dusk;
        break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
        m_pos_image_red = &m_os_image_red_night;
        m_pos_image_grey = &m_os_image_grey_night;
        m_pos_image_user = m_pos_image_user_night;
        m_pos_image_user_grey = m_pos_image_user_grey_night;
        break;
    default:
        m_pos_image_red = &m_os_image_red_day;
        m_pos_image_grey = &m_os_image_grey_day;
        m_pos_image_user = m_pos_image_user_day;
        m_pos_image_user_grey = m_pos_image_user_grey_day;
        break;
    }

    CreateDepthUnitEmbossMaps( cs );
    CreateOZEmbossMapData( cs );

    if( g_bopengl && m_glcc ) m_glcc->ClearAllRasterTextures();

    SetbTCUpdate( true );                        // force re-render of tide/current locators

    ReloadVP();

    m_cs = cs;
}

wxBitmap ChartCanvas::CreateDimBitmap( wxBitmap &Bitmap, double factor )
{
    wxImage img = Bitmap.ConvertToImage();
    int sx = img.GetWidth();
    int sy = img.GetHeight();

    wxImage new_img( img );

    for( int i = 0; i < sx; i++ ) {
        for( int j = 0; j < sy; j++ ) {
            if( !img.IsTransparent( i, j ) ) {
                new_img.SetRGB( i, j, (unsigned char) ( img.GetRed( i, j ) * factor ),
                                (unsigned char) ( img.GetGreen( i, j ) * factor ),
                                (unsigned char) ( img.GetBlue( i, j ) * factor ) );
            }
        }
    }

    wxBitmap ret = wxBitmap( new_img );

    return ret;

}

void ChartCanvas::RotateTimerEvent( wxTimerEvent& event )
{
    m_b_rot_hidef = true;
    ReloadVP();
}

void ChartCanvas::OnRouteLegPopupTimerEvent( wxTimerEvent& event )
{
    // Route info rollover
    // Show the route segment info
    bool showRollover = false;

    if( NULL == m_pRolloverRouteSeg ) {
        //    Get a list of all selectable sgements, and search for the first visible segment as the rollover target.

        SelectableItemList SelList = pSelect->FindSelectionList( m_cursor_lat, m_cursor_lon,
                                     SELTYPE_ROUTESEGMENT );
        wxSelectableItemListNode *node = SelList.GetFirst();
        while( node ) {
            SelectItem *pFindSel = node->GetData();

            Route *pr = (Route *) pFindSel->m_pData3;        //candidate

            if( pr && pr->IsVisible() ) {
                m_pRolloverRouteSeg = pFindSel;
                showRollover = true;

                if( NULL == m_pRolloverWin ) {
                    m_pRolloverWin = new RolloverWin( this );
                    m_pRolloverWin->IsActive( false );
                }

                if( !m_pRolloverWin->IsActive() ) {
                    wxString s;
                    RoutePoint *segShow_point_a = (RoutePoint *) m_pRolloverRouteSeg->m_pData1;
                    RoutePoint *segShow_point_b = (RoutePoint *) m_pRolloverRouteSeg->m_pData2;

                    double brg, dist;
                    DistanceBearingMercator( segShow_point_b->m_lat, segShow_point_b->m_lon,
                                             segShow_point_a->m_lat, segShow_point_a->m_lon, &brg, &dist );

                    s.Append( _("Route: ") );
                    if( pr->m_RouteNameString.IsEmpty() ) s.Append( _("(unnamed)") );
                    else
                        s.Append( pr->m_RouteNameString );

                    s << _T("\n") << _("Total Length: ") << FormatDistanceAdaptive( pr->m_route_length)
                    << _T("\n") << _("Leg: from ") << segShow_point_a->GetName()
                    << _(" to ") << segShow_point_b->GetName()
                    << _T("\n") << wxString::Format( wxString( "%03d°  ", wxConvUTF8 ), (int) brg )
                    << FormatDistanceAdaptive( dist );

                    // Compute and display cumulative distance from route start point to current
                    // leg end point.

                    if( segShow_point_a != pr->pRoutePointList->GetFirst()->GetData() ) {
                        wxRoutePointListNode *node = (pr->pRoutePointList)->GetFirst()->GetNext();
                        RoutePoint *prp;
                        float dist_to_endleg = 0;
                        wxString t;

                        while( node ) {
                            prp = node->GetData();
                            dist_to_endleg += prp->m_seg_len;
                            if( prp->IsSame( segShow_point_a ) ) break;
                            node = node->GetNext();
                        }
                        s << _T(" (+") << FormatDistanceAdaptive( dist_to_endleg ) << _T(")");
                    }

                    m_pRolloverWin->SetString( s );

                    wxSize win_size = GetSize();
                    if( console->IsShown() ) win_size.x -= console->GetSize().x;
                    m_pRolloverWin->SetBestPosition( mouse_x, mouse_y, 16, 16, LEG_ROLLOVER,
                                                     win_size );
                    m_pRolloverWin->SetBitmap( LEG_ROLLOVER );
                    m_pRolloverWin->IsActive( true );
                    Refresh();
                    showRollover = true;
                    break;
                }
            } else
                node = node->GetNext();
        }
    } else {
        //    Is the cursor still in select radius?
        if( !pSelect->IsSelectableSegmentSelected( m_cursor_lat, m_cursor_lon,
                m_pRolloverRouteSeg ) ) showRollover = false;
        else
            showRollover = true;
    }

    //    If currently creating a route, do not show this rollover window
    if( parent_frame->nRoute_State ) showRollover = false;

    //    Similar for AIS target rollover window
    if( m_pAISRolloverWin && m_pAISRolloverWin->IsActive() ) showRollover = false;

    if( m_pRolloverWin && m_pRolloverWin->IsActive() && !showRollover ) {
        m_pRolloverWin->IsActive( false );
        m_pRolloverRouteSeg = NULL;
        m_pRolloverWin->Destroy();
        m_pRolloverWin = NULL;
        Refresh();
    } else if( m_pRolloverWin && showRollover ) {
        m_pRolloverWin->IsActive( true );
        Refresh();
    }
}

void ChartCanvas::OnCursorTrackTimerEvent( wxTimerEvent& event )
{
#ifdef USE_S57
    if( s57_CheckExtendedLightSectors( mouse_x, mouse_y, VPoint, extendedSectorLegs ) ) ReloadVP( false );
#endif

//      This is here because GTK status window update is expensive.. Why??
//      Anyway, only update the status bar when this timer expires
#ifdef __WXGTK__
    {
        //    Check the absolute range of the cursor position
        //    There could be a window wherein the chart geoereferencing is not valid....
        double cursor_lat, cursor_lon;
        cc1->GetCanvasPixPoint ( mouse_x, mouse_y, cursor_lat, cursor_lon );

        if((fabs(cursor_lat) < 90.) && (fabs(cursor_lon) < 360.))
        {
            while(cursor_lon < -180.)
                cursor_lon += 360.;

            while(cursor_lon > 180.)
                cursor_lon -= 360.;

            if ( parent_frame->m_pStatusBar )
            {
                wxString s1;
                s1 += _T(" ");
                s1 += toSDMM(1, cursor_lat);
                s1 += _T("   ");
                s1 += toSDMM(2, cursor_lon);
                parent_frame->SetStatusText ( s1, STAT_FIELD_CURSOR_LL );

                double brg, dist;
                DistanceBearingMercator(cursor_lat, cursor_lon, gLat, gLon, &brg, &dist);
                wxString s;
                s.Printf( wxString("%03d° ", wxConvUTF8 ), (int)brg );
                s << FormatDistanceAdaptive( dist );
                parent_frame->SetStatusText ( s, STAT_FIELD_CURSOR_BRGRNG );
            }
        }
    }
#endif
}

void ChartCanvas::GetCursorLatLon( double *lat, double *lon )
{
    double clat, clon;
    GetCanvasPixPoint( mouse_x, mouse_y, clat, clon );
    *lat = clat;
    *lon = clon;
}

void ChartCanvas::GetCanvasPointPix( double rlat, double rlon, wxPoint *r )
{
    // If the Current Chart is a raster chart, and the
    // requested lat/long is within the boundaries of the chart,
    // and the VP is not rotated,
    // then use the embedded BSB chart georeferencing algorithm
    // for greater accuracy
    // Additionally, use chart embedded georef if the projection is TMERC
    //  i.e. NOT MERCATOR and NOT POLYCONIC

    // If for some reason the chart rejects the request by returning an error,
    // then fall back to Viewport Projection estimate from canvas parameters
    bool bUseVP = true;

    if( Current_Ch && ( Current_Ch->GetChartFamily() == CHART_FAMILY_RASTER )
            && ( ( ( fabs( GetVP().rotation ) < .01 ) && !g_bskew_comp )
                 || ( ( Current_Ch->GetChartProjectionType() != PROJECTION_MERCATOR )
                      && ( Current_Ch->GetChartProjectionType() != PROJECTION_POLYCONIC ) ) ) )

    {
        ChartBaseBSB *Cur_BSB_Ch = dynamic_cast<ChartBaseBSB *>( Current_Ch );
//                        bool bInside = G_FloatPtInPolygon ( ( MyFlPoint * ) Cur_BSB_Ch->GetCOVRTableHead ( 0 ),
//                                                            Cur_BSB_Ch->GetCOVRTablenPoints ( 0 ), rlon, rlat );
//                        bInside = true;
//                        if ( bInside )
        if( Cur_BSB_Ch ) {
            int rpixxd, rpixyd;
            if( 0 == Cur_BSB_Ch->latlong_to_pix_vp( rlat, rlon, rpixxd, rpixyd, GetVP() ) ) {
                r->x = rpixxd;
                r->y = rpixyd;
                bUseVP = false;
            }
        }
    }

    //    if needed, use the VPoint scaling estimator,
    if( bUseVP ) {
        wxPoint p = GetVP().GetPixFromLL( rlat, rlon );
        *r = p;
    }

}

void ChartCanvas::GetCanvasPixPoint( int x, int y, double &lat, double &lon )
{
    // If the Current Chart is a raster chart, and the
    // requested x,y is within the boundaries of the chart,
    // and the VP is not rotated,
    // then use the embedded BSB chart georeferencing algorithm
    // for greater accuracy
    // Additionally, use chart embedded georef if the projection is TMERC
    //  i.e. NOT MERCATOR and NOT POLYCONIC

    // If for some reason the chart rejects the request by returning an error,
    // then fall back to Viewport Projection  estimate from canvas parameters
    bool bUseVP = true;

    if( Current_Ch && ( Current_Ch->GetChartFamily() == CHART_FAMILY_RASTER )
            && ( ( ( fabs( GetVP().rotation ) < .01 ) && !g_bskew_comp )
                 || ( ( Current_Ch->GetChartProjectionType() != PROJECTION_MERCATOR )
                      && ( Current_Ch->GetChartProjectionType() != PROJECTION_POLYCONIC ) ) ) )

    {
        ChartBaseBSB *Cur_BSB_Ch = dynamic_cast<ChartBaseBSB *>( Current_Ch );

        // TODO     maybe need iterative process to validate bInside
        //          first pass is mercator, then check chart boundaries

        if( Cur_BSB_Ch ) {
            //    This is a Raster chart....
            //    If the VP is changing, the raster chart parameters may not yet be setup
            //    So do that before accessing the chart's embedded georeferencing
            Cur_BSB_Ch->SetVPRasterParms( GetVP() );

            double slat, slon;
            if( 0 == Cur_BSB_Ch->vp_pix_to_latlong( GetVP(), x, y, &slat, &slon ) ) {
                lat = slat;

                if( slon < -180. ) slon += 360.;
                else if( slon > 180. ) slon -= 360.;

                lon = slon;
                bUseVP = false;
            }
        }

    }

    //    if needed, use the VPoint scaling estimator
    if( bUseVP ) {
        GetVP().GetLLFromPix( wxPoint( x, y ), &lat, &lon );
    }
}

bool ChartCanvas::ZoomCanvasIn( double factor )
{
    bool b_smooth = g_bsmoothpanzoom & g_bopengl & !g_bEnableZoomToCursor;

    if( !VPoint.b_quilt ) {
        ChartBase *pc = Current_Ch;
        if( !pc ) return false;
        if( pc->GetChartFamily() == CHART_FAMILY_VECTOR ) b_smooth = false;
    } else
        b_smooth = g_bsmoothpanzoom & !m_pQuilt->IsQuiltVector() & !g_bEnableZoomToCursor;

    if( b_smooth ) {
        if( m_bzooming_out )             // Interrupt?
        {
            m_zoom_timer.Stop();
            m_bzooming_in = false;
            m_bzooming_out = false;
        }

        if( !m_bzooming_in ) {
            // Set up some parameters
            m_zoomt = 5;
            m_zoom_target_factor = factor;
            m_zoom_current_factor = 1.0;
            m_zoom_timer.Start( m_zoomt );             //, true);
            m_bzooming_in = true;
        } else        // Make sure timer is running, to recover from lost events
        {
            if( !m_zoom_timer.IsRunning() ) m_zoom_timer.Start( m_zoomt );
        }
    } else
        DoZoomCanvasIn( factor );

    extendedSectorLegs.clear();
    return true;
}

bool ChartCanvas::ZoomCanvasOut( double factor )
{
    bool b_smooth = g_bsmoothpanzoom & g_bopengl & !g_bEnableZoomToCursor;

    if( !VPoint.b_quilt ) {
        ChartBase *pc = Current_Ch;
        if( !pc ) return false;
        if( pc->GetChartFamily() == CHART_FAMILY_VECTOR ) b_smooth = false;
    } else
        b_smooth = g_bsmoothpanzoom & !m_pQuilt->IsQuiltVector() & !g_bEnableZoomToCursor;

    if( b_smooth ) {
        if( m_bzooming_in )             // Interrupt?
        {
            m_zoom_timer.Stop();
            m_bzooming_in = false;
            m_bzooming_out = false;
        }

        if( !m_bzooming_out ) {
            // Set up some parameters
            m_zoomt = 5;
            m_zoom_target_factor = factor;
            m_zoom_current_factor = 1.0;
            m_zoom_timer.Start( m_zoomt );             //, true);
            m_bzooming_out = true;
        } else        // Make sure timer is running, to recover from lost events
        {
            if( !m_zoom_timer.IsRunning() ) m_zoom_timer.Start( m_zoomt );
        }

    } else
        DoZoomCanvasOut( factor );

    extendedSectorLegs.clear();
    return true;
}

void ChartCanvas::OnZoomTimerEvent( wxTimerEvent &event )
{
    if( m_bzooming_in && !m_bzooming_out ) {
        if( m_zoom_current_factor < m_zoom_target_factor ) {
            DoZoomCanvasIn( 1.05 );
            m_zoom_current_factor *= 1.05;
            m_zoom_timer.Start( m_zoomt );        //, true);
        } else
            m_bzooming_in = false;
    } else if( m_bzooming_out && !m_bzooming_in ) {
        if( m_zoom_current_factor < m_zoom_target_factor ) {
            DoZoomCanvasOut( 1.05 );
            m_zoom_current_factor *= 1.05;
            m_zoom_timer.Start( m_zoomt );        //, true);
        } else
            m_bzooming_out = false;

        if( m_zoom_current_factor >= m_zoom_target_factor ) m_bzooming_out = false;
    } else if( m_bzooming_out && m_bzooming_in )      // incoherent, should never happen
    {
        m_zoom_timer.Stop();
        m_bzooming_out = false;
        m_bzooming_in = false;
    }
}

bool ChartCanvas::DoZoomCanvasIn( double factor )
{
    //    Cannot allow Yield() re-entrancy here
    if( m_bzooming ) return false;
    m_bzooming = true;

    bool b_do_zoom = true;

    double zoom_factor = factor;

    double min_allowed_scale = 50.0;                // meters per meter

    double proposed_scale_onscreen = GetCanvasScaleFactor() / ( GetVPScale() * zoom_factor );
    ChartBase *pc = NULL;

    if( !VPoint.b_quilt ) {
        pc = Current_Ch;
    } else {
        int new_db_index = m_pQuilt->AdjustRefOnZoomIn( proposed_scale_onscreen );
        if( new_db_index >= 0 ) pc = ChartData->OpenChartFromDB( new_db_index, FULL_INIT );

        pCurrentStack->SetCurrentEntryFromdbIndex( new_db_index ); // highlite the correct bar entry
    }

    if( pc ) {
        min_allowed_scale = pc->GetNormalScaleMin( GetCanvasScaleFactor(), g_b_overzoom_x );

        double target_scale_ppm = GetVPScale() * zoom_factor;
        double new_scale_ppm = target_scale_ppm; //pc->GetNearestPreferredScalePPM(target_scale_ppm);

        proposed_scale_onscreen = GetCanvasScaleFactor() / new_scale_ppm;

        //  Query the chart to determine the appropriate zoom range
        if( proposed_scale_onscreen < min_allowed_scale ) {
            if( min_allowed_scale == GetCanvasScaleFactor() / ( GetVPScale() ) ) b_do_zoom = false;
            else
                proposed_scale_onscreen = min_allowed_scale;
        }
    }

    if( b_do_zoom ) {
        SetVPScale( GetCanvasScaleFactor() / proposed_scale_onscreen );
        Refresh( false );
    }

    m_bzooming = false;

    return true;
}

bool ChartCanvas::DoZoomCanvasOut( double zoom_factor )
{
    if( m_bzooming ) return false;
    m_bzooming = true;

    bool b_do_zoom = true;

    double proposed_scale_onscreen = GetCanvasScaleFactor() / ( GetVPScale() / zoom_factor );
    ChartBase *pc = NULL;

    bool b_smallest = false;

    if( !VPoint.b_quilt ) {             // not quilted
        pc = Current_Ch;
        double target_scale_ppm = GetVPScale() / zoom_factor;
        double new_scale_ppm = target_scale_ppm;
        proposed_scale_onscreen = GetCanvasScaleFactor() / new_scale_ppm;

        //      If Current_Ch is not on the screen, unbound the zoomout
        LLBBox viewbox = VPoint.GetBBox();
        wxBoundingBox chart_box;
        int current_index = ChartData->FinddbIndex( pc->GetFullPath() );
        ChartData->GetDBBoundingBox( current_index, &chart_box );
        if( ( viewbox.Intersect( chart_box ) == _OUT ) ) {
            proposed_scale_onscreen = wxMin(proposed_scale_onscreen,
                                            GetCanvasScaleFactor() / m_absolute_min_scale_ppm);
        }
        else {
        //  Clamp the minimum scale zoom-out to the value specified by the chart
            double max_allowed_scale = 4.0 * ( pc->GetNormalScaleMax( GetCanvasScaleFactor(), GetCanvasWidth() ) );
            proposed_scale_onscreen = wxMin( proposed_scale_onscreen, max_allowed_scale );
        }

     } else {
        int new_db_index = m_pQuilt->AdjustRefOnZoomOut( proposed_scale_onscreen );
        if( new_db_index >= 0 ) pc = ChartData->OpenChartFromDB( new_db_index, FULL_INIT );

        pCurrentStack->SetCurrentEntryFromdbIndex( new_db_index ); // highlite the correct bar entry
        b_smallest = m_pQuilt->IsChartSmallestScale( new_db_index );

        double target_scale_ppm = GetVPScale() / zoom_factor;
        proposed_scale_onscreen = GetCanvasScaleFactor() / target_scale_ppm;

        if( b_smallest || (0 == m_pQuilt->GetExtendedStackCount()))
            proposed_scale_onscreen = wxMin(proposed_scale_onscreen,
                                            GetCanvasScaleFactor() / m_absolute_min_scale_ppm);
    }

    if( !pc ) {                         // no chart, so set a minimum scale
        if( ( GetCanvasScaleFactor() / proposed_scale_onscreen ) < m_absolute_min_scale_ppm ) b_do_zoom = false;
    }

    if( b_do_zoom ) {
        SetVPScale( GetCanvasScaleFactor() / proposed_scale_onscreen );
        Refresh( false );
    }

    m_bzooming = false;

    return true;
}

void ChartCanvas::ClearbFollow( void )
{
    m_bFollow = false;      // update the follow flag
    parent_frame->SetToolbarItemState( ID_FOLLOW, false );
}

bool ChartCanvas::PanCanvas( int dx, int dy )
{
    double dlat, dlon;
    wxPoint p;
//      CALLGRIND_START_INSTRUMENTATION

    extendedSectorLegs.clear();

    GetCanvasPointPix( GetVP().clat, GetVP().clon, &p );
    GetCanvasPixPoint( p.x + dx, p.y + dy, dlat, dlon );

    if( dlon > 360. ) dlon -= 360.;
    if( dlon < -360. ) dlon += 360.;

    //    This should not really be necessary, but round-trip georef on some charts is not perfect,
    //    So we can get creep on repeated unidimensional pans, and corrupt chart cacheing.......

    //    But this only works on north-up projections
    if( ( ( fabs( GetVP().skew ) < .001 ) ) && ( fabs( GetVP().rotation ) < .001 ) ) {

        if( dx == 0 ) dlon = GetVP().clon;
        if( dy == 0 ) dlat = GetVP().clat;
    }

    int cur_ref_dbIndex = m_pQuilt->GetRefChartdbIndex();
    SetViewPoint( dlat, dlon, VPoint.view_scale_ppm, VPoint.skew, VPoint.rotation );

//      vLat = dlat;
//      vLon = dlon;

    if( VPoint.b_quilt ) {
        int new_ref_dbIndex = m_pQuilt->GetRefChartdbIndex();
        if( ( new_ref_dbIndex != cur_ref_dbIndex ) && ( new_ref_dbIndex != -1 ) ) {
            //Tweak the scale slightly for a new ref chart
            ChartBase *pc = ChartData->OpenChartFromDB( new_ref_dbIndex, FULL_INIT );
            if( pc ) {
                double tweak_scale_ppm = pc->GetNearestPreferredScalePPM( VPoint.view_scale_ppm );
                SetVPScale( tweak_scale_ppm );
            }
        }
    }

    ClearbFollow();      // update the follow flag

    Refresh( false );

    Update();               // Force an immediate screen update
    // to be sure screen stays in sync with (fast) smooth panning
    // on truly asynchronous opengl renderers.

    return true;
}

void ChartCanvas::ReloadVP( bool b_adjust )
{
    if( g_brightness_init ) SetScreenBrightness( g_nbrightness );

    LoadVP( VPoint, b_adjust );
}

void ChartCanvas::LoadVP( ViewPort &vp, bool b_adjust )
{
    if( g_bopengl ) {
        m_glcc->Invalidate();
        if( m_glcc->GetSize().x != VPoint.pix_width || m_glcc->GetSize().y != VPoint.pix_height ) m_glcc->SetSize(
                VPoint.pix_width, VPoint.pix_height );
    } else {
        m_cache_vp.Invalidate();
        m_bm_cache_vp.Invalidate();
    }

    VPoint.Invalidate();

    if( m_pQuilt ) m_pQuilt->Invalidate();

    SetViewPoint( vp.clat, vp.clon, vp.view_scale_ppm, vp.skew, vp.rotation, b_adjust );

}

void ChartCanvas::SetQuiltRefChart( int dbIndex )
{
    m_pQuilt->SetReferenceChart( dbIndex );
    VPoint.Invalidate();
    m_pQuilt->Invalidate();
}

void ChartCanvas::UpdateCanvasOnGroupChange( void )
{
    delete pCurrentStack;
    pCurrentStack = NULL;
    pCurrentStack = new ChartStack;
    ChartData->BuildChartStack( pCurrentStack, VPoint.clat, VPoint.clon );

    if( m_pQuilt ) {
        m_pQuilt->Compose( VPoint );
    }
}

bool ChartCanvas::SetVPScale( double scale )
{
    return SetViewPoint( VPoint.clat, VPoint.clon, scale, VPoint.skew, VPoint.rotation );
}

bool ChartCanvas::SetViewPoint( double lat, double lon )
{
    return SetViewPoint( lat, lon, VPoint.view_scale_ppm, VPoint.skew, VPoint.rotation );
}

bool ChartCanvas::SetViewPoint( double lat, double lon, double scale_ppm, double skew,
                                double rotation, bool b_adjust )
{
    bool b_ret = false;

    //  Any sensible change?
    if( ( fabs( VPoint.view_scale_ppm - scale_ppm ) < 1e-9 )
            && ( fabs( VPoint.skew - skew ) < 1e-9 )
            && ( fabs( VPoint.rotation - rotation ) < 1e-9 ) && ( fabs( VPoint.clat - lat ) < 1e-9 )
            && ( fabs( VPoint.clon - lon ) < 1e-9 ) && VPoint.IsValid() ) return false;

    VPoint.SetProjectionType( PROJECTION_MERCATOR );            // default

    VPoint.Validate();                     // Mark this ViewPoint as OK

    //    Take a local copy of the last viewport
    ViewPort last_vp = VPoint;

    VPoint.skew = skew;
    VPoint.clat = lat;
    VPoint.clon = lon;
    VPoint.view_scale_ppm = scale_ppm;
    VPoint.rotation = rotation;

    if( ( VPoint.pix_width <= 0 ) || ( VPoint.pix_height <= 0 ) )    // Canvas parameters not yet set
        return false;

    //  Has the Viewport scale changed?  If so, invalidate the vp describing the cached bitmap
    if( last_vp.view_scale_ppm != scale_ppm ) {
        m_cache_vp.Invalidate();

        if( g_bopengl ) m_glcc->Invalidate();
    }

    //  A preliminary value, may be tweaked below
    VPoint.chart_scale = m_canvas_scale_factor / ( scale_ppm );

    if( !VPoint.b_quilt && Current_Ch ) {

        VPoint.SetProjectionType( Current_Ch->GetChartProjectionType() );
        VPoint.SetBoxes();

        //  Allow the chart to adjust the new ViewPort for performance optimization
        //  This will normally be only a fractional (i.e.sub-pixel) adjustment...
        if( b_adjust ) Current_Ch->AdjustVP( last_vp, VPoint );

        // If there is a sensible change in the chart render, refresh the whole screen
        if( ( !m_cache_vp.IsValid() ) || ( m_cache_vp.view_scale_ppm != VPoint.view_scale_ppm ) ) {
            Refresh( false );
            b_ret = true;
        } else {
            wxPoint cp_last, cp_this;
            GetCanvasPointPix( m_cache_vp.clat, m_cache_vp.clon, &cp_last );
            GetCanvasPointPix( VPoint.clat, VPoint.clon, &cp_this );

            if( cp_last != cp_this ) {
                Refresh( false );
                b_ret = true;
            }
        }
    }

    //  Handle the quilted case
    if( VPoint.b_quilt ) {

        if( last_vp.view_scale_ppm != scale_ppm ) m_pQuilt->InvalidateAllQuiltPatchs();

        //  Create the quilt
        if( ChartData && ChartData->IsValid() ) {
            if( !pCurrentStack ) return false;

            int current_db_index = -1;
            current_db_index = pCurrentStack->GetCurrentEntrydbIndex();       // capture the current

            ChartData->BuildChartStack( pCurrentStack, lat, lon );
            pCurrentStack->SetCurrentEntryFromdbIndex( current_db_index );

            //   Check to see if the current quilt reference chart is in the new stack
            int current_ref_stack_index = -1;
            for( int i = 0; i < pCurrentStack->nEntry; i++ ) {
                if( m_pQuilt->GetRefChartdbIndex() == pCurrentStack->GetDBIndex( i ) ) current_ref_stack_index =
                        i;
            }

            if( g_bFullScreenQuilt ) {
                current_ref_stack_index = m_pQuilt->GetRefChartdbIndex();
            }

            //    If the new stack does not contain the current ref chart....
            if( ( -1 == current_ref_stack_index ) && ( m_pQuilt->GetRefChartdbIndex() >= 0 ) ) {
                const ChartTableEntry &cte_ref = ChartData->GetChartTableEntry(
                                                     m_pQuilt->GetRefChartdbIndex() );
                int target_scale = cte_ref.GetScale();
                int target_type = cte_ref.GetChartType();
                int candidate_stack_index;

                //    reset the ref chart in a way that does not lead to excessive underzoom, for performance reasons
                //    Try to find a chart that is the same type, and has a scale of just smaller than the current ref chart

                candidate_stack_index = 0;
                while( candidate_stack_index <= pCurrentStack->nEntry - 1 ) {
                    const ChartTableEntry &cte_candidate = ChartData->GetChartTableEntry(
                            pCurrentStack->GetDBIndex( candidate_stack_index ) );
                    int candidate_scale = cte_candidate.GetScale();
                    int candidate_type = cte_candidate.GetChartType();

                    if( ( candidate_scale >= target_scale ) && ( candidate_type == target_type ) ) break;

                    candidate_stack_index++;
                }

                //    If that did not work, look for a chart of just larger scale and same type
                if( candidate_stack_index >= pCurrentStack->nEntry ) {
                    candidate_stack_index = pCurrentStack->nEntry - 1;
                    while( candidate_stack_index >= 0 ) {
                        const ChartTableEntry &cte_candidate = ChartData->GetChartTableEntry(
                                pCurrentStack->GetDBIndex( candidate_stack_index ) );
                        int candidate_scale = cte_candidate.GetScale();
                        int candidate_type = cte_candidate.GetChartType();

                        if( ( candidate_scale <= target_scale )
                                && ( candidate_type == target_type ) ) break;

                        candidate_stack_index--;
                    }
                }

                // and if that did not work, chose stack entry 0
                if( ( candidate_stack_index >= pCurrentStack->nEntry )
                        || ( candidate_stack_index < 0 ) ) candidate_stack_index = 0;

                int new_ref_index = pCurrentStack->GetDBIndex( candidate_stack_index );

                m_pQuilt->SetReferenceChart( new_ref_index ); //maybe???

            }


            // Preset the VPoint projection type to match what the quilt projection type will be
            int ref_db_index = m_pQuilt->GetRefChartdbIndex();
            int proj = ChartData->GetDBChartProj( ref_db_index );

            // Always keep the default Mercator projection if the reference chart is
            // not in the PatchList or the scale is too small for it to render.

            bool renderable = true;
            ChartBase* referenceChart = ChartData->OpenChartFromDB( ref_db_index, FULL_INIT );
            if( referenceChart ) {
                double chartMaxScale = referenceChart->GetNormalScaleMax( cc1->GetCanvasScaleFactor(), cc1->GetCanvasWidth() );
                renderable = chartMaxScale*1.5 > VPoint.chart_scale;
            }

            VPoint.b_MercatorProjectionOverride = ( m_pQuilt->GetnCharts() == 0 || !renderable );

            if( ! VPoint.b_MercatorProjectionOverride ) VPoint.SetProjectionType( proj );

            VPoint.SetBoxes();

            //    If this quilt will be a perceptible delta from the existing quilt, then refresh the entire screen
            if( m_pQuilt->IsQuiltDelta( VPoint ) ) {
                //  Allow the quilt to adjust the new ViewPort for performance optimization
                //  This will normally be only a fractional (i.e. sub-pixel) adjustment...
                if( b_adjust ) m_pQuilt->AdjustQuiltVP( last_vp, VPoint );

                ChartData->ClearCacheInUseFlags();
                unsigned long hash1 = m_pQuilt->GetXStackHash();
                m_pQuilt->Compose( VPoint );

                //      If the extended chart stack has changed, invalidate any cached render bitmap

                if(m_pQuilt->GetXStackHash() != hash1) {
                    m_bm_cache_vp.Invalidate();
                    if(g_bopengl)
                        m_glcc->Invalidate();
                }

                ChartData->UnLockCache();
                ChartData->PurgeCacheUnusedCharts( false );
                ChartData->LockCache();

                Refresh( false );
                b_ret = true;
            }
            parent_frame->UpdateControlBar();
        }

        VPoint.skew = 0.;  // Quilting supports 0 Skew
    }

    if( !VPoint.GetBBox().GetValid() ) VPoint.SetBoxes();

    if( VPoint.GetBBox().GetValid() ) {

        //    Calculate the on-screen displayed actual scale
        //    by a simple traverse northward from the center point
        //    of roughly 10 % of the Viewport extent
        double tlat, tlon;
        wxPoint r, r1;
        double delta_y = ( VPoint.GetBBox().GetMaxY() - VPoint.GetBBox().GetMinY() ) * 60.0 * .10; // roughly 10 % of lat range, in NM

        //  Make sure the two points are in phase longitudinally
        double lon_norm = VPoint.clon;
        if( lon_norm > 180. ) lon_norm -= 360;
        else if( lon_norm < -180. ) lon_norm += 360.;

        ll_gc_ll( VPoint.clat, lon_norm, 0, delta_y, &tlat, &tlon );

        GetCanvasPointPix( tlat, tlon, &r1 );
        GetCanvasPointPix( VPoint.clat, lon_norm, &r );

        m_true_scale_ppm = sqrt(
                               pow( (double) ( r.y - r1.y ), 2 ) + pow( (double) ( r.x - r1.x ), 2 ) )
                           / ( delta_y * 1852. );

        //        A fall back in case of very high zoom-out, giving delta_y == 0
        //        which can probably only happen with vector charts
        if( 0.0 == m_true_scale_ppm ) m_true_scale_ppm = scale_ppm;

        //        Another fallback, for highly zoomed out charts
        //        This adjustment makes the displayed TrueScale correspond to the
        //        same algorithm used to calculate the chart zoom-out limit for ChartDummy.
        if( scale_ppm < 1e-4 ) m_true_scale_ppm = scale_ppm;

        if( m_true_scale_ppm ) VPoint.chart_scale = m_canvas_scale_factor / ( m_true_scale_ppm );
        else
            VPoint.chart_scale = 1.0;

        if( parent_frame->m_pStatusBar ) {
            double true_scale_display = floor( VPoint.chart_scale / 100. ) * 100.;
            wxString text;

            if( Current_Ch ) {
                double chart_native_ppm = m_canvas_scale_factor / Current_Ch->GetNativeScale();
                double scale_factor = scale_ppm / chart_native_ppm;
                if( scale_factor > 1.0 ) text.Printf( _("Scale %4.0f (%1.1fx)"),
                                                          true_scale_display, scale_factor );
                else
                    text.Printf( _("Scale %4.0f (%1.2fx)"), true_scale_display,
                                 scale_factor );
            } else
                text.Printf( _("Scale %4.0f"), true_scale_display );

            parent_frame->SetStatusText( text, STAT_FIELD_SCALE );
        }
    }

    //  Maintain global vLat/vLon
    vLat = VPoint.clat;
    vLon = VPoint.clon;

    return b_ret;
}

//          Static Icon definitions for some symbols requiring scaling/rotation/translation
//          Very specific wxDC draw commands are necessary to properly render these icons...See the code in ShipDraw()

//      This icon was adapted and scaled from the S52 Presentation Library version 3_03.
//     Symbol VECGND02

static int s_png_pred_icon[] = { -10, -10, -10, 10, 10, 10, 10, -10 };

//      This ownship icon was adapted and scaled from the S52 Presentation Library version 3_03
//      Symbol OWNSHP05
static int s_ownship_icon[] = { 5, -42, 11, -28, 11, 42, -11, 42, -11, -28, -5, -42, -11, 0, 11, 0,
                                0, 42, 0, -42
                              };

wxPoint transrot( wxPoint pt, double theta, wxPoint offset )
{
    wxPoint ret;
    double px = (double) ( pt.x * sin( theta ) ) + (double) ( pt.y * cos( theta ) );
    double py = (double) ( pt.y * sin( theta ) ) - (double) ( pt.x * cos( theta ) );
    ret.x = (int) wxRound( px );
    ret.y = (int) wxRound( py );
    ret.x += offset.x;
    ret.y += offset.y;

    return ret;
}

void ChartCanvas::ShipDraw( ocpnDC& dc )
{
    if( !GetVP().IsValid() ) return;
    int drawit = 0;
    wxPoint lGPSPoint, lShipMidPoint, lPredPoint, lHeadPoint, GPSOffsetPixels(0,0);

//    Is ship in Vpoint?
    if( GetVP().GetBBox().PointInBox( gLon, gLat, 0 ) ) drawit++;                             // yep

//    Calculate ownship Position Predictor

    double pred_lat, pred_lon;

    //  COG/SOG may be undefined in NMEA data stream
    double pCog = gCog;
    if( wxIsNaN(pCog) ) pCog = 0.0;
    double pSog = gSog;
    if( wxIsNaN(pSog) ) pSog = 0.0;

    ll_gc_ll( gLat, gLon, pCog, pSog * g_ownship_predictor_minutes / 60., &pred_lat, &pred_lon );

    GetCanvasPointPix( gLat, gLon, &lGPSPoint );
    lShipMidPoint = lGPSPoint;
    GetCanvasPointPix( pred_lat, pred_lon, &lPredPoint );

    double cog_rad = atan2( (double) ( lPredPoint.y - lShipMidPoint.y ),
                            (double) ( lPredPoint.x - lShipMidPoint.x ) );
    cog_rad += PI;

    double lpp = sqrt(
                     pow( (double) ( lPredPoint.x - lShipMidPoint.x ), 2 )
                     + pow( (double) ( lPredPoint.y - lShipMidPoint.y ), 2 ) );

//    Is predicted point in the VPoint?
    if( GetVP().GetBBox().PointInBox( pred_lon, pred_lat, 0 ) ) drawit++;                     // yep

    //  Draw the icon rotated to the COG
    //  or to the Hdt if available
    double icon_hdt = pCog;
    if( g_bHDTValid ) icon_hdt = gHdt;

    //  COG may be undefined in NMEA data stream
    if( wxIsNaN(icon_hdt) ) icon_hdt = 0.0;

//    Calculate the ownship drawing angle icon_rad using an assumed 10 minute predictor
    double osd_head_lat, osd_head_lon;
    wxPoint osd_head_point;

    ll_gc_ll( gLat, gLon, icon_hdt, pSog * 10. / 60., &osd_head_lat, &osd_head_lon );

    GetCanvasPointPix( gLat, gLon, &lShipMidPoint );
    GetCanvasPointPix( osd_head_lat, osd_head_lon, &osd_head_point );

    double icon_rad = atan2( (double) ( osd_head_point.y - lShipMidPoint.y ),
                             (double) ( osd_head_point.x - lShipMidPoint.x ) );
    icon_rad += PI;

    if( pSog < 0.2 ) icon_rad = ( ( icon_hdt + 90. ) * PI / 180. ) + GetVP().rotation;

//    Calculate ownship Heading pointer as a predictor
    double hdg_pred_lat, hdg_pred_lon;

    ll_gc_ll( gLat, gLon, icon_hdt, pSog * g_ownship_predictor_minutes / 60., &hdg_pred_lat,
              &hdg_pred_lon );

    GetCanvasPointPix( gLat, gLon, &lShipMidPoint );
    GetCanvasPointPix( hdg_pred_lat, hdg_pred_lon, &lHeadPoint );

//    Should we draw the Head vector?
//    Compare the points lHeadPoint and lPredPoint
//    If they differ by more than n pixels, and the head vector is valid, then render the head vector

    double ndelta_pix = 10.;
    bool b_render_hdt = false;
    if( g_bHDTValid ) {
        double dist = sqrt(
                          pow( (double) ( lHeadPoint.x - lPredPoint.x ), 2 )
                          + pow( (double) ( lHeadPoint.y - lPredPoint.y ), 2 ) );
        if( dist > ndelta_pix ) b_render_hdt = true;
    }

//    Another draw test ,based on pixels, assuming the ship icon is a fixed nominal size
//    and is just barely outside the viewport        ....
    wxBoundingBox bb_screen( 0, 0, GetVP().pix_width, GetVP().pix_height );
    if( bb_screen.PointInBox( lShipMidPoint, 20 ) ) drawit++;

    // And one more test to catch the case where COG line crosses the screen,
    // but ownship and pred point are both off

    if( GetVP().GetBBox().LineIntersect( wxPoint2DDouble( gLon, gLat ),
                                         wxPoint2DDouble( pred_lon, pred_lat ) ) ) drawit++;

//    Do the draw if either the ship or prediction is within the current VPoint
    if( drawit ) {
        int img_height;

        wxColour pred_colour;
        pred_colour = GetGlobalColor( _T ( "URED" ) );
        if( SHIP_NORMAL != m_ownship_state ) pred_colour = GetGlobalColor( _T ( "GREY1" ) );

        //      Establish ship color
        //     It changes color based on GPS and Chart accuracy/availability
        wxColour ship_color( GetGlobalColor( _T ( "URED" ) ) );         // default is OK

        if( SHIP_NORMAL != m_ownship_state ) ship_color = GetGlobalColor( _T ( "GREY1" ) );

        if( SHIP_LOWACCURACY == m_ownship_state ) ship_color = GetGlobalColor( _T ( "YELO1" ) );

        if( GetVP().chart_scale > 300000 )             // According to S52, this should be 50,000
        {
            dc.SetPen( wxPen( pred_colour, 2 ) );

            if( SHIP_NORMAL == m_ownship_state ) dc.SetBrush(
                    wxBrush( ship_color, wxTRANSPARENT ) );
            else
                dc.SetBrush( wxBrush( GetGlobalColor( _T ( "YELO1" ) ) ) );

            dc.DrawEllipse( lShipMidPoint.x - 10, lShipMidPoint.y - 10, 20, 20 );
            dc.DrawEllipse( lShipMidPoint.x - 6, lShipMidPoint.y - 6, 12, 12 );

            dc.DrawLine( lShipMidPoint.x - 12, lShipMidPoint.y, lShipMidPoint.x + 12, lShipMidPoint.y );
            dc.DrawLine( lShipMidPoint.x, lShipMidPoint.y - 12, lShipMidPoint.x, lShipMidPoint.y + 12 );
            img_height = 20;
        } else {
            double screenResolution = (double) ::wxGetDisplaySize().y / ::wxGetDisplaySizeMM().y;

            wxImage pos_image;
            pos_image = m_pos_image_red->Copy();
            if( SHIP_NORMAL != m_ownship_state ) pos_image = m_pos_image_grey->Copy();

            //      Substitute user ownship image if found
            if( m_pos_image_user ) {
                pos_image = m_pos_image_user->Copy();
                if( SHIP_NORMAL != m_ownship_state ) pos_image = m_pos_image_user_grey->Copy();
            }

            if( g_n_ownship_beam_meters > 0.0 && g_n_ownship_length_meters > 0.0 && g_OwnShipIconType > 0 ) // use large ship
            {
                int ownShipWidth = 22; // Default values from s_ownship_icon
                int ownShipLength= 84;

                if( g_OwnShipIconType == 1 ) {
                    ownShipWidth = pos_image.GetWidth();
                    ownShipLength= pos_image.GetHeight();
                }

                //  Calculate the true ship length in exact pixels
                double ship_bow_lat, ship_bow_lon;
                ll_gc_ll( gLat, gLon, icon_hdt, g_n_ownship_length_meters / 1852., &ship_bow_lat,
                          &ship_bow_lon );
                wxPoint lShipBowPoint;
                wxPoint2DDouble b_point = GetVP().GetDoublePixFromLL( ship_bow_lat, ship_bow_lon );
                wxPoint2DDouble a_point = GetVP().GetDoublePixFromLL( gLat, gLon );

                double shipLength_px = sqrt( pow( (double) ( b_point.m_x - a_point.m_x ), 2 )
                                           + pow( (double) ( b_point.m_y - a_point.m_y ), 2 ) );

                //  And in mm
                double shipLength_mm = shipLength_px / screenResolution;

                //  Set minimum ownship drawing size
                double ownship_min_mm = g_n_ownship_min_mm;
                ownship_min_mm = wxMax(ownship_min_mm, 2.0);

                //  Calculate Nautical Miles distance from midships to gps antenna
                double hdt_ant = icon_hdt + 180.;
                double dy = ( g_n_ownship_length_meters / 2 - g_n_gps_antenna_offset_y ) / 1852.;
                double dx = g_n_gps_antenna_offset_x / 1852.;
                if( g_n_gps_antenna_offset_y > g_n_ownship_length_meters / 2 )      //reverse?
                {
                    hdt_ant = icon_hdt;
                    dy = -dy;
                }

                //  If the drawn ship size is going to be clamped, adjust the gps antenna offsets
                if( shipLength_mm < ownship_min_mm ) {
                    dy /= shipLength_mm / ownship_min_mm;
                    dx /= shipLength_mm / ownship_min_mm;
                }

                double ship_mid_lat, ship_mid_lon, ship_mid_lat1, ship_mid_lon1;

                ll_gc_ll( gLat, gLon, hdt_ant, dy, &ship_mid_lat, &ship_mid_lon );
                ll_gc_ll( ship_mid_lat, ship_mid_lon, icon_hdt - 90., dx, &ship_mid_lat1, &ship_mid_lon1 );

                GetCanvasPointPix( ship_mid_lat1, ship_mid_lon1, &lShipMidPoint );
                GPSOffsetPixels.x = lShipMidPoint.x - lGPSPoint.x;
                GPSOffsetPixels.y = lShipMidPoint.y - lGPSPoint.y;

                double scale_factor = shipLength_px / ownShipLength;

                //  Calculate a scale factor that would produce a reasonably sized icon
                double scale_factor_min = ownship_min_mm / ( ownShipLength / screenResolution );

                //  And choose the correct one
                scale_factor = wxMax(scale_factor, scale_factor_min);

                double scale_factor_y = scale_factor;
                double scale_factor_x = scale_factor_y * ( (double) ownShipLength / ownShipWidth )
                                        / ( (double) g_n_ownship_length_meters / g_n_ownship_beam_meters );

                if( g_OwnShipIconType == 1 ) { // Scaled bitmap
                    pos_image.Rescale( ownShipWidth * scale_factor_x, ownShipLength * scale_factor_y,
                            wxIMAGE_QUALITY_HIGH );
                    wxPoint rot_ctr( pos_image.GetWidth() / 2, pos_image.GetHeight() / 2 );
                    wxImage rot_image = pos_image.Rotate( -( icon_rad - ( PI / 2. ) ), rot_ctr, true );

                    // Simple sharpening algorithm.....
                    for( int ip = 0; ip < rot_image.GetWidth(); ip++ )
                        for( int jp = 0; jp < rot_image.GetHeight(); jp++ )
                            if( rot_image.GetAlpha( ip, jp ) > 64 ) rot_image.SetAlpha( ip, jp, 255 );

                    wxBitmap os_bm( rot_image );

                    int w = os_bm.GetWidth();
                    int h = os_bm.GetHeight();
                    img_height = h;

                    dc.DrawBitmap( os_bm, lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2, true );

                    // Maintain dirty box,, missing in __WXMSW__ library
                    dc.CalcBoundingBox( lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2 );
                    dc.CalcBoundingBox( lShipMidPoint.x - w / 2 + w, lShipMidPoint.y - h / 2 + h );
                }

                else if( g_OwnShipIconType == 2 ) { // Scaled Vector
                    wxPoint ownship_icon[10];

                    for( int i = 0; i < 10; i++ ) {
                        int j = i * 2;
                        double pxa = (double) ( s_ownship_icon[j] );
                        double pya = (double) ( s_ownship_icon[j + 1] );
                        pya *= scale_factor_y;
                        pxa *= scale_factor_x;

                        double px = ( pxa * sin( icon_rad ) ) + ( pya * cos( icon_rad ) );
                        double py = ( pya * sin( icon_rad ) ) - ( pxa * cos( icon_rad ) );

                        ownship_icon[i].x = (int) ( px ) + lShipMidPoint.x;
                        ownship_icon[i].y = (int) ( py ) + lShipMidPoint.y;
                    }

                    wxPen ppPen1( GetGlobalColor( _T ( "UBLCK" ) ), 1, wxSOLID );
                    dc.SetPen( ppPen1 );
                    dc.SetBrush( wxBrush( ship_color ) );

                    dc.StrokePolygon( 6, &ownship_icon[0], 0, 0 );

                    //     draw reference point (midships) cross
                    dc.StrokeLine( ownship_icon[6].x, ownship_icon[6].y, ownship_icon[7].x,
                                 ownship_icon[7].y );
                    dc.StrokeLine( ownship_icon[8].x, ownship_icon[8].y, ownship_icon[9].x,
                                 ownship_icon[9].y );
                }

                img_height = ownShipLength * scale_factor_y;

                //      Reference point, where the GPS antenna is
                int circle_rad = 3;
                if( m_pos_image_user ) circle_rad = 1;

                dc.SetPen( wxPen( GetGlobalColor( _T ( "UBLCK" ) ), 1 ) );
                dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UIBCK" ) ) ) );
                dc.StrokeCircle( lGPSPoint.x, lGPSPoint.y, circle_rad );
            }
            else { // Fixed bitmap icon.
                wxPoint rot_ctr( pos_image.GetWidth() / 2, pos_image.GetHeight() / 2 );
                wxImage rot_image = pos_image.Rotate( -( icon_rad - ( PI / 2. ) ), rot_ctr, true );

                // Simple sharpening algorithm.....
                for( int ip = 0; ip < rot_image.GetWidth(); ip++ )
                    for( int jp = 0; jp < rot_image.GetHeight(); jp++ )
                        if( rot_image.GetAlpha( ip, jp ) > 64 ) rot_image.SetAlpha( ip, jp, 255 );

                wxBitmap os_bm( rot_image );

                int w = os_bm.GetWidth();
                int h = os_bm.GetHeight();
                img_height = h;

                dc.DrawBitmap( os_bm, lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2, true );

                //      Reference point, where the GPS antenna is
                int circle_rad = 3;
                if( m_pos_image_user ) circle_rad = 1;

                dc.SetPen( wxPen( GetGlobalColor( _T ( "UBLCK" ) ), 1 ) );
                dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UIBCK" ) ) ) );
                dc.StrokeCircle( lShipMidPoint.x, lShipMidPoint.y, circle_rad );

                // Maintain dirty box,, missing in __WXMSW__ library
                dc.CalcBoundingBox( lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2 );
                dc.CalcBoundingBox( lShipMidPoint.x - w / 2 + w, lShipMidPoint.y - h / 2 + h );
            }
        }         // ownship draw

        // draw course over ground if they are longer than the ship
        if( lpp >= img_height / 2 ) {
            const double png_pred_icon_scale_factor = .4;
            wxPoint icon[4];

            for( int i = 0; i < 4; i++ ) {
                int j = i * 2;
                double pxa = (double) ( s_png_pred_icon[j] );
                double pya = (double) ( s_png_pred_icon[j + 1] );

                pya *= png_pred_icon_scale_factor;
                pxa *= png_pred_icon_scale_factor;

                double px = ( pxa * sin( cog_rad ) ) + ( pya * cos( cog_rad ) );
                double py = ( pya * sin( cog_rad ) ) - ( pxa * cos( cog_rad ) );

                icon[i].x = (int) wxRound( px ) + lPredPoint.x + GPSOffsetPixels.x;
                icon[i].y = (int) wxRound( py ) + lPredPoint.y + GPSOffsetPixels.y;
            }

            //      COG Predictor
            wxDash dash_long[2];
            dash_long[0] = (int) ( 3.0 * m_pix_per_mm );  //8// Long dash  <---------+
            dash_long[1] = (int) ( 1.5 * m_pix_per_mm );  //2// Short gap            |

            //       If COG is unknown, render the predictor in grey
            if( wxIsNaN(gCog) ) pred_colour = GetGlobalColor( _T ( "GREY1" ) );

            wxPen ppPen2( pred_colour, 3, wxUSER_DASH );
            ppPen2.SetDashes( 2, dash_long );
            dc.SetPen( ppPen2 );
            dc.StrokeLine( lGPSPoint.x + GPSOffsetPixels.x, lGPSPoint.y + GPSOffsetPixels.y,
                           lPredPoint.x + GPSOffsetPixels.x, lPredPoint.y + GPSOffsetPixels.y );

            wxDash dash_long3[2];
            dash_long3[0] = 3 * dash_long[0];
            dash_long3[1] = 3 * dash_long[1];

            wxPen ppPen3( GetGlobalColor( _T ( "UBLCK" ) ), 1, wxUSER_DASH );
            ppPen3.SetDashes( 2, dash_long3 );
            dc.SetPen( ppPen3 );
            dc.StrokeLine( lGPSPoint.x + GPSOffsetPixels.x, lGPSPoint.y + GPSOffsetPixels.y,
                           lPredPoint.x + GPSOffsetPixels.x, lPredPoint.y + GPSOffsetPixels.y );

            wxPen ppPen1( GetGlobalColor( _T ( "UBLCK" ) ), 1, wxSOLID );
            dc.SetPen( ppPen1 );
            dc.SetBrush( wxBrush( pred_colour ) ); //*wxWHITE_BRUSH);

            dc.StrokePolygon( 4, icon );
        }

        //      HDT Predictor
        if( b_render_hdt ) {
            wxDash dash_short[2];
            dash_short[0] = (int) ( 1.5 * m_pix_per_mm );  // Short dash  <---------+
            dash_short[1] = (int) ( 1.8 * m_pix_per_mm );  // Short gap            |

            wxPen ppPen2( pred_colour, 1, wxUSER_DASH );
            ppPen2.SetDashes( 2, dash_short );

            dc.SetPen( ppPen2 );
            dc.StrokeLine( lGPSPoint.x + GPSOffsetPixels.x, lGPSPoint.y + GPSOffsetPixels.y,
                           lHeadPoint.x + GPSOffsetPixels.x, lHeadPoint.y + GPSOffsetPixels.y );

            wxPen ppPen1( pred_colour, 2, wxSOLID );
            dc.SetPen( ppPen1 );
            dc.SetBrush( wxBrush( GetGlobalColor( _T ( "GREY2" ) ) ) );

            dc.StrokeCircle( lHeadPoint.x + GPSOffsetPixels.x, lHeadPoint.y + GPSOffsetPixels.y, 4 );
        }

        // Draw radar rings if activated
        if( g_iNavAidRadarRingsNumberVisible ) {
            double factor = 1.00;
            if( g_pNavAidRadarRingsStepUnits == 1 )          // nautical miles
                factor = 1 / 1.852;

            factor *= g_fNavAidRadarRingsStep;

            double tlat, tlon;
            wxPoint r;
            ll_gc_ll( gLat, gLon, 0, factor, &tlat, &tlon );
            GetCanvasPointPix( tlat, tlon, &r );

            double lpp = sqrt(
                             pow( (double) ( lShipMidPoint.x - r.x ), 2 )
                             + pow( (double) ( lShipMidPoint.y - r.y ), 2 ) );
            int pix_radius = (int) lpp;

            wxPen ppPen1( GetGlobalColor( _T ( "URED" ) ), 2 );
            dc.SetPen( ppPen1 );
            dc.SetBrush( wxBrush( GetGlobalColor( _T ( "URED" ) ), wxTRANSPARENT ) );

            for( int i = 1; i <= g_iNavAidRadarRingsNumberVisible; i++ )
                dc.StrokeCircle( lShipMidPoint.x, lShipMidPoint.y, i * pix_radius );
        }
    }         // if drawit
}

/* @ChartCanvas::CalcGridSpacing
 **
 ** Calculate the major and minor spacing between the lat/lon grid
 **
 ** @param [r] WindowDegrees [double] displayed number of lat or lan in the window
 ** @param [w] MajorSpacing [double &] Major distance between grid lines
 ** @param [w] MinorSpacing [double &] Minor distance between grid lines
 ** @return [void]
 */
void CalcGridSpacing( double WindowDegrees, double& MajorSpacing, double&MinorSpacing )
{
    int tabi; // iterator for lltab

    // table for calculating the distance between the grids
    // [0] width or height of the displayed chart in degrees
    // [1] spacing between major grid liones in degrees
    // [2] spacing between minor grid lines in degrees
    const double lltab[][3] = { { 180.0, 90.0, 30.0 }, { 90.0, 45.0, 15.0 }, { 60.0, 30.0, 10.0 }, {
            20.0, 10.0, 2.0
        }, { 10.0, 5.0, 1.0 }, { 4.0, 2.0, 30.0 / 60.0 }, {
            2.0, 1.0, 20.0
            / 60.0
        }, { 1.0, 0.5, 10.0 / 60.0 }, { 30.0 / 60.0, 15.0 / 60.0, 5.0 / 60.0 }, {
            20.0
            / 60.0, 10.0 / 60.0, 2.0 / 60.0
        }, { 10.0 / 60.0, 5.0 / 60.0, 1.0 / 60.0 }, {
            4.0
            / 60.0, 2.0 / 60.0, 0.5 / 60.0
        }, { 2.0 / 60.0, 1.0 / 60.0, 0.2 / 60.0 }, {
            1.0 / 60.0,
            0.5 / 60.0, 0.1 / 60.0
        }, { 0.4 / 60.0, 0.2 / 60.0, 0.05 / 60.0 }, {
            0.0, 0.1 / 60.0,
            0.02 / 60.0
        } // indicates last entry
    };

    for( tabi = 0; lltab[tabi][0] != 0.0; tabi++ ) {
        if( WindowDegrees > lltab[tabi][0] ) {
            break;
        }
    }
    MajorSpacing = lltab[tabi][1]; // major latitude distance
    MinorSpacing = lltab[tabi][2]; // minor latitude distance
    return;
}
/* @ChartCanvas::CalcGridText *************************************
 **
 ** Calculates text to display at the major grid lines
 **
 ** @param [r] latlon [double] latitude or longitude of grid line
 ** @param [r] spacing [double] distance between two major grid lines
 ** @param [r] bPostfix [bool] true for latitudes, false for longitudes
 ** @param [w] text [char*] textbuffer for result, minimum of 12 chars in length
 **
 ** @return [void]
 */
void CalcGridText( double latlon, double spacing, bool bPostfix, char *text )
{
    int deg = (int) fabs( latlon ); // degrees
    double min = fabs( ( fabs( latlon ) - deg ) * 60.0 ); // Minutes
    char postfix;
    const unsigned int BufLen = 12;

    // calculate postfix letter (NSEW)
    if( latlon > 0.0 ) {
        if( bPostfix ) {
            postfix = 'N';
        } else {
            postfix = 'E';
        }
    } else if( latlon < 0.0 ) {
        if( bPostfix ) {
            postfix = 'S';
        } else {
            postfix = 'W';
        }
    } else {
        postfix = ' '; // no postfix for equator and greenwich
    }
    // calculate text, display minutes only if spacing is smaller than one degree

    if( spacing >= 1.0 ) {
        snprintf( text, BufLen, "%3d° %c", deg, postfix );
    } else if( spacing >= ( 1.0 / 60.0 ) ) {
        snprintf( text, BufLen, "%3d°%02.0f %c", deg, min, postfix );
    } else {
        snprintf( text, BufLen, "%3d°%02.2f %c", deg, min, postfix );
    }
    text[BufLen - 1] = '\0';
    return;
}

/* @ChartCanvas::GridDraw *****************************************
 **
 ** Draws major and minor Lat/Lon Grid on the chart
 ** - distance between Grid-lm ines are calculated automatic
 ** - major grid lines will be across the whole chart window
 ** - minor grid lines will be 10 pixel at each edge of the chart window.
 **
 ** @param [w] dc [wxDC&] the wx drawing context
 **
 ** @return [void]
 ************************************************************************/
void ChartCanvas::GridDraw( ocpnDC& dc )
{
    if( !( g_bDisplayGrid && ( fabs( GetVP().rotation ) < 1e-5 )
            && ( ( fabs( GetVP().skew ) < 1e-9 ) || g_bskew_comp ) ) ) return;

    double nlat, elon, slat, wlon;
    double lat, lon;
    double dlat, dlon;
    double gridlatMajor, gridlatMinor, gridlonMajor, gridlonMinor;
    wxCoord w, h;
    wxPen GridPen( GetGlobalColor( _T ( "SNDG1" ) ), 1, wxSOLID );
    wxFont *font = wxTheFontList->FindOrCreateFont( 8, wxFONTFAMILY_SWISS, wxNORMAL,
                   wxFONTWEIGHT_NORMAL, FALSE, wxString( _T ( "Arial" ) ) );
    dc.SetPen( GridPen );
    dc.SetFont( *font );
    dc.SetTextForeground( GetGlobalColor( _T ( "SNDG1" ) ) );

    w = m_canvas_width;
    h = m_canvas_height;

    GetCanvasPixPoint( 0, 0, nlat, wlon ); // get lat/lon of upper left point of the window
    GetCanvasPixPoint( w, h, slat, elon ); // get lat/lon of lower right point of the window
    dlat = nlat - slat; // calculate how many degrees of latitude are shown in the window
    dlon = elon - wlon; // calculate how many degrees of longitude are shown in the window
    if( dlon < 0.0 ) // concider datum border at 180 degrees longitude
    {
        dlon = dlon + 360.0;
    }
    // calculate distance between latitude grid lines
    CalcGridSpacing( dlat, gridlatMajor, gridlatMinor );

    // calculate position of first major latitude grid line
    lat = ceil( slat / gridlatMajor ) * gridlatMajor;

    // Draw Major latitude grid lines and text
    while( lat < nlat ) {
        wxPoint r;
        char sbuf[12];
        CalcGridText( lat, gridlatMajor, true, sbuf ); // get text for grid line
        GetCanvasPointPix( lat, ( elon + wlon ) / 2, &r );
        dc.DrawLine( 0, r.y, w, r.y, false );                             // draw grid line
        dc.DrawText( wxString( sbuf, wxConvUTF8 ), 0, r.y ); // draw text
        lat = lat + gridlatMajor;

        if( fabs( lat - wxRound( lat ) ) < 1e-5 ) lat = wxRound( lat );
    }

    // calculate position of first minor latitude grid line
    lat = ceil( slat / gridlatMinor ) * gridlatMinor;

    // Draw minor latitude grid lines
    while( lat < nlat ) {
        wxPoint r;
        GetCanvasPointPix( lat, ( elon + wlon ) / 2, &r );
        dc.DrawLine( 0, r.y, 10, r.y, false );
        dc.DrawLine( w - 10, r.y, w, r.y, false );
        lat = lat + gridlatMinor;
    }

    // calculate distance between grid lines
    CalcGridSpacing( dlon, gridlonMajor, gridlonMinor );

    // calculate position of first major latitude grid line
    lon = ceil( wlon / gridlonMajor ) * gridlonMajor;

    // draw major longitude grid lines
    for( int i = 0, itermax = (int) ( dlon / gridlonMajor ); i <= itermax; i++ ) {
        wxPoint r;
        char sbuf[12];
        CalcGridText( lon, gridlonMajor, false, sbuf );
        GetCanvasPointPix( ( nlat + slat ) / 2, lon, &r );
        dc.DrawLine( r.x, 0, r.x, h, false );
        dc.DrawText( wxString( sbuf, wxConvUTF8 ), r.x, 0 );
        lon = lon + gridlonMajor;
        if( lon > 180.0 ) {
            lon = lon - 360.0;
        }

        if( fabs( lon - wxRound( lon ) ) < 1e-5 ) lon = wxRound( lon );

    }

    // calculate position of first minor longitude grid line
    lon = ceil( wlon / gridlonMinor ) * gridlonMinor;
    // draw minor longitude grid lines
    for( int i = 0, itermax = (int) ( dlon / gridlonMinor ); i <= itermax; i++ ) {
        wxPoint r;
        GetCanvasPointPix( ( nlat + slat ) / 2, lon, &r );
        dc.DrawLine( r.x, 0, r.x, 10, false );
        dc.DrawLine( r.x, h - 10, r.x, h, false );
        lon = lon + gridlonMinor;
        if( lon > 180.0 ) {
            lon = lon - 360.0;
        }
    }
}

void ChartCanvas::ScaleBarDraw( ocpnDC& dc )
{
    double blat, blon, tlat, tlon;
    wxPoint r;

    int x_origin = g_bDisplayGrid ? 60 : 20;
    int y_origin = m_canvas_height - 50;

    if( GetVP().chart_scale > 80000 )        // Draw 10 mile scale as SCALEB11
    {
        GetCanvasPixPoint( x_origin, y_origin, blat, blon );
        ll_gc_ll( blat, blon, 0, 10.0, &tlat, &tlon );
        GetCanvasPointPix( tlat, tlon, &r );

        int l1 = ( y_origin - r.y ) / 5;

        wxPen pen1( GetGlobalColor( _T ( "SNDG2" ) ), 3, wxSOLID );
        wxPen pen2( GetGlobalColor( _T ( "SNDG1" ) ), 3, wxSOLID );

        for( int i = 0; i < 5; i++ ) {
            int y = l1 * i;
            if( i & 1 ) dc.SetPen( pen1 );
            else
                dc.SetPen( pen2 );

            dc.DrawLine( x_origin, y_origin - y, x_origin, y_origin - ( y + l1 ) );
        }
    } else                                // Draw 1 mile scale as SCALEB10
    {
        GetCanvasPixPoint( x_origin, y_origin, blat, blon );
        ll_gc_ll( blat, blon, 0, 1.0, &tlat, &tlon );
        GetCanvasPointPix( tlat, tlon, &r );

        int l1 = ( y_origin - r.y ) / 10;

        wxPen pen1( GetGlobalColor( _T ( "SCLBR" ) ), 3, wxSOLID );
        wxPen pen2( GetGlobalColor( _T ( "CHDRD" ) ), 3, wxSOLID );

        for( int i = 0; i < 10; i++ ) {
            int y = l1 * i;
            if( i & 1 ) dc.SetPen( pen1 );
            else
                dc.SetPen( pen2 );

            dc.DrawLine( x_origin, y_origin - y, x_origin, y_origin - ( y + l1 ) );
        }
    }

}

void ChartCanvas::AISDrawAreaNotices( ocpnDC& dc )
{
    if( !g_pAIS || !g_bShowAIS || !g_bShowAreaNotices ) return;

    wxDateTime now = wxDateTime::Now();
    now.MakeGMT();

    bool b_pens_set = false;
    wxPen pen_save;
    wxBrush brush_save;
    wxColour yellow;
    wxColour green;
    wxPen pen;
    wxBrush *yellow_brush;
    wxBrush *green_brush;
    wxBrush *brush;

    AIS_Target_Hash *current_targets = g_pAIS->GetTargetList();

    float vp_scale = GetVPScale();

    for( AIS_Target_Hash::iterator target = current_targets->begin();
            target != current_targets->end(); ++target ) {
        AIS_Target_Data *target_data = target->second;
        if( !target_data->area_notices.empty() ) {
            if( !b_pens_set ) {
                pen_save = dc.GetPen();
                brush_save = dc.GetBrush();

                yellow = GetGlobalColor( _T ( "YELO1" ) );
                yellow.Set( yellow.Red(), yellow.Green(), yellow.Blue(), 64 );

                green = GetGlobalColor( _T ( "GREEN4" ) );
                green.Set( green.Red(), green.Green(), green.Blue(), 64 );

                pen.SetColour( yellow );
                pen.SetWidth( 2 );

                yellow_brush = wxTheBrushList->FindOrCreateBrush( yellow, wxCROSSDIAG_HATCH );
                green_brush = wxTheBrushList->FindOrCreateBrush( green, wxTRANSPARENT );
                brush = yellow_brush;

                b_pens_set = true;
            }

            for( AIS_Area_Notice_Hash::iterator ani = target_data->area_notices.begin();
                    ani != target_data->area_notices.end(); ++ani ) {
                Ais8_001_22& area_notice = ani->second;

                if( area_notice.expiry_time > now ) {
                    std::vector<wxPoint> points;
                    bool draw_polygon = false;
                    double lat, lon;

                    switch( area_notice.notice_type ) {
                    case 0:
                        pen.SetColour( green );
                        brush = green_brush;
                        break;
                    case 1:
                        pen.SetColour( yellow );
                        brush = yellow_brush;
                        break;
                    default:
                        pen.SetColour( yellow );
                        brush = yellow_brush;
                    }
                    dc.SetPen( pen );
                    dc.SetBrush( *brush );

                    for( Ais8_001_22_SubAreaList::iterator sa = area_notice.sub_areas.begin();
                            sa != area_notice.sub_areas.end(); ++sa ) {
                        switch( sa->shape ) {
                        case AIS8_001_22_SHAPE_CIRCLE: {
                            lat = sa->latitude;
                            lon = sa->longitude;

                            wxPoint target_point;
                            GetCanvasPointPix( sa->latitude, sa->longitude, &target_point );
                            points.push_back( target_point );
                            if( sa->radius_m > 0.0 ) dc.DrawCircle( target_point,
                                                                        sa->radius_m * vp_scale );
                            break;
                        }
                        case AIS8_001_22_SHAPE_POLYGON:
                            draw_polygon = true;
                        case AIS8_001_22_SHAPE_POLYLINE: {
                            for( int i = 0; i < 4; ++i ) {
                                ll_gc_ll( lat, lon, sa->angles[i], sa->dists_m[i] / 1852.0,
                                          &lat, &lon );
                                wxPoint target_point;
                                GetCanvasPointPix( lat, lon, &target_point );
                                points.push_back( target_point );
                            }
                        }
                        }
                    }
                    if( draw_polygon ) dc.DrawPolygon( points.size(), &points.front() );
                }
            }
        }
    }

    if( b_pens_set ) {
        dc.SetPen( pen_save );
        dc.SetBrush( brush_save );
    }

}

void ChartCanvas::AISDraw( ocpnDC& dc )
{
    if( !g_pAIS ) return;

// Toggling AIS display on and off

    if( !g_bShowAIS )
        return;//

    //      Iterate over the AIS Target Hashmap
    AIS_Target_Hash::iterator it;

    AIS_Target_Hash *current_targets = g_pAIS->GetTargetList();

    //    Draw all targets in three pass loop, sorted on SOG, GPSGate & DSC on top
    //    This way, fast targets are not obscured by slow/stationary targets
    for( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it ) {
        AIS_Target_Data *td = it->second;
        if( ( td->SOG < g_ShowMoored_Kts )
                && !( ( td->Class == AIS_GPSG_BUDDY ) || ( td->Class == AIS_DSC ) ) ) AISDrawTarget(
                        td, dc );
    }

    for( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it ) {
        AIS_Target_Data *td = it->second;
        if( ( td->SOG >= g_ShowMoored_Kts )
                && !( ( td->Class == AIS_GPSG_BUDDY ) || ( td->Class == AIS_DSC ) ) ) AISDrawTarget(
                        td, dc );
    }

    for( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it ) {
        AIS_Target_Data *td = it->second;
        if( ( td->Class == AIS_GPSG_BUDDY ) || ( td->Class == AIS_DSC ) ) AISDrawTarget( td, dc );
    }
}

void ChartCanvas::AISDrawTarget( AIS_Target_Data *td, ocpnDC& dc )
{
    //      Target data must be valid
    if( NULL == td ) return;

    //    Target is lost due to position report time-out, but still in Target List
    if( td->b_lost ) return;

    //      Skip anchored/moored (interpreted as low speed) targets if requested
    //      unless the target is NUC or AtoN, in which case it is always displayed.
    if( ( !g_bShowMoored ) && ( td->SOG <= g_ShowMoored_Kts )
            && ( td->NavStatus != NOT_UNDER_COMMAND )
            && ( ( td->Class == AIS_CLASS_A ) || ( td->Class == AIS_CLASS_B ) ) ) return;

    //      Target data position must have been valid once
    if( !td->b_positionOnceValid ) return;

    // And we never draw ownship
    if( td->b_OwnShip ) return;

    //    If target's speed is unavailable, use zero for further calculations
    double target_sog = td->SOG;
    if( td->SOG > 102.2 ) target_sog = 0.;

    int drawit = 0;
    wxPoint TargetPoint, PredPoint;

    //    Is target in Vpoint?
    if( GetVP().GetBBox().PointInBox( td->Lon, td->Lat, 0 ) ) drawit++;                       // yep

    //   Always draw alert targets, even if they are off the screen
    if( td->n_alarm_state == AIS_ALARM_SET ) drawit++;

    //  If AIS tracks are shown, is the first point of the track on-screen?
    if( g_bAISShowTracks ) {
        wxAISTargetTrackListNode *node = td->m_ptrack->GetFirst();
        if( node ) {
            AISTargetTrackPoint *ptrack_point = node->GetData();
            if( GetVP().GetBBox().PointInBox( ptrack_point->m_lon, ptrack_point->m_lat, 0 ) ) drawit++;
        }
    }

    //    Calculate AIS target Position Predictor, using global static variable for length of vector

    double pred_lat, pred_lon;

    ll_gc_ll( td->Lat, td->Lon, td->COG, target_sog * g_ShowCOG_Mins / 60., &pred_lat, &pred_lon );

    //    Is predicted point in the VPoint?
    if( GetVP().GetBBox().PointInBox( pred_lon, pred_lat, 0 ) ) drawit++;                     // yep

    // And one more test to catch the case where target COG line crosses the screen,
    // but the target itself and its pred point are both off-screen
    if( GetVP().GetBBox().LineIntersect( wxPoint2DDouble( td->Lon, td->Lat ),
                                         wxPoint2DDouble( pred_lon, pred_lat ) ) ) drawit++;

    //    Do the draw if conditions indicate
    if( drawit ) {
        GetCanvasPointPix( td->Lat, td->Lon, &TargetPoint );
        GetCanvasPointPix( pred_lat, pred_lon, &PredPoint );

        //  Calculate the relative angle for this chart orientation
        //    Use a 100 pixel vector to calculate angle
        double angle_distance_nm = ( 100. / GetVP().view_scale_ppm ) / 1852.;
        double angle_lat, angle_lon;
        wxPoint AnglePoint;
        ll_gc_ll( td->Lat, td->Lon, td->COG, angle_distance_nm, &angle_lat, &angle_lon );
        GetCanvasPointPix( angle_lat, angle_lon, &AnglePoint );

        double theta;

        if( abs( AnglePoint.x - TargetPoint.x ) > 0 ) {
            if( target_sog > g_ShowMoored_Kts ) theta = atan2(
                            (double) ( AnglePoint.y - TargetPoint.y ),
                            (double) ( AnglePoint.x - TargetPoint.x ) );
            else
                theta = -PI / 2;
        } else {
            if( AnglePoint.y > TargetPoint.y ) theta = PI / 2.;             // valid COG 180
            else
                theta = -PI / 2.;            //  valid COG 000 or speed is too low to resolve course
        }

        //    Of course, if the target reported a valid HDG, then use it for icon
        if( (int) ( td->HDG ) != 511 ) {
            theta = ( ( td->HDG - 90 ) * PI / 180. ) + GetVP().rotation;
            if( !g_bskew_comp && !g_bCourseUp )
                theta += GetVP().skew;
        }

        wxDash dash_long[2];
        dash_long[0] = (int) ( 1.0 * m_pix_per_mm );  // Long dash  <---------+
        dash_long[1] = (int) ( 0.5 * m_pix_per_mm );  // Short gap            |

        //  Draw the icon rotated to the COG
        wxPoint ais_quad_icon[4];
        ais_quad_icon[0].x = -8;
        ais_quad_icon[0].y = -6;
        ais_quad_icon[1].x = 0;
        ais_quad_icon[1].y = 24;
        ais_quad_icon[2].x = 8;
        ais_quad_icon[2].y = -6;
        ais_quad_icon[3].x = 0;
        ais_quad_icon[3].y = -6;

        wxPoint ais_real_size[4];
        bool bcan_draw_size;
        if (g_bDrawAISSize)
        {
            if (td->DimA + td->DimB == 0 || td->DimC + td->DimD == 0)
            {
                bcan_draw_size = false;
            }
            else
            {
                double ref_lat, ref_lon;
                ll_gc_ll( td->Lat, td->Lon, 0, 100. / 1852., &ref_lat,
                          &ref_lon );
                wxPoint2DDouble b_point = GetVP().GetDoublePixFromLL( td->Lat, td->Lon );
                wxPoint2DDouble r_point = GetVP().GetDoublePixFromLL( ref_lat, ref_lon );
                double ppm = r_point.GetDistance(b_point) / 100.;
                ais_real_size[0].x = -td->DimD * ppm;
                ais_real_size[0].y = -td->DimB * ppm;
                ais_real_size[1].x = -td->DimD * ppm;
                ais_real_size[1].y = td->DimA * ppm;
                ais_real_size[2].x = td->DimC * ppm;
                ais_real_size[2].y = td->DimA * ppm;
                ais_real_size[3].x = td->DimC * ppm;
                ais_real_size[3].y = -td->DimB * ppm;
                if (ais_real_size[2].x - ais_real_size[0].x < 16 || ais_real_size[2].y - ais_real_size[0].y < 30)
                    bcan_draw_size = false; //drawing too small does not make sense
                else
                    bcan_draw_size = true;
            }
        }

        //   If this is an AIS Class B target, so symbolize it differently
        if( td->Class == AIS_CLASS_B ) ais_quad_icon[3].y = 0;
        if( td->Class == AIS_GPSG_BUDDY ) {
            ais_quad_icon[0].x = -5;
            ais_quad_icon[0].y = -12;
            ais_quad_icon[1].x = -3;
            ais_quad_icon[1].y = 12;
            ais_quad_icon[2].x = 3;
            ais_quad_icon[2].y = 12;
            ais_quad_icon[3].x = 5;
            ais_quad_icon[3].y = -12;
        }
        if( td->Class == AIS_DSC ) {
            ais_quad_icon[0].y = 0;
            ais_quad_icon[1].y = 8;
            ais_quad_icon[2].y = 0;
            ais_quad_icon[3].y = -8;
        }

        for( int i = 0; i < 4; i++ )
        {
            double px = ( (double) ais_quad_icon[i].x ) * sin( theta )
                        + ( (double) ais_quad_icon[i].y ) * cos( theta );
            double py = ( (double) ais_quad_icon[i].y ) * sin( theta )
                        - ( (double) ais_quad_icon[i].x ) * cos( theta );
            ais_quad_icon[i].x = (int) round( px );
            ais_quad_icon[i].y = (int) round( py );
            if (g_bDrawAISSize && bcan_draw_size)
            {
                double px = ( (double) ais_real_size[i].x ) * sin( theta )
                            + ( (double) ais_real_size[i].y ) * cos( theta );
                double py = ( (double) ais_real_size[i].y ) * sin( theta )
                            - ( (double) ais_real_size[i].x ) * cos( theta );
                ais_real_size[i].x = (int) round( px );
                ais_real_size[i].y = (int) round( py );
            }
        }

        dc.SetPen( wxPen( GetGlobalColor( _T ( "UBLCK" ) ) ) );

        // Default color is green
        wxBrush target_brush = wxBrush( GetGlobalColor( _T ( "UINFG" ) ) );

        // Euro Inland targets render slightly differently
        if( td->b_isEuroInland )
            target_brush = wxBrush( GetGlobalColor( _T ( "TEAL1" ) ) );

        //and....
        if( !td->b_nameValid )
            target_brush = wxBrush( GetGlobalColor( _T ( "CHYLW" ) ) );
        if( ( td->Class == AIS_DSC ) && ( td->ShipType == 12 ) )					// distress
            target_brush = wxBrush( GetGlobalColor( _T ( "URED" ) ) );

        if( ( td->n_alarm_state == AIS_ALARM_SET ) && ( td->bCPA_Valid ) ) target_brush = wxBrush(
                        GetGlobalColor( _T ( "URED" ) ) );

        if( td->b_positionDoubtful ) target_brush = wxBrush( GetGlobalColor( _T ( "UINFF" ) ) );

        //    Check for alarms here, maintained by AIS class timer tick
        if( ((td->n_alarm_state == AIS_ALARM_SET) && (td->bCPA_Valid)) || (td->b_show_AIS_CPA && (td->bCPA_Valid))) {
            //  Calculate the point of CPA for target
            double tcpa_lat, tcpa_lon;
            ll_gc_ll( td->Lat, td->Lon, td->COG, target_sog * td->TCPA / 60., &tcpa_lat,
                      &tcpa_lon );
            wxPoint tCPAPoint;
            wxPoint TPoint = TargetPoint;
            GetCanvasPointPix( tcpa_lat, tcpa_lon, &tCPAPoint );

            //  Draw the intercept line from target
            ClipResult res = cohen_sutherland_line_clip_i( &TPoint.x, &TPoint.y, &tCPAPoint.x,
                             &tCPAPoint.y, 0, GetVP().pix_width, 0, GetVP().pix_height );

            if( res != Invisible ) {
                wxPen ppPen2( GetGlobalColor( _T ( "URED" ) ), 2, wxUSER_DASH );
                ppPen2.SetDashes( 2, dash_long );
                dc.SetPen( ppPen2 );

                dc.StrokeLine( TPoint.x, TPoint.y, tCPAPoint.x, tCPAPoint.y );
            }

            //  Calculate the point of CPA for ownship

            //  Detect and handle the case where ownship COG is undefined....
            double cog_assumed = gCog;
            if( wxIsNaN(gCog) && ( gSog < .01 ) ) cog_assumed = 0.;          // substitute value
            // for the case where SOG = 0, and COG is unknown.

            double ocpa_lat, ocpa_lon;
            ll_gc_ll( gLat, gLon, cog_assumed, gSog * td->TCPA / 60., &ocpa_lat, &ocpa_lon );
            wxPoint oCPAPoint;

            GetCanvasPointPix( ocpa_lat, ocpa_lon, &oCPAPoint );
            GetCanvasPointPix( tcpa_lat, tcpa_lon, &tCPAPoint );

            //        Save a copy of these
            wxPoint oCPAPoint_sav = oCPAPoint;
            wxPoint tCPAPoint_sav = tCPAPoint;

            //  Draw a line from target CPA point to ownship CPA point
            ClipResult ores = cohen_sutherland_line_clip_i( &tCPAPoint.x, &tCPAPoint.y,
                              &oCPAPoint.x, &oCPAPoint.y, 0, GetVP().pix_width, 0, GetVP().pix_height );

            if( ores != Invisible ) {
                wxColour yellow = GetGlobalColor( _T ( "YELO1" ) );
                dc.SetPen( wxPen( yellow, 4 ) );
                dc.StrokeLine( tCPAPoint.x, tCPAPoint.y, oCPAPoint.x, oCPAPoint.y );

                wxPen ppPen2( GetGlobalColor( _T ( "URED" ) ), 2, wxUSER_DASH );
                ppPen2.SetDashes( 2, dash_long );
                dc.SetPen( ppPen2 );
                dc.StrokeLine( tCPAPoint.x, tCPAPoint.y, oCPAPoint.x, oCPAPoint.y );

                //        Draw little circles at the ends of the CPA alert line
                wxBrush br( GetGlobalColor( _T ( "BLUE3" ) ) );
                dc.SetBrush( br );
                dc.SetPen( wxPen( GetGlobalColor( _T ( "UBLK" ) ) ) );

                //  Using the true ends, not the clipped ends
                dc.StrokeCircle( tCPAPoint_sav.x, tCPAPoint_sav.y, 5 );
                dc.StrokeCircle( oCPAPoint_sav.x, oCPAPoint_sav.y, 5 );
            }

            // Draw the intercept line from ownship
            wxPoint oShipPoint;
            GetCanvasPointPix ( gLat, gLon, &oShipPoint );
            ClipResult ownres = cohen_sutherland_line_clip_i ( &oShipPoint.x, &oShipPoint.y, &oCPAPoint.x, &oCPAPoint.y, 0, GetVP().pix_width, 0, GetVP().pix_height );

            if ( ownres != Invisible ) {
                wxPen ppPen2 ( GetGlobalColor ( _T ( "URED" )), 2, wxUSER_DASH );
                ppPen2.SetDashes( 2, dash_long );
                dc.SetPen(ppPen2);

                dc.StrokeLine ( oShipPoint.x, oShipPoint.y, oCPAPoint.x, oCPAPoint.y );
            } //TR : till here

            dc.SetPen( wxPen( GetGlobalColor( _T ( "UBLCK" ) ) ) );
            dc.SetBrush( wxBrush( GetGlobalColor( _T ( "URED" ) ) ) );
        }

        //  Highlight the AIS target symbol if an alert dialog is currently open for it
        if( g_pais_alert_dialog_active && g_pais_alert_dialog_active->IsShown() ) {
            if( g_pais_alert_dialog_active->Get_Dialog_MMSI() == td->MMSI ) JaggyCircle( dc,
                        wxPen( GetGlobalColor( _T ( "URED" ) ), 2 ), TargetPoint.x, TargetPoint.y,
                        100 );
        }

        //  Highlight the AIS target symbol if a query dialog is currently open for it
        if( g_pais_query_dialog_active && g_pais_query_dialog_active->IsShown() ) {
            if( g_pais_query_dialog_active->GetMMSI() == td->MMSI ) TargetFrame( dc,
                        wxPen( GetGlobalColor( _T ( "UBLCK" ) ), 2 ), TargetPoint.x, TargetPoint.y,
                        25 );
        }

        //       Render the COG line if the speed is greater than moored speed defined by ais options dialog
        if( ( g_bShowCOG ) && ( target_sog > g_ShowMoored_Kts ) ) {
            int pixx = TargetPoint.x;
            int pixy = TargetPoint.y;
            int pixx1 = PredPoint.x;
            int pixy1 = PredPoint.y;

            //  Don't draw the COG line  and predictor point if zoomed far out.... or if target lost/inactive
            double l = pow(
                           pow( (double) ( PredPoint.x - TargetPoint.x ), 2 )
                           + pow( (double) ( PredPoint.y - TargetPoint.y ), 2 ), 0.5 );

            if( l > 24 ) {
                ClipResult res = cohen_sutherland_line_clip_i( &pixx, &pixy, &pixx1, &pixy1, 0,
                                 GetVP().pix_width, 0, GetVP().pix_height );

                if( ( res != Invisible ) && ( td->b_active ) ) {
                    //    Draw a 3 pixel wide line
                    wxPen wide_pen( target_brush.GetColour(), 3 );
                    dc.SetPen( wide_pen );
                    dc.StrokeLine( pixx, pixy, pixx1, pixy1 );

                    //    Draw a 1 pixel wide black line
                    wxPen narrow_pen( GetGlobalColor( _T ( "UBLCK" ) ), 1 );
                    dc.SetPen( narrow_pen );
                    dc.StrokeLine( pixx, pixy, pixx1, pixy1 );

                    dc.SetBrush( target_brush );
                    dc.StrokeCircle( PredPoint.x, PredPoint.y, 5 );
                }

                //      Draw RateOfTurn Vector
                if( ( td->ROTAIS != 0 ) && ( td->ROTAIS != -128 ) && td->b_active ) {
                    double nv = 10;
                    double theta2 = theta;
                    if( td->ROTAIS > 0 ) theta2 += PI / 2.;
                    else
                        theta2 -= PI / 2.;

                    int xrot = (int) round ( pixx1 + ( nv * cos ( theta2 ) ) );
                    int yrot = (int) round ( pixy1 + ( nv * sin ( theta2 ) ) );
                    dc.StrokeLine( pixx1, pixy1, xrot, yrot );
                    dc.CalcBoundingBox( xrot, yrot );

                }
            }
        }

        //        Actually Draw the target

        if( td->Class == AIS_ATON ) {                   // Aid to Navigation
            wxPen aton_pen;
            if( ( td->NavStatus == ATON_VIRTUAL_OFFPOSITION ) || ( td->NavStatus == ATON_REAL_OFFPOSITION ) )
                aton_pen = wxPen( GetGlobalColor( _T ( "URED" ) ), 2 );
            else
                aton_pen = wxPen( GetGlobalColor( _T ( "UBLCK" ) ), 2 );

            bool b_virt = ( td->NavStatus == ATON_VIRTUAL )
                          | ( td->NavStatus == ATON_VIRTUAL_ONPOSITION )
                          | ( td->NavStatus == ATON_VIRTUAL_OFFPOSITION );

            AtoN_Diamond( dc, aton_pen, TargetPoint.x, TargetPoint.y, 12, b_virt );
        } else if( td->Class == AIS_BASE ) {                      // Base Station
            Base_Square( dc, wxPen( GetGlobalColor( _T ( "UBLCK" ) ), 2 ), TargetPoint.x,
                         TargetPoint.y, 8 );
        } else if( td->Class == AIS_SART ) {                      // SART Target
            if( td->NavStatus == 14 )       // active
                SART_Render( dc, wxPen( GetGlobalColor( _T ( "URED" ) ), 2 ), TargetPoint.x,
                             TargetPoint.y, 8 );
            else
                SART_Render( dc, wxPen( GetGlobalColor( _T ( "UGREN" ) ), 2 ),
                             TargetPoint.x, TargetPoint.y, 8 );
        } else {         // ship class A or B or a Buddy or DSC
            wxPen target_pen( GetGlobalColor( _T ( "UBLCK" ) ), 1 );

            dc.SetPen( target_pen );
            dc.SetBrush( target_brush );

            if( td->Class == AIS_CLASS_B ) {
                // decompose to two "convex" polygons and one combined outline to satisfy OpenGL's requirements
                wxPen tri_pen( target_brush.GetColour(), 1 );
                dc.SetPen( tri_pen );

                wxPoint ais_tri_icon[3];

                ais_tri_icon[0] = ais_quad_icon[0];
                ais_tri_icon[1] = ais_quad_icon[1];
                ais_tri_icon[2] = ais_quad_icon[3];
                dc.StrokePolygon( 3, ais_tri_icon, TargetPoint.x, TargetPoint.y );

                ais_tri_icon[0] = ais_quad_icon[1];
                ais_tri_icon[1] = ais_quad_icon[2];
                ais_tri_icon[2] = ais_quad_icon[3];
                dc.StrokePolygon( 3, ais_tri_icon, TargetPoint.x, TargetPoint.y );

                dc.SetPen( target_pen );
                dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UBLCK" ) ), wxTRANSPARENT ) );
                dc.StrokePolygon( 4, ais_quad_icon, TargetPoint.x, TargetPoint.y );

            } else
                dc.StrokePolygon( 4, ais_quad_icon, TargetPoint.x, TargetPoint.y );

            if (g_bDrawAISSize && bcan_draw_size)
            {
                dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UBLCK" ) ), wxTRANSPARENT ) );
                dc.StrokePolygon( 4, ais_real_size, TargetPoint.x, TargetPoint.y );
            }

            dc.SetBrush( wxBrush( GetGlobalColor( _T ( "SHIPS" ) ) ) );
            int navstatus = td->NavStatus;

            // HSC usually have correct ShipType but navstatus == 0...
            if( ( ( td->ShipType >= 40 ) && ( td->ShipType < 50 ) )
                    && navstatus == UNDERWAY_USING_ENGINE ) navstatus = HSC;

            switch( navstatus ) {
                case MOORED:
                case AT_ANCHOR: {
                    dc.StrokeCircle( TargetPoint.x, TargetPoint.y, 4 );
                    break;
                }
                case RESTRICTED_MANOEUVRABILITY: {
                    wxPoint diamond[4];
                    diamond[0] = wxPoint(  4, 0 );
                    diamond[1] = wxPoint(  0, -6 );
                    diamond[2] = wxPoint( -4, 0 );
                    diamond[3] = wxPoint(  0, 6 );
                    dc.StrokePolygon( 4, diamond, TargetPoint.x, TargetPoint.y-11 );
                    dc.StrokeCircle( TargetPoint.x, TargetPoint.y, 4 );
                    dc.StrokeCircle( TargetPoint.x, TargetPoint.y-22, 4 );
                    break;
                   break;
                }
                case CONSTRAINED_BY_DRAFT: {
                    wxPoint can[4];
                    can[0] = wxPoint( -3, 0 );
                    can[1] = wxPoint(  3, 0 );
                    can[2] = wxPoint(  3, -16 );
                    can[3] = wxPoint( -3, -16 );
                    dc.StrokePolygon( 4, can, TargetPoint.x, TargetPoint.y );
                    break;
                }
                case NOT_UNDER_COMMAND: {
                    dc.StrokeCircle( TargetPoint.x, TargetPoint.y, 4 );
                    dc.StrokeCircle( TargetPoint.x, TargetPoint.y-9, 4 );
                    break;
                }
                case FISHING: {
                    wxPoint tri[3];
                    tri[0] = wxPoint( -4, 0 );
                    tri[1] = wxPoint(  4, 0 );
                    tri[2] = wxPoint(  0, -9 );
                    dc.StrokePolygon( 3, tri, TargetPoint.x, TargetPoint.y );
                    tri[0] = wxPoint(  0, -9 );
                    tri[1] = wxPoint(  4, -18 );
                    tri[2] = wxPoint( -4, -18 );
                    dc.StrokePolygon( 3, tri, TargetPoint.x, TargetPoint.y );
                    break;
                }
                case AGROUND: {
                    dc.StrokeCircle( TargetPoint.x, TargetPoint.y, 4 );
                    dc.StrokeCircle( TargetPoint.x, TargetPoint.y-9, 4 );
                    dc.StrokeCircle( TargetPoint.x, TargetPoint.y-18, 4 );
                    break;
                }
                case HSC:
                case WIG: {
                    wxPoint arrow[3];
                    arrow[0] = wxPoint( -4, 20 );
                    arrow[1] = wxPoint(  0, 27 );
                    arrow[2] = wxPoint(  4, 20 );
                    for( int i = 0; i < 3; i++ ) {
                        double px = ( (double) arrow[i].x ) * sin( theta )
                                    + ( (double) arrow[i].y ) * cos( theta );
                        double py = ( (double) arrow[i].y ) * sin( theta )
                                    - ( (double) arrow[i].x ) * cos( theta );
                        arrow[i].x = (int) round( px );
                        arrow[i].y = (int) round( py );
                    }
                    dc.SetBrush( target_brush );
                    dc.StrokePolygon( 3, arrow, TargetPoint.x, TargetPoint.y );
                    arrow[0] = wxPoint( -4, 27 );
                    arrow[1] = wxPoint(  0, 34 );
                    arrow[2] = wxPoint(  4, 27 );
                    for( int i = 0; i < 3; i++ ) {
                        double px = ( (double) arrow[i].x ) * sin( theta )
                                    + ( (double) arrow[i].y ) * cos( theta );
                        double py = ( (double) arrow[i].y ) * sin( theta )
                                    - ( (double) arrow[i].x ) * cos( theta );
                        arrow[i].x = (int) round( px );
                        arrow[i].y = (int) round( py );
                    }
                    dc.StrokePolygon( 3, arrow, TargetPoint.x, TargetPoint.y );
                    break;
                }
            }

            //        Draw the inactive cross-out line
            if( !td->b_active ) {
                dc.SetPen( wxPen( GetGlobalColor( _T ( "UBLCK" ) ), 2 ) );

                wxPoint p1 = transrot( wxPoint( -14, 0 ), theta, TargetPoint );
                wxPoint p2 = transrot( wxPoint( 14, 0 ), theta, TargetPoint );
                dc.StrokeLine( p1.x, p1.y, p2.x, p2.y );
                dc.CalcBoundingBox( p1.x, p1.y );
                dc.CalcBoundingBox( p2.x, p2.y );

                dc.SetPen( wxPen( GetGlobalColor( _T ( "UBLCK" ) ), 1 ) );
            }

            //    European Inland AIS define a "stbd-stbd" meeting sign, a blue paddle.
            //    Symbolize it if set by most recent message
            if( td->b_blue_paddle ) {
                wxPoint ais_flag_icon[4];
                ais_flag_icon[0].x = -8;
                ais_flag_icon[0].y = -6;
                ais_flag_icon[1].x = -2;
                ais_flag_icon[1].y = 18;
                ais_flag_icon[2].x = -2;
                ais_flag_icon[2].y = 0;
                ais_flag_icon[3].x = -2;
                ais_flag_icon[3].y = -6;

                for( int i = 0; i < 4; i++ ) {
                    double px = ( (double) ais_flag_icon[i].x ) * sin( theta )
                                + ( (double) ais_flag_icon[i].y ) * cos( theta );
                    double py = ( (double) ais_flag_icon[i].y ) * sin( theta )
                                - ( (double) ais_flag_icon[i].x ) * cos( theta );
                    ais_flag_icon[i].x = (int) round( px );
                    ais_flag_icon[i].y = (int) round( py );
                }

                dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UINFB" ) ) ) );
                dc.SetPen( wxPen( GetGlobalColor( _T ( "CHWHT" ) ), 2 ) );
                dc.StrokePolygon( 4, ais_flag_icon, TargetPoint.x, TargetPoint.y );
            }
        }

        //  Draw tracks if enabled
        if( g_bAISShowTracks ) {
            wxPoint TrackPointA;
            wxPoint TrackPointB;

            dc.SetPen( wxPen( GetGlobalColor( _T ( "CHMGD" ) ), 2 ) );

            //    First point
            wxAISTargetTrackListNode *node = td->m_ptrack->GetFirst();
            if( node ) {
                AISTargetTrackPoint *ptrack_point = node->GetData();
                GetCanvasPointPix( ptrack_point->m_lat, ptrack_point->m_lon, &TrackPointA );
                node = node->GetNext();
            }
            while( node ) {
                AISTargetTrackPoint *ptrack_point = node->GetData();
                GetCanvasPointPix( ptrack_point->m_lat, ptrack_point->m_lon, &TrackPointB );

                dc.StrokeLine( TrackPointA, TrackPointB );

                node = node->GetNext();
                TrackPointA = TrackPointB;
            }

        }           // Draw tracks

    }       // drawit
}

void ChartCanvas::JaggyCircle( ocpnDC &dc, wxPen pen, int x, int y, int radius )
{
    //    Constants?
    double da_min = 2.;
    double da_max = 6.;
    double ra_min = 0.;
    double ra_max = 40.;

    wxPen pen_save = dc.GetPen();

    wxDateTime now = wxDateTime::Now();

//      srand( now.GetTicks());
    srand( 1 );
    dc.SetPen( pen );

    int x0, y0, x1, y1;

    x0 = x + radius;                    // Start point
    y0 = y;
    double angle = 0.;
    int i = 0;

    while( angle < 360. ) {
        double da = da_min + ( ( (double) rand() / RAND_MAX ) * ( da_max - da_min ) );
        angle += da;

        if( angle > 360. ) angle = 360.;

        double ra = ra_min + ( ( (double) rand() / RAND_MAX ) * ( ra_max - ra_min ) );

        double r;
        if( i % 1 ) r = radius + ra;
        else
            r = radius - ra;

        x1 = (int) ( x + cos( angle * PI / 180. ) * r );
        y1 = (int) ( y + sin( angle * PI / 180. ) * r );

        dc.DrawLine( x0, y0, x1, y1 );

        x0 = x1;
        y0 = y1;

        i++;

    }

    dc.DrawLine( x + radius, y, x1, y1 );             // closure

    dc.SetPen( pen_save );
}

void ChartCanvas::TargetFrame( ocpnDC &dc, wxPen pen, int x, int y, int radius )
{
    //    Constants?
    int gap2 = 2 * radius / 6;

    wxPen pen_save = dc.GetPen();

    dc.SetPen( pen );

    dc.DrawLine( x - radius, y + gap2, x - radius, y + radius );
    dc.DrawLine( x - radius, y + radius, x - gap2, y + radius );
    dc.DrawLine( x + gap2, y + radius, x + radius, y + radius );
    dc.DrawLine( x + radius, y + radius, x + radius, y + gap2 );
    dc.DrawLine( x + radius, y - gap2, x + radius, y - radius );
    dc.DrawLine( x + radius, y - radius, x + gap2, y - radius );
    dc.DrawLine( x - gap2, y - radius, x - radius, y - radius );
    dc.DrawLine( x - radius, y - radius, x - radius, y - gap2 );

    dc.SetPen( pen_save );
}

void ChartCanvas::AtoN_Diamond( ocpnDC &dc, wxPen pen, int x, int y, int radius, bool b_virtual )
{
    //    Constants?
    int gap2 = 2 * radius / 8;
    int pen_width = pen.GetWidth();

    wxPen pen_save = dc.GetPen();

    dc.SetPen( pen );   // draw diamond

    dc.DrawLine( x - radius, y, x, y + radius );
    dc.DrawLine( x, y + radius, x + radius, y );
    dc.DrawLine( x + radius, y, x, y - radius );
    dc.DrawLine( x, y - radius, x - radius, y );

    if( pen_width > 1 ) {
        pen_width -= 1;
        pen.SetWidth( pen_width );
        dc.SetPen( pen );
    }    // draw cross inside

    dc.DrawLine( x - gap2, y, x + gap2, y );
    dc.DrawLine( x, y - gap2, x, y + gap2 );

    if( b_virtual ) {
        dc.DrawLine( x - gap2 - 3, y - 1, x, y + gap2 + 5 );
        dc.DrawLine( x, y + gap2 + 5, x + gap2 + 4, y - 2 );
    }

    dc.SetPen( pen_save );
}

void ChartCanvas::Base_Square( ocpnDC &dc, wxPen pen, int x, int y, int radius )
{
    //    Constants?
    int gap2 = 2 * radius / 6;
    int pen_width = pen.GetWidth();

    wxPen pen_save = dc.GetPen();

    dc.SetPen( pen );   // draw square

    dc.DrawLine( x - radius, y - radius, x - radius, y + radius );
    dc.DrawLine( x - radius, y + radius, x + radius, y + radius );
    dc.DrawLine( x + radius, y + radius, x + radius, y - radius );
    dc.DrawLine( x + radius, y - radius, x - radius, y - radius );

    if( pen_width > 1 ) {
        pen_width -= 1;
        pen.SetWidth( pen_width );
    }    // draw cross inside

    dc.DrawLine( x - gap2, y, x + gap2, y );
    dc.DrawLine( x, y - gap2, x, y + gap2 );

    dc.SetPen( pen_save );
}

void ChartCanvas::SART_Render( ocpnDC &dc, wxPen pen, int x, int y, int radius )
{
    //    Constants
    int gap = ( radius * 12 ) / 10;
    int pen_width = pen.GetWidth();

    wxPen pen_save = dc.GetPen();

    dc.SetPen( pen );

    wxBrush brush_save = dc.GetBrush();
    wxBrush *ppBrush = wxTheBrushList->FindOrCreateBrush( wxColour( 0, 0, 0 ), wxTRANSPARENT );
    dc.SetBrush( *ppBrush );

    dc.DrawCircle( x, y, radius );

    if( pen_width > 1 ) {
        pen_width -= 1;
        pen.SetWidth( pen_width );
    }    // draw cross inside

    dc.DrawLine( x - gap, y - gap, x + gap, y + gap );
    dc.DrawLine( x - gap, y + gap, x + gap, y - gap );

    dc.SetBrush( brush_save );
    dc.SetPen( pen_save );
}

void ChartCanvas::AlertDraw( ocpnDC& dc )
{
// Just for prototyping, visual alert for anchorwatch goes here
    bool play_sound = false;
    if( pAnchorWatchPoint1 && AnchorAlertOn1 ) {
        if( AnchorAlertOn1 ) {
            wxPoint TargetPoint;
            GetCanvasPointPix( pAnchorWatchPoint1->m_lat, pAnchorWatchPoint1->m_lon, &TargetPoint );
            JaggyCircle( dc, wxPen( GetGlobalColor( _T("URED") ), 2 ), TargetPoint.x, TargetPoint.y,
                         100 );
            play_sound = true;
        }
    } else
        AnchorAlertOn1 = false;

    if( pAnchorWatchPoint2 && AnchorAlertOn2 ) {
        if( AnchorAlertOn2 ) {
            wxPoint TargetPoint;
            GetCanvasPointPix( pAnchorWatchPoint2->m_lat, pAnchorWatchPoint2->m_lon, &TargetPoint );
            JaggyCircle( dc, wxPen( GetGlobalColor( _T("URED") ), 2 ), TargetPoint.x, TargetPoint.y,
                         100 );
            play_sound = true;
        }
    } else
        AnchorAlertOn2 = false;

    if( play_sound ) {
        if( !m_anchorwatch_sound.IsOk() ) m_anchorwatch_sound.Create( g_sAIS_Alert_Sound_File );

#ifndef __WXMSW__
        if(m_anchorwatch_sound.IsOk() && !m_anchorwatch_sound.IsPlaying())
            m_anchorwatch_sound.Play();
#else
        if( m_anchorwatch_sound.IsOk() ) m_anchorwatch_sound.Play();
#endif
    } else {
        if( m_anchorwatch_sound.IsOk() ) m_anchorwatch_sound.Stop();
    }

}
// End of prototype anchor watch alerting-----------------------

void ChartCanvas::UpdateShips()
{
    //  Get the rectangle in the current dc which bounds the "ownship" symbol

    wxClientDC dc( this );
    if( !dc.IsOk() ) return;

    wxBitmap test_bitmap( dc.GetSize().x, dc.GetSize().y );
    wxMemoryDC temp_dc( test_bitmap );

    temp_dc.ResetBoundingBox();
    temp_dc.DestroyClippingRegion();
    temp_dc.SetClippingRegion( 0, 0, dc.GetSize().x, dc.GetSize().y );

    // Draw the ownship on the temp_dc
    ocpnDC ocpndc = ocpnDC( temp_dc );
    ShipDraw( ocpndc );

    if( g_pActiveTrack && g_pActiveTrack->IsRunning() ) {
        RoutePoint* p = g_pActiveTrack->GetLastPoint();
        if( p ) {
            wxPoint px;
            cc1->GetCanvasPointPix( p->m_lat, p->m_lon, &px );
            ocpndc.CalcBoundingBox( px.x, px.y );
        }
    }

    ship_draw_rect = wxRect( temp_dc.MinX(), temp_dc.MinY(),
            temp_dc.MaxX() - temp_dc.MinX(),
            temp_dc.MaxY() - temp_dc.MinY() );

    wxRect own_ship_update_rect = ship_draw_rect;

    if( !own_ship_update_rect.IsEmpty() ) {
        //  The required invalidate rectangle is the union of the last drawn rectangle
        //  and this drawn rectangle
        own_ship_update_rect.Union( ship_draw_last_rect );
        own_ship_update_rect.Inflate( 2 );
    }

    if( !own_ship_update_rect.IsEmpty() ) RefreshRect( own_ship_update_rect, false );

    ship_draw_last_rect = ship_draw_rect;

    temp_dc.SelectObject( wxNullBitmap );
}

void ChartCanvas::UpdateAlerts()
{
    //  Get the rectangle in the current dc which bounds the detected Alert targets

    //  Use this dc
    wxClientDC dc( this );

    // Get dc boundary
    int sx, sy;
    dc.GetSize( &sx, &sy );

    //  Need a bitmap
    wxBitmap test_bitmap( sx, sy, -1 );

    // Create a memory DC
    wxMemoryDC temp_dc;
    temp_dc.SelectObject( test_bitmap );

    temp_dc.ResetBoundingBox();
    temp_dc.DestroyClippingRegion();
    temp_dc.SetClippingRegion( wxRect( 0, 0, sx, sy ) );

    // Draw the Alert Targets on the temp_dc
    ocpnDC ocpndc = ocpnDC( temp_dc );
    AlertDraw( ocpndc );

    //  Retrieve the drawing extents
    wxRect alert_rect( temp_dc.MinX(), temp_dc.MinY(), temp_dc.MaxX() - temp_dc.MinX(),
                       temp_dc.MaxY() - temp_dc.MinY() );

    if( !alert_rect.IsEmpty() ) alert_rect.Inflate( 2 );              // clear all drawing artifacts

    if( !alert_rect.IsEmpty() || !alert_draw_rect.IsEmpty() ) {
        //  The required invalidate rectangle is the union of the last drawn rectangle
        //  and this drawn rectangle
        wxRect alert_update_rect = alert_draw_rect;
        alert_update_rect.Union( alert_rect );

        //  Invalidate the rectangular region
        RefreshRect( alert_update_rect, false );
    }

    //  Save this rectangle for next time
    alert_draw_rect = alert_rect;

    temp_dc.SelectObject( wxNullBitmap );      // clean up
}

void ChartCanvas::UpdateAIS()
{
    if(!g_pAIS) return;

    //  Get the rectangle in the current dc which bounds the detected AIS targets

    //  Use this dc
    wxClientDC dc( this );

    // Get dc boundary
    int sx, sy;
    dc.GetSize( &sx, &sy );

    wxRect ais_rect;

    //  How many targets are there?

    //  If more than "some number", it will be cheaper to refresh the entire screen
    //  than to build update rectangles for each target.
    AIS_Target_Hash *current_targets = g_pAIS->GetTargetList();
    if( current_targets->size() > 10 ) {
        ais_rect = wxRect( 0, 0, sx, sy );            // full screen
    } else {
        //  Need a bitmap
        wxBitmap test_bitmap( sx, sy, -1 );

        // Create a memory DC
        wxMemoryDC temp_dc;
        temp_dc.SelectObject( test_bitmap );

        temp_dc.ResetBoundingBox();
        temp_dc.DestroyClippingRegion();
        temp_dc.SetClippingRegion( wxRect( 0, 0, sx, sy ) );

        // Draw the AIS Targets on the temp_dc
        ocpnDC ocpndc = ocpnDC( temp_dc );
        AISDraw( ocpndc );
        AISDrawAreaNotices( ocpndc );

        //  Retrieve the drawing extents
        ais_rect = wxRect( temp_dc.MinX(), temp_dc.MinY(), temp_dc.MaxX() - temp_dc.MinX(),
                           temp_dc.MaxY() - temp_dc.MinY() );

        if( !ais_rect.IsEmpty() ) ais_rect.Inflate( 2 );              // clear all drawing artifacts

        temp_dc.SelectObject( wxNullBitmap );      // clean up

    }

    if( !ais_rect.IsEmpty() || !ais_draw_rect.IsEmpty() ) {
        //  The required invalidate rectangle is the union of the last drawn rectangle
        //  and this drawn rectangle
        wxRect ais_update_rect = ais_draw_rect;
        ais_update_rect.Union( ais_rect );

        //  Invalidate the rectangular region
        RefreshRect( ais_update_rect, false );
    }

    //  Save this rectangle for next time
    ais_draw_rect = ais_rect;

}

void ChartCanvas::OnActivate( wxActivateEvent& event )
{
    ReloadVP();
}

void ChartCanvas::OnSize( wxSizeEvent& event )
{

    GetClientSize( &m_canvas_width, &m_canvas_height );

//        Constrain the active width to be mod 4

    int wr = m_canvas_width / 4;
    m_canvas_width = wr * 4;

//    Get some canvas metrics

//          Rescale to current value, in order to rebuild VPoint data structures
//          for new canvas size
    SetVPScale( GetVPScale() );

    double display_size_meters = wxGetDisplaySizeMM().GetWidth() / 1000.; // gives screen size(width) in meters
//        m_canvas_scale_factor = m_canvas_width / display_size_meters;
    m_canvas_scale_factor = wxGetDisplaySize().GetWidth() / display_size_meters;

    m_absolute_min_scale_ppm = m_canvas_width / ( .95 * WGS84_semimajor_axis_meters * PI ); // something like 180 degrees

#ifdef USE_S57
    if( ps52plib ) ps52plib->SetPPMM( m_canvas_scale_factor / 1000. );
#endif

    //  Inform the parent Frame that I am being resized...
    gFrame->ProcessCanvasResize();

//    Set up the scroll margins
    xr_margin = m_canvas_width * 95 / 100;
    xl_margin = m_canvas_width * 5 / 100;
    yt_margin = m_canvas_height * 5 / 100;
    yb_margin = m_canvas_height * 95 / 100;

    if( m_pQuilt ) m_pQuilt->SetQuiltParameters( m_canvas_scale_factor, m_canvas_width );

//    Resize the current viewport

    VPoint.pix_width = m_canvas_width;
    VPoint.pix_height = m_canvas_height;

    // Resize the scratch BM
    delete pscratch_bm;
    pscratch_bm = new wxBitmap( VPoint.pix_width, VPoint.pix_height, -1 );

    // Resize the Route Calculation BM
    m_dc_route.SelectObject( wxNullBitmap );
    delete proute_bm;
    proute_bm = new wxBitmap( VPoint.pix_width, VPoint.pix_height, -1 );
    m_dc_route.SelectObject( *proute_bm );

    //  Resize the saved Bitmap
    m_cached_chart_bm.Create( VPoint.pix_width, VPoint.pix_height, -1 );

    //  Resize the working Bitmap
    m_working_bm.Create( VPoint.pix_width, VPoint.pix_height, -1 );

    //  Rescale again, to capture all the changes for new canvas size
    SetVPScale( GetVPScale() );

    if( g_bopengl && m_glcc ) {
        m_glcc->OnSize( event );
    }
    //  Invalidate the whole window
    ReloadVP();
}

void ChartCanvas::ShowChartInfoWindow( int x, int y, int dbIndex )
{
    if( dbIndex >= 0 ) {
        if( NULL == m_pCIWin ) {
            m_pCIWin = new ChInfoWin( this );
            m_pCIWin->Hide();
        }

        if( !m_pCIWin->IsShown() ) {
            wxString s;
            ChartBase *pc = NULL;

            if( ( ChartData->IsChartInCache( dbIndex ) ) && ChartData->IsValid() )
                pc = ChartData->OpenChartFromDB( dbIndex, FULL_INIT );   // this must come from cache

            int char_width, char_height;
            s = ChartData->GetFullChartInfo( pc, dbIndex, &char_width, &char_height );
            m_pCIWin->SetString( s );
            m_pCIWin->FitToChars( char_width, char_height );

            wxPoint p;
            p.x = x;
            if( ( p.x + m_pCIWin->GetWinSize().x ) > m_canvas_width )
                p.x = m_canvas_width - m_pCIWin->GetWinSize().x;

            int statsW, statsH;
            stats->GetSize( &statsW, &statsH );
            p.y = m_canvas_height - statsH - 4 - m_pCIWin->GetWinSize().y;

            m_pCIWin->SetPosition( p );
            m_pCIWin->SetBitmap();
            m_pCIWin->Refresh();
            m_pCIWin->Show();
        }
    } else {
        HideChartInfoWindow();
    }
}

void ChartCanvas::HideChartInfoWindow( void )
{
    if( m_pCIWin && m_pCIWin->IsShown() ) m_pCIWin->Hide();
}

void ChartCanvas::PanTimerEvent( wxTimerEvent& event )
{
    wxMouseEvent ev( wxEVT_MOTION );
    ev.m_x = mouse_x;
    ev.m_y = mouse_y;
    ev.m_leftDown = mouse_leftisdown;

    wxEvtHandler *evthp = GetEventHandler();

    ::wxPostEvent( evthp, ev );

}

bool ChartCanvas::CheckEdgePan( int x, int y, bool bdragging )
{
    bool bft = false;
    int pan_margin = m_canvas_width * 5 / 100;
    int pan_timer_set = 200;
    double pan_delta = GetVP().pix_width / 50;
    int pan_x = 0;
    int pan_y = 0;

    if( x > m_canvas_width - pan_margin ) {
        bft = true;
        pan_x = pan_delta;
    }

    else if( x < pan_margin ) {
        bft = true;
        pan_x = -pan_delta;
    }

    if( y < pan_margin ) {
        bft = true;
        pan_y = -pan_delta;
    }

    else if( y > m_canvas_height - pan_margin ) {
        bft = true;
        pan_y = pan_delta;
    }

    //    Of course, if dragging, and the mouse left button is not down, we must stop the event injection
    if( bdragging ) {
        wxMouseState state = ::wxGetMouseState();
        if( !state.LeftDown() ) bft = false;
    }

    if( ( bft ) && !pPanTimer->IsRunning() ) {
        PanCanvas( pan_x, pan_y );

        pPanTimer->Start( pan_timer_set, wxTIMER_ONE_SHOT );
        return true;
    }

    //    This mouse event must not be due to pan timer event injector
    //    Mouse is out of the pan zone, so prevent any orphan event injection
    if( ( !bft ) && pPanTimer->IsRunning() ) {
        pPanTimer->Stop();
    }

    return ( false );
}

// Look for waypoints at the current position.
// Used to determine what a mouse event should act on.

void ChartCanvas::FindRoutePointsAtCursor( float selectRadius, bool setBeingEdited ) {

    m_pRoutePointEditTarget = NULL;
    m_pFoundPoint = NULL;

    SelectItem *pFind = NULL;
    SelectableItemList SelList = pSelect->FindSelectionList( m_cursor_lat, m_cursor_lon,
                                 SELTYPE_ROUTEPOINT );
    wxSelectableItemListNode *node = SelList.GetFirst();
    while( node ) {
        pFind = node->GetData();

        RoutePoint *frp = (RoutePoint *) pFind->m_pData1;

        //    Get an array of all routes using this point
        m_pEditRouteArray = g_pRouteMan->GetRouteArrayContaining( frp );

        // Use route array to determine actual visibility for the point
        bool brp_viz = false;
        if( m_pEditRouteArray ) {
            for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                if( pr->IsVisible() ) {
                    brp_viz = true;
                    break;
                }
            }
        } else
            brp_viz = frp->IsVisible();               // isolated point

        if( brp_viz ) {
            //    Use route array to rubberband all affected routes
            if( m_pEditRouteArray )                 // Editing Waypoint as part of route
            {
                for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                    Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                    pr->m_bIsBeingEdited = setBeingEdited;
                }
                m_bRouteEditing = setBeingEdited;

            } else                                      // editing Mark
            {
                frp->m_bIsBeingEdited = setBeingEdited;
                m_bMarkEditing = setBeingEdited;
            }

            m_pRoutePointEditTarget = frp;
            m_pFoundPoint = pFind;
            break;            // out of the while(node)
        }

        node = node->GetNext();
    }       // while (node)
}

void ChartCanvas::MouseTimedEvent( wxTimerEvent& event )
{
    if( singleClickEventIsValid ) MouseEvent( singleClickEvent );
    singleClickEventIsValid = false;
    m_DoubleClickTimer->Stop();
}

void ChartCanvas::MouseEvent( wxMouseEvent& event )
{
    int x, y;
    int mx, my;

    // Protect from leftUp's coming from event handlers in child
    // windows who return focus to the canvas.
    static bool leftIsDown = false;

    // Protect from very small cursor slips during double click, which produce a
    // single Drag event.
    static bool lastEventWasDrag = false;

    if( event.Dragging() && !lastEventWasDrag ) {
        lastEventWasDrag = true;
        return;
    }
    lastEventWasDrag = event.Dragging();

    event.GetPosition( &x, &y );

#ifdef __WXMSW__
    //TODO Test carefully in other platforms, remove ifdef....
    if( event.ButtonDown() && !HasCapture() ) CaptureMouse();
    if( event.ButtonUp() && HasCapture() ) ReleaseMouse();
#endif

    // We start with Double Click processing. The first left click just starts a timer and
    // is remembered, then we actually do something if there is a LeftDClick.
    // If there is, the two single clicks are ignored.

    if( event.LeftDClick() && ( cursor_region == CENTER ) ) {
        m_DoubleClickTimer->Start();
        singleClickEventIsValid = false;

        double zlat, zlon;
        GetCanvasPixPoint( x, y, zlat, zlon );

        SelectItem *pFindAIS;
        pFindAIS = pSelectAIS->FindSelection( zlat, zlon, SELTYPE_AISTARGET );

        if( pFindAIS ) {
            m_FoundAIS_MMSI = pFindAIS->GetUserData();
            if( g_pAIS->Get_Target_Data_From_MMSI( m_FoundAIS_MMSI ) ) {
                wxWindow *pwin = wxDynamicCast(this, wxWindow);
                ShowAISTargetQueryDialog( pwin, m_FoundAIS_MMSI );
            }
            return;
        }

        if( m_pRoutePointEditTarget ) {
            ShowMarkPropertiesDialog( m_pRoutePointEditTarget );
            return;
        }

        SelectItem* cursorItem;
        cursorItem = pSelect->FindSelection( zlat, zlon, SELTYPE_ROUTESEGMENT );

        if( cursorItem ) {
            Route *pr = (Route *) cursorItem->m_pData3;
            if( pr->IsVisible() ) {
                ShowRoutePropertiesDialog( _("Route Properties"), pr );
                return;
            }
        }

        cursorItem = pSelect->FindSelection( zlat, zlon, SELTYPE_TRACKSEGMENT );

        if( cursorItem ) {
            Route *pr = (Route *) cursorItem->m_pData3;
            if( pr->IsVisible() ) {
                ShowRoutePropertiesDialog( _("Track Properties"), pr );
                return;
            }
        }

        // Found no object to act on, so show chart info.

        ShowObjectQueryWindow( x, y, zlat, zlon );
        return;
    }

    // Capture LeftUp's and time them, unless it already came from the timer.
    if( event.LeftUp() && !singleClickEventIsValid ) {

        // Ignore the second LeftUp after the DClick.
        if( m_DoubleClickTimer->IsRunning() ) {
            m_DoubleClickTimer->Stop();
            return;
        }

        // Save the event for later running if there is no DClick.
        m_DoubleClickTimer->Start( 250, wxTIMER_ONE_SHOT );
        singleClickEvent = event;
        singleClickEventIsValid = true;
        return;
    }

    //  This logic is necessary on MSW to handle the case where
    //  a context (right-click) menu is dismissed without action
    //  by clicking on the chart surface.
    //  We need to avoid an unintentional pan by eating some clicks...
#ifdef __WXMSW__
    if( event.LeftDown() || event.LeftUp() || event.Dragging() ) {
        if( g_click_stop > 0 ) {
            g_click_stop--;
            return;
        }
    }
#endif

    if( s_ProgDialog ) return;

    if( ( m_bMeasure_Active && ( m_nMeasureState >= 2 ) ) || ( parent_frame->nRoute_State > 1 )
            || ( parent_frame->nRoute_State ) > 1 ) {
        wxPoint p = ClientToScreen( wxPoint( x, y ) );
        gFrame->SubmergeToolbarIfOverlap( p.x, p.y, 20 );
    }

    //  Kick off the Rotation control timer
    if( g_bCourseUp ) {
        m_b_rot_hidef = false;
        pRotDefTimer->Start( 500, wxTIMER_ONE_SHOT );
    }

    mouse_x = x;
    mouse_y = y;
    mouse_leftisdown = event.LeftIsDown();

//      Retrigger the route leg popup timer
    if( m_pRolloverWin && m_pRolloverWin->IsActive() ) m_RouteLegPopupTimer.Start( 10,
                wxTIMER_ONE_SHOT );               // faster response while the rollover is turned on
    else
        m_RouteLegPopupTimer.Start( m_routeleg_popup_timer_msec, wxTIMER_ONE_SHOT );

//  Retrigger the cursor tracking timer
    pCurTrackTimer->Start( m_curtrack_timer_msec, wxTIMER_ONE_SHOT );

    mx = x;
    my = y;
    GetCanvasPixPoint( x, y, m_cursor_lat, m_cursor_lon );

    //    Calculate meaningful SelectRadius
    float SelectRadius;
    int sel_rad_pix = 8;
    SelectRadius = sel_rad_pix / ( m_true_scale_ppm * 1852 * 60 );  // Degrees, approximately

//      Show cursor position on Status Bar, if present
//      except for GTK, under which status bar updates are very slow
//      due to Update() call.
//      In this case, as a workaround, update the status window
//      after an interval timer (pCurTrackTimer) pops, which will happen
//      whenever the mouse has stopped moving for specified interval.
//      See the method OnCursorTrackTimerEvent()
#ifndef __WXGTK__
    if( parent_frame->m_pStatusBar ) {
        double show_cursor_lon = m_cursor_lon;
        double show_cursor_lat = m_cursor_lat;

        //    Check the absolute range of the cursor position
        //    There could be a window wherein the chart geoereferencing is not valid....
        if( ( fabs( show_cursor_lat ) < 90. ) && ( fabs( show_cursor_lon ) < 360. ) ) {
            while( show_cursor_lon < -180. )
                show_cursor_lon += 360.;

            while( show_cursor_lon > 180. )
                show_cursor_lon -= 360.;

            wxString s1 = _T(" ");
            s1 += toSDMM( 1, show_cursor_lat );
            s1 += _T("   ");
            s1 += toSDMM( 2, show_cursor_lon );
            parent_frame->SetStatusText( s1, STAT_FIELD_CURSOR_LL );

            double brg, dist;
            DistanceBearingMercator( m_cursor_lat, m_cursor_lon, gLat, gLon, &brg, &dist );
            wxString s;
            s.Printf( wxString( "%03d°  ", wxConvUTF8 ), (int) brg );
            s << FormatDistanceAdaptive( dist );
            parent_frame->SetStatusText( s, STAT_FIELD_CURSOR_BRGRNG );
        }
    }
#endif

    //  Send the current cursor lat/lon to all PlugIns requesting it
    if( g_pi_manager ) g_pi_manager->SendCursorLatLonToAllPlugIns( m_cursor_lat, m_cursor_lon );

    //        Check for wheel rotation
    m_mouse_wheel_oneshot = 50;                  //msec
    // ideally, should be just longer than the time between
    // processing accumulated mouse events from the event queue
    // as would happen during screen redraws.
    int wheel_dir = event.GetWheelRotation();

    if( m_MouseWheelTimer.IsRunning() ) {
        if( wheel_dir != m_last_wheel_dir ) m_MouseWheelTimer.Stop();
        else
            m_MouseWheelTimer.Start( m_mouse_wheel_oneshot, true );           // restart timer
    }

    m_last_wheel_dir = wheel_dir;

    if( wheel_dir ) {
        if( !m_MouseWheelTimer.IsRunning() ) {
            double factor = m_bmouse_key_mod ? 1.1 : 2.0;

            if( g_bEnableZoomToCursor ) {
                bool b_zoom_moved = false;
                if( wheel_dir > 0 ) b_zoom_moved = ZoomCanvasIn( factor );
                else if( wheel_dir < 0 ) b_zoom_moved = ZoomCanvasOut( factor );

                wxPoint r;
                GetCanvasPointPix( m_cursor_lat, m_cursor_lon, &r );
                PanCanvas( r.x - x, r.y - y );
                ClearbFollow();      // update the follow flag
            } else {
                if( wheel_dir > 0 ) ZoomCanvasIn( factor );
                else if( wheel_dir < 0 ) ZoomCanvasOut( factor );
            }

            m_MouseWheelTimer.Start( m_mouse_wheel_oneshot, true );           // start timer
        }
    }

//    Route Creation Rubber Banding
    if( parent_frame->nRoute_State >= 2 ) {
        r_rband.x = x;
        r_rband.y = y;
        m_bDrawingRoute = true;

        CheckEdgePan( x, y, event.Dragging() );
        Refresh( false );
    }

//    Measure Tool Rubber Banding
    if( m_bMeasure_Active && ( m_nMeasureState >= 2 ) ) {
        r_rband.x = x;
        r_rband.y = y;
        m_bDrawingRoute = true;

        CheckEdgePan( x, y, event.Dragging() );
        Refresh( false );
    }

    bool showRollover = false;
//    AIS Target Rollover
    if( g_pAIS && g_pAIS->GetNumTargets() && g_bShowAIS ) {
        SelectItem *pFind = pSelectAIS->FindSelection( m_cursor_lat, m_cursor_lon,
                            SELTYPE_AISTARGET );
        if( pFind ) {
            int FoundAIS_MMSI = (long) pFind->m_pData1; // cast to long avoids problems with 64bit compilers
            AIS_Target_Data *ptarget = g_pAIS->Get_Target_Data_From_MMSI( FoundAIS_MMSI );

            if( ptarget ) {
                showRollover = true;

                if( NULL == m_pAISRolloverWin ) {
                    m_pAISRolloverWin = new RolloverWin( this, 10 );
                    m_pAISRolloverWin->IsActive( false );
                    Refresh();
                }

                if( !m_pAISRolloverWin->IsActive() ) {

                    wxString s = ptarget->GetRolloverString();
                    m_pAISRolloverWin->SetString( s );

                    wxSize win_size = GetSize();
                    if( console->IsShown() ) win_size.x -= console->GetSize().x;
                    m_pAISRolloverWin->SetBestPosition( x, y, 16, 16, AIS_ROLLOVER, win_size );

                    m_pAISRolloverWin->SetBitmap( AIS_ROLLOVER );
                    m_pAISRolloverWin->IsActive( true );
                    Refresh();
                }
            }
        }
    }

    if( m_pAISRolloverWin && m_pAISRolloverWin->IsActive() && !showRollover ) {
        m_pAISRolloverWin->IsActive( false );
        Refresh();
    }

//          Mouse Clicks

    if( event.LeftDown() ) {
        //  This really should not be needed, but....
        //  on Windows, when using wxAUIManager, sometimes the focus is lost
        //  when clicking into another pane, e.g.the AIS target list, and then back to this pane.
        //  Oddly, some mouse events are not lost, however.  Like this one....
        SetFocus();

        last_drag.x = mx;
        last_drag.y = my;
        leftIsDown = true;

        if( parent_frame->nRoute_State )                  // creating route?
        {
            double rlat, rlon;

            SetCursor( *pCursorPencil );
            rlat = m_cursor_lat;
            rlon = m_cursor_lon;

            m_bRouteEditing = true;

            if( parent_frame->nRoute_State == 1 ) {
                m_pMouseRoute = new Route();
                pRouteList->Append( m_pMouseRoute );
                r_rband.x = x;
                r_rband.y = y;
            }

            //    Check to see if there is a nearby point which may be reused
            RoutePoint *pMousePoint = NULL;

            //    Calculate meaningful SelectRadius
            int nearby_sel_rad_pix = 8;
            double nearby_radius_meters = nearby_sel_rad_pix / m_true_scale_ppm;

            RoutePoint *pNearbyPoint = pWayPointMan->GetNearbyWaypoint( rlat, rlon,
                                       nearby_radius_meters );
            if( pNearbyPoint && ( pNearbyPoint != m_prev_pMousePoint )
                    && !pNearbyPoint->m_bIsInTrack && !pNearbyPoint->m_bIsInLayer )
            {
                int dlg_return = OCPNMessageBox( this, _("Use nearby waypoint?"),
                                                  _("OpenCPN Route Create"),
                                                  (long) wxYES_NO | wxCANCEL | wxYES_DEFAULT );
//                int dlg_return = near_point_dlg.ShowModal();

                if( dlg_return == wxID_YES ) {
                    pMousePoint = pNearbyPoint;

                    // Using existing waypoint, so nothing to delete for undo.
                    if( parent_frame->nRoute_State > 1 )
                        undo->BeforeUndoableAction( Undo_AppendWaypoint, pMousePoint, Undo_HasParent, NULL );

                    // check all other routes to see if this point appears in any other route
                    // If it appears in NO other route, then it should e considered an isolated mark
                    if( !g_pRouteMan->FindRouteContainingWaypoint( pMousePoint ) ) pMousePoint->m_bKeepXRoute =
                            true;
                }
            }

            if( NULL == pMousePoint ) {                 // need a new point
                pMousePoint = new RoutePoint( rlat, rlon, _T("diamond"), _T(""), GPX_EMPTY_STRING );
                pMousePoint->SetNameShown( false );

                pConfig->AddNewWayPoint( pMousePoint, -1 );    // use auto next num
                pSelect->AddSelectableRoutePoint( rlat, rlon, pMousePoint );

                if( parent_frame->nRoute_State > 1 )
                    undo->BeforeUndoableAction( Undo_AppendWaypoint, pMousePoint, Undo_IsOrphanded, NULL );
            }

            if( parent_frame->nRoute_State == 1 ) {
                // First point in the route.
                m_pMouseRoute->AddPoint( pMousePoint );
            } else {
                if( m_pMouseRoute->m_NextLegGreatCircle ) {
                    double rhumbBearing, rhumbDist, gcBearing, gcDist;
                    DistanceBearingMercator( rlat, rlon, m_prev_rlat, m_prev_rlon, &rhumbBearing, &rhumbDist );
                    Geodesic::GreatCircleDistBear( m_prev_rlon, m_prev_rlat, rlon, rlat, &gcDist, &gcBearing, NULL );
                    double gcDistNM = gcDist / 1852.0;

                    // Empirically found expression to get reasonable route segments.
                    int segmentCount = (3.0 + (rhumbDist - gcDistNM)) / pow(rhumbDist-gcDistNM-1, 0.5 );

                    wxString msg;
                    msg << _("For this leg the Great Circle route is ")
                        << FormatDistanceAdaptive( rhumbDist - gcDistNM ) << _(" shorter than rhumbline.\n\n")
                        << _("Would you like include the Great Circle routing points for this leg?");

                    int answer = OCPNMessageBox( this, msg, _("OpenCPN Route Create"), wxYES_NO | wxNO_DEFAULT );

                    if( answer == wxID_YES ) {
                        RoutePoint* gcPoint;
                        RoutePoint* prevGcPoint = m_prev_pMousePoint;
                        wxRealPoint gcCoord;

                        for( int i = 1; i <= segmentCount; i++ ) {
                            double fraction = (double) i * ( 1.0 / (double) segmentCount );
                            Geodesic::GreatCircleTravel( m_prev_rlon, m_prev_rlat, gcDist * fraction,
                                    gcBearing, &gcCoord.x, &gcCoord.y, NULL );

                            if( i < segmentCount ) {
                                gcPoint = new RoutePoint( gcCoord.y, gcCoord.x, _T("xmblue"), _T(""),
                                        GPX_EMPTY_STRING );
                                gcPoint->SetNameShown( false );
                                pConfig->AddNewWayPoint( gcPoint, -1 );
                                pSelect->AddSelectableRoutePoint( gcCoord.y, gcCoord.x, gcPoint );
                            } else {
                                gcPoint = pMousePoint; // Last point, previously exsisting!
                            }

                            m_pMouseRoute->AddPoint( gcPoint );
                            pSelect->AddSelectableRouteSegment( prevGcPoint->m_lat, prevGcPoint->m_lon,
                                    gcPoint->m_lat, gcPoint->m_lon, prevGcPoint, gcPoint, m_pMouseRoute );
                            prevGcPoint = gcPoint;
                        }

                        undo->CancelUndoableAction( true );

                    } else {
                        m_pMouseRoute->AddPoint( pMousePoint );
                        pSelect->AddSelectableRouteSegment( m_prev_rlat, m_prev_rlon,
                                rlat, rlon, m_prev_pMousePoint, pMousePoint, m_pMouseRoute );
                        undo->AfterUndoableAction( m_pMouseRoute );
                    }
                } else {
                    // Ordinary rhumblinesegment.
                    m_pMouseRoute->AddPoint( pMousePoint );
                    pSelect->AddSelectableRouteSegment( m_prev_rlat, m_prev_rlon,
                            rlat, rlon, m_prev_pMousePoint, pMousePoint, m_pMouseRoute );
                    undo->AfterUndoableAction( m_pMouseRoute );
                }
            }

            m_prev_rlat = rlat;
            m_prev_rlon = rlon;
            m_prev_pMousePoint = pMousePoint;
            m_pMouseRoute->m_lastMousePointIndex = m_pMouseRoute->GetnPoints();

            parent_frame->nRoute_State++;
            Refresh( false );
        }

        else if( m_bMeasure_Active && m_nMeasureState )   // measure tool?
        {
            double rlat, rlon;

            SetCursor( *pCursorPencil );
            rlat = m_cursor_lat;
            rlon = m_cursor_lon;

            if( m_nMeasureState == 1 ) {
                m_pMeasureRoute = new Route();
                pRouteList->Append( m_pMeasureRoute );
                r_rband.x = x;
                r_rband.y = y;
            }

            RoutePoint *pMousePoint = new RoutePoint( m_cursor_lat, m_cursor_lon,
                    wxString( _T ( "circle" ) ), wxEmptyString, GPX_EMPTY_STRING );
            pMousePoint->m_bShowName = false;

            m_pMeasureRoute->AddPoint( pMousePoint );

            m_prev_rlat = m_cursor_lat;
            m_prev_rlon = m_cursor_lon;
            m_prev_pMousePoint = pMousePoint;
            m_pMeasureRoute->m_lastMousePointIndex = m_pMeasureRoute->GetnPoints();

            m_nMeasureState++;

            Refresh( false );
        }

        else {
            FindRoutePointsAtCursor( SelectRadius, true );    // Not creating Route
        }
    }

    if( event.Dragging() ) {
        if( m_bRouteEditing && m_pRoutePointEditTarget ) {

            bool DraggingAllowed = true;

            if( NULL == pMarkPropDialog ) {
                if( g_bWayPointPreventDragging ) DraggingAllowed = false;
            } else if( !pMarkPropDialog->IsShown() && g_bWayPointPreventDragging ) DraggingAllowed =
                    false;

            if( m_pRoutePointEditTarget && ( m_pRoutePointEditTarget->m_IconName == _T("mob") ) ) DraggingAllowed =
                    false;

            if( m_pRoutePointEditTarget->m_bIsInLayer ) DraggingAllowed = false;

            if( DraggingAllowed ) {

                if( !undo->InUndoableAction() ) {
                    undo->BeforeUndoableAction( Undo_MoveWaypoint, m_pRoutePointEditTarget,
                            Undo_NeedsCopy, m_pFoundPoint );
                }

                // Get the update rectangle for the union of the un-edited routes
                wxRect pre_rect;

                if( m_pEditRouteArray ) {
                    for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                        Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                        wxRect route_rect;
                        pr->CalculateDCRect( m_dc_route, &route_rect, VPoint );
                        pre_rect.Union( route_rect );
                    }
                }

                m_pRoutePointEditTarget->m_lat = m_cursor_lat;     // update the RoutePoint entry
                m_pRoutePointEditTarget->m_lon = m_cursor_lon;
                m_pFoundPoint->m_slat = m_cursor_lat;             // update the SelectList entry
                m_pFoundPoint->m_slon = m_cursor_lon;

                if( CheckEdgePan( x, y, true ) ) {
                    double new_cursor_lat, new_cursor_lon;
                    GetCanvasPixPoint( x, y, new_cursor_lat, new_cursor_lon );
                    m_pRoutePointEditTarget->m_lat = new_cursor_lat;  // update the RoutePoint entry
                    m_pRoutePointEditTarget->m_lon = new_cursor_lon;
                    m_pFoundPoint->m_slat = new_cursor_lat;           // update the SelectList entry
                    m_pFoundPoint->m_slon = new_cursor_lon;
                }

                //    Update the MarkProperties Dialog, if currently shown
                if( ( NULL != pMarkPropDialog ) && ( pMarkPropDialog->IsShown() ) ) {
                    if( m_pRoutePointEditTarget == pMarkPropDialog->GetRoutePoint() ) pMarkPropDialog->UpdateProperties();
                }

                // Get the update rectangle for the edited route
                wxRect post_rect;

                if( m_pEditRouteArray ) {
                    for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                        Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                        wxRect route_rect;
                        pr->CalculateDCRect( m_dc_route, &route_rect, VPoint );
                        post_rect.Union( route_rect );
                    }
                }

                //    Invalidate the union region
                pre_rect.Union( post_rect );
                RefreshRect( pre_rect, false );
            }
        }     // if Route Editing

        else if( m_bMarkEditing && m_pRoutePointEditTarget ) {

            bool DraggingAllowed = true;

            if( NULL == pMarkPropDialog ) {
                if( g_bWayPointPreventDragging ) DraggingAllowed = false;
            } else if( !pMarkPropDialog->IsShown() && g_bWayPointPreventDragging ) DraggingAllowed =
                    false;

            if( m_pRoutePointEditTarget
                    && ( m_pRoutePointEditTarget->m_IconName == _T("mob") ) ) DraggingAllowed =
                            false;

            if( m_pRoutePointEditTarget->m_bIsInLayer ) DraggingAllowed = false;

            if( DraggingAllowed ) {
                if( !undo->InUndoableAction() ) {
                    undo->BeforeUndoableAction( Undo_MoveWaypoint, m_pRoutePointEditTarget,
                            Undo_NeedsCopy, m_pFoundPoint );
                }

                //      The mark may be an anchorwatch
                double lpp1 = 0.;
                double lpp2 = 0.;
                double lppmax;

                if( pAnchorWatchPoint1 == m_pRoutePointEditTarget ) {
                    lpp1 = fabs( GetAnchorWatchRadiusPixels( pAnchorWatchPoint1 ) );

                }
                if( pAnchorWatchPoint2 == m_pRoutePointEditTarget ) {
                    lpp2 = fabs( GetAnchorWatchRadiusPixels( pAnchorWatchPoint2 ) );
                }
                lppmax = wxMax(lpp1 + 10, lpp2 + 10);         // allow for cruft

                // Get the update rectangle for the un-edited mark
                wxRect pre_rect;
                m_pRoutePointEditTarget->CalculateDCRect( m_dc_route, &pre_rect );
                if( ( lppmax > pre_rect.width / 2 ) || ( lppmax > pre_rect.height / 2 ) ) pre_rect.Inflate(
                        (int) ( lppmax - ( pre_rect.width / 2 ) ),
                        (int) ( lppmax - ( pre_rect.height / 2 ) ) );
                m_pRoutePointEditTarget->m_lat = m_cursor_lat;    // update the RoutePoint entry
                m_pRoutePointEditTarget->m_lon = m_cursor_lon;
                m_pFoundPoint->m_slat = m_cursor_lat;             // update the SelectList entry
                m_pFoundPoint->m_slon = m_cursor_lon;

                //    Update the MarkProperties Dialog, if currently shown
                if( ( NULL != pMarkPropDialog ) && ( pMarkPropDialog->IsShown() ) ) {
                    if( m_pRoutePointEditTarget == pMarkPropDialog->GetRoutePoint() )
                        pMarkPropDialog->UpdateProperties( true );
                }

                // Get the update rectangle for the edited mark
                wxRect post_rect;
                m_pRoutePointEditTarget->CalculateDCRect( m_dc_route, &post_rect );
                if( ( lppmax > post_rect.width / 2 ) || ( lppmax > post_rect.height / 2 ) ) post_rect.Inflate(
                        (int) ( lppmax - ( post_rect.width / 2 ) ),
                        (int) ( lppmax - ( post_rect.height / 2 ) ) );

//                        post_rect.Inflate(200);
                //    Invalidate the union region
                pre_rect.Union( post_rect );
                RefreshRect( pre_rect, false );
            }
        }

        // must be chart dragging...
        else if( leftIsDown ) {
            if( ( last_drag.x != mx ) || ( last_drag.y != my ) ) {
                m_bChartDragging = true;
                PanCanvas( last_drag.x - mx, last_drag.y - my );

                last_drag.x = mx;
                last_drag.y = my;

                Refresh( false );
            }
        }
    }

    if( event.LeftUp() ) {
        if( m_bRouteEditing ) {
            if( m_pRoutePointEditTarget ) {
                pSelect->UpdateSelectableRouteSegments( m_pRoutePointEditTarget );

                if( m_pEditRouteArray ) {
                    for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                        Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                        pr->CalculateBBox();
                        pr->UpdateSegmentDistances();
                        pr->m_bIsBeingEdited = false;

                        pConfig->UpdateRoute( pr );
                    }
                }

                //    Update the RouteProperties Dialog, if currently shown
                if( ( NULL != pRoutePropDialog ) && ( pRoutePropDialog->IsShown() ) ) {
                    if( m_pEditRouteArray ) {
                        for( unsigned int ir = 0; ir < m_pEditRouteArray->GetCount(); ir++ ) {
                            Route *pr = (Route *) m_pEditRouteArray->Item( ir );
                            if( pRoutePropDialog->m_pRoute == pr ) {
                                pRoutePropDialog->SetRouteAndUpdate( pr );
                                pRoutePropDialog->UpdateProperties();
                            }
                        }
                    }
                }

                m_pRoutePointEditTarget->m_bPtIsSelected = false;

                delete m_pEditRouteArray;
                m_pEditRouteArray = NULL;
                undo->AfterUndoableAction( m_pRoutePointEditTarget );
            }

            m_bRouteEditing = false;
            m_pRoutePointEditTarget = NULL;
            if( !g_FloatingToolbarDialog->IsShown() ) gFrame->SurfaceToolbar();
        }

        else if( m_bMarkEditing ) {
            if( m_pRoutePointEditTarget ) {
                pConfig->UpdateWayPoint( m_pRoutePointEditTarget );
                undo->AfterUndoableAction( m_pRoutePointEditTarget );
                m_pRoutePointEditTarget->m_bIsBeingEdited = false;
                m_pRoutePointEditTarget->m_bPtIsSelected = false;
            }
            m_pRoutePointEditTarget = NULL;
            m_bMarkEditing = false;
            if( !g_FloatingToolbarDialog->IsShown() ) gFrame->SurfaceToolbar();
        }

        else if( leftIsDown ) {  // left click for chart center
            leftIsDown = false;

            if( !m_bChartDragging && !m_bMeasure_Active ) {
                switch( cursor_region ){
                    case MID_RIGHT: {
                        PanCanvas( 100, 0 );
                        break;
                    }

                    case MID_LEFT: {
                        PanCanvas( -100, 0 );
                        break;
                    }

                    case MID_TOP: {
                        PanCanvas( 0, 100 );
                        break;
                    }

                    case MID_BOT: {
                        PanCanvas( 0, -100 );
                        break;
                    }

                    case CENTER: {
                        PanCanvas( x - GetVP().pix_width / 2, y - GetVP().pix_height / 2 );
                        break;
                    }
                }
            } else {
                m_bChartDragging = false;
            }
        }
    }

    if( event.RightDown() ) {
        last_drag.x = mx;
        last_drag.y = my;

        if( parent_frame->nRoute_State )                     // creating route?
            CanvasPopupMenu( x, y, SELTYPE_ROUTECREATE );
        else                                                  // General Right Click
        {
            // Look for selectable objects
            double slat, slon;
            slat = m_cursor_lat;
            slon = m_cursor_lon;
//                      SelectItem *pFind;
 //           wxClientDC cdc( this );
//            ocpnDC dc( cdc );
#ifdef __WXMAC__
            wxScreenDC sdc;
            ocpnDC dc( sdc );
#else
            wxClientDC cdc( GetParent() );
            ocpnDC dc( cdc );
#endif

            SelectItem *pFindAIS;
            SelectItem *pFindRP;
            SelectItem *pFindRouteSeg;
            SelectItem *pFindTrackSeg;
            SelectItem *pFindCurrent = NULL;
            SelectItem *pFindTide = NULL;

            //    Deselect any current objects
            if( m_pSelectedRoute ) {
                m_pSelectedRoute->m_bRtIsSelected = false;        // Only one selection at a time
                m_pSelectedRoute->DeSelectRoute();
                m_pSelectedRoute->Draw( dc, VPoint );
            }

            if( m_pFoundRoutePoint ) {
                m_pFoundRoutePoint->m_bPtIsSelected = false;
                m_pFoundRoutePoint->Draw( dc );
                RefreshRect( m_pFoundRoutePoint->CurrentRect_in_DC );
            }

            //      Get all the selectable things at the cursor
            pFindAIS = pSelectAIS->FindSelection( slat, slon, SELTYPE_AISTARGET );
            pFindRP = pSelect->FindSelection( slat, slon, SELTYPE_ROUTEPOINT );
            pFindRouteSeg = pSelect->FindSelection( slat, slon, SELTYPE_ROUTESEGMENT );
            pFindTrackSeg = pSelect->FindSelection( slat, slon, SELTYPE_TRACKSEGMENT );
            if( m_bShowCurrent ) pFindCurrent = pSelectTC->FindSelection( slat, slon,
                                                    SELTYPE_CURRENTPOINT );

            if( m_bShowTide )                                // look for tide stations
                pFindTide = pSelectTC->FindSelection( slat, slon, SELTYPE_TIDEPOINT );

            int seltype = 0;

            //    Try for AIS targets first
            if( pFindAIS ) {
                m_FoundAIS_MMSI = pFindAIS->GetUserData();

                //      Make sure the target data is available
                if( g_pAIS->Get_Target_Data_From_MMSI( m_FoundAIS_MMSI ) ) seltype |=
                        SELTYPE_AISTARGET;
            }

            //    Now the various Route Parts

            m_pFoundRoutePoint = NULL;
            if( pFindRP ) {
                RoutePoint *pFirstVizPoint = NULL;
                RoutePoint *pFoundActiveRoutePoint = NULL;
                RoutePoint *pFoundVizRoutePoint = NULL;
                Route *pSelectedActiveRoute = NULL;
                Route *pSelectedVizRoute = NULL;

                //There is at least one routepoint, so get the whole list
                SelectableItemList SelList = pSelect->FindSelectionList( slat, slon,
                                             SELTYPE_ROUTEPOINT );
                wxSelectableItemListNode *node = SelList.GetFirst();
                while( node ) {
                    SelectItem *pFindSel = node->GetData();

                    RoutePoint *prp = (RoutePoint *) pFindSel->m_pData1;        //candidate

                    //    Get an array of all routes using this point
                    wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining( prp );

                    // Use route array (if any) to determine actual visibility for this point
                    bool brp_viz = false;
                    if( proute_array ) {
                        for( unsigned int ir = 0; ir < proute_array->GetCount(); ir++ ) {
                            Route *pr = (Route *) proute_array->Item( ir );
                            if( pr->IsVisible() ) {
                                brp_viz = true;
                                break;
                            }
                        }
                    } else
                        brp_viz = prp->IsVisible();               // isolated point

                    if( ( NULL == pFirstVizPoint ) && brp_viz ) pFirstVizPoint = prp;

                    // Use route array to choose the appropriate route
                    // Give preference to any active route, otherwise select the first visible route in the array for this point
                    m_pSelectedRoute = NULL;
                    if( proute_array ) {
                        for( unsigned int ir = 0; ir < proute_array->GetCount(); ir++ ) {
                            Route *pr = (Route *) proute_array->Item( ir );
                            if( pr->m_bRtIsActive ) {
                                pSelectedActiveRoute = pr;
                                pFoundActiveRoutePoint = prp;
                                break;
                            }
                        }

                        if( NULL == pSelectedVizRoute ) {
                            for( unsigned int ir = 0; ir < proute_array->GetCount(); ir++ ) {
                                Route *pr = (Route *) proute_array->Item( ir );
                                if( pr->IsVisible() ) {
                                    pSelectedVizRoute = pr;
                                    pFoundVizRoutePoint = prp;
                                    break;
                                }
                            }
                        }

                        delete proute_array;
                    }

                    node = node->GetNext();
                }

                //      Now choose the "best" selections
                if( pFoundActiveRoutePoint ) {
                    m_pFoundRoutePoint = pFoundActiveRoutePoint;
                    m_pSelectedRoute = pSelectedActiveRoute;
                } else if( pFoundVizRoutePoint ) {
                    m_pFoundRoutePoint = pFoundVizRoutePoint;
                    m_pSelectedRoute = pSelectedVizRoute;
                } else
                    // default is first visible point in list
                    m_pFoundRoutePoint = pFirstVizPoint;

                if( m_pSelectedRoute ) {
                    if( m_pSelectedRoute->IsVisible() ) seltype |= SELTYPE_ROUTEPOINT;
                } else if( m_pFoundRoutePoint ) seltype |= SELTYPE_MARKPOINT;
            }

            // Note here that we use SELTYPE_ROUTESEGMENT to select tracks as well as routes
            // But call the popup handler with identifier appropriate to the type
            if( pFindRouteSeg )                  // there is at least one select item
            {
                SelectableItemList SelList = pSelect->FindSelectionList( slat, slon,
                                             SELTYPE_ROUTESEGMENT );

                if( NULL == m_pSelectedRoute )  // the case where a segment only is selected
                {
                    //  Choose the first visible route containing segment in the list
                    wxSelectableItemListNode *node = SelList.GetFirst();
                    while( node ) {
                        SelectItem *pFindSel = node->GetData();

                        Route *pr = (Route *) pFindSel->m_pData3;
                        if( pr->IsVisible() ) {
                            m_pSelectedRoute = pr;
                            break;
                        }
                        node = node->GetNext();
                    }
                }

                if( m_pSelectedRoute ) {
                    if( NULL == m_pFoundRoutePoint ) m_pFoundRoutePoint =
                            (RoutePoint *) pFindRouteSeg->m_pData1;
                    m_pFoundRoutePointSecond = (RoutePoint *) pFindRouteSeg->m_pData2;

                    m_pSelectedRoute->m_bRtIsSelected = !(seltype && SELTYPE_ROUTEPOINT);
                    if( m_pSelectedRoute->m_bRtIsSelected )
                        m_pSelectedRoute->Draw( dc, GetVP() );
                    seltype |= SELTYPE_ROUTESEGMENT;
                }

            }

            if( pFindTrackSeg ) {
                m_pSelectedTrack = NULL;
                SelectableItemList SelList = pSelect->FindSelectionList( slat, slon,
                                             SELTYPE_TRACKSEGMENT );

                //  Choose the first visible track containing segment in the list
                wxSelectableItemListNode *node = SelList.GetFirst();
                while( node ) {
                    SelectItem *pFindSel = node->GetData();

                    Route *pt = (Route *) pFindSel->m_pData3;
                    if( pt->IsVisible() ) {
                        m_pSelectedTrack = pt;
                        break;
                    }
                    node = node->GetNext();
                }

                if( m_pSelectedTrack ) seltype |= SELTYPE_TRACKSEGMENT;
            }

            bool bseltc = false;
//                      if(0 == seltype)
            {
                if( pFindCurrent ) {
                    // There may be multiple current entries at the same point.
                    // For example, there often is a current substation (with directions specified)
                    // co-located with its master.  We want to select the substation, so that
                    // the direction will be properly indicated on the graphic.
                    // So, we search the select list looking for IDX_type == 'c' (i.e substation)
                    IDX_entry *pIDX_best_candidate;

                    SelectItem *pFind = NULL;
                    SelectableItemList SelList = pSelectTC->FindSelectionList( m_cursor_lat,
                                                 m_cursor_lon, SELTYPE_CURRENTPOINT );

                    //      Default is first entry
                    wxSelectableItemListNode *node = SelList.GetFirst();
                    pFind = node->GetData();
                    pIDX_best_candidate = (IDX_entry *) ( pFind->m_pData1 );

                    if( SelList.GetCount() > 1 ) {
                        node = node->GetNext();
                        while( node ) {
                            pFind = node->GetData();
                            IDX_entry *pIDX_candidate = (IDX_entry *) ( pFind->m_pData1 );
                            if( pIDX_candidate->IDX_type == 'c' ) {
                                pIDX_best_candidate = pIDX_candidate;
                                break;
                            }

                            node = node->GetNext();
                        }       // while (node)
                    } else {
                        wxSelectableItemListNode *node = SelList.GetFirst();
                        pFind = node->GetData();
                        pIDX_best_candidate = (IDX_entry *) ( pFind->m_pData1 );
                    }

                    m_pIDXCandidate = pIDX_best_candidate;

                    if( 0 == seltype ) {
                        DrawTCWindow( x, y, (void *) pIDX_best_candidate );
                        Refresh( false );
                        bseltc = true;
                    } else
                        seltype |= SELTYPE_CURRENTPOINT;
                }

                else if( pFindTide ) {
                    m_pIDXCandidate = (IDX_entry *) pFindTide->m_pData1;

                    if( 0 == seltype ) {
                        DrawTCWindow( x, y, (void *) pFindTide->m_pData1 );
                        Refresh( false );
                        bseltc = true;
                    } else
                        seltype |= SELTYPE_TIDEPOINT;
                }
            }

            if( 0 == seltype ) seltype |= SELTYPE_UNKNOWN;

            if( !bseltc ) CanvasPopupMenu( x, y, seltype );

            // Seth: Is this refresh needed?
            Refresh( false );            // needed for MSW, not GTK  Why??
        }
    }

//    Switch to the appropriate cursor on mouse movement

    wxCursor *ptarget_cursor = pCursorArrow;

    if( ( !parent_frame->nRoute_State )
            && ( !m_bMeasure_Active ) /*&& ( !m_bCM93MeasureOffset_Active )*/) {

        if( x > xr_margin ) {
            ptarget_cursor = pCursorRight;
            cursor_region = MID_RIGHT;
        } else if( x < xl_margin ) {
            ptarget_cursor = pCursorLeft;
            cursor_region = MID_LEFT;
        } else if( y > yb_margin ) {
            ptarget_cursor = pCursorDown;
            cursor_region = MID_TOP;
        } else if( y < yt_margin ) {
            ptarget_cursor = pCursorUp;
            cursor_region = MID_BOT;
        } else {
            ptarget_cursor = pCursorArrow;
            cursor_region = CENTER;
        }
    } else if( m_bMeasure_Active || parent_frame->nRoute_State ) // If Measure tool use Pencil Cursor
        ptarget_cursor = pCursorPencil;

    SetCursor( *ptarget_cursor );

}

void ChartCanvas::LostMouseCapture( wxMouseCaptureLostEvent& event )
{
    SetCursor( *pCursorArrow );
}

//-------------------------------------------------------------------------------
//          Popup Menu Handling
//-------------------------------------------------------------------------------

wxString _menuText( wxString name, wxString shortcut ) {
    wxString menutext;
    menutext << name << _T("\t") << shortcut;
    return menutext;
}

void ChartCanvas::CanvasPopupMenu( int x, int y, int seltype )
{
    wxMenu* contextMenu = new wxMenu;
    wxMenu* menuWaypoint = new wxMenu( _("Waypoint") );
    wxMenu* menuRoute = new wxMenu( _("Route") );
    wxMenu* menuTrack = new wxMenu( _("Track") );
    wxMenu* menuAIS = new wxMenu( _("AIS") );

    wxMenu *subMenuChart = new wxMenu;

    wxMenu *menuFocus = contextMenu;    // This is the one that will be shown

    popx = x;
    popy = y;

#ifdef __WXGTK__
#ifdef ocpnUSE_GTK_OPTIMIZE
    //  This code changes the background color on the popup context menu
    wxColour back_color = GetGlobalColor(_T("UIBCK"));
    GdkColor color;

    color.red = back_color.Red() << 8;
    color.green = back_color.Green() << 8;
    color.blue = back_color.Blue() << 8;

    gtk_widget_modify_bg (GTK_WIDGET(contextMenu->m_menu), GTK_STATE_NORMAL, &color);
#endif
#endif

    if( seltype == SELTYPE_ROUTECREATE ) {
#ifndef __WXOSX__
        contextMenu->Append( ID_RC_MENU_FINISH, _menuText( _( "End Route" ), _T("Esc") ) );
#else
        contextMenu->Append( ID_RC_MENU_FINISH,  _( "End Route" ) );
#endif
    }

    if( ! m_pMouseRoute ) {
        if( m_bMeasure_Active )
#ifndef __WXOSX__
            contextMenu->Prepend( ID_DEF_MENU_DEACTIVATE_MEASURE, _menuText( _("Measure Off"), _T("Esc") ) );
#else
            contextMenu->Prepend( ID_DEF_MENU_DEACTIVATE_MEASURE,  _("Measure Off") );
#endif
        else
            contextMenu->Prepend( ID_DEF_MENU_ACTIVATE_MEASURE, _menuText( _( "Measure" ), _T("F4") ) );
    }

    if( undo->AnythingToUndo() ) {
        wxString undoItem;
        undoItem << _("Undo") << _T(" ") << undo->GetNextUndoableAction()->Description();
        contextMenu->Prepend( ID_UNDO, _menuText( undoItem, _T("Ctrl-Z") ) );
    }

    if( undo->AnythingToRedo() ) {
        wxString redoItem;
        redoItem << _("Redo") << _T(" ") << undo->GetNextRedoableAction()->Description();
        contextMenu->Prepend( ID_REDO, _menuText( redoItem, _T("Ctrl-Y") ) );
    }

    if( !VPoint.b_quilt ) {
        if( parent_frame->GetnChartStack() > 1 ) {
            contextMenu->Append( ID_DEF_MENU_MAX_DETAIL, _( "Max Detail Here" ) );
            contextMenu->Append( ID_DEF_MENU_SCALE_IN, _menuText( _( "Scale In" ), _T("F7") ) );
            contextMenu->Append( ID_DEF_MENU_SCALE_OUT, _menuText( _( "Scale Out" ), _T("F8") ) );
        }

        if( Current_Ch && ( Current_Ch->GetChartFamily() == CHART_FAMILY_VECTOR ) ) {
            contextMenu->Append( ID_DEF_MENU_QUERY, _( "Object Query..." ) );
        }

    } else {
        ChartBase *pChartTest = m_pQuilt->GetChartAtPix( wxPoint( x, y ) );
        if( pChartTest && ( pChartTest->GetChartFamily() == CHART_FAMILY_VECTOR ) ) {
            contextMenu->Append( ID_DEF_MENU_QUERY, _( "Object Query..." ) );
        } else {
            if( parent_frame->GetnChartStack() > 1 ) {
                contextMenu->Append( ID_DEF_MENU_SCALE_IN, _menuText( _( "Scale In" ), _T("F7") ) );
                contextMenu->Append( ID_DEF_MENU_SCALE_OUT, _menuText( _( "Scale Out" ), _T("F8") ) );
            }
        }
    }

    contextMenu->Append( ID_DEF_MENU_DROP_WP, _menuText( _( "Drop Mark" ), _T("Ctrl-M") ) );

    if( !bGPSValid ) contextMenu->Append( ID_DEF_MENU_MOVE_BOAT_HERE, _( "Move Boat Here" ) );

    if( !( g_pRouteMan->GetpActiveRoute() || ( seltype & SELTYPE_MARKPOINT ) ) )
        contextMenu->Append( ID_DEF_MENU_GOTO_HERE, _( "Navigate To Here" ) );

    contextMenu->Append( ID_DEF_MENU_GOTOPOSITION, _("Center View...") );

    if( !g_bCourseUp ) contextMenu->Append( ID_DEF_MENU_COGUP, _("Course Up Mode") );
    else {
        if( !VPoint.b_quilt && Current_Ch && ( fabs( Current_Ch->GetChartSkew() ) > .01 )
                && !g_bskew_comp ) contextMenu->Append( ID_DEF_MENU_NORTHUP, _("Chart Up Mode") );
        else
            contextMenu->Append( ID_DEF_MENU_NORTHUP, _("North Up Mode") );
    }

    Kml* kml = new Kml;
    int pasteBuffer = kml->ParsePasteBuffer();
    if( pasteBuffer != KML_PASTE_INVALID ) {
        switch( pasteBuffer ) {
            case KML_PASTE_WAYPOINT: {
                contextMenu->Append( ID_PASTE_WAYPOINT, _( "Paste Waypoint" ) );
                break;
            }
            case KML_PASTE_ROUTE: {
                contextMenu->Append( ID_PASTE_ROUTE, _( "Paste Route" ) );
                break;
            }
            case KML_PASTE_TRACK: {
                contextMenu->Append( ID_PASTE_TRACK, _( "Paste Track" ) );
                break;
            }
            case KML_PASTE_ROUTE_TRACK: {
                contextMenu->Append( ID_PASTE_ROUTE, _( "Paste Route" ) );
                contextMenu->Append( ID_PASTE_TRACK, _( "Paste Track" ) );
                break;
            }
        }
    }
    delete kml;

    if( !VPoint.b_quilt && Current_Ch && ( Current_Ch->GetChartType() == CHART_TYPE_CM93COMP ) ) {
        contextMenu->Append( ID_DEF_MENU_CM93OFFSET_DIALOG, _( "CM93 Offset Dialog..." ) );
    }

    if( ( VPoint.b_quilt ) && ( pCurrentStack && pCurrentStack->b_valid ) ) {
        int dbIndex = m_pQuilt->GetChartdbIndexAtPix( wxPoint( popx, popy ) );
        if( dbIndex != -1 )
            contextMenu->Append( ID_DEF_MENU_QUILTREMOVE, _( "Hide This Chart" ) );
    }

#ifdef __WXMSW__
    //  If we dismiss the context menu without action, we need to discard some mouse events....
    //  Eat the next 2 button events, which happen as down-up on MSW XP
    g_click_stop = 2;
#endif

    //  ChartGroup SubMenu
    wxMenuItem* subItemChart = contextMenu->AppendSubMenu( subMenuChart, _("Chart Groups") );
    if( g_pGroupArray->GetCount() ) {
        subMenuChart->AppendRadioItem( ID_DEF_MENU_GROUPBASE, _("All Active Charts") );

        for( unsigned int i = 0; i < g_pGroupArray->GetCount(); i++ ) {
            subMenuChart->AppendRadioItem( ID_DEF_MENU_GROUPBASE + i + 1,
                                         g_pGroupArray->Item( i )->m_group_name );
            Connect( ID_DEF_MENU_GROUPBASE + i + 1, wxEVT_COMMAND_MENU_SELECTED,
                     (wxObjectEventFunction) (wxEventFunction) &ChartCanvas::PopupMenuHandler );
        }

        subMenuChart->Check( ID_DEF_MENU_GROUPBASE + g_GroupIndex, true );
    }

    //  Add PlugIn Context Menu items
    ArrayOfPlugInMenuItems item_array = g_pi_manager->GetPluginContextMenuItemArray();

    for( unsigned int i = 0; i < item_array.GetCount(); i++ ) {
        PlugInMenuItemContainer *pimis = item_array.Item( i );
        {
            if( pimis->b_viz ) {
                wxMenuItem *pmi = new wxMenuItem( contextMenu, pimis->id,
                                                  pimis->pmenu_item->GetLabel(), pimis->pmenu_item->GetHelp(),
                                                  pimis->pmenu_item->GetKind(), pimis->pmenu_item->GetSubMenu() );
                contextMenu->Append( pmi );
                contextMenu->Enable( pimis->id, !pimis->b_grey );

                Connect( pimis->id, wxEVT_COMMAND_MENU_SELECTED,
                         (wxObjectEventFunction) (wxEventFunction) &ChartCanvas::PopupMenuHandler );
            }
        }
    }

    //  This is the default context menu
    menuFocus = contextMenu;

    if( g_pAIS ) {
        contextMenu->Append( ID_DEF_MENU_AISTARGETLIST, _("AIS Target List...") );

        if( seltype & SELTYPE_AISTARGET ) {
            menuAIS->Append( ID_DEF_MENU_AIS_QUERY, _( "Target Query..." ) );
            AIS_Target_Data *myptarget = g_pAIS->Get_Target_Data_From_MMSI( m_FoundAIS_MMSI );
            if( myptarget && myptarget->bCPA_Valid && (myptarget->n_alarm_state != AIS_ALARM_SET) ) {
                if( myptarget->b_show_AIS_CPA )
                    menuAIS->Append( ID_DEF_MENU_AIS_CPA, _( "Hide Target CPA" ) );
                else
                    menuAIS->Append( ID_DEF_MENU_AIS_CPA, _( "Show Target CPA" ) );
            }
            menuAIS->Append( ID_DEF_MENU_AISTARGETLIST, _("Target List...") );
            menuFocus = menuAIS;
        }
    }

    if( seltype & SELTYPE_ROUTESEGMENT ) {
        menuRoute->Append( ID_RT_MENU_PROPERTIES, _( "Properties..." ) );
        if( m_pSelectedRoute ) {

            if( m_pSelectedRoute->IsActive() ) {
                int indexActive = m_pSelectedRoute->GetIndexOf( m_pSelectedRoute->m_pRouteActivePoint );
                if( ( indexActive + 1 ) <= m_pSelectedRoute->GetnPoints() ) {
                    menuRoute->Append( ID_RT_MENU_ACTNXTPOINT, _( "Activate Next Waypoint" ) );
                }

                menuRoute->Append( ID_RT_MENU_DEACTIVATE, _( "Deactivate" ) );
            }
            else {
                menuRoute->Append( ID_RT_MENU_ACTIVATE, _( "Activate" ) );
            }
        }
        menuRoute->Append( ID_RT_MENU_INSERT, _( "Insert Waypoint" ) );
        menuRoute->Append( ID_RT_MENU_APPEND, _( "Append Waypoint" ) );
        menuRoute->Append( ID_RT_MENU_COPY, _( "Copy..." ) );
        menuRoute->Append( ID_RT_MENU_DELETE, _( "Delete..." ) );
        menuRoute->Append( ID_RT_MENU_REVERSE, _( "Reverse..." ) );

        //      Set this menu as the "focused context menu"
        menuFocus = menuRoute;
    }

    if( seltype & SELTYPE_TRACKSEGMENT ) {
        menuTrack->Append( ID_TK_MENU_PROPERTIES, _( "Properties..." ) );
        menuTrack->Append( ID_TK_MENU_COPY, _( "Copy" ) );
        menuTrack->Append( ID_TK_MENU_DELETE, _( "Delete..." ) );

        //      Set this menu as the "focused context menu"
        menuFocus = menuTrack;
    }

    if( seltype & SELTYPE_ROUTEPOINT ) {
        menuWaypoint->Append( ID_WP_MENU_PROPERTIES, _( "Properties..." ) );
        if( m_pSelectedRoute && m_pSelectedRoute->IsActive() ) {
            menuWaypoint->Append( ID_RT_MENU_ACTPOINT, _( "Activate" ) );
        }
        if( m_pSelectedRoute->GetnPoints() > 2 )
            menuWaypoint->Append( ID_RT_MENU_REMPOINT, _( "Remove from Route" ) );

        menuWaypoint->Append( ID_WPT_MENU_COPY, _( "Copy" ) );

        if( m_pFoundRoutePoint->m_IconName != _T("mob") )
            menuWaypoint->Append( ID_RT_MENU_DELPOINT,  _( "Delete" ) );

        if( bGPSValid ) menuWaypoint->Append( ID_WPT_MENU_SENDTOGPS, _( "Send to GPS" ) );
    }

    if( seltype & SELTYPE_MARKPOINT ) {
        menuWaypoint->Append( ID_WP_MENU_PROPERTIES, _( "Properties..." ) );

        if( !g_pRouteMan->GetpActiveRoute() )
            menuWaypoint->Append( ID_WP_MENU_GOTO, _( "Navigate To This" ) );

        menuWaypoint->Append( ID_WPT_MENU_COPY, _( "Copy" ) );

        if( m_pFoundRoutePoint->m_IconName != _T("mob") )
            menuWaypoint->Append( ID_WP_MENU_DELPOINT, _( "Delete" ) );

        if( bGPSValid ) menuWaypoint->Append( ID_WPT_MENU_SENDTOGPS, _( "Send to GPS" ) );

        if( ( m_pFoundRoutePoint == pAnchorWatchPoint1 )
                || ( m_pFoundRoutePoint == pAnchorWatchPoint2 ) )
            menuWaypoint->Append( ID_WP_MENU_CLEAR_ANCHORWATCH, _( "Clear Anchor Watch" ) );
        else

            if( !( m_pFoundRoutePoint->m_bIsInLayer )
                    && ( ( NULL == pAnchorWatchPoint1 ) || ( NULL == pAnchorWatchPoint2 ) ) ) {

                double dist;
                double brg;
                DistanceBearingMercator( m_pFoundRoutePoint->m_lat, m_pFoundRoutePoint->m_lon, gLat,
                                         gLon, &brg, &dist );
                if( dist * 1852. <= g_nAWMax )
                    menuWaypoint->Append( ID_WP_MENU_SET_ANCHORWATCH,  _( "Set Anchor Watch" ) );
            }

        //      Set this menu as the "focused context menu"
        menuFocus = menuWaypoint;
    }

    if( ! subMenuChart->GetMenuItemCount() ) contextMenu->Destroy( subItemChart );

    //  Add the Tide/Current selections if the item was not activated by shortcut in right-click handlers
    bool bsep = false;
    if( seltype & SELTYPE_TIDEPOINT ){
        menuFocus->AppendSeparator();
        bsep = true;
        menuFocus->Append( ID_DEF_MENU_TIDEINFO, _( "Show Tide Information" ) );
    }

    if( seltype & SELTYPE_CURRENTPOINT ) {
        if( !bsep )
            menuFocus->AppendSeparator();
        menuFocus->Append( ID_DEF_MENU_CURRENTINFO, _( "Show Current Information" ) );
    }

    //        Invoke the correct focused drop-down menu
    PopupMenu( menuFocus, x, y );


    // Cleanup
    if( ( m_pSelectedRoute ) ) {
        m_pSelectedRoute->m_bRtIsSelected = false;
    }

    m_pSelectedRoute = NULL;

    if( m_pFoundRoutePoint ) {
        m_pFoundRoutePoint->m_bPtIsSelected = false;
    }
    m_pFoundRoutePoint = NULL;

    m_pFoundRoutePointSecond = NULL;

    delete contextMenu;
    delete menuAIS;
    delete menuRoute;
    delete menuTrack;
    delete menuWaypoint;
}

void ChartCanvas::ShowObjectQueryWindow( int x, int y, float zlat, float zlon )
{
    ChartBase *target_chart = GetChartAtCursor();
    s57chart *Chs57 = dynamic_cast<s57chart*>( target_chart );

    if( Chs57 ) {
        // Go get the array of all objects at the cursor lat/lon
        int sel_rad_pix = 5;
        float SelectRadius = sel_rad_pix / ( GetVP().view_scale_ppm * 1852 * 60 );

        // Make sure we always get the lights from an object, even if we are currently
        // not displaying lights on the chart.

        SetCursor( wxCURSOR_WAIT );
        bool lightsVis = gFrame->ToggleLights( false );
        if( !lightsVis ) gFrame->ToggleLights( true, true );
        ListOfObjRazRules* rule_list =
                Chs57->GetObjRuleListAtLatLon( zlat, zlon, SelectRadius, &GetVP() );

        ListOfObjRazRules* overlay_rule_list = NULL;
        ChartBase *overlay_chart = GetOverlayChartAtCursor();
        s57chart *CHs57_Overlay = dynamic_cast<s57chart*>( overlay_chart );

        if( CHs57_Overlay ) {
            overlay_rule_list =
                CHs57_Overlay->GetObjRuleListAtLatLon( zlat, zlon, SelectRadius, &GetVP() );
        }

        if( !lightsVis ) gFrame->ToggleLights( true, true );

        wxString objText;
        wxFont *dFont = pFontMgr->GetFont( _("ObjectQuery"), 12 );
        wxString face = dFont->GetFaceName();

        if( NULL == g_pObjectQueryDialog ) {
            g_pObjectQueryDialog = new S57QueryDialog();

            g_pObjectQueryDialog->Create( this, -1, _( "Object Query" ), wxDefaultPosition,
                                          wxSize( g_S57_dialog_sx, g_S57_dialog_sy ) );
            g_pObjectQueryDialog->Centre();
        }

        wxColor bg = g_pObjectQueryDialog->GetBackgroundColour();

        objText.Printf( _T("<html><body bgcolor=#%02x%02x%02x><font face="), bg.Red(), bg.Blue(),
                        bg.Green() );
        objText += _T("\"");
        objText += face;
        objText += _T("\">");

        if( overlay_rule_list && CHs57_Overlay) {
            objText << CHs57_Overlay->CreateObjDescriptions( overlay_rule_list );
            objText << _T("<hr noshade>");
        }

        objText << Chs57->CreateObjDescriptions( rule_list );

        objText << _T("</font></body></html>");

        g_pObjectQueryDialog->SetHTMLPage( objText );

        g_pObjectQueryDialog->Show();

        rule_list->Clear();
        delete rule_list;

        if( overlay_rule_list )
            overlay_rule_list->Clear();
        delete overlay_rule_list;

        SetCursor( wxCURSOR_ARROW );
    }
}

void ChartCanvas::RemovePointFromRoute( RoutePoint* point, Route* route ) {
    //  Rebuild the route selectables
    pSelect->DeleteAllSelectableRoutePoints( route );
    pSelect->DeleteAllSelectableRouteSegments( route );

    route->RemovePoint( point );

    //  Check for 1 point routes. If we are creating a route, this is an undo, so keep the 1 point.
    if( (route->GetnPoints() <= 1) && (parent_frame->nRoute_State == 0) ) {
        pConfig->DeleteConfigRoute( route );
        g_pRouteMan->DeleteRoute( route );
        route = NULL;
    }
    //  Add this point back into the selectables
    pSelect->AddSelectableRoutePoint( point->m_lat, point->m_lon, point );

    if( pRoutePropDialog && ( pRoutePropDialog->IsShown() ) ) {
        pRoutePropDialog->SetRouteAndUpdate( route );
        pRoutePropDialog->UpdateProperties();
    }
}

void ChartCanvas::ShowMarkPropertiesDialog( RoutePoint* markPoint ) {
    if( NULL == pMarkPropDialog )    // There is one global instance of the MarkProp Dialog
        pMarkPropDialog = new MarkInfoImpl( this );

    pMarkPropDialog->SetRoutePoint( markPoint );
    pMarkPropDialog->UpdateProperties();
    if( markPoint->m_bIsInLayer ) {
        wxString caption( _("Mark Properties, Layer: ") );
        caption.Append( GetLayerName( markPoint->m_LayerID ) );
        pMarkPropDialog->SetDialogTitle( caption );
    } else
        pMarkPropDialog->SetDialogTitle( _("Mark Properties") );

    pMarkPropDialog->Show();
    pMarkPropDialog->InitialFocus();
}

void ChartCanvas::ShowRoutePropertiesDialog( wxString title, Route* selected ) {
    if( NULL == pRoutePropDialog )  // There is one global instance of the RouteProp Dialog
        pRoutePropDialog = new RouteProp( this );

    pRoutePropDialog->SetRouteAndUpdate( selected );
    pRoutePropDialog->UpdateProperties();
    if( !selected->m_bIsInLayer ) pRoutePropDialog->SetDialogTitle( title );
    else {
        wxString caption( title << _T(", Layer: ") );
        caption.Append( GetLayerName( selected->m_LayerID ) );
        pRoutePropDialog->SetDialogTitle( caption );
    }

    pRoutePropDialog->Show();

    Refresh( false );
}

void pupHandler_PasteWaypoint() {
    Kml* kml = new Kml();
    ::wxBeginBusyCursor();

    int pasteBuffer = kml->ParsePasteBuffer();
    RoutePoint* pasted = kml->GetParsedRoutePoint();

    int nearby_sel_rad_pix = 8;
    double nearby_radius_meters = nearby_sel_rad_pix / cc1->GetCanvasTrueScale();

    RoutePoint *nearPoint = pWayPointMan->GetNearbyWaypoint( pasted->m_lat, pasted->m_lon,
                               nearby_radius_meters );

    int answer = wxID_NO;
    if( nearPoint && !nearPoint->m_bIsInTrack && !nearPoint->m_bIsInLayer ) {
        wxString msg;
        msg << _("There is an existing waypoint at the same location as the one you are pasting. Would you like to merge the pasted data with it?\n\n");
        msg << _("Answering 'No' will create a new waypoint at the same location.");
        answer = OCPNMessageBox( cc1, msg, _("Merge waypoint?"), (long) wxYES_NO | wxCANCEL | wxNO_DEFAULT );
    }

    if( answer == wxID_YES ) {
        nearPoint->SetName( pasted->GetName() );
        nearPoint->m_MarkDescription = pasted->m_MarkDescription;
        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateWptListCtrl();
    }

    if( answer == wxID_NO ) {
        RoutePoint* newPoint = new RoutePoint( pasted );
        newPoint->m_bIsolatedMark = true;
        pSelect->AddSelectableRoutePoint( newPoint->m_lat, newPoint->m_lon, newPoint );
        pConfig->AddNewWayPoint( newPoint, -1 );
        pWayPointMan->m_pWayPointList->Append( newPoint );
        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateWptListCtrl();
    }

    cc1->Refresh( false );
    delete kml;
    ::wxEndBusyCursor();
}

void pupHandler_PasteRoute() {
    Kml* kml = new Kml();
    ::wxBeginBusyCursor();

    int pasteBuffer = kml->ParsePasteBuffer();
    Route* pasted = kml->GetParsedRoute();
    if( ! pasted ) return;

    int nearby_sel_rad_pix = 8;
    double nearby_radius_meters = nearby_sel_rad_pix / cc1->GetCanvasTrueScale();

    RoutePoint* curPoint;
    RoutePoint* nearPoint;
    RoutePoint* prevPoint = NULL;

    bool mergepoints = false;
    bool createNewRoute = true;
    int existingWaypointCounter = 0;

    for( int i = 1; i <= pasted->GetnPoints(); i++ ) {
		curPoint = pasted->GetPoint( i ); // NB! n starts at 1 !
        nearPoint = pWayPointMan->GetNearbyWaypoint( curPoint->m_lat,
                curPoint->m_lon, nearby_radius_meters );
        if( nearPoint ) {
            mergepoints = true;
            existingWaypointCounter++;
            // Small hack here to avoid both extending RoutePoint and repeating all the GetNearbyWaypoint
            // calculations. Use existin data field in RoutePoint as temporary storage.
            curPoint->m_bPtIsSelected = true;
        }
    }

    int answer = wxID_NO;
    if( mergepoints ) {
        wxString msg;
        msg << _("There are existing waypoints at the same location as some of the ones you are pasting. Would you like to just merge the pasted data into them?\n\n");
        msg << _("Answering 'No' will create all new waypoints for this route.");
        answer = OCPNMessageBox( cc1, msg, _("Merge waypoints?"), (long) wxYES_NO | wxCANCEL | wxYES_DEFAULT );

        if( answer == wxID_CANCEL ) {
            delete kml;
            return;
        }
    }

    // If all waypoints exist since before, and a route with the same name, we don't create a new route.
    if( mergepoints && answer==wxID_YES && existingWaypointCounter==pasted->GetnPoints() ) {

        wxRouteListNode *route_node = pRouteList->GetFirst();
        while( route_node ) {
            Route *proute = route_node->GetData();

            if( pasted->m_RouteNameString == proute->m_RouteNameString ) {
                createNewRoute = false;
                break;
            }
            route_node = route_node->GetNext();
        }
    }

    Route* newRoute;
    RoutePoint* newPoint;

    if( createNewRoute ) {
        newRoute = new Route();
        newRoute->m_RouteNameString = pasted->m_RouteNameString;
    }

    for( int i = 1; i <= pasted->GetnPoints(); i++ ) {
        curPoint = pasted->GetPoint( i );
        if( answer == wxID_YES && curPoint->m_bPtIsSelected ) {
            curPoint->m_bPtIsSelected = false;
            newPoint = pWayPointMan->GetNearbyWaypoint( curPoint->m_lat, curPoint->m_lon,
                    nearby_radius_meters );
            newPoint->SetName( curPoint->GetName() );
            newPoint->m_MarkDescription = curPoint->m_MarkDescription;

            if( createNewRoute ) newRoute->AddPoint( newPoint );
        } else {
            curPoint->m_bPtIsSelected = false;

            newPoint = new RoutePoint( curPoint );
            newPoint->m_bIsolatedMark = false;
            newPoint->m_IconName = _T("circle");
            newPoint->m_bIsVisible = true;
            newPoint->m_bShowName = false;
            newPoint->m_bKeepXRoute = false;

            newRoute->AddPoint( newPoint );
            pSelect->AddSelectableRoutePoint( newPoint->m_lat, newPoint->m_lon, newPoint );
            pConfig->AddNewWayPoint( newPoint, -1 );
            pWayPointMan->m_pWayPointList->Append( newPoint );
        }
        if( i > 1 && createNewRoute ) pSelect->AddSelectableRouteSegment( prevPoint->m_lat,
                prevPoint->m_lon, curPoint->m_lat, curPoint->m_lon, prevPoint, newPoint, newRoute );
        prevPoint = newPoint;
    }

    if( createNewRoute ) {
        pRouteList->Append( newRoute );
        pConfig->AddNewRoute( newRoute, -1 );    // use auto next num
        newRoute->RebuildGUIDList(); // ensure the GUID list is intact and good

        if( pRoutePropDialog && ( pRoutePropDialog->IsShown() ) ) {
            pRoutePropDialog->SetRouteAndUpdate( newRoute );
            pRoutePropDialog->UpdateProperties();
        }

        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) {
            pRouteManagerDialog->UpdateRouteListCtrl();
            pRouteManagerDialog->UpdateWptListCtrl();
        }
        cc1->Refresh( false );
    }

    delete kml;
    ::wxEndBusyCursor();
}

void pupHandler_PasteTrack() {
    Kml* kml = new Kml();
    ::wxBeginBusyCursor();

    int pasteBuffer = kml->ParsePasteBuffer();
    Track* pasted = kml->GetParsedTrack();
    if( ! pasted ) return;

    RoutePoint* curPoint;

    Track* newTrack = new Track();
    RoutePoint* newPoint;
    RoutePoint* prevPoint = NULL;

    newTrack->m_RouteNameString = pasted->m_RouteNameString;

    for( int i = 1; i <= pasted->GetnPoints(); i++ ) {
        curPoint = pasted->GetPoint( i );

        newPoint = new RoutePoint( curPoint );
        newPoint->m_bShowName = false;
        newPoint->m_bIsVisible = false;
        newPoint->m_GPXTrkSegNo = 1;

        wxDateTime now = wxDateTime::Now();
        newPoint->m_CreateTime = curPoint->m_CreateTime;

        newTrack->AddPoint( newPoint );

        //    This is a hack, need to undo the action of Route::AddPoint
        newPoint->m_bIsInRoute = false;
        newPoint->m_bIsInTrack = true;

        if( prevPoint )
            pSelect->AddSelectableTrackSegment(
                prevPoint->m_lat, prevPoint->m_lon,
                newPoint->m_lat, newPoint->m_lon,
                prevPoint, newPoint, newTrack );

        prevPoint = newPoint;
    }

    pRouteList->Append( newTrack );
    pConfig->AddNewRoute( newTrack, -1 );    // use auto next num
    newTrack->RebuildGUIDList(); // ensure the GUID list is intact and good

    cc1->Refresh( false );
    delete kml;
    ::wxEndBusyCursor();
}

void ChartCanvas::PopupMenuHandler( wxCommandEvent& event )
{
    RoutePoint *pLast;

    wxPoint r;
    double zlat, zlon;

    GetCanvasPixPoint( popx, popy, zlat, zlon );

    switch( event.GetId() ) {
    case ID_DEF_MENU_MAX_DETAIL:
        vLat = zlat;
        vLon = zlon;
        ClearbFollow();

        parent_frame->DoChartUpdate();

        parent_frame->SelectChartFromStack( 0, false, CHART_TYPE_DONTCARE,
                                            CHART_FAMILY_RASTER );
        break;

    case ID_DEF_MENU_SCALE_IN:
        parent_frame->DoStackDown();
        break;

    case ID_DEF_MENU_SCALE_OUT:
        parent_frame->DoStackUp();
        break;

    case ID_UNDO:
        undo->UndoLastAction();
        Refresh( false );
        break;

    case ID_REDO:
        undo->RedoNextAction();
        Refresh( false );
        break;

    case ID_DEF_MENU_MOVE_BOAT_HERE:
        gLat = zlat;
        gLon = zlon;
        break;

    case ID_DEF_MENU_GOTO_HERE: {
        RoutePoint *pWP_dest = new RoutePoint( zlat, zlon, g_default_wp_icon, wxEmptyString,
                                               GPX_EMPTY_STRING );
        pSelect->AddSelectableRoutePoint( zlat, zlon, pWP_dest );

        RoutePoint *pWP_src = new RoutePoint( gLat, gLon, g_default_wp_icon, wxEmptyString,
                                              GPX_EMPTY_STRING );
        pSelect->AddSelectableRoutePoint( gLat, gLon, pWP_src );

        Route *temp_route = new Route();
        pRouteList->Append( temp_route );

        temp_route->AddPoint( pWP_src );
        temp_route->AddPoint( pWP_dest );

        pSelect->AddSelectableRouteSegment( gLat, gLon, zlat, zlon, pWP_src, pWP_dest,
                                            temp_route );

        temp_route->m_RouteNameString = _("Temporary GOTO Route");
        temp_route->m_RouteStartString = _("Here");
        ;
        temp_route->m_RouteEndString = _("There");

        temp_route->m_bDeleteOnArrival = true;

        if( g_pRouteMan->GetpActiveRoute() ) g_pRouteMan->DeactivateRoute();

        g_pRouteMan->ActivateRoute( temp_route, pWP_dest );

        break;
    }

    case ID_DEF_MENU_DROP_WP: {
        RoutePoint *pWP = new RoutePoint( zlat, zlon, g_default_wp_icon, wxEmptyString,
                                          GPX_EMPTY_STRING );
        pWP->m_bIsolatedMark = true;                      // This is an isolated mark
        pSelect->AddSelectableRoutePoint( zlat, zlon, pWP );
        pConfig->AddNewWayPoint( pWP, -1 );    // use auto next num

        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateWptListCtrl();
        undo->BeforeUndoableAction( Undo_CreateWaypoint, pWP, Undo_HasParent, NULL );
        undo->AfterUndoableAction( NULL );
        Refresh( false );      // Needed for MSW, why not GTK??
        break;
    }

    case ID_DEF_MENU_AISTARGETLIST:
        ShowAISTargetList();
        break;

    case ID_WP_MENU_GOTO: {
        RoutePoint *pWP_src = new RoutePoint( gLat, gLon, g_default_wp_icon, wxEmptyString,
                                              GPX_EMPTY_STRING );
        pSelect->AddSelectableRoutePoint( gLat, gLon, pWP_src );

        Route *temp_route = new Route();
        pRouteList->Append( temp_route );

        temp_route->AddPoint( pWP_src );
        temp_route->AddPoint( m_pFoundRoutePoint );
        m_pFoundRoutePoint->m_bKeepXRoute = true;

        pSelect->AddSelectableRouteSegment( gLat, gLon, m_pFoundRoutePoint->m_lat,
                                            m_pFoundRoutePoint->m_lon, pWP_src, m_pFoundRoutePoint, temp_route );

        wxString name = m_pFoundRoutePoint->GetName();
        if( name.IsEmpty() ) name = _("(Unnamed Waypoint)");
        wxString rteName = _("Go to ");
        rteName.Append( name );
        temp_route->m_RouteNameString = rteName;
        temp_route->m_RouteStartString = _("Here");
        ;
        temp_route->m_RouteEndString = name;
        temp_route->m_bDeleteOnArrival = true;

        if( g_pRouteMan->GetpActiveRoute() ) g_pRouteMan->DeactivateRoute();

        g_pRouteMan->ActivateRoute( temp_route, m_pFoundRoutePoint );

        break;
    }

    case ID_DEF_MENU_COGUP:
        gFrame->ToggleCourseUp();
        break;

    case ID_DEF_MENU_NORTHUP:
        gFrame->ToggleCourseUp();
        break;

    case ID_DEF_MENU_GOTOPOSITION:
        if( NULL == pGoToPositionDialog ) // There is one global instance of the Go To Position Dialog
            pGoToPositionDialog = new GoToPositionDialog( this );
        pGoToPositionDialog->CheckPasteBufferForPosition();
        pGoToPositionDialog->Show();
        break;

    case ID_WP_MENU_DELPOINT: {
        if( m_pFoundRoutePoint == pAnchorWatchPoint1 ) {
            pAnchorWatchPoint1 = NULL;
            g_AW1GUID.Clear();
        } else if( m_pFoundRoutePoint == pAnchorWatchPoint2 ) {
            pAnchorWatchPoint2 = NULL;
            g_AW2GUID.Clear();
        }

        if( m_pFoundRoutePoint && !( m_pFoundRoutePoint->m_bIsInLayer )
                && ( m_pFoundRoutePoint->m_IconName != _T("mob") ) ) {

            undo->BeforeUndoableAction( Undo_DeleteWaypoint, m_pFoundRoutePoint, Undo_IsOrphanded, m_pFoundPoint );
            pConfig->DeleteWayPoint( m_pFoundRoutePoint );
            pSelect->DeleteSelectablePoint( m_pFoundRoutePoint, SELTYPE_ROUTEPOINT );
            if( NULL != pWayPointMan ) pWayPointMan->m_pWayPointList->DeleteObject( m_pFoundRoutePoint );
            m_pFoundRoutePoint = NULL;
            undo->AfterUndoableAction( NULL );

            if( pMarkPropDialog ) {
                pMarkPropDialog->SetRoutePoint( NULL );
                pMarkPropDialog->UpdateProperties();
            }

            if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateWptListCtrl();
        }
        break;
    }
    case ID_WP_MENU_PROPERTIES:
        ShowMarkPropertiesDialog( m_pFoundRoutePoint );
        break;

    case ID_WP_MENU_CLEAR_ANCHORWATCH:
        if( pAnchorWatchPoint1 == m_pFoundRoutePoint ) {
            pAnchorWatchPoint1 = NULL;
            g_AW1GUID.Clear();
        } else if( pAnchorWatchPoint2 == m_pFoundRoutePoint ) {
            pAnchorWatchPoint2 = NULL;
            g_AW2GUID.Clear();
        }
        break;

    case ID_WP_MENU_SET_ANCHORWATCH:
        if( pAnchorWatchPoint1 == NULL ) {
            pAnchorWatchPoint1 = m_pFoundRoutePoint;
            g_AW1GUID = pAnchorWatchPoint1->m_GUID;
            wxString nn;
            nn = m_pFoundRoutePoint->GetName();
            if( nn.IsNull() ) {
                nn.Printf( _T("%d m"), g_nAWDefault );
                m_pFoundRoutePoint->SetName( nn );
            }
        } else if( pAnchorWatchPoint2 == NULL ) {
            pAnchorWatchPoint2 = m_pFoundRoutePoint;
            g_AW2GUID = pAnchorWatchPoint2->m_GUID;
            wxString nn;
            nn = m_pFoundRoutePoint->GetName();
            if( nn.IsNull() ) {
                nn.Printf( _T("%d m"), g_nAWDefault );
                m_pFoundRoutePoint->SetName( nn );
            }
        }
        break;

    case ID_WP_MENU_ADDITIONAL_INFO:
        if( NULL == pMarkInfoDialog )    // There is one global instance of the MarkInfo Dialog
            pMarkInfoDialog = new MarkInfoImpl( this );

        pMarkInfoDialog->SetRoutePoint( m_pFoundRoutePoint );
        pMarkInfoDialog->UpdateProperties();

        pMarkInfoDialog->Show();
        break;

    case ID_DEF_MENU_ACTIVATE_MEASURE:
//                        WarpPointer(popx,popy);
        m_bMeasure_Active = true;
        m_nMeasureState = 1;
        break;

    case ID_DEF_MENU_DEACTIVATE_MEASURE:
        m_bMeasure_Active = false;
        m_nMeasureState = 0;
        g_pRouteMan->DeleteRoute( m_pMeasureRoute );
        m_pMeasureRoute = NULL;
        gFrame->SurfaceToolbar();
        Refresh( false );
        break;

#ifdef USE_S57
    case ID_DEF_MENU_CM93OFFSET_DIALOG:
        if( NULL == g_pCM93OffsetDialog ) {
            if( !VPoint.b_quilt && Current_Ch
                    && ( Current_Ch->GetChartType() == CHART_TYPE_CM93COMP ) ) {
                cm93compchart *pch = (cm93compchart *) Current_Ch;
                g_pCM93OffsetDialog = new CM93OffsetDialog( parent_frame, pch );
            }
        }
        g_pCM93OffsetDialog->Show();
        g_pCM93OffsetDialog->UpdateMCOVRList( GetVP() );

        break;

    case ID_DEF_MENU_QUERY: {
        ShowObjectQueryWindow( popx, popy, zlat, zlon );
        break;
    }
#endif
    case ID_DEF_MENU_AIS_QUERY: {
        wxWindow *pwin = wxDynamicCast(this, wxWindow);
        ShowAISTargetQueryDialog( pwin, m_FoundAIS_MMSI );
        break;
    }

    case ID_DEF_MENU_AIS_CPA: {             //TR 2012.06.28: Show AIS-CPA
        AIS_Target_Data *myptarget = g_pAIS->Get_Target_Data_From_MMSI(m_FoundAIS_MMSI); //TR 2012.06.28: Show AIS-CPA
        if ( myptarget )                    //TR 2012.06.28: Show AIS-CPA
            myptarget->Toggle_AIS_CPA();     //TR 2012.06.28: Show AIS-CPA
        break;                              //TR 2012.06.28: Show AIS-CPA
    }

    case ID_DEF_MENU_QUILTREMOVE: {
        if( VPoint.b_quilt ) {
            int dbIndex = m_pQuilt->GetChartdbIndexAtPix( wxPoint( popx, popy ) );
            parent_frame->RemoveChartFromQuilt( dbIndex );

            ReloadVP();

        }

        break;
    }

    case ID_DEF_MENU_CURRENTINFO: {
        DrawTCWindow( popx, popy, (void *) m_pIDXCandidate );
        Refresh( false );

        break;
    }

    case ID_DEF_MENU_TIDEINFO: {
        DrawTCWindow( popx, popy, (void *) m_pIDXCandidate );
        Refresh( false );

        break;
    }
    case ID_RT_MENU_REVERSE: {
        if( m_pSelectedRoute->m_bIsInLayer ) break;

        pSelect->DeleteAllSelectableRouteSegments( m_pSelectedRoute );

        int ask_return = OCPNMessageBox( this, g_pRouteMan->GetRouteReverseMessage(),
                               _("Rename Waypoints?"), wxYES_NO );

        m_pSelectedRoute->Reverse( ask_return == wxID_YES );

        pSelect->AddAllSelectableRouteSegments( m_pSelectedRoute );

        pConfig->UpdateRoute( m_pSelectedRoute );

        if( pRoutePropDialog && ( pRoutePropDialog->IsShown() ) ) {
            pRoutePropDialog->SetRouteAndUpdate( m_pSelectedRoute );
            pRoutePropDialog->UpdateProperties();
        }
        break;
    }

    case ID_RT_MENU_DELETE: {
        int dlg_return = wxID_YES;
        if( g_bConfirmObjectDelete ) {
            dlg_return = OCPNMessageBox( this,  _("Are you sure you want to delete this route?"),
                _("OpenCPN Route Delete"), (long) wxYES_NO | wxCANCEL | wxYES_DEFAULT );
        }

        if( dlg_return == wxID_YES ) {
            if( g_pRouteMan->GetpActiveRoute() == m_pSelectedRoute ) g_pRouteMan->DeactivateRoute();

            if( m_pSelectedRoute->m_bIsInLayer ) break;

            pConfig->DeleteConfigRoute( m_pSelectedRoute );
            g_pRouteMan->DeleteRoute( m_pSelectedRoute );
            m_pSelectedRoute = NULL;
            m_pFoundRoutePoint = NULL;
            m_pFoundRoutePointSecond = NULL;
            if( pRoutePropDialog && ( pRoutePropDialog->IsShown() ) ) {
                pRoutePropDialog->SetRouteAndUpdate( m_pSelectedRoute );
                pRoutePropDialog->UpdateProperties();
            }

            if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateRouteListCtrl();

            if( pMarkPropDialog && pMarkPropDialog->IsShown() ) {
                pMarkPropDialog->ValidateMark();
                pMarkPropDialog->UpdateProperties();
            }

            undo->InvalidateUndo();
        }
        break;
    }

    case ID_RT_MENU_ACTIVATE: {
        if( g_pRouteMan->GetpActiveRoute() ) g_pRouteMan->DeactivateRoute();

        RoutePoint *best_point = g_pRouteMan->FindBestActivatePoint( m_pSelectedRoute, gLat,
                                 gLon, gCog, gSog );

        g_pRouteMan->ActivateRoute( m_pSelectedRoute, best_point );
        m_pSelectedRoute->m_bRtIsSelected = false;

        break;
    }

    case ID_RT_MENU_DEACTIVATE:
        g_pRouteMan->DeactivateRoute();
        m_pSelectedRoute->m_bRtIsSelected = false;

        break;

    case ID_RT_MENU_INSERT:

        if( m_pSelectedRoute->m_bIsInLayer ) break;

        m_pSelectedRoute->InsertPointBefore( m_pFoundRoutePointSecond, zlat, zlon );

        pSelect->DeleteAllSelectableRoutePoints( m_pSelectedRoute );
        pSelect->DeleteAllSelectableRouteSegments( m_pSelectedRoute );

        pSelect->AddAllSelectableRouteSegments( m_pSelectedRoute );
        pSelect->AddAllSelectableRoutePoints( m_pSelectedRoute );

        //    As a special case (which comes up often)...
        //    If the inserted waypoint is on the active leg of an active route
        /*            if(m_pSelectedRoute->m_bRtIsActive)
         {
         if(m_pSelectedRoute->m_nRouteActivePoint == np + 1)
         {
         pNew_Point = m_pSelectedRoute->GetPoint(np + 2);
         pRouteMan->ActivateRoutePoint(m_pSelectedRoute, pNew_Point);
         }
         }
         */
        m_pSelectedRoute->RebuildGUIDList();          // ensure the GUID list is intact and good
        pConfig->UpdateRoute( m_pSelectedRoute );

        if( pRoutePropDialog && ( pRoutePropDialog->IsShown() ) ) {
            pRoutePropDialog->SetRouteAndUpdate( m_pSelectedRoute );
            pRoutePropDialog->UpdateProperties();
        }

        break;

    case ID_RT_MENU_APPEND:

        if( m_pSelectedRoute->m_bIsInLayer ) break;

        m_pMouseRoute = m_pSelectedRoute;
        parent_frame->nRoute_State = m_pSelectedRoute->GetnPoints() + 1;
        m_pMouseRoute->m_lastMousePointIndex = m_pSelectedRoute->GetnPoints();

        pLast = m_pSelectedRoute->GetLastPoint();

        m_prev_rlat = pLast->m_lat;
        m_prev_rlon = pLast->m_lon;
        m_prev_pMousePoint = pLast;

        m_bAppendingRoute = true;

        SetCursor( *pCursorPencil );

        break;

    case ID_RT_MENU_COPY:
        if( m_pSelectedRoute ) Kml::CopyRouteToClipboard( m_pSelectedRoute );
        break;

    case ID_TK_MENU_COPY:
        if( m_pSelectedTrack ) Kml::CopyTrackToClipboard( (Track*)m_pSelectedTrack );
        break;

    case ID_WPT_MENU_COPY:
        if( m_pFoundRoutePoint ) Kml::CopyWaypointToClipboard( m_pFoundRoutePoint );
        break;

    case ID_WPT_MENU_SENDTOGPS:
        if( m_pFoundRoutePoint ) {
            wxString port;
            if( g_pConnectionParams ) {
                // With the new multiplexer code we take a bit of a chance here,
                // and use the first available serial connection which has output.
                // This could potentially fail in complex installations...
                for( size_t i = 0; i < g_pConnectionParams->Count(); i++ ) {
                    ConnectionParams *cp = g_pConnectionParams->Item( i );
                    if( cp->Output && cp->Type == Serial ) {
                        port << _T("Serial:") << cp->Port;
                    }
                }
                if( port.Length() )
                    m_pFoundRoutePoint->SendToGPS( port, NULL );
                else
                    OCPNMessageBox( NULL, _("Can't send waypoint. Found no serial data port with output defined."), _("OpenCPN Info"), wxOK | wxICON_WARNING );
            }
        }
        break;

    case ID_PASTE_WAYPOINT:
        pupHandler_PasteWaypoint();
        break;

    case ID_PASTE_ROUTE:
        pupHandler_PasteRoute();
        break;

    case ID_PASTE_TRACK:
        pupHandler_PasteTrack();
        break;

    case ID_RT_MENU_DELPOINT:
        if( m_pSelectedRoute ) {
            if( m_pSelectedRoute->m_bIsInLayer ) break;

            pWayPointMan->DestroyWaypoint( m_pFoundRoutePoint );
            m_pFoundRoutePoint = NULL;

            //    Selected route may have been deleted as one-point route, so check it
            if( !g_pRouteMan->IsRouteValid( m_pSelectedRoute ) ) m_pSelectedRoute = NULL;

            if( pRoutePropDialog && ( pRoutePropDialog->IsShown() ) ) {
                pRoutePropDialog->SetRouteAndUpdate( m_pSelectedRoute );
                pRoutePropDialog->UpdateProperties();
            }

            if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateWptListCtrl();
        }

        break;

    case ID_RT_MENU_REMPOINT:
        if( m_pSelectedRoute ) {
            if( m_pSelectedRoute->m_bIsInLayer ) break;
            RemovePointFromRoute( m_pFoundRoutePoint, m_pSelectedRoute );
        }
        break;

    case ID_RT_MENU_ACTPOINT:
        if( g_pRouteMan->GetpActiveRoute() == m_pSelectedRoute ) {
            g_pRouteMan->ActivateRoutePoint( m_pSelectedRoute, m_pFoundRoutePoint );
            m_pSelectedRoute->m_bRtIsSelected = false;
        }

        break;

    case ID_RT_MENU_DEACTPOINT:
        break;

    case ID_RT_MENU_ACTNXTPOINT:
        if( g_pRouteMan->GetpActiveRoute() == m_pSelectedRoute ) {
            g_pRouteMan->ActivateNextPoint( m_pSelectedRoute, true );
            m_pSelectedRoute->m_bRtIsSelected = false;
        }

        break;

    case ID_RT_MENU_PROPERTIES: {
        ShowRoutePropertiesDialog( _("Route Properties"), m_pSelectedRoute );
        break;
    }

    case ID_TK_MENU_PROPERTIES: {
        ShowRoutePropertiesDialog( _("Track Properties"), m_pSelectedTrack );
        break;
    }

    case ID_TK_MENU_DELETE: {
        int dlg_return = wxID_YES;
        if( g_bConfirmObjectDelete ) {
            dlg_return = OCPNMessageBox( this, _("Are you sure you want to delete this track?"),
                _("OpenCPN Track Delete"), (long) wxYES_NO | wxCANCEL | wxYES_DEFAULT );
        }

        if( dlg_return == wxID_YES ) {

            if( (Track *) ( m_pSelectedTrack ) == g_pActiveTrack ) parent_frame->TrackOff();

            pConfig->DeleteConfigRoute( m_pSelectedTrack );

            g_pRouteMan->DeleteTrack( m_pSelectedTrack );
            m_pSelectedTrack = NULL;
            m_pFoundRoutePoint = NULL;
            m_pFoundRoutePointSecond = NULL;

            if( pRoutePropDialog && ( pRoutePropDialog->IsShown() ) ) {
                pRoutePropDialog->SetRouteAndUpdate( m_pSelectedTrack );
                pRoutePropDialog->UpdateProperties();
            }

            if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) {
                pRouteManagerDialog->UpdateTrkListCtrl();
                pRouteManagerDialog->UpdateRouteListCtrl();
            }
        }
        break;
    }

    case ID_RC_MENU_SCALE_IN:
        parent_frame->DoStackDown();
        GetCanvasPointPix( zlat, zlon, &r );
        WarpPointer( r.x, r.y );
        break;

    case ID_RC_MENU_SCALE_OUT:
        parent_frame->DoStackUp();
        GetCanvasPointPix( zlat, zlon, &r );
        WarpPointer( r.x, r.y );
        break;

    case ID_RC_MENU_ZOOM_IN:
        SetVPScale( GetVPScale() * 2 );
        GetCanvasPointPix( zlat, zlon, &r );
        WarpPointer( r.x, r.y );
        break;

    case ID_RC_MENU_ZOOM_OUT:
        SetVPScale( GetVPScale() / 2 );
        GetCanvasPointPix( zlat, zlon, &r );
        WarpPointer( r.x, r.y );
        break;

    case ID_RC_MENU_FINISH:
        FinishRoute();
        gFrame->SurfaceToolbar();
        Refresh( false );
        break;

    default: {
        //  Look for PlugIn Context Menu selections
        //  If found, make the callback
        ArrayOfPlugInMenuItems item_array = g_pi_manager->GetPluginContextMenuItemArray();

        for( unsigned int i = 0; i < item_array.GetCount(); i++ ) {
            PlugInMenuItemContainer *pimis = item_array.Item( i );
            {
                if( pimis->id == event.GetId() ) {
                    if( pimis->m_pplugin ) pimis->m_pplugin->OnContextMenuItemCallback(
                            pimis->id );
                }
            }
        }

        break;
    }
    }           // switch

    //  Chart Groups....
    if( ( event.GetId() >= ID_DEF_MENU_GROUPBASE )
            && ( event.GetId() <= ID_DEF_MENU_GROUPBASE + (int) g_pGroupArray->GetCount() ) ) {
        gFrame->SetGroupIndex( event.GetId() - ID_DEF_MENU_GROUPBASE );
    }

    g_click_stop = 0;    // Context menu was processed, all is well

}

void ChartCanvas::FinishRoute( void )
{
    parent_frame->nRoute_State = 0;
    m_prev_pMousePoint = NULL;

    parent_frame->SetToolbarItemState( ID_ROUTE, false );
    SetCursor( *pCursorArrow );
    m_bDrawingRoute = false;

    if( m_pMouseRoute ) {
        if( m_bAppendingRoute ) pConfig->UpdateRoute( m_pMouseRoute );
        else {
            if( m_pMouseRoute->GetnPoints() > 1 ) {
                pConfig->AddNewRoute( m_pMouseRoute, -1 );    // use auto next num
            } else {
                g_pRouteMan->DeleteRoute( m_pMouseRoute );
                m_pMouseRoute = NULL;
            }

            if( m_pMouseRoute ) m_pMouseRoute->RebuildGUIDList(); // ensure the GUID list is intact and good
        }
        if( m_pMouseRoute ) m_pMouseRoute->RebuildGUIDList(); // ensure the GUID list is intact and good

        if( pRoutePropDialog && ( pRoutePropDialog->IsShown() ) ) {
            pRoutePropDialog->SetRouteAndUpdate( m_pMouseRoute );
            pRoutePropDialog->UpdateProperties();
        }

        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateRouteListCtrl();

    }
    m_bAppendingRoute = false;
    m_pMouseRoute = NULL;

    m_pSelectedRoute = NULL;
    m_pFoundRoutePointSecond = NULL;

    undo->InvalidateUndo();
}

void ChartCanvas::ShowAISTargetList( void )
{
    if( NULL == g_pAISTargetList ) {         // There is one global instance of the Dialog
        g_pAISTargetList = new AISTargetListDialog( parent_frame, g_pauimgr, g_pAIS );
    }

    g_pAISTargetList->UpdateAISTargetList();

}

void ChartCanvas::RenderAllChartOutlines( ocpnDC &dc, ViewPort& vp )
{
    if( !g_bShowOutlines ) return;

    int nEntry = ChartData->GetChartTableEntries();

    for( int i = 0; i < nEntry; i++ ) {
        ChartTableEntry *pt = (ChartTableEntry *) &ChartData->GetChartTableEntry( i );

        //    Check to see if the candidate chart is in the currently active group
        bool b_group_draw = false;
        if( g_GroupIndex > 0 ) {
            for( unsigned int ig = 0; ig < pt->GetGroupArray().GetCount(); ig++ ) {
                int index = pt->GetGroupArray().Item( ig );
                if( g_GroupIndex == index ) {
                    b_group_draw = true;
                    break;
                }
            }
        } else
            b_group_draw = true;

        if( b_group_draw ) RenderChartOutline( dc, i, vp );
    }

//      Could render in different color/width if thumbnail is selected
//    if(NULL !=  pthumbwin->pThumbChart)
//        int ggl = 4;

#ifdef USE_S57
    //        On CM93 Composite Charts, draw the outlines of the next smaller scale cell
    if( Current_Ch && ( Current_Ch->GetChartType() == CHART_TYPE_CM93COMP ) ) {
        cm93compchart *pch = (cm93compchart *) Current_Ch;
        if( pch ) {
            wxPen mPen( GetGlobalColor( _T("UINFM") ), 1, wxSOLID );
            dc.SetPen( mPen );
            pch->RenderNextSmallerCellOutlines( dc, vp );
        }
    }
#endif
}

void ChartCanvas::RenderChartOutline( ocpnDC &dc, int dbIndex, ViewPort& vp )
{
    float plylat, plylon;
    float plylat1, plylon1;

    int pixx, pixy, pixx1, pixy1;
    bool b_draw = false;
    double lon_bias = 0.;

    wxBoundingBox box;
    ChartData->GetDBBoundingBox( dbIndex, &box );

    if( vp.GetBBox().Intersect( box, 0 ) != _OUT )              // chart is not outside of viewport
        b_draw = true;

    //  Does simple test fail, and current vp cross international dateline?
    if( !b_draw && ( ( vp.GetBBox().GetMinX() < -180. ) || ( vp.GetBBox().GetMaxX() > 180. ) ) ) {
        //  If so, do an explicit test with alternate phasing
        if( vp.GetBBox().GetMinX() < -180. ) {
            wxPoint2DDouble p( -360., 0 );
            box.Translate( p );
            if( vp.GetBBox().Intersect( box, 0 ) != _OUT )       // chart is not outside of viewport
            {
                b_draw = true;
                lon_bias = -360.;
            }
        } else {
            wxPoint2DDouble p( 360., 0 );
            box.Translate( p );
            if( vp.GetBBox().Intersect( box, 0 ) != _OUT )       // chart is not outside of viewport
            {
                b_draw = true;
                lon_bias = 360.;
            }
        }

    }

    //  Does simple test fail, and chart box cross international dateline?
    if( !b_draw && ( box.GetMinX() < 180. ) && ( box.GetMaxX() > 180. ) ) {
        wxPoint2DDouble p( -360., 0 );
        box.Translate( p );
        if( vp.GetBBox().Intersect( box, 0 ) != _OUT )           // chart is not outside of viewport
        {
            b_draw = true;
            lon_bias = -360.;
        }
    }

    if( !b_draw ) return;

    int nPly = ChartData->GetDBPlyPoint( dbIndex, 0, &plylat, &plylon );

    if( ChartData->GetDBChartType( dbIndex ) == CHART_TYPE_S57 ) dc.SetPen(
            wxPen( GetGlobalColor( _T ( "UINFG" ) ), 1, wxSOLID ) );

    else if( ChartData->GetDBChartType( dbIndex ) == CHART_TYPE_CM93 ) dc.SetPen(
            wxPen( GetGlobalColor( _T ( "YELO1" ) ), 1, wxSOLID ) );

    else
        dc.SetPen( wxPen( GetGlobalColor( _T ( "UINFR" ) ), 1, wxSOLID ) );

    //        Are there any aux ply entries?
    int nAuxPlyEntries = ChartData->GetnAuxPlyEntries( dbIndex );
    if( 0 == nAuxPlyEntries )                 // There are no aux Ply Point entries
    {
        wxPoint r, r1;

        ChartData->GetDBPlyPoint( dbIndex, 0, &plylat, &plylon );
        plylon += lon_bias;

        GetCanvasPointPix( plylat, plylon, &r );
        pixx = r.x;
        pixy = r.y;

        for( int i = 0; i < nPly - 1; i++ ) {
            ChartData->GetDBPlyPoint( dbIndex, i + 1, &plylat1, &plylon1 );
            plylon1 += lon_bias;

            GetCanvasPointPix( plylat1, plylon1, &r1 );
            pixx1 = r1.x;
            pixy1 = r1.y;

            int pixxs1 = pixx1;
            int pixys1 = pixy1;

            bool b_skip = false;

            if( vp.chart_scale > 5e7 ) {
                //    calculate projected distance between these two points in meters
                double dist = sqrt(
                                  (double) ( ( pixx1 - pixx ) * ( pixx1 - pixx ) )
                                  + ( ( pixy1 - pixy ) * ( pixy1 - pixy ) ) ) / vp.view_scale_ppm;
                //    calculate GC distance between these two points in meters
                double distgc = DistGreatCircle( plylat, plylon, plylat1, plylon1 ) * 1852.;

                //    If the distances are nonsense, it means that the scale is very small and the segment wrapped the world
                //    So skip it....
                //    TODO improve this to draw two segments
                if( fabs( dist - distgc ) > 10000. * 1852. )          //lotsa miles
                    b_skip = true;
            }

            ClipResult res = cohen_sutherland_line_clip_i( &pixx, &pixy, &pixx1, &pixy1, 0,
                             vp.pix_width, 0, vp.pix_height );
            if( res != Invisible && !b_skip ) dc.DrawLine( pixx, pixy, pixx1, pixy1, false );

            plylat = plylat1;
            plylon = plylon1;
            pixx = pixxs1;
            pixy = pixys1;
        }

        ChartData->GetDBPlyPoint( dbIndex, 0, &plylat1, &plylon1 );
        plylon1 += lon_bias;

        GetCanvasPointPix( plylat1, plylon1, &r1 );
        pixx1 = r1.x;
        pixy1 = r1.y;

        ClipResult res = cohen_sutherland_line_clip_i( &pixx, &pixy, &pixx1, &pixy1, 0,
                         vp.pix_width, 0, vp.pix_height );
        if( res != Invisible ) dc.DrawLine( pixx, pixy, pixx1, pixy1, false );
    }

    else                              // Use Aux PlyPoints
    {
        wxPoint r, r1;

        int nAuxPlyEntries = ChartData->GetnAuxPlyEntries( dbIndex );
        for( int j = 0; j < nAuxPlyEntries; j++ ) {

            int nAuxPly = ChartData->GetDBAuxPlyPoint( dbIndex, 0, j, &plylat, &plylon );
            GetCanvasPointPix( plylat, plylon, &r );
            pixx = r.x;
            pixy = r.y;

            for( int i = 0; i < nAuxPly - 1; i++ ) {
                ChartData->GetDBAuxPlyPoint( dbIndex, i + 1, j, &plylat1, &plylon1 );

                GetCanvasPointPix( plylat1, plylon1, &r1 );
                pixx1 = r1.x;
                pixy1 = r1.y;

                int pixxs1 = pixx1;
                int pixys1 = pixy1;

                bool b_skip = false;

                if( vp.chart_scale > 5e7 ) {
                    //    calculate projected distance between these two points in meters
                    double dist = sqrt(
                                      (double) ( ( pixx1 - pixx ) * ( pixx1 - pixx ) )
                                      + ( ( pixy1 - pixy ) * ( pixy1 - pixy ) ) ) / vp.view_scale_ppm;
                    //    calculate GC distance between these two points in meters
                    double distgc = DistGreatCircle( plylat, plylon, plylat1, plylon1 ) * 1852.;

                    //    If the distances are nonsense, it means that the scale is very small and the segment wrapped the world
                    //    So skip it....
                    //    TODO improve this to draw two segments
                    if( fabs( dist - distgc ) > 10000. * 1852. )          //lotsa miles
                        b_skip = true;
                }

                ClipResult res = cohen_sutherland_line_clip_i( &pixx, &pixy, &pixx1, &pixy1, 0,
                                 vp.pix_width, 0, vp.pix_height );
                if( res != Invisible && !b_skip ) dc.DrawLine( pixx, pixy, pixx1, pixy1 );

                plylat = plylat1;
                plylon = plylon1;
                pixx = pixxs1;
                pixy = pixys1;
            }

            ChartData->GetDBAuxPlyPoint( dbIndex, 0, j, &plylat1, &plylon1 );
            GetCanvasPointPix( plylat1, plylon1, &r1 );
            pixx1 = r1.x;
            pixy1 = r1.y;

            ClipResult res = cohen_sutherland_line_clip_i( &pixx, &pixy, &pixx1, &pixy1, 0,
                             vp.pix_width, 0, vp.pix_height );
            if( res != Invisible ) dc.DrawLine( pixx, pixy, pixx1, pixy1, false );
        }
    }

}

bool ChartCanvas::PurgeGLCanvasChartCache( ChartBase *pc )
{
    if( g_bopengl && m_glcc ) m_glcc->PurgeChartTextures( pc );
    return true;
}

wxString ChartCanvas::FormatDistanceAdaptive( double distance ) {
    wxString result;
    if( distance < 0.1 ) {
        result << wxString::Format(_T("%3.0f "), distance*1852.0 ) << _T("m");
        return result;
    }
    if( distance < 5.0 ) {
        result << wxString::Format(_T("%1.2f "), distance ) << _("NMi");
        return result;
    }
    if( distance < 100.0 ) {
        result << wxString::Format(_T("%2.1f "), distance ) << _("NMi");
        return result;
    }
    if( distance < 1000.0 ) {
        result << wxString::Format(_T("%3.0f "), distance ) << _("NMi");
        return result;
    }
    result << wxString::Format(_T("%4.0f "), distance ) << _("NMi");
    return result;
}

void RenderExtraRouteLegInfo( ocpnDC &dc, wxPoint ref_point, wxString s )
{
    wxFont *dFont = pFontMgr->GetFont( _("RouteLegInfoRollover"), 12 );
    dc.SetFont( *dFont );

    int w, h;
    int xp, yp;
    int hilite_offset = 3;
#ifdef __WXMAC__
    wxScreenDC sdc;
    sdc.GetTextExtent(s, &w, &h, NULL, NULL, dFont);
#else
    dc.GetTextExtent( s, &w, &h );
#endif

    xp = ref_point.x - w;
    yp = ref_point.y + h;
    yp += hilite_offset;

    AlphaBlending( dc, xp, yp, w, h, 0.0, GetGlobalColor( _T ( "YELO1" ) ), 172 );

    dc.SetPen( wxPen( GetGlobalColor( _T ( "UBLCK" ) ) ) );
    dc.DrawText( s, xp, yp );
}

void ChartCanvas::RenderRouteLegs( ocpnDC &dc )
{
    if( (parent_frame->nRoute_State >= 2) ||
        (m_pMeasureRoute && m_bMeasure_Active && ( m_nMeasureState >= 2 )) ) {

        double rhumbBearing, rhumbDist, gcBearing, gcBearing2, gcDist;
        DistanceBearingMercator( m_cursor_lat, m_cursor_lon, m_prev_rlat, m_prev_rlon, &rhumbBearing, &rhumbDist );
        Geodesic::GreatCircleDistBear( m_prev_rlon, m_prev_rlat, m_cursor_lon, m_cursor_lat, &gcDist, &gcBearing, &gcBearing2);
        double gcDistm = gcDist / 1852.0;

        if( ( m_prev_rlat == m_cursor_lat ) && ( m_prev_rlon == m_cursor_lon ) ) rhumbBearing = 90.;

        wxPoint destPoint, lastPoint;
        Route* route;
        int state;

        if( m_pMeasureRoute ) {
            route = m_pMeasureRoute;
            state = m_nMeasureState;
        } else {
            route = m_pMouseRoute;
            state = parent_frame->nRoute_State;
        }

        double brg = rhumbBearing;
        double dist = rhumbDist;
        route->m_NextLegGreatCircle = false;
        int milesDiff = rhumbDist - gcDistm;
        if( milesDiff > 1 ) {
            brg = gcBearing;
            dist = gcDistm;
            route->m_NextLegGreatCircle = true;
        }

        route->DrawPointWhich( dc, route->m_lastMousePointIndex, &lastPoint );

        if( route->m_NextLegGreatCircle ) {
            for( int i=1; i<=milesDiff; i++ ) {
                double p = (double)i * (1.0/(double)milesDiff);
                double pLat, pLon;
                Geodesic::GreatCircleTravel( m_prev_rlon, m_prev_rlat, gcDist*p, brg, &pLon, &pLat, &gcBearing2 );
                destPoint = VPoint.GetPixFromLL( pLat, pLon );
                route->DrawSegment( dc, &lastPoint, &destPoint, GetVP(), false );
                lastPoint = destPoint;
            }
        }
        else {
            route->DrawSegment( dc, &lastPoint, &r_rband, GetVP(), false );
        }

        wxString routeInfo;
        routeInfo << wxString::Format( wxString( "%03d° ", wxConvUTF8 ), (int) brg )
        << _T(" ") << FormatDistanceAdaptive( dist );

        wxFont *dFont = pFontMgr->GetFont( _("RouteLegInfoRollover"), 12 );
        dc.SetFont( *dFont );

        int w, h;
        int xp, yp;
        int hilite_offset = 3;
    #ifdef __WXMAC__
        wxScreenDC sdc;
        sdc.GetTextExtent(routeInfo, &w, &h, NULL, NULL, dFont);
    #else
        dc.GetTextExtent( routeInfo, &w, &h );
    #endif
        xp = r_rband.x - w;
        yp = r_rband.y;
        yp += hilite_offset;

        AlphaBlending( dc, xp, yp, w, h, 0.0, GetGlobalColor( _T ( "YELO1" ) ), 172 );

        dc.SetPen( wxPen( GetGlobalColor( _T ( "UBLCK" ) ) ) );
        dc.DrawText( routeInfo, xp, yp );

        wxString s0( _("Route: ") );
        s0 += FormatDistanceAdaptive( route->m_route_length + dist );
        RenderExtraRouteLegInfo( dc, r_rband, s0 );
    }
}

void ChartCanvas::WarpPointerDeferred( int x, int y )
{
    warp_x = x;
    warp_y = y;
    warp_flag = true;
}

int spaint;
int s_in_update;
void ChartCanvas::OnPaint( wxPaintEvent& event )
{
//      CALLGRIND_START_INSTRUMENTATION

    wxPaintDC dc( this );

    m_glcc->Show( g_bopengl );

    if( g_bopengl ) {
        if( !s_in_update ) {          // no recursion allowed, seen on lo-spec Mac
            s_in_update++;
            m_glcc->Update();
            s_in_update--;
        }

        return;
    }

    if( ( GetVP().pix_width == 0 ) || ( GetVP().pix_height == 0 ) ) return;

    wxRegion ru = GetUpdateRegion();

    int rx, ry, rwidth, rheight;
    ru.GetBox( rx, ry, rwidth, rheight );
//        printf("%d Onpaint update region box: %d %d %d %d\n", spaint++, rx, ry, rwidth, rheight);

    wxBoundingBox BltBBox;

#ifdef ocpnUSE_DIBSECTION
    ocpnMemDC temp_dc;
#else
    wxMemoryDC temp_dc;
#endif

    wxRegion rgn_chart( 0, 0, GetVP().pix_width, GetVP().pix_height );

//    In case Thumbnail is shown, set up dc clipper and blt iterator regions
    if( pthumbwin ) {
        int thumbx, thumby, thumbsx, thumbsy;
        pthumbwin->GetPosition( &thumbx, &thumby );
        pthumbwin->GetSize( &thumbsx, &thumbsy );
        wxRegion rgn_thumbwin( thumbx, thumby, thumbsx - 1, thumbsy - 1 );

        if( pthumbwin->IsShown() ) {
            rgn_chart.Subtract( rgn_thumbwin );
            ru.Subtract( rgn_thumbwin );
        }
    }

    //  Is this viewpoint the same as the previously painted one?
    bool b_newview = true;

    if( ( m_cache_vp.view_scale_ppm == VPoint.view_scale_ppm )
            && ( m_cache_vp.rotation == VPoint.rotation ) && ( m_cache_vp.clat == VPoint.clat )
            && ( m_cache_vp.clon == VPoint.clon ) && m_cache_vp.IsValid() ) {
        b_newview = false;
    }

    //  If in COG UP Mode, we may be able to use the cached rotated bitmap
    bool b_rcache_ok = false;
    if( g_bCourseUp && ( fabs( VPoint.rotation ) > 0.01 ) ) b_rcache_ok = !b_newview;

    //  If in skew compensation mode, with a skewed VP shown, we may be able to use the cached rotated bitmap
    if( g_bskew_comp && ( fabs( VPoint.skew ) > 0.01 ) ) b_rcache_ok = !b_newview;

    //  Make a special VP
    if( VPoint.b_MercatorProjectionOverride ) VPoint.SetProjectionType( PROJECTION_MERCATOR );
    ViewPort svp = VPoint;

    svp.pix_width = svp.rv_rect.width;
    svp.pix_height = svp.rv_rect.height;

//        printf("Onpaint pix %d %d\n", VPoint.pix_width, VPoint.pix_height);
//        printf("OnPaint rv_rect %d %d\n", VPoint.rv_rect.width, VPoint.rv_rect.height);

    wxRegion chart_get_region( wxRect( 0, 0, svp.pix_width, svp.pix_height ) );

    //  If we are going to use the cached rotated image, there is no need to fetch any chart data
    //  and this will do it...
    if( b_rcache_ok ) chart_get_region.Clear();

    //  Blit pan acceleration
    if( VPoint.b_quilt )          // quilted
    {
        if( m_pQuilt && !m_pQuilt->IsComposed() ) return;

        if( ( m_working_bm.GetWidth() != svp.pix_width )
                || ( m_working_bm.GetHeight() != svp.pix_height ) ) m_working_bm.Create(
                        svp.pix_width, svp.pix_height, -1 ); // make sure the target is big enoug

        if( !g_bCourseUp ) {
            bool b_save = true;

            //  If the saved wxBitmap from last OnPaint is useable
            //  calculate the blit parameters

            //  We can only do screen blit painting if subsequent ViewPorts differ by whole pixels
            //  So, in small scale bFollow mode, force the full screen render.
            //  This seems a hack....There may be better logic here.....

//                  if(m_bFollow)
//                        b_save = false;

            if( m_bm_cache_vp.IsValid() && m_cache_vp.IsValid() /*&& !m_bFollow*/) {
                if( b_newview ) {
                    wxPoint c_old = VPoint.GetPixFromLL( VPoint.clat, VPoint.clon );
                    wxPoint c_new = m_bm_cache_vp.GetPixFromLL( VPoint.clat, VPoint.clon );

                    int dy = c_new.y - c_old.y;
                    int dx = c_new.x - c_old.x;

//                              printf("In OnPaint Trying Blit dx: %d  dy:%d\n\n", dx, dy);

                    if( m_pQuilt->IsVPBlittable( VPoint, dx, dy, true ) ) {
                        if( dx || dy ) {
                            //  Blit the reuseable portion of the cached wxBitmap to a working bitmap
                            temp_dc.SelectObject( m_working_bm );

                            wxMemoryDC cache_dc;
                            cache_dc.SelectObject( m_cached_chart_bm );

                            if( dy > 0 ) {
                                if( dx > 0 ) {
                                    temp_dc.Blit( 0, 0, VPoint.pix_width - dx,
                                                               VPoint.pix_height - dy, &cache_dc, dx, dy );
                                }
                                else {
                                    temp_dc.Blit( -dx, 0, VPoint.pix_width + dx,
                                                  VPoint.pix_height - dy, &cache_dc, 0, dy );
                                }

                            } else {
                                if( dx > 0 ) {
                                    temp_dc.Blit( 0, -dy, VPoint.pix_width - dx,
                                                               VPoint.pix_height + dy, &cache_dc, dx, 0 );
                                }
                                else {
                                    temp_dc.Blit( -dx, -dy, VPoint.pix_width + dx,
                                                  VPoint.pix_height + dy, &cache_dc, 0, 0 );
                                }
                            }

                            wxRegion update_region;
                            if( dy ) {
                                if( dy > 0 ) update_region.Union(
                                        wxRect( 0, VPoint.pix_height - dy, VPoint.pix_width, dy ) );
                                else
                                    update_region.Union( wxRect( 0, 0, VPoint.pix_width, -dy ) );
                            }

                            if( dx ) {
                                if( dx > 0 ) update_region.Union(
                                        wxRect( VPoint.pix_width - dx, 0, dx, VPoint.pix_height ) );
                                else
                                    update_region.Union( wxRect( 0, 0, -dx, VPoint.pix_height ) );
                            }

                            //  Render the new region
                            m_pQuilt->RenderQuiltRegionViewOnDC( temp_dc, svp, update_region );
                            cache_dc.SelectObject( wxNullBitmap );
                        } else {
                            //    No sensible (dx, dy) change in the view, so use the cached member bitmap
                            temp_dc.SelectObject( m_cached_chart_bm );
                            b_save = false;

                        }
                        m_pQuilt->ComputeRenderRegion( svp, chart_get_region );

                    } else              // not blitable
                    {
                        temp_dc.SelectObject( m_working_bm );
                        m_pQuilt->RenderQuiltRegionViewOnDC( temp_dc, svp, chart_get_region );
                    }
                } else {
                    //    No change in the view, so use the cached member bitmap2
                    temp_dc.SelectObject( m_cached_chart_bm );
                    b_save = false;
                }
            } else      //cached bitmap is not yet valid
            {
                temp_dc.SelectObject( m_working_bm );
                m_pQuilt->RenderQuiltRegionViewOnDC( temp_dc, svp, chart_get_region );
            }

            //  Save the fully rendered quilt image as a wxBitmap member of this class
            if( b_save ) {
//                        if((m_cached_chart_bm.GetWidth() != svp.pix_width) || (m_cached_chart_bm.GetHeight() != svp.pix_height))
//                              m_cached_chart_bm.Create(svp.pix_width, svp.pix_height, -1); // target wxBitmap is big enough
                wxMemoryDC scratch_dc_0;
                scratch_dc_0.SelectObject( m_cached_chart_bm );
                scratch_dc_0.Blit( 0, 0, svp.pix_width, svp.pix_height, &temp_dc, 0, 0 );

                scratch_dc_0.SelectObject( wxNullBitmap );

                m_bm_cache_vp = VPoint;     // save the ViewPort associated with the cached wxBitmap
            }
        }

        else            // quilted, course-up
        {
            temp_dc.SelectObject( m_working_bm );
            wxRegion chart_get_all_region( wxRect( 0, 0, svp.pix_width, svp.pix_height ) );
            m_pQuilt->RenderQuiltRegionViewOnDC( temp_dc, svp, chart_get_all_region );
        }
    }

    else                  // not quilted
    {
        if( !Current_Ch ) {
            dc.SetBackground( wxBrush( *wxLIGHT_GREY ) );
            dc.Clear();
            return;
        }

        Current_Ch->RenderRegionViewOnDC( temp_dc, svp, chart_get_region );
    }

    if( !temp_dc.IsOk() ) return;

//    Arrange to render the World Chart vector data behind the rendered current chart
//    so that uncovered canvas areas show at least the world chart.
    wxRegion chartValidRegion;
    if( !VPoint.b_quilt )
        Current_Ch->GetValidCanvasRegion( svp, &chartValidRegion ); // Make a region covering the current chart on the canvas
    else
        chartValidRegion = m_pQuilt->GetFullQuiltRenderedRegion();

    //    Copy current chart region
    wxRegion backgroundRegion( wxRect( 0, 0, svp.pix_width, svp.pix_height ) );

    if( chartValidRegion.IsOk() ) backgroundRegion.Subtract( chartValidRegion );

    //    Associate with temp_dc
    temp_dc.DestroyClippingRegion();
    temp_dc.SetClippingRegion( backgroundRegion );

    //    Draw the Background Chart only in the areas NOT covered by the current chart view

    if( ( fabs( GetVP().skew ) < .01 ) && ! backgroundRegion.IsEmpty() ) {
        ocpnDC bgdc( temp_dc );
        pWorldBackgroundChart->RenderViewOnDC( bgdc, VPoint );
    }

    wxMemoryDC *pChartDC = &temp_dc;
    wxMemoryDC rotd_dc;

    if( ( ( fabs( GetVP().rotation ) > 0.01 ) )
            || ( g_bskew_comp && ( fabs( GetVP().skew ) > 0.01 ) ) ) {

        //  Can we use the current rotated image cache?
        if( !b_rcache_ok ) {
#ifdef __WXMSW__
            wxMemoryDC tbase_dc;
            wxBitmap bm_base( svp.pix_width, svp.pix_height, -1 );
            tbase_dc.SelectObject( bm_base );
            tbase_dc.Blit( 0, 0, svp.pix_width, svp.pix_height, &temp_dc, 0, 0 );
            tbase_dc.SelectObject( wxNullBitmap );
#else
            const wxBitmap &bm_base = temp_dc.GetSelectedBitmap();
#endif

            wxImage base_image;
            if( bm_base.IsOk() ) base_image = bm_base.ConvertToImage();

            //    Use a local static image rotator to improve wxWidgets code profile
            //    Especially, on GTK the wxRound and wxRealPoint functions are very expensive.....
            double angle;
            angle = -GetVP().rotation;
            angle += GetVP().skew;

            wxImage ri;
            bool b_rot_ok = false;
            if( base_image.IsOk() ) {
                ViewPort rot_vp = GetVP();

                m_b_rot_hidef = false;
//                              if(g_bskew_comp && (fabs(GetVP().skew) > 0.01))
//                                    m_b_rot_hidef = true;

                ri = Image_Rotate( base_image, angle,
                                   wxPoint( GetVP().rv_rect.width / 2, GetVP().rv_rect.height / 2 ),
                                   m_b_rot_hidef, &m_roffset );

                if( ( rot_vp.view_scale_ppm == VPoint.view_scale_ppm )
                        && ( rot_vp.rotation == VPoint.rotation ) && ( rot_vp.clat == VPoint.clat )
                        && ( rot_vp.clon == VPoint.clon ) && rot_vp.IsValid() && ( ri.IsOk() ) ) {
                    b_rot_ok = true;
                }
            }

            if( b_rot_ok ) {
                delete m_prot_bm;
                m_prot_bm = new wxBitmap( ri );

            }

            m_roffset.x += VPoint.rv_rect.x;
            m_roffset.y += VPoint.rv_rect.y;

        }

        if( m_prot_bm && m_prot_bm->IsOk() ) {
            rotd_dc.SelectObject( *m_prot_bm );
            pChartDC = &rotd_dc;
        } else {
            pChartDC = &temp_dc;
            m_roffset = wxPoint( 0, 0 );
        }

    } else {
        pChartDC = &temp_dc;
        m_roffset = wxPoint( 0, 0 );

    }

    wxPoint offset = m_roffset;

    //        Save the PixelCache viewpoint for next time
    m_cache_vp = VPoint;

//    Set up a scratch DC for overlay objects
    wxRegion rgn_blit;
    wxMemoryDC mscratch_dc;
    mscratch_dc.SelectObject( *pscratch_bm );

    mscratch_dc.ResetBoundingBox();
    mscratch_dc.DestroyClippingRegion();
    mscratch_dc.SetClippingRegion( rgn_chart );

    //    Blit the externally invalidated areas of the chart onto the scratch dc
    rgn_blit = ru;
    wxRegionIterator upd( rgn_blit ); // get the update rect list
    while( upd ) {
        wxRect rect = upd.GetRect();

        mscratch_dc.Blit( rect.x, rect.y, rect.width, rect.height, pChartDC, rect.x - offset.x,
                          rect.y - offset.y );
        upd++;
    }

//    Draw the rest of the overlay objects directly on the scratch dc
    ocpnDC scratch_dc( mscratch_dc );
    DrawOverlayObjects( scratch_dc, ru );

    if( m_bShowTide ) DrawAllTidesInBBox( scratch_dc, GetVP().GetBBox(), true, true );

    if( m_bShowCurrent ) DrawAllCurrentsInBBox( scratch_dc, GetVP().GetBBox(), true, true );

#if 0
//  Using yet another bitmap and DC, draw semi-static overlay objects if necessary

    /*    Why go to all this trouble?
     Answer:  Calculating and rendering tides and currents is expensive,
     and the data only change every 15 minutes or so.  So, keep a "mask blit-able"
     copy in persistent storage, and use as necessary.
     */
    if ( m_bShowTide || m_bShowCurrent )         // Showing T/C?
    {
        if ( 1/*b_newview*/|| m_bTCupdate )         // need to update the overlay
        {
            delete pss_overlay_bmp;
            pss_overlay_bmp = DrawTCCBitmap(&mscratch_dc);
        }

        //    blit the semi-static overlay onto the scratch DC if it is needed
        if ( NULL != pss_overlay_bmp )
        {
            wxMemoryDC ssdc_r;
            ssdc_r.SelectObject ( *pss_overlay_bmp );

            wxRegionIterator upd_final ( rgn_blit );
            while ( upd_final )
            {
                wxRect rect = upd_final.GetRect();
                mscratch_dc.Blit ( rect.x, rect.y, rect.width, rect.height,
                                   &ssdc_r, rect.x, rect.y, wxCOPY, true );      // Blit with mask
                upd_final ++;
            }

            ssdc_r.SelectObject ( wxNullBitmap );
        }
    }
#endif

    //quiting?
    if( g_bquiting ) {
#ifdef ocpnUSE_DIBSECTION
        ocpnMemDC q_dc;
#else
        wxMemoryDC q_dc;
#endif
        wxBitmap qbm( GetVP().pix_width, GetVP().pix_height );
        q_dc.SelectObject( qbm );

        // Get a copy of the screen
        q_dc.Blit( 0, 0, GetVP().pix_width, GetVP().pix_height, &mscratch_dc, 0, 0 );

        //  Draw a rectangle over the screen with a stipple brush
        wxBrush qbr( *wxBLACK, wxFDIAGONAL_HATCH );
        q_dc.SetBrush( qbr );
        q_dc.DrawRectangle( 0, 0, GetVP().pix_width, GetVP().pix_height );

        // Blit back into source
        mscratch_dc.Blit( 0, 0, GetVP().pix_width, GetVP().pix_height, &q_dc, 0, 0, wxCOPY );

        q_dc.SelectObject( wxNullBitmap );

    }

//    And finally, blit the scratch dc onto the physical dc
    wxRegionIterator upd_final( rgn_blit );
    while( upd_final ) {
        wxRect rect = upd_final.GetRect();
        dc.Blit( rect.x, rect.y, rect.width, rect.height, &mscratch_dc, rect.x, rect.y );
        upd_final++;
    }

    //  Test code to validate the dc drawing rectangle....
    /*
     wxRegionIterator upd_ru ( ru ); // get the update rect list
     while ( upd_ru )
     {
     wxRect rect = upd_ru.GetRect();

     dc.SetPen(wxPen(*wxRED));
     dc.SetBrush(wxBrush(*wxRED, wxTRANSPARENT));
     dc.DrawRectangle(rect);
     upd_ru ++ ;
     }
     */

//    Deselect the chart bitmap from the temp_dc, so that it will not be destroyed in the temp_dc dtor
    temp_dc.SelectObject( wxNullBitmap );
//    And for the scratch bitmap
    mscratch_dc.SelectObject( wxNullBitmap );

    dc.DestroyClippingRegion();

    PaintCleanup();
//      CALLGRIND_STOP_INSTRUMENTATION

}

void ChartCanvas::PaintCleanup()
{
//    Handle the current graphic window, if present

    if( pCwin ) {
        pCwin->Show();
        if( m_bTCupdate ) {
            pCwin->Refresh();
            pCwin->Update();
        }
    }

//    And set flags for next time
    m_bTCupdate = false;

//    Handle deferred WarpPointer
    if( warp_flag ) {
        WarpPointer( warp_x, warp_y );
        warp_flag = false;
    }
}

#if 0
wxColour GetErrorGraphicColor(double val)
{
    /*
     double valm = wxMin(val_max, val);

     unsigned char green = (unsigned char)(255 * (1 - (valm/val_max)));
     unsigned char red   = (unsigned char)(255 * (valm/val_max));

     wxImage::HSVValue hv = wxImage::RGBtoHSV(wxImage::RGBValue(red, green, 0));

     hv.saturation = 1.0;
     hv.value = 1.0;

     wxImage::RGBValue rv = wxImage::HSVtoRGB(hv);
     return wxColour(rv.red, rv.green, rv.blue);
     */

    //    HTML colors taken from NOAA WW3 Web representation
    wxColour c;
    if((val > 0) && (val < 1)) c.Set(_T("#002ad9"));
    else if((val >= 1) && (val < 2)) c.Set(_T("#006ed9"));
    else if((val >= 2) && (val < 3)) c.Set(_T("#00b2d9"));
    else if((val >= 3) && (val < 4)) c.Set(_T("#00d4d4"));
    else if((val >= 4) && (val < 5)) c.Set(_T("#00d9a6"));
    else if((val >= 5) && (val < 7)) c.Set(_T("#00d900"));
    else if((val >= 7) && (val < 9)) c.Set(_T("#95d900"));
    else if((val >= 9) && (val < 12)) c.Set(_T("#d9d900"));
    else if((val >= 12) && (val < 15)) c.Set(_T("#d9ae00"));
    else if((val >= 15) && (val < 18)) c.Set(_T("#d98300"));
    else if((val >= 18) && (val < 21)) c.Set(_T("#d95700"));
    else if((val >= 21) && (val < 24)) c.Set(_T("#d90000"));
    else if((val >= 24) && (val < 27)) c.Set(_T("#ae0000"));
    else if((val >= 27) && (val < 30)) c.Set(_T("#8c0000"));
    else if((val >= 30) && (val < 36)) c.Set(_T("#870000"));
    else if((val >= 36) && (val < 42)) c.Set(_T("#690000"));
    else if((val >= 42) && (val < 48)) c.Set(_T("#550000"));
    else if( val >= 48) c.Set(_T("#410000"));

    return c;
}

void ChartCanvas::RenderGeorefErrorMap( wxMemoryDC *pmdc, ViewPort *vp)
{
    wxImage gr_image(vp->pix_width, vp->pix_height);
    gr_image.InitAlpha();

    double maxval = -10000;
    double minval = 10000;

    double rlat, rlon;
    double glat, glon;

    GetCanvasPixPoint(0, 0, rlat, rlon);

    for(int i=1; i < vp->pix_height-1; i++)
    {
        for(int j=0; j < vp->pix_width; j++)
        {
            // Reference mercator value
//                  vp->GetMercatorLLFromPix(wxPoint(j, i), &rlat, &rlon);

            // Georef value
            GetCanvasPixPoint(j, i, glat, glon);

            maxval = wxMax(maxval, (glat - rlat));
            minval = wxMin(minval, (glat - rlat));

        }
        rlat = glat;
    }

    GetCanvasPixPoint(0, 0, rlat, rlon);
    for(int i=1; i < vp->pix_height-1; i++)
    {
        for(int j=0; j < vp->pix_width; j++)
        {
            // Reference mercator value
//                  vp->GetMercatorLLFromPix(wxPoint(j, i), &rlat, &rlon);

            // Georef value
            GetCanvasPixPoint(j, i, glat, glon);

            double f = ((glat - rlat)-minval)/(maxval - minval);

            double dy = (f * 40);

            wxColour c = GetErrorGraphicColor(dy);
            unsigned char r = c.Red();
            unsigned char g = c.Green();
            unsigned char b = c.Blue();

            gr_image.SetRGB(j, i, r,g,b);
            if((glat - rlat )!= 0)
                gr_image.SetAlpha(j, i, 128);
            else
                gr_image.SetAlpha(j, i, 255);

        }
        rlat = glat;
    }

    //    Create a Bitmap
    wxBitmap *pbm = new wxBitmap(gr_image);
    wxMask *gr_mask = new wxMask(*pbm, wxColour(0,0,0));
    pbm->SetMask(gr_mask);

    pmdc->DrawBitmap(*pbm, 0,0);

    delete pbm;

}

#endif

void ChartCanvas::CancelMouseRoute()
{
    parent_frame->nRoute_State = 0;
    m_pMouseRoute = NULL;
}

int ChartCanvas::GetNextContextMenuId()
{
    return ID_DEF_MENU_LAST + 100;  //Allowing for 100 dynamic menu item identifiers
}

bool ChartCanvas::SetCursor( const wxCursor &c )
{
    if( g_bopengl ) return m_glcc->SetCursor( c );
    else
        return wxWindow::SetCursor( c );
}

void ChartCanvas::Refresh( bool eraseBackground, const wxRect *rect )
{

    if( g_bopengl ) {

        m_glcc->Refresh( eraseBackground, NULL ); // We always are going to render the entire screen anyway, so make
        // sure that the window managers understand the invalid area
        // is actually the entire client area.

        //  We need to selectively Refresh some child windows, if they are visible.
        //  Note that some children are refreshed elsewhere on timer ticks, so don't need attention here.

        //      ChartInfo window
        if( m_pCIWin && m_pCIWin->IsShown() ) {
            m_pCIWin->Raise();
            m_pCIWin->Refresh( false );
        }

        if( pthumbwin && pthumbwin->IsShown() ) {
            pthumbwin->Raise();
            pthumbwin->Refresh( false );
        }

    } else
        wxWindow::Refresh( eraseBackground, rect );

}

void ChartCanvas::Update()
{
    if( g_bopengl ) {
        m_glcc->Update();
//          m_glcc->render(); /* for some reason repaint not triggered */
    } else
        wxWindow::Update();
}

void ChartCanvas::EmbossCanvas( ocpnDC &dc, emboss_data *pemboss, int x, int y )
{
    const double factor = 200;

    if( dc.GetDC() ) {
        wxMemoryDC *pmdc = dynamic_cast<wxMemoryDC*>( dc.GetDC() );
        wxASSERT_MSG ( pmdc, wxT ( "dc to EmbossCanvas not a memory dc" ) );

        //Grab a snipped image out of the chart
        wxMemoryDC snip_dc;
        wxBitmap snip_bmp( pemboss->width, pemboss->height, -1 );
        snip_dc.SelectObject( snip_bmp );

        snip_dc.Blit( 0, 0, pemboss->width, pemboss->height, pmdc, x, y );

        wxImage snip_img = snip_bmp.ConvertToImage();

        //  Apply Emboss map to the snip image
        unsigned char* pdata = snip_img.GetData();
        {
            for( int y = 0; y < pemboss->height; y++ ) {
                int map_index = ( y * pemboss->width );
                for( int x = 0; x < pemboss->width; x++ ) {
                    double val = ( pemboss->pmap[map_index] * factor ) / 256.;

                    int nred = (int) ( ( *pdata ) + val );
                    nred = nred > 255 ? 255 : ( nred < 0 ? 0 : nred );
                    *pdata++ = (unsigned char) nred;

                    int ngreen = (int) ( ( *pdata ) + val );
                    ngreen = ngreen > 255 ? 255 : ( ngreen < 0 ? 0 : ngreen );
                    *pdata++ = (unsigned char) ngreen;

                    int nblue = (int) ( ( *pdata ) + val );
                    nblue = nblue > 255 ? 255 : ( nblue < 0 ? 0 : nblue );
                    *pdata++ = (unsigned char) nblue;

                    map_index++;
                }
            }
        }

        //  Convert embossed snip to a bitmap
        wxBitmap emb_bmp( snip_img );

        //  Map to another memoryDC
        wxMemoryDC result_dc;
        result_dc.SelectObject( emb_bmp );

        //  Blit to target
        pmdc->Blit( x, y, pemboss->width, pemboss->height, &result_dc, 0, 0 );

        result_dc.SelectObject( wxNullBitmap );
        snip_dc.SelectObject( wxNullBitmap );
    }
#ifndef __WXMSW__
    else if(0/*b_useTexRect*/)
    {
        int w = pemboss->width, h = pemboss->height;
        glEnable(GL_TEXTURE_RECTANGLE_ARB);

        // render using opengl and alpha blending
        if(!pemboss->gltexind) { /* upload to texture */
            /* convert to luminance alpha map */
            int size = pemboss->width*pemboss->height;
            char *data = new char[2*size];
            for(int i=0; i<size; i++) {
                data[2*i] = pemboss->pmap[i] > 0 ? 0 : 255;
                data[2*i+1] = abs(pemboss->pmap[i]);
            }

            glGenTextures(1, &pemboss->gltexind);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, pemboss->gltexind);
            glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE_ALPHA, w, h,
                         0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data);

            delete [] data;
        }

        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, pemboss->gltexind);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

        glColor4f(1, 1, 1, factor / 256);

        glBegin(GL_QUADS);
        glTexCoord2i(0, 0), glVertex2i(x, y);
        glTexCoord2i(w, 0), glVertex2i(x+w, y);
        glTexCoord2i(w, h), glVertex2i(x+w, y+h);
        glTexCoord2i(0, h), glVertex2i(x, y+h);
        glEnd();

        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_RECTANGLE_ARB);
    }
#endif
    else {
        int a = pemboss->width;
        int p = 0;
        while( a ) {
            a = a >> 1;
            p++;
        }
        int width_p2 = 1 << p;

        a = pemboss->height;
        p = 0;
        while( a ) {
            a = a >> 1;
            p++;
        }
        int height_p2 = 1 << p;

        int w = pemboss->width, h = pemboss->height;

        glEnable( GL_TEXTURE_2D );

        // render using opengl and alpha blending
        if( !pemboss->gltexind ) { /* upload to texture */
            /* convert to luminance alpha map */
            int size = width_p2 * height_p2;
            char *data = new char[2 * size];
            for( int i = 0; i < h; i++ ) {
                for( int j = 0; j < width_p2; j++ ) {
                    if( j < w ) {
                        data[2 * ( ( i * width_p2 ) + j )] =
                            pemboss->pmap[( i * w ) + j] > 0 ? 0 : 255;
                        data[2 * ( ( i * width_p2 ) + j ) + 1] = abs(
                                    pemboss->pmap[( i * w ) + j] );
                    }
                }
            }

            glGenTextures( 1, &pemboss->gltexind );
            glBindTexture( GL_TEXTURE_2D, pemboss->gltexind );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width_p2, height_p2, 0,
                          GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

            delete[] data;
        }

        glBindTexture( GL_TEXTURE_2D, pemboss->gltexind );

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );

        glColor4f( 1, 1, 1, factor / 256 );

        double wp = (double) w / width_p2;
        double hp = (double) h / height_p2;

        glBegin( GL_QUADS );
        glTexCoord2f( 0, 0 ), glVertex2i( x, y );
        glTexCoord2f( wp, 0 ), glVertex2i( x + w, y );
        glTexCoord2f( wp, hp ), glVertex2i( x + w, y + h );
        glTexCoord2f( 0, hp ), glVertex2i( x, y + h );
        glEnd();

        glDisable( GL_BLEND );
        glDisable( GL_TEXTURE_2D );
    }
}

void ChartCanvas::EmbossOverzoomIndicator( ocpnDC &dc )
{
    if( !g_bshow_overzoom_emboss ) return;

    if( GetQuiltMode() ) {
        double chart_native_ppm;
        chart_native_ppm = m_canvas_scale_factor / m_pQuilt->GetRefNativeScale();

        double zoom_factor = GetVP().view_scale_ppm / chart_native_ppm;

        if( zoom_factor <= 4.0 ) return;
    } else {
        double chart_native_ppm;
        if( Current_Ch ) chart_native_ppm = m_canvas_scale_factor / Current_Ch->GetNativeScale();
        else
            chart_native_ppm = m_true_scale_ppm;

        double zoom_factor = GetVP().view_scale_ppm / chart_native_ppm;
        if( Current_Ch ) {
#ifdef USE_S57
            //    Special case for cm93
            if( Current_Ch->GetChartType() == CHART_TYPE_CM93COMP ) {
                if( zoom_factor > 8.0 ) {

                    cm93compchart *pch = (cm93compchart *) Current_Ch;
                    if( pch ) {
                        wxPen mPen( GetGlobalColor( _T("UINFM") ), 2, wxSHORT_DASH );
                        dc.SetPen( mPen );
                        pch->RenderNextSmallerCellOutlines( dc, GetVP() );
                    }
                } else
                    return;
            } else
#endif
                if( zoom_factor <= 4.0 ) return;
        }
    }

    EmbossCanvas( dc, m_pEM_OverZoom, 0, 40 );
}

void ChartCanvas::DrawOverlayObjects( ocpnDC &dc, const wxRegion& ru )
{
    GridDraw( dc );

    if( g_pi_manager ) {
        g_pi_manager->SendViewPortToRequestingPlugIns( GetVP() );
        g_pi_manager->RenderAllCanvasOverlayPlugIns( dc, GetVP() );
    }

    AISDrawAreaNotices( dc );

    EmbossDepthScale( dc );
    EmbossOverzoomIndicator( dc );

    DrawAllRoutesInBBox( dc, GetVP().GetBBox(), ru );
    DrawAllWaypointsInBBox( dc, GetVP().GetBBox(), ru, true ); // true draws only isolated marks

    AISDraw( dc );
    ShipDraw( dc );
    AlertDraw( dc );

    RenderAllChartOutlines( dc, GetVP() );
    RenderRouteLegs( dc );
    ScaleBarDraw( dc );
#ifdef USE_S57
    s57_DrawExtendedLightSectors( dc, VPoint, extendedSectorLegs );
#endif

    if( m_pRolloverWin && m_pRolloverWin->IsActive() ) {
        dc.DrawBitmap( *(m_pRolloverWin->GetBitmap()),
                m_pRolloverWin->GetPosition().x,
                m_pRolloverWin->GetPosition().y, false );
    }
    if( m_pAISRolloverWin && m_pAISRolloverWin->IsActive() ) {
        dc.DrawBitmap( *(m_pAISRolloverWin->GetBitmap()),
                m_pAISRolloverWin->GetPosition().x,
                m_pAISRolloverWin->GetPosition().y, false );
    }
}

void ChartCanvas::EmbossDepthScale( ocpnDC &dc )
{
    if( !g_bShowDepthUnits ) return;

    int depth_unit_type = DEPTH_UNIT_UNKNOWN;

    if( GetQuiltMode() ) {
        wxString s = m_pQuilt->GetQuiltDepthUnit();
        s.MakeUpper();
        if( s == _T("FEET") ) depth_unit_type = DEPTH_UNIT_FEET;
        else if( s.StartsWith( _T("FATHOMS") ) ) depth_unit_type = DEPTH_UNIT_FATHOMS;
        else if( s.StartsWith( _T("METERS") ) ) depth_unit_type = DEPTH_UNIT_METERS;
        else if( s.StartsWith( _T("METRES") ) ) depth_unit_type = DEPTH_UNIT_METERS;
        else if( s.StartsWith( _T("METRIC") ) ) depth_unit_type = DEPTH_UNIT_METERS;

    } else {
        if( Current_Ch ) {
            depth_unit_type = Current_Ch->GetDepthUnitType();
#ifdef USE_S57
            if( Current_Ch->GetChartFamily() == CHART_FAMILY_VECTOR ) depth_unit_type =
                    ps52plib->m_nDepthUnitDisplay + 1;
#endif
        }
    }

    emboss_data *ped = NULL;
    switch( depth_unit_type ) {
    case DEPTH_UNIT_FEET:
        ped = m_pEM_Feet;
        break;
    case DEPTH_UNIT_METERS:
        ped = m_pEM_Meters;
        break;
    case DEPTH_UNIT_FATHOMS:
        ped = m_pEM_Fathoms;
        break;
    default:
        ped = NULL;
        break;
    }

    if( ped ) EmbossCanvas( dc, ped, ( GetVP().pix_width - ped->width ), 40 );
}

void ChartCanvas::CreateDepthUnitEmbossMaps( ColorScheme cs )
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    wxFont font;
    if( style->embossFont == wxEmptyString )
        font = wxFont( 60, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD );
    else
        font = wxFont( style->embossHeight, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, style->embossFont );

    int emboss_width = 500;
    int emboss_height = 100;

// Free any existing emboss maps
    delete m_pEM_Feet;
    delete m_pEM_Meters;
    delete m_pEM_Fathoms;

// Create the 3 DepthUnit emboss map structures
    m_pEM_Feet = CreateEmbossMapData( font, emboss_width, emboss_height, _("Feet"), cs );
    m_pEM_Meters = CreateEmbossMapData( font, emboss_width, emboss_height, _("Meters"), cs );
    m_pEM_Fathoms = CreateEmbossMapData( font, emboss_width, emboss_height, _("Fathoms"), cs );
}

void ChartCanvas::CreateOZEmbossMapData( ColorScheme cs )
{
    delete m_pEM_OverZoom;

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    int w, h;

    wxFont font;
    if( style->embossFont == wxEmptyString )
        font = wxFont( 40, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD );
    else
        font = wxFont( style->embossHeight, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, style->embossFont );

    wxClientDC dc( this );
    dc.SetFont( font );
    dc.GetTextExtent( _("OverZoom"), &w, &h );

    m_pEM_OverZoom = CreateEmbossMapData( font, w + 10, h + 10, _("OverZoom"), cs );
}

emboss_data *ChartCanvas::CreateEmbossMapData( wxFont &font, int width, int height,
        const wxChar *str, ColorScheme cs )
{
    int *pmap;

    //  Create a temporary bitmap
    wxBitmap bmp( width, height, -1 );

    // Create a memory DC
    wxMemoryDC temp_dc;
    temp_dc.SelectObject( bmp );

    //  Paint on it
    temp_dc.SetBackground( *wxWHITE_BRUSH );
    temp_dc.SetTextBackground( *wxWHITE );
    temp_dc.SetTextForeground( *wxBLACK );

    temp_dc.Clear();

    temp_dc.SetFont( font );

    int str_w, str_h;
    temp_dc.GetTextExtent( wxString( str, wxConvUTF8 ), &str_w, &str_h );
    temp_dc.DrawText( wxString( str, wxConvUTF8 ), width - str_w - 10, 10 );

    //  Deselect the bitmap
    temp_dc.SelectObject( wxNullBitmap );

    //  Convert bitmap the wxImage for manipulation
    wxImage img = bmp.ConvertToImage();

    double val_factor;
    switch( cs ) {
    case GLOBAL_COLOR_SCHEME_DAY:
    default:
        val_factor = 1;
        break;
    case GLOBAL_COLOR_SCHEME_DUSK:
        val_factor = .5;
        break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
        val_factor = .25;
        break;
    }

    int val;
    int index;
    pmap = (int *) calloc( width * height * sizeof(int), 1 );
    //  Create emboss map by differentiating the emboss image
    //  and storing integer results in pmap
    //  n.b. since the image is B/W, it is sufficient to check
    //  one channel (i.e. red) only
    for( int y = 1; y < height - 1; y++ ) {
        for( int x = 1; x < width - 1; x++ ) {
            val = img.GetRed( x + 1, y + 1 ) - img.GetRed( x - 1, y - 1 );  // range +/- 256
            val = (int) ( val * val_factor );
            index = ( y * width ) + x;
            pmap[index] = val;

        }
    }

    emboss_data *pret = new emboss_data;
    pret->pmap = pmap;
    pret->width = width;
    pret->height = height;

    return pret;
}

//----------------------------------------------------------------------------
//  Get a wxBitmap with wxMask associated containing the semi-static overlays
//----------------------------------------------------------------------------

wxBitmap *ChartCanvas::DrawTCCBitmap( wxDC *pbackground_dc, bool bAddNewSelpoints )
{
    wxBitmap *p_bmp = new wxBitmap( GetVP().pix_width, GetVP().pix_height, -1 );

    //      Here is the new drawing DC
    wxMemoryDC ssdc;
    ssdc.SelectObject( *p_bmp );
    ssdc.SetBackground( *wxWHITE_BRUSH );

    //  if a background dc is provided, use it as wallpaper
    if( pbackground_dc ) ssdc.Blit( 0, 0, GetVP().pix_width, GetVP().pix_height, pbackground_dc, 0,
                                        0 );
    else
        ssdc.Clear();

    //      Believe it or not, it is faster to REDRAW the overlay objects
    //      onto a mono bitmap, and then convert it into a mask bitmap
    //      than it is to create a mask from a colour bmp.
    //      Look at the wx code.  It goes through wxImage conversion, etc...
    //      So, create a mono DC, drawing white-on-black
    wxMemoryDC ssdc_mask;
    wxBitmap mask_bmp( GetVP().pix_width, GetVP().pix_height, 1 );
    ssdc_mask.SelectObject( mask_bmp );

    //      On X11, the drawing is Black on White, and the mask bitmap is inverted before
    //      making into a mask.
    //      On MSW and GTK, the drawing is White on Black, and no inversion is required
    //      Todo....  Some wxWidgets problem with this....
#ifndef __WXX11__
    ssdc_mask.SetBackground( *wxBLACK_BRUSH );
#endif

    ssdc_mask.Clear();

//    Maybe draw the Tide Points
    ocpnDC ossdc( ssdc ), ossdc_mask( ssdc_mask );

    if( m_bShowTide ) {
        // Rebuild Selpoints list on new map
        DrawAllTidesInBBox( ossdc, GetVP().GetBBox(), bAddNewSelpoints || !bShowingTide, true );
        DrawAllTidesInBBox( ossdc_mask, GetVP().GetBBox(), false, true, true );    // onto the mask
        bShowingTide = true;
    } else
        bShowingTide = false;

//    Maybe draw the current arrows
    if( m_bShowCurrent ) {
        // Rebuild Selpoints list on new map
        // and force redraw
        DrawAllCurrentsInBBox( ossdc, GetVP().GetBBox(), bAddNewSelpoints || !bShowingCurrent,
                               true );
        DrawAllCurrentsInBBox( ossdc_mask, GetVP().GetBBox(), false, true, true );  // onto the mask
        bShowingCurrent = true;
    } else
        bShowingCurrent = false;

    ssdc.SelectObject( wxNullBitmap );

#ifdef __WXX11__
    //      Invert the mono bmp, to make a useable mask bmp
    wxMemoryDC ssdc_mask_invert;
    wxBitmap mask_bmp_invert ( GetVP().pix_width, GetVP().pix_height, 1 );
    ssdc_mask_invert.SelectObject ( mask_bmp_invert );
    ssdc_mask_invert.Blit ( 0, 0, GetVP().pix_width, GetVP().pix_height,
                            &ssdc_mask, 0, 0, wxSRC_INVERT );

    ssdc_mask_invert.SelectObject ( wxNullBitmap );
    pss_overlay_mask = new wxMask ( mask_bmp_invert );
    ssdc_mask.SelectObject ( wxNullBitmap );
#else
    ssdc_mask.SelectObject( wxNullBitmap );
    pss_overlay_mask = new wxMask( mask_bmp );
#endif

    //      Create and associate the mask
    p_bmp->SetMask( pss_overlay_mask );

    return p_bmp;
}

extern bool g_bTrackActive;

void ChartCanvas::DrawAllRoutesInBBox( ocpnDC& dc, LLBBox& BltBBox, const wxRegion& clipregion )
{
    Route *active_route = NULL;
    Route *active_track = NULL;

    wxDC *pdc = dc.GetDC();
    if( pdc ) {
        pdc->DestroyClippingRegion();
        wxDCClipper( *pdc, clipregion );
    }

    wxRouteListNode *node = pRouteList->GetFirst();
    while( node ) {
        bool b_run = false;
        bool b_drawn = false;
        Route *pRouteDraw = node->GetData();
        if( pRouteDraw ) {
            if( pRouteDraw->IsTrack() ) {
                Track *trk = (Track *) pRouteDraw;
                if( trk->IsRunning() ) {
                    b_run = true;
                    active_track = pRouteDraw;
                }

                if( pRouteDraw->IsActive() || pRouteDraw->IsSelected() ) active_route = pRouteDraw;

            }

            wxBoundingBox test_box = pRouteDraw->RBBox;

            if( b_run ) test_box.Expand( gLon, gLat );

            if( BltBBox.Intersect( test_box, 0 ) != _OUT ) // Route is not wholly outside window
            {
                b_drawn = true;

                if( ( pRouteDraw != active_route ) && ( pRouteDraw != active_track ) ) pRouteDraw->Draw(
                        dc, GetVP() );
            } else if( pRouteDraw->CrossesIDL() ) {
                wxPoint2DDouble xlate( -360., 0. );
                wxBoundingBox test_box1 = pRouteDraw->RBBox;
                test_box1.Translate( xlate );
                if( b_run ) test_box1.Expand( gLon, gLat );

                if( BltBBox.Intersect( test_box1, 0 ) != _OUT ) // Route is not wholly outside window
                {
                    b_drawn = true;
                    if( ( pRouteDraw != active_route ) && ( pRouteDraw != active_track ) ) pRouteDraw->Draw(
                            dc, GetVP() );
                }
            }

            //      Need to quick check for the case where VP crosses IDL
            if( !b_drawn ) {
                if( ( BltBBox.GetMinX() < -180. ) && ( BltBBox.GetMaxX() > -180. ) ) {
                    wxPoint2DDouble xlate( -360., 0. );
                    wxBoundingBox test_box2 = pRouteDraw->RBBox;
                    test_box2.Translate( xlate );
                    if( BltBBox.Intersect( test_box2, 0 ) != _OUT ) // Route is not wholly outside window
                    {
                        b_drawn = true;
                        if( ( pRouteDraw != active_route ) && ( pRouteDraw != active_track ) ) pRouteDraw->Draw(
                                dc, GetVP() );
                    }
                } else if( !b_drawn && ( BltBBox.GetMinX() < 180. ) && ( BltBBox.GetMaxX() > 180. ) ) {
                    wxPoint2DDouble xlate( 360., 0. );
                    wxBoundingBox test_box3 = pRouteDraw->RBBox;
                    test_box3.Translate( xlate );
                    if( BltBBox.Intersect( test_box3, 0 ) != _OUT ) // Route is not wholly outside window
                    {
                        b_drawn = true;
                        if( ( pRouteDraw != active_route ) && ( pRouteDraw != active_track ) ) pRouteDraw->Draw(
                                dc, GetVP() );
                    }
                }
            }
        }

        node = node->GetNext();
    }

    //  Draw any active or selected route (or track) last, so that is is always on top
    if( active_route ) active_route->Draw( dc, GetVP() );
    if( active_track ) active_track->Draw( dc, GetVP() );
}

void ChartCanvas::DrawAllWaypointsInBBox( ocpnDC& dc, LLBBox& BltBBox, const wxRegion& clipregion,
        bool bDrawMarksOnly )
{
//        wxBoundingBox bbx;
    wxDC *pdc = dc.GetDC();
    if( pdc ) {
        wxDCClipper( *pdc, clipregion );
    }

    wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();

    while( node ) {
        RoutePoint *pWP = node->GetData();
        if( pWP ) {
            if( ( bDrawMarksOnly ) && ( pWP->m_bIsInRoute || pWP->m_bIsInTrack ) ) {
                node = node->GetNext();
                continue;
            } else {
                if( BltBBox.GetValid() ) {
                    if( BltBBox.PointInBox( pWP->m_lon, pWP->m_lat, 0 ) ) pWP->Draw( dc, NULL );
                }
            }
        }

        node = node->GetNext();
    }

    // draw anchor watch rings, if activated

    if( pAnchorWatchPoint1 || pAnchorWatchPoint2 ) {
        wxPoint r1, r2;
        wxPoint lAnchorPoint1, lAnchorPoint2;
        double lpp1 = 0.0;
        double lpp2 = 0.0;
        if( pAnchorWatchPoint1 ) {
            lpp1 = GetAnchorWatchRadiusPixels( pAnchorWatchPoint1 );
            GetCanvasPointPix( pAnchorWatchPoint1->m_lat, pAnchorWatchPoint1->m_lon,
                               &lAnchorPoint1 );

        }
        if( pAnchorWatchPoint2 ) {
            lpp2 = GetAnchorWatchRadiusPixels( pAnchorWatchPoint2 );
            GetCanvasPointPix( pAnchorWatchPoint2->m_lat, pAnchorWatchPoint2->m_lon,
                               &lAnchorPoint2 );

        }

        wxPen ppPeng( GetGlobalColor( _T ( "UGREN" ) ), 2 );
        wxPen ppPenr( GetGlobalColor( _T ( "URED" ) ), 2 );

        wxBrush *ppBrush = wxTheBrushList->FindOrCreateBrush( wxColour( 0, 0, 0 ), wxTRANSPARENT );
        dc.SetBrush( *ppBrush );

        if( lpp1 > 0 ) {
            dc.SetPen( ppPeng );
            dc.StrokeCircle( lAnchorPoint1.x, lAnchorPoint1.y, fabs( lpp1 ) );
        }

        if( lpp2 > 0 ) {
            dc.SetPen( ppPeng );
            dc.StrokeCircle( lAnchorPoint2.x, lAnchorPoint2.y, fabs( lpp2 ) );
        }

        if( lpp1 < 0 ) {
            dc.SetPen( ppPenr );
            dc.StrokeCircle( lAnchorPoint1.x, lAnchorPoint1.y, fabs( lpp1 ) );
        }

        if( lpp2 < 0 ) {
            dc.SetPen( ppPenr );
            dc.StrokeCircle( lAnchorPoint2.x, lAnchorPoint2.y, fabs( lpp2 ) );
        }
    }
}

double ChartCanvas::GetAnchorWatchRadiusPixels( RoutePoint *pAnchorWatchPoint )
{
    double lpp = 0.;
    wxPoint r1;
    wxPoint lAnchorPoint;
    double d1 = 0.0;
    double dabs;
    double tlat1, tlon1;

    if( pAnchorWatchPoint ) {
        ( pAnchorWatchPoint->GetName() ).ToDouble( &d1 );
        d1 = AnchorDistFix( d1, AnchorPointMinDist, g_nAWMax );
        dabs = fabs( d1 / 1852. );
        ll_gc_ll( pAnchorWatchPoint->m_lat, pAnchorWatchPoint->m_lon, 0, dabs, &tlat1, &tlon1 );
        GetCanvasPointPix( tlat1, tlon1, &r1 );
        GetCanvasPointPix( pAnchorWatchPoint->m_lat, pAnchorWatchPoint->m_lon, &lAnchorPoint );
        lpp = sqrt(
                  pow( (double) ( lAnchorPoint.x - r1.x ), 2 )
                  + pow( (double) ( lAnchorPoint.y - r1.y ), 2 ) );

        //    This is an entry watch
        if( d1 < 0 ) lpp = -lpp;
    }
    return lpp;
}

//------------------------------------------------------------------------------------------
//    Tides and Current Chart Canvas Interface
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//    Tides Support
//------------------------------------------------------------------------------------------

void ChartCanvas::DrawAllTidesInBBox( ocpnDC& dc, LLBBox& BBox, bool bRebuildSelList,
                                      bool bforce_redraw_tides, bool bdraw_mono_for_mask )
{
    wxPen *pblack_pen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T ( "UINFD" ) ), 1,
                        wxSOLID );
    wxPen *pyelo_pen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T ( "YELO1" ) ), 1,
                       wxSOLID );
    wxPen *pblue_pen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T ( "BLUE2" ) ), 1,
                       wxSOLID );

    wxBrush *pgreen_brush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T ( "GREEN1" ) ),
                            wxSOLID );
//        wxBrush *pblack_brush = wxTheBrushList->FindOrCreateBrush ( GetGlobalColor ( _T ( "UINFD" ) ), wxSOLID );
    wxBrush *brc_1 = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T ( "BLUE2" ) ), wxSOLID );
    wxBrush *brc_2 = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T ( "YELO1" ) ), wxSOLID );

    wxFont *dFont = pFontMgr->GetFont( _("ExtendedTideIcon"), 12 );
    dc.SetTextForeground( pFontMgr->GetFontColor( _T("ExtendedTideIcon") ) );
    int font_size = wxMax(8, dFont->GetPointSize());
    wxFont *plabelFont = wxTheFontList->FindOrCreateFont( font_size, dFont->GetFamily(),
                         dFont->GetStyle(), dFont->GetWeight() );

    if( bdraw_mono_for_mask ) {
#ifdef __WXX11__
        const wxPen *pmono_pen = wxBLACK_PEN;
        const wxBrush *pmono_brush = wxBLACK_BRUSH;
#else
        const wxPen *pmono_pen = wxWHITE_PEN;
        const wxBrush *pmono_brush = wxWHITE_BRUSH;
#endif

        pblack_pen = (wxPen *) pmono_pen;
        pgreen_brush = (wxBrush *) pmono_brush;
        brc_1 = (wxBrush *) pmono_brush;
        brc_2 = (wxBrush *) pmono_brush;

    }

    dc.SetPen( *pblack_pen );
    dc.SetBrush( *pgreen_brush );

    if( bRebuildSelList ) pSelectTC->DeleteAllSelectableTypePoints( SELTYPE_TIDEPOINT );

    wxBitmap bm;
    switch( m_cs ) {
    case GLOBAL_COLOR_SCHEME_DAY:
        bm = m_bmTideDay;
        break;
    case GLOBAL_COLOR_SCHEME_DUSK:
        bm = m_bmTideDusk;
        break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
        bm = m_bmTideNight;
        break;
    default:
        bm = m_bmTideDay;
        break;
    }

    int bmw = bm.GetWidth();
    int bmh = bm.GetHeight();

    wxDateTime this_now = wxDateTime::Now();
    time_t t_this_now = this_now.GetTicks();

//      if(1/*BBox.GetValid()*/)
    {

        double lon_last = 0.;
        double lat_last = 0.;
        for( int i = 1; i < ptcmgr->Get_max_IDX() + 1; i++ ) {
            IDX_entry *pIDX = ptcmgr->GetIDX_entry( i );

            char type = pIDX->IDX_type;             // Entry "TCtcIUu" identifier
            if( ( type == 't' ) || ( type == 'T' ) )  // only Tides
            {
                double lon = pIDX->IDX_lon;
                double lat = pIDX->IDX_lat;
                bool b_inbox = false;
                double nlon;

                if( BBox.PointInBox( lon, lat, 0 ) ) {
                    nlon = lon;
                    b_inbox = true;
                } else if( BBox.PointInBox( lon + 360., lat, 0 ) ) {
                    nlon = lon + 360.;
                    b_inbox = true;
                } else if( BBox.PointInBox( lon - 360., lat, 0 ) ) {
                    nlon = lon - 360.;
                    b_inbox = true;
                }

//try to eliminate double entry , but the only good way is to clean the file!
                if( b_inbox && ( lat != lat_last ) && ( lon != lon_last ) ) {

//    Manage the point selection list
                    if( bRebuildSelList ) pSelectTC->AddSelectablePoint( lat, lon, pIDX,
                                SELTYPE_TIDEPOINT );

                    wxPoint r;
                    GetCanvasPointPix( lat, nlon, &r );
//draw standard icons
                    if( GetVP().chart_scale > 500000 ) {

                        if( bdraw_mono_for_mask ) dc.DrawRectangle( r.x - bmw / 2, r.y - bmh / 2,
                                    bmw, bmh );
                        else
                            dc.DrawBitmap( bm, r.x - bmw / 2, r.y - bmh / 2, true );
                    }
//draw "extended" icons
                    else {
//set rectangle size and position (max text lengh)
                        int wx, hx;
                        dc.SetFont( *plabelFont );
                        dc.GetTextExtent( _T("99.9ft "), &wx, &hx );
                        int w = r.x - 6;
                        int h = r.y - 22;
//draw mask
                        if( bdraw_mono_for_mask ) dc.DrawRectangle( r.x - ( wx / 2 ), h, wx,
                                    hx + 45 );
                        //process tides
                        else {
                            if( bforce_redraw_tides ) {
                                float val, nowlev;
                                float ltleve = 0.;
                                float htleve = 0.;
                                time_t tctime;
                                time_t lttime = 0;
                                time_t httime = 0;
                                bool wt;
                                //define if flood or edd in the last ten minutes and verify if data are useable
                                if( ptcmgr->GetTideFlowSens( t_this_now, BACKWARD_TEN_MINUTES_STEP,
                                                             pIDX->IDX_rec_num, nowlev, val, wt ) ) {

                                    //search forward the first HW or LW near "now" ( starting at "now" - ten minutes )
                                    ptcmgr->GetHightOrLowTide(
                                        t_this_now + BACKWARD_TEN_MINUTES_STEP,
                                        FORWARD_TEN_MINUTES_STEP, FORWARD_ONE_MINUTES_STEP, val,
                                        wt, pIDX->IDX_rec_num, val, tctime );
                                    if( wt ) {
                                        httime = tctime;
                                        htleve = val;
                                    } else {
                                        lttime = tctime;
                                        ltleve = val;
                                    }
                                    wt = !wt;

                                    //then search opposite tide near "now"
                                    if( tctime > t_this_now )          // search backward
                                        ptcmgr->GetHightOrLowTide( t_this_now,
                                                                   BACKWARD_TEN_MINUTES_STEP, BACKWARD_ONE_MINUTES_STEP,
                                                                   nowlev, wt, pIDX->IDX_rec_num, val, tctime );
                                    else
                                        // or search forward
                                        ptcmgr->GetHightOrLowTide( t_this_now,
                                                                   FORWARD_TEN_MINUTES_STEP, FORWARD_ONE_MINUTES_STEP,
                                                                   nowlev, wt, pIDX->IDX_rec_num, val, tctime );
                                    if( wt ) {
                                        httime = tctime;
                                        htleve = val;
                                    } else {
                                        lttime = tctime;
                                        ltleve = val;
                                    }

                                    //process tide state  ( %height and flow sens )
                                    float ts = 1 - ( ( nowlev - ltleve ) / ( htleve - ltleve ) );
                                    int hs = ( httime > lttime ) ? -5 : 5;
                                    if( ts > 0.995 || ts < 0.005 ) hs = 0;
                                    int ht_y = (int) ( 45.0 * ts );

                                    //draw yellow rectangle as total amplitude (width = 12 , height = 45 )
                                    dc.SetPen( *pblack_pen );
                                    dc.SetBrush( *brc_2 );
                                    dc.DrawRectangle( w, h, 12, 45 );
                                    //draw blue rectangle as water height
                                    dc.SetPen( *pblue_pen );
                                    dc.SetBrush( *brc_1 );
                                    dc.DrawRectangle( w + 2, h + ht_y, 8, 45 - ht_y );

                                    //draw sens arrows (ensure they are not "under-drawn" by top line of blue rectangle )

                                    int hl;
                                    wxPoint arrow[3];
                                    arrow[0].x = w + 1;
                                    arrow[1].x = w + 5;
                                    arrow[2].x = w + 11;
                                    if( ts > 0.35 || ts < 0.15 )      // one arrow at 3/4 hight tide
                                    {
                                        hl = (int) ( 45.0 * 0.25 ) + h;
                                        arrow[0].y = hl;
                                        arrow[1].y = hl + hs;
                                        arrow[2].y = hl;
                                        if( ts < 0.15 ) dc.SetPen( *pyelo_pen );
                                        else
                                            dc.SetPen( *pblue_pen );

                                        dc.DrawLines( 3, arrow );
                                    }
                                    if( ts > 0.60 || ts < 0.40 )       //one arrow at 1/2 hight tide
                                    {
                                        hl = (int) ( 45.0 * 0.5 ) + h;
                                        arrow[0].y = hl;
                                        arrow[1].y = hl + hs;
                                        arrow[2].y = hl;
                                        if( ts < 0.40 ) dc.SetPen( *pyelo_pen );
                                        else
                                            dc.SetPen( *pblue_pen );
                                        dc.DrawLines( 3, arrow );
                                    }
                                    if( ts < 0.65 || ts > 0.85 )       //one arrow at 1/4 Hight tide
                                    {
                                        hl = (int) ( 45.0 * 0.75 ) + h;
                                        arrow[0].y = hl;
                                        arrow[1].y = hl + hs;
                                        arrow[2].y = hl;
                                        if( ts < 0.65 ) dc.SetPen( *pyelo_pen );
                                        else
                                            dc.SetPen( *pblue_pen );
                                        dc.DrawLines( 3, arrow );
                                    }
                                    //draw tide level text
                                    wxString s;
                                    s.Printf( _T("%3.1f"), nowlev );
                                    Station_Data *pmsd = pIDX->pref_sta_data;           //write unit
                                    if( pmsd ) s.Append(
                                            wxString( pmsd->units_abbrv, wxConvUTF8 ) );
                                    int wx1;
                                    dc.GetTextExtent( s, &wx1, NULL );
                                    dc.DrawText( s, r.x - ( wx1 / 2 ), h + 45 );
                                }
                            }
                        }
                    }
                }
                lon_last = lon;
                lat_last = lat;
            }
        }
    }
}

//------------------------------------------------------------------------------------------
//    Currents Support
//------------------------------------------------------------------------------------------

void ChartCanvas::DrawAllCurrentsInBBox( ocpnDC& dc, LLBBox& BBox, bool bRebuildSelList,
        bool bforce_redraw_currents, bool bdraw_mono_for_mask )
{
    float tcvalue, dir;
    bool bnew_val;
    char sbuf[20];
    wxFont *pTCFont;
    double lon_last = 0.;
    double lat_last = 0.;

    wxPen *pblack_pen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T ( "UINFD" ) ), 1,
                        wxSOLID );
    wxPen *porange_pen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T ( "UINFO" ) ), 1,
                         wxSOLID );
    wxBrush *porange_brush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T ( "UINFO" ) ),
                             wxSOLID );
    wxBrush *pgray_brush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T ( "UIBDR" ) ),
                           wxSOLID );
    wxBrush *pblack_brush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T ( "UINFD" ) ),
                            wxSOLID );

    double skew_angle = GetVPRotation();

    if( !g_bCourseUp && !g_bskew_comp ) skew_angle = GetVPRotation() + GetVPSkew();

    if( bdraw_mono_for_mask ) {
#ifdef __WXX11__
        const wxPen *pmono_pen = wxBLACK_PEN;
        const wxBrush *pmono_brush = wxBLACK_BRUSH;
#else
        const wxPen *pmono_pen = wxWHITE_PEN;
        const wxBrush *pmono_brush = wxWHITE_BRUSH;
#endif

        pblack_pen = (wxPen *) pmono_pen;
        porange_pen = (wxPen *) pmono_pen;
        porange_brush = (wxBrush *) pmono_brush;
        pgray_brush = (wxBrush *) pmono_brush;
    }

    pTCFont = wxTheFontList->FindOrCreateFont( 12, wxDEFAULT, wxNORMAL, wxBOLD, FALSE,
              wxString( _T ( "Eurostile Extended" ) ) );
    int now = time( NULL );

    if( bRebuildSelList ) pSelectTC->DeleteAllSelectableTypePoints( SELTYPE_CURRENTPOINT );

//     if(1/*BBox.GetValid()*/)
    {

        for( int i = 1; i < ptcmgr->Get_max_IDX() + 1; i++ ) {
            IDX_entry *pIDX = ptcmgr->GetIDX_entry( i );
            double lon = pIDX->IDX_lon;
            double lat = pIDX->IDX_lat;

            char type = pIDX->IDX_type;             // Entry "TCtcIUu" identifier
            if( ( ( type == 'c' ) || ( type == 'C' ) ) && ( 1/*pIDX->IDX_Useable*/) ) {

//  TODO This is a ---HACK---
//  try to avoid double current arrows.  Select the first in the list only
//  Proper fix is to correct the TCDATA index file for depth indication
                bool b_dup = false;
                if( ( type == 'c' ) && ( lat == lat_last ) && ( lon == lon_last ) ) b_dup = true;

                if( !b_dup && ( BBox.PointInBox( lon, lat, 0 ) ) ) {

//    Manage the point selection list
                    if( bRebuildSelList ) pSelectTC->AddSelectablePoint( lat, lon, pIDX,
                                SELTYPE_CURRENTPOINT );

                    wxPoint r;
                    GetCanvasPointPix( lat, lon, &r );

                    wxPoint d[4];
                    int dd = 6;
                    d[0].x = r.x;
                    d[0].y = r.y + dd;
                    d[1].x = r.x + dd;
                    d[1].y = r.y;
                    d[2].x = r.x;
                    d[2].y = r.y - dd;
                    d[3].x = r.x - dd;
                    d[3].y = r.y;

                    if( ptcmgr->GetTideOrCurrent15( now, i, tcvalue, dir, bnew_val ) ) {
                        porange_pen->SetWidth( 1 );
                        dc.SetPen( *pblack_pen );
                        dc.SetBrush( *porange_brush );
                        dc.DrawPolygon( 4, d );

                        if( type == 'C' ) {
                            dc.SetBrush( *pblack_brush );
                            dc.DrawCircle( r.x, r.y, 2 );
                        }

                        else if( ( type == 'c' ) && ( GetVP().chart_scale < 1000000 ) ) {
                            if( bnew_val || bforce_redraw_currents ) {

//    Get the display pixel location of the current station
                                int pixxc, pixyc;
                                wxPoint cpoint;
                                GetCanvasPointPix( lat, lon, &cpoint );
                                pixxc = cpoint.x;
                                pixyc = cpoint.y;

//    Draw arrow using preset parameters, see mm_per_knot variable
//                                                            double scale = fabs ( tcvalue ) * current_draw_scaler;
//    Adjust drawing size using logarithmic scale
                                double a1 = fabs( tcvalue ) * 10.;
                                a1 = wxMax(1.0, a1);      // Current values less than 0.1 knot
                                // will be displayed as 0
                                double a2 = log10( a1 );

                                double scale = current_draw_scaler * a2;

                                porange_pen->SetWidth( 2 );
                                dc.SetPen( *porange_pen );
                                DrawArrow( dc, pixxc, pixyc,
                                           dir - 90 + ( skew_angle * 180. / PI ), scale / 100 );
// Draw text, if enabled

                                if( bDrawCurrentValues ) {
                                    dc.SetFont( *pTCFont );
                                    snprintf( sbuf, 19, "%3.1f", fabs( tcvalue ) );
                                    dc.DrawText( wxString( sbuf, wxConvUTF8 ), pixxc, pixyc );
                                }
                            }
                        }           // scale
                    }
                    /*          This is useful for debugging the TC database
                     else
                     {
                     dc.SetPen ( *porange_pen );
                     dc.SetBrush ( *pgray_brush );
                     dc.DrawPolygon ( 4, d );
                     }
                     */

                }
                lon_last = lon;
                lat_last = lat;

            }
        }
    }
}

void ChartCanvas::DrawTCWindow( int x, int y, void *pvIDX )
{
    pCwin = new TCWin( this, x, y, pvIDX );

}

#define NUM_CURRENT_ARROW_POINTS 9
static wxPoint CurrentArrowArray[NUM_CURRENT_ARROW_POINTS] = { wxPoint( 0, 0 ), wxPoint( 0, -10 ),
        wxPoint( 55, -10 ), wxPoint( 55, -25 ), wxPoint( 100, 0 ), wxPoint( 55, 25 ), wxPoint( 55,
                10 ), wxPoint( 0, 10 ), wxPoint( 0, 0 )
                                                             };

void ChartCanvas::DrawArrow( ocpnDC& dc, int x, int y, double rot_angle, double scale )
{
    if( scale > 1e-2 ) {

        float sin_rot = sin( rot_angle * PI / 180. );
        float cos_rot = cos( rot_angle * PI / 180. );

        // Move to the first point

        float xt = CurrentArrowArray[0].x;
        float yt = CurrentArrowArray[0].y;

        float xp = ( xt * cos_rot ) - ( yt * sin_rot );
        float yp = ( xt * sin_rot ) + ( yt * cos_rot );
        int x1 = (int) ( xp * scale );
        int y1 = (int) ( yp * scale );

        // Walk thru the point list
        for( int ip = 1; ip < NUM_CURRENT_ARROW_POINTS; ip++ ) {
            xt = CurrentArrowArray[ip].x;
            yt = CurrentArrowArray[ip].y;

            float xp = ( xt * cos_rot ) - ( yt * sin_rot );
            float yp = ( xt * sin_rot ) + ( yt * cos_rot );
            int x2 = (int) ( xp * scale );
            int y2 = (int) ( yp * scale );

            dc.DrawLine( x1 + x, y1 + y, x2 + x, y2 + y );

            x1 = x2;
            y1 = y2;
        }
    }
}

//----------------------------------------------------------------------------
//    Texture descriptor
//----------------------------------------------------------------------------

class glTextureDescriptor {
public:
    glTextureDescriptor();
    ~glTextureDescriptor();

    GLuint tex_name;
    int tex_mult;
    int level_min;
    int level_max;
    int base_size;
    int GPU_base;

    unsigned char *map_array[10];
};

glTextureDescriptor::glTextureDescriptor()
{
    for( int i = 0; i < 10; i++ )
        map_array[i] = NULL;

    tex_mult = -1;
    level_min = -1;
    level_max = -1;
    base_size = -1;
    tex_name = 0;
    GPU_base = -1;
}

glTextureDescriptor::~glTextureDescriptor()
{
    for( int i = 0; i < 10; i++ )
        free( map_array[i] );

}

#ifdef __WXMSW__
#define FORMAT_INTERNAL       GL_RGB
#define FORMAT_BITS           GL_BGR
#else
#define FORMAT_INTERNAL       GL_COMPRESSED_RGB
#define FORMAT_BITS           GL_RGB
#endif

//------------------------------------------------------------------------------
//    glChartCanvas Implementation
//------------------------------------------------------------------------------
#include <wx/arrimpl.cpp>

#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#endif

WX_DEFINE_OBJARRAY( ArrayOfTexDescriptors );

//         int attribs[]={WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 24, WX_GL_STENCIL_SIZE, 8, 0};

// This attribute set works OK with vesa software only OpenGL renderer
int attribs[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, WX_GL_STENCIL_SIZE, 8, 0 };
BEGIN_EVENT_TABLE ( glChartCanvas, wxGLCanvas ) EVT_PAINT ( glChartCanvas::OnPaint )
    EVT_ACTIVATE ( glChartCanvas::OnActivate )
    EVT_SIZE ( glChartCanvas::OnSize )
    EVT_MOUSE_EVENTS ( glChartCanvas::MouseEvent )
    EVT_ERASE_BACKGROUND(glChartCanvas::OnEraseBG)
END_EVENT_TABLE()

glChartCanvas::glChartCanvas( wxWindow *parent ) :
    wxGLCanvas( parent, wxID_ANY, wxDefaultPosition, wxSize( 256, 256 ),
                wxFULL_REPAINT_ON_RESIZE | wxBG_STYLE_CUSTOM, _T(""), attribs ), m_cacheinvalid(
                    1 ), m_data( NULL ), m_datasize( 0 ), m_bsetup( false )
{
    m_ntex = 0;
}

glChartCanvas::~glChartCanvas()
{
    free( m_data );

    ClearAllRasterTextures();
}

void glChartCanvas::OnEraseBG( wxEraseEvent& evt )
{
}

void glChartCanvas::ClearAllRasterTextures( void )
{
    //     Clear and delete all the GPU textures presently loaded
    ChartPointerHashType::iterator it;
    for( it = m_chart_hash.begin(); it != m_chart_hash.end(); ++it ) {
        ChartBase *pc = (ChartBase *) it->first;

        ChartTextureHashType *pTextureHash = m_chart_hash[pc];

        // iterate over all the textures presently loaded
        // and delete the OpenGL texture and the private descriptor

        ChartTextureHashType::iterator it;
        for( it = pTextureHash->begin(); it != pTextureHash->end(); ++it ) {
            glTextureDescriptor *ptd = it->second;
            glDeleteTextures( 1, &ptd->tex_name );
            delete ptd;
        }

        pTextureHash->clear();
        delete pTextureHash;
    }
    m_chart_hash.clear();

}

void glChartCanvas::OnActivate( wxActivateEvent& event )
{
    cc1->OnActivate( event );
}

void glChartCanvas::OnSize( wxSizeEvent& event )
{
    if( !g_bopengl ) {
        SetSize( cc1->GetVP().pix_width, cc1->GetVP().pix_height );
        event.Skip();
        return;
    }

    // this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize( event );

    /* expand opengl widget to fill viewport */
    ViewPort &VP = cc1->GetVP();
    if( GetSize().x != VP.pix_width || GetSize().y != VP.pix_height ) {
        SetSize( VP.pix_width, VP.pix_height );
        if( m_bsetup && m_b_useFBO ) {
            BuildFBO();
            ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, 0 );
        }

    }

}

void glChartCanvas::MouseEvent( wxMouseEvent& event )
{
    cc1->MouseEvent( event );
}

void glChartCanvas::BuildFBO( void )
{
    if( m_bsetup && m_b_useFBO ) {
        glDeleteTextures( 1, &m_cache_tex );
        ( *s_glDeleteFramebuffersEXT )( 1, &m_fb0 );
        ( *s_glDeleteRenderbuffersEXT )( 1, &m_depth_rb );
    }

    if( m_b_useFBO ) {
        m_cache_tex_x = GetSize().x;
        m_cache_tex_y = GetSize().y;

        ( *s_glGenFramebuffersEXT )( 1, &m_fb0 );
        glGenTextures( 1, &m_cache_tex );
        ( *s_glGenRenderbuffersEXT )( 1, &m_depth_rb );

        ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, m_fb0 );

// initialize color texture

        glBindTexture( m_TEX_TYPE, m_cache_tex );
        glTexParameterf( m_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( m_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexImage2D( m_TEX_TYPE, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, NULL );
        ( *s_glFramebufferTexture2DEXT )( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, m_TEX_TYPE,
                                          m_cache_tex, 0 );

        if( m_b_useFBOStencil ) {
            // initialize composite depth/stencil renderbuffer
            ( *s_glBindRenderbufferEXT )( GL_RENDERBUFFER_EXT, m_depth_rb );
            ( *s_glRenderbufferStorageEXT )( GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT,
                                             m_cache_tex_x, m_cache_tex_y );

            // Can we attach to depth and stencil at once?  Maybe
            // it would be easier to not check for this extension and
            // always use 2 calls.
            if( QueryExtension( "GL_ARB_framebuffer_object" ) ) {
                ( *s_glFramebufferRenderbufferEXT )( GL_FRAMEBUFFER_EXT,
                                                     GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, m_depth_rb );
            } else {
                ( *s_glFramebufferRenderbufferEXT )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                                     GL_RENDERBUFFER_EXT, m_depth_rb );

                ( *s_glFramebufferRenderbufferEXT )( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                                     GL_RENDERBUFFER_EXT, m_depth_rb );
            }

        } else {
            // initialize depth renderbuffer
            ( *s_glBindRenderbufferEXT )( GL_RENDERBUFFER_EXT, m_depth_rb );
            ( *s_glRenderbufferStorageEXT )( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
                                             m_cache_tex_x, m_cache_tex_y );
            ( *s_glFramebufferRenderbufferEXT )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                                 GL_RENDERBUFFER_EXT, m_depth_rb );
        }

        ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, 0 );
    }

}

int s_in_glpaint;

void glChartCanvas::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc( this );

#if wxCHECK_VERSION(2, 9, 0)
    SetCurrent(*m_pcontext);
#else
    SetCurrent();
#endif

    Show( g_bopengl );
    if( !g_bopengl ) {
        event.Skip();
        return;
    }

    if( !m_bsetup ) {

        char render_string[80];
        strncpy( render_string, (char *) glGetString( GL_RENDERER ), 79 );
        m_renderer = wxString( render_string, wxConvUTF8 );

        wxString msg;
        msg.Printf( _T("OpenGL-> Renderer String: ") );
        msg += m_renderer;
        wxLogMessage( msg );

        if( ps52plib ) ps52plib->SetGLRendererString( m_renderer );

        //  This little hack fixes a problem seen with some Intel 945 graphics chips
        //  We need to not do anything that requires (some) complicated stencil operations.

        bool bad_stencil_code = false;
        if( GetRendererString().Find( _T("Intel") ) != wxNOT_FOUND ) {
            wxLogMessage( _T("OpenGL-> Detected Intel renderer, disabling stencil buffer") );
            bad_stencil_code = true;
        }

        //      And for the lousy Unichrome drivers, too
        if( GetRendererString().Find( _T("UniChrome") ) != wxNOT_FOUND ) {
            bad_stencil_code = true;
        }

        //      Stencil buffer test
        glEnable( GL_STENCIL_TEST );
        GLboolean stencil = glIsEnabled( GL_STENCIL_TEST );
        int sb;
        glGetIntegerv( GL_STENCIL_BITS, &sb );
        //        printf("Stencil Buffer Available: %d\nStencil bits: %d\n", stencil, sb);
        glDisable( GL_STENCIL_TEST );

        g_b_useStencil = false;
        if( !bad_stencil_code && stencil && ( sb == 8 ) ) g_b_useStencil = true;

//          GLenum err = glewInit();
//           if (GLEW_OK != err)

        m_b_useFBO = false;              // default is false

        //      We require certain extensions to support FBO rendering
        if( QueryExtension( "GL_ARB_texture_rectangle" )
                && QueryExtension( "GL_EXT_framebuffer_object" ) ) {
            m_TEX_TYPE = GL_TEXTURE_RECTANGLE_ARB;
            m_b_useFBO = true;
        }

        if( GetRendererString().Find( _T("Intel") ) != wxNOT_FOUND ) {
            wxLogMessage( _T("OpenGL-> Detected Intel renderer, disabling FBO") );
            m_b_useFBO = false;
        }

        if( !GetglEntryPoints() ) m_b_useFBO = false;              // default is false

        //      Can we use the stencil buffer in a FBO?
        if( QueryExtension( "GL_EXT_packed_depth_stencil" ) ) m_b_useFBOStencil = true;
        else
            m_b_useFBOStencil = false;

        //      Maybe build FBO(s)
        if( m_b_useFBO ) {
            BuildFBO();
// Check framebuffer completeness at the end of initialization.
            ( s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, m_fb0 );
            GLenum fb_status = ( *s_glCheckFramebufferStatusEXT )( GL_FRAMEBUFFER_EXT );
            ( s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, 0 );

            if( fb_status != GL_FRAMEBUFFER_COMPLETE_EXT ) {
                wxString msg;
                msg.Printf( _T("    OpenGL-> Framebuffer Incomplete:  %08X"), fb_status );
                wxLogMessage( msg );
                m_b_useFBO = false;
            }
        }

        if( m_b_useFBO && !m_b_useFBOStencil ) g_b_useStencil = false;

        if( m_b_useFBO ) {
            wxLogMessage( _T("OpenGL-> Using Framebuffer Objects") );

            if( m_b_useFBOStencil ) wxLogMessage( _T("OpenGL-> Using FBO Stencil buffer") );
            else
                wxLogMessage( _T("OpenGL-> FBO Stencil buffer unavailable") );
        } else
            wxLogMessage( _T("OpenGL-> Framebuffer Objects unavailable") );

        if( g_b_useStencil ) wxLogMessage( _T("OpenGL-> Using Stencil buffer clipping") );
        else
            wxLogMessage( _T("OpenGL-> Using Depth buffer clipping") );

        /* we upload non-aligned memory */
        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

        int n_tex_size = 512;
        double n_GPU_Mem = wxMax(64, g_GPU_MemSize) * ( 1 << 20 );
        m_tex_max_res = n_GPU_Mem / ( n_tex_size * n_tex_size * 4 * 1.3 ); // 1.3 multiplier allows for full mipmaps
        m_tex_max_res /= 2;
        m_tex_max_res_initial = m_tex_max_res;

        wxString str;
        str.Printf( _T("OpenGL-> Estimated Max Resident Textures: %d"), m_tex_max_res );
        wxLogMessage( str );

        m_bsetup = true;
//          g_bDebugOGL = true;
    }

    //      Recursion test, sometimes seen on GTK systems when wxBusyCursor is activated
    if( s_in_glpaint ) return;
    s_in_glpaint++;

    render();

    s_in_glpaint--;

}

bool glChartCanvas::PurgeChartTextures( ChartBase *pc )
{
    //    Look for the chart texture hashmap in the member chart hashmap
    ChartPointerHashType::iterator it0 = m_chart_hash.find( pc );

    ChartTextureHashType *pTextureHash;

    //    Found ?
    if( it0 != m_chart_hash.end() ) {
        pTextureHash = m_chart_hash[pc];

        // iterate over all the textures presently loaded
        // and delete the OpenGL texture and the private descriptor

        ChartTextureHashType::iterator it;
        for( it = pTextureHash->begin(); it != pTextureHash->end(); ++it ) {
            glTextureDescriptor *ptd = it->second;

            if( ptd->tex_name > 0 ) {
                if( g_bDebugOGL ) printf( "glDeleteTextures in Purge...()\n" );
                glDeleteTextures( 1, &ptd->tex_name );
                m_ntex--;
            }

            delete ptd;
        }

        pTextureHash->clear();

        m_chart_hash.erase( it0 );            // erase the texture hash map for this chart

        delete pTextureHash;

        return true;
    } else
        return false;
}

void HalfScaleChartBits( int width, int height, unsigned char *source, unsigned char *target )
{
    int newwidth = width / 2;
    int newheight = height / 2;
    int stride = width * 3;

    unsigned char *s = target;
    unsigned char *t = source;
    // Average 4 pixels
    for( int i = 0; i < newheight; i++ ) {
        for( int j = 0; j < newwidth; j++ ) {

            for( int k = 0; k < 3; k++ ) {
                s[0] = ( *t + *( t + 3 ) + *( t + stride ) + *( t + stride + 3 ) ) / 4;
                s++;
                t += 1;
            }
            t += 3;
        }
        t += stride;
    }
}

bool UploadTexture( glTextureDescriptor *ptd, int n_basemult )
{
    if( g_bDebugOGL ) {
        wxString msg;
        msg.Printf( _T("  -->UploadTexture %d"), ptd->tex_name );
        wxLogMessage( msg );
    }

    glBindTexture( GL_TEXTURE_2D, ptd->tex_name );

    //    Calculate the effective base level
    int base_level = 0;
    switch( n_basemult ) {
    case 1:
        base_level = 0;
        break;
    case 2:
        base_level = 1;
        break;
    case 4:
        base_level = 2;
        break;
    case 8:
        base_level = 3;
        break;
    default:
        base_level = 0;
        break;
    }

    int width = ptd->base_size;
    int height = ptd->base_size;

    int level = 0;
    while( level < ptd->level_max ) {
        if( height == 2 ) break;                  // all done;

        if( g_bDebugOGL ) {
            wxString msg;
            msg.Printf( _T("     -->glTexImage2D...level:%d"), level );
            wxLogMessage( msg );
        }

        //    Upload to GPU?
        if( level >= base_level ) glTexImage2D( GL_TEXTURE_2D, level - base_level, FORMAT_INTERNAL,
                                                    width, height, 0, FORMAT_BITS, GL_UNSIGNED_BYTE, ptd->map_array[level] );

        width /= 2;
        height /= 2;

        level++;
    }

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0 );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, level - base_level - 1 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level - base_level - 1 );

    ptd->GPU_base = n_basemult;

    return true;
}

void OCPNPopulateTD( glTextureDescriptor *ptd, int n_basemult, wxRect &rect, ChartBase *pchart )
{
    if( !pchart ) return;

    ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( pchart );
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pchart );

    if( !pPlugInWrapper && !pBSBChart ) return;

    bool b_plugin = false;
    if( pPlugInWrapper ) b_plugin = true;

    //    We do not need all possible mipmaps, since we can only zoom out so far....
    //    So, save some memory by limiting GL_TEXTURE_MAX_LEVEL

    int n_level_max = 3;
    //    Calculate the effective base level
    int base_level = 0;
    switch( n_basemult ) {
    case 1:
        base_level = 0;
        break;
    case 2:
        base_level = 1;
        break;
    case 4:
        base_level = 2;
        break;
    case 8:
        base_level = 3;
        break;
    default:
        base_level = 0;
        break;
    }

    //    Adjust the chart source rectangle to account for base multiplier
    wxRect rbits = rect;
    rbits.x *= n_basemult;
    rbits.y *= n_basemult;
    rbits.width *= n_basemult;
    rbits.height *= n_basemult;

    //    Get a buffer
    unsigned char *t_buf = (unsigned char *) malloc( rbits.width * rbits.height * 3 );

    //    Prime the pump with the "zero" level bits, ie. 1x native chart bits
    if( b_plugin ) pPlugInWrapper->GetChartBits( rbits, t_buf, 1 );
    else
        pBSBChart->GetChartBits( rbits, t_buf, 1 );

    //    and cache them here
    ptd->map_array[0] = t_buf;

    int last_height = rbits.height;
    int last_width = rbits.width;

    unsigned char *source;
    unsigned char *dest;

    int level_index = 1;
    while( level_index < n_level_max ) {
        if( last_height == 2 ) break;                  // all done;

        int newwidth = last_width / 2;
        int newheight = last_height / 2;

        dest = (unsigned char *) malloc( newwidth * newheight * 3 );
        source = ptd->map_array[level_index - 1];

        HalfScaleChartBits( last_width, last_height, source, dest );
        ptd->map_array[level_index] = dest;

        last_width = newwidth;
        last_height = newheight;

        level_index++;
    }

    ptd->level_min = 0;
    ptd->level_max = level_index - 1;
    ptd->base_size = rbits.width;
}

int s_nquickbind;

void glChartCanvas::RenderRasterChartRegionGL( ChartBase *chart, ViewPort &vp, wxRegion &region )
{
    if( !chart ) return;

    ChartPlugInWrapper *pPlugInWrapper = dynamic_cast<ChartPlugInWrapper*>( chart );
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( chart );

    if( !pPlugInWrapper && !pBSBChart ) return;

    bool b_plugin = false;
    if( pPlugInWrapper ) b_plugin = true;

    int n_longbind = 0;

    /* setup texture parameters */
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    //  Make a special VP to account for rotations
    if( vp.b_MercatorProjectionOverride ) vp.SetProjectionType( PROJECTION_MERCATOR );
    ViewPort svp = vp;

    svp.pix_width = svp.rv_rect.width;
    svp.pix_height = svp.rv_rect.height;

    wxRect R;
    double scalefactor;
    int size_X, size_Y;
    if( b_plugin ) {
        pPlugInWrapper->ComputeSourceRectangle( svp, &R );
        scalefactor = pPlugInWrapper->GetRasterScaleFactor();
        size_X = pPlugInWrapper->GetSize_X();
        size_Y = pPlugInWrapper->GetSize_Y();
    } else {
        pBSBChart->ComputeSourceRectangle( svp, &R );
        scalefactor = pBSBChart->GetRasterScaleFactor();
        size_X = pBSBChart->GetSize_X();
        size_Y = pBSBChart->GetSize_Y();
    }

    int tex_dim = 512;                  //max_texture_dimension;
    GrowData( 3 * tex_dim * tex_dim );

    SetClipRegion( vp, region, false );         // no need to clear

    //    Look for the chart texture hashmap in the member chart hashmap
    ChartPointerHashType::iterator it = m_chart_hash.find( chart );

    ChartTextureHashType *pTextureHash;

    //    Not Found ?
    if( it == m_chart_hash.end() ) {
        ChartTextureHashType *p = new ChartTextureHashType;
        m_chart_hash[chart] = p;
    }

    pTextureHash = m_chart_hash[chart];

    //    For underzoom cases, we will define the textures as having their base levels
    //    equivalent to a level "n" mipmap, where n is calculated, and is always binary

    int n_basemult = 1;
    if( scalefactor >= 2.0 ) {
        n_basemult = 2;
    }

    //    Iterate on the texture hashmap....
    //    Remove any textures whose tex_mult value does not match the target for this render (i.e. n_basemult)
    if( m_ntex > m_tex_max_res ) {
        ChartTextureHashType::iterator itt = pTextureHash->begin();
        while( itt != pTextureHash->end() ) {
            glTextureDescriptor *ptd = itt->second;

            if( ( ptd->tex_name > 0 ) && ( ptd->tex_mult != n_basemult ) ) // the texture known to the GPU does not match the target
            {
                if( g_bDebugOGL ) printf( "   glDeleteTexture on n_basemult mismatch\n" );
                glDeleteTextures( 1, &ptd->tex_name );
                m_ntex--;

                ptd->tex_name = 0;            // mark the ptd as unknown/unavailable to GPU

                //    Delete the chart data?
                if( m_b_mem_crunch ) {
                    pTextureHash->erase( itt );
                    delete ptd;
                    itt = pTextureHash->begin();              // reset the iterator
                }
            } else
                ++itt;
        }
    }

    //    Adjust the chart source rectangle by base multiplier
    R.x /= n_basemult;
    R.y /= n_basemult;
    R.width /= n_basemult;
    R.height /= n_basemult;

    //  Calculate the number of textures needed
    int nx_tex = ( ( size_X / n_basemult ) / tex_dim ) + 1;
    int ny_tex = ( ( size_Y / n_basemult ) / tex_dim ) + 1;

    glTextureDescriptor *ptd;
    wxRect rect( 0, 0, 1, 1 );

    //    For low memory systems, aggressively manage the textures in the GPU memory.
    //    Strategy:  Before loading any new textures,
    //               delete all textures from  the GPU that are not to be used
    //               in this particular VP render, on a per-chart basis.
    if( m_ntex > m_tex_max_res ) {
        rect.y = 0;
        for( int i = 0; i < ny_tex; i++ ) {
            rect.height = tex_dim;
            rect.x = 0;
            for( int j = 0; j < nx_tex; j++ ) {
                rect.width = tex_dim;

                wxASSERT(rect.x < 15383);
                wxASSERT(rect.y < 15383);

                int key = ( ( rect.x << 18 ) + ( rect.y << 4 ) ) + n_basemult;

                if( pTextureHash->find( key ) != pTextureHash->end() )  // found?
                {
                    //    Is this texture needed now?
                    wxRect ri = rect;
                    ri.Intersect( R );
                    if( !ri.width || !ri.height ) {
                        ptd = ( *pTextureHash )[key];
                        if( ptd->tex_name > 0 ) {
                            if( g_bDebugOGL ) printf( "   glDeleteTexture on m_ntex limit\n" );

                            glDeleteTextures( 1, &ptd->tex_name );
                            m_ntex--;

                            ptd->tex_name = 0;            // mark the ptd as not available to GPU

                            //    Delete the chart data?
                            if( m_b_mem_crunch ) {
                                pTextureHash->erase( key );
                                delete ptd;
                            }
                        }
                    }
                }
                rect.x += rect.width;
            }
            rect.y += rect.height;
        }
    }

    glPushMatrix();

    int spx = R.x + R.width / 2;
    int spy = R.y + R.height / 2;

    //    Calculate a sub-pixel bias for overzoom renders,
    //    anticipating and correcting for scaled up texture rendering
    double biasx = 0.;
    double biasy = 0.;
    if( scalefactor < 1.0 ) {
        double pixx, pixy;
        if( b_plugin ) pPlugInWrapper->latlong_to_chartpix( vp.clat, vp.clon, pixx, pixy );
        else
            pBSBChart->latlong_to_chartpix( vp.clat, vp.clon, pixx, pixy );

        biasy = pixy - spy;
        biasx = pixx - spx;
    }

    glScalef( 1. / scalefactor * n_basemult, 1. / scalefactor * n_basemult, 1 );

    double xt = 0.;
    double yt = 0.;

    if( ( ( fabs( vp.rotation ) > 0.01 ) ) || ( g_bskew_comp && ( fabs( vp.skew ) > 0.01 ) ) ) {

        //    Shift texture drawing positions to account for the larger chart rectangle
        //    needed to cover the screen on rotated images
        double w = vp.pix_width;
        double h = vp.pix_height;
        xt = ( R.width - ( w * scalefactor / n_basemult ) ) / 2;
        yt = ( R.height - ( h * scalefactor / n_basemult ) ) / 2;

        //    Rotations occur around 0,0, so calculate a post-rotate translation factor
        double angle = vp.rotation;
        angle -= vp.skew;
        double ddx = ( scalefactor / n_basemult ) * ( w * cos( -angle ) - h * sin( -angle ) - w )
                     / 2;
        double ddy = ( scalefactor / n_basemult ) * ( h * cos( -angle ) + w * sin( -angle ) - h )
                     / 2;

        glRotatef( angle * 180. / PI, 0, 0, 1 );

        glTranslatef( ddx, ddy, 0 );                 // post rotate translation
    }

#if 0
    Sum and Difference Formulas
    sin(A+B)=sin A cos B + cos A sin B
             sin(A-B)=sin A cos B - cos A sin B
                      cos(A+B)=cos A cos B - sin A sin B
                               cos(A-B)=cos A cos B + sin A sin B
#endif

                                        //    Using a 2D loop, iterate thru the texture tiles of the chart
                                        //    For each tile, is it (1) needed and (2) present?
                                        int n_chart_tex = 0;
    int key;

    rect.y = 0;
    for( int i = 0; i < ny_tex; i++ ) {
        rect.height = tex_dim;
        rect.x = 0;
        for( int j = 0; j < nx_tex; j++ ) {
            rect.width = tex_dim;

            //    Is this tile needed (i.e. does it intersect the chart source rectangle?)
            wxRect ri = rect;
            ri.Intersect( R );
            if( ri.width && ri.height ) {
                n_chart_tex++;

                // calculate the on-screen rectangle coordinates for this tile
                int w = ri.width, h = ri.height;
                int x1 = ri.x - rect.x;
                int y1 = ri.y - rect.y;
                double x2 = ( ri.x - R.x ) - xt;
                double y2 = ( ri.y - R.y ) - yt;

                y2 -= biasy;
                x2 -= biasx;

                wxRect rt( ( x2 ) / scalefactor * n_basemult, ( y2 ) / scalefactor * n_basemult,
                           w / scalefactor * n_basemult, h / scalefactor * n_basemult );
                rt.Offset( -vp.rv_rect.x, -vp.rv_rect.y ); // compensate for the adjustment made in quilt composition

                //    And does this tile intersect the desired render region?
                if( region.Contains( rt ) != wxOutRegion ) {
                    //    Is this texture tile already defined?

                    //    Create the hash key
                    wxASSERT(rect.x < 15383);
                    wxASSERT(rect.y < 15383);

                    key = ( ( rect.x << 18 ) + ( rect.y << 4 ) ) + n_basemult;
                    ChartTextureHashType::iterator it = pTextureHash->find( key );

                    // if not found in the hash map, then get the bits as a texture descriptor
                    if( it == pTextureHash->end() ) {

                        ptd = new glTextureDescriptor;
                        ptd->tex_mult = n_basemult;

//                                    printf("  -->PopulateTD\n");
                        OCPNPopulateTD( ptd, n_basemult, rect, chart );
                        ( *pTextureHash )[key] = ptd;
                    } else
                        ptd = ( *pTextureHash )[key];

                    //    If the GPU does not know about this texture, upload it
                    if( ptd->tex_name == 0 ) {
                        GLuint tex_name;
                        glGenTextures( 1, &tex_name );
                        ptd->tex_name = tex_name;

                        glBindTexture( GL_TEXTURE_2D, tex_name );

                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                         GL_LINEAR_MIPMAP_NEAREST );

                        UploadTexture( ptd, n_basemult );

                        m_ntex++;
                    }

                    //    The texture is known to be available to the GPU
                    //    So map it in

                    wxStopWatch sw;

                    if( g_bDebugOGL ) {
                        wxString msg;
                        msg.Printf( _T("  -->BindTexture %d"), ptd->tex_name );
                        wxLogMessage( msg );
                    }

                    glBindTexture( GL_TEXTURE_2D, ptd->tex_name );

                    GLint ltex;
                    glGetIntegerv( GL_TEXTURE_BINDING_2D, &ltex );
                    wxASSERT(ltex == (GLint)ptd->tex_name);

                    sw.Pause();
                    long tt = sw.Time();
                    if( tt > 10 ) n_longbind++;

                    double sx = rect.width;
                    double sy = rect.height;

                    if( g_bDebugOGL ) {
                        wxString msg;
                        msg.Printf( _T("     glQuads TexCoord (%g,%g) (%g,%g) (%g,%g) (%g,%g)"),
                                    x1 / sx, y1 / sy, ( x1 + w ) / sx, y1 / sy, ( x1 + w ) / sx,
                                    ( y1 + h ) / sy, x1 / sx, ( y1 + h ) / sy );
                        wxLogMessage( msg );

                        msg.Printf( _T("     glQuads Vertex2f (%g,%g) (%g,%g) (%g,%g) (%g,%g)"),
                                    ( x2 ), ( y2 ), ( w + x2 ), ( y2 ), ( w + x2 ), ( h + y2 ), ( x2 ),
                                    ( h + y2 ) );
                        wxLogMessage( msg );
                    }

                    glBegin( GL_QUADS );

                    glTexCoord2f( x1 / sx, y1 / sy );
                    glVertex2f( ( x2 ), ( y2 ) );
                    glTexCoord2f( ( x1 + w ) / sx, y1 / sy );
                    glVertex2f( ( w + x2 ), ( y2 ) );
                    glTexCoord2f( ( x1 + w ) / sx, ( y1 + h ) / sy );
                    glVertex2f( ( w + x2 ), ( h + y2 ) );
                    glTexCoord2f( x1 / sx, ( y1 + h ) / sy );
                    glVertex2f( ( x2 ), ( h + y2 ) );

                    glEnd();
                }
            }
            rect.x += rect.width;
        }

        rect.y += rect.height;
    }
//      printf("  basemult:%d  scalefactor:%g  chart_tex:%d\n", n_basemult, scalefactor, n_chart_tex);

    glPopMatrix();

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_DEPTH_TEST );

    if( n_longbind ) m_tex_max_res--;
    else {
        if( s_nquickbind++ > 100 ) {
            s_nquickbind = 0;
            m_tex_max_res++;
        }
    }
    m_tex_max_res = wxMax(m_tex_max_res, 20);
    m_tex_max_res = wxMin(m_tex_max_res, m_tex_max_res_initial);

}

void glChartCanvas::RenderQuiltViewGL( ViewPort &vp, wxRegion Region, bool b_clear )
{
    m_gl_rendered_region.Clear();

    if( cc1->m_pQuilt->GetnCharts() && !cc1->m_pQuilt->IsBusy() ) {
        //  Walk the region list to determine whether we need a clear before starting
        if( b_clear ) {
            wxRegion clear_test_region = Region;

            ChartBase *cchart = cc1->m_pQuilt->GetFirstChart();
            while( cchart ) {
                if( ! cc1->IsChartLargeEnoughToRender( cchart, vp ) ) {
                    cchart = cc1->m_pQuilt->GetNextChart();
                    continue;
                }

                QuiltPatch *pqp = cc1->m_pQuilt->GetCurrentPatch();
                if( pqp->b_Valid ) {
                    wxRegion get_region = pqp->ActiveRegion;

                    if( !get_region.IsEmpty() )
                        clear_test_region.Subtract( get_region );
                }
                cchart = cc1->m_pQuilt->GetNextChart();
            }

        //  We only need a screen clear if the test region is non-empty
            if( !clear_test_region.IsEmpty() )
                glClear( GL_COLOR_BUFFER_BIT );
        }

        //  Now render the quilt
        ChartBase *chart = cc1->m_pQuilt->GetFirstChart();

        while( chart ) {
            if( ! cc1->IsChartLargeEnoughToRender( chart, vp ) ) {
                chart = cc1->m_pQuilt->GetNextChart();
                continue;
            }

            QuiltPatch *pqp = cc1->m_pQuilt->GetCurrentPatch();
            if( pqp->b_Valid ) {
                wxRegion get_region = pqp->ActiveRegion;
                get_region.Intersect( Region );

                bool b_rendered = false;

                if( !get_region.IsEmpty() ) {
                    if( !pqp->b_overlay ) {
                        ChartBaseBSB *Patch_Ch_BSB = dynamic_cast<ChartBaseBSB*>( chart );
                        if( Patch_Ch_BSB ) {
                            RenderRasterChartRegionGL( chart, cc1->VPoint, get_region );
                            b_rendered = true;
                        } else {
                            ChartPlugInWrapper *Patch_Ch_Plugin =
                                dynamic_cast<ChartPlugInWrapper*>( chart );
                            if( Patch_Ch_Plugin ) {
                                if( Patch_Ch_Plugin->GetChartFamily() == CHART_FAMILY_RASTER ) {
                                    RenderRasterChartRegionGL( chart, cc1->VPoint, get_region );
                                    b_rendered = true;
                                }
                            }
                        }

                        if( !b_rendered ) {
                            if( chart->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                                wxRegion rr = get_region;
                                rr.Offset( vp.rv_rect.x, vp.rv_rect.y );
                                b_rendered = chart->RenderRegionViewOnGL( *m_pcontext, cc1->VPoint, rr );
                            }
                        }
                    }
                }

                if(b_rendered)
                    m_gl_rendered_region.Union(get_region);
            }


            chart = cc1->m_pQuilt->GetNextChart();
        }

        //    Render any Overlay patches for s57 charts(cells)
        if( cc1->m_pQuilt->HasOverlays() ) {
            ChartBase *pch = cc1->m_pQuilt->GetFirstChart();
            while( pch ) {
                QuiltPatch *pqp = cc1->m_pQuilt->GetCurrentPatch();
                if( pqp->b_Valid ) {
                    wxRegion get_region = pqp->ActiveRegion;

                    get_region.Intersect( Region );
                    if( !get_region.IsEmpty() ) {
                        if( pqp->b_overlay ) {
                            if( pch->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                                s57chart *Chs57 = dynamic_cast<s57chart*>( pch );
                                if( pch ) {
                                    get_region.Offset( cc1->VPoint.rv_rect.x,
                                                       cc1->VPoint.rv_rect.y );
                                    Chs57->RenderOverlayRegionViewOnGL( *m_pcontext, cc1->VPoint,
                                                                        get_region );
                                }
                            }
                        }
                    }
                }

                pch = cc1->m_pQuilt->GetNextChart();
            }
        }

        // Hilite rollover patch
        wxRegion hiregion = cc1->m_pQuilt->GetHiliteRegion( vp );

        if( !hiregion.IsEmpty() ) {
            glPushAttrib( GL_COLOR_BUFFER_BIT );
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

            double hitrans;
            switch( global_color_scheme ) {
            case GLOBAL_COLOR_SCHEME_DAY:
                hitrans = .4;
                break;
            case GLOBAL_COLOR_SCHEME_DUSK:
                hitrans = .2;
                break;
            case GLOBAL_COLOR_SCHEME_NIGHT:
                hitrans = .1;
                break;
            default:
                hitrans = .4;
                break;
            }

            glColor4f( (float) .8, (float) .4, (float) .4, (float) hitrans );

            wxRegionIterator upd ( hiregion );
            while ( upd )
            {
                wxRect rect = upd.GetRect();

                glBegin( GL_QUADS );
                glVertex2i( rect.x, rect.y );
                glVertex2i( rect.x + rect.width, rect.y );
                glVertex2i( rect.x + rect.width, rect.y + rect.height );
                glVertex2i( rect.x, rect.y + rect.height );
                glEnd();

                upd ++ ;
            }

            glDisable( GL_BLEND );
            glPopAttrib();
        }
        cc1->m_pQuilt->SetRenderedVP( vp );

    }
    else if( !cc1->m_pQuilt->GetnCharts() ) {
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

void glChartCanvas::ComputeRenderQuiltViewGLRegion( ViewPort &vp, wxRegion Region )
{
    m_gl_rendered_region.Clear();

    if( cc1->m_pQuilt->GetnCharts() && !cc1->m_pQuilt->IsBusy() ) {
            ChartBase *chart = cc1->m_pQuilt->GetFirstChart();

            while( chart ) {
                if( ! cc1->IsChartLargeEnoughToRender( chart, vp ) ) {
                    chart = cc1->m_pQuilt->GetNextChart();
                    continue;
                }

                QuiltPatch *pqp = cc1->m_pQuilt->GetCurrentPatch();
                if( pqp->b_Valid ) {
                    wxRegion get_region = pqp->ActiveRegion;
                    get_region.Intersect( Region );

                    //  Todo  If chart is cm93, and it happens not to render, then calculation will be wrong
                    //  need a "test render" method for cm93

                    m_gl_rendered_region.Union(get_region);
                }

                chart = cc1->m_pQuilt->GetNextChart();
            }
     }
}

void glChartCanvas::render()
{
    if( !m_bsetup ) return;

    if( ( !cc1->VPoint.b_quilt ) && ( !Current_Ch ) ) return;

    //    Take a look and see if memory is getting close to exceeding the user specified max
    m_b_mem_crunch = false;
    int mem_total, mem_used;
    GetMemoryStatus( &mem_total, &mem_used );
    if(mem_used > g_memCacheLimit * 8 / 10) m_b_mem_crunch = true;

    wxPaintDC( this );

    ViewPort VPoint = cc1->VPoint;
    ViewPort svp = VPoint;
    svp.pix_width = svp.rv_rect.width;
    svp.pix_height = svp.rv_rect.height;

    wxRegion ru = GetUpdateRegion();

    //  Is this viewpoint the same as the previously painted one?
    bool b_newview = true;
    ;
    if( ( m_gl_cache_vp.view_scale_ppm == VPoint.view_scale_ppm )
            && ( m_gl_cache_vp.rotation == VPoint.rotation ) && ( m_gl_cache_vp.clat == VPoint.clat )
            && ( m_gl_cache_vp.clon == VPoint.clon ) && m_gl_cache_vp.IsValid() ) {
        b_newview = false;
    }

    wxRegion chart_get_region( 0, 0, cc1->VPoint.rv_rect.width, cc1->VPoint.rv_rect.height );

    ocpnDC gldc( *this );

    int w, h;
    GetClientSize( &w, &h );
    glViewport( 0, 0, (GLint) w, (GLint) h );

    glLoadIdentity();
    gluOrtho2D( 0, (GLint) w, (GLint) h, 0 );

    if( g_b_useStencil ) {
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xff );
        glClear( GL_STENCIL_BUFFER_BIT );
        glDisable( GL_STENCIL_TEST );
    }

    //  Delete any textures known to the GPU that
    //  belong to charts which will not be used in this render
    //  This is done chart-by-chart...later we will scrub for unused textures
    //  that belong to charts which ARE used in this render, if we need to....

    if((m_ntex > m_tex_max_res) || m_b_mem_crunch ) {
        ChartPointerHashType::iterator it0;
        for( it0 = m_chart_hash.begin(); it0 != m_chart_hash.end(); ++it0 ) {
            ChartBaseBSB *pc = (ChartBaseBSB *) it0->first;

            if( VPoint.b_quilt )          // quilted
            {
                if( cc1->m_pQuilt && cc1->m_pQuilt->IsComposed() ) {
                    if( !cc1->m_pQuilt->IsChartInQuilt( pc ) ) {
                        ChartTextureHashType *pTextureHash = m_chart_hash[pc];

                        // iterate over all the textures presently loaded
                        // and delete the OpenGL texture from the GPU
                        // but keep the private texture descriptor for now

                        ChartTextureHashType::iterator it = pTextureHash->begin();
                        while( it != pTextureHash->end() ) {
                            glTextureDescriptor *ptd = it->second;

                            if( ptd->tex_name > 0 ) {
                                if( g_bDebugOGL ) printf(
                                        "glDeleteTextures in Unused chart...()\n" );
                                glDeleteTextures( 1, &ptd->tex_name );
                                m_ntex--;

                                ptd->tex_name = 0;

                                //    Delete the chart data?
                                if( m_b_mem_crunch ) {
                                    pTextureHash->erase( it );
                                    delete ptd;
                                    it = pTextureHash->begin();              // reset the iterator
                                }

                            } else
                                ++it;
                        }
                    }
                }
            }
            else      // not quilted
            {
                if( Current_Ch != pc )
                {
                    ChartTextureHashType *pTextureHash = m_chart_hash[pc];

                    // iterate over all the textures presently loaded
                    // and delete the OpenGL texture from the GPU
                    // but keep the private texture descriptor for now

                    ChartTextureHashType::iterator it = pTextureHash->begin();
                    while( it != pTextureHash->end())
                    {
                        glTextureDescriptor *ptd = it->second;

                        if(ptd->tex_name > 0)
                        {
                            if(g_bDebugOGL) printf("glDeleteTextures in Unused chart...()\n");
                            glDeleteTextures( 1, &ptd->tex_name );
                            m_ntex--;

                            ptd->tex_name = 0;

                            //    Delete the chart data?
                            if(m_b_mem_crunch)
                            {
                                pTextureHash->erase(it);
                                delete ptd;
                                it = pTextureHash->begin();    // reset the iterator
                            }

                        }
                        else
                            ++it;
                    }
                }
            }
        }
    }

    int sx = GetSize().x;
    int sy = GetSize().y;

    if( VPoint.b_quilt )          // quilted
    {
        if( cc1->m_pQuilt && !cc1->m_pQuilt->IsComposed() ) return;

        //  TODO This may not be necessary, but nice for debugging
//              glClear(GL_COLOR_BUFFER_BIT);

        // Try to do accelerated pans
        if( m_b_useFBO ) {
            if( m_gl_cache_vp.IsValid() && ( m_cache_tex > 0 ) && !g_bCourseUp ) {
                if( b_newview ) {

                    wxPoint c_old = VPoint.GetPixFromLL( VPoint.clat, VPoint.clon );
                    wxPoint c_new = m_gl_cache_vp.GetPixFromLL( VPoint.clat, VPoint.clon );

                    int dy = c_new.y - c_old.y;
                    int dx = c_new.x - c_old.x;

                    if( cc1->m_pQuilt->IsVPBlittable( VPoint, dx, dy, true ) ) // allow vector charts
                    {
                        if( dx || dy ) {
                            //    Render the reuseable portion of the cached texture
                            ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, m_fb0 );

                            //      Make a new temporary texture, and bind to FBO
                            glGenTextures( 1, &m_blit_tex );

                            glBindTexture( m_TEX_TYPE, m_blit_tex );
                            glTexParameterf( m_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                            glTexParameteri( m_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                            glTexImage2D( m_TEX_TYPE, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0,
                                          GL_RGBA, GL_UNSIGNED_BYTE, NULL );
                            ( *s_glFramebufferTexture2DEXT )( GL_FRAMEBUFFER_EXT,
                                                              GL_COLOR_ATTACHMENT0_EXT, m_TEX_TYPE, m_blit_tex, 0 );

                            glBindTexture( m_TEX_TYPE, m_cache_tex );
                            glEnable( m_TEX_TYPE );
                            glColor3f( 0, .25, 0 );
                            glDisable( GL_BLEND );
                            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

                            glDisable( GL_DEPTH_TEST );
                            glClear( GL_COLOR_BUFFER_BIT );

                            // Render the cached texture as quad to FBO(m_blit_tex) with offsets

                            if( GL_TEXTURE_RECTANGLE_ARB == m_TEX_TYPE ) {
                                wxASSERT(sx == m_cache_tex_x);
                                wxASSERT(sy == m_cache_tex_y);
                                int ow = sx - abs( dx );
                                int oh = sy - abs( dy );
                                int x1, x2, y1, y2;
                                if( dx > 0 ) {
                                    x1 = dx;
                                    x2 = 0;
                                } else {
                                    x1 = 0;
                                    x2 = -dx;
                                }

                                if( dy > 0 ) {
                                    y1 = dy;
                                    y2 = 0;
                                } else {
                                    y1 = 0;
                                    y2 = -dy;
                                }

                                glBegin( GL_QUADS );
                                glTexCoord2f( x1, sy - y1 );
                                glVertex2f( x2, y2 );
                                glTexCoord2f( x1 + ow, sy - y1 );
                                glVertex2f( x2 + ow, y2 );
                                glTexCoord2f( x1 + ow, y2 );
                                glVertex2f( x2 + ow, y2 + oh );
                                glTexCoord2f( x1, y2 );
                                glVertex2f( x2, y2 + oh );
                                glEnd();
                            }

                            //calculate the new regions to render
                            wxRegion update_region;
                            if( dy ) {
                                if( dy > 0 ) update_region.Union(
                                        wxRect( 0, VPoint.pix_height - dy, VPoint.pix_width, dy ) );
                                else
                                    update_region.Union( wxRect( 0, 0, VPoint.pix_width, -dy ) );
                            }

                            if( dx ) {
                                if( dx > 0 ) update_region.Union(
                                        wxRect( VPoint.pix_width - dx, 0, dx, VPoint.pix_height ) );
                                else
                                    update_region.Union( wxRect( 0, 0, -dx, VPoint.pix_height ) );
                            }

                            //   Done with cached texture "blit"
                            glDisable( m_TEX_TYPE );
                            //      Delete the stale cached texture
                            glDeleteTextures( 1, &m_cache_tex );
                            //    And make the blit texture "become" the cached texture
                            m_cache_tex = m_blit_tex;

                            //   Attach the renamed "blit" texture to the FBO
                            ( *s_glFramebufferTexture2DEXT )( GL_FRAMEBUFFER_EXT,
                                                              GL_COLOR_ATTACHMENT0_EXT, m_TEX_TYPE, m_cache_tex, 0 );

                            //      Render the chart(s) in update region
                            RenderQuiltViewGL( VPoint, update_region, false );          // no clear wanted here
                            ComputeRenderQuiltViewGLRegion( VPoint, chart_get_region );

                        } else {
                            //    No sensible (dx, dy) change in the view, so use the cached member bitmap
                        }
                    }

                    else              // not blitable
                    {
                        ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, m_fb0 );

                        //      Delete the current cached texture
                        glDeleteTextures( 1, &m_cache_tex );

                        //      Make a new texture, and bind to FBO
                        glGenTextures( 1, &m_cache_tex );

                        glBindTexture( m_TEX_TYPE, m_cache_tex );
                        glTexParameterf( m_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                        glTexParameteri( m_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                        glTexImage2D( m_TEX_TYPE, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0,
                                      GL_RGBA, GL_UNSIGNED_BYTE, NULL );
                        ( *s_glFramebufferTexture2DEXT )( GL_FRAMEBUFFER_EXT,
                                                          GL_COLOR_ATTACHMENT0_EXT, m_TEX_TYPE, m_cache_tex, 0 );

                        //      Render the chart(s)
                        RenderQuiltViewGL( VPoint, chart_get_region );

                    }
                }           // newview
                else {
                    //    No change in the view, so use the cached member texture
                }
            } else      //cached texture is not valid
            {
                ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, m_fb0 );

                //      Delete the current cached texture
                glDeleteTextures( 1, &m_cache_tex );

                //      Make a new texture, and bind to FBO
                glGenTextures( 1, &m_cache_tex );

                glBindTexture( m_TEX_TYPE, m_cache_tex );
                glTexParameterf( m_TEX_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                glTexParameteri( m_TEX_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                glTexImage2D( m_TEX_TYPE, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0, GL_RGBA,
                              GL_UNSIGNED_BYTE, NULL );
                ( *s_glFramebufferTexture2DEXT )( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                                  m_TEX_TYPE, m_cache_tex, 0 );

                //      Render the chart(s)
                RenderQuiltViewGL( VPoint, chart_get_region );

            }

            // Disable Render to FBO
            ( *s_glBindFramebufferEXT )( GL_FRAMEBUFFER_EXT, 0 );
            glDisable( GL_DEPTH_TEST );

            // Render the cached texture as quad to screen
            glBindTexture( m_TEX_TYPE, m_cache_tex );
            glEnable( m_TEX_TYPE );
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

            if( GL_TEXTURE_RECTANGLE_ARB == m_TEX_TYPE ) {
                glBegin( GL_QUADS );
                glTexCoord2f( 0, m_cache_tex_y );
                glVertex2f( 0, 0 );
                glTexCoord2f( m_cache_tex_x, m_cache_tex_y );
                glVertex2f( sx, 0 );
                glTexCoord2f( m_cache_tex_x, 0 );
                glVertex2f( sx, sy );
                glTexCoord2f( 0, 0 );
                glVertex2f( 0, sy );
                glEnd();
            }
            glDisable( m_TEX_TYPE );

            m_gl_cache_vp = VPoint;
            cc1->m_pQuilt->SetRenderedVP( VPoint );

        }         // useFBO
        else {
            RenderQuiltViewGL( svp, chart_get_region );
        }

    }         // quilted
    else                  // not quilted
    {
        bool b_rendered = false;
        ChartBaseBSB *Current_Ch_BSB = dynamic_cast<ChartBaseBSB*>( Current_Ch );
        if( Current_Ch_BSB ) {
            glClear( GL_COLOR_BUFFER_BIT );
            RenderRasterChartRegionGL( Current_Ch, cc1->VPoint, chart_get_region );
            b_rendered = true;
        } else {
            ChartPlugInWrapper *Current_Ch_PlugInWrapper =
                dynamic_cast<ChartPlugInWrapper*>( Current_Ch );
            if( Current_Ch_PlugInWrapper ) {
                if( Current_Ch_PlugInWrapper->GetChartFamily() == CHART_FAMILY_RASTER ) {
                    RenderRasterChartRegionGL( Current_Ch, cc1->VPoint, chart_get_region );
                    b_rendered = true;
                }
            }
        }

        if( !b_rendered ) {
            if( !dynamic_cast<ChartDummy*>( Current_Ch ) ) {
                glClear( GL_COLOR_BUFFER_BIT );
                wxRegion full_region( cc1->VPoint.rv_rect );
                Current_Ch->RenderRegionViewOnGL( *m_pcontext, cc1->VPoint, full_region );
            }
        }
    }

//    Render the WorldChart

    wxRegion chartValidRegion;

    if(!VPoint.b_quilt)
        Current_Ch->GetValidCanvasRegion ( svp, &chartValidRegion );
    else
        chartValidRegion = m_gl_rendered_region;

    // Make a region covering the current chart on the canvas
        // growing the box to account for rotation
    wxRegion backgroundRegion( VPoint.rv_rect.x, VPoint.rv_rect.y, VPoint.rv_rect.width,
                                   VPoint.rv_rect.height );


        //    Remove the valid chart area
    if( chartValidRegion.IsOk() ) {
            chartValidRegion.Offset( wxPoint(VPoint.rv_rect.x, VPoint.rv_rect.y) );
            backgroundRegion.Subtract( chartValidRegion );
    }

        //    Draw the World Chart only in the areas NOT covered by the current chart view
        //    And, only if the region is ..not.. empty

    if( !backgroundRegion.IsEmpty() && ( fabs( cc1->GetVP().skew ) < .01 ) ) {
        ViewPort nvp = VPoint;
        nvp.rv_rect.x = 0;
        nvp.rv_rect.y = 0;

        SetClipRegion( nvp, backgroundRegion, true );       // clear background

        glPushMatrix();
        if( fabs( cc1->GetVP().rotation ) > .01 ) {
            double w2 = cc1->GetVP().rv_rect.width / 2;
            double h2 = cc1->GetVP().rv_rect.height / 2;

            double angle = cc1->GetVP().rotation;

            //    Rotations occur around 0,0, so calculate a post-rotate translation factor
            double ddx = ( w2 * cos( -angle ) - h2 * sin( -angle ) - w2 ) ;
            double ddy = ( h2 * cos( -angle ) + w2 * sin( -angle ) - h2 ) ;
            glRotatef( angle * 180. / PI, 0, 0, 1 );
            glTranslatef( ddx, ddy, 0 );                 // post rotate translation

                // WorldBackgroundChart renders in an offset rectangle,
                // So translate back to standard coordinates
            double x1 = cc1->GetVP().rv_rect.x;
            double y1 = cc1->GetVP().rv_rect.y;
            double x2 =  x1 * cos( angle ) + y1 * sin( angle );
            double y2 =  y1 * cos( angle ) - x1 * sin( angle );
            glTranslatef( x2, y2, 0 );
        }

        cc1->pWorldBackgroundChart->RenderViewOnDC( gldc, VPoint );

        glDisable( GL_STENCIL_TEST );
        glDisable( GL_DEPTH_TEST );

        glPopMatrix();
    }


//    Now render overlay objects
    DrawGLOverLayObjects();
    cc1->DrawOverlayObjects( gldc, ru );

    if( cc1->m_bShowTide ) cc1->DrawAllTidesInBBox( gldc, cc1->GetVP().GetBBox(), true, true );

    if( cc1->m_bShowCurrent ) cc1->DrawAllCurrentsInBBox( gldc, cc1->GetVP().GetBBox(), true,
                true );

    //quiting?
    if( g_bquiting ) {
        GLubyte pattern[4 * 32];
        for( int y = 0; y < 32; y++ ) {
            GLubyte mask = 1 << y % 8;
            for( int x = 0; x < 4; x++ )
                pattern[y * 4 + x] = mask;
        }

        glEnable( GL_POLYGON_STIPPLE );
        glPolygonStipple( pattern );
        glBegin( GL_QUADS );
        glColor3f( 0, 0, 0 );
        glVertex2i( 0, 0 );
        glVertex2i( 0, GetSize().y );
        glVertex2i( GetSize().x, GetSize().y );
        glVertex2i( GetSize().x, 0 );
        glEnd();
        glDisable( GL_POLYGON_STIPPLE );
    }

#if 0
    //  Debug
    wxRegionIterator upd ( ru );
    while ( upd )
    {
        wxRect rect = upd.GetRect();
        glBegin(GL_LINE_LOOP);
        glColor3f(0, 0, 0);
        glVertex2i(rect.x, rect.y);
        glVertex2i(rect.x, rect.y + rect.height);
        glVertex2i(rect.x + rect.width, rect.y + rect.height);
        glVertex2i(rect.x + rect.width, rect.y);
        glEnd();

        upd ++;
    }
#endif

    SwapBuffers();
    glFinish();

    cc1->PaintCleanup();

//     if(g_bDebugOGL)  printf("m_ntex: %d %d\n\n", m_ntex, m_tex_max_res);
    if( g_bDebugOGL ) {
        wxString msg;
        msg.Printf( _T("  -->m_ntex %d %d\n"), m_ntex, m_tex_max_res );
        wxLogMessage( msg );
    }

}

void glChartCanvas::DrawGLOverLayObjects( void )
{
    if( g_pi_manager ) g_pi_manager->RenderAllGLCanvasOverlayPlugIns( m_pcontext, cc1->GetVP() );
}

void glChartCanvas::GrowData( int size )
{
    /* grow the temporary ram buffer used to load charts into textures */
    if( size > m_datasize ) {
        unsigned char* tmp = (unsigned char*) realloc( m_data, m_datasize );
        if( tmp != NULL ) {
            m_data = tmp;
            m_datasize = size;
            tmp = NULL;
        }
    }
}

void glChartCanvas::SetClipRegion( ViewPort &vp, wxRegion &region, bool b_clear )
{
    if( g_b_useStencil ) {
        glPushMatrix();

        if( ( ( fabs( vp.rotation ) > 0.01 ) ) || ( g_bskew_comp && ( fabs( vp.skew ) > 0.01 ) ) ) {

            //    Shift texture drawing positions to account for the larger chart rectangle
            //    needed to cover the screen on rotated images
            double w = vp.pix_width;
            double h = vp.pix_height;

            double angle = vp.rotation;
            angle -= vp.skew;

            //    Rotations occur around 0,0, so calculate a post-rotate translation factor
            double ddx = ( w * cos( -angle ) - h * sin( -angle ) - w ) / 2;
            double ddy = ( h * cos( -angle ) + w * sin( -angle ) - h ) / 2;

            glRotatef( angle * 180. / PI, 0, 0, 1 );

            glTranslatef( ddx, ddy, 0 );                 // post rotate translation
        }

        //    Create a stencil buffer for clipping to the region
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0x1 );                 // write only into bit 0 of the stencil buffer
        glClear( GL_STENCIL_BUFFER_BIT );

        //    As a convenience, while we are creating the stencil or depth mask,
        //    also clear the background if selected
        if( b_clear ) {
            glColor3f( 0, 255, 0 );
            glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // enable color buffer
        } else
            glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );   // disable color buffer

        //    We are going to write "1" into the stencil buffer wherever the region is valid
        glStencilFunc( GL_ALWAYS, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

        //    Decompose the region into rectangles, and draw as quads
        wxRegionIterator clipit( region );
        while( clipit ) {
            wxRect rect = clipit.GetRect();

            if(vp.b_quilt)
                rect.Offset(vp.rv_rect.x, vp.rv_rect.y); // undo the adjustment made in quilt composition
            else if(Current_Ch && Current_Ch->GetChartFamily() != CHART_FAMILY_VECTOR)
                rect.Offset(vp.rv_rect.x, vp.rv_rect.y);

            glBegin( GL_QUADS );

            glVertex2f( rect.x, rect.y );
            glVertex2f( rect.x + rect.width, rect.y );
            glVertex2f( rect.x + rect.width, rect.y + rect.height );
            glVertex2f( rect.x, rect.y + rect.height );
            glEnd();

            clipit++;
        }

        //    Now set the stencil ops to subsequently render only where the stencil bit is "1"
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer

        glPopMatrix();

    } else              //  Use depth buffer for clipping
    {
        glPushMatrix();

        if( ( ( fabs( vp.rotation ) > 0.01 ) ) || ( g_bskew_comp && ( fabs( vp.skew ) > 0.01 ) ) ) {

            //    Shift texture drawing positions to account for the larger chart rectangle
            //    needed to cover the screen on rotated images
            double w = vp.pix_width;
            double h = vp.pix_height;

            double angle = vp.rotation;
            angle -= vp.skew;

            //    Rotations occur around 0,0, so calculate a post-rotate translation factor
            double ddx = ( w * cos( -angle ) - h * sin( -angle ) - w ) / 2;
            double ddy = ( h * cos( -angle ) + w * sin( -angle ) - h ) / 2;

            glRotatef( angle * 180. / PI, 0, 0, 1 );

            glTranslatef( ddx, ddy, 0 );                 // post rotate translation
        }

        glEnable( GL_DEPTH_TEST ); // to enable writing to the depth buffer
        glDepthFunc( GL_ALWAYS );  // to ensure everything you draw passes
        glDepthMask( GL_TRUE );    // to allow writes to the depth buffer

        glClear( GL_DEPTH_BUFFER_BIT ); // for a fresh start

        //    As a convenience, while we are creating the stencil or depth mask,
        //    also clear the background if selected
        if( b_clear ) {
            glColor3f( 255, 0, 0 );
            glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // enable color buffer
        } else
            glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );   // disable color buffer

        //    Decompose the region into rectangles, and draw as quads
        //    With z = 1
        wxRegionIterator clipit( region );
        while( clipit ) {
            wxRect rect = clipit.GetRect();

            if(vp.b_quilt)
                rect.Offset(vp.rv_rect.x, vp.rv_rect.y); // undo the adjustment made in quilt composition
            else if(Current_Ch && Current_Ch->GetChartFamily() != CHART_FAMILY_VECTOR)
                rect.Offset(vp.rv_rect.x, vp.rv_rect.y);

            glBegin( GL_QUADS );

            // dep buffer clear = 1
            // 1 makes 0 in dep buffer, works
            // 0 make .5 in depth buffer
            // -1 makes 1 in dep buffer

            //    Depth buffer runs from 0 at z = 1 to 1 at z = -1
            //    Draw the clip geometry at z = 0.5, giving a depth buffer value of 0.25
            //    Subsequent drawing at z=0 (depth = 0.5) will pass if using glDepthFunc(GL_GREATER);
            glVertex3f( rect.x, rect.y, 0.5 );
            glVertex3f( rect.x + rect.width, rect.y, 0.5 );
            glVertex3f( rect.x + rect.width, rect.y + rect.height, 0.5 );
            glVertex3f( rect.x, rect.y + rect.height, 0.5 );
            glEnd();

            clipit++;
        }

        glDepthFunc( GL_GREATER );                          // Set the test value
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer
        glDepthMask( GL_FALSE );                            // disable depth buffer

        glPopMatrix();

    }
}
//------------------------------------------------------------------------------
//    TCwin Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE ( TCWin, wxWindow ) EVT_PAINT ( TCWin::OnPaint )
    EVT_SIZE ( TCWin::OnSize )
    EVT_MOTION ( TCWin::MouseEvent )
    EVT_BUTTON ( wxID_OK, TCWin::OKEvent )
    EVT_BUTTON ( ID_TCWIN_NX, TCWin::NXEvent )
    EVT_BUTTON ( ID_TCWIN_PR, TCWin::PREvent )
    EVT_CLOSE ( TCWin::OnCloseWindow )
    EVT_TIMER ( TCWININF_TIMER, TCWin::OnTCWinPopupTimerEvent )
END_EVENT_TABLE()

WX_DEFINE_LIST( SplineList );

// Define a constructor
TCWin::TCWin( ChartCanvas *parent, int x, int y, void *pvIDX )
{

    //    As a display optimization....
    //    if current color scheme is other than DAY,
    //    Then create the dialog ..WITHOUT.. borders and title bar.
    //    This way, any window decorations set by external themes, etc
    //    will not detract from night-vision

    long wstyle = wxCLIP_CHILDREN | wxDEFAULT_DIALOG_STYLE;
    if( ( global_color_scheme != GLOBAL_COLOR_SCHEME_DAY )
            && ( global_color_scheme != GLOBAL_COLOR_SCHEME_RGB ) ) wstyle |= ( wxNO_BORDER );

    wxDialog::Create( parent, wxID_ANY, wxString( _T ( "test" ) ), wxPoint( x, y ),
                      wxSize( 550, 480 ), wstyle );

    pParent = parent;

    pIDX = (IDX_entry *) pvIDX;
    gpIDXn++;

//    Set up plot type
    if( strchr( "Tt", pIDX->IDX_type ) ) {
        m_plot_type = TIDE_PLOT;
        SetTitle( wxString( _( "Tide" ) ) );
        gpIDX = pIDX;       // remember pointer for routeplan

    } else {
        m_plot_type = CURRENT_PLOT;
        SetTitle( wxString( _( "Current" ) ) );
    }

    m_pTCRolloverWin = NULL;

    int sx, sy;
    GetClientSize( &sx, &sy );
    int swx, swy;
    GetSize( &swx, &swy );
    int parent_sx, parent_sy;
    pParent->GetClientSize( &parent_sx, &parent_sy );

    int xc = x + 8;
    int yc = y;

//  Arrange for tcWindow to be always totally visible
    if( ( x + 8 + swx ) > parent_sx ) xc = xc - swx - 16;
    if( ( y + swy ) > parent_sy ) yc = yc - swy;

//  Don't let the window origin move out of client area
    if( yc < 0 ) yc = 0;
    if( xc < 0 ) xc = 0;

    pParent->ClientToScreen( &xc, &yc );
    wxPoint r( xc, yc );
    Move( r );

//    Figure out this computer timezone minute offset
    wxDateTime this_now = wxDateTime::Now();
    wxDateTime this_gmt = this_now.ToGMT();

#if wxCHECK_VERSION(2, 6, 2)
    wxTimeSpan diff = this_now.Subtract( this_gmt );
#else
    wxTimeSpan diff = this_gmt.Subtract ( this_now );
#endif

    int diff_mins = diff.GetMinutes();

    int station_offset = ptcmgr->GetStationTimeOffset( pIDX );

    m_corr_mins = station_offset - diff_mins;
    if( this_now.IsDST() ) m_corr_mins += 60;

//    Establish the inital drawing day as today
    m_graphday = wxDateTime::Now();
    wxDateTime graphday_00 = wxDateTime::Today();
    time_t t_graphday_00 = graphday_00.GetTicks();

    //    Correct a Bug in wxWidgets time support
    if( !graphday_00.IsDST() && m_graphday.IsDST() ) t_graphday_00 -= 3600;
    if( graphday_00.IsDST() && !m_graphday.IsDST() ) t_graphday_00 += 3600;

    m_t_graphday_00_at_station = t_graphday_00 - ( m_corr_mins * 60 );

    btc_valid = false;

    wxString* TClist = NULL;
    m_tList = new wxListBox( this, -1, wxPoint( sx * 65 / 100, 11 ),
                             wxSize( ( sx * 32 / 100 ), ( sy * 20 / 100 ) ), 0, TClist,
                             wxLB_SINGLE | wxLB_NEEDED_SB );

    OK_button = new wxButton( this, wxID_OK, _( "OK" ), wxPoint( sx - 100, sy - 32 ),
                              wxDefaultSize );

    PR_button = new wxButton( this, ID_TCWIN_PR, _( "Prev" ), wxPoint( 10, sy - 32 ),
                              wxSize( 60, -1 ) );

    m_ptextctrl = new wxTextCtrl( this, -1, _T(""), wxPoint( sx * 3 / 100, 6 ),
                                  wxSize( ( sx * 60 / 100 ), ( sy *29 / 100 ) ) ,
                                  wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
    int bsx, bsy, bpx, bpy;
    PR_button->GetSize( &bsx, &bsy );
    PR_button->GetPosition( &bpx, &bpy );

    NX_button = new wxButton( this, ID_TCWIN_NX, _( "Next" ), wxPoint( bpx + bsx + 5, bpy ),
                              wxSize( 60, -1 ) );

    m_TCWinPopupTimer.SetOwner( this, TCWININF_TIMER );


    //  establish some graphic element sizes/locations
    int x_graph = sx * 1 / 10;
    int y_graph = sy * 32 / 100;
    int x_graph_w = sx * 8 / 10;
    int y_graph_h = sy * 50 / 100;
    m_graph_rect = wxRect(x_graph, y_graph, x_graph_w, y_graph_h);


    // Build graphics tools

    pSFont = wxTheFontList->FindOrCreateFont( 8, wxFONTFAMILY_SWISS, wxNORMAL,
                                                    wxFONTWEIGHT_NORMAL, FALSE, wxString( _T ( "Arial" ) ) );
    pSMFont = wxTheFontList->FindOrCreateFont( 10, wxFONTFAMILY_SWISS, wxNORMAL,
                                                       wxFONTWEIGHT_NORMAL, FALSE, wxString( _T ( "Arial" ) ) );
    pMFont = wxTheFontList->FindOrCreateFont( 11, wxFONTFAMILY_SWISS, wxNORMAL, wxBOLD,
                                                      FALSE, wxString( _T ( "Arial" ) ) );
    pLFont = wxTheFontList->FindOrCreateFont( 12, wxFONTFAMILY_SWISS, wxNORMAL, wxBOLD,
                                                      FALSE, wxString( _T ( "Arial" ) ) );

    pblack_1 = wxThePenList->FindOrCreatePen( GetGlobalColor( _T ( "UINFD" ) ), 1,
                                                                          wxSOLID );
    pblack_2 = wxThePenList->FindOrCreatePen( GetGlobalColor( _T ( "UINFD" ) ), 2,
                                                                          wxSOLID );
    pblack_3 = wxThePenList->FindOrCreatePen( GetGlobalColor( _T ( "UWHIT" ) ), 1,
                                                                          wxSOLID );
    pred_2 = wxThePenList->FindOrCreatePen( GetGlobalColor( _T ( "UINFR" ) ), 4,
                                                                        wxSOLID );
    pltgray = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T ( "UIBCK" ) ),
                                                                               wxSOLID );
    pltgray2 = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T ( "DILG1" ) ),
                                                                                wxSOLID );

    DimeControl( this );

    //  Fill in some static text control information

    //  Tidi station information
    m_ptextctrl->Clear();

    wxString locn( pIDX->IDX_station_name, wxConvUTF8 );
    wxString locna, locnb;
    if( locn.Contains( wxString( _T ( "," ) ) ) ) {
        locna = locn.BeforeFirst( ',' );
        locnb = locn.AfterFirst( ',' );
    } else {
        locna = locn;
        locnb.Empty();
    }

    // write the first line
    wxTextAttr style;
    style.SetFont( *pLFont );
    m_ptextctrl->SetDefaultStyle( style );

    m_ptextctrl->AppendText( locna );
    m_ptextctrl->AppendText(_T("\n"));

    style.SetFont( *pSMFont );
    m_ptextctrl->SetDefaultStyle( style );

    if( !locnb.IsEmpty() ) m_ptextctrl->AppendText( locnb );
    m_ptextctrl->AppendText(_T("\n\n"));


    //Reference to the master station
    if(( 't' == pIDX->IDX_type ) || ( 'c' == pIDX->IDX_type )) {
        wxString mref( pIDX->IDX_reference_name, wxConvUTF8 );
        mref.Prepend(_T(" "));

        m_ptextctrl->AppendText( _( "Reference Station :" ) );
        m_ptextctrl->AppendText(_T("\n"));

        m_ptextctrl->AppendText( mref );
        m_ptextctrl->AppendText(_T("\n"));

    }
    else {
        m_ptextctrl->AppendText(_T("\n"));
        m_ptextctrl->AppendText(_T("\n"));
    }

    //      Show the data source
    wxString dsource( pIDX->source_ident, wxConvUTF8 );
    dsource.Prepend(_T(" "));

    m_ptextctrl->AppendText( _( "Data Source :" ) );
    m_ptextctrl->AppendText(_T("\n"));

    m_ptextctrl->AppendText( dsource );

    m_ptextctrl->ShowPosition( 0 );
}

TCWin::~TCWin()
{
    pParent->Refresh( false );
}

void TCWin::OKEvent( wxCommandEvent& event )
{
    Hide();
    pParent->pCwin = NULL;
    if( --gpIDXn == 0 ) gpIDX = NULL;
    delete m_pTCRolloverWin;
    delete m_tList;
    pParent->Refresh( false );
    Destroy();                          // that hurts
}

void TCWin::OnCloseWindow( wxCloseEvent& event )
{
    Hide();
    pParent->pCwin = NULL;
    if( --gpIDXn == 0 ) gpIDX = NULL;
    delete m_pTCRolloverWin;
    delete m_tList;

    Destroy();                          // that hurts
}

void TCWin::NXEvent( wxCommandEvent& event )
{
    wxTimeSpan dt( 24, 0, 0, 0 );
    m_graphday.Add( dt );
    wxDateTime dm = m_graphday;

    wxDateTime graphday_00 = dm.ResetTime();
    if(graphday_00.GetYear() == 2013)
        int yyp = 4;

    time_t t_graphday_00 = graphday_00.GetTicks();
    if( !graphday_00.IsDST() && m_graphday.IsDST() ) t_graphday_00 -= 3600;
    if( graphday_00.IsDST() && !m_graphday.IsDST() ) t_graphday_00 += 3600;
    m_t_graphday_00_at_station = t_graphday_00 - ( m_corr_mins * 60 );

    btc_valid = false;
    Refresh();

}

void TCWin::PREvent( wxCommandEvent& event )
{
    wxTimeSpan dt( -24, 0, 0, 0 );
    m_graphday.Add( dt );
    wxDateTime dm = m_graphday;

    wxDateTime graphday_00 = dm.ResetTime();
    time_t t_graphday_00 = graphday_00.GetTicks();

    if( !graphday_00.IsDST() && m_graphday.IsDST() ) t_graphday_00 -= 3600;
    if( graphday_00.IsDST() && !m_graphday.IsDST() ) t_graphday_00 += 3600;

    m_t_graphday_00_at_station = t_graphday_00 - ( m_corr_mins * 60 );

    btc_valid = false;
    Refresh();
}

void TCWin::Resize( void )
{
}

void TCWin::RePosition( void )
{
//    Position the window
    double lon = pIDX->IDX_lon;
    double lat = pIDX->IDX_lat;

    wxPoint r;
    pParent->GetCanvasPointPix( lat, lon, &r );
    pParent->ClientToScreen( &r.x, &r.y );
    Move( r );
}

void TCWin::OnPaint( wxPaintEvent& event )
{
    int x, y;
    int i;
    char sbuf[100];
    int w;
    float tcmax, tcmin;

    GetClientSize( &x, &y );

    wxPaintDC dc( this );

    wxString tlocn( pIDX->IDX_station_name, wxConvUTF8 );

//     if(1/*bForceRedraw*/)
    {


        int x_textbox = x * 5 / 100;
        int y_textbox = 6;

        int x_textbox_w = x * 51 / 100;
        int y_textbox_h = y * 25 / 100;

        // box the location text & tide-current table
        dc.SetPen( *pblack_3 );
        dc.SetBrush( *pltgray2 );
        dc.DrawRoundedRectangle( x_textbox, y_textbox, x_textbox_w, y_textbox_h, 4 );    //location text box

        wxRect tab_rect = m_tList->GetRect();
        dc.DrawRoundedRectangle( tab_rect.x - 4, y_textbox, tab_rect.width + 8, y_textbox_h, 4 ); //tide-current table box

        //    Box the graph
        dc.SetPen( *pblack_1 );
        dc.SetBrush( *pltgray );
        dc.DrawRectangle( m_graph_rect.x, m_graph_rect.y, m_graph_rect.width, m_graph_rect.height );

        //    Horizontal axis
        dc.SetFont( *pSFont );
        for( i = 0; i < 25; i++ ) {
            int xd = m_graph_rect.x + ( ( i ) * m_graph_rect.width / 25 );
            dc.DrawLine( xd, m_graph_rect.y, xd, m_graph_rect.y + m_graph_rect.height + 5 );

            char sbuf[5];
            sprintf( sbuf, "%02d", i );
#ifdef __WXMSW__
            wxString sst;
            sst.Printf( _T("%02d"), i );
            dc.DrawRotatedText( sst, xd + ( m_graph_rect.width / 25 ) / 2, m_graph_rect.y + m_graph_rect.height + 8, 270. );
#else
            int x_shim = -12;
            dc.DrawText ( wxString ( sbuf, wxConvUTF8 ), xd + x_shim + ( m_graph_rect.width/25 ) /2, m_graph_rect.y + m_graph_rect.height + 8 );
#endif
        }

        //    Make a line for "right now"
        time_t t_now = wxDateTime::Now().GetTicks();       // now, in ticks

        float t_ratio = m_graph_rect.width * ( t_now - m_t_graphday_00_at_station ) / ( 25 * 3600 );

        //must eliminate line outside the graph (in that case put it outside the window)
        int xnow = ( t_ratio < 0 || t_ratio > m_graph_rect.width ) ? -1 : m_graph_rect.x + (int) t_ratio;
        dc.SetPen( *pred_2 );
        dc.DrawLine( xnow, m_graph_rect.y, xnow, m_graph_rect.y + m_graph_rect.height );
        dc.SetPen( *pblack_1 );

        //    Build the array of values, capturing max and min and HW/LW list

        if( !btc_valid ) {

            float dir;
            tcmax = -10;
            tcmin = 10;
            float val;
            m_tList->Clear();
            int list_index = 0;
            bool wt;

            wxBeginBusyCursor();

            // get tide flow sens ( flood or ebb ? )
            ptcmgr->GetTideFlowSens( m_t_graphday_00_at_station, BACKWARD_ONE_HOUR_STEP,
                                     pIDX->IDX_rec_num, tcv[0], val, wt );

            for( i = 0; i < 26; i++ ) {
                int tt = m_t_graphday_00_at_station + ( i * FORWARD_ONE_HOUR_STEP );
                ptcmgr->GetTideOrCurrent( tt, pIDX->IDX_rec_num, tcv[i], dir );
                if( tcv[i] > tcmax ) tcmax = tcv[i];

                if( tcv[i] < tcmin ) tcmin = tcv[i];
                if( TIDE_PLOT == m_plot_type ) {
                    if( !( ( tcv[i] > val ) == wt ) )                // if tide flow sens change
                    {
                        float tcvalue;                                  //look backward for HW or LW
                        time_t tctime;
                        ptcmgr->GetHightOrLowTide( tt, BACKWARD_TEN_MINUTES_STEP,
                                                   BACKWARD_ONE_MINUTES_STEP, tcv[i], wt, pIDX->IDX_rec_num, tcvalue,
                                                   tctime );

                        wxDateTime tcd;                                                 //write date
                        wxString s, s1;
                        tcd.Set( tctime + ( m_corr_mins * 60 ) );
                        s.Printf( tcd.Format( _T("%H:%M  ") ) );
                        s1.Printf( _T("%05.2f "), tcvalue );                           //write value
                        s.Append( s1 );
                        Station_Data *pmsd = pIDX->pref_sta_data;                       //write unit
                        if( pmsd ) s.Append( wxString( pmsd->units_abbrv, wxConvUTF8 ) );
                        s.Append( _T("   ") );
                        ( wt ) ? s.Append( _("HW") ) : s.Append( _("LW") );         //write HW or LT

                        m_tList->Insert( s, list_index );                       // update table list
                        list_index++;

                        wt = !wt;                                            //change tide flow sens
                    }
                    val = tcv[i];
                }
                if( CURRENT_PLOT == m_plot_type ) {
                    wxDateTime thx;                                                     //write date
                    wxString s, s1;
                    thx.Set( (time_t) ( tt + ( m_corr_mins * 60 ) ) );
                    s.Printf( thx.Format( _T("%H:%M  ") ) );
                    s1.Printf( _T("%05.2f "), fabs( tcv[i] ) );                        //write value
                    s.Append( s1 );
                    Station_Data *pmsd = pIDX->pref_sta_data;                           //write unit
                    if( pmsd ) s.Append( wxString( pmsd->units_abbrv, wxConvUTF8 ) );
                    s1.Printf( _T("  %03.0f"), dir );                              //write direction
                    s.Append( s1 );
                    m_tList->Insert( s, list_index );                           // update table list
                    list_index++;
                }

            }

            wxEndBusyCursor();

//    Set up the vertical parameters based on Tide or Current plot
            if( CURRENT_PLOT == m_plot_type ) {
                it = __max ( abs (( int ) tcmin - 1 ), abs ( ( int ) tcmax + 1 ) );
                ib = -it;

                im = 2 * it;
                m_plot_y_offset = m_graph_rect.height / 2;
                val_off = 0;
            } else {
                ib = (int) tcmin;
                if( tcmin < 0 ) ib -= 1;
                it = (int) tcmax + 1;

                im = it - ib; //abs ( ib ) + abs ( it );
                m_plot_y_offset = ( m_graph_rect.height * ( it - ib ) ) / im;
                val_off = ib;
            }

//    Build spline list of points

            m_sList.DeleteContents( true );
            m_sList.Clear();

            for( i = 0; i < 26; i++ ) {
                wxPoint *pp = new wxPoint;
                pp->x = m_graph_rect.x + ( ( i ) * m_graph_rect.width / 25 );
                pp->y = m_graph_rect.y + ( m_plot_y_offset )
                - (int) ( ( tcv[i] - val_off ) * m_graph_rect.height / im );

                m_sList.Append( pp );
            }

            btc_valid = true;
        }

        dc.SetTextForeground( GetGlobalColor( _T ( "DILG3" ) ) );

        //    Vertical Axis

        //      Maybe skip some lines and legends if the range is too high
        int height_stext;
         dc.GetTextExtent( _T("1"), NULL, &height_stext );

        int i_skip = 1;
        if( height_stext > m_graph_rect.height / im ) i_skip = 2;

        i = ib;
        while( i < it + 1 ) {
            int yd = m_graph_rect.y + ( m_plot_y_offset ) - ( ( i - val_off ) * m_graph_rect.height / im );

            if( ( m_plot_y_offset + m_graph_rect.y ) == yd ) dc.SetPen( *pblack_2 );
            else
                dc.SetPen( *pblack_1 );

            dc.DrawLine( m_graph_rect.x, yd, m_graph_rect.x + m_graph_rect.width, yd );
            snprintf( sbuf, 99, "%d", i );
            dc.DrawText( wxString( sbuf, wxConvUTF8 ), m_graph_rect.x - 20, yd - 5 );
            i += i_skip;

        }

        //    Draw the Value curve
#if wxCHECK_VERSION(2, 9, 0)
        wxPointList *list = (wxPointList *)&m_sList;
#else
        wxList *list = (wxList *) &m_sList;
#endif

        dc.SetPen( *pblack_2 );
#if wxUSE_SPLINES
        dc.DrawSpline( list );
#else
        dc.DrawLines ( list );
#endif
        //  More Info

///
        int station_offset = ptcmgr->GetStationTimeOffset( pIDX );
        int h = station_offset / 60;
        int m = station_offset - ( h * 60 );
        if( m_graphday.IsDST() ) h += 1;
        m_stz.Printf( _T("Z %+03d:%02d"), h, m );

//    Make the "nice" (for the US) station time-zone string, brutally by hand
        wxString mtz;
        switch( ptcmgr->GetStationTimeOffset( pIDX ) ) {
        case -240:
            mtz = _T( "AST" );
            break;
        case -300:
            mtz = _T( "EST" );
            break;
        case -360:
            mtz = _T( "CST" );
            break;
        }

        if( mtz.Len() ) {
            if( m_graphday.IsDST() ) mtz[1] = 'D';

            m_stz = mtz;
        }

///
        dc.SetFont( *pSFont );
        dc.GetTextExtent( m_stz, &w, &h );
        dc.DrawText( m_stz, x / 2 - w / 2, y * 88 / 100 );

        // There seems to be some confusion about format specifiers
        //  Hack this.....
        //  Find and use the longest "sprintf" result......
        wxString sdate;
        wxString s1 = m_graphday.Format( _T ( "%#x" ) );
        wxString s2 = m_graphday.Format( _T ( "%x" ) );

        if( s2.Len() > s1.Len() ) sdate = s2;
        else
            sdate = s1;
        dc.SetFont( *pMFont );
        dc.GetTextExtent( sdate, &w, &h );
        dc.DrawText( sdate, x / 2 - w / 2, y * 92 / 100 );

        ///
        Station_Data *pmsd = pIDX->pref_sta_data;
        if( pmsd ) {
            dc.GetTextExtent( wxString( pmsd->units_conv, wxConvUTF8 ), &w, &h );
            dc.DrawRotatedText( wxString( pmsd->units_conv, wxConvUTF8 ), 5,
                                m_graph_rect.y + m_graph_rect.height / 2 + w / 2, 90. );
        }

        //      Show flood and ebb directions
        if(( strchr( "c", pIDX->IDX_type ) ) || ( strchr( "C", pIDX->IDX_type ) )) {
            dc.SetFont( *pSFont );

            wxString fdir;
            fdir.Printf( _T("%03d"), pIDX->IDX_flood_dir );
            dc.DrawText( fdir, m_graph_rect.x + m_graph_rect.width + 4, m_graph_rect.y + m_graph_rect.height * 1 / 4 );

            wxString edir;
            edir.Printf( _T("%03d"), pIDX->IDX_ebb_dir );
            dc.DrawText( edir, m_graph_rect.x + m_graph_rect.width + 4, m_graph_rect.y + m_graph_rect.height * 3 / 4 );

        }

//    Today or tomorrow
        wxString sday;
        wxDateTime this_now = wxDateTime::Now();

        int day = m_graphday.GetDayOfYear();
        if( m_graphday.GetYear() == this_now.GetYear() ) {
            if( day == this_now.GetDayOfYear() ) sday.Append( _( "Today" ) );
            else if( day == this_now.GetDayOfYear() + 1 ) sday.Append( _( "Tomorrow" ) );
            else
                sday.Append( m_graphday.GetWeekDayName( m_graphday.GetWeekDay() ) );
        } else if( m_graphday.GetYear() == this_now.GetYear() + 1
                   && day == this_now.Add( wxTimeSpan::Day() ).GetDayOfYear() ) sday.Append(
                           _( "Tomorrow" ) );

        dc.SetFont( *pSFont );
//                dc.GetTextExtent ( wxString ( sday, wxConvUTF8 ), &w, &h );       2.9.1
//                dc.DrawText ( wxString ( sday, wxConvUTF8 ), 55 - w/2, y * 88/100 );    2.9.1
        dc.GetTextExtent( sday, &w, &h );
        dc.DrawText( sday, 55 - w / 2, y * 88 / 100 );

    }
}

void TCWin::OnSize( wxSizeEvent& event )
{
    int width, height;
    GetClientSize( &width, &height );
    int x, y;
    GetPosition( &x, &y );
}

void TCWin::MouseEvent( wxMouseEvent& event )
{
    event.GetPosition( &curs_x, &curs_y );

    if( !m_TCWinPopupTimer.IsRunning() ) m_TCWinPopupTimer.Start( 20, wxTIMER_ONE_SHOT );
}

void TCWin::OnTCWinPopupTimerEvent( wxTimerEvent& event )
{

    int x, y;
    bool ShowRollover;

    GetClientSize( &x, &y );
    wxRegion cursorarea(m_graph_rect);
    if( cursorarea.Contains( curs_x, curs_y ) ) {
        ShowRollover = true;
        SetCursor( *pParent->pCursorCross );
        if( NULL == m_pTCRolloverWin ) {
            m_pTCRolloverWin = new RolloverWin( this );
            m_pTCRolloverWin->SetMousePropogation( 1 );
            m_pTCRolloverWin->Hide();
        }
        float t, d;
        wxString p, s;
        //set time on x cursor position
        t = ( 25 / ( (float) x * 8 / 10 ) ) * ( (float) curs_x - ( (float) x * 1 / 10 ) );
        int tt = m_t_graphday_00_at_station + (int) ( t * 3600 );
        wxDateTime thd;
        time_t ths = tt + ( m_corr_mins * 60 );
        thd.Set( ths );
        p.Printf( thd.Format( _T("%Hh %Mmn") ) );
        p.Append( _T("\n") );

        //set tide level or current speed at that time
        ptcmgr->GetTideOrCurrent( tt, pIDX->IDX_rec_num, t, d );
        s.Printf( _T("%3.2f "), ( t < 0 && CURRENT_PLOT == m_plot_type ) ? -t : t ); // always positive if current
        p.Append( s );

        //set unit
        Station_Data *pmsd = pIDX->pref_sta_data;
        if( pmsd ) p.Append( wxString( pmsd->units_abbrv, wxConvUTF8 ) );

        //set current direction
        if( CURRENT_PLOT == m_plot_type ) {
            s.Printf( wxString("%3.0f°", wxConvUTF8), d );
            p.Append( _T("\n") );
            p.Append( s );
        }

        //set rollover area size
        wxSize win_size;
        win_size.Set( x * 90 / 100, y * 80 / 100 );

        m_pTCRolloverWin->SetString( p );
        m_pTCRolloverWin->SetBestPosition( curs_x, curs_y, 1, 1, TC_ROLLOVER, win_size );
        m_pTCRolloverWin->SetBitmap( TC_ROLLOVER );
        m_pTCRolloverWin->Refresh();
        m_pTCRolloverWin->Show();
    } else {
        SetCursor( *pParent->pCursorArrow );
        ShowRollover = false;
    }

    if( m_pTCRolloverWin && m_pTCRolloverWin->IsShown() && !ShowRollover ) {
        m_pTCRolloverWin->Hide();
        m_pTCRolloverWin = NULL;
    }

}

#ifdef __WXX11__
//----------------------------------------------------------------------------------------------------------
//    ocpCursor Implementation
//----------------------------------------------------------------------------------------------------------
#include "wx/x11/private.h"

//----------------------------------------------------------------------------------------------
//      ocpCursorRefData Definition/Implementation
//----------------------------------------------------------------------------------------------

class ocpCursorRefData: public wxObjectRefData
{
public:

    ocpCursorRefData();
    ~ocpCursorRefData();

    WXCursor m_cursor;
    WXDisplay *m_display;
};

ocpCursorRefData::ocpCursorRefData()
{
    m_cursor = NULL;
    m_display = NULL;
}

ocpCursorRefData::~ocpCursorRefData()
{
    if ( m_cursor )
        XFreeCursor ( ( Display* ) m_display, ( Cursor ) m_cursor );
}

//-----------------------------------------------------------------------------

#define M_CURSORDATA ((ocpCursorRefData *)m_refData)

/*
 IMPLEMENT_DYNAMIC_CLASS(ocpCursor,wxObject)

 ocpCursor::ocpCursor()
 {

 }
 */

//----------------------------------------------------------------------------------------------
//      ocpCursor Implementation
//
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
//      A new constructor taking a file name to load and assign as a cursor
//----------------------------------------------------------------------------------------------
ocpCursor::ocpCursor ( const wxString& cursorName, long type,
                       int hotSpotX, int hotSpotY ) : wxCursor ( wxCURSOR_CROSS )
{
    wxImage cImage;

    if ( !cImage.CanRead ( cursorName ) )
        ::wxInitAllImageHandlers();

    cImage.LoadFile ( cursorName );

    int width = cImage.GetWidth();
    int height = cImage.GetHeight();

//    m_refData = new wxCursorRefData();

    //    Get some X parameters
    int xscreen = DefaultScreen ( ( Display* ) wxGlobalDisplay() );
    Window xroot = RootWindow ( ( Display* ) wxGlobalDisplay(), xscreen );
    Visual* xvisual = DefaultVisual ( ( Display* ) wxGlobalDisplay(), xscreen );

    M_CURSORDATA->m_display = wxGlobalDisplay();
    wxASSERT_MSG ( M_CURSORDATA->m_display, wxT ( "No display" ) );

    //    Make a pixmap
    Pixmap cpixmap = XCreatePixmap ( ( Display* ) wxGlobalDisplay(),
                                     xroot, width, height, 1 );

    //    Make an Ximage
    XImage *data_image = XCreateImage ( ( Display* ) wxGlobalDisplay(), xvisual,
                                        1, ZPixmap, 0, 0, width, height, 32, 0 );
    data_image->data = ( char* ) malloc ( data_image->bytes_per_line * data_image->height );

    int index = 0;
    int pixel = 0;
    unsigned char* data = cImage.GetData();

    // Create mask

    Pixmap cmask;
    unsigned char mr, mg, mb;

    if ( cImage.HasMask() )
    {
        XImage *mask_image = XCreateImage ( ( Display* ) wxGlobalDisplay(), xvisual,
                                            1, ZPixmap, 0, 0, width, height, 32, 0 );
        mask_image->data = ( char* ) malloc ( mask_image->bytes_per_line * mask_image->height );

        cImage.GetOrFindMaskColour ( &mr, &mg, &mb );

        int rit = ( mr << 16 ) + ( mg << 8 ) + mb;
        for ( int y = 0; y < height; y++ )
        {
            for ( int x = 0; x < width; x++ )
            {
                int ri = ( int ) data[index++];
                ri += data[index++] << 8;
                ri += data[index++] << 16;

                /*
                 int ri = *(int *)(&data[index]);
                 ri &= 0x00ffffff;
                 index++;
                 index++;
                 index++;
                 */
                pixel = 1;
                if ( ri == rit )        // if data is mask value, mask pixel gets 0
                    pixel = 0;

                XPutPixel ( mask_image, x, y, pixel );

            }
        }

        cmask = XCreatePixmap ( ( Display* ) wxGlobalDisplay(), xroot, width, height, 1 );

        GC gc = XCreateGC ( ( Display* ) wxGlobalDisplay(), cmask, 0, NULL );
        XPutImage ( ( Display* ) wxGlobalDisplay(), cmask, gc, mask_image,
                    0, 0, 0, 0, width, height );

        XDestroyImage ( mask_image );
        XFreeGC ( ( Display* ) wxGlobalDisplay(), gc );

    }

    //    Render the wxImage cImage onto the Ximage
    //    Simple black/white cursors only, please

    index = 0;

    for ( int y = 0; y < height; y++ )
    {
        for ( int x = 0; x < width; x++ )
        {
            int ri = ( int ) data[index++];
            ri += data[index++] << 8;
            ri += data[index++] << 16;

            /*
             int ri = *(int *)(&data[index]);
             ri &= 0x00ffffff;
             index++;
             index++;
             index++;
             */

            pixel = 0;
            if ( ri )
                pixel = 1;

            XPutPixel ( data_image, x, y, pixel );

        }
    }

    //    Put the Ximage into the pixmap

    GC gc = XCreateGC ( ( Display* ) wxGlobalDisplay(), cpixmap, 0, NULL );
    XPutImage ( ( Display* ) wxGlobalDisplay(), cpixmap, gc, data_image,
                0, 0, 0, 0, width, height );

    //    Free the Ximage stuff
    XDestroyImage ( data_image );
    XFreeGC ( ( Display* ) wxGlobalDisplay(), gc );

    //    Make a X cursor from the pixmap

    XColor fg, bg;
    fg.red = fg.blue = fg.green = 0xffff;
    bg.red = bg.blue = bg.green = 0;

    M_CURSORDATA->m_cursor = ( WXCursor ) XCreatePixmapCursor ( ( Display* ) wxGlobalDisplay(), cpixmap, cmask,
                             &fg, &bg, hotSpotX, hotSpotY );

}

//----------------------------------------------------------------------------------------------
//      A new constructor taking a static char ** of XPM data and assign as a cursor
//----------------------------------------------------------------------------------------------

ocpCursor::ocpCursor ( const char **xpm_data, long type,
                       int hotSpotX, int hotSpotY ) : wxCursor ( wxCURSOR_CROSS )
{
    wxImage cImage ( xpm_data );

    int width = cImage.GetWidth();
    int height = cImage.GetHeight();

//    m_refData = new wxCursorRefData();

    //    Get some X parameters
    int xscreen = DefaultScreen ( ( Display* ) wxGlobalDisplay() );
    Window xroot = RootWindow ( ( Display* ) wxGlobalDisplay(), xscreen );
    Visual* xvisual = DefaultVisual ( ( Display* ) wxGlobalDisplay(), xscreen );

    M_CURSORDATA->m_display = wxGlobalDisplay();
    wxASSERT_MSG ( M_CURSORDATA->m_display, wxT ( "No display" ) );

    //    Make a pixmap
    Pixmap cpixmap = XCreatePixmap ( ( Display* ) wxGlobalDisplay(),
                                     xroot, width, height, 1 );

    //    Make an Ximage
    XImage *data_image = XCreateImage ( ( Display* ) wxGlobalDisplay(), xvisual,
                                        1, ZPixmap, 0, 0, width, height, 32, 0 );
    data_image->data = ( char* ) malloc ( data_image->bytes_per_line * data_image->height );

    int index = 0;
    int pixel = 0;
    unsigned char* data = cImage.GetData();

    // Create mask

    Pixmap cmask;
    unsigned char mr, mg, mb;

    if ( cImage.HasMask() )
    {
        XImage *mask_image = XCreateImage ( ( Display* ) wxGlobalDisplay(), xvisual,
                                            1, ZPixmap, 0, 0, width, height, 32, 0 );
        mask_image->data = ( char* ) malloc ( mask_image->bytes_per_line * mask_image->height );

        cImage.GetOrFindMaskColour ( &mr, &mg, &mb );

        int rit = ( mr << 16 ) + ( mg << 8 ) + mb;
        for ( int y = 0; y < height; y++ )
        {
            for ( int x = 0; x < width; x++ )
            {
                int ri = ( int ) data[index++];
                ri += data[index++] << 8;
                ri += data[index++] << 16;

                /*
                 int ri = *(int *)(&data[index]);
                 ri &= 0x00ffffff;
                 index++;
                 index++;
                 index++;
                 */
                pixel = 1;
                if ( ri == rit )        // if data is mask value, mask pixel gets 0
                    pixel = 0;

                XPutPixel ( mask_image, x, y, pixel );

            }
        }

        cmask = XCreatePixmap ( ( Display* ) wxGlobalDisplay(), xroot, width, height, 1 );

        GC gc = XCreateGC ( ( Display* ) wxGlobalDisplay(), cmask, 0, NULL );
        XPutImage ( ( Display* ) wxGlobalDisplay(), cmask, gc, mask_image,
                    0, 0, 0, 0, width, height );

        XDestroyImage ( mask_image );
        XFreeGC ( ( Display* ) wxGlobalDisplay(), gc );

    }

    //    Render the wxImage cImage onto the Ximage
    //    Simple black/white cursors only, please

    index = 0;

    for ( int y = 0; y < height; y++ )
    {
        for ( int x = 0; x < width; x++ )
        {
            int ri = ( int ) data[index++];
            ri += data[index++] << 8;
            ri += data[index++] << 16;

            /*
             int ri = *(int *)(&data[index]);
             ri &= 0x00ffffff;
             index++;
             index++;
             index++;
             */

            pixel = 0;
            if ( ri )
                pixel = 1;

            XPutPixel ( data_image, x, y, pixel );

        }
    }

    //    Put the Ximage into the pixmap

    GC gc = XCreateGC ( ( Display* ) wxGlobalDisplay(), cpixmap, 0, NULL );
    XPutImage ( ( Display* ) wxGlobalDisplay(), cpixmap, gc, data_image,
                0, 0, 0, 0, width, height );

    //    Free the Ximage stuff
    XDestroyImage ( data_image );
    XFreeGC ( ( Display* ) wxGlobalDisplay(), gc );

    //    Make a X cursor from the pixmap

    XColor fg, bg;
    fg.red = fg.blue = fg.green = 0xffff;
    bg.red = bg.blue = bg.green = 0;

    M_CURSORDATA->m_cursor = ( WXCursor ) XCreatePixmapCursor ( ( Display* ) wxGlobalDisplay(), cpixmap, cmask,
                             &fg, &bg, hotSpotX, hotSpotY );

}

#endif      // __WXX11__

//      We derive a class from wxCursor to create ocpCursor
//      Specifically to fix a bug in wxImage-wxBitmap conversions

#ifdef __WXMSW__

/*
 //----------------------------------------------------------------------------------------------
 //      ocpCursorRefData Definition/Implementation
 //----------------------------------------------------------------------------------------------
 class ocpCursorRefData: public wxObjectRefData
 {
 public:
 ocpCursorRefData();
 ocpCursorRefData(HCURSOR);
 ~ocpCursorRefData();
 };

 ocpCursorRefData::ocpCursorRefData()
 {

 }

 ocpCursorRefData::ocpCursorRefData(HCURSOR hcursor)
 {
 }


 ocpCursorRefData::~ocpCursorRefData()
 {
 }
 */

//----------------------------------------------------------------------------------------------
//      ocpCursor Implementation
//
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
//      A new constructor taking a file name to load and assign as a cursor
//----------------------------------------------------------------------------------------------

ocpCursor::ocpCursor( const wxString& cursorName, long type, int hotSpotX, int hotSpotY ) :
    wxCursor( wxCURSOR_ARROW )

{
    wxImage cImage;

    if( !cImage.CanRead( cursorName ) ) ::wxInitAllImageHandlers();

    cImage.LoadFile( cursorName );

//      wxMSW Bug???
//      On Windows XP, conversion from wxImage to wxBitmap fails at the ::CreateDIBitmap() call
//      unless a "compatible" dc is provided.  Why??
//      As a workaround, just make a simple wxDC for temporary use

    wxBitmap tbmp( cImage.GetWidth(), cImage.GetHeight(), -1 );
    wxMemoryDC dwxdc;
    dwxdc.SelectObject( tbmp );

//        HCURSOR hcursor = wxBitmapToHCURSOR ( wxBitmap ( cImage, ( wxDC & ) dwxdc ),
//                                              hotSpotX, hotSpotY );
    HCURSOR hcursor = NULL;

    if( !hcursor ) {
        wxLogWarning( _T( "Failed to create ocpCursor." ) );
        return;
    }

//  Replace the HANDLE created in the base class constructor
//  Probably leaks....
    GetGDIImageData()->m_handle = hcursor;
}

//----------------------------------------------------------------------------------------------
//      A new constructor taking a static char ** of XPM data and assign as a cursor
//----------------------------------------------------------------------------------------------

ocpCursor::ocpCursor( const char **xpm_data, long type, int hotSpotX, int hotSpotY ) :
    wxCursor( wxCURSOR_ARROW )

{
    wxImage cImage( xpm_data );

//      wxMSW Bug???
//      On Windows XP, conversion from wxImage to wxBitmap fails at the ::CreateDIBitmap() call
//      unless a "compatible" dc is provided.  Why??
//      As a workaround, just make a simple wxDC for temporary use

    wxBitmap tbmp( cImage.GetWidth(), cImage.GetHeight(), -1 );
    wxMemoryDC dwxdc;
    dwxdc.SelectObject( tbmp );

//        HCURSOR hcursor = wxBitmapToHCURSOR ( wxBitmap ( cImage, ( wxDC & ) dwxdc ),
//                                              hotSpotX, hotSpotY );

    HCURSOR hcursor = NULL;

    if( !hcursor ) {
        wxLogWarning( _T( "Failed to create ocpCursor." ) );
        return;
    }

//  Replace the HANDLE created in the base class constructor
//  Probably leaks....
    GetGDIImageData()->m_handle = hcursor;
}
#endif   // __MSW
#ifdef __WXOSX__  // begin rms
/*
 //----------------------------------------------------------------------------------------------
 //      ocpCursorRefData Definition/Implementation
 //----------------------------------------------------------------------------------------------
 class ocpCursorRefData: public wxObjectRefData
 {
 public:
 ocpCursorRefData();
 ocpCursorRefData(HCURSOR);
 ~ocpCursorRefData();
 };

 ocpCursorRefData::ocpCursorRefData()
 {

 }

 ocpCursorRefData::ocpCursorRefData(HCURSOR hcursor)
 {
 }


 ocpCursorRefData::~ocpCursorRefData()
 {
 }
 */
//----------------------------------------------------------------------------------------------
//      ocpCursor Implementation
//
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//      A new constructor taking a file name to load and assign as a cursor
//----------------------------------------------------------------------------------------------
ocpCursor::ocpCursor ( const wxString& cursorName, long type,
                       int hotSpotX, int hotSpotY ) : wxCursor ( wxCURSOR_ARROW )

{
    wxImage cImage;

    if ( !cImage.CanRead ( cursorName ) )
        ::wxInitAllImageHandlers();

    cImage.LoadFile ( cursorName );

//      wxMSW Bug???
//      On Windows XP, conversion from wxImage to wxBitmap fails at the ::CreateDIBitmap() call
//      unless a "compatible" dc is provided.  Why??
//      As a workaround, just make a simple wxDC for temporary use

    wxBitmap tbmp ( cImage.GetWidth(),cImage.GetHeight(),-1 );
}

//----------------------------------------------------------------------------------------------
//      A new constructor taking a static char ** of XPM data and assign as a cursor
//----------------------------------------------------------------------------------------------

ocpCursor::ocpCursor ( const char **xpm_data, long type,
                       int hotSpotX, int hotSpotY ) : wxCursor ( wxCURSOR_ARROW )

{
    wxImage cImage ( xpm_data );

    wxBitmap tbmp ( cImage.GetWidth(),cImage.GetHeight(),-1 );
}

#endif   // __WXOSX__ end rms

//---------------------------------------------------------------------------------------
//          AISTargetQueryDialog Implementation
//---------------------------------------------------------------------------------------
#define xID_OK 10009
IMPLEMENT_CLASS ( AISTargetQueryDialog, wxDialog )
// AISTargetQueryDialog event table definition
BEGIN_EVENT_TABLE ( AISTargetQueryDialog, wxDialog ) EVT_BUTTON( xID_OK, AISTargetQueryDialog::OnIdOKClick )
    EVT_CLOSE(AISTargetQueryDialog::OnClose)
    EVT_MOVE( AISTargetQueryDialog::OnMove )
END_EVENT_TABLE()

AISTargetQueryDialog::AISTargetQueryDialog()
{
    Init();
}

AISTargetQueryDialog::AISTargetQueryDialog( wxWindow* parent, wxWindowID id,
        const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );
}

AISTargetQueryDialog::~AISTargetQueryDialog()
{
    delete m_pQueryTextCtl;
}

void AISTargetQueryDialog::Init()
{
    m_MMSI = -1;
    m_pQueryTextCtl = NULL;
    m_nl = 0;
    m_colorscheme = (ColorScheme) ( -1 );
    m_okButton = NULL;

}
void AISTargetQueryDialog::OnClose( wxCloseEvent& event )
{
    Destroy();
    g_pais_query_dialog_active = NULL;
}

void AISTargetQueryDialog::OnIdOKClick( wxCommandEvent& event )
{
    Close();
}

bool AISTargetQueryDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption,
                                   const wxPoint& pos, const wxSize& size, long style )
{
    //    As a display optimization....
    //    if current color scheme is other than DAY,
    //    Then create the dialog ..WITHOUT.. borders and title bar.
    //    This way, any window decorations set by external themes, etc
    //    will not detract from night-vision

    long wstyle = wxDEFAULT_FRAME_STYLE;
    if( ( global_color_scheme != GLOBAL_COLOR_SCHEME_DAY )
            && ( global_color_scheme != GLOBAL_COLOR_SCHEME_RGB ) ) wstyle |= ( wxNO_BORDER );

    if( !wxDialog::Create( parent, id, caption, pos, size, wstyle ) ) return false;

    wxFont *dFont = pFontMgr->GetFont( _("AISTargetQuery"), 12 );
    int font_size = wxMax(8, dFont->GetPointSize());
    wxString face = dFont->GetFaceName();
#ifdef __WXGTK__
    face = _T("Monospace");
#endif
    wxFont *fp_font = wxTheFontList->FindOrCreateFont( font_size, wxFONTFAMILY_MODERN,
                      wxFONTSTYLE_NORMAL, dFont->GetWeight(), false, face );

    SetFont( *fp_font );

    CreateControls();

    SetColorScheme( global_color_scheme );

// This ensures that the dialog cannot be sized smaller
// than the minimum size
    GetSizer()->SetSizeHints( this );

    return true;
}

void AISTargetQueryDialog::SetColorScheme( ColorScheme cs )
{
    if( cs != m_colorscheme ) {
        DimeControl( this );
        Refresh();
    }
}

void AISTargetQueryDialog::CreateControls()
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( topSizer );

    m_pQueryTextCtl = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                wxHW_SCROLLBAR_AUTO );
    m_pQueryTextCtl->SetBorders( 5 );

    topSizer->Add( m_pQueryTextCtl, 1, wxALIGN_CENTER_HORIZONTAL | wxALL | wxEXPAND, 5 );

    wxSizer* ok = CreateButtonSizer( wxOK );
    topSizer->Add( ok, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5 );
}

void AISTargetQueryDialog::UpdateText()
{
    wxString html;

    if( !m_pQueryTextCtl ) return;

    DimeControl( this );
    wxColor bg = GetBackgroundColour();
    m_pQueryTextCtl->SetBackgroundColour( bg );

    if( m_MMSI != 0 ) { //  Faulty MMSI could be reported as 0
        AIS_Target_Data *td = g_pAIS->Get_Target_Data_From_MMSI( m_MMSI );
        if( td ) {
            wxFont *dFont = pFontMgr->GetFont( _("AISTargetQuery"), 12 );
            wxString face = dFont->GetFaceName();
            int sizes[7];
            for( int i=-2; i<5; i++ ) {
                sizes[i+2] = dFont->GetPointSize() + i + (i>0?i:0);
            }

            html.Printf( _T("<html><body bgcolor=#%02x%02x%02x><center>"), bg.Red(), bg.Blue(),
                            bg.Green() );

            html << td->BuildQueryResult();
            html << _T("</center></font></body></html>");

            m_pQueryTextCtl->SetFonts( face, face, sizes );
            m_pQueryTextCtl->SetPage( html );

            // Try to create a min size that works across font sizes.
            wxSize sz;
            if( ! IsShown() ) {
                sz = m_pQueryTextCtl->GetVirtualSize();
                sz.x = 300;
                m_pQueryTextCtl->SetSize( sz );
            }
            m_pQueryTextCtl->Layout();
            wxSize ir(m_pQueryTextCtl->GetInternalRepresentation()->GetWidth(),
                    m_pQueryTextCtl->GetInternalRepresentation()->GetHeight() );
            sz.x = wxMax( m_pQueryTextCtl->GetSize().x, ir.x );
            sz.y = wxMax( m_pQueryTextCtl->GetSize().y, ir.y );
            m_pQueryTextCtl->SetMinSize( sz );
            Fit();
            sz -= wxSize( 200, 200 );
            m_pQueryTextCtl->SetMinSize( sz );
        }
    }
}

void AISTargetQueryDialog::OnMove( wxMoveEvent& event )
{
    //    Record the dialog position
    wxPoint p = event.GetPosition();
    g_ais_query_dialog_x = p.x;
    g_ais_query_dialog_y = p.y;
    event.Skip();
}

void ShowAISTargetQueryDialog( wxWindow *win, int mmsi )
{
    if( !win ) return;

    if( NULL == g_pais_query_dialog_active ) {
        int pos_x = g_ais_query_dialog_x;
        int pos_y = g_ais_query_dialog_y;

        if( g_pais_query_dialog_active ) {
            delete g_pais_query_dialog_active;
            g_pais_query_dialog_active = new AISTargetQueryDialog();
        }
        else {
            g_pais_query_dialog_active = new AISTargetQueryDialog();
        }

        g_pais_query_dialog_active->Create( win, -1, _( "AIS Target Query" ),
                                            wxPoint( pos_x, pos_y ) );

        g_pais_query_dialog_active->SetMMSI( mmsi );
        g_pais_query_dialog_active->UpdateText();
        wxSize sz = g_pais_query_dialog_active->GetSize();

        bool b_reset_pos = false;
#ifdef __WXMSW__
        //  Support MultiMonitor setups which an allow negative window positions.
        //  If the requested window title bar does not intersect any installed monitor,
        //  then default to simple primary monitor positioning.
        RECT frame_title_rect;
        frame_title_rect.left = pos_x;
        frame_title_rect.top = pos_y;
        frame_title_rect.right = pos_x + sz.x;
        frame_title_rect.bottom = pos_y + 30;

        if( NULL == MonitorFromRect( &frame_title_rect, MONITOR_DEFAULTTONULL ) ) b_reset_pos =
                true;
#else

        //    Make sure drag bar (title bar) of window intersects wxClient Area of screen, with a little slop...
        wxRect window_title_rect;// conservative estimate
        window_title_rect.x = pos_x;
        window_title_rect.y = pos_y;
        window_title_rect.width = sz.x;
        window_title_rect.height = 30;

        wxRect ClientRect = wxGetClientDisplayRect();
        ClientRect.Deflate(60, 60);// Prevent the new window from being too close to the edge
        if(!ClientRect.Intersects(window_title_rect))
            b_reset_pos = true;

#endif

        if( b_reset_pos ) g_pais_query_dialog_active->Move( 50, 200 );

    } else {
        g_pais_query_dialog_active->SetMMSI( mmsi );
        g_pais_query_dialog_active->UpdateText();
    }

    g_pais_query_dialog_active->Show();
}
//-----------------------------------------------------------------------
//
//    Generic Rollover window implementation
//
//-----------------------------------------------------------------------
BEGIN_EVENT_TABLE(RolloverWin, wxWindow) EVT_PAINT(RolloverWin::OnPaint)
    EVT_TIMER(ROLLOVER_TIMER, RolloverWin::OnTimer)
    EVT_MOUSE_EVENTS ( RolloverWin::OnMouseEvent )

END_EVENT_TABLE()

// Define a constructor
RolloverWin::RolloverWin( wxWindow *parent, int timeout ) :
    wxWindow( parent, wxID_ANY, wxPoint( 0, 0 ), wxSize( 1, 1 ), wxNO_BORDER )
{
    m_pbm = NULL;

    m_timer_timeout.SetOwner( this, ROLLOVER_TIMER );
    m_timeout_sec = timeout;
    m_mmouse_propogate = 0;
    isActive = false;
    m_plabelFont = NULL;
    Hide();
}

RolloverWin::~RolloverWin()
{
    delete m_pbm;
}
void RolloverWin::OnTimer( wxTimerEvent& event )
{
    if( IsShown() ) Hide();
}

void RolloverWin::OnMouseEvent( wxMouseEvent& event )
{
    //    If directed, send mouse events up the window family tree,
    //    until some parent window does NOT call event.Skip()
    if( m_mmouse_propogate ) {
        event.ResumePropagation( m_mmouse_propogate );
        event.Skip();
    }
}

void RolloverWin::SetBitmap( int rollover )
{
    wxDC* cdc = new wxScreenDC();
    wxPoint canvasPos = GetParent()->GetScreenPosition();

    wxMemoryDC mdc;
    delete m_pbm;
    m_pbm = new wxBitmap( m_size.x, m_size.y, -1 );
    mdc.SelectObject( *m_pbm );

    mdc.Blit( 0, 0, m_size.x, m_size.y, cdc, m_position.x + canvasPos.x,
              m_position.y + canvasPos.y );
    delete cdc;

    ocpnDC dc( mdc );

    switch( rollover ) {
        case AIS_ROLLOVER:
            AlphaBlending( dc, 0, 0, m_size.x, m_size.y, 6.0, GetGlobalColor( _T ( "YELO1" ) ), 172 );
            mdc.SetTextForeground( pFontMgr->GetFontColor( _T("AISRollover") ) );
            break;

        case TC_ROLLOVER:
            AlphaBlending( dc, 0, 0, m_size.x, m_size.y, 0.0, GetGlobalColor( _T ( "YELO1" ) ), 255 );
            mdc.SetTextForeground( pFontMgr->GetFontColor( _T("TideCurrentGraphRollover") ) );
            break;
        default:
        case LEG_ROLLOVER:
            AlphaBlending( dc, 0, 0, m_size.x, m_size.y, 6.0, GetGlobalColor( _T ( "YELO1" ) ), 172 );
            mdc.SetTextForeground( pFontMgr->GetFontColor( _T("RouteLegInfoRollover") ) );
            break;
    }


    if(m_plabelFont && m_plabelFont->IsOk()) {
        
    //    Draw the text
        mdc.SetFont( *m_plabelFont );

        mdc.DrawLabel( m_string, wxRect( 0, 0, m_size.x, m_size.y ), wxALIGN_CENTRE_HORIZONTAL | wxALIGN_CENTRE_VERTICAL);
    }

    SetSize( m_position.x, m_position.y, m_size.x, m_size.y );   // Assumes a nominal 32 x 32 cursor

    // Retrigger the auto timeout
    if( m_timeout_sec > 0 ) m_timer_timeout.Start( m_timeout_sec * 1000, wxTIMER_ONE_SHOT );
}

void RolloverWin::OnPaint( wxPaintEvent& event )
{
    int width, height;
    GetClientSize( &width, &height );
    wxPaintDC dc( this );

    if( m_string.Len() ) {
        wxMemoryDC mdc;
        mdc.SelectObject( *m_pbm );
        dc.Blit( 0, 0, width, height, &mdc, 0, 0 );
    }
}

void RolloverWin::SetBestPosition( int x, int y, int off_x, int off_y, int rollover,
                                   wxSize parent_size )
{
    int h, w;

    wxFont *dFont;
    switch( rollover ) {

    case AIS_ROLLOVER:
        dFont = pFontMgr->GetFont( _("AISRollover"), 12 );
        break;

    case TC_ROLLOVER:
        dFont = pFontMgr->GetFont( _("TideCurrentGraphRollover"), 12 );
        break;

    default:
    case LEG_ROLLOVER:
        dFont = pFontMgr->GetFont( _("RouteLegInfoRollover"), 12 );
        break;

    }
    
    int font_size = wxMax(8, dFont->GetPointSize());
    m_plabelFont = wxTheFontList->FindOrCreateFont( font_size, dFont->GetFamily(),
                         dFont->GetStyle(), dFont->GetWeight(), false, dFont->GetFaceName() );

    if(m_plabelFont && m_plabelFont->IsOk()) {
#ifdef __WXMAC__
        wxScreenDC sdc;
        sdc.GetMultiLineTextExtent(m_string, &w, &h, NULL, m_plabelFont);
#else
        wxClientDC cdc( GetParent() );
        cdc.GetMultiLineTextExtent( m_string, &w, &h, NULL, m_plabelFont );
#endif
    }
    else {
        w = 10;
        h = 10;
    }
    
    m_size.x = w + 8;
    m_size.y = h + 8;

    int xp, yp;
    if( ( x + off_x + m_size.x ) > parent_size.x ) {
        xp = x - ( off_x / 2 ) - m_size.x;
        xp = wxMax(0, xp);
    } else
        xp = x + off_x;

    if( ( y + off_y + m_size.y ) > parent_size.y ) {
        yp = y - ( off_y / 2 ) - m_size.y;
    } else
        yp = y + off_y;

    SetPosition( wxPoint( xp, yp ) );

}
//------------------------------------------------------------------------------
//    CM93 Detail Slider Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(CM93DSlide, wxDialog)
    EVT_MOVE( CM93DSlide::OnMove )
    EVT_COMMAND_SCROLL_THUMBRELEASE(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_LINEUP(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_LINEDOWN(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_PAGEUP(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_PAGEDOWN(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_BOTTOM(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_TOP(-1, CM93DSlide::OnChangeValue)
    EVT_CLOSE( CM93DSlide::OnClose )
END_EVENT_TABLE()

CM93DSlide::CM93DSlide( wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue,
                        const wxPoint& pos, const wxSize& size, long style, const wxString& title )
{
    Init();
    Create( parent, ID_CM93ZOOMG, value, minValue, maxValue, pos, size, style, title );
}

CM93DSlide::~CM93DSlide()
{
    delete m_pCM93DetailSlider;
}

void CM93DSlide::Init( void )
{
    m_pCM93DetailSlider = NULL;
}

bool CM93DSlide::Create( wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue,
                         const wxPoint& pos, const wxSize& size, long style, const wxString& title )
{
    if( !wxDialog::Create( parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE ) ) return false;

    m_pparent = parent;

    m_pCM93DetailSlider = new wxSlider( this, id, value, minValue, maxValue, wxPoint( 0, 0 ),
                                        wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS, wxDefaultValidator,
                                        title );

    m_pCM93DetailSlider->SetSize( wxSize( 200, -1 ) );

    m_pCM93DetailSlider->InvalidateBestSize();
    wxSize bs = m_pCM93DetailSlider->GetBestSize();

    m_pCM93DetailSlider->SetSize( wxSize( 200, bs.y ) );
    Fit();

    m_pCM93DetailSlider->SetValue( g_cm93_zoom_factor );

    Hide();

    return true;
}

void CM93DSlide::OnCancelClick( wxCommandEvent& event )
{
    g_bShowCM93DetailSlider = false;
    Close();
}

void CM93DSlide::OnClose( wxCloseEvent& event )
{
    g_bShowCM93DetailSlider = false;

    Destroy();
    pCM93DetailSlider = NULL;
}

void CM93DSlide::OnMove( wxMoveEvent& event )
{
    //    Record the dialog position
    wxPoint p = event.GetPosition();
    g_cm93detail_dialog_x = p.x;
    g_cm93detail_dialog_y = p.y;

    event.Skip();
}

void CM93DSlide::OnChangeValue( wxScrollEvent& event )
{
    g_cm93_zoom_factor = m_pCM93DetailSlider->GetValue();

    ::wxBeginBusyCursor();

    cc1->ReloadVP();
    cc1->Refresh( false );

    ::wxEndBusyCursor();
}

//-------------------------------------------------------------------------------
//
//    Go To Position Dialog Implementation
//
//-------------------------------------------------------------------------------
/*!
 * GoToPositionDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( GoToPositionDialog, wxDialog )
/*!
 * GoToPositionDialog event table definition
 */BEGIN_EVENT_TABLE( GoToPositionDialog, wxDialog )

////@begin GoToPositionDialog event table entries

    EVT_BUTTON( ID_GOTOPOS_CANCEL, GoToPositionDialog::OnGoToPosCancelClick )
    EVT_BUTTON( ID_GOTOPOS_OK, GoToPositionDialog::OnGoToPosOkClick )
    EVT_COMMAND(ID_LATCTRL, EVT_LLCHANGE, GoToPositionDialog::OnPositionCtlUpdated)
    EVT_COMMAND(ID_LONCTRL, EVT_LLCHANGE, GoToPositionDialog::OnPositionCtlUpdated)

////@end GoToPositionDialog event table entries

END_EVENT_TABLE()

/*!
 * GoToPositionDialog constructors
 */

GoToPositionDialog::GoToPositionDialog()
{
}

GoToPositionDialog::GoToPositionDialog( wxWindow* parent, wxWindowID id, const wxString& caption,
                                        const wxPoint& pos, const wxSize& size, long style )
{

    long wstyle = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;
    wxDialog::Create( parent, id, caption, pos, size, wstyle );

    CreateControls();
    GetSizer()->SetSizeHints( this );
    Centre();

}

GoToPositionDialog::~GoToPositionDialog()
{
    delete m_MarkLatCtl;
    delete m_MarkLonCtl;
}

/*!
 * GoToPositionDialog creator
 */

bool GoToPositionDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption,
                                 const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->SetSizeHints( this );
    Centre();

    return TRUE;
}

/*!
 * Control creation for GoToPositionDialog
 */

void GoToPositionDialog::CreateControls()
{
    GoToPositionDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox( itemDialog1, wxID_ANY,
            _("Position") );

    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer( itemStaticBoxSizer4Static,
            wxVERTICAL );
    itemBoxSizer2->Add( itemStaticBoxSizer4, 0, wxEXPAND | wxALL, 5 );

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Latitude"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer4->Add( itemStaticText5, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE, 5 );

    m_MarkLatCtl = new LatLonTextCtrl( itemDialog1, ID_LATCTRL, _T(""), wxDefaultPosition,
                                       wxSize( 180, -1 ), 0 );
    itemStaticBoxSizer4->Add( m_MarkLatCtl, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5 );

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Longitude"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer4->Add( itemStaticText6, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE, 5 );

    m_MarkLonCtl = new LatLonTextCtrl( itemDialog1, ID_LONCTRL, _T(""), wxDefaultPosition,
                                       wxSize( 180, -1 ), 0 );
    itemStaticBoxSizer4->Add( m_MarkLonCtl, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5 );

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5 );

    m_CancelButton = new wxButton( itemDialog1, ID_GOTOPOS_CANCEL, _("Cancel"), wxDefaultPosition,
                                   wxDefaultSize, 0 );
    itemBoxSizer16->Add( m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    m_OKButton = new wxButton( itemDialog1, ID_GOTOPOS_OK, _("OK"), wxDefaultPosition,
                               wxDefaultSize, 0 );
    itemBoxSizer16->Add( m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
    m_OKButton->SetDefault();

    SetColorScheme( (ColorScheme) 0 );
}

void GoToPositionDialog::SetColorScheme( ColorScheme cs )
{
    DimeControl( this );
}

bool GoToPositionDialog::ShowToolTips()
{
    return TRUE;
}

void GoToPositionDialog::OnGoToPosCancelClick( wxCommandEvent& event )
{
    Hide();
    cc1->ReloadVP();

    event.Skip();
}

void GoToPositionDialog::OnGoToPosOkClick( wxCommandEvent& event )
{
    double lat, lon;

    if( m_MarkLatCtl->GetValue().Length() == 0 ) goto noGo;
    if( m_MarkLonCtl->GetValue().Length() == 0 ) goto noGo;

    lat = fromDMM( m_MarkLatCtl->GetValue() );
    lon = fromDMM( m_MarkLonCtl->GetValue() );

    if( lat == 0.0 && lon == 0.0 ) goto noGo;
    if( lat > 80.0 || lat < -80.0 ) goto noGo;
    if( lon > 180.0 || lon < -180.0 ) goto noGo;

    gFrame->JumpToPosition( lat, lon, cc1->GetVPScale() );
    Hide();
    event.Skip();
    return;

    noGo:
    wxBell();
    event.Skip();
    return;
}

void GoToPositionDialog::CheckPasteBufferForPosition() {
    if( wxTheClipboard->Open() ) {
        wxTextDataObject data;
        wxTheClipboard->GetData( data );
        wxString pasteBuf = data.GetText();

        PositionParser pparse( pasteBuf );

        if( pparse.IsOk() ) {
            m_MarkLatCtl->SetValue( pparse.GetLatitudeString() );
            m_MarkLonCtl->SetValue( pparse.GetLongitudeString() );
        }
        wxTheClipboard->Close();
    }
}

void GoToPositionDialog::OnPositionCtlUpdated( wxCommandEvent& event )
{
    // We do not want to change the position on lat/lon now
}

#ifdef __WXGTK__
#define BRIGHT_XCALIB
#define __OPCPN_USEICC__
#endif

//--------------------------------------------------------------------------------------------------------
//    Screen Brightness Control Support Routines
//
//--------------------------------------------------------------------------------------------------------

#ifdef __OPCPN_USEICC__
int CreateSimpleICCProfileFile(const char *file_name, double co_red, double co_green, double co_blue);

wxString temp_file_name;
#endif

#if 0
class ocpnCurtain: public wxDialog
{
    DECLARE_CLASS( ocpnCurtain )
    DECLARE_EVENT_TABLE()

public:
    ocpnCurtain( wxWindow *parent, wxPoint position, wxSize size, long wstyle );
    ~ocpnCurtain( );
    bool ProcessEvent(wxEvent& event);

};

IMPLEMENT_CLASS ( ocpnCurtain, wxDialog )

BEGIN_EVENT_TABLE(ocpnCurtain, wxDialog)
END_EVENT_TABLE()

ocpnCurtain::ocpnCurtain( wxWindow *parent, wxPoint position, wxSize size, long wstyle )
{
    wxDialog::Create( parent, -1, _T("ocpnCurtain"), position, size, wxNO_BORDER | wxSTAY_ON_TOP );
}

ocpnCurtain::~ocpnCurtain()
{
}

bool ocpnCurtain::ProcessEvent(wxEvent& event)
{
    GetParent()->GetEventHandler()->SetEvtHandlerEnabled(true);
    return GetParent()->GetEventHandler()->ProcessEvent(event);
}
#endif

#ifdef __WIN32__
#include <windows.h>

HMODULE hGDI32DLL;
typedef BOOL (WINAPI *SetDeviceGammaRamp_ptr_type)( HDC hDC, LPVOID lpRampTable );
typedef BOOL (WINAPI *GetDeviceGammaRamp_ptr_type)( HDC hDC, LPVOID lpRampTable );
SetDeviceGammaRamp_ptr_type g_pSetDeviceGammaRamp;            // the API entry points in the dll
GetDeviceGammaRamp_ptr_type g_pGetDeviceGammaRamp;

WORD *g_pSavedGammaMap;

#endif

int InitScreenBrightness( void )
{
#ifdef __WIN32__
    if( g_bopengl ) {
        HDC hDC;
        BOOL bbr;

        if( NULL == hGDI32DLL ) {
            hGDI32DLL = LoadLibrary( TEXT("gdi32.dll") );

            if( NULL != hGDI32DLL ) {
                //Get the entry points of the required functions
                g_pSetDeviceGammaRamp = (SetDeviceGammaRamp_ptr_type) GetProcAddress( hGDI32DLL,
                                        "SetDeviceGammaRamp" );
                g_pGetDeviceGammaRamp = (GetDeviceGammaRamp_ptr_type) GetProcAddress( hGDI32DLL,
                                        "GetDeviceGammaRamp" );

                //    If the functions are not found, unload the DLL and return false
                if( ( NULL == g_pSetDeviceGammaRamp ) || ( NULL == g_pGetDeviceGammaRamp ) ) {
                    FreeLibrary( hGDI32DLL );
                    hGDI32DLL = NULL;
                    return 0;
                }
            }
        }

        //    Interface is ready, so....
        //    Get some storage
        if( !g_pSavedGammaMap ) {
            g_pSavedGammaMap = (WORD *) malloc( 3 * 256 * sizeof(WORD) );

            hDC = GetDC( NULL );                                      // Get the full screen DC
            bbr = g_pGetDeviceGammaRamp( hDC, g_pSavedGammaMap );    // Get the existing ramp table
            ReleaseDC( NULL, hDC );                                       // Release the DC
        }

        //    On Windows hosts, try to adjust the registry to allow full range setting of Gamma table
        //    This is an undocumented Windows hack.....
        wxRegKey *pRegKey =
            new wxRegKey(
            _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ICM") );
        if( !pRegKey->Exists() ) pRegKey->Create();
        pRegKey->SetValue( _T("GdiIcmGammaRange"), 256 );

        g_brightness_init = true;
        return 1;
    }

    else {
        if( NULL == g_pcurtain ) {
            if( gFrame->CanSetTransparent() ) {
                //    Build the curtain window
                g_pcurtain = new wxDialog( cc1, -1, _T(""), wxPoint( 0, 0 ), ::wxGetDisplaySize(),
                                           wxNO_BORDER | wxTRANSPARENT_WINDOW | wxSTAY_ON_TOP | wxDIALOG_NO_PARENT );

                //                  g_pcurtain = new ocpnCurtain(gFrame, wxPoint(0,0),::wxGetDisplaySize(),
                //                      wxNO_BORDER | wxTRANSPARENT_WINDOW |wxSTAY_ON_TOP | wxDIALOG_NO_PARENT);

                g_pcurtain->Hide();

                HWND hWnd = GetHwndOf(g_pcurtain);
                SetWindowLong( hWnd, GWL_EXSTYLE,
                               GetWindowLong( hWnd, GWL_EXSTYLE ) | ~WS_EX_APPWINDOW );
                g_pcurtain->SetBackgroundColour( wxColour( 0, 0, 0 ) );
                g_pcurtain->SetTransparent( 0 );

                g_pcurtain->Maximize();
                g_pcurtain->Show();

                //    All of this is obtuse, but necessary for Windows...
                g_pcurtain->Enable();
                g_pcurtain->Disable();

                gFrame->Disable();
                gFrame->Enable();
                cc1->SetFocus();

            }
        }
        g_brightness_init = true;

        return 1;
    }
#else
    //    Look for "xcalib" application
    wxString cmd ( _T ( "xcalib -version" ) );

    wxArrayString output;
    long r = wxExecute ( cmd, output );
    if(0 != r)
        wxLogMessage(_("   External application \"xcalib\" not found. Screen brightness not changed."));

    g_brightness_init = true;
    return 0;
#endif
}

int RestoreScreenBrightness( void )
{
#ifdef __WIN32__

    if( g_pSavedGammaMap ) {
        HDC hDC = GetDC( NULL );                                 // Get the full screen DC
        g_pSetDeviceGammaRamp( hDC, g_pSavedGammaMap );          // Restore the saved ramp table
        ReleaseDC( NULL, hDC );                                  // Release the DC

        free( g_pSavedGammaMap );
        g_pSavedGammaMap = NULL;
    }

    if( g_pcurtain ) {
        g_pcurtain->Close();
        g_pcurtain->Destroy();
        g_pcurtain = NULL;
    }

    g_brightness_init = false;
    return 1;

#endif

#ifdef BRIGHT_XCALIB
    if(g_brightness_init)
    {
        wxString cmd;
        cmd = _T("xcalib -clear");
        wxExecute(cmd, wxEXEC_ASYNC);
        g_brightness_init = false;
    }

    return 1;
#endif

    return 0;
}

//    Set brightness. [0..100]
int SetScreenBrightness( int brightness )
{
#ifdef __WIN32__

    //    Under Windows, we use the SetDeviceGammaRamp function which exists in some (most modern?) versions of gdi32.dll
    //    Load the required library dll, if not already in place
    if( g_bopengl ) {
        if( g_pcurtain ) {
            g_pcurtain->Close();
            g_pcurtain->Destroy();
            g_pcurtain = NULL;
        }

        InitScreenBrightness();

        if( NULL == hGDI32DLL ) {
            // Unicode stuff.....
            wchar_t wdll_name[80];
            MultiByteToWideChar( 0, 0, "gdi32.dll", -1, wdll_name, 80 );
            LPCWSTR cstr = wdll_name;

            hGDI32DLL = LoadLibrary( cstr );

            if( NULL != hGDI32DLL ) {
                //Get the entry points of the required functions
                g_pSetDeviceGammaRamp = (SetDeviceGammaRamp_ptr_type) GetProcAddress( hGDI32DLL,
                                        "SetDeviceGammaRamp" );
                g_pGetDeviceGammaRamp = (GetDeviceGammaRamp_ptr_type) GetProcAddress( hGDI32DLL,
                                        "GetDeviceGammaRamp" );

                //    If the functions are not found, unload the DLL and return false
                if( ( NULL == g_pSetDeviceGammaRamp ) || ( NULL == g_pGetDeviceGammaRamp ) ) {
                    FreeLibrary( hGDI32DLL );
                    hGDI32DLL = NULL;
                    return 0;
                }
            }
        }

        HDC hDC = GetDC( NULL );                          // Get the full screen DC

        /*
         int cmcap = GetDeviceCaps(hDC, COLORMGMTCAPS);
         if (cmcap != CM_GAMMA_RAMP)
         {
         wxLogMessage(_T("    Video hardware does not support brightness control by gamma ramp adjustment."));
         return false;
         }
         */

        int increment = brightness * 256 / 100;

        // Build the Gamma Ramp table
        WORD GammaTable[3][256];

        int table_val = 0;
        for( int i = 0; i < 256; i++ ) {

            GammaTable[0][i] = r_gamma_mult * (WORD) table_val;
            GammaTable[1][i] = g_gamma_mult * (WORD) table_val;
            GammaTable[2][i] = b_gamma_mult * (WORD) table_val;

            table_val += increment;

            if( table_val > 65535 ) table_val = 65535;

        }

        g_pSetDeviceGammaRamp( hDC, GammaTable );          // Set the ramp table
        ReleaseDC( NULL, hDC );                                     // Release the DC

        return 1;
    } else {
        if( g_pSavedGammaMap ) {
            HDC hDC = GetDC( NULL );                                       // Get the full screen DC
            g_pSetDeviceGammaRamp( hDC, g_pSavedGammaMap );          // Restore the saved ramp table
            ReleaseDC( NULL, hDC );                                             // Release the DC
        }

        if( NULL == g_pcurtain ) InitScreenBrightness();

        if( g_pcurtain ) {
            int sbrite = wxMax(1, brightness);
            sbrite = wxMin(100, sbrite);

            g_pcurtain->SetTransparent( ( 100 - sbrite ) * 256 / 100 );
        }
        return 1;
    }

#endif

#ifdef BRIGHT_XCALIB

    if(!g_brightness_init)
    {
        last_brightness = 100;
        g_brightness_init = true;
        temp_file_name = wxFileName::CreateTempFileName(_T(""));
        InitScreenBrightness();
    }

#ifdef __OPCPN_USEICC__
    //  Create a dead simple temporary ICC profile file, with gamma ramps set as desired,
    //  and then activate this temporary profile using xcalib <filename>
    if(!CreateSimpleICCProfileFile ( ( const char * ) temp_file_name.fn_str(),
                                     brightness * r_gamma_mult,
                                     brightness * g_gamma_mult,
                                     brightness * b_gamma_mult ))
    {
        wxString cmd ( _T ( "xcalib " ) );
        cmd += temp_file_name;

        wxExecute ( cmd, wxEXEC_ASYNC );
    }

#else
    //    Or, use "xcalib -co" to set overall contrast value
    //    This is not as nice, since the -co parameter wants to be a fraction of the current contrast,
    //    and values greater than 100 are not allowed.  As a result, increases of contrast must do a "-clear" step
    //    first, which produces objectionable flashing.
    if(brightness > last_brightness)
    {
        wxString cmd;
        cmd = _T("xcalib -clear");
        wxExecute(cmd, wxEXEC_ASYNC);

        ::wxMilliSleep(10);

        int brite_adj = wxMax(1, brightness);
        cmd.Printf(_T("xcalib -co %2d -a"), brite_adj);
        wxExecute(cmd, wxEXEC_ASYNC);
    }
    else
    {
        int brite_adj = wxMax(1, brightness);
        int factor = (brite_adj * 100) / last_brightness;
        factor = wxMax(1, factor);
        wxString cmd;
        cmd.Printf(_T("xcalib -co %2d -a"), factor);
        wxExecute(cmd, wxEXEC_ASYNC);
    }

#endif

    last_brightness = brightness;

#endif

    return 0;
}

#ifdef __OPCPN_USEICC__

#define MLUT_TAG     0x6d4c5554L
#define VCGT_TAG     0x76636774L

int GetIntEndian(unsigned char *s)
{
    int ret;
    unsigned char *p;
    int i;

    p = (unsigned char *)&ret;

    if(1)
        for(i=sizeof(int)-1; i>-1; --i)
            *p++ = s[i];
    else
        for(i=0; i<(int)sizeof(int); ++i)
            *p++ = s[i];

    return ret;
}

unsigned short GetShortEndian(unsigned char *s)
{
    unsigned short ret;
    unsigned char *p;
    int i;

    p = (unsigned char *)&ret;

    if(1)
        for(i=sizeof(unsigned short)-1; i>-1; --i)
            *p++ = s[i];
    else
        for(i=0; i<(int)sizeof(unsigned short); ++i)
            *p++ = s[i];

    return ret;
}

//    Create a very simple Gamma correction file readable by xcalib
int CreateSimpleICCProfileFile(const char *file_name, double co_red, double co_green, double co_blue)
{
    FILE *fp;

    if(file_name)
    {
        fp = fopen(file_name, "wb");
        if(!fp)
            return -1; /* file can not be created */
    }
    else
        return -1; /* filename char pointer not valid */

    //    Write header
    char header[128];
    for(int i=0; i< 128; i++)
        header[i] = 0;

    fwrite(header, 128, 1, fp);

    //    Num tags
    int numTags0 = 1;
    int numTags = GetIntEndian((unsigned char *)&numTags0);
    fwrite(&numTags, 1, 4, fp);

    int tagName0 = VCGT_TAG;
    int tagName = GetIntEndian((unsigned char *)&tagName0);
    fwrite(&tagName, 1, 4, fp);

    int tagOffset0 = 128 + 4 * sizeof(int);
    int tagOffset = GetIntEndian((unsigned char *)&tagOffset0);
    fwrite(&tagOffset, 1, 4, fp);

    int tagSize0 = 1;
    int tagSize = GetIntEndian((unsigned char *)&tagSize0);
    fwrite(&tagSize, 1, 4, fp);

    fwrite(&tagName, 1, 4, fp);// another copy of tag

    fwrite(&tagName, 1, 4, fp);// dummy

    //  Table type

    /* VideoCardGammaTable (The simplest type) */
    int gammatype0 = 0;
    int gammatype = GetIntEndian((unsigned char *)&gammatype0);
    fwrite(&gammatype, 1, 4, fp);

    int numChannels0 = 3;
    unsigned short numChannels = GetShortEndian((unsigned char *)&numChannels0);
    fwrite(&numChannels, 1, 2, fp);

    int numEntries0 = 256;
    unsigned short numEntries = GetShortEndian((unsigned char *)&numEntries0);
    fwrite(&numEntries, 1, 2, fp);

    int entrySize0 = 1;
    unsigned short entrySize = GetShortEndian((unsigned char *)&entrySize0);
    fwrite(&entrySize, 1, 2, fp);

    unsigned char ramp[256];

    //    Red ramp
    for(int i=0; i< 256; i++)
        ramp[i] = i * co_red/100.;
    fwrite(ramp, 256, 1, fp);

    //    Green ramp
    for(int i=0; i< 256; i++)
        ramp[i] = i * co_green/100.;
    fwrite(ramp, 256, 1, fp);

    //    Blue ramp
    for(int i=0; i< 256; i++)
        ramp[i] = i * co_blue/100.;
    fwrite(ramp, 256, 1, fp);

    fclose(fp);

    return 0;
}
#endif // __OPCPN_USEICC__

void DimeControl( wxWindow* ctrl )
{
    if( NULL == ctrl ) return;

    wxColour col, col1, gridline, uitext, udkrd, back_color, text_color;
    col = GetGlobalColor( _T("DILG0") );       // Dialog Background white
    col1 = GetGlobalColor( _T("DILG1") );      // Dialog Background
    back_color = GetGlobalColor( _T("DILG1") );      // Control Background
    text_color = GetGlobalColor( _T("DILG3") );      // Text
    uitext = GetGlobalColor( _T("UITX1") );    // Menu Text, derived from UINFF
    udkrd = GetGlobalColor( _T("UDKRD") );
    gridline = GetGlobalColor( _T("GREY2") );

    DimeControl( ctrl, col, col1, back_color, text_color, uitext, udkrd, gridline );
}

void DimeControl( wxWindow* ctrl, wxColour col, wxColour col1, wxColour back_color,
                  wxColour text_color, wxColour uitext, wxColour udkrd, wxColour gridline )
{
    ColorScheme cs = cc1->GetColorScheme();
    if( cs != GLOBAL_COLOR_SCHEME_DAY && cs != GLOBAL_COLOR_SCHEME_RGB ) ctrl->SetBackgroundColour(
            back_color );
    else
        ctrl->SetBackgroundColour( wxNullColour );

    wxWindowList kids = ctrl->GetChildren();
    for( unsigned int i = 0; i < kids.GetCount(); i++ ) {
        wxWindowListNode *node = kids.Item( i );
        wxWindow *win = node->GetData();

        if( win->IsKindOf( CLASSINFO(wxListBox) ) )
            ( (wxListBox*) win )->SetBackgroundColour( col );

        if( win->IsKindOf( CLASSINFO(wxListCtrl) ) )
            ( (wxListCtrl*) win )->SetBackgroundColour( col1 );

        if( win->IsKindOf( CLASSINFO(wxTextCtrl) ) )
            ( (wxTextCtrl*) win )->SetBackgroundColour( col );

        if( win->IsKindOf( CLASSINFO(wxStaticText) ) )
            ( (wxStaticText*) win )->SetForegroundColour( uitext );

        else if( win->IsKindOf( CLASSINFO(wxBitmapComboBox) ) ) {
#if wxCHECK_VERSION(2,9,0)
            ((wxBitmapComboBox*)win)->GetTextCtrl()->SetBackgroundColour(col);
#else
            ( (wxBitmapComboBox*) win )->SetBackgroundColour( col );
#endif
        }

        else if( win->IsKindOf( CLASSINFO(wxChoice) ) )
            ( (wxChoice*) win )->SetBackgroundColour( col );

        else if( win->IsKindOf( CLASSINFO(wxComboBox) ) )
            ( (wxComboBox*) win )->SetBackgroundColour( col );

        else if( win->IsKindOf( CLASSINFO(wxScrolledWindow) ) )
            ( (wxScrolledWindow*) win )->SetBackgroundColour( col1 );

        else if( win->IsKindOf( CLASSINFO(wxGenericDirCtrl) ) )
            ( (wxGenericDirCtrl*) win )->SetBackgroundColour( col1 );

        else if( win->IsKindOf( CLASSINFO(wxListbook) ) )
            ( (wxListbook*) win )->SetBackgroundColour( col1 );

        else if( win->IsKindOf( CLASSINFO(wxTreeCtrl) ) )
            ( (wxTreeCtrl*) win )->SetBackgroundColour( col );

        else if( win->IsKindOf( CLASSINFO(wxRadioButton) ) )
            ( (wxRadioButton*) win )->SetBackgroundColour( col1 );

        else if( win->IsKindOf( CLASSINFO(wxNotebook) ) ) {
            ( (wxNotebook*) win )->SetBackgroundColour( col1 );
            ( (wxNotebook*) win )->SetForegroundColour( text_color );
        }

        else if( win->IsKindOf( CLASSINFO(wxButton) ) ) {
            ( (wxButton*) win )->SetBackgroundColour( col1 );
        }

        else if( win->IsKindOf( CLASSINFO(wxToggleButton) ) ) {
            ( (wxToggleButton*) win )->SetBackgroundColour( col1 );
        }

        else if( win->IsKindOf( CLASSINFO(wxPanel) ) ) {
//                  ((wxPanel*)win)->SetBackgroundColour(col1);
            if( cs != GLOBAL_COLOR_SCHEME_DAY
                    && cs != GLOBAL_COLOR_SCHEME_RGB ) ( (wxPanel*) win )->SetBackgroundColour(
                            back_color );
            else
                ( (wxPanel*) win )->SetBackgroundColour(
                    wxNullColour );
        }

        else if( win->IsKindOf( CLASSINFO(wxHtmlWindow) ) ) {
            if( cs != GLOBAL_COLOR_SCHEME_DAY
                    && cs != GLOBAL_COLOR_SCHEME_RGB ) ( (wxPanel*) win )->SetBackgroundColour(
                            back_color );
            else
                ( (wxPanel*) win )->SetBackgroundColour(
                    wxNullColour );

        }

        else if( win->IsKindOf( CLASSINFO(wxGrid) ) ) {
            ( (wxGrid*) win )->SetDefaultCellBackgroundColour(
                col1 );
            ( (wxGrid*) win )->SetDefaultCellTextColour(
                uitext );
            ( (wxGrid*) win )->SetLabelBackgroundColour(
                col );
            ( (wxGrid*) win )->SetLabelTextColour(
                uitext );
            ( (wxGrid*) win )->SetDividerPen(
                wxPen( col ) );
            ( (wxGrid*) win )->SetGridLineColour(
                gridline );
        }

        else {
            ;
        }

        if( win->GetChildren().GetCount() > 0 ) {
            wxWindow * w = win;
            DimeControl( w, col, col1, back_color, text_color, uitext, udkrd, gridline );
        }
    }
}
//---------------------------------------------------------------------------------------
//          S57QueryDialog Implementation
//---------------------------------------------------------------------------------------
IMPLEMENT_CLASS ( S57QueryDialog, wxDialog )
// S57QueryDialog event table definition
BEGIN_EVENT_TABLE ( S57QueryDialog, wxDialog )  //ws wxDialog
    EVT_SIZE ( S57QueryDialog::OnSize )
    EVT_CLOSE( S57QueryDialog::OnClose)
END_EVENT_TABLE()

S57QueryDialog::S57QueryDialog()
{
    Init();
}

S57QueryDialog::S57QueryDialog( wxWindow* parent, wxWindowID id, const wxString& caption,
                                const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );

}

S57QueryDialog::~S57QueryDialog()
{
    g_S57_dialog_sx = GetSize().x;
    g_S57_dialog_sy = GetSize().y;

}

void S57QueryDialog::Init()
{
}

bool S57QueryDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption,
                             const wxPoint& pos, const wxSize& size, long style )
{
    //    As a display optimization....
    //    if current color scheme is other than DAY,
    //    Then create the dialog ..WITHOUT.. borders and title bar.
    //    This way, any window decorations set by external themes, etc
    //    will not detract from night-vision

    long wstyle = wxDEFAULT_FRAME_STYLE;
    if( ( global_color_scheme != GLOBAL_COLOR_SCHEME_DAY )
            && ( global_color_scheme != GLOBAL_COLOR_SCHEME_RGB ) ) wstyle |= ( wxNO_BORDER );

    if( !wxDialog::Create( parent, id, caption, pos, size, wstyle ) ) return false;

    wxFont *dFont = wxTheFontList->FindOrCreateFont( 10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                    wxFONTWEIGHT_NORMAL );

    SetFont( *dFont );
    CreateControls();

// This ensures that the dialog cannot be sized smaller
// than the minimum size
    GetSizer()->SetSizeHints( this );

// Explicitely set the size
    SetSize( size );

// Centre the dialog on the parent or (if none) screen
    Centre();

    DimeControl( this );
    return true;

}

void S57QueryDialog::CreateControls()
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( topSizer );

    m_phtml = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                wxHW_SCROLLBAR_AUTO );
    m_phtml->SetBorders( 5 );

    m_phtml->SetMinSize( wxSize( 100, 100 ) );            // this will constrain the dialog, too
    topSizer->Add( m_phtml, 1, wxBOTTOM | wxEXPAND, 10 );

    topSizer->FitInside( this );

    wxSizer* ok = CreateButtonSizer( wxOK );
    topSizer->Add( ok, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5 );
}

void S57QueryDialog::SetColorScheme( void )
{
    DimeControl( this );
    wxColor bg = GetBackgroundColour();
    m_phtml->SetBackgroundColour( bg );

}

void S57QueryDialog::SetHTMLPage( wxString& page )
{
    m_phtml->SetPage( page );
    SetColorScheme();
}

void S57QueryDialog::OnSize( wxSizeEvent& event )
{
    g_S57_dialog_sx = GetSize().x;
    g_S57_dialog_sy = GetSize().y;
    wxDialog::OnSize( event );
}

void S57QueryDialog::OnClose( wxCloseEvent& event )
{
    g_S57_dialog_sx = GetSize().x;
    g_S57_dialog_sy = GetSize().y;
    Destroy();
    g_pObjectQueryDialog = NULL;
}

