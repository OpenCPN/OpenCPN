/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Object
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
 *
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
  #include <wx/glcanvas.h>
#endif //precompiled headers

#include "GribUIDialog.h"

// Calculates if two boxes intersect. If so, the function returns _ON.
// If they do not intersect, two scenario's are possible:
// other is outside this -> return _OUT
// other is inside this -> return _IN
OVERLAP Intersect( PlugIn_ViewPort *vp, double lat_min, double lat_max, double lon_min,
        double lon_max, double Marge )
{

    if( ( ( vp->lon_min - Marge ) > ( lon_max + Marge ) )
            || ( ( vp->lon_max + Marge ) < ( lon_min - Marge ) )
            || ( ( vp->lat_max + Marge ) < ( lat_min - Marge ) )
            || ( ( vp->lat_min - Marge ) > ( lat_max + Marge ) ) ) return _OUT;

    // Check if other.bbox is inside this bbox
    if( ( vp->lon_min <= lon_min ) && ( vp->lon_max >= lon_max ) && ( vp->lat_max >= lat_max )
            && ( vp->lat_min <= lat_min ) ) return _IN;

    // Boundingboxes intersect
    return _ON;
}

// Is the given point in the vp ??
bool PointInLLBox( PlugIn_ViewPort *vp, double x, double y )
{

    if( x >= ( vp->lon_min ) && x <= ( vp->lon_max ) && y >= ( vp->lat_min )
            && y <= ( vp->lat_max ) ) return TRUE;
    return FALSE;
}

//----------------------------------------------------------------------------------------------------------
//    Grib Overlay Factory Implementation
//----------------------------------------------------------------------------------------------------------
GRIBOverlayFactory::GRIBOverlayFactory( GRIBUIDialog &dlg )
    : m_dlg(dlg)
{
    m_pGribRecordSet = NULL;
    m_last_vp_scale = 0.;

    m_bReadyToRender = false;

    m_pgob_sigwh = NULL;
    m_pgob_crain = NULL;
    m_pgob_seatemp = NULL;
    m_pgob_current = NULL;
}

GRIBOverlayFactory::~GRIBOverlayFactory()
{
    delete m_pgob_sigwh;
    delete m_pgob_crain;
    delete m_pgob_seatemp;
    delete m_pgob_current;
}

void GRIBOverlayFactory::Reset()
{
    m_pGribRecordSet = NULL;

    ClearCachedData();

    m_bReadyToRender = false;

}

void GRIBOverlayFactory::SetGribRecordSet( GribRecordSet *pGribRecordSet )
{
    Reset();

    m_pGribRecordSet = pGribRecordSet;

    m_bReadyToRender = true;

}
void GRIBOverlayFactory::ClearCachedData( void )
{
    //    Clear out the cached bitmaps
    delete m_pgob_sigwh;
    m_pgob_sigwh = NULL;

    delete m_pgob_crain;
    m_pgob_crain = NULL;

    delete m_pgob_seatemp;
    m_pgob_seatemp = NULL;

    delete m_pgob_current;
    m_pgob_current = NULL;

}

bool GRIBOverlayFactory::RenderGLGribOverlay( wxGLContext *pcontext, PlugIn_ViewPort *vp )
{
    m_pdc = NULL;                  // inform lower layers that this is OpenGL render
    return DoRenderGribOverlay( vp );
}

bool GRIBOverlayFactory::RenderGribOverlay( wxDC &dc, PlugIn_ViewPort *vp )
{
#if wxUSE_GRAPHICS_CONTEXT
    wxMemoryDC *pmdc;
    pmdc = wxDynamicCast(&dc, wxMemoryDC);
    wxGraphicsContext *pgc = wxGraphicsContext::Create( *pmdc );
    m_gdc = pgc;
    m_pdc = &dc;
#else
    m_pdc = &dc;
#endif
    return DoRenderGribOverlay( vp );
}

bool GRIBOverlayFactory::DoRenderGribOverlay( PlugIn_ViewPort *vp )
{
    if( !m_pGribRecordSet )
        return false;

    //    If the scale has changed, clear out the cached bitmaps
    if( vp->view_scale_ppm != m_last_vp_scale )
        ClearCachedData();

    m_last_vp_scale = vp->view_scale_ppm;

    //     render each type of record
    GribRecord **pGR = m_pGribRecordSet->m_GribRecordPtrArray;
    
    // Wind
    //    Actually need two records to draw the wind arrows
    RenderGribWind( pGR[Idx_WIND_VX], pGR[Idx_WIND_VY], vp );

    //Pressure
    RenderGribPressure( pGR[Idx_PRESS], vp );

    // Significant Wave Height
    RenderGribWaveHeight( pGR[Idx_HTSIGW], vp );

    // Wind wave direction
    RenderGribWaveDirection( pGR[Idx_WVDIR], vp );

    // GFS SEATEMP
    // RTOFS SEATEMP ? which one is best?
    RenderGribSeaTemperature(pGR[Idx_SEATEMP], vp);

    // RTOFS Current
    //    Actually need two records to render the current speed
    RenderGribCurrent( pGR[Idx_SEACURRENT_VX], pGR[Idx_SEACURRENT_VY], vp );

    return true;
}

