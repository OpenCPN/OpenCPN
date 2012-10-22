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

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

//#include "dychart.h"        // for some compile time fixups
//#include "chcanv.h"
//#include "cutil.h"
//#include "georef.h"
#include <wx/graphics.h>

#include "IsoLine.h"

static void GenerateSpline(int n, wxPoint points[]);
static void ClearSplineList();
wxList ocpn_wx_spline_point_list;

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(MySegList);
WX_DEFINE_LIST(MySegListList);

#ifndef PI
#define PI 3.14159
#endif
#define CTRUE -1
#define CFALSE 0


/* Local variables for cohen_sutherland_line_clip: */
struct LOC_cohen_sutherland_line_clip {
      double xmin, xmax, ymin, ymax;
} ;
void CompOutCode (double x, double y, outcode *code, struct LOC_cohen_sutherland_line_clip *LINK)
{
      /*Compute outcode for the point (x,y) */
      *code = 0;
      if (y > LINK->ymax)
            *code = 1L << ((long)TOP);
      else if (y < LINK->ymin)
            *code = 1L << ((long)BOTTOM);
      if (x > LINK->xmax)
            *code |= 1L << ((long)RIGHT);
      else if (x < LINK->xmin)
            *code |= 1L << ((long)LEFT);
}


ClipResult cohen_sutherland_line_clip_d (double *x0, double *y0, double *x1, double *y1,
                                         double xmin_, double xmax_, double ymin_, double ymax_)
{
      /* Cohen-Sutherland clipping algorithm for line P0=(x1,y0) to P1=(x1,y1)
       *    and clip rectangle with diagonal from (xmin,ymin) to (xmax,ymax).*/
      struct LOC_cohen_sutherland_line_clip V;
      int done = CFALSE;
      ClipResult clip = Visible;
      outcode outcode0, outcode1, outcodeOut;
      /*Outcodes for P0,P1, and whichever point lies outside the clip rectangle*/
      double x=0., y=0.;

      V.xmin = xmin_;
      V.xmax = xmax_;
      V.ymin = ymin_;
      V.ymax = ymax_;
      CompOutCode(*x0, *y0, &outcode0, &V);
      CompOutCode(*x1, *y1, &outcode1, &V);
      do {
            if (outcode0 == 0 && outcode1 == 0) {   /*Trivial accept and exit*/
                  done = CTRUE;
      } else if ((outcode0 & outcode1) != 0) {
            clip = Invisible;
            done = CTRUE;
      }
      /*Logical intersection is true, so trivial reject and exit.*/
      else {
            clip = Visible;
            /*Failed both tests, so calculate the line segment to clip;
             *            from an outside point to an intersection with clip edge.*/
            /*At least one endpoint is outside the clip rectangle; pick it.*/
            if (outcode0 != 0)
                  outcodeOut = outcode0;
            else
                  outcodeOut = outcode1;
            /*Now find intersection point;
             *            use formulas y=y0+slope*(x-x0),x=x0+(1/slope)*(y-y0).*/

            if (((1L << ((long)TOP)) & outcodeOut) != 0) {
                  /*Divide line at top of clip rectangle*/
                  x = *x0 + (*x1 - *x0) * (V.ymax - *y0) / (*y1 - *y0);
                  y = V.ymax;
            } else if (((1L << ((long)BOTTOM)) & outcodeOut) != 0) {
                  /*Divide line at bottom of clip rectangle*/
                  x = *x0 + (*x1 - *x0) * (V.ymin - *y0) / (*y1 - *y0);
                  y = V.ymin;
            } else if (((1L << ((long)RIGHT)) & outcodeOut) != 0) {
                  /*Divide line at right edge of clip rectangle*/
                  y = *y0 + (*y1 - *y0) * (V.xmax - *x0) / (*x1 - *x0);
                  x = V.xmax;
            } else if (((1L << ((long)LEFT)) & outcodeOut) != 0) {
                  /*Divide line at left edge of clip rectangle*/
                  y = *y0 + (*y1 - *y0) * (V.xmin - *x0) / (*x1 - *x0);
                  x = V.xmin;
            }
            /*Now we move outside point to intersection point to clip,
             *            and get ready for next pass.*/
            if (outcodeOut == outcode0) {
                  *x0 = x;
                  *y0 = y;
                  CompOutCode(*x0, *y0, &outcode0, &V);
            } else {
                  *x1 = x;
                  *y1 = y;
                  CompOutCode(*x1, *y1, &outcode1, &V);
            }
      }
} while (!done);
return clip;
}

