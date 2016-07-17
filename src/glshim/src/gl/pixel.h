#ifndef PIXEL_H
#define PIXEL_H

#include <GL/gl.h>
#include <stdbool.h>

typedef struct {
    GLenum type;
    GLint red, green, blue, alpha;
} colorlayout_t;

typedef struct {
    GLfloat r, g, b, a;
} pixel_t;

bool pixel_convert(const GLvoid *src, GLvoid **dst,
                   GLuint width, GLuint height,
                   GLenum src_format, GLenum src_type,
                   GLenum dst_format, GLenum dst_type);

bool pixel_convert_direct(const GLvoid *src, GLvoid *dst, GLuint pixels,
                          GLenum src_format, GLenum src_type, GLsizei src_stride,
                          GLenum dst_format, GLenum dst_type, GLsizei dst_stride);

bool pixel_scale(const GLvoid *src, GLvoid **dst,
                  GLuint width, GLuint height,
                  GLfloat ratio,
                  GLenum format, GLenum type);

bool pixel_to_ppm(const GLvoid *pixels,
                  GLuint width, GLuint height,
                  GLenum format, GLenum type, GLuint name);

#endif