bool GRIBOverlayFactory::RenderGribWind( GribRecord *pGRX, GribRecord *pGRY, PlugIn_ViewPort *vp )
{
    if(!m_dlg.m_cbWind->GetValue() || !pGRX || !pGRY)
        return false;

    //    Get the the grid
    int imax = pGRX->getNi();                  // Longitude
    int jmax = pGRX->getNj();                  // Latitude

    //    Barbs?
    bool barbs = true;

    //    Set minimum spacing between wind arrows
    int space;

    if( barbs ) space = 30;
    else
        space = 20;

    int oldx = -1000;
    int oldy = -1000;

    wxColour colour;
    GetGlobalColor( _T ( "YELO2" ), &colour );

    for( int i = 0; i < imax; i++ ) {
        double lonl = pGRX->getX( i );
        double latl = pGRX->getY( 0 );
        wxPoint pl;
        GetCanvasPixLL( vp, &pl, latl, lonl );

        if( abs( pl.x - oldx ) >= space ) {
            oldx = pl.x;
            for( int j = 0; j < jmax; j++ ) {
                double lon = pGRX->getX( i );
                double lat = pGRX->getY( j );
                wxPoint p;
                GetCanvasPixLL( vp, &p, lat, lon );

                if( abs( p.y - oldy ) >= space ) {
                    oldy = p.y;

                    if( PointInLLBox( vp, lon, lat ) || PointInLLBox( vp, lon - 360., lat ) ) {
                        double vx =  pGRX->getValue( i, j );
                        double vy =  pGRY->getValue( i, j );

                        if( vx != GRIB_NOTDEF && vy != GRIB_NOTDEF )
                            drawWindArrowWithBarbs( p.x, p.y, vx, vy, ( lat < 0. ), colour );
                    }
                }
            }
        }
    }
    return true;
}

bool GRIBOverlayFactory::RenderGribScatWind( GribRecord *pGRX, GribRecord *pGRY,
                                             PlugIn_ViewPort *vp )
{
    if(!m_dlg.m_cbWind->GetValue() || !pGRX || !pGRY)
        return false;

//    wxDateTime t( m_pGribRecordSet->m_Reference_Time );

    //    Get the the grid
    int imax = pGRX->getNi();                  // Longitude
    int jmax = pGRX->getNj();                  // Latitude

    //    Barbs?
    bool barbs = true;

    //    Set minimum spacing between wind arrows
    int space;

    if( barbs ) space = 10;
    else
        space = 10;

    int oldx = -1000;
    int oldy = -1000;

    for( int i = 0; i < imax; i++ ) {
        double lonl = pGRX->getX( i );
        double latl = pGRX->getY( 0 );
        wxPoint pl;
        GetCanvasPixLL( vp, &pl, latl, lonl );

        if( abs( pl.x - oldx ) >= space ) {
            oldx = pl.x;
            for( int j = 0; j < jmax; j++ ) {
                double lon = pGRX->getX( i );
                double lat = pGRX->getY( j );
                wxPoint p;
                GetCanvasPixLL( vp, &p, lat, lon );

                if( abs( p.y - oldy ) >= space ) {
                    oldy = p.y;

                    if( PointInLLBox( vp, lon, lat ) || PointInLLBox( vp, lon - 360., lat ) ) {
                        double vx = pGRX->getValue( i, j );
                        double vy = pGRY->getValue( i, j );
                        double vkn = sqrt( vx * vx + vy * vy ) * 3.6 / 1.852;
                        wxColour c = GetGraphicColor( vkn, QUICKSCAT_GRAPHIC_INDEX );

                        if( vx != GRIB_NOTDEF && vy != GRIB_NOTDEF ) drawWindArrowWithBarbs( p.x,
                                p.y, vx, vy, ( lat < 0. ), c );
                    }
                }
            }
        }
    }
    return true;
}

bool GRIBOverlayFactory::RenderGribWaveHeight( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    if(!m_dlg.m_cbWave->GetValue() || !pGR)
        return false;

    bool b_drawn = RenderGribFieldOverlay( pGR, NULL,
                                           vp, 16, GENERIC_GRAPHIC_INDEX, &m_pgob_sigwh );
    if( !b_drawn ) DrawMessageWindow(
            wxString( _("Please Zoom or Scale Out to view suppressed HTSGW GRIB") ),
            vp->pix_width / 2, vp->pix_height / 2 );
    return true;
}

bool GRIBOverlayFactory::RenderGribWaveDirection( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    if(!m_dlg.m_cbWave->GetValue() || !pGR)
        return false;

    //    Get the the grid
    int imax = pGR->getNi();                  // Longitude
    int jmax = pGR->getNj();                  // Latitude

    //    Set minimum spacing between wave arrows
    int space;
    space = 60;

    int oldx = -1000;
    int oldy = -1000;

    wxColour colour;
    GetGlobalColor( _T ( "UBLCK" ), &colour );

    for( int i = 0; i < imax; i++ ) {
        double lonl = pGR->getX( i );
        double latl = pGR->getY( 0 );
        wxPoint pl;
        GetCanvasPixLL( vp, &pl, latl, lonl );

        if( abs( pl.x - oldx ) >= space ) {
            oldx = pl.x;
            for( int j = 0; j < jmax; j++ ) {
                double lon = pGR->getX( i );
                double lat = pGR->getY( j );
                wxPoint p;
                GetCanvasPixLL( vp, &p, lat, lon );

                if( abs( p.y - oldy ) >= space ) {
                    oldy = p.y;

                    if( PointInLLBox( vp, lon, lat ) || PointInLLBox( vp, lon - 360., lat ) ) {
                        double dir = pGR->getValue( i, j );

                        if( dir != GRIB_NOTDEF ) drawWaveArrow( p.x, p.y, dir - 90., colour );
                    }
                }
            }
        }
    }

    return true;
}

