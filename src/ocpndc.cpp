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

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "config.h"

#include "dychart.h"
#include "ocpn_plugin.h"
#include "chcanv.h"
#include "chart1.h"
#include "linmath.h"

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
#ifdef ocpnUSE_GL
  m_textforegroundcolour = wxColour(0, 0, 0);
#endif
  m_buseTex = GetLocaleCanonicalName().IsSameAs(_T("en_US"));
  workBuf = NULL;
  workBufSize = 0;
  s_odc_tess_work_buf = NULL;
  m_canvasIndex = m_glchartCanvas->GetCanvasIndex();

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
  s_odc_tess_vertex_idx = 0;
  s_odc_tess_vertex_idx_this = 0;
  s_odc_tess_buf_len = 0;

  s_odc_tess_work_buf = (GLfloat *)malloc(100 * sizeof(GLfloat));
  s_odc_tess_buf_len = 100;

#endif
}

ocpnDC::ocpnDC(wxGLCanvas &canvas)
    : m_glchartCanvas(NULL), m_glcanvas(&canvas), dc(NULL), m_pen(wxNullPen), m_brush(wxNullBrush) {
#if wxUSE_GRAPHICS_CONTEXT
  pgc = NULL;
#endif
#ifdef ocpnUSE_GL
  m_textforegroundcolour = wxColour(0, 0, 0);
#endif
  m_buseTex = GetLocaleCanonicalName().IsSameAs(_T("en_US"));
  workBuf = NULL;
  workBufSize = 0;
  s_odc_tess_work_buf = NULL;
  m_canvasIndex = 0;

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
  s_odc_tess_vertex_idx = 0;
  s_odc_tess_vertex_idx_this = 0;
  s_odc_tess_buf_len = 0;

  s_odc_tess_work_buf = (GLfloat *)malloc(100 * sizeof(GLfloat));
  s_odc_tess_buf_len = 100;

#endif
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
  m_textforegroundcolour = wxColour(0, 0, 0);
  m_buseTex = GetLocaleCanonicalName().IsSameAs(_T("en_US"));
  workBuf = NULL;
  workBufSize = 0;
  s_odc_tess_work_buf = NULL;
  m_canvasIndex = 0;
}

ocpnDC::ocpnDC()
    : m_glchartCanvas(NULL), m_glcanvas(NULL), dc(NULL), m_pen(wxNullPen), m_brush(wxNullBrush) {
#if wxUSE_GRAPHICS_CONTEXT
  pgc = NULL;
#endif
  m_buseTex = GetLocaleCanonicalName().IsSameAs(_T("en_US"));
  workBuf = NULL;
  workBufSize = 0;
  s_odc_tess_work_buf = NULL;
  m_canvasIndex = 0;
}

ocpnDC::~ocpnDC() {
#if wxUSE_GRAPHICS_CONTEXT
  if (pgc) delete pgc;
#endif
  free(workBuf);

  free(s_odc_tess_work_buf);
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
    if (m_glchartCanvas)
      m_glchartCanvas->GetSize(width, height);
    else if (m_glcanvas)
      m_glcanvas->GetSize(width, height);
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

#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
  glBegin(GL_TRIANGLES);

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

      glVertex2f(xa + t2sina1, ya - t2cosa1);
      glVertex2f(xb + t2sina1, yb - t2cosa1);
      glVertex2f(xb - t2sina1, yb + t2cosa1);

      glVertex2f(xb - t2sina1, yb + t2cosa1);
      glVertex2f(xa - t2sina1, ya + t2cosa1);
      glVertex2f(xa + t2sina1, ya - t2cosa1);

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
    glVertex2f(x1 + t2sina1, y1 - t2cosa1);
    glVertex2f(x2 + t2sina1, y2 - t2cosa1);
    glVertex2f(x2 - t2sina1, y2 + t2cosa1);

    glVertex2f(x2 - t2sina1, y2 + t2cosa1);
    glVertex2f(x1 - t2sina1, y1 + t2cosa1);
    glVertex2f(x1 + t2sina1, y1 - t2cosa1);

    /* wx draws a nice rounded end in dc mode, so replicate
       this for opengl mode, should this be done for the dashed mode case? */
    if (pen.GetCap() == wxCAP_ROUND) {
      DrawEndCap(x1, y1, t1, angle);
      DrawEndCap(x2, y2, t1, angle + M_PI);
    }
  }

  glEnd();
