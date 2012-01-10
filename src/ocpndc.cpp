/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Layer to perform wxDC drawing using wxDC or opengl
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2011 by Sean D'Epagnier                                 *
 *   sean at depagnier dot com                                             *
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
#ifdef __MSVC__
  #include <windows.h>
#endif

#ifdef __WXMSW__
#include "GL/gl.h"            // local copy for Windows
#else
#include <GL/gl.h>
#endif

#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/image.h>

#include <wx/glcanvas.h>


#include "ocpndc.h"

/* pass the dc to the constructor, or NULL to use opengl */
ocpnDC::ocpnDC(wxGLCanvas &canvas) : glcanvas(&canvas), dc(NULL), m_pen(wxNullPen), m_brush(wxNullBrush), pgc(NULL)
{
     glGenTextures(1, &tex);
     m_textforegroundcolour = wxColour(0,0,0);
}

ocpnDC::ocpnDC(wxDC &pdc) : glcanvas(NULL), dc(&pdc), m_pen(wxNullPen), m_brush(wxNullBrush)
{
     pgc = NULL;
#if wxUSE_GRAPHICS_CONTEXT
     wxMemoryDC *pmdc = wxDynamicCast(dc, wxMemoryDC);
     if(pmdc)
          pgc = wxGraphicsContext::Create(*pmdc);
     else
     {
          wxClientDC *pcdc = wxDynamicCast(dc, wxClientDC);
          if(pcdc)
               pgc = wxGraphicsContext::Create(*pcdc);
     }
#endif
     m_textforegroundcolour = wxColour(0,0,0);
}

ocpnDC::~ocpnDC()
{
     if(pgc)
          delete pgc;
     if(glcanvas)
          glDeleteTextures(1, &tex);
}

void ocpnDC::SetPen(const wxPen &pen)
{
     if(dc)
          dc->SetPen(pen);
     else
          m_pen = pen;
}

void ocpnDC::SetBrush(const wxBrush &brush)
{
     if(dc)
          dc->SetBrush(brush);
     else
          m_brush = brush;
}

void ocpnDC::SetTextForeground(const wxColour &colour)
{
     if(dc)
          dc->SetTextForeground(colour);
     else
          m_textforegroundcolour = colour;
}

void ocpnDC::SetFont(const wxFont& font)
{
     if(dc)
          dc->SetFont(font);
     else
          m_font = font;
}

const wxPen& ocpnDC::GetPen() const
{
     if(dc)
          return dc->GetPen();
     return m_pen;
}

const wxBrush& ocpnDC::GetBrush() const
{
     if(dc)
          return dc->GetBrush();
     return m_brush;
}

const wxFont& ocpnDC::GetFont() const
{
     if(dc)
          return dc->GetFont();
     return m_font;
}

void ocpnDC::GetSize(wxCoord *width, wxCoord *height) const
{
     if(dc)
          dc->GetSize(width, height);
     else
          glcanvas->GetSize(width, height);
}

// Draws a line between (x1,y1) - (x2,y2) with a start thickness of t1
void DrawThickLine(double x1, double y1, double x2, double y2, double t1)
{
      double angle = atan2(y2 - y1, x2 - x1);
      double t2sina1 = t1 / 2 * sin(angle);
      double t2cosa1 = t1 / 2 * cos(angle);

      glBegin(GL_TRIANGLES);
      glVertex2f(x1 + t2sina1, y1 - t2cosa1);
      glVertex2f(x2 + t2sina1, y2 - t2cosa1);
      glVertex2f(x2 - t2sina1, y2 + t2cosa1);
      glVertex2f(x2 - t2sina1, y2 + t2cosa1);
      glVertex2f(x1 - t2sina1, y1 + t2cosa1);
      glVertex2f(x1 + t2sina1, y1 - t2cosa1);
      glEnd();
}

void ocpnDC::DrawLine( wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2)
{
     if(dc)
          dc->DrawLine ( x1, y1, x2, y2 );
     else if(ConfigurePen()) {

           glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_HINT_BIT);      //Save state

          //      Enable anti-aliased lines, at best quality
           glEnable(GL_LINE_SMOOTH);
           glEnable(GL_BLEND);
           glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
           glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

           if(m_pen.GetWidth() > 1)
                  DrawThickLine(x1, y1, x2, y2, m_pen.GetWidth());
           else
           {
                  glBegin(GL_LINES);
                  glVertex2i(x1, y1);
                  glVertex2i(x2, y2);
                  glEnd();
           }
           glPopAttrib();
     }
}


