/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin Freinds
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 */

#ifndef _GRIB_H_
#define _GRIB_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers
#include <wx/fileconf.h>
#include <wx/notebook.h>
#include <wx/glcanvas.h>

#include "GribReader.h"
#include "GribRecord.h"
#include "IsoLine.h"

#include <wx/stdpaths.h>

#include <wx/net/email.h>

//Timer Constants
#define GRIB_FORECAST_TIMER         10100
#define CANVAS_REFRESH_TIMER        10101
#define FORECAST_TIMER_10MN         600000      //10 mn
#define CANVAS_REFRESH_100MS         100         //100 ms

//Config constants
enum {          //numeric data display ( includes wind barbs and isobards )
    ID_CB_NOMAP = 0,ID_CB_WINDM, ID_CB_PRESSM, ID_CB_WIGUST, ID_CB_PRESSD, ID_CB_SIWAVD, ID_CB_PRETOD, ID_CB_CLOCVD,
    ID_CB_ATEM2D, ID_CB_SEATED, ID_CB_CURRED
};
enum {          //graphic colored data maps display 
    ID_CB_WINDSP = 97, ID_CB_WINDIR, ID_CB_WIGUSM, ID_CB_PRESS, ID_CB_SIWAVM, ID_CB_PRETOM, ID_CB_CLOCVM,
    ID_CB_ATEM2M, ID_CB_SEATEM, ID_CB_CURREM
};

#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

enum OVERLAP {
    _IN, _ON, _OUT
};

enum COLORINDEX {
    GENERIC_GRAPHIC_INDEX, CURRENT_GRAPHIC_INDEX, SEATEMP_GRAPHIC_INDEX, CRAIN_GRAPHIC_INDEX,
    TRAIN_GRAPHIC_INDEX, CLOCV_GRAPHIC_INDEX, ATEMP2_GRAPHIC_INDEX
};

class GRIBFile;
class GRIBRecord;
class GRIBOverlayFactory;
class GribRecordSet;

class wxFileConfig;
class grib_pi;
class wxGraphicsContext;

WX_DECLARE_OBJARRAY( GribRecordSet, ArrayOfGribRecordSets );
WX_DECLARE_OBJARRAY( GribRecord *, ArrayOfGribRecordPtrs );

OVERLAP Intersect( PlugIn_ViewPort *vp, double lat_min, double lat_max, double lon_min,
        double lon_max, double Marge );
bool PointInLLBox( PlugIn_ViewPort *vp, double x, double y );

class GribRecordSet {
public:
    time_t m_Reference_Time;
    ArrayOfGribRecordPtrs m_GribRecordPtrArray;          // all GribRecords at this time
};

//----------------------------------------------------------------------------------------------------------
//    GRIB Selector/Control Dialog Specification
//----------------------------------------------------------------------------------------------------------
class GRIBUIDialog: public wxDialog {
DECLARE_CLASS( GRIBUIDialog )DECLARE_EVENT_TABLE()
public:

    GRIBUIDialog();

    ~GRIBUIDialog();
    void Init();

    GRIBFile        *m_bGRIBActiveFile;

