/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  MBTiles chart type support
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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
 *
 */


// ============================================================================
// declarations
// ============================================================================


// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers


//  Why are these not in wx/prec.h?
#include "wx/dir.h"
#include "wx/stream.h"
#include "wx/wfstream.h"
#include "wx/tokenzr.h"
#include "wx/filename.h"
#include <wx/image.h>
#include <wx/fileconf.h>
#include <wx/mstream.h>
#include <sys/stat.h>
#include <sstream>

#include "mbtiles.h"
#include "ocpn_pixel.h"
#include "ChartDataInputStream.h"

#include <SQLiteCpp/SQLiteCpp.h>
#define SQLITE_DONE        101  /* sqlite3_step() has finished executing */

//  Missing from MSW include files
#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif

// ----------------------------------------------------------------------------
// Random Prototypes
// ----------------------------------------------------------------------------

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

#ifdef OCPN_USE_CONFIG
class MyConfig;
extern MyConfig        *pConfig;
#endif

#define LON_UNDEF NAN
#define LAT_UNDEF NAN

// The OpenStreetMaps zommlevel translation tables
//  https://wiki.openstreetmap.org/wiki/Zoom_levels

/*Level   Degree  Area          m / pixel       ~Scale          # Tiles
0       360     whole world     156,412         1:500 million   1
1       180                     78,206          1:250 million   4
2       90                      39,103          1:150 million   16
3       45                      19,551          1:70 million    64
4       22.5                    9,776           1:35 million    256
5       11.25                   4,888           1:15 million    1,024
6       5.625                   2,444           1:10 million    4,096
7       2.813                   1,222           1:4 million     16,384
8       1.406                   610.984         1:2 million     65,536
9       0.703   wide area       305.492         1:1 million     262,144
10      0.352                   152.746         1:500,000       1,048,576
11      0.176   area            76.373          1:250,000       4,194,304
12      0.088                   38.187          1:150,000       16,777,216
13      0.044  village or town  19.093          1:70,000        67,108,864
14      0.022                   9.547           1:35,000        268,435,456
15      0.011                   4.773           1:15,000        1,073,741,824
16      0.005   small road      2.387           1:8,000         4,294,967,296
17      0.003                   1.193           1:4,000         17,179,869,184
18      0.001                   0.596           1:2,000         68,719,476,736
19      0.0005                  0.298           1:1,000         274,877,906,944
*/

// A "nominal" scale value, by zoom factor.  Estimated at equator, with monitor pixel size of 0.3mm 
double OSM_zoomScale[] = { 5e8,
                            2.5e8,
                            1.5e8,
                            7.0e7,
                            3.5e7,
                            1.5e7,
                            1.0e7,
                            4.0e6,
                            2.0e6,
                            1.0e6,
                            5.0e5,
                            2.5e5,
                            1.5e5,
                            7.0e4,
                            3.5e4,
                            1.5e4,
                            8.0e3,
                            4.0e3,
                            2.0e3,
                            1.0e3};
        
//  Meters per pixel, by zoom factor                            
double OSM_zoomMPP[] = { 156412,
                            78206,
                            39103,
                            19551,
                            9776,
                            4888,
                            2444,
                            1222,
                            610,984,
                            305.492,
                            152.746,
                            76.373,
                            38.187,
                            19.093,
                            9.547,
                            4.773,
                            2.387,
                            1.193,
                            0.596,
                            0.298 };
                            
                            


#if defined( __UNIX__ ) && !defined(__WXOSX__)  // high resolution stopwatch for profiling
class OCPNStopWatch
{
public:
    OCPNStopWatch() { Reset(); }
    void Reset() { clock_gettime(CLOCK_REALTIME, &tp); }

    double GetTime() {
        timespec tp_end;
        clock_gettime(CLOCK_REALTIME, &tp_end);
        return (tp_end.tv_sec - tp.tv_sec) * 1.e3 + (tp_end.tv_nsec - tp.tv_nsec) / 1.e6;
    }

private:
    timespec tp;
};
#endif

// *********************************************
//   Utility Functions
// *********************************************

// https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#C.2FC.2B.2B
int long2tilex(double lon, int z)
{
    return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, z)));
}

int lat2tiley(double lat, int z)
{
    int y = (int)(floor((1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z)));
    int ymax  = 1 << z;
    y = ymax - y - 1;
    return y;
}

double tilex2long(int x, int z)
{
    return x / pow(2.0, z) * 360.0 - 180;
}

double tiley2lat(int y, int z)
{
    double n = pow(2.0,z);
    int ymax  = 1 << z;
    y = ymax - y - 1;
    double latRad = atan(sinh(M_PI*(1-(2*y/n))));
    return 180.0 / M_PI * latRad;
}



// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------


//  Per tile descriptor
class mbTileDescriptor
{
public:
    mbTileDescriptor() {  glTextureName = 0; m_bNotAvailable = false; m_bgeomSet = false;}
    
    virtual ~mbTileDescriptor() { }
    
    int tile_x, tile_y;
    int m_zoomLevel;
    float latmin, lonmin, latmax, lonmax;
    LLBBox box;
    
    GLuint glTextureName;
    bool m_bNotAvailable;
    bool m_bgeomSet;
    
};

//  Per zoomlevel descriptor of tile array for that zoomlevel
class mbTileZoomDescriptor
{
public:
    mbTileZoomDescriptor(){}
    virtual ~mbTileZoomDescriptor(){}
    
    int tile_x_min, tile_x_max;
    int tile_y_min, tile_y_max;
    
    int nx_tile, ny_tile;
    
    mbTileDescriptor **m_tileDesc;

};    






// ============================================================================
// ChartMBTiles implementation
// ============================================================================


ChartMBTiles::ChartMBTiles()
{
      //    Init some private data
      m_ChartFamily = CHART_FAMILY_RASTER;
      m_ChartType = CHART_TYPE_MBTILES;

      m_Chart_Skew = 0.0;

      m_datum_str = _T("WGS84");                // assume until proven otherwise
      m_projection = PROJECTION_WEB_MERCATOR;
      m_imageType = wxBITMAP_TYPE_ANY;

      m_b_cdebug = 0;

      m_minZoom = 0;
      m_maxZoom = 19;

      m_nNoCOVREntries = 0;
      m_nCOVREntries = 0;
      m_pCOVRTablePoints = NULL;
      m_pCOVRTable = NULL;
      m_pNoCOVRTablePoints = NULL;
      m_pNoCOVRTable = NULL;
    
      m_LonMin = LON_UNDEF;
      m_LonMax = LON_UNDEF;
      m_LatMin = LAT_UNDEF;
      m_LatMax = LAT_UNDEF;
      
#ifdef OCPN_USE_CONFIG
      wxFileConfig *pfc = (wxFileConfig *)pConfig;
      pfc->SetPath ( _T ( "/Settings" ) );
      pfc->Read ( _T ( "DebugMBTiles" ),  &m_b_cdebug, 0 );
#endif

}

ChartMBTiles::~ChartMBTiles()
{
    FlushTiles();
}

//-------------------------------------------------------------------------------------------------
//          Get the Chart thumbnail data structure
//          Creating the thumbnail bitmap as required
//-------------------------------------------------------------------------------------------------

ThumbData *ChartMBTiles::GetThumbData()
{
    return NULL;
}
    
ThumbData *ChartMBTiles::GetThumbData(int tnx, int tny, float lat, float lon)
{
    return NULL;
}

bool ChartMBTiles::UpdateThumbData(double lat, double lon)
{
    return true;
}

bool ChartMBTiles::AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed)
{
    return true;
}
    


//    Report recommended minimum and maximum scale values for which use of this chart is valid

double ChartMBTiles::GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom)
{
//      if(b_allow_overzoom)
            return (canvas_scale_factor / m_ppm_avg) / 32;         // allow wide range overzoom overscale
//      else
//            return (canvas_scale_factor / m_ppm_avg) / 2;         // don't suggest too much overscale

}

double ChartMBTiles::GetNormalScaleMax(double canvas_scale_factor, int canvas_width)
{
      return (canvas_scale_factor / m_ppm_avg) * 4.0;        // excessive underscale is slow, and unreadable
}


double ChartMBTiles::GetNearestPreferredScalePPM(double target_scale_ppm)
{
    return target_scale_ppm;
}

