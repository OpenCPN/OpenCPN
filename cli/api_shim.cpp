/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Plugin API shim providing linkage but no functionality.
 * Author:   Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2021 Alec Leamas                                        *
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
#ifdef __WINDOWS__
#include <windows.h>
#endif

#include <wx/wx.h>
#include <wx/aui/framemanager.h>
#include <wx/bitmap.h>
#include <wx/fileconf.h>
#include <wx/font.h>
#include <wx/string.h>
#include <wx/window.h>

#include "config.h"
#include "ocpn_plugin.h"
#include "model/base_platform.h"

extern BasePlatform *g_BasePlatform;

class GLUtesselator;
extern "C" {
typedef void (*_GLUfuncptr)();
DECL_EXP GLUtesselator *gluNewTess(void) { return 0; }
DECL_EXP void gluTessProperty(GLUtesselator *tess, unsigned which,
                              double data) {}
DECL_EXP void gluTessEndPolygon(GLUtesselator *tess) {}
DECL_EXP void gluTessBeginPolygon(GLUtesselator *tess, void *data) {}
DECL_EXP void gluTessNormal(GLUtesselator *tess, double X, double Y, double Z) {
}
DECL_EXP void gluTessVertex(GLUtesselator *tess, double *location, void *data) {
}
DECL_EXP void gluTessCallback(GLUtesselator *tess, unsigned which,
                              _GLUfuncptr CallBackFunc) {}
DECL_EXP const unsigned char *gluErrorString(unsigned error) {
  return reinterpret_cast<const unsigned char *>("");
}
DECL_EXP void gluDeleteTess(GLUtesselator *tess) {}
DECL_EXP void gluTessBeginContour(GLUtesselator *tess) {}
DECL_EXP void gluTessEndContour(GLUtesselator *tess) {}
DECL_EXP void glLineWidth(GLfloat width) {}

DECL_EXP void glClearIndex(GLfloat c) {}

DECL_EXP void glClearColor(GLclampf red, GLclampf green, GLclampf blue,
                           GLclampf alpha) {}

DECL_EXP void glClear(GLbitfield mask) {}

DECL_EXP void glIndexMask(GLbitfield mask) {}

DECL_EXP void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {}

DECL_EXP void glAlphaFunc(GLenum func, GLclampf ref) {}

DECL_EXP void glBlendFunc(GLenum sfactor, GLenum dfactor) {}

DECL_EXP void glLogicOp(GLenum opcode) {}

DECL_EXP void glCullFace(GLenum mode) {}

DECL_EXP void glFrontFace(GLenum mode) {}

DECL_EXP void glPointSize(GLfloat size) {}

DECL_EXP void glLineStipple(GLint factor, GLushort pattern) {}

DECL_EXP void glPolygonMode(GLenum face, GLenum mode) {}

DECL_EXP void glPolygonOffset(GLfloat factor, GLfloat units) {}

DECL_EXP void glPolygonStipple(const GLubyte *mask) {}

DECL_EXP void glGetPolygonStipple(GLubyte *mask) {}

DECL_EXP void glEdgeFlag(GLboolean flag) {}

DECL_EXP void glEdgeFlagv(const GLboolean *flag) {}

DECL_EXP void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {}

DECL_EXP void glClipPlane(GLenum plane, const GLdouble *equation) {}

DECL_EXP void glGetClipPlane(GLenum plane, GLdouble *equation) {}

DECL_EXP void glDrawBuffer(GLenum mode) {}

DECL_EXP void glReadBuffer(GLenum mode) {}

DECL_EXP void glEnable(GLenum cap) {}

DECL_EXP void glDisable(GLenum cap) {}

DECL_EXP GLboolean glIsEnabled(GLenum cap) { return true; }

DECL_EXP void glEnableClientState(GLenum cap) {} /* 1.1 */

DECL_EXP void glDisableClientState(GLenum cap) {} /* 1.1 */

DECL_EXP void glGetBooleanv(GLenum pname, GLboolean *params) {}

DECL_EXP void glGetDoublev(GLenum pname, GLdouble *params) {}

DECL_EXP void glGetFloatv(GLenum pname, GLfloat *params) {}

DECL_EXP void glGetIntegerv(GLenum pname, GLint *params) {}

DECL_EXP void glPushAttrib(GLbitfield mask) {}

DECL_EXP void glPopAttrib(void) {}

DECL_EXP void glPushClientAttrib(GLbitfield mask) {} /* 1.1 */

DECL_EXP void glPopClientAttrib(void) {} /* 1.1 */

DECL_EXP GLint glRenderMode(GLenum mode) { return 0; }

DECL_EXP GLenum glGetError(void) { return 0; }

DECL_EXP const GLubyte *glGetString(GLenum name) {
  return reinterpret_cast<const GLubyte *>("");
}

DECL_EXP void glFinish(void) {}

DECL_EXP void glFlush(void) {}

DECL_EXP void glHint(GLenum target, GLenum mode) {}

/*
 * Depth Buffer
 */

DECL_EXP void glClearDepth(GLclampd depth) {}

DECL_EXP void glDepthFunc(GLenum func) {}

DECL_EXP void glDepthMask(GLboolean flag) {}

DECL_EXP void glDepthRange(GLclampd near_val, GLclampd far_val) {}

/*
 * Accumulation Buffer
 */

DECL_EXP void glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {}

DECL_EXP void glAccum(GLenum op, GLfloat value) {}

/*
 * Transformation
 */

DECL_EXP void glMatrixMode(GLenum mode) {}

DECL_EXP void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
                      GLdouble near_val, GLdouble far_val) {}

DECL_EXP void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
                        GLdouble near_val, GLdouble far_val) {}

DECL_EXP void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {}

DECL_EXP void glPushMatrix(void) {}

DECL_EXP void glPopMatrix(void) {}

DECL_EXP void glLoadIdentity(void) {}

DECL_EXP void glLoadMatrixd(const GLdouble *m) {}
DECL_EXP void glLoadMatrixf(const GLfloat *m) {}

DECL_EXP void glMultMatrixd(const GLdouble *m) {}
DECL_EXP void glMultMatrixf(const GLfloat *m) {}

DECL_EXP void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {}
DECL_EXP void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {}

DECL_EXP void glScaled(GLdouble x, GLdouble y, GLdouble z) {}
DECL_EXP void glScalef(GLfloat x, GLfloat y, GLfloat z) {}

DECL_EXP void glTranslated(GLdouble x, GLdouble y, GLdouble z) {}
DECL_EXP void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {}

/*
 * Display Lists
 */

DECL_EXP GLboolean glIsList(GLuint list) { return true; }

DECL_EXP void glDeleteLists(GLuint list, GLsizei range) {}

DECL_EXP GLuint glGenLists(GLsizei range) { return 0; }

DECL_EXP void glNewList(GLuint list, GLenum mode) {}

DECL_EXP void glEndList(void) {}

DECL_EXP void glCallList(GLuint list) {}

DECL_EXP void glCallLists(GLsizei n, GLenum type, const GLvoid *lists) {}

DECL_EXP void glListBase(GLuint base) {}

/*
 * Drawing Functions
 */

DECL_EXP void glBegin(GLenum mode) {}

DECL_EXP void glEnd(void) {}

DECL_EXP void glVertex2d(GLdouble x, GLdouble y) {}
DECL_EXP void glVertex2f(GLfloat x, GLfloat y) {}
DECL_EXP void glVertex2i(GLint x, GLint y) {}
DECL_EXP void glVertex2s(GLshort x, GLshort y) {}

DECL_EXP void glVertex3d(GLdouble x, GLdouble y, GLdouble z) {}
DECL_EXP void glVertex3f(GLfloat x, GLfloat y, GLfloat z) {}
DECL_EXP void glVertex3i(GLint x, GLint y, GLint z) {}
DECL_EXP void glVertex3s(GLshort x, GLshort y, GLshort z) {}

DECL_EXP void glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) {}
DECL_EXP void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {}
DECL_EXP void glVertex4i(GLint x, GLint y, GLint z, GLint w) {}
DECL_EXP void glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w) {}

DECL_EXP void glVertex2dv(const GLdouble *v) {}
DECL_EXP void glVertex2fv(const GLfloat *v) {}
DECL_EXP void glVertex2iv(const GLint *v) {}
DECL_EXP void glVertex2sv(const GLshort *v) {}

DECL_EXP void glVertex3dv(const GLdouble *v) {}
DECL_EXP void glVertex3fv(const GLfloat *v) {}
DECL_EXP void glVertex3iv(const GLint *v) {}
DECL_EXP void glVertex3sv(const GLshort *v) {}

DECL_EXP void glVertex4dv(const GLdouble *v) {}
DECL_EXP void glVertex4fv(const GLfloat *v) {}
DECL_EXP void glVertex4iv(const GLint *v) {}
DECL_EXP void glVertex4sv(const GLshort *v) {}

DECL_EXP void glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz) {
}
DECL_EXP void glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz) {}
DECL_EXP void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) {}
DECL_EXP void glNormal3i(GLint nx, GLint ny, GLint nz) {}
DECL_EXP void glNormal3s(GLshort nx, GLshort ny, GLshort nz) {}

DECL_EXP void glNormal3bv(const GLbyte *v) {}
DECL_EXP void glNormal3dv(const GLdouble *v) {}
DECL_EXP void glNormal3fv(const GLfloat *v) {}
DECL_EXP void glNormal3iv(const GLint *v) {}
DECL_EXP void glNormal3sv(const GLshort *v) {}

