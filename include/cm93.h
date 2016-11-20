/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  CM93 Chart Object
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

#ifndef __CM93CHART_H__
#define __CM93CHART_H__

#include <wx/listctrl.h>			// Somehow missing from wx build

#include    "s57chart.h"
#include    "cutil.h"               // for types

//    Some constants
#define     INDEX_m_sor       217                // cm93 dictionary index for object type _m_sor

#define CM93_ZOOM_FACTOR_MAX_RANGE 5

//    Static functions
int Get_CM93_CellIndex(double lat, double lon, int scale);
void Get_CM93_Cell_Origin(int cellindex, int scale, double *lat, double *lon);

//    Fwd definitions
class covr_set;
class wxSpinCtrl;

class M_COVR_Desc
{
   public:
      M_COVR_Desc();
      ~M_COVR_Desc();

      int      GetWKBSize();
      bool     WriteWKB(void *p);
      int      ReadWKB(wxFFileInputStream &ifs);
      void     Update(M_COVR_Desc *pmcd);
      OCPNRegion GetRegion(const ViewPort &vp, wxPoint *pwp);


      int         m_cell_index;
      int         m_object_id;
      int         m_subcell;

      int         m_nvertices;
      float_2Dpt  *pvertices;

      int   m_ngl_vertices;
      float_2Dpt *gl_screen_vertices;
      int gl_screen_projection_type;

      int         m_npub_year;
      double      transform_WGS84_offset_x;
      double      transform_WGS84_offset_y;
      double      m_covr_lat_min;
      double      m_covr_lat_max;
      double      m_covr_lon_min;
      double      m_covr_lon_max;
      double      user_xoff;
      double      user_yoff;
      double      m_centerlat_cos;
      
      LLBBox m_covr_bbox;
      bool        m_buser_offsets;

};

WX_DECLARE_OBJARRAY(M_COVR_Desc, Array_Of_M_COVR_Desc);
WX_DEFINE_ARRAY_PTR(M_COVR_Desc *, Array_Of_M_COVR_Desc_Ptr);

WX_DECLARE_LIST(M_COVR_Desc, List_Of_M_COVR_Desc);

//    Georeferencing constants

      //    This constant was developed empirically by looking at a
      //    representative cell, comparing the cm93 point transform coefficients
      //    to the stated lat/lon bounding box.
      //    This value corresponds to the semi-major axis for the "International 1924" geo-standard
      //    For WGS84, it should be 6378137.0......

static const double CM93_semimajor_axis_meters        = 6378388.0;            // CM93 semimajor axis

//    CM93 Data structures

class Extended_Geometry;

//#pragma pack(push,1)

typedef struct{
      unsigned short    x;
      unsigned short    y;
}cm93_point;


typedef struct{
      unsigned short    x;
      unsigned short    y;
      unsigned short    z;
}cm93_point_3d;

//#pragma pack(pop)


typedef struct{
      double                  lon_min;
      double                  lat_min;
      double                  lon_max;
      double                  lat_max;
                                          // Bounding Box, in Mercator transformed co-ordinates
      double                  easting_min;
      double                  northing_min;
      double                  easting_max;
      double                  northing_max;

      unsigned short          usn_vector_records;        //  number of spacial(vector) records
      int                     n_vector_record_points;    //  number of cm93 points in vector record block
      int                     m_46;
      int                     m_4a;
      unsigned short          usn_point3d_records;
      int                     m_50;
      int                     m_54;
      unsigned short          usn_point2d_records;             //m_58;
      unsigned short          m_5a;
      unsigned short          m_5c;
      unsigned short          usn_feature_records;            //m_5e, number of feature records

      int                     m_60;
      int                     m_64;
      unsigned short          m_68;
      unsigned short          m_6a;
      unsigned short          m_6c;
      int                     m_nrelated_object_pointers;

      int                     m_72;
      unsigned short          m_76;

      int                     m_78;
      int                     m_7c;

}header_struct;

typedef struct{
      unsigned short          n_points;
      unsigned short          x_min;
      unsigned short          y_min;
      unsigned short          x_max;
      unsigned short          y_max;
      int                     index;
      cm93_point              *p_points;
}geometry_descriptor;

typedef struct{
      geometry_descriptor     *pGeom_Description;
      unsigned char           segment_usage;
}vector_record_descriptor;



