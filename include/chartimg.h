/******************************************************************************
 * $Id: chartimg.h,v 1.24 2010/05/19 01:05:19 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  ChartBaseBSB and Friends
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 * $Log: chartimg.h,v $
 * Revision 1.24  2010/05/19 01:05:19  bdbcat
 * Build 518
 *
 * Revision 1.23  2010/05/15 03:55:04  bdbcat
 * Build 514
 *
 * Revision 1.22  2010/04/27 01:44:36  bdbcat
 * Build 426
 *
 * Revision 1.21  2010/03/29 02:59:02  bdbcat
 * 2.1.0 Beta Initial
 *
 *
 */


#ifndef _CHARTIMG_H_
#define _CHARTIMG_H_


#include "chartbase.h"
#include "georef.h"                 // for GeoRef type

typedef enum ScaleTypeEnum
{
      RENDER_LODEF = 0,
      RENDER_HIDEF,
}_ScaleTypeEnum;



class WXDLLEXPORT ChartImg;

//-----------------------------------------------------------------------------
//    Constants, etc.
//-----------------------------------------------------------------------------

typedef enum PaletteDir
{
      PaletteFwd,
      PaletteRev
}_PaletteDir;


typedef enum BSB_Color_Capability
{
    COLOR_RGB_DEFAULT = 0,                   // Default corresponds to bsb entries "RGB"
    DAY,
    DUSK,
    NIGHT,
    NIGHTRED,
    GRAY,
    PRC,
    PRG,
    N_BSB_COLORS
}_BSB_Color_Capability;

//-----------------------------------------------------------------------------
//    Fwd Refs
//-----------------------------------------------------------------------------

class ChartKAP;
class ViewPort;
class PixelCache;
class ocpnBitmap;

class wxFileInputStream;

//-----------------------------------------------------------------------------
//    Helper classes
//-----------------------------------------------------------------------------

class Refpoint
{
public:
      int         bXValid;
      int         bYValid;
      float       xr;
      float       yr;
      float       latr;
      float       lonr;
      float       xpl_error;
      float       xlp_error;
      float       ypl_error;
      float       ylp_error;

};





class CachedLine
{
public:
      int               xstart;
      int               xlength;
      unsigned char     *pPix;
      bool              bValid;
};

class opncpnPalette
{
    public:
        opncpnPalette();
        ~opncpnPalette();

        int *FwdPalette;
        int *RevPalette;
        int nFwd;
        int nRev;
};

// ----------------------------------------------------------------------------
// ChartBaseBSB
// ----------------------------------------------------------------------------

class  ChartBaseBSB     :public ChartBase
{
    public:
      //    Public methods

      ChartBaseBSB();
      virtual ~ChartBaseBSB();

      //    Accessors
      virtual ThumbData *GetThumbData(int tnx, int tny, float lat, float lon);
      virtual ThumbData *GetThumbData() {return pThumbData;}
      virtual bool UpdateThumbData(float lat, float lon);

      int GetNativeScale(){return m_Chart_Scale;}
      double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);
      double GetNormalScaleMax(double canvas_scale_factor, int canvas_width);

      double GetChartSkew(){return Chart_Skew;}

      virtual InitReturn Init( const wxString& name, ChartInitFlag init_flags );

      virtual int latlong_to_pix_vp(double lat, double lon, int &pixx, int &pixy, ViewPort& vp);
      virtual int vp_pix_to_latlong(ViewPort& vp, int pixx, int pixy, double *lat, double *lon);

      bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const wxRegion &Region);

      virtual void SetVPParms(const ViewPort &vpt);

      virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);
      virtual double GetNearestPreferredScalePPM(double target_scale_ppm);

      virtual bool IsRenderDelta(ViewPort &vp_last, ViewPort &vp_proposed);

      void GetValidCanvasRegion(const ViewPort& VPoint, wxRegion  *pValidRegion);

      virtual bool GetChartExtent(Extent *pext);

      void SetColorScheme(ColorScheme cs, bool bApplyImmediate);

      virtual int Continue_BackgroundHiDefRender(void);

      void ComputeSourceRectangle(const ViewPort &vp, wxRect *pSourceRect);
      wxRect GetSourceRect(){ return Rsrc; }

      wxImage *GetImage();

