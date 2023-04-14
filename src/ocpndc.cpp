/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Layer to perform wxDC drawing using wxDC or opengl
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2011 by Sean D'Epagnier                                 *
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
 */

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "config.h"

#include "dychart.h"
#include "ocpn_plugin.h"
#include "chcanv.h"
#include "linmath.h"
#include "ocpn_frame.h"

#ifdef __MSVC__
#include <windows.h>
#endif

#ifdef ocpnUSE_GL
#include "shaders.h"
#endif

#include <wx/graphics.h>
#include <wx/dcclient.h>

#include <vector>

#include "ocpndc.h"
#include "wx28compat.h"
#include "cutil.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
extern ocpnGLOptions g_GLOptions;
#endif

extern float g_GLMinSymbolLineWidth;
wxArrayPtrVoid gTesselatorVertices;

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
extern GLint color_tri_shader_program;
extern GLint circle_filled_shader_program;
extern GLint texture_2D_shader_program;
#endif

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#ifndef PI
#define PI 3.1415926535897931160E0 /* pi */
#endif

//----------------------------------------------------------------------------
ocpnDC::ocpnDC(glChartCanvas &canvas)
    : m_glchartCanvas(&canvas), m_glcanvas(NULL), dc(NULL), m_pen(wxNullPen), m_brush(wxNullBrush) {
#if wxUSE_GRAPHICS_CONTEXT
  pgc = NULL;
#endif
  Init();
  m_canvasIndex = m_glchartCanvas->GetCanvasIndex();
}

ocpnDC::ocpnDC(wxGLCanvas &canvas)
    : m_glchartCanvas(NULL), m_glcanvas(&canvas), dc(NULL), m_pen(wxNullPen), m_brush(wxNullBrush) {
#if wxUSE_GRAPHICS_CONTEXT
  pgc = NULL;
#endif
  Init();
}

ocpnDC::ocpnDC(wxDC &pdc)
    : m_glchartCanvas(NULL), m_glcanvas(NULL), dc(&pdc), m_pen(wxNullPen), m_brush(wxNullBrush) {
#if wxUSE_GRAPHICS_CONTEXT
  pgc = NULL;
  wxMemoryDC *pmdc = wxDynamicCast(dc, wxMemoryDC);
  if (pmdc)
    pgc = wxGraphicsContext::Create(*pmdc);
  else {
    wxClientDC *pcdc = wxDynamicCast(dc, wxClientDC);
    if (pcdc) pgc = wxGraphicsContext::Create(*pcdc);
  }
#endif

  Init();
}

ocpnDC::ocpnDC()
    : m_glchartCanvas(NULL), m_glcanvas(NULL), dc(NULL), m_pen(wxNullPen), m_brush(wxNullBrush) {
#if wxUSE_GRAPHICS_CONTEXT
  pgc = NULL;
#endif
  Init();
}

ocpnDC::~ocpnDC() {
#if wxUSE_GRAPHICS_CONTEXT
  if (pgc) delete pgc;
#endif
  free(workBuf);

  free(s_odc_tess_work_buf);

  delete m_pcolor_tri_shader_program;
  delete m_pAALine_shader_program;
  delete m_pcircle_filled_shader_program;
  delete m_ptexture_2D_shader_program;
}

void ocpnDC::Init(){
  m_buseTex = GetLocaleCanonicalName().IsSameAs(_T("en_US"));
  workBuf = NULL;
  workBufSize = 0;
  s_odc_tess_work_buf = NULL;
  m_dpi_factor = 1.0;
  m_canvasIndex = 0;
  m_textforegroundcolour = wxColour(0, 0, 0);

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
  s_odc_tess_vertex_idx = 0;
  s_odc_tess_vertex_idx_this = 0;
  s_odc_tess_buf_len = 0;

  s_odc_tess_work_buf = (GLfloat *)malloc(100 * sizeof(GLfloat));
  s_odc_tess_buf_len = 100;

  m_pcolor_tri_shader_program = NULL;
  m_pAALine_shader_program = NULL;
  m_pcircle_filled_shader_program = NULL;
  m_ptexture_2D_shader_program = NULL;
#endif

}

void ocpnDC::SetGLCanvas(glChartCanvas *canvas) {
  m_glchartCanvas = canvas;
  m_canvasIndex = m_glchartCanvas->GetCanvasIndex();
}

void ocpnDC::Clear() {
  if (dc)
    dc->Clear();
  else {
#ifdef ocpnUSE_GL
    wxBrush tmpBrush = m_brush;
    int w, h;
    if (m_glchartCanvas) {
      SetBrush(wxBrush(m_glchartCanvas->GetBackgroundColour()));
      m_glchartCanvas->GetSize(&w, &h);
    }
    else if (m_glcanvas) {
      SetBrush(wxBrush(m_glcanvas->GetBackgroundColour()));
      m_glcanvas->GetSize(&w, &h);
    }
    else
      return;

    DrawRectangle(0, 0, w, h);
    SetBrush(tmpBrush);
#endif
  }
}

void ocpnDC::SetBackground(const wxBrush &brush) {
  if (dc)
    dc->SetBackground(brush);
  else {
#ifdef ocpnUSE_GL
    if (m_glchartCanvas)
      m_glchartCanvas->SetBackgroundColour(brush.GetColour());
    else if (m_glcanvas)
      m_glcanvas->SetBackgroundColour(brush.GetColour());
    else
      return;
#endif
  }
}

void ocpnDC::SetPen(const wxPen &pen) {
  if (dc) {
    if (pen == wxNullPen)
      dc->SetPen(*wxTRANSPARENT_PEN);
    else
      dc->SetPen(pen);
  } else
    m_pen = pen;
}

void ocpnDC::SetBrush(const wxBrush &brush) {
  if (dc)
    dc->SetBrush(brush);
  else
    m_brush = brush;
}

void ocpnDC::SetTextForeground(const wxColour &colour) {
  if (dc)
    dc->SetTextForeground(colour);
  else
    m_textforegroundcolour = colour;
}

void ocpnDC::SetFont(const wxFont &font) {
  if (dc)
    dc->SetFont(font);
  else
    m_font = font;
}

const wxPen &ocpnDC::GetPen() const {
  if (dc) return dc->GetPen();
  return m_pen;
}

const wxBrush &ocpnDC::GetBrush() const {
  if (dc) return dc->GetBrush();
  return m_brush;
}

const wxFont &ocpnDC::GetFont() const {
  if (dc) return dc->GetFont();
  return m_font;
}

void ocpnDC::GetSize(wxCoord *width, wxCoord *height) const {
  if (dc)
    dc->GetSize(width, height);
  else {
#ifdef ocpnUSE_GL
    if (m_glchartCanvas){
      *width = m_glchartCanvas->GetGLCanvasWidth();
      *height = m_glchartCanvas->GetGLCanvasHeight();
    }
    else if (m_glcanvas){
      m_glcanvas->GetSize(width, height);
    }
#endif
  }
}

void ocpnDC::SetGLAttrs(bool highQuality) {
#ifdef ocpnUSE_GL

  // Enable anti-aliased polys, at best quality
  if (highQuality) {
    if (g_GLOptions.m_GLLineSmoothing) glEnable(GL_LINE_SMOOTH);
    if (g_GLOptions.m_GLPolygonSmoothing) glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
  } else {
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_BLEND);
  }
#endif
}

void ocpnDC::SetGLStipple() const {
#ifdef ocpnUSE_GL

#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
  switch (m_pen.GetStyle()) {
    case wxPENSTYLE_DOT: {
      glLineStipple(1, 0x3333);
      glEnable(GL_LINE_STIPPLE);
      break;
    }
    case wxPENSTYLE_LONG_DASH: {
      glLineStipple(1, 0xFFF8);
      glEnable(GL_LINE_STIPPLE);
      break;
    }
    case wxPENSTYLE_SHORT_DASH: {
      glLineStipple(1, 0x3F3F);
      glEnable(GL_LINE_STIPPLE);
      break;
    }
    case wxPENSTYLE_DOT_DASH: {
      glLineStipple(1, 0x8FF1);
      glEnable(GL_LINE_STIPPLE);
      break;
    }
    default:
      break;
  }
#endif
#endif
}

