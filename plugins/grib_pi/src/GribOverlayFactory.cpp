/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2014 by David S. Register                               *
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
#endif //precompiled headers

#include <wx/glcanvas.h>
#include <wx/graphics.h>
#include <wx/progdlg.h>

#include "GribUIDialog.h"
#include "GribOverlayFactory.h"

enum GRIB_OVERLAP { _GIN, _GON, _GOUT };

// Calculates if two boxes intersect. If so, the function returns _ON.
// If they do not intersect, two scenario's are possible:
// other is outside this -> return _OUT
// other is inside this -> return _IN
GRIB_OVERLAP Intersect( PlugIn_ViewPort *vp, double lat_min, double lat_max, double lon_min,
                   double lon_max, double Marge )
{

    if( ( ( vp->lon_min - Marge ) > ( lon_max + Marge ) )
            || ( ( vp->lon_max + Marge ) < ( lon_min - Marge ) )
            || ( ( vp->lat_max + Marge ) < ( lat_min - Marge ) )
            || ( ( vp->lat_min - Marge ) > ( lat_max + Marge ) ) ) return _GOUT;

    // Check if other.bbox is inside this bbox
    if( ( vp->lon_min <= lon_min ) && ( vp->lon_max >= lon_max ) && ( vp->lat_max >= lat_max )
            && ( vp->lat_min <= lat_min ) ) return _GIN;

    // Boundingboxes intersect
    return _GON;
}

// Is the given point in the vp ??
bool PointInLLBox( PlugIn_ViewPort *vp, double x, double y )
{
    double Marge = 0.;
    double m_minx = vp->lon_min;
    double m_maxx = vp->lon_max;
    double m_miny = vp->lat_min;
    double m_maxy = vp->lat_max;

    //    Box is centered in East lon, crossing IDL
    if(m_maxx > 180.)
    {
        if( x < m_maxx - 360.)
            x +=  360.;

        if (  x >= (m_minx - Marge) && x <= (m_maxx + Marge) &&
            y >= (m_miny - Marge) && y <= (m_maxy + Marge) )
            return TRUE;
        return FALSE;
    }

    //    Box is centered in Wlon, crossing IDL
    else if(m_minx < -180.)
    {
        if(x > m_minx + 360.)
            x -= 360.;

        if (  x >= (m_minx - Marge) && x <= (m_maxx + Marge) &&
            y >= (m_miny - Marge) && y <= (m_maxy + Marge) )
            return TRUE;
        return FALSE;
    }

    else
    {
        if (  x >= (m_minx - Marge) && x <= (m_maxx + Marge) &&
            y >= (m_miny - Marge) && y <= (m_maxy + Marge) )
            return TRUE;
        return FALSE;
    }
}

#if 0
static wxString MToString( int DataCenterModel )
{
    switch( DataCenterModel ) {
    case NOAA_GFS: return  _T("NOAA_GFS");
    case NOAA_NCEP_WW3: return  _T("NOAA_NCEP_WW3");
    case NOAA_NCEP_SST: return  _T("NOAA_NCEP_SST");
    case NOAA_RTOFS: return  _T("NOAA_RTOFS");
    case FNMOC_WW3_GLB: return  _T("FNMOC_WW3");
    case FNMOC_WW3_MED: return  _T("FNMOC_WW3");
    case NORWAY_METNO: return  _T("NORWAY_METNO");
    default : return  _T("OTHER_DATA_CENTER");
    }
}
#endif

#if 0
static wxString TToString( const wxDateTime date_time, const int time_zone )
{
    wxDateTime t( date_time );
    t.MakeFromTimezone( wxDateTime::UTC );
    if( t.IsDST() ) t.Subtract( wxTimeSpan( 1, 0, 0, 0 ) );
    switch( time_zone ) {
        case 0: return t.Format( _T(" %a %d-%b-%Y  %H:%M "), wxDateTime::Local ) + _T("LOC");//:%S
        case 1:
        default: return t.Format( _T(" %a %d-%b-%Y %H:%M  "), wxDateTime::UTC ) + _T("UTC");
    }
}
#endif

#ifdef ocpnUSE_GL
static GLuint texture_format = 0;
#endif

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

#if defined(__WXMSW__)
#define systemGetProcAddress(ADDR) wglGetProcAddress(ADDR)
#elif defined(__WXOSX__)
#include <dlfcn.h>
#define systemGetProcAddress(ADDR) dlsym( RTLD_DEFAULT, ADDR)
#else
#define systemGetProcAddress(ADDR) glXGetProcAddress((const GLubyte*)ADDR)
#endif

//----------------------------------------------------------------------------------------------------------
//    Grib Overlay Factory Implementation
//----------------------------------------------------------------------------------------------------------
GRIBOverlayFactory::GRIBOverlayFactory( GRIBUIDialog &dlg )
    : m_dlg(dlg), m_Settings(dlg.m_OverlaySettings)
{
    m_dFont_map = new wxFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    m_dFont_war = new wxFont( 16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL );

    m_pGribTimelineRecordSet = NULL;
    m_last_vp_scale = 0.;

    for(int i=0; i<GribOverlaySettings::SETTINGS_COUNT; i++)
        m_pOverlay[i] = NULL;

    m_ParticleMap = NULL;
    m_tParticleTimer.Connect(wxEVT_TIMER, wxTimerEventHandler( GRIBOverlayFactory::OnParticleTimer ), NULL, this);
    m_bUpdateParticles = false;
}

GRIBOverlayFactory::~GRIBOverlayFactory()
{
    ClearCachedData();

    delete m_ParticleMap;
    m_ParticleMap = NULL;
}

void GRIBOverlayFactory::Reset()
{
    m_pGribTimelineRecordSet = NULL;

    ClearCachedData();
}

void GRIBOverlayFactory::SetGribTimelineRecordSet( GribTimelineRecordSet *pGribTimelineRecordSet )
{
    Reset();
    m_pGribTimelineRecordSet = pGribTimelineRecordSet;

}