bool GRIBOverlayFactory::RenderGribCRAIN( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    bool b_drawn = RenderGribFieldOverlay( pGR, NULL,
                                           vp, 4, CRAIN_GRAPHIC_INDEX, &m_pgob_crain );

    if( !b_drawn ) {
        DrawMessageWindow( wxString( _("Please Zoom or Scale Out to view suppressed CRAIN GRIB") ),
                vp->pix_width / 2, vp->pix_height / 2 );
    }
    return true;
}

bool GRIBOverlayFactory::RenderGribSeaTemperature( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    if(!m_dlg.m_cbSeaTemperature->GetValue() || !pGR)
        return false;

    bool b_drawn = RenderGribFieldOverlay( pGR, NULL,
                                           vp, 4, SEATEMP_GRAPHIC_INDEX, &m_pgob_seatemp );

    if( !b_drawn ) {
        DrawMessageWindow( wxString( _("Please Zoom or Scale Out to view suppressed SEATEMP GRIB") ),
                           vp->pix_width / 2, vp->pix_height / 2 );
    }

    return true;
}

bool GRIBOverlayFactory::RenderGribCurrent( GribRecord *pGRX, GribRecord *pGRY, PlugIn_ViewPort *vp )
{
    if(!m_dlg.m_cbCurrent->GetValue() || !pGRX || !pGRY)
        return false;

    bool b_drawn = RenderGribFieldOverlay( pGRX, pGRY, vp, 4,
                                           CURRENT_GRAPHIC_INDEX, &m_pgob_current );

    if( b_drawn ) {
        //    Draw little arrows for current direction
        {
            int width, height;
            if( m_pdc ) {
                width = m_pgob_current->m_pDCBitmap->GetWidth();
                height = m_pgob_current->m_pDCBitmap->GetHeight();
            } else {
                width = m_pgob_current->m_width;
                height = m_pgob_current->m_height;
            }

            wxPoint porg;
            GetCanvasPixLL( vp, &porg, pGRX->getLatMax(), pGRX->getLonMin() );
            int arrow_pixel_size = 60;
            for( int ipix = 0; ipix < ( width - arrow_pixel_size + 1 ); ipix += arrow_pixel_size ) {
                for( int jpix = 0; jpix < ( height - arrow_pixel_size + 1 ); jpix +=
                        arrow_pixel_size ) {
                    double lat, lon;
                    wxPoint p;
                    p.x = ipix + porg.x;
                    p.y = jpix + porg.y;
                    GetCanvasLLPix( vp, p, &lat, &lon );

                    double vx = pGRX->getInterpolatedValue( lon, lat );
                    double vy = pGRY->getInterpolatedValue( lon, lat );

                    if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
                        double angle = atan2( vx, vy ) * 180. / PI;
                        drawSingleArrow( p.x, p.y, angle + 90., *wxLIGHT_GREY, 2);
                        drawSingleArrow( p.x + 1, p.y + 1, angle + 90., *wxBLACK, 2);
                    }
                }
            }
        }
    }

    if( !b_drawn ) {
        DrawMessageWindow(
                wxString( _("Please Zoom or Scale Out to view suppressed CURRENT GRIB") ),
                vp->pix_width / 2, vp->pix_height / 2 );
    }
    return true;
}

bool GRIBOverlayFactory::RenderGribFieldOverlay( GribRecord *pGRA, GribRecord *pGRB,
                                                 PlugIn_ViewPort *vp, int grib_pixel_size,
                                                 int colormap_index, GribOverlay **ppGO )
{
    wxPoint porg;
    GetCanvasPixLL( vp, &porg, pGRA->getLatMax(), pGRA->getLonMin() );

    //    Check two BBoxes....
    //    TODO Make a better Intersect method
    bool bdraw = false;
    if( Intersect( vp, pGRA->getLatMin(), pGRA->getLatMax(),
                   pGRA->getLonMin(), pGRA->getLonMax(),
                   0. ) != _OUT ) bdraw = true;
    if( Intersect( vp, pGRA->getLatMin(), pGRA->getLatMax(),
                   pGRA->getLonMin() - 360., pGRA->getLonMax() - 360.,
                   0. ) != _OUT ) bdraw = true;

    if( bdraw ) {
        // If needed, create the overlay
        if( !*ppGO )
            *ppGO = new GribOverlay;

        GribOverlay *pGO = *ppGO;

        if( m_pdc == NULL )       //OpenGL mode
        {
            if( !pGO->m_iTexture )
                CreateGribGLTexture( pGO, pGRA, pGRB, vp,
                                     grib_pixel_size, colormap_index, porg);

            if( pGO->m_iTexture ) {
                DrawGLTexture( pGO->m_iTexture, pGO->m_width, pGO->m_height,
                               porg.x, porg.y, grib_pixel_size );
                return true;
            }
        } else        //DC mode
        {
            if( pGO->m_pDCBitmap == NULL ) {
                wxImage bl_image = CreateGribImage( pGRA, pGRB, vp,
                                                    grib_pixel_size, colormap_index,
                        porg );
                if( bl_image.IsOk() ) {
                    //    Create a Bitmap
                    pGO->m_pDCBitmap = new wxBitmap( bl_image );
                    wxMask *gr_mask = new wxMask( *( pGO->m_pDCBitmap ), wxColour( 0, 0, 0 ) );
                    pGO->m_pDCBitmap->SetMask( gr_mask );

                }
            }

            if( pGO->m_pDCBitmap ) {
                m_pdc->DrawBitmap( *( pGO->m_pDCBitmap ), porg.x, porg.y, true );
                return true;
            }
        }

    }
    return false;
}

