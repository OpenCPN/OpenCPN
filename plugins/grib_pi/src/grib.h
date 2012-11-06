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
#include <wx/treectrl.h>
#include <wx/fileconf.h>
#include <wx/notebook.h>
#include <wx/glcanvas.h>

#include "GribReader.h"
#include "GribRecord.h"
#include "IsoLine.h"

#define ID_OK                       10001
#define ID_GRIBRECORDREECTRL        10002
#define ID_CHOOSEGRIBDIR            10003

#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

enum {
    ID_CB_WINDSPEED = 11000, ID_CB_WINDDIR, ID_CB_PRESS, ID_CB_SIGHW, ID_CB_SEATMP, ID_CB_SEACURRENT
};

enum OVERLAP {
    _IN, _ON, _OUT
};

enum {
    GENERIC_GRAPHIC_INDEX, CURRENT_GRAPHIC_INDEX, SEATEMP_GRAPHIC_INDEX, CRAIN_GRAPHIC_INDEX
};

class GRIBFile;
class GRIBRecord;
class GribRecordTree;
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

enum GribTreeItemType {
    GRIB_FILE_TYPE, GRIB_RECORD_SET_TYPE
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

    bool Create( wxWindow *parent, grib_pi *ppi, wxWindowID id = wxID_ANY, const wxString& caption =
            _("GRIB Display Control"), const wxString initial_grib_dir = wxT(""),
            const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
            long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU );

    void CreateControls();

    void PopulateTreeControlGRS( GRIBFile *pgribfile, int file_index );
    void SetGribRecordSet( GribRecordSet *pGribRecordSet ); // a "notification" from Record Tree control

    void SetCursorLatLon( double lat, double lon );

private:
    void OnClose( wxCloseEvent& event );
    void OnIdOKClick( wxCommandEvent& event );
    void OnMove( wxMoveEvent& event );
    void OnSize( wxSizeEvent& event );
    void OnChooseDirClick( wxCommandEvent& event );
    void UpdateTrackingControls( void );
    void PopulateTreeControl( void );
    void SetFactoryOptions();

    void OnCBWindspeedClick( wxCommandEvent& event );
    void OnCBWinddirClick( wxCommandEvent& event );
    void OnCBPressureClick( wxCommandEvent& event );
    void OnCBSigHwClick( wxCommandEvent& event );
    void OnCBSeatempClick( wxCommandEvent& event );
    void OnCBSeaCurrentClick( wxCommandEvent& event );

    //    Data
    wxWindow *pParent;
    grib_pi *pPlugIn;

    wxFont *m_dFont;

    GribRecordTree *m_pRecordTree;
    wxTreeItemId m_RecordTree_root_id;

    wxTextCtrl *m_pWindSpeedTextCtrl;
    wxTextCtrl *m_pWindDirTextCtrl;
    wxTextCtrl *m_pPressureTextCtrl;
    wxTextCtrl *m_pSigWHTextCtrl;
    wxTextCtrl *m_pSeaTmpTextCtrl;
    wxTextCtrl *m_pSeaCurrentTextCtrl;

    wxCheckBox m_cbWindSpeed;
    wxCheckBox m_cbWindDir;
    wxCheckBox m_cbPress;
    wxCheckBox m_cbSigHw;
    wxCheckBox m_cbSeaTmp;
    wxCheckBox m_cbSeaCurrent;

    int m_n_files;

    wxTextCtrl *m_pitemCurrentGribDirectoryCtrl;
    wxString m_currentGribDir;
    wxBitmap *m_pfolder_bitmap;
    GribRecordSet *m_pCurrentGribRecordSet;

    int m_sequence_active;

    double m_cursor_lat, m_cursor_lon;

    int m_RS_Idx_WIND_VX;             // These are indexes into the m_pCurrentGribRecordSet
    int m_RS_Idx_WIND_VY;
    int m_RS_Idx_PRESS;
    int m_RS_Idx_HTSIGW;
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