ClipResult cohen_sutherland_line_clip_i (int *x0_, int *y0_, int *x1_, int *y1_,
                                         int xmin_, int xmax_, int ymin_, int ymax_)
{
      ClipResult ret;
      double x0,y0,x1,y1;
      x0 = *x0_;
      y0 = *y0_;
      x1 = *x1_;
      y1 = *y1_;
      ret = cohen_sutherland_line_clip_d (&x0, &y0, &x1, &y1,
                                          (double)xmin_, (double)xmax_,
                                          (double)ymin_, (double)ymax_);
      *x0_ = (int)x0;
      *y0_ = (int)y0;
      *x1_ = (int)x1;
      *y1_ = (int)y1;
      return ret;
}


double      round_msvc (double x)
{
      return(floor(x + 0.5));

}


//---------------------------------------------------------------
IsoLine::IsoLine(double val, const GribRecord *rec_)
{
    value = val;

    rec = rec_;
    W = rec->getNi();
    H = rec->getNj();

    //---------------------------------------------------------
    // Génère la liste des segments.
    extractIsoLine(rec);

    if(trace.size() == 0)
          return;

    //      Join the isoline segments into a nice list
    //      Which is end-to-end continuous and unidirectional

    //      Create a master wxList of the trace list
    std::list<Segment *>::iterator it;
    for (it=trace.begin(); it!=trace.end(); it++)
    {
          Segment *seg = *it;
          seg->bUsed = false;
          m_seglist.Append(*it);
    }

    //      Isoline may be discontinuous....
    //      So build a list of continuous segments
    bool bdone = false;
    while(!bdone)
    {
          MySegList *ps = BuildContinuousSegment();

          m_SegListList.Append(ps);

          MySegList::Node *node;
          Segment *seg;

          // recreate the master list, removing used segs

          node = m_seglist.GetFirst();
          while(node)
          {
                seg = node->GetData();
                if(seg->bUsed)
                {
                      m_seglist.Erase(node);
                      node = m_seglist.GetFirst();
                }
                else
                      node = node->GetNext();
          }

          if(0 == m_seglist.GetCount())
                bdone = true;
    }

///printf("create Isobar : press=%4.0f long=%d\n", pressure/100, trace.size());
}
//---------------------------------------------------------------
IsoLine::~IsoLine()
{
//printf("delete Isobar : press=%4.0f long=%d\n", pressure/100, trace.size());

    std::list<Segment *>::iterator it;
    for (it=trace.begin(); it!=trace.end(); it++) {
        delete *it;
        *it = NULL;
    }
    trace.clear();

    m_SegListList.DeleteContents(true);
    m_SegListList.Clear();

}