DECL_EXP void glIndexd(GLdouble c) {}
DECL_EXP void glIndexf(GLfloat c) {}
DECL_EXP void glIndexi(GLint c) {}
DECL_EXP void glIndexs(GLshort c) {}
DECL_EXP void glIndexub(GLubyte c) {} /* 1.1 */

DECL_EXP void glIndexdv(const GLdouble *c) {}
DECL_EXP void glIndexfv(const GLfloat *c) {}
DECL_EXP void glIndexiv(const GLint *c) {}
DECL_EXP void glIndexsv(const GLshort *c) {}
DECL_EXP void glIndexubv(const GLubyte *c) {} /* 1.1 */

DECL_EXP void glColor3b(GLbyte red, GLbyte green,
                        GLbyte blue) {}
DECL_EXP void glColor3d(GLdouble red, GLdouble green, GLdouble blue) {}
DECL_EXP void glColor3f(GLfloat red, GLfloat green, GLfloat blue) {}
DECL_EXP void glColor3i(GLint red, GLint green, GLint blue) {}
DECL_EXP void glColor3s(GLshort red, GLshort green, GLshort blue) {}
DECL_EXP void glColor3ub(GLubyte red, GLubyte green,
                         GLubyte blue) {}
DECL_EXP void glColor3ui(GLuint red, GLuint green, GLuint blue) {}
DECL_EXP void glColor3us(GLushort red, GLushort green, GLushort blue) {}

DECL_EXP void glColor4b(GLbyte red, GLbyte green,
                        GLbyte blue, GLbyte alpha) {}
DECL_EXP void glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha) {}
DECL_EXP void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {}
DECL_EXP void glColor4i(GLint red, GLint green, GLint blue, GLint alpha) {}
DECL_EXP void glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha) {}
DECL_EXP void glColor4ub(GLubyte red, GLubyte green,
                         GLubyte blue, GLubyte alpha) {}
DECL_EXP void glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha) {}
DECL_EXP void glColor4us(GLushort red, GLushort green, GLushort blue,
                         GLushort alpha) {}

DECL_EXP void glColor3bv(const GLbyte *v) {}
DECL_EXP void glColor3dv(const GLdouble *v) {}
DECL_EXP void glColor3fv(const GLfloat *v) {}
DECL_EXP void glColor3iv(const GLint *v) {}
DECL_EXP void glColor3sv(const GLshort *v) {}
DECL_EXP void glColor3ubv(const GLubyte *v) {}
DECL_EXP void glColor3uiv(const GLuint *v) {}
DECL_EXP void glColor3usv(const GLushort *v) {}

DECL_EXP void glColor4bv(const GLbyte *v) {}
DECL_EXP void glColor4dv(const GLdouble *v) {}
DECL_EXP void glColor4fv(const GLfloat *v) {}
DECL_EXP void glColor4iv(const GLint *v) {}
DECL_EXP void glColor4sv(const GLshort *v) {}
DECL_EXP void glColor4ubv(const GLubyte *v) {}
DECL_EXP void glColor4uiv(const GLuint *v) {}
DECL_EXP void glColor4usv(const GLushort *v) {}

DECL_EXP void glTexCoord1d(GLdouble s) {}
DECL_EXP void glTexCoord1f(GLfloat s) {}
DECL_EXP void glTexCoord1i(GLint s) {}
DECL_EXP void glTexCoord1s(GLshort s) {}

DECL_EXP void glTexCoord2d(GLdouble s, GLdouble t) {}
DECL_EXP void glTexCoord2f(GLfloat s, GLfloat t) {}
DECL_EXP void glTexCoord2i(GLint s, GLint t) {}
DECL_EXP void glTexCoord2s(GLshort s, GLshort t) {}

DECL_EXP void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r) {}
DECL_EXP void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r) {}
DECL_EXP void glTexCoord3i(GLint s, GLint t, GLint r) {}
DECL_EXP void glTexCoord3s(GLshort s, GLshort t, GLshort r) {}

DECL_EXP void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q) {}
DECL_EXP void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q) {}
DECL_EXP void glTexCoord4i(GLint s, GLint t, GLint r, GLint q) {}
DECL_EXP void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q) {}

DECL_EXP void glTexCoord1dv(const GLdouble *v) {}
DECL_EXP void glTexCoord1fv(const GLfloat *v) {}
DECL_EXP void glTexCoord1iv(const GLint *v) {}
DECL_EXP void glTexCoord1sv(const GLshort *v) {}

DECL_EXP void glTexCoord2dv(const GLdouble *v) {}
DECL_EXP void glTexCoord2fv(const GLfloat *v) {}
DECL_EXP void glTexCoord2iv(const GLint *v) {}
DECL_EXP void glTexCoord2sv(const GLshort *v) {}

DECL_EXP void glTexCoord3dv(const GLdouble *v) {}
DECL_EXP void glTexCoord3fv(const GLfloat *v) {}
DECL_EXP void glTexCoord3iv(const GLint *v) {}
DECL_EXP void glTexCoord3sv(const GLshort *v) {}

DECL_EXP void glTexCoord4dv(const GLdouble *v) {}
DECL_EXP void glTexCoord4fv(const GLfloat *v) {}
DECL_EXP void glTexCoord4iv(const GLint *v) {}
DECL_EXP void glTexCoord4sv(const GLshort *v) {}

DECL_EXP void glRasterPos2d(GLdouble x, GLdouble y) {}
DECL_EXP void glRasterPos2f(GLfloat x, GLfloat y) {}
DECL_EXP void glRasterPos2i(GLint x, GLint y) {}
DECL_EXP void glRasterPos2s(GLshort x, GLshort y) {}

DECL_EXP void glRasterPos3d(GLdouble x, GLdouble y, GLdouble z) {}
DECL_EXP void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z) {}
DECL_EXP void glRasterPos3i(GLint x, GLint y, GLint z) {}
DECL_EXP void glRasterPos3s(GLshort x, GLshort y, GLshort z) {}

DECL_EXP void glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) {}
DECL_EXP void glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {}
DECL_EXP void glRasterPos4i(GLint x, GLint y, GLint z, GLint w) {}
DECL_EXP void glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w) {}

DECL_EXP void glRasterPos2dv(const GLdouble *v) {}
DECL_EXP void glRasterPos2fv(const GLfloat *v) {}
DECL_EXP void glRasterPos2iv(const GLint *v) {}
DECL_EXP void glRasterPos2sv(const GLshort *v) {}

DECL_EXP void glRasterPos3dv(const GLdouble *v) {}
DECL_EXP void glRasterPos3fv(const GLfloat *v) {}
DECL_EXP void glRasterPos3iv(const GLint *v) {}
DECL_EXP void glRasterPos3sv(const GLshort *v) {}

DECL_EXP void glRasterPos4dv(const GLdouble *v) {}
DECL_EXP void glRasterPos4fv(const GLfloat *v) {}
DECL_EXP void glRasterPos4iv(const GLint *v) {}
DECL_EXP void glRasterPos4sv(const GLshort *v) {}

DECL_EXP void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2) {}
DECL_EXP void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {}
DECL_EXP void glRecti(GLint x1, GLint y1, GLint x2, GLint y2) {}
DECL_EXP void glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2) {}

DECL_EXP void glRectdv(const GLdouble *v1, const GLdouble *v2) {}
DECL_EXP void glRectfv(const GLfloat *v1, const GLfloat *v2) {}
DECL_EXP void glRectiv(const GLint *v1, const GLint *v2) {}
DECL_EXP void glRectsv(const GLshort *v1, const GLshort *v2) {}

DECL_EXP void glVertexPointer(GLint size, GLenum type, GLsizei stride,
                              const GLvoid *ptr) {}

DECL_EXP void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *ptr) {}

DECL_EXP void glColorPointer(GLint size, GLenum type, GLsizei stride,
                             const GLvoid *ptr) {}

DECL_EXP void glIndexPointer(GLenum type, GLsizei stride, const GLvoid *ptr) {}

DECL_EXP void glTexCoordPointer(GLint size, GLenum type, GLsizei stride,
                                const GLvoid *ptr) {}

DECL_EXP void glEdgeFlagPointer(GLsizei stride, const GLvoid *ptr) {}

DECL_EXP void glGetPointerv(GLenum pname, void **params) {}

DECL_EXP void glArrayElement(GLint i) {}

DECL_EXP void glDrawArrays(GLenum mode, GLint first, GLsizei count) {}

DECL_EXP void glDrawElements(GLenum mode, GLsizei count, GLenum type,
                             const GLvoid *indices) {}

DECL_EXP void glInterleavedArrays(GLenum format, GLsizei stride,
                                  const GLvoid *pointer) {}

/*
 * Lighting
 */

DECL_EXP void glShadeModel(GLenum mode) {}

DECL_EXP void glLightf(GLenum light, GLenum pname, GLfloat param) {}
DECL_EXP void glLighti(GLenum light, GLenum pname, GLint param) {}
DECL_EXP void glLightfv(GLenum light, GLenum pname, const GLfloat *params) {}
DECL_EXP void glLightiv(GLenum light, GLenum pname, const GLint *params) {}

DECL_EXP void glGetLightfv(GLenum light, GLenum pname, GLfloat *params) {}
DECL_EXP void glGetLightiv(GLenum light, GLenum pname, GLint *params) {}

