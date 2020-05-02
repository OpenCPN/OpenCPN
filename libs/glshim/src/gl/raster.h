#include "gl.h"

#ifndef RASTER_H
#define RASTER_H

typedef struct {
    GLfloat x;
    GLfloat y;
    GLfloat z;
} rasterpos_t;

typedef struct {
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
} viewport_t;

extern void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig,
                     GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
extern void glDrawPixels(GLsizei width, GLsizei height, GLenum format,
                         GLenum type, const GLvoid *data);
extern void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z);
extern void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
extern void render_raster();

#endif
