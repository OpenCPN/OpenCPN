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
 * OpenGL ES CM 1.0 port of part of GLU by Mike Gorchak <mike@malva.ua>
 */

#ifndef __GLUES_PROJECT_H__
#define __GLUES_PROJECT_H__

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
  #define APIENTRY
  #define GLAPI
#else
   #error "Platform is unsupported"
#endif

#ifdef __cplusplus
   extern "C" {
#endif

GLAPI void APIENTRY gluOrtho2D(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top);
GLAPI void APIENTRY gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
GLAPI void APIENTRY gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
                              GLfloat centerx, GLfloat centery, GLfloat centerz,
                              GLfloat upx, GLfloat upy, GLfloat upz);
GLAPI GLint APIENTRY gluProject(GLfloat objx, GLfloat objy, GLfloat objz,
                                const GLfloat modelMatrix[16], const GLfloat projMatrix[16],
                                const GLint viewport[4], GLfloat* winx, GLfloat* winy, GLfloat* winz);
GLAPI GLint APIENTRY gluUnProject(GLfloat winx, GLfloat winy, GLfloat winz,
                                  const GLfloat modelMatrix[16], const GLfloat projMatrix[16],
                                  const GLint viewport[4], GLfloat* objx, GLfloat* objy, GLfloat* objz);
GLAPI GLint APIENTRY gluUnProject4(GLfloat winx, GLfloat winy, GLfloat winz, GLfloat clipw,
                                   const GLfloat modelMatrix[16], const GLfloat projMatrix[16],
                                   const GLint viewport[4], GLclampf nearVal, GLclampf farVal,
                                   GLfloat* objx, GLfloat* objy, GLfloat* objz, GLfloat* objw);
GLAPI void APIENTRY gluPickMatrix(GLfloat x, GLfloat y, GLfloat deltax, GLfloat deltay, GLint viewport[4]);

#ifdef __cplusplus
}
#endif

#endif /* __GLUES_PROJECT_H__ */
