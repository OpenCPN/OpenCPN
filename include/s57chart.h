/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S57 Chart Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 **************************************************************************/

#ifndef __S57CHART_H__
#define __S57CHART_H__

#include <wx/wx.h>
#include <wx/progdlg.h>
#include "bbox.h"
#include "chartbase.h"
#include "wx/dir.h"
#include "wx/filename.h"
#include "wx/file.h"
#include "wx/stream.h"
#include "wx/wfstream.h"
#include "mygdal/ogrsf_frmts.h"

#include "iso8211.h"

#include "mygdal/gdal.h"
#include "s57RegistrarMgr.h"
#include "S57ClassRegistrar.h"
#include "S57Light.h"
#include "S57Sector.h"
#include "s52s57.h"                 //types
#include "OCPNRegion.h"
#include "ocpndc.h"
#include "viewport.h"

// ----------------------------------------------------------------------------
// Useful Prototypes
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// S57 Utility Prototypes
// ----------------------------------------------------------------------------
extern "C" bool s57_GetChartExtent(const wxString& FullPath, Extent *pext);

void s57_DrawExtendedLightSectors( ocpnDC& temp_dc, ViewPort& VPoint, std::vector<s57Sector_t>& sectorlegs );
bool s57_CheckExtendedLightSectors( int mx, int my, ViewPort& VPoint, std::vector<s57Sector_t>& sectorlegs );

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------

enum
{
      BUILD_SENC_OK,
      BUILD_SENC_NOK_RETRY,
      BUILD_SENC_NOK_PERMANENT
};

//----------------------------------------------------------------------------
// Fwd Defns
//----------------------------------------------------------------------------

class ChartBase;
class ViewPort;
class ocpnBitmap;
class PixelCache;
class S57ObjectDesc;
class S57Reader;
class OGRS57DataSource;
class S57ClassRegistrar;
class S57Obj;
class VE_Element;
class VC_Element;
class connector_segment;

#include <wx/dynarray.h>

// Declare the Array of S57Obj
WX_DECLARE_OBJARRAY(S57Obj, ArrayOfS57Obj);
WX_DECLARE_OBJARRAY(S57Obj *, ArrayOfS57ObjPtr);

// And also a list
WX_DECLARE_LIST(S57Obj, ListOfS57Obj);


WX_DECLARE_LIST(ObjRazRules, ListOfObjRazRules);



//----------------------------------------------------------------------------
// s57 Chart object class
//----------------------------------------------------------------------------
class s57chart : public ChartBase
{
public:
      s57chart();
      ~s57chart();

      virtual InitReturn Init( const wxString& name, ChartInitFlag flags );

//    Accessors

      virtual ThumbData *GetThumbData(int tnx, int tny, float lat, float lon);
      virtual ThumbData *GetThumbData() {return pThumbData;}
      bool UpdateThumbData(double lat, double lon);

      virtual int GetNativeScale(){return m_Chart_Scale;}
      virtual double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);
      virtual double GetNormalScaleMax(double canvas_scale_factor, int canvas_width);

      void SetNativeScale(int s){m_Chart_Scale = s;}

      virtual bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region);
      virtual bool RenderOverlayRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region);

      virtual void GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion);
      virtual LLRegion GetValidRegion();

      virtual void GetPointPix(ObjRazRules *rzRules, float rlat, float rlon, wxPoint *r);
      virtual void GetPointPix(ObjRazRules *rzRules, wxPoint2DDouble *en, wxPoint *r, int nPoints);
      virtual void GetPixPoint(int pixx, int pixy, double *plat, double *plon, ViewPort *vpt);

      virtual void SetVPParms(const ViewPort &vpt);

      virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);
