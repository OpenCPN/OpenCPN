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
#include "base_platform.h"

extern BasePlatform* g_BasePlatform;

typedef unsigned short OCushort;

class GLUtesselator;
extern "C" {
typedef void (* _GLUfuncptr)();
DECL_EXP GLUtesselator* gluNewTess(void) { return 0; }
DECL_EXP void gluTessProperty(GLUtesselator* tess, unsigned which, double data) {}
DECL_EXP void gluTessEndPolygon(GLUtesselator* tess) {}
DECL_EXP void gluTessBeginPolygon(GLUtesselator* tess, void* data) {}
DECL_EXP void gluTessNormal(GLUtesselator* tess, double X, double Y, double Z) {}
DECL_EXP void gluTessVertex(GLUtesselator* tess, double *location, void* data) {}
DECL_EXP void gluTessCallback(GLUtesselator* tess, unsigned which, _GLUfuncptr CallBackFunc) {}
DECL_EXP const unsigned char* gluErrorString(unsigned error) { return reinterpret_cast<const unsigned char*>(""); }
DECL_EXP void gluDeleteTess(GLUtesselator* tess) {}
DECL_EXP void gluTessBeginContour(GLUtesselator* tess) {}
DECL_EXP void gluTessEndContour(GLUtesselator* tess) {}
DECL_EXP void glLineWidth(float width) {}

DECL_EXP void glClearIndex( float c ) {}

DECL_EXP void glClearColor( unsigned red, unsigned green, unsigned blue, unsigned alpha ) {}

DECL_EXP void glClear( int mask ) {}

DECL_EXP void glIndexMask( int mask ) {}

DECL_EXP void glColorMask( bool red, bool green, bool blue, bool alpha ) {}

DECL_EXP void glAlphaFunc( unsigned func, unsigned ref ) {}

DECL_EXP void glBlendFunc( unsigned sfactor, unsigned dfactor ) {}

DECL_EXP void glLogicOp( unsigned opcode ) {}

DECL_EXP void glCullFace( unsigned mode ) {}

DECL_EXP void glFrontFace( unsigned mode ) {}

DECL_EXP void glPointSize( float size ) {}

DECL_EXP void glLineStipple( int factor, OCushort pattern ) {}

DECL_EXP void glPolygonMode( unsigned face, unsigned mode ) {}

DECL_EXP void glPolygonOffset( float factor, float units ) {}

DECL_EXP void glPolygonStipple( const unsigned char *mask ) {}

DECL_EXP void glGetPolygonStipple( unsigned char *mask ) {}

DECL_EXP void glEdgeFlag( bool flag ) {}

DECL_EXP void glEdgeFlagv( const bool *flag ) {}

DECL_EXP void glScissor( int x, int y, size_t width, size_t height) {}

DECL_EXP void glClipPlane( unsigned plane, const double *equation ) {}

DECL_EXP void glGetClipPlane( unsigned plane, double *equation ) {}

DECL_EXP void glDrawBuffer( unsigned mode ) {}

DECL_EXP void glReadBuffer( unsigned mode ) {}

DECL_EXP void glEnable( unsigned cap ) {}

DECL_EXP void glDisable( unsigned cap ) {}

DECL_EXP bool glIsEnabled( unsigned cap ) { return true; }


DECL_EXP void glEnableClientState( unsigned cap ) {}  /* 1.1 */

DECL_EXP void glDisableClientState( unsigned cap ) {}  /* 1.1 */


DECL_EXP void glGetBooleanv( unsigned pname, bool *params ) {}

DECL_EXP void glGetDoublev( unsigned pname, double *params ) {}

DECL_EXP void glGetFloatv( unsigned pname, float *params ) {}

DECL_EXP void glGetIntegerv( unsigned pname, int *params ) {}


DECL_EXP void glPushAttrib( int mask ) {}

DECL_EXP void glPopAttrib( void ) {}


DECL_EXP void glPushClientAttrib( int mask ) {}  /* 1.1 */

DECL_EXP void glPopClientAttrib( void ) {}  /* 1.1 */


DECL_EXP int glRenderMode( unsigned mode ) { return 0; }

DECL_EXP unsigned glGetError( void ) { return 0; }

DECL_EXP const unsigned char * glGetString( unsigned name ) { return reinterpret_cast<const unsigned char*>(""); }

DECL_EXP void glFinish( void ) {}

DECL_EXP void glFlush( void ) {}

DECL_EXP void glHint( unsigned target, unsigned mode ) {}


/*
 * Depth Buffer
 */

DECL_EXP void glClearDepth( unsigned depth ) {}

DECL_EXP void glDepthFunc( unsigned func ) {}

DECL_EXP void glDepthMask( bool flag ) {}

DECL_EXP void glDepthRange( unsigned near_val, unsigned far_val ) {}


/*
 * Accumulation Buffer
 */

DECL_EXP void glClearAccum( float red, float green, float blue, float alpha ) {}

DECL_EXP void glAccum( unsigned op, float value ) {}


/*
 * Transformation
 */

DECL_EXP void glMatrixMode( unsigned mode ) {}

DECL_EXP void glOrtho( double left, double right,
                                 double bottom, double top,
                                 double near_val, double far_val ) {}

DECL_EXP void glFrustum( double left, double right,
                                   double bottom, double top,
                                   double near_val, double far_val ) {}

DECL_EXP void glViewport( int x, int y,
                                    size_t width, size_t height ) {}

DECL_EXP void glPushMatrix( void ) {}

DECL_EXP void glPopMatrix( void ) {}

DECL_EXP void glLoadIdentity( void ) {}

DECL_EXP void glLoadMatrixd( const double *m ) {}
DECL_EXP void glLoadMatrixf( const float *m ) {}

DECL_EXP void glMultMatrixd( const double *m ) {}
DECL_EXP void glMultMatrixf( const float *m ) {}

DECL_EXP void glRotated( double angle,
                                   double x, double y, double z ) {}
DECL_EXP void glRotatef( float angle,
                                   float x, float y, float z ) {}

DECL_EXP void glScaled( double x, double y, double z ) {}
DECL_EXP void glScalef( float x, float y, float z ) {}

DECL_EXP void glTranslated( double x, double y, double z ) {}
DECL_EXP void glTranslatef( float x, float y, float z ) {}


/*
 * Display Lists
 */

DECL_EXP bool glIsList( int list ) { return true; }

DECL_EXP void glDeleteLists( int list, size_t range ) {}

DECL_EXP int glGenLists( size_t range ) { return 0; }

DECL_EXP void glNewList( int list, unsigned mode ) {}

DECL_EXP void glEndList( void ) {}

DECL_EXP void glCallList( int list ) {}

DECL_EXP void glCallLists( size_t n, unsigned type,
                                     const void *lists ) {}

DECL_EXP void glListBase( int base ) {}


/*
 * Drawing Functions
 */

DECL_EXP void glBegin( unsigned mode ) {}

DECL_EXP void glEnd( void ) {}


DECL_EXP void glVertex2d( double x, double y ) {}
DECL_EXP void glVertex2f( float x, float y ) {}
DECL_EXP void glVertex2i( int x, int y ) {}
DECL_EXP void glVertex2s( short x, short y ) {}

DECL_EXP void glVertex3d( double x, double y, double z ) {}
DECL_EXP void glVertex3f( float x, float y, float z ) {}
DECL_EXP void glVertex3i( int x, int y, int z ) {}
DECL_EXP void glVertex3s( short x, short y, short z ) {}

DECL_EXP void glVertex4d( double x, double y, double z, double w ) {}
DECL_EXP void glVertex4f( float x, float y, float z, float w ) {}
DECL_EXP void glVertex4i( int x, int y, int z, int w ) {}
DECL_EXP void glVertex4s( short x, short y, short z, short w ) {}

DECL_EXP void glVertex2dv( const double *v ) {}
DECL_EXP void glVertex2fv( const float *v ) {}
DECL_EXP void glVertex2iv( const int *v ) {}
DECL_EXP void glVertex2sv( const short *v ) {}

DECL_EXP void glVertex3dv( const double *v ) {}
DECL_EXP void glVertex3fv( const float *v ) {}
DECL_EXP void glVertex3iv( const int *v ) {}
DECL_EXP void glVertex3sv( const short *v ) {}

DECL_EXP void glVertex4dv( const double *v ) {}
DECL_EXP void glVertex4fv( const float *v ) {}
DECL_EXP void glVertex4iv( const int *v ) {}
DECL_EXP void glVertex4sv( const short *v ) {}


DECL_EXP void glNormal3b( unsigned char nx, unsigned char ny, unsigned char nz ) {}
DECL_EXP void glNormal3d( double nx, double ny, double nz ) {}
DECL_EXP void glNormal3f( float nx, float ny, float nz ) {}
DECL_EXP void glNormal3i( int nx, int ny, int nz ) {}
DECL_EXP void glNormal3s( short nx, short ny, short nz ) {}

DECL_EXP void glNormal3bv( const unsigned char *v ) {}
DECL_EXP void glNormal3dv( const double *v ) {}
DECL_EXP void glNormal3fv( const float *v ) {}
DECL_EXP void glNormal3iv( const int *v ) {}
DECL_EXP void glNormal3sv( const short *v ) {}


DECL_EXP void glIndexd( double c ) {}
DECL_EXP void glIndexf( float c ) {}
DECL_EXP void glIndexi( int c ) {}
DECL_EXP void glIndexs( short c ) {}
DECL_EXP void glIndexub( unsigned char c ) {}  /* 1.1 */

DECL_EXP void glIndexdv( const double *c ) {}
DECL_EXP void glIndexfv( const float *c ) {}
DECL_EXP void glIndexiv( const int *c ) {}
DECL_EXP void glIndexsv( const short *c ) {}
DECL_EXP void glIndexubv( const unsigned char *c ) {}  /* 1.1 */

DECL_EXP void glColor3b( unsigned char red, unsigned char green, unsigned char blue ) {}
DECL_EXP void glColor3d( double red, double green, double blue ) {}
DECL_EXP void glColor3f( float red, float green, float blue ) {}
DECL_EXP void glColor3i( int red, int green, int blue ) {}
DECL_EXP void glColor3s( short red, short green, short blue ) {}
DECL_EXP void glColor3ub( unsigned char red, unsigned char green, unsigned char blue ) {}
DECL_EXP void glColor3ui( int red, int green, int blue ) {}
DECL_EXP void glColor3us( OCushort red, OCushort green, OCushort blue ) {}

DECL_EXP void glColor4b( unsigned char red, unsigned char green,
                                   unsigned char blue, unsigned char alpha ) {}
DECL_EXP void glColor4d( double red, double green,
                                   double blue, double alpha ) {}
DECL_EXP void glColor4f( float red, float green,
                                   float blue, float alpha ) {}
DECL_EXP void glColor4i( int red, int green,
                                   int blue, int alpha ) {}
DECL_EXP void glColor4s( short red, short green,
                                   short blue, short alpha ) {}
DECL_EXP void glColor4ub( unsigned char red, unsigned char green,
                                    unsigned char blue, unsigned char alpha ) {}
DECL_EXP void glColor4ui( int red, int green,
                                    int blue, int alpha ) {}
DECL_EXP void glColor4us( OCushort red, OCushort green,
                                    OCushort blue, OCushort alpha ) {}


DECL_EXP void glColor3bv( const unsigned char *v ) {}
DECL_EXP void glColor3dv( const double *v ) {}
DECL_EXP void glColor3fv( const float *v ) {}
DECL_EXP void glColor3iv( const int *v ) {}
DECL_EXP void glColor3sv( const short *v ) {}
DECL_EXP void glColor3ubv( const unsigned char *v ) {}
DECL_EXP void glColor3uiv( const int *v ) {}
DECL_EXP void glColor3usv( const OCushort *v ) {}

DECL_EXP void glColor4bv( const unsigned char *v ) {}
DECL_EXP void glColor4dv( const double *v ) {}
DECL_EXP void glColor4fv( const float *v ) {}
DECL_EXP void glColor4iv( const int *v ) {}
DECL_EXP void glColor4sv( const short *v ) {}
DECL_EXP void glColor4ubv( const unsigned char *v ) {}
DECL_EXP void glColor4uiv( const int *v ) {}
DECL_EXP void glColor4usv( const OCushort *v ) {}


DECL_EXP void glTexCoord1d( double s ) {}
DECL_EXP void glTexCoord1f( float s ) {}
DECL_EXP void glTexCoord1i( int s ) {}
DECL_EXP void glTexCoord1s( short s ) {}

DECL_EXP void glTexCoord2d( double s, double t ) {}
DECL_EXP void glTexCoord2f( float s, float t ) {}
DECL_EXP void glTexCoord2i( int s, int t ) {}
DECL_EXP void glTexCoord2s( short s, short t ) {}

DECL_EXP void glTexCoord3d( double s, double t, double r ) {}
DECL_EXP void glTexCoord3f( float s, float t, float r ) {}
DECL_EXP void glTexCoord3i( int s, int t, int r ) {}
DECL_EXP void glTexCoord3s( short s, short t, short r ) {}

DECL_EXP void glTexCoord4d( double s, double t, double r, double q ) {}
DECL_EXP void glTexCoord4f( float s, float t, float r, float q ) {}
DECL_EXP void glTexCoord4i( int s, int t, int r, int q ) {}
DECL_EXP void glTexCoord4s( short s, short t, short r, short q ) {}

DECL_EXP void glTexCoord1dv( const double *v ) {}
DECL_EXP void glTexCoord1fv( const float *v ) {}
DECL_EXP void glTexCoord1iv( const int *v ) {}
DECL_EXP void glTexCoord1sv( const short *v ) {}

DECL_EXP void glTexCoord2dv( const double *v ) {}
DECL_EXP void glTexCoord2fv( const float *v ) {}
DECL_EXP void glTexCoord2iv( const int *v ) {}
DECL_EXP void glTexCoord2sv( const short *v ) {}

DECL_EXP void glTexCoord3dv( const double *v ) {}
DECL_EXP void glTexCoord3fv( const float *v ) {}
DECL_EXP void glTexCoord3iv( const int *v ) {}
DECL_EXP void glTexCoord3sv( const short *v ) {}

DECL_EXP void glTexCoord4dv( const double *v ) {}
DECL_EXP void glTexCoord4fv( const float *v ) {}
DECL_EXP void glTexCoord4iv( const int *v ) {}
DECL_EXP void glTexCoord4sv( const short *v ) {}


DECL_EXP void glRasterPos2d( double x, double y ) {}
DECL_EXP void glRasterPos2f( float x, float y ) {}
DECL_EXP void glRasterPos2i( int x, int y ) {}
DECL_EXP void glRasterPos2s( short x, short y ) {}

DECL_EXP void glRasterPos3d( double x, double y, double z ) {}
DECL_EXP void glRasterPos3f( float x, float y, float z ) {}
DECL_EXP void glRasterPos3i( int x, int y, int z ) {}
DECL_EXP void glRasterPos3s( short x, short y, short z ) {}

DECL_EXP void glRasterPos4d( double x, double y, double z, double w ) {}
DECL_EXP void glRasterPos4f( float x, float y, float z, float w ) {}
DECL_EXP void glRasterPos4i( int x, int y, int z, int w ) {}
DECL_EXP void glRasterPos4s( short x, short y, short z, short w ) {}

DECL_EXP void glRasterPos2dv( const double *v ) {}
DECL_EXP void glRasterPos2fv( const float *v ) {}
DECL_EXP void glRasterPos2iv( const int *v ) {}
DECL_EXP void glRasterPos2sv( const short *v ) {}

DECL_EXP void glRasterPos3dv( const double *v ) {}
DECL_EXP void glRasterPos3fv( const float *v ) {}
DECL_EXP void glRasterPos3iv( const int *v ) {}
DECL_EXP void glRasterPos3sv( const short *v ) {}

DECL_EXP void glRasterPos4dv( const double *v ) {}
DECL_EXP void glRasterPos4fv( const float *v ) {}
DECL_EXP void glRasterPos4iv( const int *v ) {}
DECL_EXP void glRasterPos4sv( const short *v ) {}


DECL_EXP void glRectd( double x1, double y1, double x2, double y2 ) {}
DECL_EXP void glRectf( float x1, float y1, float x2, float y2 ) {}
DECL_EXP void glRecti( int x1, int y1, int x2, int y2 ) {}
DECL_EXP void glRects( short x1, short y1, short x2, short y2 ) {}


DECL_EXP void glRectdv( const double *v1, const double *v2 ) {}
DECL_EXP void glRectfv( const float *v1, const float *v2 ) {}
DECL_EXP void glRectiv( const int *v1, const int *v2 ) {}
DECL_EXP void glRectsv( const short *v1, const short *v2 ) {}


DECL_EXP void  glVertexPointer( int size, unsigned type,
                                       size_t stride, const void *ptr ){}

DECL_EXP void  glNormalPointer( unsigned type, size_t stride,
                                       const void *ptr ){}

DECL_EXP void  glColorPointer( int size, unsigned type,
                                      size_t stride, const void *ptr ){}

DECL_EXP void  glIndexPointer( unsigned type, size_t stride,
                                      const void *ptr ){}

DECL_EXP void  glTexCoordPointer( int size, unsigned type,
                                         size_t stride, const void *ptr ){}

DECL_EXP void  glEdgeFlagPointer( size_t stride, const void *ptr ){}

DECL_EXP void  glGetPointerv( unsigned pname, void **params ){}

DECL_EXP void  glArrayElement( int i ){}

DECL_EXP void  glDrawArrays( unsigned mode, int first, size_t count ){}

DECL_EXP void  glDrawElements( unsigned mode, size_t count,
                                      unsigned type, const void *indices ){}

DECL_EXP void  glInterleavedArrays( unsigned format, size_t stride,
                                           const void *pointer ){}

/*
 * Lighting
 */

DECL_EXP void  glShadeModel( unsigned mode ){}

DECL_EXP void  glLightf( unsigned light, unsigned pname, float param ){}
DECL_EXP void  glLighti( unsigned light, unsigned pname, int param ){}
DECL_EXP void  glLightfv( unsigned light, unsigned pname,
                                 const float *params ){}
DECL_EXP void  glLightiv( unsigned light, unsigned pname,
                                 const int *params ){}

DECL_EXP void  glGetLightfv( unsigned light, unsigned pname,
                                    float *params ){}
DECL_EXP void  glGetLightiv( unsigned light, unsigned pname,
                                    int *params ){}

DECL_EXP void  glLightModelf( unsigned pname, float param ){}
DECL_EXP void  glLightModeli( unsigned pname, int param ){}
DECL_EXP void  glLightModelfv( unsigned pname, const float *params ){}
DECL_EXP void  glLightModeliv( unsigned pname, const int *params ){}

DECL_EXP void  glMaterialf( unsigned face, unsigned pname, float param ){}
DECL_EXP void  glMateriali( unsigned face, unsigned pname, int param ){}
DECL_EXP void  glMaterialfv( unsigned face, unsigned pname, const float *params ){}
DECL_EXP void  glMaterialiv( unsigned face, unsigned pname, const int *params ){}

DECL_EXP void  glGetMaterialfv( unsigned face, unsigned pname, float *params ){}
DECL_EXP void  glGetMaterialiv( unsigned face, unsigned pname, int *params ){}

DECL_EXP void  glColorMaterial( unsigned face, unsigned mode ){}


/*
 * Raster functions
 */

DECL_EXP void  glPixelZoom( float xfactor, float yfactor ){}

DECL_EXP void  glPixelStoref( unsigned pname, float param ){}
DECL_EXP void  glPixelStorei( unsigned pname, int param ){}

DECL_EXP void  glPixelTransferf( unsigned pname, float param ){}
DECL_EXP void  glPixelTransferi( unsigned pname, int param ){}

DECL_EXP void  glPixelMapfv( unsigned map, size_t mapsize,
                             const float *values ){}
DECL_EXP void  glPixelMapuiv( unsigned map, size_t mapsize,
                              const unsigned *values ){}
DECL_EXP void  glPixelMapusv( unsigned map, size_t mapsize,
                              const OCushort *values ){}

DECL_EXP void  glGetPixelMapfv( unsigned map, float *values ){}
DECL_EXP void  glGetPixelMapuiv( unsigned map, unsigned *values ){}
DECL_EXP void  glGetPixelMapusv( unsigned map, OCushort *values ){}

DECL_EXP void  glBitmap( size_t width, size_t height,
                         float xorig, float yorig,
                         float xmove, float ymove,
                         const unsigned char *bitmap ){}

DECL_EXP void  glReadPixels( int x, int y,
                             size_t width, size_t height,
                             unsigned format, unsigned type,
                             void *pixels ){}

DECL_EXP void  glDrawPixels( size_t width, size_t height,
                             unsigned format, unsigned type,
                             const void *pixels ){}

DECL_EXP void  glCopyPixels( int x, int y,
                             size_t width, size_t height,
                             unsigned type ){}

/*
 * Stenciling
 */

DECL_EXP void  glStencilFunc( unsigned func, int ref, unsigned mask ){}

DECL_EXP void  glStencilMask( unsigned mask ){}

DECL_EXP void  glStencilOp( unsigned fail, unsigned zfail, unsigned zpass ){}

DECL_EXP void  glClearStencil( int s ){}



/*
 * Texture mapping
 */

DECL_EXP void  glTexGend( unsigned coord, unsigned pname, double param ){}
DECL_EXP void  glTexGenf( unsigned coord, unsigned pname, float param ){}
DECL_EXP void  glTexGeni( unsigned coord, unsigned pname, int param ){}

DECL_EXP void  glTexGendv( unsigned coord, unsigned pname, const double *params ){}
DECL_EXP void  glTexGenfv( unsigned coord, unsigned pname, const float *params ){}
DECL_EXP void  glTexGeniv( unsigned coord, unsigned pname, const int *params ){}

DECL_EXP void  glGetTexGendv( unsigned coord, unsigned pname, double *params ){}
DECL_EXP void  glGetTexGenfv( unsigned coord, unsigned pname, float *params ){}
DECL_EXP void  glGetTexGeniv( unsigned coord, unsigned pname, int *params ){}


DECL_EXP void  glTexEnvf( unsigned target, unsigned pname, float param ){}
DECL_EXP void  glTexEnvi( unsigned target, unsigned pname, int param ){}

DECL_EXP void  glTexEnvfv( unsigned target, unsigned pname, const float *params ){}
DECL_EXP void  glTexEnviv( unsigned target, unsigned pname, const int *params ){}

DECL_EXP void  glGetTexEnvfv( unsigned target, unsigned pname, float *params ){}
DECL_EXP void  glGetTexEnviv( unsigned target, unsigned pname, int *params ){}


DECL_EXP void  glTexParameterf( unsigned target, unsigned pname, float param ){}
DECL_EXP void  glTexParameteri( unsigned target, unsigned pname, int param ){}

DECL_EXP void  glTexParameterfv( unsigned target, unsigned pname,
                                          const float *params ){}
DECL_EXP void  glTexParameteriv( unsigned target, unsigned pname,
                                          const int *params ){}

DECL_EXP void  glGetTexParameterfv( unsigned target,
                                    unsigned pname, float *params){}
DECL_EXP void  glGetTexParameteriv( unsigned target,
                                    unsigned pname, int *params ){}

DECL_EXP void  glGetTexLevelParameterfv( unsigned target, int level,
                                         unsigned pname, float *params ){}
DECL_EXP void  glGetTexLevelParameteriv( unsigned target, int level,
                                         unsigned pname, int *params ){}


DECL_EXP void  glTexImage1D( unsigned target, int level,
                             int internalFormat,
                             size_t width, int border,
                             unsigned format, unsigned type,
                             const void *pixels ){}

DECL_EXP void  glTexImage2D( unsigned target, int level,
                             int internalFormat,
                             size_t width, size_t height,
                             int border, unsigned format, unsigned type,
                             const void *pixels ){}

DECL_EXP void  glGetTexImage( unsigned target, int level,
                              unsigned format, unsigned type,
                              void *pixels ){}


/* 1.1 functions */

DECL_EXP void  glGenTextures( size_t n, unsigned *textures ){}

DECL_EXP void  glDeleteTextures( size_t n, const unsigned *textures){}

DECL_EXP void  glBindTexture( unsigned target, unsigned texture ){}

DECL_EXP void  glPrioritizeTextures( size_t n,
                                     const unsigned *textures,
                                     const unsigned *priorities ){}

DECL_EXP bool  glAreTexturesResident( size_t n,
                                      const unsigned *textures,
                                      bool *residences ){ return true; }

DECL_EXP bool  glIsTexture( unsigned texture ){ return true; }


DECL_EXP void  glTexSubImage1D( unsigned target, int level,
                                int xoffset,
                                size_t width, unsigned format,
                                unsigned type, const void *pixels ){}


DECL_EXP void  glTexSubImage2D( unsigned target, int level,
                                int xoffset, int yoffset,
                                size_t width, size_t height,
                                unsigned format, unsigned type,
                                const void *pixels ){}


DECL_EXP void  glCopyTexImage1D( unsigned target, int level,
                                 unsigned internalformat,
                                 int x, int y,
                                 size_t width, int border ){}


DECL_EXP void  glCopyTexImage2D( unsigned target, int level,
                                 unsigned internalformat,
                                 int x, int y,
                                 size_t width, size_t height,
                                 int border ){}


DECL_EXP void  glCopyTexSubImage1D( unsigned target, int level,
                                    int xoffset, int x, int y,
                                    size_t width ){}


DECL_EXP void  glCopyTexSubImage2D( unsigned target, int level,
                                    int xoffset, int yoffset,
                                    int x, int y,
                                    size_t width, size_t height ){}


/*
 * Evaluators
 */

DECL_EXP void  glMap1d( unsigned target, double u1, double u2,
                        int stride,
                        int order, const double *points ){}
DECL_EXP void  glMap1f( unsigned target, float u1, float u2,
                        int stride,
                        int order, const float *points ){}

DECL_EXP void  glMap2d( unsigned target,
             double u1, double u2, int ustride, int uorder,
             double v1, double v2, int vstride, int vorder,
             const double *points ){}
DECL_EXP void  glMap2f( unsigned target,
             float u1, float u2, int ustride, int uorder,
             float v1, float v2, int vstride, int vorder,
             const float *points ){}

DECL_EXP void  glGetMapdv( unsigned target, unsigned query, double *v ){}
DECL_EXP void  glGetMapfv( unsigned target, unsigned query, float *v ){}
DECL_EXP void  glGetMapiv( unsigned target, unsigned query, int *v ){}

DECL_EXP void  glEvalCoord1d( double u ){}
DECL_EXP void  glEvalCoord1f( float u ){}

DECL_EXP void  glEvalCoord1dv( const double *u ){}
DECL_EXP void  glEvalCoord1fv( const float *u ){}

DECL_EXP void  glEvalCoord2d( double u, double v ){}
DECL_EXP void  glEvalCoord2f( float u, float v ){}

DECL_EXP void  glEvalCoord2dv( const double *u ){}
DECL_EXP void  glEvalCoord2fv( const float *u ){}

DECL_EXP void  glMapGrid1d( int un, double u1, double u2 ){}
DECL_EXP void  glMapGrid1f( int un, float u1, float u2 ){}

DECL_EXP void  glMapGrid2d( int un, double u1, double u2,
                            int vn, double v1, double v2 ){}
DECL_EXP void  glMapGrid2f( int un, float u1, float u2,
                            int vn, float v1, float v2 ){}

DECL_EXP void  glEvalPoint1( int i ){}

DECL_EXP void  glEvalPoint2( int i, int j ){}

DECL_EXP void  glEvalMesh1( unsigned mode, int i1, int i2 ){}

DECL_EXP void  glEvalMesh2( unsigned mode, int i1, int i2, int j1, int j2 ){}


/*
 * Fog
 */

DECL_EXP void  glFogf( unsigned pname, float param ){}

DECL_EXP void  glFogi( unsigned pname, int param ){}

DECL_EXP void  glFogfv( unsigned pname, const float *params ){}

DECL_EXP void  glFogiv( unsigned pname, const int *params ){}


/*
 * Selection and Feedback
 */

DECL_EXP void  glFeedbackBuffer( size_t size, unsigned type, float *buffer ){}

DECL_EXP void  glPassThrough( float token ){}

DECL_EXP void  glSelectBuffer( size_t size, unsigned *buffer ){}

DECL_EXP void  glInitNames( void ){}

DECL_EXP void  glLoadName( unsigned name ){}

DECL_EXP void  glPushName( unsigned name ){}

DECL_EXP void  glPopName( void ){}



/*
 * OpenGL 1.2
 */

DECL_EXP void  glDrawRangeElements( unsigned mode, unsigned start,
    unsigned end, size_t count, unsigned type, const void *indices ){}

DECL_EXP void  glTexImage3D( unsigned target, int level,
                             int internalFormat,
                             size_t width, size_t height,
                             size_t depth, int border,
                             unsigned format, unsigned type,
                             const void *pixels ){}

DECL_EXP void  glTexSubImage3D( unsigned target, int level,
                                int xoffset, int yoffset,
                                int zoffset, size_t width,
                                size_t height, size_t depth,
                                unsigned format,
                                unsigned type, const void *pixels){}

DECL_EXP void  glCopyTexSubImage3D( unsigned target, int level,
                                    int xoffset, int yoffset,
                                    int zoffset, int x,
                                    int y, size_t width,
                                    size_t height ){}



DECL_EXP void  glColorTable( unsigned target, unsigned internalformat,
                             size_t width, unsigned format,
                             unsigned type, const void *table ){}

DECL_EXP void  glColorSubTable( unsigned target,
                                size_t start, size_t count,
                                unsigned format, unsigned type,
                                const void *data ){}

DECL_EXP void  glColorTableParameteriv(unsigned target, unsigned pname,
                                       const int *params){}

DECL_EXP void  glColorTableParameterfv(unsigned target, unsigned pname,
                                       const float *params){}

DECL_EXP void  glCopyColorSubTable( unsigned target, size_t start,
                                    int x, int y, size_t width ){}

DECL_EXP void  glCopyColorTable( unsigned target, unsigned internalformat,
                                 int x, int y, size_t width ){}

DECL_EXP void  glGetColorTable( unsigned target, unsigned format,
                                unsigned type, void *table ){}

DECL_EXP void  glGetColorTableParameterfv( unsigned target, unsigned pname,
                                           float *params ){}

DECL_EXP void  glGetColorTableParameteriv( unsigned target, unsigned pname,
                                           int *params ){}

DECL_EXP void  glBlendEquation( unsigned mode ){}

DECL_EXP void  glBlendColor( unsigned red, unsigned green,
                             unsigned blue, unsigned alpha ){}

DECL_EXP void  glHistogram( unsigned target, size_t width,
                   unsigned internalformat, bool sink ){}

DECL_EXP void  glResetHistogram( unsigned target ){}

DECL_EXP void  glGetHistogram( unsigned target, bool reset,
                      unsigned format, unsigned type,
                      void *values ){}

DECL_EXP void  glGetHistogramParameterfv( unsigned target, unsigned pname,
                         float *params ){}

DECL_EXP void  glGetHistogramParameteriv( unsigned target, unsigned pname,
                         int *params ){}

DECL_EXP void  glMinmax( unsigned target, unsigned internalformat,
                bool sink ){}

DECL_EXP void  glResetMinmax( unsigned target ){}

DECL_EXP void  glGetMinmax( unsigned target, bool reset,
                            unsigned format, unsigned types,
                            void *values ){}

DECL_EXP void  glGetMinmaxParameterfv( unsigned target, unsigned pname,
                                      float *params ){}

DECL_EXP void  glGetMinmaxParameteriv( unsigned target, unsigned pname,
                          int *params ){}

DECL_EXP void  glConvolutionFilter1D( unsigned target,
    unsigned internalformat, size_t width, unsigned format, unsigned type,
    const void *image ){}

DECL_EXP void  glConvolutionFilter2D( unsigned target,
    unsigned internalformat, size_t width, size_t height, unsigned format,
    unsigned type, const void *image ){}

DECL_EXP void  glConvolutionParameterf( unsigned target, unsigned pname,
    float params ){}

DECL_EXP void  glConvolutionParameterfv( unsigned target, unsigned pname,
    const float *params ){}

DECL_EXP void  glConvolutionParameteri( unsigned target, unsigned pname,
    int params ){}

DECL_EXP void  glConvolutionParameteriv( unsigned target, unsigned pname,
    const int *params ){}

DECL_EXP void  glCopyConvolutionFilter1D( unsigned target,
    unsigned internalformat, int x, int y, size_t width ){}

DECL_EXP void  glCopyConvolutionFilter2D( unsigned target,
    unsigned internalformat, int x, int y, size_t width,
    size_t height){}

DECL_EXP void  glGetConvolutionFilter( unsigned target, unsigned format,
    unsigned type, void *image ){}

DECL_EXP void  glGetConvolutionParameterfv( unsigned target, unsigned pname,
    float *params ){}

DECL_EXP void  glGetConvolutionParameteriv( unsigned target, unsigned pname,
    int *params ){}

DECL_EXP void  glSeparableFilter2D( unsigned target,
    unsigned internalformat, size_t width, size_t height, unsigned format,
    unsigned type, const void *row, const void *column ){}

DECL_EXP void  glGetSeparableFilter( unsigned target, unsigned format,
    unsigned type, void *row, void *column, void *span ){}


/*
 * OpenGL 1.3
 */

/* multitexture */

DECL_EXP void  glActiveTexture( unsigned texture ){}

DECL_EXP void  glClientActiveTexture( unsigned texture ){}

DECL_EXP void  glCompressedTexImage1D( unsigned target, int level, unsigned internalformat, size_t width, int border, size_t imageSize, const void *data ){}

DECL_EXP void  glCompressedTexImage2D( unsigned target, int level, unsigned internalformat, size_t width, size_t height, int border, size_t imageSize, const void *data ){}

DECL_EXP void  glCompressedTexImage3D( unsigned target, int level, unsigned internalformat, size_t width, size_t height, size_t depth, int border, size_t imageSize, const void *data ){}

DECL_EXP void  glCompressedTexSubImage1D( unsigned target, int level, int xoffset, size_t width, unsigned format, size_t imageSize, const void *data ){}

DECL_EXP void  glCompressedTexSubImage2D( unsigned target, int level, int xoffset, int yoffset, size_t width, size_t height, unsigned format, size_t imageSize, const void *data ){}

DECL_EXP void  glCompressedTexSubImage3D( unsigned target, int level, int xoffset, int yoffset, int zoffset, size_t width, size_t height, size_t depth, unsigned format, size_t imageSize, const void *data ){}

DECL_EXP void  glGetCompressedTexImage( unsigned target, int lod, void *img ){}

DECL_EXP void  glMultiTexCoord1d( unsigned target, double s ){}

DECL_EXP void  glMultiTexCoord1dv( unsigned target, const double *v ){}

DECL_EXP void  glMultiTexCoord1f( unsigned target, float s ){}

DECL_EXP void  glMultiTexCoord1fv( unsigned target, const float *v ){}

DECL_EXP void  glMultiTexCoord1i( unsigned target, int s ){}

DECL_EXP void  glMultiTexCoord1iv( unsigned target, const int *v ){}

DECL_EXP void  glMultiTexCoord1s( unsigned target, short s ){}

DECL_EXP void  glMultiTexCoord1sv( unsigned target, const short *v ){}

DECL_EXP void  glMultiTexCoord2d( unsigned target, double s, double t ){}

DECL_EXP void  glMultiTexCoord2dv( unsigned target, const double *v ){}

DECL_EXP void  glMultiTexCoord2f( unsigned target, float s, float t ){}

DECL_EXP void  glMultiTexCoord2fv( unsigned target, const float *v ){}

DECL_EXP void  glMultiTexCoord2i( unsigned target, int s, int t ){}

DECL_EXP void  glMultiTexCoord2iv( unsigned target, const int *v ){}

DECL_EXP void  glMultiTexCoord2s( unsigned target, short s, short t ){}

DECL_EXP void  glMultiTexCoord2sv( unsigned target, const short *v ){}

DECL_EXP void  glMultiTexCoord3d( unsigned target, double s, double t, double r ){}

DECL_EXP void  glMultiTexCoord3dv( unsigned target, const double *v ){}

DECL_EXP void  glMultiTexCoord3f( unsigned target, float s, float t, float r ){}

DECL_EXP void  glMultiTexCoord3fv( unsigned target, const float *v ){}

DECL_EXP void  glMultiTexCoord3i( unsigned target, int s, int t, int r ){}

DECL_EXP void  glMultiTexCoord3iv( unsigned target, const int *v ){}

DECL_EXP void  glMultiTexCoord3s( unsigned target, short s, short t, short r ){}

DECL_EXP void  glMultiTexCoord3sv( unsigned target, const short *v ){}

DECL_EXP void  glMultiTexCoord4d( unsigned target, double s, double t, double r, double q ){}

DECL_EXP void  glMultiTexCoord4dv( unsigned target, const double *v ){}

DECL_EXP void  glMultiTexCoord4f( unsigned target, float s, float t, float r, float q ){}

DECL_EXP void  glMultiTexCoord4fv( unsigned target, const float *v ){}

DECL_EXP void  glMultiTexCoord4i( unsigned target, int s, int t, int r, int q ){}

DECL_EXP void  glMultiTexCoord4iv( unsigned target, const int *v ){}

DECL_EXP void  glMultiTexCoord4s( unsigned target, short s, short t, short r, short q ){}

DECL_EXP void  glMultiTexCoord4sv( unsigned target, const short *v ){}


DECL_EXP void  glLoadTransposeMatrixd( const double m[16] ){}

DECL_EXP void  glLoadTransposeMatrixf( const float m[16] ){}

DECL_EXP void  glMultTransposeMatrixd( const double m[16] ){}

DECL_EXP void  glMultTransposeMatrixf( const float m[16] ){}

DECL_EXP void  glSampleCoverage( unsigned value, bool invert ){}


/*
 * GL_ARB_multitexture (ARB extension 1 and OpenGL 1.2.1)
 */
DECL_EXP void  glActiveTextureARB(unsigned texture){}
DECL_EXP void  glClientActiveTextureARB(unsigned texture){}
DECL_EXP void  glMultiTexCoord1dARB(unsigned target, double s){}
DECL_EXP void  glMultiTexCoord1dvARB(unsigned target, const double *v){}
DECL_EXP void  glMultiTexCoord1fARB(unsigned target, float s){}
DECL_EXP void  glMultiTexCoord1fvARB(unsigned target, const float *v){}
DECL_EXP void  glMultiTexCoord1iARB(unsigned target, int s){}
DECL_EXP void  glMultiTexCoord1ivARB(unsigned target, const int *v){}
DECL_EXP void  glMultiTexCoord1sARB(unsigned target, short s){}
DECL_EXP void  glMultiTexCoord1svARB(unsigned target, const short *v){}
DECL_EXP void  glMultiTexCoord2dARB(unsigned target, double s, double t){}
DECL_EXP void  glMultiTexCoord2dvARB(unsigned target, const double *v){}
DECL_EXP void  glMultiTexCoord2fARB(unsigned target, float s, float t){}
DECL_EXP void  glMultiTexCoord2fvARB(unsigned target, const float *v){}
DECL_EXP void  glMultiTexCoord2iARB(unsigned target, int s, int t){}
DECL_EXP void  glMultiTexCoord2ivARB(unsigned target, const int *v){}
DECL_EXP void  glMultiTexCoord2sARB(unsigned target, short s, short t){}
DECL_EXP void  glMultiTexCoord2svARB(unsigned target, const short *v){}
DECL_EXP void  glMultiTexCoord3dARB(unsigned target, double s, double t, double r){}
DECL_EXP void  glMultiTexCoord3dvARB(unsigned target, const double *v){}
DECL_EXP void  glMultiTexCoord3fARB(unsigned target, float s, float t, float r){}
DECL_EXP void  glMultiTexCoord3fvARB(unsigned target, const float *v){}
DECL_EXP void  glMultiTexCoord3iARB(unsigned target, int s, int t, int r){}
DECL_EXP void  glMultiTexCoord3ivARB(unsigned target, const int *v){}
DECL_EXP void  glMultiTexCoord3sARB(unsigned target, short s, short t, short r){}
DECL_EXP void  glMultiTexCoord3svARB(unsigned target, const short *v){}
DECL_EXP void  glMultiTexCoord4dARB(unsigned target, double s, double t, double r, double q){}
DECL_EXP void  glMultiTexCoord4dvARB(unsigned target, const double *v){}
DECL_EXP void  glMultiTexCoord4fARB(unsigned target, float s, float t, float r, float q){}
DECL_EXP void  glMultiTexCoord4fvARB(unsigned target, const float *v){}
DECL_EXP void  glMultiTexCoord4iARB(unsigned target, int s, int t, int r, int q){}
DECL_EXP void  glMultiTexCoord4ivARB(unsigned target, const int *v){}
DECL_EXP void  glMultiTexCoord4sARB(unsigned target, short s, short t, short r, short q){}
DECL_EXP void  glMultiTexCoord4svARB(unsigned target, const short *v){}

//DECL_EXP void  glProgramCallbackMESA(unsigned target, GLprogramcallbackMESA callback, void *data){}

DECL_EXP void  glGetProgramRegisterfvMESA(unsigned target, size_t len, const unsigned char *name, float *v){}


DECL_EXP void  glBlendEquationSeparateATI( unsigned modeRGB, unsigned modeA ){}

}  // extern "C"


