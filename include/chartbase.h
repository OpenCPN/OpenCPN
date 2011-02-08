/******************************************************************************
 * $Id: chartbase.h,v 1.24 2010/06/25 02:00:36 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  ChartBase Definition
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
 */

#ifndef _CHARTBASE_H_
#define _CHARTBASE_H_

#include "dychart.h"

#include "bbox.h"
#include "ocpn_types.h"



//----------------------------------------------------------------------------
//  Forward Declarations
//----------------------------------------------------------------------------
class ViewPort;

//----------------------------------------------------------------------------
// Constants. etc
//----------------------------------------------------------------------------

//    ChartBase::Init()  init_flags constants
typedef enum ChartInitFlag
{
      FULL_INIT = 0,
      HEADER_ONLY,
      THUMB_ONLY
}_ChartInitFlag;


typedef enum RenderTypeEnum
{
      DC_RENDER_ONLY = 0,
      DC_RENDER_RETURN_DIB,
      DC_RENDER_RETURN_IMAGE
}_RenderTypeEnum;

typedef enum InitReturn
{
      INIT_OK = 0,
      INIT_FAIL_RETRY,        // Init failed, retry suggested
      INIT_FAIL_REMOVE,       // Init failed, suggest remove from further use
      INIT_FAIL_NOERROR       // Init failed, request no explicit error message
}_InitReturn;



class ThumbData
{
public:
    ThumbData();
    virtual ~ThumbData();

      wxBitmap    *pDIBThumb;
      int         ShipX;
      int         ShipY;
      int         Thumb_Size_X;
      int         Thumb_Size_Y;
};



typedef struct _Extent{
  double SLAT;
  double WLON;
  double NLAT;
  double ELON;
}Extent;

//          Depth unit type enum
typedef enum ChartDepthUnitType
{
    DEPTH_UNIT_UNKNOWN,
    DEPTH_UNIT_FEET,
    DEPTH_UNIT_METERS,
    DEPTH_UNIT_FATHOMS
}_ChartDepthUnitType;

//          Projection type enum
typedef enum OcpnProjType
{
      PROJECTION_UNKNOWN,
      PROJECTION_MERCATOR,
      PROJECTION_TRANSVERSE_MERCATOR,
      PROJECTION_POLYCONIC
}_OcpnProjType;



class Plypoint
{
      public:
            float ltp;
            float lnp;
};


// ----------------------------------------------------------------------------
// ChartBase
// ----------------------------------------------------------------------------

class ChartBase
{

public:
      ChartBase();
      virtual ~ChartBase() = 0;

      virtual InitReturn Init( const wxString& name, ChartInitFlag init_flags) = 0;

      virtual void Activate(void) {};
      virtual void Deactivate(void) {};

//    Accessors
      virtual ThumbData *GetThumbData(int tnx, int tny, float lat, float lon) = 0;
      virtual ThumbData *GetThumbData() = 0;
      virtual bool UpdateThumbData(double lat, double lon) = 0;

      virtual double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom) = 0;
      virtual double GetNormalScaleMax(double canvas_scale_factor, int canvas_width) = 0;

      virtual bool GetChartExtent(Extent *pext) = 0;


      virtual OcpnProjType GetChartProjectionType(){ return m_projection;}
      virtual wxDateTime GetEditionDate(void){ return m_EdDate;}

      virtual wxString GetPubDate(){ return m_PubYear;}
      virtual int GetNativeScale(){ return m_Chart_Scale;}
      wxString GetFullPath() const { return m_FullPath;}
      wxString GetName(){ return m_Name;}
      wxString GetDescription() { return m_Description;}
      wxString GetID(){ return m_ID;}
      wxString GetSE(){ return m_SE;}
      wxString GetDepthUnits(){ return m_DepthUnits;}
      wxString GetSoundingsDatum(){ return m_SoundingsDatum;}
      wxString GetDatumString(){ return m_datum_str;}
      wxString GetExtraInfo(){ return m_ExtraInfo; }
      double GetChart_Error_Factor(){ return Chart_Error_Factor; }
      ChartTypeEnum GetChartType(){ return m_ChartType;}
      ChartFamilyEnum GetChartFamily(){ return m_ChartFamily;}
      double GetChartSkew(){ return m_Chart_Skew; }


      virtual ChartDepthUnitType GetDepthUnitType(void) { return m_depth_unit_id;}

      virtual bool IsReadyToRender(){ return bReadyToRender;}

      virtual bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
                                        const wxRegion &Region) = 0;

      virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed) = 0;

      virtual void GetValidCanvasRegion(const ViewPort& VPoint, wxRegion *pValidRegion) = 0;

      virtual void SetColorScheme(ColorScheme cs, bool bApplyImmediate = true ) = 0;

      virtual double GetNearestPreferredScalePPM(double target_scale_ppm) = 0;

      virtual int GetCOVREntries(){ return  m_nCOVREntries; }
      virtual int GetCOVRTablePoints(int iTable) { return m_pCOVRTablePoints[iTable]; }
      virtual int  GetCOVRTablenPoints(int iTable){ return m_pCOVRTablePoints[iTable]; }
      virtual float *GetCOVRTableHead(int iTable){ return m_pCOVRTable[iTable]; }

