#ifdef USE_ES2
#include "glpack.h"
#include "../loader.h"

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
#ifndef skip_glAttachShader
void glAttachShader(GLuint program, GLuint shader) {
#ifndef direct_glAttachShader
    PUSH_IF_COMPILING(glAttachShader);
#endif
    FORWARD_IF_REMOTE(glAttachShader);
    LOAD_GLES(glAttachShader);
    gles_glAttachShader(program, shader);
}
#endif
#ifndef skip_glBindAttribLocation
void glBindAttribLocation(GLuint program, GLuint index, const GLchar * name) {
#ifndef direct_glBindAttribLocation
    PUSH_IF_COMPILING(glBindAttribLocation);
#endif
    FORWARD_IF_REMOTE(glBindAttribLocation);
    LOAD_GLES(glBindAttribLocation);
    gles_glBindAttribLocation(program, index, name);
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
#ifndef skip_glBindFramebuffer
void glBindFramebuffer(GLenum target, GLuint framebuffer) {
#ifndef direct_glBindFramebuffer
    PUSH_IF_COMPILING(glBindFramebuffer);
#endif
    FORWARD_IF_REMOTE(glBindFramebuffer);
    LOAD_GLES(glBindFramebuffer);
    gles_glBindFramebuffer(target, framebuffer);
}
#endif
#ifndef skip_glBindRenderbuffer
void glBindRenderbuffer(GLenum target, GLuint renderbuffer) {
#ifndef direct_glBindRenderbuffer
    PUSH_IF_COMPILING(glBindRenderbuffer);
#endif
    FORWARD_IF_REMOTE(glBindRenderbuffer);
    LOAD_GLES(glBindRenderbuffer);
    gles_glBindRenderbuffer(target, renderbuffer);
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
#ifndef skip_glBlendColor
void glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
#ifndef direct_glBlendColor
    PUSH_IF_COMPILING(glBlendColor);
#endif
    FORWARD_IF_REMOTE(glBlendColor);
    LOAD_GLES(glBlendColor);
    gles_glBlendColor(red, green, blue, alpha);
}
#endif
#ifndef skip_glBlendEquation
void glBlendEquation(GLenum mode) {
#ifndef direct_glBlendEquation
    PUSH_IF_COMPILING(glBlendEquation);
#endif
    FORWARD_IF_REMOTE(glBlendEquation);
    LOAD_GLES(glBlendEquation);
    gles_glBlendEquation(mode);
}
#endif
#ifndef skip_glBlendEquationSeparate
void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) {
#ifndef direct_glBlendEquationSeparate
    PUSH_IF_COMPILING(glBlendEquationSeparate);
#endif
    FORWARD_IF_REMOTE(glBlendEquationSeparate);
    LOAD_GLES(glBlendEquationSeparate);
    gles_glBlendEquationSeparate(modeRGB, modeAlpha);
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
#ifndef skip_glBlendFuncSeparate
void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
#ifndef direct_glBlendFuncSeparate
    PUSH_IF_COMPILING(glBlendFuncSeparate);
#endif
    FORWARD_IF_REMOTE(glBlendFuncSeparate);
    LOAD_GLES(glBlendFuncSeparate);
    gles_glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
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
#ifndef skip_glCheckFramebufferStatus
GLenum glCheckFramebufferStatus(GLenum target) {
#ifndef direct_glCheckFramebufferStatus
    PUSH_IF_COMPILING(glCheckFramebufferStatus);
#endif
    FORWARD_IF_REMOTE(glCheckFramebufferStatus);
    LOAD_GLES(glCheckFramebufferStatus);
    return gles_glCheckFramebufferStatus(target);
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
#ifndef skip_glCompileShader
void glCompileShader(GLuint shader) {
#ifndef direct_glCompileShader
    PUSH_IF_COMPILING(glCompileShader);
#endif
    FORWARD_IF_REMOTE(glCompileShader);
    LOAD_GLES(glCompileShader);
    gles_glCompileShader(shader);
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
#ifndef skip_glCreateProgram
GLuint glCreateProgram() {
#ifndef direct_glCreateProgram
    PUSH_IF_COMPILING(glCreateProgram);
#endif
    FORWARD_IF_REMOTE(glCreateProgram);
    LOAD_GLES(glCreateProgram);
    return gles_glCreateProgram();
}
#endif
#ifndef skip_glCreateShader
GLuint glCreateShader(GLenum type) {
#ifndef direct_glCreateShader
    PUSH_IF_COMPILING(glCreateShader);
#endif
    FORWARD_IF_REMOTE(glCreateShader);
    LOAD_GLES(glCreateShader);
    return gles_glCreateShader(type);
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
#ifndef skip_glDeleteFramebuffers
void glDeleteFramebuffers(GLsizei n, const GLuint * framebuffers) {
#ifndef direct_glDeleteFramebuffers
    PUSH_IF_COMPILING(glDeleteFramebuffers);
#endif
    FORWARD_IF_REMOTE(glDeleteFramebuffers);
    LOAD_GLES(glDeleteFramebuffers);
    gles_glDeleteFramebuffers(n, framebuffers);
}
#endif
#ifndef skip_glDeleteProgram
void glDeleteProgram(GLuint program) {
#ifndef direct_glDeleteProgram
    PUSH_IF_COMPILING(glDeleteProgram);
#endif
    FORWARD_IF_REMOTE(glDeleteProgram);
    LOAD_GLES(glDeleteProgram);
    gles_glDeleteProgram(program);
}
#endif
#ifndef skip_glDeleteRenderbuffers
void glDeleteRenderbuffers(GLsizei n, const GLuint * renderbuffers) {
#ifndef direct_glDeleteRenderbuffers
    PUSH_IF_COMPILING(glDeleteRenderbuffers);
#endif
    FORWARD_IF_REMOTE(glDeleteRenderbuffers);
    LOAD_GLES(glDeleteRenderbuffers);
    gles_glDeleteRenderbuffers(n, renderbuffers);
}
#endif
#ifndef skip_glDeleteShader
void glDeleteShader(GLuint shader) {
#ifndef direct_glDeleteShader
    PUSH_IF_COMPILING(glDeleteShader);
#endif
    FORWARD_IF_REMOTE(glDeleteShader);
    LOAD_GLES(glDeleteShader);
    gles_glDeleteShader(shader);
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
#ifndef skip_glDetachShader
void glDetachShader(GLuint program, GLuint shader) {
#ifndef direct_glDetachShader
    PUSH_IF_COMPILING(glDetachShader);
#endif
    FORWARD_IF_REMOTE(glDetachShader);
    LOAD_GLES(glDetachShader);
    gles_glDetachShader(program, shader);
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
#ifndef skip_glDisableVertexAttribArray
void glDisableVertexAttribArray(GLuint index) {
#ifndef direct_glDisableVertexAttribArray
    PUSH_IF_COMPILING(glDisableVertexAttribArray);
#endif
    FORWARD_IF_REMOTE(glDisableVertexAttribArray);
    LOAD_GLES(glDisableVertexAttribArray);
    gles_glDisableVertexAttribArray(index);
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
#ifndef skip_glEnableVertexAttribArray
void glEnableVertexAttribArray(GLuint index) {
#ifndef direct_glEnableVertexAttribArray
    PUSH_IF_COMPILING(glEnableVertexAttribArray);
#endif
    FORWARD_IF_REMOTE(glEnableVertexAttribArray);
    LOAD_GLES(glEnableVertexAttribArray);
    gles_glEnableVertexAttribArray(index);
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
#ifndef skip_glFramebufferRenderbuffer
void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
#ifndef direct_glFramebufferRenderbuffer
    PUSH_IF_COMPILING(glFramebufferRenderbuffer);
#endif
    FORWARD_IF_REMOTE(glFramebufferRenderbuffer);
    LOAD_GLES(glFramebufferRenderbuffer);
    gles_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}
#endif
#ifndef skip_glFramebufferTexture2D
void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
#ifndef direct_glFramebufferTexture2D
    PUSH_IF_COMPILING(glFramebufferTexture2D);
#endif
    FORWARD_IF_REMOTE(glFramebufferTexture2D);
    LOAD_GLES(glFramebufferTexture2D);
    gles_glFramebufferTexture2D(target, attachment, textarget, texture, level);
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
#ifndef skip_glGenFramebuffers
void glGenFramebuffers(GLsizei n, GLuint * framebuffers) {
#ifndef direct_glGenFramebuffers
    PUSH_IF_COMPILING(glGenFramebuffers);
#endif
    FORWARD_IF_REMOTE(glGenFramebuffers);
    LOAD_GLES(glGenFramebuffers);
    gles_glGenFramebuffers(n, framebuffers);
}
#endif
#ifndef skip_glGenRenderbuffers
void glGenRenderbuffers(GLsizei n, GLuint * renderbuffers) {
#ifndef direct_glGenRenderbuffers
    PUSH_IF_COMPILING(glGenRenderbuffers);
#endif
    FORWARD_IF_REMOTE(glGenRenderbuffers);
    LOAD_GLES(glGenRenderbuffers);
    gles_glGenRenderbuffers(n, renderbuffers);
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
#ifndef skip_glGenerateMipmap
void glGenerateMipmap(GLenum target) {
#ifndef direct_glGenerateMipmap
    PUSH_IF_COMPILING(glGenerateMipmap);
#endif
    FORWARD_IF_REMOTE(glGenerateMipmap);
    LOAD_GLES(glGenerateMipmap);
    gles_glGenerateMipmap(target);
}
#endif
#ifndef skip_glGetActiveAttrib
void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name) {
#ifndef direct_glGetActiveAttrib
    PUSH_IF_COMPILING(glGetActiveAttrib);
#endif
    FORWARD_IF_REMOTE(glGetActiveAttrib);
    LOAD_GLES(glGetActiveAttrib);
    gles_glGetActiveAttrib(program, index, bufSize, length, size, type, name);
}
#endif
#ifndef skip_glGetActiveUniform
void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name) {
#ifndef direct_glGetActiveUniform
    PUSH_IF_COMPILING(glGetActiveUniform);
#endif
    FORWARD_IF_REMOTE(glGetActiveUniform);
    LOAD_GLES(glGetActiveUniform);
    gles_glGetActiveUniform(program, index, bufSize, length, size, type, name);
}
#endif
#ifndef skip_glGetAttachedShaders
void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei * count, GLuint * obj) {
#ifndef direct_glGetAttachedShaders
    PUSH_IF_COMPILING(glGetAttachedShaders);
#endif
    FORWARD_IF_REMOTE(glGetAttachedShaders);
    LOAD_GLES(glGetAttachedShaders);
    gles_glGetAttachedShaders(program, maxCount, count, obj);
}
#endif
#ifndef skip_glGetAttribLocation
GLint glGetAttribLocation(GLuint program, const GLchar * name) {
#ifndef direct_glGetAttribLocation
    PUSH_IF_COMPILING(glGetAttribLocation);
#endif
    FORWARD_IF_REMOTE(glGetAttribLocation);
    LOAD_GLES(glGetAttribLocation);
    return gles_glGetAttribLocation(program, name);
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
#ifndef skip_glGetFramebufferAttachmentParameteriv
void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint * params) {
#ifndef direct_glGetFramebufferAttachmentParameteriv
    PUSH_IF_COMPILING(glGetFramebufferAttachmentParameteriv);
#endif
    FORWARD_IF_REMOTE(glGetFramebufferAttachmentParameteriv);
    LOAD_GLES(glGetFramebufferAttachmentParameteriv);
    gles_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
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
#ifndef skip_glGetProgramInfoLog
void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog) {
#ifndef direct_glGetProgramInfoLog
    PUSH_IF_COMPILING(glGetProgramInfoLog);
#endif
    FORWARD_IF_REMOTE(glGetProgramInfoLog);
    LOAD_GLES(glGetProgramInfoLog);
    gles_glGetProgramInfoLog(program, bufSize, length, infoLog);
}
#endif
#ifndef skip_glGetProgramiv
void glGetProgramiv(GLuint program, GLenum pname, GLint * params) {
#ifndef direct_glGetProgramiv
    PUSH_IF_COMPILING(glGetProgramiv);
#endif
    FORWARD_IF_REMOTE(glGetProgramiv);
    LOAD_GLES(glGetProgramiv);
    gles_glGetProgramiv(program, pname, params);
}
#endif
#ifndef skip_glGetRenderbufferParameteriv
void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint * params) {
#ifndef direct_glGetRenderbufferParameteriv
    PUSH_IF_COMPILING(glGetRenderbufferParameteriv);
#endif
    FORWARD_IF_REMOTE(glGetRenderbufferParameteriv);
    LOAD_GLES(glGetRenderbufferParameteriv);
    gles_glGetRenderbufferParameteriv(target, pname, params);
}
#endif
#ifndef skip_glGetShaderInfoLog
void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog) {
#ifndef direct_glGetShaderInfoLog
    PUSH_IF_COMPILING(glGetShaderInfoLog);
#endif
    FORWARD_IF_REMOTE(glGetShaderInfoLog);
    LOAD_GLES(glGetShaderInfoLog);
    gles_glGetShaderInfoLog(shader, bufSize, length, infoLog);
}
#endif
#ifndef skip_glGetShaderPrecisionFormat
void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint * range, GLint * precision) {
#ifndef direct_glGetShaderPrecisionFormat
    PUSH_IF_COMPILING(glGetShaderPrecisionFormat);
#endif
    FORWARD_IF_REMOTE(glGetShaderPrecisionFormat);
    LOAD_GLES(glGetShaderPrecisionFormat);
    gles_glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
}
#endif
#ifndef skip_glGetShaderSource
void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source) {
#ifndef direct_glGetShaderSource
    PUSH_IF_COMPILING(glGetShaderSource);
#endif
    FORWARD_IF_REMOTE(glGetShaderSource);
    LOAD_GLES(glGetShaderSource);
    gles_glGetShaderSource(shader, bufSize, length, source);
}
#endif
#ifndef skip_glGetShaderiv
void glGetShaderiv(GLuint shader, GLenum pname, GLint * params) {
#ifndef direct_glGetShaderiv
    PUSH_IF_COMPILING(glGetShaderiv);
#endif
    FORWARD_IF_REMOTE(glGetShaderiv);
    LOAD_GLES(glGetShaderiv);
    gles_glGetShaderiv(shader, pname, params);
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
#ifndef skip_glGetUniformLocation
GLint glGetUniformLocation(GLuint program, const GLchar * name) {
#ifndef direct_glGetUniformLocation
    PUSH_IF_COMPILING(glGetUniformLocation);
#endif
    FORWARD_IF_REMOTE(glGetUniformLocation);
    LOAD_GLES(glGetUniformLocation);
    return gles_glGetUniformLocation(program, name);
}
#endif
#ifndef skip_glGetUniformfv
void glGetUniformfv(GLuint program, GLint location, GLfloat * params) {
#ifndef direct_glGetUniformfv
    PUSH_IF_COMPILING(glGetUniformfv);
#endif
    FORWARD_IF_REMOTE(glGetUniformfv);
    LOAD_GLES(glGetUniformfv);
    gles_glGetUniformfv(program, location, params);
}
#endif
#ifndef skip_glGetUniformiv
void glGetUniformiv(GLuint program, GLint location, GLint * params) {
#ifndef direct_glGetUniformiv
    PUSH_IF_COMPILING(glGetUniformiv);
#endif
    FORWARD_IF_REMOTE(glGetUniformiv);
    LOAD_GLES(glGetUniformiv);
    gles_glGetUniformiv(program, location, params);
}
#endif
#ifndef skip_glGetVertexAttribPointerv
void glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid ** pointer) {
#ifndef direct_glGetVertexAttribPointerv
    PUSH_IF_COMPILING(glGetVertexAttribPointerv);
#endif
    FORWARD_IF_REMOTE(glGetVertexAttribPointerv);
    LOAD_GLES(glGetVertexAttribPointerv);
    gles_glGetVertexAttribPointerv(index, pname, pointer);
}
#endif
#ifndef skip_glGetVertexAttribfv
void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat * params) {
#ifndef direct_glGetVertexAttribfv
    PUSH_IF_COMPILING(glGetVertexAttribfv);
#endif
    FORWARD_IF_REMOTE(glGetVertexAttribfv);
    LOAD_GLES(glGetVertexAttribfv);
    gles_glGetVertexAttribfv(index, pname, params);
}
#endif
#ifndef skip_glGetVertexAttribiv
void glGetVertexAttribiv(GLuint index, GLenum pname, GLint * params) {
#ifndef direct_glGetVertexAttribiv
    PUSH_IF_COMPILING(glGetVertexAttribiv);
#endif
    FORWARD_IF_REMOTE(glGetVertexAttribiv);
    LOAD_GLES(glGetVertexAttribiv);
    gles_glGetVertexAttribiv(index, pname, params);
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
#ifndef skip_glIsFramebuffer
GLboolean glIsFramebuffer(GLuint framebuffer) {
#ifndef direct_glIsFramebuffer
    PUSH_IF_COMPILING(glIsFramebuffer);
#endif
    FORWARD_IF_REMOTE(glIsFramebuffer);
    LOAD_GLES(glIsFramebuffer);
    return gles_glIsFramebuffer(framebuffer);
}
#endif
#ifndef skip_glIsProgram
GLboolean glIsProgram(GLuint program) {
#ifndef direct_glIsProgram
    PUSH_IF_COMPILING(glIsProgram);
#endif
    FORWARD_IF_REMOTE(glIsProgram);
    LOAD_GLES(glIsProgram);
    return gles_glIsProgram(program);
}
#endif
#ifndef skip_glIsRenderbuffer
GLboolean glIsRenderbuffer(GLuint renderbuffer) {
#ifndef direct_glIsRenderbuffer
    PUSH_IF_COMPILING(glIsRenderbuffer);
#endif
    FORWARD_IF_REMOTE(glIsRenderbuffer);
    LOAD_GLES(glIsRenderbuffer);
    return gles_glIsRenderbuffer(renderbuffer);
}
#endif
#ifndef skip_glIsShader
GLboolean glIsShader(GLuint shader) {
#ifndef direct_glIsShader
    PUSH_IF_COMPILING(glIsShader);
#endif
    FORWARD_IF_REMOTE(glIsShader);
    LOAD_GLES(glIsShader);
    return gles_glIsShader(shader);
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
#ifndef skip_glLinkProgram
void glLinkProgram(GLuint program) {
#ifndef direct_glLinkProgram
    PUSH_IF_COMPILING(glLinkProgram);
#endif
    FORWARD_IF_REMOTE(glLinkProgram);
    LOAD_GLES(glLinkProgram);
    gles_glLinkProgram(program);
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
#ifndef skip_glReleaseShaderCompiler
void glReleaseShaderCompiler() {
#ifndef direct_glReleaseShaderCompiler
    PUSH_IF_COMPILING(glReleaseShaderCompiler);
#endif
    FORWARD_IF_REMOTE(glReleaseShaderCompiler);
    LOAD_GLES(glReleaseShaderCompiler);
    gles_glReleaseShaderCompiler();
}
#endif
#ifndef skip_glRenderbufferStorage
void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
#ifndef direct_glRenderbufferStorage
    PUSH_IF_COMPILING(glRenderbufferStorage);
#endif
    FORWARD_IF_REMOTE(glRenderbufferStorage);
    LOAD_GLES(glRenderbufferStorage);
    gles_glRenderbufferStorage(target, internalformat, width, height);
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
#ifndef skip_glShaderBinary
void glShaderBinary(GLsizei count, const GLuint * shaders, GLenum binaryformat, const GLvoid * binary, GLsizei length) {
#ifndef direct_glShaderBinary
    PUSH_IF_COMPILING(glShaderBinary);
#endif
    FORWARD_IF_REMOTE(glShaderBinary);
    LOAD_GLES(glShaderBinary);
    gles_glShaderBinary(count, shaders, binaryformat, binary, length);
}
#endif
#ifndef skip_glShaderSource
void glShaderSource(GLuint shader, GLsizei count, const GLchar * const * string, const GLint * length) {
#ifndef direct_glShaderSource
    PUSH_IF_COMPILING(glShaderSource);
#endif
    FORWARD_IF_REMOTE(glShaderSource);
    LOAD_GLES(glShaderSource);
    gles_glShaderSource(shader, count, string, length);
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
#ifndef skip_glStencilFuncSeparate
void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
#ifndef direct_glStencilFuncSeparate
    PUSH_IF_COMPILING(glStencilFuncSeparate);
#endif
    FORWARD_IF_REMOTE(glStencilFuncSeparate);
    LOAD_GLES(glStencilFuncSeparate);
    gles_glStencilFuncSeparate(face, func, ref, mask);
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
#ifndef skip_glStencilMaskSeparate
void glStencilMaskSeparate(GLenum face, GLuint mask) {
#ifndef direct_glStencilMaskSeparate
    PUSH_IF_COMPILING(glStencilMaskSeparate);
#endif
    FORWARD_IF_REMOTE(glStencilMaskSeparate);
    LOAD_GLES(glStencilMaskSeparate);
    gles_glStencilMaskSeparate(face, mask);
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
#ifndef skip_glStencilOpSeparate
void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
#ifndef direct_glStencilOpSeparate
    PUSH_IF_COMPILING(glStencilOpSeparate);
#endif
    FORWARD_IF_REMOTE(glStencilOpSeparate);
    LOAD_GLES(glStencilOpSeparate);
    gles_glStencilOpSeparate(face, sfail, dpfail, dppass);
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
#ifndef skip_glUniform1f
void glUniform1f(GLint location, GLfloat v0) {
#ifndef direct_glUniform1f
    PUSH_IF_COMPILING(glUniform1f);
#endif
    FORWARD_IF_REMOTE(glUniform1f);
    LOAD_GLES(glUniform1f);
    gles_glUniform1f(location, v0);
}
#endif
#ifndef skip_glUniform1fv
void glUniform1fv(GLint location, GLsizei count, const GLfloat * value) {
#ifndef direct_glUniform1fv
    PUSH_IF_COMPILING(glUniform1fv);
#endif
    FORWARD_IF_REMOTE(glUniform1fv);
    LOAD_GLES(glUniform1fv);
    gles_glUniform1fv(location, count, value);
}
#endif
#ifndef skip_glUniform1i
void glUniform1i(GLint location, GLint v0) {
#ifndef direct_glUniform1i
    PUSH_IF_COMPILING(glUniform1i);
#endif
    FORWARD_IF_REMOTE(glUniform1i);
    LOAD_GLES(glUniform1i);
    gles_glUniform1i(location, v0);
}
#endif
#ifndef skip_glUniform1iv
void glUniform1iv(GLint location, GLsizei count, const GLint * value) {
#ifndef direct_glUniform1iv
    PUSH_IF_COMPILING(glUniform1iv);
#endif
    FORWARD_IF_REMOTE(glUniform1iv);
    LOAD_GLES(glUniform1iv);
    gles_glUniform1iv(location, count, value);
}
#endif
#ifndef skip_glUniform2f
void glUniform2f(GLint location, GLfloat v0, GLfloat v1) {
#ifndef direct_glUniform2f
    PUSH_IF_COMPILING(glUniform2f);
#endif
    FORWARD_IF_REMOTE(glUniform2f);
    LOAD_GLES(glUniform2f);
    gles_glUniform2f(location, v0, v1);
}
#endif
#ifndef skip_glUniform2fv
void glUniform2fv(GLint location, GLsizei count, const GLfloat * value) {
#ifndef direct_glUniform2fv
    PUSH_IF_COMPILING(glUniform2fv);
#endif
    FORWARD_IF_REMOTE(glUniform2fv);
    LOAD_GLES(glUniform2fv);
    gles_glUniform2fv(location, count, value);
}
#endif
#ifndef skip_glUniform2i
void glUniform2i(GLint location, GLint v0, GLint v1) {
#ifndef direct_glUniform2i
    PUSH_IF_COMPILING(glUniform2i);
#endif
    FORWARD_IF_REMOTE(glUniform2i);
    LOAD_GLES(glUniform2i);
    gles_glUniform2i(location, v0, v1);
}
#endif
#ifndef skip_glUniform2iv
void glUniform2iv(GLint location, GLsizei count, const GLint * value) {
#ifndef direct_glUniform2iv
    PUSH_IF_COMPILING(glUniform2iv);
#endif
    FORWARD_IF_REMOTE(glUniform2iv);
    LOAD_GLES(glUniform2iv);
    gles_glUniform2iv(location, count, value);
}
#endif
#ifndef skip_glUniform3f
void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
#ifndef direct_glUniform3f
    PUSH_IF_COMPILING(glUniform3f);
#endif
    FORWARD_IF_REMOTE(glUniform3f);
    LOAD_GLES(glUniform3f);
    gles_glUniform3f(location, v0, v1, v2);
}
#endif
#ifndef skip_glUniform3fv
void glUniform3fv(GLint location, GLsizei count, const GLfloat * value) {
#ifndef direct_glUniform3fv
    PUSH_IF_COMPILING(glUniform3fv);
#endif
    FORWARD_IF_REMOTE(glUniform3fv);
    LOAD_GLES(glUniform3fv);
    gles_glUniform3fv(location, count, value);
}
#endif
#ifndef skip_glUniform3i
void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) {
#ifndef direct_glUniform3i
    PUSH_IF_COMPILING(glUniform3i);
#endif
    FORWARD_IF_REMOTE(glUniform3i);
    LOAD_GLES(glUniform3i);
    gles_glUniform3i(location, v0, v1, v2);
}
#endif
#ifndef skip_glUniform3iv
void glUniform3iv(GLint location, GLsizei count, const GLint * value) {
#ifndef direct_glUniform3iv
    PUSH_IF_COMPILING(glUniform3iv);
#endif
    FORWARD_IF_REMOTE(glUniform3iv);
    LOAD_GLES(glUniform3iv);
    gles_glUniform3iv(location, count, value);
}
#endif
#ifndef skip_glUniform4f
void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
#ifndef direct_glUniform4f
    PUSH_IF_COMPILING(glUniform4f);
#endif
    FORWARD_IF_REMOTE(glUniform4f);
    LOAD_GLES(glUniform4f);
    gles_glUniform4f(location, v0, v1, v2, v3);
}
#endif
#ifndef skip_glUniform4fv
void glUniform4fv(GLint location, GLsizei count, const GLfloat * value) {
#ifndef direct_glUniform4fv
    PUSH_IF_COMPILING(glUniform4fv);
#endif
    FORWARD_IF_REMOTE(glUniform4fv);
    LOAD_GLES(glUniform4fv);
    gles_glUniform4fv(location, count, value);
}
#endif
#ifndef skip_glUniform4i
void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
#ifndef direct_glUniform4i
    PUSH_IF_COMPILING(glUniform4i);
#endif
    FORWARD_IF_REMOTE(glUniform4i);
    LOAD_GLES(glUniform4i);
    gles_glUniform4i(location, v0, v1, v2, v3);
}
#endif
#ifndef skip_glUniform4iv
void glUniform4iv(GLint location, GLsizei count, const GLint * value) {
#ifndef direct_glUniform4iv
    PUSH_IF_COMPILING(glUniform4iv);
#endif
    FORWARD_IF_REMOTE(glUniform4iv);
    LOAD_GLES(glUniform4iv);
    gles_glUniform4iv(location, count, value);
}
#endif
#ifndef skip_glUniformMatrix2fv
void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
#ifndef direct_glUniformMatrix2fv
    PUSH_IF_COMPILING(glUniformMatrix2fv);
#endif
    FORWARD_IF_REMOTE(glUniformMatrix2fv);
    LOAD_GLES(glUniformMatrix2fv);
    gles_glUniformMatrix2fv(location, count, transpose, value);
}
#endif
#ifndef skip_glUniformMatrix3fv
void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
#ifndef direct_glUniformMatrix3fv
    PUSH_IF_COMPILING(glUniformMatrix3fv);