void RemovePlugInTool(int tool_id) {}
DECL_EXP int InsertPlugInToolSVG(wxString label, wxString SVGfile,
                        wxString SVGfileRollover, wxString SVGfileToggled,
                        wxItemKind kind, wxString shortHelp, wxString longHelp,
                        wxObject *clientData, int position, int tool_sel,
                        opencpn_plugin *pplugin) { return 0; }

DECL_EXP int InsertPlugInTool(wxString label, wxBitmap *bitmap, wxBitmap *bmpRollover,
                     wxItemKind kind, wxString shortHelp, wxString longHelp,
                     wxObject *clientData, int position, int tool_sel,
                     opencpn_plugin *pplugin) { return 0; }
void DimeWindow(wxWindow* win) {}
wxFont* OCPNGetFont(wxString TextElement, int default_size) { return 0; }
void SetToolbarItemState(int item, bool toggle) {}
wxAuiManager *GetFrameAuiManager(void) { return 0; }
wxWindow *GetOCPNCanvasWindow() { return 0; }

DECL_EXP int AddCanvasContextMenuItem(wxMenuItem *pitem,
                                      opencpn_plugin *pplugin) {
  return 1;
}
DECL_EXP void SetToolbarToolBitmaps(int item, wxBitmap *bitmap, wxBitmap *bmpRollover) {}
DECL_EXP void SetToolbarToolBitmapsSVG(int item, wxString SVGfile,
                              wxString SVGfileRollover,
                              wxString SVGfileToggled) {}