#else

  // Set up the shader
    GLShaderProgram *shader = pcolor_tri_shader_program[m_canvasIndex];
    shader->Bind();

    wxColor c = pen.GetColour();
    float colorv[4];
    colorv[0] = c.Red() / float(256);
    colorv[1] = c.Green() / float(256);
    colorv[2] = c.Blue() / float(256);
    colorv[3] = c.Alpha() / float(256);
    shader->SetUniform4fv("color", colorv);

    // Build Transform matrix
    mat4x4 I;
    mat4x4_identity(I);

    shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_transform);
    shader->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)I);

//     GLint matloc =
//         glGetUniformLocation(color_tri_shader_program, "TransformMatrix");
//     glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)I);
//
//     matloc = glGetUniformLocation(color_tri_shader_program, "MVMatrix");
//     glUniformMatrix4fv(
//         matloc, 1, GL_FALSE,
//         (const GLfloat *)gFrame->GetPrimaryCanvas()->GetpVP()->vp_transform);


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

//     GLShaderProgramA *shader = pcolor_tri_shader_program[0];
//     shader->Bind();
//
//     wxColor c = pen.GetColour();
//     float colorv[4];
//     colorv[0] = c.Red() / float(256);
//     colorv[1] = c.Green() / float(256);
//     colorv[2] = c.Blue() / float(256);
//     colorv[3] = c.Alpha() / float(256);
//     shader->SetUniform4fv("color", colorv);
//
//     // Build Transform matrix
//     mat4x4 I;
//     mat4x4_identity(I);
//
//     shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)gFrame->GetPrimaryCanvas()->GetpVP()->vp_transform);
//     shader->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)I);
//
//     GLint matloc =
//         glGetUniformLocation(color_tri_shader_program, "TransformMatrix");
//     glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)I);
//
//     matloc = glGetUniformLocation(color_tri_shader_program, "MVMatrix");
//     glUniformMatrix4fv(
//         matloc, 1, GL_FALSE,
//         (const GLfloat *)gFrame->GetPrimaryCanvas()->GetpVP()->vp_transform);
//
//
//     float vert[12];
//     shader->SetAttributePointerf("position", vert);

#if 0
    glUseProgram(color_tri_shader_program);


    // Disable VBO's (vertex buffer objects) for attributes.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLint pos = glGetAttribLocation(color_tri_shader_program, "position");
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), vert);

    // Build Transform matrix
    mat4x4 I;
    mat4x4_identity(I);

    GLint matloc =
        glGetUniformLocation(color_tri_shader_program, "TransformMatrix");
    glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)I);

    matloc = glGetUniformLocation(color_tri_shader_program, "MVMatrix");
    glUniformMatrix4fv(
        matloc, 1, GL_FALSE,
        (const GLfloat *)gFrame->GetPrimaryCanvas()->GetpVP()->vp_transform);

    wxColor c = pen.GetColour();
    float colorv[4];
    colorv[0] = c.Red() / float(256);
    colorv[1] = c.Green() / float(256);
    colorv[2] = c.Blue() / float(256);
    colorv[3] = c.Alpha() / float(256);

    GLint colloc = glGetUniformLocation(color_tri_shader_program, "color");
    glUniform4fv(colloc, 1, colorv);
#endif

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

#if 0
    glUseProgram(color_tri_shader_program);

    // Disable VBO's (vertex buffer objects) for attributes.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLint pos = glGetAttribLocation(color_tri_shader_program, "position");
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), vert);

    // Build Transform matrix
    mat4x4 I;
    mat4x4_identity(I);

    GLint matloc =
        glGetUniformLocation(color_tri_shader_program, "TransformMatrix");
    glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)I);

    matloc = glGetUniformLocation(color_tri_shader_program, "MVMatrix");
    glUniformMatrix4fv(
        matloc, 1, GL_FALSE,
        (const GLfloat *)gFrame->GetPrimaryCanvas()->GetpVP()->vp_transform);

    wxColor c = pen.GetColour();
    float colorv[4];
    colorv[0] = c.Red() / float(256);
    colorv[1] = c.Green() / float(256);
    colorv[2] = c.Blue() / float(256);
    colorv[3] = c.Alpha() / float(256);

    GLint colloc = glGetUniformLocation(color_tri_shader_program, "color");
    glUniform4fv(colloc, 1, colorv);
#endif

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
      GLShaderProgram *shader = pAALine_shader_program[m_canvasIndex];
      shader->Bind();

      shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_transform);
      shader->SetUniform1f("uLineWidth", pen_width);
      shader->SetUniform1f("uBlendFactor", 2.0);

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

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
  wxPoint p0 = points[0];
  for (int i = 1; i < n; i++) {
    DrawGLThickLine(p0.x + xoffset, p0.y + yoffset, points[i].x + xoffset,
                    points[i].y + yoffset, pen, b_hiqual);
    p0 = points[i];
  }
  return;