protected:
//    Methods

      bool RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint);

      bool IsCacheValid(){ return cached_image_ok; }
      void InvalidateCache(){cached_image_ok = 0;}

      void CreatePaletteEntry(char *buffer, int palette_index);
      PaletteDir GetPaletteDir(void);
      int  *GetPalettePtr(BSB_Color_Capability);

      bool Initialize_BackgroundHiDefRender(const ViewPort &VPoint);
      bool Finish_BackgroundHiDefRender(void);

      double GetClosestValidNaturalScalePPM(double target_scale, double scale_factor_min, double scale_factor_max);

      double GetPPM(){ return m_ppm_avg;}
      int GetSize_X(){ return Size_X;}
      int GetSize_Y(){ return Size_Y;}

      virtual void InvalidateLineCache();
      virtual bool CreateLineIndex(void);


      virtual wxBitmap *CreateThumbnail(int tnx, int tny, ColorScheme cs);
      virtual bool GetChartBits( wxRect& source, unsigned char *pPix, int sub_samp );
      virtual int BSBGetScanline( unsigned char *pLineBuf, int y, int xs, int xl, int sub_samp);

      virtual bool GetAndScaleData(unsigned char **ppn,
                                   wxRect& source, int source_stride, wxRect& dest, int dest_stride,
                                   double scale_factor, ScaleTypeEnum scale_type);


      bool GetViewUsingCache( wxRect& source, wxRect& dest, ScaleTypeEnum scale_type );
      bool GetView( wxRect& source, wxRect& dest, ScaleTypeEnum scale_type );


      virtual int BSBScanScanline(wxInputStream *pinStream);
      virtual int ReadBSBHdrLine( wxFileInputStream*, char *, int );
      virtual int AnalyzeRefpoints(void);
      virtual bool SetMinMax(void);

      InitReturn PreInit( const wxString& name, ChartInitFlag init_flags, ColorScheme cs );
      InitReturn PostInit(void);



//    Protected Data
      PixelCache        *pPixCache;

      int         Size_X;                 // Chart native pixel dimensions
      int         Size_Y;
      int         m_Chart_DU;
      double      m_cph;
      double      Chart_Skew;
      double      m_proj_parameter;                     // Mercator:               Projection Latitude
                                                      // Transverse Mercator:    Central Meridian
      double      m_dx;                                 // Pixel scale factors, from KAP header
      double      m_dy;

      int         m_datum_index;
      double      m_dtm_lat;
      double      m_dtm_lon;


      wxRect      cache_rect;
      wxRect      cache_rect_scaled;
      bool        cached_image_ok;
      ScaleTypeEnum cache_scale_method;
      double      m_cached_scale_ppm;
      wxRect      m_last_vprect;


      wxRect      Rsrc;                   // Current chart source rectangle


      int         nRefpoint;
      Refpoint    *pRefTable;


      int         nColorSize;
      int         *pline_table;           // pointer to Line offset table

      CachedLine  *pLineCache;

      wxFileInputStream     *ifs_hdr;
      wxFileInputStream     *ifss_bitmap;
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

      opncpnPalette     *pPalettes[N_BSB_COLORS];

      BSB_Color_Capability m_mapped_color_index;

//    Integer digital scale value above which bilinear scaling is not allowed,
//      and subsampled scaling must be performed
      int         m_bilinear_limit;


      bool        bUseLineCache;

      float       m_LonMax;
      float       m_LonMin;
      float       m_LatMax;
      float       m_LatMin;

      int         *pPalette;
      PaletteDir  palette_direction;

      bool        bGeoErrorSent;

      double      m_ppm_avg;              // Calculated true scale factor of the 1X chart,
                                        // pixels per meter

      double      m_raster_scale_factor;        // exact scaling factor for pixel oversampling calcs

      //    Storage for background render machine

      PixelCache    *pPixCacheBackground;
      unsigned char *background_work_buffer;
      unsigned char *br_target_data;

      int       br_target_height;
      int       br_target_width;
      int       br_target_y;
      double    br_factor;
      wxRect    br_Rsrc;
      double    m_br_scale;
      bool      m_br_bpending;

      bool      m_bIDLcross;

      wxRegion  m_last_region;

      int       m_b_cdebug;

      double    m_proj_lat, m_proj_lon;
};


// ----------------------------------------------------------------------------
// ChartKAP
// ----------------------------------------------------------------------------

class  ChartKAP : public ChartBaseBSB
{
public:
//    Methods
      ChartKAP();
      ~ChartKAP();

      InitReturn Init( const wxString& name, ChartInitFlag init_flags );


};

// ----------------------------------------------------------------------------
// ChartGEO
// ----------------------------------------------------------------------------

class  ChartGEO : public ChartBaseBSB
{
public:
//    Methods
      ChartGEO();
      ~ChartGEO();

      InitReturn Init( const wxString& name, ChartInitFlag init_flags );


};

#endif