DECL_EXP void glLightModelf(GLenum pname, GLfloat param) {}
DECL_EXP void glLightModeli(GLenum pname, GLint param) {}
DECL_EXP void glLightModelfv(GLenum pname, const GLfloat *params) {}
DECL_EXP void glLightModeliv(GLenum pname, const GLint *params) {}

DECL_EXP void glMaterialf(GLenum face, GLenum pname, GLfloat param) {}
DECL_EXP void glMateriali(GLenum face, GLenum pname, GLint param) {}
DECL_EXP void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params) {
}
DECL_EXP void glMaterialiv(GLenum face, GLenum pname, const GLint *params) {}

DECL_EXP void glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params) {}
DECL_EXP void glGetMaterialiv(GLenum face, GLenum pname, GLint *params) {}

DECL_EXP void glColorMaterial(GLenum face, GLenum mode) {}

/*
 * Raster functions
 */

DECL_EXP void glPixelZoom(GLfloat xfactor, GLfloat yfactor) {}

DECL_EXP void glPixelStoref(GLenum pname, GLfloat param) {}
DECL_EXP void glPixelStorei(GLenum pname, GLint param) {}

DECL_EXP void glPixelTransferf(GLenum pname, GLfloat param) {}
DECL_EXP void glPixelTransferi(GLenum pname, GLint param) {}

DECL_EXP void glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat *values) {}
DECL_EXP void glPixelMapuiv(GLenum map, GLsizei mapsize,
                            const GLuint *values) {}
DECL_EXP void glPixelMapusv(GLenum map, GLsizei mapsize,
                            const GLushort *values) {}

DECL_EXP void glGetPixelMapfv(GLenum map, GLfloat *values) {}
DECL_EXP void glGetPixelMapuiv(GLenum map, GLuint *values) {}
DECL_EXP void glGetPixelMapusv(GLenum map, GLushort *values) {}

DECL_EXP void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig,
                       GLfloat xmove, GLfloat ymove, const GLubyte *bitmap) {}

DECL_EXP void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                           GLenum format, GLenum type, void *pixels) {}

DECL_EXP void glDrawPixels(GLsizei width, GLsizei height, GLenum format,
                           GLenum type, const GLvoid *pixels) {}

DECL_EXP void glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                           GLenum type) {}

/*
 * Stenciling
 */

DECL_EXP void glStencilFunc(GLenum func, GLint ref, GLuint mask) {}

DECL_EXP void glStencilMask(GLuint mask) {}

DECL_EXP void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {}

DECL_EXP void glClearStencil(GLint s) {}

/*
 * Texture mapping
 */

DECL_EXP void glTexGend(GLenum coord, GLenum pname, GLdouble param) {}
DECL_EXP void glTexGenf(GLenum coord, GLenum pname, GLfloat param) {}
DECL_EXP void glTexGeni(GLenum coord, GLenum pname, GLint param) {}

DECL_EXP void glTexGendv(GLenum coord, GLenum pname, const GLdouble *params) {
}
DECL_EXP void glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params) {}
DECL_EXP void glTexGeniv(GLenum coord, GLenum pname, const GLint *params) {}

DECL_EXP void glGetTexGendv(GLenum coord, GLenum pname, GLdouble *params) {}
DECL_EXP void glGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params) {}
DECL_EXP void glGetTexGeniv(GLenum coord, GLenum pname, GLint *params) {}

DECL_EXP void glTexEnvf(GLenum target, GLenum pname, GLfloat param) {}
DECL_EXP void glTexEnvi(GLenum target, GLenum pname, GLint param) {}

DECL_EXP void glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params) {
}
DECL_EXP void glTexEnviv(GLenum target, GLenum pname, const GLint *params) {}

DECL_EXP void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params) {}
DECL_EXP void glGetTexEnviv(GLenum target, GLenum pname, GLint *params) {}

DECL_EXP void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {}
DECL_EXP void glTexParameteri(GLenum target, GLenum pname, GLint param) {}

DECL_EXP void glTexParameterfv(GLenum target, GLenum pname,
                               const GLfloat *params) {}
DECL_EXP void glTexParameteriv(GLenum target, GLenum pname,
                               const GLint *params) {}

DECL_EXP void glGetTexParameterfv(GLenum target, GLenum pname,
                                  GLfloat *params) {}
DECL_EXP void glGetTexParameteriv(GLenum target, GLenum pname,
                                  GLint *params) {}

DECL_EXP void glGetTexLevelParameterfv(GLenum target, GLint level,
                                       GLenum pname, GLfloat *params) {}
DECL_EXP void glGetTexLevelParameteriv(GLenum target, GLint level,
                                       GLenum pname, GLint *params) {}

DECL_EXP void glTexImage1D(GLenum target, GLint level, GLint internalFormat,
                           GLsizei width, GLint border, GLenum format,
                           GLenum type, const GLvoid *pixels) {}

DECL_EXP void glTexImage2D(GLenum target, GLint level, GLint internalFormat,
                           GLsizei width, GLsizei height, GLint border,
                           GLenum format, GLenum type, const GLvoid *pixels) {
}

DECL_EXP void glGetTexImage(GLenum target, GLint level, GLenum format,
                            GLenum type, void *pixels) {}

/* 1.1 functions */

DECL_EXP void glGenTextures(GLsizei n, GLuint *textures) {}

DECL_EXP void glDeleteTextures(GLsizei n, const GLuint *textures) {}

DECL_EXP void glBindTexture(GLenum target, GLenum) {}

DECL_EXP void glPrioritizeTextures(GLsizei n, const GLuint *textures,
                                   const GLclampf *priorities) {}

DECL_EXP GLboolean glAreTexturesResident(GLsizei n, const GLuint *textures,
                                    GLboolean *residences) {
  return true;
}

DECL_EXP GLboolean glIsTexture(GLenum) { return true; }

DECL_EXP void glTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                              GLsizei width, GLenum format, GLenum type,
                              const GLvoid *pixels) {}

DECL_EXP void glTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                              GLint yoffset, GLsizei width, GLsizei height,
                              GLenum format, GLenum type,
                              const GLvoid *pixels) {}

DECL_EXP void glCopyTexImage1D(GLenum target, GLint level,
                               GLenum internalformat, GLint x, GLint y,
                               GLsizei width, GLint border) {}

DECL_EXP void glCopyTexImage2D(GLenum target, GLint level,
                               GLenum internalformat, GLint x, GLint y,
                               GLsizei width, GLsizei height, GLint border) {}

DECL_EXP void glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                                  GLint x, GLint y, GLsizei width) {}

DECL_EXP void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                                  GLint yoffset, GLint x, GLint y, GLsizei width,
                                  GLsizei height) {}

/*
 * Evaluators
 */

DECL_EXP void glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride,
                      GLint order, const GLdouble *points) {}
DECL_EXP void glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride,
                      GLint order, const GLfloat *points) {}

DECL_EXP void glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride,
                      GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
                      const GLdouble *points) {}
DECL_EXP void glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride,
                      GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
                      const GLfloat *points) {}

DECL_EXP void glGetMapdv(GLenum target, GLenum query, GLdouble *v) {}
DECL_EXP void glGetMapfv(GLenum target, GLenum query, GLfloat *v) {}
DECL_EXP void glGetMapiv(GLenum target, GLenum query, GLint *v) {}

DECL_EXP void glEvalCoord1d(GLdouble u) {}
DECL_EXP void glEvalCoord1f(GLfloat u) {}

DECL_EXP void glEvalCoord1dv(const GLdouble *u) {}
DECL_EXP void glEvalCoord1fv(const GLfloat *u) {}

DECL_EXP void glEvalCoord2d(GLdouble u, GLdouble v) {}
DECL_EXP void glEvalCoord2f(GLfloat u, GLfloat v) {}

DECL_EXP void glEvalCoord2dv(const GLdouble *u) {}
DECL_EXP void glEvalCoord2fv(const GLfloat *u) {}

DECL_EXP void glMapGrid1d(GLint un, GLdouble u1, GLdouble u2) {}
DECL_EXP void glMapGrid1f(GLint un, GLfloat u1, GLfloat u2) {}

DECL_EXP void glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1,
                          GLdouble v2) {}
DECL_EXP void glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1,
                          GLfloat v2) {}

DECL_EXP void glEvalPoint1(GLint i) {}

DECL_EXP void glEvalPoint2(GLint i, GLint j) {}

DECL_EXP void glEvalMesh1(GLenum mode, GLint i1, GLint i2) {}

DECL_EXP void glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2) {}

/*
 * Fog
 */

DECL_EXP void glFogf(GLenum pname, GLfloat param) {}

DECL_EXP void glFogi(GLenum pname, GLint param) {}

DECL_EXP void glFogfv(GLenum pname, const GLfloat *params) {}

DECL_EXP void glFogiv(GLenum pname, const GLint *params) {}

/*
 * Selection and Feedback
 */

DECL_EXP void glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer) {}

DECL_EXP void glPassThrough(GLfloat token) {}

DECL_EXP void glSelectBuffer(GLsizei size, GLuint *buffer) {}

DECL_EXP void glInitNames(void) {}

DECL_EXP void glLoadName(GLuint name) {}

DECL_EXP void glPushName(GLuint name) {}

DECL_EXP void glPopName(void) {}

/*
 * OpenGL 1.2
 */

DECL_EXP void glDrawRangeElements(GLenum mode, GLuint start, GLuint end,
                                  GLsizei count, GLenum type,
                                  const GLvoid *indices) {}