#else

  /* for dashed case, for now just draw thick lines */
  wxDash *dashes;
  if (pen.GetDashes(&dashes)) {
    wxPoint p0 = points[0];
    for (int i = 1; i < n; i++) {
      DrawGLThickLine(p0.x + xoffset, p0.y + yoffset, points[i].x + xoffset,
                      points[i].y + yoffset, pen, b_hiqual);
      p0 = points[i];
    }
    return;
  }

  /* cull zero segments */
  wxPoint *cpoints = new wxPoint[n];
  cpoints[0] = points[0];
  int c = 1;
  for (int i = 1; i < n; i++) {
    if (points[i].x != points[i - 1].x || points[i].y != points[i - 1].y)
      cpoints[c++] = points[i];
  }

  /* nicer than than rendering each segment separately, this is because thick
     line segments drawn as rectangles which have different angles have
     rectangles which overlap and also leave a gap.
     This code properly calculates vertexes for adjoining segments */
  float t1 = pen.GetWidth();

  float x0 = cpoints[0].x, y0 = cpoints[0].y, x1 = cpoints[1].x,
        y1 = cpoints[1].y;
  float a0 = atan2f(y1 - y0, x1 - x0);

  // It is also possible to use triangle strip, (and triangle fan for endcap)
  // to reduce vertex count.. is it worth it?
  glBegin(GL_TRIANGLES);

  float t2sina0 = t1 / 2 * sinf(a0);
  float t2cosa0 = t1 / 2 * cosf(a0);

  for (int i = 1; i < c; i++) {
    float x2, y2;
    float a1;

    if (i < c - 1) {
      x2 = cpoints[i + 1].x, y2 = cpoints[i + 1].y;
      a1 = atan2f(y2 - y1, x2 - x1);
    } else {
      x2 = x1, y2 = y1;
      a1 = a0;
    }

    float aa = (a0 + a1) / 2;
    float diff = fabsf(a0 - a1);
    if (diff > M_PI) diff -= 2 * (float)M_PI;
    float rad = t1 / 2 / wxMax(cosf(diff / 2), .4);

    float t2sina1 = rad * sinf(aa);
    float t2cosa1 = rad * cosf(aa);

    glVertex2f(x1 + t2sina1, y1 - t2cosa1);
    glVertex2f(x1 - t2sina1, y1 + t2cosa1);
    glVertex2f(x0 + t2sina0, y0 - t2cosa0);

    glVertex2f(x0 - t2sina0, y0 + t2cosa0);
    glVertex2f(x0 + t2sina0, y0 - t2cosa0);

    float dot = t2sina0 * t2sina1 + t2cosa0 * t2cosa1;
    if (dot > 0)
      glVertex2f(x1 - t2sina1, y1 + t2cosa1);
    else
      glVertex2f(x1 + t2sina1, y1 - t2cosa1);

    x0 = x1, x1 = x2;
    y0 = y1, y1 = y2;
    a0 = a1;
    t2sina0 = t2sina1, t2cosa0 = t2cosa1;
  }

  if (pen.GetCap() == wxCAP_ROUND) {
    DrawEndCap(x0, y0, t1, a0);
    DrawEndCap(x0, y0, t1, a0 + M_PI);
  }

  glEnd();

  delete[] cpoints;
#endif
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
        if (g_GLOptions.m_GLLineSmoothing) glEnable(GL_LINE_SMOOTH);
        //                SetGLStipple(m_pen.GetStyle());
      }

      return;
    }

#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < n; i++)
      glVertex2i(points[i].x + xoffset, points[i].y + yoffset);
    glEnd();

#else

    //  Grow the work buffer as necessary
    if (workBufSize < (size_t)n * 2) {
      workBuf = (float *)realloc(workBuf, (n * 4) * sizeof(float));
      workBufSize = n * 4;
    }

    for (int i = 0; i < n; i++) {
      workBuf[i * 2] = points[i].x + xoffset;
      workBuf[(i * 2) + 1] = points[i].y + yoffset;
    }

      GLShaderProgram *shader = pAALine_shader_program[m_canvasIndex];
      shader->Bind();

      shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_transform);
      shader->SetUniform1f("uLineWidth", m_pen.GetWidth());
      shader->SetUniform1f("uBlendFactor", 2.0);

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