DECL_EXP void RemoveCanvasContextMenuItem(int item) {}
DECL_EXP void JumpToPosition(double lat, double lon, double scale){};
DECL_EXP void SetCanvasContextMenuItemViz(int item, bool viz) {}
DECL_EXP void SetCanvasContextMenuItemGrey(int item, bool grey) {}
DECL_EXP void RequestRefresh(wxWindow *) {}
DECL_EXP void DistanceBearingMercator_Plugin(double lat0, double lon0,
                                             double lat1, double lon1,
                                             double *brg, double *dist) {}
DECL_EXP void GetCanvasPixLL(PlugIn_ViewPort *vp, wxPoint *pp, double lat,
                             double lon) {}
DECL_EXP void GetCanvasLLPix(PlugIn_ViewPort *vp, wxPoint p,
                             double *plat, double *plon) {}
DECL_EXP void PushNMEABuffer(wxString str) {}
DECL_EXP void PositionBearingDistanceMercator_Plugin(double lat, double lon,
                                                     double brg, double dist,
                                                     double *dlat,
                                                     double *dlon) {}
DECL_EXP double DistGreatCircle_Plugin(double slat, double slon, double dlat,
                                       double dlon) {
  return 0.0;
}
wxXmlDocument dummy_xml_doc;
DECL_EXP wxXmlDocument GetChartDatabaseEntryXML(int dbIndex, bool b_getGeom) {
  return dummy_xml_doc;
}
DECL_EXP bool UpdateChartDBInplace(wxArrayString dir_array, bool b_force_update,
                                   bool b_ProgressDialog) {
  return true;
}
wxArrayString dummy_array_string;
DECL_EXP wxArrayString GetChartDBDirArrayString() { return dummy_array_string; }
DECL_EXP void toTM_Plugin(float lat, float lon, float lat0, float lon0,
                          double *x, double *y) {}