DECL_EXP void glTexImage3D(GLenum target, GLint level, GLint internalFormat,
                           GLsizei width, GLsizei height, GLsizei depth,
                           GLint border, GLenum format, GLenum type,
                           const GLvoid *pixels) {}

DECL_EXP void glTexSubImage3D(GLenum target, GLint level, GLint xoffset,
                              GLint yoffset, GLint zoffset, GLsizei width,
                              GLsizei height, GLsizei depth, GLenum format,
                              GLenum type, const GLvoid *pixels) {}

DECL_EXP void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset,
                                  GLint yoffset, GLint zoffset, GLint x, GLint y,
                                  GLsizei width, GLsizei height) {}

DECL_EXP void glColorTable(GLenum target, GLenum internalformat,
                           GLsizei width, GLenum format, GLenum type,
                           const GLvoid *table) {}

DECL_EXP void glColorSubTable(GLenum target, GLsizei start, GLsizei count,
                              GLenum format, GLenum type,
                              const GLvoid *data) {}

DECL_EXP void glColorTableParameteriv(GLenum target, GLenum pname,
                                      const GLint *params) {}

DECL_EXP void glColorTableParameterfv(GLenum target, GLenum pname,
                                      const GLfloat *params) {}

DECL_EXP void glCopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y,
                                  GLsizei width) {}

DECL_EXP void glCopyColorTable(GLenum target, GLenum internalformat, GLint x,
                               GLint y, GLsizei width) {}

DECL_EXP void glGetColorTable(GLenum target, GLenum format, GLenum type,
                              void *table) {}

DECL_EXP void glGetColorTableParameterfv(GLenum target, GLenum pname,
                                         GLfloat *params) {}

DECL_EXP void glGetColorTableParameteriv(GLenum target, GLenum pname,
                                         GLint *params) {}

DECL_EXP void glBlendEquation(GLenum mode) {}

DECL_EXP void glBlendColor(GLclampf red, GLclampf green, GLclampf blue,
                           GLclampf alpha) {}

DECL_EXP void glHistogram(GLenum target, GLsizei width,
                          GLenum internalformat, GLboolean sink) {}

DECL_EXP void glResetHistogram(GLenum target) {}

DECL_EXP void glGetHistogram(GLenum target, GLboolean reset, GLenum format,
                             GLenum type, void *values) {}

DECL_EXP void glGetHistogramParameterfv(GLenum target, GLenum pname,
                                        GLfloat *params) {}

DECL_EXP void glGetHistogramParameteriv(GLenum target, GLenum pname,
                                        GLint *params) {}

DECL_EXP void glMinmax(GLenum target, GLenum internalformat, GLboolean sink) {}

DECL_EXP void glResetMinmax(GLenum target) {}

DECL_EXP void glGetMinmax(GLenum target, GLboolean reset, GLenum format,
                          GLenum types, GLvoid *values) {}

DECL_EXP void glGetMinmaxParameterfv(GLenum target, GLenum pname,
                                     GLfloat *params) {}

DECL_EXP void glGetMinmaxParameteriv(GLenum target, GLenum pname,
                                     GLint *params) {}

DECL_EXP void glConvolutionFilter1D(GLenum target, GLenum internalformat,
                                    GLsizei width, GLenum format,
                                    GLenum type, const GLvoid *image) {}

DECL_EXP void glConvolutionFilter2D(GLenum target, GLenum internalformat,
                                    GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    const GLvoid *image) {}

DECL_EXP void glConvolutionParameterf(GLenum target, GLenum pname,
                                      GLfloat params) {}

DECL_EXP void glConvolutionParameterfv(GLenum target, GLenum pname,
                                       const GLfloat *params) {}

DECL_EXP void glConvolutionParameteri(GLenum target, GLenum pname,
                                      GLint params) {}

DECL_EXP void glConvolutionParameteriv(GLenum target, GLenum pname,
                                       const GLint *params) {}

DECL_EXP void glCopyConvolutionFilter1D(GLenum target,
                                        GLenum internalformat, GLint x, GLint y,
                                        GLsizei width) {}

DECL_EXP void glCopyConvolutionFilter2D(GLenum target,
                                        GLenum internalformat, GLint x, GLint y,
                                        GLsizei width, GLsizei height) {}

DECL_EXP void glGetConvolutionFilter(GLenum target, GLenum format,
                                     GLenum type, void *image) {}

DECL_EXP void glGetConvolutionParameterfv(GLenum target, GLenum pname,
                                          GLfloat *params) {}

DECL_EXP void glGetConvolutionParameteriv(GLenum target, GLenum pname,
                                          GLint *params) {}

DECL_EXP void glSeparableFilter2D(GLenum target, GLenum internalformat,
                                  GLsizei width, GLsizei height, GLenum format,
                                  GLenum type, const GLvoid *row,
                                  const GLvoid *column) {}

DECL_EXP void glGetSeparableFilter(GLenum target, GLenum format,
                                   GLenum type, void *row, void *column,
                                   void *span) {}

/*
 * OpenGL 1.3
 */

/* multitexture */

DECL_EXP void glActiveTexture(GLenum) {}

DECL_EXP void glClientActiveTexture(GLenum) {}

DECL_EXP void glCompressedTexImage1D(GLenum target, GLint level,
                                     GLenum internalformat, GLsizei width,
                                     GLint border, GLsizei imageSize,
                                     const GLvoid *data) {}

DECL_EXP void glCompressedTexImage2D(GLenum target, GLint level,
                                     GLenum internalformat, GLsizei width,
                                     GLsizei height, GLint border,
                                     GLsizei imageSize, const GLvoid *data) {}

DECL_EXP void glCompressedTexImage3D(GLenum target, GLint level,
                                     GLenum internalformat, GLsizei width,
                                     GLsizei height, GLsizei depth, GLint border,
                                     GLsizei imageSize, const GLvoid *data) {}

DECL_EXP void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                                        GLsizei width, GLenum format,
                                        GLsizei imageSize, const GLvoid *data) {}

DECL_EXP void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                                        GLint yoffset, GLsizei width,
                                        GLsizei height, GLenum format,
                                        GLsizei imageSize, const GLvoid *data) {}

DECL_EXP void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset,
                                        GLint yoffset, GLint zoffset, GLsizei width,
                                        GLsizei height, GLsizei depth,
                                        GLenum format, GLsizei imageSize,
                                        const GLvoid *data) {}

DECL_EXP void glGetCompressedTexImage(GLenum target, GLint lod, void *img) {}

DECL_EXP void glMultiTexCoord1d(GLenum target, GLdouble s) {}

DECL_EXP void glMultiTexCoord1dv(GLenum target, const GLdouble *v) {}

DECL_EXP void glMultiTexCoord1f(GLenum target, GLfloat s) {}

DECL_EXP void glMultiTexCoord1fv(GLenum target, const GLfloat *v) {}

DECL_EXP void glMultiTexCoord1i(GLenum target, GLint s) {}

DECL_EXP void glMultiTexCoord1iv(GLenum target, const GLint *v) {}

DECL_EXP void glMultiTexCoord1s(GLenum target, GLshort s) {}

DECL_EXP void glMultiTexCoord1sv(GLenum target, const GLshort *v) {}

DECL_EXP void glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t) {}

DECL_EXP void glMultiTexCoord2dv(GLenum target, const GLdouble *v) {}

DECL_EXP void glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t) {}

DECL_EXP void glMultiTexCoord2fv(GLenum target, const GLfloat *v) {}

DECL_EXP void glMultiTexCoord2i(GLenum target, GLint s, GLint t) {}

DECL_EXP void glMultiTexCoord2iv(GLenum target, const GLint *v) {}

DECL_EXP void glMultiTexCoord2s(GLenum target, GLshort s, GLshort t) {}

DECL_EXP void glMultiTexCoord2sv(GLenum target, const GLshort *v) {}

DECL_EXP void glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r) {
}

DECL_EXP void glMultiTexCoord3dv(GLenum target, const GLdouble *v) {}

DECL_EXP void glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r) {}

DECL_EXP void glMultiTexCoord3fv(GLenum target, const GLfloat *v) {}

DECL_EXP void glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r) {}

DECL_EXP void glMultiTexCoord3iv(GLenum target, const GLint *v) {}

DECL_EXP void glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r) {}

DECL_EXP void glMultiTexCoord3sv(GLenum target, const GLshort *v) {}

DECL_EXP void glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r,
                                GLdouble q) {}

DECL_EXP void glMultiTexCoord4dv(GLenum target, const GLdouble *v) {}

DECL_EXP void glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r,
                                GLfloat q) {}

DECL_EXP void glMultiTexCoord4fv(GLenum target, const GLfloat *v) {}

DECL_EXP void glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q) {}

DECL_EXP void glMultiTexCoord4iv(GLenum target, const GLint *v) {}

DECL_EXP void glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r,
                                GLshort q) {}

DECL_EXP void glMultiTexCoord4sv(GLenum target, const GLshort *v) {}

DECL_EXP void glLoadTransposeMatrixd(const GLdouble m[16]) {}

DECL_EXP void glLoadTransposeMatrixf(const GLfloat m[16]) {}

DECL_EXP void glMultTransposeMatrixd(const GLdouble m[16]) {}

DECL_EXP void glMultTransposeMatrixf(const GLfloat m[16]) {}

DECL_EXP void glSampleCoverage(GLclampf value, GLboolean invert) {}

/*
 * GL_ARB_multitexture (ARB extension 1 and OpenGL 1.2.1)
 */