#if 0
    glUseProgram(color_tri_shader_program);

    GLint pos = glGetAttribLocation(color_tri_shader_program, "position");
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          workBuf);
    glEnableVertexAttribArray(pos);
    GLint matloc = glGetUniformLocation(color_tri_shader_program, "MVMatrix");
    glUniformMatrix4fv(
        matloc, 1, GL_FALSE,
        (const GLfloat *)gFrame->GetPrimaryCanvas()->GetpVP()->vp_transform);

    float colorv[4];
    colorv[0] = m_pen.GetColour().Red() / float(256);
    colorv[1] = m_pen.GetColour().Green() / float(256);
    colorv[2] = m_pen.GetColour().Blue() / float(256);
    colorv[3] = m_pen.GetColour().Alpha() / float(256);
    1.0;

    GLint colloc = glGetUniformLocation(color_tri_shader_program, "color");
    glUniform4fv(colloc, 1, colorv);

    glDrawArrays(GL_LINE_STRIP, 0, n);

    glUseProgram(0);
#endif
#endif

    if (b_hiqual) {
      glDisable(GL_LINE_STIPPLE);
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
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
    if (ConfigureBrush()) {
      glBegin(GL_QUADS);
      glVertex2i(x, y);
      glVertex2i(x + w, y);
      glVertex2i(x + w, y + h);
      glVertex2i(x, y + h);
      glEnd();
    }

    if (ConfigurePen()) {
      glBegin(GL_LINE_LOOP);
      glVertex2i(x, y);
      glVertex2i(x + w, y);
      glVertex2i(x + w, y + h);
      glVertex2i(x, y + h);
      glEnd();
    }
#else

    DrawRoundedRectangle(x, y, w, h, 0);
/*
        ConfigureBrush();

        glUseProgram(color_tri_shader_program);

            float pf[8];
            pf[0] = x + w; pf[1] = y; pf[2] = x; pf[3] = y; pf[4] = x + w; pf[5]
   = y + h; pf[6] = x; pf[7] = y + h;

            GLint pos = glGetAttribLocation(color_tri_shader_program,
   "position"); glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE,
   2*sizeof(float), pf); glEnableVertexAttribArray(pos);

            //GLint matloc =
   glGetUniformLocation(color_tri_shader_program,"MVMatrix");
            //glUniformMatrix4fv( matloc, 1, GL_FALSE, (const
   GLfloat*)cc1->GetpVP()->vp_transform);

            float bcolorv[4];
            bcolorv[0] = m_brush.GetColour().Red() / float(256);
            bcolorv[1] = m_brush.GetColour().Green() / float(256);
            bcolorv[2] = m_brush.GetColour().Blue() / float(256);
            bcolorv[3] = m_brush.GetColour().Alpha() / float(256);

            GLint colloc =
   glGetUniformLocation(color_tri_shader_program,"color"); glUniform4fv(colloc,
   1, bcolorv);


            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
*/
#endif
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
    r++;
    int steps = ceil(sqrt((float)r));

    wxCoord x1 = x + r, x2 = x + w - r;
    wxCoord y1 = y + r, y2 = y + h - r;

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)

    ConfigureBrush();
    ConfigurePen();

    //  Grow the work buffer as necessary
    size_t bufReq = steps * 8 * 2 * sizeof(float);  // large, to be sure

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
        X, (float(*)[4])m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_transform, Q);
    shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)X);


    shader->SetAttributePointerf("position", workBuf);

#if 0
    glUseProgram(color_tri_shader_program);

    // Get pointers to the attributes in the program.
    GLint mPosAttrib =
        glGetAttribLocation(color_tri_shader_program, "position");

    // Disable VBO's (vertex buffer objects) for attributes.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, workBuf);
    glEnableVertexAttribArray(mPosAttrib);

    //  Fill color
    float fcolorv[4];
    fcolorv[0] = m_brush.GetColour().Red() / float(256);
    fcolorv[1] = m_brush.GetColour().Green() / float(256);
    fcolorv[2] = m_brush.GetColour().Blue() / float(256);
    fcolorv[3] = m_brush.GetColour().Alpha() / float(256);

    GLint fcolloc = glGetUniformLocation(color_tri_shader_program, "color");
    glUniform4fv(fcolloc, 1, fcolorv);

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
        X, (float(*)[4])gFrame->GetPrimaryCanvas()->GetpVP()->vp_transform, Q);
    GLint matloc = glGetUniformLocation(color_tri_shader_program, "MVMatrix");
    glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)X);