//Checks/corrects/completes the initialization based on real data from the tiles table
void ChartMBTiles::InitFromTiles( const wxString& name )
{
    try
    {
        // Open the MBTiles database file
        SQLite::Database  db(name.mb_str());
        
        // Check if tiles with advertised min and max zoom level really exist, or correct the defaults
        // We can't blindly use what we find though - the DB often contains empty cells at very low zoom levels, so if we have some info from metadata, we will use that if more conservative...
        SQLite::Statement query(db, "SELECT min(zoom_level) AS min_zoom, max(zoom_level) AS max_zoom FROM tiles");
        while (query.executeStep())
        {
            const char* colMinZoom = query.getColumn(0);
            const char* colMaxZoom = query.getColumn(1);
            
            int zoom;
            sscanf( colMinZoom, "%i", &zoom );
            m_minZoom = wxMax(m_minZoom, zoom);
            sscanf( colMaxZoom, "%i", &zoom );
            m_maxZoom = wxMin(m_maxZoom, zoom);
        }
        
        std::cout << name.c_str() << " zoom_min: " << m_minZoom << " zoom_max: " << m_maxZoom << std::endl;
 
        // Traversing the entire tile table can be expensive....
        //  Use declared bounds if present.
        
        if(!wxIsNaN(m_LatMin) && !wxIsNaN(m_LatMax) && !wxIsNaN(m_LonMin) && !wxIsNaN(m_LonMax) )
            return;
            
        // Try to guess the coverage extents from the tiles. This will be hard to get right - the finest resolution likely does not cover the whole area, while the lowest resolution tiles probably contain a lot of theoretical space which actually is not covered. And some resolutions may be actually missing... What do we use?
        // If we have the metadata and it is not completely off, we should probably prefer it.
        SQLite::Statement query1(db, wxString::Format("SELECT min(tile_row) AS min_row, max(tile_row) as max_row, min(tile_column) as min_column, max(tile_column) as max_column, count(*) as cnt, zoom_level FROM tiles  WHERE zoom_level >= %d AND zoom_level <= %d GROUP BY zoom_level ORDER BY zoom_level ASC", m_minZoom, m_maxZoom).c_str());
        float minLat = 999., maxLat = -999.0, minLon = 999., maxLon = -999.0;
        while (query1.executeStep())
        {
            const char* colMinRow = query1.getColumn(0);
            const char* colMaxRow = query1.getColumn(1);
            const char* colMinCol = query1.getColumn(2);
            const char* colMaxCol = query1.getColumn(3);
            const char* colCnt = query1.getColumn(4);
            const char* colZoom = query1.getColumn(5);
            
            int minRow, maxRow, minCol, maxCol, cnt, zoom;
            sscanf( colMinRow, "%i", &minRow );
            sscanf( colMaxRow, "%i", &maxRow );
            sscanf( colMinCol, "%i", &minCol );
            sscanf( colMaxCol, "%i", &maxCol );
            sscanf( colMinRow, "%i", &minRow );
            sscanf( colMaxRow, "%i", &maxRow );
            sscanf( colCnt, "%i", &cnt );
            sscanf( colZoom, "%i", &zoom );
            
            // Let's try to use the simplest possible algo and just look for the zoom level with largest extent (Which probably be the one with lowest resolution?)...
            minLat = wxMin(minLat, tiley2lat(minRow, zoom));
            maxLat = wxMax(maxLat, tiley2lat(maxRow - 1, zoom));
            minLon = wxMin(minLon, tilex2long(minCol, zoom));
            maxLon = wxMax(maxLon, tilex2long(maxCol + 1, zoom));
            std::cout << "Zoom: " << zoom << " minlat: " << tiley2lat(minRow, zoom) << " maxlat: " << tiley2lat(maxRow - 1, zoom) << " minlon: " << tilex2long(minCol, zoom) << " maxlon: " << tilex2long(maxCol + 1, zoom) << std::endl;
        }
        // ... and use what we found only in case we miss some of the values from metadata...
        if(wxIsNaN(m_LatMin))
            m_LatMin = minLat;
        if(wxIsNaN(m_LatMax))
            m_LatMax = maxLat;
        if(wxIsNaN(m_LonMin))
            m_LonMin = minLon;
        if(wxIsNaN(m_LonMax))
            m_LonMax = maxLon;
    }
    catch (std::exception& e)
    {
        const char *t = e.what();
        std::cout << "exception: " << e.what() << std::endl;
    }
}