    bool Create( wxWindow *parent, grib_pi *ppi, wxWindowID id = wxID_ANY, const wxString& caption =
            wxT(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, 
            long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU );

    void CreateControls();

    void GetFirstrFileInDirectory();
    void PopulateComboDataList( int index );
    void CreateActiveFileFromName( wxString filename );
    void DisplayDataGRS( void );
    void SetGribRecordSet( GribRecordSet *pGribRecordSet );
    void ComputeBestForecastForNow( void );
    void SetCursorLatLon( double lat, double lon );
    void UpdateTrackingControls( void );
    void UpdateTrackingLabels( void );
    bool HasValidData(void) { return  m_pRecordForecast->GetCount() != 0; }
    int  GetActiveForecastIndex( void ) { return m_pRecordForecast->GetCurrentSelection(); }
    //wxString GetFileName(void) {return m_bGRIBActiveFile->GetFileName();}
    void SetViewPort( PlugIn_ViewPort *vp ) { m_vp = new PlugIn_ViewPort(*vp); }
    void StartRefreshTimer( void ) { m_pCanvasRefreshTimer.Start( CANVAS_REFRESH_100MS, wxTIMER_ONE_SHOT ); }
private:
    void OnClose( wxCloseEvent& event );
    void OnMove( wxMoveEvent& event );
    void OnSize( wxSizeEvent& event );
    void SetFactoryOptions();
    int  GetNumberDataShown();
    void ShowSendRequest( wxString zone );
    void ShowGribReqPrefDialog( wxString zone );
    void OnGRIBForecastTimerEvent( wxTimerEvent& event );
    void OnCanvasRefreshTimerEvent( wxTimerEvent& event );
    void OnGRIBForecastChange( wxCommandEvent& event );
    void OnButtonOpenFileClick( wxCommandEvent& event );
    void OnButtonSettingClick( wxCommandEvent& event );
    void OnButtonNextClick( wxCommandEvent& event );
    void OnButtonPrevClick( wxCommandEvent& event );
    void OnButtonNowClick( wxCommandEvent& event );
    void OnGribRequestClick(  wxCommandEvent& event );

    //    Data
    wxWindow        *pParent;
    grib_pi         *pPlugIn;
    //GRIBFile        *m_bGRIBActiveFile;
    PlugIn_ViewPort  *m_vp;
    wxString        m_DataDisplayConfig;

    wxTimer         m_pGribForecastTimer;
    wxTimer         m_pCanvasRefreshTimer;

    wxComboBox      *m_pRecordForecast;
   
    wxTextCtrl      *m_pT0TextCtrl;
    wxTextCtrl      *m_pT1TextCtrl;
    wxTextCtrl      *m_pT2TextCtrl;
    wxTextCtrl      *m_pT3TextCtrl;
    wxTextCtrl      *m_pT4TextCtrl;
    wxTextCtrl      *m_pT5TextCtrl;
    wxTextCtrl      *m_pT6TextCtrl;
    wxTextCtrl      *m_pT7TextCtrl;
    wxTextCtrl      *m_pT8TextCtrl;

    wxStaticText    *m_pT0StaticText;
    wxStaticText    *m_pT1StaticText;
    wxStaticText    *m_pT2StaticText;
    wxStaticText    *m_pT3StaticText;
    wxStaticText    *m_pT4StaticText;
    wxStaticText    *m_pT5StaticText;
    wxStaticText    *m_pT6StaticText;
    wxStaticText    *m_pT7StaticText;
    wxStaticText    *m_pT8StaticText;

    wxBitmapButton  *m_pButtonNext;
    wxBitmapButton  *m_pButtonPrev;
    wxBitmapButton  *m_pButtonNow;
    wxBitmapButton  *m_pButtonOpen;
    wxBitmapButton  *m_pButtonPref;
    wxBitmapButton  *m_pButtonRequ;

    int             m_NumberDataShown;
    int             m_height;
    bool            m_cbWindSpeed;
    bool            m_cbPress;
    bool            m_cbSigHw;
    bool            m_cbSeaTmp;
    bool            m_cbSeaCurrent;
    
    wxBitmap        *m_pNext_bitmap;
    wxBitmap        *m_pPrev_bitmap;
    wxBitmap        *m_pNow_bitmap;
    wxBitmap        *m_pOpen_bitmap;
    wxBitmap        *m_pPref_bitmap;
    wxBitmap        *m_pRequ_bitmap;

    GribRecordSet   *m_pCurrentGribRecordSet;

    double m_cursor_lat, m_cursor_lon;

    int m_RS_Idx_WIND_VX;             // These are indexes into the m_pCurrentGribRecordSet
    int m_RS_Idx_WIND_VY;
    int m_RS_Idx_WIND_GUST;
    int m_RS_Idx_PRESS;
    int m_RS_Idx_HTSIGW;
    int m_RS_Idx_PRECIP_TOT;
    int m_RS_Idx_CLOUD_TOT;
    int m_RS_Idx_AIR_TEMP_2M;
    int m_RS_Idx_WINDSCAT_VY;
    int m_RS_Idx_WINDSCAT_VX;
    int m_RS_Idx_SEATEMP;
    int m_RS_Idx_SEACURRENT_VX;
    int m_RS_Idx_SEACURRENT_VY;

};

class GribOverlayBitmap {
public:
    GribOverlayBitmap( void )
    {
        m_pDCBitmap = NULL, m_pRGBA = NULL;
    }
    ~GribOverlayBitmap( void )
    {
        delete m_pDCBitmap, delete[] m_pRGBA;
    }

