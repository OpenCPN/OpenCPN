/**********************************************************************
zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://www.zygrib.org

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

/*************************************
Dessin des données GRIB (avec QT)
*************************************/

#ifndef ISOLINE_H
#define ISOLINE_H

#include <iostream>
#include <cmath>
#include <vector>
#include <list>
#include <set>

#include "../../../include/ocpn_plugin.h"

#include "GribReader.h"

class ViewPort;
class wxDC;

class Segment;
WX_DECLARE_LIST(Segment, MySegList);
WX_DECLARE_LIST(MySegList, MySegListList);


//-------------------------------------------------------------------------------------------------------
//  Cohen & Sutherland Line clipping algorithms
//-------------------------------------------------------------------------------------------------------
/*
 * 
 * Copyright (C) 1999,2000,2001,2002,2003 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * additional features: Andreas Klust <klust@users.sf.net>
 * WWW Home: http://gxsm.sf.net
 *
 */

typedef enum { Visible, Invisible } ClipResult;
typedef enum {
      LEFT, RIGHT, BOTTOM, TOP
} edge;
typedef long outcode;


void CompOutCode (double x, double y, outcode *code, struct LOC_cohen_sutherland_line_clip *LINK);
#ifdef __cplusplus
ClipResult cohen_sutherland_line_clip_d (double *x0, double *y0, double *x1, double *y1,
                                                     double xmin_, double xmax_, double ymin_, double ymax_);

extern "C"  ClipResult cohen_sutherland_line_clip_i (int *x0, int *y0, int *x1, int *y1,
                                                     int xmin_, int xmax_, int ymin_, int ymax_);

#endif


// TODO: join segments and draw a spline

//===============================================================
// Elément d'isobare qui passe dans un carré (ab-cd)de la grille.
// a  b
// c  d
// Rejoint l'arête (i,j)-(k,l) à l'arête (m,n)-(o,p) (indices ds la grille GRIB)

class Segment
{
    public:
        Segment (int I, int J,
                char c1, char c2, char c3, char c4,
                const GribRecord *rec, double pressure);

        int   i,j,  k,l;   // arête 1
        double px1,  py1;   // Coordonées de l'intersection (i,j)-(k,l)
        int m,n, o,p;      // arête 2
        double px2,  py2;   // Coordonées de l'intersection (m,n)-(o,p)
        bool  bUsed;

    private:
        void traduitCode(int I, int J, char c1, int &i, int &j);

        void intersectionAreteGrille(int i,int j, int k,int l,
                double *x, double *y,
                const GribRecord *rec, double pressure);
};

class GRIBOverlayFactory;
class TexFont;

//===============================================================
class IsoLine
{
    public:
         IsoLine(double val, double coeff, double offset, const GribRecord *rec);
        ~IsoLine();


        void drawIsoLine(GRIBOverlayFactory *pof, wxDC *dc, PlugIn_ViewPort *vp, bool bHiDef);

        void drawIsoLineLabels(GRIBOverlayFactory *pof, wxDC *dc,
                               PlugIn_ViewPort *vp, int density, int first,
                               wxImage &imageLabel);
        void drawIsoLineLabelsGL(GRIBOverlayFactory *pof, PlugIn_ViewPort *vp,
                                 int density, int first,
                                 wxString label, wxColour &color, TexFont &texfont);

        int getNbSegments()     {return trace.size();}

        double getValue() {return value;}
    private:
        double value;
        int    W, H;     // taille de la grille
        const  GribRecord *rec;

        wxColour  isoLineColor;
        std::list<Segment *> trace;


        void intersectionAreteGrille(int i,int j, int k,int l, double *x, double *y,
                        const GribRecord *rec);

        //-----------------------------------------------------------------------
        // Génère la liste des segments.
        // Les coordonnées sont les indices dans la grille du GribRecord
        //---------------------------------------------------------
        void extractIsoLine(const GribRecord *rec);
        MySegList *BuildContinuousSegment(void);

        MySegList       m_seglist;
        MySegListList   m_SegListList;
};




#endif