void GRIBOverlayFactory::CreateGribGLTexture( GribOverlay *pGO,
                                              GribRecord *pGRA, GribRecord *pGRB,
                                              PlugIn_ViewPort *vp, int grib_pixel_size,
                                              int colormap_index, const wxPoint &porg )
{
    wxPoint pmin;
    GetCanvasPixLL( vp, &pmin, pGRA->getLatMin(), pGRA->getLonMin() );
    wxPoint pmax;
    GetCanvasPixLL( vp, &pmax, pGRA->getLatMax(), pGRA->getLonMax() );

    int width = abs( pmax.x - pmin.x )/grib_pixel_size;
    int height = abs( pmax.y - pmin.y )/grib_pixel_size;

    //    Dont try to create enormous GRIB textures
    if( ( width > 512 ) || ( height > 512 ))
        return;

    unsigned char *data = new unsigned char[width*height*4];

    for( int ipix = 0; ipix < width; ipix++ ) {
        for( int jpix = 0; jpix < height; jpix++ ) {
            wxPoint p;
            p.x = grib_pixel_size*ipix + porg.x;
            p.y = grib_pixel_size*jpix + porg.y;

            double lat, lon;
            GetCanvasLLPix( vp, p, &lat, &lon );

            double vkn = 0.;
            bool n_def = true;
            double vx = pGRA->getInterpolatedValue(lon, lat);
            if( pGRB )                // two component, e.g. velocity
            {
                double vy = pGRB->getInterpolatedValue(lon, lat);
                if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
                    vkn = sqrt( vx * vx + vy * vy ) * 3.6 / 1.852;
                    n_def = false;
                }
            } else {
                if( vx != GRIB_NOTDEF ) {
                    vkn = vx;
                    n_def = false;
                }
            }

            unsigned char r, g, b, a;
            if( !n_def ) {
                wxColour c = GetGraphicColor(vkn, colormap_index);
                r = c.Red();
                g = c.Green();
                b = c.Blue();
                a = 220;
            } else {
                r = 255;
                g = 255;
                b = 255;
                a = 0;
            }

            int doff = 4*(jpix*width + ipix);
            /* for some reason r g b values are inverted, but not alpha,
               this fixes it, but I would like to find the actual cause */
            data[doff + 0] = 255-r;
            data[doff + 1] = 255-g;
            data[doff + 2] = 255-b;
            data[doff + 3] = a;
        }
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture);

    glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0 );
    glPixelStorei( GL_UNPACK_SKIP_ROWS, 0 );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, width );

    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glPopClientAttrib();

    delete [] data;

    pGO->m_iTexture = texture;
    pGO->m_width = width;
    pGO->m_height = height;
}

wxImage GRIBOverlayFactory::CreateGribImage(  GribRecord *pGRA, GribRecord *pGRB,
                                              PlugIn_ViewPort *vp, int grib_pixel_size,
                                              int colormap_index, const wxPoint &porg )
{
    wxPoint pmin;
    GetCanvasPixLL( vp, &pmin, pGRA->getLatMin(), pGRA->getLonMin() );
    wxPoint pmax;
    GetCanvasPixLL( vp, &pmax, pGRA->getLatMax(), pGRA->getLonMax() );

    int width = abs( pmax.x - pmin.x );
    int height = abs( pmax.y - pmin.y );

    //    Dont try to create enormous GRIB bitmaps
    if( ( width < 2000 ) && ( height < 2000 ) ) {
        //    This could take a while....
        wxImage gr_image( width, height );
        gr_image.InitAlpha();

        wxPoint p;
        for( int ipix = 0; ipix < ( width - grib_pixel_size + 1 ); ipix += grib_pixel_size ) {
            for( int jpix = 0; jpix < ( height - grib_pixel_size + 1 ); jpix += grib_pixel_size ) {
                double lat, lon;
                p.x = ipix + porg.x;
                p.y = jpix + porg.y;
                GetCanvasLLPix( vp, p, &lat, &lon );

                double vkn = 0.;
                bool n_def = true;
                double vx = pGRA->getInterpolatedValue(lon, lat);

                if( pGRB )                // two component, e.g. velocity
                {
                    double vy = pGRA->getInterpolatedValue(lon, lat);
                    if( ( vx != GRIB_NOTDEF ) && ( vy != GRIB_NOTDEF ) ) {
                        vkn = sqrt( vx * vx + vy * vy ) * 3.6 / 1.852;
                        n_def = false;
                    }
                } else {
                    if( vx != GRIB_NOTDEF ) {
                        vkn = vx;
                        n_def = false;
                    }
                }

                if( !n_def ) {
                    wxColour c = GetGraphicColor(vkn, colormap_index);

                    unsigned char r = c.Red();
                    unsigned char g = c.Green();
                    unsigned char b = c.Blue();

                    for( int xp = 0; xp < grib_pixel_size; xp++ )
                        for( int yp = 0; yp < grib_pixel_size; yp++ ) {
                            gr_image.SetRGB( ipix + xp, jpix + yp, r, g, b );
                            gr_image.SetAlpha( ipix + xp, jpix + yp, 220 );
                        }
                } else {
                    for( int xp = 0; xp < grib_pixel_size; xp++ )
                        for( int yp = 0; yp < grib_pixel_size; yp++ ) {
                            gr_image.SetAlpha( ipix + xp, jpix + yp, 0 );
                        }
                }
            }
        }

        wxImage bl_image = gr_image.Blur( 4 );
        return bl_image;
    } else
        return wxNullImage;
}

struct ColorMap {
    int val;
    wxString text;
};