#endif

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

#else
    if (ConfigureBrush()) {
      glBegin(GL_TRIANGLE_FAN);
      drawrrhelper(x2, y1, r, 0, steps);
      drawrrhelper(x1, y1, r, 1, steps);
      drawrrhelper(x1, y2, r, 2, steps);
      drawrrhelper(x2, y2, r, 3, steps);
      glEnd();
    }

    if (ConfigurePen()) {
      glBegin(GL_LINE_LOOP);
      drawrrhelper(x2, y1, r, 0, steps);
      drawrrhelper(x1, y1, r, 1, steps);
      drawrrhelper(x1, y2, r, 2, steps);
      drawrrhelper(x2, y2, r, 3, steps);
      glEnd();
    }
#endif
  }
#endif
}

void ocpnDC::DrawCircle(wxCoord x, wxCoord y, wxCoord radius) {
#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)

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

  GLShaderProgram *shader = pcircle_filled_shader_program[m_canvasIndex];
  shader->Bind();

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


#if 0
  glUseProgram(circle_filled_shader_program);

  // Get pointers to the attributes in the program.
  GLint mPosAttrib = glGetAttribLocation(circle_filled_shader_program, "aPos");

  // Disable VBO's (vertex buffer objects) for attributes.
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords);
  glEnableVertexAttribArray(mPosAttrib);

  //  Circle radius
  GLint radiusloc =
      glGetUniformLocation(circle_filled_shader_program, "circle_radius");
  glUniform1f(radiusloc, radius);

  //  Circle center point
  GLint centerloc =
      glGetUniformLocation(circle_filled_shader_program, "circle_center");
  float ctrv[2];
  ctrv[0] = x;
  ctrv[1] = gFrame->GetPrimaryCanvas()->GetSize().y - y;
  glUniform2fv(centerloc, 1, ctrv);

  //  Circle color
  float colorv[4];
  colorv[0] = m_brush.GetColour().Red() / float(256);
  colorv[1] = m_brush.GetColour().Green() / float(256);
  colorv[2] = m_brush.GetColour().Blue() / float(256);
  colorv[3] = (m_brush.GetStyle() == wxBRUSHSTYLE_TRANSPARENT) ? 0.0 : 1.0;

  GLint colloc =
      glGetUniformLocation(circle_filled_shader_program, "circle_color");
  glUniform4fv(colloc, 1, colorv);

  //  Border color
  float bcolorv[4];
  bcolorv[0] = m_pen.GetColour().Red() / float(256);
  bcolorv[1] = m_pen.GetColour().Green() / float(256);
  bcolorv[2] = m_pen.GetColour().Blue() / float(256);
  bcolorv[3] = m_pen.GetColour().Alpha() / float(256);

  GLint bcolloc =
      glGetUniformLocation(circle_filled_shader_program, "border_color");
  glUniform4fv(bcolloc, 1, bcolorv);

  //  Border Width
  GLint borderWidthloc =
      glGetUniformLocation(circle_filled_shader_program, "border_width");
  glUniform1f(borderWidthloc, m_pen.GetWidth());

  GLint matloc = glGetUniformLocation(circle_filled_shader_program, "MVMatrix");
  glUniformMatrix4fv(
      matloc, 1, GL_FALSE,
      (const GLfloat *)(gFrame->GetPrimaryCanvas()->GetpVP()->vp_transform));

// Perform the actual drawing.
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glUseProgram(0);
#endif

  glDisable(GL_BLEND);


#else
  DrawEllipse(x - radius, y - radius, 2 * radius, 2 * radius);
#endif
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

#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
    if (ConfigureBrush()) {
      glBegin(GL_TRIANGLE_FAN);
      glVertex2f(cx, cy);
      for (float a = 0; a <= 2 * M_PI + M_PI / steps; a += 2 * M_PI / steps)
        glVertex2f(cx + r1 * sinf(a), cy + r2 * cosf(a));
      glEnd();
    }

    if (ConfigurePen()) {
      glBegin(GL_LINE_LOOP);
      for (float a = 0; a < 2 * M_PI - M_PI / steps; a += 2 * M_PI / steps)
        glVertex2f(cx + r1 * sinf(a), cy + r2 * cosf(a));
      glEnd();
    }