#ifdef ocpnUSE_GL
/* draw a half circle using triangles */
void DrawEndCap(float x1, float y1, float t1, float angle) {
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
  const int steps = 16;
  float xa, ya;
  bool first = true;
  for (int i = 0; i <= steps; i++) {
    float a = angle + M_PI / 2 + M_PI / steps * i;

    float xb = x1 + t1 / 2 * cos(a);
    float yb = y1 + t1 / 2 * sin(a);
    if (first)
      first = false;
    else {
      glVertex2f(x1, y1);
      glVertex2f(xa, ya);
      glVertex2f(xb, yb);
    }
    xa = xb, ya = yb;
  }
#endif
}
#endif

// Draws a line between (x1,y1) - (x2,y2) with a start thickness of t1
void ocpnDC::DrawGLThickLine(float x1, float y1, float x2, float y2, wxPen pen,
                     bool b_hiqual) {
#ifdef ocpnUSE_GL

  float angle = atan2f(y2 - y1, x2 - x1);
  float t1 = pen.GetWidth();
  float t2sina1 = t1 / 2 * sinf(angle);
  float t2cosa1 = t1 / 2 * cosf(angle);

  // Set up the shader
  GLShaderProgram *shader;
  if (m_glchartCanvas){
    shader = pcolor_tri_shader_program[m_canvasIndex];
    shader->Bind();
    // Assuming here that transform matrix for this shader is preset for canvas.
    //shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform);
  }
  else{
    shader = m_pcolor_tri_shader_program;
    shader->Bind();
    shader->SetUniformMatrix4fv("MVMatrix", (float *)&(m_vp.vp_matrix_transform));
  }

    wxColor c = pen.GetColour();
    float colorv[4];
    colorv[0] = c.Red() / float(256);
    colorv[1] = c.Green() / float(256);
    colorv[2] = c.Blue() / float(256);
    colorv[3] = c.Alpha() / float(256);
    shader->SetUniform4fv("color", colorv);

    float vert[12];
    shader->SetAttributePointerf("position", vert);


  //    n.b.  The dwxDash interpretation for GL only allows for 2 elements in
  //    the dash table. The first is assumed drawn, second is assumed space
  wxDash *dashes;
  int n_dashes = pen.GetDashes(&dashes);
  if (n_dashes) {
    float lpix = sqrtf(powf((float)(x1 - x2), 2) + powf((float)(y1 - y2), 2));
    float lrun = 0.;
    float xa = x1;
    float ya = y1;
    float ldraw = t1 * dashes[0];
    float lspace = t1 * dashes[1];

    while (lrun < lpix) {
      //    Dash
      float xb = xa + ldraw * cosf(angle);
      float yb = ya + ldraw * sinf(angle);

      if ((lrun + ldraw) >= lpix)  // last segment is partial draw
      {
        xb = x2;
        yb = y2;
      }

      vert[0] = xa + t2sina1;
      vert[1] = ya - t2cosa1;
      vert[2] = xb + t2sina1;
      vert[3] = yb - t2cosa1;
      vert[4] = xb - t2sina1;
      vert[5] = yb + t2cosa1;
      vert[6] = xb - t2sina1;
      vert[7] = yb + t2cosa1;
      vert[8] = xa - t2sina1;
      vert[9] = ya + t2cosa1;
      vert[10] = xa + t2sina1;
      vert[11] = ya - t2cosa1;

      glDrawArrays(GL_TRIANGLES, 0, 6);

      xa = xb;
      ya = yb;
      lrun += ldraw;

      //    Space
      xb = xa + lspace * cos(angle);
      yb = ya + lspace * sin(angle);

      xa = xb;
      ya = yb;
      lrun += lspace;
    }

  } else {
    vert[0] = x1 + t2sina1;
    vert[1] = y1 - t2cosa1;
    vert[2] = x2 + t2sina1;
    vert[3] = y2 - t2cosa1;
    vert[4] = x2 - t2sina1;
    vert[5] = y2 + t2cosa1;
    vert[6] = x2 - t2sina1;
    vert[7] = y2 + t2cosa1;
    vert[8] = x1 - t2sina1;
    vert[9] = y1 + t2cosa1;
    vert[10] = x1 + t2sina1;
    vert[11] = y1 - t2cosa1;


    glDrawArrays(GL_TRIANGLES, 0, 6);

    /* wx draws a nice rounded end in dc mode, so replicate
     *           this for opengl mode, should this be done for the dashed mode
     * case? */
    //         if(pen.GetCap() == wxCAP_ROUND) {
    //             DrawEndCap( x1, y1, t1, angle);
    //             DrawEndCap( x2, y2, t1, angle + M_PI);
    //         }
    //
  }
  shader->UnBind();


#endif
}

void ocpnDC::DrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2,
                      bool b_hiqual) {
  if (dc) dc->DrawLine(x1, y1, x2, y2);
#ifdef ocpnUSE_GL
  else if (ConfigurePen()) {
    bool b_draw_thick = false;

    float pen_width = wxMax(g_GLMinSymbolLineWidth, m_pen.GetWidth());

    //      Enable anti-aliased lines, at best quality
    if (b_hiqual) {
      SetGLStipple();

#ifndef __WXQT__
      glEnable(GL_BLEND);
      if (g_GLOptions.m_GLLineSmoothing) glEnable(GL_LINE_SMOOTH);
#endif

      if (pen_width > 1.0) {
        GLint parms[2];
        glGetIntegerv(GL_SMOOTH_LINE_WIDTH_RANGE, &parms[0]);
        if (glGetError()) glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, &parms[0]);
        if (pen_width > parms[1])
          b_draw_thick = true;
        else
          glLineWidth(pen_width);
      } else
        glLineWidth(pen_width);
    } else {
      if (pen_width > 1) {
        GLint parms[2];
        glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, &parms[0]);
        if (pen_width > parms[1])
          b_draw_thick = true;
        else
          glLineWidth(pen_width);
      } else
        glLineWidth(pen_width);
    }

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
    if (b_draw_thick)
      DrawGLThickLine(x1, y1, x2, y2, m_pen, b_hiqual);
    else {
#if 0       // Use AA lines

      GLShaderProgram *shader = pAALine_shader_program[m_canvasIndex];
      shader->Bind();

      // Assuming here that transform matrix for this shader is preset for canvas.
      shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform);

      float vpx[2];
      int width = 0;
      int height = 0;
      GetSize(&width, &height);
      vpx[0] = width;
      vpx[1] = height;

      shader->SetUniform2fv("uViewPort", vpx);

      float colorv[4];
      colorv[0] = m_pen.GetColour().Red() / float(256);
      colorv[1] = m_pen.GetColour().Green() / float(256);
      colorv[2] = m_pen.GetColour().Blue() / float(256);
      colorv[3] = 1.0;

      shader->SetUniform4fv("color", colorv);

      float fBuf[4];
      shader->SetAttributePointerf("position", fBuf);


      wxDash *dashes;
      int n_dashes = m_pen.GetDashes(&dashes);
      if (n_dashes) {
        float angle = atan2f((float)(y2 - y1), (float)(x2 - x1));
        float cosa = cosf(angle);
        float sina = sinf(angle);
        float t1 = m_pen.GetWidth();

        float lpix = sqrtf(powf(x1 - x2, 2) + powf(y1 - y2, 2));
        float lrun = 0.;
        float xa = x1;
        float ya = y1;
        float ldraw = t1 * dashes[0];
        float lspace = t1 * dashes[1];

        ldraw = wxMax(ldraw, 4.0);
        lspace = wxMax(lspace, 4.0);
        lpix = wxMin(lpix, 2000.0);

        while (lrun < lpix) {
          //    Dash
          float xb = xa + ldraw * cosa;
          float yb = ya + ldraw * sina;

          if ((lrun + ldraw) >= lpix)  // last segment is partial draw
          {
            xb = x2;
            yb = y2;
          }

          fBuf[0] = xa;
          fBuf[1] = ya;
          fBuf[2] = xb;
          fBuf[3] = yb;

          glDrawArrays(GL_LINES, 0, 2);

          xa = xa + (lspace + ldraw) * cosa;
          ya = ya + (lspace + ldraw) * sina;
          lrun += lspace + ldraw;
        }
      } else  // not dashed
      {
        fBuf[0] = x1;
        fBuf[1] = y1;
        fBuf[2] = x2;
        fBuf[3] = y2;

        glDrawArrays(GL_LINES, 0, 2);
      }
      shader->UnBind();
