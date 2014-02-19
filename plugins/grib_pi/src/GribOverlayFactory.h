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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include <map>


//----------------------------------------------------------------------------------------------------------
//    Grib Overlay Specification
//----------------------------------------------------------------------------------------------------------

class GribOverlay {
public:
    GribOverlay( void )
    {
        m_iTexture = 0;
        m_pDCBitmap = NULL, m_pRGBA = NULL;
    }

    ~GribOverlay( void )
    {
        if(m_iTexture)
#ifdef ocpnUSE_GL            
          glDeleteTextures( 1, &m_iTexture );
#endif          
        delete m_pDCBitmap, delete[] m_pRGBA;
    }

    unsigned int m_iTexture; /* opengl mode */

    wxBitmap *m_pDCBitmap; /* dc mode */
    unsigned char *m_pRGBA;

    int m_width;
    int m_height;
};

//----------------------------------------------------------------------------------------------------------
//    Grib Overlay Factory Specification
//----------------------------------------------------------------------------------------------------------

class GRIBUIDialog;
class GribRecord;
class GribTimelineRecordSet;

class GRIBOverlayFactory {
public:
    GRIBOverlayFactory( GRIBUIDialog &dlg );
    ~GRIBOverlayFactory();

    void SetSettings( bool hiDefGraphics, bool GradualColors )
    {
      m_hiDefGraphics = hiDefGraphics;
      m_bGradualColors = GradualColors;
      ClearCachedData();
    }

    void SetMessage( wxString message ) { m_Message = message; }
    void SetTimeZone( int TimeZone ) { m_TimeZone = TimeZone; }
    void SetParentSize( int w, int h ) { m_ParentSize.SetWidth(w) ; m_ParentSize.SetHeight(h) ;}

    void SetGribTimelineRecordSet( GribTimelineRecordSet *pGribTimelineRecordSet1 );
    bool RenderGribOverlay( wxDC &dc, PlugIn_ViewPort *vp );
    bool RenderGLGribOverlay( wxGLContext *pcontext, PlugIn_ViewPort *vp );

    void Reset();
    void ClearCachedData( void );

    GribTimelineRecordSet *m_pGribTimelineRecordSet;

    void DrawGLLine( double x1, double y1, double x2, double y2, double width );
    void DrawOLBitmap( const wxBitmap &bitmap, wxCoord x, wxCoord y, bool usemask );
    void DrawGLImage( wxImage *pimage, wxCoord x, wxCoord y, bool usemask );
    void DrawMessageZoomOut( PlugIn_ViewPort *vp );
    wxColour GetGraphicColor(int config, double val);

    wxSize  m_ParentSize;
private:

    bool DoRenderGribOverlay( PlugIn_ViewPort *vp );
    void RenderGribBarbedArrows( int config, GribRecord **pGR, PlugIn_ViewPort *vp );
    void RenderGribIsobar( int config, GribRecord **pGR, wxArrayPtrVoid **pIsobarArray, PlugIn_ViewPort *vp );
    void RenderGribDirectionArrows( int config, GribRecord **pGR, PlugIn_ViewPort *vp );
    void RenderGribOverlayMap( int config, GribRecord **pGR, PlugIn_ViewPort *vp);
    void RenderGribNumbers( int config, GribRecord **pGR, PlugIn_ViewPort *vp );

    wxString GetRefString( GribRecord *rec, int map );
    void DrawMessageWindow( wxString msg, int x, int y , wxFont *mfont);

    void drawWindArrowWithBarbs( int config, int x, int y, double vx, double vy,
                                 bool polar, bool south, wxColour arrowColor, double rotate_angle );
    void drawDoubleArrow( int i, int j, double dir, wxColour arrowColor, int arrowWidth, int arrowSize );
    void drawSingleArrow( int i, int j, double dir, wxColour arrowColor, int arrowWidth, int arrowSize );

    void drawTransformedLine( wxPen pen, double si, double co, int di, int dj,
                              int i, int j, int k, int l );
    void drawPetiteBarbule( wxPen pen, bool south, double si, double co, int di, int dj, int b );
    void drawGrandeBarbule( wxPen pen, bool south, double si, double co, int di, int dj, int b );
    void drawTriangle( wxPen pen, bool south, double si, double co, int di, int dj, int b );

    wxImage &getLabel(double value, int settings);

    
#ifdef ocpnUSE_GL    
    void DrawGLTexture( GLuint texture, int width, int height, int xd, int yd, int grib_pixel_size, PlugIn_ViewPort *vp );
    void DrawGLRGBA( unsigned char *pRGBA, int RGBA_width, int RGBA_height, int xd, int yd );
    bool CreateGribGLTexture( GribOverlay *pGO, int config, GribRecord *pGR,
                              PlugIn_ViewPort *vp, int grib_pixel_size );
#endif    
    wxImage CreateGribImage( int config, GribRecord *pGR, PlugIn_ViewPort *vp,
                             int grib_pixel_size, const wxPoint &porg );

    double m_last_vp_scale;

    GribOverlay *m_pOverlay[GribOverlaySettings::SETTINGS_COUNT];

    wxString m_Message;
    wxString m_Message_Hiden;
    int  m_TimeZone;

    wxDC *m_pdc;
    wxGraphicsContext *m_gdc;

    wxFont *m_dFont_map;
    wxFont *m_dFont_war;

    bool m_hiDefGraphics;
    bool m_bGradualColors;

    std::map < double , wxImage > m_labelCache;

    GRIBUIDialog &m_dlg;
    GribOverlaySettings &m_Settings;
};