typedef struct{
      unsigned char     otype;
      unsigned char     geotype;
      unsigned short    n_geom_elements;
      void              *pGeometry;       // may be a (cm93_point*) or other geom;
      unsigned char     n_related_objects;
      void              *p_related_object_pointer_array;
      unsigned char     n_attributes;     // number of attributes
      unsigned char     *attributes_block;      // encoded attributes

}Object;


typedef struct{

      //          Georeferencing transform coefficients
      double                        transform_x_rate;
      double                        transform_y_rate;
      double                        transform_x_origin;
      double                        transform_y_origin;

      cm93_point                    *p2dpoint_array;
      Object                        **pprelated_object_block;
      unsigned char                 *attribute_block_top;               // attributes block
      geometry_descriptor           *edge_vector_descriptor_block;      // edge vector descriptor block
      geometry_descriptor           *point3d_descriptor_block;
      cm93_point                    *pvector_record_block_top;
      cm93_point_3d                 *p3dpoint_array;

      int                           m_nvector_records;
      int                           m_nfeature_records;
      int                           m_n_point3d_records;
      int                           m_n_point2d_records;

      List_Of_M_COVR_Desc          m_cell_mcovr_list;
      bool                         b_have_offsets;
      bool                         b_have_user_offsets;

      double                       user_xoff;
      double                       user_yoff;

      //    Allocated working blocks
      vector_record_descriptor      *object_vector_record_descriptor_block;
      Object                        *pobject_block;

}Cell_Info_Block;




//----------------------------------------------------------------------------
// cm93_dictionary class
//    Encapsulating the conversion between binary cm_93 object class, attributes, etc
//    to standard S57 text conventions
//----------------------------------------------------------------------------

class cm93_dictionary
{
      public:

            cm93_dictionary();
            ~cm93_dictionary();

            bool LoadDictionary(const wxString & dictionary_dir);
            bool IsOk(void){ return m_ok; }
            wxString GetDictDir(void){ return m_dict_dir; }

            wxString GetClassName(int iclass);
            wxString GetAttrName(int iattr);
            char GetAttrType(int iattr);

      private:
            int               m_max_class;
            int               m_max_attr;
            wxArrayString     *m_S57ClassArray;
            wxArrayString     *m_AttrArray;
            int               *m_GeomTypeArray;
            char              *m_ValTypeArray;
            bool              m_ok;
            wxString          m_dict_dir;

};


class cm93_attr_block
{
      public:
            cm93_attr_block(void * block, cm93_dictionary *pdict);
            unsigned char *GetNextAttr();

            int m_cptr;
            unsigned char *m_block;

            cm93_dictionary *m_pDict;
};


//----------------------------------------------------------------------------
// cm93 Chart Manager class
//----------------------------------------------------------------------------
class cm93manager
{
public:
    cm93manager();
    ~cm93manager();
    bool Loadcm93Dictionary(const wxString & name);
    cm93_dictionary *FindAndLoadDict(const wxString &file);


    cm93_dictionary   *m_pcm93Dict;

    //  Member variables used to record the calling of cm93chart::CreateHeaderDataFromCM93Cell()
    //  for each available scale value.  This allows that routine to return quickly with no error
    //  for all cells other than the first, at each scale....

    bool    m_bfoundA;
    bool    m_bfoundB;
    bool    m_bfoundC;
    bool    m_bfoundD;
    bool    m_bfoundE;
    bool    m_bfoundF;
    bool    m_bfoundG;
    bool    m_bfoundZ;


};

//----------------------------------------------------------------------------
// cm93 Chart object class
//----------------------------------------------------------------------------
class cm93chart : public s57chart
{
      public:
            cm93chart();
            ~cm93chart();

            cm93chart(int scale_index);
            InitReturn Init( const wxString& name, ChartInitFlag flags );

            void ResetSubcellKey(){ m_loadcell_key = '0'; }