    void EnableRenderWind( bool b_rend )
    {
        m_ben_Wind = b_rend;
    }
    void EnableRenderPressure( bool b_rend )
    {
        m_ben_Pressure = b_rend;
    }
    void EnableRenderSigHw( bool b_rend )
    {
        m_ben_SigHw = b_rend;
    }
    void EnableRenderQuickscat( bool b_rend )
    {
        m_ben_Quickscat = b_rend;
    }
    void EnableRenderSeatmp( bool b_rend )
    {
        m_ben_Seatmp = b_rend;
    }
    void EnableRenderSeaCurrent( bool b_rend )
    {
        m_ben_SeaCurrent = b_rend;
    }

    void DrawGLLine( double x1, double y1, double x2, double y2, double width );
    void DrawOLBitmap( const wxBitmap &bitmap, wxCoord x, wxCoord y, bool usemask );
    void DrawGLImage( wxImage *pimage, wxCoord x, wxCoord y, bool usemask );
    void DrawMessageWindow( wxString msg, int x, int y );

private:
    bool DoRenderGribOverlay( PlugIn_ViewPort *vp );

    bool RenderGribWind( GribRecord *pGRX, GribRecord *pGRY, PlugIn_ViewPort *vp );
    bool RenderGribPressure( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribSigWh( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribWvDir( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribScatWind( GribRecord *pGRX, GribRecord *pGRY, PlugIn_ViewPort *vp );
    bool RenderGribCRAIN( GribRecord *pGR, PlugIn_ViewPort *vp );
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

    wxColour GetGenericGraphicColor( double val );
    wxColour GetQuickscatColor( double val );
    wxColour GetSeaCurrentGraphicColor( double val_in );
    wxColour GetSeaTempGraphicColor( double val );

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
    GribOverlayBitmap *m_pgob_seatemp;
    GribOverlayBitmap *m_pgob_current;

    wxDC *m_pdc;
    wxGraphicsContext *m_gdc;

    bool m_ben_Wind;
    bool m_ben_Pressure;
    bool m_ben_SigHw;
    bool m_ben_Quickscat;
    bool m_ben_Seatmp;
    bool m_ben_SeaCurrent;

    bool m_bReadyToRender;
    bool m_hiDefGraphics;
};

//----------------------------------------------------------------------------------------------------------
//    Grib File/Record selector Tree Control Specification
//----------------------------------------------------------------------------------------------------------

class GribRecordTree: public wxTreeCtrl {
DECLARE_CLASS( GribRecordTree )DECLARE_EVENT_TABLE()
public:
    // Constructors
    GribRecordTree();
    GribRecordTree( GRIBUIDialog* parent, wxWindowID id = wxID_ANY, const wxPoint& pos =
            wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS );

    ~GribRecordTree();

    void Init();

    //  Override events
    void OnItemExpanding( wxTreeEvent& event );
    void OnItemSelectChange( wxTreeEvent& event );

    //    Data
    GRIBUIDialog *m_parent;
    wxTreeItemId *m_file_id_array;             // an array of wxTreeItemIDs

};

class GribTreeItemData: public wxTreeItemData {
public:
    GribTreeItemData( const GribTreeItemType type );
    ~GribTreeItemData();

    GribTreeItemType m_type;

    //    Data for type GRIB_FILE
    wxString m_file_name;
    GRIBFile *m_pGribFile;
    int m_file_index;

    //    Data for type GRIB_RECORD
    GribRecordSet *m_pGribRecordSet;

};

//----------------------------------------------------------------------------------------------------------
//    GRIBFile Object Specification
//----------------------------------------------------------------------------------------------------------
class GRIBFile {
public:

    GRIBFile( const wxString file_name );
    ~GRIBFile();

    bool IsOK( void )
    {
        return m_bOK;
    }
    wxString GetLastErrorMessage( void )
    {
        return m_last_error_message;
    }
    ArrayOfGribRecordSets *GetRecordSetArrayPtr( void )
    {
        return &m_GribRecordSetArray;
    }

private:

    bool m_bOK;
    wxString m_last_error_message;
    GribReader *m_pGribReader;

    //    An array of GribRecordSets found in this GRIB file
    ArrayOfGribRecordSets m_GribRecordSetArray;

    int m_nGribRecords;

};

#endif

