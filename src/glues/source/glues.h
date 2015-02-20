/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice including the dates of first publication and
 * either this permission notice or a reference to
 * http://oss.sgi.com/projects/FreeB/
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * SILICON GRAPHICS, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Silicon Graphics, Inc.
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization from
 * Silicon Graphics, Inc.
 *
 * OpenGL ES 1.0 CM port of part of GLU by Mike Gorchak <mike@malva.ua>
 */

#ifndef __glues_h__
#define __glues_h__

#if defined(__USE_SDL_GLES__)
   #include <SDL/SDL_opengles.h>
   #ifndef GLAPI
      #define GLAPI GL_API
   #endif
#elif defined (__QNXNTO__)
   #include <GLES/gl.h>
#elif defined(_WIN32) && (defined(_M_IX86) || defined(_M_X64))
   /* mainly for PowerVR OpenGL ES 1.x win32 emulator */
   #include <GLES\gl.h>
   #undef APIENTRY
   #define APIENTRY
   #if defined(GLUES_EXPORTS)
      #define GLAPI __declspec(dllexport)
   #else
      #define GLAPI __declspec(dllimport)
   #endif
#elif defined (ANDROID)
  #include <GLES/gl.h>
  #include <GLES/glext.h>

    #ifndef APIENTRY
        #define APIENTRY
    #endif
    #ifndef GLAPI
        #define GLAPI
    #endif
#else
   #error "Platform is unsupported"
#endif

#ifndef APIENTRYP
   #define APIENTRYP APIENTRY *
#endif /* APIENTRYP */