#endif
    FORWARD_IF_REMOTE(glUniformMatrix3fv);
    LOAD_GLES(glUniformMatrix3fv);
    gles_glUniformMatrix3fv(location, count, transpose, value);
}
#endif
#ifndef skip_glUniformMatrix4fv
void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
#ifndef direct_glUniformMatrix4fv
    PUSH_IF_COMPILING(glUniformMatrix4fv);
#endif
    FORWARD_IF_REMOTE(glUniformMatrix4fv);
    LOAD_GLES(glUniformMatrix4fv);
    gles_glUniformMatrix4fv(location, count, transpose, value);
}
#endif
#ifndef skip_glUseProgram
void glUseProgram(GLuint program) {
#ifndef direct_glUseProgram
    PUSH_IF_COMPILING(glUseProgram);
#endif
    FORWARD_IF_REMOTE(glUseProgram);
    LOAD_GLES(glUseProgram);
    gles_glUseProgram(program);
}
#endif
#ifndef skip_glValidateProgram
void glValidateProgram(GLuint program) {
#ifndef direct_glValidateProgram
    PUSH_IF_COMPILING(glValidateProgram);
#endif
    FORWARD_IF_REMOTE(glValidateProgram);
    LOAD_GLES(glValidateProgram);
    gles_glValidateProgram(program);
}
#endif
#ifndef skip_glVertexAttrib1f
void glVertexAttrib1f(GLuint index, GLfloat x) {
#ifndef direct_glVertexAttrib1f
    PUSH_IF_COMPILING(glVertexAttrib1f);
#endif
    FORWARD_IF_REMOTE(glVertexAttrib1f);
    LOAD_GLES(glVertexAttrib1f);
    gles_glVertexAttrib1f(index, x);
}
#endif
#ifndef skip_glVertexAttrib1fv
void glVertexAttrib1fv(GLuint index, const GLfloat * v) {
#ifndef direct_glVertexAttrib1fv
    PUSH_IF_COMPILING(glVertexAttrib1fv);
#endif
    FORWARD_IF_REMOTE(glVertexAttrib1fv);
    LOAD_GLES(glVertexAttrib1fv);
    gles_glVertexAttrib1fv(index, v);
}
#endif
#ifndef skip_glVertexAttrib2f
void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y) {
#ifndef direct_glVertexAttrib2f
    PUSH_IF_COMPILING(glVertexAttrib2f);
#endif
    FORWARD_IF_REMOTE(glVertexAttrib2f);
    LOAD_GLES(glVertexAttrib2f);
    gles_glVertexAttrib2f(index, x, y);
}
#endif
#ifndef skip_glVertexAttrib2fv
void glVertexAttrib2fv(GLuint index, const GLfloat * v) {
#ifndef direct_glVertexAttrib2fv
    PUSH_IF_COMPILING(glVertexAttrib2fv);
#endif
    FORWARD_IF_REMOTE(glVertexAttrib2fv);
    LOAD_GLES(glVertexAttrib2fv);
    gles_glVertexAttrib2fv(index, v);
}
#endif
#ifndef skip_glVertexAttrib3f
void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) {
#ifndef direct_glVertexAttrib3f
    PUSH_IF_COMPILING(glVertexAttrib3f);
#endif
    FORWARD_IF_REMOTE(glVertexAttrib3f);
    LOAD_GLES(glVertexAttrib3f);
    gles_glVertexAttrib3f(index, x, y, z);
}
#endif
#ifndef skip_glVertexAttrib3fv
void glVertexAttrib3fv(GLuint index, const GLfloat * v) {
#ifndef direct_glVertexAttrib3fv
    PUSH_IF_COMPILING(glVertexAttrib3fv);
#endif
    FORWARD_IF_REMOTE(glVertexAttrib3fv);
    LOAD_GLES(glVertexAttrib3fv);
    gles_glVertexAttrib3fv(index, v);
}
#endif
#ifndef skip_glVertexAttrib4f
void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
#ifndef direct_glVertexAttrib4f
    PUSH_IF_COMPILING(glVertexAttrib4f);
#endif
    FORWARD_IF_REMOTE(glVertexAttrib4f);
    LOAD_GLES(glVertexAttrib4f);
    gles_glVertexAttrib4f(index, x, y, z, w);
}
#endif
#ifndef skip_glVertexAttrib4fv
void glVertexAttrib4fv(GLuint index, const GLfloat * v) {
#ifndef direct_glVertexAttrib4fv
    PUSH_IF_COMPILING(glVertexAttrib4fv);
#endif
    FORWARD_IF_REMOTE(glVertexAttrib4fv);
    LOAD_GLES(glVertexAttrib4fv);
    gles_glVertexAttrib4fv(index, v);
}
#endif
#ifndef skip_glVertexAttribPointer
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer) {
#ifndef direct_glVertexAttribPointer
    PUSH_IF_COMPILING(glVertexAttribPointer);
#endif
    FORWARD_IF_REMOTE(glVertexAttribPointer);
    LOAD_GLES(glVertexAttribPointer);
    gles_glVertexAttribPointer(index, size, type, normalized, stride, pointer);
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
