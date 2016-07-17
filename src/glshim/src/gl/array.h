#include "gl.h"

#ifndef GL_ARRAY_H
#define GL_ARRAY_H

#include "types.h"

GLvoid *gl_copy_array(const GLvoid *src,
                      GLenum from, GLsizei width, GLsizei stride,
                      GLenum to, GLsizei to_width, GLsizei skip, GLsizei count,
                      GLboolean normalize);

GLvoid *gl_copy_pointer(pointer_state_t *ptr, GLsizei width, GLsizei skip, GLsizei count, GLboolean);
GLfloat *gl_pointer_index(pointer_state_t *ptr, GLint index);
GLfloat *copy_eval_double(GLenum target, GLint ustride, GLint uorder, GLint vstride, GLint vorder, const GLdouble *points);
void normalize_indices(GLushort *indices, GLsizei *max, GLsizei *min, GLsizei count);
#endif