    wxBitmap *m_pDCBitmap;
    unsigned char *m_pRGBA;
    int m_RGBA_width;
    int m_RGBA_height;
};

//----------------------------------------------------------------------------------------------------------
//    Grib Overlay Factory Specification
//----------------------------------------------------------------------------------------------------------
class GRIBOverlayFactory {
public:
    GRIBOverlayFactory( bool hiDefGraphics );
    ~GRIBOverlayFactory();

    void SetGribRecordSet( GribRecordSet *pGribRecordSet );
    bool RenderGribOverlay( wxDC &dc, PlugIn_ViewPort *vp );
    bool RenderGLGribOverlay( wxGLContext *pcontext, PlugIn_ViewPort *vp );
    bool IsReadyToRender()
    {
        return m_bReadyToRender;
    }
    void Reset();
    void ClearCachedData( void );

    GribRecordSet *m_pGribRecordSet;

    void SetMessage( wxString message ) { m_Message = message; }
    void SetTimeZone( int TimeZone ) { m_TimeZone = TimeZone; }
    void EnableRenderWind( bool b_rend )
    {
        m_ben_Wind = b_rend;
    }
    void EnableRenderPressure( bool b_rend )
    {
        m_ben_Pressure = b_rend;
    } 
    void EnableRenderGMap( int b_rend )
    {
        m_ben_GMap = b_rend;
    }
    void EnableRenderQuickscat( bool b_rend )
    {
        m_ben_Quickscat = b_rend;
    }
    void DrawGLLine( double x1, double y1, double x2, double y2, double width );
    void DrawOLBitmap( const wxBitmap &bitmap, wxCoord x, wxCoord y, bool usemask );
    void DrawGLImage( wxImage *pimage, wxCoord x, wxCoord y, bool usemask );
    void DrawMessageWindow( wxString msg, int x, int y , wxFont *font);

private:
    bool DoRenderGribOverlay( PlugIn_ViewPort *vp );

