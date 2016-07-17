#ifndef GL_MATRIX_H
#define GL_MATRIX_H

#include <GL/gl.h>

void glLoadIdentity();
void glLoadMatrixf(const GLfloat *m);
void glMatrixMode(GLenum mode);
void glMultMatrixf(const GLfloat *m);
void glPopMatrix();
void glPushMatrix();
void gl_get_matrix(GLenum mode, GLfloat *out);
void gl_transform_light(GLfloat out[3], const GLfloat in[3]);
void gl_transform_texture(GLenum texture, GLfloat out[2], const GLfloat in[2]);
void gl_transform_vertex(GLfloat out[3], GLfloat in[3]);

#endif
