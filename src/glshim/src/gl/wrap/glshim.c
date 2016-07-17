#ifndef USE_ES2
#include "glpack.h"
#include "../loader.h"
#include "../skip.h"
#include "../remote.h"

#ifndef skip_glActiveTexture
void glActiveTexture(GLenum texture) {
#ifndef direct_glActiveTexture
    PUSH_IF_COMPILING(glActiveTexture);
#endif
    FORWARD_IF_REMOTE(glActiveTexture);
    LOAD_GLES(glActiveTexture);
    gles_glActiveTexture(texture);
}
#endif
#ifndef skip_glAlphaFunc
void glAlphaFunc(GLenum func, GLclampf ref) {
#ifndef direct_glAlphaFunc
    PUSH_IF_COMPILING(glAlphaFunc);
#endif
    FORWARD_IF_REMOTE(glAlphaFunc);
    LOAD_GLES(glAlphaFunc);
    gles_glAlphaFunc(func, ref);
}
#endif
#ifndef skip_glAlphaFuncx
void glAlphaFuncx(GLenum func, GLclampx ref) {
#ifndef direct_glAlphaFuncx
    PUSH_IF_COMPILING(glAlphaFuncx);
#endif
    FORWARD_IF_REMOTE(glAlphaFuncx);
    LOAD_GLES(glAlphaFuncx);
    gles_glAlphaFuncx(func, ref);
}
#endif
#ifndef skip_glBindBuffer
void glBindBuffer(GLenum target, GLuint buffer) {
#ifndef direct_glBindBuffer
    PUSH_IF_COMPILING(glBindBuffer);
#endif
    FORWARD_IF_REMOTE(glBindBuffer);
    LOAD_GLES(glBindBuffer);
    gles_glBindBuffer(target, buffer);
}
#endif
#ifndef skip_glBindTexture
void glBindTexture(GLenum target, GLuint texture) {
#ifndef direct_glBindTexture
    PUSH_IF_COMPILING(glBindTexture);
#endif
    FORWARD_IF_REMOTE(glBindTexture);
    LOAD_GLES(glBindTexture);
    gles_glBindTexture(target, texture);
}
#endif
#ifndef skip_glBlendColorOES
void glBlendColorOES(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
#ifndef direct_glBlendColorOES
    PUSH_IF_COMPILING(glBlendColorOES);
#endif
    FORWARD_IF_REMOTE(glBlendColorOES);
    LOAD_GLES(glBlendColorOES);
    gles_glBlendColorOES(red, green, blue, alpha);
}
#endif
#ifndef skip_glBlendEquationOES
void glBlendEquationOES(GLenum mode) {
#ifndef direct_glBlendEquationOES
    PUSH_IF_COMPILING(glBlendEquationOES);
#endif
    FORWARD_IF_REMOTE(glBlendEquationOES);
    LOAD_GLES(glBlendEquationOES);
    gles_glBlendEquationOES(mode);
}
#endif
#ifndef skip_glBlendEquationSeparateOES
void glBlendEquationSeparateOES(GLenum modeRGB, GLenum modeAlpha) {
#ifndef direct_glBlendEquationSeparateOES
    PUSH_IF_COMPILING(glBlendEquationSeparateOES);
#endif
    FORWARD_IF_REMOTE(glBlendEquationSeparateOES);
    LOAD_GLES(glBlendEquationSeparateOES);
    gles_glBlendEquationSeparateOES(modeRGB, modeAlpha);
}
#endif
#ifndef skip_glBlendFunc
void glBlendFunc(GLenum sfactor, GLenum dfactor) {
#ifndef direct_glBlendFunc
    PUSH_IF_COMPILING(glBlendFunc);
#endif
    FORWARD_IF_REMOTE(glBlendFunc);
    LOAD_GLES(glBlendFunc);
    gles_glBlendFunc(sfactor, dfactor);
}
#endif
#ifndef skip_glBlendFuncSeparateOES
void glBlendFuncSeparateOES(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
#ifndef direct_glBlendFuncSeparateOES
    PUSH_IF_COMPILING(glBlendFuncSeparateOES);
#endif
    FORWARD_IF_REMOTE(glBlendFuncSeparateOES);
    LOAD_GLES(glBlendFuncSeparateOES);
    gles_glBlendFuncSeparateOES(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}
#endif
#ifndef skip_glBufferData
void glBufferData(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage) {
#ifndef direct_glBufferData
    PUSH_IF_COMPILING(glBufferData);
#endif
    FORWARD_IF_REMOTE(glBufferData);
    LOAD_GLES(glBufferData);
    gles_glBufferData(target, size, data, usage);
}
#endif
#ifndef skip_glBufferSubData
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data) {
#ifndef direct_glBufferSubData
    PUSH_IF_COMPILING(glBufferSubData);
#endif
    FORWARD_IF_REMOTE(glBufferSubData);
    LOAD_GLES(glBufferSubData);
    gles_glBufferSubData(target, offset, size, data);
}
#endif
#ifndef skip_glClear
void glClear(GLbitfield mask) {
#ifndef direct_glClear
    PUSH_IF_COMPILING(glClear);
#endif
    FORWARD_IF_REMOTE(glClear);
    LOAD_GLES(glClear);
    gles_glClear(mask);
}
#endif
#ifndef skip_glClearColor
void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
#ifndef direct_glClearColor
    PUSH_IF_COMPILING(glClearColor);
#endif
    FORWARD_IF_REMOTE(glClearColor);
    LOAD_GLES(glClearColor);
    gles_glClearColor(red, green, blue, alpha);
}
#endif
#ifndef skip_glClearColorx
void glClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha) {
#ifndef direct_glClearColorx
    PUSH_IF_COMPILING(glClearColorx);
#endif
    FORWARD_IF_REMOTE(glClearColorx);
    LOAD_GLES(glClearColorx);
    gles_glClearColorx(red, green, blue, alpha);
}
#endif
#ifndef skip_glClearDepthf
void glClearDepthf(GLclampf depth) {
#ifndef direct_glClearDepthf
    PUSH_IF_COMPILING(glClearDepthf);
#endif
    FORWARD_IF_REMOTE(glClearDepthf);
    LOAD_GLES(glClearDepthf);
    gles_glClearDepthf(depth);
}
#endif
#ifndef skip_glClearDepthx
void glClearDepthx(GLclampx depth) {
#ifndef direct_glClearDepthx
    PUSH_IF_COMPILING(glClearDepthx);
#endif
    FORWARD_IF_REMOTE(glClearDepthx);
    LOAD_GLES(glClearDepthx);
    gles_glClearDepthx(depth);
}
#endif
#ifndef skip_glClearStencil
void glClearStencil(GLint s) {
#ifndef direct_glClearStencil
    PUSH_IF_COMPILING(glClearStencil);
#endif
    FORWARD_IF_REMOTE(glClearStencil);
    LOAD_GLES(glClearStencil);
    gles_glClearStencil(s);
}
#endif
#ifndef skip_glClientActiveTexture
void glClientActiveTexture(GLenum texture) {
#ifndef direct_glClientActiveTexture
    PUSH_IF_COMPILING(glClientActiveTexture);
#endif
    FORWARD_IF_REMOTE(glClientActiveTexture);
    LOAD_GLES(glClientActiveTexture);
    gles_glClientActiveTexture(texture);
}
#endif
#ifndef skip_glClipPlanef
void glClipPlanef(GLenum plane, const GLfloat * equation) {
#ifndef direct_glClipPlanef
    PUSH_IF_COMPILING(glClipPlanef);
#endif
    FORWARD_IF_REMOTE(glClipPlanef);
    LOAD_GLES(glClipPlanef);
    gles_glClipPlanef(plane, equation);
}
#endif
#ifndef skip_glClipPlanex
void glClipPlanex(GLenum plane, const GLfixed * equation) {
#ifndef direct_glClipPlanex
    PUSH_IF_COMPILING(glClipPlanex);
#endif
    FORWARD_IF_REMOTE(glClipPlanex);
    LOAD_GLES(glClipPlanex);
    gles_glClipPlanex(plane, equation);
}
#endif
#ifndef skip_glColor4f
void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
#ifndef direct_glColor4f
    PUSH_IF_COMPILING(glColor4f);