#else

      GLShaderProgram *shader;
      if (m_glchartCanvas){
        shader = pcolor_tri_shader_program[m_canvasIndex];
        shader->Bind();
        // Assuming here that transform matrix for this shader is preset for canvas.
        //shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform);
      }
      else{
        shader = m_pcolor_tri_shader_program;
        shader->Bind();
        shader->SetUniformMatrix4fv("MVMatrix", (float *)&(m_vp.vp_matrix_transform));
      }

      float colorv[4];
      colorv[0] = m_pen.GetColour().Red() / float(256);
      colorv[1] = m_pen.GetColour().Green() / float(256);
      colorv[2] = m_pen.GetColour().Blue() / float(256);
      colorv[3] = 1.0;

      shader->SetUniform4fv("color", colorv);

      glLineWidth(wxMax(g_GLMinSymbolLineWidth, m_pen.GetWidth()));

      float fBuf[4];
      shader->SetAttributePointerf("position", fBuf);


      wxDash *dashes;
      int n_dashes = m_pen.GetDashes(&dashes);
      if (n_dashes) {
        float angle = atan2f((float)(y2 - y1), (float)(x2 - x1));
        float cosa = cosf(angle);
        float sina = sinf(angle);
        float t1 = m_pen.GetWidth();

        float lpix = sqrtf(powf(x1 - x2, 2) + powf(y1 - y2, 2));
        float lrun = 0.;
        float xa = x1;
        float ya = y1;
        float ldraw = t1 * dashes[0];
        float lspace = t1 * dashes[1];

        ldraw = wxMax(ldraw, 4.0);
        lspace = wxMax(lspace, 4.0);
        lpix = wxMin(lpix, 2000.0);

        while (lrun < lpix) {
          //    Dash
          float xb = xa + ldraw * cosa;
          float yb = ya + ldraw * sina;

          if ((lrun + ldraw) >= lpix)  // last segment is partial draw
          {
            xb = x2;
            yb = y2;
          }

          fBuf[0] = xa;
          fBuf[1] = ya;
          fBuf[2] = xb;
          fBuf[3] = yb;

          glDrawArrays(GL_LINES, 0, 2);

          xa = xa + (lspace + ldraw) * cosa;
          ya = ya + (lspace + ldraw) * sina;
          lrun += lspace + ldraw;
        }
      } else  // not dashed
      {
        fBuf[0] = x1;
        fBuf[1] = y1;
        fBuf[2] = x2;
        fBuf[3] = y2;

        glDrawArrays(GL_LINES, 0, 2);
      }
      shader->UnBind();

#endif

    }

#else
#endif
    glDisable(GL_LINE_STIPPLE);

    if (b_hiqual) {
      glDisable(GL_LINE_SMOOTH);
      glDisable(GL_BLEND);
    }
  }
#endif
}


// Draws thick lines from triangles
void ocpnDC::DrawGLThickLines(int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset,
                      wxPen pen, bool b_hiqual) {
#ifdef ocpnUSE_GL
  if (n < 2) return;

  wxPoint p0 = points[0];
  for (int i = 1; i < n; i++) {
    DrawGLThickLine(p0.x + xoffset, p0.y + yoffset, points[i].x + xoffset,
                    points[i].y + yoffset, pen, b_hiqual);
    p0 = points[i];
  }
  return;
#endif
}

void ocpnDC::DrawLines(int n, wxPoint points[], wxCoord xoffset,
                       wxCoord yoffset, bool b_hiqual) {
  if (dc) dc->DrawLines(n, points, xoffset, yoffset);
#ifdef ocpnUSE_GL
  else if (ConfigurePen()) {
#ifdef __WXQT__
    SetGLAttrs(false);  // Some QT platforms (Android) have trouble with
                        // GL_BLEND / GL_LINE_SMOOTH
#else
    SetGLAttrs(b_hiqual);
#endif
    bool b_draw_thick = false;

    glDisable(GL_LINE_STIPPLE);
    SetGLStipple();

    //      Enable anti-aliased lines, at best quality
    if (b_hiqual) {
      glEnable(GL_BLEND);
      if (m_pen.GetWidth() > 1) {
        GLint parms[2];
        glGetIntegerv(GL_SMOOTH_LINE_WIDTH_RANGE, &parms[0]);
        if (glGetError()) glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, &parms[0]);

        if (m_pen.GetWidth() > parms[1])
          b_draw_thick = true;
        else
          glLineWidth(wxMax(g_GLMinSymbolLineWidth, m_pen.GetWidth()));
      } else
        glLineWidth(wxMax(g_GLMinSymbolLineWidth, 1));
    } else {
      if (m_pen.GetWidth() > 1) {
        GLint parms[2];
        glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, &parms[0]);
        if (m_pen.GetWidth() > parms[1])
          b_draw_thick = true;
        else
          glLineWidth(wxMax(g_GLMinSymbolLineWidth, m_pen.GetWidth()));
      } else
        glLineWidth(wxMax(g_GLMinSymbolLineWidth, 1));
    }

    if (b_draw_thick) {
      DrawGLThickLines(n, points, xoffset, yoffset, m_pen, b_hiqual);

      if (b_hiqual) {
        glDisable(GL_LINE_STIPPLE);
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_POLYGON_SMOOTH);
        glDisable(GL_BLEND);
      }

      return;
    }

    //  Grow the work buffer as necessary
    if (workBufSize < (size_t)n * 2) {
      workBuf = (float *)realloc(workBuf, (n * 4) * sizeof(float));
      workBufSize = n * 4;
    }

    for (int i = 0; i < n; i++) {
      workBuf[i * 2] = points[i].x + xoffset;
      workBuf[(i * 2) + 1] = points[i].y + yoffset;
    }

#if 0     // Use AA lines
      GLShaderProgram *shader = pAALine_shader_program[m_canvasIndex];
      shader->Bind();

      // Assuming here that transform matrix for this shader is preset for canvas.
      //shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform);

      float vpx[2];
      int width = 0;
      int height = 0;
      GetSize(&width, &height);
      vpx[0] = width;
      vpx[1] = height;

      shader->SetUniform2fv("uViewPort", vpx);

      float colorv[4];
      colorv[0] = m_pen.GetColour().Red() / float(256);
      colorv[1] = m_pen.GetColour().Green() / float(256);
      colorv[2] = m_pen.GetColour().Blue() / float(256);
      colorv[3] = 1.0;

      shader->SetUniform4fv("color", colorv);

      shader->SetAttributePointerf("position", workBuf);

      glDrawArrays(GL_LINE_STRIP, 0, n);

      shader->UnBind();
#else
      GLShaderProgram *shader;
      if (m_glchartCanvas){
        shader = pcolor_tri_shader_program[m_canvasIndex];
        shader->Bind();
        // Assuming here that transform matrix for this shader is preset for canvas.
        //shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform);
      }
      else{
        shader = m_pcolor_tri_shader_program;
        shader->Bind();
        shader->SetUniformMatrix4fv("MVMatrix", (float *)&(m_vp.vp_matrix_transform));
      }


      float colorv[4];
      colorv[0] = m_pen.GetColour().Red() / float(256);
      colorv[1] = m_pen.GetColour().Green() / float(256);
      colorv[2] = m_pen.GetColour().Blue() / float(256);
      colorv[3] = 1.0;

      shader->SetUniform4fv("color", colorv);

      shader->SetAttributePointerf("position", workBuf);

      glDrawArrays(GL_LINE_STRIP, 0, n);

      shader->UnBind();

#endif


    if (b_hiqual) {
      glDisable(GL_LINE_STIPPLE);
      glDisable(GL_LINE_SMOOTH);
      glDisable(GL_POLYGON_SMOOTH);
      glDisable(GL_BLEND);
    }
  }