#else
#endif
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

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)

    ConfigurePen();

    // Prepare the line rendering shader
      GLShaderProgram *shader = pAALine_shader_program[m_canvasIndex];
      shader->Bind();

      shader->SetUniform1f("uLineWidth", m_pen.GetWidth());
      shader->SetUniform1f("uBlendFactor", 2.0);

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
          X, (float(*)[4])m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_transform,
          Q);

      shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)X);

      shader->UnBind();

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

      GLShaderProgram *shader = pAALine_shader_program[m_canvasIndex];
      shader->Bind();

      shader->SetAttributePointerf("position", workBuf);

      // Render the polygon outline.
      glDrawArrays(GL_LINE_LOOP, 0, n);

      // Restore the default matrix
      //TODO  This will not work for multicanvas
      shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_transform);

      shader->UnBind();

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

      GLShaderProgram *shader = pcolor_tri_shader_program[m_canvasIndex];
      shader->Bind();

      //  Fill color
      float bcolorv[4];
      bcolorv[0] = m_brush.GetColour().Red() / float(256);
      bcolorv[1] = m_brush.GetColour().Green() / float(256);
      bcolorv[2] = m_brush.GetColour().Blue() / float(256);
      bcolorv[3] = m_brush.GetColour().Alpha() / float(256);
      shader->SetUniform4fv("color", bcolorv);

      // Rotate
      mat4x4 I, Q;
      mat4x4_identity(I);
      mat4x4_rotate_Z(Q, I, angle);

      // Translate
      Q[3][0] = xoffset;
      Q[3][1] = yoffset;

      mat4x4 X;
      mat4x4_mul(
          X, (float(*)[4])m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_transform,
          Q);
      shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)X);

      shader->SetAttributePointerf("position", workBuf);

#if 0
      glUseProgram(color_tri_shader_program);

      // Get pointers to the attributes in the program.
      GLint mPosAttrib = glGetAttribLocation(color_tri_shader_program, "position");

      // Disable VBO's (vertex buffer objects) for attributes.
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, workBuf);
      glEnableVertexAttribArray(mPosAttrib);

      //  Fill color
      float bcolorv[4];
      bcolorv[0] = m_brush.GetColour().Red() / float(256);
      bcolorv[1] = m_brush.GetColour().Green() / float(256);
      bcolorv[2] = m_brush.GetColour().Blue() / float(256);
      bcolorv[3] = m_brush.GetColour().Alpha() / float(256);

      GLint bcolloc = glGetUniformLocation(color_tri_shader_program, "color");
      glUniform4fv(bcolloc, 1, bcolorv);

      // Rotate
      mat4x4 I, Q;
      mat4x4_identity(I);
      mat4x4_rotate_Z(Q, I, angle);

      // Translate
      Q[3][0] = xoffset;
      Q[3][1] = yoffset;

      mat4x4 X;
      mat4x4_mul(
          X, (float(*)[4])gFrame->GetPrimaryCanvas()->GetpVP()->vp_transform,
          Q);
      GLint matloc = glGetUniformLocation(color_tri_shader_program, "MVMatrix");
      glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)X);
#endif


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

      // Restore the default matrix
      shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_transform);

      shader->UnBind();

      // Draw the polygon outline
      // Reset the workbuf, corrupted in swizzle above
      for (int i = 0; i < n; i++) {
        workBuf[i * 2] = (points[i].x * scale);      // + xoffset;
        workBuf[i * 2 + 1] = (points[i].y * scale);  // + yoffset;
      }

      shader = pAALine_shader_program[m_canvasIndex];
      shader->Bind();

      shader->SetAttributePointerf("position", workBuf);

      glDrawArrays(GL_LINE_LOOP, 0, n);

      // Restore the default matrix
      //TODO  This will not work for multicanvas
      shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)m_glchartCanvas->m_pParentCanvas->GetpVP()->vp_transform);

      shader->UnBind();
    }

#else
#endif

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

#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
void ocpnDCcombineCallback(GLdouble coords[3],
                                    GLdouble *vertex_data[4], GLfloat weight[4],
                                    GLdouble **dataOut) {
  GLvertex *vertex;

  vertex = new GLvertex();
  gTesselatorVertices.Add(vertex);

  vertex->info.x = coords[0];
  vertex->info.y = coords[1];
  vertex->info.z = coords[2];

  for (int i = 3; i < 6; i++) {
    vertex->data[i] =
        weight[0] * vertex_data[0][i] + weight[1] * vertex_data[1][i];
  }

  *dataOut = &(vertex->data[0]);
}

void ocpnDCvertexCallback(GLvoid *arg) {
  GLvertex *vertex;
  vertex = (GLvertex *)arg;
  glVertex2f((float)vertex->info.x, (float)vertex->info.y);
}