DECL_EXP void glActiveTextureARB(GLenum) {}
DECL_EXP void glClientActiveTextureARB(GLenum) {}
DECL_EXP void glMultiTexCoord1dARB(GLenum target, GLdouble s) {}
DECL_EXP void glMultiTexCoord1dvARB(GLenum target, const GLdouble *v) {}
DECL_EXP void glMultiTexCoord1fARB(GLenum target, GLfloat s) {}
DECL_EXP void glMultiTexCoord1fvARB(GLenum target, const GLfloat *v) {}
DECL_EXP void glMultiTexCoord1iARB(GLenum target, GLint s) {}
DECL_EXP void glMultiTexCoord1ivARB(GLenum target, const GLint *v) {}
DECL_EXP void glMultiTexCoord1sARB(GLenum target, GLshort s) {}
DECL_EXP void glMultiTexCoord1svARB(GLenum target, const GLshort *v) {}
DECL_EXP void glMultiTexCoord2dARB(GLenum target, GLdouble s, GLdouble t) {}
DECL_EXP void glMultiTexCoord2dvARB(GLenum target, const GLdouble *v) {}
DECL_EXP void glMultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t) {}
DECL_EXP void glMultiTexCoord2fvARB(GLenum target, const GLfloat *v) {}
DECL_EXP void glMultiTexCoord2iARB(GLenum target, GLint s, GLint t) {}
DECL_EXP void glMultiTexCoord2ivARB(GLenum target, const GLint *v) {}
DECL_EXP void glMultiTexCoord2sARB(GLenum target, GLshort s, GLshort t) {}
DECL_EXP void glMultiTexCoord2svARB(GLenum target, const GLshort *v) {}
DECL_EXP void glMultiTexCoord3dARB(GLenum target, GLdouble s, GLdouble t,
                                   GLdouble r) {}
DECL_EXP void glMultiTexCoord3dvARB(GLenum target, const GLdouble *v) {}
DECL_EXP void glMultiTexCoord3fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r) {
}
DECL_EXP void glMultiTexCoord3fvARB(GLenum target, const GLfloat *v) {}
DECL_EXP void glMultiTexCoord3iARB(GLenum target, GLint s, GLint t, GLint r) {}
DECL_EXP void glMultiTexCoord3ivARB(GLenum target, const GLint *v) {}
DECL_EXP void glMultiTexCoord3sARB(GLenum target, GLshort s, GLshort t, GLshort r) {
}
DECL_EXP void glMultiTexCoord3svARB(GLenum target, const GLshort *v) {}
DECL_EXP void glMultiTexCoord4dARB(GLenum target, GLdouble s, GLdouble t,
                                   GLdouble r, GLdouble q) {}
DECL_EXP void glMultiTexCoord4dvARB(GLenum target, const GLdouble *v) {}
DECL_EXP void glMultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r,
                                   GLfloat q) {}
DECL_EXP void glMultiTexCoord4fvARB(GLenum target, const GLfloat *v) {}
DECL_EXP void glMultiTexCoord4iARB(GLenum target, GLint s, GLint t, GLint r,
                                   GLint q) {}
DECL_EXP void glMultiTexCoord4ivARB(GLenum target, const GLint *v) {}
DECL_EXP void glMultiTexCoord4sARB(GLenum target, GLshort s, GLshort t, GLshort r,
                                   GLshort q) {}
DECL_EXP void glMultiTexCoord4svARB(GLenum target, const GLshort *v) {}

// DECL_EXP void  glProgramCallbackMESA(GLenum target, GLprogramcallbackMESA
// callback, void *data){}

DECL_EXP void glGetProgramRegisterfvMESA(GLenum target, GLsizei len,
                                         const GLubyte *name, GLfloat *v) {}

DECL_EXP void glBlendEquationSeparateATI(GLenum modeRGB, GLenum modeA) {}

}  // extern "C"

void RemovePlugInTool(int tool_id) {}
DECL_EXP int InsertPlugInToolSVG(wxString label, wxString SVGfile,
                                 wxString SVGfileRollover,
                                 wxString SVGfileToggled, wxItemKind kind,
                                 wxString shortHelp, wxString longHelp,
                                 wxObject *clientData, int position,
                                 int tool_sel, opencpn_plugin *pplugin) {
  return 0;
}

DECL_EXP int InsertPlugInTool(wxString label, wxBitmap *bitmap,
                              wxBitmap *bmpRollover, wxItemKind kind,
                              wxString shortHelp, wxString longHelp,
                              wxObject *clientData, int position, int tool_sel,
                              opencpn_plugin *pplugin) {
  return 0;
}
DECL_EXP void SetToolbarToolViz(int item, GLboolean viz) {}
void DimeWindow(wxWindow *win) {}
wxFont *OCPNGetFont(wxString TextElement, int default_size) { return 0; }
void SetToolbarItemState(int item, GLboolean toggle) {}
wxAuiManager *GetFrameAuiManager(void) { return 0; }
wxWindow *GetOCPNCanvasWindow() { return 0; }

DECL_EXP int AddCanvasContextMenuItem(wxMenuItem *pitem,
                                      opencpn_plugin *pplugin) {
  return 1;
}
DECL_EXP void SetToolbarToolBitmaps(int item, wxBitmap *bitmap,
                                    wxBitmap *bmpRollover) {}
DECL_EXP void SetToolbarToolBitmapsSVG(int item, wxString SVGfile,
                                       wxString SVGfileRollover,
                                       wxString SVGfileToggled) {}
DECL_EXP void RemoveCanvasContextMenuItem(int item) {}
DECL_EXP void JumpToPosition(GLdouble lat, GLdouble lon, GLdouble scale) {};
DECL_EXP void SetCanvasContextMenuItemViz(int item, GLboolean viz) {}
DECL_EXP void SetCanvasContextMenuItemGrey(int item, GLboolean grey) {}
DECL_EXP void RequestRefresh(wxWindow *) {}
DECL_EXP void DistanceBearingMercator_Plugin(GLdouble lat0, GLdouble lon0,
                                             GLdouble lat1, GLdouble lon1,
                                             GLdouble *brg, GLdouble *dist) {}
DECL_EXP void GetCanvasPixLL(PlugIn_ViewPort *vp, wxPoint *pp, GLdouble lat,
                             GLdouble lon) {}
DECL_EXP void GetCanvasLLPix(PlugIn_ViewPort *vp, wxPoint p, GLdouble *plat,
                             GLdouble *plon) {}
DECL_EXP void PushNMEABuffer(wxString str) {}
DECL_EXP void PositionBearingDistanceMercator_Plugin(GLdouble lat, GLdouble lon,
                                                     GLdouble brg, GLdouble dist,
                                                     GLdouble *dlat,
                                                     GLdouble *dlon) {}
DECL_EXP GLdouble DistGreatCircle_Plugin(GLdouble slat, GLdouble slon, GLdouble dlat,
                                       GLdouble dlon) {
  return 0.0;
}
wxXmlDocument dummy_xml_doc;
DECL_EXP wxXmlDocument GetChartDatabaseEntryXML(int dbIndex, GLboolean b_getGeom) {
  return dummy_xml_doc;
}
DECL_EXP bool UpdateChartDBInplace(wxArrayString dir_array, GLboolean b_force_update,
                                   GLboolean b_ProgressDialog) {
  return true;
}
wxArrayString dummy_array_string;
DECL_EXP wxArrayString GetChartDBDirArrayString() { return dummy_array_string; }
DECL_EXP void toTM_Plugin(GLfloat lat, GLfloat lon, GLfloat lat0, GLfloat lon0,
                          GLdouble *x, GLdouble *y) {}
DECL_EXP void fromTM_Plugin(GLdouble x, GLdouble y, GLdouble lat0, GLdouble lon0,
                            GLdouble *lat, GLdouble *lon) {}
DECL_EXP void toSM_Plugin(GLdouble lat, GLdouble lon, GLdouble lat0, GLdouble lon0,
                          GLdouble *x, GLdouble *y) {}
DECL_EXP void fromSM_Plugin(GLdouble x, GLdouble y, GLdouble lat0, GLdouble lon0,
                            GLdouble *lat, GLdouble *lon) {}
DECL_EXP void toSM_ECC_Plugin(GLdouble lat, GLdouble lon, GLdouble lat0, GLdouble lon0,
                              GLdouble *x, GLdouble *y) {}
DECL_EXP void fromSM_ECC_Plugin(GLdouble x, GLdouble y, GLdouble lat0, GLdouble lon0,
                                GLdouble *lat, GLdouble *lon) {}
DECL_EXP bool DecodeSingleVDOMessage(const wxString &str,
                                     PlugIn_Position_Fix_Ex *pos,
                                     wxString *acc) {
  return true;
}
DECL_EXP bool GetActiveRoutepointGPX(char *buffer, unsigned int buffer_length) {
  return true;
}
DECL_EXP int GetChartbarHeight(void) { return 1; }
void SendPluginMessage(wxString message_id, wxString message_body) {}
bool AddLocaleCatalog(wxString catalog) { return true; }
bool GetGlobalColor(wxString colorName, wxColour *pcolour) { return true; }
wxFileConfig *GetOCPNConfigObject(void) { return 0; }

DECL_EXP wxScrolledWindow *AddOptionsPage(OptionsParentPI parent,
                                          wxString title) {
  return 0;
}
DECL_EXP bool DeleteOptionsPage(wxScrolledWindow *page) { return true; }