DECL_EXP void fromTM_Plugin(double x, double y, double lat0, double lon0,
                            double *lat, double *lon) {}
DECL_EXP void toSM_Plugin(double lat, double lon, double lat0, double lon0,
                          double *x, double *y) {}
DECL_EXP void fromSM_Plugin(double x, double y, double lat0, double lon0,
                            double *lat, double *lon) {}
DECL_EXP void toSM_ECC_Plugin(double lat, double lon, double lat0, double lon0,
                              double *x, double *y) {}
DECL_EXP void fromSM_ECC_Plugin(double x, double y, double lat0, double lon0,
                                double *lat, double *lon) {}
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

DECL_EXP double toUsrDistance_Plugin(double nm_distance, int unit) {
  return 0.0;
}
DECL_EXP double fromUsrDistance_Plugin(double usr_distance, int unit) {
  return 0.0;
}
DECL_EXP double toUsrSpeed_Plugin(double kts_speed, int unit) { return 0.0; }
DECL_EXP double fromUsrSpeed_Plugin(double usr_speed, int unit) { return 0.0; }
DECL_EXP double toUsrTemp_Plugin(double cel_temp, int unit) { return 0.0; }
DECL_EXP double fromUsrTemp_Plugin(double usr_temp, int unit) { return 0.0; }
DECL_EXP wxString getUsrDistanceUnit_Plugin(int unit) { return ""; }
DECL_EXP wxString getUsrSpeedUnit_Plugin(int unit) { return ""; }
DECL_EXP wxString getUsrTempUnit_Plugin(int unit) { return ""; }
DECL_EXP wxString GetNewGUID() { return ""; }
DECL_EXP bool PlugIn_GSHHS_CrossesLand(double lat1, double lon1, double lat2,
                                       double lon2) {
  return true;
}