#ifdef __cplusplus
   extern "C" {
#endif

/*************************************************************/

/* Boolean */
#define GLU_FALSE                          0
#define GLU_TRUE                           1

/* Version */
#define GLU_VERSION_1_1                    1
#define GLU_VERSION_1_2                    1
#define GLU_VERSION_1_3                    1

/* StringName */
#define GLU_VERSION                        100800
#define GLU_EXTENSIONS                     100801

/* ErrorCode */
#define GLU_INVALID_ENUM                   100900
#define GLU_INVALID_VALUE                  100901
#define GLU_OUT_OF_MEMORY                  100902
#define GLU_INCOMPATIBLE_GL_VERSION        100903
#define GLU_INVALID_OPERATION              100904

/* QuadricDrawStyle */
#define GLU_POINT                          100010
#define GLU_LINE                           100011
#define GLU_FILL                           100012
#define GLU_SILHOUETTE                     100013

/* QuadricCallback */
#define GLU_ERROR                          100103

/* QuadricNormal */
#define GLU_SMOOTH                         100000
#define GLU_FLAT                           100001
#define GLU_NONE                           100002

/* QuadricOrientation */
#define GLU_OUTSIDE                        100020
#define GLU_INSIDE                         100021

/*************************************************************/


#ifdef __cplusplus
class GLUquadric;
class GLUtesselator;
#else
typedef struct GLUquadric GLUquadric;
typedef struct GLUtesselator GLUtesselator;
#endif

typedef GLUquadric GLUquadricObj;
typedef GLUtesselator GLUtesselatorObj;
typedef GLUtesselator GLUtriangulatorObj;

/* Internal convenience typedefs */
typedef void (APIENTRYP _GLUfuncptr)();

GLAPI GLboolean APIENTRY gluCheckExtension(const GLubyte* extName, const GLubyte* extString);
GLAPI void APIENTRY gluCylinder(GLUquadric* quad, GLfloat base, GLfloat top, GLfloat height, GLint slices, GLint stacks);
GLAPI void APIENTRY gluDeleteQuadric(GLUquadric* quad);
GLAPI void APIENTRY gluDisk(GLUquadric* quad, GLfloat inner, GLfloat outer, GLint slices, GLint loops);
GLAPI const GLubyte* APIENTRY gluErrorString(GLenum error);
GLAPI const GLubyte * APIENTRY gluGetString(GLenum name);
GLAPI void APIENTRY gluLookAt(GLfloat eyeX, GLfloat eyeY, GLfloat eyeZ, GLfloat centerX, GLfloat centerY, GLfloat centerZ, GLfloat upX, GLfloat upY, GLfloat upZ);
GLAPI GLUquadric* APIENTRY gluNewQuadric(void);
GLAPI void APIENTRY gluOrtho2D(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top);
GLAPI void APIENTRY gluPartialDisk(GLUquadric* quad, GLfloat inner, GLfloat outer, GLint slices, GLint loops, GLfloat start, GLfloat sweep);
GLAPI void APIENTRY gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
GLAPI void APIENTRY gluPickMatrix(GLfloat x, GLfloat y, GLfloat delX, GLfloat delY, GLint *viewport);
GLAPI GLint APIENTRY gluProject(GLfloat objX, GLfloat objY, GLfloat objZ, const GLfloat *model, const GLfloat *proj, const GLint *view, GLfloat* winX, GLfloat* winY, GLfloat* winZ);
GLAPI void APIENTRY gluQuadricCallback(GLUquadric* quad, GLenum which, _GLUfuncptr CallBackFunc);
GLAPI void APIENTRY gluQuadricDrawStyle(GLUquadric* quad, GLenum draw);
GLAPI void APIENTRY gluQuadricNormals(GLUquadric* quad, GLenum normal);
GLAPI void APIENTRY gluQuadricOrientation(GLUquadric* quad, GLenum orientation);
GLAPI void APIENTRY gluQuadricTexture(GLUquadric* quad, GLboolean texture);
GLAPI void APIENTRY gluSphere(GLUquadric* quad, GLfloat radius, GLint slices, GLint stacks);
GLAPI GLint APIENTRY gluUnProject(GLfloat winX, GLfloat winY, GLfloat winZ, const GLfloat *model, const GLfloat *proj, const GLint *view, GLfloat* objX, GLfloat* objY, GLfloat* objZ);
GLAPI GLint APIENTRY gluUnProject4(GLfloat winX, GLfloat winY, GLfloat winZ, GLfloat clipW, const GLfloat *model, const GLfloat *proj, const GLint *view, GLfloat nearVal, GLfloat farVal, GLfloat* objX, GLfloat* objY, GLfloat* objZ, GLfloat* objW);
GLAPI GLint APIENTRY gluScaleImage(GLenum format, GLsizei widthin,
                             GLsizei heightin, GLenum typein,
                             const void* datain, GLsizei widthout,
                             GLsizei heightout, GLenum typeout, void* dataout);
GLAPI GLint APIENTRY gluBuild2DMipmapLevels(GLenum target, GLint internalFormat,
                             GLsizei width, GLsizei height, GLenum format,
                             GLenum type, GLint userLevel, GLint baseLevel,
                             GLint maxLevel, const void *data);
GLAPI GLint APIENTRY gluBuild2DMipmaps(GLenum target, GLint internalFormat,
                             GLsizei width, GLsizei height, GLenum format,
                             GLenum type, const void* data);

#define GLU_TESS_MAX_COORD 1.0e37f

/* TessCallback */
#define GLU_TESS_BEGIN                     100100
#define GLU_BEGIN                          100100
#define GLU_TESS_VERTEX                    100101
#define GLU_VERTEX                         100101
#define GLU_TESS_END                       100102
#define GLU_END                            100102
#define GLU_TESS_ERROR                     100103
#define GLU_TESS_EDGE_FLAG                 100104
#define GLU_EDGE_FLAG                      100104
#define GLU_TESS_COMBINE                   100105
#define GLU_TESS_BEGIN_DATA                100106
#define GLU_TESS_VERTEX_DATA               100107
#define GLU_TESS_END_DATA                  100108
#define GLU_TESS_ERROR_DATA                100109
#define GLU_TESS_EDGE_FLAG_DATA            100110
#define GLU_TESS_COMBINE_DATA              100111

/* TessContour */
#define GLU_CW                             100120
#define GLU_CCW                            100121
#define GLU_INTERIOR                       100122
#define GLU_EXTERIOR                       100123
#define GLU_UNKNOWN                        100124

/* TessProperty */
#define GLU_TESS_WINDING_RULE              100140
#define GLU_TESS_BOUNDARY_ONLY             100141
#define GLU_TESS_TOLERANCE                 100142

/* TessError */
#define GLU_TESS_ERROR1                    100151
#define GLU_TESS_ERROR2                    100152
#define GLU_TESS_ERROR3                    100153
#define GLU_TESS_ERROR4                    100154
#define GLU_TESS_ERROR5                    100155
#define GLU_TESS_ERROR6                    100156
#define GLU_TESS_ERROR7                    100157
#define GLU_TESS_ERROR8                    100158
#define GLU_TESS_MISSING_BEGIN_POLYGON     100151
#define GLU_TESS_MISSING_BEGIN_CONTOUR     100152
#define GLU_TESS_MISSING_END_POLYGON       100153
#define GLU_TESS_MISSING_END_CONTOUR       100154
#define GLU_TESS_COORD_TOO_LARGE           100155
#define GLU_TESS_NEED_COMBINE_CALLBACK     100156

/* TessWinding */
#define GLU_TESS_WINDING_ODD               100130
#define GLU_TESS_WINDING_NONZERO           100131
#define GLU_TESS_WINDING_POSITIVE          100132
#define GLU_TESS_WINDING_NEGATIVE          100133
#define GLU_TESS_WINDING_ABS_GEQ_TWO       100134

GLAPI void APIENTRY gluBeginPolygon(GLUtesselator* tess);
GLAPI void APIENTRY gluDeleteTess(GLUtesselator* tess);
GLAPI void APIENTRY gluEndPolygon(GLUtesselator* tess);
GLAPI void APIENTRY gluGetTessProperty(GLUtesselator* tess, GLenum which, GLfloat* data);
GLAPI GLUtesselator* APIENTRY gluNewTess(void);
GLAPI void APIENTRY gluNextContour(GLUtesselator* tess, GLenum type);
GLAPI void APIENTRY gluTessBeginContour(GLUtesselator* tess);
GLAPI void APIENTRY gluTessBeginPolygon(GLUtesselator* tess, GLvoid* data);
GLAPI void APIENTRY gluTessCallback(GLUtesselator* tess, GLenum which, _GLUfuncptr CallBackFunc);
GLAPI void APIENTRY gluTessEndContour(GLUtesselator* tess);
GLAPI void APIENTRY gluTessEndPolygon(GLUtesselator* tess);
GLAPI void APIENTRY gluTessNormal(GLUtesselator* tess, GLfloat valueX, GLfloat valueY, GLfloat valueZ);
GLAPI void APIENTRY gluTessProperty(GLUtesselator* tess, GLenum which, GLfloat data);
GLAPI void APIENTRY gluTessVertex(GLUtesselator* tess, GLfloat* location, GLvoid* data);

/* NurbsError */
#define GLU_NURBS_ERROR1                   100251
#define GLU_NURBS_ERROR2                   100252
#define GLU_NURBS_ERROR3                   100253
#define GLU_NURBS_ERROR4                   100254
#define GLU_NURBS_ERROR5                   100255
#define GLU_NURBS_ERROR6                   100256
#define GLU_NURBS_ERROR7                   100257
#define GLU_NURBS_ERROR8                   100258
#define GLU_NURBS_ERROR9                   100259
#define GLU_NURBS_ERROR10                  100260
#define GLU_NURBS_ERROR11                  100261
#define GLU_NURBS_ERROR12                  100262
#define GLU_NURBS_ERROR13                  100263
#define GLU_NURBS_ERROR14                  100264
#define GLU_NURBS_ERROR15                  100265
#define GLU_NURBS_ERROR16                  100266
#define GLU_NURBS_ERROR17                  100267
#define GLU_NURBS_ERROR18                  100268
#define GLU_NURBS_ERROR19                  100269
#define GLU_NURBS_ERROR20                  100270
#define GLU_NURBS_ERROR21                  100271
#define GLU_NURBS_ERROR22                  100272
#define GLU_NURBS_ERROR23                  100273
#define GLU_NURBS_ERROR24                  100274
#define GLU_NURBS_ERROR25                  100275
#define GLU_NURBS_ERROR26                  100276
#define GLU_NURBS_ERROR27                  100277
#define GLU_NURBS_ERROR28                  100278
#define GLU_NURBS_ERROR29                  100279
#define GLU_NURBS_ERROR30                  100280
#define GLU_NURBS_ERROR31                  100281
#define GLU_NURBS_ERROR32                  100282
#define GLU_NURBS_ERROR33                  100283
#define GLU_NURBS_ERROR34                  100284
#define GLU_NURBS_ERROR35                  100285
#define GLU_NURBS_ERROR36                  100286
#define GLU_NURBS_ERROR37                  100287

#ifdef __cplusplus
}
#endif

#endif /* __glues_h__ */