InitReturn ChartMBTiles::Init( const wxString& name, ChartInitFlag init_flags )
{
      m_global_color_scheme = GLOBAL_COLOR_SCHEME_RGB;

      m_FullPath = name;
      m_Description = m_FullPath;
      
      try
      {
            // Open the MBTiles database file
        SQLite::Database  db(name.mb_str());
        
        // Compile a SQL query, getting everything from the "metadata" table
        SQLite::Statement   query(db, "SELECT * FROM metadata ");
        
        // Loop to execute the query step by step, to get rows of result
        while (query.executeStep())
        {
            const char* colName = query.getColumn(0);
            const char* colValue = query.getColumn(1);
            
            //Get the geometric extent of the data
            if(!strncmp(colName, "bounds", 6)){
                float lon1, lat1, lon2, lat2;
                sscanf( colValue, "%g,%g,%g,%g", &lon1, &lat1, &lon2, &lat2 );
                
                // There is some confusion over the layout of this field...
                m_LatMax = wxMax(lat1, lat2);
                m_LatMin = wxMin(lat1, lat2);
                m_LonMax = wxMax(lon1, lon2);
                m_LonMin = wxMin(lon1, lon2);
                
            }
            
            // Not very interesting as it may be wrong, we better find out from first loaded tile and adjust m_imageType accordingly
            //else if(!strncmp(colName, "format", 6) ){
            //    m_bPNG = !strncmp(colValue, "png", 3);
            //}
            
            //Get the min and max zoom values present in the db
            else if(!strncmp(colName, "minzoom", 7)){
                sscanf( colValue, "%i", &m_minZoom );
            }
            else if(!strncmp(colName, "maxzoom", 7)){
                sscanf( colValue, "%i", &m_maxZoom );
            }
            
            else if(!strncmp(colName, "description", 11)){
                m_Description = wxString(colValue,  wxConvUTF8);
            }
            
            
                
        }
      }
      catch (std::exception& e)
      {
          const char *t = e.what();
          std::cout << "exception: " << e.what() << std::endl;
          return INIT_FAIL_REMOVE;
      }     
    
      // Fix the missing/wrong metadata values
      InitFromTiles(name);
 
      // Bound the max zoom reasonably
      m_maxZoom = wxMin(m_maxZoom, 16);
 
      // set the chart scale parameters based on the minzoom factor
      m_ppm_avg = 1.0 / OSM_zoomMPP[m_minZoom];
      m_Chart_Scale = OSM_zoomScale[m_minZoom];
      

      // Create the coverage area
      //  We do this by building a LLRegion containing the minZoom level tiles that actually exist
      
      
      PrepareTiles();           // Initialize the tile data structures
      
      LLRegion covrRegion;
      
      LLBBox extentBox;
      extentBox.Set(m_LatMin, m_LonMin, m_LatMax, m_LonMax);
      
      int zoomFactor = m_minZoom;
      mbTileZoomDescriptor *tzd = m_tileArray[zoomFactor - m_minZoom];
      
      int numtiles = tzd->nx_tile * tzd->ny_tile;
      
      mbTileDescriptor **tiles = tzd->m_tileDesc;
      for(int i = 0; i<numtiles; i++) {
          mbTileDescriptor *tile = tiles[i];
          if(!extentBox.IntersectOut(tile->box)) {
              
              // Does this tile contain data?
              // If so, add to the region.
              if(tileIsPopulated(tile)){
                LLBBox box = tile->box;
                
                // Grow the tile lat/lon extents by nominally 1 meter to avoid LLRegion precision difficulties
                double factor = 1.0 / (1852. * 60.);
                box.EnLarge(factor);
                LLRegion tileRegion(box);
                covrRegion.Union(tileRegion);
              }
          } 
      }

      //  The coverage region must be reduced if necessary to include only the db specified bounds.
      covrRegion.Intersect(extentBox);

      m_minZoomRegion = covrRegion;
      
      //  Populate M_COVR entries for the OCPN chart database
      if(covrRegion.contours.size()){   // Check for no intersection caused by ??
        m_nCOVREntries = covrRegion.contours.size();
        m_pCOVRTablePoints = (int *)malloc(m_nCOVREntries * sizeof(int));
        m_pCOVRTable = (float **)malloc(m_nCOVREntries * sizeof(float *));
        std::list<poly_contour>::iterator it = covrRegion.contours.begin();
        for(int i=0; i<m_nCOVREntries; i++) {
            m_pCOVRTablePoints[i] = it->size();
            m_pCOVRTable[i] = (float *)malloc(m_pCOVRTablePoints[i] * 2 * sizeof(float));
            std::list<contour_pt>::iterator jt = it->begin();
            for(int j=0; j<m_pCOVRTablePoints[i]; j++) {
                 m_pCOVRTable[i][2*j+0] = jt->y;
                 m_pCOVRTable[i][2*j+1] = jt->x;
                 jt++;
            }
            it++;
         }
      }
      
      
      if(init_flags == HEADER_ONLY)
          return INIT_OK;
      
      InitReturn pi_ret = PostInit();
      if( pi_ret  != INIT_OK)
          return pi_ret;
      else
          return INIT_OK;
      
}

InitReturn ChartMBTiles::PreInit( const wxString& name, ChartInitFlag init_flags, ColorScheme cs )
{
      m_global_color_scheme = cs;
      return INIT_OK;
}


InitReturn ChartMBTiles::PostInit(void)
{
    
      bReadyToRender = true;
      return INIT_OK;
}

