/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Tesselation of Polygon Object
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
 * $Log: mygeom.h,v $
 * Revision 1.13  2010/06/07 15:28:56  bdbcat
 * 607a
 *
 * Revision 1.12  2010/06/06 20:49:58  bdbcat
 * 606a
 *
 * Revision 1.11  2010/04/27 01:44:56  bdbcat
 * Build 426
 *
 * Revision 1.10  2009/06/03 03:21:51  bdbcat
 * Cleanup.
 *
 * Revision 1.9  2009/03/26 22:35:35  bdbcat
 * Opencpn 1.3.0 Update
 *
 * Revision 1.8  2008/12/09 03:32:16  bdbcat
 * Add stream method
 *
 * Revision 1.7  2008/03/30 23:28:29  bdbcat
 * Cleanup/optimize
 *
 * Revision 1.6  2007/06/10 02:37:18  bdbcat
 * Cleanup
 *
 * Revision 1.5  2007/05/03 13:31:19  dsr
 * Major refactor for 1.2.0
 *
 * Revision 1.4  2007/03/02 02:06:21  dsr
 * Convert to UTM Projection
 *
 * Revision 1.3  2006/10/01 03:24:40  dsr
 * no message
 *
 * Revision 1.2  2006/09/21 01:38:23  dsr
 * Major refactor/cleanup
 *
 * Revision 1.1.1.1  2006/08/21 05:52:11  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.5  2006/07/28 20:47:50  dsr
 * Cleanup
 *
 * Revision 1.4  2006/06/02 02:06:57  dsr
 * Cleanup
 *
 * Revision 1.1.1.1  2006/04/19 03:23:27  dsr
 * Rename/Import to OpenCPN
 *
 * Revision 1.3  2006/03/16 03:28:12  dsr
 * Cleanup tabs
 *
 * Revision 1.2  2006/02/23 01:22:42  dsr
 * Cleanup
 *
 *
 *
 */


#ifndef __MYGEOM_H
#define __MYGEOM_H

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include <wx/wfstream.h>


#include <ogr_geometry.h>
#include "s52s57.h"

#define TESS_VERT   0                           // constants describing preferred tess orientation
#define TESS_HORZ   1

#define EQUAL_EPS 1.0e-7                        // tolerance value


//  nota bene  These definitions are identical to OpenGL prototypes
#define PTG_TRIANGLES                      0x0004
#define PTG_TRIANGLE_STRIP                 0x0005
#define PTG_TRIANGLE_FAN                   0x0006

//  Error Return Codes
#define ERROR_NO_DLL            1

//  Some external prototypes


//--------------------------------------------------------------------------------------------------
//
//      Internal data structures used for describing/rendering tesselated polygons
//
//--------------------------------------------------------------------------------------------------

class Extended_Geometry
{
public:

      OGRGeometry       *pogrGeom;
      int               n_vector_indices;
      int               *pvector_index;
      int               n_contours;                          // parameters passed to trapezoid tesselator
      int               *contour_array;
      int               n_max_vertex;
      int               pointx;
      int               pointy;
      wxPoint2DDouble   *vertex_array;
      int               xmin, xmax, ymin, ymax;
      int               n_max_edge_points;
};


class TriPrim
{
public:
        TriPrim();
        ~TriPrim();

        unsigned int type;                  // Type of triangle primitive
                                            //  May be PTG_TRIANGLES
                                            //         PTG_TRIANGLE_STRIP
                                            //         PTG_TRIANGLE_FAN

        int         nVert;
        double      *p_vertex;              //  Pointer to vertex array, x,y,x,y.....

        wxBoundingBox *p_bbox;

        TriPrim     *p_next;                // chain link
};



class PolyTriGroup
{
public:
        PolyTriGroup();
        ~PolyTriGroup();

        int             nContours;
        int             *pn_vertex;             // pointer to array of poly vertex counts
        float           *pgroup_geom;           // pointer to Raw geometry, used for contour line drawing

        TriPrim         *tri_prim_head;         // head of linked list of TriPrims

    private:
        int my_bufgets( char *buf, int buf_len_max );

};


typedef struct {
      int         ilseg;
      int         irseg;
      double      loy;
      double      hiy;

} trapz_t;



class PolyTrapGroup
{
      public:
            PolyTrapGroup();
            PolyTrapGroup(Extended_Geometry *pxGeom);
            ~PolyTrapGroup();

            int             nContours;
            int             *pn_vertex;             // pointer to array of poly vertex counts
            wxPoint2DDouble *ptrapgroup_geom;       // pointer to Raw geometry, used for contour line drawing

            int             ntrap_count;
            trapz_t         *trap_array;
            int             m_trap_error;
};




//--------------------------------------------------------------------------------------------------
//
//      Triangle Tesselator Class
//
//--------------------------------------------------------------------------------------------------
class PolyTessGeo
{
    public:
        PolyTessGeo();
        ~PolyTessGeo();

        PolyTessGeo(unsigned char *polybuf, int nrecl, int index);      // Build this from SENC file record

        PolyTessGeo(OGRPolygon *poly, bool bSENC_SM,
            double ref_lat, double ref_lon,  bool bUseInternalTess);  // Build this from OGRPolygon

        int Write_PolyTriGroup( FILE *ofs);
        int Write_PolyTriGroup( wxOutputStream &ostream);

        double Get_xmin(){ return xmin;}
        double Get_xmax(){ return xmax;}
        double Get_ymin(){ return ymin;}
        double Get_ymax(){ return ymax;}
        PolyTriGroup *Get_PolyTriGroup_head(){ return m_ppg_head;}
        int GetnVertexMax(){ return m_nvertex_max; }
        int     ErrorCode;


    private:
        int PolyTessGeoGL(OGRPolygon *poly, bool bSENC_SM, double ref_lat, double ref_lon);
        int PolyTessGeoTri(OGRPolygon *poly, bool bSENC_SM, double ref_lat, double ref_lon);
        int my_bufgets( char *buf, int buf_len_max );



    //  Data

        double         xmin, xmax, ymin, ymax;
        PolyTriGroup    *m_ppg_head;                  // head of a PolyTriGroup chain
        int             m_nvertex_max;                 // and computed max vertex count
                                                      // used by drawing primitives as
                                                      // optimization

        int             ncnt;
        int             nwkb;

        char           *m_buf_head;
        char           *m_buf_ptr;                   // used to read passed SENC record
        int            m_nrecl;

        double         m_ref_lat, m_ref_lon;

};


//--------------------------------------------------------------------------------------------------
//
//      Trapezoid Tesselator Class
//
//--------------------------------------------------------------------------------------------------
class PolyTessGeoTrap
{
      public:
            PolyTessGeoTrap();
            ~PolyTessGeoTrap();


            PolyTessGeoTrap(Extended_Geometry *pxGeom);  // Build this from Extended Geometry

            void BuildTess();

            double Get_xmin(){ return xmin;}
            double Get_xmax(){ return xmax;}
            double Get_ymin(){ return ymin;}
            double Get_ymax(){ return ymax;}
            PolyTrapGroup *Get_PolyTrapGroup_head(){ return m_ptg_head;}
            int GetnVertexMax(){ return m_nvertex_max; }
            bool IsOk(){ return m_bOK;}
            int     ErrorCode;


      private:



    //  Data
            bool            m_bOK;

            double          xmin, xmax, ymin, ymax;
            PolyTrapGroup   *m_ptg_head;                  // PolyTrapGroup
            int             m_nvertex_max;                // computed max vertex count
                                                          // used by drawing primitives as
                                                          // optimization for malloc
            int             m_ncnt;
            int             m_nwkb;

};




#endif