#endif
    FORWARD_IF_REMOTE(glColor4f);
    LOAD_GLES(glColor4f);
    gles_glColor4f(red, green, blue, alpha);
}
#endif
#ifndef skip_glColor4ub
void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) {
#ifndef direct_glColor4ub
    PUSH_IF_COMPILING(glColor4ub);
#endif
    FORWARD_IF_REMOTE(glColor4ub);
    LOAD_GLES(glColor4ub);
    gles_glColor4ub(red, green, blue, alpha);
}
#endif
#ifndef skip_glColor4x
void glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) {
#ifndef direct_glColor4x
    PUSH_IF_COMPILING(glColor4x);
#endif
    FORWARD_IF_REMOTE(glColor4x);
    LOAD_GLES(glColor4x);
    gles_glColor4x(red, green, blue, alpha);
}
#endif
#ifndef skip_glColorMask
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
#ifndef direct_glColorMask
    PUSH_IF_COMPILING(glColorMask);
#endif
    FORWARD_IF_REMOTE(glColorMask);
    LOAD_GLES(glColorMask);
    gles_glColorMask(red, green, blue, alpha);
}
#endif
#ifndef skip_glColorPointer
void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer) {
#ifndef direct_glColorPointer
    PUSH_IF_COMPILING(glColorPointer);
#endif
    FORWARD_IF_REMOTE(glColorPointer);
    LOAD_GLES(glColorPointer);
    gles_glColorPointer(size, type, stride, pointer);
}
#endif
#ifndef skip_glCompressedTexImage2D
void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data) {
#ifndef direct_glCompressedTexImage2D
    PUSH_IF_COMPILING(glCompressedTexImage2D);
#endif
    FORWARD_IF_REMOTE(glCompressedTexImage2D);
    LOAD_GLES(glCompressedTexImage2D);
    gles_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}
#endif
#ifndef skip_glCompressedTexSubImage2D
void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data) {
#ifndef direct_glCompressedTexSubImage2D
    PUSH_IF_COMPILING(glCompressedTexSubImage2D);
#endif
    FORWARD_IF_REMOTE(glCompressedTexSubImage2D);
    LOAD_GLES(glCompressedTexSubImage2D);
    gles_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}
#endif
#ifndef skip_glCopyTexImage2D
void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
#ifndef direct_glCopyTexImage2D
    PUSH_IF_COMPILING(glCopyTexImage2D);
#endif
    FORWARD_IF_REMOTE(glCopyTexImage2D);
    LOAD_GLES(glCopyTexImage2D);
    gles_glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}
#endif
#ifndef skip_glCopyTexSubImage2D
void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
#ifndef direct_glCopyTexSubImage2D
    PUSH_IF_COMPILING(glCopyTexSubImage2D);
#endif
    FORWARD_IF_REMOTE(glCopyTexSubImage2D);
    LOAD_GLES(glCopyTexSubImage2D);
    gles_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}
#endif
#ifndef skip_glCullFace
void glCullFace(GLenum mode) {
#ifndef direct_glCullFace
    PUSH_IF_COMPILING(glCullFace);
#endif
    FORWARD_IF_REMOTE(glCullFace);
    LOAD_GLES(glCullFace);
    gles_glCullFace(mode);
}
#endif
#ifndef skip_glDeleteBuffers
void glDeleteBuffers(GLsizei n, const GLuint * buffers) {
#ifndef direct_glDeleteBuffers
    PUSH_IF_COMPILING(glDeleteBuffers);
#endif
    FORWARD_IF_REMOTE(glDeleteBuffers);
    LOAD_GLES(glDeleteBuffers);
    gles_glDeleteBuffers(n, buffers);
}
#endif
#ifndef skip_glDeleteTextures
void glDeleteTextures(GLsizei n, const GLuint * textures) {
#ifndef direct_glDeleteTextures
    PUSH_IF_COMPILING(glDeleteTextures);
#endif
    FORWARD_IF_REMOTE(glDeleteTextures);
    LOAD_GLES(glDeleteTextures);
    gles_glDeleteTextures(n, textures);
}
#endif
#ifndef skip_glDepthFunc
void glDepthFunc(GLenum func) {
#ifndef direct_glDepthFunc
    PUSH_IF_COMPILING(glDepthFunc);
#endif
    FORWARD_IF_REMOTE(glDepthFunc);
    LOAD_GLES(glDepthFunc);
    gles_glDepthFunc(func);
}
#endif
#ifndef skip_glDepthMask
void glDepthMask(GLboolean flag) {
#ifndef direct_glDepthMask
    PUSH_IF_COMPILING(glDepthMask);
#endif
    FORWARD_IF_REMOTE(glDepthMask);
    LOAD_GLES(glDepthMask);
    gles_glDepthMask(flag);
}
#endif
#ifndef skip_glDepthRangef
void glDepthRangef(GLclampf near, GLclampf far) {
#ifndef direct_glDepthRangef
    PUSH_IF_COMPILING(glDepthRangef);
#endif
    FORWARD_IF_REMOTE(glDepthRangef);
    LOAD_GLES(glDepthRangef);
    gles_glDepthRangef(near, far);
}
#endif
#ifndef skip_glDepthRangex
void glDepthRangex(GLclampx near, GLclampx far) {
#ifndef direct_glDepthRangex
    PUSH_IF_COMPILING(glDepthRangex);
#endif
    FORWARD_IF_REMOTE(glDepthRangex);
    LOAD_GLES(glDepthRangex);
    gles_glDepthRangex(near, far);
}
#endif
#ifndef skip_glDisable
void glDisable(GLenum cap) {
#ifndef direct_glDisable
    PUSH_IF_COMPILING(glDisable);
#endif
    FORWARD_IF_REMOTE(glDisable);
    LOAD_GLES(glDisable);
    gles_glDisable(cap);
}
#endif
#ifndef skip_glDisableClientState
void glDisableClientState(GLenum array) {
#ifndef direct_glDisableClientState
    PUSH_IF_COMPILING(glDisableClientState);
#endif
    FORWARD_IF_REMOTE(glDisableClientState);
    LOAD_GLES(glDisableClientState);
    gles_glDisableClientState(array);
}
#endif
#ifndef skip_glDrawArrays
void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
#ifndef direct_glDrawArrays
    PUSH_IF_COMPILING(glDrawArrays);