            double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);
            double GetNormalScaleMax(double canvas_scale_factor, int canvas_width);

            bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);
            void SetVPParms(const ViewPort &vpt);
            void GetPointPix(ObjRazRules *rzRules, float northing, float easting, wxPoint *r);
            void GetPointPix(ObjRazRules *rzRules, wxPoint2DDouble *en, wxPoint *r, int nPoints);
            void GetPixPoint(int pixx, int pixy, double *plat, double *plon, ViewPort *vpt);

            void SetCM93Dict(cm93_dictionary *pDict){m_pDict = pDict;}
            void SetCM93Prefix(const wxString &prefix){m_prefix = prefix;}
            void SetCM93Manager(cm93manager *pManager){m_pManager = pManager;}

            bool UpdateCovrSet(ViewPort *vpt);
            bool IsPointInLoadedM_COVR(double xc, double yc);
            covr_set *GetCoverSet(){ return m_pcovr_set; }
            LLRegion GetValidRegion();

            const wxString & GetLastFileName(void) const { return m_LastFileName; }

            ArrayOfInts GetVPCellArray(const ViewPort &vpt);

            Array_Of_M_COVR_Desc_Ptr    m_pcovr_array_loaded;

            void SetUserOffsets(int cell_index, int object_id, int subcell, int xoff, int yoff);
            wxString GetScaleChar(){ return m_scalechar; }

            wxPoint *GetDrawBuffer(int nSize);

            OCPNRegion          m_render_region;

            LLBBox      m_covr_bbox; /* bounding box for entire covr_set */

      private:
            InitReturn CreateHeaderDataFromCM93Cell(void);
            int read_header_and_populate_cib(header_struct *ph, Cell_Info_Block *pCIB);
            Extended_Geometry *BuildGeom(Object *pobject, wxFileOutputStream *postream, int iobject);

            S57Obj *CreateS57Obj( int cell_index, int iobject, int subcell, Object *pobject, cm93_dictionary *pDict,
                                  Extended_Geometry *xgeom, double ref_lat, double ref_lon, double scale,
                                  double view_scale_ppm);

            void ProcessMCOVRObjects(int cell_index, char subcell);

            void translate_colmar(const wxString &sclass, S57attVal *pattValTmp);

            int CreateObjChain(int cell_index, int subcell, double view_scale_ppm);

            void Unload_CM93_Cell(void);

            //    cm93 point manipulation methods
            void Transform(cm93_point *s, double trans_x, double trans_y, double *lat, double *lon);

            int loadcell_in_sequence(int, char);
            int loadsubcell(int, wxChar);
            void ProcessVectorEdges(void);

            wxPoint2DDouble FindM_COVROffset(double lat, double lon);
            M_COVR_Desc *FindM_COVR_InWorkingSet(double lat, double lon);


            Cell_Info_Block   m_CIB;

            cm93_dictionary   *m_pDict;
            cm93manager       *m_pManager;

            wxString          m_prefix;

            double            m_sfactor;

            wxString          m_scalechar;
            ArrayOfInts       m_cells_loaded_array;

            int               m_current_cell_vearray_offset;
            int               *m_pcontour_array;
            int               m_ncontour_alloc;
            ViewPort          m_vp_current;
            wxChar            m_loadcell_key;
            double            m_dval;

            covr_set          *m_pcovr_set;

            wxPoint     *m_pDrawBuffer;               // shared outline drawing buffer
            int         m_nDrawBufferSize;

            wxString          m_LastFileName;

            LLRegion            m_region;
};

//----------------------------------------------------------------------------
// cm93 Composite Chart object class
//----------------------------------------------------------------------------
class CM93OffsetDialog;

class cm93compchart : public s57chart
{
      public:
            cm93compchart();
            ~cm93compchart();


            InitReturn Init( const wxString& name, ChartInitFlag flags );

            void Activate(void);
            void Deactivate(void);

