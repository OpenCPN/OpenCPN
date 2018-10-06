#include "../gl.h"
#include <GLES/glext.h>

#ifndef GL_WRAP_H
#define GL_WRAP_H

// misc naive wrappers
#ifdef USE_ES2
void glCompileShaderARB(GLuint shader);
GLuint glCreateShaderObjectARB(GLenum shaderType);
void glGetObjectParameterivARB(GLuint shader, GLenum pname, GLint *params);
void glShaderSourceARB(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
#endif

void glActiveTextureARB(GLenum texture);
void glClearDepth(GLdouble depth);
void glClientActiveTextureARB(GLenum texture);
void glClipPlane(GLenum plane, const GLdouble *equation);
void glDepthRange(GLdouble nearVal, GLdouble farVal);
void glFogi(GLenum pname, GLint param);
void glFogiv(GLenum pname, GLint *params);
void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
void glGetDoublev(GLenum pname, GLdouble *params);
void glLighti(GLenum light, GLenum pname, GLint param);
void glLightiv(GLenum light, GLenum pname, GLint *iparams);
void glLightModeli(GLenum pname, GLint param);
void glLightModeliv(GLenum pname, GLint *iparams);
void glMateriali(GLenum face, GLenum pname, GLint param);
void glMaterialiv(GLenum face, GLenum pname, GLint param);
void glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t);
void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);

// color
void glColor3f(GLfloat r, GLfloat g, GLfloat b);
void glColor3fv(GLfloat *c);
void glColor4fv(GLfloat *c);
void glIndexfv(const GLfloat *c);
void glSecondaryColor3fv(const GLfloat *v);

// raster
void glRasterPos2f(GLfloat x, GLfloat y);
void glRasterPos2fv(const GLfloat *v);
void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z);
void glRasterPos3fv(const GLfloat *v);
void glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void glRasterPos4fv(const GLfloat *v);

// eval
void glEvalCoord1d(GLdouble u);
void glEvalCoord1dv(GLdouble *v);
void glEvalCoord1fv(GLfloat *v);
void glEvalCoord2d(GLdouble u, GLdouble v);
void glEvalCoord2dv(GLdouble *v);
void glEvalCoord2fv(GLfloat *v);
void glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);
void glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);

// matrix
void glLoadMatrixd(const GLdouble *m);
void glMultMatrixd(const GLdouble *m);

// normal
void glNormal3fv(GLfloat *v);

// rect
#define GL_RECT(suffix, type)                                \
    void glRect##suffix(type x1, type y1, type x2, type y2); \
    void glRect##suffix##v(const type *v);

GL_RECT(d, GLdouble)
GL_RECT(f, GLfloat)
GL_RECT(i, GLint)
GL_RECT(s, GLshort)
#undef GL_RECT

// textures
void glTexCoord1f(GLfloat s);
void glTexCoord1fv(GLfloat *t);
void glTexCoord2fv(GLfloat *t);
void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r);
void glTexCoord3fv(GLfloat *t);
void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void glTexCoord4fv(GLfloat *t);

// texgen
void glTexGend(GLenum coord, GLenum pname, GLdouble param);
void glTexGeni(GLenum coord, GLenum pname, GLint param);
void glTexGenf(GLenum coord, GLenum pname, GLfloat param);
void glTexGendv(GLenum coord, GLenum pname, GLdouble *params);
void glTexGenfv(GLenum coord, GLenum pname, GLfloat *params);
void glTexGeniv(GLenum coord, GLenum pname, GLint *params);

// transforms
void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void glScaled(GLdouble x, GLdouble y, GLdouble z);
void glTranslated(GLdouble x, GLdouble y, GLdouble z);

// vertex
void glVertex2f(GLfloat x, GLfloat y);
void glVertex2fv(GLfloat *v);
void glVertex3fv(GLfloat *v);
void glVertex4f(GLfloat r, GLfloat g, GLfloat b, GLfloat w);
void glVertex4fv(GLfloat *v);

// OES wrappers

void glClearDepthfOES(GLfloat depth);
void glClipPlanefOES(GLenum plane, const GLfloat *equation);
void glDepthRangefOES(GLclampf near, GLclampf far);
void glFrustumfOES(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);
void glGetClipPlanefOES(GLenum plane, GLfloat equation[4]);
void glOrthofOES(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);

// basic thunking

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
void glRasterPos2##suffix##v(type *v);                     \
void glRasterPos2##suffix(type x, type y);                 \
void glRasterPos3##suffix##v(type *v);                     \
void glRasterPos3##suffix(type x, type y, type z);         \
void glRasterPos4##suffix##v(type *v);                     \
void glRasterPos4##suffix(type x, type y, type z, type w); \
void glVertex2##suffix##v(type *v);                        \
void glVertex2##suffix(type x, type y);                    \
void glVertex3##suffix##v(type *v);                        \
void glVertex3##suffix(type x, type y, type z);            \
void glVertex4##suffix(type x, type y, type z, type w);    \
void glVertex4##suffix##v(type *v);                        \
void glTexCoord1##suffix(type s);                          \
void glTexCoord1##suffix##v(type *t);                      \
void glTexCoord2##suffix(type s, type t);                  \
void glTexCoord2##suffix##v(type *t);                      \
void glTexCoord3##suffix(type s, type t, type r);          \
void glTexCoord3##suffix##v(type *t);                      \
void glTexCoord4##suffix(type s, type t, type r, type q);  \
void glTexCoord4##suffix##v(type *t);

THUNK(b, GLbyte)
THUNK(d, GLdouble)
THUNK(i, GLint)
THUNK(s, GLshort)
THUNK(ub, GLubyte)
THUNK(ui, GLuint)
THUNK(us, GLushort)
#undef THUNK

#define THUNK(suffix, type) \
    extern void glGet##suffix##v(GLenum pname, type *params);

THUNK(Double, GLdouble)
THUNK(Integer, GLint)
#undef THUNK

#endif
