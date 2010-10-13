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
 * $Log: chartbase.h,v $
 * Revision 1.24  2010/06/25 02:00:36  bdbcat
 * 624
 *
 * Revision 1.23  2010/05/29 17:34:21  bdbcat
 * 529a
 *
 * Revision 1.22  2010/05/19 01:04:52  bdbcat
 * Build 518
 *
 * Revision 1.21  2010/05/15 03:55:04  bdbcat
 * Build 514
 *
 * Revision 1.20  2010/04/27 01:44:36  bdbcat
 * Build 426
 *
 * Revision 1.19  2010/03/29 02:59:02  bdbcat
 * 2.1.0 Beta Initial
 *
 */

#ifndef _CHARTBASE_H_
#define _CHARTBASE_H_

#include "dychart.h"

//#include "ocpn_pixel.h"
#include "bbox.h"
//#include "chcanv.h"         // for enums
//#include "chart1.h"
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


enum
{
      BR_CONTINUE,            // Continue background render
      BR_DONE_REFRESH,        // Background render is done, need a repaint
      BR_DONE_NOP             // Background render is done, no repaint needed
};


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
      virtual bool UpdateThumbData(float lat, float lon) = 0;

      virtual int GetNativeScale() = 0;
      virtual double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom) = 0;
      virtual double GetNormalScaleMax(double canvas_scale_factor, int canvas_width) = 0;

      virtual double GetChartSkew() = 0;
      virtual bool GetChartExtent(Extent *pext) = 0;

      virtual OcpnProjType GetChartProjectionType(){ return m_projection;}

      virtual wxString GetPubDate(){ return m_PubYear;}
      virtual wxDateTime GetEditionDate(void){ return m_EdDate;}

      wxString GetFullPath() const { return m_FullPath;}
      wxString GetName(){ return m_Name;}
      wxString GetDescription() { return m_Description;}
      wxString GetID(){ return m_ID;}
      wxString GetSE(){ return m_SE;}
      wxString GetDepthUnits(){ return m_DepthUnits;}
      wxString GetSoundingsDatum(){ return m_SoundingsDatum;}
      wxString GetDatumString(){ return m_datum_str;}
      wxString GetExtraInfo(){ return m_ExtraInfo; }

      ChartTypeEnum GetChartType(){ return m_ChartType;}

      int GetCOVREntries(){ return  m_nCOVREntries; }
      int GetCOVRTablePoints(int iTable) const { return m_pCOVRTablePoints[iTable]; }

      virtual ChartDepthUnitType GetDepthUnitType(void) { return m_depth_unit_id;}

      virtual bool IsReadyToRender(){ return bReadyToRender;}

      virtual void InvalidateCache(void) = 0;

      virtual bool RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, ScaleTypeEnum scale_type) = 0;
      virtual bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
                                        const wxRegion &Region, ScaleTypeEnum scale_type) = 0;

      virtual void SetVPParms(const ViewPort &vpt) = 0;

      virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed) = 0;
      virtual bool IsRenderDelta(ViewPort &vp_last, ViewPort &vp_proposed) = 0;

      virtual void GetValidCanvasRegion(const ViewPort& VPoint, wxRegion *pValidRegion) = 0;

      virtual void SetColorScheme(ColorScheme cs, bool bApplyImmediate = true ) = 0;

      virtual bool IsCacheValid(void) = 0;

      virtual double GetNearestPreferredScalePPM(double target_scale_ppm) = 0;

      virtual int  GetCOVRTablenPoints(int iTable){ return m_pCOVRTablePoints[iTable]; }
      virtual float *GetCOVRTableHead(int iTable){ return m_pCOVRTable[iTable]; }

      //    Background HiDef Render Methods
      virtual int Continue_BackgroundHiDefRender(void);


      ChartTypeEnum     m_ChartType;
      ChartFamilyEnum   m_ChartFamily;

      wxString          m_FullPath;
      wxString          m_Name;
      wxString          m_Description;
      wxString          m_ID;
      wxString          m_SE;
      wxString          m_SoundingsDatum;
      wxString          m_datum_str;

      wxString          m_PubYear;
      wxString          m_DepthUnits;

      wxString          m_ExtraInfo;

      wxBitmap          *pcached_bitmap;

      ThumbData         *pThumbData;

      ColorScheme       m_global_color_scheme;
      bool              bReadyToRender;

      double            Chart_Error_Factor;

      ChartDepthUnitType m_depth_unit_id;



      //    Chart region coverage information
      //    Charts may have multiple valid regions within the lat/lon box described by the chart extent
      //    The following table structure contains this embedded information

      //    Typically, BSB charts will contain only one entry, corresponding to the PLY information in the chart header
      //    ENC charts often contain multiple entries

      int         m_nCOVREntries;                       // number of coverage table entries
      int         *m_pCOVRTablePoints;                  // int table of number of points in each coverage table entry
      float       **m_pCOVRTable;                       // table of pointers to list of floats describing valid COVR

      //    Todo  Define invalid COVR regions

protected:
      int               m_Chart_Scale;
      wxDateTime        m_EdDate;

      double            m_lon_datum_adjust;             // Add these numbers to WGS84 position to obtain internal chart position
      double            m_lat_datum_adjust;

      OcpnProjType      m_projection;


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
      virtual bool UpdateThumbData(float lat, float lon);

      virtual int GetNativeScale();
      double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom){return 1.0;}
      double GetNormalScaleMax(double canvas_scale_factor, int canvas_width){ return 2.0e7;}

      virtual wxString GetPubDate();
      virtual double GetChartSkew(){ return 0.0;}
      virtual bool GetChartExtent(Extent *pext);

      virtual void InvalidateCache(void);

      virtual bool RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, ScaleTypeEnum scale_type);
      virtual bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
                                        const wxRegion &Region, ScaleTypeEnum scale_type);

      virtual void SetVPParms(const ViewPort &vpt);

      virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);

      virtual bool IsRenderDelta(ViewPort &vp_last, ViewPort &vp_proposed);

      virtual void GetValidCanvasRegion(const ViewPort& VPoint, wxRegion *pValidRegion);

      virtual void SetColorScheme(ColorScheme cs, bool bApplyImmediate);

      virtual bool IsCacheValid(){ return false; }

      virtual double GetNearestPreferredScalePPM(double target_scale_ppm){ return target_scale_ppm; }

private:
      wxBitmap    *m_pBM;
};



#endif