MySegList *IsoLine::BuildContinuousSegment(void)
{
      MySegList::Node *node;
      Segment *seg;

      MySegList *ret_list = new MySegList;

    //     Build a chain extending from the "2" end of the target segment
    //      The joined list, side 2...
      MySegList segjoin2;

    //      Add any first segment to the list
      node = m_seglist.GetFirst();
      Segment *seg0 = node->GetData();
      seg0->bUsed = true;
      segjoin2.Append(seg0);

      Segment *tseg = seg0;

      while(tseg)
      {
            bool badded = false;
            Segment *seg;
            node = m_seglist.GetFirst();
            while (node)
            {
                  seg = node->GetData();

                  if((!seg->bUsed) && (seg->py1 == tseg->py2) && (seg->px1 == tseg->px2))              // fits without reverse
                  {
                        seg->bUsed = true;
                        segjoin2.Append(seg);
                        badded = true;
                        break;
                  }
                  else if((!seg->bUsed) && (seg->py2 == tseg->py2) && (seg->px2 == tseg->px2))         // fits, needs reverse
                  {
                        seg->bUsed = true;
                        double a = seg->px2; seg->px2 = seg->px1; seg->px1 = a;
                        double b = seg->py2; seg->py2 = seg->py1; seg->py1 = b;
                        segjoin2.Append(seg);
                        badded = true;
                        break;
                  }

                  node = node->GetNext();
            }
            if(badded == true)
                  tseg = seg;
            else
                  tseg = NULL;
      }



    //     Build a chain extending from the "1" end of the target segment
    //      The joined list, side 1...
      MySegList segjoin1;

    //      Add the same first segment to the list
      node = m_seglist.GetFirst();
      seg0 = node->GetData();
      seg0->bUsed = true;
      segjoin1.Append(seg0);

      tseg = seg0;

      while(tseg)
      {
            bool badded = false;
            node = m_seglist.GetFirst();
            while (node)
            {
                  seg = node->GetData();

                  if((!seg->bUsed) && (seg->py2 == tseg->py1) && (seg->px2 == tseg->px1))              // fits without reverse
                  {
                        seg->bUsed = true;
                        segjoin1.Append(seg);
                        badded = true;
                        break;
                  }
                  else if((!seg->bUsed) && (seg->py1 == tseg->py1) && (seg->px1 == tseg->px1))         // fits, needs reverse
                  {
                        seg->bUsed = true;
                        double a = seg->px2; seg->px2 = seg->px1; seg->px1 = a;
                        double b = seg->py2; seg->py2 = seg->py1; seg->py1 = b;
                        segjoin1.Append(seg);
                        badded = true;
                        break;
                  }

                  node = node->GetNext();
            }
            if(badded == true)
                  tseg = seg;
            else
                  tseg = NULL;
      }


     //     Now have two lists...

     //     Start with "1" side list,
     //    starting from the end, and skipping the first segment

      int n1 = segjoin1.GetCount();
      for(int i=n1 - 1 ; i > 0 ; i--)
      {
            node = segjoin1.Item( i );
            seg = node->GetData();
            ret_list->Append(seg);
      }

     //     Now add the "2"side list
      int n2 = segjoin2.GetCount();
      for(int i=0 ; i < n2 ; i++)
      {
            node = segjoin2.Item(i);
            seg = node->GetData();
            ret_list->Append(seg);
      }

     //     And there it is

      return ret_list;
}


