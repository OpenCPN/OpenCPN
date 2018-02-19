/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  MBTiles Chart Support
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 *
 */


#ifndef _CHARTMBTILES_H_
#define _CHARTMBTILES_H_


#include "chartbase.h"
#include "georef.h"                 // for GeoRef type
#include "OCPNRegion.h"
#include "viewport.h"
#include <SQLiteCpp/SQLiteCpp.h>
//#include <sqlite3.h>



class WXDLLEXPORT ChartMbTiles;

//-----------------------------------------------------------------------------
//    Constants, etc.
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//    Fwd Refs
//-----------------------------------------------------------------------------

class ViewPort;
class PixelCache;
class ocpnBitmap;
class mbTileZoomDescriptor;
class mbTileDescriptor;


//-----------------------------------------------------------------------------
//    Helper classes
//-----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// ChartMBTiles
// ----------------------------------------------------------------------------

class  ChartMBTiles     :public ChartBase
{
    public:
      //    Public methods

      ChartMBTiles();
      virtual ~ChartMBTiles();
      void FreeLineCacheRows(int start=0, int end=-1);
      bool HaveLineCacheRow(int row);

      //    Accessors
      virtual ThumbData *GetThumbData(int tnx, int tny, float lat, float lon);
      virtual ThumbData *GetThumbData();
      virtual bool UpdateThumbData(double lat, double lon);
      
      virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);
      
      int GetNativeScale(){return m_Chart_Scale;}
      double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);
      double GetNormalScaleMax(double canvas_scale_factor, int canvas_width);

      virtual InitReturn Init( const wxString& name, ChartInitFlag init_flags );

      virtual int latlong_to_pix_vp(double lat, double lon, double &pixx, double &pixy, ViewPort& vp);
      virtual int vp_pix_to_latlong(ViewPort& vp, double pixx, double pixy, double *lat, double *lon);

      bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region);

      virtual bool RenderRegionViewOnGL(const wxGLContext &glc, const ViewPort& VPoint,
                                        const OCPNRegion &RectRegion, const LLRegion &Region);

      //virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);
      virtual double GetNearestPreferredScalePPM(double target_scale_ppm);

      virtual void GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion);
      virtual LLRegion GetValidRegion();

      virtual bool GetChartExtent(Extent *pext);

      void SetColorScheme(ColorScheme cs, bool bApplyImmediate);

      wxImage *GetImage();

      void SetVPRasterParms(const ViewPort &vpt);

      virtual bool GetChartBits( wxRect& source, unsigned char *pPix, int sub_samp );
      virtual int GetSize_X(){ return Size_X;}
      virtual int GetSize_Y(){ return Size_Y;}

      virtual void latlong_to_chartpix(double lat, double lon, double &pixx, double &pixy);
      void chartpix_to_latlong(double pixx, double pixy, double *plat, double *plon);

      double GetPPM(){ return m_ppm_avg;}

protected:
//    Methods


      bool RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint);




      InitReturn PreInit( const wxString& name, ChartInitFlag init_flags, ColorScheme cs );
      InitReturn PostInit(void);

      void PrepareTiles();
      void PrepareTilesForZoom(int zoomFactor);
      bool getTileTexture(SQLite::Database &db, mbTileDescriptor *tile);
      bool tileIsPopulated(mbTileDescriptor *tile);
      

//    Protected Data
      //PixelCache        *pPixCache;

      int         Size_X;                 // Chart native pixel dimensions
      int         Size_Y;
      int         m_Chart_DU;
      double      m_cph;
      double      m_proj_parameter;                     // Mercator:               Projection Latitude
                                                      // Transverse Mercator:    Central Meridian
      double      m_dx;                                 // Pixel scale factors, from KAP header
      double      m_dy;

      wxString    m_bsb_ver;
      bool        m_b_apply_dtm;

      int         m_datum_index;
      double      m_dtm_lat;
      double      m_dtm_lon;


      wxRect      cache_rect;
      wxRect      cache_rect_scaled;
      bool        cached_image_ok;
      double      m_cached_scale_ppm;
      wxRect      m_last_vprect;


      wxRect      Rsrc;                   // Current chart source rectangle
      double      m_raster_scale_factor;

      int         nRefpoint;


      int         nColorSize;
      int         *pline_table;           // pointer to Line offset table


      wxInputStream    *ifs_hdr;
      wxInputStream    *ifss_bitmap;
      wxBufferedInputStream *ifs_bitmap;

      wxString          *pBitmapFilePath;

      unsigned char     *ifs_buf;
      unsigned char     *ifs_bufend;
      int               ifs_bufsize;
      unsigned char     *ifs_lp;
      int               ifs_file_offset;
      int               nFileOffsetDataStart;
      int               m_nLineOffset;

      GeoRef            cPoints;

      double            wpx[12], wpy[12], pwx[12], pwy[12];     // Embedded georef coefficients
      int               wpx_type, wpy_type, pwx_type, pwy_type;
      int               n_wpx, n_wpy, n_pwx, n_pwy;
      bool              bHaveEmbeddedGeoref;



//    Integer digital scale value above which bilinear scaling is not allowed,
//      and subsampled scaling must be performed
      int         m_bilinear_limit;


      bool        bUseLineCache;

      float       m_LonMax;
      float       m_LonMin;
      float       m_LatMax;
      float       m_LatMin;



      double      m_ppm_avg;              // Calculated true scale factor of the 1X chart,
                                        // pixels per meter
      OCPNRegion  m_last_region;

      int       m_b_cdebug;

      double    m_proj_lat, m_proj_lon;

      ViewPort  m_vp_render_last;
      
      wxCriticalSection m_critSect;
      wxULongLong m_filesize;
      
      // MBTiles
      int       m_minZoom, m_maxZoom;
      mbTileZoomDescriptor      **m_tileArray;
      LLRegion  m_minZoomRegion;
      
};



#endif
