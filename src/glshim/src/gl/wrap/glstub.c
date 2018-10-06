#include "stub.h"

#define STUB(def)\
def {\
    char *debug = getenv("LIBGL_DEBUG");\
    if (debug && strcmp(debug, "1") == 0)\
        printf("stub: %s;\n", #def);\
}

STUB(void glFogCoordd(GLdouble coord))
STUB(void glFogCoordf(GLfloat coord))
STUB(void glFogCoorddv(const GLdouble *coord))
STUB(void glFogCoordfv(const GLfloat *coord))
#ifdef BCMHOST
STUB(void glDiscardFramebufferEXT(GLenum target, GLsizei numAttachments, const GLenum *attachments))
#endif

#ifdef USE_ES2
STUB(void glClipPlanef(GLenum plane, const GLfloat *equation));
STUB(void glDisableClientState(GLenum state));
STUB(void glEnableClientState(GLenum state));
STUB(void glFogf(GLenum pname, GLfloat param));
STUB(void glFogfv(GLenum pname, const GLfloat *params));
STUB(void glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far));
STUB(void glGetClipPlanef(GLenum plane, GLfloat *equation));
STUB(void glLightf(GLenum light, GLenum pname, GLfloat param));
STUB(void glLightfv(GLenum light, GLenum pname, const GLfloat *params));
STUB(void glLoadIdentity());
STUB(void glLoadMatrixf(const GLfloat *m));
STUB(void glMaterialf(GLenum face, GLenum pname, GLfloat param));
STUB(void glMatrixMode(GLenum mode));
STUB(void glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat r, GLfloat q, GLfloat t));
STUB(void glMultMatrixf(const GLfloat *m));
STUB(void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearVal, GLfloat farVal));
STUB(void glPopMatrix());
STUB(void glPushMatrix());
STUB(void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z));
STUB(void glScalef(GLfloat x, GLfloat y, GLfloat z));
STUB(void glTexEnvf(GLenum target, GLenum pname, GLfloat param));
STUB(void glTexEnvi(GLenum target, GLenum pname, GLint param));
STUB(void glTranslatef(GLfloat x, GLfloat y, GLfloat z));
#endif

// STUB(void glMultiTexCoord());
// STUB(void glVertexAttrib());
STUB(GLint glRenderMode(GLenum mode))
#ifndef USE_ES2
STUB(void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha))
#endif
STUB(void glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha))
#ifndef USE_ES2
STUB(void glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha))
#endif
STUB(void glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha))
STUB(void glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))
STUB(void glColorMaterial(GLenum face, GLenum mode))
STUB(void glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type))
STUB(void glDrawBuffer(GLenum mode))
STUB(void glEdgeFlag(GLboolean flag))
STUB(void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * img))
STUB(void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params))
STUB(void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params))
STUB(void glIndexf(GLfloat c))
STUB(void glInitNames())
STUB(void glLoadName(GLuint name))
STUB(void glPixelTransferf(GLenum pname, GLfloat param))
STUB(void glPixelTransferi(GLenum pname, GLint param))
STUB(void glPixelZoom(GLfloat xfactor, GLfloat yfactor))
STUB(void glPolygonMode(GLenum face, GLenum mode))
STUB(void glPolygonStipple(const GLubyte *mask))
STUB(void glPopName())
STUB(void glPushName())
STUB(void glReadBuffer(GLenum mode))
STUB(void glSecondaryColor3f(GLfloat r, GLfloat g, GLfloat b))

// glSelectBuffer: http://www.lighthouse3d.com/opengl/picking/index.php?color1
STUB(void glSelectBuffer(GLsizei size, GLuint *buffer))

#undef STUB