//---------------------------------------------------------------
void IsoLine::drawIsoLine(GRIBOverlayFactory *pof, wxDC &dc, PlugIn_ViewPort *vp, bool bShowLabels, bool bHiDef)
{
      int nsegs = trace.size();
      if(nsegs < 1)
            return;

      GetGlobalColor ( _T ( "UITX1" ), &isoLineColor );
      wxPen ppISO ( isoLineColor, 2 );

#if wxUSE_GRAPHICS_CONTEXT
      wxMemoryDC *pmdc;
      pmdc= wxDynamicCast(&dc, wxMemoryDC);
      wxGraphicsContext *pgc = wxGraphicsContext::Create(*pmdc);
      pgc->SetPen(ppISO);
#endif

      dc.SetPen(ppISO);

      std::list<Segment *>::iterator it;

    //---------------------------------------------------------
    // Dessine les segments
    //---------------------------------------------------------
    for (it=trace.begin(); it!=trace.end(); it++)
    {
        Segment *seg = *it;

        {
            wxPoint ab;
            GetCanvasPixLL(vp, &ab, seg->py1, seg->px1);
            wxPoint cd;
            GetCanvasPixLL(vp, &cd, seg->py2, seg->px2);

             {
#if wxUSE_GRAPHICS_CONTEXT
                  if(bHiDef && pgc)
                        pgc->StrokeLine(ab.x, ab.y, cd.x, cd.y);
                  else
                        dc.DrawLine(ab.x, ab.y, cd.x, cd.y);
#else
                  dc.DrawLine(ab.x, ab.y, cd.x, cd.y);
#endif
             }

        }
    }
//#endif

      int text_sx, text_sy;
      dc.GetTextExtent(_T("10000"), &text_sx, &text_sy);
//      double m = text_sy / 2;
      int label_size = text_sx;
      int label_space = 400;
//      double coef = .01;
      int len = label_space/4;

      //    Allocate an array big enough
      wxPoint *pPoints = new wxPoint[nsegs+1];

      MySegListList::Node *listnode;
      listnode = m_SegListList.GetFirst();
      while(listnode)
      {
            MySegList *listsort = listnode->GetData();

            //    Fill in the array
            MySegList::Node *node;
            Segment *seg;

            node = listsort->GetFirst();
            if(node)
            {
                  seg = node->GetData();
//                  wxPoint ab = vp->GetMercatorPixFromLL(seg->py1, seg->px1);
//                  wxPoint ab(0,0);
				  wxPoint ab;
				  GetCanvasPixLL(vp, &ab, seg->py1, seg->px1);
                  pPoints[0] = ab;
            }
            int ip=1;

            while (node)
            {
                  seg = node->GetData();
//                  wxPoint cd = vp->GetMercatorPixFromLL(seg->py2, seg->px2);
//                  wxPoint cd(0,0);
				  wxPoint cd;
				  GetCanvasPixLL(vp, &cd, seg->py2, seg->px2);
                  pPoints[ip++] = cd;

                  node=node->GetNext();
            }

            int np = listsort->GetCount() + 1;


            if(np > 1)
            {

      // Test code
      //          dc.DrawLines(np, pPoints);

                  GenerateSpline(np, pPoints);

      //    Test Code
      //            dc.DrawLines(&ocpn_wx_spline_point_list, 0, 0 );

                  bool bDrawing = true;
                  wxPoint lstart;

                  wxList::compatibility_iterator snode = ocpn_wx_spline_point_list.GetFirst();
                  wxPoint *point0 = (wxPoint *)snode->GetData();
                  snode=snode->GetNext();

                  while (snode)
                  {
                        wxPoint *point = (wxPoint *)snode->GetData();

                        ClipResult res = cohen_sutherland_line_clip_i ( &point0->x, &point0->y, &point->x, &point->y,
                                    0, vp->pix_width, 0, vp->pix_height );
                        if ( res != Invisible )
                        {
                              int dl = (int)sqrt(
                                            (double)((point0->x - point->x) * (point0->x - point->x))
                                          +(double)((point0->y - point->y) * (point0->y - point->y)));
                              if(bDrawing)
                              {
                                    len += dl;
                                    if(len > label_space)
                                    {
                                          bDrawing = false;
                                          len = 0;
                                          lstart = *point;
                                    }
                              }
                              else
                              {
                                    len += dl;
                                    if(len > label_size)
                                    {
                                          bDrawing = true;
                                          len = 0;
#if 0
                                          if(bShowLabels)
                                          {
                                                double label_angle = atan2((double)(lstart.y - point->y),
                                                    (double)(point->x - lstart.x)) * 180. / PI;
                                                wxString label;
                                                label.Printf(_T("%d"), (int)(value*coef+0.5));

                                                double xs = lstart.x - (m * sin(label_angle * PI / 180.));
                                                double ys = lstart.y - (m * cos(label_angle * PI / 180.));
                                                dc.DrawRotatedText(label, (int)xs, (int)ys, label_angle);
                                          }
#endif
                                    }
                              }

#if 0
//                              if(bDrawing || !bShowLabels)
                              {
                                    if(bHiDef)
                                          dc.StrokeLine(point0->x, point0->y, point->x, point->y);
                                    else
                                          dc.DrawLine(point0->x, point0->y, point->x, point->y);
                              }
#endif
                        }

                        *point0 = *point;
                        snode=snode->GetNext();
                  }

                  ClearSplineList();
            }

            listnode = listnode->GetNext();             // Next continuous chain

      }

      delete[] pPoints;

#if wxUSE_GRAPHICS_CONTEXT
      delete pgc;
#endif

}

//---------------------------------------------------------------