ColorMap CurrentMap[] =
{{0,  _T("#0000d9")},  {1, _T("#002ad9")},  {2, _T("#006ed9")},  {3, _T("#00b2d9")},
 {4,  _T("#00d4d4")},  {5, _T("#00d9a6")},  {7, _T("#00d900")},  {9, _T("#95d900")},
 {12, _T("#d9d900")}, {15, _T("#d9ae00")}, {18, _T("#d98300")}, {21, _T("#d95700")},
 {24, _T("#d90000")}, {27, _T("#ae0000")}, {30, _T("#8c0000")}, {36, _T("#870000")},
 {42, _T("#690000")}, {48, _T("#550000")}, {56, _T("#410000")}};

ColorMap GenericMap[] =
{{0, _T("#0000d9")},  {1, _T("#002ad9")},  {2, _T("#006ed9")},  {3, _T("#00b2d9")},
 {4, _T("#00d4d4")},  {5, _T("#00d9a6")},  {7, _T("#00d900")},  {9, _T("#95d900")},
 {12, _T("#d9d900")}, {15, _T("#d9ae00")}, {18, _T("#d98300")}, {21, _T("#d95700")},
 {24, _T("#d90000")}, {27, _T("#ae0000")}, {30, _T("#8c0000")}, {36, _T("#870000")},
 {42, _T("#690000")}, {48, _T("#550000")}, {56, _T("#410000")}};

ColorMap QuickscatMap[] =
{{0, _T("#000000")},  {5, _T("#000000")},  {10, _T("#00b2d9")}, {15, _T("#00d4d4")},
 {20, _T("#00d900")}, {25, _T("#d9d900")}, {30, _T("#d95700")}, {35, _T("#ae0000")},
 {40, _T("#870000")}, {45, _T("#414100")}};

ColorMap SeaTempMap[] =
{{0, _T("#0000d9")},  {1, _T("#002ad9")},  {2, _T("#006ed9")},  {3, _T("#00b2d9")},
 {4, _T("#00d4d4")},  {5, _T("#00d9a6")},  {7, _T("#00d900")},  {9, _T("#95d900")},
 {12, _T("#d9d900")}, {15, _T("#d9ae00")}, {18, _T("#d98300")}, {21, _T("#d95700")},
 {24, _T("#d90000")}, {27, _T("#ae0000")}, {30, _T("#8c0000")}, {36, _T("#870000")},
 {42, _T("#690000")}, {48, _T("#550000")}, {56, _T("#410000")}};

wxColour GRIBOverlayFactory::GetGraphicColor(double val_in, int colormap_index)
{
    double val  = val_in;
    ColorMap *map;
    int maplen;

    switch(colormap_index) {
    case CURRENT_GRAPHIC_INDEX:
        val *= 50. / 2.;
        val = wxMax(val_in, 0.0);
        map = CurrentMap;
        maplen = (sizeof CurrentMap) / (sizeof *CurrentMap);
        break;
    case SEATEMP_GRAPHIC_INDEX: 
        val -= 273.0;
        val -= 15.;
        val *= 50. / 15.;
        val = wxMax(val_in, 0.0);
        map = SeaTempMap;
        maplen = (sizeof SeaTempMap) / (sizeof *SeaTempMap);
        break;
    case CRAIN_GRAPHIC_INDEX:
        return wxColour((unsigned char) val * 255, 0, 0 );  break;
    case QUICKSCAT_GRAPHIC_INDEX:
        map = QuickscatMap;
        maplen = (sizeof QuickscatMap) / (sizeof *QuickscatMap);
        break;
    default:
        map = GenericMap;
        maplen = (sizeof GenericMap) / (sizeof *GenericMap);
    }

    for(int i=1; i<maplen; i++) {
        if(map[i].val > val_in || i==maplen-1) {
            wxColour b, c;
            c.Set(map[i].text);
            if(m_bGradualColors) {
                b.Set(map[i-1].text);
                double d = (val_in-map[i-1].val)/(map[i].val-map[i-1].val);
                c.Set((1-d)*b.Red()   + d*c.Red(),
                      (1-d)*b.Green() + d*c.Green(),
                      (1-d)*b.Blue()  + d*c.Blue());
            }
            return c;
        }
    }
    return wxColour(0, 0, 0); /* unreachable */

}

wxImage &GRIBOverlayFactory::getLabel(double value)
{
    std::map <double, wxImage >::iterator it;
    it = m_labelCache.find(value);
    if (it != m_labelCache.end())
        return m_labelCache[value];

    wxString labels;

    double coef = .01;
    labels.Printf(_T("%d"), (int)(value*coef+0.5));

    int w, h;

    wxColour text_color;
    GetGlobalColor( _T ( "DILG3" ), &text_color );

    wxColour back_color;
    GetGlobalColor( _T ( "DILG0" ), &back_color );

    wxPen penText(text_color);
    wxBrush backBrush(back_color);

    wxBitmap bm(100,100);          // big enough
    wxMemoryDC mdc(bm);
    mdc.Clear();

    mdc.GetTextExtent(labels, &w, &h);
          
    mdc.SetPen(penText);
    mdc.SetBrush(backBrush);
    mdc.SetTextForeground(text_color);
    mdc.SetTextBackground(back_color);

    int label_offset = 10;          
    int xd = 0;
    int yd = 0;
//            mdc.DrawRoundedRectangle(xd, yd, w+(label_offset * 2), h, -.25);
    mdc.DrawRectangle(xd, yd, w+(label_offset * 2), h+2);
    mdc.DrawText(labels, label_offset/2 + xd, yd-1);
          
    mdc.SelectObject(wxNullBitmap);

    wxBitmap sub_BMLabel = bm.GetSubBitmap(wxRect(0,0,w+(label_offset * 2), h+2));
    m_labelCache[value] = sub_BMLabel.ConvertToImage();
    return m_labelCache[value];
}

