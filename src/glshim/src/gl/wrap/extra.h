#include "../gl.h"
#include <GLES/glext.h>

#ifndef GL_WRAP_H
#define GL_WRAP_H

#ifdef USE_ES2
void glCompileShaderARB(GLuint shader);
GLuint glCreateShaderObjectARB(GLenum shaderType);
void glGetObjectParameterivARB(GLuint shader, GLenum pname, GLint *params);
void glShaderSourceARB(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
#endif

#define THUNK(suffix, type)                                \
void glColor3##suffix##v(const type *v);                   \
void glColor3##suffix(type r, type g, type b);             \
void glColor4##suffix##v(const type *v);                   \
void glColor4##suffix(type r, type g, type b, type a);     \
void glSecondaryColor3##suffix##v(const type *v);          \
void glSecondaryColor3##suffix(type r, type g, type b);    \
void glIndex##suffix##v(const type *c);                    \
void glIndex##suffix(type c);                              \
void glNormal3##suffix##v(const type *v);                  \
void glNormal3##suffix(type x, type y, type z);            \
void glRasterPos2##suffix##v(const type *v);               \
void glRasterPos2##suffix(type x, type y);                 \
void glRasterPos3##suffix##v(const type *v);               \
void glRasterPos3##suffix(type x, type y, type z);         \
void glRasterPos4##suffix##v(const type *v);               \
void glRasterPos4##suffix(type x, type y, type z, type w); \
void glVertex2##suffix##v(const type *v);                  \
void glVertex2##suffix(type x, type y);                    \
void glVertex3##suffix##v(const type *v);                  \
void glVertex3##suffix(type x, type y, type z);            \
void glVertex4##suffix(type x, type y, type z, type w);    \
void glVertex4##suffix##v(const type *v);                  \
void glTexCoord1##suffix(type s);                          \
void glTexCoord1##suffix##v(const type *t);                \
void glTexCoord2##suffix(type s, type t);                  \
void glTexCoord2##suffix##v(const type *t);                \
void glTexCoord3##suffix(type s, type t, type r);          \
void glTexCoord3##suffix##v(const type *t);                \
void glTexCoord4##suffix(type s, type t, type r, type q);  \
void glTexCoord4##suffix##v(const type *t);                \
void glMultiTexCoord1##suffix(GLenum target, type s);                         \
void glMultiTexCoord1##suffix##v(GLenum target, const type *v);               \
void glMultiTexCoord2##suffix(GLenum target, type s, type t);                 \
void glMultiTexCoord2##suffix##v(GLenum target, const type *v);               \
void glMultiTexCoord3##suffix(GLenum target, type s, type t, type r);         \
void glMultiTexCoord3##suffix##v(GLenum target, const type *v);               \
void glMultiTexCoord4##suffix(GLenum target, type s, type t, type r, type q); \
void glMultiTexCoord4##suffix##v(GLenum target, const type *v);

THUNK(b, GLbyte)
THUNK(d, GLdouble)
THUNK(i, GLint)
THUNK(s, GLshort)
THUNK(ub, GLubyte)
THUNK(ui, GLuint)
THUNK(us, GLushort)
#undef THUNK

#endif
