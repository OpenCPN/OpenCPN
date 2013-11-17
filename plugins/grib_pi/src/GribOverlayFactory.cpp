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
#endif //precompiled headers

#include <wx/glcanvas.h>
#include <wx/graphics.h>
#include <wx/progdlg.h>

#include "GribUIDialog.h"
#include "GribOverlayFactory.h"

enum OVERLAP { _IN, _ON, _OUT };

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
}

GRIBOverlayFactory::~GRIBOverlayFactory()
{
    ClearCachedData();
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

void SettingsIdToGribId(int i, int &idx, int &idy, bool &polar)
{
    idx = idy = -1;
    polar = false;
    switch(i) {
    case GribOverlaySettings::WIND:
        idx = Idx_WIND_VX, idy = Idx_WIND_VY; break;
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
        idx = Idx_AIR_TEMP_2M; break;
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
    m_Message_Hiden.Empty();

    //    If the scale has changed, clear out the cached bitmaps
    if( vp->view_scale_ppm != m_last_vp_scale )
        ClearCachedData();

    m_last_vp_scale = vp->view_scale_ppm;

    //     render each type of record
    GribRecord **pGR = m_pGribTimelineRecordSet->m_GribRecordPtrArray;
    wxArrayPtrVoid **pIA = m_pGribTimelineRecordSet->m_IsobarArray;
    
    for(int overlay = 1; overlay >= 0; overlay--)
    for(int i=0; i<GribOverlaySettings::SETTINGS_COUNT; i++) {
        if((i == GribOverlaySettings::WIND            && !m_dlg.m_cbWind->GetValue()) ||
           (i == GribOverlaySettings::WIND_GUST       && !m_dlg.m_cbWindGust->GetValue()) ||
           (i == GribOverlaySettings::PRESSURE        && !m_dlg.m_cbPressure->GetValue()) ||
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
        }
    }
    if( !m_Message_Hiden.IsEmpty() )
        DrawMessageWindow( m_Message_Hiden , vp->pix_width, vp->pix_height, m_dFont_map );
    return true;
}

bool GRIBOverlayFactory::CreateGribGLTexture( GribOverlay *pGO, int settings, GribRecord *pGR,
                                              PlugIn_ViewPort *vp, int grib_pixel_size,
                                              const wxPoint &porg )
{
    wxPoint pmin;
    GetCanvasPixLL( vp, &pmin, pGR->getLatMin(), pGR->getLonMin() );
    wxPoint pmax;
    GetCanvasPixLL( vp, &pmax, pGR->getLatMax(), pGR->getLonMax() );

    int width = abs( pmax.x - pmin.x );// /grib_pixel_size;
    int height = abs( pmax.y - pmin.y );// /grib_pixel_size;

    //    Dont try to create enormous GRIB textures ( no more than the screen size )
    if( width > m_ParentSize.GetWidth() || height > m_ParentSize.GetHeight() )
        return false;

    unsigned char *data = new unsigned char[width*height*4];

    for( int ipix = 0; ipix < width; ipix++ ) {
        for( int jpix = 0; jpix < height; jpix++ ) {
            wxPoint p;
            p.x = grib_pixel_size*ipix + porg.x;
            p.y = grib_pixel_size*jpix + porg.y;

            double lat, lon;
            GetCanvasLLPix( vp, p, &lat, &lon );

            double v = pGR->getInterpolatedValue(lon, lat);
            unsigned char r, g, b, a;
            if( v != GRIB_NOTDEF ) {
                v = m_Settings.CalibrateValue(settings, v);
                wxColour c = GetGraphicColor(settings, v);
                r = c.Red();
                g = c.Green();
                b = c.Blue();
                a = m_Settings.m_iOverlayTransparency;
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

    return true;
}

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
                
                unsigned char r = c.Red();
                unsigned char g = c.Green();
                unsigned char b = c.Blue();
                
                for( int xp = 0; xp < grib_pixel_size; xp++ )
                    for( int yp = 0; yp < grib_pixel_size; yp++ ) {
                        gr_image.SetRGB( ipix + xp, jpix + yp, r, g, b );
                        gr_image.SetAlpha( ipix + xp, jpix + yp, m_Settings.m_iOverlayTransparency);
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

/* return cached wxImage for a given number, or create it if not in the cache */
wxImage &GRIBOverlayFactory::getLabel(double value, int settings)
{
    std::map <double, wxImage >::iterator it;
    it = m_labelCache.find(value);
    if (it != m_labelCache.end())
        return m_labelCache[value];

    wxString labels;
    int p =  settings == 2 && m_Settings.Settings[2].m_Units == 2 ? 2 : value < 10 ? 1 : 0;//two decimals for pressure & inHG, one for small values
    labels.Printf( _T("%.*f"), p, value );

    wxColour text_color;
    GetGlobalColor( _T ( "DILG3" ), &text_color );
    wxColour back_color;
    wxPen penText(text_color);

    GetGlobalColor( _T ( "DILG0" ), &back_color );
    wxBrush backBrush(back_color);

    wxMemoryDC mdc(wxNullBitmap);

    wxFont mfont( 9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    mdc.SetFont( mfont );

    int w, h;
    mdc.GetTextExtent(labels, &w, &h);

    int label_offset = 5;

    wxBitmap bm(w +  label_offset*2, h + 2);
    mdc.SelectObject(bm);
    mdc.Clear();

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

    wxImage &image = m_labelCache[value];

    unsigned char *d = image.GetData();
    unsigned char *a = image.GetAlpha();

    w = image.GetWidth(), h = image.GetHeight();
    for( int y = 0; y < h; y++ )
        for( int x = 0; x < w; x++ ) {
            int r, g, b;
            int ioff = (y * w + x);
            r = d[ioff* 3 + 0];
            g = d[ioff* 3 + 1];
            b = d[ioff* 3 + 2];

            a[ioff] = 255-(r+g+b)/3;
        }

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
    int space;

    if( barbs )
        space = 30;
    else
        space = 20;

    int oldx = -1000;
    int oldy = -1000;

    wxColour colour;
    GetGlobalColor( _T ( "YELO2" ), &colour );
    for( int i = 0; i < imax; i++ ) {
        double lonl = pGRX->getX( i );
        /* at midpoint of grib so as to avoid problems in projection on
           gribs that go all the way to the north or south pole */
        double latl = pGRX->getY( pGRX->getNj()/2 );
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

                        if( vx != GRIB_NOTDEF && vy != GRIB_NOTDEF ) {
                            vx = m_Settings.CalibrateValue(settings, vx);
                            vy = m_Settings.CalibrateValue(settings, vy);

                            drawWindArrowWithBarbs( settings, p.x, p.y, vx, vy, polar, ( lat < 0. ), colour );
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
                                m_Settings.CalibrationFactor(settings),
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

        piso->drawIsoLineLabels( this, m_pdc, vp, density,
                                 first, getLabel(piso->getValue(), settings) );
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

    if(polar) {
        pGRX = pGR[idy];
        if(!pGRX)
            return;
    } else {
        pGRX = pGR[idx];
        pGRY = pGR[idy];
        if(!pGRX || !pGRY)
            return;
    }

    //    Get the the grid
    int imax = pGRX->getNi();                  // Longitude
    int jmax = pGRX->getNj();                  // Latitude

    //    Set minimum spacing between arrows
    int space;
    space = 60;

    int oldx = -1000;
    int oldy = -1000;

    wxColour colour;
    GetGlobalColor( _T ( "UBLCK" ), &colour );

    for( int i = 0; i < imax; i++ ) {
        double lonl = pGRX->getX( i );
        double latl = pGRX->getY( pGRX->getNj()/2 );
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
                        if(polar) {
                            double dir = pGRX->getValue( i, j );
                            if( dir != GRIB_NOTDEF )
                                drawWaveArrow( p.x, p.y, (dir - 90) * M_PI / 180, colour );
                        } else {
                            double vx = pGRX->getValue( i,j ), vy = pGRY->getValue( i,j );
                            if( vx != GRIB_NOTDEF || vy != GRIB_NOTDEF )
                                drawWaveArrow( p.x, p.y, atan2(vy, -vx), colour );
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
                   0. ) != _OUT ) bdraw = true;
    if( Intersect( vp, pGRA->getLatMin(), pGRA->getLatMax(),
                   pGRA->getLonMin() - 360., pGRA->getLonMax() - 360.,
                   0. ) != _OUT ) bdraw = true;

    if( bdraw ) {
        // If needed, create the overlay
        if( !m_pOverlay[settings] )
            m_pOverlay[settings] = new GribOverlay;

        GribOverlay *pGO = m_pOverlay[settings];

        if( !m_pdc )       //OpenGL mode
        {
            if( !pGO->m_iTexture )
                CreateGribGLTexture( pGO, settings, pGRA, vp,
                                     grib_pixel_size, porg);

            if( pGO->m_iTexture )
                DrawGLTexture( pGO->m_iTexture, pGO->m_width, pGO->m_height,
                               porg.x, porg.y, grib_pixel_size );
            else
                m_Message_Hiden.IsEmpty()?
                    m_Message_Hiden.Append(_("Please Zoom or Scale Out to view invisible overlays:"))
                    .Append(_T(" ")).Append(GribOverlaySettings::NameFromIndex(settings))
                    : m_Message_Hiden.Append(_T(",")).Append(GribOverlaySettings::NameFromIndex(settings));
        }
        else        //DC mode
        {
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

    //    Set minimum spacing between arrows
    int space = m_Settings.Settings[settings].m_iNumbersSpacing;

    int oldx = -1000;
    int oldy = -1000;

    wxColour colour;
    GetGlobalColor( _T ( "UBLCK" ), &colour );

    for( int i = 0; i < imax; i++ ) {
        double lonl = pGRA->getX( i );
        double latl = pGRA->getY( pGRA->getNj()/2 );
        wxPoint pl;
        GetCanvasPixLL( vp, &pl, latl, lonl );

        if( abs( pl.x - oldx ) >= space ) {
            oldx = pl.x;
            for( int j = 0; j < jmax; j++ ) {
                double lon = pGRA->getX( i );
                double lat = pGRA->getY( j );
                wxPoint p;
                GetCanvasPixLL( vp, &p, lat, lon );

                if( abs( p.y - oldy ) >= space ) {
                    oldy = p.y;

                    if( PointInLLBox( vp, lon, lat ) || PointInLLBox( vp, lon - 360., lat ) ) {
                        double mag = pGRA->getValue( i, j );

                        if( mag != GRIB_NOTDEF ) {
                            double value = m_Settings.CalibrateValue(settings, mag);
                            wxImage &label = getLabel(value, settings);
                            if( m_pdc ) {
                                m_pdc->DrawBitmap(label, p.x, p.y, true);
                            } else {
                                int w = label.GetWidth(), h = label.GetHeight();
#if 0 /* this way is more work on our part.. try it for debugging purposes */
                                unsigned char *d = label.GetData(), *a = label.GetAlpha();
                                unsigned char *e = new unsigned char[4*w*h];
                                for(int i=0; i<w*h; i++) {
                                    for(int c=0; c<3; c++)
                                        e[4*i+c] = d[3*i+c];
                                    e[4*i+3] = a[3*i];
                                }

                                glRasterPos2i(p.x, p.y);
                                glPixelZoom(1, -1); /* draw data from top to bottom */

                                glEnable( GL_BLEND );
                                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                                glDrawPixels(label.GetWidth(), label.GetHeight(),
                                             GL_RGBA, GL_UNSIGNED_BYTE, e);
                                glPixelZoom(1, 1);

                                delete [] e;
#else /* this way use opengl textures.. should be best */
                                glEnable( GL_BLEND );
                                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                                glColor4f(0, 0, 0, 1);
                                glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
                                glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
                                glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA,
                                             w, h, 0,
                                             GL_RGB, GL_UNSIGNED_BYTE, label.GetData());
                                glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0,
                                                w, h,
                                                GL_ALPHA, GL_UNSIGNED_BYTE, label.GetAlpha());

                                glEnable(GL_TEXTURE_RECTANGLE_ARB);
                                glBegin(GL_QUADS);
                                glTexCoord2i(0, 0), glVertex2i(p.x,   p.y);
                                glTexCoord2i(w, 0), glVertex2i(p.x+w, p.y);
                                glTexCoord2i(w, h), glVertex2i(p.x+w, p.y+h);
                                glTexCoord2i(0, h), glVertex2i(p.x,   p.y+h);
                                glEnd();
                                glDisable(GL_TEXTURE_RECTANGLE_ARB);
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

void GRIBOverlayFactory::DrawMessageWindow( wxString msg, int x, int y , wxFont *mfont)
{
    if(msg.empty())
        return;

    wxMemoryDC mdc;
    wxBitmap bm( 1000, 1000 );
    mdc.SelectObject( bm );
    mdc.Clear();

    //wxFont mfont( 15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL );
    mdc.SetFont( *mfont );
    mdc.SetPen( *wxTRANSPARENT_PEN);
   // mdc.SetBrush( *wxLIGHT_GREY_BRUSH );
    mdc.SetBrush( wxColour(243, 229, 47 ) );
    int w, h;
    mdc.GetMultiLineTextExtent( msg, &w, &h );
    h += 2;
    int label_offset = 10;
    int wdraw = w + ( label_offset * 2 );
    mdc.DrawRectangle( 0, 0, wdraw, h );

    mdc.DrawLabel( msg, wxRect( label_offset, 0, wdraw, h ), wxALIGN_LEFT| wxALIGN_CENTRE_VERTICAL);
    mdc.SelectObject( wxNullBitmap );

    wxBitmap sbm = bm.GetSubBitmap( wxRect( 0, 0, wdraw, h ) );

    DrawOLBitmap( sbm, 0, y - ( GetChartbarHeight() + h ), false );
}

void GRIBOverlayFactory::drawWaveArrow( int i, int j, double ang, wxColour arrowColor )
{
    double si = sin( ang ), co = cos( ang );

    wxPen pen( arrowColor, 2 );

    if( m_pdc ) {
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

    if( m_pdc ) {
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
    }

    int arrowSize = 26;
    int dec = -arrowSize / 2;

    drawTransformedLine( pen, si, co, i, j, dec, 0, dec + arrowSize, 0 );

    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, 6 );    // flèche
    drawTransformedLine( pen, si, co, i, j, dec - 2, 0, dec + 5, -6 );   // flèche

}

void GRIBOverlayFactory::drawWindArrowWithBarbs( int settings, int i, int j, double vx, double vy,
                                                 bool polar, bool south, wxColour arrowColor )
{
    double vkn, ang;

    if(polar) {
        vkn = vx;
        ang = vy * M_PI/180;
    } else {
        vkn = sqrt( vx * vx + vy * vy );
        ang = atan2( vy, -vx );
    }

    double si = sin( ang ), co = cos( ang );

    wxPen pen( arrowColor, 2 );

    if( m_pdc ) {
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
    }

    /* normalize vkn to be from 0-100 instead of 0-m_Settings.Settings[settings].m_bBarbedRange */
    vkn *= 100.0/m_Settings.Settings[settings].m_iBarbedRange;

    if( vkn < 1 ) {
        int r = 5;     // wind is very light, draw a circle
        if( m_pdc )
            m_pdc->DrawCircle( i, j, r );
        else {
            double w = pen.GetWidth(), s = 2 * M_PI / 10;
            if( m_hiDefGraphics ) w *= 0.75;
            wxColour c = pen.GetColour();
            glColor4ub( c.Red(), c.Green(), c.Blue(), 255);
            for( double a = 0; a < 2 * M_PI; a += s )
                DrawGLLine( i + r*sin(a), j + r*cos(a), i + r*sin(a+s), j + r*cos(a+s), w );
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
    if( m_pdc )
        m_pdc->DrawBitmap( bmp, x, y, usemask );
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