#endif
    FORWARD_IF_REMOTE(glDrawArrays);
    LOAD_GLES(glDrawArrays);
    gles_glDrawArrays(mode, first, count);
}
#endif
#ifndef skip_glDrawElements
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices) {
#ifndef direct_glDrawElements
    PUSH_IF_COMPILING(glDrawElements);
#endif
    FORWARD_IF_REMOTE(glDrawElements);
    LOAD_GLES(glDrawElements);
    gles_glDrawElements(mode, count, type, indices);
}
#endif
#ifndef skip_glEnable
void glEnable(GLenum cap) {
#ifndef direct_glEnable
    PUSH_IF_COMPILING(glEnable);
#endif
    FORWARD_IF_REMOTE(glEnable);
    LOAD_GLES(glEnable);
    gles_glEnable(cap);
}
#endif
#ifndef skip_glEnableClientState
void glEnableClientState(GLenum array) {
#ifndef direct_glEnableClientState
    PUSH_IF_COMPILING(glEnableClientState);
#endif
    FORWARD_IF_REMOTE(glEnableClientState);
    LOAD_GLES(glEnableClientState);
    gles_glEnableClientState(array);
}
#endif
#ifndef skip_glFinish
void glFinish() {
#ifndef direct_glFinish
    PUSH_IF_COMPILING(glFinish);
#endif
    FORWARD_IF_REMOTE(glFinish);
    LOAD_GLES(glFinish);
    gles_glFinish();
}
#endif
#ifndef skip_glFlush
void glFlush() {
#ifndef direct_glFlush
    PUSH_IF_COMPILING(glFlush);
#endif
    FORWARD_IF_REMOTE(glFlush);
    LOAD_GLES(glFlush);
    gles_glFlush();
}
#endif
#ifndef skip_glFogf
void glFogf(GLenum pname, GLfloat param) {
#ifndef direct_glFogf
    PUSH_IF_COMPILING(glFogf);
#endif
    FORWARD_IF_REMOTE(glFogf);
    LOAD_GLES(glFogf);
    gles_glFogf(pname, param);
}
#endif
#ifndef skip_glFogfv
void glFogfv(GLenum pname, const GLfloat * params) {
#ifndef direct_glFogfv
    PUSH_IF_COMPILING(glFogfv);
#endif
    FORWARD_IF_REMOTE(glFogfv);
    LOAD_GLES(glFogfv);
    gles_glFogfv(pname, params);
}
#endif
#ifndef skip_glFogx
void glFogx(GLenum pname, GLfixed param) {
#ifndef direct_glFogx
    PUSH_IF_COMPILING(glFogx);
#endif
    FORWARD_IF_REMOTE(glFogx);
    LOAD_GLES(glFogx);
    gles_glFogx(pname, param);
}
#endif
#ifndef skip_glFogxv
void glFogxv(GLenum pname, const GLfixed * params) {
#ifndef direct_glFogxv
    PUSH_IF_COMPILING(glFogxv);
#endif
    FORWARD_IF_REMOTE(glFogxv);
    LOAD_GLES(glFogxv);
    gles_glFogxv(pname, params);
}
#endif
#ifndef skip_glFrontFace
void glFrontFace(GLenum mode) {
#ifndef direct_glFrontFace
    PUSH_IF_COMPILING(glFrontFace);
#endif
    FORWARD_IF_REMOTE(glFrontFace);
    LOAD_GLES(glFrontFace);
    gles_glFrontFace(mode);
}
#endif
#ifndef skip_glFrustumf
void glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
#ifndef direct_glFrustumf
    PUSH_IF_COMPILING(glFrustumf);