bool ChartMBTiles::tileIsPopulated(mbTileDescriptor *tile)
{
    try
    {
        // Open the MBTiles database file
        SQLite::Database  db(m_FullPath.mb_str());
        
        char qrs[100];
        sprintf(qrs, "select * from tiles where zoom_level = %d AND tile_column=%d AND tile_row=%d", tile->m_zoomLevel, tile->tile_x, tile->tile_y);
        
        // Compile a SQL query, getting the specific  blob
        SQLite::Statement query(db, qrs);
        
        int queryResult = query.tryExecuteStep();
        if(SQLITE_DONE == queryResult){
            return false;                           // requested ROW not found
        }
        else{
            return true;
        }
    }
    catch (std::exception& e)
    {
        const char *t = e.what();
        std::cout << "exception: " << e.what() << std::endl;
    }     

    return false;
}


void ChartMBTiles::PrepareTiles()
{
    //OCPNStopWatch sw;
    m_tileArray = new mbTileZoomDescriptor* [(m_maxZoom - m_minZoom) + 1];
    
    for(int i=0 ; i < (m_maxZoom - m_minZoom) + 1 ; i++){
        PrepareTilesForZoom(m_minZoom + i, (i==0));        // Preset the geometry only on the minZoom tiles
    }
    //printf("PrepareTiles time: %f\n", sw.GetTime());
    
}

void ChartMBTiles::FlushTiles( void )
{
    if(!bReadyToRender)
        return;
    
    for(int iz=0 ; iz < (m_maxZoom - m_minZoom) + 1 ; iz++){
        mbTileZoomDescriptor *tzd = m_tileArray[iz];
        
        for( int i = 0; i < tzd->ny_tile; i++ ) {
            for( int j = 0; j < tzd->nx_tile; j++ ) {
                mbTileDescriptor *tile = tzd->m_tileDesc[i*tzd->nx_tile + j];
                glDeleteTextures(1, &tile->glTextureName);
                delete tile;
            }
        }
        delete tzd;
    }
}

void ChartMBTiles::PrepareTilesForZoom(int zoomFactor, bool bset_geom)
{
    mbTileZoomDescriptor *tzd = new mbTileZoomDescriptor;
    
    m_tileArray[zoomFactor - m_minZoom] = tzd;
    
    // Calculate the tile counts in x and y, based on zoomfactor and chart extents
    tzd->tile_x_min = long2tilex(m_LonMin, zoomFactor);
    tzd->tile_x_max = long2tilex(m_LonMax, zoomFactor);
    tzd->tile_y_min = lat2tiley(m_LatMin, zoomFactor);
    tzd->tile_y_max = lat2tiley(m_LatMax, zoomFactor);
    
    tzd->nx_tile = tzd->tile_x_max - tzd->tile_x_min + 1;
    tzd->ny_tile = tzd->tile_y_max - tzd->tile_y_min + 1;
 
    // Build the required tileDescriptor pointers
    tzd->m_tileDesc = new mbTileDescriptor*[tzd->nx_tile * tzd->ny_tile];
    
  
    int tex_dim = 256;
    
    //    Using a 2D loop, iterate thru the tiles at this zoom level
    int tile_y = tzd->tile_y_min;
    
    for( int i = 0; i < tzd->ny_tile; i++ ) {
        int tile_x= tzd->tile_x_min;
        
        for( int j = 0; j < tzd->nx_tile; j++ ) {
           
            mbTileDescriptor *tile = tzd->m_tileDesc[i*tzd->nx_tile + j] = new mbTileDescriptor;
            tile->tile_x = tile_x;
            tile->tile_y = tile_y;
            tile->m_zoomLevel = zoomFactor;
            
            //  If directed, defer expensize geometry computation until actually needed for drawing.
            if(bset_geom){
                const double eps = 6e-6;  // about 1cm on earth's surface at equator
                
                tile->lonmin = round(tilex2long(tile_x, zoomFactor)/eps)*eps;
                tile->lonmax = round(tilex2long(tile_x + 1, zoomFactor)/eps)*eps;
                tile->latmin = round(tiley2lat(tile_y - 1, zoomFactor)/eps)*eps;
                tile->latmax = round(tiley2lat(tile_y, zoomFactor)/eps)*eps;

                tile->box.Set(tile->latmin, tile->lonmin, tile->latmax, tile->lonmax);
                tile->m_bgeomSet = true;
            }
            tile_x++;
        }
        tile_y++;
    }
}



bool ChartMBTiles::GetChartExtent(Extent *pext)
{
      pext->NLAT = m_LatMax;
      pext->SLAT = m_LatMin;
      pext->ELON = m_LonMax;
      pext->WLON = m_LonMin;

      return true;
}



void ChartMBTiles::SetColorScheme(ColorScheme cs, bool bApplyImmediate)
{
}





void ChartMBTiles::GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion)
{
    
    pValidRegion->Clear();
    pValidRegion->Union(0, 0, VPoint.pix_width, VPoint.pix_height);
    return;
}

LLRegion ChartMBTiles::GetValidRegion()
{
    return m_minZoomRegion;
}