void IsoLine::drawIsoLineLabels(GRIBOverlayFactory *pof, wxDC &dc, wxColour text_color, wxColour back_color,
                                PlugIn_ViewPort *vp, int density, int first, double coef)
{
///
//#if 0
    std::list<Segment *>::iterator it;
    int nb = first;
    wxString label;

    label.Printf(_T("%d"), (int)(value*coef+0.5));

    wxPen penText(text_color);

    int w, h;
    dc.GetTextExtent(label, &w, &h);

    dc.SetPen(penText);
    dc.SetBrush(wxBrush(back_color));
    dc.SetTextForeground(text_color);
    dc.SetTextBackground(back_color);

    //---------------------------------------------------------
    // Ecrit les labels
    //---------------------------------------------------------
    for (it=trace.begin(); it!=trace.end(); it++,nb++)
    {
        if (nb % density == 0)
		{
            Segment *seg = *it;

//            if(vp->vpBBox.PointInBox((seg->px1 + seg->px2)/2., (seg->py1 + seg->py2)/2., 0.))
            {
 //                 wxPoint ab = vp->GetMercatorPixFromLL(seg->py1, seg->px1);
 //                 wxPoint cd = vp->GetMercatorPixFromLL(seg->py2, seg->px2);
			wxPoint ab;
			GetCanvasPixLL(vp, &ab, seg->py1, seg->px1);
			wxPoint cd;
			GetCanvasPixLL(vp, &cd, seg->py1, seg->px1);

			int label_offset = 6;
                  int xd = (ab.x + cd.x-(w+label_offset * 2))/2;
                  int yd = (ab.y + cd.y - h)/2;


                  dc.DrawRoundedRectangle(xd, yd, w+(label_offset * 2), h, -.25);
                  dc.DrawText(label, label_offset/2 + xd, yd-1);
            }

        }
    }
//#endif
///
}


void IsoLine::drawGLIsoLine(GRIBOverlayFactory *pof, PlugIn_ViewPort *vp, bool bShowLabels, bool bHiDef)
{
      int nsegs = trace.size();
      if(nsegs < 1)
            return;

     int width = 2;
     GetGlobalColor ( _T ( "UITX1" ), &isoLineColor );

     glColor4ub(isoLineColor.Red(), isoLineColor.Green(), isoLineColor.Blue(), 255/*isoLineColor.Alpha()*/);
     glLineWidth(width);

     std::list<Segment *>::iterator it;

    //---------------------------------------------------------
    // Dessine les segments
    //---------------------------------------------------------
    for (it=trace.begin(); it!=trace.end(); it++)
    {
        Segment *seg = *it;

        {
            wxPoint ab;
            GetCanvasPixLL(vp, &ab, seg->py1, seg->px1);
            wxPoint cd;
            GetCanvasPixLL(vp, &cd, seg->py2, seg->px2);


///            ClipResult res = cohen_sutherland_line_clip_i ( &ab.x, &ab.y, &cd.x, &cd.y,
///                         0, vp->pix_width, 0, vp->pix_height );
///            if ( res != Invisible )
             {
                  pof->DrawGLLine(ab.x, ab.y, cd.x, cd.y, width);
             }

        }
    }

}

void IsoLine::drawGLIsoLineLabels(GRIBOverlayFactory *pof, wxColour text_color, wxColour back_color,
                                PlugIn_ViewPort *vp, int density, int first, double coef)
{
    std::list<Segment *>::iterator it;
    int nb = first;
    wxString label;

    label.Printf(_T("%d"), (int)(value*coef+0.5));

    int w, h;

    wxPen penText(text_color);
    wxBrush backBrush(back_color);

    int label_offset = 10;

      if(!m_imageLabel.IsOk())
      {
            wxBitmap bm(100,100);          // big enough
            wxMemoryDC mdc(bm);
            mdc.Clear();

            mdc.GetTextExtent(label, &w, &h);

            mdc.SetPen(penText);
            mdc.SetBrush(backBrush);
            mdc.SetTextForeground(text_color);
            mdc.SetTextBackground(back_color);

            int xd = 0;
            int yd = 0;
//            mdc.DrawRoundedRectangle(xd, yd, w+(label_offset * 2), h, -.25);
            mdc.DrawRectangle(xd, yd, w+(label_offset * 2), h+2);
            mdc.DrawText(label, label_offset/2 + xd, yd-1);

            mdc.SelectObject(wxNullBitmap);

            wxBitmap sub_BMLabel = bm.GetSubBitmap(wxRect(0,0,w+(label_offset * 2), h+2));
            m_imageLabel = sub_BMLabel.ConvertToImage();
      }

    //---------------------------------------------------------
    // Ecrit les labels
    //---------------------------------------------------------
    for (it=trace.begin(); it!=trace.end(); it++,nb++)
    {
        if (nb % density == 0)
		{
            Segment *seg = *it;

//            if(vp->vpBBox.PointInBox((seg->px1 + seg->px2)/2., (seg->py1 + seg->py2)/2., 0.))
            {
                  int w = m_imageLabel.GetWidth();
                  int h = m_imageLabel.GetHeight();
                  wxPoint ab;
                  GetCanvasPixLL(vp, &ab, seg->py1, seg->px1);
                  wxPoint cd;
                  GetCanvasPixLL(vp, &cd, seg->py1, seg->px1);

                  int xd = (ab.x + cd.x-(w+label_offset * 2))/2;
                  int yd = (ab.y + cd.y - h)/2;

                  glRasterPos2i(xd, yd);
                  glPixelZoom(1, -1); /* draw data from top to bottom */
                  glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, m_imageLabel.GetData());
                  glPixelZoom(1, 1);
            }
        }
    }
}