void ocpnDC::DrawLines( int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset)
{
     if(dc)
          dc->DrawLines(n, points, xoffset, yoffset);
     else if(ConfigurePen()) {

           if(m_pen.GetWidth() > 1)
           {
              wxPoint p0 = points[0];
              for(int i=1; i<n; i++)
              {
                DrawThickLine(p0.x + xoffset, p0.y + yoffset, points[i].x + xoffset, points[i].y + yoffset, m_pen.GetWidth());
                p0 = points[i];
              }
           }
           else
           {
                 glBegin(GL_LINE_STRIP);
                 for(int i=0; i<n; i++)
                       glVertex2i(points[i].x + xoffset, points[i].y + yoffset);
                 glEnd();
           }
     }
}

void ocpnDC::StrokeLine( wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2)
{
#if wxUSE_GRAPHICS_CONTEXT
     if(pgc) {
          pgc->SetPen(dc->GetPen());
          pgc->StrokeLine( x1, y1, x2, y2);

          dc->CalcBoundingBox(x1, y1);
          dc->CalcBoundingBox(x2, y2);
     } else
#endif
          DrawLine ( x1, y1, x2, y2);
}

void ocpnDC::DrawRectangle( wxCoord x, wxCoord y, wxCoord w, wxCoord h )
{
     if(dc)
          dc->DrawRectangle(x, y, w, h);
     else {
          if(ConfigureBrush()) {
               glBegin(GL_QUADS);
               glVertex2i(x, y);
               glVertex2i(x+w, y);
               glVertex2i(x+w, y+h);
               glVertex2i(x, y+h);
               glEnd();
          }

          if(ConfigurePen()) {
               glBegin(GL_LINE_LOOP);
               glVertex2i(x, y);
               glVertex2i(x+w, y);
               glVertex2i(x+w, y+h);
               glVertex2i(x, y+h);
               glEnd();
          }
     }
}

/* draw the arc along corners */
static void drawrrhelper( wxCoord x, wxCoord y, wxCoord r, double st, double et )
{
     const int slices = 10;
     double dt = (et - st) / slices;
     for(double t = st; t <= et; t += dt)
          glVertex2i(x + r*cos(t), y + r*sin(t));
}

void ocpnDC::DrawRoundedRectangle( wxCoord x, wxCoord y, wxCoord w, wxCoord h, wxCoord r )
{
     if(dc)
          dc->DrawRoundedRectangle(x, y, w, h, r);
     else {
          wxCoord x0 = x, x1 = x + r, x2 = x + w - r, x3 = x+h;
          wxCoord y0 = y, y1 = y + r, y2 = y + h - r, y3 = y+h;
          if(ConfigureBrush()) {
               glBegin(GL_QUADS);
               glVertex2i(x0, y1); glVertex2i(x1, y1);
               glVertex2i(x1, y2); glVertex2i(x0, y2);

               glVertex2i(x1, y0); glVertex2i(x2, y0);
               glVertex2i(x2, y3); glVertex2i(x0, y3);

               glVertex2i(x2, y1); glVertex2i(x3, y1);
               glVertex2i(x3, y2); glVertex2i(x2, y2);
               glEnd();

               glBegin(GL_TRIANGLE_FAN);
               glVertex2i(x1, y2);
               drawrrhelper(x1, y2, r, -M_PI, -M_PI/2);
               glEnd();

               glBegin(GL_TRIANGLE_FAN);
               glVertex2i(x2, y2);
               drawrrhelper(x2, y2, r, -M_PI/2, 0);
               glEnd();

               glBegin(GL_TRIANGLE_FAN);
               glVertex2i(x2, y1);
               drawrrhelper(x2, y1, r, 0, M_PI/2);
               glEnd();

               glBegin(GL_TRIANGLE_FAN);
               glVertex2i(x1, y1);
               drawrrhelper(x1, y1, r, M_PI/2, M_PI);
               glEnd();
          }

          if(ConfigurePen()) {
               glBegin(GL_LINE_LOOP);
               drawrrhelper(x1, y2, r, -M_PI, -M_PI/2);
               drawrrhelper(x2, y2, r, -M_PI/2, 0);
               drawrrhelper(x2, y1, r, 0, M_PI/2);
               drawrrhelper(x1, y1, r, M_PI/2, M_PI);
               glEnd();
          }
     }
}

void ocpnDC::DrawCircle(wxCoord x, wxCoord y, wxCoord radius)
{
     DrawEllipse(x-radius, y-radius, 2*radius, 2*radius);
}