void GRIBOverlayFactory::ClearCachedData( void )
{
    //    Clear out the cached bitmaps
    for(int i=0; i<GribOverlaySettings::SETTINGS_COUNT; i++) {
        delete m_pOverlay[i];
        m_pOverlay[i] = NULL;
    }
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

void GRIBOverlayFactory::SettingsIdToGribId(int i, int &idx, int &idy, bool &polar)
{
    idx = idy = -1;
    polar = false;
    switch(i) {
    case GribOverlaySettings::WIND:
        idx = Idx_WIND_VX + m_Altitude, idy = Idx_WIND_VY + m_Altitude; break;
    case GribOverlaySettings::WIND_GUST:
        idx = Idx_WIND_GUST; break;
    case GribOverlaySettings::PRESSURE:
        idx = Idx_PRESSURE; break;
    case GribOverlaySettings::WAVE:
        idx = Idx_HTSIGW, idy = Idx_WVDIR; polar = true; break;
    case GribOverlaySettings::CURRENT:
        idx = Idx_SEACURRENT_VX, idy = Idx_SEACURRENT_VY; break;
    case GribOverlaySettings::PRECIPITATION:
        idx = Idx_PRECIP_TOT; break;
    case GribOverlaySettings::CLOUD:
        idx = Idx_CLOUD_TOT; break;
    case GribOverlaySettings::AIR_TEMPERATURE:
        idx = Idx_AIR_TEMP; break;
    case GribOverlaySettings::SEA_TEMPERATURE:
        idx = Idx_SEA_TEMP; break;
    case GribOverlaySettings::CAPE:
        idx = Idx_CAPE; break;
    }
}

bool GRIBOverlayFactory::DoRenderGribOverlay( PlugIn_ViewPort *vp )
{
    if( !m_pGribTimelineRecordSet ) {
        DrawMessageWindow( ( m_Message ), vp->pix_width, vp->pix_height, m_dFont_war );
        return false;
    }

    // setup numbers texture if needed
    if(!m_pdc) {
        wxFont font( 9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
        m_TexFontNumbers.Build(font);
    }

    m_Message_Hiden.Empty();

    //    If the scale has changed, clear out the cached bitmaps in DC mode
    if( m_pdc && vp->view_scale_ppm != m_last_vp_scale )
        ClearCachedData();

    m_last_vp_scale = vp->view_scale_ppm;

    //     render each type of record
    GribRecord **pGR = m_pGribTimelineRecordSet->m_GribRecordPtrArray;
    wxArrayPtrVoid **pIA = m_pGribTimelineRecordSet->m_IsobarArray;

    for(int overlay = 1; overlay >= 0; overlay--)
        for(int i=0; i<GribOverlaySettings::SETTINGS_COUNT; i++) {
            if(i == GribOverlaySettings::WIND ) {
                if(overlay) {   /* render overlays first */
                    if(m_dlg.m_cbWind->GetValue()) RenderGribOverlayMap( i, pGR, vp );
                } else {
                    if(m_dlg.m_cbWind->GetValue()){
                        RenderGribBarbedArrows( i, pGR, vp );
                        RenderGribIsobar( i, pGR, pIA, vp );
                        RenderGribNumbers( i, pGR, vp );
                        RenderGribParticles( i, pGR, vp );
                    } else {
                        if(m_Settings.Settings[i].m_iBarbedVisibility) RenderGribBarbedArrows( i, pGR, vp );
                    }
                }
                continue;
            }
            if(i == GribOverlaySettings::PRESSURE ) {
                if(!overlay) {   /*no overalay for pressure*/
                    if( m_dlg.m_cbPressure->GetValue() ) {
                        RenderGribIsobar( i, pGR, pIA, vp );
                        RenderGribNumbers( i, pGR, vp );
                    } else {
                        if(m_Settings.Settings[i].m_iIsoBarVisibility) RenderGribIsobar( i, pGR, pIA, vp );
                    }
                }
                continue;
            }
        if((i == GribOverlaySettings::WIND_GUST       && !m_dlg.m_cbWindGust->GetValue()) ||
           (i == GribOverlaySettings::WAVE            && !m_dlg.m_cbWave->GetValue()) ||
           (i == GribOverlaySettings::CURRENT         && !m_dlg.m_cbCurrent->GetValue()) ||
           (i == GribOverlaySettings::PRECIPITATION   && !m_dlg.m_cbPrecipitation->GetValue()) ||
           (i == GribOverlaySettings::CLOUD           && !m_dlg.m_cbCloud->GetValue()) ||
           (i == GribOverlaySettings::AIR_TEMPERATURE && !m_dlg.m_cbAirTemperature->GetValue()) ||
           (i == GribOverlaySettings::SEA_TEMPERATURE && !m_dlg.m_cbSeaTemperature->GetValue()) ||
           (i == GribOverlaySettings::CAPE            && !m_dlg.m_cbCAPE->GetValue()))
            continue;

        if(overlay) /* render overlays first */
            RenderGribOverlayMap( i, pGR, vp );
        else {
            RenderGribBarbedArrows( i, pGR, vp );
            RenderGribIsobar( i, pGR, pIA, vp );
            RenderGribDirectionArrows( i, pGR, vp );
            RenderGribNumbers( i, pGR, vp );
            RenderGribParticles( i, pGR, vp );
        }
    }
    if( m_Altitude ) {
        if( !m_Message_Hiden.IsEmpty() ) m_Message_Hiden.Append(_T("   "));
        m_Message_Hiden.Append(_("WIND data at")).Append(_T(" "))
            .Append(m_Settings.GetAltitudeFromIndex(m_Altitude, m_Settings.Settings[GribOverlaySettings::PRESSURE].m_Units)).Append(_T(" "))
            .Append(m_Settings.GetUnitSymbol(GribOverlaySettings::PRESSURE))
            .Append(_T(" !"));
    }
    if( !m_Message_Hiden.IsEmpty() )
        DrawMessageWindow( m_Message_Hiden , vp->pix_width, vp->pix_height, m_dFont_map );
    return true;
}

#ifdef ocpnUSE_GL
bool GRIBOverlayFactory::CreateGribGLTexture( GribOverlay *pGO, int settings, GribRecord *pGR,
                                              PlugIn_ViewPort *vp, int grib_pixel_size )
{
    const double scalef = 0.00005;
    PlugIn_ViewPort uvp = *vp;
    uvp.rotation = uvp.skew = 0;
    uvp.view_scale_ppm = scalef;

    wxPoint porg;
    GetCanvasPixLL( &uvp, &porg, pGR->getLatMax(), pGR->getLonMin() );
    wxPoint pmin;
    GetCanvasPixLL( &uvp, &pmin, pGR->getLatMin(), pGR->getLonMin() );
    wxPoint pmax;
    GetCanvasPixLL( &uvp, &pmax, pGR->getLatMax(), pGR->getLonMax() );
    int width = abs( pmax.x - pmin.x );
    int height = abs( pmax.y - pmin.y );
    //    Dont try to create enormous GRIB textures
    if( width > 1024 || height > 1024 )
        return false;

    unsigned char *data = new unsigned char[width*height*4];
    for( int ipix = 0; ipix < width; ipix++ ) {
        for( int jpix = 0; jpix < height; jpix++ ) {
            wxPoint p;
            p.x = grib_pixel_size*ipix + porg.x;
            p.y = grib_pixel_size*jpix + porg.y;
            double lat, lon;
            GetCanvasLLPix( &uvp, p, &lat, &lon );
            double v = pGR->getInterpolatedValue(lon, lat);
            unsigned char r, g, b, a;
            if( v != GRIB_NOTDEF ) {
                v = m_Settings.CalibrateValue(settings, v);
                wxColour c = GetGraphicColor(settings, v);
                r = c.Red();
                g = c.Green();
                b = c.Blue();
                //set full transparency if no rain or no clouds at all
                a = ( ( settings == GribOverlaySettings::PRECIPITATION || GribOverlaySettings::CLOUD ) && v < 0.01 ) ? 0 : m_Settings.m_iOverlayTransparency;
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
    glBindTexture(texture_format, texture);

    glTexParameteri( texture_format, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( texture_format, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( texture_format, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( texture_format, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0 );
    glPixelStorei( GL_UNPACK_SKIP_ROWS, 0 );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, width );

    glTexImage2D(texture_format, 0, GL_RGBA, width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glPopClientAttrib();

    delete [] data;

    pGO->m_iTexture = texture;
    pGO->m_width = width;
    pGO->m_height = height;

    pGO->m_dwidth = (pmax.x - pmin.x) / scalef * grib_pixel_size;
    pGO->m_dheight = (pmin.y - pmax.y) / scalef * grib_pixel_size;

    return true;
}
#endif

wxImage GRIBOverlayFactory::CreateGribImage( int settings, GribRecord *pGR,
                                             PlugIn_ViewPort *vp, int grib_pixel_size,
                                             const wxPoint &porg )
{
    wxPoint pmin;
    GetCanvasPixLL( vp, &pmin, pGR->getLatMin(), pGR->getLonMin() );
    wxPoint pmax;
    GetCanvasPixLL( vp, &pmax, pGR->getLatMax(), pGR->getLonMax() );

    int width = abs( pmax.x - pmin.x );
    int height = abs( pmax.y - pmin.y );

    //    Dont try to create enormous GRIB bitmaps ( no more than the screen size )
    if( width > m_ParentSize.GetWidth() || height > m_ParentSize.GetHeight() )
        return wxNullImage;

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

            double v = pGR->getInterpolatedValue(lon, lat);
            if( v != GRIB_NOTDEF ) {
                v = m_Settings.CalibrateValue(settings, v);
                wxColour c = GetGraphicColor(settings, v);

                //set full transparency if no rain or no clouds at all
                unsigned char a = ( ( settings == GribOverlaySettings::PRECIPITATION || GribOverlaySettings::CLOUD ) && v < 0.01 ) ? 0 :
                            m_Settings.m_iOverlayTransparency;

                unsigned char r = c.Red();
                unsigned char g = c.Green();
                unsigned char b = c.Blue();

                for( int xp = 0; xp < grib_pixel_size; xp++ )
                    for( int yp = 0; yp < grib_pixel_size; yp++ ) {
                        gr_image.SetRGB( ipix + xp, jpix + yp, r, g, b );
                        gr_image.SetAlpha( ipix + xp, jpix + yp, a );
                    }
            } else {
                for( int xp = 0; xp < grib_pixel_size; xp++ )
                    for( int yp = 0; yp < grib_pixel_size; yp++ )
                        gr_image.SetAlpha( ipix + xp, jpix + yp, 0 );
            }
        }
    }

    return gr_image.Blur( 4 );
}

struct ColorMap {
    double val;
    wxString text;
};

ColorMap CurrentMap[] =
{{0,  _T("#d90000")},  {1, _T("#d92a00")},  {2, _T("#d96e00")},  {3, _T("#d9b200")},
 {4,  _T("#d4d404")},  {5, _T("#a6d906")},  {7, _T("#06d9a0")},  {9, _T("#00d9b0")},
 {12, _T("#00d9c0")}, {15, _T("#00aed0")}, {18, _T("#0083e0")}, {21, _T("#0057e0")},
 {24, _T("#0000f0")}, {27, _T("#0400f0")}, {30, _T("#1c00f0")}, {36, _T("#4800f0")},
 {42, _T("#6900f0")}, {48, _T("#a000f0")}, {56, _T("#f000f0")}};

ColorMap GenericMap[] =
{{0, _T("#00d900")},  {1, _T("#2ad900")},  {2, _T("#6ed900")},  {3, _T("#b2d900")},
 {4, _T("#d4d400")},  {5, _T("#d9a600")},  {7, _T("#d90000")},  {9, _T("#d90040")},
 {12, _T("#d90060")}, {15, _T("#ae0080")}, {18, _T("#8300a0")}, {21, _T("#5700c0")},
 {24, _T("#0000d0")}, {27, _T("#0400e0")}, {30, _T("#0800e0")}, {36, _T("#a000e0")},
 {42, _T("#c004c0")}, {48, _T("#c008a0")}, {56, _T("#c0a008")}};

//    HTML colors taken from zygrib representation
ColorMap WindMap[] =
{{0, _T("#288CFF")},{3, _T("#00AFFF")},{6, _T("#00DCE1")},{9, _T("#00F7B0")},{12, _T("#00EA9C")},
{15, _T("#82F059")},{18, _T("#F0F503")},{21, _T("#FFED00")},{24, _T("#FFDB00")},{27, _T("#FFC700")},
{30, _T("#FFB400")},{33, _T("#FF9800")},{36, _T("#FF7E00")},{39, _T("#F77800")},{42, _T("#EC7814")},
{45, _T("#E4711E")},{48, _T("#E06128")},{51, _T("#DC5132")},{54, _T("#D5453C")},{57, _T("#CD3A46")},
{60, _T("#BE2C50")},{63, _T("#B41A5A")},{66, _T("#AA1464")},{70, _T("#962878")},{75, _T("#8C328C")}};

//    HTML colors taken from zygrib representation
ColorMap AirTempMap[] =
{{0, _T("#283282")}, {5, _T("#273c8c")}, {10, _T("#264696")}, {14, _T("#2350a0")},
 {18, _T("#1f5aaa")}, {22, _T("#1a64b4")}, {26, _T("#136ec8")}, {29, _T("#0c78e1")},
 {32, _T("#0382e6")}, {35, _T("#0091e6")}, {38, _T("#009ee1")}, {41 , _T("#00a6dc")},
 {44 , _T("#00b2d7")}, {47 , _T("#00bed2")}, {50  , _T("#28c8c8")}, { 53 , _T("#78d2aa")},
 { 56 , _T("#8cdc78")}, { 59 , _T("#a0eb5f")}, {62 , _T("#c8f550")}, {65 , _T("#f3fb02")},
 {68 , _T("#ffed00")}, { 71, _T("#ffdd00")}, {74 , _T("#ffc900")}, {78 , _T("#ffab00")},
 {82 , _T("#ff8100")}, { 86, _T("#f1780c")}, {90 , _T("#e26a23")}, {95 , _T("#d5453c")},
 {100 , _T("#b53c59")}};

ColorMap SeaTempMap[] =
{{0, _T("#0000d9")},  {1, _T("#002ad9")},  {2, _T("#006ed9")},  {3, _T("#00b2d9")},
 {4, _T("#00d4d4")},  {5, _T("#00d9a6")},  {7, _T("#00d900")},  {9, _T("#95d900")},
 {12, _T("#d9d900")}, {15, _T("#d9ae00")}, {18, _T("#d98300")}, {21, _T("#d95700")},
 {24, _T("#d90000")}, {27, _T("#ae0000")}, {30, _T("#8c0000")}, {36, _T("#870000")},
 {42, _T("#690000")}, {48, _T("#550000")}, {56, _T("#410000")}};

    //    HTML colors taken from ZyGrib representation
ColorMap PrecipitationMap[] =
{{0,   _T("#ffffff")}, {.01, _T("#c8f0ff")}, {.02, _T("#b4e6ff")}, {.05, _T("#8cd3ff")},
 {.07, _T("#78caff")}, {.1 , _T("#6ec1ff")}, {.2 , _T("#64b8ff")}, {.5 , _T("#50a6ff")},
 {.7 , _T("#469eff")}, {1.0, _T("#3c96ff")}, {2.0, _T("#328eff")}, {5.0, _T("#1e7eff")},
 {7.0, _T("#1476f0")}, {10 , _T("#0a6edc")}, {20 , _T("#0064c8")}, {50, _T("#0052aa")}};

    //    HTML colors taken from ZyGrib representation
ColorMap CloudMap[] =
{{0,  _T("#ffffff")}, {1,  _T("#f0f0e6")}, {10, _T("#e6e6dc")}, {20, _T("#dcdcd2")},
 {30, _T("#c8c8b4")}, {40, _T("#aaaa8c")}, {50, _T("#969678")}, {60, _T("#787864")},
 {70, _T("#646450")}, {80, _T("#5a5a46")}, {90, _T("#505036")}};

ColorMap *ColorMaps[] = {CurrentMap, GenericMap, WindMap, AirTempMap, SeaTempMap, PrecipitationMap, CloudMap};

enum {
    GENERIC_GRAPHIC_INDEX, WIND_GRAPHIC_INDEX, AIRTEMP__GRAPHIC_INDEX, SEATEMP_GRAPHIC_INDEX,
    PRECIPITATION_GRAPHIC_INDEX, CLOUD_GRAPHIC_INDEX, CURRENT_GRAPHIC_INDEX
};

wxColour GRIBOverlayFactory::GetGraphicColor(int settings, double val_in)
{
    int colormap_index = m_Settings.Settings[settings].m_iOverlayMapColors;
    ColorMap *map;
    int maplen;

    /* normalize input value */
    double min = m_Settings.GetMin(settings), max = m_Settings.GetMax(settings);

    val_in -= min;
    val_in /= max-min;

    switch(colormap_index) {
    case CURRENT_GRAPHIC_INDEX:
        map = CurrentMap;
        maplen = (sizeof CurrentMap) / (sizeof *CurrentMap);
        break;
    case GENERIC_GRAPHIC_INDEX:
        map = GenericMap;
        maplen = (sizeof GenericMap) / (sizeof *GenericMap);
        break;
    case WIND_GRAPHIC_INDEX:
        map = WindMap;
        maplen = (sizeof WindMap) / (sizeof *WindMap);
        break;
    case AIRTEMP__GRAPHIC_INDEX:
        map = AirTempMap;
        maplen = (sizeof AirTempMap) / (sizeof *AirTempMap);
        break;
    case SEATEMP_GRAPHIC_INDEX:
        map = SeaTempMap;
        maplen = (sizeof SeaTempMap) / (sizeof *SeaTempMap);
        break;
    case PRECIPITATION_GRAPHIC_INDEX:
        map = PrecipitationMap;
        maplen = (sizeof PrecipitationMap) / (sizeof *PrecipitationMap);
        break;
    case CLOUD_GRAPHIC_INDEX:
        map = CloudMap;
        maplen = (sizeof CloudMap) / (sizeof *CloudMap);
        break;
    default:
        return *wxBLACK;
    }

    /* normalize map from 0 to 1 */
    double cmax = map[maplen-1].val;

    for(int i=1; i<maplen; i++) {
        double nmapvala = map[i-1].val/cmax;
        double nmapvalb = map[i].val/cmax;
        if(nmapvalb > val_in || i==maplen-1) {
            wxColour b, c;
            c.Set(map[i].text);
            if(m_bGradualColors) {
                b.Set(map[i-1].text);
                double d = (val_in-nmapvala)/(nmapvalb-nmapvala);
                c.Set((1-d)*b.Red()   + d*c.Red(),
                      (1-d)*b.Green() + d*c.Green(),
                      (1-d)*b.Blue()  + d*c.Blue());
            }
            return c;
        }
    }
    return wxColour(0, 0, 0); /* unreachable */
}

wxString GRIBOverlayFactory::getLabelString(double value, int settings)
{
    int p;
    switch(settings) {
    case 2:
        p = m_Settings.Settings[2].m_Units == 2 ? 2 : 0;
        break;
    case 3:
    case 4:
    case 7:
    case 8:
        p = 1;
        break;
    case 5:
        p = value < 100. ? 2 : value < 10. ? 1 : 0;
        p += m_Settings.Settings[5].m_Units == 1 ? 1 : 0;
            break;

    default :
        p = 0;
    }
    return wxString::Format( _T("%.*f"), p, value );
}

/* return cached wxImage for a given number, or create it if not in the cache */
wxImage &GRIBOverlayFactory::getLabel(double value, int settings, wxColour back_color )
{
    std::map <double, wxImage >::iterator it;
    it = m_labelCache.find(value);
    if (it != m_labelCache.end())
        return m_labelCache[value];

    wxString labels = getLabelString(value, settings);

    wxColour text_color;
    GetGlobalColor( _T ( "UBLCK" ), &text_color );
    wxPen penText(text_color);

    wxBrush backBrush(back_color);

    wxFont mfont( 9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );

    wxScreenDC sdc;
    int w, h;
    sdc.GetTextExtent(labels, &w, &h, NULL, NULL, &mfont);

    int label_offset = 5;

    wxBitmap bm(w +  label_offset*2, h + 2);
    wxMemoryDC mdc(bm);
    mdc.Clear();

    mdc.SetFont( mfont );
    mdc.SetPen(penText);
    mdc.SetBrush(backBrush);
    mdc.SetTextForeground(text_color);
    mdc.SetTextBackground(back_color);

    int xd = 0;
    int yd = 0;
//    mdc.DrawRoundedRectangle(xd, yd, w+(label_offset * 2), h+2, -.25);
    mdc.DrawRectangle(xd, yd, w+(label_offset * 2), h+2);
    mdc.DrawText(labels, label_offset + xd, yd+1);

    mdc.SelectObject(wxNullBitmap);

    m_labelCache[value] = bm.ConvertToImage();

    m_labelCache[value].InitAlpha();

    return m_labelCache[value];
}

void GRIBOverlayFactory::RenderGribBarbedArrows( int settings, GribRecord **pGR,
                                                 PlugIn_ViewPort *vp )
{
    if(!m_Settings.Settings[settings].m_bBarbedArrows)
        return;

    //  Need two records to draw the barbed arrows
    GribRecord *pGRX, *pGRY;
    int idx, idy;
    bool polar;
    SettingsIdToGribId(settings, idx, idy, polar);
    if(idx < 0 || idy < 0)
        return;

    pGRX = pGR[idx];
    pGRY = pGR[idy];

    if(!pGRX || !pGRY)
        return;

    //    Get the the grid
    int imax = pGRX->getNi();                  // Longitude
    int jmax = pGRX->getNj();                  // Latitude

    //    Barbs?
    bool barbs = true;

    //    Set minimum spacing between wind arrows
    double space = barbs ? 30 : 20;

    wxPoint oldpx(-1000, -1000);
    wxPoint oldpy(-1000, -1000);

    wxColour colour;
    GetGlobalColor( _T ( "YELO2" ), &colour );
    for( int i = 0; i < imax; i++ ) {
        double lonl = pGRX->getX( i );

        /* at midpoint of grib so as to avoid problems in projection on
           gribs that go all the way to the north or south pole */
        double latl = pGRX->getY( pGRX->getNj()/2 );
        wxPoint pl;
        GetCanvasPixLL( vp, &pl, latl, lonl );

        if( hypot( pl.x - oldpx.x, pl.y - oldpx.y ) >= space ) {
            oldpx = pl;
            for( int j = 0; j < jmax; j++ ) {
                double lon = pGRX->getX( i );
                double lat = pGRX->getY( j );
                wxPoint p;
                GetCanvasPixLL( vp, &p, lat, lon );

                if( hypot( p.x - oldpy.x, p.y - oldpy.y ) >= space ) {
                    oldpy = p;

                    if(lon > 180)
                        lon -= 360;

                    if( PointInLLBox( vp, lon, lat ) ) {
                        double vx =  pGRX->getValue( i, j );
                        double vy =  pGRY->getValue( i, j );

                        if( vx != GRIB_NOTDEF && vy != GRIB_NOTDEF ) {
                            vx *= 3.6/1.852;            //always in knots to respect the standard
                            vy *= 3.6/1.852;

                            drawWindArrowWithBarbs( settings, p.x, p.y, vx, vy, polar, ( lat < 0. ), colour, vp->rotation );
                        }
                    }
                }
            }
        }
    }
}

void GRIBOverlayFactory::RenderGribIsobar( int settings, GribRecord **pGR,
                                           wxArrayPtrVoid **pIsobarArray, PlugIn_ViewPort *vp )
{
    if(!m_Settings.Settings[settings].m_bIsoBars)
        return;

    //  Need magnitude to draw isobars
    int idx, idy;
    bool polar;
    SettingsIdToGribId(settings, idx, idy, polar);
    if(idx < 0)
        return;

    GribRecord *pGRA = pGR[idx], *pGRM = NULL;

    if(!pGRA)
        return;

    wxColour back_color;
    GetGlobalColor( _T ( "DILG1" ), &back_color );

    /* build magnitude from multiple record types like wind and current */
    if(idy >= 0 && !polar && pGR[idy]) {
        pGRM = GribRecord::MagnitudeRecord(*pGR[idx], *pGR[idy]);
        pGRA = pGRM;
    }

    //    Initialize the array of Isobars if necessary
    if( !pIsobarArray[idx] ) {
        pIsobarArray[idx] = new wxArrayPtrVoid;
        IsoLine *piso;

        wxProgressDialog *progressdialog = NULL;
        wxDateTime start = wxDateTime::Now();

        double min = m_Settings.GetMin(settings);
        double max = m_Settings.GetMax(settings);

        /* convert min and max to units being used */
        double factor = ( settings == 2 && m_Settings.Settings[2].m_Units == 2 ) ? 0.03 : 1.;//divide spacing by 1/3 for PRESURRE & inHG
        for( double press = min; press <= max; press += (m_Settings.Settings[settings].m_iIsoBarSpacing * factor) ) {
            if(progressdialog)
                progressdialog->Update(press-min);
            else {
                wxDateTime now = wxDateTime::Now();
                if((now-start).GetSeconds() > 3 && press-min < (max-min)/2) {
                    progressdialog = new wxProgressDialog(
                        _("Building Isobar map"), _("Wind"), max-min+1, NULL,
                        wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
                }
            }

            piso = new IsoLine( press,
                                m_Settings.CalibrationFactor(settings, press, true),
                                m_Settings.CalibrationOffset(settings), pGRA );

            pIsobarArray[idx]->Add( piso );
        }
        delete progressdialog;
    }

    //    Draw the Isobars
    for( unsigned int i = 0; i < pIsobarArray[idx]->GetCount(); i++ ) {
        IsoLine *piso = (IsoLine *) pIsobarArray[idx]->Item( i );
        piso->drawIsoLine( this, m_pdc, vp, true); //g_bGRIBUseHiDef

        // Draw Isobar labels

        int density = 40;
        int first = 0;

        if(m_pdc)
            piso->drawIsoLineLabels( this, m_pdc, vp, density,
                                     first, getLabel(piso->getValue(), settings, back_color) );
        else
            piso->drawIsoLineLabelsGL( this, vp, density,
                                       first, getLabelString(piso->getValue(), settings),
                                       back_color, m_TexFontNumbers );
    }

    delete pGRM;
}

void GRIBOverlayFactory::RenderGribDirectionArrows( int settings, GribRecord **pGR,
                                                    PlugIn_ViewPort *vp )
{
    if(!m_Settings.Settings[settings].m_bDirectionArrows)
        return;

    //   need two records or a polar record to draw arrows
    GribRecord *pGRX, *pGRY;
    int idx, idy;
    bool polar;
    SettingsIdToGribId(settings, idx, idy, polar);
    if(idx < 0 || idy < 0)
        return;

    pGRX = pGR[idx];
    pGRY = pGR[idy];
    if(!pGRX || !pGRY)
        return;

    //    Get the the grid
    int imax = pGRX->getNi();                  // Longitude
    int jmax = pGRX->getNj();                  // Latitude

    // Set Size
    int arrowWidth = 2;
    int arrowSize;
    if(m_Settings.Settings[settings].m_iDirectionArrowSize == 0)
        arrowSize = 26;
    else
        arrowSize = 16;

    //    Set minimum spacing between arrows
    double space;
    space = arrowSize * 1.1;

    wxPoint oldpx(-1000, -1000);
    wxPoint oldpy(-1000, -1000);

    wxColour colour;
    GetGlobalColor( _T ( "DILG3" ), &colour );

    for( int i = 0; i < imax; i++ ) {
        double lonl = pGRX->getX( i );
        double latl = pGRX->getY( pGRX->getNj()/2 );
        wxPoint pl;
        GetCanvasPixLL( vp, &pl, latl, lonl );

        if( hypot( pl.x - oldpx.x, pl.y - oldpx.y ) >= space ) {
            oldpx = pl;
            for( int j = 0; j < jmax; j++ ) {
                double lon = pGRX->getX( i );
                double lat = pGRX->getY( j );
                wxPoint p;
                GetCanvasPixLL( vp, &p, lat, lon );

                if( hypot( p.x - oldpy.x, p.y - oldpy.y ) >= space ) {
                    oldpy = p;

                    if(lon > 180)
                        lon -= 360;

                    if( PointInLLBox( vp, lon, lat ) ) {
                        if(polar) {
                            double dir = pGRY->getValue( i, j );
                            double sh = pGRX->getValue( i, j );
                            if( dir != GRIB_NOTDEF && sh != GRIB_NOTDEF ){
                                if(m_Settings.Settings[settings].m_iDirectionArrowForm == 0)
                                    drawSingleArrow( p.x, p.y, ((dir - 90) * M_PI / 180.) + vp->rotation, colour, arrowWidth, arrowSize );
                                else if( m_Settings.Settings[settings].m_iDirectionArrowForm == 1 )
                                    drawDoubleArrow( p.x, p.y, ((dir - 90) * M_PI / 180.) + vp->rotation, colour, arrowWidth, arrowSize );
                                else
                                    drawSingleArrow( p.x, p.y, ((dir - 90) * M_PI / 180.) + vp->rotation, colour,
                                        wxMax( 1, wxMin( 8, (int)(sh+0.5)) ), arrowSize );
                            }
                        } else {
                            double vx = pGRX->getValue( i,j ), vy = pGRY->getValue( i,j );
                            if( vx != GRIB_NOTDEF && vy != GRIB_NOTDEF ) {
                                double dir = atan2(vy, -vx);
                                double sh = sqrt( vx * vx + vy * vy );
                                if(m_Settings.Settings[settings].m_iDirectionArrowForm == 0)
                                    drawSingleArrow( p.x, p.y, dir + vp->rotation, colour, arrowWidth, arrowSize );
                                else if( m_Settings.Settings[settings].m_iDirectionArrowForm == 1 )
                                    drawDoubleArrow( p.x, p.y, dir + vp->rotation, colour, arrowWidth, arrowSize );
                                else
                                    drawSingleArrow( p.x, p.y, dir + vp->rotation, colour,
                                        wxMax( 1, wxMin( 8, (int)((8/2.5*sh)+0.5)) ), arrowSize );
                            }
                        }
                    }
                }
            }
        }
    }
}

void GRIBOverlayFactory::RenderGribOverlayMap( int settings, GribRecord **pGR, PlugIn_ViewPort *vp)
{
    if(!m_Settings.Settings[settings].m_bOverlayMap)
        return;

    const int grib_pixel_size = 4;
    bool polar;
    int idx, idy;
    SettingsIdToGribId(settings, idx, idy, polar);
    if(idx < 0 || !pGR[idx])
        return;

    GribRecord *pGRA = pGR[idx], *pGRM = NULL;
    if(!pGRA)
        return;

    if(idy >= 0 && !polar && pGR[idy]) {
        pGRM = GribRecord::MagnitudeRecord(*pGR[idx], *pGR[idy]);
        pGRA = pGRM;
    }

    wxPoint porg;
    GetCanvasPixLL( vp, &porg, pGRA->getLatMax(), pGRA->getLonMin() );

    //    Check two BBoxes....
    //    TODO Make a better Intersect method
    bool bdraw = false;
    if( Intersect( vp, pGRA->getLatMin(), pGRA->getLatMax(),
                   pGRA->getLonMin(), pGRA->getLonMax(),
                   0. ) != _GOUT ) bdraw = true;
    if( Intersect( vp, pGRA->getLatMin(), pGRA->getLatMax(),
                   pGRA->getLonMin() - 360., pGRA->getLonMax() - 360.,
                   0. ) != _GOUT ) bdraw = true;

    if( bdraw ) {
        // If needed, create the overlay
        if( !m_pOverlay[settings] )
            m_pOverlay[settings] = new GribOverlay;

        GribOverlay *pGO = m_pOverlay[settings];

        if( !m_pdc )       //OpenGL mode
        {
#ifdef ocpnUSE_GL

            if(!texture_format) {
                // prefer npot over texture rectangles
                if( QueryExtension( "GL_ARB_texture_non_power_of_two" ) ||
                    QueryExtension( "GL_OES_texture_npot" ) )
                    texture_format = GL_TEXTURE_2D;
                else if( QueryExtension( "GL_ARB_texture_rectangle" ) )
                    texture_format = GL_TEXTURE_RECTANGLE_ARB;
            }

            if(!texture_format) // it's very unlikely to not have any of the above extensions
                m_Message_Hiden.Append(_("Overlays not supported by this graphics hardware (Disable OpenGL)"));
            else {
                if( !pGO->m_iTexture )
                    CreateGribGLTexture( pGO, settings, pGRA, vp, 1 );

                if( pGO->m_iTexture )
                    DrawGLTexture( pGO->m_iTexture, pGO->m_width, pGO->m_height,
                               porg.x, porg.y, pGO->m_dwidth, pGO->m_dheight, vp );
                else
                    m_Message_Hiden.IsEmpty()?
                        m_Message_Hiden.Append(_("Overlays too wide and can't be displayed:"))
                        .Append(_T(" ")).Append(GribOverlaySettings::NameFromIndex(settings))
                        : m_Message_Hiden.Append(_T(",")).Append(GribOverlaySettings::NameFromIndex(settings));
            }
#endif
        }
        else        //DC mode
        {
            if(fabs(vp->rotation) > 0.1){
                m_Message_Hiden.Append(_("overlays suppressed if not north-up in DC mode (enable OpenGL)"));
            }
            else {
                if( !pGO->m_pDCBitmap ) {
                    wxImage bl_image = CreateGribImage( settings, pGRA, vp, grib_pixel_size, porg );
                    if( bl_image.IsOk() ) {
                    //    Create a Bitmap
                        pGO->m_pDCBitmap = new wxBitmap( bl_image );
                        wxMask *gr_mask = new wxMask( *( pGO->m_pDCBitmap ), wxColour( 0, 0, 0 ) );
                        pGO->m_pDCBitmap->SetMask( gr_mask );
                    }
                }

                if( pGO->m_pDCBitmap )
                    m_pdc->DrawBitmap( *( pGO->m_pDCBitmap ), porg.x, porg.y, true );
                else
                    m_Message_Hiden.IsEmpty()?
                        m_Message_Hiden.Append(_("Please Zoom or Scale Out to view invisible overlays:"))
                        .Append(_T(" ")).Append(GribOverlaySettings::NameFromIndex(settings))
                        : m_Message_Hiden.Append(_T(",")).Append(GribOverlaySettings::NameFromIndex(settings));
            }
        }
    }

    delete pGRM;
}

void GRIBOverlayFactory::RenderGribNumbers( int settings, GribRecord **pGR, PlugIn_ViewPort *vp )
{
    if(!m_Settings.Settings[settings].m_bNumbers)
        return;

    //  Need magnitude to draw numbers
    int idx, idy;
    bool polar;
    SettingsIdToGribId(settings, idx, idy, polar);
    if(idx < 0)
        return;

    GribRecord *pGRA = pGR[idx], *pGRM = NULL;

    if(!pGRA)
        return;

    /* build magnitude from multiple record types like wind and current */
    if(idy >= 0 && !polar && pGR[idy]) {
        pGRM = GribRecord::MagnitudeRecord(*pGR[idx], *pGR[idy]);
        pGRA = pGRM;
    }

    //    Get the the grid
    int imax = pGRA->getNi();                  // Longitude
    int jmax = pGRA->getNj();                  // Latitude

    //    Set minimum spacing between numbers
    int space = m_Settings.Settings[settings].m_iNumbersSpacing;

    wxPoint oldpx(-1000, -1000);
    wxPoint oldpy(-1000, -1000);

    for( int i = 0; i < imax; i++ ) {
        double lonl = pGRA->getX( i );
        double latl = pGRA->getY( pGRA->getNj()/2 );
        wxPoint pl;
        GetCanvasPixLL( vp, &pl, latl, lonl );

        if( hypot( pl.x - oldpx.x, pl.y - oldpx.y ) >= space ) {
            oldpx = pl;
            for( int j = 0; j < jmax; j++ ) {
                double lon = pGRA->getX( i );
                double lat = pGRA->getY( j );
                wxPoint p;
                GetCanvasPixLL( vp, &p, lat, lon );

                if( hypot( p.x - oldpy.x, p.y - oldpy.y ) >= space ) {
                    oldpy = p;

                    if(lon > 180)
                        lon -= 360;

                    if( PointInLLBox( vp, lon, lat ) ) {
                        double mag = pGRA->getValue( i, j );

                        if( mag != GRIB_NOTDEF ) {
                            double value = m_Settings.CalibrateValue(settings, mag);
                            wxColour back_color = GetGraphicColor(settings, value);

                            if( m_pdc ) {
                                wxImage &label = getLabel(value, settings, back_color);
                                //set alpha chanel
                                int w = label.GetWidth(), h = label.GetHeight();
                                for( int y = 0; y < h; y++ )
                                    for( int x = 0; x < w; x++ )
                                    label.SetAlpha( x, y, m_Settings.m_iOverlayTransparency );

                                m_pdc->DrawBitmap(label, p.x, p.y, true);
                            } else {
#ifdef ocpnUSE_GL
                                glEnable( GL_BLEND );
                                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                                glColor4ub(back_color.Red(), back_color.Green(),
                                           back_color.Blue(), m_Settings.m_iOverlayTransparency);

                                wxString label = getLabelString(value, settings);
                                int w, h;
                                m_TexFontNumbers.GetTextExtent( label, &w, &h );

                                int label_offsetx = 5, label_offsety = 1;
                                int x = p.x - label_offsetx, y = p.y - label_offsety;
                                w += 2*label_offsetx, h += 2*label_offsety;

                                /* draw bounding rectangle */
                                glBegin(GL_QUADS);
                                glVertex2i(x,   y);
                                glVertex2i(x+w, y);
                                glVertex2i(x+w, y+h);
                                glVertex2i(x,   y+h);
                                glEnd();

                                glColor4ub( 0, 0, 0, m_Settings.m_iOverlayTransparency );

                                glBegin(GL_LINE_LOOP);
                                glVertex2i(x,   y);
                                glVertex2i(x+w, y);
                                glVertex2i(x+w, y+h);
                                glVertex2i(x,   y+h);
                                glEnd();

                                glEnable(GL_TEXTURE_2D);
                                m_TexFontNumbers.RenderString( label, p.x, p.y );
                                glDisable(GL_TEXTURE_2D);
#endif
                            }
                        }
                    }
                }
            }
        }
    }

    delete pGRM;
}

void GRIBOverlayFactory::RenderGribParticles( int settings, GribRecord **pGR,
                                              PlugIn_ViewPort *vp )
{
    if(!m_Settings.Settings[settings].m_bParticles)
        return;

    //   need two records or a polar record to draw arrows
    GribRecord *pGRX, *pGRY;
    int idx, idy;
    bool polar;
    SettingsIdToGribId(settings, idx, idy, polar);
    if(idx < 0 || idy < 0)
        return;

    pGRX = pGR[idx];
    pGRY = pGR[idy];

    if(!pGRX || !pGRY)
        return;

    wxStopWatch sw;
    sw.Start();

    if(m_ParticleMap && m_ParticleMap->m_Setting != settings) {
        delete m_ParticleMap;
        m_ParticleMap = NULL;
    }

   if(!m_ParticleMap)
        m_ParticleMap = new ParticleMap(settings);

    std::list<Particle> &particles = m_ParticleMap->m_Particles;

    const int max_duration = 200;
    int history_size = 32 / sqrt(m_Settings.Settings[settings].m_dParticleDensity);
    history_size = wxMin(history_size, MAX_PARTICLE_HISTORY);

    std::list<Particle>::iterator it;
    // if the history size changed
    if(m_ParticleMap->history_size != history_size) {
        it = particles.begin();

        while(it != particles.end()) {
            if(m_ParticleMap->history_size > history_size) {
                if(it->m_HistoryPos >= history_size) {
                    it = particles.erase(it);
                    continue;
                }
            }

            it->m_HistorySize = it->m_HistoryPos+1;
            it++;
        }

        m_ParticleMap->history_size = history_size;
    }

    // update particle map
    if(m_bUpdateParticles) {
        it = particles.begin();
        while(it != particles.end()) {
        // don't allow particle to live too long
            if(it->m_Duration > max_duration) {
                it = particles.erase(it);
                continue;
            }

            it->m_Duration++;

            wxPoint2DDouble &pp = it->m_History[it->m_HistoryPos].m_Pos;

            // maximum history size
            if(++it->m_HistorySize > history_size)
                it->m_HistorySize = history_size;

            if(++it->m_HistoryPos >= history_size)
                it->m_HistoryPos = 0;

            wxPoint2DDouble &p = it->m_History[it->m_HistoryPos].m_Pos;
            double vkn=0, ang;
            if(it->m_Duration < max_duration - history_size &&
               GribRecord::getInterpolatedValues(vkn, ang, pGRX, pGRY, pp.m_x, pp.m_y) &&
               vkn > 0 && vkn < 100 ) {
                vkn = m_Settings.CalibrateValue(settings, vkn);
                double d;
                if(settings == GribOverlaySettings::CURRENT)
                    d = vkn;
                else
                    d = vkn/4;

                PositionBearingDistanceMercator_Plugin(pp.m_y, pp.m_x, ang+180,
                                                       d,
                                                       &p.m_y, &p.m_x);

                wxColor c = GetGraphicColor(settings, vkn);

                it->m_History[it->m_HistoryPos].m_Color[0] = c.Red();
                it->m_History[it->m_HistoryPos].m_Color[1] = c.Green();
                it->m_History[it->m_HistoryPos].m_Color[2] = c.Blue();
            } else
                p.m_x = -10000;

            it++;
        }
    }

    m_bUpdateParticles = false;

    int total_particles = m_Settings.Settings[settings].m_dParticleDensity * pGRX->getNi() * pGRX->getNj();

    // set max cap to avoid locking the program up
    if(total_particles > 200000)
        total_particles = 200000;

    // remove particles if needed;
    int size = (int)particles.size();
    for(int i = 0; i<size - total_particles; i++)
        particles.pop_back();

    // add new particles as needed
    while(size < total_particles) {

        wxPoint2DDouble p;
        double vkn, ang;
        for(int i=0; i<20; i++) {
            // random position in the grib area
            p.m_x = (double)rand() / RAND_MAX * (pGRX->getLonMax() - pGRX->getLonMin()) + pGRX->getLonMin();
            p.m_y = (double)rand() / RAND_MAX * (pGRX->getLatMax() - pGRX->getLatMin()) + pGRX->getLatMin();

            if(GribRecord::getInterpolatedValues(vkn, ang, pGRX, pGRY, p.m_x, p.m_y) &&
               vkn > 0 && vkn < 100)
                vkn = m_Settings.CalibrateValue(settings, vkn);
            else
                continue; // try again

            /* try hard to find a random position where current is faster than 1 knot */
            if(settings != GribOverlaySettings::CURRENT || vkn > 1 - (double)i/20)
                break;
        }

        Particle np;
        np.m_Duration = rand()%(max_duration/2);
        np.m_HistoryPos = 0;
        np.m_HistorySize = 1;
        np.m_History[np.m_HistoryPos].m_Pos = p;

        wxColour c;
        c = GetGraphicColor(settings, vkn);
        np.m_History[np.m_HistoryPos].m_Color[0] = c.Red();
        np.m_History[np.m_HistoryPos].m_Color[1] = c.Green();
        np.m_History[np.m_HistoryPos].m_Color[2] = c.Blue();

        particles.push_back(np);
        size++;
    }

    // settings for opengl lines
    if( !m_pdc ) {
        glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_ENABLE_BIT |
                     GL_POLYGON_BIT | GL_HINT_BIT ); //Save state

        //      Enable anti-aliased lines, at best quality
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
        glLineWidth( 2.3f );
    }

    // draw particles
    for(std::list<Particle>::iterator it = particles.begin();
        it != particles.end(); it++) {

        int alpha = 250;

        int i = it->m_HistoryPos;

        bool first = true;
        wxPoint l;

        if( !m_pdc )
            glBegin( GL_LINE_STRIP );

        do {
            int runs = 4;
            wxPoint2DDouble dp = it->m_History[i].m_Pos;
            if(dp.m_x != -10000) {
                wxPoint p;
                GetCanvasPixLL( vp, &p, dp.m_y, dp.m_x);

                wxUint8 (&c)[3] = it->m_History[i].m_Color;
                bool wrap = first ? false : abs(l.x - p.x) > vp->pix_width;

                if( m_pdc ) {
                    if(!first && !wrap) {
                        m_pdc->SetPen(wxPen( wxColour(c[0], c[1], c[2] + 240-alpha/2), 2 ));
                        m_pdc->DrawLine( p.x, p.y, l.x, l.y );
                    }

                } else {
                    if(wrap) {
                        glEnd();
                        glBegin( GL_LINE_STRIP );
                    }

                    glColor4ub( c[0], c[1], c[2] + 240-alpha/2, alpha);
                    glVertex2d( p.x, p.y );
                }

                l = p;
                first = false;
            } else
                runs = 1;

            alpha -= 240 / history_size * runs;

            /* skip entries */
            for(int c = 0; c<runs; c++) {
                if(--i < 0) {
                    i = history_size - 1;
                    if(i >= it->m_HistorySize)
                        goto outer_break;
                }

                if(i == it->m_HistoryPos)
                    goto outer_break;
            }

        } while(1);
    outer_break:

        if( !m_pdc ){
            glEnd();
        }

    //  On some platforms, especially slow ones, the GPU will lag behind the CPU.
    //  This affects the UI in strange ways.
    //  So, force the GPU to flush all of its outstanding commands on the outer loop
    //  This will have no real affect on most machines.
#ifdef __WXMSW__
    if( !m_pdc ){
        glFlush();
    }
#endif

    }

    if( !m_pdc )
        glPopAttrib();

    int time = sw.Time();

    //  Try to run at 20 fps,
    //  But also arrange not to consume more than 33% CPU(core) duty cycle
    m_tParticleTimer.Start(wxMax(50 - time, 2 * time), wxTIMER_ONE_SHOT);

#if 0
    static int total_time;
    total_time += time;
    static int total_count;
    if(++total_count == 100) {
        printf("time: %.2f\n", (double)total_time / total_count);
        total_time = total_count = 0;
    }
#endif
}

void GRIBOverlayFactory::OnParticleTimer( wxTimerEvent & event )
{
    m_bUpdateParticles = true;
    GetOCPNCanvasWindow()->Refresh(false);
}

void GRIBOverlayFactory::DrawMessageWindow( wxString msg, int x, int y , wxFont *mfont)
{
    if(msg.empty())
        return;

    if(m_pdc) {
        wxDC &dc = *m_pdc;
        dc.SetFont( *mfont );
        dc.SetPen( *wxTRANSPARENT_PEN);

        dc.SetBrush( wxColour(243, 229, 47 ) );
        int w, h;
        dc.GetMultiLineTextExtent( msg, &w, &h );
        h += 2;
        int yp = y - ( GetChartbarHeight() + h );

        int label_offset = 10;
        int wdraw = w + ( label_offset * 2 );
        dc.DrawRectangle( 0, yp, wdraw, h );
        dc.DrawLabel( msg, wxRect( label_offset, yp, wdraw, h ),
                      wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL);
    } else {
        m_TexFontMessage.Build(*mfont);
        int w, h;
        m_TexFontMessage.GetTextExtent( msg, &w, &h);
        h += 2;
        int yp = y - ( GetChartbarHeight() + h );

        glColor3ub( 243, 229, 47 );

        glBegin(GL_QUADS);
        glVertex2i(0, yp);
        glVertex2i(w, yp);
        glVertex2i(w, yp+h);
        glVertex2i(0, yp+h);
        glEnd();

        glEnable(GL_BLEND);
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        glColor3ub( 0, 0, 0 );
        glEnable(GL_TEXTURE_2D);
        m_TexFontMessage.RenderString( msg, 0, yp);
        glDisable(GL_TEXTURE_2D);
    }
}

void GRIBOverlayFactory::drawDoubleArrow( int i, int j, double ang, wxColour arrowColor, int arrowWidth, int arrowSize )
{
    double si = sin( ang ), co = cos( ang );

    wxPen pen( arrowColor, 2 );

    if( m_pdc ) {
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
    }

    int dec = -arrowSize / 2;

    drawTransformedLine( pen, si, co, i, j, dec, -2, dec + arrowSize, -2 );
    drawTransformedLine( pen, si, co, i, j, dec, 2, dec + arrowSize, +2 );

    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, 6 );    // flèche
    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, -6 );   // flèche

}

void GRIBOverlayFactory::drawSingleArrow( int i, int j, double ang, wxColour arrowColor, int arrowWidth, int arrowSize )
{
    double si = sin( ang ), co = cos( ang );

    wxPen pen( arrowColor, arrowWidth );

    if( m_pdc ) {
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
    }

    int dec = -arrowSize / 2;

    drawTransformedLine( pen, si, co, i, j, dec, 0, dec + arrowSize, 0 );

    pen.SetWidth( arrowWidth < 2 ? 1 : 2 );

    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, 6 );    // flèche
    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, -6 );   // flèche

}

void GRIBOverlayFactory::drawWindArrowWithBarbs( int settings, int i, int j, double vx, double vy,
                                                 bool polar, bool south, wxColour arrowColor,
                                                 double rotate_angle )
{
    double vkn, ang;

    if(polar) {
        vkn = vx;
        ang = vy * M_PI/180;
    } else {
        vkn = sqrt( vx * vx + vy * vy );
        ang = atan2( vy, -vx );
    }

    if(m_Settings.Settings[settings].m_iBarbedColour == 1)
        arrowColor = GetGraphicColor(settings, vkn);

    ang += rotate_angle;
    double si = sin( ang ), co = cos( ang );

    wxPen pen( arrowColor, 2 );

    if( m_pdc ) {
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
    }

    if( vkn < 1 ) {
        int r = 5;     // wind is very light, draw a circle
        if( m_pdc )
            m_pdc->DrawCircle( i, j, r );
        else {
#ifdef ocpnUSE_GL
            double w = pen.GetWidth(), s = 2 * M_PI / 10.;
            if( m_hiDefGraphics ) w *= 0.75;
            wxColour c = pen.GetColour();
            glColor4ub( c.Red(), c.Green(), c.Blue(), 255);
            for( double a = 0; a < 2 * M_PI; a += s )
                DrawGLLine( i + r*sin(a), j + r*cos(a), i + r*sin(a+s), j + r*cos(a+s), w );
#endif
        }
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

    if( m_pdc ) {
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
#ifdef ocpnUSE_GL
        wxColour c = pen.GetColour();
        glColor4ub( c.Red(), c.Green(), c.Blue(), 255);
        double w = pen.GetWidth();
        if( m_hiDefGraphics ) w *= 0.75;
        DrawGLLine( fi, fj, fk, fl, w );
#endif
    }
}

void GRIBOverlayFactory::drawPetiteBarbule( wxPen pen, bool south, double si, double co, int di,
        int dj, int b )
{
    if( south )
        drawTransformedLine( pen, si, co, di, dj, b, 0, b + 2, -5 );
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

#ifdef ocpnUSE_GL
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
#endif

#ifdef ocpnUSE_GL
void GRIBOverlayFactory::DrawGLTexture( GLuint texture, int width, int height,
                                        int xd, int yd, double dwidth, double dheight,
                                        PlugIn_ViewPort *vp )
{
    glEnable(texture_format);
    glBindTexture(texture_format, texture);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1, 1, 1, 1);

    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

    //    Adjust for rotation
    glPushMatrix();
    if( fabs( vp->rotation ) > 0.01 ) {

        //    Rotations occur around 0,0, so calculate a post-rotate translation factor
        double angle = vp->rotation;
        angle -= vp->skew;

        glTranslatef( xd, yd, 0 );
        glRotatef( angle * 180. / PI, 0, 0, 1 );
        glTranslatef( -xd, -yd, 0 );
    }

    double x = xd, y = yd;

    double w = dwidth * vp->view_scale_ppm;
    double h = dheight * vp->view_scale_ppm;

    if(texture_format == GL_TEXTURE_2D)
        width = height = 1;

    glBegin(GL_QUADS);
    glTexCoord2i(0, 0),          glVertex2i(x, y);
    glTexCoord2i(width, 0),      glVertex2i(x+w, y);
    glTexCoord2i(width, height), glVertex2i(x+w, y+h);
    glTexCoord2i(0, height),     glVertex2i(x, y+h);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(texture_format);

    glPopMatrix();
}
#endif