#endif
}

void ocpnDC::StrokeLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2) {
#if wxUSE_GRAPHICS_CONTEXT
  if (pgc) {
    pgc->SetPen(dc->GetPen());
    pgc->StrokeLine(x1, y1, x2, y2);

    dc->CalcBoundingBox(x1, y1);
    dc->CalcBoundingBox(x2, y2);
  } else
#endif
    DrawLine(x1, y1, x2, y2, true);
}

void ocpnDC::StrokeLines(int n, wxPoint *points) {
  if (n < 2) /* optimization and also to avoid assertion in pgc->StrokeLines */
    return;

#if wxUSE_GRAPHICS_CONTEXT
  if (pgc) {
    wxPoint2DDouble *dPoints =
        (wxPoint2DDouble *)malloc(n * sizeof(wxPoint2DDouble));
    for (int i = 0; i < n; i++) {
      dPoints[i].m_x = points[i].x;
      dPoints[i].m_y = points[i].y;
    }
    pgc->SetPen(dc->GetPen());
    pgc->StrokeLines(n, dPoints);
    free(dPoints);
  } else
#endif
    DrawLines(n, points, 0, 0, true);
}

void ocpnDC::DrawRectangle(wxCoord x, wxCoord y, wxCoord w, wxCoord h) {
  if (dc) dc->DrawRectangle(x, y, w, h);
#ifdef ocpnUSE_GL
  else {
    DrawRoundedRectangle(x, y, w, h, 0);
  }
#endif
}

/* draw the arc along corners */
static void drawrrhelper(wxCoord x0, wxCoord y0, wxCoord r, int quadrant,
                         int steps) {
#ifdef ocpnUSE_GL
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
  float step = 1.0 / steps, rs = 2.0 * r * step, rss = rs * step, x, y, dx, dy,
        ddx, ddy;
  switch (quadrant) {
    case 0:
      x = r, y = 0, dx = 0, dy = -rs, ddx = -rss, ddy = rss;
      break;
    case 1:
      x = 0, y = -r, dx = -rs, dy = 0, ddx = rss, ddy = rss;
      break;
    case 2:
      x = -r, y = 0, dx = 0, dy = rs, ddx = rss, ddy = -rss;
      break;
    case 3:
      x = 0, y = r, dx = rs, dy = 0, ddx = -rss, ddy = -rss;
      break;
    default:
      return;  // avoid unitialized compiler warnings
  }

  for (int i = 0; i < steps; i++) {
    glVertex2i(x0 + floor(x), y0 + floor(y));
    x += dx + ddx / 2, y += dy + ddy / 2;
    dx += ddx, dy += ddy;
  }
  glVertex2i(x0 + floor(x), y0 + floor(y));
#endif
#endif
}

void ocpnDC::drawrrhelperGLES2(wxCoord x0, wxCoord y0, wxCoord r, int quadrant,
                               int steps) {
#ifdef ocpnUSE_GL
  if (steps == 0) return;

  float step = 1.0 / steps, rs = 2.0 * r * step, rss = rs * step, x, y, dx, dy,
        ddx, ddy;
  switch (quadrant) {
    case 0:
      x = r, y = 0, dx = 0, dy = -rs, ddx = -rss, ddy = rss;
      break;
    case 1:
      x = 0, y = -r, dx = -rs, dy = 0, ddx = rss, ddy = rss;
      break;
    case 2:
      x = -r, y = 0, dx = 0, dy = rs, ddx = rss, ddy = -rss;
      break;
    case 3:
      x = 0, y = r, dx = rs, dy = 0, ddx = -rss, ddy = -rss;
      break;
    default:
      return;  // avoid unitialized compiler warnings
  }

  for (int i = 0; i < steps; i++) {
    workBuf[workBufIndex++] = x0 + floor(x);
    workBuf[workBufIndex++] = y0 + floor(y);

    x += dx + ddx / 2, y += dy + ddy / 2;
    dx += ddx, dy += ddy;
  }

  workBuf[workBufIndex++] = x0 + floor(x);
  workBuf[workBufIndex++] = y0 + floor(y);
#endif
}

void ocpnDC::DrawRoundedRectangle(wxCoord x, wxCoord y, wxCoord w, wxCoord h,
                                  wxCoord r) {
  if (dc) dc->DrawRoundedRectangle(x, y, w, h, r);
#ifdef ocpnUSE_GL
  else {
    if (!m_glchartCanvas)
      return;

    r++;
    int steps = ceil(sqrt((float)r));

    wxCoord x1 = x + r, x2 = x + w - r;
    wxCoord y1 = y + r, y2 = y + h - r;

    ConfigureBrush();
    ConfigurePen();

    //  Grow the work buffer as necessary
    size_t bufReq = (steps+1) * 8 * 2 * sizeof(float);  // large, to be sure

    if (workBufSize < bufReq) {
      workBuf = (float *)realloc(workBuf, bufReq);
      workBufSize = bufReq;
    }
    workBufIndex = 0;

    if (steps) {
      drawrrhelperGLES2(x2, y1, r, 0, steps);
      drawrrhelperGLES2(x1, y1, r, 1, steps);
      drawrrhelperGLES2(x1, y2, r, 2, steps);
      drawrrhelperGLES2(x2, y2, r, 3, steps);
    }

    GLShaderProgram *shader = pcolor_tri_shader_program[m_canvasIndex];
    shader->Bind();

    float fcolorv[4];
    fcolorv[0] = m_brush.GetColour().Red() / float(256);
    fcolorv[1] = m_brush.GetColour().Green() / float(256);
    fcolorv[2] = m_brush.GetColour().Blue() / float(256);
    fcolorv[3] = m_brush.GetColour().Alpha() / float(256);
    shader->SetUniform4fv("color", fcolorv);

    float angle = 0.;
    float xoffset = 0;
    float yoffset = 0;

    // Rotate
    mat4x4 I, Q;
    mat4x4_identity(I);
    mat4x4_rotate_Z(Q, I, angle);

    // Translate
    Q[3][0] = xoffset;
    Q[3][1] = yoffset;

    mat4x4 X;
    mat4x4_mul(
        X, (float(*)[4])m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform, Q);
    shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)X);


    shader->SetAttributePointerf("position", workBuf);

    // Perform the actual drawing.
    glDrawArrays(GL_TRIANGLE_FAN, 0, workBufIndex / 2);

    // Border color
    float bcolorv[4];
    bcolorv[0] = m_pen.GetColour().Red() / float(256);
    bcolorv[1] = m_pen.GetColour().Green() / float(256);
    bcolorv[2] = m_pen.GetColour().Blue() / float(256);
    bcolorv[3] = m_pen.GetColour().Alpha() / float(256);

    shader->SetUniform4fv("color", bcolorv);

    // Perform the actual drawing.
    glDrawArrays(GL_LINE_LOOP, 0, workBufIndex / 2);

    shader->UnBind();
  }
#endif
}