DECL_EXP void PlugInPlaySound(wxString &sound_file){};

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

DECL_EXP wxString toSDMM_PlugIn(int NEflag, double a, bool hi_precision) {
  return "";
}
wxString dummy_string;
DECL_EXP wxString *GetpPrivateApplicationDataLocation() {
  return &dummy_string;
}
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

double PlugInChartBase::GetNormalScaleMin(double canvas_scale_factor,
                                          bool b_allow_overzoom) {
  return 0.0;
}
double PlugInChartBase::GetNormalScaleMax(double canvas_scale_factor,
                                          int canvas_width) {
  return 0.0;
}
double PlugInChartBase::GetNearestPreferredScalePPM(double target_scale_ppm) {
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
double PlugInChartBase::GetRasterScaleFactor() { return 0.0; }
bool PlugInChartBase::GetChartBits(wxRect &source, unsigned char *pPix,
                                   int sub_samp) {
  return true;
}
int PlugInChartBase::GetSize_X() { return 0; }
int PlugInChartBase::GetSize_Y() { return 0; }
void PlugInChartBase::latlong_to_chartpix(double lat, double lon, double &pixx,
                                          double &pixy) {}
void PlugInChartBase::chartpix_to_latlong(double pixx, double pixy,
                                          double *plat, double *plon) {}

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
    float lat, float lon, float select_radius, PlugIn_ViewPort *VPoint) {
  return 0;
}
wxString PlugInChartBaseGL::CreateObjDescriptions(ListOfPI_S57Obj *obj_list) {
  return "";
}

