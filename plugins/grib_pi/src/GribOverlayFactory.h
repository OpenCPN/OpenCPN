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
          glDeleteTextures( 1, &m_iTexture );
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
class GribRecordSet;

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

    void SetGribRecordSet( GribRecordSet *pGribRecordSet1 );
    bool RenderGribOverlay( wxDC &dc, PlugIn_ViewPort *vp );
    bool RenderGLGribOverlay( wxGLContext *pcontext, PlugIn_ViewPort *vp );
    bool IsReadyToRender()
    {
        return m_bReadyToRender;
    }
    void Reset();
    void ClearCachedData( void );

    GribRecordSet *m_pGribRecordSet;

    void DrawGLLine( double x1, double y1, double x2, double y2, double width );
    void DrawOLBitmap( const wxBitmap &bitmap, wxCoord x, wxCoord y, bool usemask );
    void DrawGLImage( wxImage *pimage, wxCoord x, wxCoord y, bool usemask );
    void DrawMessageWindow( wxString msg, int x, int y );

private:
    bool DoRenderGribOverlay( PlugIn_ViewPort *vp );

    bool RenderGribWind( GribRecord *pGRX, GribRecord *pGRY, PlugIn_ViewPort *vp );
    wxImage &getLabel(double value);
    bool RenderGribPressure( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribWaveHeight( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribWaveDirection( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribScatWind( GribRecord *pGRX, GribRecord *pGRY, PlugIn_ViewPort *vp );
    bool RenderGribCRAIN( GribRecord *pGR, PlugIn_ViewPort *vp );
    bool RenderGribSeaTemperature( GribRecord *pGR, PlugIn_ViewPort *vp );
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

    wxColour GetGraphicColor(double val, int colormap_index);

    void DrawGLTexture( GLuint texture, int width, int height, int xd, int yd, int grib_pixel_size );
    void DrawGLRGBA( unsigned char *pRGBA, int RGBA_width, int RGBA_height, int xd, int yd );
    void CreateGribGLTexture( GribOverlay *pGO, GribRecord *pGRA, GribRecord *pGRB,
                              PlugIn_ViewPort *vp, int grib_pixel_size,
                              int colormap_index, const wxPoint &porg );
    wxImage CreateGribImage( GribRecord *pGRA, GribRecord *pGRB, PlugIn_ViewPort *vp,
            int grib_pixel_size, int colormap_index, const wxPoint &porg );

    bool RenderGribFieldOverlay( GribRecord *pGRA, GribRecord *pGRB, PlugIn_ViewPort *vp,
            int grib_pixel_size, int colormap_index, GribOverlay **ppGO );

    double m_last_vp_scale;

    GribOverlay *m_pgob_sigwh;
    GribOverlay *m_pgob_crain;
    GribOverlay *m_pgob_seatemp;
    GribOverlay *m_pgob_current;

    wxDC *m_pdc;
    wxGraphicsContext *m_gdc;

    bool m_bReadyToRender;
    bool m_hiDefGraphics;
    bool m_bGradualColors;

    std::map < double , wxImage > m_labelCache;

    GRIBUIDialog &m_dlg;
};
