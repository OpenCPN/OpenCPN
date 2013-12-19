/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Tesselation of Polygon Object
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
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
#define ERROR_NONE              0
#define ERROR_NO_DLL            1
#define ERROR_BAD_OGRPOLY       2

//  Some external prototypes


//--------------------------------------------------------------------------------------------------
//
//      Internal data structures used for describing/rendering tesselated polygons
//
//--------------------------------------------------------------------------------------------------

class Extended_Geometry
{
public:
      Extended_Geometry();
      ~Extended_Geometry();

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

      //    Conversion parameters
      //    for (assummed linear) convertions from vertex_array points to easting/northing, metres from 0,0
      //    To convert to lat/lon, use simple merctor equations
      double            x_rate;
      double            x_offset;
      double            y_rate;
      double            y_offset;
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

        double      minx, miny, maxx, maxy;

        TriPrim     *p_next;                // chain link
};



class PolyTriGroup
{
public:
        PolyTriGroup();
        PolyTriGroup(Extended_Geometry *pxGeom);
        ~PolyTriGroup();

        int             nContours;
        int             *pn_vertex;             // pointer to array of poly vertex counts
        float           *pgroup_geom;           // pointer to Raw geometry, used for contour line drawing

        TriPrim         *tri_prim_head;         // head of linked list of TriPrims
        bool            m_bSMSENC;

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

        PolyTessGeo(Extended_Geometry *pxGeom);

        bool IsOk(){ return m_bOK;}

        int BuildDeferredTess(void);

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
        int BuildTessGL(void);
        int PolyTessGeoGL(OGRPolygon *poly, bool bSENC_SM, double ref_lat, double ref_lon);
        int PolyTessGeoTri(OGRPolygon *poly, bool bSENC_SM, double ref_lat, double ref_lon);
        int my_bufgets( char *buf, int buf_len_max );



    //  Data

        bool            m_bOK;

        Extended_Geometry     *m_pxgeom;

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