DECL_EXP GLdouble toUsrDistance_Plugin(GLdouble nm_distance, int unit) {
  return 0.0;
}
DECL_EXP GLdouble fromUsrDistance_Plugin(GLdouble usr_distance, int unit) {
  return 0.0;
}
DECL_EXP GLdouble toUsrSpeed_Plugin(GLdouble kts_speed, int unit) { return 0.0; }
DECL_EXP GLdouble fromUsrSpeed_Plugin(GLdouble usr_speed, int unit) { return 0.0; }
DECL_EXP GLdouble toUsrTemp_Plugin(GLdouble cel_temp, int unit) { return 0.0; }
DECL_EXP GLdouble fromUsrTemp_Plugin(GLdouble usr_temp, int unit) { return 0.0; }
DECL_EXP wxString getUsrDistanceUnit_Plugin(int unit) { return ""; }
DECL_EXP wxString getUsrSpeedUnit_Plugin(int unit) { return ""; }
DECL_EXP wxString getUsrTempUnit_Plugin(int unit) { return ""; }
DECL_EXP wxString GetNewGUID() { return ""; }
DECL_EXP bool PlugIn_GSHHS_CrossesLand(GLdouble lat1, GLdouble lon1, GLdouble lat2,
                                       GLdouble lon2) {
  return true;
}

DECL_EXP void PlugInPlaySound(wxString &sound_file) {};

// API 1.10 Route and Waypoint Support
DECL_EXP wxBitmap *FindSystemWaypointIcon(wxString &icon_name) { return 0; }
DECL_EXP bool AddCustomWaypointIcon(wxBitmap *pimage, wxString key,
                                    wxString description) {
  return false;
}

DECL_EXP bool AddSingleWaypoint(PlugIn_Waypoint *pwaypoint, bool b_permanent) {
  return true;
}
DECL_EXP bool DeleteSingleWaypoint(wxString &GUID) { return true; }
DECL_EXP bool UpdateSingleWaypoint(PlugIn_Waypoint *pwaypoint) { return true; }

DECL_EXP bool AddPlugInRoute(PlugIn_Route *proute, bool b_permanent) {
  return true;
}
DECL_EXP bool DeletePlugInRoute(wxString &GUID) { return true; }
DECL_EXP bool UpdatePlugInRoute(PlugIn_Route *proute) { return true; }

DECL_EXP PlugIn_Track::PlugIn_Track() {}
DECL_EXP PlugIn_Track::~PlugIn_Track() {}
DECL_EXP bool AddPlugInTrack(PlugIn_Track *ptrack, bool b_permanent) {
  return true;
}
DECL_EXP bool DeletePlugInTrack(wxString &GUID) { return true; }
DECL_EXP bool UpdatePlugInTrack(PlugIn_Track *ptrack) { return true; }

DECL_EXP wxColour GetBaseGlobalColor(wxString colorName) { return *wxRED; }
int DECL_EXP OCPNMessageBox_PlugIn(wxWindow *parent, const wxString &message,
                                   const wxString &caption, int style, int x,
                                   int y) {
  return 0;
}

DECL_EXP wxString toSDMM_PlugIn(int NEflag, GLdouble a, bool hi_precision) {
  return "";
}
wxString dummy_string;
DECL_EXP wxString GetOCPN_ExePath(void) { return wxString(""); }
DECL_EXP wxString *GetpPlugInLocation() { return &dummy_string; }
DECL_EXP wxString GetPlugInPath(opencpn_plugin *pplugin) {
  return wxString("");
}

DECL_EXP int AddChartToDBInPlace(wxString &full_path, bool b_RefreshCanvas) {
  return 0;
}
DECL_EXP int RemoveChartFromDBInPlace(wxString &full_path) { return 0; }
DECL_EXP wxString GetLocaleCanonicalName() { return wxString(""); }

class PI_S57Obj;
PlugInChartBase::PlugInChartBase() {}
PlugInChartBase::~PlugInChartBase() {}

wxString PlugInChartBase::GetFileSearchMask(void) { return wxString(""); }

int PlugInChartBase::Init(const wxString &full_path, int init_flags) {
  return 0;
}
void PlugInChartBase::SetColorScheme(int cs, bool bApplyImmediate) {}

GLdouble PlugInChartBase::GetNormalScaleMin(GLdouble canvas_scale_factor,
                                          bool b_allow_overzoom) {
  return 0.0;
}
GLdouble PlugInChartBase::GetNormalScaleMax(GLdouble canvas_scale_factor,
                                          int canvas_width) {
  return 0.0;
}
GLdouble PlugInChartBase::GetNearestPreferredScalePPM(GLdouble target_scale_ppm) {
  return 0.0;
}

bool PlugInChartBase::GetChartExtent(ExtentPI *pext) { return true; }

wxBitmap dummy_bitmap;
wxBitmap &PlugInChartBase::RenderRegionView(const PlugIn_ViewPort &VPoint,
                                            const wxRegion &Region) {
  return dummy_bitmap;
}

bool PlugInChartBase::AdjustVP(PlugIn_ViewPort &vp_last,
                               PlugIn_ViewPort &vp_proposed) {
  return true;
}

void PlugInChartBase::GetValidCanvasRegion(const PlugIn_ViewPort &VPoint,
                                           wxRegion *pValidRegion) {}

wxBitmap *PlugInChartBase::GetThumbnail(int tnx, int tny, int cs) {
  return &dummy_bitmap;
}

void PlugInChartBase::ComputeSourceRectangle(const PlugIn_ViewPort &vp,
                                             wxRect *pSourceRect) {}
GLdouble PlugInChartBase::GetRasterScaleFactor() { return 0.0; }
bool PlugInChartBase::GetChartBits(wxRect &source, GLubyte *pPix,
                                   int sub_samp) {
  return true;
}
int PlugInChartBase::GetSize_X() { return 0; }
int PlugInChartBase::GetSize_Y() { return 0; }
void PlugInChartBase::latlong_to_chartpix(GLdouble lat, GLdouble lon, GLdouble &pixx,
                                          GLdouble &pixy) {}
void PlugInChartBase::chartpix_to_latlong(GLdouble pixx, GLdouble pixy,
                                          GLdouble *plat, GLdouble *plon) {}

// ----------------------------------------------------------------------------
// PlugInChartBaseGL
//  Derived from PlugInChartBase, add OpenGL Vector chart support
// ----------------------------------------------------------------------------

PlugInChartBaseGL::PlugInChartBaseGL() {}
PlugInChartBaseGL::~PlugInChartBaseGL() {}

int PlugInChartBaseGL::RenderRegionViewOnGL(const wxGLContext &glc,
                                            const PlugIn_ViewPort &VPoint,
                                            const wxRegion &Region,
                                            bool b_use_stencil) {
  return 0;
}

ListOfPI_S57Obj *PlugInChartBaseGL::GetObjRuleListAtLatLon(
    GLfloat lat, GLfloat lon, GLfloat select_radius, PlugIn_ViewPort *VPoint) {
  return 0;
}
wxString PlugInChartBaseGL::CreateObjDescriptions(ListOfPI_S57Obj *obj_list) {
  return "";
}

int PlugInChartBaseGL::GetNoCOVREntries() { return 0; }
int PlugInChartBaseGL::GetNoCOVRTablePoints(int iTable) { return 0; }
int PlugInChartBaseGL::GetNoCOVRTablenPoints(int iTable) { return 0; }
GLfloat *PlugInChartBaseGL::GetNoCOVRTableHead(int iTable) { return 0; }

// ----------------------------------------------------------------------------
// PlugInChartBaseGLPlus2
//  Derived from PlugInChartBaseGL, add additional chart management methods
// ----------------------------------------------------------------------------

PlugInChartBaseGLPlus2::PlugInChartBaseGLPlus2() {}
PlugInChartBaseGLPlus2::~PlugInChartBaseGLPlus2() {}

ListOfPI_S57Obj *PlugInChartBaseGLPlus2::GetLightsObjRuleListVisibleAtLatLon(
    GLfloat lat, GLfloat lon, PlugIn_ViewPort *VPoint) {
  return 0;
}

// ----------------------------------------------------------------------------
// PlugInChartBaseExtended
//  Derived from PlugInChartBase, add extended chart support methods
// ----------------------------------------------------------------------------

PlugInChartBaseExtended::PlugInChartBaseExtended() {}
PlugInChartBaseExtended::~PlugInChartBaseExtended() {}

int PlugInChartBaseExtended::RenderRegionViewOnGL(const wxGLContext &glc,
                                                  const PlugIn_ViewPort &VPoint,
                                                  const wxRegion &Region,
                                                  bool b_use_stencil) {
  return 0;
}
wxBitmap &PlugInChartBaseExtended::RenderRegionViewOnDCNoText(
    const PlugIn_ViewPort &VPoint, const wxRegion &Region) {
  return dummy_bitmap;
}
bool PlugInChartBaseExtended::RenderRegionViewOnDCTextOnly(
    wxMemoryDC &dc, const PlugIn_ViewPort &VPoint, const wxRegion &Region) {
  return false;
}

int PlugInChartBaseExtended::RenderRegionViewOnGLNoText(
    const wxGLContext &glc, const PlugIn_ViewPort &VPoint,
    const wxRegion &Region, bool b_use_stencil) {
  return 0;
}

int PlugInChartBaseExtended::RenderRegionViewOnGLTextOnly(
    const wxGLContext &glc, const PlugIn_ViewPort &VPoint,
    const wxRegion &Region, bool b_use_stencil) {
  return 0;
}