bool GRIBOverlayFactory::RenderGribPressure( GribRecord *pGR, PlugIn_ViewPort *vp )
{
    if(!m_dlg.m_cbPressure->GetValue() || !pGR)
        return false;

    //    Initialize the array of Isobars if necessary
    if( !m_pGribRecordSet->m_IsobarArray.GetCount() ) {
        IsoLine *piso;
        for( double press = 840; press < 1120; press += 2/*isobarsStep*/) {
            piso = new IsoLine( press * 100, pGR );
            m_pGribRecordSet->m_IsobarArray.Add( piso );
        }
    }

    //    Draw the Isobars
    for( unsigned int i = 0; i < m_pGribRecordSet->m_IsobarArray.GetCount(); i++ ) {
        IsoLine *piso = (IsoLine *) m_pGribRecordSet->m_IsobarArray.Item( i );
        if( m_pdc )
            piso->drawIsoLine( this, *m_pdc, vp, true, true ); //g_bGRIBUseHiDef
        else
            piso->drawGLIsoLine( this, vp, true, true ); //g_bGRIBUseHiDef

        // Draw Isobar labels

        int density = 40;
        int first = 0;

        if( m_pdc ) piso->drawIsoLineLabels( this, *m_pdc, vp, density,
                                             first, getLabel(piso->getValue()) );
        else
            piso->drawGLIsoLineLabels( this, vp, density, first, getLabel(piso->getValue()));
    }
    return true;
}

#if 0
bool GRIBOverlayFactory::RenderGLGribPressure(GribRecord *pGR, wxGLContext *pcontext, PlugIn_ViewPort *vp)
{

    //    Initialize the array of Isobars if necessary
    if(!m_IsobarArray.GetCount())
    {
        IsoLine *piso;
        for (double press=840; press<1120; press += 2) // 2 = isobarsStep
        {
            piso = new IsoLine(press*100, pGR);
            m_IsobarArray.Add(piso);
        }
    }

    //    Draw the Isobars
    for(unsigned int i = 0; i < m_IsobarArray.GetCount(); i++)
    {
        IsoLine *piso = (IsoLine *)m_IsobarArray.Item(i);
        if(m_pdc)
            piso->drawGLIsoLine(this, pcontext, vp, true, true); //g_bGRIBUseHiDef

        // Draw Isobar labels
        int gr = 80;
        wxColour color = wxColour(gr,gr,gr);
        int density = 40;//40;
        int first = 0;

        double coef = .01;
        piso->drawGLIsoLineLabels(this, pcontext, color, vp, density, first, coef);

    }
    return true;
}

#endif

void GRIBOverlayFactory::drawWaveArrow( int i, int j, double ang, wxColour arrowColor )
{
    double si = sin( ang * PI / 180. ), co = cos( ang * PI / 180. );

    wxPen pen( arrowColor, 1 );

    if( m_pdc && m_pdc->IsOk() ) {
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
    }

    int arrowSize = 26;
    int dec = -arrowSize / 2;

    drawTransformedLine( pen, si, co, i, j, dec, -2, dec + arrowSize, -2 );
    drawTransformedLine( pen, si, co, i, j, dec, 2, dec + arrowSize, +2 );

    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, 6 );    // flèche
    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, -6 );   // flèche

}

void GRIBOverlayFactory::drawSingleArrow( int i, int j, double ang, wxColour arrowColor, int width )
{
    double si = sin( ang * PI / 180. ), co = cos( ang * PI / 180. );

    wxPen pen( arrowColor, width );

    if( m_pdc && m_pdc->IsOk() ) {
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
    }

    int arrowSize = 26;
    int dec = -arrowSize / 2;

    drawTransformedLine( pen, si, co, i, j, dec, 0, dec + arrowSize, 0 );

    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, 6 );    // flèche
    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, -6 );   // flèche

}

void GRIBOverlayFactory::drawWindArrowWithBarbs( int i, int j, double vx, double vy, bool south,
        wxColour arrowColor )
{
    double vkn = sqrt( vx * vx + vy * vy ) * 3.6 / 1.852;
    double ang = atan2( vy, -vx );
    double si = sin( ang ), co = cos( ang );

    wxPen pen( arrowColor, 2 );

    if( m_pdc && m_pdc->IsOk() ) {
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
    }

    if( vkn < 1 ) {
        int r = 5;     // wind is very light, draw a circle
        if( m_pdc && m_pdc->IsOk() ) m_pdc->DrawCircle( i, j, r );
    } else {
        // Arrange for arrows to be centered on origin
        int windBarbuleSize = 26;
        int dec = -windBarbuleSize / 2;
        drawTransformedLine( pen, si, co, i, j, dec, 0, dec + windBarbuleSize, 0 );   // hampe
        drawTransformedLine( pen, si, co, i, j, dec, 0, dec + 5, 2 );    // flèche
        drawTransformedLine( pen, si, co, i, j, dec, 0, dec + 5, -2 );   // flèche

        int b1 = dec + windBarbuleSize - 4;  // position de la 1ère barbule
        if( vkn >= 7.5 && vkn < 45 ) {
            b1 = dec + windBarbuleSize;  // position de la 1ère barbule si >= 10 noeuds
        }

        if( vkn < 7.5 ) {  // 5 ktn
            drawPetiteBarbule( pen, south, si, co, i, j, b1 );
        } else if( vkn < 12.5 ) { // 10 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
        } else if( vkn < 17.5 ) { // 15 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
            drawPetiteBarbule( pen, south, si, co, i, j, b1 - 4 );
        } else if( vkn < 22.5 ) { // 20 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 4 );
        } else if( vkn < 27.5 ) { // 25 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 4 );
            drawPetiteBarbule( pen, south, si, co, i, j, b1 - 8 );
        } else if( vkn < 32.5 ) { // 30 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 4 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 8 );
        } else if( vkn < 37.5 ) { // 35 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 4 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 8 );
            drawPetiteBarbule( pen, south, si, co, i, j, b1 - 12 );
        } else if( vkn < 45 ) { // 40 ktn
            drawGrandeBarbule( pen, south, si, co, i, j, b1 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 4 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 8 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 12 );
        } else if( vkn < 55 ) { // 50 ktn
            drawTriangle( pen, south, si, co, i, j, b1 - 4 );
        } else if( vkn < 65 ) { // 60 ktn
            drawTriangle( pen, south, si, co, i, j, b1 - 4 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 8 );
        } else if( vkn < 75 ) { // 70 ktn
            drawTriangle( pen, south, si, co, i, j, b1 - 4 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 8 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 12 );
        } else if( vkn < 85 ) { // 80 ktn
            drawTriangle( pen, south, si, co, i, j, b1 - 4 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 8 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 12 );
            drawGrandeBarbule( pen, south, si, co, i, j, b1 - 16 );
        } else { // > 90 ktn
            drawTriangle( pen, south, si, co, i, j, b1 - 4 );
            drawTriangle( pen, south, si, co, i, j, b1 - 12 );
        }
    }
}