void ocpnDCerrorCallback(GLenum errorCode) {
  const GLubyte *estring;
  estring = gluErrorString(errorCode);
  // wxLogMessage( _T("OpenGL Tessellation Error: %s"), (char *)estring );
}

void ocpnDCbeginCallback(GLenum type) { glBegin(type); }

void ocpnDCendCallback() { glEnd(); }
#endif

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

#if 0
  glUseProgram(color_tri_shader_program);

  // Disable VBO's (vertex buffer objects) for attributes.
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  float *bufPt = &(pDC->s_odc_tess_work_buf[pDC->s_odc_tess_vertex_idx_this]);
  GLint pos = glGetAttribLocation(color_tri_shader_program, "position");
  glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), bufPt);
  glEnableVertexAttribArray(pos);

  /// GLint matloc = glGetUniformLocation(color_tri_shader_program,"MVMatrix");
  /// glUniformMatrix4fv( matloc, 1, GL_FALSE, (const
  /// GLfloat*)s_tessVP.vp_transform);

  float colorv[4];
  wxColour c = pDC->GetBrush().GetColour();

  colorv[0] = c.Red() / float(256);
  colorv[1] = c.Green() / float(256);
  colorv[2] = c.Blue() / float(256);
  colorv[3] = c.Alpha() / float(256);

  GLint colloc = glGetUniformLocation(color_tri_shader_program, "color");
  glUniform4fv(colloc, 1, colorv);

  glDrawArrays(pDC->s_odc_tess_mode, 0, pDC->s_odc_nvertex);
  glUseProgram(0);
#endif

#endif
}
#endif

#endif  //#ifdef ocpnUSE_GL