protected:

      int               m_Chart_Scale;
      ChartTypeEnum     m_ChartType;
      ChartFamilyEnum   m_ChartFamily;

      wxString          m_FullPath;
      wxString          m_Name;
      wxString          m_Description;
      wxString          m_ID;
      wxString          m_SE;
      wxString          m_SoundingsDatum;
      wxString          m_datum_str;
      wxString          m_ExtraInfo;
      wxString          m_PubYear;
      wxString          m_DepthUnits;

      OcpnProjType      m_projection;
      ChartDepthUnitType m_depth_unit_id;

      wxDateTime        m_EdDate;

      wxBitmap          *pcached_bitmap;

      ThumbData         *pThumbData;

      ColorScheme       m_global_color_scheme;
      bool              bReadyToRender;

      double            Chart_Error_Factor;

      double            m_lon_datum_adjust;             // Add these numbers to WGS84 position to obtain internal chart position
      double            m_lat_datum_adjust;

      double            m_Chart_Skew;


      //    Chart region coverage information
      //    Charts may have multiple valid regions within the lat/lon box described by the chart extent
      //    The following table structure contains this embedded information

      //    Typically, BSB charts will contain only one entry, corresponding to the PLY information in the chart header
      //    ENC charts often contain multiple entries

      int         m_nCOVREntries;                       // number of coverage table entries
      int         *m_pCOVRTablePoints;                  // int table of number of points in each coverage table entry
      float       **m_pCOVRTable;                       // table of pointers to list of floats describing valid COVR

      //    Todo  Define invalid COVR regions

};


// ----------------------------------------------------------------------------
// ChartDummy
// ----------------------------------------------------------------------------

class ChartDummy : public ChartBase
{

public:
      ChartDummy();
      virtual ~ChartDummy();

      virtual InitReturn Init( const wxString& name, ChartInitFlag init_flags );

//    Accessors
      virtual ThumbData *GetThumbData(int tnx, int tny, float lat, float lon);
      virtual ThumbData *GetThumbData() {return pThumbData;}
      virtual bool UpdateThumbData(double lat, double lon);

      double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom){return 1.0;}
      double GetNormalScaleMax(double canvas_scale_factor, int canvas_width){ return 2.0e7;}

      virtual bool GetChartExtent(Extent *pext);

      virtual bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
                                        const wxRegion &Region);

      virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);

      virtual void GetValidCanvasRegion(const ViewPort& VPoint, wxRegion *pValidRegion);

      virtual void SetColorScheme(ColorScheme cs, bool bApplyImmediate);

      virtual double GetNearestPreferredScalePPM(double target_scale_ppm){ return target_scale_ppm; }

private:
      bool RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint);

      wxBitmap    *m_pBM;
};


// ----------------------------------------------------------------------------
// ChartPlugInWrapper
//    This class is a wrapper/interface to PlugIn charts(PlugInChartBase) as defined in ocpn_plugin.h
// ----------------------------------------------------------------------------

class PlugInChartBase;                  // found in ocpn_plugin.h

class ChartPlugInWrapper : public ChartBase
{
      public:
            ChartPlugInWrapper();
            ChartPlugInWrapper(wxString &chart_class);
            virtual ~ChartPlugInWrapper();

            virtual wxString GetFileSearchMask(void);

            virtual InitReturn Init( const wxString& name, ChartInitFlag init_flags );

//    Accessors
            virtual ThumbData *GetThumbData(int tnx, int tny, float lat, float lon);
            virtual ThumbData *GetThumbData();
            virtual bool UpdateThumbData(double lat, double lon);

            double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);
            double GetNormalScaleMax(double canvas_scale_factor, int canvas_width);

            virtual bool GetChartExtent(Extent *pext);

            virtual bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
                                              const wxRegion &Region);

            virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);

            virtual void GetValidCanvasRegion(const ViewPort& VPoint, wxRegion *pValidRegion);

            virtual void SetColorScheme(ColorScheme cs, bool bApplyImmediate);

            virtual double GetNearestPreferredScalePPM(double target_scale_ppm);

            virtual int GetCOVREntries();
            virtual int GetCOVRTablePoints(int iTable);
            virtual int GetCOVRTablenPoints(int iTable);
            virtual float *GetCOVRTableHead(int iTable);


      private:
            PlugInChartBase *m_ppicb;
            wxObject          *m_ppo;
};



#endif