void GRIBOverlayFactory::drawTransformedLine( wxPen pen, double si, double co, int di, int dj,
        int i, int j, int k, int l )
{
    int ii, jj, kk, ll;
    double fi, fj, fk, fl; // For Hi Def Graphics.

    fi = ( i * co - j * si + 0.5 ) + di;
    fj = ( i * si + j * co + 0.5 ) + dj;
    fk = ( k * co - l * si + 0.5 ) + di;
    fl = ( k * si + l * co + 0.5 ) + dj;

    ii = fi; jj = fj; kk = fk; ll = fl;

    if( m_pdc && m_pdc->IsOk() ) {
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
#if wxUSE_GRAPHICS_CONTEXT
        if( m_hiDefGraphics && m_gdc ) {
            m_gdc->SetPen( pen );
            m_gdc->StrokeLine( fi, fj, fk, fl );
        }
        else {
            m_pdc->DrawLine( ii, jj, kk, ll );
        }
#else
        m_pdc->DrawLine(ii, jj, kk, ll);
#endif
    } else {                       // OpenGL mode
        wxColour c = pen.GetColour();
        glColor4ub( c.Red(), c.Green(), c.Blue(), 255);
        double w = pen.GetWidth();
        if( m_hiDefGraphics ) w *= 0.75;
        DrawGLLine( fi, fj, fk, fl, w );
    }
}

void GRIBOverlayFactory::drawPetiteBarbule( wxPen pen, bool south, double si, double co, int di,
        int dj, int b )
{
    if( south ) drawTransformedLine( pen, si, co, di, dj, b, 0, b + 2, -5 );
    else
        drawTransformedLine( pen, si, co, di, dj, b, 0, b + 2, 5 );
}

void GRIBOverlayFactory::drawGrandeBarbule( wxPen pen, bool south, double si, double co, int di,
        int dj, int b )
{
    if( south ) drawTransformedLine( pen, si, co, di, dj, b, 0, b + 4, -10 );
    else
        drawTransformedLine( pen, si, co, di, dj, b, 0, b + 4, 10 );
}

void GRIBOverlayFactory::drawTriangle( wxPen pen, bool south, double si, double co, int di, int dj,
        int b )
{
    if( south ) {
        drawTransformedLine( pen, si, co, di, dj, b, 0, b + 4, -10 );
        drawTransformedLine( pen, si, co, di, dj, b + 8, 0, b + 4, -10 );
    } else {
        drawTransformedLine( pen, si, co, di, dj, b, 0, b + 4, 10 );
        drawTransformedLine( pen, si, co, di, dj, b + 8, 0, b + 4, 10 );
    }
}

void GRIBOverlayFactory::DrawGLLine( double x1, double y1, double x2, double y2, double width )
{
    {
        glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_ENABLE_BIT |
                     GL_POLYGON_BIT | GL_HINT_BIT ); //Save state
        {

            //      Enable anti-aliased lines, at best quality
            glEnable( GL_LINE_SMOOTH );
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
            glLineWidth( width );

            glBegin( GL_LINES );
            glVertex2d( x1, y1 );
            glVertex2d( x2, y2 );
            glEnd();
        }

        glPopAttrib();
    }
}