#endif
    FORWARD_IF_REMOTE(glFrustumf);
    LOAD_GLES(glFrustumf);
    gles_glFrustumf(left, right, bottom, top, near, far);
}
#endif
#ifndef skip_glFrustumx
void glFrustumx(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed near, GLfixed far) {
#ifndef direct_glFrustumx
    PUSH_IF_COMPILING(glFrustumx);
#endif
    FORWARD_IF_REMOTE(glFrustumx);
    LOAD_GLES(glFrustumx);
    gles_glFrustumx(left, right, bottom, top, near, far);
}
#endif
#ifndef skip_glGenBuffers
void glGenBuffers(GLsizei n, GLuint * buffers) {
#ifndef direct_glGenBuffers
    PUSH_IF_COMPILING(glGenBuffers);
#endif
    FORWARD_IF_REMOTE(glGenBuffers);
    LOAD_GLES(glGenBuffers);
    gles_glGenBuffers(n, buffers);
}
#endif
#ifndef skip_glGenTextures
void glGenTextures(GLsizei n, GLuint * textures) {
#ifndef direct_glGenTextures
    PUSH_IF_COMPILING(glGenTextures);
#endif
    FORWARD_IF_REMOTE(glGenTextures);
    LOAD_GLES(glGenTextures);
    gles_glGenTextures(n, textures);
}
#endif
#ifndef skip_glGetBooleanv
void glGetBooleanv(GLenum pname, GLboolean * params) {
#ifndef direct_glGetBooleanv
    PUSH_IF_COMPILING(glGetBooleanv);
#endif
    FORWARD_IF_REMOTE(glGetBooleanv);
    LOAD_GLES(glGetBooleanv);
    gles_glGetBooleanv(pname, params);
}
#endif
#ifndef skip_glGetBufferParameteriv
void glGetBufferParameteriv(GLenum target, GLenum pname, GLint * params) {
#ifndef direct_glGetBufferParameteriv
    PUSH_IF_COMPILING(glGetBufferParameteriv);
#endif
    FORWARD_IF_REMOTE(glGetBufferParameteriv);
    LOAD_GLES(glGetBufferParameteriv);
    gles_glGetBufferParameteriv(target, pname, params);
}
#endif
#ifndef skip_glGetClipPlanef
void glGetClipPlanef(GLenum plane, GLfloat * equation) {
#ifndef direct_glGetClipPlanef
    PUSH_IF_COMPILING(glGetClipPlanef);
#endif
    FORWARD_IF_REMOTE(glGetClipPlanef);
    LOAD_GLES(glGetClipPlanef);
    gles_glGetClipPlanef(plane, equation);
}
#endif
#ifndef skip_glGetClipPlanex
void glGetClipPlanex(GLenum plane, GLfixed * equation) {
#ifndef direct_glGetClipPlanex
    PUSH_IF_COMPILING(glGetClipPlanex);
#endif
    FORWARD_IF_REMOTE(glGetClipPlanex);
    LOAD_GLES(glGetClipPlanex);
    gles_glGetClipPlanex(plane, equation);
}
#endif
#ifndef skip_glGetError
GLenum glGetError() {
#ifndef direct_glGetError
    PUSH_IF_COMPILING(glGetError);
#endif
    FORWARD_IF_REMOTE(glGetError);
    LOAD_GLES(glGetError);
    return gles_glGetError();
}
#endif
#ifndef skip_glGetFixedv
void glGetFixedv(GLenum pname, GLfixed * params) {
#ifndef direct_glGetFixedv
    PUSH_IF_COMPILING(glGetFixedv);
#endif
    FORWARD_IF_REMOTE(glGetFixedv);
    LOAD_GLES(glGetFixedv);
    gles_glGetFixedv(pname, params);
}
#endif
#ifndef skip_glGetFloatv
void glGetFloatv(GLenum pname, GLfloat * params) {
#ifndef direct_glGetFloatv
    PUSH_IF_COMPILING(glGetFloatv);
#endif
    FORWARD_IF_REMOTE(glGetFloatv);
    LOAD_GLES(glGetFloatv);
    gles_glGetFloatv(pname, params);
}
#endif
#ifndef skip_glGetIntegerv
void glGetIntegerv(GLenum pname, GLint * params) {
#ifndef direct_glGetIntegerv
    PUSH_IF_COMPILING(glGetIntegerv);
#endif
    FORWARD_IF_REMOTE(glGetIntegerv);
    LOAD_GLES(glGetIntegerv);
    gles_glGetIntegerv(pname, params);
}
#endif
#ifndef skip_glGetLightfv
void glGetLightfv(GLenum light, GLenum pname, GLfloat * params) {
#ifndef direct_glGetLightfv
    PUSH_IF_COMPILING(glGetLightfv);
#endif
    FORWARD_IF_REMOTE(glGetLightfv);
    LOAD_GLES(glGetLightfv);
    gles_glGetLightfv(light, pname, params);
}
#endif
#ifndef skip_glGetLightxv
void glGetLightxv(GLenum light, GLenum pname, GLfixed * params) {
#ifndef direct_glGetLightxv
    PUSH_IF_COMPILING(glGetLightxv);
#endif
    FORWARD_IF_REMOTE(glGetLightxv);
    LOAD_GLES(glGetLightxv);
    gles_glGetLightxv(light, pname, params);
}
#endif
#ifndef skip_glGetMaterialfv
void glGetMaterialfv(GLenum face, GLenum pname, GLfloat * params) {
#ifndef direct_glGetMaterialfv
    PUSH_IF_COMPILING(glGetMaterialfv);
#endif
    FORWARD_IF_REMOTE(glGetMaterialfv);
    LOAD_GLES(glGetMaterialfv);
    gles_glGetMaterialfv(face, pname, params);
}
#endif
#ifndef skip_glGetMaterialxv
void glGetMaterialxv(GLenum face, GLenum pname, GLfixed * params) {
#ifndef direct_glGetMaterialxv
    PUSH_IF_COMPILING(glGetMaterialxv);
#endif
    FORWARD_IF_REMOTE(glGetMaterialxv);
    LOAD_GLES(glGetMaterialxv);
    gles_glGetMaterialxv(face, pname, params);
}
#endif
#ifndef skip_glGetPointerv
void glGetPointerv(GLenum pname, GLvoid ** params) {
#ifndef direct_glGetPointerv
    PUSH_IF_COMPILING(glGetPointerv);
#endif
    FORWARD_IF_REMOTE(glGetPointerv);
    LOAD_GLES(glGetPointerv);
    gles_glGetPointerv(pname, params);
}
#endif
#ifndef skip_glGetString
const GLubyte * glGetString(GLenum name) {
#ifndef direct_glGetString
    PUSH_IF_COMPILING(glGetString);
#endif
    FORWARD_IF_REMOTE(glGetString);
    LOAD_GLES(glGetString);
    return gles_glGetString(name);
}
#endif
#ifndef skip_glGetTexEnvfv
void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat * params) {
#ifndef direct_glGetTexEnvfv
    PUSH_IF_COMPILING(glGetTexEnvfv);
#endif
    FORWARD_IF_REMOTE(glGetTexEnvfv);
    LOAD_GLES(glGetTexEnvfv);
    gles_glGetTexEnvfv(target, pname, params);
}
#endif
#ifndef skip_glGetTexEnviv
void glGetTexEnviv(GLenum target, GLenum pname, GLint * params) {
#ifndef direct_glGetTexEnviv
    PUSH_IF_COMPILING(glGetTexEnviv);
#endif
    FORWARD_IF_REMOTE(glGetTexEnviv);
    LOAD_GLES(glGetTexEnviv);
    gles_glGetTexEnviv(target, pname, params);
}
#endif
#ifndef skip_glGetTexEnvxv
void glGetTexEnvxv(GLenum target, GLenum pname, GLfixed * params) {
#ifndef direct_glGetTexEnvxv
    PUSH_IF_COMPILING(glGetTexEnvxv);
#endif
    FORWARD_IF_REMOTE(glGetTexEnvxv);
    LOAD_GLES(glGetTexEnvxv);
    gles_glGetTexEnvxv(target, pname, params);
}
#endif
#ifndef skip_glGetTexParameterfv
void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat * params) {
#ifndef direct_glGetTexParameterfv
    PUSH_IF_COMPILING(glGetTexParameterfv);
#endif
    FORWARD_IF_REMOTE(glGetTexParameterfv);
    LOAD_GLES(glGetTexParameterfv);
    gles_glGetTexParameterfv(target, pname, params);
}
#endif
#ifndef skip_glGetTexParameteriv
void glGetTexParameteriv(GLenum target, GLenum pname, GLint * params) {
#ifndef direct_glGetTexParameteriv
    PUSH_IF_COMPILING(glGetTexParameteriv);
#endif
    FORWARD_IF_REMOTE(glGetTexParameteriv);
    LOAD_GLES(glGetTexParameteriv);
    gles_glGetTexParameteriv(target, pname, params);
}
#endif
#ifndef skip_glGetTexParameterxv
void glGetTexParameterxv(GLenum target, GLenum pname, GLfixed * params) {
#ifndef direct_glGetTexParameterxv
    PUSH_IF_COMPILING(glGetTexParameterxv);
#endif
    FORWARD_IF_REMOTE(glGetTexParameterxv);
    LOAD_GLES(glGetTexParameterxv);
    gles_glGetTexParameterxv(target, pname, params);
}
#endif
#ifndef skip_glHint
void glHint(GLenum target, GLenum mode) {
#ifndef direct_glHint
    PUSH_IF_COMPILING(glHint);
#endif
    FORWARD_IF_REMOTE(glHint);
    LOAD_GLES(glHint);
    gles_glHint(target, mode);
}
#endif
#ifndef skip_glIsBuffer
GLboolean glIsBuffer(GLuint buffer) {
#ifndef direct_glIsBuffer
    PUSH_IF_COMPILING(glIsBuffer);
#endif
    FORWARD_IF_REMOTE(glIsBuffer);
    LOAD_GLES(glIsBuffer);
    return gles_glIsBuffer(buffer);
}
#endif
#ifndef skip_glIsEnabled
GLboolean glIsEnabled(GLenum cap) {
#ifndef direct_glIsEnabled
    PUSH_IF_COMPILING(glIsEnabled);
#endif
    FORWARD_IF_REMOTE(glIsEnabled);
    LOAD_GLES(glIsEnabled);
    return gles_glIsEnabled(cap);
}
#endif
#ifndef skip_glIsTexture
GLboolean glIsTexture(GLuint texture) {
#ifndef direct_glIsTexture
    PUSH_IF_COMPILING(glIsTexture);
#endif
    FORWARD_IF_REMOTE(glIsTexture);
    LOAD_GLES(glIsTexture);
    return gles_glIsTexture(texture);
}
#endif
#ifndef skip_glLightModelf
void glLightModelf(GLenum pname, GLfloat param) {
#ifndef direct_glLightModelf
    PUSH_IF_COMPILING(glLightModelf);
#endif
    FORWARD_IF_REMOTE(glLightModelf);
    LOAD_GLES(glLightModelf);
    gles_glLightModelf(pname, param);
}
#endif
#ifndef skip_glLightModelfv
void glLightModelfv(GLenum pname, const GLfloat * params) {
#ifndef direct_glLightModelfv
    PUSH_IF_COMPILING(glLightModelfv);
#endif
    FORWARD_IF_REMOTE(glLightModelfv);
    LOAD_GLES(glLightModelfv);
    gles_glLightModelfv(pname, params);
}
#endif
#ifndef skip_glLightModelx
void glLightModelx(GLenum pname, GLfixed param) {
#ifndef direct_glLightModelx
    PUSH_IF_COMPILING(glLightModelx);
#endif
    FORWARD_IF_REMOTE(glLightModelx);
    LOAD_GLES(glLightModelx);
    gles_glLightModelx(pname, param);
}
#endif
#ifndef skip_glLightModelxv
void glLightModelxv(GLenum pname, const GLfixed * params) {
#ifndef direct_glLightModelxv
    PUSH_IF_COMPILING(glLightModelxv);
#endif
    FORWARD_IF_REMOTE(glLightModelxv);
    LOAD_GLES(glLightModelxv);
    gles_glLightModelxv(pname, params);
}
#endif
#ifndef skip_glLightf
void glLightf(GLenum light, GLenum pname, GLfloat param) {
#ifndef direct_glLightf
    PUSH_IF_COMPILING(glLightf);
#endif
    FORWARD_IF_REMOTE(glLightf);
    LOAD_GLES(glLightf);
    gles_glLightf(light, pname, param);
}
#endif
#ifndef skip_glLightfv
void glLightfv(GLenum light, GLenum pname, const GLfloat * params) {
#ifndef direct_glLightfv
    PUSH_IF_COMPILING(glLightfv);
#endif
    FORWARD_IF_REMOTE(glLightfv);
    LOAD_GLES(glLightfv);
    gles_glLightfv(light, pname, params);
}
#endif
#ifndef skip_glLightx
void glLightx(GLenum light, GLenum pname, GLfixed param) {
#ifndef direct_glLightx
    PUSH_IF_COMPILING(glLightx);
#endif
    FORWARD_IF_REMOTE(glLightx);
    LOAD_GLES(glLightx);
    gles_glLightx(light, pname, param);
}
#endif
#ifndef skip_glLightxv
void glLightxv(GLenum light, GLenum pname, const GLfixed * params) {
#ifndef direct_glLightxv
    PUSH_IF_COMPILING(glLightxv);
#endif
    FORWARD_IF_REMOTE(glLightxv);
    LOAD_GLES(glLightxv);
    gles_glLightxv(light, pname, params);
}
#endif
#ifndef skip_glLineWidth
void glLineWidth(GLfloat width) {
#ifndef direct_glLineWidth
    PUSH_IF_COMPILING(glLineWidth);
#endif
    FORWARD_IF_REMOTE(glLineWidth);
    LOAD_GLES(glLineWidth);
    gles_glLineWidth(width);
}
#endif
#ifndef skip_glLineWidthx
void glLineWidthx(GLfixed width) {
#ifndef direct_glLineWidthx
    PUSH_IF_COMPILING(glLineWidthx);
#endif
    FORWARD_IF_REMOTE(glLineWidthx);
    LOAD_GLES(glLineWidthx);
    gles_glLineWidthx(width);
}
#endif
#ifndef skip_glLoadIdentity
void glLoadIdentity() {
#ifndef direct_glLoadIdentity
    PUSH_IF_COMPILING(glLoadIdentity);
#endif
    FORWARD_IF_REMOTE(glLoadIdentity);
    LOAD_GLES(glLoadIdentity);
    gles_glLoadIdentity();
}
#endif
#ifndef skip_glLoadMatrixf
void glLoadMatrixf(const GLfloat * m) {
#ifndef direct_glLoadMatrixf
    PUSH_IF_COMPILING(glLoadMatrixf);
#endif
    FORWARD_IF_REMOTE(glLoadMatrixf);
    LOAD_GLES(glLoadMatrixf);
    gles_glLoadMatrixf(m);
}
#endif
#ifndef skip_glLoadMatrixx
void glLoadMatrixx(const GLfixed * m) {
#ifndef direct_glLoadMatrixx
    PUSH_IF_COMPILING(glLoadMatrixx);
#endif
    FORWARD_IF_REMOTE(glLoadMatrixx);
    LOAD_GLES(glLoadMatrixx);
    gles_glLoadMatrixx(m);
}
#endif
#ifndef skip_glLogicOp
void glLogicOp(GLenum opcode) {
#ifndef direct_glLogicOp
    PUSH_IF_COMPILING(glLogicOp);
#endif
    FORWARD_IF_REMOTE(glLogicOp);
    LOAD_GLES(glLogicOp);
    gles_glLogicOp(opcode);
}
#endif
#ifndef skip_glMaterialf
void glMaterialf(GLenum face, GLenum pname, GLfloat param) {
#ifndef direct_glMaterialf
    PUSH_IF_COMPILING(glMaterialf);
#endif
    FORWARD_IF_REMOTE(glMaterialf);
    LOAD_GLES(glMaterialf);
    gles_glMaterialf(face, pname, param);
}
#endif
#ifndef skip_glMaterialfv
void glMaterialfv(GLenum face, GLenum pname, const GLfloat * params) {
#ifndef direct_glMaterialfv
    PUSH_IF_COMPILING(glMaterialfv);
#endif
    FORWARD_IF_REMOTE(glMaterialfv);
    LOAD_GLES(glMaterialfv);
    gles_glMaterialfv(face, pname, params);
}
#endif
#ifndef skip_glMaterialx
void glMaterialx(GLenum face, GLenum pname, GLfixed param) {
#ifndef direct_glMaterialx
    PUSH_IF_COMPILING(glMaterialx);
#endif
    FORWARD_IF_REMOTE(glMaterialx);
    LOAD_GLES(glMaterialx);
    gles_glMaterialx(face, pname, param);
}
#endif
#ifndef skip_glMaterialxv
void glMaterialxv(GLenum face, GLenum pname, const GLfixed * params) {
#ifndef direct_glMaterialxv
    PUSH_IF_COMPILING(glMaterialxv);
#endif
    FORWARD_IF_REMOTE(glMaterialxv);
    LOAD_GLES(glMaterialxv);
    gles_glMaterialxv(face, pname, params);
}
#endif
#ifndef skip_glMatrixMode
void glMatrixMode(GLenum mode) {
#ifndef direct_glMatrixMode
    PUSH_IF_COMPILING(glMatrixMode);
#endif
    FORWARD_IF_REMOTE(glMatrixMode);
    LOAD_GLES(glMatrixMode);
    gles_glMatrixMode(mode);
}
#endif
#ifndef skip_glMultMatrixf
void glMultMatrixf(const GLfloat * m) {
#ifndef direct_glMultMatrixf
    PUSH_IF_COMPILING(glMultMatrixf);
#endif
    FORWARD_IF_REMOTE(glMultMatrixf);
    LOAD_GLES(glMultMatrixf);
    gles_glMultMatrixf(m);
}
#endif
#ifndef skip_glMultMatrixx
void glMultMatrixx(const GLfixed * m) {
#ifndef direct_glMultMatrixx
    PUSH_IF_COMPILING(glMultMatrixx);
#endif
    FORWARD_IF_REMOTE(glMultMatrixx);
    LOAD_GLES(glMultMatrixx);
    gles_glMultMatrixx(m);
}
#endif
#ifndef skip_glMultiTexCoord4f
void glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
#ifndef direct_glMultiTexCoord4f
    PUSH_IF_COMPILING(glMultiTexCoord4f);