ListOfPI_S57Obj *PlugInChartBaseExtended::GetObjRuleListAtLatLon(
    GLfloat lat, GLfloat lon, GLfloat select_radius, PlugIn_ViewPort *VPoint) {
  return 0;
}
wxString PlugInChartBaseExtended::CreateObjDescriptions(
    ListOfPI_S57Obj *obj_list) {
  return wxString("");
}

int PlugInChartBaseExtended::GetNoCOVREntries() { return 0; }
int PlugInChartBaseExtended::GetNoCOVRTablePoints(int iTable) { return 0; }
int PlugInChartBaseExtended::GetNoCOVRTablenPoints(int iTable) { return 0; }
GLfloat *PlugInChartBaseExtended::GetNoCOVRTableHead(int iTable) { return 0; }

void PlugInChartBaseExtended::ClearPLIBTextList() {}

// ----------------------------------------------------------------------------
// PlugInChartBaseExtendedPlus2
//  Derived from PlugInChartBaseExtended, add additional extended chart support
//  methods
// ----------------------------------------------------------------------------

PlugInChartBaseExtendedPlus2::PlugInChartBaseExtendedPlus2() {}
PlugInChartBaseExtendedPlus2::~PlugInChartBaseExtendedPlus2() {}
ListOfPI_S57Obj *
PlugInChartBaseExtendedPlus2::GetLightsObjRuleListVisibleAtLatLon(
    GLfloat lat, GLfloat lon, PlugIn_ViewPort *VPoint) {
  return 0;
}

class wxArrayOfS57attVal;

PI_S57Obj::PI_S57Obj() {}

DECL_EXP wxString PI_GetPLIBColorScheme() { return ""; }
DECL_EXP int PI_GetPLIBDepthUnitInt() { return 0; }
DECL_EXP int PI_GetPLIBSymbolStyle() { return 0; }
DECL_EXP int PI_GetPLIBBoundaryStyle() { return 0; }
DECL_EXP int PI_GetPLIBStateHash() { return 0; }
DECL_EXP GLdouble PI_GetPLIBMarinerSafetyContour() { return 0; }
DECL_EXP bool PI_GetObjectRenderBox(PI_S57Obj *pObj, GLdouble *lat_min,
                                    GLdouble *lat_max, GLdouble *lon_min,
                                    GLdouble *lon_max) {
  return true;
}
DECL_EXP void PI_UpdateContext(PI_S57Obj *pObj) {}
DECL_EXP bool PI_PLIBObjectRenderCheck(PI_S57Obj *pObj, PlugIn_ViewPort *vp) {
  return true;
}
DECL_EXP PI_LUPname PI_GetObjectLUPName(PI_S57Obj *pObj) {
  return PI_SIMPLIFIED;
}
DECL_EXP PI_DisPrio PI_GetObjectDisplayPriority(PI_S57Obj *pObj) {
  return PI_PRIO_NODATA;
}
DECL_EXP PI_DisCat PI_GetObjectDisplayCategory(PI_S57Obj *pObj) {
  return PI_DISPLAYBASE;
}
DECL_EXP void PI_PLIBSetLineFeaturePriority(PI_S57Obj *pObj, int prio) {}
DECL_EXP void PI_PLIBPrepareForNewRender(void) {}
DECL_EXP void PI_PLIBFreeContext(void *pContext) {}
DECL_EXP void PI_PLIBSetRenderCaps(unsigned int flags) {}

DECL_EXP bool PI_PLIBSetContext(PI_S57Obj *pObj) { return true; }
DECL_EXP int PI_PLIBRenderObjectToDC(wxDC *pdc, PI_S57Obj *pObj,
                                     PlugIn_ViewPort *vp) {
  return 0;
}
DECL_EXP int PI_PLIBRenderAreaToDC(wxDC *pdc, PI_S57Obj *pObj,
                                   PlugIn_ViewPort *vp, wxRect rect,
                                   GLubyte *pixbuf) {
  return 0;
}

DECL_EXP int PI_PLIBRenderAreaToGL(const wxGLContext &glcc, PI_S57Obj *pObj,
                                   PlugIn_ViewPort *vp, wxRect &render_rect) {
  return 0;
}

DECL_EXP int PI_PLIBRenderObjectToGL(const wxGLContext &glcc, PI_S57Obj *pObj,
                                     PlugIn_ViewPort *vp, wxRect &render_rect) {
  return 0;
}

DECL_EXP bool PlugInHasNormalizedViewPort(PlugIn_ViewPort *vp) { return false; }
DECL_EXP void PlugInMultMatrixViewport(PlugIn_ViewPort *vp, GLfloat lat,
                                       GLfloat lon) {}
DECL_EXP void PlugInNormalizeViewport(PlugIn_ViewPort *vp, GLfloat lat,
                                      GLfloat lon) {}

class wxPoint2DDouble;
DECL_EXP void GetDoubleCanvasPixLL(PlugIn_ViewPort *vp, wxPoint2DDouble *pp,
                                   GLdouble lat, GLdouble lon) {}

DECL_EXP GLdouble fromDMM_Plugin(wxString sdms) { return 0.0; }
DECL_EXP void SetCanvasRotation(GLdouble rotation) {}
DECL_EXP void SetCanvasProjection(int projection) {}
DECL_EXP bool GetSingleWaypoint(wxString GUID, PlugIn_Waypoint *pwaypoint) {
  return true;
}
DECL_EXP bool CheckEdgePan_PlugIn(int x, int y, bool dragging, int margin,
                                  int delta) {
  return true;
}
DECL_EXP wxBitmap GetIcon_PlugIn(const wxString &name) { return dummy_bitmap; }
DECL_EXP void SetCursor_PlugIn(wxCursor *pPlugin_Cursor) {}
DECL_EXP wxFont *GetOCPNScaledFont_PlugIn(wxString TextElement,
                                          int default_size) {
  return 0;
}
wxFont dummyFont;
DECL_EXP wxFont GetOCPNGUIScaledFont_PlugIn(wxString item) { return dummyFont; }
DECL_EXP GLdouble GetOCPNGUIToolScaleFactor_PlugIn(int GUIScaledFactor) {
  return 0.0;
}
DECL_EXP GLdouble GetOCPNGUIToolScaleFactor_PlugIn() { return 0.0; }
DECL_EXP GLfloat GetOCPNChartScaleFactor_Plugin() { return 0.0; }
DECL_EXP wxColour GetFontColour_PlugIn(wxString TextElement) { return *wxRED; }

DECL_EXP GLdouble GetCanvasTilt() { return 0.0; }
DECL_EXP void SetCanvasTilt(GLdouble tilt) {}

/**
 * Start playing a sound file asynchronously. Supported formats depends
 * on sound backend. The deviceIx is only used on platforms using the
 * portaudio sound backend where -1 indicates the default device.
 */
DECL_EXP bool PlugInPlaySoundEx(wxString &sound_file, int deviceIndex) {
  return true;
}
DECL_EXP void AddChartDirectory(wxString &path) {}
DECL_EXP void ForceChartDBUpdate() {}
DECL_EXP void ForceChartDBRebuild() {}

DECL_EXP wxString GetWritableDocumentsDir(void) { return wxString(""); }
DECL_EXP wxDialog *GetActiveOptionsDialog() { return 0; }
DECL_EXP wxArrayString GetWaypointGUIDArray(void) { return dummy_array_string; }
DECL_EXP wxArrayString GetIconNameArray(void) { return dummy_array_string; }

DECL_EXP bool AddPersistentFontKey(wxString TextElement) { return true; }
DECL_EXP wxString GetActiveStyleName() { return wxString(""); }

DECL_EXP wxBitmap GetBitmapFromSVGFile(wxString filename, unsigned int width,
                                       unsigned int height) {
  return dummy_bitmap;
}
DECL_EXP bool IsTouchInterface_PlugIn(void) { return true; }

/*  Platform optimized File/Dir selector dialogs */
DECL_EXP int PlatformDirSelectorDialog(wxWindow *parent, wxString *file_spec,
                                       wxString Title, wxString initDir) {
  return 0;
}

DECL_EXP int PlatformFileSelectorDialog(wxWindow *parent, wxString *file_spec,
                                        wxString Title, wxString initDir,
                                        wxString suggestedName,
                                        wxString wildcard) {
  return 0;
}

/*  OpenCPN HTTP File Download PlugIn Interface   */

/*   Synchronous (Blocking) download of a single file  */

DECL_EXP wxEventType wxEVT_DOWNLOAD_EVENT = wxNewEventType();

DECL_EXP _OCPN_DLStatus OCPN_downloadFile(
    const wxString &url, const wxString &outputFile, const wxString &title,
    const wxString &message, const wxBitmap &bitmap, wxWindow *parent,
    long style, int timeout_secs) {
  return OCPN_DL_UNKNOWN;
}
/*   Asynchronous (Background) download of a single file  */

DECL_EXP _OCPN_DLStatus OCPN_downloadFileBackground(const wxString &url,
                                                    const wxString &outputFile,
                                                    wxEvtHandler *handler,
                                                    long *handle) {
  return OCPN_DL_UNKNOWN;
}

DECL_EXP void OCPN_cancelDownloadFileBackground(long handle) {}

/*   Synchronous (Blocking) HTTP POST operation for small amounts of data */

DECL_EXP _OCPN_DLStatus OCPN_postDataHttp(const wxString &url,
                                          const wxString &parameters,
                                          wxString &result, int timeout_secs) {
  return OCPN_DL_UNKNOWN;
}

/*   Check whether connection to the Internet is working */

DECL_EXP bool OCPN_isOnline() { return true; }