void GRIBOverlayFactory::DrawOLBitmap( const wxBitmap &bitmap, wxCoord x, wxCoord y, bool usemask )
{
    wxBitmap bmp;
    if( x < 0 || y < 0 ) {
        int dx = ( x < 0 ? -x : 0 );
        int dy = ( y < 0 ? -y : 0 );
        int w = bitmap.GetWidth() - dx;
        int h = bitmap.GetHeight() - dy;
        /* picture is out of viewport */
        if( w <= 0 || h <= 0 ) return;
        wxBitmap newBitmap = bitmap.GetSubBitmap( wxRect( dx, dy, w, h ) );
        x += dx;
        y += dy;
        bmp = newBitmap;
    } else {
        bmp = bitmap;
    }
    if( m_pdc && m_pdc->IsOk() ) m_pdc->DrawBitmap( bmp, x, y, usemask );
    else {
        wxImage image = bmp.ConvertToImage();
        int w = image.GetWidth(), h = image.GetHeight();

        if( usemask ) {
            unsigned char *d = image.GetData();
            unsigned char *a = image.GetAlpha();

            unsigned char mr, mg, mb;
            if( !image.GetOrFindMaskColour( &mr, &mg, &mb ) && !a ) printf(
                    "trying to use mask to draw a bitmap without alpha or mask\n" );

            unsigned char *e = new unsigned char[4 * w * h];
            {
                for( int y = 0; y < h; y++ )
                    for( int x = 0; x < w; x++ ) {
                        unsigned char r, g, b;
                        int off = ( y * image.GetWidth() + x );
                        r = d[off * 3 + 0];
                        g = d[off * 3 + 1];
                        b = d[off * 3 + 2];

                        e[off * 4 + 0] = r;
                        e[off * 4 + 1] = g;
                        e[off * 4 + 2] = b;

                        e[off * 4 + 3] =
                                a ? a[off] : ( ( r == mr ) && ( g == mg ) && ( b == mb ) ? 0 : 255 );
                    }
            }

            glColor4f( 1, 1, 1, 1 );

            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glRasterPos2i( x, y );
            glPixelZoom( 1, -1 );
            glDrawPixels( w, h, GL_RGBA, GL_UNSIGNED_BYTE, e );
            glPixelZoom( 1, 1 );
            glDisable( GL_BLEND );

            delete[] ( e );
        } else {
            glRasterPos2i( x, y );
            glPixelZoom( 1, -1 ); /* draw data from top to bottom */
            glDrawPixels( w, h, GL_RGB, GL_UNSIGNED_BYTE, image.GetData() );
            glPixelZoom( 1, 1 );
        }
    }
}

void GRIBOverlayFactory::DrawGLImage( wxImage *pimage, wxCoord xd, wxCoord yd, bool usemask )
{
    int w = pimage->GetWidth(), h = pimage->GetHeight();
    int x_offset = 0;
    int y_offset = 0;

    unsigned char *d = pimage->GetData();
    unsigned char *a = pimage->GetAlpha();

    unsigned char *e = new unsigned char[4 * w * h];
    {
        for( int y = 0; y < h; y++ )
            for( int x = 0; x < w; x++ ) {
                unsigned char r, g, b;
                int off = ( ( y + y_offset ) * pimage->GetWidth() + x + x_offset );
                r = d[off * 3 + 0];
                g = d[off * 3 + 1];
                b = d[off * 3 + 2];

                int doff = ( y * w + x );
                e[doff * 4 + 0] = r;
                e[doff * 4 + 1] = g;
                e[doff * 4 + 2] = b;

                e[doff * 4 + 3] = a ? a[off] : 255;
            }
    }

    DrawGLRGBA( e, w, h, xd, yd );
    delete[] e;
}

void GRIBOverlayFactory::DrawGLTexture( GLuint texture, int width, int height,
                                        int xd, int yd, int grib_pixel_size )
{ 
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture);

    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    glDisable( GL_MULTISAMPLE );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1, 1, 1, 1);

    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    
    int x = xd, y = yd, w = width*grib_pixel_size, h = height*grib_pixel_size;
    
    glBegin(GL_QUADS);
    glTexCoord2i(0, 0),          glVertex2i(x, y);
    glTexCoord2i(width, 0),      glVertex2i(x+w, y);
    glTexCoord2i(width, height), glVertex2i(x+w, y+h);
    glTexCoord2i(0, height),     glVertex2i(x, y+h);
    glEnd();
    
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
}

void GRIBOverlayFactory::DrawGLRGBA( unsigned char *pRGBA, int width, int height, int xd,
        int yd )
{
    int x_offset = 0;
    int y_offset = 0;
    int draw_width = width;
    int draw_height = height;

    glColor4f( 1, 1, 1, 1 );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glPixelZoom( 1, -1 );

    glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );

    glPixelStorei( GL_UNPACK_ROW_LENGTH, width );
    if( xd < 0 ) {
        x_offset = -xd;
        draw_width += xd;
    }
    if( yd < 0 ) {
        y_offset = -yd;
        draw_height += yd;
    }

    glRasterPos2i( xd + x_offset, yd + y_offset );

    glPixelStorei( GL_UNPACK_SKIP_PIXELS, x_offset );
    glPixelStorei( GL_UNPACK_SKIP_ROWS, y_offset );

    glDrawPixels( draw_width, draw_height, GL_RGBA, GL_UNSIGNED_BYTE, pRGBA );
    glPixelZoom( 1, 1 );
    glDisable( GL_BLEND );

    glPopClientAttrib();

}

void GRIBOverlayFactory::DrawMessageWindow( wxString msg, int x, int y )
{
    wxMemoryDC mdc;
    wxBitmap bm( 1000, 1000 );
    mdc.SelectObject( bm );
    mdc.Clear();

    wxFont mfont( 15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL );
    mdc.SetFont( mfont );
    mdc.SetPen( *wxBLACK_PEN);
    mdc.SetBrush( *wxWHITE_BRUSH);

    int w, h;
    mdc.GetTextExtent( msg, &w, &h );

    int label_offset = 10;
    int wdraw = w + ( label_offset * 2 );
    mdc.DrawRectangle( 0, 0, wdraw, h + 2 );
    mdc.DrawText( msg, label_offset / 2, -1 );

    mdc.SelectObject( wxNullBitmap );

    wxBitmap sbm = bm.GetSubBitmap( wxRect( 0, 0, wdraw, h + 2 ) );
    DrawOLBitmap( sbm, x - wdraw / 2, y, false );
}