#endif
    FORWARD_IF_REMOTE(glMultiTexCoord4f);
    LOAD_GLES(glMultiTexCoord4f);
    gles_glMultiTexCoord4f(target, s, t, r, q);
}
#endif
#ifndef skip_glMultiTexCoord4x
void glMultiTexCoord4x(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q) {
#ifndef direct_glMultiTexCoord4x
    PUSH_IF_COMPILING(glMultiTexCoord4x);
#endif
    FORWARD_IF_REMOTE(glMultiTexCoord4x);
    LOAD_GLES(glMultiTexCoord4x);
    gles_glMultiTexCoord4x(target, s, t, r, q);
}
#endif
#ifndef skip_glNormal3f
void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) {
#ifndef direct_glNormal3f
    PUSH_IF_COMPILING(glNormal3f);
#endif
    FORWARD_IF_REMOTE(glNormal3f);
    LOAD_GLES(glNormal3f);
    gles_glNormal3f(nx, ny, nz);
}
#endif
#ifndef skip_glNormal3x
void glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz) {
#ifndef direct_glNormal3x
    PUSH_IF_COMPILING(glNormal3x);
#endif
    FORWARD_IF_REMOTE(glNormal3x);
    LOAD_GLES(glNormal3x);
    gles_glNormal3x(nx, ny, nz);
}
#endif
#ifndef skip_glNormalPointer
void glNormalPointer(GLenum type, GLsizei stride, const GLvoid * pointer) {
#ifndef direct_glNormalPointer
    PUSH_IF_COMPILING(glNormalPointer);
#endif
    FORWARD_IF_REMOTE(glNormalPointer);
    LOAD_GLES(glNormalPointer);
    gles_glNormalPointer(type, stride, pointer);
}
#endif
#ifndef skip_glOrthof
void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
#ifndef direct_glOrthof
    PUSH_IF_COMPILING(glOrthof);