    bool RenderGribWind( GribRecord *pGRX, GribRecord *pGRY, PlugIn_ViewPort *vp );
    bool RenderGribPressure( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribSigWh( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribWvDir( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribScatWind( GribRecord *pGRX, GribRecord *pGRY, PlugIn_ViewPort *vp );
    //bool RenderGribCRAIN( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribTRAIN( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribCloudCover( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribAirTemp2m( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribSeaTemp( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribCurrent( GribRecord *pGRX, GribRecord *pGRY, PlugIn_ViewPort *vp );

    void drawWindArrowWithBarbs( int x, int y, double vx, double vy, bool south,
            wxColour arrowColor );
    void drawWaveArrow( int i, int j, double dir, wxColour arrowColor );
    void drawSingleArrow( int i, int j, double dir, wxColour arrowColor, int width = 1 );

    void drawTransformedLine( wxPen pen, double si, double co, int di, int dj, int i, int j, int k,
            int l );

    void drawPetiteBarbule( wxPen pen, bool south, double si, double co, int di, int dj, int b );
    void drawGrandeBarbule( wxPen pen, bool south, double si, double co, int di, int dj, int b );
    void drawTriangle( wxPen pen, bool south, double si, double co, int di, int dj, int b );

    wxString GetRefString( GribRecord *rec, int map );

    wxColour GetGenericGraphicColor( double val );
    wxColour GetQuickscatColor( double val );
    wxColour GetSeaCurrentGraphicColor( double val_in );
   // wxColour GetSeaTempGraphicColor( double val );
    wxColour GetTotRainGraphicColor( double val );
    wxColour GetCloudCoverGraphicColor( double val );
    wxColour GetTempGraphicColor( double val_in );

    void CreateRGBAfromImage( wxImage *pimage, GribOverlayBitmap *pGOB );
    void DrawGLRGBA( unsigned char *pRGBA, int RGBA_width, int RGBA_height, int xd, int yd );
    wxImage CreateGribImage( GribRecord *pGRA, GribRecord *pGRB, PlugIn_ViewPort *vp,
            int grib_pixel_size, int colormap_index, const wxPoint &porg );

    bool RenderGribFieldOverlay( GribRecord *pGRA, GribRecord *pGRB, PlugIn_ViewPort *vp,
            int grib_pixel_size, int colormap_index, GribOverlayBitmap **ppGOB );

    double m_last_vp_scale;
    wxArrayPtrVoid m_IsobarArray;

    GribOverlayBitmap *m_pgob_sigwh;
    GribOverlayBitmap *m_pgob_crain;
    GribOverlayBitmap *m_pgob_train;
    GribOverlayBitmap *m_pgob_clocv;
    GribOverlayBitmap *m_pgob_atemp2;
    GribOverlayBitmap *m_pgob_seatemp;
    GribOverlayBitmap *m_pgob_current;

    wxDC *m_pdc;
    wxGraphicsContext *m_gdc;

    wxFont *m_dFont_map;
    wxFont *m_dFont_war;

    bool m_ben_Wind;
    bool m_ben_Pressure;
    bool m_ben_Quickscat;
    int  m_ben_GMap;

    bool m_bReadyToRender;
    bool m_hiDefGraphics;
   
    int  m_TimeZone;
    wxString m_SecString;
    wxString m_Message;
};

//----------------------------------------------------------------------------------------------------------
//    GRIBFile Object Specification
//----------------------------------------------------------------------------------------------------------
class GRIBFile {
public:

    GRIBFile( const wxString file_name, bool CumRec, bool WaveRec );
    ~GRIBFile();

    bool IsOK( void )
    {
        return m_bOK;
    }
    wxString GetFileName( void )
    {
        return m_FileName;
    }
    wxString GetLastMessage( void )
    {
        return m_last_message;
    }
    ArrayOfGribRecordSets *GetRecordSetArrayPtr( void )
    {
        return &m_GribRecordSetArray;
    }

private:

    bool m_bOK;
    wxString m_last_message;
    wxString m_FileName;
    GribReader *m_pGribReader;

    //    An array of GribRecordSets found in this GRIB file
    ArrayOfGribRecordSets m_GribRecordSetArray;

    int m_nGribRecords;

};

//----------------------------------------------------------------------------------------
// Request Profile display definition
//----------------------------------------------------------------------------------------

class GribPofileDisplay : public wxDialog
{
public:
      GribPofileDisplay( wxWindow *parent, wxWindowID id, wxString profile );
      ~GribPofileDisplay() {}
      
private:
    void OnModifyButtonClick(wxCommandEvent &event);
};

//----------------------------------------------------------------------------------------
// Request Prefrences dialog definition
//----------------------------------------------------------------------------------------

class GribReqPrefDialog : public wxDialog
{
public:
      GribReqPrefDialog( wxWindow *pparent, wxWindowID id, wxString config );
      ~GribReqPrefDialog() {}

    int  GetModel() { return  m_pModel->GetCurrentSelection();}
    int  GetResolution() { return  m_pResolution->GetCurrentSelection();}
    int  GetInterval() { return  m_pInterval->GetCurrentSelection();}
    int  GetHorizon() { return  m_pHorizon->GetCurrentSelection();}
    bool  GetWaves() { return  m_pWaves->GetValue();}
    bool  GetRain() { return  m_pRainfall->GetValue();}
    bool  GetClouds() { return  m_pCloudCover->GetValue();}
    bool  GetAirTemp() { return  m_pAirTemp->GetValue();}
    bool  GetSeaTemp() { return  m_pSeaTemp->GetValue();}

private:
    void OnChoiceChange(wxCommandEvent &event);

    wxFlexGridSizer     *m_pTopSizer;
    wxString            m_DataRequestConfig;

    wxChoice            *m_pModel;
    wxChoice            *m_pResolution;
    wxChoice            *m_pInterval;
    wxChoice            *m_pHorizon;

    wxCheckBox          *m_pWind;
    wxCheckBox          *m_pPress;
    wxCheckBox          *m_pWaves;
    wxCheckBox          *m_pRainfall;
    wxCheckBox          *m_pCloudCover;
    wxCheckBox          *m_pAirTemp;
    wxCheckBox          *m_pSeaTemp;

};
#endif