            double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);
            double GetNormalScaleMax(double canvas_scale_factor, int canvas_width);
            int GetNativeScale(void);

            wxString GetPubDate();

            void SetVPParms(const ViewPort &vpt);
            void GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion);
            LLRegion GetValidRegion();
            
            ThumbData *GetThumbData(int tnx, int tny, float lat, float lon);
            ThumbData *GetThumbData() {return (ThumbData *)NULL;}

            bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);

            bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region);

            virtual bool RenderRegionViewOnGL(const wxGLContext &glc, const ViewPort& VPoint,
                                              const OCPNRegion &RectRegion, const LLRegion &Region);
            void SetColorScheme(ColorScheme cs, bool bApplyImmediate);

            bool RenderNextSmallerCellOutlines( ocpnDC &dc, ViewPort& vp);

            void GetPointPix(ObjRazRules *rzRules, float rlat, float rlon, wxPoint *r);
            void GetPixPoint(int pixx, int pixy, double *plat, double *plon, ViewPort *vpt);
            void GetPointPix(ObjRazRules *rzRules, wxPoint2DDouble *en, wxPoint *r, int nPoints);


            ListOfObjRazRules *GetObjRuleListAtLatLon(float lat, float lon, float select_radius,
                                                      ViewPort *VPoint, int selection_mask = MASK_ALL);
            S57ObjectDesc *CreateObjDescription(const ObjRazRules *obj);

            VE_Hash&  Get_ve_hash(void);
            VC_Hash&  Get_vc_hash(void);

            void UpdateLUPs(s57chart *pOwner);
            void ForceEdgePriorityEvaluate(void);
            ListOfS57Obj *GetAssociatedObjects(S57Obj *obj);
            cm93chart *GetCurrentSingleScaleChart(){ return m_pcm93chart_current; }

            void SetSpecialOutlineCellIndex(int cell_index, int object_id, int subcell)
                  { m_cell_index_special_outline = cell_index;
                    m_object_id_special_outline = object_id;
                    m_subcell_special_outline = subcell; }

            void SetSpecialCellIndexOffset(int cell_index, int object_id, int subcell, int xoff, int yoff);
            void CloseandReopenCurrentSubchart(void);

            void InvalidateCache();
      private:
            void UpdateRenderRegions ( const ViewPort& VPoint );
            OCPNRegion GetValidScreenCanvasRegion(const ViewPort& VPoint, const OCPNRegion &ScreenRegion);

            bool RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint);

            InitReturn CreateHeaderData();
            cm93_dictionary *FindAndLoadDictFromDir(const wxString &dir);
            void FillScaleArray(double lat, double lon);
            int PrepareChartScale(const ViewPort &vpt, int cmscale, bool bOZ_protect = true);
            int GetCMScaleFromVP(const ViewPort &vpt);
            bool DoRenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region);

            bool DoRenderRegionViewOnGL (const wxGLContext &glc, const ViewPort& VPoint,
                                         const OCPNRegion &RectRegion, const LLRegion &Region );

            bool RenderCellOutlinesOnDC( ocpnDC &dc, ViewPort& vp, wxPoint *pwp, M_COVR_Desc *mcd );
            void RenderCellOutlinesOnGL( ViewPort& vp, M_COVR_Desc *mcd );

            //    Data members

            cm93_dictionary   *m_pDictComposite;
            cm93manager       *m_pcm93mgr;


            cm93chart         *m_pcm93chart_array[8];
            bool              m_bScale_Array[8];
            cm93chart         *m_pcm93chart_current;
            int               m_cmscale;

            wxString          m_prefixComposite;

            int               m_current_cell_pub_date;      // the (integer) publish date of the cell at the current VP

            wxBitmap          *m_pDummyBM;
            int               m_cell_index_special_outline;
            int               m_object_id_special_outline;
            int               m_subcell_special_outline;
            int               m_special_offset_x;
            int               m_special_offset_y;
            ViewPort          m_vpt;


            cm93chart *m_last_cell_adjustvp;
};


class OCPNOffsetListCtrl;
//----------------------------------------------------------------------------------------------------------
//    CM93OffsetDialog Specification
//----------------------------------------------------------------------------------------------------------
class CM93OffsetDialog: public wxDialog
{
      DECLARE_CLASS( CM93OffsetDialog )
      DECLARE_EVENT_TABLE()

      public:
            CM93OffsetDialog( wxWindow *parent );
            ~CM93OffsetDialog( );

            void OnClose(wxCloseEvent& event);
            void OnOK(wxCommandEvent& event);

            void SetCM93Chart( cm93compchart *pchart );
            void SetColorScheme( );
            void UpdateMCOVRList( const ViewPort &vpt );     // Rebuild MCOVR list

            OCPNOffsetListCtrl            *m_pListCtrlMCOVRs;
            Array_Of_M_COVR_Desc_Ptr      m_pcovr_array;

            wxString                      m_selected_chart_scale_char;

      private:
            void OnCellSelected( wxListEvent &event );
            void OnOffSetSet( wxCommandEvent& event );

            void UpdateOffsets(void);

            wxSpinCtrl        *m_pSpinCtrlXoff;
            wxSpinCtrl        *m_pSpinCtrlYoff;
            wxButton          *m_OKButton;

            wxWindow          *m_pparent;
            cm93compchart     *m_pcompchart;

            int               m_xoff;
            int               m_yoff;
            int               m_selected_cell_index;
            int               m_selected_object_id;
            int               m_selected_subcell;
            int               m_selected_list_index;
            double            m_centerlat_cos;

};



#endif