int PlugInChartBaseGL::GetNoCOVREntries() { return 0; }
int PlugInChartBaseGL::GetNoCOVRTablePoints(int iTable) { return 0; }
int PlugInChartBaseGL::GetNoCOVRTablenPoints(int iTable) { return 0; }
float *PlugInChartBaseGL::GetNoCOVRTableHead(int iTable) { return 0; }

// ----------------------------------------------------------------------------
// PlugInChartBaseGLPlus2
//  Derived from PlugInChartBaseGL, add additional chart management methods
// ----------------------------------------------------------------------------

PlugInChartBaseGLPlus2::PlugInChartBaseGLPlus2() {}
PlugInChartBaseGLPlus2::~PlugInChartBaseGLPlus2() {}

ListOfPI_S57Obj *PlugInChartBaseGLPlus2::GetLightsObjRuleListVisibleAtLatLon(
    float lat, float lon, PlugIn_ViewPort *VPoint) {
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
    float lat, float lon, float select_radius, PlugIn_ViewPort *VPoint) {
  return 0;
}
wxString PlugInChartBaseExtended::CreateObjDescriptions(
    ListOfPI_S57Obj *obj_list) {
  return wxString("");
}

int PlugInChartBaseExtended::GetNoCOVREntries() { return 0; }
int PlugInChartBaseExtended::GetNoCOVRTablePoints(int iTable) { return 0; }
int PlugInChartBaseExtended::GetNoCOVRTablenPoints(int iTable) { return 0; }
float *PlugInChartBaseExtended::GetNoCOVRTableHead(int iTable) { return 0; }

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
    float lat, float lon, PlugIn_ViewPort *VPoint) {
  return 0;
}