void ocpnDC::StrokeCircle(wxCoord x, wxCoord y, wxCoord radius)
{
#if wxUSE_GRAPHICS_CONTEXT
     if(pgc) {
          wxGraphicsPath gpath = pgc->CreatePath();
          gpath.AddCircle(x, y, radius);

          pgc->SetPen(GetPen());
          pgc->SetBrush(GetBrush());
          pgc->DrawPath(gpath);

          // keep dc dirty box up-to-date
          dc->CalcBoundingBox(x+radius+2, y+radius+2);
          dc->CalcBoundingBox(x-radius-2, y-radius-2);
     } else
#endif
          DrawCircle(x, y, radius);
}

void ocpnDC::DrawEllipse(wxCoord x, wxCoord y, wxCoord width, wxCoord height)
{
     if(dc)
          dc->DrawEllipse(x, y, width, height);
     else {
          double r1 = width/2, r2 = height/2;
          double cx = x + r1, cy = y + r2;

          glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_HINT_BIT);      //Save state

          //      Enable anti-aliased lines, at best quality
          glEnable(GL_LINE_SMOOTH);
          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

          if(ConfigureBrush()) {
               glBegin(GL_TRIANGLE_FAN);
               glVertex2d(cx, cy);
               for(double a = 0; a <= 2*M_PI; a+=2*M_PI/20)
                    glVertex2d(cx + r1*sin(a), cy + r2*cos(a));
               glEnd();
          }

          if(ConfigurePen()) {
               glBegin(GL_LINE_STRIP);
               for(double a = 0; a <= 2*M_PI; a+=2*M_PI/200)
                    glVertex2d(cx + r1*sin(a), cy + r2*cos(a));
               glEnd();
          }

          glPopAttrib();            // restore state
     }
}

void ocpnDC::DrawPolygon(int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset)
{
      if(dc)
            dc->DrawPolygon(n, points, xoffset, yoffset);
      else
      {
            glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_HINT_BIT);      //Save state

            //      Enable anti-aliased lines, at best quality
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

            if(ConfigureBrush()) {
                  glBegin(GL_POLYGON);
                  for(int i=0; i<n; i++)
                        glVertex2i(points[i].x + xoffset, points[i].y + yoffset);
                  glEnd();
            }

            if(ConfigurePen()) {
                  glBegin(GL_LINE_LOOP);
                  for(int i=0; i<n; i++)
                        glVertex2i(points[i].x + xoffset, points[i].y + yoffset);
                  glEnd();
            }
            glPopAttrib();
      }
}

void ocpnDC::StrokePolygon(int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset)
{
#if wxUSE_GRAPHICS_CONTEXT
     if(pgc) {
          wxGraphicsPath gpath = pgc->CreatePath();
          gpath.MoveToPoint(points[0].x+xoffset,points[0].y+yoffset);
          for(int i=1; i<n; i++)
               gpath.AddLineToPoint(points[i].x+xoffset,points[i].y+yoffset);
          gpath.AddLineToPoint(points[0].x+xoffset,points[0].y+yoffset);

          pgc->SetPen(GetPen());
          pgc->SetBrush(GetBrush());
          pgc->DrawPath(gpath);

          for(int i=0; i<n; i++)
               dc->CalcBoundingBox(points[i].x+xoffset,points[i].y+yoffset);
     } else
#endif
          DrawPolygon(n, points, xoffset, yoffset);
}

void ocpnDC::DrawBitmap(const wxBitmap &bitmap, wxCoord x, wxCoord y, bool usemask)
{
     if(dc)
          dc->DrawBitmap(bitmap, x, y, usemask);
     else {
          wxImage image = bitmap.ConvertToImage();
          int w = image.GetWidth(), h = image.GetHeight();

          if(usemask) {
               unsigned char *d = image.GetData();
               unsigned char *a = image.GetAlpha();

               unsigned char mr, mg, mb;
               if(!image.GetOrFindMaskColour(&mr, &mg, &mb) && !a)
                    printf("trying to use mask to draw a bitmap without alpha or mask\n");

               unsigned char *e = new unsigned char[4*w*h];
//               int w = image.GetWidth(), h = image.GetHeight();
               {
                    for(int y=0; y<h; y++)
                         for(int x=0; x<w; x++) {
                              unsigned char r, g, b;
                              int off = (y*image.GetWidth()+x);
                              r = d[off*3 + 0];
                              g = d[off*3 + 1];
                              b = d[off*3 + 2];

                              e[off*4 + 0] = r;
                              e[off*4 + 1] = g;
                              e[off*4 + 2] = b;

                              e[off*4 + 3] = a ? a[off] :
                                   ((r==mr)&&(g==mg)&&(b==mb) ? 0 : 255);
                         }
               }

               glColor4f(1, 1, 1, 1);
               GLDrawBlendData(x, y, w, h, GL_RGBA, e);
               free(e);
          } else {
               glRasterPos2i(x, y);
               glPixelZoom(1, -1); /* draw data from top to bottom */
               glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, image.GetData());
               glPixelZoom(1, 1);
          }
     }
}

