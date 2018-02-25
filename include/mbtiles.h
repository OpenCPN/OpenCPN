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
 
      //    Accessors
      virtual ThumbData *GetThumbData(int tnx, int tny, float lat, float lon);
      virtual ThumbData *GetThumbData();
      virtual bool UpdateThumbData(double lat, double lon);
      
      virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);
      
      int GetNativeScale(){return m_Chart_Scale;}
      double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);
      double GetNormalScaleMax(double canvas_scale_factor, int canvas_width);

      virtual InitReturn Init( const wxString& name, ChartInitFlag init_flags );


      bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region);

      virtual bool RenderRegionViewOnGL(const wxGLContext &glc, const ViewPort& VPoint,
                                        const OCPNRegion &RectRegion, const LLRegion &Region);

      virtual double GetNearestPreferredScalePPM(double target_scale_ppm);

      virtual void GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion);
      virtual LLRegion GetValidRegion();

      virtual bool GetChartExtent(Extent *pext);

      void SetColorScheme(ColorScheme cs, bool bApplyImmediate);

      double GetPPM(){ return m_ppm_avg;}

protected:
//    Methods
      bool RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint);
      InitReturn PreInit( const wxString& name, ChartInitFlag init_flags, ColorScheme cs );
      InitReturn PostInit(void);

      void PrepareTiles();
      void PrepareTilesForZoom(int zoomFactor, bool bset_geom);
      bool getTileTexture(SQLite::Database &db, mbTileDescriptor *tile);
      bool tileIsPopulated(mbTileDescriptor *tile);
      void FlushTiles( void );
      

//    Protected Data


      float       m_LonMax, m_LonMin, m_LatMax, m_LatMin;

      double      m_ppm_avg;              // Calculated true scale factor of the 1X chart,
                                        // pixels per meter

      int       m_b_cdebug;

      int       m_minZoom, m_maxZoom;
      mbTileZoomDescriptor      **m_tileArray;
      LLRegion  m_minZoomRegion;
      wxBitmapType m_imageType;

private:
      void InitFromTiles( const wxString& name );
};



#endif