#endif
    FORWARD_IF_REMOTE(glOrthof);
    LOAD_GLES(glOrthof);
    gles_glOrthof(left, right, bottom, top, near, far);
}
#endif
#ifndef skip_glOrthox
void glOrthox(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed near, GLfixed far) {
#ifndef direct_glOrthox
    PUSH_IF_COMPILING(glOrthox);
#endif
    FORWARD_IF_REMOTE(glOrthox);
    LOAD_GLES(glOrthox);
    gles_glOrthox(left, right, bottom, top, near, far);
}
#endif
#ifndef skip_glPixelStorei
void glPixelStorei(GLenum pname, GLint param) {
#ifndef direct_glPixelStorei
    PUSH_IF_COMPILING(glPixelStorei);
#endif
    FORWARD_IF_REMOTE(glPixelStorei);
    LOAD_GLES(glPixelStorei);
    gles_glPixelStorei(pname, param);
}
#endif
#ifndef skip_glPointParameterf
void glPointParameterf(GLenum pname, GLfloat param) {
#ifndef direct_glPointParameterf
    PUSH_IF_COMPILING(glPointParameterf);
#endif
    FORWARD_IF_REMOTE(glPointParameterf);
    LOAD_GLES(glPointParameterf);
    gles_glPointParameterf(pname, param);
}
#endif
#ifndef skip_glPointParameterfv
void glPointParameterfv(GLenum pname, const GLfloat * params) {
#ifndef direct_glPointParameterfv
    PUSH_IF_COMPILING(glPointParameterfv);
#endif
    FORWARD_IF_REMOTE(glPointParameterfv);
    LOAD_GLES(glPointParameterfv);
    gles_glPointParameterfv(pname, params);
}
#endif
#ifndef skip_glPointParameterx
void glPointParameterx(GLenum pname, GLfixed param) {
#ifndef direct_glPointParameterx
    PUSH_IF_COMPILING(glPointParameterx);
#endif
    FORWARD_IF_REMOTE(glPointParameterx);
    LOAD_GLES(glPointParameterx);
    gles_glPointParameterx(pname, param);
}
#endif
#ifndef skip_glPointParameterxv
void glPointParameterxv(GLenum pname, const GLfixed * params) {
#ifndef direct_glPointParameterxv
    PUSH_IF_COMPILING(glPointParameterxv);
#endif
    FORWARD_IF_REMOTE(glPointParameterxv);
    LOAD_GLES(glPointParameterxv);
    gles_glPointParameterxv(pname, params);
}
#endif
#ifndef skip_glPointSize
void glPointSize(GLfloat size) {
#ifndef direct_glPointSize
    PUSH_IF_COMPILING(glPointSize);
#endif
    FORWARD_IF_REMOTE(glPointSize);
    LOAD_GLES(glPointSize);
    gles_glPointSize(size);
}
#endif
#ifndef skip_glPointSizePointerOES
void glPointSizePointerOES(GLenum type, GLsizei stride, const GLvoid * pointer) {
#ifndef direct_glPointSizePointerOES
    PUSH_IF_COMPILING(glPointSizePointerOES);
#endif
    FORWARD_IF_REMOTE(glPointSizePointerOES);
    LOAD_GLES(glPointSizePointerOES);
    gles_glPointSizePointerOES(type, stride, pointer);
}
#endif
#ifndef skip_glPointSizex
void glPointSizex(GLfixed size) {
#ifndef direct_glPointSizex
    PUSH_IF_COMPILING(glPointSizex);
#endif
    FORWARD_IF_REMOTE(glPointSizex);
    LOAD_GLES(glPointSizex);
    gles_glPointSizex(size);
}
#endif
#ifndef skip_glPolygonOffset
void glPolygonOffset(GLfloat factor, GLfloat units) {
#ifndef direct_glPolygonOffset
    PUSH_IF_COMPILING(glPolygonOffset);
#endif
    FORWARD_IF_REMOTE(glPolygonOffset);
    LOAD_GLES(glPolygonOffset);
    gles_glPolygonOffset(factor, units);
}
#endif
#ifndef skip_glPolygonOffsetx
void glPolygonOffsetx(GLfixed factor, GLfixed units) {
#ifndef direct_glPolygonOffsetx
    PUSH_IF_COMPILING(glPolygonOffsetx);
#endif
    FORWARD_IF_REMOTE(glPolygonOffsetx);
    LOAD_GLES(glPolygonOffsetx);
    gles_glPolygonOffsetx(factor, units);
}
#endif
#ifndef skip_glPopMatrix
void glPopMatrix() {
#ifndef direct_glPopMatrix
    PUSH_IF_COMPILING(glPopMatrix);
#endif
    FORWARD_IF_REMOTE(glPopMatrix);
    LOAD_GLES(glPopMatrix);
    gles_glPopMatrix();
}
#endif
#ifndef skip_glPushMatrix
void glPushMatrix() {
#ifndef direct_glPushMatrix
    PUSH_IF_COMPILING(glPushMatrix);
#endif
    FORWARD_IF_REMOTE(glPushMatrix);
    LOAD_GLES(glPushMatrix);
    gles_glPushMatrix();
}
#endif
#ifndef skip_glReadPixels
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * pixels) {
#ifndef direct_glReadPixels
    PUSH_IF_COMPILING(glReadPixels);
#endif
    FORWARD_IF_REMOTE(glReadPixels);
    LOAD_GLES(glReadPixels);
    gles_glReadPixels(x, y, width, height, format, type, pixels);
}
#endif
#ifndef skip_glRotatef
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
#ifndef direct_glRotatef
    PUSH_IF_COMPILING(glRotatef);