void ocpnDC::DrawCircle(wxCoord x, wxCoord y, wxCoord radius) {
  glEnable(GL_BLEND);

  float coords[8];
  coords[0] = x - radius;
  coords[1] = y + radius;
  coords[2] = x + radius;
  coords[3] = y + radius;
  coords[4] = x - radius;
  coords[5] = y - radius;
  coords[6] = x + radius;
  coords[7] = y - radius;

  GLShaderProgram *shader;
  if (m_glchartCanvas){
    shader = pcircle_filled_shader_program[m_canvasIndex];
    shader->Bind();
    // Assuming here that transform matrix for this shader is preset for canvas.
    //shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform);
  }
  else{
    shader = m_pcircle_filled_shader_program;
    shader->Bind();
    shader->SetUniformMatrix4fv("MVMatrix", (float *)&(m_vp.vp_matrix_transform));
  }

//   GLShaderProgram *shader = pcircle_filled_shader_program[m_canvasIndex];
//   shader->Bind();

  shader->SetUniform1f("circle_radius", radius);

  //  Circle center point
  float ctrv[2];
  ctrv[0] = x;
  int width, height;
  GetSize(&width, &height);
  ctrv[1] = height - y;
  shader->SetUniform2fv("circle_center", ctrv);

    //  Circle color
  float colorv[4];
  colorv[0] = m_brush.GetColour().Red() / float(256);
  colorv[1] = m_brush.GetColour().Green() / float(256);
  colorv[2] = m_brush.GetColour().Blue() / float(256);
  colorv[3] = (m_brush.GetStyle() == wxBRUSHSTYLE_TRANSPARENT) ? 0.0 : 1.0;

  shader->SetUniform4fv("circle_color", colorv);

  //  Border color
  float bcolorv[4];
  bcolorv[0] = m_pen.GetColour().Red() / float(256);
  bcolorv[1] = m_pen.GetColour().Green() / float(256);
  bcolorv[2] = m_pen.GetColour().Blue() / float(256);
  bcolorv[3] = m_pen.GetColour().Alpha() / float(256);

  shader->SetUniform4fv("border_color", bcolorv);

  //  Border Width
  shader->SetUniform1f("border_width", m_pen.GetWidth());

  shader->SetAttributePointerf("aPos", coords);

  // Perform the actual drawing.
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  shader->UnBind();
}

void ocpnDC::StrokeCircle(wxCoord x, wxCoord y, wxCoord radius) {
#if wxUSE_GRAPHICS_CONTEXT
  if (pgc) {
    wxGraphicsPath gpath = pgc->CreatePath();
    gpath.AddCircle(x, y, radius);

    pgc->SetPen(GetPen());
    pgc->SetBrush(GetBrush());
    pgc->DrawPath(gpath);

    // keep dc dirty box up-to-date
    dc->CalcBoundingBox(x + radius + 2, y + radius + 2);
    dc->CalcBoundingBox(x - radius - 2, y - radius - 2);
  } else
#endif
    DrawCircle(x, y, radius);
}

void ocpnDC::DrawEllipse(wxCoord x, wxCoord y, wxCoord width, wxCoord height) {
  if (dc) dc->DrawEllipse(x, y, width, height);
#ifdef ocpnUSE_GL
  else {
    float r1 = width / 2, r2 = height / 2;
    float cx = x + r1, cy = y + r2;

    //      Enable anti-aliased lines, at best quality
    glEnable(GL_BLEND);

    /* formula for variable step count to produce smooth ellipse */
    float steps = floorf(
        wxMax(sqrtf(sqrtf((float)(width * width + height * height))), 1) *
        M_PI);

    // FIXME (dave??)  Unimplemented for GLSL and GLES2
    glDisable(GL_BLEND);
  }
#endif
}

void ocpnDC::DrawPolygon(int n, wxPoint points[], wxCoord xoffset,
                         wxCoord yoffset, float scale, float angle) {
  if (dc) dc->DrawPolygon(n, points, xoffset, yoffset);
#ifdef ocpnUSE_GL
  else {

#ifdef __WXQT__
    SetGLAttrs(false);  // Some QT platforms (Android) have trouble with
                        // GL_BLEND / GL_LINE_SMOOTH
#else
    SetGLAttrs(true);
#endif
    ConfigurePen();

    // Prepare the line rendering shader
    GLShaderProgram *line_shader;
    float* mvmatrix;

    if(m_glchartCanvas){
      line_shader = pAALine_shader_program[m_canvasIndex];
      mvmatrix = (float *)&(m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform);
    }
    else{
      line_shader = m_pAALine_shader_program;
      mvmatrix = (float *)&(m_vp.vp_matrix_transform);
    }

    {
      //GLShaderProgram *shader = pAALine_shader_program[m_canvasIndex];
      line_shader->Bind();

      line_shader->SetUniform1f("uLineWidth", m_pen.GetWidth());
      line_shader->SetUniform1f("uBlendFactor", 2.0);

      float vpx[2];
      int width = 0;
      int height = 0;
      GetSize(&width, &height);
      vpx[0] = width;
      vpx[1] = height;

      line_shader->SetUniform2fv("uViewPort", vpx);

      float colorv[4];
      colorv[0] = m_pen.GetColour().Red() / float(256);
      colorv[1] = m_pen.GetColour().Green() / float(256);
      colorv[2] = m_pen.GetColour().Blue() / float(256);
      colorv[3] = 1.0;

      line_shader->SetUniform4fv("color", colorv);

      //shader->SetAttributePointerf("position", workBuf);

      // Rotate
      mat4x4 I, Q;
      mat4x4_identity(I);
      mat4x4_rotate_Z(Q, I, angle);

      // Translate
      Q[3][0] = xoffset;
      Q[3][1] = yoffset;

      mat4x4 X;
      mat4x4_mul(
          X, (float(*)[4])mvmatrix, //m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform,
          Q);

      line_shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)X);

      line_shader->UnBind();
    }

    if (n > 4) {
      if (ConfigureBrush())  // Check for transparent brush
        DrawPolygonTessellated(n, points, xoffset, yoffset);

      // Draw the polygon ouline
      //  Grow the work buffer as necessary
      if (workBufSize < (size_t)n * 2) {
        workBuf = (float *)realloc(workBuf, (n * 4) * sizeof(float));
        workBufSize = n * 4;
      }

      for (int i = 0; i < n; i++) {
        workBuf[i * 2] = (points[i].x * scale);
        workBuf[i * 2 + 1] = (points[i].y * scale);
      }

      line_shader->Bind();
      line_shader->SetAttributePointerf("position", workBuf);

      // Render the polygon outline.
      glDrawArrays(GL_LINE_LOOP, 0, n);

      // Restore the default matrix
      //TODO  This will not work for multicanvas
      //shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform);

      line_shader->UnBind();

    } else {  // n = 3 or 4, most common case for pre-tesselated shapes

      //  Grow the work buffer as necessary
      if (workBufSize < (size_t)n * 2) {
        workBuf = (float *)realloc(workBuf, (n * 4) * sizeof(float));
        workBufSize = n * 4;
      }

      for (int i = 0; i < n; i++) {
        workBuf[i * 2] = (points[i].x * scale);      // + xoffset;
        workBuf[i * 2 + 1] = (points[i].y * scale);  // + yoffset;
      }

      // Draw the triangle fill
      GLShaderProgram *shader;
      float* mvmatrix;

      if(m_glchartCanvas){
        shader = pcolor_tri_shader_program[m_canvasIndex];
        mvmatrix = (float *)&(m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform);
      }
      else{
        shader = m_pcolor_tri_shader_program;
        mvmatrix = (float *)&(m_vp.vp_matrix_transform);
      }
      shader->Bind();

      // Rotate
      mat4x4 I, Q;
      mat4x4_identity(I);
      mat4x4_rotate_Z(Q, I, angle);

      // Translate
      Q[3][0] = xoffset;
      Q[3][1] = yoffset;
      mat4x4 X;
      mat4x4_mul(X, (float(*)[4])mvmatrix, Q);
      shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)X);


      //  Fill color
      float bcolorv[4];
      bcolorv[0] = m_brush.GetColour().Red() / float(256);
      bcolorv[1] = m_brush.GetColour().Green() / float(256);
      bcolorv[2] = m_brush.GetColour().Blue() / float(256);
      bcolorv[3] = m_brush.GetColour().Alpha() / float(256);
      shader->SetUniform4fv("color", bcolorv);

      shader->SetAttributePointerf("position", workBuf);

      // For the simple common case of a convex rectangle...
      //  swizzle the array points to enable GL_TRIANGLE_STRIP
      if (n == 4) {
        float x1 = workBuf[4];
        float y1 = workBuf[5];
        workBuf[4] = workBuf[6];
        workBuf[5] = workBuf[7];
        workBuf[6] = x1;
        workBuf[7] = y1;

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      } else if (n == 3) {
        glDrawArrays(GL_TRIANGLES, 0, 3);
      }

      // Restore the default glCanvas matrix
      if (m_glchartCanvas){
        shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform);
      }

      shader->UnBind();

      // Draw the polygon outline

      // Reset the workbuf, corrupted in swizzle above
      for (int i = 0; i < n; i++) {
        workBuf[i * 2] = (points[i].x * scale);      // + xoffset;
        workBuf[i * 2 + 1] = (points[i].y * scale);  // + yoffset;
      }

      line_shader->Bind();

      line_shader->SetAttributePointerf("position", workBuf);

      glDrawArrays(GL_LINE_LOOP, 0, n);

      // Restore the default matrix
      //TODO  This will not work for multicanvas
      //shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform);

      line_shader->UnBind();
    }

    SetGLAttrs(false);
  }
