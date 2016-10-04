/******************************************************************************
 *
 * Project:  OpenCP
 * Purpose:  S52 PLIB and S57 Chart data types
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


#ifndef _S52S57_H_
#define _S52S57_H_

#include "bbox.h"
#include "ocpn_types.h"

#include <vector>

#define CURRENT_SENC_FORMAT_VERSION  200

//    Fwd Defns
class wxArrayOfS57attVal;
class OGREnvelope;
class OGRGeometry;

// name of the addressed look up table set (fifth letter)
typedef enum _LUPname{
    SIMPLIFIED                             = 'L', // points
    PAPER_CHART                            = 'R', // points
    LINES                                  = 'S', // lines
    PLAIN_BOUNDARIES                       = 'N', // areas
    SYMBOLIZED_BOUNDARIES                  = 'O', // areas
    LUPNAME_NUM                            = 5
}LUPname;




// Addressed Object Type
typedef enum _Object_t{
   POINT_T  = 'P',
   LINES_T  = 'L',
   AREAS_T  = 'A',
   OBJ_NUM  = 3         // number of object type
}Object_t;


// Display Priority
typedef enum _DisPrio{
   PRIO_NODATA          = '0',                  // no data fill area pattern
   PRIO_GROUP1          = '1',                  // S57 group 1 filled areas
   PRIO_AREA_1          = '2',                  // superimposed areas
   PRIO_AREA_2          = '3',                  // superimposed areas also water features
   PRIO_SYMB_POINT      = '4',                  // point symbol also land features
   PRIO_SYMB_LINE       = '5',                  // line symbol also restricted areas
   PRIO_SYMB_AREA       = '6',                  // area symbol also traffic areas
   PRIO_ROUTEING        = '7',                  // routeing lines
   PRIO_HAZARDS         = '8',                  // hazards
   PRIO_MARINERS        = '9',                  // VRM, EBL, own ship
   PRIO_NUM             = 10                    // number of priority levels

}DisPrio;

// RADAR Priority
typedef enum _RadPrio{
   RAD_OVER = 'O',            // presentation on top of RADAR
   RAD_SUPP = 'S',            // presentation suppressed by RADAR
   RAD_NUM  = 2
}RadPrio;

//    PixelPtr type flags
enum{
      ID_EMPTY          = 0,
      ID_wxBitmap,
      ID_RGBA,
      ID_GL_PATT_SPEC,
      ID_RGB_PATT_SPEC,
      ID_GLIST

};

// display category type
typedef enum _DisCat{
   DISPLAYBASE          = 'D',            //
   STANDARD             = 'S',            //
   OTHER                = 'O',            // O for OTHER
   MARINERS_STANDARD    = 'M',            // Mariner specified
   MARINERS_OTHER,                        // value not defined
   DISP_CAT_NUM,                          // value not defined
}DisCat;


#define MASK_POINT      1
#define MASK_LINE       2
#define MASK_AREA       4
#define MASK_MPS        8
#define MASK_ALL        MASK_POINT + MASK_LINE + MASK_AREA + MASK_MPS


    
typedef enum _Rules_t{
   RUL_NONE,                        // no rule type (init)
   RUL_TXT_TX,                      // TX
   RUL_TXT_TE,                      // TE
   RUL_SYM_PT,                      // SY
   RUL_SIM_LN,                      // LS
   RUL_COM_LN,                      // LC
   RUL_ARE_CO,                      // AC
   RUL_ARE_PA,                      // AP
   RUL_CND_SY,                      // CS
   RUL_MUL_SG,                      // Multipoint Sounding
   RUL_ARC_2C                       // Circular Arc, used for sector lights, opencpn private
}Rules_t;

//-- SYMBOLISATION MODULE STRUCTURE -----------------------------
// position parameter:  LINE,       PATTERN, SYMBOL
typedef struct _position{
   union          {int              dummy1,     PAMI,       dummy2;     } minDist;
   union          {int              dummy1,     PAMA,       dummy2;     } maxDist;
   union          {int              LICL,       PACL,       SYCL;       } pivot_x;
   union          {int              LIRW,       PARW,       SYRW;       } pivot_y;
   union          {int              LIHL,       PAHL,       SYHL;       } bnbox_w;
   union          {int              LIVL,       PAVL,       SYVL;       } bnbox_h;
   union          {int              LBXC,       PBXC,       SBXC;       } bnbox_x; // UpLft crnr
   union          {int              LBXR,       PBXR,       SBXR;       } bnbox_y; // UpLft crnr
}position;

// rule parameter for : LINE,       PATTERN,    SYMBOL
typedef struct _Rule{
   int            RCID;
   union          {char             LINM[8],    PANM[8], SYNM[8];       } name;
   union          {char             dummy,      PADF,       SYDF;       } definition;
   union          {char             dummy1,     PATP,       dummy2;     } fillType;
   union          {char             dummy1,     PASP,       dummy2;     } spacing;
   union          {position   line,       patt,       symb;       } pos;
   union          {wxString   *LXPO,      *PXPO,      *SXPO;      } exposition;
   union          {wxString   *dummy,     *PBTM,      *SBTM;      } bitmap;
   union          {char       *LCRF,      *PCRF,      *SCRF;      } colRef;
   union          {char       *LVCT,      *PVCT,      *SVCT;      } vector;

   // Private data
   int     parm0;                      // always indicates form of pixelPtr
   int     parm1;                      // integer parameters
   int     parm2;
   int     parm3;
   int     parm4;
   int     parm5;
   int     parm6;
   int     parm7;
   void    *pixelPtr;                  // opaque pointer

   }Rule;

typedef struct _Rules{
   Rules_t  ruleType;
   char    *INSTstr;          // Symbology Instruction string
   Rule    *razRule;          // rule
   char    *INST0;            // Head of the entire object Instruction string
   int     n_sequence;        // sequence number in list, used to identify a particular rule
   bool    b_private_razRule; // marker indicating that razRule should be free'd on Rules destroy
   struct _Rules *next;
}Rules;


// LOOKUP MODULE CLASS

class LUPrec{
public:
   int            RCID;             // record identifier
   char           OBCL[7];          // Name (6 char) '\0' terminated
   Object_t       FTYP;             // 'A' Area, 'L' Line, 'P' Point
   DisPrio        DPRI;             // Display Priority
   RadPrio        RPRI;             // 'O' or 'S', Radar Priority
   LUPname        TNAM;             // FTYP:  areas, points, lines
   wxArrayString *ATTCArray;        // ArrayString of LUP Attributes
   wxString       *INST;            // Instruction Field (rules)
   DisCat         DISC;             // Display Categorie: D/S/O, DisplayBase, Standard, Other
   int            LUCM;             // Look-Up Comment (PLib3.x put 'groupes' here,
                                    // hence 'int', but its a string in the specs)
   int            nSequence;        // A sequence number, indicating order of encounter in
                                    //  the PLIB file
   Rules          *ruleList;        // rasterization rule list
};

// Conditional Symbology
typedef struct _Cond{
   const char *name;
   void *(*condInst)(void *param);
}Cond;




class S52_TextC
{
public:
      S52_TextC(){ pcol = NULL, pFont = NULL, m_pRGBA = NULL, bnat = false, bspecial_char = false; }
      ~S52_TextC(){ free(m_pRGBA); }

    wxString   frmtd;       // formated text string
    char       hjust;
    char       vjust;
    char       space;
    char       style;       // CHARS
    char       weight;      // CHARS
    char       width;       // CHARS
    int        bsize;       // CHARS -body size
    int        xoffs;       // text offsets, in units of bsize
    int        yoffs;       //
    S52color   *pcol;       // pointer to S52colour
    int        dis;         // display
    wxFont     *pFont;
    int        rul_seq_creator;  // sequence number of the Rule creating this object
    unsigned char *m_pRGBA;
    int           RGBA_width;
    int           RGBA_height;
    int           rendered_char_height;
    wxRect      rText;          // rectangle of the text as currently rendered, used for declutter
    bool        bnat;           // frmtd is National text, UTF-8 encoded
    bool        bspecial_char;  // frmtd has special ASCII characters, i.e. > 127
};




//
// WARNING: must be in sync OGRatt_t

typedef enum _OGRatt_t{
   OGR_INT,
   OGR_INT_LST,
   OGR_REAL,
   OGR_REAL_LST,
   OGR_STR,
}OGRatt_t;

typedef struct _S57attVal {
    void * value;
    OGRatt_t valType;
} S57attVal;

WX_DEFINE_ARRAY( S57attVal *, wxArrayOfS57attVal );

typedef struct _OBJLElement {
    char OBJLName[6];
    int nViz;
} OBJLElement;

// OGR primitive type
typedef enum _geoPrim_t{
   GEO_POINT,
   GEO_LINE,
   GEO_AREA,
   GEO_META,
   GEO_PRIM,            // number of primitive
}GeoPrim_t;


typedef struct _pt{
   double x;
   double y;
}pt;


//      Fwd References
class s57chart;
class S57Obj;
class OGRFeature;
class PolyTessGeo;
class PolyTessGeoTrap;
class line_segment_element;
class PI_line_segment_element;

typedef struct _chart_context{
    void                    *m_pvc_hash;
    void                    *m_pve_hash;
    double                  ref_lat;
    double                  ref_lon;
    wxArrayPtrVoid          *pFloatingATONArray;
    wxArrayPtrVoid          *pRigidATONArray;
    s57chart                *chart;
    double                  safety_contour; 
    float                   *vertex_buffer;
    
}chart_context;



class LineGeometryDescriptor{
public:
    double          extent_s_lat;
    double          extent_n_lat;
    double          extent_w_lon;
    double          extent_e_lon;
    int             indexCount;
    int *           indexTable;
};


typedef struct _MultipointGeometryDescriptor{
    double          extent_s_lat;
    double          extent_n_lat;
    double          extent_w_lon;
    double          extent_e_lon;
    int             pointCount;
    void *          pointTable;
}MultipointGeometryDescriptor;


class S57Obj
{
public:

      //  Public Methods
      S57Obj();
      ~S57Obj();

      S57Obj( const char* featureName );
      
      wxString GetAttrValueAsString ( const char *attr );
      int GetAttributeIndex( const char *AttrSeek );
      
      bool AddIntegerAttribute( const char *acronym, int val );
      bool AddIntegerListAttribute( const char *acronym, int *pval, int nValue );
      bool AddDoubleAttribute( const char *acronym, double val );
      bool AddDoubleListAttribute( const char *acronym, double *pval, int nValue );
      bool AddStringAttribute( const char *acronym, char *val );

      bool SetPointGeometry( double lat, double lon, double ref_lat, double ref_lon);
      bool SetLineGeometry( LineGeometryDescriptor *pGeo, GeoPrim_t geoType, double ref_lat, double ref_lon);
      bool SetAreaGeometry( PolyTessGeo *ppg, double ref_lat, double ref_lon);
      bool SetMultipointGeometry( MultipointGeometryDescriptor *pGeo, double ref_lat, double ref_lon);
      
      
          
      // Private Methods
private:
      void Init();
    
public:
      // Instance Data
      char                    FeatureName[8];
      GeoPrim_t               Primitive_type;

      char                    *att_array;
      wxArrayOfS57attVal      *attVal;
      int                     n_attr;

      int                     iOBJL;
      int                     Index;

      double                  x;                      // for POINT
      double                  y;
      double                  z;
      int                     npt;                    // number of points as needed by arrays
      
      pt                      *geoPt;                 // used for cm93 line feature select check
      
      double                  *geoPtz;                // an array[3] for MultiPoint, SM with Z, i.e. depth
      double                  *geoPtMulti;            // an array[2] for MultiPoint, lat/lon to make bbox
                                                      // of decomposed points
      PolyTessGeo             *pPolyTessGeo;
      PolyTessGeoTrap         *pPolyTrapGeo;

      LLBBox                  BBObj;                  // lat/lon BBox of the rendered object
      double                  m_lat;                  // The lat/lon of the object's "reference" point
      double                  m_lon;

      Rules                   *CSrules;               // per object conditional symbology
      int                     bCS_Added;

      S52_TextC               *FText;
      int                     bFText_Added;
      wxRect                  rText;

      int                     Scamin;                 // SCAMIN attribute decoded during load
      bool                    bIsClone;
      int                     nRef;                   // Reference counter, to signal OK for deletion
      bool                    bIsAton;                // This object is an aid-to-navigation
      bool                    bIsAssociable;          // This object is DRGARE or DEPARE

      int                     m_n_lsindex;
      int                     *m_lsindex_array;
      int                     m_n_edge_max_points;
      line_segment_element    *m_ls_list;
      PI_line_segment_element *m_ls_list_legacy;
      
      DisCat                  m_DisplayCat;
      int                     m_DPRI;                 // display priority, assigned from initial LUP
                                                      // May be adjusted by CS
      bool                    m_bcategory_mutable;    //  CS procedure may move this object to a higher category.
                                                      //  Used as a hint to rendering filter logic

                                                      // This transform converts from object geometry
                                                      // to SM coordinates.
      double                  x_rate;                 // These auxiliary transform coefficients are
      double                  y_rate;                 // to be used in GetPointPix() and friends
      double                  x_origin;               // on a per-object basis if necessary
      double                  y_origin;
      
      chart_context           *m_chart_context;       // per-chart constants, carried in each object for convenience
      int auxParm0;                                   // some per-object auxiliary parameters, used for OpenGL
      int auxParm1;
      int auxParm2;
      int auxParm3;
      
      bool                    bBBObj_valid;
};

typedef std::vector<S57Obj *> S57ObjVector;

typedef struct _sm_parms{
    double easting_vp_center;
    double northing_vp_center;
}sm_parms;


WX_DEFINE_ARRAY_PTR(Rules*, ArrayOfRules);

typedef struct _mps_container{
    ArrayOfRules *cs_rules;
}mps_container;

// object rasterization rules
typedef struct _ObjRazRules{
   LUPrec          *LUP;
   S57Obj          *obj;
   sm_parms        *sm_transform_parms;
   struct _ObjRazRules *child;            // child list, used only for MultiPoint Soundings
   struct _ObjRazRules *next;
   struct _mps_container *mps;
}ObjRazRules;






//----------------------------------------------------------------------------------
//          Used for s52 Fast Polygon Renderer
//----------------------------------------------------------------------------------
class render_canvas_parms
{
public:
      render_canvas_parms(void);
      ~render_canvas_parms(void);

      unsigned char           *pix_buff;
      int                     lclip;
      int                     rclip;
      int                     pb_pitch;
      int                     x;
      int                     y;
      int                     width;
      int                     height;
      int                     w_pot;
      int                     h_pot;
      int                     depth;
      bool                    b_stagger;
      int                     OGL_tex_name;
      bool                    b_revrgb;
};

//----------------------------------------------------------------------------------
//          Classes used to create arrays of geometry elements
//----------------------------------------------------------------------------------

class VE_Element
{
public:
      unsigned int index;
      unsigned int nCount;
      float      *pPoints;
      int         max_priority;
      size_t      vbo_offset;
      LLBBox      edgeBBox;
      
};

class VC_Element
{
public:
      unsigned int index;
      float      *pPoint;
};

WX_DECLARE_OBJARRAY(VE_Element, ArrayOfVE_Elements);
WX_DECLARE_OBJARRAY(VC_Element, ArrayOfVC_Elements);

typedef std::vector<VE_Element *> VE_ElementVector;
typedef std::vector<VC_Element *> VC_ElementVector;

typedef enum
{
    TYPE_CE = 0,
    TYPE_CC,
    TYPE_EC,
    TYPE_EE,
    TYPE_EE_REV
} SegmentType;

class connector_segment
{
public:
    int vbo_offset;
    int max_priority_cs;
    float               cs_lat_avg;                // segment centroid
    float               cs_lon_avg;
    
};

class line_segment_element
{
public:
    int                 priority;
    union{              connector_segment   *pcs;
    VE_Element          *pedge;
    };
    SegmentType         ls_type;
    
    line_segment_element *next;
};

#if 0 //TODO
class line_segment_element_legacy
{
public:
    size_t              vbo_offset;
    size_t              n_points;
    int                 priority;
    float               lat_max;                // segment bounding box
    float               lat_min;
    float               lon_max;
    float               lon_min;
    int                 type;
    void                *private0;
    
    line_segment_element *next;
};


class connector_segment
{
public:
    void *start;
    void *end;
    SegmentType type;
    int vbo_offset;
    int max_priority;
};

#endif


WX_DECLARE_HASH_MAP( unsigned int, VE_Element *, wxIntegerHash, wxIntegerEqual, VE_Hash );
WX_DECLARE_HASH_MAP( unsigned int, VC_Element *, wxIntegerHash, wxIntegerEqual, VC_Hash );

class connector_key
{
public:
    connector_key() 
    {
      memset(k, 0 , sizeof k);
    }
        
    connector_key(SegmentType t, int a, int b)
    {
      set(t,a,b);
    }   

    void set(SegmentType t, int a, int b) 
    {
      memcpy(k, &a, sizeof a);
      memcpy(&k[sizeof a], &b, sizeof b);
      k[sizeof (a) + sizeof (b)] = (unsigned char)t;      
    }

    unsigned long hash() const;
 
    unsigned char k[sizeof(int) + sizeof(int) + sizeof(char)];
};

class connHash
{
public:
  connHash() { }
  unsigned long operator()( const connector_key& k ) const
  { return k.hash(); }
  
  connHash& operator=(const connHash&) { return *this; }
};

// comparison operator
class connEqual
{
public:
connEqual() { }
bool operator()( const connector_key& a, const connector_key& b ) const
{
  return memcmp(a.k, b.k, sizeof b.k) == 0; 
}

connEqual& operator=(const connEqual&) { return *this; }
};

WX_DECLARE_HASH_MAP( connector_key, connector_segment *, connHash, connEqual, connected_segment_hash );

#endif