#endif
    FORWARD_IF_REMOTE(glRotatef);
    LOAD_GLES(glRotatef);
    gles_glRotatef(angle, x, y, z);
}
#endif
#ifndef skip_glRotatex
void glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z) {
#ifndef direct_glRotatex
    PUSH_IF_COMPILING(glRotatex);
#endif
    FORWARD_IF_REMOTE(glRotatex);
    LOAD_GLES(glRotatex);
    gles_glRotatex(angle, x, y, z);
}
#endif
#ifndef skip_glSampleCoverage
void glSampleCoverage(GLclampf value, GLboolean invert) {
#ifndef direct_glSampleCoverage
    PUSH_IF_COMPILING(glSampleCoverage);
#endif
    FORWARD_IF_REMOTE(glSampleCoverage);
    LOAD_GLES(glSampleCoverage);
    gles_glSampleCoverage(value, invert);
}
#endif
#ifndef skip_glSampleCoveragex
void glSampleCoveragex(GLclampx value, GLboolean invert) {
#ifndef direct_glSampleCoveragex
    PUSH_IF_COMPILING(glSampleCoveragex);
#endif
    FORWARD_IF_REMOTE(glSampleCoveragex);
    LOAD_GLES(glSampleCoveragex);
    gles_glSampleCoveragex(value, invert);
}
#endif
#ifndef skip_glScalef
void glScalef(GLfloat x, GLfloat y, GLfloat z) {
#ifndef direct_glScalef
    PUSH_IF_COMPILING(glScalef);
#endif
    FORWARD_IF_REMOTE(glScalef);
    LOAD_GLES(glScalef);
    gles_glScalef(x, y, z);
}
#endif
#ifndef skip_glScalex
void glScalex(GLfixed x, GLfixed y, GLfixed z) {
#ifndef direct_glScalex
    PUSH_IF_COMPILING(glScalex);
#endif
    FORWARD_IF_REMOTE(glScalex);
    LOAD_GLES(glScalex);
    gles_glScalex(x, y, z);
}
#endif
#ifndef skip_glScissor
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
#ifndef direct_glScissor
    PUSH_IF_COMPILING(glScissor);
#endif
    FORWARD_IF_REMOTE(glScissor);
    LOAD_GLES(glScissor);
    gles_glScissor(x, y, width, height);
}
#endif
#ifndef skip_glShadeModel
void glShadeModel(GLenum mode) {
#ifndef direct_glShadeModel
    PUSH_IF_COMPILING(glShadeModel);
#endif
    FORWARD_IF_REMOTE(glShadeModel);
    LOAD_GLES(glShadeModel);
    gles_glShadeModel(mode);
}
#endif
#ifndef skip_glStencilFunc
void glStencilFunc(GLenum func, GLint ref, GLuint mask) {
#ifndef direct_glStencilFunc
    PUSH_IF_COMPILING(glStencilFunc);
#endif
    FORWARD_IF_REMOTE(glStencilFunc);
    LOAD_GLES(glStencilFunc);
    gles_glStencilFunc(func, ref, mask);
}
#endif
#ifndef skip_glStencilMask
void glStencilMask(GLuint mask) {
#ifndef direct_glStencilMask
    PUSH_IF_COMPILING(glStencilMask);
#endif
    FORWARD_IF_REMOTE(glStencilMask);
    LOAD_GLES(glStencilMask);
    gles_glStencilMask(mask);
}
#endif
#ifndef skip_glStencilOp
void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
#ifndef direct_glStencilOp
    PUSH_IF_COMPILING(glStencilOp);