#endif
}

#ifdef ocpnUSE_GL

// GL callbacks

typedef union {
  GLdouble data[6];
  struct sGLvertex {
    GLdouble x;
    GLdouble y;
    GLdouble z;
    GLdouble r;
    GLdouble g;
    GLdouble b;
  } info;
} GLvertex;

// GLSL callbacks

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)

static std::list<double *> odc_combine_work_data;
static void odc_combineCallbackD(GLdouble coords[3], GLdouble *vertex_data[4],
                                 GLfloat weight[4], GLdouble **dataOut,
                                 void *data) {
  //     double *vertex = new double[3];
  //     odc_combine_work_data.push_back(vertex);
  //     memcpy(vertex, coords, 3*(sizeof *coords));
  //     *dataOut = vertex;
}

void odc_vertexCallbackD_GLSL(GLvoid *vertex, void *data) {
  ocpnDC *pDC = (ocpnDC *)data;

  // Grow the work buffer if necessary
  if (pDC->s_odc_tess_vertex_idx > pDC->s_odc_tess_buf_len - 8) {
    int new_buf_len = pDC->s_odc_tess_buf_len + 100;
    GLfloat *tmp = pDC->s_odc_tess_work_buf;

    pDC->s_odc_tess_work_buf = (GLfloat *)realloc(
        pDC->s_odc_tess_work_buf, new_buf_len * sizeof(GLfloat));
    if (NULL == pDC->s_odc_tess_work_buf) {
      free(tmp);
      tmp = NULL;
    } else
      pDC->s_odc_tess_buf_len = new_buf_len;
  }

  GLdouble *pointer = (GLdouble *)vertex;

  pDC->s_odc_tess_work_buf[pDC->s_odc_tess_vertex_idx++] = (float)pointer[0];
  pDC->s_odc_tess_work_buf[pDC->s_odc_tess_vertex_idx++] = (float)pointer[1];

  pDC->s_odc_nvertex++;
}

void odc_beginCallbackD_GLSL(GLenum mode, void *data) {
  ocpnDC *pDC = (ocpnDC *)data;
  pDC->s_odc_tess_vertex_idx_this = pDC->s_odc_tess_vertex_idx;
  pDC->s_odc_tess_mode = mode;
  pDC->s_odc_nvertex = 0;
}

void odc_endCallbackD_GLSL(void *data) {
#if 1
  ocpnDC *pDC = (ocpnDC *)data;

  GLShaderProgram *shader = pcolor_tri_shader_program[pDC->m_canvasIndex];
  shader->Bind();

  float colorv[4];
  wxColour c = pDC->GetBrush().GetColour();

  colorv[0] = c.Red() / float(256);
  colorv[1] = c.Green() / float(256);
  colorv[2] = c.Blue() / float(256);
  colorv[3] = c.Alpha() / float(256);
  shader->SetUniform4fv("color", colorv);

  float *bufPt = &(pDC->s_odc_tess_work_buf[pDC->s_odc_tess_vertex_idx_this]);
  shader->SetAttributePointerf("position", bufPt);

  glDrawArrays(pDC->s_odc_tess_mode, 0, pDC->s_odc_nvertex);

  shader->UnBind();


#endif
}
#endif

#endif  //#ifdef ocpnUSE_GL

void ocpnDC::DrawPolygonTessellated(int n, wxPoint points[], wxCoord xoffset,
                                    wxCoord yoffset) {
  if (dc) dc->DrawPolygon(n, points, xoffset, yoffset);
#ifdef ocpnUSE_GL
  else {
    if (!m_glchartCanvas)
      return;

#if !defined(ocpnUSE_GLES) || \
    defined(USE_ANDROID_GLES2)  // tessalator in glues is broken
    if (n < 5)
#endif
    {
      DrawPolygon(n, points, xoffset, yoffset);
      return;
    }

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
    m_tobj = gluNewTess();
    s_odc_tess_vertex_idx = 0;

    gluTessCallback(m_tobj, GLU_TESS_VERTEX_DATA,
                    (_GLUfuncptr)&odc_vertexCallbackD_GLSL);
    gluTessCallback(m_tobj, GLU_TESS_BEGIN_DATA,
                    (_GLUfuncptr)&odc_beginCallbackD_GLSL);
    gluTessCallback(m_tobj, GLU_TESS_END_DATA,
                    (_GLUfuncptr)&odc_endCallbackD_GLSL);
    gluTessCallback(m_tobj, GLU_TESS_COMBINE_DATA,
                    (_GLUfuncptr)&odc_combineCallbackD);
    // s_tessVP = vp;

    gluTessNormal(m_tobj, 0, 0, 1);
    gluTessProperty(m_tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

    if (ConfigureBrush()) {
      gluTessBeginPolygon(m_tobj, this);
      gluTessBeginContour(m_tobj);

      ViewPort *pvp = m_glchartCanvas->m_pParentCanvas->GetpVP(); //gFrame->GetPrimaryCanvas()->GetpVP();

      for (int i = 0; i < n; i++) {
        double *p = new double[6];

        if (fabs(pvp->rotation) > 0.01) {
          float cx = pvp->pix_width / 2.;
          float cy = pvp->pix_height / 2.;
          float c = cosf(pvp->rotation);
          float s = sinf(pvp->rotation);
          float xn = points[i].x - cx;
          float yn = points[i].y - cy;
          p[0] = xn * c - yn * s + cx;
          p[1] = xn * s + yn * c + cy;
          p[2] = 0;
        } else
          p[0] = points[i].x, p[1] = points[i].y, p[2] = 0;

        gluTessVertex(m_tobj, p, p);
      }
      gluTessEndContour(m_tobj);
      gluTessEndPolygon(m_tobj);
    }

    gluDeleteTess(m_tobj);

    //         for(std::list<double*>::iterator i =
    //         odc_combine_work_data.begin(); i!=odc_combine_work_data.end();
    //         i++)
    //             delete [] *i;
    //         odc_combine_work_data.clear();
  }
#else

 }

#endif
#endif
}

void ocpnDC::StrokePolygon(int n, wxPoint points[], wxCoord xoffset,
                           wxCoord yoffset, float scale) {
#if wxUSE_GRAPHICS_CONTEXT
  if (pgc) {
    wxGraphicsPath gpath = pgc->CreatePath();
    gpath.MoveToPoint(points[0].x * scale + xoffset,
                      points[0].y * scale + yoffset);
    for (int i = 1; i < n; i++)
      gpath.AddLineToPoint(points[i].x * scale + xoffset,
                           points[i].y * scale + yoffset);
    gpath.AddLineToPoint(points[0].x * scale + xoffset,
                         points[0].y * scale + yoffset);

    pgc->SetPen(GetPen());
    pgc->SetBrush(GetBrush());
    pgc->DrawPath(gpath);

    for (int i = 0; i < n; i++)
      dc->CalcBoundingBox(points[i].x * scale + xoffset,
                          points[i].y * scale + yoffset);
  } else
#endif
    DrawPolygon(n, points, xoffset, yoffset, scale);
}