void ocpnDC::DrawPolygonTessellated(int n, wxPoint points[], wxCoord xoffset,
                                    wxCoord yoffset) {
  if (dc) dc->DrawPolygon(n, points, xoffset, yoffset);
#ifdef ocpnUSE_GL
  else {
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
    static GLUtesselator *tobj = NULL;
    if (!tobj) tobj = gluNewTess();

    gluTessCallback(tobj, GLU_TESS_VERTEX, (_GLUfuncptr)&ocpnDCvertexCallback);
    gluTessCallback(tobj, GLU_TESS_BEGIN, (_GLUfuncptr)&ocpnDCbeginCallback);
    gluTessCallback(tobj, GLU_TESS_END, (_GLUfuncptr)&ocpnDCendCallback);
    gluTessCallback(tobj, GLU_TESS_COMBINE,
                    (_GLUfuncptr)&ocpnDCcombineCallback);
    gluTessCallback(tobj, GLU_TESS_ERROR, (_GLUfuncptr)&ocpnDCerrorCallback);

    gluTessNormal(tobj, 0, 0, 1);
    gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

    if (ConfigureBrush()) {
      gluTessBeginPolygon(tobj, NULL);
      gluTessBeginContour(tobj);

      for (int i = 0; i < n; i++) {
        GLvertex *vertex = new GLvertex();
        gTesselatorVertices.Add(vertex);
        vertex->info.x = (GLdouble)points[i].x;
        vertex->info.y = (GLdouble)points[i].y;
        vertex->info.z = (GLdouble)0.0;
        vertex->info.r = (GLdouble)0.0;
        vertex->info.g = (GLdouble)0.0;
        vertex->info.b = (GLdouble)0.0;
        gluTessVertex(tobj, (GLdouble *)vertex, (GLdouble *)vertex);
      }
      gluTessEndContour(tobj);
      gluTessEndPolygon(tobj);
    }

    for (unsigned int i = 0; i < gTesselatorVertices.Count(); i++)
      delete (GLvertex *)gTesselatorVertices[i];
    gTesselatorVertices.Clear();

    gluDeleteTess(tobj);
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

void ocpnDC::DrawText(const wxString &text, wxCoord x, wxCoord y) {
  if (dc) dc->DrawText(text, x, y);
#ifdef ocpnUSE_GL
  else {
    wxCoord w = 0;
    wxCoord h = 0;

    if (m_buseTex) {
      m_texfont.Build(m_font);  // make sure the font is ready
      m_texfont.GetTextExtent(text, &w, &h);
      m_texfont.SetColor(m_textforegroundcolour);

      if (w && h) {
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
        glPushMatrix();
        glTranslatef(x, y, 0);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor3ub(m_textforegroundcolour.Red(), m_textforegroundcolour.Green(),
                   m_textforegroundcolour.Blue());

        m_texfont.RenderString(text);
        glPopMatrix();
#else
        m_texfont.RenderString(text, x, y);
#endif
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
      }
    } else {
      wxScreenDC sdc;
      sdc.SetFont(m_font);
      sdc.GetTextExtent(text, &w, &h, NULL, NULL, &m_font);

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
#if 0
            glColor4ub( 255, 255, 255, 255 );
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glRasterPos2i( x, y );
            glPixelZoom( 1, -1 );
            glDrawPixels( w, h, GL_RGBA, GL_UNSIGNED_BYTE, data );
            glPixelZoom( 1, 1 );
            glDisable( GL_BLEND );
#else
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

#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
      glColor3ub(0, 0, 0);

      glBegin(GL_QUADS);
      glTexCoord2f(0, 0);
      glVertex2f(x, y);
      glTexCoord2f(u, 0);
      glVertex2f(x + w, y);
      glTexCoord2f(u, v);
      glVertex2f(x + w, y + h);
      glTexCoord2f(0, v);
      glVertex2f(x, y + h);
      glEnd();
#else
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

    GLShaderProgram *shader = ptexture_2D_shader_program[m_canvasIndex];
    shader->Bind();

   // Set up the texture sampler to texture unit 0
    shader->SetUniform1i("uTex", 0);

    // Rotate
    float angle = 0;
    mat4x4 I, Q;
    mat4x4_identity(I);
    mat4x4_rotate_Z(Q, I, angle);

    // Translate
    Q[3][0] = x;
    Q[3][1] = y;

    shader->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)Q);

    float co1[8];
    float tco1[8];



// Perform the actual drawing.

// For some reason, glDrawElements is busted on Android
// So we do this a hard ugly way, drawing two triangles...
#if 0
    GLushort indices1[] = {0,1,3,2};
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices1);
#else

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

#endif
    shader->SetAttributePointerf("aPos", co1);
    shader->SetAttributePointerf("aUV", tco1);


    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    shader->UnBind();

#if 0
      glUseProgram(texture_2D_shader_program);

      // Get pointers to the attributes in the program.
      GLint mPosAttrib = glGetAttribLocation(texture_2D_shader_program, "aPos");
      GLint mUvAttrib = glGetAttribLocation(texture_2D_shader_program, "aUV");

      // Set up the texture sampler to texture unit 0
      GLint texUni = glGetUniformLocation(texture_2D_shader_program, "uTex");
      glUniform1i(texUni, 0);

      // Disable VBO's (vertex buffer objects) for attributes.
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      // Set the attribute mPosAttrib with the vertices in the screen
      // coordinates...
      glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords);
      // ... and enable it.
      glEnableVertexAttribArray(mPosAttrib);

      // Set the attribute mUvAttrib with the vertices in the GL coordinates...
      glVertexAttribPointer(mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, uv);
      // ... and enable it.
      glEnableVertexAttribArray(mUvAttrib);

      // Rotate
      float angle = 0;
      mat4x4 I, Q;
      mat4x4_identity(I);
      mat4x4_rotate_Z(Q, I, angle);

      // Translate
      Q[3][0] = x;
      Q[3][1] = y;

      GLint matloc =
          glGetUniformLocation(texture_2D_shader_program, "TransformMatrix");
      glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)Q);

      // Select the active texture unit.
      glActiveTexture(GL_TEXTURE0);

// For some reason, glDrawElements is busted on Android
// So we do this a hard ugly way, drawing two triangles...
#if 0
            GLushort indices1[] = {0,1,3,2};
            glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices1);
#else

      float co1[8];
      co1[0] = coords[0];
      co1[1] = coords[1];
      co1[2] = coords[2];
      co1[3] = coords[3];
      co1[4] = coords[6];
      co1[5] = coords[7];
      co1[6] = coords[4];
      co1[7] = coords[5];

      float tco1[8];
      tco1[0] = uv[0];
      tco1[1] = uv[1];
      tco1[2] = uv[2];
      tco1[3] = uv[3];
      tco1[4] = uv[6];
      tco1[5] = uv[7];
      tco1[6] = uv[4];
      tco1[7] = uv[5];

      glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, co1);
      glVertexAttribPointer(mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, tco1);

      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      glUseProgram(0);
#endif
#endif

#endif
      glDisable(GL_BLEND);
      glDisable(GL_TEXTURE_2D);

      glDeleteTextures(1, &texobj);
#endif
      delete[] data;
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

    if (m_buseTex) {
#ifdef ocpnUSE_GL
      m_texfont.Build(f);  // make sure the font is ready
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