bool ChartMBTiles::RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint)
{
    return true;
#if 0    
      SetVPRasterParms(VPoint);

      OCPNRegion rgn(0,0,VPoint.pix_width, VPoint.pix_height);

      bool bsame_region = (rgn == m_last_region);          // only want to do this once


      if(!bsame_region)
            cached_image_ok = false;

      m_last_region = rgn;

      return RenderRegionViewOnDC(dc, VPoint, rgn);
#endif
}

bool ChartMBTiles::getTileTexture(SQLite::Database &db, mbTileDescriptor *tile)
{
    // Is the texture ready?
    if(tile->glTextureName > 0){
        glBindTexture( GL_TEXTURE_2D, tile->glTextureName );
        
        return true;
    }
    else{
        if(tile->m_bNotAvailable)
            return false;
        // fetch the tile data from the mbtile database
        try
        {
          
            char qrs[100];
            sprintf(qrs, "select * from tiles where zoom_level = %d AND tile_column=%d AND tile_row=%d", tile->m_zoomLevel, tile->tile_x, tile->tile_y);
            
            // Compile a SQL query, getting the specific  blob
            SQLite::Statement query(db, qrs);
            
            int queryResult = query.tryExecuteStep();
            if(SQLITE_DONE == queryResult){
                tile->m_bNotAvailable = true;
                return false;                           // requested ROW not found
            }
            else{
                SQLite::Column blobColumn = query.getColumn(3);         // Get the blob
                const void* blob = blobColumn.getBlob();
                
                sprintf(qrs, "select length(tile_data) from tiles where zoom_level = %d AND tile_column=%d AND tile_row=%d", tile->m_zoomLevel, tile->tile_x, tile->tile_y);
                SQLite::Statement lquery(db, qrs);
                int queryResult = lquery.tryExecuteStep();
                int length = lquery.getColumn(0);         // Get the length
                
                
                wxMemoryInputStream blobStream(blob, length);
                wxImage blobImage;

                blobImage = wxImage(blobStream, m_imageType);
                
                int blobWidth = blobImage.GetWidth();
                int blobHeight = blobImage.GetHeight();
                
                int stride = 4;
                int tex_w = 256;
                int tex_h = 256;
                unsigned char *imgdata = blobImage.GetData();
                if( !imgdata )
                    return false;
                m_imageType = blobImage.GetType();
                unsigned char *teximage = (unsigned char *) malloc( stride * tex_w * tex_h );
                
                for( int j = 0; j < tex_w*tex_h; j++ ){
                    for( int k = 0; k < 3; k++ )
                        teximage[j * stride + k] = imgdata[3*j + k];
                    teximage[j * stride + 3] = 255;
                }
                
                    
                glGenTextures( 1, &tile->glTextureName );
                glBindTexture( GL_TEXTURE_2D, tile->glTextureName );
                
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
                
                glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, teximage );
                
                free(teximage);
                
                return true;
            }
            
        }
        catch (std::exception& e)
        {
            const char *t = e.what();
            std::cout << "exception: " << e.what() << std::endl;
        }     
    }
        
        
    
    return false;
}