#endif
    FORWARD_IF_REMOTE(glStencilOp);
    LOAD_GLES(glStencilOp);
    gles_glStencilOp(fail, zfail, zpass);
}
#endif
#ifndef skip_glTexCoordPointer
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer) {
#ifndef direct_glTexCoordPointer
    PUSH_IF_COMPILING(glTexCoordPointer);
#endif
    FORWARD_IF_REMOTE(glTexCoordPointer);
    LOAD_GLES(glTexCoordPointer);
    gles_glTexCoordPointer(size, type, stride, pointer);
}
#endif
#ifndef skip_glTexEnvf
void glTexEnvf(GLenum target, GLenum pname, GLfloat param) {
#ifndef direct_glTexEnvf
    PUSH_IF_COMPILING(glTexEnvf);
#endif
    FORWARD_IF_REMOTE(glTexEnvf);
    LOAD_GLES(glTexEnvf);
    gles_glTexEnvf(target, pname, param);
}
#endif
#ifndef skip_glTexEnvfv
void glTexEnvfv(GLenum target, GLenum pname, const GLfloat * params) {
#ifndef direct_glTexEnvfv
    PUSH_IF_COMPILING(glTexEnvfv);
#endif
    FORWARD_IF_REMOTE(glTexEnvfv);
    LOAD_GLES(glTexEnvfv);
    gles_glTexEnvfv(target, pname, params);
}
#endif
#ifndef skip_glTexEnvi
void glTexEnvi(GLenum target, GLenum pname, GLint param) {
#ifndef direct_glTexEnvi
    PUSH_IF_COMPILING(glTexEnvi);
#endif
    FORWARD_IF_REMOTE(glTexEnvi);
    LOAD_GLES(glTexEnvi);
    gles_glTexEnvi(target, pname, param);
}
#endif
#ifndef skip_glTexEnviv
void glTexEnviv(GLenum target, GLenum pname, const GLint * params) {
#ifndef direct_glTexEnviv
    PUSH_IF_COMPILING(glTexEnviv);
#endif
    FORWARD_IF_REMOTE(glTexEnviv);
    LOAD_GLES(glTexEnviv);
    gles_glTexEnviv(target, pname, params);
}
#endif
#ifndef skip_glTexEnvx
void glTexEnvx(GLenum target, GLenum pname, GLfixed param) {
#ifndef direct_glTexEnvx
    PUSH_IF_COMPILING(glTexEnvx);
#endif
    FORWARD_IF_REMOTE(glTexEnvx);
    LOAD_GLES(glTexEnvx);
    gles_glTexEnvx(target, pname, param);
}
#endif
#ifndef skip_glTexEnvxv
void glTexEnvxv(GLenum target, GLenum pname, const GLfixed * params) {
#ifndef direct_glTexEnvxv
    PUSH_IF_COMPILING(glTexEnvxv);
#endif
    FORWARD_IF_REMOTE(glTexEnvxv);
    LOAD_GLES(glTexEnvxv);
    gles_glTexEnvxv(target, pname, params);
}
#endif
#ifndef skip_glTexImage2D
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels) {
#ifndef direct_glTexImage2D
    PUSH_IF_COMPILING(glTexImage2D);
#endif
    FORWARD_IF_REMOTE(glTexImage2D);
    LOAD_GLES(glTexImage2D);
    gles_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}
#endif
#ifndef skip_glTexParameterf
void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
#ifndef direct_glTexParameterf
    PUSH_IF_COMPILING(glTexParameterf);
#endif
    FORWARD_IF_REMOTE(glTexParameterf);
    LOAD_GLES(glTexParameterf);
    gles_glTexParameterf(target, pname, param);
}
#endif
#ifndef skip_glTexParameterfv
void glTexParameterfv(GLenum target, GLenum pname, const GLfloat * params) {
#ifndef direct_glTexParameterfv
    PUSH_IF_COMPILING(glTexParameterfv);
#endif
    FORWARD_IF_REMOTE(glTexParameterfv);
    LOAD_GLES(glTexParameterfv);
    gles_glTexParameterfv(target, pname, params);
}
#endif
#ifndef skip_glTexParameteri
void glTexParameteri(GLenum target, GLenum pname, GLint param) {
#ifndef direct_glTexParameteri
    PUSH_IF_COMPILING(glTexParameteri);
#endif
    FORWARD_IF_REMOTE(glTexParameteri);
    LOAD_GLES(glTexParameteri);
    gles_glTexParameteri(target, pname, param);
}
#endif
#ifndef skip_glTexParameteriv
void glTexParameteriv(GLenum target, GLenum pname, const GLint * params) {
#ifndef direct_glTexParameteriv
    PUSH_IF_COMPILING(glTexParameteriv);
#endif
    FORWARD_IF_REMOTE(glTexParameteriv);
    LOAD_GLES(glTexParameteriv);
    gles_glTexParameteriv(target, pname, params);
}
#endif
#ifndef skip_glTexParameterx
void glTexParameterx(GLenum target, GLenum pname, GLfixed param) {
#ifndef direct_glTexParameterx
    PUSH_IF_COMPILING(glTexParameterx);
#endif
    FORWARD_IF_REMOTE(glTexParameterx);
    LOAD_GLES(glTexParameterx);
    gles_glTexParameterx(target, pname, param);
}
#endif
#ifndef skip_glTexParameterxv
void glTexParameterxv(GLenum target, GLenum pname, const GLfixed * params) {
#ifndef direct_glTexParameterxv
    PUSH_IF_COMPILING(glTexParameterxv);
#endif
    FORWARD_IF_REMOTE(glTexParameterxv);
    LOAD_GLES(glTexParameterxv);
    gles_glTexParameterxv(target, pname, params);
}
#endif
#ifndef skip_glTexSubImage2D
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels) {
#ifndef direct_glTexSubImage2D
    PUSH_IF_COMPILING(glTexSubImage2D);
#endif
    FORWARD_IF_REMOTE(glTexSubImage2D);
    LOAD_GLES(glTexSubImage2D);
    gles_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}
#endif
#ifndef skip_glTranslatef
void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {
#ifndef direct_glTranslatef
    PUSH_IF_COMPILING(glTranslatef);
#endif
    FORWARD_IF_REMOTE(glTranslatef);
    LOAD_GLES(glTranslatef);
    gles_glTranslatef(x, y, z);
}
#endif
#ifndef skip_glTranslatex
void glTranslatex(GLfixed x, GLfixed y, GLfixed z) {
#ifndef direct_glTranslatex
    PUSH_IF_COMPILING(glTranslatex);
#endif
    FORWARD_IF_REMOTE(glTranslatex);
    LOAD_GLES(glTranslatex);
    gles_glTranslatex(x, y, z);
}
#endif
#ifndef skip_glVertexPointer
void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer) {
#ifndef direct_glVertexPointer
    PUSH_IF_COMPILING(glVertexPointer);
#endif
    FORWARD_IF_REMOTE(glVertexPointer);
    LOAD_GLES(glVertexPointer);
    gles_glVertexPointer(size, type, stride, pointer);
}
#endif
#ifndef skip_glViewport
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
#ifndef direct_glViewport
    PUSH_IF_COMPILING(glViewport);
#endif
    FORWARD_IF_REMOTE(glViewport);
    LOAD_GLES(glViewport);
    gles_glViewport(x, y, width, height);
}
#endif
#endif