//==================================================================================
// Segment
//==================================================================================
Segment::Segment(int I, int J,
                char c1, char c2, char c3, char c4,
                const GribRecord *rec, double pressure)
{
    traduitCode(I,J, c1, i,j);
    traduitCode(I,J, c2, k,l);
    traduitCode(I,J, c3, m,n);
    traduitCode(I,J, c4, o,p);

    intersectionAreteGrille(i,j, k,l,  &px1,&py1, rec, pressure);
    intersectionAreteGrille(m,n, o,p,  &px2,&py2, rec, pressure);
}
//-----------------------------------------------------------------------
void Segment::intersectionAreteGrille(int i,int j, int k,int l, double *x, double *y,
                const GribRecord *rec, double pressure)
{
    double a,b, pa, pb, dec;
    pa = rec->getValue(i,j);
    pb = rec->getValue(k,l);
    // Abscisse
    a = rec->getX(i);
    b = rec->getX(k);
    if (pb != pa)
        dec = (pressure-pa)/(pb-pa);
    else
        dec = 0.5;
    if (fabs(dec)>1)
        dec = 0.5;
    *x = a+(b-a)*dec;
    // Ordonnée
    a = rec->getY(j);
    b = rec->getY(l);
    if (pb != pa)
        dec = (pressure-pa)/(pb-pa);
    else
        dec = 0.5;
    if (fabs(dec)>1)
        dec = 0.5;
    *y = a+(b-a)*dec;
}
//---------------------------------------------------------------
void Segment::traduitCode(int I, int J, char c1, int &i, int &j) {
    switch (c1) {
        case 'a':  i=I-1;  j=J-1; break;
        case 'b':  i=I  ;  j=J-1; break;
        case 'c':  i=I-1;  j=J  ; break;
        case 'd':  i=I  ;  j=J  ; break;
        default:   i=I  ;  j=J  ;
    }
}