bool ChartMBTiles::RenderRegionViewOnGL(const wxGLContext &glc, const ViewPort& VPoint, const OCPNRegion &RectRegion, const LLRegion &Region)
{
    // Do not render if significantly underzoomed
    if( VPoint.chart_scale > (10 * m_Chart_Scale) )
        return true;
    
    ViewPort vp = VPoint;
    
    // Open the MBTiles database file
    SQLite::Database  db(m_FullPath.mb_str());
    
    /* setup opengl parameters */
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    int viewZoom = m_minZoom;
    double zoomMod = 2.0;              // decrease to get more detail, nominal 4?, 2 works OK for NOAA.
    
    for(int kz=m_minZoom ; kz < 19 ; kz++){
        double db_mpp = OSM_zoomMPP[kz];
        double vp_mpp = 1. / VPoint.view_scale_ppm;
        
        if(db_mpp < vp_mpp * zoomMod){
            viewZoom = kz;
            break;
        }
    }
    
    viewZoom = wxMin(viewZoom, m_maxZoom);
    int zoomFactor = m_minZoom;
    
    // DEBUG TODO   Show single zoom
    //zoomFactor = m_minZoom;
    //viewZoom = zoomFactor;
    
    float coords[8];
    float texcoords[] = { 0., 1., 0., 0., 1., 0., 1., 1. };
    
    while(zoomFactor <= viewZoom){
        //printf("zoomFactor: %d\n", zoomFactor);
        mbTileZoomDescriptor *tzd = m_tileArray[zoomFactor - m_minZoom];

        LLBBox box = Region.GetBox();
        
        // Get the tile numbers of the box corners of this render region, at this zoom level
        int topTile =   wxMin(tzd->tile_y_max, lat2tiley(box.GetMaxLat(), zoomFactor));
        int botTile =   wxMax(tzd->tile_y_min, lat2tiley(box.GetMinLat(), zoomFactor));
        int leftTile =  wxMax(tzd->tile_x_min, long2tilex(box.GetMinLon(), zoomFactor));
        int rightTile = wxMin(tzd->tile_x_max, long2tilex(box.GetMaxLon(), zoomFactor));
        

        mbTileDescriptor **tiles = tzd->m_tileDesc;

        for(int i=botTile ; i < topTile+1 ; i++){
            for(int j = leftTile ; j < rightTile+1 ; j++){
                int index = (i - tzd->tile_y_min) * tzd->nx_tile;
                index += (j - tzd->tile_x_min);
                
                mbTileDescriptor *tile = tiles[index];
                if(!tile->m_bgeomSet){
                    const double eps = 6e-6;  // about 1cm on earth's surface at equator
                    
                    tile->lonmin = round(tilex2long(tile->tile_x, zoomFactor)/eps)*eps;
                    tile->lonmax = round(tilex2long(tile->tile_x + 1, zoomFactor)/eps)*eps;
                    tile->latmin = round(tiley2lat(tile->tile_y - 1, zoomFactor)/eps)*eps;
                    tile->latmax = round(tiley2lat(tile->tile_y, zoomFactor)/eps)*eps;
                    
                    tile->box.Set(tile->latmin, tile->lonmin, tile->latmax, tile->lonmax);
                    tile->m_bgeomSet = true;
                }
                
                if(!Region.IntersectOut(tile->box)) {
                    
                    bool btexture = getTileTexture(db, tile);
                    if(!btexture) { // failed to load, draw red
                        glDisable(GL_TEXTURE_2D);
                        glColor3f(1, 0, 0);
                        continue;
                    }
                    else{
                        glEnable(GL_TEXTURE_2D);
                        glColor4f(1, 1, 1, 1);
                    }
                
                    wxPoint2DDouble p;
                    p = vp.GetDoublePixFromLL(tile->latmin, tile->lonmin); coords[0] = p.m_x;  coords[1] = p.m_y;
                    p = vp.GetDoublePixFromLL(tile->latmax, tile->lonmin); coords[2] = p.m_x;  coords[3] = p.m_y;
                    p = vp.GetDoublePixFromLL(tile->latmax, tile->lonmax); coords[4] = p.m_x;  coords[5] = p.m_y;
                    p = vp.GetDoublePixFromLL(tile->latmin, tile->lonmax); coords[6] = p.m_x;  coords[7] = p.m_y;
                
                
                    glTexCoordPointer(2, GL_FLOAT, 2*sizeof(GLfloat), texcoords);
                    glVertexPointer(2, GL_FLOAT, 2*sizeof(GLfloat), coords);
                    glDrawArrays(GL_QUADS, 0, 4);
                
                
                }
                
            }
        }
        
                
        zoomFactor++;
        //printf("\n");
    }
    
    glDisable(GL_TEXTURE_2D);
    
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    
    return true;
}