OCPN_downloadEvent::OCPN_downloadEvent(wxEventType commandType, int id) {}

OCPN_downloadEvent::~OCPN_downloadEvent() {}

wxEvent *OCPN_downloadEvent::Clone() const { return 0; }

/* API 1.14  */
/* API 1.14  adds some more common functions to avoid unnecessary code
 * duplication */

bool LaunchDefaultBrowser_Plugin(wxString url) { return true; }

// API 1.14 Extra canvas Support

/* Allow drawing of objects onto other OpenGL canvases */
DECL_EXP void PlugInAISDrawGL(wxGLCanvas *glcanvas, const PlugIn_ViewPort &vp) {
}
DECL_EXP bool PlugInSetFontColor(const wxString TextElement,
                                 const wxColour color) {
  return true;
}

// API 1.15
DECL_EXP GLdouble PlugInGetDisplaySizeMM() { return 0.0; }

//
DECL_EXP wxFont *FindOrCreateFont_PlugIn(int point_size, wxFontFamily family,
                                         wxFontStyle style, wxFontWeight weight,
                                         bool ul, const wxString &face,
                                         wxFontEncoding enc) {
  return 0;
}

DECL_EXP int PlugInGetMinAvailableGshhgQuality() { return 0; }
DECL_EXP int PlugInGetMaxAvailableGshhgQuality() { return 0; }

DECL_EXP void PlugInHandleAutopilotRoute(bool enable) {}

wxString *GetpSharedDataLocation(void) {
  return g_BasePlatform->GetSharedDataDirPtr();
}
DECL_EXP ArrayOfPlugIn_AIS_Targets *GetAISTargetArray() { return 0; }
DECL_EXP bool ShuttingDown(void) { return true; }

DECL_EXP wxWindow *PluginGetFocusCanvas() { return 0; }
DECL_EXP wxWindow *PluginGetOverlayRenderCanvas() { return 0; }

DECL_EXP void CanvasJumpToPosition(wxWindow *canvas, GLdouble lat, GLdouble lon,
                                   GLdouble scale) {}
DECL_EXP int AddCanvasMenuItem(wxMenuItem *pitem, opencpn_plugin *pplugin,
                               const char *name) {
  return 0;
}
DECL_EXP void RemoveCanvasMenuItem(int item, const char *name) {}
DECL_EXP void SetCanvasMenuItemViz(int item, bool viz, const char *name) {}
DECL_EXP void SetCanvasMenuItemGrey(int item, bool grey, const char *name) {}

DECL_EXP wxString GetSelectedWaypointGUID_Plugin() { return wxString(""); }
DECL_EXP wxString GetSelectedRouteGUID_Plugin() { return wxString(""); }
DECL_EXP wxString GetSelectedTrackGUID_Plugin() { return wxString(""); }

DECL_EXP std::unique_ptr<PlugIn_Waypoint> GetWaypoint_Plugin(const wxString &) {
  return nullptr;
}
DECL_EXP std::unique_ptr<PlugIn_Route> GetRoute_Plugin(const wxString &) {
  return nullptr;
}
DECL_EXP std::unique_ptr<PlugIn_Track> GetTrack_Plugin(const wxString &) {
  return nullptr;
}

DECL_EXP wxWindow *GetCanvasUnderMouse() { return 0; }
DECL_EXP int GetCanvasIndexUnderMouse() { return 0; }
DECL_EXP wxWindow *GetCanvasByIndex(int canvasIndex) { return 0; }
DECL_EXP int GetCanvasCount() { return 0; }
DECL_EXP bool CheckMUIEdgePan_PlugIn(int x, int y, bool dragging, int margin,
                                     int delta, int canvasIndex) {
  return true;
}
DECL_EXP void SetMUICursor_PlugIn(wxCursor *pCursor, int canvasIndex) {}
wxRect dummy_rectangle;
DECL_EXP wxRect GetMasterToolbarRect() { return dummy_rectangle; }

DECL_EXP int GetLatLonFormat(void) { return 0; }

DECL_EXP void ZeroXTE() {}

DECL_EXP PlugIn_Waypoint::PlugIn_Waypoint() {}
DECL_EXP PlugIn_Waypoint::PlugIn_Waypoint(GLdouble, GLdouble, const wxString &,
                                          const wxString &, const wxString &) {}
DECL_EXP PlugIn_Waypoint::~PlugIn_Waypoint() {}

DECL_EXP PlugIn_Waypoint_Ex::PlugIn_Waypoint_Ex() {}
DECL_EXP PlugIn_Waypoint_Ex::PlugIn_Waypoint_Ex(
    GLdouble lat, GLdouble lon, const wxString &icon_ident, const wxString &wp_name,
    const wxString &GUID, const GLdouble ScaMin, const bool bNameVisible,
    const int nRanges, const GLdouble RangeDistance, const wxColor RangeColor) {}
DECL_EXP PlugIn_Waypoint_Ex::~PlugIn_Waypoint_Ex() {}
DECL_EXP void PlugIn_Waypoint_Ex::InitDefaults() {}

DECL_EXP bool PlugIn_Waypoint_Ex::GetFSStatus() { return true; }

DECL_EXP int PlugIn_Waypoint_Ex::GetRouteMembershipCount() { return 0; }

DECL_EXP PlugIn_Waypoint_ExV2::PlugIn_Waypoint_ExV2() {}
DECL_EXP PlugIn_Waypoint_ExV2::PlugIn_Waypoint_ExV2(
    GLdouble lat, GLdouble lon, const wxString &icon_ident, const wxString &wp_name,
    const wxString &GUID, const GLdouble ScaMin, const GLdouble ScaMax,
    const bool bNameVisible, const int nRangeRings, const GLdouble RangeDistance,
    const int RangeDistanceUnits, const wxColor RangeColor,
    const GLdouble WaypointArrivalRadius, const bool ShowWaypointRangeRings,
    const GLdouble PlannedSpeed, const wxString TideStation) {}

DECL_EXP PlugIn_Waypoint_ExV2::~PlugIn_Waypoint_ExV2() {}

DECL_EXP void PlugIn_Waypoint_ExV2::InitDefaults() {}

DECL_EXP bool PlugIn_Waypoint_ExV2::GetFSStatus() { return true; }

DECL_EXP int PlugIn_Waypoint_ExV2::GetRouteMembershipCount() { return 0; }

DECL_EXP PlugIn_Route::PlugIn_Route(void) {}
DECL_EXP PlugIn_Route::~PlugIn_Route(void) {}

DECL_EXP PlugIn_Route_Ex::PlugIn_Route_Ex(void) {}
DECL_EXP PlugIn_Route_Ex::~PlugIn_Route_Ex(void) {}

DECL_EXP PlugIn_Route_ExV2::PlugIn_Route_ExV2() {}
DECL_EXP PlugIn_Route_ExV2::~PlugIn_Route_ExV2() {}

DECL_EXP PlugIn_Route_ExV3::PlugIn_Route_ExV3() {}
DECL_EXP PlugIn_Route_ExV3::~PlugIn_Route_ExV3() {}

DECL_EXP wxArrayString GetRouteGUIDArray(void) { return dummy_array_string; }
DECL_EXP wxArrayString GetTrackGUIDArray(void) { return dummy_array_string; }

DECL_EXP bool GetSingleWaypointEx(wxString GUID,
                                  PlugIn_Waypoint_Ex *pwaypoint) {
  return true;
}

DECL_EXP bool AddSingleWaypointEx(PlugIn_Waypoint_Ex *pwaypoint,
                                  bool b_permanent) {
  return true;
}
DECL_EXP bool UpdateSingleWaypointEx(PlugIn_Waypoint_Ex *pwaypoint) {
  return true;
}

DECL_EXP bool AddPlugInRouteEx(PlugIn_Route_Ex *proute, bool b_permanent) {
  return true;
}
DECL_EXP bool UpdatePlugInRouteEx(PlugIn_Route_Ex *proute) { return true; }

DECL_EXP std::unique_ptr<PlugIn_Waypoint_Ex> GetWaypointEx_Plugin(
    const wxString &) {
  return nullptr;
}
DECL_EXP std::unique_ptr<PlugIn_Route_Ex> GetRouteEx_Plugin(const wxString &) {
  return nullptr;
}

DECL_EXP wxString GetActiveWaypointGUID(void) { return wxString(""); }
DECL_EXP wxString GetActiveRouteGUID(void) { return wxString(""); }

// API 1.18

//  Scaled display support, as on some GTK3 and Mac Retina devices
DECL_EXP GLdouble OCPN_GetDisplayContentScaleFactor() { return 0; }

//  Scaled display support, on Windows devices
DECL_EXP GLdouble OCPN_GetWinDIPScaleFactor() { return 0; }

//  Comm Priority query support
DECL_EXP std::vector<std::string> GetPriorityMaps() {
  std::vector<std::string> v;
  return v;
}
DECL_EXP std::vector<std::string> GetActivePriorityIdentifiers() {
  std::vector<std::string> v;
  return v;
}

extern DECL_EXP int GetGlobalWatchdogTimoutSeconds() { return 0; }

DECL_EXP wxArrayString GetRouteGUIDArray(OBJECT_LAYER_REQ req) {
  wxArrayString was;
  return was;
}
DECL_EXP wxArrayString GetTrackGUIDArray(OBJECT_LAYER_REQ req) {
  wxArrayString was;
  return was;
}
DECL_EXP wxArrayString GetWaypointGUIDArray(OBJECT_LAYER_REQ req) {
  wxArrayString was;
  return was;
}