//-----------------------------------------------------------------------
// Génère la liste des segments.
// Les coordonnées sont les indices dans la grille du GribRecord
//---------------------------------------------------------
void IsoLine::extractIsoLine(const GribRecord *rec)
{
    int i, j, W, H;
    double  a,b,c,d;
    W = rec->getNi();
    H = rec->getNj();

    for (j=1; j<H; j++)     // !!!! 1 to end
    {
        for (i=1; i<W; i++)
        {
//            x = rec->getX(i);
//            y = rec->getY(j);

            a = rec->getValue( i-1, j-1 );
            b = rec->getValue( i,   j-1 );
            c = rec->getValue( i-1, j   );
            d = rec->getValue( i,   j   );

            // Détermine si 1 ou 2 segments traversent la case ab-cd
            // a  b
            // c  d
            //--------------------------------
            // 1 segment en diagonale
            //--------------------------------
            if     ((a<=value && b<=value && c<=value  && d>value)
                 || (a>value && b>value && c>value  && d<=value))
                trace.push_back(new Segment(i,j, 'c','d',  'b','d', rec,value));
            else if ((a<=value && c<=value && d<=value  && b>value)
                 || (a>value && c>value && d>value  && b<=value))
                trace.push_back(new Segment(i,j, 'a','b',  'b','d', rec,value));
            else if ((c<=value && d<=value && b<=value  && a>value)
                 || (c>value && d>value && b>value  && a<=value))
                trace.push_back(new Segment(i,j, 'a','b',  'a','c', rec,value));
            else if ((a<=value && b<=value && d<=value  && c>value)
                 || (a>value && b>value && d>value  && c<=value))
                trace.push_back(new Segment(i,j, 'a','c',  'c','d', rec,value));
            //--------------------------------
            // 1 segment H ou V
            //--------------------------------
            else if ((a<=value && b<=value   &&  c>value && d>value)
                 || (a>value && b>value   &&  c<=value && d<=value))
                trace.push_back(new Segment(i,j, 'a','c',  'b','d', rec,value));
            else if ((a<=value && c<=value   &&  b>value && d>value)
                 || (a>value && c>value   &&  b<=value && d<=value))
                trace.push_back(new Segment(i,j, 'a','b',  'c','d', rec,value));
            //--------------------------------
            // 2 segments en diagonale
            //--------------------------------
            else if  (a<=value && d<=value   &&  c>value && b>value) {
                trace.push_back(new Segment(i,j, 'a','b',  'b','d', rec,value));
                trace.push_back(new Segment(i,j, 'a','c',  'c','d', rec,value));
            }
            else if  (a>value && d>value   &&  c<=value && b<=value) {
                trace.push_back(new Segment(i,j, 'a','b',  'a','c', rec,value));
                trace.push_back(new Segment(i,j, 'b','d',  'c','d', rec,value));
            }

        }
    }
}


// ----------------------------------------------------------------------------
// splines code lifted from wxWidgets
// ----------------------------------------------------------------------------






// ----------------------------------- spline code ----------------------------------------

void ocpn_wx_quadratic_spline(double a1, double b1, double a2, double b2,
                         double a3, double b3, double a4, double b4);
void ocpn_wx_clear_stack();
int ocpn_wx_spline_pop(double *x1, double *y1, double *x2, double *y2, double *x3,
                  double *y3, double *x4, double *y4);
void ocpn_wx_spline_push(double x1, double y1, double x2, double y2, double x3, double y3,
                    double x4, double y4);
static bool ocpn_wx_spline_add_point(double x, double y);


#define                half(z1, z2)        ((z1+z2)/2.0)
#define                THRESHOLD        5

/* iterative version */

void ocpn_wx_quadratic_spline(double a1, double b1, double a2, double b2, double a3, double b3, double a4,
                         double b4)
{
      register double  xmid, ymid;
      double           x1, y1, x2, y2, x3, y3, x4, y4;

      ocpn_wx_clear_stack();
      ocpn_wx_spline_push(a1, b1, a2, b2, a3, b3, a4, b4);

      while (ocpn_wx_spline_pop(&x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4)) {
            xmid = (double)half(x2, x3);
            ymid = (double)half(y2, y3);
            if (fabs(x1 - xmid) < THRESHOLD && fabs(y1 - ymid) < THRESHOLD &&
                fabs(xmid - x4) < THRESHOLD && fabs(ymid - y4) < THRESHOLD) {
                  ocpn_wx_spline_add_point( x1, y1 );
                  ocpn_wx_spline_add_point( xmid, ymid );
                } else {
                      ocpn_wx_spline_push(xmid, ymid, (double)half(xmid, x3), (double)half(ymid, y3),
                                     (double)half(x3, x4), (double)half(y3, y4), x4, y4);
                      ocpn_wx_spline_push(x1, y1, (double)half(x1, x2), (double)half(y1, y2),
                                     (double)half(x2, xmid), (double)half(y2, ymid), xmid, ymid);
                }
      }
}

/* utilities used by spline drawing routines */

typedef struct ocpn_wx_spline_stack_struct {
      double           x1, y1, x2, y2, x3, y3, x4, y4;
} Stack;