bool ChartMBTiles::RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region)
{
    return true;
#if 0    
      SetVPRasterParms(VPoint);

      wxRect dest(0,0,VPoint.pix_width, VPoint.pix_height);
//      double factor = ((double)Rsrc.width)/((double)dest.width);
      double factor = GetRasterScaleFactor(VPoint);
      if(m_b_cdebug)
      {
            printf("%d RenderRegion  ScaleType:  %d   factor:  %g\n", s_dc++, RENDER_HIDEF, factor );
            printf("Rect list:\n");
            OCPNRegionIterator upd ( Region ); // get the requested rect list
            while ( upd.HaveRects() )
            {
                  wxRect rect = upd.GetRect();
                  printf("   %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height);
                  upd.NextRect() ;
            }
      }

            //    Invalidate the cache if the scale has changed or the viewport size has changed....
      if((fabs(m_cached_scale_ppm - VPoint.view_scale_ppm) > 1e-9) || (m_last_vprect != dest))
      {
            cached_image_ok = false;
            m_vp_render_last.Invalidate();
      }
/*
      if(pPixCache)
      {
            if((pPixCache->GetWidth() != dest.width) || (pPixCache->GetHeight() != dest.height))
            {
                  delete pPixCache;
                  pPixCache = new PixelCache(dest.width, dest.height, BPP);
            }
      }
      else
            pPixCache = new PixelCache(dest.width, dest.height, BPP);
*/

      m_cached_scale_ppm = VPoint.view_scale_ppm;
      m_last_vprect = dest;


      if(cached_image_ok)
      {
            //    Anything to do?
           bool bsame_region = (Region == m_last_region);          // only want to do this once

           if((bsame_region) && (Rsrc == cache_rect)  )
           {
              pPixCache->SelectIntoDC(dc);
              if(m_b_cdebug)printf("  Using Current PixelCache\n");
              return false;
           }
      }

     m_last_region = Region;


     //     Analyze the region requested
     //     When rendering complex regions, (more than say 4 rectangles)
     //     .OR. small proportions, then rectangle rendering may be faster
     //     Also true  if the scale is less than near unity, or overzoom.
     //     This will be the case for backgrounds of the quilt.


     /*  Update for Version 2.4.0
     This logic seems flawed, at least for quilts which contain charts having non-rectangular coverage areas.
     These quilt regions decompose to ...LOTS... of rectangles, most of which are 1 pixel in height.
     This is very slow, due to the overhead of GetAndScaleData().
     However, remember that overzoom never uses the cache, nor does non-binary scale factors..
     So, we check to see if this is a cacheable render, and that the number of rectangles is "reasonable"
     */

     //     Get the region rectangle count

     int n_rect =0;
     OCPNRegionIterator upd ( Region ); // get the requested rect list
     while ( upd.HaveRects() )
     {
           n_rect++;
           upd.NextRect();
     }

     if((!IsRenderCacheable( Rsrc, dest ) && ( n_rect > 4 ) && (n_rect < 20)) || ( factor < 1))
     {
         if(m_b_cdebug)printf("   RenderRegion by rect iterator   n_rect: %d\n", n_rect);
                              
           // Verify that the persistent pixel cache is at least as large as the largest rectangle in the region
           wxRect dest_check_rect = dest;
           OCPNRegionIterator upd_check ( Region ); // get the requested rect list
           while ( upd_check.HaveRects() )
           {
               wxRect rect = upd_check.GetRect();
               dest_check_rect.Union(rect);
               upd_check.NextRect();
           }
 
            if(pPixCache)
            {
                if((pPixCache->GetWidth() != dest_check_rect.width) || (pPixCache->GetHeight() != dest_check_rect.height))
                {
                    delete pPixCache;
                    pPixCache = new PixelCache(dest_check_rect.width, dest_check_rect.height, BPP);
                }
            }
            else
                pPixCache = new PixelCache(dest_check_rect.width, dest_check_rect.height, BPP);

           
           ScaleTypeEnum ren_type = RENDER_LODEF;



      //    Decompose the region into rectangles, and fetch them into the target dc
           OCPNRegionIterator upd ( Region ); // get the requested rect list
           int ir = 0;
           while ( upd.HaveRects() )
           {
                 wxRect rect = upd.GetRect();
                 
                 //  Floating point math can lead to negative rectangle origin.
                 //  If this happens, we arbitrarily shift the rectangle to be positive semidefinite.
                 //  This will cause at most a 1 pixlel error onscreen.
                 if(rect.y < 0) rect.Offset(0, -rect.y);
                 if(rect.x < 0) rect.Offset(-rect.x, 0);
           
                 
                 GetAndScaleData(pPixCache->GetpData(), pPixCache->GetLength(),
                        Rsrc, Rsrc.width, rect, pPixCache->GetWidth(), factor, ren_type);
                 
                 ir++;
                 upd.NextRect();;
           }

           pPixCache->Update();

      //    Update cache parameters
           cache_rect = Rsrc;
           cache_scale_method = ren_type;
           cached_image_ok = false;            // Never cache this type of render

      //    Select the data into the dc
           pPixCache->SelectIntoDC(dc);

           return true;
     }


     //     Default is to try using the cache
     
     if(pPixCache)
     {
         if((pPixCache->GetWidth() != dest.width) || (pPixCache->GetHeight() != dest.height))
         {
             delete pPixCache;
             pPixCache = new PixelCache(dest.width, dest.height, BPP);
         }
     }
     else
         pPixCache = new PixelCache(dest.width, dest.height, BPP);
     
     

     if(m_b_cdebug)printf("  Render Region By GVUC\n");

     //     A performance enhancement.....
     ScaleTypeEnum scale_type_zoom = RENDER_HIDEF;
     double binary_scale_factor = VPoint.view_scale_ppm / GetPPM();
     if(binary_scale_factor < .20)
           scale_type_zoom = RENDER_LODEF;

     bool bnewview = GetViewUsingCache(Rsrc, dest, Region, scale_type_zoom);

     //    Select the data into the dc
     pPixCache->SelectIntoDC(dc);

     return bnewview;
#endif
}