class wxArrayOfS57attVal;

PI_S57Obj::PI_S57Obj() {}

DECL_EXP wxString DECL_EXP PI_GetPLIBColorScheme() { return ""; }
DECL_EXP int PI_GetPLIBDepthUnitInt() { return 0; }
DECL_EXP int PI_GetPLIBSymbolStyle() { return 0; }
DECL_EXP int PI_GetPLIBBoundaryStyle() { return 0; }
DECL_EXP int PI_GetPLIBStateHash() { return 0; }
DECL_EXP double PI_GetPLIBMarinerSafetyContour() { return 0; }
DECL_EXP bool PI_GetObjectRenderBox(PI_S57Obj *pObj, double *lat_min,
                                    double *lat_max, double *lon_min,
                                    double *lon_max) {
  return true;
}
DECL_EXP void DECL_EXP PI_UpdateContext(PI_S57Obj *pObj) {}
DECL_EXP bool DECL_EXP PI_PLIBObjectRenderCheck(PI_S57Obj *pObj,
                                                PlugIn_ViewPort *vp) {
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
DECL_EXP int DECL_EXP PI_PLIBRenderObjectToDC(wxDC *pdc, PI_S57Obj *pObj,
                                              PlugIn_ViewPort *vp) {
  return 0;
}
DECL_EXP int DECL_EXP PI_PLIBRenderAreaToDC(wxDC *pdc, PI_S57Obj *pObj,
                                            PlugIn_ViewPort *vp, wxRect rect,
                                            unsigned char *pixbuf) {
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
DECL_EXP void PlugInMultMatrixViewport(PlugIn_ViewPort *vp, float lat,
                                       float lon) {}
DECL_EXP void PlugInNormalizeViewport(PlugIn_ViewPort *vp, float lat,
                                      float lon) {}

class wxPoint2DDouble;
DECL_EXP void GetDoubleCanvasPixLL(PlugIn_ViewPort *vp, wxPoint2DDouble *pp,
                                   double lat, double lon) {}

DECL_EXP double fromDMM_Plugin(wxString sdms) { return 0.0; }
DECL_EXP void SetCanvasRotation(double rotation) {}
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
DECL_EXP double GetOCPNGUIToolScaleFactor_PlugIn(int GUIScaledFactor) {
  return 0.0;
}
DECL_EXP double GetOCPNGUIToolScaleFactor_PlugIn() { return 0.0; }
DECL_EXP float GetOCPNChartScaleFactor_Plugin() { return 0.0; }
DECL_EXP wxColour GetFontColour_PlugIn(wxString TextElement) { return *wxRED; }

DECL_EXP double GetCanvasTilt() { return 0.0; }
DECL_EXP void SetCanvasTilt(double tilt) {}

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
DECL_EXP double PlugInGetDisplaySizeMM() { return 0.0; }

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
DECL_EXP bool ShuttingDown(void) { return true; }

DECL_EXP wxWindow *PluginGetFocusCanvas() { return 0; }
DECL_EXP wxWindow *PluginGetOverlayRenderCanvas() { return 0; }

DECL_EXP void CanvasJumpToPosition(wxWindow *canvas, double lat, double lon,
                                   double scale) {}
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
DECL_EXP PlugIn_Waypoint::~PlugIn_Waypoint() {}
DECL_EXP PlugIn_Waypoint_Ex::PlugIn_Waypoint_Ex() {}
DECL_EXP PlugIn_Waypoint_Ex::PlugIn_Waypoint_Ex(
    double lat, double lon, const wxString &icon_ident, const wxString &wp_name,
    const wxString &GUID, const double ScaMin, const bool bNameVisible,
    const int nRanges, const double RangeDistance, const wxColor RangeColor) {}
DECL_EXP PlugIn_Waypoint_Ex::~PlugIn_Waypoint_Ex() {}
DECL_EXP void PlugIn_Waypoint_Ex::InitDefaults() {}

DECL_EXP bool PlugIn_Waypoint_Ex::GetFSStatus() { return true; }

DECL_EXP int PlugIn_Waypoint_Ex::GetRouteMembershipCount() { return 0; }

DECL_EXP PlugIn_Route::PlugIn_Route(void) {}
DECL_EXP PlugIn_Route::~PlugIn_Route(void) {}

DECL_EXP PlugIn_Route_Ex::PlugIn_Route_Ex(void) {}
DECL_EXP PlugIn_Route_Ex::~PlugIn_Route_Ex(void) {}

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