#define         SPLINE_STACK_DEPTH             20
static Stack    ocpn_wx_spline_stack[SPLINE_STACK_DEPTH];
static Stack   *ocpn_wx_stack_top;
static int      ocpn_wx_stack_count;

void ocpn_wx_clear_stack()
{
      ocpn_wx_stack_top = ocpn_wx_spline_stack;
      ocpn_wx_stack_count = 0;
}

void ocpn_wx_spline_push(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
      ocpn_wx_stack_top->x1 = x1;
      ocpn_wx_stack_top->y1 = y1;
      ocpn_wx_stack_top->x2 = x2;
      ocpn_wx_stack_top->y2 = y2;
      ocpn_wx_stack_top->x3 = x3;
      ocpn_wx_stack_top->y3 = y3;
      ocpn_wx_stack_top->x4 = x4;
      ocpn_wx_stack_top->y4 = y4;
      ocpn_wx_stack_top++;
      ocpn_wx_stack_count++;
}

int ocpn_wx_spline_pop(double *x1, double *y1, double *x2, double *y2,
                  double *x3, double *y3, double *x4, double *y4)
{
      if (ocpn_wx_stack_count == 0)
            return (0);
      ocpn_wx_stack_top--;
      ocpn_wx_stack_count--;
      *x1 = ocpn_wx_stack_top->x1;
      *y1 = ocpn_wx_stack_top->y1;
      *x2 = ocpn_wx_stack_top->x2;
      *y2 = ocpn_wx_stack_top->y2;
      *x3 = ocpn_wx_stack_top->x3;
      *y3 = ocpn_wx_stack_top->y3;
      *x4 = ocpn_wx_stack_top->x4;
      *y4 = ocpn_wx_stack_top->y4;
      return (1);
}

static bool ocpn_wx_spline_add_point(double x, double y)
{
      wxPoint *point = new wxPoint ;
      point->x = (int) x;
      point->y = (int) y;
      ocpn_wx_spline_point_list.Append((wxObject*)point);
      return true;
}


void GenSpline( wxList *points )
{

      wxPoint *p;
      double           cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4;
      double           x1, y1, x2, y2;

      wxList::compatibility_iterator node = points->GetFirst();
      if (!node)
        // empty list
            return;

      p = (wxPoint *)node->GetData();

      x1 = p->x;
      y1 = p->y;

      node = node->GetNext();
      p = (wxPoint *)node->GetData();

      x2 = p->x;
      y2 = p->y;
      cx1 = (double)((x1 + x2) / 2);
      cy1 = (double)((y1 + y2) / 2);
      cx2 = (double)((cx1 + x2) / 2);
      cy2 = (double)((cy1 + y2) / 2);

      ocpn_wx_spline_add_point(x1, y1);

      while ((node = node->GetNext())
#if !wxUSE_STL
              != NULL
#endif // !wxUSE_STL
            )
      {
            p = (wxPoint *)node->GetData();
            x1 = x2;
            y1 = y2;
            x2 = p->x;
            y2 = p->y;
            cx4 = (double)(x1 + x2) / 2;
            cy4 = (double)(y1 + y2) / 2;
            cx3 = (double)(x1 + cx4) / 2;
            cy3 = (double)(y1 + cy4) / 2;

            ocpn_wx_quadratic_spline(cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4);

            cx1 = cx4;
            cy1 = cy4;
            cx2 = (double)(cx1 + x2) / 2;
            cy2 = (double)(cy1 + y2) / 2;
      }

      ocpn_wx_spline_add_point( cx1, cy1 );
      ocpn_wx_spline_add_point( x2, y2 );

}


static void GenerateSpline(int n, wxPoint points[])
{
      wxList list;
      for (int i =0; i < n; i++)
      {
            list.Append((wxObject*)&points[i]);
      }

      GenSpline(&list);
}

static void ClearSplineList()
{
      wxList::compatibility_iterator node = ocpn_wx_spline_point_list.GetFirst();
      while (node)
      {
            wxPoint *point = (wxPoint *)node->GetData();
            delete point;
            ocpn_wx_spline_point_list.Erase(node);
            node = ocpn_wx_spline_point_list.GetFirst();
      }
}

