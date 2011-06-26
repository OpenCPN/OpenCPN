/******************************************************************************
 *
 * Project:  OpenCP
 * Purpose:  S52 PLIB and S57 Chart data types
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   bdbcat@yahoo.com   *
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
 * $Log: s52s57.h,v $
 * Revision 1.23  2010/06/24 02:03:28  bdbcat
 * 623
 *
 * Revision 1.22  2010/06/06 20:50:05  bdbcat
 * 606a
 *
 * Revision 1.21  2010/04/27 01:45:21  bdbcat
 * Build 426
 *
 */


#ifndef _S52S57_H_
#define _S52S57_H_

#include "bbox.h"

#define CURRENT_SENC_FORMAT_VERSION  122

//    Fwd Defns
class wxArrayOfS57attVal;
class OGREnvelope;
class OGRGeometry;
class wxBoundingBox;

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



// display category type
typedef enum _DisCat{
   DISPLAYBASE          = 'D',            //
   STANDARD             = 'S',            //
   OTHER                = 'O',            // O for OTHER
   MARINERS_STANDARD    = 'M',            // Mariner specified
   MARINERS_OTHER,                        // value not defined
   DISP_CAT_NUM,                          // value not defined
}DisCat;


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
   int     parm1;                      // integer parameters
   int     parm2;
   int     parm3;
   int     parm4;
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
   wxString       *ATTC;            // Attribute Code/Value (repeat)
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




typedef struct _S52color{
   char colName[20];
   double x;
   double y;
   double L;
   unsigned char  R;
   unsigned char  G;
   unsigned char  B;
}S52color;

typedef struct _S52_Text {
    wxString   *frmtd;       // formated text string
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
} S52_Text;



//-- COLOR MODULE STRUCTURE ---------------------------------------
typedef struct _colTable{
   wxString         *tableName;
   wxArrayPtrVoid   *color;
}colTable;

//
// WARNING: must be in sync OGRatt_t

typedef enum _OGRatt_t{
   OGR_INT,
   OGR_INT_LST,
   OGR_REAL,
   OGR_REAL_LST,
   OGR_STR,
}OGRatt_t;

typedef struct _S57attVal{
   void *   value;
   OGRatt_t valType;
}S57attVal;



typedef struct _OBJLElement{
      char  OBJLName[6];
      int         nViz;
}OBJLElement;



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


class S57Obj
{
public:

      //  Public Methods
      S57Obj();
      ~S57Obj();
      S57Obj(char *first_line, wxInputStream *fpx, double ref_lat, double ref_lon);

      wxString GetAttrValueAsString ( char *attr );


      // Private Methods
private:
      bool IsUsefulAttribute(char *buf);
      int my_fgets( char *buf, int buf_len_max, wxInputStream& ifs );
      int my_bufgetl( char *ib_read, char *ib_end, char *buf, int buf_len_max );

public:
      // Instance Data
      char                    FeatureName[8];
      GeoPrim_t               Primitive_type;

      wxString                *attList;
      wxArrayOfS57attVal      *attVal;

      int                     iOBJL;
      int                     Index;

      double                  x;                      // for POINT
      double                  y;
      double                  z;
      int                     npt;                    // number of points as needed by arrays
      pt                      *geoPt;                 // for LINE & AREA not described by PolyTessGeo
      double                  *geoPtz;                // an array[3] for MultiPoint, SM with Z, i.e. depth
      double                  *geoPtMulti;            // an array[2] for MultiPoint, lat/lon to make bbox
                                                      // of decomposed points
      PolyTessGeo             *pPolyTessGeo;
      PolyTessGeoTrap         *pPolyTrapGeo;

      wxBoundingBox           BBObj;                  // lat/lon BBox of the rendered object
      double                  m_lat;                  // The lat/lon of the object's "reference" point
      double                  m_lon;
      bool                    bBBObj_valid;           // set after the BBObj has been calculated once.

      Rules                   *CSrules;               // per object conditional symbology
      int                     bCS_Added;

      S52_Text                *FText;
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

      DisCat                  m_DisplayCat;


                                                      // This transform converts from object geometry
                                                      // to SM coordinates.
      double                  x_rate;                 // These auxiliary transform coefficients are
      double                  y_rate;                 // to be used in GetPointPix() and friends
      double                  x_origin;               // on a per-object basis if necessary
      double                  y_origin;

};





// object rasterization rules
typedef struct _ObjRazRules{
   LUPrec          *LUP;
   S57Obj          *obj;
   s57chart        *chart;                //dsr ... chart object owning this rule set
   struct _ObjRazRules *child;            // child list, used only for MultiPoint Soundings
   struct _ObjRazRules *next;
}ObjRazRules;






//----------------------------------------------------------------------------------
//          Used for s52 Fast Polygon Renderer
//----------------------------------------------------------------------------------
class render_canvas_parms
{
public:
      render_canvas_parms(void);
      render_canvas_parms(int x, int y, int width, int height, wxColour color);
      ~render_canvas_parms(void);

      unsigned char           *pix_buff;
      int                     lclip;
      int                     rclip;
      int                     pb_pitch;
      int                     x;
      int                     y;
      int                     width;
      int                     height;
      int                     depth;
      bool                    b_stagger;
};

//----------------------------------------------------------------------------------
//          Classes used to create arrays of geometry elements
//----------------------------------------------------------------------------------

class VE_Element
{
public:
      int         index;
      int         nCount;
      double      *pPoints;
      int         max_priority;
};

class VC_Element
{
public:
      int         index;
      double      *pPoint;
};




#endif