void ocpnDC::DrawBitmap(const wxBitmap &bitmap, wxCoord x, wxCoord y,
                        bool usemask) {
  wxBitmap bmp;
  if (x < 0 || y < 0) {
    int dx = (x < 0 ? -x : 0);
    int dy = (y < 0 ? -y : 0);
    int w = bitmap.GetWidth() - dx;
    int h = bitmap.GetHeight() - dy;
    /* picture is out of viewport */
    if (w <= 0 || h <= 0) return;
    wxBitmap newBitmap = bitmap.GetSubBitmap(wxRect(dx, dy, w, h));
    x += dx;
    y += dy;
    bmp = newBitmap;
  } else {
    bmp = bitmap;
  }
  if (dc) dc->DrawBitmap(bmp, x, y, usemask);
#ifdef ocpnUSE_GL
  else {
#ifdef ocpnUSE_GLES  // Do not attempt to do anything with glDrawPixels if using
                     // opengles
    return;          // this should not be hit anymore ever anyway
#endif

#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
    wxImage image = bmp.ConvertToImage();
    int w = image.GetWidth(), h = image.GetHeight();

    if (usemask) {
      unsigned char *d = image.GetData();
      unsigned char *a = image.GetAlpha();

#ifdef __WXOSX__
      if (image.HasMask()) a = 0;
#endif
      unsigned char mr, mg, mb;
      if (!a && !image.GetOrFindMaskColour(&mr, &mg, &mb)) {
        printf("trying to use mask to draw a bitmap without alpha or mask\n");
      }

      unsigned char *e = new unsigned char[4 * w * h];
      if (e && d) {
        for (int y = 0; y < h; y++)
          for (int x = 0; x < w; x++) {
            unsigned char r, g, b;
            int off = (y * w + x);
            r = d[off * 3 + 0];
            g = d[off * 3 + 1];
            b = d[off * 3 + 2];

            e[off * 4 + 0] = r;
            e[off * 4 + 1] = g;
            e[off * 4 + 2] = b;

            e[off * 4 + 3] =
                a ? a[off] : ((r == mr) && (g == mg) && (b == mb) ? 0 : 255);
            //                        e[off * 4 + 3] = ( ( r == mr ) && ( g ==
            //                        mg ) && ( b == mb ) ? 0 : 255 );
          }
      }

      glColor4f(1, 1, 1, 1);
      GLDrawBlendData(x, y, w, h, GL_RGBA, e);
      delete[](e);
    } else {
      glRasterPos2i(x, y);
      glPixelZoom(1, -1); /* draw data from top to bottom */
      if (image.GetData())
        glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, image.GetData());
      glPixelZoom(1, 1);
    }
#endif  // GLES2
  }
#endif
}

void ocpnDC::DrawText(const wxString &text, wxCoord x, wxCoord y, float angle) {
  if (dc) dc->DrawText(text, x, y);
#ifdef ocpnUSE_GL
  else {
    wxCoord w = 0;
    wxCoord h = 0;

    //FIXME Dave Re-enable, and fix rotation logic.
    if (0/*m_buseTex*/) {
      m_texfont.Build(m_font, 1.0, m_dpi_factor);  // make sure the font is ready
      m_texfont.GetTextExtent(text, &w, &h);
      m_texfont.SetColor(m_textforegroundcolour);

      if (w && h) {
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_texfont.RenderString(text, x, y, angle);

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
      }
    } else {
      wxScreenDC sdc;
      sdc.SetFont(m_font);
      sdc.GetTextExtent(text, &w, &h, NULL, NULL, &m_font);
      if(w && h){
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
        temp_dc.DrawText(text, 0, 0);
        temp_dc.SelectObject(wxNullBitmap);

        /* use the data in the bitmap for alpha channel,
        and set the color to text foreground */
        wxImage image = bmp.ConvertToImage();
        if (x < 0 ||
            y < 0) {  // Allow Drawing text which is offset to start off screen
          int dx = (x < 0 ? -x : 0);
          int dy = (y < 0 ? -y : 0);
          w = bmp.GetWidth() - dx;
          h = bmp.GetHeight() - dy;
          /* picture is out of viewport */
          if (w <= 0 || h <= 0) return;
          image = image.GetSubImage(wxRect(dx, dy, w, h));
          x += dx;
          y += dy;
        }

        unsigned char *data = new unsigned char[w * h * 4];
        unsigned char *im = image.GetData();

        if (im) {
          unsigned int r = m_textforegroundcolour.Red();
          unsigned int g = m_textforegroundcolour.Green();
          unsigned int b = m_textforegroundcolour.Blue();
          for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
              unsigned int index = ((i * w) + j) * 4;
              data[index] = r;
              data[index + 1] = g;
              data[index + 2] = b;
              data[index + 3] = im[((i * w) + j) * 3];
            }
          }
        }

        unsigned int texobj;

        glGenTextures(1, &texobj);
        glBindTexture(GL_TEXTURE_2D, texobj);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        int TextureWidth = NextPow2(w);
        int TextureHeight = NextPow2(h);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TextureWidth, TextureHeight, 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE,
                        data);

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float u = (float)w / TextureWidth, v = (float)h / TextureHeight;

        float uv[8];
        float coords[8];

        // normal uv
        uv[0] = 0;
        uv[1] = 0;
        uv[2] = u;
        uv[3] = 0;
        uv[4] = u;
        uv[5] = v;
        uv[6] = 0;
        uv[7] = v;

        // pixels
        coords[0] = 0;
        coords[1] = 0;
        coords[2] = w;
        coords[3] = 0;
        coords[4] = w;
        coords[5] = h;
        coords[6] = 0;
        coords[7] = h;

        // Set up the shader
        GLShaderProgram *shader;
        if (m_glchartCanvas){
          shader = ptexture_2D_shader_program[m_canvasIndex];
          shader->Bind();
          // Assuming here that transform matrix for this shader is preset for canvas.
          //shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_matrix_transform);
        }
        else{
          shader = m_ptexture_2D_shader_program;
          shader->Bind();
          shader->SetUniformMatrix4fv("MVMatrix", (float *)&(m_vp.vp_matrix_transform));
        }

      // Set up the texture sampler to texture unit 0
        shader->SetUniform1i("uTex", 0);

        // Rotate
        mat4x4 I, Q;
        mat4x4_identity(I);
        mat4x4_rotate_Z(Q, I, 0);

        // Translate
        Q[3][0] = x;
        Q[3][1] = y;

        shader->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)Q);

        float co1[8];
        float tco1[8];



    // Perform the actual drawing.

    // For some reason, glDrawElements is busted on Android
    // So we do this a hard ugly way, drawing two triangles...
        co1[0] = coords[0];
        co1[1] = coords[1];
        co1[2] = coords[2];
        co1[3] = coords[3];
        co1[4] = coords[6];
        co1[5] = coords[7];
        co1[6] = coords[4];
        co1[7] = coords[5];

        tco1[0] = uv[0];
        tco1[1] = uv[1];
        tco1[2] = uv[2];
        tco1[3] = uv[3];
        tco1[4] = uv[6];
        tco1[5] = uv[7];
        tco1[6] = uv[4];
        tco1[7] = uv[5];

        shader->SetAttributePointerf("aPos", co1);
        shader->SetAttributePointerf("aUV", tco1);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        shader->UnBind();


        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);

        glDeleteTextures(1, &texobj);
        delete[] data;
      }
    }
  }
#endif
}

void ocpnDC::GetTextExtent(const wxString &string, wxCoord *w, wxCoord *h,
                           wxCoord *descent, wxCoord *externalLeading,
                           wxFont *font) {
  //  Give at least reasonable results on failure.
  if (w) *w = 100;
  if (h) *h = 100;

  if (dc)
    dc->GetTextExtent(string, w, h, descent, externalLeading, font);
  else {
    wxFont f = m_font;
    if (font) f = *font;

    //FIXME Dave Re-enable, and fix rotation logic.
    if (0/*m_buseTex*/) {
#ifdef ocpnUSE_GL
      m_texfont.Build(f, 1.0, m_dpi_factor);  // make sure the font is ready
      m_texfont.GetTextExtent(string, w, h);
#else
      wxMemoryDC temp_dc;
      temp_dc.GetTextExtent(string, w, h, descent, externalLeading, &f);
#endif
    } else {
      wxMemoryDC temp_dc;
      temp_dc.GetTextExtent(string, w, h, descent, externalLeading, &f);
    }
  }

  //  Sometimes GetTextExtent returns really wrong, uninitialized results.
  //  Dunno why....
  if (w && (*w > 500)) *w = 500;
  if (h && (*h > 500)) *h = 500;
}