//      virtual bool IsRenderDelta(ViewPort &vp_last, ViewPort &vp_proposed);

      virtual double GetNearestPreferredScalePPM(double target_scale_ppm){ return target_scale_ppm; }

      void SetFullExtent(Extent& ext);
      bool GetChartExtent(Extent *pext);

      void SetColorScheme(ColorScheme cs, bool bApplyImmediate = true);
      virtual void UpdateLUPs(s57chart *pOwner);

      int _insertRules(S57Obj *obj, LUPrec *LUP, s57chart *pOwner);

      virtual ListOfObjRazRules *GetObjRuleListAtLatLon(float lat, float lon, float select_radius, 
                                                        ViewPort *VPoint, int selection_mask = MASK_ALL);
      bool DoesLatLonSelectObject(float lat, float lon, float select_radius, S57Obj *obj);
      bool IsPointInObjArea(float lat, float lon, float select_radius, S57Obj *obj);
      wxString GetObjectAttributeValueAsString( S57Obj *obj, int iatt, wxString curAttrName );
      static wxString GetAttributeValueAsString( S57attVal *pAttrVal, wxString AttrName );
      static int CompareLights( const void** l1, const void** l2 );
      wxString CreateObjDescriptions( ListOfObjRazRules* rule);
      static wxString GetAttributeDecode(wxString& att, int ival);

      int BuildRAZFromSENCFile(const wxString& SENCPath);
      static void GetChartNameFromTXT(const wxString& FullPath, wxString &Name);
      wxString buildSENCName( const wxString& name);
      
      //    DEPCNT VALDCO array access
      bool GetNearestSafeContour(double safe_cnt, double &next_safe_cnt);

      virtual ListOfS57Obj *GetAssociatedObjects(S57Obj *obj);

      virtual VE_Hash&  Get_ve_hash(void){ return m_ve_hash; }
      virtual VC_Hash&  Get_vc_hash(void){ return m_vc_hash; }

      virtual void ForceEdgePriorityEvaluate(void);

      float *GetLineVertexBuffer( void ){ return m_line_vertex_buffer; }
      
      void ClearRenderedTextCache();
      
      double GetCalculatedSafetyContour(void){ return m_next_safe_cnt; }

//#ifdef ocpnUSE_GL
      virtual bool RenderRegionViewOnGL(const wxGLContext &glc, const ViewPort& VPoint,
                                        const OCPNRegion &RectRegion, const LLRegion &Region);
      virtual bool RenderOverlayRegionViewOnGL(const wxGLContext &glc, const ViewPort& VPoint,
                                               const OCPNRegion &RectRegion, const LLRegion &Region);
//#endif
      
// Public data
//Todo Accessors here
      //  Object arrays used by S52PLIB TOPMAR rendering logic
      wxArrayPtrVoid *pFloatingATONArray;
      wxArrayPtrVoid *pRigidATONArray;

      double        ref_lat, ref_lon;             // Common reference point, derived from FullExtent
      Extent        m_FullExtent;
      bool          m_bExtentSet;
      bool          m_bLinePrioritySet;

      //  SM Projection parms, stored as convenience to expedite pixel conversions
      double    m_easting_vp_center, m_northing_vp_center;
      double    m_pixx_vp_center, m_pixy_vp_center;
      double    m_view_scale_ppm;

      //    Last ViewPort succesfully rendered, stored as an aid to calculating pixel cache address offsets and regions
      ViewPort    m_last_vp;
      OCPNRegion    m_last_Region;

      virtual bool IsCacheValid(){ return (pDIB != NULL); }
      virtual void InvalidateCache();
      virtual bool RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint);

      virtual void ClearDepthContourArray(void);
      virtual void BuildDepthContourArray(void);
      int ValidateAndCountUpdates( const wxFileName file000, const wxString CopyDir,
                                   wxString &LastUpdateDate, bool b_copyfiles);
      static int GetUpdateFileArray(const wxFileName file000, wxArrayString *UpFiles,
                                    wxDateTime date000, wxString edtn000 );
      wxString GetISDT(void);

      char GetUsageChar(void){ return m_usage_char; }
      static bool IsCellOverlayType(char *pFullPath);

      bool        m_b2pointLUPS;
      bool        m_b2lineLUPS;
      
      struct _chart_context     *m_this_chart_context;

      InitReturn FindOrCreateSenc( const wxString& name, bool b_progress = true );
      