void ocpnDC::DrawText(const wxString &text, wxCoord x, wxCoord y)
{
     if(dc)
          dc->DrawText(text, x, y);
     else {
          wxCoord w, h;
          GetTextExtent(text, &w, &h);

          if(w && h)
          {
            /* create bitmap of appropriate size and select it */
            wxBitmap bmp(w, h);
            wxMemoryDC temp_dc;
            temp_dc.SelectObject(bmp);

            /* fill bitmap with black */
            temp_dc.SetBackground(wxBrush(wxColour(0, 0, 0)));
            temp_dc.Clear();

            /* draw the text white */
            temp_dc.SetFont(m_font);
            temp_dc.SetTextForeground(wxColour(255, 255, 255));
            temp_dc.DrawText( text, 0, 0);
            temp_dc.SelectObject ( wxNullBitmap );

            /* use the data in the bitmap for alpha channel,
                  and set the color to text foreground */
            wxImage image = bmp.ConvertToImage();
            unsigned char *data = new unsigned char[image.GetWidth()*image.GetHeight()];
            unsigned char *im = image.GetData();
            for(int i=0; i<w*h; i++)
                  data[i] = im[3*i];

            glColor4ub(m_textforegroundcolour.Red(),
                        m_textforegroundcolour.Green(),
                        m_textforegroundcolour.Blue(),
                        255);
            GLDrawBlendData(x, y, w, h, GL_ALPHA, data);
            free(data);
          }
     }
}

void ocpnDC::GetTextExtent(const wxString &string, wxCoord *w, wxCoord *h, wxCoord *descent,
                           wxCoord *externalLeading, wxFont *font) const
{
     if(dc)
          dc->GetTextExtent(string, w, h, descent, externalLeading, font);
     else {
          wxFont f = m_font;
          if(font)
               f = *font;

          wxMemoryDC temp_dc;
          temp_dc.GetTextExtent(string, w, h, descent, externalLeading, &f);
     }
}

void ocpnDC::ResetBoundingBox()
{
     if(dc)
          dc->ResetBoundingBox();
}

void ocpnDC::CalcBoundingBox(wxCoord x, wxCoord y)
{
     if(dc)
          dc->CalcBoundingBox(x, y);
}

bool ocpnDC::ConfigurePen()
{
     if(m_pen == wxNullPen)
          return false;

     wxColour c = m_pen.GetColour();
     int width = m_pen.GetWidth();
     glColor4ub(c.Red(), c.Green(), c.Blue(), c.Alpha());

     glLineWidth(width);
     return true;
}

bool ocpnDC::ConfigureBrush()
{
     if(m_brush == wxNullBrush || m_brush.GetStyle() == wxTRANSPARENT)
          return false;

     wxColour c = m_brush.GetColour();
     glColor4ub(c.Red(), c.Green(), c.Blue(), c.Alpha());
     return true;
}

void  ocpnDC::GLDrawBlendData(wxCoord x, wxCoord y, wxCoord w, wxCoord h,
                              int format, const unsigned char *data)
{
      /*  Hmmmm.... I find that the texture version below does not work on my dodgy OpenChrome gl drivers
          but the glDrawPixels works fine.*/

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glRasterPos2i(x, y);
      glPixelZoom(1, -1);
      glDrawPixels(w, h, format, GL_UNSIGNED_BYTE, data);
      glPixelZoom(1, 1);
      glDisable(GL_BLEND);

      return;


     /* I would prefer to just use glDrawPixels than need a texture,
      but sometimes it did not perform alpha blending correctly,
      this way always works */

#if 0
     glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex);
     glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, format, w, h, 0, format,
                  GL_UNSIGNED_BYTE, data);

     glEnable(GL_TEXTURE_RECTANGLE_ARB);
     glEnable(GL_BLEND);
     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
     glBegin(GL_QUADS);
     glTexCoord2f(0, 0); glVertex2f(x,   y);
     glTexCoord2f(w, 0); glVertex2f(x+w, y);
     glTexCoord2f(w, h); glVertex2f(x+w, y+h);
     glTexCoord2f(0, h); glVertex2f(x,   y+h);
     glEnd();
     glDisable(GL_BLEND);
     glDisable(GL_TEXTURE_RECTANGLE_ARB);

#endif
}