void ocpnDC::ResetBoundingBox() {
  if (dc) dc->ResetBoundingBox();
}

void ocpnDC::CalcBoundingBox(wxCoord x, wxCoord y) {
  if (dc) dc->CalcBoundingBox(x, y);
}

bool ocpnDC::ConfigurePen() {
  if (!m_pen.IsOk()) return false;
  if (m_pen == *wxTRANSPARENT_PEN) return false;

  wxColour c = m_pen.GetColour();
  int width = m_pen.GetWidth();
#ifdef ocpnUSE_GL
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
  glColor4ub(c.Red(), c.Green(), c.Blue(), c.Alpha());
#endif
  glLineWidth(wxMax(g_GLMinSymbolLineWidth, width));
#endif
  return true;
}

bool ocpnDC::ConfigureBrush() {
  if (m_brush == wxNullBrush || m_brush.GetStyle() == wxBRUSHSTYLE_TRANSPARENT)
    return false;
#ifdef ocpnUSE_GL
  wxColour c = m_brush.GetColour();
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
  glColor4ub(c.Red(), c.Green(), c.Blue(), c.Alpha());
#endif
#endif
  return true;
}

void ocpnDC::GLDrawBlendData(wxCoord x, wxCoord y, wxCoord w, wxCoord h,
                             int format, const unsigned char *data) {
#ifdef ocpnUSE_GL
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
  glEnable(GL_BLEND);
  glRasterPos2i(x, y);
  glPixelZoom(1, -1);
  glDrawPixels(w, h, format, GL_UNSIGNED_BYTE, data);
  glPixelZoom(1, 1);
  glDisable(GL_BLEND);
#endif
#endif
}

void ocpnDC::SetVP(ViewPort vp){
  m_vp = vp;
  m_vp.SetVPTransformMatrix();
  BuildShaders();

  // Program the matrix transforms for the several private shaders
  mat4x4 I;
  mat4x4_identity(I);


  m_pcolor_tri_shader_program->Bind();
  m_pcolor_tri_shader_program->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_vp.vp_matrix_transform);
  m_pcolor_tri_shader_program->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)I);
  m_pcolor_tri_shader_program->UnBind();

  m_pAALine_shader_program->Bind();
  m_pAALine_shader_program->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_vp.vp_matrix_transform);
  m_pAALine_shader_program->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)I);
  m_pAALine_shader_program->UnBind();

  m_pcircle_filled_shader_program->Bind();
  m_pcircle_filled_shader_program->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_vp.vp_matrix_transform);
  m_pcircle_filled_shader_program->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)I);
  m_pcircle_filled_shader_program->UnBind();

  m_ptexture_2D_shader_program->Bind();
  m_ptexture_2D_shader_program->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_vp.vp_matrix_transform);
  m_ptexture_2D_shader_program->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)I);
  m_ptexture_2D_shader_program->UnBind();

}



//  Private shaders, used when drawing to a context which is not a glChartCanvas (i.e. radar_pi)
#ifdef USE_ANDROID_GLES2
const GLchar* odc_preamble =
"\n";
#else
const GLchar* odc_preamble =
"#version 120\n"
"#define precision\n"
"#define lowp\n"
"#define mediump\n"
"#define highp\n";
#endif


// Simple colored triangle shader

static const GLchar* odc_color_tri_vertex_shader_source =
    "attribute vec2 position;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "uniform vec4 color;\n"
    "varying vec4 fragColor;\n"
    "void main() {\n"
    "   fragColor = color;\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(position, 0.0, 1.0);\n"
    "}\n";

static const GLchar* odc_color_tri_fragment_shader_source =
    "precision lowp float;\n"
    "varying vec4 fragColor;\n"
    "void main() {\n"
    "   gl_FragColor = fragColor;\n"
    "}\n";

static const GLchar* odc_AALine_vertex_shader_source =
    "uniform vec2 uViewPort; //Width and Height of the viewport\n"
    "varying vec2 vLineCenter;\n"
    "attribute vec2 position;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
  "void main()\n"
  "{\n"
  "   vec4 pp = MVMatrix * vec4(position, 0.0, 1.0);\n"
  "   gl_Position = pp;\n"
  "   vec2 vp = uViewPort;\n"
  "   vLineCenter = 0.5*(pp.xy + vec2(1, 1))*vp;\n"
  "}\n";


static const GLchar* odc_AALine_fragment_shader_source =
    "precision mediump float;\n"
    "uniform float uLineWidth;\n"
    "uniform vec4 color;\n"
    "uniform float uBlendFactor; //1.5..2.5\n"
    "varying vec2 vLineCenter;\n"
    "void main()\n"
    "{\n"
    "    vec4 col = color;\n"
    "    float d = length(vLineCenter-gl_FragCoord.xy);\n"
    "    float w = uLineWidth;\n"
    "    if (d>w)\n"
    "      col.w = 0.0;\n"
    "    else{\n"
    "      if(float((w/2-d)/(w/2)) < .5){\n"
    "        //col.w *= pow(float((w-d)/w), uBlendFactor);\n"
    "        col.w *= pow(float((w/2-d)/(w/2)), uBlendFactor);\n"
    "      }\n"
    "    }\n"
    "    gl_FragColor = col;\n"
    "}\n";

//  Circle shader
static const GLchar* odc_circle_filled_vertex_shader_source =
    "precision highp float;\n"
    "attribute vec2 aPos;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const GLchar* odc_circle_filled_fragment_shader_source =
    "precision highp float;\n"
    "uniform float border_width;\n"
    "uniform float circle_radius;\n"
    "uniform vec4 circle_color;\n"
    "uniform vec4 border_color;\n"
    "uniform vec2 circle_center;\n"
    "void main(){\n"
    "float d = distance(gl_FragCoord.xy, circle_center);\n"
    "if (d < (circle_radius - border_width)) { gl_FragColor = circle_color; }\n"
    "else if (d < circle_radius) { gl_FragColor = border_color; }\n"
    "else { gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); }\n"
    "}\n";

// Simple 2D texture shader
static const GLchar* odc_texture_2D_vertex_shader_source =
    "attribute vec2 aPos;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "   varCoord = aUV;\n"
    "}\n";

static const GLchar* odc_texture_2D_fragment_shader_source =
    "precision lowp float;\n"
    "uniform sampler2D uTex;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_FragColor = texture2D(uTex, varCoord);\n"
    "}\n";

void ocpnDC::BuildShaders(){

  // Simple colored triangle shader
  if (!m_pcolor_tri_shader_program) {
    GLShaderProgram *shaderProgram = new GLShaderProgram;
    shaderProgram->addShaderFromSource(odc_color_tri_vertex_shader_source, GL_VERTEX_SHADER);
    shaderProgram->addShaderFromSource(odc_color_tri_fragment_shader_source, GL_FRAGMENT_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK())
      m_pcolor_tri_shader_program = shaderProgram;
  }

  if (!m_pAALine_shader_program) {
    GLShaderProgram *shaderProgram = new GLShaderProgram;
    shaderProgram->addShaderFromSource(odc_AALine_fragment_shader_source, GL_FRAGMENT_SHADER);
    shaderProgram->addShaderFromSource(odc_AALine_vertex_shader_source, GL_VERTEX_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK())
      m_pAALine_shader_program = shaderProgram;
  }

  if (!m_pcircle_filled_shader_program) {
    GLShaderProgram *shaderProgram = new GLShaderProgram;
    shaderProgram->addShaderFromSource(odc_circle_filled_vertex_shader_source, GL_VERTEX_SHADER);
    shaderProgram->addShaderFromSource(odc_circle_filled_fragment_shader_source, GL_FRAGMENT_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK())
      m_pcircle_filled_shader_program = shaderProgram;
  }

  if (!m_ptexture_2D_shader_program) {
    GLShaderProgram *shaderProgram = new GLShaderProgram;
    shaderProgram->addShaderFromSource(odc_texture_2D_vertex_shader_source, GL_VERTEX_SHADER);
    shaderProgram->addShaderFromSource(odc_texture_2D_fragment_shader_source, GL_FRAGMENT_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK())
      m_ptexture_2D_shader_program = shaderProgram;
  }

}