protected:
    void AssembleLineGeometry( void );
    
private:
      int GetLineFeaturePointArray(S57Obj *obj, void **ret_array);
      void SetSafetyContour(void);
    
      bool DoRenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, RenderTypeEnum option, bool force_new_view);

      bool DoRenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region, bool b_overlay);

      int DCRenderRect(wxMemoryDC& dcinput, const ViewPort& vp, wxRect *rect);
      bool DCRenderLPB(wxMemoryDC& dcinput, const ViewPort& vp, wxRect* rect);


      InitReturn PostInit( ChartInitFlag flags, ColorScheme cs );
      int BuildSENCFile(const wxString& FullPath000, const wxString& SENCFileName, bool b_progress = true);
      
      void SetLinePriorities(void);

      bool BuildThumbnail(const wxString &bmpname);
      bool CreateHeaderDataFromENC(void);
      bool CreateHeaderDataFromSENC(void);
      bool CreateHeaderDataFromoSENC(void);
      bool GetBaseFileAttr( const wxString& file000 );
      
      void ResetPointBBoxes(const ViewPort &vp_last, const ViewPort &vp_this);

           //    Access to raw ENC DataSet
      bool InitENCMinimal( const wxString& FullPath );
      int GetENCScale();
      OGRFeature *GetChartFirstM_COVR(int &catcov);
      OGRFeature *GetChartNextM_COVR(int &catcov);

      void FreeObjectsAndRules();
      const char *getName(OGRFeature *feature);

      bool DoRenderOnGL(const wxGLContext &glc, const ViewPort& VPoint);
      bool DoRenderRegionViewOnGL(const wxGLContext &glc, const ViewPort& VPoint,
                                  const OCPNRegion &RectRegion, const LLRegion &Region, bool b_overlay);

      void BuildLineVBO( void );
      
 // Private Data
      char        *hdr_buf;
      char        *mybuf_ptr;
      int         hdr_len;
      wxString    m_SENCFileName;
      ObjRazRules *razRules[PRIO_NUM][LUPNAME_NUM];


      wxArrayString *m_tmpup_array;
      PixelCache   *pDIB;

      wxBitmap     *m_pCloneBM;
      wxMask       *m_pMask;

      bool         bGLUWarningSent;

      wxBitmap    *m_pDIBThumbDay;
      wxBitmap    *m_pDIBThumbDim;
      wxBitmap    *m_pDIBThumbOrphan;
      bool        m_bneed_new_thumbnail;

      bool        m_bbase_file_attr_known;
      wxDateTime  m_date000;                    // extracted from DSID:ISDT
      wxString    m_edtn000;                    // extracted from DSID:EDTN
      int         m_nGeoRecords;                // extracted from DSSI:NOGR
      int         m_native_scale;               // extracted from DSPM:CSCL


//  Raw ENC DataSet members
      OGRS57DataSource  *m_pENCDS;

//  DEPCNT VALDCO array members
      int         m_nvaldco;
      int         m_nvaldco_alloc;
      double       *m_pvaldco_array;

      
      float      *m_line_vertex_buffer;
      size_t      m_vbo_byte_length;
      
      bool        m_blastS57TextRender;
      wxString    m_lastColorScheme;
      wxRect      m_last_vprect;
      long        m_plib_state_hash;
      bool        m_btex_mem;
      char        m_usage_char;
      
      double      m_next_safe_cnt;
      double      m_LOD_meters;

      int         m_LineVBO_name;
      
      VE_Hash     m_ve_hash;
      VC_Hash     m_vc_hash;
      std::vector<connector_segment *> m_pcs_vector;
      std::vector<VE_Element *> m_pve_vector;
      

      wxString    m_TempFilePath;
protected:      
      sm_parms    vp_transform;
      
};


#endif
