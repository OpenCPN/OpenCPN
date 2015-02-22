#include <GL/gl.h>
#include <GL/glx.h>

#ifndef GL_H
#define GL_H

typedef struct {
    int format;
    void *func;
    void *args;
} packed_call_t;

typedef struct {
    int func;
    void *args;
} indexed_call_t;

enum FORMAT {
    FORMAT_void_GLenum_GLfloat,
    FORMAT_void_GLenum,
    FORMAT_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__,
    FORMAT_void_GLint,
    FORMAT_void_GLenum_GLuint,
    FORMAT_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__,
    FORMAT_void_GLfloat_GLfloat_GLfloat_GLfloat,
    FORMAT_void_GLenum_GLenum,
    FORMAT_void_GLenum_GLenum_GLenum_GLenum,
    FORMAT_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum,
    FORMAT_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__,
    FORMAT_void_GLuint,
    FORMAT_void_GLsizei_GLenum_const_GLvoid___GENPT__,
    FORMAT_void_GLbitfield,
    FORMAT_void_GLdouble,
    FORMAT_void_GLfloat,
    FORMAT_void_GLenum_const_GLdouble___GENPT__,
    FORMAT_void_GLbyte_GLbyte_GLbyte,
    FORMAT_void_const_GLbyte___GENPT__,
    FORMAT_void_GLdouble_GLdouble_GLdouble,
    FORMAT_void_const_GLdouble___GENPT__,
    FORMAT_void_GLfloat_GLfloat_GLfloat,
    FORMAT_void_const_GLfloat___GENPT__,
    FORMAT_void_GLint_GLint_GLint,
    FORMAT_void_const_GLint___GENPT__,
    FORMAT_void_GLshort_GLshort_GLshort,
    FORMAT_void_const_GLshort___GENPT__,
    FORMAT_void_GLubyte_GLubyte_GLubyte,
    FORMAT_void_const_GLubyte___GENPT__,
    FORMAT_void_GLuint_GLuint_GLuint,
    FORMAT_void_const_GLuint___GENPT__,
    FORMAT_void_GLushort_GLushort_GLushort,
    FORMAT_void_const_GLushort___GENPT__,
    FORMAT_void_GLbyte_GLbyte_GLbyte_GLbyte,
    FORMAT_void_GLdouble_GLdouble_GLdouble_GLdouble,
    FORMAT_void_GLint_GLint_GLint_GLint,
    FORMAT_void_GLshort_GLshort_GLshort_GLshort,
    FORMAT_void_GLubyte_GLubyte_GLubyte_GLubyte,
    FORMAT_void_GLuint_GLuint_GLuint_GLuint,
    FORMAT_void_GLushort_GLushort_GLushort_GLushort,
    FORMAT_void_GLboolean_GLboolean_GLboolean_GLboolean,
    FORMAT_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLenum_const_GLfloat___GENPT__,
    FORMAT_void_GLenum_GLenum_const_GLint___GENPT__,
    FORMAT_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLenum_GLfloat,
    FORMAT_void_GLenum_GLenum_GLint,
    FORMAT_void_GLenum_GLsizei_GLint_GLint_GLsizei,
    FORMAT_void_GLenum_GLenum_GLint_GLint_GLsizei,
    FORMAT_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei,
    FORMAT_void_GLint_GLint_GLsizei_GLsizei_GLenum,
    FORMAT_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint,
    FORMAT_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint,
    FORMAT_void_GLenum_GLint_GLint_GLint_GLint_GLsizei,
    FORMAT_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei,
    FORMAT_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei,
    FORMAT_void_GLsizei_const_GLuint___GENPT__,
    FORMAT_void_GLuint_GLsizei,
    FORMAT_void_GLboolean,
    FORMAT_void_GLdouble_GLdouble,
    FORMAT_void_GLenum_GLint_GLsizei,
    FORMAT_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__,
    FORMAT_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__,
    FORMAT_void_GLsizei_const_GLvoid___GENPT__,
    FORMAT_void_const_GLboolean___GENPT__,
    FORMAT_void,
    FORMAT_void_GLfloat_GLfloat,
    FORMAT_void_GLenum_GLint_GLint,
    FORMAT_void_GLenum_GLint_GLint_GLint_GLint,
    FORMAT_void_GLint_GLint,
    FORMAT_void_GLsizei_GLenum_GLfloat___GENPT__,
    FORMAT_void_GLenum_GLsizei_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_const_GLfloat___GENPT__,
    FORMAT_void_GLenum_GLint,
    FORMAT_void_GLenum_const_GLint___GENPT__,
    FORMAT_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble,
    FORMAT_void_GLsizei_GLuint___GENPT__,
    FORMAT_GLuint_GLsizei,
    FORMAT_void_GLenum_GLboolean___GENPT__,
    FORMAT_void_GLenum_GLenum_GLint___GENPT__,
    FORMAT_void_GLenum_GLenum_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLdouble___GENPT__,
    FORMAT_void_GLenum_GLenum_GLenum_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLenum_GLfloat___GENPT__,
    FORMAT_void_GLenum_GLint_GLvoid___GENPT__,
    FORMAT_GLenum,
    FORMAT_void_GLenum_GLfloat___GENPT__,
    FORMAT_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLint___GENPT__,
    FORMAT_void_GLenum_GLenum_GLdouble___GENPT__,
    FORMAT_void_GLenum_GLuint___GENPT__,
    FORMAT_void_GLenum_GLushort___GENPT__,
    FORMAT_void_GLenum_GLvoid___GENPT____GENPT__,
    FORMAT_void_GLubyte___GENPT__,
    FORMAT_void_GLuint_GLenum_GLint___GENPT__,
    FORMAT_void_GLuint_GLenum_GLuint___GENPT__,
    FORMAT_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__,
    FORMAT_const_GLubyte___GENPT___GLenum,
    FORMAT_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLint_GLenum_GLfloat___GENPT__,
    FORMAT_void_GLenum_GLint_GLenum_GLint___GENPT__,
    FORMAT_void_GLenum_GLsizei_GLenum_GLboolean,
    FORMAT_void_GLshort,
    FORMAT_void_GLubyte,
    FORMAT_GLboolean_GLuint,
    FORMAT_GLboolean_GLenum,
    FORMAT_void_GLint_GLushort,
    FORMAT_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__,
    FORMAT_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__,
    FORMAT_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__,
    FORMAT_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__,
    FORMAT_GLvoid___GENPT___GLenum_GLenum,
    FORMAT_void_GLint_GLdouble_GLdouble,
    FORMAT_void_GLint_GLfloat_GLfloat,
    FORMAT_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble,
    FORMAT_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat,
    FORMAT_void_GLenum_GLenum_GLboolean,
    FORMAT_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei,
    FORMAT_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei,
    FORMAT_void_GLenum_GLdouble,
    FORMAT_void_GLenum_GLshort,
    FORMAT_void_GLenum_const_GLshort___GENPT__,
    FORMAT_void_GLenum_GLdouble_GLdouble,
    FORMAT_void_GLenum_GLfloat_GLfloat,
    FORMAT_void_GLenum_GLshort_GLshort,
    FORMAT_void_GLenum_GLdouble_GLdouble_GLdouble,
    FORMAT_void_GLenum_GLfloat_GLfloat_GLfloat,
    FORMAT_void_GLenum_GLint_GLint_GLint,
    FORMAT_void_GLenum_GLshort_GLshort_GLshort,
    FORMAT_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble,
    FORMAT_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat,
    FORMAT_void_GLenum_GLshort_GLshort_GLshort_GLshort,
    FORMAT_void_GLuint_GLenum,
    FORMAT_void_GLenum_GLsizei_const_GLfloat___GENPT__,
    FORMAT_void_GLenum_GLsizei_const_GLuint___GENPT__,
    FORMAT_void_GLenum_GLsizei_const_GLushort___GENPT__,
    FORMAT_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__,
    FORMAT_void_GLshort_GLshort,
    FORMAT_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__,
    FORMAT_void_const_GLdouble___GENPT___const_GLdouble___GENPT__,
    FORMAT_void_const_GLfloat___GENPT___const_GLfloat___GENPT__,
    FORMAT_void_const_GLint___GENPT___const_GLint___GENPT__,
    FORMAT_void_const_GLshort___GENPT___const_GLshort___GENPT__,
    FORMAT_GLint_GLenum,
    FORMAT_void_GLfloat_GLboolean,
    FORMAT_void_GLint_GLint_GLsizei_GLsizei,
    FORMAT_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLint_GLuint,
    FORMAT_void_GLenum_GLenum_GLenum,
    FORMAT_void_GLenum_GLenum_GLdouble,
    FORMAT_void_GLenum_GLenum_const_GLdouble___GENPT__,
    FORMAT_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__,
    FORMAT_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__,
    FORMAT_int_Display___GENPT___int,
    FORMAT_void_uint32_t_uint32_t,
    FORMAT_void_uint32_t,
    FORMAT_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__,
    FORMAT_void_Display___GENPT___GLXContext_GLXContext_unsigned_long,
    FORMAT_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool,
    FORMAT_void_uint32_t_uint32_t_uint32_t_uint32_t,
    FORMAT_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap,
    FORMAT_void_uint32_t_uint32_t_uint32_t,
    FORMAT_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t,
    FORMAT_void_Display___GENPT___GLXContext,
    FORMAT_void_Display___GENPT___GLXPixmap,
    FORMAT_void_Display___GENPT___uint32_t,
    FORMAT_int_Display___GENPT___int_int_int_const_void___GENPT__,
    FORMAT_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__,
    FORMAT_Bool_Display___GENPT___GLXContext,
    FORMAT_Bool_Display___GENPT___GLXDrawable_GLXContext,
    FORMAT_const_char___GENPT___Display___GENPT___int,
    FORMAT_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__,
    FORMAT_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__,
    FORMAT_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__,
    FORMAT_const_char___GENPT___Display___GENPT___int_int,
    FORMAT_Bool_Display___GENPT___int___GENPT___int___GENPT__,
    FORMAT_void_Display___GENPT___GLXDrawable,
    FORMAT_void_Font_int_int_int,
    FORMAT_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__,
    FORMAT_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool,
    FORMAT_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__,
    FORMAT_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__,
    FORMAT_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__,
    FORMAT_void_Display___GENPT___GLXPbuffer,
    FORMAT_void_Display___GENPT___GLXWindow,
    FORMAT_Display___GENPT__,
    FORMAT_GLXDrawable,
    FORMAT_int_Display___GENPT___GLXFBConfig_int_int___GENPT__,
    FORMAT_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__,
    FORMAT___GLXextFuncPtr_const_GLubyte___GENPT__,
    FORMAT_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__,
    FORMAT_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig,
    FORMAT_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext,
    FORMAT_int_Display___GENPT___GLXContext_int_int___GENPT__,
    FORMAT_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__,
    FORMAT_void_Display___GENPT___GLXDrawable_unsigned_long,
};

typedef void (*FUNC_void_GLenum_GLfloat)(GLenum op, GLfloat value);
typedef struct {
    GLenum a1;
    GLfloat a2;
} ARGS_void_GLenum_GLfloat;
typedef struct {
    int format;
    FUNC_void_GLenum_GLfloat func;
    ARGS_void_GLenum_GLfloat args;
} PACKED_void_GLenum_GLfloat;
typedef struct {
    int func;
    ARGS_void_GLenum_GLfloat args;
} INDEXED_void_GLenum_GLfloat;
typedef void (*FUNC_void_GLenum)(GLenum texture);
typedef struct {
    GLenum a1;
} ARGS_void_GLenum;
typedef struct {
    int format;
    FUNC_void_GLenum func;
    ARGS_void_GLenum args;
} PACKED_void_GLenum;
typedef struct {
    int func;
    ARGS_void_GLenum args;
} INDEXED_void_GLenum;
typedef GLboolean (*FUNC_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__)(GLsizei n, const GLuint * textures, GLboolean * residences);
typedef struct {
    GLsizei a1;
    const GLuint * a2;
    GLboolean * a3;
} ARGS_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__;
typedef struct {
    int format;
    FUNC_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__ func;
    ARGS_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__ args;
} PACKED_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__;
typedef struct {
    int func;
    ARGS_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__ args;
} INDEXED_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__;
typedef void (*FUNC_void_GLint)(GLint i);
typedef struct {
    GLint a1;
} ARGS_void_GLint;
typedef struct {
    int format;
    FUNC_void_GLint func;
    ARGS_void_GLint args;
} PACKED_void_GLint;
typedef struct {
    int func;
    ARGS_void_GLint args;
} INDEXED_void_GLint;
typedef void (*FUNC_void_GLenum_GLuint)(GLenum target, GLuint id);
typedef struct {
    GLenum a1;
    GLuint a2;
} ARGS_void_GLenum_GLuint;
typedef struct {
    int format;
    FUNC_void_GLenum_GLuint func;
    ARGS_void_GLenum_GLuint args;
} PACKED_void_GLenum_GLuint;
typedef struct {
    int func;
    ARGS_void_GLenum_GLuint args;
} INDEXED_void_GLenum_GLuint;
typedef void (*FUNC_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte * bitmap);
typedef struct {
    GLsizei a1;
    GLsizei a2;
    GLfloat a3;
    GLfloat a4;
    GLfloat a5;
    GLfloat a6;
    const GLubyte * a7;
} ARGS_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__ func;
    ARGS_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__ args;
} PACKED_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__ args;
} INDEXED_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__;
typedef void (*FUNC_void_GLfloat_GLfloat_GLfloat_GLfloat)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef struct {
    GLfloat a1;
    GLfloat a2;
    GLfloat a3;
    GLfloat a4;
} ARGS_void_GLfloat_GLfloat_GLfloat_GLfloat;
typedef struct {
    int format;
    FUNC_void_GLfloat_GLfloat_GLfloat_GLfloat func;
    ARGS_void_GLfloat_GLfloat_GLfloat_GLfloat args;
} PACKED_void_GLfloat_GLfloat_GLfloat_GLfloat;
typedef struct {
    int func;
    ARGS_void_GLfloat_GLfloat_GLfloat_GLfloat args;
} INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat;
typedef void (*FUNC_void_GLenum_GLenum)(GLenum sfactor, GLenum dfactor);
typedef struct {
    GLenum a1;
    GLenum a2;
} ARGS_void_GLenum_GLenum;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum func;
    ARGS_void_GLenum_GLenum args;
} PACKED_void_GLenum_GLenum;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum args;
} INDEXED_void_GLenum_GLenum;
typedef void (*FUNC_void_GLenum_GLenum_GLenum_GLenum)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLenum a3;
    GLenum a4;
} ARGS_void_GLenum_GLenum_GLenum_GLenum;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLenum_GLenum func;
    ARGS_void_GLenum_GLenum_GLenum_GLenum args;
} PACKED_void_GLenum_GLenum_GLenum_GLenum;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLenum_GLenum args;
} INDEXED_void_GLenum_GLenum_GLenum_GLenum;
typedef void (*FUNC_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum)(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage);
typedef struct {
    GLenum a1;
    GLsizeiptr a2;
    const GLvoid * a3;
    GLenum a4;
} ARGS_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum;
typedef struct {
    int format;
    FUNC_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum func;
    ARGS_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum args;
} PACKED_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum;
typedef struct {
    int func;
    ARGS_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum args;
} INDEXED_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum;
typedef void (*FUNC_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data);
typedef struct {
    GLenum a1;
    GLintptr a2;
    GLsizeiptr a3;
    const GLvoid * a4;
} ARGS_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLuint)(GLuint list);
typedef struct {
    GLuint a1;
} ARGS_void_GLuint;
typedef struct {
    int format;
    FUNC_void_GLuint func;
    ARGS_void_GLuint args;
} PACKED_void_GLuint;
typedef struct {
    int func;
    ARGS_void_GLuint args;
} INDEXED_void_GLuint;
typedef void (*FUNC_void_GLsizei_GLenum_const_GLvoid___GENPT__)(GLsizei n, GLenum type, const GLvoid * lists);
typedef struct {
    GLsizei a1;
    GLenum a2;
    const GLvoid * a3;
} ARGS_void_GLsizei_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLsizei_GLenum_const_GLvoid___GENPT__ func;
    ARGS_void_GLsizei_GLenum_const_GLvoid___GENPT__ args;
} PACKED_void_GLsizei_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLsizei_GLenum_const_GLvoid___GENPT__ args;
} INDEXED_void_GLsizei_GLenum_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLbitfield)(GLbitfield mask);
typedef struct {
    GLbitfield a1;
} ARGS_void_GLbitfield;
typedef struct {
    int format;
    FUNC_void_GLbitfield func;
    ARGS_void_GLbitfield args;
} PACKED_void_GLbitfield;
typedef struct {
    int func;
    ARGS_void_GLbitfield args;
} INDEXED_void_GLbitfield;
typedef void (*FUNC_void_GLdouble)(GLdouble depth);
typedef struct {
    GLdouble a1;
} ARGS_void_GLdouble;
typedef struct {
    int format;
    FUNC_void_GLdouble func;
    ARGS_void_GLdouble args;
} PACKED_void_GLdouble;
typedef struct {
    int func;
    ARGS_void_GLdouble args;
} INDEXED_void_GLdouble;
typedef void (*FUNC_void_GLfloat)(GLfloat c);
typedef struct {
    GLfloat a1;
} ARGS_void_GLfloat;
typedef struct {
    int format;
    FUNC_void_GLfloat func;
    ARGS_void_GLfloat args;
} PACKED_void_GLfloat;
typedef struct {
    int func;
    ARGS_void_GLfloat args;
} INDEXED_void_GLfloat;
typedef void (*FUNC_void_GLenum_const_GLdouble___GENPT__)(GLenum plane, const GLdouble * equation);
typedef struct {
    GLenum a1;
    const GLdouble * a2;
} ARGS_void_GLenum_const_GLdouble___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_const_GLdouble___GENPT__ func;
    ARGS_void_GLenum_const_GLdouble___GENPT__ args;
} PACKED_void_GLenum_const_GLdouble___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_const_GLdouble___GENPT__ args;
} INDEXED_void_GLenum_const_GLdouble___GENPT__;
typedef void (*FUNC_void_GLbyte_GLbyte_GLbyte)(GLbyte red, GLbyte green, GLbyte blue);
typedef struct {
    GLbyte a1;
    GLbyte a2;
    GLbyte a3;
} ARGS_void_GLbyte_GLbyte_GLbyte;
typedef struct {
    int format;
    FUNC_void_GLbyte_GLbyte_GLbyte func;
    ARGS_void_GLbyte_GLbyte_GLbyte args;
} PACKED_void_GLbyte_GLbyte_GLbyte;
typedef struct {
    int func;
    ARGS_void_GLbyte_GLbyte_GLbyte args;
} INDEXED_void_GLbyte_GLbyte_GLbyte;
typedef void (*FUNC_void_const_GLbyte___GENPT__)(const GLbyte * v);
typedef struct {
    const GLbyte * a1;
} ARGS_void_const_GLbyte___GENPT__;
typedef struct {
    int format;
    FUNC_void_const_GLbyte___GENPT__ func;
    ARGS_void_const_GLbyte___GENPT__ args;
} PACKED_void_const_GLbyte___GENPT__;
typedef struct {
    int func;
    ARGS_void_const_GLbyte___GENPT__ args;
} INDEXED_void_const_GLbyte___GENPT__;
typedef void (*FUNC_void_GLdouble_GLdouble_GLdouble)(GLdouble red, GLdouble green, GLdouble blue);
typedef struct {
    GLdouble a1;
    GLdouble a2;
    GLdouble a3;
} ARGS_void_GLdouble_GLdouble_GLdouble;
typedef struct {
    int format;
    FUNC_void_GLdouble_GLdouble_GLdouble func;
    ARGS_void_GLdouble_GLdouble_GLdouble args;
} PACKED_void_GLdouble_GLdouble_GLdouble;
typedef struct {
    int func;
    ARGS_void_GLdouble_GLdouble_GLdouble args;
} INDEXED_void_GLdouble_GLdouble_GLdouble;
typedef void (*FUNC_void_const_GLdouble___GENPT__)(const GLdouble * v);
typedef struct {
    const GLdouble * a1;
} ARGS_void_const_GLdouble___GENPT__;
typedef struct {
    int format;
    FUNC_void_const_GLdouble___GENPT__ func;
    ARGS_void_const_GLdouble___GENPT__ args;
} PACKED_void_const_GLdouble___GENPT__;
typedef struct {
    int func;
    ARGS_void_const_GLdouble___GENPT__ args;
} INDEXED_void_const_GLdouble___GENPT__;
typedef void (*FUNC_void_GLfloat_GLfloat_GLfloat)(GLfloat red, GLfloat green, GLfloat blue);
typedef struct {
    GLfloat a1;
    GLfloat a2;
    GLfloat a3;
} ARGS_void_GLfloat_GLfloat_GLfloat;
typedef struct {
    int format;
    FUNC_void_GLfloat_GLfloat_GLfloat func;
    ARGS_void_GLfloat_GLfloat_GLfloat args;
} PACKED_void_GLfloat_GLfloat_GLfloat;
typedef struct {
    int func;
    ARGS_void_GLfloat_GLfloat_GLfloat args;
} INDEXED_void_GLfloat_GLfloat_GLfloat;
typedef void (*FUNC_void_const_GLfloat___GENPT__)(const GLfloat * v);
typedef struct {
    const GLfloat * a1;
} ARGS_void_const_GLfloat___GENPT__;
typedef struct {
    int format;
    FUNC_void_const_GLfloat___GENPT__ func;
    ARGS_void_const_GLfloat___GENPT__ args;
} PACKED_void_const_GLfloat___GENPT__;
typedef struct {
    int func;
    ARGS_void_const_GLfloat___GENPT__ args;
} INDEXED_void_const_GLfloat___GENPT__;
typedef void (*FUNC_void_GLint_GLint_GLint)(GLint red, GLint green, GLint blue);
typedef struct {
    GLint a1;
    GLint a2;
    GLint a3;
} ARGS_void_GLint_GLint_GLint;
typedef struct {
    int format;
    FUNC_void_GLint_GLint_GLint func;
    ARGS_void_GLint_GLint_GLint args;
} PACKED_void_GLint_GLint_GLint;
typedef struct {
    int func;
    ARGS_void_GLint_GLint_GLint args;
} INDEXED_void_GLint_GLint_GLint;
typedef void (*FUNC_void_const_GLint___GENPT__)(const GLint * v);
typedef struct {
    const GLint * a1;
} ARGS_void_const_GLint___GENPT__;
typedef struct {
    int format;
    FUNC_void_const_GLint___GENPT__ func;
    ARGS_void_const_GLint___GENPT__ args;
} PACKED_void_const_GLint___GENPT__;
typedef struct {
    int func;
    ARGS_void_const_GLint___GENPT__ args;
} INDEXED_void_const_GLint___GENPT__;
typedef void (*FUNC_void_GLshort_GLshort_GLshort)(GLshort red, GLshort green, GLshort blue);
typedef struct {
    GLshort a1;
    GLshort a2;
    GLshort a3;
} ARGS_void_GLshort_GLshort_GLshort;
typedef struct {
    int format;
    FUNC_void_GLshort_GLshort_GLshort func;
    ARGS_void_GLshort_GLshort_GLshort args;
} PACKED_void_GLshort_GLshort_GLshort;
typedef struct {
    int func;
    ARGS_void_GLshort_GLshort_GLshort args;
} INDEXED_void_GLshort_GLshort_GLshort;
typedef void (*FUNC_void_const_GLshort___GENPT__)(const GLshort * v);
typedef struct {
    const GLshort * a1;
} ARGS_void_const_GLshort___GENPT__;
typedef struct {
    int format;
    FUNC_void_const_GLshort___GENPT__ func;
    ARGS_void_const_GLshort___GENPT__ args;
} PACKED_void_const_GLshort___GENPT__;
typedef struct {
    int func;
    ARGS_void_const_GLshort___GENPT__ args;
} INDEXED_void_const_GLshort___GENPT__;
typedef void (*FUNC_void_GLubyte_GLubyte_GLubyte)(GLubyte red, GLubyte green, GLubyte blue);
typedef struct {
    GLubyte a1;
    GLubyte a2;
    GLubyte a3;
} ARGS_void_GLubyte_GLubyte_GLubyte;
typedef struct {
    int format;
    FUNC_void_GLubyte_GLubyte_GLubyte func;
    ARGS_void_GLubyte_GLubyte_GLubyte args;
} PACKED_void_GLubyte_GLubyte_GLubyte;
typedef struct {
    int func;
    ARGS_void_GLubyte_GLubyte_GLubyte args;
} INDEXED_void_GLubyte_GLubyte_GLubyte;
typedef void (*FUNC_void_const_GLubyte___GENPT__)(const GLubyte * v);
typedef struct {
    const GLubyte * a1;
} ARGS_void_const_GLubyte___GENPT__;
typedef struct {
    int format;
    FUNC_void_const_GLubyte___GENPT__ func;
    ARGS_void_const_GLubyte___GENPT__ args;
} PACKED_void_const_GLubyte___GENPT__;
typedef struct {
    int func;
    ARGS_void_const_GLubyte___GENPT__ args;
} INDEXED_void_const_GLubyte___GENPT__;
typedef void (*FUNC_void_GLuint_GLuint_GLuint)(GLuint red, GLuint green, GLuint blue);
typedef struct {
    GLuint a1;
    GLuint a2;
    GLuint a3;
} ARGS_void_GLuint_GLuint_GLuint;
typedef struct {
    int format;
    FUNC_void_GLuint_GLuint_GLuint func;
    ARGS_void_GLuint_GLuint_GLuint args;
} PACKED_void_GLuint_GLuint_GLuint;
typedef struct {
    int func;
    ARGS_void_GLuint_GLuint_GLuint args;
} INDEXED_void_GLuint_GLuint_GLuint;
typedef void (*FUNC_void_const_GLuint___GENPT__)(const GLuint * v);
typedef struct {
    const GLuint * a1;
} ARGS_void_const_GLuint___GENPT__;
typedef struct {
    int format;
    FUNC_void_const_GLuint___GENPT__ func;
    ARGS_void_const_GLuint___GENPT__ args;
} PACKED_void_const_GLuint___GENPT__;
typedef struct {
    int func;
    ARGS_void_const_GLuint___GENPT__ args;
} INDEXED_void_const_GLuint___GENPT__;
typedef void (*FUNC_void_GLushort_GLushort_GLushort)(GLushort red, GLushort green, GLushort blue);
typedef struct {
    GLushort a1;
    GLushort a2;
    GLushort a3;
} ARGS_void_GLushort_GLushort_GLushort;
typedef struct {
    int format;
    FUNC_void_GLushort_GLushort_GLushort func;
    ARGS_void_GLushort_GLushort_GLushort args;
} PACKED_void_GLushort_GLushort_GLushort;
typedef struct {
    int func;
    ARGS_void_GLushort_GLushort_GLushort args;
} INDEXED_void_GLushort_GLushort_GLushort;
typedef void (*FUNC_void_const_GLushort___GENPT__)(const GLushort * v);
typedef struct {
    const GLushort * a1;
} ARGS_void_const_GLushort___GENPT__;
typedef struct {
    int format;
    FUNC_void_const_GLushort___GENPT__ func;
    ARGS_void_const_GLushort___GENPT__ args;
} PACKED_void_const_GLushort___GENPT__;
typedef struct {
    int func;
    ARGS_void_const_GLushort___GENPT__ args;
} INDEXED_void_const_GLushort___GENPT__;
typedef void (*FUNC_void_GLbyte_GLbyte_GLbyte_GLbyte)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
typedef struct {
    GLbyte a1;
    GLbyte a2;
    GLbyte a3;
    GLbyte a4;
} ARGS_void_GLbyte_GLbyte_GLbyte_GLbyte;
typedef struct {
    int format;
    FUNC_void_GLbyte_GLbyte_GLbyte_GLbyte func;
    ARGS_void_GLbyte_GLbyte_GLbyte_GLbyte args;
} PACKED_void_GLbyte_GLbyte_GLbyte_GLbyte;
typedef struct {
    int func;
    ARGS_void_GLbyte_GLbyte_GLbyte_GLbyte args;
} INDEXED_void_GLbyte_GLbyte_GLbyte_GLbyte;
typedef void (*FUNC_void_GLdouble_GLdouble_GLdouble_GLdouble)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
typedef struct {
    GLdouble a1;
    GLdouble a2;
    GLdouble a3;
    GLdouble a4;
} ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble;
typedef struct {
    int format;
    FUNC_void_GLdouble_GLdouble_GLdouble_GLdouble func;
    ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble args;
} PACKED_void_GLdouble_GLdouble_GLdouble_GLdouble;
typedef struct {
    int func;
    ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble args;
} INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble;
typedef void (*FUNC_void_GLint_GLint_GLint_GLint)(GLint red, GLint green, GLint blue, GLint alpha);
typedef struct {
    GLint a1;
    GLint a2;
    GLint a3;
    GLint a4;
} ARGS_void_GLint_GLint_GLint_GLint;
typedef struct {
    int format;
    FUNC_void_GLint_GLint_GLint_GLint func;
    ARGS_void_GLint_GLint_GLint_GLint args;
} PACKED_void_GLint_GLint_GLint_GLint;
typedef struct {
    int func;
    ARGS_void_GLint_GLint_GLint_GLint args;
} INDEXED_void_GLint_GLint_GLint_GLint;
typedef void (*FUNC_void_GLshort_GLshort_GLshort_GLshort)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
typedef struct {
    GLshort a1;
    GLshort a2;
    GLshort a3;
    GLshort a4;
} ARGS_void_GLshort_GLshort_GLshort_GLshort;
typedef struct {
    int format;
    FUNC_void_GLshort_GLshort_GLshort_GLshort func;
    ARGS_void_GLshort_GLshort_GLshort_GLshort args;
} PACKED_void_GLshort_GLshort_GLshort_GLshort;
typedef struct {
    int func;
    ARGS_void_GLshort_GLshort_GLshort_GLshort args;
} INDEXED_void_GLshort_GLshort_GLshort_GLshort;
typedef void (*FUNC_void_GLubyte_GLubyte_GLubyte_GLubyte)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
typedef struct {
    GLubyte a1;
    GLubyte a2;
    GLubyte a3;
    GLubyte a4;
} ARGS_void_GLubyte_GLubyte_GLubyte_GLubyte;
typedef struct {
    int format;
    FUNC_void_GLubyte_GLubyte_GLubyte_GLubyte func;
    ARGS_void_GLubyte_GLubyte_GLubyte_GLubyte args;
} PACKED_void_GLubyte_GLubyte_GLubyte_GLubyte;
typedef struct {
    int func;
    ARGS_void_GLubyte_GLubyte_GLubyte_GLubyte args;
} INDEXED_void_GLubyte_GLubyte_GLubyte_GLubyte;
typedef void (*FUNC_void_GLuint_GLuint_GLuint_GLuint)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
typedef struct {
    GLuint a1;
    GLuint a2;
    GLuint a3;
    GLuint a4;
} ARGS_void_GLuint_GLuint_GLuint_GLuint;
typedef struct {
    int format;
    FUNC_void_GLuint_GLuint_GLuint_GLuint func;
    ARGS_void_GLuint_GLuint_GLuint_GLuint args;
} PACKED_void_GLuint_GLuint_GLuint_GLuint;
typedef struct {
    int func;
    ARGS_void_GLuint_GLuint_GLuint_GLuint args;
} INDEXED_void_GLuint_GLuint_GLuint_GLuint;
typedef void (*FUNC_void_GLushort_GLushort_GLushort_GLushort)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
typedef struct {
    GLushort a1;
    GLushort a2;
    GLushort a3;
    GLushort a4;
} ARGS_void_GLushort_GLushort_GLushort_GLushort;
typedef struct {
    int format;
    FUNC_void_GLushort_GLushort_GLushort_GLushort func;
    ARGS_void_GLushort_GLushort_GLushort_GLushort args;
} PACKED_void_GLushort_GLushort_GLushort_GLushort;
typedef struct {
    int func;
    ARGS_void_GLushort_GLushort_GLushort_GLushort args;
} INDEXED_void_GLushort_GLushort_GLushort_GLushort;
typedef void (*FUNC_void_GLboolean_GLboolean_GLboolean_GLboolean)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef struct {
    GLboolean a1;
    GLboolean a2;
    GLboolean a3;
    GLboolean a4;
} ARGS_void_GLboolean_GLboolean_GLboolean_GLboolean;
typedef struct {
    int format;
    FUNC_void_GLboolean_GLboolean_GLboolean_GLboolean func;
    ARGS_void_GLboolean_GLboolean_GLboolean_GLboolean args;
} PACKED_void_GLboolean_GLboolean_GLboolean_GLboolean;
typedef struct {
    int func;
    ARGS_void_GLboolean_GLboolean_GLboolean_GLboolean args;
} INDEXED_void_GLboolean_GLboolean_GLboolean_GLboolean;
typedef void (*FUNC_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__)(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);
typedef struct {
    GLint a1;
    GLenum a2;
    GLsizei a3;
    const GLvoid * a4;
} ARGS_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ func;
    ARGS_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ args;
} PACKED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ args;
} INDEXED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__)(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid * data);
typedef struct {
    GLenum a1;
    GLsizei a2;
    GLsizei a3;
    GLenum a4;
    GLenum a5;
    const GLvoid * a6;
} ARGS_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * table);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLsizei a3;
    GLenum a4;
    GLenum a5;
    const GLvoid * a6;
} ARGS_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLenum_const_GLfloat___GENPT__)(GLenum target, GLenum pname, const GLfloat * params);
typedef struct {
    GLenum a1;
    GLenum a2;
    const GLfloat * a3;
} ARGS_void_GLenum_GLenum_const_GLfloat___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_const_GLfloat___GENPT__ func;
    ARGS_void_GLenum_GLenum_const_GLfloat___GENPT__ args;
} PACKED_void_GLenum_GLenum_const_GLfloat___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_const_GLfloat___GENPT__ args;
} INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__;
typedef void (*FUNC_void_GLenum_GLenum_const_GLint___GENPT__)(GLenum target, GLenum pname, const GLint * params);
typedef struct {
    GLenum a1;
    GLenum a2;
    const GLint * a3;
} ARGS_void_GLenum_GLenum_const_GLint___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_const_GLint___GENPT__ func;
    ARGS_void_GLenum_GLenum_const_GLint___GENPT__ args;
} PACKED_void_GLenum_GLenum_const_GLint___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_const_GLint___GENPT__ args;
} INDEXED_void_GLenum_GLenum_const_GLint___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid * data);
typedef struct {
    GLenum a1;
    GLint a2;
    GLenum a3;
    GLsizei a4;
    GLint a5;
    GLsizei a6;
    const GLvoid * a7;
} ARGS_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data);
typedef struct {
    GLenum a1;
    GLint a2;
    GLenum a3;
    GLsizei a4;
    GLsizei a5;
    GLint a6;
    GLsizei a7;
    const GLvoid * a8;
} ARGS_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data);
typedef struct {
    GLenum a1;
    GLint a2;
    GLenum a3;
    GLsizei a4;
    GLsizei a5;
    GLsizei a6;
    GLint a7;
    GLsizei a8;
    const GLvoid * a9;
} ARGS_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLsizei a4;
    GLenum a5;
    GLsizei a6;
    const GLvoid * a7;
} ARGS_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLint a4;
    GLsizei a5;
    GLsizei a6;
    GLenum a7;
    GLsizei a8;
    const GLvoid * a9;
} ARGS_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLint a4;
    GLint a5;
    GLsizei a6;
    GLsizei a7;
    GLsizei a8;
    GLenum a9;
    GLsizei a10;
    const GLvoid * a11;
} ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * image);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLsizei a3;
    GLsizei a4;
    GLenum a5;
    GLenum a6;
    const GLvoid * a7;
} ARGS_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLenum_GLfloat)(GLenum target, GLenum pname, GLfloat params);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLfloat a3;
} ARGS_void_GLenum_GLenum_GLfloat;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLfloat func;
    ARGS_void_GLenum_GLenum_GLfloat args;
} PACKED_void_GLenum_GLenum_GLfloat;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLfloat args;
} INDEXED_void_GLenum_GLenum_GLfloat;
typedef void (*FUNC_void_GLenum_GLenum_GLint)(GLenum target, GLenum pname, GLint params);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLint a3;
} ARGS_void_GLenum_GLenum_GLint;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLint func;
    ARGS_void_GLenum_GLenum_GLint args;
} PACKED_void_GLenum_GLenum_GLint;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLint args;
} INDEXED_void_GLenum_GLenum_GLint;
typedef void (*FUNC_void_GLenum_GLsizei_GLint_GLint_GLsizei)(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef struct {
    GLenum a1;
    GLsizei a2;
    GLint a3;
    GLint a4;
    GLsizei a5;
} ARGS_void_GLenum_GLsizei_GLint_GLint_GLsizei;
typedef struct {
    int format;
    FUNC_void_GLenum_GLsizei_GLint_GLint_GLsizei func;
    ARGS_void_GLenum_GLsizei_GLint_GLint_GLsizei args;
} PACKED_void_GLenum_GLsizei_GLint_GLint_GLsizei;
typedef struct {
    int func;
    ARGS_void_GLenum_GLsizei_GLint_GLint_GLsizei args;
} INDEXED_void_GLenum_GLsizei_GLint_GLint_GLsizei;
typedef void (*FUNC_void_GLenum_GLenum_GLint_GLint_GLsizei)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLint a3;
    GLint a4;
    GLsizei a5;
} ARGS_void_GLenum_GLenum_GLint_GLint_GLsizei;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLint_GLint_GLsizei func;
    ARGS_void_GLenum_GLenum_GLint_GLint_GLsizei args;
} PACKED_void_GLenum_GLenum_GLint_GLint_GLsizei;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLint_GLint_GLsizei args;
} INDEXED_void_GLenum_GLenum_GLint_GLint_GLsizei;
typedef void (*FUNC_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLint a3;
    GLint a4;
    GLsizei a5;
    GLsizei a6;
} ARGS_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei func;
    ARGS_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei args;
} PACKED_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei args;
} INDEXED_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei;
typedef void (*FUNC_void_GLint_GLint_GLsizei_GLsizei_GLenum)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
typedef struct {
    GLint a1;
    GLint a2;
    GLsizei a3;
    GLsizei a4;
    GLenum a5;
} ARGS_void_GLint_GLint_GLsizei_GLsizei_GLenum;
typedef struct {
    int format;
    FUNC_void_GLint_GLint_GLsizei_GLsizei_GLenum func;
    ARGS_void_GLint_GLint_GLsizei_GLsizei_GLenum args;
} PACKED_void_GLint_GLint_GLsizei_GLsizei_GLenum;
typedef struct {
    int func;
    ARGS_void_GLint_GLint_GLsizei_GLsizei_GLenum args;
} INDEXED_void_GLint_GLint_GLsizei_GLsizei_GLenum;
typedef void (*FUNC_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef struct {
    GLenum a1;
    GLint a2;
    GLenum a3;
    GLint a4;
    GLint a5;
    GLsizei a6;
    GLint a7;
} ARGS_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint func;
    ARGS_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint args;
} PACKED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint args;
} INDEXED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint;
typedef void (*FUNC_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef struct {
    GLenum a1;
    GLint a2;
    GLenum a3;
    GLint a4;
    GLint a5;
    GLsizei a6;
    GLsizei a7;
    GLint a8;
} ARGS_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint func;
    ARGS_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint args;
} PACKED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint args;
} INDEXED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLint_GLint_GLsizei)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLint a4;
    GLint a5;
    GLsizei a6;
} ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLsizei;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLint_GLint_GLsizei func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLsizei args;
} PACKED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLsizei args;
} INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLint a4;
    GLint a5;
    GLint a6;
    GLsizei a7;
    GLsizei a8;
} ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei args;
} PACKED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei args;
} INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLint a4;
    GLint a5;
    GLint a6;
    GLint a7;
    GLsizei a8;
    GLsizei a9;
} ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei args;
} PACKED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei args;
} INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei;
typedef void (*FUNC_void_GLsizei_const_GLuint___GENPT__)(GLsizei n, const GLuint * buffers);
typedef struct {
    GLsizei a1;
    const GLuint * a2;
} ARGS_void_GLsizei_const_GLuint___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLsizei_const_GLuint___GENPT__ func;
    ARGS_void_GLsizei_const_GLuint___GENPT__ args;
} PACKED_void_GLsizei_const_GLuint___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLsizei_const_GLuint___GENPT__ args;
} INDEXED_void_GLsizei_const_GLuint___GENPT__;
typedef void (*FUNC_void_GLuint_GLsizei)(GLuint list, GLsizei range);
typedef struct {
    GLuint a1;
    GLsizei a2;
} ARGS_void_GLuint_GLsizei;
typedef struct {
    int format;
    FUNC_void_GLuint_GLsizei func;
    ARGS_void_GLuint_GLsizei args;
} PACKED_void_GLuint_GLsizei;
typedef struct {
    int func;
    ARGS_void_GLuint_GLsizei args;
} INDEXED_void_GLuint_GLsizei;
typedef void (*FUNC_void_GLboolean)(GLboolean flag);
typedef struct {
    GLboolean a1;
} ARGS_void_GLboolean;
typedef struct {
    int format;
    FUNC_void_GLboolean func;
    ARGS_void_GLboolean args;
} PACKED_void_GLboolean;
typedef struct {
    int func;
    ARGS_void_GLboolean args;
} INDEXED_void_GLboolean;
typedef void (*FUNC_void_GLdouble_GLdouble)(GLdouble near, GLdouble far);
typedef struct {
    GLdouble a1;
    GLdouble a2;
} ARGS_void_GLdouble_GLdouble;
typedef struct {
    int format;
    FUNC_void_GLdouble_GLdouble func;
    ARGS_void_GLdouble_GLdouble args;
} PACKED_void_GLdouble_GLdouble;
typedef struct {
    int func;
    ARGS_void_GLdouble_GLdouble args;
} INDEXED_void_GLdouble_GLdouble;
typedef void (*FUNC_void_GLenum_GLint_GLsizei)(GLenum mode, GLint first, GLsizei count);
typedef struct {
    GLenum a1;
    GLint a2;
    GLsizei a3;
} ARGS_void_GLenum_GLint_GLsizei;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLsizei func;
    ARGS_void_GLenum_GLint_GLsizei args;
} PACKED_void_GLenum_GLint_GLsizei;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLsizei args;
} INDEXED_void_GLenum_GLint_GLsizei;
typedef void (*FUNC_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
typedef struct {
    GLenum a1;
    GLsizei a2;
    GLenum a3;
    const GLvoid * a4;
} ARGS_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);
typedef struct {
    GLsizei a1;
    GLsizei a2;
    GLenum a3;
    GLenum a4;
    const GLvoid * a5;
} ARGS_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ func;
    ARGS_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} PACKED_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} INDEXED_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices);
typedef struct {
    GLenum a1;
    GLuint a2;
    GLuint a3;
    GLsizei a4;
    GLenum a5;
    const GLvoid * a6;
} ARGS_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLsizei_const_GLvoid___GENPT__)(GLsizei stride, const GLvoid * pointer);
typedef struct {
    GLsizei a1;
    const GLvoid * a2;
} ARGS_void_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLsizei_const_GLvoid___GENPT__ func;
    ARGS_void_GLsizei_const_GLvoid___GENPT__ args;
} PACKED_void_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLsizei_const_GLvoid___GENPT__ args;
} INDEXED_void_GLsizei_const_GLvoid___GENPT__;
typedef void (*FUNC_void_const_GLboolean___GENPT__)(const GLboolean * flag);
typedef struct {
    const GLboolean * a1;
} ARGS_void_const_GLboolean___GENPT__;
typedef struct {
    int format;
    FUNC_void_const_GLboolean___GENPT__ func;
    ARGS_void_const_GLboolean___GENPT__ args;
} PACKED_void_const_GLboolean___GENPT__;
typedef struct {
    int func;
    ARGS_void_const_GLboolean___GENPT__ args;
} INDEXED_void_const_GLboolean___GENPT__;
typedef void (*FUNC_void)();
typedef struct {
} ARGS_void;
typedef struct {
    int format;
    FUNC_void func;
    ARGS_void args;
} PACKED_void;
typedef struct {
    int func;
    ARGS_void args;
} INDEXED_void;
typedef void (*FUNC_void_GLfloat_GLfloat)(GLfloat u, GLfloat v);
typedef struct {
    GLfloat a1;
    GLfloat a2;
} ARGS_void_GLfloat_GLfloat;
typedef struct {
    int format;
    FUNC_void_GLfloat_GLfloat func;
    ARGS_void_GLfloat_GLfloat args;
} PACKED_void_GLfloat_GLfloat;
typedef struct {
    int func;
    ARGS_void_GLfloat_GLfloat args;
} INDEXED_void_GLfloat_GLfloat;
typedef void (*FUNC_void_GLenum_GLint_GLint)(GLenum mode, GLint i1, GLint i2);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
} ARGS_void_GLenum_GLint_GLint;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint func;
    ARGS_void_GLenum_GLint_GLint args;
} PACKED_void_GLenum_GLint_GLint;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint args;
} INDEXED_void_GLenum_GLint_GLint;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLint_GLint)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLint a4;
    GLint a5;
} ARGS_void_GLenum_GLint_GLint_GLint_GLint;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLint_GLint func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLint args;
} PACKED_void_GLenum_GLint_GLint_GLint_GLint;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLint args;
} INDEXED_void_GLenum_GLint_GLint_GLint_GLint;
typedef void (*FUNC_void_GLint_GLint)(GLint i, GLint j);
typedef struct {
    GLint a1;
    GLint a2;
} ARGS_void_GLint_GLint;
typedef struct {
    int format;
    FUNC_void_GLint_GLint func;
    ARGS_void_GLint_GLint args;
} PACKED_void_GLint_GLint;
typedef struct {
    int func;
    ARGS_void_GLint_GLint args;
} INDEXED_void_GLint_GLint;
typedef void (*FUNC_void_GLsizei_GLenum_GLfloat___GENPT__)(GLsizei size, GLenum type, GLfloat * buffer);
typedef struct {
    GLsizei a1;
    GLenum a2;
    GLfloat * a3;
} ARGS_void_GLsizei_GLenum_GLfloat___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLsizei_GLenum_GLfloat___GENPT__ func;
    ARGS_void_GLsizei_GLenum_GLfloat___GENPT__ args;
} PACKED_void_GLsizei_GLenum_GLfloat___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLsizei_GLenum_GLfloat___GENPT__ args;
} INDEXED_void_GLsizei_GLenum_GLfloat___GENPT__;
typedef void (*FUNC_void_GLenum_GLsizei_const_GLvoid___GENPT__)(GLenum type, GLsizei stride, const GLvoid * pointer);
typedef struct {
    GLenum a1;
    GLsizei a2;
    const GLvoid * a3;
} ARGS_void_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLsizei_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLsizei_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLsizei_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLsizei_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_const_GLfloat___GENPT__)(GLenum pname, const GLfloat * params);
typedef struct {
    GLenum a1;
    const GLfloat * a2;
} ARGS_void_GLenum_const_GLfloat___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_const_GLfloat___GENPT__ func;
    ARGS_void_GLenum_const_GLfloat___GENPT__ args;
} PACKED_void_GLenum_const_GLfloat___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_const_GLfloat___GENPT__ args;
} INDEXED_void_GLenum_const_GLfloat___GENPT__;
typedef void (*FUNC_void_GLenum_GLint)(GLenum pname, GLint param);
typedef struct {
    GLenum a1;
    GLint a2;
} ARGS_void_GLenum_GLint;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint func;
    ARGS_void_GLenum_GLint args;
} PACKED_void_GLenum_GLint;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint args;
} INDEXED_void_GLenum_GLint;
typedef void (*FUNC_void_GLenum_const_GLint___GENPT__)(GLenum pname, const GLint * params);
typedef struct {
    GLenum a1;
    const GLint * a2;
} ARGS_void_GLenum_const_GLint___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_const_GLint___GENPT__ func;
    ARGS_void_GLenum_const_GLint___GENPT__ args;
} PACKED_void_GLenum_const_GLint___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_const_GLint___GENPT__ args;
} INDEXED_void_GLenum_const_GLint___GENPT__;
typedef void (*FUNC_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
typedef struct {
    GLdouble a1;
    GLdouble a2;
    GLdouble a3;
    GLdouble a4;
    GLdouble a5;
    GLdouble a6;
} ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble;
typedef struct {
    int format;
    FUNC_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble func;
    ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble args;
} PACKED_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble;
typedef struct {
    int func;
    ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble args;
} INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble;
typedef void (*FUNC_void_GLsizei_GLuint___GENPT__)(GLsizei n, GLuint * buffers);
typedef struct {
    GLsizei a1;
    GLuint * a2;
} ARGS_void_GLsizei_GLuint___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLsizei_GLuint___GENPT__ func;
    ARGS_void_GLsizei_GLuint___GENPT__ args;
} PACKED_void_GLsizei_GLuint___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLsizei_GLuint___GENPT__ args;
} INDEXED_void_GLsizei_GLuint___GENPT__;
typedef GLuint (*FUNC_GLuint_GLsizei)(GLsizei range);
typedef struct {
    GLsizei a1;
} ARGS_GLuint_GLsizei;
typedef struct {
    int format;
    FUNC_GLuint_GLsizei func;
    ARGS_GLuint_GLsizei args;
} PACKED_GLuint_GLsizei;
typedef struct {
    int func;
    ARGS_GLuint_GLsizei args;
} INDEXED_GLuint_GLsizei;
typedef void (*FUNC_void_GLenum_GLboolean___GENPT__)(GLenum pname, GLboolean * params);
typedef struct {
    GLenum a1;
    GLboolean * a2;
} ARGS_void_GLenum_GLboolean___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLboolean___GENPT__ func;
    ARGS_void_GLenum_GLboolean___GENPT__ args;
} PACKED_void_GLenum_GLboolean___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLboolean___GENPT__ args;
} INDEXED_void_GLenum_GLboolean___GENPT__;
typedef void (*FUNC_void_GLenum_GLenum_GLint___GENPT__)(GLenum target, GLenum pname, GLint * params);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLint * a3;
} ARGS_void_GLenum_GLenum_GLint___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLint___GENPT__ func;
    ARGS_void_GLenum_GLenum_GLint___GENPT__ args;
} PACKED_void_GLenum_GLenum_GLint___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLint___GENPT__ args;
} INDEXED_void_GLenum_GLenum_GLint___GENPT__;
typedef void (*FUNC_void_GLenum_GLenum_GLvoid___GENPT__)(GLenum target, GLenum pname, GLvoid * params);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLvoid * a3;
} ARGS_void_GLenum_GLenum_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLenum_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLenum_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLenum_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__)(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid * data);
typedef struct {
    GLenum a1;
    GLintptr a2;
    GLsizeiptr a3;
    GLvoid * a4;
} ARGS_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLdouble___GENPT__)(GLenum plane, GLdouble * equation);
typedef struct {
    GLenum a1;
    GLdouble * a2;
} ARGS_void_GLenum_GLdouble___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLdouble___GENPT__ func;
    ARGS_void_GLenum_GLdouble___GENPT__ args;
} PACKED_void_GLenum_GLdouble___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLdouble___GENPT__ args;
} INDEXED_void_GLenum_GLdouble___GENPT__;
typedef void (*FUNC_void_GLenum_GLenum_GLenum_GLvoid___GENPT__)(GLenum target, GLenum format, GLenum type, GLvoid * table);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLenum a3;
    GLvoid * a4;
} ARGS_void_GLenum_GLenum_GLenum_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLenum_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLenum_GLenum_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLenum_GLenum_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLenum_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLenum_GLenum_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLenum_GLfloat___GENPT__)(GLenum target, GLenum pname, GLfloat * params);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLfloat * a3;
} ARGS_void_GLenum_GLenum_GLfloat___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLfloat___GENPT__ func;
    ARGS_void_GLenum_GLenum_GLfloat___GENPT__ args;
} PACKED_void_GLenum_GLenum_GLfloat___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLfloat___GENPT__ args;
} INDEXED_void_GLenum_GLenum_GLfloat___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLvoid___GENPT__)(GLenum target, GLint level, GLvoid * img);
typedef struct {
    GLenum a1;
    GLint a2;
    GLvoid * a3;
} ARGS_void_GLenum_GLint_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLvoid___GENPT__;
typedef GLenum (*FUNC_GLenum)();
typedef struct {
} ARGS_GLenum;
typedef struct {
    int format;
    FUNC_GLenum func;
    ARGS_GLenum args;
} PACKED_GLenum;
typedef struct {
    int func;
    ARGS_GLenum args;
} INDEXED_GLenum;
typedef void (*FUNC_void_GLenum_GLfloat___GENPT__)(GLenum pname, GLfloat * params);
typedef struct {
    GLenum a1;
    GLfloat * a2;
} ARGS_void_GLenum_GLfloat___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLfloat___GENPT__ func;
    ARGS_void_GLenum_GLfloat___GENPT__ args;
} PACKED_void_GLenum_GLfloat___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLfloat___GENPT__ args;
} INDEXED_void_GLenum_GLfloat___GENPT__;
typedef void (*FUNC_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values);
typedef struct {
    GLenum a1;
    GLboolean a2;
    GLenum a3;
    GLenum a4;
    GLvoid * a5;
} ARGS_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLint___GENPT__)(GLenum pname, GLint * params);
typedef struct {
    GLenum a1;
    GLint * a2;
} ARGS_void_GLenum_GLint___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint___GENPT__ func;
    ARGS_void_GLenum_GLint___GENPT__ args;
} PACKED_void_GLenum_GLint___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint___GENPT__ args;
} INDEXED_void_GLenum_GLint___GENPT__;
typedef void (*FUNC_void_GLenum_GLenum_GLdouble___GENPT__)(GLenum target, GLenum query, GLdouble * v);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLdouble * a3;
} ARGS_void_GLenum_GLenum_GLdouble___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLdouble___GENPT__ func;
    ARGS_void_GLenum_GLenum_GLdouble___GENPT__ args;
} PACKED_void_GLenum_GLenum_GLdouble___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLdouble___GENPT__ args;
} INDEXED_void_GLenum_GLenum_GLdouble___GENPT__;
typedef void (*FUNC_void_GLenum_GLuint___GENPT__)(GLenum map, GLuint * values);
typedef struct {
    GLenum a1;
    GLuint * a2;
} ARGS_void_GLenum_GLuint___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLuint___GENPT__ func;
    ARGS_void_GLenum_GLuint___GENPT__ args;
} PACKED_void_GLenum_GLuint___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLuint___GENPT__ args;
} INDEXED_void_GLenum_GLuint___GENPT__;
typedef void (*FUNC_void_GLenum_GLushort___GENPT__)(GLenum map, GLushort * values);
typedef struct {
    GLenum a1;
    GLushort * a2;
} ARGS_void_GLenum_GLushort___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLushort___GENPT__ func;
    ARGS_void_GLenum_GLushort___GENPT__ args;
} PACKED_void_GLenum_GLushort___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLushort___GENPT__ args;
} INDEXED_void_GLenum_GLushort___GENPT__;
typedef void (*FUNC_void_GLenum_GLvoid___GENPT____GENPT__)(GLenum pname, GLvoid ** params);
typedef struct {
    GLenum a1;
    GLvoid ** a2;
} ARGS_void_GLenum_GLvoid___GENPT____GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLvoid___GENPT____GENPT__ func;
    ARGS_void_GLenum_GLvoid___GENPT____GENPT__ args;
} PACKED_void_GLenum_GLvoid___GENPT____GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLvoid___GENPT____GENPT__ args;
} INDEXED_void_GLenum_GLvoid___GENPT____GENPT__;
typedef void (*FUNC_void_GLubyte___GENPT__)(GLubyte * mask);
typedef struct {
    GLubyte * a1;
} ARGS_void_GLubyte___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLubyte___GENPT__ func;
    ARGS_void_GLubyte___GENPT__ args;
} PACKED_void_GLubyte___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLubyte___GENPT__ args;
} INDEXED_void_GLubyte___GENPT__;
typedef void (*FUNC_void_GLuint_GLenum_GLint___GENPT__)(GLuint id, GLenum pname, GLint * params);
typedef struct {
    GLuint a1;
    GLenum a2;
    GLint * a3;
} ARGS_void_GLuint_GLenum_GLint___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLuint_GLenum_GLint___GENPT__ func;
    ARGS_void_GLuint_GLenum_GLint___GENPT__ args;
} PACKED_void_GLuint_GLenum_GLint___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLuint_GLenum_GLint___GENPT__ args;
} INDEXED_void_GLuint_GLenum_GLint___GENPT__;
typedef void (*FUNC_void_GLuint_GLenum_GLuint___GENPT__)(GLuint id, GLenum pname, GLuint * params);
typedef struct {
    GLuint a1;
    GLenum a2;
    GLuint * a3;
} ARGS_void_GLuint_GLenum_GLuint___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLuint_GLenum_GLuint___GENPT__ func;
    ARGS_void_GLuint_GLenum_GLuint___GENPT__ args;
} PACKED_void_GLuint_GLenum_GLuint___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLuint_GLenum_GLuint___GENPT__ args;
} INDEXED_void_GLuint_GLenum_GLuint___GENPT__;
typedef void (*FUNC_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__)(GLenum target, GLenum format, GLenum type, GLvoid * row, GLvoid * column, GLvoid * span);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLenum a3;
    GLvoid * a4;
    GLvoid * a5;
    GLvoid * a6;
} ARGS_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__;
typedef const GLubyte * (*FUNC_const_GLubyte___GENPT___GLenum)(GLenum name);
typedef struct {
    GLenum a1;
} ARGS_const_GLubyte___GENPT___GLenum;
typedef struct {
    int format;
    FUNC_const_GLubyte___GENPT___GLenum func;
    ARGS_const_GLubyte___GENPT___GLenum args;
} PACKED_const_GLubyte___GENPT___GLenum;
typedef struct {
    int func;
    ARGS_const_GLubyte___GENPT___GLenum args;
} INDEXED_const_GLubyte___GENPT___GLenum;
typedef void (*FUNC_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels);
typedef struct {
    GLenum a1;
    GLint a2;
    GLenum a3;
    GLenum a4;
    GLvoid * a5;
} ARGS_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLenum_GLfloat___GENPT__)(GLenum target, GLint level, GLenum pname, GLfloat * params);
typedef struct {
    GLenum a1;
    GLint a2;
    GLenum a3;
    GLfloat * a4;
} ARGS_void_GLenum_GLint_GLenum_GLfloat___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLenum_GLfloat___GENPT__ func;
    ARGS_void_GLenum_GLint_GLenum_GLfloat___GENPT__ args;
} PACKED_void_GLenum_GLint_GLenum_GLfloat___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLenum_GLfloat___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLenum_GLfloat___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLenum_GLint___GENPT__)(GLenum target, GLint level, GLenum pname, GLint * params);
typedef struct {
    GLenum a1;
    GLint a2;
    GLenum a3;
    GLint * a4;
} ARGS_void_GLenum_GLint_GLenum_GLint___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLenum_GLint___GENPT__ func;
    ARGS_void_GLenum_GLint_GLenum_GLint___GENPT__ args;
} PACKED_void_GLenum_GLint_GLenum_GLint___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLenum_GLint___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLenum_GLint___GENPT__;
typedef void (*FUNC_void_GLenum_GLsizei_GLenum_GLboolean)(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef struct {
    GLenum a1;
    GLsizei a2;
    GLenum a3;
    GLboolean a4;
} ARGS_void_GLenum_GLsizei_GLenum_GLboolean;
typedef struct {
    int format;
    FUNC_void_GLenum_GLsizei_GLenum_GLboolean func;
    ARGS_void_GLenum_GLsizei_GLenum_GLboolean args;
} PACKED_void_GLenum_GLsizei_GLenum_GLboolean;
typedef struct {
    int func;
    ARGS_void_GLenum_GLsizei_GLenum_GLboolean args;
} INDEXED_void_GLenum_GLsizei_GLenum_GLboolean;
typedef void (*FUNC_void_GLshort)(GLshort c);
typedef struct {
    GLshort a1;
} ARGS_void_GLshort;
typedef struct {
    int format;
    FUNC_void_GLshort func;
    ARGS_void_GLshort args;
} PACKED_void_GLshort;
typedef struct {
    int func;
    ARGS_void_GLshort args;
} INDEXED_void_GLshort;
typedef void (*FUNC_void_GLubyte)(GLubyte c);
typedef struct {
    GLubyte a1;
} ARGS_void_GLubyte;
typedef struct {
    int format;
    FUNC_void_GLubyte func;
    ARGS_void_GLubyte args;
} PACKED_void_GLubyte;
typedef struct {
    int func;
    ARGS_void_GLubyte args;
} INDEXED_void_GLubyte;
typedef GLboolean (*FUNC_GLboolean_GLuint)(GLuint buffer);
typedef struct {
    GLuint a1;
} ARGS_GLboolean_GLuint;
typedef struct {
    int format;
    FUNC_GLboolean_GLuint func;
    ARGS_GLboolean_GLuint args;
} PACKED_GLboolean_GLuint;
typedef struct {
    int func;
    ARGS_GLboolean_GLuint args;
} INDEXED_GLboolean_GLuint;
typedef GLboolean (*FUNC_GLboolean_GLenum)(GLenum cap);
typedef struct {
    GLenum a1;
} ARGS_GLboolean_GLenum;
typedef struct {
    int format;
    FUNC_GLboolean_GLenum func;
    ARGS_GLboolean_GLenum args;
} PACKED_GLboolean_GLenum;
typedef struct {
    int func;
    ARGS_GLboolean_GLenum args;
} INDEXED_GLboolean_GLenum;
typedef void (*FUNC_void_GLint_GLushort)(GLint factor, GLushort pattern);
typedef struct {
    GLint a1;
    GLushort a2;
} ARGS_void_GLint_GLushort;
typedef struct {
    int format;
    FUNC_void_GLint_GLushort func;
    ARGS_void_GLint_GLushort args;
} PACKED_void_GLint_GLushort;
typedef struct {
    int func;
    ARGS_void_GLint_GLushort args;
} INDEXED_void_GLint_GLushort;
typedef void (*FUNC_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble * points);
typedef struct {
    GLenum a1;
    GLdouble a2;
    GLdouble a3;
    GLint a4;
    GLint a5;
    const GLdouble * a6;
} ARGS_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__ func;
    ARGS_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__ args;
} PACKED_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__ args;
} INDEXED_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__;
typedef void (*FUNC_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat * points);
typedef struct {
    GLenum a1;
    GLfloat a2;
    GLfloat a3;
    GLint a4;
    GLint a5;
    const GLfloat * a6;
} ARGS_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__ func;
    ARGS_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__ args;
} PACKED_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__ args;
} INDEXED_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__;
typedef void (*FUNC_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble * points);
typedef struct {
    GLenum a1;
    GLdouble a2;
    GLdouble a3;
    GLint a4;
    GLint a5;
    GLdouble a6;
    GLdouble a7;
    GLint a8;
    GLint a9;
    const GLdouble * a10;
} ARGS_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__ func;
    ARGS_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__ args;
} PACKED_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__ args;
} INDEXED_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__;
typedef void (*FUNC_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat * points);
typedef struct {
    GLenum a1;
    GLfloat a2;
    GLfloat a3;
    GLint a4;
    GLint a5;
    GLfloat a6;
    GLfloat a7;
    GLint a8;
    GLint a9;
    const GLfloat * a10;
} ARGS_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__ func;
    ARGS_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__ args;
} PACKED_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__ args;
} INDEXED_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__;
typedef GLvoid * (*FUNC_GLvoid___GENPT___GLenum_GLenum)(GLenum target, GLenum access);
typedef struct {
    GLenum a1;
    GLenum a2;
} ARGS_GLvoid___GENPT___GLenum_GLenum;
typedef struct {
    int format;
    FUNC_GLvoid___GENPT___GLenum_GLenum func;
    ARGS_GLvoid___GENPT___GLenum_GLenum args;
} PACKED_GLvoid___GENPT___GLenum_GLenum;
typedef struct {
    int func;
    ARGS_GLvoid___GENPT___GLenum_GLenum args;
} INDEXED_GLvoid___GENPT___GLenum_GLenum;
typedef void (*FUNC_void_GLint_GLdouble_GLdouble)(GLint un, GLdouble u1, GLdouble u2);
typedef struct {
    GLint a1;
    GLdouble a2;
    GLdouble a3;
} ARGS_void_GLint_GLdouble_GLdouble;
typedef struct {
    int format;
    FUNC_void_GLint_GLdouble_GLdouble func;
    ARGS_void_GLint_GLdouble_GLdouble args;
} PACKED_void_GLint_GLdouble_GLdouble;
typedef struct {
    int func;
    ARGS_void_GLint_GLdouble_GLdouble args;
} INDEXED_void_GLint_GLdouble_GLdouble;
typedef void (*FUNC_void_GLint_GLfloat_GLfloat)(GLint un, GLfloat u1, GLfloat u2);
typedef struct {
    GLint a1;
    GLfloat a2;
    GLfloat a3;
} ARGS_void_GLint_GLfloat_GLfloat;
typedef struct {
    int format;
    FUNC_void_GLint_GLfloat_GLfloat func;
    ARGS_void_GLint_GLfloat_GLfloat args;
} PACKED_void_GLint_GLfloat_GLfloat;
typedef struct {
    int func;
    ARGS_void_GLint_GLfloat_GLfloat args;
} INDEXED_void_GLint_GLfloat_GLfloat;
typedef void (*FUNC_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
typedef struct {
    GLint a1;
    GLdouble a2;
    GLdouble a3;
    GLint a4;
    GLdouble a5;
    GLdouble a6;
} ARGS_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble;
typedef struct {
    int format;
    FUNC_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble func;
    ARGS_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble args;
} PACKED_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble;
typedef struct {
    int func;
    ARGS_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble args;
} INDEXED_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble;
typedef void (*FUNC_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
typedef struct {
    GLint a1;
    GLfloat a2;
    GLfloat a3;
    GLint a4;
    GLfloat a5;
    GLfloat a6;
} ARGS_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat;
typedef struct {
    int format;
    FUNC_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat func;
    ARGS_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat args;
} PACKED_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat;
typedef struct {
    int func;
    ARGS_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat args;
} INDEXED_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat;
typedef void (*FUNC_void_GLenum_GLenum_GLboolean)(GLenum target, GLenum internalformat, GLboolean sink);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLboolean a3;
} ARGS_void_GLenum_GLenum_GLboolean;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLboolean func;
    ARGS_void_GLenum_GLenum_GLboolean args;
} PACKED_void_GLenum_GLenum_GLboolean;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLboolean args;
} INDEXED_void_GLenum_GLenum_GLboolean;
typedef void (*FUNC_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei)(GLenum mode, const GLint * first, const GLsizei * count, GLsizei drawcount);
typedef struct {
    GLenum a1;
    const GLint * a2;
    const GLsizei * a3;
    GLsizei a4;
} ARGS_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei;
typedef struct {
    int format;
    FUNC_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei func;
    ARGS_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei args;
} PACKED_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei;
typedef struct {
    int func;
    ARGS_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei args;
} INDEXED_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei;
typedef void (*FUNC_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei)(GLenum mode, const GLsizei * count, GLenum type, GLvoid*const * indices, GLsizei drawcount);
typedef struct {
    GLenum a1;
    const GLsizei * a2;
    GLenum a3;
    GLvoid*const * a4;
    GLsizei a5;
} ARGS_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei;
typedef struct {
    int format;
    FUNC_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei func;
    ARGS_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei args;
} PACKED_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei;
typedef struct {
    int func;
    ARGS_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei args;
} INDEXED_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei;
typedef void (*FUNC_void_GLenum_GLdouble)(GLenum target, GLdouble s);
typedef struct {
    GLenum a1;
    GLdouble a2;
} ARGS_void_GLenum_GLdouble;
typedef struct {
    int format;
    FUNC_void_GLenum_GLdouble func;
    ARGS_void_GLenum_GLdouble args;
} PACKED_void_GLenum_GLdouble;
typedef struct {
    int func;
    ARGS_void_GLenum_GLdouble args;
} INDEXED_void_GLenum_GLdouble;
typedef void (*FUNC_void_GLenum_GLshort)(GLenum target, GLshort s);
typedef struct {
    GLenum a1;
    GLshort a2;
} ARGS_void_GLenum_GLshort;
typedef struct {
    int format;
    FUNC_void_GLenum_GLshort func;
    ARGS_void_GLenum_GLshort args;
} PACKED_void_GLenum_GLshort;
typedef struct {
    int func;
    ARGS_void_GLenum_GLshort args;
} INDEXED_void_GLenum_GLshort;
typedef void (*FUNC_void_GLenum_const_GLshort___GENPT__)(GLenum target, const GLshort * v);
typedef struct {
    GLenum a1;
    const GLshort * a2;
} ARGS_void_GLenum_const_GLshort___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_const_GLshort___GENPT__ func;
    ARGS_void_GLenum_const_GLshort___GENPT__ args;
} PACKED_void_GLenum_const_GLshort___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_const_GLshort___GENPT__ args;
} INDEXED_void_GLenum_const_GLshort___GENPT__;
typedef void (*FUNC_void_GLenum_GLdouble_GLdouble)(GLenum target, GLdouble s, GLdouble t);
typedef struct {
    GLenum a1;
    GLdouble a2;
    GLdouble a3;
} ARGS_void_GLenum_GLdouble_GLdouble;
typedef struct {
    int format;
    FUNC_void_GLenum_GLdouble_GLdouble func;
    ARGS_void_GLenum_GLdouble_GLdouble args;
} PACKED_void_GLenum_GLdouble_GLdouble;
typedef struct {
    int func;
    ARGS_void_GLenum_GLdouble_GLdouble args;
} INDEXED_void_GLenum_GLdouble_GLdouble;
typedef void (*FUNC_void_GLenum_GLfloat_GLfloat)(GLenum target, GLfloat s, GLfloat t);
typedef struct {
    GLenum a1;
    GLfloat a2;
    GLfloat a3;
} ARGS_void_GLenum_GLfloat_GLfloat;
typedef struct {
    int format;
    FUNC_void_GLenum_GLfloat_GLfloat func;
    ARGS_void_GLenum_GLfloat_GLfloat args;
} PACKED_void_GLenum_GLfloat_GLfloat;
typedef struct {
    int func;
    ARGS_void_GLenum_GLfloat_GLfloat args;
} INDEXED_void_GLenum_GLfloat_GLfloat;
typedef void (*FUNC_void_GLenum_GLshort_GLshort)(GLenum target, GLshort s, GLshort t);
typedef struct {
    GLenum a1;
    GLshort a2;
    GLshort a3;
} ARGS_void_GLenum_GLshort_GLshort;
typedef struct {
    int format;
    FUNC_void_GLenum_GLshort_GLshort func;
    ARGS_void_GLenum_GLshort_GLshort args;
} PACKED_void_GLenum_GLshort_GLshort;
typedef struct {
    int func;
    ARGS_void_GLenum_GLshort_GLshort args;
} INDEXED_void_GLenum_GLshort_GLshort;
typedef void (*FUNC_void_GLenum_GLdouble_GLdouble_GLdouble)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef struct {
    GLenum a1;
    GLdouble a2;
    GLdouble a3;
    GLdouble a4;
} ARGS_void_GLenum_GLdouble_GLdouble_GLdouble;
typedef struct {
    int format;
    FUNC_void_GLenum_GLdouble_GLdouble_GLdouble func;
    ARGS_void_GLenum_GLdouble_GLdouble_GLdouble args;
} PACKED_void_GLenum_GLdouble_GLdouble_GLdouble;
typedef struct {
    int func;
    ARGS_void_GLenum_GLdouble_GLdouble_GLdouble args;
} INDEXED_void_GLenum_GLdouble_GLdouble_GLdouble;
typedef void (*FUNC_void_GLenum_GLfloat_GLfloat_GLfloat)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef struct {
    GLenum a1;
    GLfloat a2;
    GLfloat a3;
    GLfloat a4;
} ARGS_void_GLenum_GLfloat_GLfloat_GLfloat;
typedef struct {
    int format;
    FUNC_void_GLenum_GLfloat_GLfloat_GLfloat func;
    ARGS_void_GLenum_GLfloat_GLfloat_GLfloat args;
} PACKED_void_GLenum_GLfloat_GLfloat_GLfloat;
typedef struct {
    int func;
    ARGS_void_GLenum_GLfloat_GLfloat_GLfloat args;
} INDEXED_void_GLenum_GLfloat_GLfloat_GLfloat;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLint)(GLenum target, GLint s, GLint t, GLint r);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLint a4;
} ARGS_void_GLenum_GLint_GLint_GLint;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLint func;
    ARGS_void_GLenum_GLint_GLint_GLint args;
} PACKED_void_GLenum_GLint_GLint_GLint;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLint args;
} INDEXED_void_GLenum_GLint_GLint_GLint;
typedef void (*FUNC_void_GLenum_GLshort_GLshort_GLshort)(GLenum target, GLshort s, GLshort t, GLshort r);
typedef struct {
    GLenum a1;
    GLshort a2;
    GLshort a3;
    GLshort a4;
} ARGS_void_GLenum_GLshort_GLshort_GLshort;
typedef struct {
    int format;
    FUNC_void_GLenum_GLshort_GLshort_GLshort func;
    ARGS_void_GLenum_GLshort_GLshort_GLshort args;
} PACKED_void_GLenum_GLshort_GLshort_GLshort;
typedef struct {
    int func;
    ARGS_void_GLenum_GLshort_GLshort_GLshort args;
} INDEXED_void_GLenum_GLshort_GLshort_GLshort;
typedef void (*FUNC_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef struct {
    GLenum a1;
    GLdouble a2;
    GLdouble a3;
    GLdouble a4;
    GLdouble a5;
} ARGS_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble;
typedef struct {
    int format;
    FUNC_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble func;
    ARGS_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble args;
} PACKED_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble;
typedef struct {
    int func;
    ARGS_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble args;
} INDEXED_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble;
typedef void (*FUNC_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef struct {
    GLenum a1;
    GLfloat a2;
    GLfloat a3;
    GLfloat a4;
    GLfloat a5;
} ARGS_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat;
typedef struct {
    int format;
    FUNC_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat func;
    ARGS_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat args;
} PACKED_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat;
typedef struct {
    int func;
    ARGS_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat args;
} INDEXED_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat;
typedef void (*FUNC_void_GLenum_GLshort_GLshort_GLshort_GLshort)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef struct {
    GLenum a1;
    GLshort a2;
    GLshort a3;
    GLshort a4;
    GLshort a5;
} ARGS_void_GLenum_GLshort_GLshort_GLshort_GLshort;
typedef struct {
    int format;
    FUNC_void_GLenum_GLshort_GLshort_GLshort_GLshort func;
    ARGS_void_GLenum_GLshort_GLshort_GLshort_GLshort args;
} PACKED_void_GLenum_GLshort_GLshort_GLshort_GLshort;
typedef struct {
    int func;
    ARGS_void_GLenum_GLshort_GLshort_GLshort_GLshort args;
} INDEXED_void_GLenum_GLshort_GLshort_GLshort_GLshort;
typedef void (*FUNC_void_GLuint_GLenum)(GLuint list, GLenum mode);
typedef struct {
    GLuint a1;
    GLenum a2;
} ARGS_void_GLuint_GLenum;
typedef struct {
    int format;
    FUNC_void_GLuint_GLenum func;
    ARGS_void_GLuint_GLenum args;
} PACKED_void_GLuint_GLenum;
typedef struct {
    int func;
    ARGS_void_GLuint_GLenum args;
} INDEXED_void_GLuint_GLenum;
typedef void (*FUNC_void_GLenum_GLsizei_const_GLfloat___GENPT__)(GLenum map, GLsizei mapsize, const GLfloat * values);
typedef struct {
    GLenum a1;
    GLsizei a2;
    const GLfloat * a3;
} ARGS_void_GLenum_GLsizei_const_GLfloat___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLsizei_const_GLfloat___GENPT__ func;
    ARGS_void_GLenum_GLsizei_const_GLfloat___GENPT__ args;
} PACKED_void_GLenum_GLsizei_const_GLfloat___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLsizei_const_GLfloat___GENPT__ args;
} INDEXED_void_GLenum_GLsizei_const_GLfloat___GENPT__;
typedef void (*FUNC_void_GLenum_GLsizei_const_GLuint___GENPT__)(GLenum map, GLsizei mapsize, const GLuint * values);
typedef struct {
    GLenum a1;
    GLsizei a2;
    const GLuint * a3;
} ARGS_void_GLenum_GLsizei_const_GLuint___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLsizei_const_GLuint___GENPT__ func;
    ARGS_void_GLenum_GLsizei_const_GLuint___GENPT__ args;
} PACKED_void_GLenum_GLsizei_const_GLuint___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLsizei_const_GLuint___GENPT__ args;
} INDEXED_void_GLenum_GLsizei_const_GLuint___GENPT__;
typedef void (*FUNC_void_GLenum_GLsizei_const_GLushort___GENPT__)(GLenum map, GLsizei mapsize, const GLushort * values);
typedef struct {
    GLenum a1;
    GLsizei a2;
    const GLushort * a3;
} ARGS_void_GLenum_GLsizei_const_GLushort___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLsizei_const_GLushort___GENPT__ func;
    ARGS_void_GLenum_GLsizei_const_GLushort___GENPT__ args;
} PACKED_void_GLenum_GLsizei_const_GLushort___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLsizei_const_GLushort___GENPT__ args;
} INDEXED_void_GLenum_GLsizei_const_GLushort___GENPT__;
typedef void (*FUNC_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__)(GLsizei n, const GLuint * textures, const GLfloat * priorities);
typedef struct {
    GLsizei a1;
    const GLuint * a2;
    const GLfloat * a3;
} ARGS_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__ func;
    ARGS_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__ args;
} PACKED_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__ args;
} INDEXED_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__;
typedef void (*FUNC_void_GLshort_GLshort)(GLshort x, GLshort y);
typedef struct {
    GLshort a1;
    GLshort a2;
} ARGS_void_GLshort_GLshort;
typedef struct {
    int format;
    FUNC_void_GLshort_GLshort func;
    ARGS_void_GLshort_GLshort args;
} PACKED_void_GLshort_GLshort;
typedef struct {
    int func;
    ARGS_void_GLshort_GLshort args;
} INDEXED_void_GLshort_GLshort;
typedef void (*FUNC_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * pixels);
typedef struct {
    GLint a1;
    GLint a2;
    GLsizei a3;
    GLsizei a4;
    GLenum a5;
    GLenum a6;
    GLvoid * a7;
} ARGS_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__ func;
    ARGS_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__ args;
} PACKED_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__ args;
} INDEXED_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__;
typedef void (*FUNC_void_const_GLdouble___GENPT___const_GLdouble___GENPT__)(const GLdouble * v1, const GLdouble * v2);
typedef struct {
    const GLdouble * a1;
    const GLdouble * a2;
} ARGS_void_const_GLdouble___GENPT___const_GLdouble___GENPT__;
typedef struct {
    int format;
    FUNC_void_const_GLdouble___GENPT___const_GLdouble___GENPT__ func;
    ARGS_void_const_GLdouble___GENPT___const_GLdouble___GENPT__ args;
} PACKED_void_const_GLdouble___GENPT___const_GLdouble___GENPT__;
typedef struct {
    int func;
    ARGS_void_const_GLdouble___GENPT___const_GLdouble___GENPT__ args;
} INDEXED_void_const_GLdouble___GENPT___const_GLdouble___GENPT__;
typedef void (*FUNC_void_const_GLfloat___GENPT___const_GLfloat___GENPT__)(const GLfloat * v1, const GLfloat * v2);
typedef struct {
    const GLfloat * a1;
    const GLfloat * a2;
} ARGS_void_const_GLfloat___GENPT___const_GLfloat___GENPT__;
typedef struct {
    int format;
    FUNC_void_const_GLfloat___GENPT___const_GLfloat___GENPT__ func;
    ARGS_void_const_GLfloat___GENPT___const_GLfloat___GENPT__ args;
} PACKED_void_const_GLfloat___GENPT___const_GLfloat___GENPT__;
typedef struct {
    int func;
    ARGS_void_const_GLfloat___GENPT___const_GLfloat___GENPT__ args;
} INDEXED_void_const_GLfloat___GENPT___const_GLfloat___GENPT__;
typedef void (*FUNC_void_const_GLint___GENPT___const_GLint___GENPT__)(const GLint * v1, const GLint * v2);
typedef struct {
    const GLint * a1;
    const GLint * a2;
} ARGS_void_const_GLint___GENPT___const_GLint___GENPT__;
typedef struct {
    int format;
    FUNC_void_const_GLint___GENPT___const_GLint___GENPT__ func;
    ARGS_void_const_GLint___GENPT___const_GLint___GENPT__ args;
} PACKED_void_const_GLint___GENPT___const_GLint___GENPT__;
typedef struct {
    int func;
    ARGS_void_const_GLint___GENPT___const_GLint___GENPT__ args;
} INDEXED_void_const_GLint___GENPT___const_GLint___GENPT__;
typedef void (*FUNC_void_const_GLshort___GENPT___const_GLshort___GENPT__)(const GLshort * v1, const GLshort * v2);
typedef struct {
    const GLshort * a1;
    const GLshort * a2;
} ARGS_void_const_GLshort___GENPT___const_GLshort___GENPT__;
typedef struct {
    int format;
    FUNC_void_const_GLshort___GENPT___const_GLshort___GENPT__ func;
    ARGS_void_const_GLshort___GENPT___const_GLshort___GENPT__ args;
} PACKED_void_const_GLshort___GENPT___const_GLshort___GENPT__;
typedef struct {
    int func;
    ARGS_void_const_GLshort___GENPT___const_GLshort___GENPT__ args;
} INDEXED_void_const_GLshort___GENPT___const_GLshort___GENPT__;
typedef GLint (*FUNC_GLint_GLenum)(GLenum mode);
typedef struct {
    GLenum a1;
} ARGS_GLint_GLenum;
typedef struct {
    int format;
    FUNC_GLint_GLenum func;
    ARGS_GLint_GLenum args;
} PACKED_GLint_GLenum;
typedef struct {
    int func;
    ARGS_GLint_GLenum args;
} INDEXED_GLint_GLenum;
typedef void (*FUNC_void_GLfloat_GLboolean)(GLfloat value, GLboolean invert);
typedef struct {
    GLfloat a1;
    GLboolean a2;
} ARGS_void_GLfloat_GLboolean;
typedef struct {
    int format;
    FUNC_void_GLfloat_GLboolean func;
    ARGS_void_GLfloat_GLboolean args;
} PACKED_void_GLfloat_GLboolean;
typedef struct {
    int func;
    ARGS_void_GLfloat_GLboolean args;
} INDEXED_void_GLfloat_GLboolean;
typedef void (*FUNC_void_GLint_GLint_GLsizei_GLsizei)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef struct {
    GLint a1;
    GLint a2;
    GLsizei a3;
    GLsizei a4;
} ARGS_void_GLint_GLint_GLsizei_GLsizei;
typedef struct {
    int format;
    FUNC_void_GLint_GLint_GLsizei_GLsizei func;
    ARGS_void_GLint_GLint_GLsizei_GLsizei args;
} PACKED_void_GLint_GLint_GLsizei_GLsizei;
typedef struct {
    int func;
    ARGS_void_GLint_GLint_GLsizei_GLsizei args;
} INDEXED_void_GLint_GLint_GLsizei_GLsizei;
typedef void (*FUNC_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * row, const GLvoid * column);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLsizei a3;
    GLsizei a4;
    GLenum a5;
    GLenum a6;
    const GLvoid * a7;
    const GLvoid * a8;
} ARGS_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLuint)(GLenum func, GLint ref, GLuint mask);
typedef struct {
    GLenum a1;
    GLint a2;
    GLuint a3;
} ARGS_void_GLenum_GLint_GLuint;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLuint func;
    ARGS_void_GLenum_GLint_GLuint args;
} PACKED_void_GLenum_GLint_GLuint;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLuint args;
} INDEXED_void_GLenum_GLint_GLuint;
typedef void (*FUNC_void_GLenum_GLenum_GLenum)(GLenum fail, GLenum zfail, GLenum zpass);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLenum a3;
} ARGS_void_GLenum_GLenum_GLenum;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLenum func;
    ARGS_void_GLenum_GLenum_GLenum args;
} PACKED_void_GLenum_GLenum_GLenum;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLenum args;
} INDEXED_void_GLenum_GLenum_GLenum;
typedef void (*FUNC_void_GLenum_GLenum_GLdouble)(GLenum coord, GLenum pname, GLdouble param);
typedef struct {
    GLenum a1;
    GLenum a2;
    GLdouble a3;
} ARGS_void_GLenum_GLenum_GLdouble;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_GLdouble func;
    ARGS_void_GLenum_GLenum_GLdouble args;
} PACKED_void_GLenum_GLenum_GLdouble;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_GLdouble args;
} INDEXED_void_GLenum_GLenum_GLdouble;
typedef void (*FUNC_void_GLenum_GLenum_const_GLdouble___GENPT__)(GLenum coord, GLenum pname, const GLdouble * params);
typedef struct {
    GLenum a1;
    GLenum a2;
    const GLdouble * a3;
} ARGS_void_GLenum_GLenum_const_GLdouble___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLenum_const_GLdouble___GENPT__ func;
    ARGS_void_GLenum_GLenum_const_GLdouble___GENPT__ args;
} PACKED_void_GLenum_GLenum_const_GLdouble___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLenum_const_GLdouble___GENPT__ args;
} INDEXED_void_GLenum_GLenum_const_GLdouble___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLsizei a4;
    GLint a5;
    GLenum a6;
    GLenum a7;
    const GLvoid * a8;
} ARGS_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLsizei a4;
    GLsizei a5;
    GLint a6;
    GLenum a7;
    GLenum a8;
    const GLvoid * a9;
} ARGS_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLsizei a4;
    GLsizei a5;
    GLsizei a6;
    GLint a7;
    GLenum a8;
    GLenum a9;
    const GLvoid * a10;
} ARGS_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLsizei a4;
    GLenum a5;
    GLenum a6;
    const GLvoid * a7;
} ARGS_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLint a4;
    GLsizei a5;
    GLsizei a6;
    GLenum a7;
    GLenum a8;
    const GLvoid * a9;
} ARGS_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef void (*FUNC_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels);
typedef struct {
    GLenum a1;
    GLint a2;
    GLint a3;
    GLint a4;
    GLint a5;
    GLsizei a6;
    GLsizei a7;
    GLsizei a8;
    GLenum a9;
    GLenum a10;
    const GLvoid * a11;
} ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int format;
    FUNC_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} PACKED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef struct {
    int func;
    ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args;
} INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__;
typedef int (*FUNC_int_Display___GENPT___int)(Display * dpy, int hpId);
typedef struct {
    Display * a1;
    int a2;
} ARGS_int_Display___GENPT___int;
typedef struct {
    int format;
    FUNC_int_Display___GENPT___int func;
    ARGS_int_Display___GENPT___int args;
} PACKED_int_Display___GENPT___int;
typedef struct {
    int func;
    ARGS_int_Display___GENPT___int args;
} INDEXED_int_Display___GENPT___int;
typedef void (*FUNC_void_uint32_t_uint32_t)(uint32_t window, uint32_t barrier);
typedef struct {
    uint32_t a1;
    uint32_t a2;
} ARGS_void_uint32_t_uint32_t;
typedef struct {
    int format;
    FUNC_void_uint32_t_uint32_t func;
    ARGS_void_uint32_t_uint32_t args;
} PACKED_void_uint32_t_uint32_t;
typedef struct {
    int func;
    ARGS_void_uint32_t_uint32_t args;
} INDEXED_void_uint32_t_uint32_t;
typedef void (*FUNC_void_uint32_t)(uint32_t drawable);
typedef struct {
    uint32_t a1;
} ARGS_void_uint32_t;
typedef struct {
    int format;
    FUNC_void_uint32_t func;
    ARGS_void_uint32_t args;
} PACKED_void_uint32_t;
typedef struct {
    int func;
    ARGS_void_uint32_t args;
} INDEXED_void_uint32_t;
typedef XVisualInfo * (*FUNC_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__)(Display * dpy, int screen, int * attribList);
typedef struct {
    Display * a1;
    int a2;
    int * a3;
} ARGS_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__;
typedef struct {
    int format;
    FUNC_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__ func;
    ARGS_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__ args;
} PACKED_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__;
typedef struct {
    int func;
    ARGS_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__ args;
} INDEXED_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__;
typedef void (*FUNC_void_Display___GENPT___GLXContext_GLXContext_unsigned_long)(Display * dpy, GLXContext src, GLXContext dst, unsigned long mask);
typedef struct {
    Display * a1;
    GLXContext a2;
    GLXContext a3;
    unsigned long a4;
} ARGS_void_Display___GENPT___GLXContext_GLXContext_unsigned_long;
typedef struct {
    int format;
    FUNC_void_Display___GENPT___GLXContext_GLXContext_unsigned_long func;
    ARGS_void_Display___GENPT___GLXContext_GLXContext_unsigned_long args;
} PACKED_void_Display___GENPT___GLXContext_GLXContext_unsigned_long;
typedef struct {
    int func;
    ARGS_void_Display___GENPT___GLXContext_GLXContext_unsigned_long args;
} INDEXED_void_Display___GENPT___GLXContext_GLXContext_unsigned_long;
typedef GLXContext (*FUNC_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool)(Display * dpy, XVisualInfo * vis, GLXContext shareList, Bool direct);
typedef struct {
    Display * a1;
    XVisualInfo * a2;
    GLXContext a3;
    Bool a4;
} ARGS_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool;
typedef struct {
    int format;
    FUNC_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool func;
    ARGS_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool args;
} PACKED_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool;
typedef struct {
    int func;
    ARGS_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool args;
} INDEXED_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool;
typedef void (*FUNC_void_uint32_t_uint32_t_uint32_t_uint32_t)(uint32_t gc_id, uint32_t screen, uint32_t config, uint32_t share_list);
typedef struct {
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
} ARGS_void_uint32_t_uint32_t_uint32_t_uint32_t;
typedef struct {
    int format;
    FUNC_void_uint32_t_uint32_t_uint32_t_uint32_t func;
    ARGS_void_uint32_t_uint32_t_uint32_t_uint32_t args;
} PACKED_void_uint32_t_uint32_t_uint32_t_uint32_t;
typedef struct {
    int func;
    ARGS_void_uint32_t_uint32_t_uint32_t_uint32_t args;
} INDEXED_void_uint32_t_uint32_t_uint32_t_uint32_t;
typedef GLXPixmap (*FUNC_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap)(Display * dpy, XVisualInfo * visual, Pixmap pixmap);
typedef struct {
    Display * a1;
    XVisualInfo * a2;
    Pixmap a3;
} ARGS_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap;
typedef struct {
    int format;
    FUNC_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap func;
    ARGS_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap args;
} PACKED_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap;
typedef struct {
    int func;
    ARGS_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap args;
} INDEXED_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap;
typedef void (*FUNC_void_uint32_t_uint32_t_uint32_t)(uint32_t config, uint32_t pixmap, uint32_t glxpixmap);
typedef struct {
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
} ARGS_void_uint32_t_uint32_t_uint32_t;
typedef struct {
    int format;
    FUNC_void_uint32_t_uint32_t_uint32_t func;
    ARGS_void_uint32_t_uint32_t_uint32_t args;
} PACKED_void_uint32_t_uint32_t_uint32_t;
typedef struct {
    int func;
    ARGS_void_uint32_t_uint32_t_uint32_t args;
} INDEXED_void_uint32_t_uint32_t_uint32_t;
typedef void (*FUNC_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t)(Display * dpy, uint32_t screen, uint32_t server, uint32_t path, uint32_t class, uint32_t node);
typedef struct {
    Display * a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
} ARGS_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t;
typedef struct {
    int format;
    FUNC_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t func;
    ARGS_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t args;
} PACKED_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t;
typedef struct {
    int func;
    ARGS_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t args;
} INDEXED_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t;
typedef void (*FUNC_void_Display___GENPT___GLXContext)(Display * dpy, GLXContext ctx);
typedef struct {
    Display * a1;
    GLXContext a2;
} ARGS_void_Display___GENPT___GLXContext;
typedef struct {
    int format;
    FUNC_void_Display___GENPT___GLXContext func;
    ARGS_void_Display___GENPT___GLXContext args;
} PACKED_void_Display___GENPT___GLXContext;
typedef struct {
    int func;
    ARGS_void_Display___GENPT___GLXContext args;
} INDEXED_void_Display___GENPT___GLXContext;
typedef void (*FUNC_void_Display___GENPT___GLXPixmap)(Display * dpy, GLXPixmap pixmap);
typedef struct {
    Display * a1;
    GLXPixmap a2;
} ARGS_void_Display___GENPT___GLXPixmap;
typedef struct {
    int format;
    FUNC_void_Display___GENPT___GLXPixmap func;
    ARGS_void_Display___GENPT___GLXPixmap args;
} PACKED_void_Display___GENPT___GLXPixmap;
typedef struct {
    int func;
    ARGS_void_Display___GENPT___GLXPixmap args;
} INDEXED_void_Display___GENPT___GLXPixmap;
typedef void (*FUNC_void_Display___GENPT___uint32_t)(Display * dpy, uint32_t glxvideosource);
typedef struct {
    Display * a1;
    uint32_t a2;
} ARGS_void_Display___GENPT___uint32_t;
typedef struct {
    int format;
    FUNC_void_Display___GENPT___uint32_t func;
    ARGS_void_Display___GENPT___uint32_t args;
} PACKED_void_Display___GENPT___uint32_t;
typedef struct {
    int func;
    ARGS_void_Display___GENPT___uint32_t args;
} INDEXED_void_Display___GENPT___uint32_t;
typedef int (*FUNC_int_Display___GENPT___int_int_int_const_void___GENPT__)(Display * dpy, int timeSlice, int attrib, int size, const void * attribList);
typedef struct {
    Display * a1;
    int a2;
    int a3;
    int a4;
    const void * a5;
} ARGS_int_Display___GENPT___int_int_int_const_void___GENPT__;
typedef struct {
    int format;
    FUNC_int_Display___GENPT___int_int_int_const_void___GENPT__ func;
    ARGS_int_Display___GENPT___int_int_int_const_void___GENPT__ args;
} PACKED_int_Display___GENPT___int_int_int_const_void___GENPT__;
typedef struct {
    int func;
    ARGS_int_Display___GENPT___int_int_int_const_void___GENPT__ args;
} INDEXED_int_Display___GENPT___int_int_int_const_void___GENPT__;
typedef int (*FUNC_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__)(Display * dpy, int networkId, int npipes, GLXHyperpipeConfigSGIX cfg, int * hpId);
typedef struct {
    Display * a1;
    int a2;
    int a3;
    GLXHyperpipeConfigSGIX a4;
    int * a5;
} ARGS_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__;
typedef struct {
    int format;
    FUNC_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__ func;
    ARGS_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__ args;
} PACKED_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__;
typedef struct {
    int func;
    ARGS_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__ args;
} INDEXED_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__;
typedef Bool (*FUNC_Bool_Display___GENPT___GLXContext)(Display * dpy, GLXContext ctx);
typedef struct {
    Display * a1;
    GLXContext a2;
} ARGS_Bool_Display___GENPT___GLXContext;
typedef struct {
    int format;
    FUNC_Bool_Display___GENPT___GLXContext func;
    ARGS_Bool_Display___GENPT___GLXContext args;
} PACKED_Bool_Display___GENPT___GLXContext;
typedef struct {
    int func;
    ARGS_Bool_Display___GENPT___GLXContext args;
} INDEXED_Bool_Display___GENPT___GLXContext;
typedef Bool (*FUNC_Bool_Display___GENPT___GLXDrawable_GLXContext)(Display * dpy, GLXDrawable drawable, GLXContext ctx);
typedef struct {
    Display * a1;
    GLXDrawable a2;
    GLXContext a3;
} ARGS_Bool_Display___GENPT___GLXDrawable_GLXContext;
typedef struct {
    int format;
    FUNC_Bool_Display___GENPT___GLXDrawable_GLXContext func;
    ARGS_Bool_Display___GENPT___GLXDrawable_GLXContext args;
} PACKED_Bool_Display___GENPT___GLXDrawable_GLXContext;
typedef struct {
    int func;
    ARGS_Bool_Display___GENPT___GLXDrawable_GLXContext args;
} INDEXED_Bool_Display___GENPT___GLXDrawable_GLXContext;
typedef const char * (*FUNC_const_char___GENPT___Display___GENPT___int)(Display * dpy, int screen);
typedef struct {
    Display * a1;
    int a2;
} ARGS_const_char___GENPT___Display___GENPT___int;
typedef struct {
    int format;
    FUNC_const_char___GENPT___Display___GENPT___int func;
    ARGS_const_char___GENPT___Display___GENPT___int args;
} PACKED_const_char___GENPT___Display___GENPT___int;
typedef struct {
    int func;
    ARGS_const_char___GENPT___Display___GENPT___int args;
} INDEXED_const_char___GENPT___Display___GENPT___int;
typedef int (*FUNC_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__)(Display * dpy, int timeSlice, int attrib, int size, const void * attribList, void * returnAttribList);
typedef struct {
    Display * a1;
    int a2;
    int a3;
    int a4;
    const void * a5;
    void * a6;
} ARGS_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__;
typedef struct {
    int format;
    FUNC_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__ func;
    ARGS_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__ args;
} PACKED_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__;
typedef struct {
    int func;
    ARGS_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__ args;
} INDEXED_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__;
typedef GLXHyperpipeConfigSGIX * (*FUNC_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__)(Display * dpy, int hpId, int * npipes);
typedef struct {
    Display * a1;
    int a2;
    int * a3;
} ARGS_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__;
typedef struct {
    int format;
    FUNC_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__ func;
    ARGS_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__ args;
} PACKED_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__;
typedef struct {
    int func;
    ARGS_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__ args;
} INDEXED_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__;
typedef GLXHyperpipeNetworkSGIX * (*FUNC_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__)(Display * dpy, int * npipes);
typedef struct {
    Display * a1;
    int * a2;
} ARGS_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__;
typedef struct {
    int format;
    FUNC_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__ func;
    ARGS_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__ args;
} PACKED_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__;
typedef struct {
    int func;
    ARGS_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__ args;
} INDEXED_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__;
typedef const char * (*FUNC_const_char___GENPT___Display___GENPT___int_int)(Display * dpy, int screen, int name);
typedef struct {
    Display * a1;
    int a2;
    int a3;
} ARGS_const_char___GENPT___Display___GENPT___int_int;
typedef struct {
    int format;
    FUNC_const_char___GENPT___Display___GENPT___int_int func;
    ARGS_const_char___GENPT___Display___GENPT___int_int args;
} PACKED_const_char___GENPT___Display___GENPT___int_int;
typedef struct {
    int func;
    ARGS_const_char___GENPT___Display___GENPT___int_int args;
} INDEXED_const_char___GENPT___Display___GENPT___int_int;
typedef Bool (*FUNC_Bool_Display___GENPT___int___GENPT___int___GENPT__)(Display * dpy, int * maj, int * min);
typedef struct {
    Display * a1;
    int * a2;
    int * a3;
} ARGS_Bool_Display___GENPT___int___GENPT___int___GENPT__;
typedef struct {
    int format;
    FUNC_Bool_Display___GENPT___int___GENPT___int___GENPT__ func;
    ARGS_Bool_Display___GENPT___int___GENPT___int___GENPT__ args;
} PACKED_Bool_Display___GENPT___int___GENPT___int___GENPT__;
typedef struct {
    int func;
    ARGS_Bool_Display___GENPT___int___GENPT___int___GENPT__ args;
} INDEXED_Bool_Display___GENPT___int___GENPT___int___GENPT__;
typedef void (*FUNC_void_Display___GENPT___GLXDrawable)(Display * dpy, GLXDrawable drawable);
typedef struct {
    Display * a1;
    GLXDrawable a2;
} ARGS_void_Display___GENPT___GLXDrawable;
typedef struct {
    int format;
    FUNC_void_Display___GENPT___GLXDrawable func;
    ARGS_void_Display___GENPT___GLXDrawable args;
} PACKED_void_Display___GENPT___GLXDrawable;
typedef struct {
    int func;
    ARGS_void_Display___GENPT___GLXDrawable args;
} INDEXED_void_Display___GENPT___GLXDrawable;
typedef void (*FUNC_void_Font_int_int_int)(Font font, int first, int count, int list);
typedef struct {
    Font a1;
    int a2;
    int a3;
    int a4;
} ARGS_void_Font_int_int_int;
typedef struct {
    int format;
    FUNC_void_Font_int_int_int func;
    ARGS_void_Font_int_int_int args;
} PACKED_void_Font_int_int_int;
typedef struct {
    int func;
    ARGS_void_Font_int_int_int args;
} INDEXED_void_Font_int_int_int;
typedef GLXFBConfig * (*FUNC_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__)(Display * dpy, int screen, const int * attrib_list, int * nelements);
typedef struct {
    Display * a1;
    int a2;
    const int * a3;
    int * a4;
} ARGS_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__;
typedef struct {
    int format;
    FUNC_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__ func;
    ARGS_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__ args;
} PACKED_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__;
typedef struct {
    int func;
    ARGS_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__ args;
} INDEXED_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__;
typedef GLXContext (*FUNC_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool)(Display * dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct);
typedef struct {
    Display * a1;
    GLXFBConfig a2;
    int a3;
    GLXContext a4;
    Bool a5;
} ARGS_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool;
typedef struct {
    int format;
    FUNC_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool func;
    ARGS_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool args;
} PACKED_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool;
typedef struct {
    int func;
    ARGS_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool args;
} INDEXED_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool;
typedef GLXPbuffer (*FUNC_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__)(Display * dpy, GLXFBConfig config, const int * attrib_list);
typedef struct {
    Display * a1;
    GLXFBConfig a2;
    const int * a3;
} ARGS_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__;
typedef struct {
    int format;
    FUNC_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__ func;
    ARGS_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__ args;
} PACKED_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__;
typedef struct {
    int func;
    ARGS_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__ args;
} INDEXED_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__;
typedef GLXPixmap (*FUNC_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__)(Display * dpy, GLXFBConfig config, Pixmap pixmap, const int * attrib_list);
typedef struct {
    Display * a1;
    GLXFBConfig a2;
    Pixmap a3;
    const int * a4;
} ARGS_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__;
typedef struct {
    int format;
    FUNC_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__ func;
    ARGS_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__ args;
} PACKED_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__;
typedef struct {
    int func;
    ARGS_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__ args;
} INDEXED_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__;
typedef GLXWindow (*FUNC_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__)(Display * dpy, GLXFBConfig config, Window win, const int * attrib_list);
typedef struct {
    Display * a1;
    GLXFBConfig a2;
    Window a3;
    const int * a4;
} ARGS_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__;
typedef struct {
    int format;
    FUNC_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__ func;
    ARGS_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__ args;
} PACKED_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__;
typedef struct {
    int func;
    ARGS_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__ args;
} INDEXED_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__;
typedef void (*FUNC_void_Display___GENPT___GLXPbuffer)(Display * dpy, GLXPbuffer pbuf);
typedef struct {
    Display * a1;
    GLXPbuffer a2;
} ARGS_void_Display___GENPT___GLXPbuffer;
typedef struct {
    int format;
    FUNC_void_Display___GENPT___GLXPbuffer func;
    ARGS_void_Display___GENPT___GLXPbuffer args;
} PACKED_void_Display___GENPT___GLXPbuffer;
typedef struct {
    int func;
    ARGS_void_Display___GENPT___GLXPbuffer args;
} INDEXED_void_Display___GENPT___GLXPbuffer;
typedef void (*FUNC_void_Display___GENPT___GLXWindow)(Display * dpy, GLXWindow win);
typedef struct {
    Display * a1;
    GLXWindow a2;
} ARGS_void_Display___GENPT___GLXWindow;
typedef struct {
    int format;
    FUNC_void_Display___GENPT___GLXWindow func;
    ARGS_void_Display___GENPT___GLXWindow args;
} PACKED_void_Display___GENPT___GLXWindow;
typedef struct {
    int func;
    ARGS_void_Display___GENPT___GLXWindow args;
} INDEXED_void_Display___GENPT___GLXWindow;
typedef Display * (*FUNC_Display___GENPT__)();
typedef struct {
} ARGS_Display___GENPT__;
typedef struct {
    int format;
    FUNC_Display___GENPT__ func;
    ARGS_Display___GENPT__ args;
} PACKED_Display___GENPT__;
typedef struct {
    int func;
    ARGS_Display___GENPT__ args;
} INDEXED_Display___GENPT__;
typedef GLXDrawable (*FUNC_GLXDrawable)();
typedef struct {
} ARGS_GLXDrawable;
typedef struct {
    int format;
    FUNC_GLXDrawable func;
    ARGS_GLXDrawable args;
} PACKED_GLXDrawable;
typedef struct {
    int func;
    ARGS_GLXDrawable args;
} INDEXED_GLXDrawable;
typedef int (*FUNC_int_Display___GENPT___GLXFBConfig_int_int___GENPT__)(Display * dpy, GLXFBConfig config, int attribute, int * value);
typedef struct {
    Display * a1;
    GLXFBConfig a2;
    int a3;
    int * a4;
} ARGS_int_Display___GENPT___GLXFBConfig_int_int___GENPT__;
typedef struct {
    int format;
    FUNC_int_Display___GENPT___GLXFBConfig_int_int___GENPT__ func;
    ARGS_int_Display___GENPT___GLXFBConfig_int_int___GENPT__ args;
} PACKED_int_Display___GENPT___GLXFBConfig_int_int___GENPT__;
typedef struct {
    int func;
    ARGS_int_Display___GENPT___GLXFBConfig_int_int___GENPT__ args;
} INDEXED_int_Display___GENPT___GLXFBConfig_int_int___GENPT__;
typedef GLXFBConfig * (*FUNC_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__)(Display * dpy, int screen, int * nelements);
typedef struct {
    Display * a1;
    int a2;
    int * a3;
} ARGS_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__;
typedef struct {
    int format;
    FUNC_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__ func;
    ARGS_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__ args;
} PACKED_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__;
typedef struct {
    int func;
    ARGS_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__ args;
} INDEXED_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__;
typedef __GLXextFuncPtr (*FUNC___GLXextFuncPtr_const_GLubyte___GENPT__)(const GLubyte * procName);
typedef struct {
    const GLubyte * a1;
} ARGS___GLXextFuncPtr_const_GLubyte___GENPT__;
typedef struct {
    int format;
    FUNC___GLXextFuncPtr_const_GLubyte___GENPT__ func;
    ARGS___GLXextFuncPtr_const_GLubyte___GENPT__ args;
} PACKED___GLXextFuncPtr_const_GLubyte___GENPT__;
typedef struct {
    int func;
    ARGS___GLXextFuncPtr_const_GLubyte___GENPT__ args;
} INDEXED___GLXextFuncPtr_const_GLubyte___GENPT__;
typedef void (*FUNC_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__)(Display * dpy, GLXDrawable draw, unsigned long * event_mask);
typedef struct {
    Display * a1;
    GLXDrawable a2;
    unsigned long * a3;
} ARGS_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__;
typedef struct {
    int format;
    FUNC_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__ func;
    ARGS_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__ args;
} PACKED_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__;
typedef struct {
    int func;
    ARGS_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__ args;
} INDEXED_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__;
typedef XVisualInfo * (*FUNC_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig)(Display * dpy, GLXFBConfig config);
typedef struct {
    Display * a1;
    GLXFBConfig a2;
} ARGS_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig;
typedef struct {
    int format;
    FUNC_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig func;
    ARGS_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig args;
} PACKED_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig;
typedef struct {
    int func;
    ARGS_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig args;
} INDEXED_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig;
typedef Bool (*FUNC_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext)(Display * dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);
typedef struct {
    Display * a1;
    GLXDrawable a2;
    GLXDrawable a3;
    GLXContext a4;
} ARGS_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext;
typedef struct {
    int format;
    FUNC_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext func;
    ARGS_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext args;
} PACKED_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext;
typedef struct {
    int func;
    ARGS_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext args;
} INDEXED_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext;
typedef int (*FUNC_int_Display___GENPT___GLXContext_int_int___GENPT__)(Display * dpy, GLXContext ctx, int attribute, int * value);
typedef struct {
    Display * a1;
    GLXContext a2;
    int a3;
    int * a4;
} ARGS_int_Display___GENPT___GLXContext_int_int___GENPT__;
typedef struct {
    int format;
    FUNC_int_Display___GENPT___GLXContext_int_int___GENPT__ func;
    ARGS_int_Display___GENPT___GLXContext_int_int___GENPT__ args;
} PACKED_int_Display___GENPT___GLXContext_int_int___GENPT__;
typedef struct {
    int func;
    ARGS_int_Display___GENPT___GLXContext_int_int___GENPT__ args;
} INDEXED_int_Display___GENPT___GLXContext_int_int___GENPT__;
typedef void (*FUNC_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__)(Display * dpy, GLXDrawable draw, int attribute, unsigned int * value);
typedef struct {
    Display * a1;
    GLXDrawable a2;
    int a3;
    unsigned int * a4;
} ARGS_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__;
typedef struct {
    int format;
    FUNC_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__ func;
    ARGS_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__ args;
} PACKED_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__;
typedef struct {
    int func;
    ARGS_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__ args;
} INDEXED_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__;
typedef void (*FUNC_void_Display___GENPT___GLXDrawable_unsigned_long)(Display * dpy, GLXDrawable draw, unsigned long event_mask);
typedef struct {
    Display * a1;
    GLXDrawable a2;
    unsigned long a3;
} ARGS_void_Display___GENPT___GLXDrawable_unsigned_long;
typedef struct {
    int format;
    FUNC_void_Display___GENPT___GLXDrawable_unsigned_long func;
    ARGS_void_Display___GENPT___GLXDrawable_unsigned_long args;
} PACKED_void_Display___GENPT___GLXDrawable_unsigned_long;
typedef struct {
    int func;
    ARGS_void_Display___GENPT___GLXDrawable_unsigned_long args;
} INDEXED_void_Display___GENPT___GLXDrawable_unsigned_long;

extern void glPushCall(void *data);
void glPackedCall(const packed_call_t *packed);
void glIndexedCall(const indexed_call_t *packed, void *ret_v);

#define glAccum_INDEX 1
#define glAccum_RETURN void
#define glAccum_ARG_NAMES op, value
#define glAccum_ARG_EXPAND GLenum op, GLfloat value
#define glAccum_PACKED PACKED_void_GLenum_GLfloat
#define glAccum_INDEXED INDEXED_void_GLenum_GLfloat
#define glAccum_FORMAT FORMAT_void_GLenum_GLfloat
#define glActiveTexture_INDEX 2
#define glActiveTexture_RETURN void
#define glActiveTexture_ARG_NAMES texture
#define glActiveTexture_ARG_EXPAND GLenum texture
#define glActiveTexture_PACKED PACKED_void_GLenum
#define glActiveTexture_INDEXED INDEXED_void_GLenum
#define glActiveTexture_FORMAT FORMAT_void_GLenum
#define glAlphaFunc_INDEX 3
#define glAlphaFunc_RETURN void
#define glAlphaFunc_ARG_NAMES func, ref
#define glAlphaFunc_ARG_EXPAND GLenum func, GLfloat ref
#define glAlphaFunc_PACKED PACKED_void_GLenum_GLfloat
#define glAlphaFunc_INDEXED INDEXED_void_GLenum_GLfloat
#define glAlphaFunc_FORMAT FORMAT_void_GLenum_GLfloat
#define glAreTexturesResident_INDEX 4
#define glAreTexturesResident_RETURN GLboolean
#define glAreTexturesResident_ARG_NAMES n, textures, residences
#define glAreTexturesResident_ARG_EXPAND GLsizei n, const GLuint * textures, GLboolean * residences
#define glAreTexturesResident_PACKED PACKED_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__
#define glAreTexturesResident_INDEXED INDEXED_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__
#define glAreTexturesResident_FORMAT FORMAT_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__
#define glArrayElement_INDEX 5
#define glArrayElement_RETURN void
#define glArrayElement_ARG_NAMES i
#define glArrayElement_ARG_EXPAND GLint i
#define glArrayElement_PACKED PACKED_void_GLint
#define glArrayElement_INDEXED INDEXED_void_GLint
#define glArrayElement_FORMAT FORMAT_void_GLint
#define glBegin_INDEX 6
#define glBegin_RETURN void
#define glBegin_ARG_NAMES mode
#define glBegin_ARG_EXPAND GLenum mode
#define glBegin_PACKED PACKED_void_GLenum
#define glBegin_INDEXED INDEXED_void_GLenum
#define glBegin_FORMAT FORMAT_void_GLenum
#define glBeginQuery_INDEX 7
#define glBeginQuery_RETURN void
#define glBeginQuery_ARG_NAMES target, id
#define glBeginQuery_ARG_EXPAND GLenum target, GLuint id
#define glBeginQuery_PACKED PACKED_void_GLenum_GLuint
#define glBeginQuery_INDEXED INDEXED_void_GLenum_GLuint
#define glBeginQuery_FORMAT FORMAT_void_GLenum_GLuint
#define glBindBuffer_INDEX 8
#define glBindBuffer_RETURN void
#define glBindBuffer_ARG_NAMES target, buffer
#define glBindBuffer_ARG_EXPAND GLenum target, GLuint buffer
#define glBindBuffer_PACKED PACKED_void_GLenum_GLuint
#define glBindBuffer_INDEXED INDEXED_void_GLenum_GLuint
#define glBindBuffer_FORMAT FORMAT_void_GLenum_GLuint
#define glBindTexture_INDEX 9
#define glBindTexture_RETURN void
#define glBindTexture_ARG_NAMES target, texture
#define glBindTexture_ARG_EXPAND GLenum target, GLuint texture
#define glBindTexture_PACKED PACKED_void_GLenum_GLuint
#define glBindTexture_INDEXED INDEXED_void_GLenum_GLuint
#define glBindTexture_FORMAT FORMAT_void_GLenum_GLuint
#define glBitmap_INDEX 10
#define glBitmap_RETURN void
#define glBitmap_ARG_NAMES width, height, xorig, yorig, xmove, ymove, bitmap
#define glBitmap_ARG_EXPAND GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte * bitmap
#define glBitmap_PACKED PACKED_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__
#define glBitmap_INDEXED INDEXED_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__
#define glBitmap_FORMAT FORMAT_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__
#define glBlendColor_INDEX 11
#define glBlendColor_RETURN void
#define glBlendColor_ARG_NAMES red, green, blue, alpha
#define glBlendColor_ARG_EXPAND GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha
#define glBlendColor_PACKED PACKED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glBlendColor_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glBlendColor_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glBlendEquation_INDEX 12
#define glBlendEquation_RETURN void
#define glBlendEquation_ARG_NAMES mode
#define glBlendEquation_ARG_EXPAND GLenum mode
#define glBlendEquation_PACKED PACKED_void_GLenum
#define glBlendEquation_INDEXED INDEXED_void_GLenum
#define glBlendEquation_FORMAT FORMAT_void_GLenum
#define glBlendFunc_INDEX 13
#define glBlendFunc_RETURN void
#define glBlendFunc_ARG_NAMES sfactor, dfactor
#define glBlendFunc_ARG_EXPAND GLenum sfactor, GLenum dfactor
#define glBlendFunc_PACKED PACKED_void_GLenum_GLenum
#define glBlendFunc_INDEXED INDEXED_void_GLenum_GLenum
#define glBlendFunc_FORMAT FORMAT_void_GLenum_GLenum
#define glBlendFuncSeparate_INDEX 14
#define glBlendFuncSeparate_RETURN void
#define glBlendFuncSeparate_ARG_NAMES sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha
#define glBlendFuncSeparate_ARG_EXPAND GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha
#define glBlendFuncSeparate_PACKED PACKED_void_GLenum_GLenum_GLenum_GLenum
#define glBlendFuncSeparate_INDEXED INDEXED_void_GLenum_GLenum_GLenum_GLenum
#define glBlendFuncSeparate_FORMAT FORMAT_void_GLenum_GLenum_GLenum_GLenum
#define glBufferData_INDEX 15
#define glBufferData_RETURN void
#define glBufferData_ARG_NAMES target, size, data, usage
#define glBufferData_ARG_EXPAND GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage
#define glBufferData_PACKED PACKED_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum
#define glBufferData_INDEXED INDEXED_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum
#define glBufferData_FORMAT FORMAT_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum
#define glBufferSubData_INDEX 16
#define glBufferSubData_RETURN void
#define glBufferSubData_ARG_NAMES target, offset, size, data
#define glBufferSubData_ARG_EXPAND GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data
#define glBufferSubData_PACKED PACKED_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__
#define glBufferSubData_INDEXED INDEXED_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__
#define glBufferSubData_FORMAT FORMAT_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__
#define glCallList_INDEX 17
#define glCallList_RETURN void
#define glCallList_ARG_NAMES list
#define glCallList_ARG_EXPAND GLuint list
#define glCallList_PACKED PACKED_void_GLuint
#define glCallList_INDEXED INDEXED_void_GLuint
#define glCallList_FORMAT FORMAT_void_GLuint
#define glCallLists_INDEX 18
#define glCallLists_RETURN void
#define glCallLists_ARG_NAMES n, type, lists
#define glCallLists_ARG_EXPAND GLsizei n, GLenum type, const GLvoid * lists
#define glCallLists_PACKED PACKED_void_GLsizei_GLenum_const_GLvoid___GENPT__
#define glCallLists_INDEXED INDEXED_void_GLsizei_GLenum_const_GLvoid___GENPT__
#define glCallLists_FORMAT FORMAT_void_GLsizei_GLenum_const_GLvoid___GENPT__
#define glClear_INDEX 19
#define glClear_RETURN void
#define glClear_ARG_NAMES mask
#define glClear_ARG_EXPAND GLbitfield mask
#define glClear_PACKED PACKED_void_GLbitfield
#define glClear_INDEXED INDEXED_void_GLbitfield
#define glClear_FORMAT FORMAT_void_GLbitfield
#define glClearAccum_INDEX 20
#define glClearAccum_RETURN void
#define glClearAccum_ARG_NAMES red, green, blue, alpha
#define glClearAccum_ARG_EXPAND GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha
#define glClearAccum_PACKED PACKED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glClearAccum_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glClearAccum_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glClearColor_INDEX 21
#define glClearColor_RETURN void
#define glClearColor_ARG_NAMES red, green, blue, alpha
#define glClearColor_ARG_EXPAND GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha
#define glClearColor_PACKED PACKED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glClearColor_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glClearColor_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glClearDepth_INDEX 22
#define glClearDepth_RETURN void
#define glClearDepth_ARG_NAMES depth
#define glClearDepth_ARG_EXPAND GLdouble depth
#define glClearDepth_PACKED PACKED_void_GLdouble
#define glClearDepth_INDEXED INDEXED_void_GLdouble
#define glClearDepth_FORMAT FORMAT_void_GLdouble
#define glClearIndex_INDEX 23
#define glClearIndex_RETURN void
#define glClearIndex_ARG_NAMES c
#define glClearIndex_ARG_EXPAND GLfloat c
#define glClearIndex_PACKED PACKED_void_GLfloat
#define glClearIndex_INDEXED INDEXED_void_GLfloat
#define glClearIndex_FORMAT FORMAT_void_GLfloat
#define glClearStencil_INDEX 24
#define glClearStencil_RETURN void
#define glClearStencil_ARG_NAMES s
#define glClearStencil_ARG_EXPAND GLint s
#define glClearStencil_PACKED PACKED_void_GLint
#define glClearStencil_INDEXED INDEXED_void_GLint
#define glClearStencil_FORMAT FORMAT_void_GLint
#define glClientActiveTexture_INDEX 25
#define glClientActiveTexture_RETURN void
#define glClientActiveTexture_ARG_NAMES texture
#define glClientActiveTexture_ARG_EXPAND GLenum texture
#define glClientActiveTexture_PACKED PACKED_void_GLenum
#define glClientActiveTexture_INDEXED INDEXED_void_GLenum
#define glClientActiveTexture_FORMAT FORMAT_void_GLenum
#define glClipPlane_INDEX 26
#define glClipPlane_RETURN void
#define glClipPlane_ARG_NAMES plane, equation
#define glClipPlane_ARG_EXPAND GLenum plane, const GLdouble * equation
#define glClipPlane_PACKED PACKED_void_GLenum_const_GLdouble___GENPT__
#define glClipPlane_INDEXED INDEXED_void_GLenum_const_GLdouble___GENPT__
#define glClipPlane_FORMAT FORMAT_void_GLenum_const_GLdouble___GENPT__
#define glColor3b_INDEX 27
#define glColor3b_RETURN void
#define glColor3b_ARG_NAMES red, green, blue
#define glColor3b_ARG_EXPAND GLbyte red, GLbyte green, GLbyte blue
#define glColor3b_PACKED PACKED_void_GLbyte_GLbyte_GLbyte
#define glColor3b_INDEXED INDEXED_void_GLbyte_GLbyte_GLbyte
#define glColor3b_FORMAT FORMAT_void_GLbyte_GLbyte_GLbyte
#define glColor3bv_INDEX 28
#define glColor3bv_RETURN void
#define glColor3bv_ARG_NAMES v
#define glColor3bv_ARG_EXPAND const GLbyte * v
#define glColor3bv_PACKED PACKED_void_const_GLbyte___GENPT__
#define glColor3bv_INDEXED INDEXED_void_const_GLbyte___GENPT__
#define glColor3bv_FORMAT FORMAT_void_const_GLbyte___GENPT__
#define glColor3d_INDEX 29
#define glColor3d_RETURN void
#define glColor3d_ARG_NAMES red, green, blue
#define glColor3d_ARG_EXPAND GLdouble red, GLdouble green, GLdouble blue
#define glColor3d_PACKED PACKED_void_GLdouble_GLdouble_GLdouble
#define glColor3d_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble
#define glColor3d_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble
#define glColor3dv_INDEX 30
#define glColor3dv_RETURN void
#define glColor3dv_ARG_NAMES v
#define glColor3dv_ARG_EXPAND const GLdouble * v
#define glColor3dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glColor3dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glColor3dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glColor3f_INDEX 31
#define glColor3f_RETURN void
#define glColor3f_ARG_NAMES red, green, blue
#define glColor3f_ARG_EXPAND GLfloat red, GLfloat green, GLfloat blue
#define glColor3f_PACKED PACKED_void_GLfloat_GLfloat_GLfloat
#define glColor3f_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat
#define glColor3f_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat
#define glColor3fv_INDEX 32
#define glColor3fv_RETURN void
#define glColor3fv_ARG_NAMES v
#define glColor3fv_ARG_EXPAND const GLfloat * v
#define glColor3fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glColor3fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glColor3fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glColor3i_INDEX 33
#define glColor3i_RETURN void
#define glColor3i_ARG_NAMES red, green, blue
#define glColor3i_ARG_EXPAND GLint red, GLint green, GLint blue
#define glColor3i_PACKED PACKED_void_GLint_GLint_GLint
#define glColor3i_INDEXED INDEXED_void_GLint_GLint_GLint
#define glColor3i_FORMAT FORMAT_void_GLint_GLint_GLint
#define glColor3iv_INDEX 34
#define glColor3iv_RETURN void
#define glColor3iv_ARG_NAMES v
#define glColor3iv_ARG_EXPAND const GLint * v
#define glColor3iv_PACKED PACKED_void_const_GLint___GENPT__
#define glColor3iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glColor3iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glColor3s_INDEX 35
#define glColor3s_RETURN void
#define glColor3s_ARG_NAMES red, green, blue
#define glColor3s_ARG_EXPAND GLshort red, GLshort green, GLshort blue
#define glColor3s_PACKED PACKED_void_GLshort_GLshort_GLshort
#define glColor3s_INDEXED INDEXED_void_GLshort_GLshort_GLshort
#define glColor3s_FORMAT FORMAT_void_GLshort_GLshort_GLshort
#define glColor3sv_INDEX 36
#define glColor3sv_RETURN void
#define glColor3sv_ARG_NAMES v
#define glColor3sv_ARG_EXPAND const GLshort * v
#define glColor3sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glColor3sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glColor3sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glColor3ub_INDEX 37
#define glColor3ub_RETURN void
#define glColor3ub_ARG_NAMES red, green, blue
#define glColor3ub_ARG_EXPAND GLubyte red, GLubyte green, GLubyte blue
#define glColor3ub_PACKED PACKED_void_GLubyte_GLubyte_GLubyte
#define glColor3ub_INDEXED INDEXED_void_GLubyte_GLubyte_GLubyte
#define glColor3ub_FORMAT FORMAT_void_GLubyte_GLubyte_GLubyte
#define glColor3ubv_INDEX 38
#define glColor3ubv_RETURN void
#define glColor3ubv_ARG_NAMES v
#define glColor3ubv_ARG_EXPAND const GLubyte * v
#define glColor3ubv_PACKED PACKED_void_const_GLubyte___GENPT__
#define glColor3ubv_INDEXED INDEXED_void_const_GLubyte___GENPT__
#define glColor3ubv_FORMAT FORMAT_void_const_GLubyte___GENPT__
#define glColor3ui_INDEX 39
#define glColor3ui_RETURN void
#define glColor3ui_ARG_NAMES red, green, blue
#define glColor3ui_ARG_EXPAND GLuint red, GLuint green, GLuint blue
#define glColor3ui_PACKED PACKED_void_GLuint_GLuint_GLuint
#define glColor3ui_INDEXED INDEXED_void_GLuint_GLuint_GLuint
#define glColor3ui_FORMAT FORMAT_void_GLuint_GLuint_GLuint
#define glColor3uiv_INDEX 40
#define glColor3uiv_RETURN void
#define glColor3uiv_ARG_NAMES v
#define glColor3uiv_ARG_EXPAND const GLuint * v
#define glColor3uiv_PACKED PACKED_void_const_GLuint___GENPT__
#define glColor3uiv_INDEXED INDEXED_void_const_GLuint___GENPT__
#define glColor3uiv_FORMAT FORMAT_void_const_GLuint___GENPT__
#define glColor3us_INDEX 41
#define glColor3us_RETURN void
#define glColor3us_ARG_NAMES red, green, blue
#define glColor3us_ARG_EXPAND GLushort red, GLushort green, GLushort blue
#define glColor3us_PACKED PACKED_void_GLushort_GLushort_GLushort
#define glColor3us_INDEXED INDEXED_void_GLushort_GLushort_GLushort
#define glColor3us_FORMAT FORMAT_void_GLushort_GLushort_GLushort
#define glColor3usv_INDEX 42
#define glColor3usv_RETURN void
#define glColor3usv_ARG_NAMES v
#define glColor3usv_ARG_EXPAND const GLushort * v
#define glColor3usv_PACKED PACKED_void_const_GLushort___GENPT__
#define glColor3usv_INDEXED INDEXED_void_const_GLushort___GENPT__
#define glColor3usv_FORMAT FORMAT_void_const_GLushort___GENPT__
#define glColor4b_INDEX 43
#define glColor4b_RETURN void
#define glColor4b_ARG_NAMES red, green, blue, alpha
#define glColor4b_ARG_EXPAND GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha
#define glColor4b_PACKED PACKED_void_GLbyte_GLbyte_GLbyte_GLbyte
#define glColor4b_INDEXED INDEXED_void_GLbyte_GLbyte_GLbyte_GLbyte
#define glColor4b_FORMAT FORMAT_void_GLbyte_GLbyte_GLbyte_GLbyte
#define glColor4bv_INDEX 44
#define glColor4bv_RETURN void
#define glColor4bv_ARG_NAMES v
#define glColor4bv_ARG_EXPAND const GLbyte * v
#define glColor4bv_PACKED PACKED_void_const_GLbyte___GENPT__
#define glColor4bv_INDEXED INDEXED_void_const_GLbyte___GENPT__
#define glColor4bv_FORMAT FORMAT_void_const_GLbyte___GENPT__
#define glColor4d_INDEX 45
#define glColor4d_RETURN void
#define glColor4d_ARG_NAMES red, green, blue, alpha
#define glColor4d_ARG_EXPAND GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha
#define glColor4d_PACKED PACKED_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glColor4d_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glColor4d_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glColor4dv_INDEX 46
#define glColor4dv_RETURN void
#define glColor4dv_ARG_NAMES v
#define glColor4dv_ARG_EXPAND const GLdouble * v
#define glColor4dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glColor4dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glColor4dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glColor4f_INDEX 47
#define glColor4f_RETURN void
#define glColor4f_ARG_NAMES red, green, blue, alpha
#define glColor4f_ARG_EXPAND GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha
#define glColor4f_PACKED PACKED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glColor4f_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glColor4f_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glColor4fv_INDEX 48
#define glColor4fv_RETURN void
#define glColor4fv_ARG_NAMES v
#define glColor4fv_ARG_EXPAND const GLfloat * v
#define glColor4fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glColor4fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glColor4fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glColor4i_INDEX 49
#define glColor4i_RETURN void
#define glColor4i_ARG_NAMES red, green, blue, alpha
#define glColor4i_ARG_EXPAND GLint red, GLint green, GLint blue, GLint alpha
#define glColor4i_PACKED PACKED_void_GLint_GLint_GLint_GLint
#define glColor4i_INDEXED INDEXED_void_GLint_GLint_GLint_GLint
#define glColor4i_FORMAT FORMAT_void_GLint_GLint_GLint_GLint
#define glColor4iv_INDEX 50
#define glColor4iv_RETURN void
#define glColor4iv_ARG_NAMES v
#define glColor4iv_ARG_EXPAND const GLint * v
#define glColor4iv_PACKED PACKED_void_const_GLint___GENPT__
#define glColor4iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glColor4iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glColor4s_INDEX 51
#define glColor4s_RETURN void
#define glColor4s_ARG_NAMES red, green, blue, alpha
#define glColor4s_ARG_EXPAND GLshort red, GLshort green, GLshort blue, GLshort alpha
#define glColor4s_PACKED PACKED_void_GLshort_GLshort_GLshort_GLshort
#define glColor4s_INDEXED INDEXED_void_GLshort_GLshort_GLshort_GLshort
#define glColor4s_FORMAT FORMAT_void_GLshort_GLshort_GLshort_GLshort
#define glColor4sv_INDEX 52
#define glColor4sv_RETURN void
#define glColor4sv_ARG_NAMES v
#define glColor4sv_ARG_EXPAND const GLshort * v
#define glColor4sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glColor4sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glColor4sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glColor4ub_INDEX 53
#define glColor4ub_RETURN void
#define glColor4ub_ARG_NAMES red, green, blue, alpha
#define glColor4ub_ARG_EXPAND GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha
#define glColor4ub_PACKED PACKED_void_GLubyte_GLubyte_GLubyte_GLubyte
#define glColor4ub_INDEXED INDEXED_void_GLubyte_GLubyte_GLubyte_GLubyte
#define glColor4ub_FORMAT FORMAT_void_GLubyte_GLubyte_GLubyte_GLubyte
#define glColor4ubv_INDEX 54
#define glColor4ubv_RETURN void
#define glColor4ubv_ARG_NAMES v
#define glColor4ubv_ARG_EXPAND const GLubyte * v
#define glColor4ubv_PACKED PACKED_void_const_GLubyte___GENPT__
#define glColor4ubv_INDEXED INDEXED_void_const_GLubyte___GENPT__
#define glColor4ubv_FORMAT FORMAT_void_const_GLubyte___GENPT__
#define glColor4ui_INDEX 55
#define glColor4ui_RETURN void
#define glColor4ui_ARG_NAMES red, green, blue, alpha
#define glColor4ui_ARG_EXPAND GLuint red, GLuint green, GLuint blue, GLuint alpha
#define glColor4ui_PACKED PACKED_void_GLuint_GLuint_GLuint_GLuint
#define glColor4ui_INDEXED INDEXED_void_GLuint_GLuint_GLuint_GLuint
#define glColor4ui_FORMAT FORMAT_void_GLuint_GLuint_GLuint_GLuint
#define glColor4uiv_INDEX 56
#define glColor4uiv_RETURN void
#define glColor4uiv_ARG_NAMES v
#define glColor4uiv_ARG_EXPAND const GLuint * v
#define glColor4uiv_PACKED PACKED_void_const_GLuint___GENPT__
#define glColor4uiv_INDEXED INDEXED_void_const_GLuint___GENPT__
#define glColor4uiv_FORMAT FORMAT_void_const_GLuint___GENPT__
#define glColor4us_INDEX 57
#define glColor4us_RETURN void
#define glColor4us_ARG_NAMES red, green, blue, alpha
#define glColor4us_ARG_EXPAND GLushort red, GLushort green, GLushort blue, GLushort alpha
#define glColor4us_PACKED PACKED_void_GLushort_GLushort_GLushort_GLushort
#define glColor4us_INDEXED INDEXED_void_GLushort_GLushort_GLushort_GLushort
#define glColor4us_FORMAT FORMAT_void_GLushort_GLushort_GLushort_GLushort
#define glColor4usv_INDEX 58
#define glColor4usv_RETURN void
#define glColor4usv_ARG_NAMES v
#define glColor4usv_ARG_EXPAND const GLushort * v
#define glColor4usv_PACKED PACKED_void_const_GLushort___GENPT__
#define glColor4usv_INDEXED INDEXED_void_const_GLushort___GENPT__
#define glColor4usv_FORMAT FORMAT_void_const_GLushort___GENPT__
#define glColorMask_INDEX 59
#define glColorMask_RETURN void
#define glColorMask_ARG_NAMES red, green, blue, alpha
#define glColorMask_ARG_EXPAND GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha
#define glColorMask_PACKED PACKED_void_GLboolean_GLboolean_GLboolean_GLboolean
#define glColorMask_INDEXED INDEXED_void_GLboolean_GLboolean_GLboolean_GLboolean
#define glColorMask_FORMAT FORMAT_void_GLboolean_GLboolean_GLboolean_GLboolean
#define glColorMaterial_INDEX 60
#define glColorMaterial_RETURN void
#define glColorMaterial_ARG_NAMES face, mode
#define glColorMaterial_ARG_EXPAND GLenum face, GLenum mode
#define glColorMaterial_PACKED PACKED_void_GLenum_GLenum
#define glColorMaterial_INDEXED INDEXED_void_GLenum_GLenum
#define glColorMaterial_FORMAT FORMAT_void_GLenum_GLenum
#define glColorPointer_INDEX 61
#define glColorPointer_RETURN void
#define glColorPointer_ARG_NAMES size, type, stride, pointer
#define glColorPointer_ARG_EXPAND GLint size, GLenum type, GLsizei stride, const GLvoid * pointer
#define glColorPointer_PACKED PACKED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__
#define glColorPointer_INDEXED INDEXED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__
#define glColorPointer_FORMAT FORMAT_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__
#define glColorSubTable_INDEX 62
#define glColorSubTable_RETURN void
#define glColorSubTable_ARG_NAMES target, start, count, format, type, data
#define glColorSubTable_ARG_EXPAND GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid * data
#define glColorSubTable_PACKED PACKED_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glColorSubTable_INDEXED INDEXED_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glColorSubTable_FORMAT FORMAT_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glColorTable_INDEX 63
#define glColorTable_RETURN void
#define glColorTable_ARG_NAMES target, internalformat, width, format, type, table
#define glColorTable_ARG_EXPAND GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * table
#define glColorTable_PACKED PACKED_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glColorTable_INDEXED INDEXED_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glColorTable_FORMAT FORMAT_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glColorTableParameterfv_INDEX 64
#define glColorTableParameterfv_RETURN void
#define glColorTableParameterfv_ARG_NAMES target, pname, params
#define glColorTableParameterfv_ARG_EXPAND GLenum target, GLenum pname, const GLfloat * params
#define glColorTableParameterfv_PACKED PACKED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glColorTableParameterfv_INDEXED INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glColorTableParameterfv_FORMAT FORMAT_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glColorTableParameteriv_INDEX 65
#define glColorTableParameteriv_RETURN void
#define glColorTableParameteriv_ARG_NAMES target, pname, params
#define glColorTableParameteriv_ARG_EXPAND GLenum target, GLenum pname, const GLint * params
#define glColorTableParameteriv_PACKED PACKED_void_GLenum_GLenum_const_GLint___GENPT__
#define glColorTableParameteriv_INDEXED INDEXED_void_GLenum_GLenum_const_GLint___GENPT__
#define glColorTableParameteriv_FORMAT FORMAT_void_GLenum_GLenum_const_GLint___GENPT__
#define glCompressedTexImage1D_INDEX 66
#define glCompressedTexImage1D_RETURN void
#define glCompressedTexImage1D_ARG_NAMES target, level, internalformat, width, border, imageSize, data
#define glCompressedTexImage1D_ARG_EXPAND GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid * data
#define glCompressedTexImage1D_PACKED PACKED_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexImage1D_INDEXED INDEXED_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexImage1D_FORMAT FORMAT_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexImage2D_INDEX 67
#define glCompressedTexImage2D_RETURN void
#define glCompressedTexImage2D_ARG_NAMES target, level, internalformat, width, height, border, imageSize, data
#define glCompressedTexImage2D_ARG_EXPAND GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data
#define glCompressedTexImage2D_PACKED PACKED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexImage2D_INDEXED INDEXED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexImage2D_FORMAT FORMAT_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexImage3D_INDEX 68
#define glCompressedTexImage3D_RETURN void
#define glCompressedTexImage3D_ARG_NAMES target, level, internalformat, width, height, depth, border, imageSize, data
#define glCompressedTexImage3D_ARG_EXPAND GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data
#define glCompressedTexImage3D_PACKED PACKED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexImage3D_INDEXED INDEXED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexImage3D_FORMAT FORMAT_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexSubImage1D_INDEX 69
#define glCompressedTexSubImage1D_RETURN void
#define glCompressedTexSubImage1D_ARG_NAMES target, level, xoffset, width, format, imageSize, data
#define glCompressedTexSubImage1D_ARG_EXPAND GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data
#define glCompressedTexSubImage1D_PACKED PACKED_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexSubImage1D_INDEXED INDEXED_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexSubImage1D_FORMAT FORMAT_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexSubImage2D_INDEX 70
#define glCompressedTexSubImage2D_RETURN void
#define glCompressedTexSubImage2D_ARG_NAMES target, level, xoffset, yoffset, width, height, format, imageSize, data
#define glCompressedTexSubImage2D_ARG_EXPAND GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data
#define glCompressedTexSubImage2D_PACKED PACKED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexSubImage2D_INDEXED INDEXED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexSubImage2D_FORMAT FORMAT_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexSubImage3D_INDEX 71
#define glCompressedTexSubImage3D_RETURN void
#define glCompressedTexSubImage3D_ARG_NAMES target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data
#define glCompressedTexSubImage3D_ARG_EXPAND GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data
#define glCompressedTexSubImage3D_PACKED PACKED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexSubImage3D_INDEXED INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__
#define glCompressedTexSubImage3D_FORMAT FORMAT_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__
#define glConvolutionFilter1D_INDEX 72
#define glConvolutionFilter1D_RETURN void
#define glConvolutionFilter1D_ARG_NAMES target, internalformat, width, format, type, image
#define glConvolutionFilter1D_ARG_EXPAND GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * image
#define glConvolutionFilter1D_PACKED PACKED_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glConvolutionFilter1D_INDEXED INDEXED_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glConvolutionFilter1D_FORMAT FORMAT_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glConvolutionFilter2D_INDEX 73
#define glConvolutionFilter2D_RETURN void
#define glConvolutionFilter2D_ARG_NAMES target, internalformat, width, height, format, type, image
#define glConvolutionFilter2D_ARG_EXPAND GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * image
#define glConvolutionFilter2D_PACKED PACKED_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glConvolutionFilter2D_INDEXED INDEXED_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glConvolutionFilter2D_FORMAT FORMAT_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glConvolutionParameterf_INDEX 74
#define glConvolutionParameterf_RETURN void
#define glConvolutionParameterf_ARG_NAMES target, pname, params
#define glConvolutionParameterf_ARG_EXPAND GLenum target, GLenum pname, GLfloat params
#define glConvolutionParameterf_PACKED PACKED_void_GLenum_GLenum_GLfloat
#define glConvolutionParameterf_INDEXED INDEXED_void_GLenum_GLenum_GLfloat
#define glConvolutionParameterf_FORMAT FORMAT_void_GLenum_GLenum_GLfloat
#define glConvolutionParameterfv_INDEX 75
#define glConvolutionParameterfv_RETURN void
#define glConvolutionParameterfv_ARG_NAMES target, pname, params
#define glConvolutionParameterfv_ARG_EXPAND GLenum target, GLenum pname, const GLfloat * params
#define glConvolutionParameterfv_PACKED PACKED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glConvolutionParameterfv_INDEXED INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glConvolutionParameterfv_FORMAT FORMAT_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glConvolutionParameteri_INDEX 76
#define glConvolutionParameteri_RETURN void
#define glConvolutionParameteri_ARG_NAMES target, pname, params
#define glConvolutionParameteri_ARG_EXPAND GLenum target, GLenum pname, GLint params
#define glConvolutionParameteri_PACKED PACKED_void_GLenum_GLenum_GLint
#define glConvolutionParameteri_INDEXED INDEXED_void_GLenum_GLenum_GLint
#define glConvolutionParameteri_FORMAT FORMAT_void_GLenum_GLenum_GLint
#define glConvolutionParameteriv_INDEX 77
#define glConvolutionParameteriv_RETURN void
#define glConvolutionParameteriv_ARG_NAMES target, pname, params
#define glConvolutionParameteriv_ARG_EXPAND GLenum target, GLenum pname, const GLint * params
#define glConvolutionParameteriv_PACKED PACKED_void_GLenum_GLenum_const_GLint___GENPT__
#define glConvolutionParameteriv_INDEXED INDEXED_void_GLenum_GLenum_const_GLint___GENPT__
#define glConvolutionParameteriv_FORMAT FORMAT_void_GLenum_GLenum_const_GLint___GENPT__
#define glCopyColorSubTable_INDEX 78
#define glCopyColorSubTable_RETURN void
#define glCopyColorSubTable_ARG_NAMES target, start, x, y, width
#define glCopyColorSubTable_ARG_EXPAND GLenum target, GLsizei start, GLint x, GLint y, GLsizei width
#define glCopyColorSubTable_PACKED PACKED_void_GLenum_GLsizei_GLint_GLint_GLsizei
#define glCopyColorSubTable_INDEXED INDEXED_void_GLenum_GLsizei_GLint_GLint_GLsizei
#define glCopyColorSubTable_FORMAT FORMAT_void_GLenum_GLsizei_GLint_GLint_GLsizei
#define glCopyColorTable_INDEX 79
#define glCopyColorTable_RETURN void
#define glCopyColorTable_ARG_NAMES target, internalformat, x, y, width
#define glCopyColorTable_ARG_EXPAND GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width
#define glCopyColorTable_PACKED PACKED_void_GLenum_GLenum_GLint_GLint_GLsizei
#define glCopyColorTable_INDEXED INDEXED_void_GLenum_GLenum_GLint_GLint_GLsizei
#define glCopyColorTable_FORMAT FORMAT_void_GLenum_GLenum_GLint_GLint_GLsizei
#define glCopyConvolutionFilter1D_INDEX 80
#define glCopyConvolutionFilter1D_RETURN void
#define glCopyConvolutionFilter1D_ARG_NAMES target, internalformat, x, y, width
#define glCopyConvolutionFilter1D_ARG_EXPAND GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width
#define glCopyConvolutionFilter1D_PACKED PACKED_void_GLenum_GLenum_GLint_GLint_GLsizei
#define glCopyConvolutionFilter1D_INDEXED INDEXED_void_GLenum_GLenum_GLint_GLint_GLsizei
#define glCopyConvolutionFilter1D_FORMAT FORMAT_void_GLenum_GLenum_GLint_GLint_GLsizei
#define glCopyConvolutionFilter2D_INDEX 81
#define glCopyConvolutionFilter2D_RETURN void
#define glCopyConvolutionFilter2D_ARG_NAMES target, internalformat, x, y, width, height
#define glCopyConvolutionFilter2D_ARG_EXPAND GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height
#define glCopyConvolutionFilter2D_PACKED PACKED_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei
#define glCopyConvolutionFilter2D_INDEXED INDEXED_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei
#define glCopyConvolutionFilter2D_FORMAT FORMAT_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei
#define glCopyPixels_INDEX 82
#define glCopyPixels_RETURN void
#define glCopyPixels_ARG_NAMES x, y, width, height, type
#define glCopyPixels_ARG_EXPAND GLint x, GLint y, GLsizei width, GLsizei height, GLenum type
#define glCopyPixels_PACKED PACKED_void_GLint_GLint_GLsizei_GLsizei_GLenum
#define glCopyPixels_INDEXED INDEXED_void_GLint_GLint_GLsizei_GLsizei_GLenum
#define glCopyPixels_FORMAT FORMAT_void_GLint_GLint_GLsizei_GLsizei_GLenum
#define glCopyTexImage1D_INDEX 83
#define glCopyTexImage1D_RETURN void
#define glCopyTexImage1D_ARG_NAMES target, level, internalformat, x, y, width, border
#define glCopyTexImage1D_ARG_EXPAND GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border
#define glCopyTexImage1D_PACKED PACKED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint
#define glCopyTexImage1D_INDEXED INDEXED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint
#define glCopyTexImage1D_FORMAT FORMAT_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint
#define glCopyTexImage2D_INDEX 84
#define glCopyTexImage2D_RETURN void
#define glCopyTexImage2D_ARG_NAMES target, level, internalformat, x, y, width, height, border
#define glCopyTexImage2D_ARG_EXPAND GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border
#define glCopyTexImage2D_PACKED PACKED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint
#define glCopyTexImage2D_INDEXED INDEXED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint
#define glCopyTexImage2D_FORMAT FORMAT_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint
#define glCopyTexSubImage1D_INDEX 85
#define glCopyTexSubImage1D_RETURN void
#define glCopyTexSubImage1D_ARG_NAMES target, level, xoffset, x, y, width
#define glCopyTexSubImage1D_ARG_EXPAND GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width
#define glCopyTexSubImage1D_PACKED PACKED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei
#define glCopyTexSubImage1D_INDEXED INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei
#define glCopyTexSubImage1D_FORMAT FORMAT_void_GLenum_GLint_GLint_GLint_GLint_GLsizei
#define glCopyTexSubImage2D_INDEX 86
#define glCopyTexSubImage2D_RETURN void
#define glCopyTexSubImage2D_ARG_NAMES target, level, xoffset, yoffset, x, y, width, height
#define glCopyTexSubImage2D_ARG_EXPAND GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height
#define glCopyTexSubImage2D_PACKED PACKED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei
#define glCopyTexSubImage2D_INDEXED INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei
#define glCopyTexSubImage2D_FORMAT FORMAT_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei
#define glCopyTexSubImage3D_INDEX 87
#define glCopyTexSubImage3D_RETURN void
#define glCopyTexSubImage3D_ARG_NAMES target, level, xoffset, yoffset, zoffset, x, y, width, height
#define glCopyTexSubImage3D_ARG_EXPAND GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height
#define glCopyTexSubImage3D_PACKED PACKED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei
#define glCopyTexSubImage3D_INDEXED INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei
#define glCopyTexSubImage3D_FORMAT FORMAT_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei
#define glCullFace_INDEX 88
#define glCullFace_RETURN void
#define glCullFace_ARG_NAMES mode
#define glCullFace_ARG_EXPAND GLenum mode
#define glCullFace_PACKED PACKED_void_GLenum
#define glCullFace_INDEXED INDEXED_void_GLenum
#define glCullFace_FORMAT FORMAT_void_GLenum
#define glDeleteBuffers_INDEX 89
#define glDeleteBuffers_RETURN void
#define glDeleteBuffers_ARG_NAMES n, buffers
#define glDeleteBuffers_ARG_EXPAND GLsizei n, const GLuint * buffers
#define glDeleteBuffers_PACKED PACKED_void_GLsizei_const_GLuint___GENPT__
#define glDeleteBuffers_INDEXED INDEXED_void_GLsizei_const_GLuint___GENPT__
#define glDeleteBuffers_FORMAT FORMAT_void_GLsizei_const_GLuint___GENPT__
#define glDeleteLists_INDEX 90
#define glDeleteLists_RETURN void
#define glDeleteLists_ARG_NAMES list, range
#define glDeleteLists_ARG_EXPAND GLuint list, GLsizei range
#define glDeleteLists_PACKED PACKED_void_GLuint_GLsizei
#define glDeleteLists_INDEXED INDEXED_void_GLuint_GLsizei
#define glDeleteLists_FORMAT FORMAT_void_GLuint_GLsizei
#define glDeleteQueries_INDEX 91
#define glDeleteQueries_RETURN void
#define glDeleteQueries_ARG_NAMES n, ids
#define glDeleteQueries_ARG_EXPAND GLsizei n, const GLuint * ids
#define glDeleteQueries_PACKED PACKED_void_GLsizei_const_GLuint___GENPT__
#define glDeleteQueries_INDEXED INDEXED_void_GLsizei_const_GLuint___GENPT__
#define glDeleteQueries_FORMAT FORMAT_void_GLsizei_const_GLuint___GENPT__
#define glDeleteTextures_INDEX 92
#define glDeleteTextures_RETURN void
#define glDeleteTextures_ARG_NAMES n, textures
#define glDeleteTextures_ARG_EXPAND GLsizei n, const GLuint * textures
#define glDeleteTextures_PACKED PACKED_void_GLsizei_const_GLuint___GENPT__
#define glDeleteTextures_INDEXED INDEXED_void_GLsizei_const_GLuint___GENPT__
#define glDeleteTextures_FORMAT FORMAT_void_GLsizei_const_GLuint___GENPT__
#define glDepthFunc_INDEX 93
#define glDepthFunc_RETURN void
#define glDepthFunc_ARG_NAMES func
#define glDepthFunc_ARG_EXPAND GLenum func
#define glDepthFunc_PACKED PACKED_void_GLenum
#define glDepthFunc_INDEXED INDEXED_void_GLenum
#define glDepthFunc_FORMAT FORMAT_void_GLenum
#define glDepthMask_INDEX 94
#define glDepthMask_RETURN void
#define glDepthMask_ARG_NAMES flag
#define glDepthMask_ARG_EXPAND GLboolean flag
#define glDepthMask_PACKED PACKED_void_GLboolean
#define glDepthMask_INDEXED INDEXED_void_GLboolean
#define glDepthMask_FORMAT FORMAT_void_GLboolean
#define glDepthRange_INDEX 95
#define glDepthRange_RETURN void
#define glDepthRange_ARG_NAMES near, far
#define glDepthRange_ARG_EXPAND GLdouble near, GLdouble far
#define glDepthRange_PACKED PACKED_void_GLdouble_GLdouble
#define glDepthRange_INDEXED INDEXED_void_GLdouble_GLdouble
#define glDepthRange_FORMAT FORMAT_void_GLdouble_GLdouble
#define glDisable_INDEX 96
#define glDisable_RETURN void
#define glDisable_ARG_NAMES cap
#define glDisable_ARG_EXPAND GLenum cap
#define glDisable_PACKED PACKED_void_GLenum
#define glDisable_INDEXED INDEXED_void_GLenum
#define glDisable_FORMAT FORMAT_void_GLenum
#define glDisableClientState_INDEX 97
#define glDisableClientState_RETURN void
#define glDisableClientState_ARG_NAMES array
#define glDisableClientState_ARG_EXPAND GLenum array
#define glDisableClientState_PACKED PACKED_void_GLenum
#define glDisableClientState_INDEXED INDEXED_void_GLenum
#define glDisableClientState_FORMAT FORMAT_void_GLenum
#define glDrawArrays_INDEX 98
#define glDrawArrays_RETURN void
#define glDrawArrays_ARG_NAMES mode, first, count
#define glDrawArrays_ARG_EXPAND GLenum mode, GLint first, GLsizei count
#define glDrawArrays_PACKED PACKED_void_GLenum_GLint_GLsizei
#define glDrawArrays_INDEXED INDEXED_void_GLenum_GLint_GLsizei
#define glDrawArrays_FORMAT FORMAT_void_GLenum_GLint_GLsizei
#define glDrawBuffer_INDEX 99
#define glDrawBuffer_RETURN void
#define glDrawBuffer_ARG_NAMES mode
#define glDrawBuffer_ARG_EXPAND GLenum mode
#define glDrawBuffer_PACKED PACKED_void_GLenum
#define glDrawBuffer_INDEXED INDEXED_void_GLenum
#define glDrawBuffer_FORMAT FORMAT_void_GLenum
#define glDrawElements_INDEX 100
#define glDrawElements_RETURN void
#define glDrawElements_ARG_NAMES mode, count, type, indices
#define glDrawElements_ARG_EXPAND GLenum mode, GLsizei count, GLenum type, const GLvoid * indices
#define glDrawElements_PACKED PACKED_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__
#define glDrawElements_INDEXED INDEXED_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__
#define glDrawElements_FORMAT FORMAT_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__
#define glDrawPixels_INDEX 101
#define glDrawPixels_RETURN void
#define glDrawPixels_ARG_NAMES width, height, format, type, pixels
#define glDrawPixels_ARG_EXPAND GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels
#define glDrawPixels_PACKED PACKED_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glDrawPixels_INDEXED INDEXED_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glDrawPixels_FORMAT FORMAT_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glDrawRangeElements_INDEX 102
#define glDrawRangeElements_RETURN void
#define glDrawRangeElements_ARG_NAMES mode, start, end, count, type, indices
#define glDrawRangeElements_ARG_EXPAND GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices
#define glDrawRangeElements_PACKED PACKED_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__
#define glDrawRangeElements_INDEXED INDEXED_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__
#define glDrawRangeElements_FORMAT FORMAT_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__
#define glEdgeFlag_INDEX 103
#define glEdgeFlag_RETURN void
#define glEdgeFlag_ARG_NAMES flag
#define glEdgeFlag_ARG_EXPAND GLboolean flag
#define glEdgeFlag_PACKED PACKED_void_GLboolean
#define glEdgeFlag_INDEXED INDEXED_void_GLboolean
#define glEdgeFlag_FORMAT FORMAT_void_GLboolean
#define glEdgeFlagPointer_INDEX 104
#define glEdgeFlagPointer_RETURN void
#define glEdgeFlagPointer_ARG_NAMES stride, pointer
#define glEdgeFlagPointer_ARG_EXPAND GLsizei stride, const GLvoid * pointer
#define glEdgeFlagPointer_PACKED PACKED_void_GLsizei_const_GLvoid___GENPT__
#define glEdgeFlagPointer_INDEXED INDEXED_void_GLsizei_const_GLvoid___GENPT__
#define glEdgeFlagPointer_FORMAT FORMAT_void_GLsizei_const_GLvoid___GENPT__
#define glEdgeFlagv_INDEX 105
#define glEdgeFlagv_RETURN void
#define glEdgeFlagv_ARG_NAMES flag
#define glEdgeFlagv_ARG_EXPAND const GLboolean * flag
#define glEdgeFlagv_PACKED PACKED_void_const_GLboolean___GENPT__
#define glEdgeFlagv_INDEXED INDEXED_void_const_GLboolean___GENPT__
#define glEdgeFlagv_FORMAT FORMAT_void_const_GLboolean___GENPT__
#define glEnable_INDEX 106
#define glEnable_RETURN void
#define glEnable_ARG_NAMES cap
#define glEnable_ARG_EXPAND GLenum cap
#define glEnable_PACKED PACKED_void_GLenum
#define glEnable_INDEXED INDEXED_void_GLenum
#define glEnable_FORMAT FORMAT_void_GLenum
#define glEnableClientState_INDEX 107
#define glEnableClientState_RETURN void
#define glEnableClientState_ARG_NAMES array
#define glEnableClientState_ARG_EXPAND GLenum array
#define glEnableClientState_PACKED PACKED_void_GLenum
#define glEnableClientState_INDEXED INDEXED_void_GLenum
#define glEnableClientState_FORMAT FORMAT_void_GLenum
#define glEnd_INDEX 108
#define glEnd_RETURN void
#define glEnd_ARG_NAMES 
#define glEnd_ARG_EXPAND 
#define glEnd_PACKED PACKED_void
#define glEnd_INDEXED INDEXED_void
#define glEnd_FORMAT FORMAT_void
#define glEndList_INDEX 109
#define glEndList_RETURN void
#define glEndList_ARG_NAMES 
#define glEndList_ARG_EXPAND 
#define glEndList_PACKED PACKED_void
#define glEndList_INDEXED INDEXED_void
#define glEndList_FORMAT FORMAT_void
#define glEndQuery_INDEX 110
#define glEndQuery_RETURN void
#define glEndQuery_ARG_NAMES target
#define glEndQuery_ARG_EXPAND GLenum target
#define glEndQuery_PACKED PACKED_void_GLenum
#define glEndQuery_INDEXED INDEXED_void_GLenum
#define glEndQuery_FORMAT FORMAT_void_GLenum
#define glEvalCoord1d_INDEX 111
#define glEvalCoord1d_RETURN void
#define glEvalCoord1d_ARG_NAMES u
#define glEvalCoord1d_ARG_EXPAND GLdouble u
#define glEvalCoord1d_PACKED PACKED_void_GLdouble
#define glEvalCoord1d_INDEXED INDEXED_void_GLdouble
#define glEvalCoord1d_FORMAT FORMAT_void_GLdouble
#define glEvalCoord1dv_INDEX 112
#define glEvalCoord1dv_RETURN void
#define glEvalCoord1dv_ARG_NAMES u
#define glEvalCoord1dv_ARG_EXPAND const GLdouble * u
#define glEvalCoord1dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glEvalCoord1dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glEvalCoord1dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glEvalCoord1f_INDEX 113
#define glEvalCoord1f_RETURN void
#define glEvalCoord1f_ARG_NAMES u
#define glEvalCoord1f_ARG_EXPAND GLfloat u
#define glEvalCoord1f_PACKED PACKED_void_GLfloat
#define glEvalCoord1f_INDEXED INDEXED_void_GLfloat
#define glEvalCoord1f_FORMAT FORMAT_void_GLfloat
#define glEvalCoord1fv_INDEX 114
#define glEvalCoord1fv_RETURN void
#define glEvalCoord1fv_ARG_NAMES u
#define glEvalCoord1fv_ARG_EXPAND const GLfloat * u
#define glEvalCoord1fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glEvalCoord1fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glEvalCoord1fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glEvalCoord2d_INDEX 115
#define glEvalCoord2d_RETURN void
#define glEvalCoord2d_ARG_NAMES u, v
#define glEvalCoord2d_ARG_EXPAND GLdouble u, GLdouble v
#define glEvalCoord2d_PACKED PACKED_void_GLdouble_GLdouble
#define glEvalCoord2d_INDEXED INDEXED_void_GLdouble_GLdouble
#define glEvalCoord2d_FORMAT FORMAT_void_GLdouble_GLdouble
#define glEvalCoord2dv_INDEX 116
#define glEvalCoord2dv_RETURN void
#define glEvalCoord2dv_ARG_NAMES u
#define glEvalCoord2dv_ARG_EXPAND const GLdouble * u
#define glEvalCoord2dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glEvalCoord2dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glEvalCoord2dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glEvalCoord2f_INDEX 117
#define glEvalCoord2f_RETURN void
#define glEvalCoord2f_ARG_NAMES u, v
#define glEvalCoord2f_ARG_EXPAND GLfloat u, GLfloat v
#define glEvalCoord2f_PACKED PACKED_void_GLfloat_GLfloat
#define glEvalCoord2f_INDEXED INDEXED_void_GLfloat_GLfloat
#define glEvalCoord2f_FORMAT FORMAT_void_GLfloat_GLfloat
#define glEvalCoord2fv_INDEX 118
#define glEvalCoord2fv_RETURN void
#define glEvalCoord2fv_ARG_NAMES u
#define glEvalCoord2fv_ARG_EXPAND const GLfloat * u
#define glEvalCoord2fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glEvalCoord2fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glEvalCoord2fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glEvalMesh1_INDEX 119
#define glEvalMesh1_RETURN void
#define glEvalMesh1_ARG_NAMES mode, i1, i2
#define glEvalMesh1_ARG_EXPAND GLenum mode, GLint i1, GLint i2
#define glEvalMesh1_PACKED PACKED_void_GLenum_GLint_GLint
#define glEvalMesh1_INDEXED INDEXED_void_GLenum_GLint_GLint
#define glEvalMesh1_FORMAT FORMAT_void_GLenum_GLint_GLint
#define glEvalMesh2_INDEX 120
#define glEvalMesh2_RETURN void
#define glEvalMesh2_ARG_NAMES mode, i1, i2, j1, j2
#define glEvalMesh2_ARG_EXPAND GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2
#define glEvalMesh2_PACKED PACKED_void_GLenum_GLint_GLint_GLint_GLint
#define glEvalMesh2_INDEXED INDEXED_void_GLenum_GLint_GLint_GLint_GLint
#define glEvalMesh2_FORMAT FORMAT_void_GLenum_GLint_GLint_GLint_GLint
#define glEvalPoint1_INDEX 121
#define glEvalPoint1_RETURN void
#define glEvalPoint1_ARG_NAMES i
#define glEvalPoint1_ARG_EXPAND GLint i
#define glEvalPoint1_PACKED PACKED_void_GLint
#define glEvalPoint1_INDEXED INDEXED_void_GLint
#define glEvalPoint1_FORMAT FORMAT_void_GLint
#define glEvalPoint2_INDEX 122
#define glEvalPoint2_RETURN void
#define glEvalPoint2_ARG_NAMES i, j
#define glEvalPoint2_ARG_EXPAND GLint i, GLint j
#define glEvalPoint2_PACKED PACKED_void_GLint_GLint
#define glEvalPoint2_INDEXED INDEXED_void_GLint_GLint
#define glEvalPoint2_FORMAT FORMAT_void_GLint_GLint
#define glFeedbackBuffer_INDEX 123
#define glFeedbackBuffer_RETURN void
#define glFeedbackBuffer_ARG_NAMES size, type, buffer
#define glFeedbackBuffer_ARG_EXPAND GLsizei size, GLenum type, GLfloat * buffer
#define glFeedbackBuffer_PACKED PACKED_void_GLsizei_GLenum_GLfloat___GENPT__
#define glFeedbackBuffer_INDEXED INDEXED_void_GLsizei_GLenum_GLfloat___GENPT__
#define glFeedbackBuffer_FORMAT FORMAT_void_GLsizei_GLenum_GLfloat___GENPT__
#define glFinish_INDEX 124
#define glFinish_RETURN void
#define glFinish_ARG_NAMES 
#define glFinish_ARG_EXPAND 
#define glFinish_PACKED PACKED_void
#define glFinish_INDEXED INDEXED_void
#define glFinish_FORMAT FORMAT_void
#define glFlush_INDEX 125
#define glFlush_RETURN void
#define glFlush_ARG_NAMES 
#define glFlush_ARG_EXPAND 
#define glFlush_PACKED PACKED_void
#define glFlush_INDEXED INDEXED_void
#define glFlush_FORMAT FORMAT_void
#define glFogCoordPointer_INDEX 126
#define glFogCoordPointer_RETURN void
#define glFogCoordPointer_ARG_NAMES type, stride, pointer
#define glFogCoordPointer_ARG_EXPAND GLenum type, GLsizei stride, const GLvoid * pointer
#define glFogCoordPointer_PACKED PACKED_void_GLenum_GLsizei_const_GLvoid___GENPT__
#define glFogCoordPointer_INDEXED INDEXED_void_GLenum_GLsizei_const_GLvoid___GENPT__
#define glFogCoordPointer_FORMAT FORMAT_void_GLenum_GLsizei_const_GLvoid___GENPT__
#define glFogCoordd_INDEX 127
#define glFogCoordd_RETURN void
#define glFogCoordd_ARG_NAMES coord
#define glFogCoordd_ARG_EXPAND GLdouble coord
#define glFogCoordd_PACKED PACKED_void_GLdouble
#define glFogCoordd_INDEXED INDEXED_void_GLdouble
#define glFogCoordd_FORMAT FORMAT_void_GLdouble
#define glFogCoorddv_INDEX 128
#define glFogCoorddv_RETURN void
#define glFogCoorddv_ARG_NAMES coord
#define glFogCoorddv_ARG_EXPAND const GLdouble * coord
#define glFogCoorddv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glFogCoorddv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glFogCoorddv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glFogCoordf_INDEX 129
#define glFogCoordf_RETURN void
#define glFogCoordf_ARG_NAMES coord
#define glFogCoordf_ARG_EXPAND GLfloat coord
#define glFogCoordf_PACKED PACKED_void_GLfloat
#define glFogCoordf_INDEXED INDEXED_void_GLfloat
#define glFogCoordf_FORMAT FORMAT_void_GLfloat
#define glFogCoordfv_INDEX 130
#define glFogCoordfv_RETURN void
#define glFogCoordfv_ARG_NAMES coord
#define glFogCoordfv_ARG_EXPAND const GLfloat * coord
#define glFogCoordfv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glFogCoordfv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glFogCoordfv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glFogf_INDEX 131
#define glFogf_RETURN void
#define glFogf_ARG_NAMES pname, param
#define glFogf_ARG_EXPAND GLenum pname, GLfloat param
#define glFogf_PACKED PACKED_void_GLenum_GLfloat
#define glFogf_INDEXED INDEXED_void_GLenum_GLfloat
#define glFogf_FORMAT FORMAT_void_GLenum_GLfloat
#define glFogfv_INDEX 132
#define glFogfv_RETURN void
#define glFogfv_ARG_NAMES pname, params
#define glFogfv_ARG_EXPAND GLenum pname, const GLfloat * params
#define glFogfv_PACKED PACKED_void_GLenum_const_GLfloat___GENPT__
#define glFogfv_INDEXED INDEXED_void_GLenum_const_GLfloat___GENPT__
#define glFogfv_FORMAT FORMAT_void_GLenum_const_GLfloat___GENPT__
#define glFogi_INDEX 133
#define glFogi_RETURN void
#define glFogi_ARG_NAMES pname, param
#define glFogi_ARG_EXPAND GLenum pname, GLint param
#define glFogi_PACKED PACKED_void_GLenum_GLint
#define glFogi_INDEXED INDEXED_void_GLenum_GLint
#define glFogi_FORMAT FORMAT_void_GLenum_GLint
#define glFogiv_INDEX 134
#define glFogiv_RETURN void
#define glFogiv_ARG_NAMES pname, params
#define glFogiv_ARG_EXPAND GLenum pname, const GLint * params
#define glFogiv_PACKED PACKED_void_GLenum_const_GLint___GENPT__
#define glFogiv_INDEXED INDEXED_void_GLenum_const_GLint___GENPT__
#define glFogiv_FORMAT FORMAT_void_GLenum_const_GLint___GENPT__
#define glFrontFace_INDEX 135
#define glFrontFace_RETURN void
#define glFrontFace_ARG_NAMES mode
#define glFrontFace_ARG_EXPAND GLenum mode
#define glFrontFace_PACKED PACKED_void_GLenum
#define glFrontFace_INDEXED INDEXED_void_GLenum
#define glFrontFace_FORMAT FORMAT_void_GLenum
#define glFrustum_INDEX 136
#define glFrustum_RETURN void
#define glFrustum_ARG_NAMES left, right, bottom, top, zNear, zFar
#define glFrustum_ARG_EXPAND GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar
#define glFrustum_PACKED PACKED_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble
#define glFrustum_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble
#define glFrustum_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble
#define glGenBuffers_INDEX 137
#define glGenBuffers_RETURN void
#define glGenBuffers_ARG_NAMES n, buffers
#define glGenBuffers_ARG_EXPAND GLsizei n, GLuint * buffers
#define glGenBuffers_PACKED PACKED_void_GLsizei_GLuint___GENPT__
#define glGenBuffers_INDEXED INDEXED_void_GLsizei_GLuint___GENPT__
#define glGenBuffers_FORMAT FORMAT_void_GLsizei_GLuint___GENPT__
#define glGenLists_INDEX 138
#define glGenLists_RETURN GLuint
#define glGenLists_ARG_NAMES range
#define glGenLists_ARG_EXPAND GLsizei range
#define glGenLists_PACKED PACKED_GLuint_GLsizei
#define glGenLists_INDEXED INDEXED_GLuint_GLsizei
#define glGenLists_FORMAT FORMAT_GLuint_GLsizei
#define glGenQueries_INDEX 139
#define glGenQueries_RETURN void
#define glGenQueries_ARG_NAMES n, ids
#define glGenQueries_ARG_EXPAND GLsizei n, GLuint * ids
#define glGenQueries_PACKED PACKED_void_GLsizei_GLuint___GENPT__
#define glGenQueries_INDEXED INDEXED_void_GLsizei_GLuint___GENPT__
#define glGenQueries_FORMAT FORMAT_void_GLsizei_GLuint___GENPT__
#define glGenTextures_INDEX 140
#define glGenTextures_RETURN void
#define glGenTextures_ARG_NAMES n, textures
#define glGenTextures_ARG_EXPAND GLsizei n, GLuint * textures
#define glGenTextures_PACKED PACKED_void_GLsizei_GLuint___GENPT__
#define glGenTextures_INDEXED INDEXED_void_GLsizei_GLuint___GENPT__
#define glGenTextures_FORMAT FORMAT_void_GLsizei_GLuint___GENPT__
#define glGetBooleanv_INDEX 141
#define glGetBooleanv_RETURN void
#define glGetBooleanv_ARG_NAMES pname, params
#define glGetBooleanv_ARG_EXPAND GLenum pname, GLboolean * params
#define glGetBooleanv_PACKED PACKED_void_GLenum_GLboolean___GENPT__
#define glGetBooleanv_INDEXED INDEXED_void_GLenum_GLboolean___GENPT__
#define glGetBooleanv_FORMAT FORMAT_void_GLenum_GLboolean___GENPT__
#define glGetBufferParameteriv_INDEX 142
#define glGetBufferParameteriv_RETURN void
#define glGetBufferParameteriv_ARG_NAMES target, pname, params
#define glGetBufferParameteriv_ARG_EXPAND GLenum target, GLenum pname, GLint * params
#define glGetBufferParameteriv_PACKED PACKED_void_GLenum_GLenum_GLint___GENPT__
#define glGetBufferParameteriv_INDEXED INDEXED_void_GLenum_GLenum_GLint___GENPT__
#define glGetBufferParameteriv_FORMAT FORMAT_void_GLenum_GLenum_GLint___GENPT__
#define glGetBufferPointerv_INDEX 143
#define glGetBufferPointerv_RETURN void
#define glGetBufferPointerv_ARG_NAMES target, pname, params
#define glGetBufferPointerv_ARG_EXPAND GLenum target, GLenum pname, GLvoid * params
#define glGetBufferPointerv_PACKED PACKED_void_GLenum_GLenum_GLvoid___GENPT__
#define glGetBufferPointerv_INDEXED INDEXED_void_GLenum_GLenum_GLvoid___GENPT__
#define glGetBufferPointerv_FORMAT FORMAT_void_GLenum_GLenum_GLvoid___GENPT__
#define glGetBufferSubData_INDEX 144
#define glGetBufferSubData_RETURN void
#define glGetBufferSubData_ARG_NAMES target, offset, size, data
#define glGetBufferSubData_ARG_EXPAND GLenum target, GLintptr offset, GLsizeiptr size, GLvoid * data
#define glGetBufferSubData_PACKED PACKED_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__
#define glGetBufferSubData_INDEXED INDEXED_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__
#define glGetBufferSubData_FORMAT FORMAT_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__
#define glGetClipPlane_INDEX 145
#define glGetClipPlane_RETURN void
#define glGetClipPlane_ARG_NAMES plane, equation
#define glGetClipPlane_ARG_EXPAND GLenum plane, GLdouble * equation
#define glGetClipPlane_PACKED PACKED_void_GLenum_GLdouble___GENPT__
#define glGetClipPlane_INDEXED INDEXED_void_GLenum_GLdouble___GENPT__
#define glGetClipPlane_FORMAT FORMAT_void_GLenum_GLdouble___GENPT__
#define glGetColorTable_INDEX 146
#define glGetColorTable_RETURN void
#define glGetColorTable_ARG_NAMES target, format, type, table
#define glGetColorTable_ARG_EXPAND GLenum target, GLenum format, GLenum type, GLvoid * table
#define glGetColorTable_PACKED PACKED_void_GLenum_GLenum_GLenum_GLvoid___GENPT__
#define glGetColorTable_INDEXED INDEXED_void_GLenum_GLenum_GLenum_GLvoid___GENPT__
#define glGetColorTable_FORMAT FORMAT_void_GLenum_GLenum_GLenum_GLvoid___GENPT__
#define glGetColorTableParameterfv_INDEX 147
#define glGetColorTableParameterfv_RETURN void
#define glGetColorTableParameterfv_ARG_NAMES target, pname, params
#define glGetColorTableParameterfv_ARG_EXPAND GLenum target, GLenum pname, GLfloat * params
#define glGetColorTableParameterfv_PACKED PACKED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetColorTableParameterfv_INDEXED INDEXED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetColorTableParameterfv_FORMAT FORMAT_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetColorTableParameteriv_INDEX 148
#define glGetColorTableParameteriv_RETURN void
#define glGetColorTableParameteriv_ARG_NAMES target, pname, params
#define glGetColorTableParameteriv_ARG_EXPAND GLenum target, GLenum pname, GLint * params
#define glGetColorTableParameteriv_PACKED PACKED_void_GLenum_GLenum_GLint___GENPT__
#define glGetColorTableParameteriv_INDEXED INDEXED_void_GLenum_GLenum_GLint___GENPT__
#define glGetColorTableParameteriv_FORMAT FORMAT_void_GLenum_GLenum_GLint___GENPT__
#define glGetCompressedTexImage_INDEX 149
#define glGetCompressedTexImage_RETURN void
#define glGetCompressedTexImage_ARG_NAMES target, level, img
#define glGetCompressedTexImage_ARG_EXPAND GLenum target, GLint level, GLvoid * img
#define glGetCompressedTexImage_PACKED PACKED_void_GLenum_GLint_GLvoid___GENPT__
#define glGetCompressedTexImage_INDEXED INDEXED_void_GLenum_GLint_GLvoid___GENPT__
#define glGetCompressedTexImage_FORMAT FORMAT_void_GLenum_GLint_GLvoid___GENPT__
#define glGetConvolutionFilter_INDEX 150
#define glGetConvolutionFilter_RETURN void
#define glGetConvolutionFilter_ARG_NAMES target, format, type, image
#define glGetConvolutionFilter_ARG_EXPAND GLenum target, GLenum format, GLenum type, GLvoid * image
#define glGetConvolutionFilter_PACKED PACKED_void_GLenum_GLenum_GLenum_GLvoid___GENPT__
#define glGetConvolutionFilter_INDEXED INDEXED_void_GLenum_GLenum_GLenum_GLvoid___GENPT__
#define glGetConvolutionFilter_FORMAT FORMAT_void_GLenum_GLenum_GLenum_GLvoid___GENPT__
#define glGetConvolutionParameterfv_INDEX 151
#define glGetConvolutionParameterfv_RETURN void
#define glGetConvolutionParameterfv_ARG_NAMES target, pname, params
#define glGetConvolutionParameterfv_ARG_EXPAND GLenum target, GLenum pname, GLfloat * params
#define glGetConvolutionParameterfv_PACKED PACKED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetConvolutionParameterfv_INDEXED INDEXED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetConvolutionParameterfv_FORMAT FORMAT_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetConvolutionParameteriv_INDEX 152
#define glGetConvolutionParameteriv_RETURN void
#define glGetConvolutionParameteriv_ARG_NAMES target, pname, params
#define glGetConvolutionParameteriv_ARG_EXPAND GLenum target, GLenum pname, GLint * params
#define glGetConvolutionParameteriv_PACKED PACKED_void_GLenum_GLenum_GLint___GENPT__
#define glGetConvolutionParameteriv_INDEXED INDEXED_void_GLenum_GLenum_GLint___GENPT__
#define glGetConvolutionParameteriv_FORMAT FORMAT_void_GLenum_GLenum_GLint___GENPT__
#define glGetDoublev_INDEX 153
#define glGetDoublev_RETURN void
#define glGetDoublev_ARG_NAMES pname, params
#define glGetDoublev_ARG_EXPAND GLenum pname, GLdouble * params
#define glGetDoublev_PACKED PACKED_void_GLenum_GLdouble___GENPT__
#define glGetDoublev_INDEXED INDEXED_void_GLenum_GLdouble___GENPT__
#define glGetDoublev_FORMAT FORMAT_void_GLenum_GLdouble___GENPT__
#define glGetError_INDEX 154
#define glGetError_RETURN GLenum
#define glGetError_ARG_NAMES 
#define glGetError_ARG_EXPAND 
#define glGetError_PACKED PACKED_GLenum
#define glGetError_INDEXED INDEXED_GLenum
#define glGetError_FORMAT FORMAT_GLenum
#define glGetFloatv_INDEX 155
#define glGetFloatv_RETURN void
#define glGetFloatv_ARG_NAMES pname, params
#define glGetFloatv_ARG_EXPAND GLenum pname, GLfloat * params
#define glGetFloatv_PACKED PACKED_void_GLenum_GLfloat___GENPT__
#define glGetFloatv_INDEXED INDEXED_void_GLenum_GLfloat___GENPT__
#define glGetFloatv_FORMAT FORMAT_void_GLenum_GLfloat___GENPT__
#define glGetHistogram_INDEX 156
#define glGetHistogram_RETURN void
#define glGetHistogram_ARG_NAMES target, reset, format, type, values
#define glGetHistogram_ARG_EXPAND GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values
#define glGetHistogram_PACKED PACKED_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__
#define glGetHistogram_INDEXED INDEXED_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__
#define glGetHistogram_FORMAT FORMAT_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__
#define glGetHistogramParameterfv_INDEX 157
#define glGetHistogramParameterfv_RETURN void
#define glGetHistogramParameterfv_ARG_NAMES target, pname, params
#define glGetHistogramParameterfv_ARG_EXPAND GLenum target, GLenum pname, GLfloat * params
#define glGetHistogramParameterfv_PACKED PACKED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetHistogramParameterfv_INDEXED INDEXED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetHistogramParameterfv_FORMAT FORMAT_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetHistogramParameteriv_INDEX 158
#define glGetHistogramParameteriv_RETURN void
#define glGetHistogramParameteriv_ARG_NAMES target, pname, params
#define glGetHistogramParameteriv_ARG_EXPAND GLenum target, GLenum pname, GLint * params
#define glGetHistogramParameteriv_PACKED PACKED_void_GLenum_GLenum_GLint___GENPT__
#define glGetHistogramParameteriv_INDEXED INDEXED_void_GLenum_GLenum_GLint___GENPT__
#define glGetHistogramParameteriv_FORMAT FORMAT_void_GLenum_GLenum_GLint___GENPT__
#define glGetIntegerv_INDEX 159
#define glGetIntegerv_RETURN void
#define glGetIntegerv_ARG_NAMES pname, params
#define glGetIntegerv_ARG_EXPAND GLenum pname, GLint * params
#define glGetIntegerv_PACKED PACKED_void_GLenum_GLint___GENPT__
#define glGetIntegerv_INDEXED INDEXED_void_GLenum_GLint___GENPT__
#define glGetIntegerv_FORMAT FORMAT_void_GLenum_GLint___GENPT__
#define glGetLightfv_INDEX 160
#define glGetLightfv_RETURN void
#define glGetLightfv_ARG_NAMES light, pname, params
#define glGetLightfv_ARG_EXPAND GLenum light, GLenum pname, GLfloat * params
#define glGetLightfv_PACKED PACKED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetLightfv_INDEXED INDEXED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetLightfv_FORMAT FORMAT_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetLightiv_INDEX 161
#define glGetLightiv_RETURN void
#define glGetLightiv_ARG_NAMES light, pname, params
#define glGetLightiv_ARG_EXPAND GLenum light, GLenum pname, GLint * params
#define glGetLightiv_PACKED PACKED_void_GLenum_GLenum_GLint___GENPT__
#define glGetLightiv_INDEXED INDEXED_void_GLenum_GLenum_GLint___GENPT__
#define glGetLightiv_FORMAT FORMAT_void_GLenum_GLenum_GLint___GENPT__
#define glGetMapdv_INDEX 162
#define glGetMapdv_RETURN void
#define glGetMapdv_ARG_NAMES target, query, v
#define glGetMapdv_ARG_EXPAND GLenum target, GLenum query, GLdouble * v
#define glGetMapdv_PACKED PACKED_void_GLenum_GLenum_GLdouble___GENPT__
#define glGetMapdv_INDEXED INDEXED_void_GLenum_GLenum_GLdouble___GENPT__
#define glGetMapdv_FORMAT FORMAT_void_GLenum_GLenum_GLdouble___GENPT__
#define glGetMapfv_INDEX 163
#define glGetMapfv_RETURN void
#define glGetMapfv_ARG_NAMES target, query, v
#define glGetMapfv_ARG_EXPAND GLenum target, GLenum query, GLfloat * v
#define glGetMapfv_PACKED PACKED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetMapfv_INDEXED INDEXED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetMapfv_FORMAT FORMAT_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetMapiv_INDEX 164
#define glGetMapiv_RETURN void
#define glGetMapiv_ARG_NAMES target, query, v
#define glGetMapiv_ARG_EXPAND GLenum target, GLenum query, GLint * v
#define glGetMapiv_PACKED PACKED_void_GLenum_GLenum_GLint___GENPT__
#define glGetMapiv_INDEXED INDEXED_void_GLenum_GLenum_GLint___GENPT__
#define glGetMapiv_FORMAT FORMAT_void_GLenum_GLenum_GLint___GENPT__
#define glGetMaterialfv_INDEX 165
#define glGetMaterialfv_RETURN void
#define glGetMaterialfv_ARG_NAMES face, pname, params
#define glGetMaterialfv_ARG_EXPAND GLenum face, GLenum pname, GLfloat * params
#define glGetMaterialfv_PACKED PACKED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetMaterialfv_INDEXED INDEXED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetMaterialfv_FORMAT FORMAT_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetMaterialiv_INDEX 166
#define glGetMaterialiv_RETURN void
#define glGetMaterialiv_ARG_NAMES face, pname, params
#define glGetMaterialiv_ARG_EXPAND GLenum face, GLenum pname, GLint * params
#define glGetMaterialiv_PACKED PACKED_void_GLenum_GLenum_GLint___GENPT__
#define glGetMaterialiv_INDEXED INDEXED_void_GLenum_GLenum_GLint___GENPT__
#define glGetMaterialiv_FORMAT FORMAT_void_GLenum_GLenum_GLint___GENPT__
#define glGetMinmax_INDEX 167
#define glGetMinmax_RETURN void
#define glGetMinmax_ARG_NAMES target, reset, format, type, values
#define glGetMinmax_ARG_EXPAND GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values
#define glGetMinmax_PACKED PACKED_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__
#define glGetMinmax_INDEXED INDEXED_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__
#define glGetMinmax_FORMAT FORMAT_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__
#define glGetMinmaxParameterfv_INDEX 168
#define glGetMinmaxParameterfv_RETURN void
#define glGetMinmaxParameterfv_ARG_NAMES target, pname, params
#define glGetMinmaxParameterfv_ARG_EXPAND GLenum target, GLenum pname, GLfloat * params
#define glGetMinmaxParameterfv_PACKED PACKED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetMinmaxParameterfv_INDEXED INDEXED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetMinmaxParameterfv_FORMAT FORMAT_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetMinmaxParameteriv_INDEX 169
#define glGetMinmaxParameteriv_RETURN void
#define glGetMinmaxParameteriv_ARG_NAMES target, pname, params
#define glGetMinmaxParameteriv_ARG_EXPAND GLenum target, GLenum pname, GLint * params
#define glGetMinmaxParameteriv_PACKED PACKED_void_GLenum_GLenum_GLint___GENPT__
#define glGetMinmaxParameteriv_INDEXED INDEXED_void_GLenum_GLenum_GLint___GENPT__
#define glGetMinmaxParameteriv_FORMAT FORMAT_void_GLenum_GLenum_GLint___GENPT__
#define glGetPixelMapfv_INDEX 170
#define glGetPixelMapfv_RETURN void
#define glGetPixelMapfv_ARG_NAMES map, values
#define glGetPixelMapfv_ARG_EXPAND GLenum map, GLfloat * values
#define glGetPixelMapfv_PACKED PACKED_void_GLenum_GLfloat___GENPT__
#define glGetPixelMapfv_INDEXED INDEXED_void_GLenum_GLfloat___GENPT__
#define glGetPixelMapfv_FORMAT FORMAT_void_GLenum_GLfloat___GENPT__
#define glGetPixelMapuiv_INDEX 171
#define glGetPixelMapuiv_RETURN void
#define glGetPixelMapuiv_ARG_NAMES map, values
#define glGetPixelMapuiv_ARG_EXPAND GLenum map, GLuint * values
#define glGetPixelMapuiv_PACKED PACKED_void_GLenum_GLuint___GENPT__
#define glGetPixelMapuiv_INDEXED INDEXED_void_GLenum_GLuint___GENPT__
#define glGetPixelMapuiv_FORMAT FORMAT_void_GLenum_GLuint___GENPT__
#define glGetPixelMapusv_INDEX 172
#define glGetPixelMapusv_RETURN void
#define glGetPixelMapusv_ARG_NAMES map, values
#define glGetPixelMapusv_ARG_EXPAND GLenum map, GLushort * values
#define glGetPixelMapusv_PACKED PACKED_void_GLenum_GLushort___GENPT__
#define glGetPixelMapusv_INDEXED INDEXED_void_GLenum_GLushort___GENPT__
#define glGetPixelMapusv_FORMAT FORMAT_void_GLenum_GLushort___GENPT__
#define glGetPointerv_INDEX 173
#define glGetPointerv_RETURN void
#define glGetPointerv_ARG_NAMES pname, params
#define glGetPointerv_ARG_EXPAND GLenum pname, GLvoid ** params
#define glGetPointerv_PACKED PACKED_void_GLenum_GLvoid___GENPT____GENPT__
#define glGetPointerv_INDEXED INDEXED_void_GLenum_GLvoid___GENPT____GENPT__
#define glGetPointerv_FORMAT FORMAT_void_GLenum_GLvoid___GENPT____GENPT__
#define glGetPolygonStipple_INDEX 174
#define glGetPolygonStipple_RETURN void
#define glGetPolygonStipple_ARG_NAMES mask
#define glGetPolygonStipple_ARG_EXPAND GLubyte * mask
#define glGetPolygonStipple_PACKED PACKED_void_GLubyte___GENPT__
#define glGetPolygonStipple_INDEXED INDEXED_void_GLubyte___GENPT__
#define glGetPolygonStipple_FORMAT FORMAT_void_GLubyte___GENPT__
#define glGetQueryObjectiv_INDEX 175
#define glGetQueryObjectiv_RETURN void
#define glGetQueryObjectiv_ARG_NAMES id, pname, params
#define glGetQueryObjectiv_ARG_EXPAND GLuint id, GLenum pname, GLint * params
#define glGetQueryObjectiv_PACKED PACKED_void_GLuint_GLenum_GLint___GENPT__
#define glGetQueryObjectiv_INDEXED INDEXED_void_GLuint_GLenum_GLint___GENPT__
#define glGetQueryObjectiv_FORMAT FORMAT_void_GLuint_GLenum_GLint___GENPT__
#define glGetQueryObjectuiv_INDEX 176
#define glGetQueryObjectuiv_RETURN void
#define glGetQueryObjectuiv_ARG_NAMES id, pname, params
#define glGetQueryObjectuiv_ARG_EXPAND GLuint id, GLenum pname, GLuint * params
#define glGetQueryObjectuiv_PACKED PACKED_void_GLuint_GLenum_GLuint___GENPT__
#define glGetQueryObjectuiv_INDEXED INDEXED_void_GLuint_GLenum_GLuint___GENPT__
#define glGetQueryObjectuiv_FORMAT FORMAT_void_GLuint_GLenum_GLuint___GENPT__
#define glGetQueryiv_INDEX 177
#define glGetQueryiv_RETURN void
#define glGetQueryiv_ARG_NAMES target, pname, params
#define glGetQueryiv_ARG_EXPAND GLenum target, GLenum pname, GLint * params
#define glGetQueryiv_PACKED PACKED_void_GLenum_GLenum_GLint___GENPT__
#define glGetQueryiv_INDEXED INDEXED_void_GLenum_GLenum_GLint___GENPT__
#define glGetQueryiv_FORMAT FORMAT_void_GLenum_GLenum_GLint___GENPT__
#define glGetSeparableFilter_INDEX 178
#define glGetSeparableFilter_RETURN void
#define glGetSeparableFilter_ARG_NAMES target, format, type, row, column, span
#define glGetSeparableFilter_ARG_EXPAND GLenum target, GLenum format, GLenum type, GLvoid * row, GLvoid * column, GLvoid * span
#define glGetSeparableFilter_PACKED PACKED_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__
#define glGetSeparableFilter_INDEXED INDEXED_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__
#define glGetSeparableFilter_FORMAT FORMAT_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__
#define glGetString_INDEX 179
#define glGetString_RETURN const GLubyte *
#define glGetString_ARG_NAMES name
#define glGetString_ARG_EXPAND GLenum name
#define glGetString_PACKED PACKED_const_GLubyte___GENPT___GLenum
#define glGetString_INDEXED INDEXED_const_GLubyte___GENPT___GLenum
#define glGetString_FORMAT FORMAT_const_GLubyte___GENPT___GLenum
#define glGetTexEnvfv_INDEX 180
#define glGetTexEnvfv_RETURN void
#define glGetTexEnvfv_ARG_NAMES target, pname, params
#define glGetTexEnvfv_ARG_EXPAND GLenum target, GLenum pname, GLfloat * params
#define glGetTexEnvfv_PACKED PACKED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetTexEnvfv_INDEXED INDEXED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetTexEnvfv_FORMAT FORMAT_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetTexEnviv_INDEX 181
#define glGetTexEnviv_RETURN void
#define glGetTexEnviv_ARG_NAMES target, pname, params
#define glGetTexEnviv_ARG_EXPAND GLenum target, GLenum pname, GLint * params
#define glGetTexEnviv_PACKED PACKED_void_GLenum_GLenum_GLint___GENPT__
#define glGetTexEnviv_INDEXED INDEXED_void_GLenum_GLenum_GLint___GENPT__
#define glGetTexEnviv_FORMAT FORMAT_void_GLenum_GLenum_GLint___GENPT__
#define glGetTexGendv_INDEX 182
#define glGetTexGendv_RETURN void
#define glGetTexGendv_ARG_NAMES coord, pname, params
#define glGetTexGendv_ARG_EXPAND GLenum coord, GLenum pname, GLdouble * params
#define glGetTexGendv_PACKED PACKED_void_GLenum_GLenum_GLdouble___GENPT__
#define glGetTexGendv_INDEXED INDEXED_void_GLenum_GLenum_GLdouble___GENPT__
#define glGetTexGendv_FORMAT FORMAT_void_GLenum_GLenum_GLdouble___GENPT__
#define glGetTexGenfv_INDEX 183
#define glGetTexGenfv_RETURN void
#define glGetTexGenfv_ARG_NAMES coord, pname, params
#define glGetTexGenfv_ARG_EXPAND GLenum coord, GLenum pname, GLfloat * params
#define glGetTexGenfv_PACKED PACKED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetTexGenfv_INDEXED INDEXED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetTexGenfv_FORMAT FORMAT_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetTexGeniv_INDEX 184
#define glGetTexGeniv_RETURN void
#define glGetTexGeniv_ARG_NAMES coord, pname, params
#define glGetTexGeniv_ARG_EXPAND GLenum coord, GLenum pname, GLint * params
#define glGetTexGeniv_PACKED PACKED_void_GLenum_GLenum_GLint___GENPT__
#define glGetTexGeniv_INDEXED INDEXED_void_GLenum_GLenum_GLint___GENPT__
#define glGetTexGeniv_FORMAT FORMAT_void_GLenum_GLenum_GLint___GENPT__
#define glGetTexImage_INDEX 185
#define glGetTexImage_RETURN void
#define glGetTexImage_ARG_NAMES target, level, format, type, pixels
#define glGetTexImage_ARG_EXPAND GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels
#define glGetTexImage_PACKED PACKED_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__
#define glGetTexImage_INDEXED INDEXED_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__
#define glGetTexImage_FORMAT FORMAT_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__
#define glGetTexLevelParameterfv_INDEX 186
#define glGetTexLevelParameterfv_RETURN void
#define glGetTexLevelParameterfv_ARG_NAMES target, level, pname, params
#define glGetTexLevelParameterfv_ARG_EXPAND GLenum target, GLint level, GLenum pname, GLfloat * params
#define glGetTexLevelParameterfv_PACKED PACKED_void_GLenum_GLint_GLenum_GLfloat___GENPT__
#define glGetTexLevelParameterfv_INDEXED INDEXED_void_GLenum_GLint_GLenum_GLfloat___GENPT__
#define glGetTexLevelParameterfv_FORMAT FORMAT_void_GLenum_GLint_GLenum_GLfloat___GENPT__
#define glGetTexLevelParameteriv_INDEX 187
#define glGetTexLevelParameteriv_RETURN void
#define glGetTexLevelParameteriv_ARG_NAMES target, level, pname, params
#define glGetTexLevelParameteriv_ARG_EXPAND GLenum target, GLint level, GLenum pname, GLint * params
#define glGetTexLevelParameteriv_PACKED PACKED_void_GLenum_GLint_GLenum_GLint___GENPT__
#define glGetTexLevelParameteriv_INDEXED INDEXED_void_GLenum_GLint_GLenum_GLint___GENPT__
#define glGetTexLevelParameteriv_FORMAT FORMAT_void_GLenum_GLint_GLenum_GLint___GENPT__
#define glGetTexParameterfv_INDEX 188
#define glGetTexParameterfv_RETURN void
#define glGetTexParameterfv_ARG_NAMES target, pname, params
#define glGetTexParameterfv_ARG_EXPAND GLenum target, GLenum pname, GLfloat * params
#define glGetTexParameterfv_PACKED PACKED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetTexParameterfv_INDEXED INDEXED_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetTexParameterfv_FORMAT FORMAT_void_GLenum_GLenum_GLfloat___GENPT__
#define glGetTexParameteriv_INDEX 189
#define glGetTexParameteriv_RETURN void
#define glGetTexParameteriv_ARG_NAMES target, pname, params
#define glGetTexParameteriv_ARG_EXPAND GLenum target, GLenum pname, GLint * params
#define glGetTexParameteriv_PACKED PACKED_void_GLenum_GLenum_GLint___GENPT__
#define glGetTexParameteriv_INDEXED INDEXED_void_GLenum_GLenum_GLint___GENPT__
#define glGetTexParameteriv_FORMAT FORMAT_void_GLenum_GLenum_GLint___GENPT__
#define glHint_INDEX 190
#define glHint_RETURN void
#define glHint_ARG_NAMES target, mode
#define glHint_ARG_EXPAND GLenum target, GLenum mode
#define glHint_PACKED PACKED_void_GLenum_GLenum
#define glHint_INDEXED INDEXED_void_GLenum_GLenum
#define glHint_FORMAT FORMAT_void_GLenum_GLenum
#define glHistogram_INDEX 191
#define glHistogram_RETURN void
#define glHistogram_ARG_NAMES target, width, internalformat, sink
#define glHistogram_ARG_EXPAND GLenum target, GLsizei width, GLenum internalformat, GLboolean sink
#define glHistogram_PACKED PACKED_void_GLenum_GLsizei_GLenum_GLboolean
#define glHistogram_INDEXED INDEXED_void_GLenum_GLsizei_GLenum_GLboolean
#define glHistogram_FORMAT FORMAT_void_GLenum_GLsizei_GLenum_GLboolean
#define glIndexMask_INDEX 192
#define glIndexMask_RETURN void
#define glIndexMask_ARG_NAMES mask
#define glIndexMask_ARG_EXPAND GLuint mask
#define glIndexMask_PACKED PACKED_void_GLuint
#define glIndexMask_INDEXED INDEXED_void_GLuint
#define glIndexMask_FORMAT FORMAT_void_GLuint
#define glIndexPointer_INDEX 193
#define glIndexPointer_RETURN void
#define glIndexPointer_ARG_NAMES type, stride, pointer
#define glIndexPointer_ARG_EXPAND GLenum type, GLsizei stride, const GLvoid * pointer
#define glIndexPointer_PACKED PACKED_void_GLenum_GLsizei_const_GLvoid___GENPT__
#define glIndexPointer_INDEXED INDEXED_void_GLenum_GLsizei_const_GLvoid___GENPT__
#define glIndexPointer_FORMAT FORMAT_void_GLenum_GLsizei_const_GLvoid___GENPT__
#define glIndexd_INDEX 194
#define glIndexd_RETURN void
#define glIndexd_ARG_NAMES c
#define glIndexd_ARG_EXPAND GLdouble c
#define glIndexd_PACKED PACKED_void_GLdouble
#define glIndexd_INDEXED INDEXED_void_GLdouble
#define glIndexd_FORMAT FORMAT_void_GLdouble
#define glIndexdv_INDEX 195
#define glIndexdv_RETURN void
#define glIndexdv_ARG_NAMES c
#define glIndexdv_ARG_EXPAND const GLdouble * c
#define glIndexdv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glIndexdv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glIndexdv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glIndexf_INDEX 196
#define glIndexf_RETURN void
#define glIndexf_ARG_NAMES c
#define glIndexf_ARG_EXPAND GLfloat c
#define glIndexf_PACKED PACKED_void_GLfloat
#define glIndexf_INDEXED INDEXED_void_GLfloat
#define glIndexf_FORMAT FORMAT_void_GLfloat
#define glIndexfv_INDEX 197
#define glIndexfv_RETURN void
#define glIndexfv_ARG_NAMES c
#define glIndexfv_ARG_EXPAND const GLfloat * c
#define glIndexfv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glIndexfv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glIndexfv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glIndexi_INDEX 198
#define glIndexi_RETURN void
#define glIndexi_ARG_NAMES c
#define glIndexi_ARG_EXPAND GLint c
#define glIndexi_PACKED PACKED_void_GLint
#define glIndexi_INDEXED INDEXED_void_GLint
#define glIndexi_FORMAT FORMAT_void_GLint
#define glIndexiv_INDEX 199
#define glIndexiv_RETURN void
#define glIndexiv_ARG_NAMES c
#define glIndexiv_ARG_EXPAND const GLint * c
#define glIndexiv_PACKED PACKED_void_const_GLint___GENPT__
#define glIndexiv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glIndexiv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glIndexs_INDEX 200
#define glIndexs_RETURN void
#define glIndexs_ARG_NAMES c
#define glIndexs_ARG_EXPAND GLshort c
#define glIndexs_PACKED PACKED_void_GLshort
#define glIndexs_INDEXED INDEXED_void_GLshort
#define glIndexs_FORMAT FORMAT_void_GLshort
#define glIndexsv_INDEX 201
#define glIndexsv_RETURN void
#define glIndexsv_ARG_NAMES c
#define glIndexsv_ARG_EXPAND const GLshort * c
#define glIndexsv_PACKED PACKED_void_const_GLshort___GENPT__
#define glIndexsv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glIndexsv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glIndexub_INDEX 202
#define glIndexub_RETURN void
#define glIndexub_ARG_NAMES c
#define glIndexub_ARG_EXPAND GLubyte c
#define glIndexub_PACKED PACKED_void_GLubyte
#define glIndexub_INDEXED INDEXED_void_GLubyte
#define glIndexub_FORMAT FORMAT_void_GLubyte
#define glIndexubv_INDEX 203
#define glIndexubv_RETURN void
#define glIndexubv_ARG_NAMES c
#define glIndexubv_ARG_EXPAND const GLubyte * c
#define glIndexubv_PACKED PACKED_void_const_GLubyte___GENPT__
#define glIndexubv_INDEXED INDEXED_void_const_GLubyte___GENPT__
#define glIndexubv_FORMAT FORMAT_void_const_GLubyte___GENPT__
#define glInitNames_INDEX 204
#define glInitNames_RETURN void
#define glInitNames_ARG_NAMES 
#define glInitNames_ARG_EXPAND 
#define glInitNames_PACKED PACKED_void
#define glInitNames_INDEXED INDEXED_void
#define glInitNames_FORMAT FORMAT_void
#define glInterleavedArrays_INDEX 205
#define glInterleavedArrays_RETURN void
#define glInterleavedArrays_ARG_NAMES format, stride, pointer
#define glInterleavedArrays_ARG_EXPAND GLenum format, GLsizei stride, const GLvoid * pointer
#define glInterleavedArrays_PACKED PACKED_void_GLenum_GLsizei_const_GLvoid___GENPT__
#define glInterleavedArrays_INDEXED INDEXED_void_GLenum_GLsizei_const_GLvoid___GENPT__
#define glInterleavedArrays_FORMAT FORMAT_void_GLenum_GLsizei_const_GLvoid___GENPT__
#define glIsBuffer_INDEX 206
#define glIsBuffer_RETURN GLboolean
#define glIsBuffer_ARG_NAMES buffer
#define glIsBuffer_ARG_EXPAND GLuint buffer
#define glIsBuffer_PACKED PACKED_GLboolean_GLuint
#define glIsBuffer_INDEXED INDEXED_GLboolean_GLuint
#define glIsBuffer_FORMAT FORMAT_GLboolean_GLuint
#define glIsEnabled_INDEX 207
#define glIsEnabled_RETURN GLboolean
#define glIsEnabled_ARG_NAMES cap
#define glIsEnabled_ARG_EXPAND GLenum cap
#define glIsEnabled_PACKED PACKED_GLboolean_GLenum
#define glIsEnabled_INDEXED INDEXED_GLboolean_GLenum
#define glIsEnabled_FORMAT FORMAT_GLboolean_GLenum
#define glIsList_INDEX 208
#define glIsList_RETURN GLboolean
#define glIsList_ARG_NAMES list
#define glIsList_ARG_EXPAND GLuint list
#define glIsList_PACKED PACKED_GLboolean_GLuint
#define glIsList_INDEXED INDEXED_GLboolean_GLuint
#define glIsList_FORMAT FORMAT_GLboolean_GLuint
#define glIsQuery_INDEX 209
#define glIsQuery_RETURN GLboolean
#define glIsQuery_ARG_NAMES id
#define glIsQuery_ARG_EXPAND GLuint id
#define glIsQuery_PACKED PACKED_GLboolean_GLuint
#define glIsQuery_INDEXED INDEXED_GLboolean_GLuint
#define glIsQuery_FORMAT FORMAT_GLboolean_GLuint
#define glIsTexture_INDEX 210
#define glIsTexture_RETURN GLboolean
#define glIsTexture_ARG_NAMES texture
#define glIsTexture_ARG_EXPAND GLuint texture
#define glIsTexture_PACKED PACKED_GLboolean_GLuint
#define glIsTexture_INDEXED INDEXED_GLboolean_GLuint
#define glIsTexture_FORMAT FORMAT_GLboolean_GLuint
#define glLightModelf_INDEX 211
#define glLightModelf_RETURN void
#define glLightModelf_ARG_NAMES pname, param
#define glLightModelf_ARG_EXPAND GLenum pname, GLfloat param
#define glLightModelf_PACKED PACKED_void_GLenum_GLfloat
#define glLightModelf_INDEXED INDEXED_void_GLenum_GLfloat
#define glLightModelf_FORMAT FORMAT_void_GLenum_GLfloat
#define glLightModelfv_INDEX 212
#define glLightModelfv_RETURN void
#define glLightModelfv_ARG_NAMES pname, params
#define glLightModelfv_ARG_EXPAND GLenum pname, const GLfloat * params
#define glLightModelfv_PACKED PACKED_void_GLenum_const_GLfloat___GENPT__
#define glLightModelfv_INDEXED INDEXED_void_GLenum_const_GLfloat___GENPT__
#define glLightModelfv_FORMAT FORMAT_void_GLenum_const_GLfloat___GENPT__
#define glLightModeli_INDEX 213
#define glLightModeli_RETURN void
#define glLightModeli_ARG_NAMES pname, param
#define glLightModeli_ARG_EXPAND GLenum pname, GLint param
#define glLightModeli_PACKED PACKED_void_GLenum_GLint
#define glLightModeli_INDEXED INDEXED_void_GLenum_GLint
#define glLightModeli_FORMAT FORMAT_void_GLenum_GLint
#define glLightModeliv_INDEX 214
#define glLightModeliv_RETURN void
#define glLightModeliv_ARG_NAMES pname, params
#define glLightModeliv_ARG_EXPAND GLenum pname, const GLint * params
#define glLightModeliv_PACKED PACKED_void_GLenum_const_GLint___GENPT__
#define glLightModeliv_INDEXED INDEXED_void_GLenum_const_GLint___GENPT__
#define glLightModeliv_FORMAT FORMAT_void_GLenum_const_GLint___GENPT__
#define glLightf_INDEX 215
#define glLightf_RETURN void
#define glLightf_ARG_NAMES light, pname, param
#define glLightf_ARG_EXPAND GLenum light, GLenum pname, GLfloat param
#define glLightf_PACKED PACKED_void_GLenum_GLenum_GLfloat
#define glLightf_INDEXED INDEXED_void_GLenum_GLenum_GLfloat
#define glLightf_FORMAT FORMAT_void_GLenum_GLenum_GLfloat
#define glLightfv_INDEX 216
#define glLightfv_RETURN void
#define glLightfv_ARG_NAMES light, pname, params
#define glLightfv_ARG_EXPAND GLenum light, GLenum pname, const GLfloat * params
#define glLightfv_PACKED PACKED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glLightfv_INDEXED INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glLightfv_FORMAT FORMAT_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glLighti_INDEX 217
#define glLighti_RETURN void
#define glLighti_ARG_NAMES light, pname, param
#define glLighti_ARG_EXPAND GLenum light, GLenum pname, GLint param
#define glLighti_PACKED PACKED_void_GLenum_GLenum_GLint
#define glLighti_INDEXED INDEXED_void_GLenum_GLenum_GLint
#define glLighti_FORMAT FORMAT_void_GLenum_GLenum_GLint
#define glLightiv_INDEX 218
#define glLightiv_RETURN void
#define glLightiv_ARG_NAMES light, pname, params
#define glLightiv_ARG_EXPAND GLenum light, GLenum pname, const GLint * params
#define glLightiv_PACKED PACKED_void_GLenum_GLenum_const_GLint___GENPT__
#define glLightiv_INDEXED INDEXED_void_GLenum_GLenum_const_GLint___GENPT__
#define glLightiv_FORMAT FORMAT_void_GLenum_GLenum_const_GLint___GENPT__
#define glLineStipple_INDEX 219
#define glLineStipple_RETURN void
#define glLineStipple_ARG_NAMES factor, pattern
#define glLineStipple_ARG_EXPAND GLint factor, GLushort pattern
#define glLineStipple_PACKED PACKED_void_GLint_GLushort
#define glLineStipple_INDEXED INDEXED_void_GLint_GLushort
#define glLineStipple_FORMAT FORMAT_void_GLint_GLushort
#define glLineWidth_INDEX 220
#define glLineWidth_RETURN void
#define glLineWidth_ARG_NAMES width
#define glLineWidth_ARG_EXPAND GLfloat width
#define glLineWidth_PACKED PACKED_void_GLfloat
#define glLineWidth_INDEXED INDEXED_void_GLfloat
#define glLineWidth_FORMAT FORMAT_void_GLfloat
#define glListBase_INDEX 221
#define glListBase_RETURN void
#define glListBase_ARG_NAMES base
#define glListBase_ARG_EXPAND GLuint base
#define glListBase_PACKED PACKED_void_GLuint
#define glListBase_INDEXED INDEXED_void_GLuint
#define glListBase_FORMAT FORMAT_void_GLuint
#define glLoadIdentity_INDEX 222
#define glLoadIdentity_RETURN void
#define glLoadIdentity_ARG_NAMES 
#define glLoadIdentity_ARG_EXPAND 
#define glLoadIdentity_PACKED PACKED_void
#define glLoadIdentity_INDEXED INDEXED_void
#define glLoadIdentity_FORMAT FORMAT_void
#define glLoadMatrixd_INDEX 223
#define glLoadMatrixd_RETURN void
#define glLoadMatrixd_ARG_NAMES m
#define glLoadMatrixd_ARG_EXPAND const GLdouble * m
#define glLoadMatrixd_PACKED PACKED_void_const_GLdouble___GENPT__
#define glLoadMatrixd_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glLoadMatrixd_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glLoadMatrixf_INDEX 224
#define glLoadMatrixf_RETURN void
#define glLoadMatrixf_ARG_NAMES m
#define glLoadMatrixf_ARG_EXPAND const GLfloat * m
#define glLoadMatrixf_PACKED PACKED_void_const_GLfloat___GENPT__
#define glLoadMatrixf_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glLoadMatrixf_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glLoadName_INDEX 225
#define glLoadName_RETURN void
#define glLoadName_ARG_NAMES name
#define glLoadName_ARG_EXPAND GLuint name
#define glLoadName_PACKED PACKED_void_GLuint
#define glLoadName_INDEXED INDEXED_void_GLuint
#define glLoadName_FORMAT FORMAT_void_GLuint
#define glLoadTransposeMatrixd_INDEX 226
#define glLoadTransposeMatrixd_RETURN void
#define glLoadTransposeMatrixd_ARG_NAMES m
#define glLoadTransposeMatrixd_ARG_EXPAND const GLdouble * m
#define glLoadTransposeMatrixd_PACKED PACKED_void_const_GLdouble___GENPT__
#define glLoadTransposeMatrixd_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glLoadTransposeMatrixd_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glLoadTransposeMatrixf_INDEX 227
#define glLoadTransposeMatrixf_RETURN void
#define glLoadTransposeMatrixf_ARG_NAMES m
#define glLoadTransposeMatrixf_ARG_EXPAND const GLfloat * m
#define glLoadTransposeMatrixf_PACKED PACKED_void_const_GLfloat___GENPT__
#define glLoadTransposeMatrixf_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glLoadTransposeMatrixf_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glLogicOp_INDEX 228
#define glLogicOp_RETURN void
#define glLogicOp_ARG_NAMES opcode
#define glLogicOp_ARG_EXPAND GLenum opcode
#define glLogicOp_PACKED PACKED_void_GLenum
#define glLogicOp_INDEXED INDEXED_void_GLenum
#define glLogicOp_FORMAT FORMAT_void_GLenum
#define glMap1d_INDEX 229
#define glMap1d_RETURN void
#define glMap1d_ARG_NAMES target, u1, u2, stride, order, points
#define glMap1d_ARG_EXPAND GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble * points
#define glMap1d_PACKED PACKED_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__
#define glMap1d_INDEXED INDEXED_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__
#define glMap1d_FORMAT FORMAT_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__
#define glMap1f_INDEX 230
#define glMap1f_RETURN void
#define glMap1f_ARG_NAMES target, u1, u2, stride, order, points
#define glMap1f_ARG_EXPAND GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat * points
#define glMap1f_PACKED PACKED_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__
#define glMap1f_INDEXED INDEXED_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__
#define glMap1f_FORMAT FORMAT_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__
#define glMap2d_INDEX 231
#define glMap2d_RETURN void
#define glMap2d_ARG_NAMES target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points
#define glMap2d_ARG_EXPAND GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble * points
#define glMap2d_PACKED PACKED_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__
#define glMap2d_INDEXED INDEXED_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__
#define glMap2d_FORMAT FORMAT_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__
#define glMap2f_INDEX 232
#define glMap2f_RETURN void
#define glMap2f_ARG_NAMES target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points
#define glMap2f_ARG_EXPAND GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat * points
#define glMap2f_PACKED PACKED_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__
#define glMap2f_INDEXED INDEXED_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__
#define glMap2f_FORMAT FORMAT_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__
#define glMapBuffer_INDEX 233
#define glMapBuffer_RETURN GLvoid *
#define glMapBuffer_ARG_NAMES target, access
#define glMapBuffer_ARG_EXPAND GLenum target, GLenum access
#define glMapBuffer_PACKED PACKED_GLvoid___GENPT___GLenum_GLenum
#define glMapBuffer_INDEXED INDEXED_GLvoid___GENPT___GLenum_GLenum
#define glMapBuffer_FORMAT FORMAT_GLvoid___GENPT___GLenum_GLenum
#define glMapGrid1d_INDEX 234
#define glMapGrid1d_RETURN void
#define glMapGrid1d_ARG_NAMES un, u1, u2
#define glMapGrid1d_ARG_EXPAND GLint un, GLdouble u1, GLdouble u2
#define glMapGrid1d_PACKED PACKED_void_GLint_GLdouble_GLdouble
#define glMapGrid1d_INDEXED INDEXED_void_GLint_GLdouble_GLdouble
#define glMapGrid1d_FORMAT FORMAT_void_GLint_GLdouble_GLdouble
#define glMapGrid1f_INDEX 235
#define glMapGrid1f_RETURN void
#define glMapGrid1f_ARG_NAMES un, u1, u2
#define glMapGrid1f_ARG_EXPAND GLint un, GLfloat u1, GLfloat u2
#define glMapGrid1f_PACKED PACKED_void_GLint_GLfloat_GLfloat
#define glMapGrid1f_INDEXED INDEXED_void_GLint_GLfloat_GLfloat
#define glMapGrid1f_FORMAT FORMAT_void_GLint_GLfloat_GLfloat
#define glMapGrid2d_INDEX 236
#define glMapGrid2d_RETURN void
#define glMapGrid2d_ARG_NAMES un, u1, u2, vn, v1, v2
#define glMapGrid2d_ARG_EXPAND GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2
#define glMapGrid2d_PACKED PACKED_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble
#define glMapGrid2d_INDEXED INDEXED_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble
#define glMapGrid2d_FORMAT FORMAT_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble
#define glMapGrid2f_INDEX 237
#define glMapGrid2f_RETURN void
#define glMapGrid2f_ARG_NAMES un, u1, u2, vn, v1, v2
#define glMapGrid2f_ARG_EXPAND GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2
#define glMapGrid2f_PACKED PACKED_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat
#define glMapGrid2f_INDEXED INDEXED_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat
#define glMapGrid2f_FORMAT FORMAT_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat
#define glMaterialf_INDEX 238
#define glMaterialf_RETURN void
#define glMaterialf_ARG_NAMES face, pname, param
#define glMaterialf_ARG_EXPAND GLenum face, GLenum pname, GLfloat param
#define glMaterialf_PACKED PACKED_void_GLenum_GLenum_GLfloat
#define glMaterialf_INDEXED INDEXED_void_GLenum_GLenum_GLfloat
#define glMaterialf_FORMAT FORMAT_void_GLenum_GLenum_GLfloat
#define glMaterialfv_INDEX 239
#define glMaterialfv_RETURN void
#define glMaterialfv_ARG_NAMES face, pname, params
#define glMaterialfv_ARG_EXPAND GLenum face, GLenum pname, const GLfloat * params
#define glMaterialfv_PACKED PACKED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glMaterialfv_INDEXED INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glMaterialfv_FORMAT FORMAT_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glMateriali_INDEX 240
#define glMateriali_RETURN void
#define glMateriali_ARG_NAMES face, pname, param
#define glMateriali_ARG_EXPAND GLenum face, GLenum pname, GLint param
#define glMateriali_PACKED PACKED_void_GLenum_GLenum_GLint
#define glMateriali_INDEXED INDEXED_void_GLenum_GLenum_GLint
#define glMateriali_FORMAT FORMAT_void_GLenum_GLenum_GLint
#define glMaterialiv_INDEX 241
#define glMaterialiv_RETURN void
#define glMaterialiv_ARG_NAMES face, pname, params
#define glMaterialiv_ARG_EXPAND GLenum face, GLenum pname, const GLint * params
#define glMaterialiv_PACKED PACKED_void_GLenum_GLenum_const_GLint___GENPT__
#define glMaterialiv_INDEXED INDEXED_void_GLenum_GLenum_const_GLint___GENPT__
#define glMaterialiv_FORMAT FORMAT_void_GLenum_GLenum_const_GLint___GENPT__
#define glMatrixMode_INDEX 242
#define glMatrixMode_RETURN void
#define glMatrixMode_ARG_NAMES mode
#define glMatrixMode_ARG_EXPAND GLenum mode
#define glMatrixMode_PACKED PACKED_void_GLenum
#define glMatrixMode_INDEXED INDEXED_void_GLenum
#define glMatrixMode_FORMAT FORMAT_void_GLenum
#define glMinmax_INDEX 243
#define glMinmax_RETURN void
#define glMinmax_ARG_NAMES target, internalformat, sink
#define glMinmax_ARG_EXPAND GLenum target, GLenum internalformat, GLboolean sink
#define glMinmax_PACKED PACKED_void_GLenum_GLenum_GLboolean
#define glMinmax_INDEXED INDEXED_void_GLenum_GLenum_GLboolean
#define glMinmax_FORMAT FORMAT_void_GLenum_GLenum_GLboolean
#define glMultMatrixd_INDEX 244
#define glMultMatrixd_RETURN void
#define glMultMatrixd_ARG_NAMES m
#define glMultMatrixd_ARG_EXPAND const GLdouble * m
#define glMultMatrixd_PACKED PACKED_void_const_GLdouble___GENPT__
#define glMultMatrixd_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glMultMatrixd_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glMultMatrixf_INDEX 245
#define glMultMatrixf_RETURN void
#define glMultMatrixf_ARG_NAMES m
#define glMultMatrixf_ARG_EXPAND const GLfloat * m
#define glMultMatrixf_PACKED PACKED_void_const_GLfloat___GENPT__
#define glMultMatrixf_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glMultMatrixf_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glMultTransposeMatrixd_INDEX 246
#define glMultTransposeMatrixd_RETURN void
#define glMultTransposeMatrixd_ARG_NAMES m
#define glMultTransposeMatrixd_ARG_EXPAND const GLdouble * m
#define glMultTransposeMatrixd_PACKED PACKED_void_const_GLdouble___GENPT__
#define glMultTransposeMatrixd_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glMultTransposeMatrixd_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glMultTransposeMatrixf_INDEX 247
#define glMultTransposeMatrixf_RETURN void
#define glMultTransposeMatrixf_ARG_NAMES m
#define glMultTransposeMatrixf_ARG_EXPAND const GLfloat * m
#define glMultTransposeMatrixf_PACKED PACKED_void_const_GLfloat___GENPT__
#define glMultTransposeMatrixf_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glMultTransposeMatrixf_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glMultiDrawArrays_INDEX 248
#define glMultiDrawArrays_RETURN void
#define glMultiDrawArrays_ARG_NAMES mode, first, count, drawcount
#define glMultiDrawArrays_ARG_EXPAND GLenum mode, const GLint * first, const GLsizei * count, GLsizei drawcount
#define glMultiDrawArrays_PACKED PACKED_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei
#define glMultiDrawArrays_INDEXED INDEXED_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei
#define glMultiDrawArrays_FORMAT FORMAT_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei
#define glMultiDrawElements_INDEX 249
#define glMultiDrawElements_RETURN void
#define glMultiDrawElements_ARG_NAMES mode, count, type, indices, drawcount
#define glMultiDrawElements_ARG_EXPAND GLenum mode, const GLsizei * count, GLenum type, GLvoid*const * indices, GLsizei drawcount
#define glMultiDrawElements_PACKED PACKED_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei
#define glMultiDrawElements_INDEXED INDEXED_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei
#define glMultiDrawElements_FORMAT FORMAT_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei
#define glMultiTexCoord1d_INDEX 250
#define glMultiTexCoord1d_RETURN void
#define glMultiTexCoord1d_ARG_NAMES target, s
#define glMultiTexCoord1d_ARG_EXPAND GLenum target, GLdouble s
#define glMultiTexCoord1d_PACKED PACKED_void_GLenum_GLdouble
#define glMultiTexCoord1d_INDEXED INDEXED_void_GLenum_GLdouble
#define glMultiTexCoord1d_FORMAT FORMAT_void_GLenum_GLdouble
#define glMultiTexCoord1dv_INDEX 251
#define glMultiTexCoord1dv_RETURN void
#define glMultiTexCoord1dv_ARG_NAMES target, v
#define glMultiTexCoord1dv_ARG_EXPAND GLenum target, const GLdouble * v
#define glMultiTexCoord1dv_PACKED PACKED_void_GLenum_const_GLdouble___GENPT__
#define glMultiTexCoord1dv_INDEXED INDEXED_void_GLenum_const_GLdouble___GENPT__
#define glMultiTexCoord1dv_FORMAT FORMAT_void_GLenum_const_GLdouble___GENPT__
#define glMultiTexCoord1f_INDEX 252
#define glMultiTexCoord1f_RETURN void
#define glMultiTexCoord1f_ARG_NAMES target, s
#define glMultiTexCoord1f_ARG_EXPAND GLenum target, GLfloat s
#define glMultiTexCoord1f_PACKED PACKED_void_GLenum_GLfloat
#define glMultiTexCoord1f_INDEXED INDEXED_void_GLenum_GLfloat
#define glMultiTexCoord1f_FORMAT FORMAT_void_GLenum_GLfloat
#define glMultiTexCoord1fv_INDEX 253
#define glMultiTexCoord1fv_RETURN void
#define glMultiTexCoord1fv_ARG_NAMES target, v
#define glMultiTexCoord1fv_ARG_EXPAND GLenum target, const GLfloat * v
#define glMultiTexCoord1fv_PACKED PACKED_void_GLenum_const_GLfloat___GENPT__
#define glMultiTexCoord1fv_INDEXED INDEXED_void_GLenum_const_GLfloat___GENPT__
#define glMultiTexCoord1fv_FORMAT FORMAT_void_GLenum_const_GLfloat___GENPT__
#define glMultiTexCoord1i_INDEX 254
#define glMultiTexCoord1i_RETURN void
#define glMultiTexCoord1i_ARG_NAMES target, s
#define glMultiTexCoord1i_ARG_EXPAND GLenum target, GLint s
#define glMultiTexCoord1i_PACKED PACKED_void_GLenum_GLint
#define glMultiTexCoord1i_INDEXED INDEXED_void_GLenum_GLint
#define glMultiTexCoord1i_FORMAT FORMAT_void_GLenum_GLint
#define glMultiTexCoord1iv_INDEX 255
#define glMultiTexCoord1iv_RETURN void
#define glMultiTexCoord1iv_ARG_NAMES target, v
#define glMultiTexCoord1iv_ARG_EXPAND GLenum target, const GLint * v
#define glMultiTexCoord1iv_PACKED PACKED_void_GLenum_const_GLint___GENPT__
#define glMultiTexCoord1iv_INDEXED INDEXED_void_GLenum_const_GLint___GENPT__
#define glMultiTexCoord1iv_FORMAT FORMAT_void_GLenum_const_GLint___GENPT__
#define glMultiTexCoord1s_INDEX 256
#define glMultiTexCoord1s_RETURN void
#define glMultiTexCoord1s_ARG_NAMES target, s
#define glMultiTexCoord1s_ARG_EXPAND GLenum target, GLshort s
#define glMultiTexCoord1s_PACKED PACKED_void_GLenum_GLshort
#define glMultiTexCoord1s_INDEXED INDEXED_void_GLenum_GLshort
#define glMultiTexCoord1s_FORMAT FORMAT_void_GLenum_GLshort
#define glMultiTexCoord1sv_INDEX 257
#define glMultiTexCoord1sv_RETURN void
#define glMultiTexCoord1sv_ARG_NAMES target, v
#define glMultiTexCoord1sv_ARG_EXPAND GLenum target, const GLshort * v
#define glMultiTexCoord1sv_PACKED PACKED_void_GLenum_const_GLshort___GENPT__
#define glMultiTexCoord1sv_INDEXED INDEXED_void_GLenum_const_GLshort___GENPT__
#define glMultiTexCoord1sv_FORMAT FORMAT_void_GLenum_const_GLshort___GENPT__
#define glMultiTexCoord2d_INDEX 258
#define glMultiTexCoord2d_RETURN void
#define glMultiTexCoord2d_ARG_NAMES target, s, t
#define glMultiTexCoord2d_ARG_EXPAND GLenum target, GLdouble s, GLdouble t
#define glMultiTexCoord2d_PACKED PACKED_void_GLenum_GLdouble_GLdouble
#define glMultiTexCoord2d_INDEXED INDEXED_void_GLenum_GLdouble_GLdouble
#define glMultiTexCoord2d_FORMAT FORMAT_void_GLenum_GLdouble_GLdouble
#define glMultiTexCoord2dv_INDEX 259
#define glMultiTexCoord2dv_RETURN void
#define glMultiTexCoord2dv_ARG_NAMES target, v
#define glMultiTexCoord2dv_ARG_EXPAND GLenum target, const GLdouble * v
#define glMultiTexCoord2dv_PACKED PACKED_void_GLenum_const_GLdouble___GENPT__
#define glMultiTexCoord2dv_INDEXED INDEXED_void_GLenum_const_GLdouble___GENPT__
#define glMultiTexCoord2dv_FORMAT FORMAT_void_GLenum_const_GLdouble___GENPT__
#define glMultiTexCoord2f_INDEX 260
#define glMultiTexCoord2f_RETURN void
#define glMultiTexCoord2f_ARG_NAMES target, s, t
#define glMultiTexCoord2f_ARG_EXPAND GLenum target, GLfloat s, GLfloat t
#define glMultiTexCoord2f_PACKED PACKED_void_GLenum_GLfloat_GLfloat
#define glMultiTexCoord2f_INDEXED INDEXED_void_GLenum_GLfloat_GLfloat
#define glMultiTexCoord2f_FORMAT FORMAT_void_GLenum_GLfloat_GLfloat
#define glMultiTexCoord2fv_INDEX 261
#define glMultiTexCoord2fv_RETURN void
#define glMultiTexCoord2fv_ARG_NAMES target, v
#define glMultiTexCoord2fv_ARG_EXPAND GLenum target, const GLfloat * v
#define glMultiTexCoord2fv_PACKED PACKED_void_GLenum_const_GLfloat___GENPT__
#define glMultiTexCoord2fv_INDEXED INDEXED_void_GLenum_const_GLfloat___GENPT__
#define glMultiTexCoord2fv_FORMAT FORMAT_void_GLenum_const_GLfloat___GENPT__
#define glMultiTexCoord2i_INDEX 262
#define glMultiTexCoord2i_RETURN void
#define glMultiTexCoord2i_ARG_NAMES target, s, t
#define glMultiTexCoord2i_ARG_EXPAND GLenum target, GLint s, GLint t
#define glMultiTexCoord2i_PACKED PACKED_void_GLenum_GLint_GLint
#define glMultiTexCoord2i_INDEXED INDEXED_void_GLenum_GLint_GLint
#define glMultiTexCoord2i_FORMAT FORMAT_void_GLenum_GLint_GLint
#define glMultiTexCoord2iv_INDEX 263
#define glMultiTexCoord2iv_RETURN void
#define glMultiTexCoord2iv_ARG_NAMES target, v
#define glMultiTexCoord2iv_ARG_EXPAND GLenum target, const GLint * v
#define glMultiTexCoord2iv_PACKED PACKED_void_GLenum_const_GLint___GENPT__
#define glMultiTexCoord2iv_INDEXED INDEXED_void_GLenum_const_GLint___GENPT__
#define glMultiTexCoord2iv_FORMAT FORMAT_void_GLenum_const_GLint___GENPT__
#define glMultiTexCoord2s_INDEX 264
#define glMultiTexCoord2s_RETURN void
#define glMultiTexCoord2s_ARG_NAMES target, s, t
#define glMultiTexCoord2s_ARG_EXPAND GLenum target, GLshort s, GLshort t
#define glMultiTexCoord2s_PACKED PACKED_void_GLenum_GLshort_GLshort
#define glMultiTexCoord2s_INDEXED INDEXED_void_GLenum_GLshort_GLshort
#define glMultiTexCoord2s_FORMAT FORMAT_void_GLenum_GLshort_GLshort
#define glMultiTexCoord2sv_INDEX 265
#define glMultiTexCoord2sv_RETURN void
#define glMultiTexCoord2sv_ARG_NAMES target, v
#define glMultiTexCoord2sv_ARG_EXPAND GLenum target, const GLshort * v
#define glMultiTexCoord2sv_PACKED PACKED_void_GLenum_const_GLshort___GENPT__
#define glMultiTexCoord2sv_INDEXED INDEXED_void_GLenum_const_GLshort___GENPT__
#define glMultiTexCoord2sv_FORMAT FORMAT_void_GLenum_const_GLshort___GENPT__
#define glMultiTexCoord3d_INDEX 266
#define glMultiTexCoord3d_RETURN void
#define glMultiTexCoord3d_ARG_NAMES target, s, t, r
#define glMultiTexCoord3d_ARG_EXPAND GLenum target, GLdouble s, GLdouble t, GLdouble r
#define glMultiTexCoord3d_PACKED PACKED_void_GLenum_GLdouble_GLdouble_GLdouble
#define glMultiTexCoord3d_INDEXED INDEXED_void_GLenum_GLdouble_GLdouble_GLdouble
#define glMultiTexCoord3d_FORMAT FORMAT_void_GLenum_GLdouble_GLdouble_GLdouble
#define glMultiTexCoord3dv_INDEX 267
#define glMultiTexCoord3dv_RETURN void
#define glMultiTexCoord3dv_ARG_NAMES target, v
#define glMultiTexCoord3dv_ARG_EXPAND GLenum target, const GLdouble * v
#define glMultiTexCoord3dv_PACKED PACKED_void_GLenum_const_GLdouble___GENPT__
#define glMultiTexCoord3dv_INDEXED INDEXED_void_GLenum_const_GLdouble___GENPT__
#define glMultiTexCoord3dv_FORMAT FORMAT_void_GLenum_const_GLdouble___GENPT__
#define glMultiTexCoord3f_INDEX 268
#define glMultiTexCoord3f_RETURN void
#define glMultiTexCoord3f_ARG_NAMES target, s, t, r
#define glMultiTexCoord3f_ARG_EXPAND GLenum target, GLfloat s, GLfloat t, GLfloat r
#define glMultiTexCoord3f_PACKED PACKED_void_GLenum_GLfloat_GLfloat_GLfloat
#define glMultiTexCoord3f_INDEXED INDEXED_void_GLenum_GLfloat_GLfloat_GLfloat
#define glMultiTexCoord3f_FORMAT FORMAT_void_GLenum_GLfloat_GLfloat_GLfloat
#define glMultiTexCoord3fv_INDEX 269
#define glMultiTexCoord3fv_RETURN void
#define glMultiTexCoord3fv_ARG_NAMES target, v
#define glMultiTexCoord3fv_ARG_EXPAND GLenum target, const GLfloat * v
#define glMultiTexCoord3fv_PACKED PACKED_void_GLenum_const_GLfloat___GENPT__
#define glMultiTexCoord3fv_INDEXED INDEXED_void_GLenum_const_GLfloat___GENPT__
#define glMultiTexCoord3fv_FORMAT FORMAT_void_GLenum_const_GLfloat___GENPT__
#define glMultiTexCoord3i_INDEX 270
#define glMultiTexCoord3i_RETURN void
#define glMultiTexCoord3i_ARG_NAMES target, s, t, r
#define glMultiTexCoord3i_ARG_EXPAND GLenum target, GLint s, GLint t, GLint r
#define glMultiTexCoord3i_PACKED PACKED_void_GLenum_GLint_GLint_GLint
#define glMultiTexCoord3i_INDEXED INDEXED_void_GLenum_GLint_GLint_GLint
#define glMultiTexCoord3i_FORMAT FORMAT_void_GLenum_GLint_GLint_GLint
#define glMultiTexCoord3iv_INDEX 271
#define glMultiTexCoord3iv_RETURN void
#define glMultiTexCoord3iv_ARG_NAMES target, v
#define glMultiTexCoord3iv_ARG_EXPAND GLenum target, const GLint * v
#define glMultiTexCoord3iv_PACKED PACKED_void_GLenum_const_GLint___GENPT__
#define glMultiTexCoord3iv_INDEXED INDEXED_void_GLenum_const_GLint___GENPT__
#define glMultiTexCoord3iv_FORMAT FORMAT_void_GLenum_const_GLint___GENPT__
#define glMultiTexCoord3s_INDEX 272
#define glMultiTexCoord3s_RETURN void
#define glMultiTexCoord3s_ARG_NAMES target, s, t, r
#define glMultiTexCoord3s_ARG_EXPAND GLenum target, GLshort s, GLshort t, GLshort r
#define glMultiTexCoord3s_PACKED PACKED_void_GLenum_GLshort_GLshort_GLshort
#define glMultiTexCoord3s_INDEXED INDEXED_void_GLenum_GLshort_GLshort_GLshort
#define glMultiTexCoord3s_FORMAT FORMAT_void_GLenum_GLshort_GLshort_GLshort
#define glMultiTexCoord3sv_INDEX 273
#define glMultiTexCoord3sv_RETURN void
#define glMultiTexCoord3sv_ARG_NAMES target, v
#define glMultiTexCoord3sv_ARG_EXPAND GLenum target, const GLshort * v
#define glMultiTexCoord3sv_PACKED PACKED_void_GLenum_const_GLshort___GENPT__
#define glMultiTexCoord3sv_INDEXED INDEXED_void_GLenum_const_GLshort___GENPT__
#define glMultiTexCoord3sv_FORMAT FORMAT_void_GLenum_const_GLshort___GENPT__
#define glMultiTexCoord4d_INDEX 274
#define glMultiTexCoord4d_RETURN void
#define glMultiTexCoord4d_ARG_NAMES target, s, t, r, q
#define glMultiTexCoord4d_ARG_EXPAND GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q
#define glMultiTexCoord4d_PACKED PACKED_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble
#define glMultiTexCoord4d_INDEXED INDEXED_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble
#define glMultiTexCoord4d_FORMAT FORMAT_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble
#define glMultiTexCoord4dv_INDEX 275
#define glMultiTexCoord4dv_RETURN void
#define glMultiTexCoord4dv_ARG_NAMES target, v
#define glMultiTexCoord4dv_ARG_EXPAND GLenum target, const GLdouble * v
#define glMultiTexCoord4dv_PACKED PACKED_void_GLenum_const_GLdouble___GENPT__
#define glMultiTexCoord4dv_INDEXED INDEXED_void_GLenum_const_GLdouble___GENPT__
#define glMultiTexCoord4dv_FORMAT FORMAT_void_GLenum_const_GLdouble___GENPT__
#define glMultiTexCoord4f_INDEX 276
#define glMultiTexCoord4f_RETURN void
#define glMultiTexCoord4f_ARG_NAMES target, s, t, r, q
#define glMultiTexCoord4f_ARG_EXPAND GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q
#define glMultiTexCoord4f_PACKED PACKED_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat
#define glMultiTexCoord4f_INDEXED INDEXED_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat
#define glMultiTexCoord4f_FORMAT FORMAT_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat
#define glMultiTexCoord4fv_INDEX 277
#define glMultiTexCoord4fv_RETURN void
#define glMultiTexCoord4fv_ARG_NAMES target, v
#define glMultiTexCoord4fv_ARG_EXPAND GLenum target, const GLfloat * v
#define glMultiTexCoord4fv_PACKED PACKED_void_GLenum_const_GLfloat___GENPT__
#define glMultiTexCoord4fv_INDEXED INDEXED_void_GLenum_const_GLfloat___GENPT__
#define glMultiTexCoord4fv_FORMAT FORMAT_void_GLenum_const_GLfloat___GENPT__
#define glMultiTexCoord4i_INDEX 278
#define glMultiTexCoord4i_RETURN void
#define glMultiTexCoord4i_ARG_NAMES target, s, t, r, q
#define glMultiTexCoord4i_ARG_EXPAND GLenum target, GLint s, GLint t, GLint r, GLint q
#define glMultiTexCoord4i_PACKED PACKED_void_GLenum_GLint_GLint_GLint_GLint
#define glMultiTexCoord4i_INDEXED INDEXED_void_GLenum_GLint_GLint_GLint_GLint
#define glMultiTexCoord4i_FORMAT FORMAT_void_GLenum_GLint_GLint_GLint_GLint
#define glMultiTexCoord4iv_INDEX 279
#define glMultiTexCoord4iv_RETURN void
#define glMultiTexCoord4iv_ARG_NAMES target, v
#define glMultiTexCoord4iv_ARG_EXPAND GLenum target, const GLint * v
#define glMultiTexCoord4iv_PACKED PACKED_void_GLenum_const_GLint___GENPT__
#define glMultiTexCoord4iv_INDEXED INDEXED_void_GLenum_const_GLint___GENPT__
#define glMultiTexCoord4iv_FORMAT FORMAT_void_GLenum_const_GLint___GENPT__
#define glMultiTexCoord4s_INDEX 280
#define glMultiTexCoord4s_RETURN void
#define glMultiTexCoord4s_ARG_NAMES target, s, t, r, q
#define glMultiTexCoord4s_ARG_EXPAND GLenum target, GLshort s, GLshort t, GLshort r, GLshort q
#define glMultiTexCoord4s_PACKED PACKED_void_GLenum_GLshort_GLshort_GLshort_GLshort
#define glMultiTexCoord4s_INDEXED INDEXED_void_GLenum_GLshort_GLshort_GLshort_GLshort
#define glMultiTexCoord4s_FORMAT FORMAT_void_GLenum_GLshort_GLshort_GLshort_GLshort
#define glMultiTexCoord4sv_INDEX 281
#define glMultiTexCoord4sv_RETURN void
#define glMultiTexCoord4sv_ARG_NAMES target, v
#define glMultiTexCoord4sv_ARG_EXPAND GLenum target, const GLshort * v
#define glMultiTexCoord4sv_PACKED PACKED_void_GLenum_const_GLshort___GENPT__
#define glMultiTexCoord4sv_INDEXED INDEXED_void_GLenum_const_GLshort___GENPT__
#define glMultiTexCoord4sv_FORMAT FORMAT_void_GLenum_const_GLshort___GENPT__
#define glNewList_INDEX 282
#define glNewList_RETURN void
#define glNewList_ARG_NAMES list, mode
#define glNewList_ARG_EXPAND GLuint list, GLenum mode
#define glNewList_PACKED PACKED_void_GLuint_GLenum
#define glNewList_INDEXED INDEXED_void_GLuint_GLenum
#define glNewList_FORMAT FORMAT_void_GLuint_GLenum
#define glNormal3b_INDEX 283
#define glNormal3b_RETURN void
#define glNormal3b_ARG_NAMES nx, ny, nz
#define glNormal3b_ARG_EXPAND GLbyte nx, GLbyte ny, GLbyte nz
#define glNormal3b_PACKED PACKED_void_GLbyte_GLbyte_GLbyte
#define glNormal3b_INDEXED INDEXED_void_GLbyte_GLbyte_GLbyte
#define glNormal3b_FORMAT FORMAT_void_GLbyte_GLbyte_GLbyte
#define glNormal3bv_INDEX 284
#define glNormal3bv_RETURN void
#define glNormal3bv_ARG_NAMES v
#define glNormal3bv_ARG_EXPAND const GLbyte * v
#define glNormal3bv_PACKED PACKED_void_const_GLbyte___GENPT__
#define glNormal3bv_INDEXED INDEXED_void_const_GLbyte___GENPT__
#define glNormal3bv_FORMAT FORMAT_void_const_GLbyte___GENPT__
#define glNormal3d_INDEX 285
#define glNormal3d_RETURN void
#define glNormal3d_ARG_NAMES nx, ny, nz
#define glNormal3d_ARG_EXPAND GLdouble nx, GLdouble ny, GLdouble nz
#define glNormal3d_PACKED PACKED_void_GLdouble_GLdouble_GLdouble
#define glNormal3d_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble
#define glNormal3d_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble
#define glNormal3dv_INDEX 286
#define glNormal3dv_RETURN void
#define glNormal3dv_ARG_NAMES v
#define glNormal3dv_ARG_EXPAND const GLdouble * v
#define glNormal3dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glNormal3dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glNormal3dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glNormal3f_INDEX 287
#define glNormal3f_RETURN void
#define glNormal3f_ARG_NAMES nx, ny, nz
#define glNormal3f_ARG_EXPAND GLfloat nx, GLfloat ny, GLfloat nz
#define glNormal3f_PACKED PACKED_void_GLfloat_GLfloat_GLfloat
#define glNormal3f_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat
#define glNormal3f_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat
#define glNormal3fv_INDEX 288
#define glNormal3fv_RETURN void
#define glNormal3fv_ARG_NAMES v
#define glNormal3fv_ARG_EXPAND const GLfloat * v
#define glNormal3fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glNormal3fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glNormal3fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glNormal3i_INDEX 289
#define glNormal3i_RETURN void
#define glNormal3i_ARG_NAMES nx, ny, nz
#define glNormal3i_ARG_EXPAND GLint nx, GLint ny, GLint nz
#define glNormal3i_PACKED PACKED_void_GLint_GLint_GLint
#define glNormal3i_INDEXED INDEXED_void_GLint_GLint_GLint
#define glNormal3i_FORMAT FORMAT_void_GLint_GLint_GLint
#define glNormal3iv_INDEX 290
#define glNormal3iv_RETURN void
#define glNormal3iv_ARG_NAMES v
#define glNormal3iv_ARG_EXPAND const GLint * v
#define glNormal3iv_PACKED PACKED_void_const_GLint___GENPT__
#define glNormal3iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glNormal3iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glNormal3s_INDEX 291
#define glNormal3s_RETURN void
#define glNormal3s_ARG_NAMES nx, ny, nz
#define glNormal3s_ARG_EXPAND GLshort nx, GLshort ny, GLshort nz
#define glNormal3s_PACKED PACKED_void_GLshort_GLshort_GLshort
#define glNormal3s_INDEXED INDEXED_void_GLshort_GLshort_GLshort
#define glNormal3s_FORMAT FORMAT_void_GLshort_GLshort_GLshort
#define glNormal3sv_INDEX 292
#define glNormal3sv_RETURN void
#define glNormal3sv_ARG_NAMES v
#define glNormal3sv_ARG_EXPAND const GLshort * v
#define glNormal3sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glNormal3sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glNormal3sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glNormalPointer_INDEX 293
#define glNormalPointer_RETURN void
#define glNormalPointer_ARG_NAMES type, stride, pointer
#define glNormalPointer_ARG_EXPAND GLenum type, GLsizei stride, const GLvoid * pointer
#define glNormalPointer_PACKED PACKED_void_GLenum_GLsizei_const_GLvoid___GENPT__
#define glNormalPointer_INDEXED INDEXED_void_GLenum_GLsizei_const_GLvoid___GENPT__
#define glNormalPointer_FORMAT FORMAT_void_GLenum_GLsizei_const_GLvoid___GENPT__
#define glOrtho_INDEX 294
#define glOrtho_RETURN void
#define glOrtho_ARG_NAMES left, right, bottom, top, zNear, zFar
#define glOrtho_ARG_EXPAND GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar
#define glOrtho_PACKED PACKED_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble
#define glOrtho_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble
#define glOrtho_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble
#define glPassThrough_INDEX 295
#define glPassThrough_RETURN void
#define glPassThrough_ARG_NAMES token
#define glPassThrough_ARG_EXPAND GLfloat token
#define glPassThrough_PACKED PACKED_void_GLfloat
#define glPassThrough_INDEXED INDEXED_void_GLfloat
#define glPassThrough_FORMAT FORMAT_void_GLfloat
#define glPixelMapfv_INDEX 296
#define glPixelMapfv_RETURN void
#define glPixelMapfv_ARG_NAMES map, mapsize, values
#define glPixelMapfv_ARG_EXPAND GLenum map, GLsizei mapsize, const GLfloat * values
#define glPixelMapfv_PACKED PACKED_void_GLenum_GLsizei_const_GLfloat___GENPT__
#define glPixelMapfv_INDEXED INDEXED_void_GLenum_GLsizei_const_GLfloat___GENPT__
#define glPixelMapfv_FORMAT FORMAT_void_GLenum_GLsizei_const_GLfloat___GENPT__
#define glPixelMapuiv_INDEX 297
#define glPixelMapuiv_RETURN void
#define glPixelMapuiv_ARG_NAMES map, mapsize, values
#define glPixelMapuiv_ARG_EXPAND GLenum map, GLsizei mapsize, const GLuint * values
#define glPixelMapuiv_PACKED PACKED_void_GLenum_GLsizei_const_GLuint___GENPT__
#define glPixelMapuiv_INDEXED INDEXED_void_GLenum_GLsizei_const_GLuint___GENPT__
#define glPixelMapuiv_FORMAT FORMAT_void_GLenum_GLsizei_const_GLuint___GENPT__
#define glPixelMapusv_INDEX 298
#define glPixelMapusv_RETURN void
#define glPixelMapusv_ARG_NAMES map, mapsize, values
#define glPixelMapusv_ARG_EXPAND GLenum map, GLsizei mapsize, const GLushort * values
#define glPixelMapusv_PACKED PACKED_void_GLenum_GLsizei_const_GLushort___GENPT__
#define glPixelMapusv_INDEXED INDEXED_void_GLenum_GLsizei_const_GLushort___GENPT__
#define glPixelMapusv_FORMAT FORMAT_void_GLenum_GLsizei_const_GLushort___GENPT__
#define glPixelStoref_INDEX 299
#define glPixelStoref_RETURN void
#define glPixelStoref_ARG_NAMES pname, param
#define glPixelStoref_ARG_EXPAND GLenum pname, GLfloat param
#define glPixelStoref_PACKED PACKED_void_GLenum_GLfloat
#define glPixelStoref_INDEXED INDEXED_void_GLenum_GLfloat
#define glPixelStoref_FORMAT FORMAT_void_GLenum_GLfloat
#define glPixelStorei_INDEX 300
#define glPixelStorei_RETURN void
#define glPixelStorei_ARG_NAMES pname, param
#define glPixelStorei_ARG_EXPAND GLenum pname, GLint param
#define glPixelStorei_PACKED PACKED_void_GLenum_GLint
#define glPixelStorei_INDEXED INDEXED_void_GLenum_GLint
#define glPixelStorei_FORMAT FORMAT_void_GLenum_GLint
#define glPixelTransferf_INDEX 301
#define glPixelTransferf_RETURN void
#define glPixelTransferf_ARG_NAMES pname, param
#define glPixelTransferf_ARG_EXPAND GLenum pname, GLfloat param
#define glPixelTransferf_PACKED PACKED_void_GLenum_GLfloat
#define glPixelTransferf_INDEXED INDEXED_void_GLenum_GLfloat
#define glPixelTransferf_FORMAT FORMAT_void_GLenum_GLfloat
#define glPixelTransferi_INDEX 302
#define glPixelTransferi_RETURN void
#define glPixelTransferi_ARG_NAMES pname, param
#define glPixelTransferi_ARG_EXPAND GLenum pname, GLint param
#define glPixelTransferi_PACKED PACKED_void_GLenum_GLint
#define glPixelTransferi_INDEXED INDEXED_void_GLenum_GLint
#define glPixelTransferi_FORMAT FORMAT_void_GLenum_GLint
#define glPixelZoom_INDEX 303
#define glPixelZoom_RETURN void
#define glPixelZoom_ARG_NAMES xfactor, yfactor
#define glPixelZoom_ARG_EXPAND GLfloat xfactor, GLfloat yfactor
#define glPixelZoom_PACKED PACKED_void_GLfloat_GLfloat
#define glPixelZoom_INDEXED INDEXED_void_GLfloat_GLfloat
#define glPixelZoom_FORMAT FORMAT_void_GLfloat_GLfloat
#define glPointParameterf_INDEX 304
#define glPointParameterf_RETURN void
#define glPointParameterf_ARG_NAMES pname, param
#define glPointParameterf_ARG_EXPAND GLenum pname, GLfloat param
#define glPointParameterf_PACKED PACKED_void_GLenum_GLfloat
#define glPointParameterf_INDEXED INDEXED_void_GLenum_GLfloat
#define glPointParameterf_FORMAT FORMAT_void_GLenum_GLfloat
#define glPointParameterfv_INDEX 305
#define glPointParameterfv_RETURN void
#define glPointParameterfv_ARG_NAMES pname, params
#define glPointParameterfv_ARG_EXPAND GLenum pname, const GLfloat * params
#define glPointParameterfv_PACKED PACKED_void_GLenum_const_GLfloat___GENPT__
#define glPointParameterfv_INDEXED INDEXED_void_GLenum_const_GLfloat___GENPT__
#define glPointParameterfv_FORMAT FORMAT_void_GLenum_const_GLfloat___GENPT__
#define glPointParameteri_INDEX 306
#define glPointParameteri_RETURN void
#define glPointParameteri_ARG_NAMES pname, param
#define glPointParameteri_ARG_EXPAND GLenum pname, GLint param
#define glPointParameteri_PACKED PACKED_void_GLenum_GLint
#define glPointParameteri_INDEXED INDEXED_void_GLenum_GLint
#define glPointParameteri_FORMAT FORMAT_void_GLenum_GLint
#define glPointParameteriv_INDEX 307
#define glPointParameteriv_RETURN void
#define glPointParameteriv_ARG_NAMES pname, params
#define glPointParameteriv_ARG_EXPAND GLenum pname, const GLint * params
#define glPointParameteriv_PACKED PACKED_void_GLenum_const_GLint___GENPT__
#define glPointParameteriv_INDEXED INDEXED_void_GLenum_const_GLint___GENPT__
#define glPointParameteriv_FORMAT FORMAT_void_GLenum_const_GLint___GENPT__
#define glPointSize_INDEX 308
#define glPointSize_RETURN void
#define glPointSize_ARG_NAMES size
#define glPointSize_ARG_EXPAND GLfloat size
#define glPointSize_PACKED PACKED_void_GLfloat
#define glPointSize_INDEXED INDEXED_void_GLfloat
#define glPointSize_FORMAT FORMAT_void_GLfloat
#define glPolygonMode_INDEX 309
#define glPolygonMode_RETURN void
#define glPolygonMode_ARG_NAMES face, mode
#define glPolygonMode_ARG_EXPAND GLenum face, GLenum mode
#define glPolygonMode_PACKED PACKED_void_GLenum_GLenum
#define glPolygonMode_INDEXED INDEXED_void_GLenum_GLenum
#define glPolygonMode_FORMAT FORMAT_void_GLenum_GLenum
#define glPolygonOffset_INDEX 310
#define glPolygonOffset_RETURN void
#define glPolygonOffset_ARG_NAMES factor, units
#define glPolygonOffset_ARG_EXPAND GLfloat factor, GLfloat units
#define glPolygonOffset_PACKED PACKED_void_GLfloat_GLfloat
#define glPolygonOffset_INDEXED INDEXED_void_GLfloat_GLfloat
#define glPolygonOffset_FORMAT FORMAT_void_GLfloat_GLfloat
#define glPolygonStipple_INDEX 311
#define glPolygonStipple_RETURN void
#define glPolygonStipple_ARG_NAMES mask
#define glPolygonStipple_ARG_EXPAND const GLubyte * mask
#define glPolygonStipple_PACKED PACKED_void_const_GLubyte___GENPT__
#define glPolygonStipple_INDEXED INDEXED_void_const_GLubyte___GENPT__
#define glPolygonStipple_FORMAT FORMAT_void_const_GLubyte___GENPT__
#define glPopAttrib_INDEX 312
#define glPopAttrib_RETURN void
#define glPopAttrib_ARG_NAMES 
#define glPopAttrib_ARG_EXPAND 
#define glPopAttrib_PACKED PACKED_void
#define glPopAttrib_INDEXED INDEXED_void
#define glPopAttrib_FORMAT FORMAT_void
#define glPopClientAttrib_INDEX 313
#define glPopClientAttrib_RETURN void
#define glPopClientAttrib_ARG_NAMES 
#define glPopClientAttrib_ARG_EXPAND 
#define glPopClientAttrib_PACKED PACKED_void
#define glPopClientAttrib_INDEXED INDEXED_void
#define glPopClientAttrib_FORMAT FORMAT_void
#define glPopMatrix_INDEX 314
#define glPopMatrix_RETURN void
#define glPopMatrix_ARG_NAMES 
#define glPopMatrix_ARG_EXPAND 
#define glPopMatrix_PACKED PACKED_void
#define glPopMatrix_INDEXED INDEXED_void
#define glPopMatrix_FORMAT FORMAT_void
#define glPopName_INDEX 315
#define glPopName_RETURN void
#define glPopName_ARG_NAMES 
#define glPopName_ARG_EXPAND 
#define glPopName_PACKED PACKED_void
#define glPopName_INDEXED INDEXED_void
#define glPopName_FORMAT FORMAT_void
#define glPrioritizeTextures_INDEX 316
#define glPrioritizeTextures_RETURN void
#define glPrioritizeTextures_ARG_NAMES n, textures, priorities
#define glPrioritizeTextures_ARG_EXPAND GLsizei n, const GLuint * textures, const GLfloat * priorities
#define glPrioritizeTextures_PACKED PACKED_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__
#define glPrioritizeTextures_INDEXED INDEXED_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__
#define glPrioritizeTextures_FORMAT FORMAT_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__
#define glPushAttrib_INDEX 317
#define glPushAttrib_RETURN void
#define glPushAttrib_ARG_NAMES mask
#define glPushAttrib_ARG_EXPAND GLbitfield mask
#define glPushAttrib_PACKED PACKED_void_GLbitfield
#define glPushAttrib_INDEXED INDEXED_void_GLbitfield
#define glPushAttrib_FORMAT FORMAT_void_GLbitfield
#define glPushClientAttrib_INDEX 318
#define glPushClientAttrib_RETURN void
#define glPushClientAttrib_ARG_NAMES mask
#define glPushClientAttrib_ARG_EXPAND GLbitfield mask
#define glPushClientAttrib_PACKED PACKED_void_GLbitfield
#define glPushClientAttrib_INDEXED INDEXED_void_GLbitfield
#define glPushClientAttrib_FORMAT FORMAT_void_GLbitfield
#define glPushMatrix_INDEX 319
#define glPushMatrix_RETURN void
#define glPushMatrix_ARG_NAMES 
#define glPushMatrix_ARG_EXPAND 
#define glPushMatrix_PACKED PACKED_void
#define glPushMatrix_INDEXED INDEXED_void
#define glPushMatrix_FORMAT FORMAT_void
#define glPushName_INDEX 320
#define glPushName_RETURN void
#define glPushName_ARG_NAMES name
#define glPushName_ARG_EXPAND GLuint name
#define glPushName_PACKED PACKED_void_GLuint
#define glPushName_INDEXED INDEXED_void_GLuint
#define glPushName_FORMAT FORMAT_void_GLuint
#define glRasterPos2d_INDEX 321
#define glRasterPos2d_RETURN void
#define glRasterPos2d_ARG_NAMES x, y
#define glRasterPos2d_ARG_EXPAND GLdouble x, GLdouble y
#define glRasterPos2d_PACKED PACKED_void_GLdouble_GLdouble
#define glRasterPos2d_INDEXED INDEXED_void_GLdouble_GLdouble
#define glRasterPos2d_FORMAT FORMAT_void_GLdouble_GLdouble
#define glRasterPos2dv_INDEX 322
#define glRasterPos2dv_RETURN void
#define glRasterPos2dv_ARG_NAMES v
#define glRasterPos2dv_ARG_EXPAND const GLdouble * v
#define glRasterPos2dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glRasterPos2dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glRasterPos2dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glRasterPos2f_INDEX 323
#define glRasterPos2f_RETURN void
#define glRasterPos2f_ARG_NAMES x, y
#define glRasterPos2f_ARG_EXPAND GLfloat x, GLfloat y
#define glRasterPos2f_PACKED PACKED_void_GLfloat_GLfloat
#define glRasterPos2f_INDEXED INDEXED_void_GLfloat_GLfloat
#define glRasterPos2f_FORMAT FORMAT_void_GLfloat_GLfloat
#define glRasterPos2fv_INDEX 324
#define glRasterPos2fv_RETURN void
#define glRasterPos2fv_ARG_NAMES v
#define glRasterPos2fv_ARG_EXPAND const GLfloat * v
#define glRasterPos2fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glRasterPos2fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glRasterPos2fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glRasterPos2i_INDEX 325
#define glRasterPos2i_RETURN void
#define glRasterPos2i_ARG_NAMES x, y
#define glRasterPos2i_ARG_EXPAND GLint x, GLint y
#define glRasterPos2i_PACKED PACKED_void_GLint_GLint
#define glRasterPos2i_INDEXED INDEXED_void_GLint_GLint
#define glRasterPos2i_FORMAT FORMAT_void_GLint_GLint
#define glRasterPos2iv_INDEX 326
#define glRasterPos2iv_RETURN void
#define glRasterPos2iv_ARG_NAMES v
#define glRasterPos2iv_ARG_EXPAND const GLint * v
#define glRasterPos2iv_PACKED PACKED_void_const_GLint___GENPT__
#define glRasterPos2iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glRasterPos2iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glRasterPos2s_INDEX 327
#define glRasterPos2s_RETURN void
#define glRasterPos2s_ARG_NAMES x, y
#define glRasterPos2s_ARG_EXPAND GLshort x, GLshort y
#define glRasterPos2s_PACKED PACKED_void_GLshort_GLshort
#define glRasterPos2s_INDEXED INDEXED_void_GLshort_GLshort
#define glRasterPos2s_FORMAT FORMAT_void_GLshort_GLshort
#define glRasterPos2sv_INDEX 328
#define glRasterPos2sv_RETURN void
#define glRasterPos2sv_ARG_NAMES v
#define glRasterPos2sv_ARG_EXPAND const GLshort * v
#define glRasterPos2sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glRasterPos2sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glRasterPos2sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glRasterPos3d_INDEX 329
#define glRasterPos3d_RETURN void
#define glRasterPos3d_ARG_NAMES x, y, z
#define glRasterPos3d_ARG_EXPAND GLdouble x, GLdouble y, GLdouble z
#define glRasterPos3d_PACKED PACKED_void_GLdouble_GLdouble_GLdouble
#define glRasterPos3d_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble
#define glRasterPos3d_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble
#define glRasterPos3dv_INDEX 330
#define glRasterPos3dv_RETURN void
#define glRasterPos3dv_ARG_NAMES v
#define glRasterPos3dv_ARG_EXPAND const GLdouble * v
#define glRasterPos3dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glRasterPos3dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glRasterPos3dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glRasterPos3f_INDEX 331
#define glRasterPos3f_RETURN void
#define glRasterPos3f_ARG_NAMES x, y, z
#define glRasterPos3f_ARG_EXPAND GLfloat x, GLfloat y, GLfloat z
#define glRasterPos3f_PACKED PACKED_void_GLfloat_GLfloat_GLfloat
#define glRasterPos3f_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat
#define glRasterPos3f_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat
#define glRasterPos3fv_INDEX 332
#define glRasterPos3fv_RETURN void
#define glRasterPos3fv_ARG_NAMES v
#define glRasterPos3fv_ARG_EXPAND const GLfloat * v
#define glRasterPos3fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glRasterPos3fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glRasterPos3fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glRasterPos3i_INDEX 333
#define glRasterPos3i_RETURN void
#define glRasterPos3i_ARG_NAMES x, y, z
#define glRasterPos3i_ARG_EXPAND GLint x, GLint y, GLint z
#define glRasterPos3i_PACKED PACKED_void_GLint_GLint_GLint
#define glRasterPos3i_INDEXED INDEXED_void_GLint_GLint_GLint
#define glRasterPos3i_FORMAT FORMAT_void_GLint_GLint_GLint
#define glRasterPos3iv_INDEX 334
#define glRasterPos3iv_RETURN void
#define glRasterPos3iv_ARG_NAMES v
#define glRasterPos3iv_ARG_EXPAND const GLint * v
#define glRasterPos3iv_PACKED PACKED_void_const_GLint___GENPT__
#define glRasterPos3iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glRasterPos3iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glRasterPos3s_INDEX 335
#define glRasterPos3s_RETURN void
#define glRasterPos3s_ARG_NAMES x, y, z
#define glRasterPos3s_ARG_EXPAND GLshort x, GLshort y, GLshort z
#define glRasterPos3s_PACKED PACKED_void_GLshort_GLshort_GLshort
#define glRasterPos3s_INDEXED INDEXED_void_GLshort_GLshort_GLshort
#define glRasterPos3s_FORMAT FORMAT_void_GLshort_GLshort_GLshort
#define glRasterPos3sv_INDEX 336
#define glRasterPos3sv_RETURN void
#define glRasterPos3sv_ARG_NAMES v
#define glRasterPos3sv_ARG_EXPAND const GLshort * v
#define glRasterPos3sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glRasterPos3sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glRasterPos3sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glRasterPos4d_INDEX 337
#define glRasterPos4d_RETURN void
#define glRasterPos4d_ARG_NAMES x, y, z, w
#define glRasterPos4d_ARG_EXPAND GLdouble x, GLdouble y, GLdouble z, GLdouble w
#define glRasterPos4d_PACKED PACKED_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glRasterPos4d_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glRasterPos4d_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glRasterPos4dv_INDEX 338
#define glRasterPos4dv_RETURN void
#define glRasterPos4dv_ARG_NAMES v
#define glRasterPos4dv_ARG_EXPAND const GLdouble * v
#define glRasterPos4dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glRasterPos4dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glRasterPos4dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glRasterPos4f_INDEX 339
#define glRasterPos4f_RETURN void
#define glRasterPos4f_ARG_NAMES x, y, z, w
#define glRasterPos4f_ARG_EXPAND GLfloat x, GLfloat y, GLfloat z, GLfloat w
#define glRasterPos4f_PACKED PACKED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glRasterPos4f_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glRasterPos4f_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glRasterPos4fv_INDEX 340
#define glRasterPos4fv_RETURN void
#define glRasterPos4fv_ARG_NAMES v
#define glRasterPos4fv_ARG_EXPAND const GLfloat * v
#define glRasterPos4fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glRasterPos4fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glRasterPos4fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glRasterPos4i_INDEX 341
#define glRasterPos4i_RETURN void
#define glRasterPos4i_ARG_NAMES x, y, z, w
#define glRasterPos4i_ARG_EXPAND GLint x, GLint y, GLint z, GLint w
#define glRasterPos4i_PACKED PACKED_void_GLint_GLint_GLint_GLint
#define glRasterPos4i_INDEXED INDEXED_void_GLint_GLint_GLint_GLint
#define glRasterPos4i_FORMAT FORMAT_void_GLint_GLint_GLint_GLint
#define glRasterPos4iv_INDEX 342
#define glRasterPos4iv_RETURN void
#define glRasterPos4iv_ARG_NAMES v
#define glRasterPos4iv_ARG_EXPAND const GLint * v
#define glRasterPos4iv_PACKED PACKED_void_const_GLint___GENPT__
#define glRasterPos4iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glRasterPos4iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glRasterPos4s_INDEX 343
#define glRasterPos4s_RETURN void
#define glRasterPos4s_ARG_NAMES x, y, z, w
#define glRasterPos4s_ARG_EXPAND GLshort x, GLshort y, GLshort z, GLshort w
#define glRasterPos4s_PACKED PACKED_void_GLshort_GLshort_GLshort_GLshort
#define glRasterPos4s_INDEXED INDEXED_void_GLshort_GLshort_GLshort_GLshort
#define glRasterPos4s_FORMAT FORMAT_void_GLshort_GLshort_GLshort_GLshort
#define glRasterPos4sv_INDEX 344
#define glRasterPos4sv_RETURN void
#define glRasterPos4sv_ARG_NAMES v
#define glRasterPos4sv_ARG_EXPAND const GLshort * v
#define glRasterPos4sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glRasterPos4sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glRasterPos4sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glReadBuffer_INDEX 345
#define glReadBuffer_RETURN void
#define glReadBuffer_ARG_NAMES mode
#define glReadBuffer_ARG_EXPAND GLenum mode
#define glReadBuffer_PACKED PACKED_void_GLenum
#define glReadBuffer_INDEXED INDEXED_void_GLenum
#define glReadBuffer_FORMAT FORMAT_void_GLenum
#define glReadPixels_INDEX 346
#define glReadPixels_RETURN void
#define glReadPixels_ARG_NAMES x, y, width, height, format, type, pixels
#define glReadPixels_ARG_EXPAND GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * pixels
#define glReadPixels_PACKED PACKED_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__
#define glReadPixels_INDEXED INDEXED_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__
#define glReadPixels_FORMAT FORMAT_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__
#define glRectd_INDEX 347
#define glRectd_RETURN void
#define glRectd_ARG_NAMES x1, y1, x2, y2
#define glRectd_ARG_EXPAND GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2
#define glRectd_PACKED PACKED_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glRectd_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glRectd_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glRectdv_INDEX 348
#define glRectdv_RETURN void
#define glRectdv_ARG_NAMES v1, v2
#define glRectdv_ARG_EXPAND const GLdouble * v1, const GLdouble * v2
#define glRectdv_PACKED PACKED_void_const_GLdouble___GENPT___const_GLdouble___GENPT__
#define glRectdv_INDEXED INDEXED_void_const_GLdouble___GENPT___const_GLdouble___GENPT__
#define glRectdv_FORMAT FORMAT_void_const_GLdouble___GENPT___const_GLdouble___GENPT__
#define glRectf_INDEX 349
#define glRectf_RETURN void
#define glRectf_ARG_NAMES x1, y1, x2, y2
#define glRectf_ARG_EXPAND GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2
#define glRectf_PACKED PACKED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glRectf_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glRectf_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glRectfv_INDEX 350
#define glRectfv_RETURN void
#define glRectfv_ARG_NAMES v1, v2
#define glRectfv_ARG_EXPAND const GLfloat * v1, const GLfloat * v2
#define glRectfv_PACKED PACKED_void_const_GLfloat___GENPT___const_GLfloat___GENPT__
#define glRectfv_INDEXED INDEXED_void_const_GLfloat___GENPT___const_GLfloat___GENPT__
#define glRectfv_FORMAT FORMAT_void_const_GLfloat___GENPT___const_GLfloat___GENPT__
#define glRecti_INDEX 351
#define glRecti_RETURN void
#define glRecti_ARG_NAMES x1, y1, x2, y2
#define glRecti_ARG_EXPAND GLint x1, GLint y1, GLint x2, GLint y2
#define glRecti_PACKED PACKED_void_GLint_GLint_GLint_GLint
#define glRecti_INDEXED INDEXED_void_GLint_GLint_GLint_GLint
#define glRecti_FORMAT FORMAT_void_GLint_GLint_GLint_GLint
#define glRectiv_INDEX 352
#define glRectiv_RETURN void
#define glRectiv_ARG_NAMES v1, v2
#define glRectiv_ARG_EXPAND const GLint * v1, const GLint * v2
#define glRectiv_PACKED PACKED_void_const_GLint___GENPT___const_GLint___GENPT__
#define glRectiv_INDEXED INDEXED_void_const_GLint___GENPT___const_GLint___GENPT__
#define glRectiv_FORMAT FORMAT_void_const_GLint___GENPT___const_GLint___GENPT__
#define glRects_INDEX 353
#define glRects_RETURN void
#define glRects_ARG_NAMES x1, y1, x2, y2
#define glRects_ARG_EXPAND GLshort x1, GLshort y1, GLshort x2, GLshort y2
#define glRects_PACKED PACKED_void_GLshort_GLshort_GLshort_GLshort
#define glRects_INDEXED INDEXED_void_GLshort_GLshort_GLshort_GLshort
#define glRects_FORMAT FORMAT_void_GLshort_GLshort_GLshort_GLshort
#define glRectsv_INDEX 354
#define glRectsv_RETURN void
#define glRectsv_ARG_NAMES v1, v2
#define glRectsv_ARG_EXPAND const GLshort * v1, const GLshort * v2
#define glRectsv_PACKED PACKED_void_const_GLshort___GENPT___const_GLshort___GENPT__
#define glRectsv_INDEXED INDEXED_void_const_GLshort___GENPT___const_GLshort___GENPT__
#define glRectsv_FORMAT FORMAT_void_const_GLshort___GENPT___const_GLshort___GENPT__
#define glRenderMode_INDEX 355
#define glRenderMode_RETURN GLint
#define glRenderMode_ARG_NAMES mode
#define glRenderMode_ARG_EXPAND GLenum mode
#define glRenderMode_PACKED PACKED_GLint_GLenum
#define glRenderMode_INDEXED INDEXED_GLint_GLenum
#define glRenderMode_FORMAT FORMAT_GLint_GLenum
#define glResetHistogram_INDEX 356
#define glResetHistogram_RETURN void
#define glResetHistogram_ARG_NAMES target
#define glResetHistogram_ARG_EXPAND GLenum target
#define glResetHistogram_PACKED PACKED_void_GLenum
#define glResetHistogram_INDEXED INDEXED_void_GLenum
#define glResetHistogram_FORMAT FORMAT_void_GLenum
#define glResetMinmax_INDEX 357
#define glResetMinmax_RETURN void
#define glResetMinmax_ARG_NAMES target
#define glResetMinmax_ARG_EXPAND GLenum target
#define glResetMinmax_PACKED PACKED_void_GLenum
#define glResetMinmax_INDEXED INDEXED_void_GLenum
#define glResetMinmax_FORMAT FORMAT_void_GLenum
#define glRotated_INDEX 358
#define glRotated_RETURN void
#define glRotated_ARG_NAMES angle, x, y, z
#define glRotated_ARG_EXPAND GLdouble angle, GLdouble x, GLdouble y, GLdouble z
#define glRotated_PACKED PACKED_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glRotated_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glRotated_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glRotatef_INDEX 359
#define glRotatef_RETURN void
#define glRotatef_ARG_NAMES angle, x, y, z
#define glRotatef_ARG_EXPAND GLfloat angle, GLfloat x, GLfloat y, GLfloat z
#define glRotatef_PACKED PACKED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glRotatef_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glRotatef_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glSampleCoverage_INDEX 360
#define glSampleCoverage_RETURN void
#define glSampleCoverage_ARG_NAMES value, invert
#define glSampleCoverage_ARG_EXPAND GLfloat value, GLboolean invert
#define glSampleCoverage_PACKED PACKED_void_GLfloat_GLboolean
#define glSampleCoverage_INDEXED INDEXED_void_GLfloat_GLboolean
#define glSampleCoverage_FORMAT FORMAT_void_GLfloat_GLboolean
#define glScaled_INDEX 361
#define glScaled_RETURN void
#define glScaled_ARG_NAMES x, y, z
#define glScaled_ARG_EXPAND GLdouble x, GLdouble y, GLdouble z
#define glScaled_PACKED PACKED_void_GLdouble_GLdouble_GLdouble
#define glScaled_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble
#define glScaled_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble
#define glScalef_INDEX 362
#define glScalef_RETURN void
#define glScalef_ARG_NAMES x, y, z
#define glScalef_ARG_EXPAND GLfloat x, GLfloat y, GLfloat z
#define glScalef_PACKED PACKED_void_GLfloat_GLfloat_GLfloat
#define glScalef_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat
#define glScalef_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat
#define glScissor_INDEX 363
#define glScissor_RETURN void
#define glScissor_ARG_NAMES x, y, width, height
#define glScissor_ARG_EXPAND GLint x, GLint y, GLsizei width, GLsizei height
#define glScissor_PACKED PACKED_void_GLint_GLint_GLsizei_GLsizei
#define glScissor_INDEXED INDEXED_void_GLint_GLint_GLsizei_GLsizei
#define glScissor_FORMAT FORMAT_void_GLint_GLint_GLsizei_GLsizei
#define glSecondaryColor3b_INDEX 364
#define glSecondaryColor3b_RETURN void
#define glSecondaryColor3b_ARG_NAMES red, green, blue
#define glSecondaryColor3b_ARG_EXPAND GLbyte red, GLbyte green, GLbyte blue
#define glSecondaryColor3b_PACKED PACKED_void_GLbyte_GLbyte_GLbyte
#define glSecondaryColor3b_INDEXED INDEXED_void_GLbyte_GLbyte_GLbyte
#define glSecondaryColor3b_FORMAT FORMAT_void_GLbyte_GLbyte_GLbyte
#define glSecondaryColor3bv_INDEX 365
#define glSecondaryColor3bv_RETURN void
#define glSecondaryColor3bv_ARG_NAMES v
#define glSecondaryColor3bv_ARG_EXPAND const GLbyte * v
#define glSecondaryColor3bv_PACKED PACKED_void_const_GLbyte___GENPT__
#define glSecondaryColor3bv_INDEXED INDEXED_void_const_GLbyte___GENPT__
#define glSecondaryColor3bv_FORMAT FORMAT_void_const_GLbyte___GENPT__
#define glSecondaryColor3d_INDEX 366
#define glSecondaryColor3d_RETURN void
#define glSecondaryColor3d_ARG_NAMES red, green, blue
#define glSecondaryColor3d_ARG_EXPAND GLdouble red, GLdouble green, GLdouble blue
#define glSecondaryColor3d_PACKED PACKED_void_GLdouble_GLdouble_GLdouble
#define glSecondaryColor3d_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble
#define glSecondaryColor3d_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble
#define glSecondaryColor3dv_INDEX 367
#define glSecondaryColor3dv_RETURN void
#define glSecondaryColor3dv_ARG_NAMES v
#define glSecondaryColor3dv_ARG_EXPAND const GLdouble * v
#define glSecondaryColor3dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glSecondaryColor3dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glSecondaryColor3dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glSecondaryColor3f_INDEX 368
#define glSecondaryColor3f_RETURN void
#define glSecondaryColor3f_ARG_NAMES red, green, blue
#define glSecondaryColor3f_ARG_EXPAND GLfloat red, GLfloat green, GLfloat blue
#define glSecondaryColor3f_PACKED PACKED_void_GLfloat_GLfloat_GLfloat
#define glSecondaryColor3f_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat
#define glSecondaryColor3f_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat
#define glSecondaryColor3fv_INDEX 369
#define glSecondaryColor3fv_RETURN void
#define glSecondaryColor3fv_ARG_NAMES v
#define glSecondaryColor3fv_ARG_EXPAND const GLfloat * v
#define glSecondaryColor3fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glSecondaryColor3fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glSecondaryColor3fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glSecondaryColor3i_INDEX 370
#define glSecondaryColor3i_RETURN void
#define glSecondaryColor3i_ARG_NAMES red, green, blue
#define glSecondaryColor3i_ARG_EXPAND GLint red, GLint green, GLint blue
#define glSecondaryColor3i_PACKED PACKED_void_GLint_GLint_GLint
#define glSecondaryColor3i_INDEXED INDEXED_void_GLint_GLint_GLint
#define glSecondaryColor3i_FORMAT FORMAT_void_GLint_GLint_GLint
#define glSecondaryColor3iv_INDEX 371
#define glSecondaryColor3iv_RETURN void
#define glSecondaryColor3iv_ARG_NAMES v
#define glSecondaryColor3iv_ARG_EXPAND const GLint * v
#define glSecondaryColor3iv_PACKED PACKED_void_const_GLint___GENPT__
#define glSecondaryColor3iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glSecondaryColor3iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glSecondaryColor3s_INDEX 372
#define glSecondaryColor3s_RETURN void
#define glSecondaryColor3s_ARG_NAMES red, green, blue
#define glSecondaryColor3s_ARG_EXPAND GLshort red, GLshort green, GLshort blue
#define glSecondaryColor3s_PACKED PACKED_void_GLshort_GLshort_GLshort
#define glSecondaryColor3s_INDEXED INDEXED_void_GLshort_GLshort_GLshort
#define glSecondaryColor3s_FORMAT FORMAT_void_GLshort_GLshort_GLshort
#define glSecondaryColor3sv_INDEX 373
#define glSecondaryColor3sv_RETURN void
#define glSecondaryColor3sv_ARG_NAMES v
#define glSecondaryColor3sv_ARG_EXPAND const GLshort * v
#define glSecondaryColor3sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glSecondaryColor3sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glSecondaryColor3sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glSecondaryColor3ub_INDEX 374
#define glSecondaryColor3ub_RETURN void
#define glSecondaryColor3ub_ARG_NAMES red, green, blue
#define glSecondaryColor3ub_ARG_EXPAND GLubyte red, GLubyte green, GLubyte blue
#define glSecondaryColor3ub_PACKED PACKED_void_GLubyte_GLubyte_GLubyte
#define glSecondaryColor3ub_INDEXED INDEXED_void_GLubyte_GLubyte_GLubyte
#define glSecondaryColor3ub_FORMAT FORMAT_void_GLubyte_GLubyte_GLubyte
#define glSecondaryColor3ubv_INDEX 375
#define glSecondaryColor3ubv_RETURN void
#define glSecondaryColor3ubv_ARG_NAMES v
#define glSecondaryColor3ubv_ARG_EXPAND const GLubyte * v
#define glSecondaryColor3ubv_PACKED PACKED_void_const_GLubyte___GENPT__
#define glSecondaryColor3ubv_INDEXED INDEXED_void_const_GLubyte___GENPT__
#define glSecondaryColor3ubv_FORMAT FORMAT_void_const_GLubyte___GENPT__
#define glSecondaryColor3ui_INDEX 376
#define glSecondaryColor3ui_RETURN void
#define glSecondaryColor3ui_ARG_NAMES red, green, blue
#define glSecondaryColor3ui_ARG_EXPAND GLuint red, GLuint green, GLuint blue
#define glSecondaryColor3ui_PACKED PACKED_void_GLuint_GLuint_GLuint
#define glSecondaryColor3ui_INDEXED INDEXED_void_GLuint_GLuint_GLuint
#define glSecondaryColor3ui_FORMAT FORMAT_void_GLuint_GLuint_GLuint
#define glSecondaryColor3uiv_INDEX 377
#define glSecondaryColor3uiv_RETURN void
#define glSecondaryColor3uiv_ARG_NAMES v
#define glSecondaryColor3uiv_ARG_EXPAND const GLuint * v
#define glSecondaryColor3uiv_PACKED PACKED_void_const_GLuint___GENPT__
#define glSecondaryColor3uiv_INDEXED INDEXED_void_const_GLuint___GENPT__
#define glSecondaryColor3uiv_FORMAT FORMAT_void_const_GLuint___GENPT__
#define glSecondaryColor3us_INDEX 378
#define glSecondaryColor3us_RETURN void
#define glSecondaryColor3us_ARG_NAMES red, green, blue
#define glSecondaryColor3us_ARG_EXPAND GLushort red, GLushort green, GLushort blue
#define glSecondaryColor3us_PACKED PACKED_void_GLushort_GLushort_GLushort
#define glSecondaryColor3us_INDEXED INDEXED_void_GLushort_GLushort_GLushort
#define glSecondaryColor3us_FORMAT FORMAT_void_GLushort_GLushort_GLushort
#define glSecondaryColor3usv_INDEX 379
#define glSecondaryColor3usv_RETURN void
#define glSecondaryColor3usv_ARG_NAMES v
#define glSecondaryColor3usv_ARG_EXPAND const GLushort * v
#define glSecondaryColor3usv_PACKED PACKED_void_const_GLushort___GENPT__
#define glSecondaryColor3usv_INDEXED INDEXED_void_const_GLushort___GENPT__
#define glSecondaryColor3usv_FORMAT FORMAT_void_const_GLushort___GENPT__
#define glSecondaryColorPointer_INDEX 380
#define glSecondaryColorPointer_RETURN void
#define glSecondaryColorPointer_ARG_NAMES size, type, stride, pointer
#define glSecondaryColorPointer_ARG_EXPAND GLint size, GLenum type, GLsizei stride, const GLvoid * pointer
#define glSecondaryColorPointer_PACKED PACKED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__
#define glSecondaryColorPointer_INDEXED INDEXED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__
#define glSecondaryColorPointer_FORMAT FORMAT_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__
#define glSelectBuffer_INDEX 381
#define glSelectBuffer_RETURN void
#define glSelectBuffer_ARG_NAMES size, buffer
#define glSelectBuffer_ARG_EXPAND GLsizei size, GLuint * buffer
#define glSelectBuffer_PACKED PACKED_void_GLsizei_GLuint___GENPT__
#define glSelectBuffer_INDEXED INDEXED_void_GLsizei_GLuint___GENPT__
#define glSelectBuffer_FORMAT FORMAT_void_GLsizei_GLuint___GENPT__
#define glSeparableFilter2D_INDEX 382
#define glSeparableFilter2D_RETURN void
#define glSeparableFilter2D_ARG_NAMES target, internalformat, width, height, format, type, row, column
#define glSeparableFilter2D_ARG_EXPAND GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * row, const GLvoid * column
#define glSeparableFilter2D_PACKED PACKED_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__
#define glSeparableFilter2D_INDEXED INDEXED_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__
#define glSeparableFilter2D_FORMAT FORMAT_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__
#define glShadeModel_INDEX 383
#define glShadeModel_RETURN void
#define glShadeModel_ARG_NAMES mode
#define glShadeModel_ARG_EXPAND GLenum mode
#define glShadeModel_PACKED PACKED_void_GLenum
#define glShadeModel_INDEXED INDEXED_void_GLenum
#define glShadeModel_FORMAT FORMAT_void_GLenum
#define glStencilFunc_INDEX 384
#define glStencilFunc_RETURN void
#define glStencilFunc_ARG_NAMES func, ref, mask
#define glStencilFunc_ARG_EXPAND GLenum func, GLint ref, GLuint mask
#define glStencilFunc_PACKED PACKED_void_GLenum_GLint_GLuint
#define glStencilFunc_INDEXED INDEXED_void_GLenum_GLint_GLuint
#define glStencilFunc_FORMAT FORMAT_void_GLenum_GLint_GLuint
#define glStencilMask_INDEX 385
#define glStencilMask_RETURN void
#define glStencilMask_ARG_NAMES mask
#define glStencilMask_ARG_EXPAND GLuint mask
#define glStencilMask_PACKED PACKED_void_GLuint
#define glStencilMask_INDEXED INDEXED_void_GLuint
#define glStencilMask_FORMAT FORMAT_void_GLuint
#define glStencilOp_INDEX 386
#define glStencilOp_RETURN void
#define glStencilOp_ARG_NAMES fail, zfail, zpass
#define glStencilOp_ARG_EXPAND GLenum fail, GLenum zfail, GLenum zpass
#define glStencilOp_PACKED PACKED_void_GLenum_GLenum_GLenum
#define glStencilOp_INDEXED INDEXED_void_GLenum_GLenum_GLenum
#define glStencilOp_FORMAT FORMAT_void_GLenum_GLenum_GLenum
#define glTexCoord1d_INDEX 387
#define glTexCoord1d_RETURN void
#define glTexCoord1d_ARG_NAMES s
#define glTexCoord1d_ARG_EXPAND GLdouble s
#define glTexCoord1d_PACKED PACKED_void_GLdouble
#define glTexCoord1d_INDEXED INDEXED_void_GLdouble
#define glTexCoord1d_FORMAT FORMAT_void_GLdouble
#define glTexCoord1dv_INDEX 388
#define glTexCoord1dv_RETURN void
#define glTexCoord1dv_ARG_NAMES v
#define glTexCoord1dv_ARG_EXPAND const GLdouble * v
#define glTexCoord1dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glTexCoord1dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glTexCoord1dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glTexCoord1f_INDEX 389
#define glTexCoord1f_RETURN void
#define glTexCoord1f_ARG_NAMES s
#define glTexCoord1f_ARG_EXPAND GLfloat s
#define glTexCoord1f_PACKED PACKED_void_GLfloat
#define glTexCoord1f_INDEXED INDEXED_void_GLfloat
#define glTexCoord1f_FORMAT FORMAT_void_GLfloat
#define glTexCoord1fv_INDEX 390
#define glTexCoord1fv_RETURN void
#define glTexCoord1fv_ARG_NAMES v
#define glTexCoord1fv_ARG_EXPAND const GLfloat * v
#define glTexCoord1fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glTexCoord1fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glTexCoord1fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glTexCoord1i_INDEX 391
#define glTexCoord1i_RETURN void
#define glTexCoord1i_ARG_NAMES s
#define glTexCoord1i_ARG_EXPAND GLint s
#define glTexCoord1i_PACKED PACKED_void_GLint
#define glTexCoord1i_INDEXED INDEXED_void_GLint
#define glTexCoord1i_FORMAT FORMAT_void_GLint
#define glTexCoord1iv_INDEX 392
#define glTexCoord1iv_RETURN void
#define glTexCoord1iv_ARG_NAMES v
#define glTexCoord1iv_ARG_EXPAND const GLint * v
#define glTexCoord1iv_PACKED PACKED_void_const_GLint___GENPT__
#define glTexCoord1iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glTexCoord1iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glTexCoord1s_INDEX 393
#define glTexCoord1s_RETURN void
#define glTexCoord1s_ARG_NAMES s
#define glTexCoord1s_ARG_EXPAND GLshort s
#define glTexCoord1s_PACKED PACKED_void_GLshort
#define glTexCoord1s_INDEXED INDEXED_void_GLshort
#define glTexCoord1s_FORMAT FORMAT_void_GLshort
#define glTexCoord1sv_INDEX 394
#define glTexCoord1sv_RETURN void
#define glTexCoord1sv_ARG_NAMES v
#define glTexCoord1sv_ARG_EXPAND const GLshort * v
#define glTexCoord1sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glTexCoord1sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glTexCoord1sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glTexCoord2d_INDEX 395
#define glTexCoord2d_RETURN void
#define glTexCoord2d_ARG_NAMES s, t
#define glTexCoord2d_ARG_EXPAND GLdouble s, GLdouble t
#define glTexCoord2d_PACKED PACKED_void_GLdouble_GLdouble
#define glTexCoord2d_INDEXED INDEXED_void_GLdouble_GLdouble
#define glTexCoord2d_FORMAT FORMAT_void_GLdouble_GLdouble
#define glTexCoord2dv_INDEX 396
#define glTexCoord2dv_RETURN void
#define glTexCoord2dv_ARG_NAMES v
#define glTexCoord2dv_ARG_EXPAND const GLdouble * v
#define glTexCoord2dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glTexCoord2dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glTexCoord2dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glTexCoord2f_INDEX 397
#define glTexCoord2f_RETURN void
#define glTexCoord2f_ARG_NAMES s, t
#define glTexCoord2f_ARG_EXPAND GLfloat s, GLfloat t
#define glTexCoord2f_PACKED PACKED_void_GLfloat_GLfloat
#define glTexCoord2f_INDEXED INDEXED_void_GLfloat_GLfloat
#define glTexCoord2f_FORMAT FORMAT_void_GLfloat_GLfloat
#define glTexCoord2fv_INDEX 398
#define glTexCoord2fv_RETURN void
#define glTexCoord2fv_ARG_NAMES v
#define glTexCoord2fv_ARG_EXPAND const GLfloat * v
#define glTexCoord2fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glTexCoord2fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glTexCoord2fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glTexCoord2i_INDEX 399
#define glTexCoord2i_RETURN void
#define glTexCoord2i_ARG_NAMES s, t
#define glTexCoord2i_ARG_EXPAND GLint s, GLint t
#define glTexCoord2i_PACKED PACKED_void_GLint_GLint
#define glTexCoord2i_INDEXED INDEXED_void_GLint_GLint
#define glTexCoord2i_FORMAT FORMAT_void_GLint_GLint
#define glTexCoord2iv_INDEX 400
#define glTexCoord2iv_RETURN void
#define glTexCoord2iv_ARG_NAMES v
#define glTexCoord2iv_ARG_EXPAND const GLint * v
#define glTexCoord2iv_PACKED PACKED_void_const_GLint___GENPT__
#define glTexCoord2iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glTexCoord2iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glTexCoord2s_INDEX 401
#define glTexCoord2s_RETURN void
#define glTexCoord2s_ARG_NAMES s, t
#define glTexCoord2s_ARG_EXPAND GLshort s, GLshort t
#define glTexCoord2s_PACKED PACKED_void_GLshort_GLshort
#define glTexCoord2s_INDEXED INDEXED_void_GLshort_GLshort
#define glTexCoord2s_FORMAT FORMAT_void_GLshort_GLshort
#define glTexCoord2sv_INDEX 402
#define glTexCoord2sv_RETURN void
#define glTexCoord2sv_ARG_NAMES v
#define glTexCoord2sv_ARG_EXPAND const GLshort * v
#define glTexCoord2sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glTexCoord2sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glTexCoord2sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glTexCoord3d_INDEX 403
#define glTexCoord3d_RETURN void
#define glTexCoord3d_ARG_NAMES s, t, r
#define glTexCoord3d_ARG_EXPAND GLdouble s, GLdouble t, GLdouble r
#define glTexCoord3d_PACKED PACKED_void_GLdouble_GLdouble_GLdouble
#define glTexCoord3d_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble
#define glTexCoord3d_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble
#define glTexCoord3dv_INDEX 404
#define glTexCoord3dv_RETURN void
#define glTexCoord3dv_ARG_NAMES v
#define glTexCoord3dv_ARG_EXPAND const GLdouble * v
#define glTexCoord3dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glTexCoord3dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glTexCoord3dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glTexCoord3f_INDEX 405
#define glTexCoord3f_RETURN void
#define glTexCoord3f_ARG_NAMES s, t, r
#define glTexCoord3f_ARG_EXPAND GLfloat s, GLfloat t, GLfloat r
#define glTexCoord3f_PACKED PACKED_void_GLfloat_GLfloat_GLfloat
#define glTexCoord3f_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat
#define glTexCoord3f_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat
#define glTexCoord3fv_INDEX 406
#define glTexCoord3fv_RETURN void
#define glTexCoord3fv_ARG_NAMES v
#define glTexCoord3fv_ARG_EXPAND const GLfloat * v
#define glTexCoord3fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glTexCoord3fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glTexCoord3fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glTexCoord3i_INDEX 407
#define glTexCoord3i_RETURN void
#define glTexCoord3i_ARG_NAMES s, t, r
#define glTexCoord3i_ARG_EXPAND GLint s, GLint t, GLint r
#define glTexCoord3i_PACKED PACKED_void_GLint_GLint_GLint
#define glTexCoord3i_INDEXED INDEXED_void_GLint_GLint_GLint
#define glTexCoord3i_FORMAT FORMAT_void_GLint_GLint_GLint
#define glTexCoord3iv_INDEX 408
#define glTexCoord3iv_RETURN void
#define glTexCoord3iv_ARG_NAMES v
#define glTexCoord3iv_ARG_EXPAND const GLint * v
#define glTexCoord3iv_PACKED PACKED_void_const_GLint___GENPT__
#define glTexCoord3iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glTexCoord3iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glTexCoord3s_INDEX 409
#define glTexCoord3s_RETURN void
#define glTexCoord3s_ARG_NAMES s, t, r
#define glTexCoord3s_ARG_EXPAND GLshort s, GLshort t, GLshort r
#define glTexCoord3s_PACKED PACKED_void_GLshort_GLshort_GLshort
#define glTexCoord3s_INDEXED INDEXED_void_GLshort_GLshort_GLshort
#define glTexCoord3s_FORMAT FORMAT_void_GLshort_GLshort_GLshort
#define glTexCoord3sv_INDEX 410
#define glTexCoord3sv_RETURN void
#define glTexCoord3sv_ARG_NAMES v
#define glTexCoord3sv_ARG_EXPAND const GLshort * v
#define glTexCoord3sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glTexCoord3sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glTexCoord3sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glTexCoord4d_INDEX 411
#define glTexCoord4d_RETURN void
#define glTexCoord4d_ARG_NAMES s, t, r, q
#define glTexCoord4d_ARG_EXPAND GLdouble s, GLdouble t, GLdouble r, GLdouble q
#define glTexCoord4d_PACKED PACKED_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glTexCoord4d_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glTexCoord4d_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glTexCoord4dv_INDEX 412
#define glTexCoord4dv_RETURN void
#define glTexCoord4dv_ARG_NAMES v
#define glTexCoord4dv_ARG_EXPAND const GLdouble * v
#define glTexCoord4dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glTexCoord4dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glTexCoord4dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glTexCoord4f_INDEX 413
#define glTexCoord4f_RETURN void
#define glTexCoord4f_ARG_NAMES s, t, r, q
#define glTexCoord4f_ARG_EXPAND GLfloat s, GLfloat t, GLfloat r, GLfloat q
#define glTexCoord4f_PACKED PACKED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glTexCoord4f_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glTexCoord4f_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glTexCoord4fv_INDEX 414
#define glTexCoord4fv_RETURN void
#define glTexCoord4fv_ARG_NAMES v
#define glTexCoord4fv_ARG_EXPAND const GLfloat * v
#define glTexCoord4fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glTexCoord4fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glTexCoord4fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glTexCoord4i_INDEX 415
#define glTexCoord4i_RETURN void
#define glTexCoord4i_ARG_NAMES s, t, r, q
#define glTexCoord4i_ARG_EXPAND GLint s, GLint t, GLint r, GLint q
#define glTexCoord4i_PACKED PACKED_void_GLint_GLint_GLint_GLint
#define glTexCoord4i_INDEXED INDEXED_void_GLint_GLint_GLint_GLint
#define glTexCoord4i_FORMAT FORMAT_void_GLint_GLint_GLint_GLint
#define glTexCoord4iv_INDEX 416
#define glTexCoord4iv_RETURN void
#define glTexCoord4iv_ARG_NAMES v
#define glTexCoord4iv_ARG_EXPAND const GLint * v
#define glTexCoord4iv_PACKED PACKED_void_const_GLint___GENPT__
#define glTexCoord4iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glTexCoord4iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glTexCoord4s_INDEX 417
#define glTexCoord4s_RETURN void
#define glTexCoord4s_ARG_NAMES s, t, r, q
#define glTexCoord4s_ARG_EXPAND GLshort s, GLshort t, GLshort r, GLshort q
#define glTexCoord4s_PACKED PACKED_void_GLshort_GLshort_GLshort_GLshort
#define glTexCoord4s_INDEXED INDEXED_void_GLshort_GLshort_GLshort_GLshort
#define glTexCoord4s_FORMAT FORMAT_void_GLshort_GLshort_GLshort_GLshort
#define glTexCoord4sv_INDEX 418
#define glTexCoord4sv_RETURN void
#define glTexCoord4sv_ARG_NAMES v
#define glTexCoord4sv_ARG_EXPAND const GLshort * v
#define glTexCoord4sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glTexCoord4sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glTexCoord4sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glTexCoordPointer_INDEX 419
#define glTexCoordPointer_RETURN void
#define glTexCoordPointer_ARG_NAMES size, type, stride, pointer
#define glTexCoordPointer_ARG_EXPAND GLint size, GLenum type, GLsizei stride, const GLvoid * pointer
#define glTexCoordPointer_PACKED PACKED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__
#define glTexCoordPointer_INDEXED INDEXED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__
#define glTexCoordPointer_FORMAT FORMAT_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__
#define glTexEnvf_INDEX 420
#define glTexEnvf_RETURN void
#define glTexEnvf_ARG_NAMES target, pname, param
#define glTexEnvf_ARG_EXPAND GLenum target, GLenum pname, GLfloat param
#define glTexEnvf_PACKED PACKED_void_GLenum_GLenum_GLfloat
#define glTexEnvf_INDEXED INDEXED_void_GLenum_GLenum_GLfloat
#define glTexEnvf_FORMAT FORMAT_void_GLenum_GLenum_GLfloat
#define glTexEnvfv_INDEX 421
#define glTexEnvfv_RETURN void
#define glTexEnvfv_ARG_NAMES target, pname, params
#define glTexEnvfv_ARG_EXPAND GLenum target, GLenum pname, const GLfloat * params
#define glTexEnvfv_PACKED PACKED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glTexEnvfv_INDEXED INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glTexEnvfv_FORMAT FORMAT_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glTexEnvi_INDEX 422
#define glTexEnvi_RETURN void
#define glTexEnvi_ARG_NAMES target, pname, param
#define glTexEnvi_ARG_EXPAND GLenum target, GLenum pname, GLint param
#define glTexEnvi_PACKED PACKED_void_GLenum_GLenum_GLint
#define glTexEnvi_INDEXED INDEXED_void_GLenum_GLenum_GLint
#define glTexEnvi_FORMAT FORMAT_void_GLenum_GLenum_GLint
#define glTexEnviv_INDEX 423
#define glTexEnviv_RETURN void
#define glTexEnviv_ARG_NAMES target, pname, params
#define glTexEnviv_ARG_EXPAND GLenum target, GLenum pname, const GLint * params
#define glTexEnviv_PACKED PACKED_void_GLenum_GLenum_const_GLint___GENPT__
#define glTexEnviv_INDEXED INDEXED_void_GLenum_GLenum_const_GLint___GENPT__
#define glTexEnviv_FORMAT FORMAT_void_GLenum_GLenum_const_GLint___GENPT__
#define glTexGend_INDEX 424
#define glTexGend_RETURN void
#define glTexGend_ARG_NAMES coord, pname, param
#define glTexGend_ARG_EXPAND GLenum coord, GLenum pname, GLdouble param
#define glTexGend_PACKED PACKED_void_GLenum_GLenum_GLdouble
#define glTexGend_INDEXED INDEXED_void_GLenum_GLenum_GLdouble
#define glTexGend_FORMAT FORMAT_void_GLenum_GLenum_GLdouble
#define glTexGendv_INDEX 425
#define glTexGendv_RETURN void
#define glTexGendv_ARG_NAMES coord, pname, params
#define glTexGendv_ARG_EXPAND GLenum coord, GLenum pname, const GLdouble * params
#define glTexGendv_PACKED PACKED_void_GLenum_GLenum_const_GLdouble___GENPT__
#define glTexGendv_INDEXED INDEXED_void_GLenum_GLenum_const_GLdouble___GENPT__
#define glTexGendv_FORMAT FORMAT_void_GLenum_GLenum_const_GLdouble___GENPT__
#define glTexGenf_INDEX 426
#define glTexGenf_RETURN void
#define glTexGenf_ARG_NAMES coord, pname, param
#define glTexGenf_ARG_EXPAND GLenum coord, GLenum pname, GLfloat param
#define glTexGenf_PACKED PACKED_void_GLenum_GLenum_GLfloat
#define glTexGenf_INDEXED INDEXED_void_GLenum_GLenum_GLfloat
#define glTexGenf_FORMAT FORMAT_void_GLenum_GLenum_GLfloat
#define glTexGenfv_INDEX 427
#define glTexGenfv_RETURN void
#define glTexGenfv_ARG_NAMES coord, pname, params
#define glTexGenfv_ARG_EXPAND GLenum coord, GLenum pname, const GLfloat * params
#define glTexGenfv_PACKED PACKED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glTexGenfv_INDEXED INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glTexGenfv_FORMAT FORMAT_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glTexGeni_INDEX 428
#define glTexGeni_RETURN void
#define glTexGeni_ARG_NAMES coord, pname, param
#define glTexGeni_ARG_EXPAND GLenum coord, GLenum pname, GLint param
#define glTexGeni_PACKED PACKED_void_GLenum_GLenum_GLint
#define glTexGeni_INDEXED INDEXED_void_GLenum_GLenum_GLint
#define glTexGeni_FORMAT FORMAT_void_GLenum_GLenum_GLint
#define glTexGeniv_INDEX 429
#define glTexGeniv_RETURN void
#define glTexGeniv_ARG_NAMES coord, pname, params
#define glTexGeniv_ARG_EXPAND GLenum coord, GLenum pname, const GLint * params
#define glTexGeniv_PACKED PACKED_void_GLenum_GLenum_const_GLint___GENPT__
#define glTexGeniv_INDEXED INDEXED_void_GLenum_GLenum_const_GLint___GENPT__
#define glTexGeniv_FORMAT FORMAT_void_GLenum_GLenum_const_GLint___GENPT__
#define glTexImage1D_INDEX 430
#define glTexImage1D_RETURN void
#define glTexImage1D_ARG_NAMES target, level, internalformat, width, border, format, type, pixels
#define glTexImage1D_ARG_EXPAND GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid * pixels
#define glTexImage1D_PACKED PACKED_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexImage1D_INDEXED INDEXED_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexImage1D_FORMAT FORMAT_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexImage2D_INDEX 431
#define glTexImage2D_RETURN void
#define glTexImage2D_ARG_NAMES target, level, internalformat, width, height, border, format, type, pixels
#define glTexImage2D_ARG_EXPAND GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels
#define glTexImage2D_PACKED PACKED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexImage2D_INDEXED INDEXED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexImage2D_FORMAT FORMAT_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexImage3D_INDEX 432
#define glTexImage3D_RETURN void
#define glTexImage3D_ARG_NAMES target, level, internalformat, width, height, depth, border, format, type, pixels
#define glTexImage3D_ARG_EXPAND GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels
#define glTexImage3D_PACKED PACKED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexImage3D_INDEXED INDEXED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexImage3D_FORMAT FORMAT_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexParameterf_INDEX 433
#define glTexParameterf_RETURN void
#define glTexParameterf_ARG_NAMES target, pname, param
#define glTexParameterf_ARG_EXPAND GLenum target, GLenum pname, GLfloat param
#define glTexParameterf_PACKED PACKED_void_GLenum_GLenum_GLfloat
#define glTexParameterf_INDEXED INDEXED_void_GLenum_GLenum_GLfloat
#define glTexParameterf_FORMAT FORMAT_void_GLenum_GLenum_GLfloat
#define glTexParameterfv_INDEX 434
#define glTexParameterfv_RETURN void
#define glTexParameterfv_ARG_NAMES target, pname, params
#define glTexParameterfv_ARG_EXPAND GLenum target, GLenum pname, const GLfloat * params
#define glTexParameterfv_PACKED PACKED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glTexParameterfv_INDEXED INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glTexParameterfv_FORMAT FORMAT_void_GLenum_GLenum_const_GLfloat___GENPT__
#define glTexParameteri_INDEX 435
#define glTexParameteri_RETURN void
#define glTexParameteri_ARG_NAMES target, pname, param
#define glTexParameteri_ARG_EXPAND GLenum target, GLenum pname, GLint param
#define glTexParameteri_PACKED PACKED_void_GLenum_GLenum_GLint
#define glTexParameteri_INDEXED INDEXED_void_GLenum_GLenum_GLint
#define glTexParameteri_FORMAT FORMAT_void_GLenum_GLenum_GLint
#define glTexParameteriv_INDEX 436
#define glTexParameteriv_RETURN void
#define glTexParameteriv_ARG_NAMES target, pname, params
#define glTexParameteriv_ARG_EXPAND GLenum target, GLenum pname, const GLint * params
#define glTexParameteriv_PACKED PACKED_void_GLenum_GLenum_const_GLint___GENPT__
#define glTexParameteriv_INDEXED INDEXED_void_GLenum_GLenum_const_GLint___GENPT__
#define glTexParameteriv_FORMAT FORMAT_void_GLenum_GLenum_const_GLint___GENPT__
#define glTexSubImage1D_INDEX 437
#define glTexSubImage1D_RETURN void
#define glTexSubImage1D_ARG_NAMES target, level, xoffset, width, format, type, pixels
#define glTexSubImage1D_ARG_EXPAND GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels
#define glTexSubImage1D_PACKED PACKED_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexSubImage1D_INDEXED INDEXED_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexSubImage1D_FORMAT FORMAT_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexSubImage2D_INDEX 438
#define glTexSubImage2D_RETURN void
#define glTexSubImage2D_ARG_NAMES target, level, xoffset, yoffset, width, height, format, type, pixels
#define glTexSubImage2D_ARG_EXPAND GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels
#define glTexSubImage2D_PACKED PACKED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexSubImage2D_INDEXED INDEXED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexSubImage2D_FORMAT FORMAT_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexSubImage3D_INDEX 439
#define glTexSubImage3D_RETURN void
#define glTexSubImage3D_ARG_NAMES target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels
#define glTexSubImage3D_ARG_EXPAND GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels
#define glTexSubImage3D_PACKED PACKED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexSubImage3D_INDEXED INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glTexSubImage3D_FORMAT FORMAT_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__
#define glTranslated_INDEX 440
#define glTranslated_RETURN void
#define glTranslated_ARG_NAMES x, y, z
#define glTranslated_ARG_EXPAND GLdouble x, GLdouble y, GLdouble z
#define glTranslated_PACKED PACKED_void_GLdouble_GLdouble_GLdouble
#define glTranslated_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble
#define glTranslated_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble
#define glTranslatef_INDEX 441
#define glTranslatef_RETURN void
#define glTranslatef_ARG_NAMES x, y, z
#define glTranslatef_ARG_EXPAND GLfloat x, GLfloat y, GLfloat z
#define glTranslatef_PACKED PACKED_void_GLfloat_GLfloat_GLfloat
#define glTranslatef_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat
#define glTranslatef_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat
#define glUnmapBuffer_INDEX 442
#define glUnmapBuffer_RETURN GLboolean
#define glUnmapBuffer_ARG_NAMES target
#define glUnmapBuffer_ARG_EXPAND GLenum target
#define glUnmapBuffer_PACKED PACKED_GLboolean_GLenum
#define glUnmapBuffer_INDEXED INDEXED_GLboolean_GLenum
#define glUnmapBuffer_FORMAT FORMAT_GLboolean_GLenum
#define glVertex2d_INDEX 443
#define glVertex2d_RETURN void
#define glVertex2d_ARG_NAMES x, y
#define glVertex2d_ARG_EXPAND GLdouble x, GLdouble y
#define glVertex2d_PACKED PACKED_void_GLdouble_GLdouble
#define glVertex2d_INDEXED INDEXED_void_GLdouble_GLdouble
#define glVertex2d_FORMAT FORMAT_void_GLdouble_GLdouble
#define glVertex2dv_INDEX 444
#define glVertex2dv_RETURN void
#define glVertex2dv_ARG_NAMES v
#define glVertex2dv_ARG_EXPAND const GLdouble * v
#define glVertex2dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glVertex2dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glVertex2dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glVertex2f_INDEX 445
#define glVertex2f_RETURN void
#define glVertex2f_ARG_NAMES x, y
#define glVertex2f_ARG_EXPAND GLfloat x, GLfloat y
#define glVertex2f_PACKED PACKED_void_GLfloat_GLfloat
#define glVertex2f_INDEXED INDEXED_void_GLfloat_GLfloat
#define glVertex2f_FORMAT FORMAT_void_GLfloat_GLfloat
#define glVertex2fv_INDEX 446
#define glVertex2fv_RETURN void
#define glVertex2fv_ARG_NAMES v
#define glVertex2fv_ARG_EXPAND const GLfloat * v
#define glVertex2fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glVertex2fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glVertex2fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glVertex2i_INDEX 447
#define glVertex2i_RETURN void
#define glVertex2i_ARG_NAMES x, y
#define glVertex2i_ARG_EXPAND GLint x, GLint y
#define glVertex2i_PACKED PACKED_void_GLint_GLint
#define glVertex2i_INDEXED INDEXED_void_GLint_GLint
#define glVertex2i_FORMAT FORMAT_void_GLint_GLint
#define glVertex2iv_INDEX 448
#define glVertex2iv_RETURN void
#define glVertex2iv_ARG_NAMES v
#define glVertex2iv_ARG_EXPAND const GLint * v
#define glVertex2iv_PACKED PACKED_void_const_GLint___GENPT__
#define glVertex2iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glVertex2iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glVertex2s_INDEX 449
#define glVertex2s_RETURN void
#define glVertex2s_ARG_NAMES x, y
#define glVertex2s_ARG_EXPAND GLshort x, GLshort y
#define glVertex2s_PACKED PACKED_void_GLshort_GLshort
#define glVertex2s_INDEXED INDEXED_void_GLshort_GLshort
#define glVertex2s_FORMAT FORMAT_void_GLshort_GLshort
#define glVertex2sv_INDEX 450
#define glVertex2sv_RETURN void
#define glVertex2sv_ARG_NAMES v
#define glVertex2sv_ARG_EXPAND const GLshort * v
#define glVertex2sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glVertex2sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glVertex2sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glVertex3d_INDEX 451
#define glVertex3d_RETURN void
#define glVertex3d_ARG_NAMES x, y, z
#define glVertex3d_ARG_EXPAND GLdouble x, GLdouble y, GLdouble z
#define glVertex3d_PACKED PACKED_void_GLdouble_GLdouble_GLdouble
#define glVertex3d_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble
#define glVertex3d_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble
#define glVertex3dv_INDEX 452
#define glVertex3dv_RETURN void
#define glVertex3dv_ARG_NAMES v
#define glVertex3dv_ARG_EXPAND const GLdouble * v
#define glVertex3dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glVertex3dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glVertex3dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glVertex3f_INDEX 453
#define glVertex3f_RETURN void
#define glVertex3f_ARG_NAMES x, y, z
#define glVertex3f_ARG_EXPAND GLfloat x, GLfloat y, GLfloat z
#define glVertex3f_PACKED PACKED_void_GLfloat_GLfloat_GLfloat
#define glVertex3f_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat
#define glVertex3f_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat
#define glVertex3fv_INDEX 454
#define glVertex3fv_RETURN void
#define glVertex3fv_ARG_NAMES v
#define glVertex3fv_ARG_EXPAND const GLfloat * v
#define glVertex3fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glVertex3fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glVertex3fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glVertex3i_INDEX 455
#define glVertex3i_RETURN void
#define glVertex3i_ARG_NAMES x, y, z
#define glVertex3i_ARG_EXPAND GLint x, GLint y, GLint z
#define glVertex3i_PACKED PACKED_void_GLint_GLint_GLint
#define glVertex3i_INDEXED INDEXED_void_GLint_GLint_GLint
#define glVertex3i_FORMAT FORMAT_void_GLint_GLint_GLint
#define glVertex3iv_INDEX 456
#define glVertex3iv_RETURN void
#define glVertex3iv_ARG_NAMES v
#define glVertex3iv_ARG_EXPAND const GLint * v
#define glVertex3iv_PACKED PACKED_void_const_GLint___GENPT__
#define glVertex3iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glVertex3iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glVertex3s_INDEX 457
#define glVertex3s_RETURN void
#define glVertex3s_ARG_NAMES x, y, z
#define glVertex3s_ARG_EXPAND GLshort x, GLshort y, GLshort z
#define glVertex3s_PACKED PACKED_void_GLshort_GLshort_GLshort
#define glVertex3s_INDEXED INDEXED_void_GLshort_GLshort_GLshort
#define glVertex3s_FORMAT FORMAT_void_GLshort_GLshort_GLshort
#define glVertex3sv_INDEX 458
#define glVertex3sv_RETURN void
#define glVertex3sv_ARG_NAMES v
#define glVertex3sv_ARG_EXPAND const GLshort * v
#define glVertex3sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glVertex3sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glVertex3sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glVertex4d_INDEX 459
#define glVertex4d_RETURN void
#define glVertex4d_ARG_NAMES x, y, z, w
#define glVertex4d_ARG_EXPAND GLdouble x, GLdouble y, GLdouble z, GLdouble w
#define glVertex4d_PACKED PACKED_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glVertex4d_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glVertex4d_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble_GLdouble
#define glVertex4dv_INDEX 460
#define glVertex4dv_RETURN void
#define glVertex4dv_ARG_NAMES v
#define glVertex4dv_ARG_EXPAND const GLdouble * v
#define glVertex4dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glVertex4dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glVertex4dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glVertex4f_INDEX 461
#define glVertex4f_RETURN void
#define glVertex4f_ARG_NAMES x, y, z, w
#define glVertex4f_ARG_EXPAND GLfloat x, GLfloat y, GLfloat z, GLfloat w
#define glVertex4f_PACKED PACKED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glVertex4f_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glVertex4f_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat_GLfloat
#define glVertex4fv_INDEX 462
#define glVertex4fv_RETURN void
#define glVertex4fv_ARG_NAMES v
#define glVertex4fv_ARG_EXPAND const GLfloat * v
#define glVertex4fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glVertex4fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glVertex4fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glVertex4i_INDEX 463
#define glVertex4i_RETURN void
#define glVertex4i_ARG_NAMES x, y, z, w
#define glVertex4i_ARG_EXPAND GLint x, GLint y, GLint z, GLint w
#define glVertex4i_PACKED PACKED_void_GLint_GLint_GLint_GLint
#define glVertex4i_INDEXED INDEXED_void_GLint_GLint_GLint_GLint
#define glVertex4i_FORMAT FORMAT_void_GLint_GLint_GLint_GLint
#define glVertex4iv_INDEX 464
#define glVertex4iv_RETURN void
#define glVertex4iv_ARG_NAMES v
#define glVertex4iv_ARG_EXPAND const GLint * v
#define glVertex4iv_PACKED PACKED_void_const_GLint___GENPT__
#define glVertex4iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glVertex4iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glVertex4s_INDEX 465
#define glVertex4s_RETURN void
#define glVertex4s_ARG_NAMES x, y, z, w
#define glVertex4s_ARG_EXPAND GLshort x, GLshort y, GLshort z, GLshort w
#define glVertex4s_PACKED PACKED_void_GLshort_GLshort_GLshort_GLshort
#define glVertex4s_INDEXED INDEXED_void_GLshort_GLshort_GLshort_GLshort
#define glVertex4s_FORMAT FORMAT_void_GLshort_GLshort_GLshort_GLshort
#define glVertex4sv_INDEX 466
#define glVertex4sv_RETURN void
#define glVertex4sv_ARG_NAMES v
#define glVertex4sv_ARG_EXPAND const GLshort * v
#define glVertex4sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glVertex4sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glVertex4sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glVertexPointer_INDEX 467
#define glVertexPointer_RETURN void
#define glVertexPointer_ARG_NAMES size, type, stride, pointer
#define glVertexPointer_ARG_EXPAND GLint size, GLenum type, GLsizei stride, const GLvoid * pointer
#define glVertexPointer_PACKED PACKED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__
#define glVertexPointer_INDEXED INDEXED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__
#define glVertexPointer_FORMAT FORMAT_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__
#define glViewport_INDEX 468
#define glViewport_RETURN void
#define glViewport_ARG_NAMES x, y, width, height
#define glViewport_ARG_EXPAND GLint x, GLint y, GLsizei width, GLsizei height
#define glViewport_PACKED PACKED_void_GLint_GLint_GLsizei_GLsizei
#define glViewport_INDEXED INDEXED_void_GLint_GLint_GLsizei_GLsizei
#define glViewport_FORMAT FORMAT_void_GLint_GLint_GLsizei_GLsizei
#define glWindowPos2d_INDEX 469
#define glWindowPos2d_RETURN void
#define glWindowPos2d_ARG_NAMES x, y
#define glWindowPos2d_ARG_EXPAND GLdouble x, GLdouble y
#define glWindowPos2d_PACKED PACKED_void_GLdouble_GLdouble
#define glWindowPos2d_INDEXED INDEXED_void_GLdouble_GLdouble
#define glWindowPos2d_FORMAT FORMAT_void_GLdouble_GLdouble
#define glWindowPos2dv_INDEX 470
#define glWindowPos2dv_RETURN void
#define glWindowPos2dv_ARG_NAMES v
#define glWindowPos2dv_ARG_EXPAND const GLdouble * v
#define glWindowPos2dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glWindowPos2dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glWindowPos2dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glWindowPos2f_INDEX 471
#define glWindowPos2f_RETURN void
#define glWindowPos2f_ARG_NAMES x, y
#define glWindowPos2f_ARG_EXPAND GLfloat x, GLfloat y
#define glWindowPos2f_PACKED PACKED_void_GLfloat_GLfloat
#define glWindowPos2f_INDEXED INDEXED_void_GLfloat_GLfloat
#define glWindowPos2f_FORMAT FORMAT_void_GLfloat_GLfloat
#define glWindowPos2fv_INDEX 472
#define glWindowPos2fv_RETURN void
#define glWindowPos2fv_ARG_NAMES v
#define glWindowPos2fv_ARG_EXPAND const GLfloat * v
#define glWindowPos2fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glWindowPos2fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glWindowPos2fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glWindowPos2i_INDEX 473
#define glWindowPos2i_RETURN void
#define glWindowPos2i_ARG_NAMES x, y
#define glWindowPos2i_ARG_EXPAND GLint x, GLint y
#define glWindowPos2i_PACKED PACKED_void_GLint_GLint
#define glWindowPos2i_INDEXED INDEXED_void_GLint_GLint
#define glWindowPos2i_FORMAT FORMAT_void_GLint_GLint
#define glWindowPos2iv_INDEX 474
#define glWindowPos2iv_RETURN void
#define glWindowPos2iv_ARG_NAMES v
#define glWindowPos2iv_ARG_EXPAND const GLint * v
#define glWindowPos2iv_PACKED PACKED_void_const_GLint___GENPT__
#define glWindowPos2iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glWindowPos2iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glWindowPos2s_INDEX 475
#define glWindowPos2s_RETURN void
#define glWindowPos2s_ARG_NAMES x, y
#define glWindowPos2s_ARG_EXPAND GLshort x, GLshort y
#define glWindowPos2s_PACKED PACKED_void_GLshort_GLshort
#define glWindowPos2s_INDEXED INDEXED_void_GLshort_GLshort
#define glWindowPos2s_FORMAT FORMAT_void_GLshort_GLshort
#define glWindowPos2sv_INDEX 476
#define glWindowPos2sv_RETURN void
#define glWindowPos2sv_ARG_NAMES v
#define glWindowPos2sv_ARG_EXPAND const GLshort * v
#define glWindowPos2sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glWindowPos2sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glWindowPos2sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glWindowPos3d_INDEX 477
#define glWindowPos3d_RETURN void
#define glWindowPos3d_ARG_NAMES x, y, z
#define glWindowPos3d_ARG_EXPAND GLdouble x, GLdouble y, GLdouble z
#define glWindowPos3d_PACKED PACKED_void_GLdouble_GLdouble_GLdouble
#define glWindowPos3d_INDEXED INDEXED_void_GLdouble_GLdouble_GLdouble
#define glWindowPos3d_FORMAT FORMAT_void_GLdouble_GLdouble_GLdouble
#define glWindowPos3dv_INDEX 478
#define glWindowPos3dv_RETURN void
#define glWindowPos3dv_ARG_NAMES v
#define glWindowPos3dv_ARG_EXPAND const GLdouble * v
#define glWindowPos3dv_PACKED PACKED_void_const_GLdouble___GENPT__
#define glWindowPos3dv_INDEXED INDEXED_void_const_GLdouble___GENPT__
#define glWindowPos3dv_FORMAT FORMAT_void_const_GLdouble___GENPT__
#define glWindowPos3f_INDEX 479
#define glWindowPos3f_RETURN void
#define glWindowPos3f_ARG_NAMES x, y, z
#define glWindowPos3f_ARG_EXPAND GLfloat x, GLfloat y, GLfloat z
#define glWindowPos3f_PACKED PACKED_void_GLfloat_GLfloat_GLfloat
#define glWindowPos3f_INDEXED INDEXED_void_GLfloat_GLfloat_GLfloat
#define glWindowPos3f_FORMAT FORMAT_void_GLfloat_GLfloat_GLfloat
#define glWindowPos3fv_INDEX 480
#define glWindowPos3fv_RETURN void
#define glWindowPos3fv_ARG_NAMES v
#define glWindowPos3fv_ARG_EXPAND const GLfloat * v
#define glWindowPos3fv_PACKED PACKED_void_const_GLfloat___GENPT__
#define glWindowPos3fv_INDEXED INDEXED_void_const_GLfloat___GENPT__
#define glWindowPos3fv_FORMAT FORMAT_void_const_GLfloat___GENPT__
#define glWindowPos3i_INDEX 481
#define glWindowPos3i_RETURN void
#define glWindowPos3i_ARG_NAMES x, y, z
#define glWindowPos3i_ARG_EXPAND GLint x, GLint y, GLint z
#define glWindowPos3i_PACKED PACKED_void_GLint_GLint_GLint
#define glWindowPos3i_INDEXED INDEXED_void_GLint_GLint_GLint
#define glWindowPos3i_FORMAT FORMAT_void_GLint_GLint_GLint
#define glWindowPos3iv_INDEX 482
#define glWindowPos3iv_RETURN void
#define glWindowPos3iv_ARG_NAMES v
#define glWindowPos3iv_ARG_EXPAND const GLint * v
#define glWindowPos3iv_PACKED PACKED_void_const_GLint___GENPT__
#define glWindowPos3iv_INDEXED INDEXED_void_const_GLint___GENPT__
#define glWindowPos3iv_FORMAT FORMAT_void_const_GLint___GENPT__
#define glWindowPos3s_INDEX 483
#define glWindowPos3s_RETURN void
#define glWindowPos3s_ARG_NAMES x, y, z
#define glWindowPos3s_ARG_EXPAND GLshort x, GLshort y, GLshort z
#define glWindowPos3s_PACKED PACKED_void_GLshort_GLshort_GLshort
#define glWindowPos3s_INDEXED INDEXED_void_GLshort_GLshort_GLshort
#define glWindowPos3s_FORMAT FORMAT_void_GLshort_GLshort_GLshort
#define glWindowPos3sv_INDEX 484
#define glWindowPos3sv_RETURN void
#define glWindowPos3sv_ARG_NAMES v
#define glWindowPos3sv_ARG_EXPAND const GLshort * v
#define glWindowPos3sv_PACKED PACKED_void_const_GLshort___GENPT__
#define glWindowPos3sv_INDEXED INDEXED_void_const_GLshort___GENPT__
#define glWindowPos3sv_FORMAT FORMAT_void_const_GLshort___GENPT__
#define glXBindHyperpipeSGIX_INDEX 485
#define glXBindHyperpipeSGIX_RETURN int
#define glXBindHyperpipeSGIX_ARG_NAMES dpy, hpId
#define glXBindHyperpipeSGIX_ARG_EXPAND Display * dpy, int hpId
#define glXBindHyperpipeSGIX_PACKED PACKED_int_Display___GENPT___int
#define glXBindHyperpipeSGIX_INDEXED INDEXED_int_Display___GENPT___int
#define glXBindHyperpipeSGIX_FORMAT FORMAT_int_Display___GENPT___int
#define glXBindSwapBarrierSGIX_INDEX 486
#define glXBindSwapBarrierSGIX_RETURN void
#define glXBindSwapBarrierSGIX_ARG_NAMES window, barrier
#define glXBindSwapBarrierSGIX_ARG_EXPAND uint32_t window, uint32_t barrier
#define glXBindSwapBarrierSGIX_PACKED PACKED_void_uint32_t_uint32_t
#define glXBindSwapBarrierSGIX_INDEXED INDEXED_void_uint32_t_uint32_t
#define glXBindSwapBarrierSGIX_FORMAT FORMAT_void_uint32_t_uint32_t
#define glXChangeDrawableAttributes_INDEX 487
#define glXChangeDrawableAttributes_RETURN void
#define glXChangeDrawableAttributes_ARG_NAMES drawable
#define glXChangeDrawableAttributes_ARG_EXPAND uint32_t drawable
#define glXChangeDrawableAttributes_PACKED PACKED_void_uint32_t
#define glXChangeDrawableAttributes_INDEXED INDEXED_void_uint32_t
#define glXChangeDrawableAttributes_FORMAT FORMAT_void_uint32_t
#define glXChangeDrawableAttributesSGIX_INDEX 488
#define glXChangeDrawableAttributesSGIX_RETURN void
#define glXChangeDrawableAttributesSGIX_ARG_NAMES drawable
#define glXChangeDrawableAttributesSGIX_ARG_EXPAND uint32_t drawable
#define glXChangeDrawableAttributesSGIX_PACKED PACKED_void_uint32_t
#define glXChangeDrawableAttributesSGIX_INDEXED INDEXED_void_uint32_t
#define glXChangeDrawableAttributesSGIX_FORMAT FORMAT_void_uint32_t
#define glXChooseFBConfig_INDEX 489
#define glXChooseFBConfig_RETURN GLXFBConfig *
#define glXChooseFBConfig_ARG_NAMES dpy, screen, attrib_list, nelements
#define glXChooseFBConfig_ARG_EXPAND Display * dpy, int screen, const int * attrib_list, int * nelements
#define glXChooseFBConfig_PACKED PACKED_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__
#define glXChooseFBConfig_INDEXED INDEXED_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__
#define glXChooseFBConfig_FORMAT FORMAT_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__
#define glXChooseVisual_INDEX 490
#define glXChooseVisual_RETURN XVisualInfo *
#define glXChooseVisual_ARG_NAMES dpy, screen, attribList
#define glXChooseVisual_ARG_EXPAND Display * dpy, int screen, int * attribList
#define glXChooseVisual_PACKED PACKED_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__
#define glXChooseVisual_INDEXED INDEXED_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__
#define glXChooseVisual_FORMAT FORMAT_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__
#define glXClientInfo_INDEX 491
#define glXClientInfo_RETURN void
#define glXClientInfo_ARG_NAMES 
#define glXClientInfo_ARG_EXPAND 
#define glXClientInfo_PACKED PACKED_void
#define glXClientInfo_INDEXED INDEXED_void
#define glXClientInfo_FORMAT FORMAT_void
#define glXCopyContext_INDEX 492
#define glXCopyContext_RETURN void
#define glXCopyContext_ARG_NAMES dpy, src, dst, mask
#define glXCopyContext_ARG_EXPAND Display * dpy, GLXContext src, GLXContext dst, unsigned long mask
#define glXCopyContext_PACKED PACKED_void_Display___GENPT___GLXContext_GLXContext_unsigned_long
#define glXCopyContext_INDEXED INDEXED_void_Display___GENPT___GLXContext_GLXContext_unsigned_long
#define glXCopyContext_FORMAT FORMAT_void_Display___GENPT___GLXContext_GLXContext_unsigned_long
#define glXCreateContext_INDEX 493
#define glXCreateContext_RETURN GLXContext
#define glXCreateContext_ARG_NAMES dpy, vis, shareList, direct
#define glXCreateContext_ARG_EXPAND Display * dpy, XVisualInfo * vis, GLXContext shareList, Bool direct
#define glXCreateContext_PACKED PACKED_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool
#define glXCreateContext_INDEXED INDEXED_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool
#define glXCreateContext_FORMAT FORMAT_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool
#define glXCreateContextWithConfigSGIX_INDEX 494
#define glXCreateContextWithConfigSGIX_RETURN void
#define glXCreateContextWithConfigSGIX_ARG_NAMES gc_id, screen, config, share_list
#define glXCreateContextWithConfigSGIX_ARG_EXPAND uint32_t gc_id, uint32_t screen, uint32_t config, uint32_t share_list
#define glXCreateContextWithConfigSGIX_PACKED PACKED_void_uint32_t_uint32_t_uint32_t_uint32_t
#define glXCreateContextWithConfigSGIX_INDEXED INDEXED_void_uint32_t_uint32_t_uint32_t_uint32_t
#define glXCreateContextWithConfigSGIX_FORMAT FORMAT_void_uint32_t_uint32_t_uint32_t_uint32_t
#define glXCreateGLXPbufferSGIX_INDEX 495
#define glXCreateGLXPbufferSGIX_RETURN void
#define glXCreateGLXPbufferSGIX_ARG_NAMES config, pbuffer
#define glXCreateGLXPbufferSGIX_ARG_EXPAND uint32_t config, uint32_t pbuffer
#define glXCreateGLXPbufferSGIX_PACKED PACKED_void_uint32_t_uint32_t
#define glXCreateGLXPbufferSGIX_INDEXED INDEXED_void_uint32_t_uint32_t
#define glXCreateGLXPbufferSGIX_FORMAT FORMAT_void_uint32_t_uint32_t
#define glXCreateGLXPixmap_INDEX 496
#define glXCreateGLXPixmap_RETURN GLXPixmap
#define glXCreateGLXPixmap_ARG_NAMES dpy, visual, pixmap
#define glXCreateGLXPixmap_ARG_EXPAND Display * dpy, XVisualInfo * visual, Pixmap pixmap
#define glXCreateGLXPixmap_PACKED PACKED_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap
#define glXCreateGLXPixmap_INDEXED INDEXED_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap
#define glXCreateGLXPixmap_FORMAT FORMAT_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap
#define glXCreateGLXPixmapWithConfigSGIX_INDEX 497
#define glXCreateGLXPixmapWithConfigSGIX_RETURN void
#define glXCreateGLXPixmapWithConfigSGIX_ARG_NAMES config, pixmap, glxpixmap
#define glXCreateGLXPixmapWithConfigSGIX_ARG_EXPAND uint32_t config, uint32_t pixmap, uint32_t glxpixmap
#define glXCreateGLXPixmapWithConfigSGIX_PACKED PACKED_void_uint32_t_uint32_t_uint32_t
#define glXCreateGLXPixmapWithConfigSGIX_INDEXED INDEXED_void_uint32_t_uint32_t_uint32_t
#define glXCreateGLXPixmapWithConfigSGIX_FORMAT FORMAT_void_uint32_t_uint32_t_uint32_t
#define glXCreateGLXVideoSourceSGIX_INDEX 498
#define glXCreateGLXVideoSourceSGIX_RETURN void
#define glXCreateGLXVideoSourceSGIX_ARG_NAMES dpy, screen, server, path, class, node
#define glXCreateGLXVideoSourceSGIX_ARG_EXPAND Display * dpy, uint32_t screen, uint32_t server, uint32_t path, uint32_t class, uint32_t node
#define glXCreateGLXVideoSourceSGIX_PACKED PACKED_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t
#define glXCreateGLXVideoSourceSGIX_INDEXED INDEXED_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t
#define glXCreateGLXVideoSourceSGIX_FORMAT FORMAT_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t
#define glXCreateNewContext_INDEX 499
#define glXCreateNewContext_RETURN GLXContext
#define glXCreateNewContext_ARG_NAMES dpy, config, render_type, share_list, direct
#define glXCreateNewContext_ARG_EXPAND Display * dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct
#define glXCreateNewContext_PACKED PACKED_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool
#define glXCreateNewContext_INDEXED INDEXED_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool
#define glXCreateNewContext_FORMAT FORMAT_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool
#define glXCreatePbuffer_INDEX 500
#define glXCreatePbuffer_RETURN GLXPbuffer
#define glXCreatePbuffer_ARG_NAMES dpy, config, attrib_list
#define glXCreatePbuffer_ARG_EXPAND Display * dpy, GLXFBConfig config, const int * attrib_list
#define glXCreatePbuffer_PACKED PACKED_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__
#define glXCreatePbuffer_INDEXED INDEXED_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__
#define glXCreatePbuffer_FORMAT FORMAT_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__
#define glXCreatePixmap_INDEX 501
#define glXCreatePixmap_RETURN GLXPixmap
#define glXCreatePixmap_ARG_NAMES dpy, config, pixmap, attrib_list
#define glXCreatePixmap_ARG_EXPAND Display * dpy, GLXFBConfig config, Pixmap pixmap, const int * attrib_list
#define glXCreatePixmap_PACKED PACKED_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__
#define glXCreatePixmap_INDEXED INDEXED_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__
#define glXCreatePixmap_FORMAT FORMAT_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__
#define glXCreateWindow_INDEX 502
#define glXCreateWindow_RETURN GLXWindow
#define glXCreateWindow_ARG_NAMES dpy, config, win, attrib_list
#define glXCreateWindow_ARG_EXPAND Display * dpy, GLXFBConfig config, Window win, const int * attrib_list
#define glXCreateWindow_PACKED PACKED_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__
#define glXCreateWindow_INDEXED INDEXED_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__
#define glXCreateWindow_FORMAT FORMAT_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__
#define glXDestroyContext_INDEX 503
#define glXDestroyContext_RETURN void
#define glXDestroyContext_ARG_NAMES dpy, ctx
#define glXDestroyContext_ARG_EXPAND Display * dpy, GLXContext ctx
#define glXDestroyContext_PACKED PACKED_void_Display___GENPT___GLXContext
#define glXDestroyContext_INDEXED INDEXED_void_Display___GENPT___GLXContext
#define glXDestroyContext_FORMAT FORMAT_void_Display___GENPT___GLXContext
#define glXDestroyGLXPbufferSGIX_INDEX 504
#define glXDestroyGLXPbufferSGIX_RETURN void
#define glXDestroyGLXPbufferSGIX_ARG_NAMES pbuffer
#define glXDestroyGLXPbufferSGIX_ARG_EXPAND uint32_t pbuffer
#define glXDestroyGLXPbufferSGIX_PACKED PACKED_void_uint32_t
#define glXDestroyGLXPbufferSGIX_INDEXED INDEXED_void_uint32_t
#define glXDestroyGLXPbufferSGIX_FORMAT FORMAT_void_uint32_t
#define glXDestroyGLXPixmap_INDEX 505
#define glXDestroyGLXPixmap_RETURN void
#define glXDestroyGLXPixmap_ARG_NAMES dpy, pixmap
#define glXDestroyGLXPixmap_ARG_EXPAND Display * dpy, GLXPixmap pixmap
#define glXDestroyGLXPixmap_PACKED PACKED_void_Display___GENPT___GLXPixmap
#define glXDestroyGLXPixmap_INDEXED INDEXED_void_Display___GENPT___GLXPixmap
#define glXDestroyGLXPixmap_FORMAT FORMAT_void_Display___GENPT___GLXPixmap
#define glXDestroyGLXVideoSourceSGIX_INDEX 506
#define glXDestroyGLXVideoSourceSGIX_RETURN void
#define glXDestroyGLXVideoSourceSGIX_ARG_NAMES dpy, glxvideosource
#define glXDestroyGLXVideoSourceSGIX_ARG_EXPAND Display * dpy, uint32_t glxvideosource
#define glXDestroyGLXVideoSourceSGIX_PACKED PACKED_void_Display___GENPT___uint32_t
#define glXDestroyGLXVideoSourceSGIX_INDEXED INDEXED_void_Display___GENPT___uint32_t
#define glXDestroyGLXVideoSourceSGIX_FORMAT FORMAT_void_Display___GENPT___uint32_t
#define glXDestroyHyperpipeConfigSGIX_INDEX 507
#define glXDestroyHyperpipeConfigSGIX_RETURN int
#define glXDestroyHyperpipeConfigSGIX_ARG_NAMES dpy, hpId
#define glXDestroyHyperpipeConfigSGIX_ARG_EXPAND Display * dpy, int hpId
#define glXDestroyHyperpipeConfigSGIX_PACKED PACKED_int_Display___GENPT___int
#define glXDestroyHyperpipeConfigSGIX_INDEXED INDEXED_int_Display___GENPT___int
#define glXDestroyHyperpipeConfigSGIX_FORMAT FORMAT_int_Display___GENPT___int
#define glXDestroyPbuffer_INDEX 508
#define glXDestroyPbuffer_RETURN void
#define glXDestroyPbuffer_ARG_NAMES dpy, pbuf
#define glXDestroyPbuffer_ARG_EXPAND Display * dpy, GLXPbuffer pbuf
#define glXDestroyPbuffer_PACKED PACKED_void_Display___GENPT___GLXPbuffer
#define glXDestroyPbuffer_INDEXED INDEXED_void_Display___GENPT___GLXPbuffer
#define glXDestroyPbuffer_FORMAT FORMAT_void_Display___GENPT___GLXPbuffer
#define glXDestroyPixmap_INDEX 509
#define glXDestroyPixmap_RETURN void
#define glXDestroyPixmap_ARG_NAMES dpy, pixmap
#define glXDestroyPixmap_ARG_EXPAND Display * dpy, GLXPixmap pixmap
#define glXDestroyPixmap_PACKED PACKED_void_Display___GENPT___GLXPixmap
#define glXDestroyPixmap_INDEXED INDEXED_void_Display___GENPT___GLXPixmap
#define glXDestroyPixmap_FORMAT FORMAT_void_Display___GENPT___GLXPixmap
#define glXDestroyWindow_INDEX 510
#define glXDestroyWindow_RETURN void
#define glXDestroyWindow_ARG_NAMES dpy, win
#define glXDestroyWindow_ARG_EXPAND Display * dpy, GLXWindow win
#define glXDestroyWindow_PACKED PACKED_void_Display___GENPT___GLXWindow
#define glXDestroyWindow_INDEXED INDEXED_void_Display___GENPT___GLXWindow
#define glXDestroyWindow_FORMAT FORMAT_void_Display___GENPT___GLXWindow
#define glXGetCurrentDisplay_INDEX 511
#define glXGetCurrentDisplay_RETURN Display *
#define glXGetCurrentDisplay_ARG_NAMES 
#define glXGetCurrentDisplay_ARG_EXPAND 
#define glXGetCurrentDisplay_PACKED PACKED_Display___GENPT__
#define glXGetCurrentDisplay_INDEXED INDEXED_Display___GENPT__
#define glXGetCurrentDisplay_FORMAT FORMAT_Display___GENPT__
#define glXGetCurrentReadDrawable_INDEX 512
#define glXGetCurrentReadDrawable_RETURN GLXDrawable
#define glXGetCurrentReadDrawable_ARG_NAMES 
#define glXGetCurrentReadDrawable_ARG_EXPAND 
#define glXGetCurrentReadDrawable_PACKED PACKED_GLXDrawable
#define glXGetCurrentReadDrawable_INDEXED INDEXED_GLXDrawable
#define glXGetCurrentReadDrawable_FORMAT FORMAT_GLXDrawable
#define glXGetDrawableAttributes_INDEX 513
#define glXGetDrawableAttributes_RETURN void
#define glXGetDrawableAttributes_ARG_NAMES drawable
#define glXGetDrawableAttributes_ARG_EXPAND uint32_t drawable
#define glXGetDrawableAttributes_PACKED PACKED_void_uint32_t
#define glXGetDrawableAttributes_INDEXED INDEXED_void_uint32_t
#define glXGetDrawableAttributes_FORMAT FORMAT_void_uint32_t
#define glXGetDrawableAttributesSGIX_INDEX 514
#define glXGetDrawableAttributesSGIX_RETURN void
#define glXGetDrawableAttributesSGIX_ARG_NAMES drawable
#define glXGetDrawableAttributesSGIX_ARG_EXPAND uint32_t drawable
#define glXGetDrawableAttributesSGIX_PACKED PACKED_void_uint32_t
#define glXGetDrawableAttributesSGIX_INDEXED INDEXED_void_uint32_t
#define glXGetDrawableAttributesSGIX_FORMAT FORMAT_void_uint32_t
#define glXGetFBConfigAttrib_INDEX 515
#define glXGetFBConfigAttrib_RETURN int
#define glXGetFBConfigAttrib_ARG_NAMES dpy, config, attribute, value
#define glXGetFBConfigAttrib_ARG_EXPAND Display * dpy, GLXFBConfig config, int attribute, int * value
#define glXGetFBConfigAttrib_PACKED PACKED_int_Display___GENPT___GLXFBConfig_int_int___GENPT__
#define glXGetFBConfigAttrib_INDEXED INDEXED_int_Display___GENPT___GLXFBConfig_int_int___GENPT__
#define glXGetFBConfigAttrib_FORMAT FORMAT_int_Display___GENPT___GLXFBConfig_int_int___GENPT__
#define glXGetFBConfigs_INDEX 516
#define glXGetFBConfigs_RETURN GLXFBConfig *
#define glXGetFBConfigs_ARG_NAMES dpy, screen, nelements
#define glXGetFBConfigs_ARG_EXPAND Display * dpy, int screen, int * nelements
#define glXGetFBConfigs_PACKED PACKED_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__
#define glXGetFBConfigs_INDEXED INDEXED_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__
#define glXGetFBConfigs_FORMAT FORMAT_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__
#define glXGetFBConfigsSGIX_INDEX 517
#define glXGetFBConfigsSGIX_RETURN void
#define glXGetFBConfigsSGIX_ARG_NAMES 
#define glXGetFBConfigsSGIX_ARG_EXPAND 
#define glXGetFBConfigsSGIX_PACKED PACKED_void
#define glXGetFBConfigsSGIX_INDEXED INDEXED_void
#define glXGetFBConfigsSGIX_FORMAT FORMAT_void
#define glXGetProcAddress_INDEX 518
#define glXGetProcAddress_RETURN __GLXextFuncPtr
#define glXGetProcAddress_ARG_NAMES procName
#define glXGetProcAddress_ARG_EXPAND const GLubyte * procName
#define glXGetProcAddress_PACKED PACKED___GLXextFuncPtr_const_GLubyte___GENPT__
#define glXGetProcAddress_INDEXED INDEXED___GLXextFuncPtr_const_GLubyte___GENPT__
#define glXGetProcAddress_FORMAT FORMAT___GLXextFuncPtr_const_GLubyte___GENPT__
#define glXGetSelectedEvent_INDEX 519
#define glXGetSelectedEvent_RETURN void
#define glXGetSelectedEvent_ARG_NAMES dpy, draw, event_mask
#define glXGetSelectedEvent_ARG_EXPAND Display * dpy, GLXDrawable draw, unsigned long * event_mask
#define glXGetSelectedEvent_PACKED PACKED_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__
#define glXGetSelectedEvent_INDEXED INDEXED_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__
#define glXGetSelectedEvent_FORMAT FORMAT_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__
#define glXGetVisualConfigs_INDEX 520
#define glXGetVisualConfigs_RETURN void
#define glXGetVisualConfigs_ARG_NAMES 
#define glXGetVisualConfigs_ARG_EXPAND 
#define glXGetVisualConfigs_PACKED PACKED_void
#define glXGetVisualConfigs_INDEXED INDEXED_void
#define glXGetVisualConfigs_FORMAT FORMAT_void
#define glXGetVisualFromFBConfig_INDEX 521
#define glXGetVisualFromFBConfig_RETURN XVisualInfo *
#define glXGetVisualFromFBConfig_ARG_NAMES dpy, config
#define glXGetVisualFromFBConfig_ARG_EXPAND Display * dpy, GLXFBConfig config
#define glXGetVisualFromFBConfig_PACKED PACKED_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig
#define glXGetVisualFromFBConfig_INDEXED INDEXED_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig
#define glXGetVisualFromFBConfig_FORMAT FORMAT_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig
#define glXHyperpipeAttribSGIX_INDEX 522
#define glXHyperpipeAttribSGIX_RETURN int
#define glXHyperpipeAttribSGIX_ARG_NAMES dpy, timeSlice, attrib, size, attribList
#define glXHyperpipeAttribSGIX_ARG_EXPAND Display * dpy, int timeSlice, int attrib, int size, const void * attribList
#define glXHyperpipeAttribSGIX_PACKED PACKED_int_Display___GENPT___int_int_int_const_void___GENPT__
#define glXHyperpipeAttribSGIX_INDEXED INDEXED_int_Display___GENPT___int_int_int_const_void___GENPT__
#define glXHyperpipeAttribSGIX_FORMAT FORMAT_int_Display___GENPT___int_int_int_const_void___GENPT__
#define glXHyperpipeConfigSGIX_INDEX 523
#define glXHyperpipeConfigSGIX_RETURN int
#define glXHyperpipeConfigSGIX_ARG_NAMES dpy, networkId, npipes, cfg, hpId
#define glXHyperpipeConfigSGIX_ARG_EXPAND Display * dpy, int networkId, int npipes, GLXHyperpipeConfigSGIX cfg, int * hpId
#define glXHyperpipeConfigSGIX_PACKED PACKED_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__
#define glXHyperpipeConfigSGIX_INDEXED INDEXED_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__
#define glXHyperpipeConfigSGIX_FORMAT FORMAT_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__
#define glXIsDirect_INDEX 524
#define glXIsDirect_RETURN Bool
#define glXIsDirect_ARG_NAMES dpy, ctx
#define glXIsDirect_ARG_EXPAND Display * dpy, GLXContext ctx
#define glXIsDirect_PACKED PACKED_Bool_Display___GENPT___GLXContext
#define glXIsDirect_INDEXED INDEXED_Bool_Display___GENPT___GLXContext
#define glXIsDirect_FORMAT FORMAT_Bool_Display___GENPT___GLXContext
#define glXJoinSwapGroupSGIX_INDEX 525
#define glXJoinSwapGroupSGIX_RETURN void
#define glXJoinSwapGroupSGIX_ARG_NAMES window, group
#define glXJoinSwapGroupSGIX_ARG_EXPAND uint32_t window, uint32_t group
#define glXJoinSwapGroupSGIX_PACKED PACKED_void_uint32_t_uint32_t
#define glXJoinSwapGroupSGIX_INDEXED INDEXED_void_uint32_t_uint32_t
#define glXJoinSwapGroupSGIX_FORMAT FORMAT_void_uint32_t_uint32_t
#define glXMakeContextCurrent_INDEX 526
#define glXMakeContextCurrent_RETURN Bool
#define glXMakeContextCurrent_ARG_NAMES dpy, draw, read, ctx
#define glXMakeContextCurrent_ARG_EXPAND Display * dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx
#define glXMakeContextCurrent_PACKED PACKED_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext
#define glXMakeContextCurrent_INDEXED INDEXED_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext
#define glXMakeContextCurrent_FORMAT FORMAT_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext
#define glXMakeCurrent_INDEX 527
#define glXMakeCurrent_RETURN Bool
#define glXMakeCurrent_ARG_NAMES dpy, drawable, ctx
#define glXMakeCurrent_ARG_EXPAND Display * dpy, GLXDrawable drawable, GLXContext ctx
#define glXMakeCurrent_PACKED PACKED_Bool_Display___GENPT___GLXDrawable_GLXContext
#define glXMakeCurrent_INDEXED INDEXED_Bool_Display___GENPT___GLXDrawable_GLXContext
#define glXMakeCurrent_FORMAT FORMAT_Bool_Display___GENPT___GLXDrawable_GLXContext
#define glXMakeCurrentReadSGI_INDEX 528
#define glXMakeCurrentReadSGI_RETURN void
#define glXMakeCurrentReadSGI_ARG_NAMES drawable, readdrawable, context
#define glXMakeCurrentReadSGI_ARG_EXPAND uint32_t drawable, uint32_t readdrawable, uint32_t context
#define glXMakeCurrentReadSGI_PACKED PACKED_void_uint32_t_uint32_t_uint32_t
#define glXMakeCurrentReadSGI_INDEXED INDEXED_void_uint32_t_uint32_t_uint32_t
#define glXMakeCurrentReadSGI_FORMAT FORMAT_void_uint32_t_uint32_t_uint32_t
#define glXQueryContext_INDEX 529
#define glXQueryContext_RETURN int
#define glXQueryContext_ARG_NAMES dpy, ctx, attribute, value
#define glXQueryContext_ARG_EXPAND Display * dpy, GLXContext ctx, int attribute, int * value
#define glXQueryContext_PACKED PACKED_int_Display___GENPT___GLXContext_int_int___GENPT__
#define glXQueryContext_INDEXED INDEXED_int_Display___GENPT___GLXContext_int_int___GENPT__
#define glXQueryContext_FORMAT FORMAT_int_Display___GENPT___GLXContext_int_int___GENPT__
#define glXQueryContextInfoEXT_INDEX 530
#define glXQueryContextInfoEXT_RETURN void
#define glXQueryContextInfoEXT_ARG_NAMES 
#define glXQueryContextInfoEXT_ARG_EXPAND 
#define glXQueryContextInfoEXT_PACKED PACKED_void
#define glXQueryContextInfoEXT_INDEXED INDEXED_void
#define glXQueryContextInfoEXT_FORMAT FORMAT_void
#define glXQueryDrawable_INDEX 531
#define glXQueryDrawable_RETURN void
#define glXQueryDrawable_ARG_NAMES dpy, draw, attribute, value
#define glXQueryDrawable_ARG_EXPAND Display * dpy, GLXDrawable draw, int attribute, unsigned int * value
#define glXQueryDrawable_PACKED PACKED_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__
#define glXQueryDrawable_INDEXED INDEXED_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__
#define glXQueryDrawable_FORMAT FORMAT_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__
#define glXQueryExtensionsString_INDEX 532
#define glXQueryExtensionsString_RETURN const char *
#define glXQueryExtensionsString_ARG_NAMES dpy, screen
#define glXQueryExtensionsString_ARG_EXPAND Display * dpy, int screen
#define glXQueryExtensionsString_PACKED PACKED_const_char___GENPT___Display___GENPT___int
#define glXQueryExtensionsString_INDEXED INDEXED_const_char___GENPT___Display___GENPT___int
#define glXQueryExtensionsString_FORMAT FORMAT_const_char___GENPT___Display___GENPT___int
#define glXQueryHyperpipeAttribSGIX_INDEX 533
#define glXQueryHyperpipeAttribSGIX_RETURN int
#define glXQueryHyperpipeAttribSGIX_ARG_NAMES dpy, timeSlice, attrib, size, returnAttribList
#define glXQueryHyperpipeAttribSGIX_ARG_EXPAND Display * dpy, int timeSlice, int attrib, int size, const void * returnAttribList
#define glXQueryHyperpipeAttribSGIX_PACKED PACKED_int_Display___GENPT___int_int_int_const_void___GENPT__
#define glXQueryHyperpipeAttribSGIX_INDEXED INDEXED_int_Display___GENPT___int_int_int_const_void___GENPT__
#define glXQueryHyperpipeAttribSGIX_FORMAT FORMAT_int_Display___GENPT___int_int_int_const_void___GENPT__
#define glXQueryHyperpipeBestAttribSGIX_INDEX 534
#define glXQueryHyperpipeBestAttribSGIX_RETURN int
#define glXQueryHyperpipeBestAttribSGIX_ARG_NAMES dpy, timeSlice, attrib, size, attribList, returnAttribList
#define glXQueryHyperpipeBestAttribSGIX_ARG_EXPAND Display * dpy, int timeSlice, int attrib, int size, const void * attribList, void * returnAttribList
#define glXQueryHyperpipeBestAttribSGIX_PACKED PACKED_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__
#define glXQueryHyperpipeBestAttribSGIX_INDEXED INDEXED_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__
#define glXQueryHyperpipeBestAttribSGIX_FORMAT FORMAT_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__
#define glXQueryHyperpipeConfigSGIX_INDEX 535
#define glXQueryHyperpipeConfigSGIX_RETURN GLXHyperpipeConfigSGIX *
#define glXQueryHyperpipeConfigSGIX_ARG_NAMES dpy, hpId, npipes
#define glXQueryHyperpipeConfigSGIX_ARG_EXPAND Display * dpy, int hpId, int * npipes
#define glXQueryHyperpipeConfigSGIX_PACKED PACKED_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__
#define glXQueryHyperpipeConfigSGIX_INDEXED INDEXED_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__
#define glXQueryHyperpipeConfigSGIX_FORMAT FORMAT_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__
#define glXQueryHyperpipeNetworkSGIX_INDEX 536
#define glXQueryHyperpipeNetworkSGIX_RETURN GLXHyperpipeNetworkSGIX *
#define glXQueryHyperpipeNetworkSGIX_ARG_NAMES dpy, npipes
#define glXQueryHyperpipeNetworkSGIX_ARG_EXPAND Display * dpy, int * npipes
#define glXQueryHyperpipeNetworkSGIX_PACKED PACKED_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__
#define glXQueryHyperpipeNetworkSGIX_INDEXED INDEXED_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__
#define glXQueryHyperpipeNetworkSGIX_FORMAT FORMAT_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__
#define glXQueryMaxSwapBarriersSGIX_INDEX 537
#define glXQueryMaxSwapBarriersSGIX_RETURN void
#define glXQueryMaxSwapBarriersSGIX_ARG_NAMES 
#define glXQueryMaxSwapBarriersSGIX_ARG_EXPAND 
#define glXQueryMaxSwapBarriersSGIX_PACKED PACKED_void
#define glXQueryMaxSwapBarriersSGIX_INDEXED INDEXED_void
#define glXQueryMaxSwapBarriersSGIX_FORMAT FORMAT_void
#define glXQueryServerString_INDEX 538
#define glXQueryServerString_RETURN const char *
#define glXQueryServerString_ARG_NAMES dpy, screen, name
#define glXQueryServerString_ARG_EXPAND Display * dpy, int screen, int name
#define glXQueryServerString_PACKED PACKED_const_char___GENPT___Display___GENPT___int_int
#define glXQueryServerString_INDEXED INDEXED_const_char___GENPT___Display___GENPT___int_int
#define glXQueryServerString_FORMAT FORMAT_const_char___GENPT___Display___GENPT___int_int
#define glXQueryVersion_INDEX 539
#define glXQueryVersion_RETURN Bool
#define glXQueryVersion_ARG_NAMES dpy, maj, min
#define glXQueryVersion_ARG_EXPAND Display * dpy, int * maj, int * min
#define glXQueryVersion_PACKED PACKED_Bool_Display___GENPT___int___GENPT___int___GENPT__
#define glXQueryVersion_INDEXED INDEXED_Bool_Display___GENPT___int___GENPT___int___GENPT__
#define glXQueryVersion_FORMAT FORMAT_Bool_Display___GENPT___int___GENPT___int___GENPT__
#define glXRender_INDEX 540
#define glXRender_RETURN void
#define glXRender_ARG_NAMES 
#define glXRender_ARG_EXPAND 
#define glXRender_PACKED PACKED_void
#define glXRender_INDEXED INDEXED_void
#define glXRender_FORMAT FORMAT_void
#define glXRenderLarge_INDEX 541
#define glXRenderLarge_RETURN void
#define glXRenderLarge_ARG_NAMES 
#define glXRenderLarge_ARG_EXPAND 
#define glXRenderLarge_PACKED PACKED_void
#define glXRenderLarge_INDEXED INDEXED_void
#define glXRenderLarge_FORMAT FORMAT_void
#define glXSelectEvent_INDEX 542
#define glXSelectEvent_RETURN void
#define glXSelectEvent_ARG_NAMES dpy, draw, event_mask
#define glXSelectEvent_ARG_EXPAND Display * dpy, GLXDrawable draw, unsigned long event_mask
#define glXSelectEvent_PACKED PACKED_void_Display___GENPT___GLXDrawable_unsigned_long
#define glXSelectEvent_INDEXED INDEXED_void_Display___GENPT___GLXDrawable_unsigned_long
#define glXSelectEvent_FORMAT FORMAT_void_Display___GENPT___GLXDrawable_unsigned_long
#define glXSwapBuffers_INDEX 543
#define glXSwapBuffers_RETURN void
#define glXSwapBuffers_ARG_NAMES dpy, drawable
#define glXSwapBuffers_ARG_EXPAND Display * dpy, GLXDrawable drawable
#define glXSwapBuffers_PACKED PACKED_void_Display___GENPT___GLXDrawable
#define glXSwapBuffers_INDEXED INDEXED_void_Display___GENPT___GLXDrawable
#define glXSwapBuffers_FORMAT FORMAT_void_Display___GENPT___GLXDrawable
#define glXSwapIntervalSGI_INDEX 544
#define glXSwapIntervalSGI_RETURN void
#define glXSwapIntervalSGI_ARG_NAMES 
#define glXSwapIntervalSGI_ARG_EXPAND 
#define glXSwapIntervalSGI_PACKED PACKED_void
#define glXSwapIntervalSGI_INDEXED INDEXED_void
#define glXSwapIntervalSGI_FORMAT FORMAT_void
#define glXUseXFont_INDEX 545
#define glXUseXFont_RETURN void
#define glXUseXFont_ARG_NAMES font, first, count, list
#define glXUseXFont_ARG_EXPAND Font font, int first, int count, int list
#define glXUseXFont_PACKED PACKED_void_Font_int_int_int
#define glXUseXFont_INDEXED INDEXED_void_Font_int_int_int
#define glXUseXFont_FORMAT FORMAT_void_Font_int_int_int
#define glXVendorPrivate_INDEX 546
#define glXVendorPrivate_RETURN void
#define glXVendorPrivate_ARG_NAMES 
#define glXVendorPrivate_ARG_EXPAND 
#define glXVendorPrivate_PACKED PACKED_void
#define glXVendorPrivate_INDEXED INDEXED_void
#define glXVendorPrivate_FORMAT FORMAT_void
#define glXVendorPrivateWithReply_INDEX 547
#define glXVendorPrivateWithReply_RETURN void
#define glXVendorPrivateWithReply_ARG_NAMES 
#define glXVendorPrivateWithReply_ARG_EXPAND 
#define glXVendorPrivateWithReply_PACKED PACKED_void
#define glXVendorPrivateWithReply_INDEXED INDEXED_void
#define glXVendorPrivateWithReply_FORMAT FORMAT_void
#define glXWaitGL_INDEX 548
#define glXWaitGL_RETURN void
#define glXWaitGL_ARG_NAMES 
#define glXWaitGL_ARG_EXPAND 
#define glXWaitGL_PACKED PACKED_void
#define glXWaitGL_INDEXED INDEXED_void
#define glXWaitGL_FORMAT FORMAT_void
#define glXWaitX_INDEX 549
#define glXWaitX_RETURN void
#define glXWaitX_ARG_NAMES 
#define glXWaitX_ARG_EXPAND 
#define glXWaitX_PACKED PACKED_void
#define glXWaitX_INDEXED INDEXED_void
#define glXWaitX_FORMAT FORMAT_void

void glAccum(glAccum_ARG_EXPAND);
typedef void (*glesptr_glAccum)(glAccum_ARG_EXPAND);
void glActiveTexture(glActiveTexture_ARG_EXPAND);
typedef void (*glesptr_glActiveTexture)(glActiveTexture_ARG_EXPAND);
void glAlphaFunc(glAlphaFunc_ARG_EXPAND);
typedef void (*glesptr_glAlphaFunc)(glAlphaFunc_ARG_EXPAND);
GLboolean glAreTexturesResident(glAreTexturesResident_ARG_EXPAND);
typedef GLboolean (*glesptr_glAreTexturesResident)(glAreTexturesResident_ARG_EXPAND);
void glArrayElement(glArrayElement_ARG_EXPAND);
typedef void (*glesptr_glArrayElement)(glArrayElement_ARG_EXPAND);
void glBegin(glBegin_ARG_EXPAND);
typedef void (*glesptr_glBegin)(glBegin_ARG_EXPAND);
void glBeginQuery(glBeginQuery_ARG_EXPAND);
typedef void (*glesptr_glBeginQuery)(glBeginQuery_ARG_EXPAND);
void glBindBuffer(glBindBuffer_ARG_EXPAND);
typedef void (*glesptr_glBindBuffer)(glBindBuffer_ARG_EXPAND);
void glBindTexture(glBindTexture_ARG_EXPAND);
typedef void (*glesptr_glBindTexture)(glBindTexture_ARG_EXPAND);
void glBitmap(glBitmap_ARG_EXPAND);
typedef void (*glesptr_glBitmap)(glBitmap_ARG_EXPAND);
void glBlendColor(glBlendColor_ARG_EXPAND);
typedef void (*glesptr_glBlendColor)(glBlendColor_ARG_EXPAND);
void glBlendEquation(glBlendEquation_ARG_EXPAND);
typedef void (*glesptr_glBlendEquation)(glBlendEquation_ARG_EXPAND);
void glBlendFunc(glBlendFunc_ARG_EXPAND);
typedef void (*glesptr_glBlendFunc)(glBlendFunc_ARG_EXPAND);
void glBlendFuncSeparate(glBlendFuncSeparate_ARG_EXPAND);
typedef void (*glesptr_glBlendFuncSeparate)(glBlendFuncSeparate_ARG_EXPAND);
void glBufferData(glBufferData_ARG_EXPAND);
typedef void (*glesptr_glBufferData)(glBufferData_ARG_EXPAND);
void glBufferSubData(glBufferSubData_ARG_EXPAND);
typedef void (*glesptr_glBufferSubData)(glBufferSubData_ARG_EXPAND);
void glCallList(glCallList_ARG_EXPAND);
typedef void (*glesptr_glCallList)(glCallList_ARG_EXPAND);
void glCallLists(glCallLists_ARG_EXPAND);
typedef void (*glesptr_glCallLists)(glCallLists_ARG_EXPAND);
void glClear(glClear_ARG_EXPAND);
typedef void (*glesptr_glClear)(glClear_ARG_EXPAND);
void glClearAccum(glClearAccum_ARG_EXPAND);
typedef void (*glesptr_glClearAccum)(glClearAccum_ARG_EXPAND);
void glClearColor(glClearColor_ARG_EXPAND);
typedef void (*glesptr_glClearColor)(glClearColor_ARG_EXPAND);
void glClearDepth(glClearDepth_ARG_EXPAND);
typedef void (*glesptr_glClearDepth)(glClearDepth_ARG_EXPAND);
void glClearIndex(glClearIndex_ARG_EXPAND);
typedef void (*glesptr_glClearIndex)(glClearIndex_ARG_EXPAND);
void glClearStencil(glClearStencil_ARG_EXPAND);
typedef void (*glesptr_glClearStencil)(glClearStencil_ARG_EXPAND);
void glClientActiveTexture(glClientActiveTexture_ARG_EXPAND);
typedef void (*glesptr_glClientActiveTexture)(glClientActiveTexture_ARG_EXPAND);
void glClipPlane(glClipPlane_ARG_EXPAND);
typedef void (*glesptr_glClipPlane)(glClipPlane_ARG_EXPAND);
void glColor3b(glColor3b_ARG_EXPAND);
typedef void (*glesptr_glColor3b)(glColor3b_ARG_EXPAND);
void glColor3bv(glColor3bv_ARG_EXPAND);
typedef void (*glesptr_glColor3bv)(glColor3bv_ARG_EXPAND);
void glColor3d(glColor3d_ARG_EXPAND);
typedef void (*glesptr_glColor3d)(glColor3d_ARG_EXPAND);
void glColor3dv(glColor3dv_ARG_EXPAND);
typedef void (*glesptr_glColor3dv)(glColor3dv_ARG_EXPAND);
void glColor3f(glColor3f_ARG_EXPAND);
typedef void (*glesptr_glColor3f)(glColor3f_ARG_EXPAND);
void glColor3fv(glColor3fv_ARG_EXPAND);
typedef void (*glesptr_glColor3fv)(glColor3fv_ARG_EXPAND);
void glColor3i(glColor3i_ARG_EXPAND);
typedef void (*glesptr_glColor3i)(glColor3i_ARG_EXPAND);
void glColor3iv(glColor3iv_ARG_EXPAND);
typedef void (*glesptr_glColor3iv)(glColor3iv_ARG_EXPAND);
void glColor3s(glColor3s_ARG_EXPAND);
typedef void (*glesptr_glColor3s)(glColor3s_ARG_EXPAND);
void glColor3sv(glColor3sv_ARG_EXPAND);
typedef void (*glesptr_glColor3sv)(glColor3sv_ARG_EXPAND);
void glColor3ub(glColor3ub_ARG_EXPAND);
typedef void (*glesptr_glColor3ub)(glColor3ub_ARG_EXPAND);
void glColor3ubv(glColor3ubv_ARG_EXPAND);
typedef void (*glesptr_glColor3ubv)(glColor3ubv_ARG_EXPAND);
void glColor3ui(glColor3ui_ARG_EXPAND);
typedef void (*glesptr_glColor3ui)(glColor3ui_ARG_EXPAND);
void glColor3uiv(glColor3uiv_ARG_EXPAND);
typedef void (*glesptr_glColor3uiv)(glColor3uiv_ARG_EXPAND);
void glColor3us(glColor3us_ARG_EXPAND);
typedef void (*glesptr_glColor3us)(glColor3us_ARG_EXPAND);
void glColor3usv(glColor3usv_ARG_EXPAND);
typedef void (*glesptr_glColor3usv)(glColor3usv_ARG_EXPAND);
void glColor4b(glColor4b_ARG_EXPAND);
typedef void (*glesptr_glColor4b)(glColor4b_ARG_EXPAND);
void glColor4bv(glColor4bv_ARG_EXPAND);
typedef void (*glesptr_glColor4bv)(glColor4bv_ARG_EXPAND);
void glColor4d(glColor4d_ARG_EXPAND);
typedef void (*glesptr_glColor4d)(glColor4d_ARG_EXPAND);
void glColor4dv(glColor4dv_ARG_EXPAND);
typedef void (*glesptr_glColor4dv)(glColor4dv_ARG_EXPAND);
void glColor4f(glColor4f_ARG_EXPAND);
typedef void (*glesptr_glColor4f)(glColor4f_ARG_EXPAND);
void glColor4fv(glColor4fv_ARG_EXPAND);
typedef void (*glesptr_glColor4fv)(glColor4fv_ARG_EXPAND);
void glColor4i(glColor4i_ARG_EXPAND);
typedef void (*glesptr_glColor4i)(glColor4i_ARG_EXPAND);
void glColor4iv(glColor4iv_ARG_EXPAND);
typedef void (*glesptr_glColor4iv)(glColor4iv_ARG_EXPAND);
void glColor4s(glColor4s_ARG_EXPAND);
typedef void (*glesptr_glColor4s)(glColor4s_ARG_EXPAND);
void glColor4sv(glColor4sv_ARG_EXPAND);
typedef void (*glesptr_glColor4sv)(glColor4sv_ARG_EXPAND);
void glColor4ub(glColor4ub_ARG_EXPAND);
typedef void (*glesptr_glColor4ub)(glColor4ub_ARG_EXPAND);
void glColor4ubv(glColor4ubv_ARG_EXPAND);
typedef void (*glesptr_glColor4ubv)(glColor4ubv_ARG_EXPAND);
void glColor4ui(glColor4ui_ARG_EXPAND);
typedef void (*glesptr_glColor4ui)(glColor4ui_ARG_EXPAND);
void glColor4uiv(glColor4uiv_ARG_EXPAND);
typedef void (*glesptr_glColor4uiv)(glColor4uiv_ARG_EXPAND);
void glColor4us(glColor4us_ARG_EXPAND);
typedef void (*glesptr_glColor4us)(glColor4us_ARG_EXPAND);
void glColor4usv(glColor4usv_ARG_EXPAND);
typedef void (*glesptr_glColor4usv)(glColor4usv_ARG_EXPAND);
void glColorMask(glColorMask_ARG_EXPAND);
typedef void (*glesptr_glColorMask)(glColorMask_ARG_EXPAND);
void glColorMaterial(glColorMaterial_ARG_EXPAND);
typedef void (*glesptr_glColorMaterial)(glColorMaterial_ARG_EXPAND);
void glColorPointer(glColorPointer_ARG_EXPAND);
typedef void (*glesptr_glColorPointer)(glColorPointer_ARG_EXPAND);
void glColorSubTable(glColorSubTable_ARG_EXPAND);
typedef void (*glesptr_glColorSubTable)(glColorSubTable_ARG_EXPAND);
void glColorTable(glColorTable_ARG_EXPAND);
typedef void (*glesptr_glColorTable)(glColorTable_ARG_EXPAND);
void glColorTableParameterfv(glColorTableParameterfv_ARG_EXPAND);
typedef void (*glesptr_glColorTableParameterfv)(glColorTableParameterfv_ARG_EXPAND);
void glColorTableParameteriv(glColorTableParameteriv_ARG_EXPAND);
typedef void (*glesptr_glColorTableParameteriv)(glColorTableParameteriv_ARG_EXPAND);
void glCompressedTexImage1D(glCompressedTexImage1D_ARG_EXPAND);
typedef void (*glesptr_glCompressedTexImage1D)(glCompressedTexImage1D_ARG_EXPAND);
void glCompressedTexImage2D(glCompressedTexImage2D_ARG_EXPAND);
typedef void (*glesptr_glCompressedTexImage2D)(glCompressedTexImage2D_ARG_EXPAND);
void glCompressedTexImage3D(glCompressedTexImage3D_ARG_EXPAND);
typedef void (*glesptr_glCompressedTexImage3D)(glCompressedTexImage3D_ARG_EXPAND);
void glCompressedTexSubImage1D(glCompressedTexSubImage1D_ARG_EXPAND);
typedef void (*glesptr_glCompressedTexSubImage1D)(glCompressedTexSubImage1D_ARG_EXPAND);
void glCompressedTexSubImage2D(glCompressedTexSubImage2D_ARG_EXPAND);
typedef void (*glesptr_glCompressedTexSubImage2D)(glCompressedTexSubImage2D_ARG_EXPAND);
void glCompressedTexSubImage3D(glCompressedTexSubImage3D_ARG_EXPAND);
typedef void (*glesptr_glCompressedTexSubImage3D)(glCompressedTexSubImage3D_ARG_EXPAND);
void glConvolutionFilter1D(glConvolutionFilter1D_ARG_EXPAND);
typedef void (*glesptr_glConvolutionFilter1D)(glConvolutionFilter1D_ARG_EXPAND);
void glConvolutionFilter2D(glConvolutionFilter2D_ARG_EXPAND);
typedef void (*glesptr_glConvolutionFilter2D)(glConvolutionFilter2D_ARG_EXPAND);
void glConvolutionParameterf(glConvolutionParameterf_ARG_EXPAND);
typedef void (*glesptr_glConvolutionParameterf)(glConvolutionParameterf_ARG_EXPAND);
void glConvolutionParameterfv(glConvolutionParameterfv_ARG_EXPAND);
typedef void (*glesptr_glConvolutionParameterfv)(glConvolutionParameterfv_ARG_EXPAND);
void glConvolutionParameteri(glConvolutionParameteri_ARG_EXPAND);
typedef void (*glesptr_glConvolutionParameteri)(glConvolutionParameteri_ARG_EXPAND);
void glConvolutionParameteriv(glConvolutionParameteriv_ARG_EXPAND);
typedef void (*glesptr_glConvolutionParameteriv)(glConvolutionParameteriv_ARG_EXPAND);
void glCopyColorSubTable(glCopyColorSubTable_ARG_EXPAND);
typedef void (*glesptr_glCopyColorSubTable)(glCopyColorSubTable_ARG_EXPAND);
void glCopyColorTable(glCopyColorTable_ARG_EXPAND);
typedef void (*glesptr_glCopyColorTable)(glCopyColorTable_ARG_EXPAND);
void glCopyConvolutionFilter1D(glCopyConvolutionFilter1D_ARG_EXPAND);
typedef void (*glesptr_glCopyConvolutionFilter1D)(glCopyConvolutionFilter1D_ARG_EXPAND);
void glCopyConvolutionFilter2D(glCopyConvolutionFilter2D_ARG_EXPAND);
typedef void (*glesptr_glCopyConvolutionFilter2D)(glCopyConvolutionFilter2D_ARG_EXPAND);
void glCopyPixels(glCopyPixels_ARG_EXPAND);
typedef void (*glesptr_glCopyPixels)(glCopyPixels_ARG_EXPAND);
void glCopyTexImage1D(glCopyTexImage1D_ARG_EXPAND);
typedef void (*glesptr_glCopyTexImage1D)(glCopyTexImage1D_ARG_EXPAND);
void glCopyTexImage2D(glCopyTexImage2D_ARG_EXPAND);
typedef void (*glesptr_glCopyTexImage2D)(glCopyTexImage2D_ARG_EXPAND);
void glCopyTexSubImage1D(glCopyTexSubImage1D_ARG_EXPAND);
typedef void (*glesptr_glCopyTexSubImage1D)(glCopyTexSubImage1D_ARG_EXPAND);
void glCopyTexSubImage2D(glCopyTexSubImage2D_ARG_EXPAND);
typedef void (*glesptr_glCopyTexSubImage2D)(glCopyTexSubImage2D_ARG_EXPAND);
void glCopyTexSubImage3D(glCopyTexSubImage3D_ARG_EXPAND);
typedef void (*glesptr_glCopyTexSubImage3D)(glCopyTexSubImage3D_ARG_EXPAND);
void glCullFace(glCullFace_ARG_EXPAND);
typedef void (*glesptr_glCullFace)(glCullFace_ARG_EXPAND);
void glDeleteBuffers(glDeleteBuffers_ARG_EXPAND);
typedef void (*glesptr_glDeleteBuffers)(glDeleteBuffers_ARG_EXPAND);
void glDeleteLists(glDeleteLists_ARG_EXPAND);
typedef void (*glesptr_glDeleteLists)(glDeleteLists_ARG_EXPAND);
void glDeleteQueries(glDeleteQueries_ARG_EXPAND);
typedef void (*glesptr_glDeleteQueries)(glDeleteQueries_ARG_EXPAND);
void glDeleteTextures(glDeleteTextures_ARG_EXPAND);
typedef void (*glesptr_glDeleteTextures)(glDeleteTextures_ARG_EXPAND);
void glDepthFunc(glDepthFunc_ARG_EXPAND);
typedef void (*glesptr_glDepthFunc)(glDepthFunc_ARG_EXPAND);
void glDepthMask(glDepthMask_ARG_EXPAND);
typedef void (*glesptr_glDepthMask)(glDepthMask_ARG_EXPAND);
void glDepthRange(glDepthRange_ARG_EXPAND);
typedef void (*glesptr_glDepthRange)(glDepthRange_ARG_EXPAND);
void glDisable(glDisable_ARG_EXPAND);
typedef void (*glesptr_glDisable)(glDisable_ARG_EXPAND);
void glDisableClientState(glDisableClientState_ARG_EXPAND);
typedef void (*glesptr_glDisableClientState)(glDisableClientState_ARG_EXPAND);
void glDrawArrays(glDrawArrays_ARG_EXPAND);
typedef void (*glesptr_glDrawArrays)(glDrawArrays_ARG_EXPAND);
void glDrawBuffer(glDrawBuffer_ARG_EXPAND);
typedef void (*glesptr_glDrawBuffer)(glDrawBuffer_ARG_EXPAND);
void glDrawElements(glDrawElements_ARG_EXPAND);
typedef void (*glesptr_glDrawElements)(glDrawElements_ARG_EXPAND);
void glDrawPixels(glDrawPixels_ARG_EXPAND);
typedef void (*glesptr_glDrawPixels)(glDrawPixels_ARG_EXPAND);
void glDrawRangeElements(glDrawRangeElements_ARG_EXPAND);
typedef void (*glesptr_glDrawRangeElements)(glDrawRangeElements_ARG_EXPAND);
void glEdgeFlag(glEdgeFlag_ARG_EXPAND);
typedef void (*glesptr_glEdgeFlag)(glEdgeFlag_ARG_EXPAND);
void glEdgeFlagPointer(glEdgeFlagPointer_ARG_EXPAND);
typedef void (*glesptr_glEdgeFlagPointer)(glEdgeFlagPointer_ARG_EXPAND);
void glEdgeFlagv(glEdgeFlagv_ARG_EXPAND);
typedef void (*glesptr_glEdgeFlagv)(glEdgeFlagv_ARG_EXPAND);
void glEnable(glEnable_ARG_EXPAND);
typedef void (*glesptr_glEnable)(glEnable_ARG_EXPAND);
void glEnableClientState(glEnableClientState_ARG_EXPAND);
typedef void (*glesptr_glEnableClientState)(glEnableClientState_ARG_EXPAND);
void glEnd(glEnd_ARG_EXPAND);
typedef void (*glesptr_glEnd)(glEnd_ARG_EXPAND);
void glEndList(glEndList_ARG_EXPAND);
typedef void (*glesptr_glEndList)(glEndList_ARG_EXPAND);
void glEndQuery(glEndQuery_ARG_EXPAND);
typedef void (*glesptr_glEndQuery)(glEndQuery_ARG_EXPAND);
void glEvalCoord1d(glEvalCoord1d_ARG_EXPAND);
typedef void (*glesptr_glEvalCoord1d)(glEvalCoord1d_ARG_EXPAND);
void glEvalCoord1dv(glEvalCoord1dv_ARG_EXPAND);
typedef void (*glesptr_glEvalCoord1dv)(glEvalCoord1dv_ARG_EXPAND);
void glEvalCoord1f(glEvalCoord1f_ARG_EXPAND);
typedef void (*glesptr_glEvalCoord1f)(glEvalCoord1f_ARG_EXPAND);
void glEvalCoord1fv(glEvalCoord1fv_ARG_EXPAND);
typedef void (*glesptr_glEvalCoord1fv)(glEvalCoord1fv_ARG_EXPAND);
void glEvalCoord2d(glEvalCoord2d_ARG_EXPAND);
typedef void (*glesptr_glEvalCoord2d)(glEvalCoord2d_ARG_EXPAND);
void glEvalCoord2dv(glEvalCoord2dv_ARG_EXPAND);
typedef void (*glesptr_glEvalCoord2dv)(glEvalCoord2dv_ARG_EXPAND);
void glEvalCoord2f(glEvalCoord2f_ARG_EXPAND);
typedef void (*glesptr_glEvalCoord2f)(glEvalCoord2f_ARG_EXPAND);
void glEvalCoord2fv(glEvalCoord2fv_ARG_EXPAND);
typedef void (*glesptr_glEvalCoord2fv)(glEvalCoord2fv_ARG_EXPAND);
void glEvalMesh1(glEvalMesh1_ARG_EXPAND);
typedef void (*glesptr_glEvalMesh1)(glEvalMesh1_ARG_EXPAND);
void glEvalMesh2(glEvalMesh2_ARG_EXPAND);
typedef void (*glesptr_glEvalMesh2)(glEvalMesh2_ARG_EXPAND);
void glEvalPoint1(glEvalPoint1_ARG_EXPAND);
typedef void (*glesptr_glEvalPoint1)(glEvalPoint1_ARG_EXPAND);
void glEvalPoint2(glEvalPoint2_ARG_EXPAND);
typedef void (*glesptr_glEvalPoint2)(glEvalPoint2_ARG_EXPAND);
void glFeedbackBuffer(glFeedbackBuffer_ARG_EXPAND);
typedef void (*glesptr_glFeedbackBuffer)(glFeedbackBuffer_ARG_EXPAND);
void glFinish(glFinish_ARG_EXPAND);
typedef void (*glesptr_glFinish)(glFinish_ARG_EXPAND);
void glFlush(glFlush_ARG_EXPAND);
typedef void (*glesptr_glFlush)(glFlush_ARG_EXPAND);
void glFogCoordPointer(glFogCoordPointer_ARG_EXPAND);
typedef void (*glesptr_glFogCoordPointer)(glFogCoordPointer_ARG_EXPAND);
void glFogCoordd(glFogCoordd_ARG_EXPAND);
typedef void (*glesptr_glFogCoordd)(glFogCoordd_ARG_EXPAND);
void glFogCoorddv(glFogCoorddv_ARG_EXPAND);
typedef void (*glesptr_glFogCoorddv)(glFogCoorddv_ARG_EXPAND);
void glFogCoordf(glFogCoordf_ARG_EXPAND);
typedef void (*glesptr_glFogCoordf)(glFogCoordf_ARG_EXPAND);
void glFogCoordfv(glFogCoordfv_ARG_EXPAND);
typedef void (*glesptr_glFogCoordfv)(glFogCoordfv_ARG_EXPAND);
void glFogf(glFogf_ARG_EXPAND);
typedef void (*glesptr_glFogf)(glFogf_ARG_EXPAND);
void glFogfv(glFogfv_ARG_EXPAND);
typedef void (*glesptr_glFogfv)(glFogfv_ARG_EXPAND);
void glFogi(glFogi_ARG_EXPAND);
typedef void (*glesptr_glFogi)(glFogi_ARG_EXPAND);
void glFogiv(glFogiv_ARG_EXPAND);
typedef void (*glesptr_glFogiv)(glFogiv_ARG_EXPAND);
void glFrontFace(glFrontFace_ARG_EXPAND);
typedef void (*glesptr_glFrontFace)(glFrontFace_ARG_EXPAND);
void glFrustum(glFrustum_ARG_EXPAND);
typedef void (*glesptr_glFrustum)(glFrustum_ARG_EXPAND);
void glGenBuffers(glGenBuffers_ARG_EXPAND);
typedef void (*glesptr_glGenBuffers)(glGenBuffers_ARG_EXPAND);
GLuint glGenLists(glGenLists_ARG_EXPAND);
typedef GLuint (*glesptr_glGenLists)(glGenLists_ARG_EXPAND);
void glGenQueries(glGenQueries_ARG_EXPAND);
typedef void (*glesptr_glGenQueries)(glGenQueries_ARG_EXPAND);
void glGenTextures(glGenTextures_ARG_EXPAND);
typedef void (*glesptr_glGenTextures)(glGenTextures_ARG_EXPAND);
void glGetBooleanv(glGetBooleanv_ARG_EXPAND);
typedef void (*glesptr_glGetBooleanv)(glGetBooleanv_ARG_EXPAND);
void glGetBufferParameteriv(glGetBufferParameteriv_ARG_EXPAND);
typedef void (*glesptr_glGetBufferParameteriv)(glGetBufferParameteriv_ARG_EXPAND);
void glGetBufferPointerv(glGetBufferPointerv_ARG_EXPAND);
typedef void (*glesptr_glGetBufferPointerv)(glGetBufferPointerv_ARG_EXPAND);
void glGetBufferSubData(glGetBufferSubData_ARG_EXPAND);
typedef void (*glesptr_glGetBufferSubData)(glGetBufferSubData_ARG_EXPAND);
void glGetClipPlane(glGetClipPlane_ARG_EXPAND);
typedef void (*glesptr_glGetClipPlane)(glGetClipPlane_ARG_EXPAND);
void glGetColorTable(glGetColorTable_ARG_EXPAND);
typedef void (*glesptr_glGetColorTable)(glGetColorTable_ARG_EXPAND);
void glGetColorTableParameterfv(glGetColorTableParameterfv_ARG_EXPAND);
typedef void (*glesptr_glGetColorTableParameterfv)(glGetColorTableParameterfv_ARG_EXPAND);
void glGetColorTableParameteriv(glGetColorTableParameteriv_ARG_EXPAND);
typedef void (*glesptr_glGetColorTableParameteriv)(glGetColorTableParameteriv_ARG_EXPAND);
void glGetCompressedTexImage(glGetCompressedTexImage_ARG_EXPAND);
typedef void (*glesptr_glGetCompressedTexImage)(glGetCompressedTexImage_ARG_EXPAND);
void glGetConvolutionFilter(glGetConvolutionFilter_ARG_EXPAND);
typedef void (*glesptr_glGetConvolutionFilter)(glGetConvolutionFilter_ARG_EXPAND);
void glGetConvolutionParameterfv(glGetConvolutionParameterfv_ARG_EXPAND);
typedef void (*glesptr_glGetConvolutionParameterfv)(glGetConvolutionParameterfv_ARG_EXPAND);
void glGetConvolutionParameteriv(glGetConvolutionParameteriv_ARG_EXPAND);
typedef void (*glesptr_glGetConvolutionParameteriv)(glGetConvolutionParameteriv_ARG_EXPAND);
void glGetDoublev(glGetDoublev_ARG_EXPAND);
typedef void (*glesptr_glGetDoublev)(glGetDoublev_ARG_EXPAND);
GLenum glGetError(glGetError_ARG_EXPAND);
typedef GLenum (*glesptr_glGetError)(glGetError_ARG_EXPAND);
void glGetFloatv(glGetFloatv_ARG_EXPAND);
typedef void (*glesptr_glGetFloatv)(glGetFloatv_ARG_EXPAND);
void glGetHistogram(glGetHistogram_ARG_EXPAND);
typedef void (*glesptr_glGetHistogram)(glGetHistogram_ARG_EXPAND);
void glGetHistogramParameterfv(glGetHistogramParameterfv_ARG_EXPAND);
typedef void (*glesptr_glGetHistogramParameterfv)(glGetHistogramParameterfv_ARG_EXPAND);
void glGetHistogramParameteriv(glGetHistogramParameteriv_ARG_EXPAND);
typedef void (*glesptr_glGetHistogramParameteriv)(glGetHistogramParameteriv_ARG_EXPAND);
void glGetIntegerv(glGetIntegerv_ARG_EXPAND);
typedef void (*glesptr_glGetIntegerv)(glGetIntegerv_ARG_EXPAND);
void glGetLightfv(glGetLightfv_ARG_EXPAND);
typedef void (*glesptr_glGetLightfv)(glGetLightfv_ARG_EXPAND);
void glGetLightiv(glGetLightiv_ARG_EXPAND);
typedef void (*glesptr_glGetLightiv)(glGetLightiv_ARG_EXPAND);
void glGetMapdv(glGetMapdv_ARG_EXPAND);
typedef void (*glesptr_glGetMapdv)(glGetMapdv_ARG_EXPAND);
void glGetMapfv(glGetMapfv_ARG_EXPAND);
typedef void (*glesptr_glGetMapfv)(glGetMapfv_ARG_EXPAND);
void glGetMapiv(glGetMapiv_ARG_EXPAND);
typedef void (*glesptr_glGetMapiv)(glGetMapiv_ARG_EXPAND);
void glGetMaterialfv(glGetMaterialfv_ARG_EXPAND);
typedef void (*glesptr_glGetMaterialfv)(glGetMaterialfv_ARG_EXPAND);
void glGetMaterialiv(glGetMaterialiv_ARG_EXPAND);
typedef void (*glesptr_glGetMaterialiv)(glGetMaterialiv_ARG_EXPAND);
void glGetMinmax(glGetMinmax_ARG_EXPAND);
typedef void (*glesptr_glGetMinmax)(glGetMinmax_ARG_EXPAND);
void glGetMinmaxParameterfv(glGetMinmaxParameterfv_ARG_EXPAND);
typedef void (*glesptr_glGetMinmaxParameterfv)(glGetMinmaxParameterfv_ARG_EXPAND);
void glGetMinmaxParameteriv(glGetMinmaxParameteriv_ARG_EXPAND);
typedef void (*glesptr_glGetMinmaxParameteriv)(glGetMinmaxParameteriv_ARG_EXPAND);
void glGetPixelMapfv(glGetPixelMapfv_ARG_EXPAND);
typedef void (*glesptr_glGetPixelMapfv)(glGetPixelMapfv_ARG_EXPAND);
void glGetPixelMapuiv(glGetPixelMapuiv_ARG_EXPAND);
typedef void (*glesptr_glGetPixelMapuiv)(glGetPixelMapuiv_ARG_EXPAND);
void glGetPixelMapusv(glGetPixelMapusv_ARG_EXPAND);
typedef void (*glesptr_glGetPixelMapusv)(glGetPixelMapusv_ARG_EXPAND);
void glGetPointerv(glGetPointerv_ARG_EXPAND);
typedef void (*glesptr_glGetPointerv)(glGetPointerv_ARG_EXPAND);
void glGetPolygonStipple(glGetPolygonStipple_ARG_EXPAND);
typedef void (*glesptr_glGetPolygonStipple)(glGetPolygonStipple_ARG_EXPAND);
void glGetQueryObjectiv(glGetQueryObjectiv_ARG_EXPAND);
typedef void (*glesptr_glGetQueryObjectiv)(glGetQueryObjectiv_ARG_EXPAND);
void glGetQueryObjectuiv(glGetQueryObjectuiv_ARG_EXPAND);
typedef void (*glesptr_glGetQueryObjectuiv)(glGetQueryObjectuiv_ARG_EXPAND);
void glGetQueryiv(glGetQueryiv_ARG_EXPAND);
typedef void (*glesptr_glGetQueryiv)(glGetQueryiv_ARG_EXPAND);
void glGetSeparableFilter(glGetSeparableFilter_ARG_EXPAND);
typedef void (*glesptr_glGetSeparableFilter)(glGetSeparableFilter_ARG_EXPAND);
const GLubyte * glGetString(glGetString_ARG_EXPAND);
typedef const GLubyte * (*glesptr_glGetString)(glGetString_ARG_EXPAND);
void glGetTexEnvfv(glGetTexEnvfv_ARG_EXPAND);
typedef void (*glesptr_glGetTexEnvfv)(glGetTexEnvfv_ARG_EXPAND);
void glGetTexEnviv(glGetTexEnviv_ARG_EXPAND);
typedef void (*glesptr_glGetTexEnviv)(glGetTexEnviv_ARG_EXPAND);
void glGetTexGendv(glGetTexGendv_ARG_EXPAND);
typedef void (*glesptr_glGetTexGendv)(glGetTexGendv_ARG_EXPAND);
void glGetTexGenfv(glGetTexGenfv_ARG_EXPAND);
typedef void (*glesptr_glGetTexGenfv)(glGetTexGenfv_ARG_EXPAND);
void glGetTexGeniv(glGetTexGeniv_ARG_EXPAND);
typedef void (*glesptr_glGetTexGeniv)(glGetTexGeniv_ARG_EXPAND);
void glGetTexImage(glGetTexImage_ARG_EXPAND);
typedef void (*glesptr_glGetTexImage)(glGetTexImage_ARG_EXPAND);
void glGetTexLevelParameterfv(glGetTexLevelParameterfv_ARG_EXPAND);
typedef void (*glesptr_glGetTexLevelParameterfv)(glGetTexLevelParameterfv_ARG_EXPAND);
void glGetTexLevelParameteriv(glGetTexLevelParameteriv_ARG_EXPAND);
typedef void (*glesptr_glGetTexLevelParameteriv)(glGetTexLevelParameteriv_ARG_EXPAND);
void glGetTexParameterfv(glGetTexParameterfv_ARG_EXPAND);
typedef void (*glesptr_glGetTexParameterfv)(glGetTexParameterfv_ARG_EXPAND);
void glGetTexParameteriv(glGetTexParameteriv_ARG_EXPAND);
typedef void (*glesptr_glGetTexParameteriv)(glGetTexParameteriv_ARG_EXPAND);
void glHint(glHint_ARG_EXPAND);
typedef void (*glesptr_glHint)(glHint_ARG_EXPAND);
void glHistogram(glHistogram_ARG_EXPAND);
typedef void (*glesptr_glHistogram)(glHistogram_ARG_EXPAND);
void glIndexMask(glIndexMask_ARG_EXPAND);
typedef void (*glesptr_glIndexMask)(glIndexMask_ARG_EXPAND);
void glIndexPointer(glIndexPointer_ARG_EXPAND);
typedef void (*glesptr_glIndexPointer)(glIndexPointer_ARG_EXPAND);
void glIndexd(glIndexd_ARG_EXPAND);
typedef void (*glesptr_glIndexd)(glIndexd_ARG_EXPAND);
void glIndexdv(glIndexdv_ARG_EXPAND);
typedef void (*glesptr_glIndexdv)(glIndexdv_ARG_EXPAND);
void glIndexf(glIndexf_ARG_EXPAND);
typedef void (*glesptr_glIndexf)(glIndexf_ARG_EXPAND);
void glIndexfv(glIndexfv_ARG_EXPAND);
typedef void (*glesptr_glIndexfv)(glIndexfv_ARG_EXPAND);
void glIndexi(glIndexi_ARG_EXPAND);
typedef void (*glesptr_glIndexi)(glIndexi_ARG_EXPAND);
void glIndexiv(glIndexiv_ARG_EXPAND);
typedef void (*glesptr_glIndexiv)(glIndexiv_ARG_EXPAND);
void glIndexs(glIndexs_ARG_EXPAND);
typedef void (*glesptr_glIndexs)(glIndexs_ARG_EXPAND);
void glIndexsv(glIndexsv_ARG_EXPAND);
typedef void (*glesptr_glIndexsv)(glIndexsv_ARG_EXPAND);
void glIndexub(glIndexub_ARG_EXPAND);
typedef void (*glesptr_glIndexub)(glIndexub_ARG_EXPAND);
void glIndexubv(glIndexubv_ARG_EXPAND);
typedef void (*glesptr_glIndexubv)(glIndexubv_ARG_EXPAND);
void glInitNames(glInitNames_ARG_EXPAND);
typedef void (*glesptr_glInitNames)(glInitNames_ARG_EXPAND);
void glInterleavedArrays(glInterleavedArrays_ARG_EXPAND);
typedef void (*glesptr_glInterleavedArrays)(glInterleavedArrays_ARG_EXPAND);
GLboolean glIsBuffer(glIsBuffer_ARG_EXPAND);
typedef GLboolean (*glesptr_glIsBuffer)(glIsBuffer_ARG_EXPAND);
GLboolean glIsEnabled(glIsEnabled_ARG_EXPAND);
typedef GLboolean (*glesptr_glIsEnabled)(glIsEnabled_ARG_EXPAND);
GLboolean glIsList(glIsList_ARG_EXPAND);
typedef GLboolean (*glesptr_glIsList)(glIsList_ARG_EXPAND);
GLboolean glIsQuery(glIsQuery_ARG_EXPAND);
typedef GLboolean (*glesptr_glIsQuery)(glIsQuery_ARG_EXPAND);
GLboolean glIsTexture(glIsTexture_ARG_EXPAND);
typedef GLboolean (*glesptr_glIsTexture)(glIsTexture_ARG_EXPAND);
void glLightModelf(glLightModelf_ARG_EXPAND);
typedef void (*glesptr_glLightModelf)(glLightModelf_ARG_EXPAND);
void glLightModelfv(glLightModelfv_ARG_EXPAND);
typedef void (*glesptr_glLightModelfv)(glLightModelfv_ARG_EXPAND);
void glLightModeli(glLightModeli_ARG_EXPAND);
typedef void (*glesptr_glLightModeli)(glLightModeli_ARG_EXPAND);
void glLightModeliv(glLightModeliv_ARG_EXPAND);
typedef void (*glesptr_glLightModeliv)(glLightModeliv_ARG_EXPAND);
void glLightf(glLightf_ARG_EXPAND);
typedef void (*glesptr_glLightf)(glLightf_ARG_EXPAND);
void glLightfv(glLightfv_ARG_EXPAND);
typedef void (*glesptr_glLightfv)(glLightfv_ARG_EXPAND);
void glLighti(glLighti_ARG_EXPAND);
typedef void (*glesptr_glLighti)(glLighti_ARG_EXPAND);
void glLightiv(glLightiv_ARG_EXPAND);
typedef void (*glesptr_glLightiv)(glLightiv_ARG_EXPAND);
void glLineStipple(glLineStipple_ARG_EXPAND);
typedef void (*glesptr_glLineStipple)(glLineStipple_ARG_EXPAND);
void glLineWidth(glLineWidth_ARG_EXPAND);
typedef void (*glesptr_glLineWidth)(glLineWidth_ARG_EXPAND);
void glListBase(glListBase_ARG_EXPAND);
typedef void (*glesptr_glListBase)(glListBase_ARG_EXPAND);
void glLoadIdentity(glLoadIdentity_ARG_EXPAND);
typedef void (*glesptr_glLoadIdentity)(glLoadIdentity_ARG_EXPAND);
void glLoadMatrixd(glLoadMatrixd_ARG_EXPAND);
typedef void (*glesptr_glLoadMatrixd)(glLoadMatrixd_ARG_EXPAND);
void glLoadMatrixf(glLoadMatrixf_ARG_EXPAND);
typedef void (*glesptr_glLoadMatrixf)(glLoadMatrixf_ARG_EXPAND);
void glLoadName(glLoadName_ARG_EXPAND);
typedef void (*glesptr_glLoadName)(glLoadName_ARG_EXPAND);
void glLoadTransposeMatrixd(glLoadTransposeMatrixd_ARG_EXPAND);
typedef void (*glesptr_glLoadTransposeMatrixd)(glLoadTransposeMatrixd_ARG_EXPAND);
void glLoadTransposeMatrixf(glLoadTransposeMatrixf_ARG_EXPAND);
typedef void (*glesptr_glLoadTransposeMatrixf)(glLoadTransposeMatrixf_ARG_EXPAND);
void glLogicOp(glLogicOp_ARG_EXPAND);
typedef void (*glesptr_glLogicOp)(glLogicOp_ARG_EXPAND);
void glMap1d(glMap1d_ARG_EXPAND);
typedef void (*glesptr_glMap1d)(glMap1d_ARG_EXPAND);
void glMap1f(glMap1f_ARG_EXPAND);
typedef void (*glesptr_glMap1f)(glMap1f_ARG_EXPAND);
void glMap2d(glMap2d_ARG_EXPAND);
typedef void (*glesptr_glMap2d)(glMap2d_ARG_EXPAND);
void glMap2f(glMap2f_ARG_EXPAND);
typedef void (*glesptr_glMap2f)(glMap2f_ARG_EXPAND);
GLvoid * glMapBuffer(glMapBuffer_ARG_EXPAND);
typedef GLvoid * (*glesptr_glMapBuffer)(glMapBuffer_ARG_EXPAND);
void glMapGrid1d(glMapGrid1d_ARG_EXPAND);
typedef void (*glesptr_glMapGrid1d)(glMapGrid1d_ARG_EXPAND);
void glMapGrid1f(glMapGrid1f_ARG_EXPAND);
typedef void (*glesptr_glMapGrid1f)(glMapGrid1f_ARG_EXPAND);
void glMapGrid2d(glMapGrid2d_ARG_EXPAND);
typedef void (*glesptr_glMapGrid2d)(glMapGrid2d_ARG_EXPAND);
void glMapGrid2f(glMapGrid2f_ARG_EXPAND);
typedef void (*glesptr_glMapGrid2f)(glMapGrid2f_ARG_EXPAND);
void glMaterialf(glMaterialf_ARG_EXPAND);
typedef void (*glesptr_glMaterialf)(glMaterialf_ARG_EXPAND);
void glMaterialfv(glMaterialfv_ARG_EXPAND);
typedef void (*glesptr_glMaterialfv)(glMaterialfv_ARG_EXPAND);
void glMateriali(glMateriali_ARG_EXPAND);
typedef void (*glesptr_glMateriali)(glMateriali_ARG_EXPAND);
void glMaterialiv(glMaterialiv_ARG_EXPAND);
typedef void (*glesptr_glMaterialiv)(glMaterialiv_ARG_EXPAND);
void glMatrixMode(glMatrixMode_ARG_EXPAND);
typedef void (*glesptr_glMatrixMode)(glMatrixMode_ARG_EXPAND);
void glMinmax(glMinmax_ARG_EXPAND);
typedef void (*glesptr_glMinmax)(glMinmax_ARG_EXPAND);
void glMultMatrixd(glMultMatrixd_ARG_EXPAND);
typedef void (*glesptr_glMultMatrixd)(glMultMatrixd_ARG_EXPAND);
void glMultMatrixf(glMultMatrixf_ARG_EXPAND);
typedef void (*glesptr_glMultMatrixf)(glMultMatrixf_ARG_EXPAND);
void glMultTransposeMatrixd(glMultTransposeMatrixd_ARG_EXPAND);
typedef void (*glesptr_glMultTransposeMatrixd)(glMultTransposeMatrixd_ARG_EXPAND);
void glMultTransposeMatrixf(glMultTransposeMatrixf_ARG_EXPAND);
typedef void (*glesptr_glMultTransposeMatrixf)(glMultTransposeMatrixf_ARG_EXPAND);
void glMultiDrawArrays(glMultiDrawArrays_ARG_EXPAND);
typedef void (*glesptr_glMultiDrawArrays)(glMultiDrawArrays_ARG_EXPAND);
void glMultiDrawElements(glMultiDrawElements_ARG_EXPAND);
typedef void (*glesptr_glMultiDrawElements)(glMultiDrawElements_ARG_EXPAND);
void glMultiTexCoord1d(glMultiTexCoord1d_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord1d)(glMultiTexCoord1d_ARG_EXPAND);
void glMultiTexCoord1dv(glMultiTexCoord1dv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord1dv)(glMultiTexCoord1dv_ARG_EXPAND);
void glMultiTexCoord1f(glMultiTexCoord1f_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord1f)(glMultiTexCoord1f_ARG_EXPAND);
void glMultiTexCoord1fv(glMultiTexCoord1fv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord1fv)(glMultiTexCoord1fv_ARG_EXPAND);
void glMultiTexCoord1i(glMultiTexCoord1i_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord1i)(glMultiTexCoord1i_ARG_EXPAND);
void glMultiTexCoord1iv(glMultiTexCoord1iv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord1iv)(glMultiTexCoord1iv_ARG_EXPAND);
void glMultiTexCoord1s(glMultiTexCoord1s_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord1s)(glMultiTexCoord1s_ARG_EXPAND);
void glMultiTexCoord1sv(glMultiTexCoord1sv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord1sv)(glMultiTexCoord1sv_ARG_EXPAND);
void glMultiTexCoord2d(glMultiTexCoord2d_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord2d)(glMultiTexCoord2d_ARG_EXPAND);
void glMultiTexCoord2dv(glMultiTexCoord2dv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord2dv)(glMultiTexCoord2dv_ARG_EXPAND);
void glMultiTexCoord2f(glMultiTexCoord2f_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord2f)(glMultiTexCoord2f_ARG_EXPAND);
void glMultiTexCoord2fv(glMultiTexCoord2fv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord2fv)(glMultiTexCoord2fv_ARG_EXPAND);
void glMultiTexCoord2i(glMultiTexCoord2i_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord2i)(glMultiTexCoord2i_ARG_EXPAND);
void glMultiTexCoord2iv(glMultiTexCoord2iv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord2iv)(glMultiTexCoord2iv_ARG_EXPAND);
void glMultiTexCoord2s(glMultiTexCoord2s_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord2s)(glMultiTexCoord2s_ARG_EXPAND);
void glMultiTexCoord2sv(glMultiTexCoord2sv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord2sv)(glMultiTexCoord2sv_ARG_EXPAND);
void glMultiTexCoord3d(glMultiTexCoord3d_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord3d)(glMultiTexCoord3d_ARG_EXPAND);
void glMultiTexCoord3dv(glMultiTexCoord3dv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord3dv)(glMultiTexCoord3dv_ARG_EXPAND);
void glMultiTexCoord3f(glMultiTexCoord3f_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord3f)(glMultiTexCoord3f_ARG_EXPAND);
void glMultiTexCoord3fv(glMultiTexCoord3fv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord3fv)(glMultiTexCoord3fv_ARG_EXPAND);
void glMultiTexCoord3i(glMultiTexCoord3i_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord3i)(glMultiTexCoord3i_ARG_EXPAND);
void glMultiTexCoord3iv(glMultiTexCoord3iv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord3iv)(glMultiTexCoord3iv_ARG_EXPAND);
void glMultiTexCoord3s(glMultiTexCoord3s_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord3s)(glMultiTexCoord3s_ARG_EXPAND);
void glMultiTexCoord3sv(glMultiTexCoord3sv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord3sv)(glMultiTexCoord3sv_ARG_EXPAND);
void glMultiTexCoord4d(glMultiTexCoord4d_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord4d)(glMultiTexCoord4d_ARG_EXPAND);
void glMultiTexCoord4dv(glMultiTexCoord4dv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord4dv)(glMultiTexCoord4dv_ARG_EXPAND);
void glMultiTexCoord4f(glMultiTexCoord4f_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord4f)(glMultiTexCoord4f_ARG_EXPAND);
void glMultiTexCoord4fv(glMultiTexCoord4fv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord4fv)(glMultiTexCoord4fv_ARG_EXPAND);
void glMultiTexCoord4i(glMultiTexCoord4i_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord4i)(glMultiTexCoord4i_ARG_EXPAND);
void glMultiTexCoord4iv(glMultiTexCoord4iv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord4iv)(glMultiTexCoord4iv_ARG_EXPAND);
void glMultiTexCoord4s(glMultiTexCoord4s_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord4s)(glMultiTexCoord4s_ARG_EXPAND);
void glMultiTexCoord4sv(glMultiTexCoord4sv_ARG_EXPAND);
typedef void (*glesptr_glMultiTexCoord4sv)(glMultiTexCoord4sv_ARG_EXPAND);
void glNewList(glNewList_ARG_EXPAND);
typedef void (*glesptr_glNewList)(glNewList_ARG_EXPAND);
void glNormal3b(glNormal3b_ARG_EXPAND);
typedef void (*glesptr_glNormal3b)(glNormal3b_ARG_EXPAND);
void glNormal3bv(glNormal3bv_ARG_EXPAND);
typedef void (*glesptr_glNormal3bv)(glNormal3bv_ARG_EXPAND);
void glNormal3d(glNormal3d_ARG_EXPAND);
typedef void (*glesptr_glNormal3d)(glNormal3d_ARG_EXPAND);
void glNormal3dv(glNormal3dv_ARG_EXPAND);
typedef void (*glesptr_glNormal3dv)(glNormal3dv_ARG_EXPAND);
void glNormal3f(glNormal3f_ARG_EXPAND);
typedef void (*glesptr_glNormal3f)(glNormal3f_ARG_EXPAND);
void glNormal3fv(glNormal3fv_ARG_EXPAND);
typedef void (*glesptr_glNormal3fv)(glNormal3fv_ARG_EXPAND);
void glNormal3i(glNormal3i_ARG_EXPAND);
typedef void (*glesptr_glNormal3i)(glNormal3i_ARG_EXPAND);
void glNormal3iv(glNormal3iv_ARG_EXPAND);
typedef void (*glesptr_glNormal3iv)(glNormal3iv_ARG_EXPAND);
void glNormal3s(glNormal3s_ARG_EXPAND);
typedef void (*glesptr_glNormal3s)(glNormal3s_ARG_EXPAND);
void glNormal3sv(glNormal3sv_ARG_EXPAND);
typedef void (*glesptr_glNormal3sv)(glNormal3sv_ARG_EXPAND);
void glNormalPointer(glNormalPointer_ARG_EXPAND);
typedef void (*glesptr_glNormalPointer)(glNormalPointer_ARG_EXPAND);
void glOrtho(glOrtho_ARG_EXPAND);
typedef void (*glesptr_glOrtho)(glOrtho_ARG_EXPAND);
void glPassThrough(glPassThrough_ARG_EXPAND);
typedef void (*glesptr_glPassThrough)(glPassThrough_ARG_EXPAND);
void glPixelMapfv(glPixelMapfv_ARG_EXPAND);
typedef void (*glesptr_glPixelMapfv)(glPixelMapfv_ARG_EXPAND);
void glPixelMapuiv(glPixelMapuiv_ARG_EXPAND);
typedef void (*glesptr_glPixelMapuiv)(glPixelMapuiv_ARG_EXPAND);
void glPixelMapusv(glPixelMapusv_ARG_EXPAND);
typedef void (*glesptr_glPixelMapusv)(glPixelMapusv_ARG_EXPAND);
void glPixelStoref(glPixelStoref_ARG_EXPAND);
typedef void (*glesptr_glPixelStoref)(glPixelStoref_ARG_EXPAND);
void glPixelStorei(glPixelStorei_ARG_EXPAND);
typedef void (*glesptr_glPixelStorei)(glPixelStorei_ARG_EXPAND);
void glPixelTransferf(glPixelTransferf_ARG_EXPAND);
typedef void (*glesptr_glPixelTransferf)(glPixelTransferf_ARG_EXPAND);
void glPixelTransferi(glPixelTransferi_ARG_EXPAND);
typedef void (*glesptr_glPixelTransferi)(glPixelTransferi_ARG_EXPAND);
void glPixelZoom(glPixelZoom_ARG_EXPAND);
typedef void (*glesptr_glPixelZoom)(glPixelZoom_ARG_EXPAND);
void glPointParameterf(glPointParameterf_ARG_EXPAND);
typedef void (*glesptr_glPointParameterf)(glPointParameterf_ARG_EXPAND);
void glPointParameterfv(glPointParameterfv_ARG_EXPAND);
typedef void (*glesptr_glPointParameterfv)(glPointParameterfv_ARG_EXPAND);
void glPointParameteri(glPointParameteri_ARG_EXPAND);
typedef void (*glesptr_glPointParameteri)(glPointParameteri_ARG_EXPAND);
void glPointParameteriv(glPointParameteriv_ARG_EXPAND);
typedef void (*glesptr_glPointParameteriv)(glPointParameteriv_ARG_EXPAND);
void glPointSize(glPointSize_ARG_EXPAND);
typedef void (*glesptr_glPointSize)(glPointSize_ARG_EXPAND);
void glPolygonMode(glPolygonMode_ARG_EXPAND);
typedef void (*glesptr_glPolygonMode)(glPolygonMode_ARG_EXPAND);
void glPolygonOffset(glPolygonOffset_ARG_EXPAND);
typedef void (*glesptr_glPolygonOffset)(glPolygonOffset_ARG_EXPAND);
void glPolygonStipple(glPolygonStipple_ARG_EXPAND);
typedef void (*glesptr_glPolygonStipple)(glPolygonStipple_ARG_EXPAND);
void glPopAttrib(glPopAttrib_ARG_EXPAND);
typedef void (*glesptr_glPopAttrib)(glPopAttrib_ARG_EXPAND);
void glPopClientAttrib(glPopClientAttrib_ARG_EXPAND);
typedef void (*glesptr_glPopClientAttrib)(glPopClientAttrib_ARG_EXPAND);
void glPopMatrix(glPopMatrix_ARG_EXPAND);
typedef void (*glesptr_glPopMatrix)(glPopMatrix_ARG_EXPAND);
void glPopName(glPopName_ARG_EXPAND);
typedef void (*glesptr_glPopName)(glPopName_ARG_EXPAND);
void glPrioritizeTextures(glPrioritizeTextures_ARG_EXPAND);
typedef void (*glesptr_glPrioritizeTextures)(glPrioritizeTextures_ARG_EXPAND);
void glPushAttrib(glPushAttrib_ARG_EXPAND);
typedef void (*glesptr_glPushAttrib)(glPushAttrib_ARG_EXPAND);
void glPushClientAttrib(glPushClientAttrib_ARG_EXPAND);
typedef void (*glesptr_glPushClientAttrib)(glPushClientAttrib_ARG_EXPAND);
void glPushMatrix(glPushMatrix_ARG_EXPAND);
typedef void (*glesptr_glPushMatrix)(glPushMatrix_ARG_EXPAND);
void glPushName(glPushName_ARG_EXPAND);
typedef void (*glesptr_glPushName)(glPushName_ARG_EXPAND);
void glRasterPos2d(glRasterPos2d_ARG_EXPAND);
typedef void (*glesptr_glRasterPos2d)(glRasterPos2d_ARG_EXPAND);
void glRasterPos2dv(glRasterPos2dv_ARG_EXPAND);
typedef void (*glesptr_glRasterPos2dv)(glRasterPos2dv_ARG_EXPAND);
void glRasterPos2f(glRasterPos2f_ARG_EXPAND);
typedef void (*glesptr_glRasterPos2f)(glRasterPos2f_ARG_EXPAND);
void glRasterPos2fv(glRasterPos2fv_ARG_EXPAND);
typedef void (*glesptr_glRasterPos2fv)(glRasterPos2fv_ARG_EXPAND);
void glRasterPos2i(glRasterPos2i_ARG_EXPAND);
typedef void (*glesptr_glRasterPos2i)(glRasterPos2i_ARG_EXPAND);
void glRasterPos2iv(glRasterPos2iv_ARG_EXPAND);
typedef void (*glesptr_glRasterPos2iv)(glRasterPos2iv_ARG_EXPAND);
void glRasterPos2s(glRasterPos2s_ARG_EXPAND);
typedef void (*glesptr_glRasterPos2s)(glRasterPos2s_ARG_EXPAND);
void glRasterPos2sv(glRasterPos2sv_ARG_EXPAND);
typedef void (*glesptr_glRasterPos2sv)(glRasterPos2sv_ARG_EXPAND);
void glRasterPos3d(glRasterPos3d_ARG_EXPAND);
typedef void (*glesptr_glRasterPos3d)(glRasterPos3d_ARG_EXPAND);
void glRasterPos3dv(glRasterPos3dv_ARG_EXPAND);
typedef void (*glesptr_glRasterPos3dv)(glRasterPos3dv_ARG_EXPAND);
void glRasterPos3f(glRasterPos3f_ARG_EXPAND);
typedef void (*glesptr_glRasterPos3f)(glRasterPos3f_ARG_EXPAND);
void glRasterPos3fv(glRasterPos3fv_ARG_EXPAND);
typedef void (*glesptr_glRasterPos3fv)(glRasterPos3fv_ARG_EXPAND);
void glRasterPos3i(glRasterPos3i_ARG_EXPAND);
typedef void (*glesptr_glRasterPos3i)(glRasterPos3i_ARG_EXPAND);
void glRasterPos3iv(glRasterPos3iv_ARG_EXPAND);
typedef void (*glesptr_glRasterPos3iv)(glRasterPos3iv_ARG_EXPAND);
void glRasterPos3s(glRasterPos3s_ARG_EXPAND);
typedef void (*glesptr_glRasterPos3s)(glRasterPos3s_ARG_EXPAND);
void glRasterPos3sv(glRasterPos3sv_ARG_EXPAND);
typedef void (*glesptr_glRasterPos3sv)(glRasterPos3sv_ARG_EXPAND);
void glRasterPos4d(glRasterPos4d_ARG_EXPAND);
typedef void (*glesptr_glRasterPos4d)(glRasterPos4d_ARG_EXPAND);
void glRasterPos4dv(glRasterPos4dv_ARG_EXPAND);
typedef void (*glesptr_glRasterPos4dv)(glRasterPos4dv_ARG_EXPAND);
void glRasterPos4f(glRasterPos4f_ARG_EXPAND);
typedef void (*glesptr_glRasterPos4f)(glRasterPos4f_ARG_EXPAND);
void glRasterPos4fv(glRasterPos4fv_ARG_EXPAND);
typedef void (*glesptr_glRasterPos4fv)(glRasterPos4fv_ARG_EXPAND);
void glRasterPos4i(glRasterPos4i_ARG_EXPAND);
typedef void (*glesptr_glRasterPos4i)(glRasterPos4i_ARG_EXPAND);
void glRasterPos4iv(glRasterPos4iv_ARG_EXPAND);
typedef void (*glesptr_glRasterPos4iv)(glRasterPos4iv_ARG_EXPAND);
void glRasterPos4s(glRasterPos4s_ARG_EXPAND);
typedef void (*glesptr_glRasterPos4s)(glRasterPos4s_ARG_EXPAND);
void glRasterPos4sv(glRasterPos4sv_ARG_EXPAND);
typedef void (*glesptr_glRasterPos4sv)(glRasterPos4sv_ARG_EXPAND);
void glReadBuffer(glReadBuffer_ARG_EXPAND);
typedef void (*glesptr_glReadBuffer)(glReadBuffer_ARG_EXPAND);
void glReadPixels(glReadPixels_ARG_EXPAND);
typedef void (*glesptr_glReadPixels)(glReadPixels_ARG_EXPAND);
void glRectd(glRectd_ARG_EXPAND);
typedef void (*glesptr_glRectd)(glRectd_ARG_EXPAND);
void glRectdv(glRectdv_ARG_EXPAND);
typedef void (*glesptr_glRectdv)(glRectdv_ARG_EXPAND);
void glRectf(glRectf_ARG_EXPAND);
typedef void (*glesptr_glRectf)(glRectf_ARG_EXPAND);
void glRectfv(glRectfv_ARG_EXPAND);
typedef void (*glesptr_glRectfv)(glRectfv_ARG_EXPAND);
void glRecti(glRecti_ARG_EXPAND);
typedef void (*glesptr_glRecti)(glRecti_ARG_EXPAND);
void glRectiv(glRectiv_ARG_EXPAND);
typedef void (*glesptr_glRectiv)(glRectiv_ARG_EXPAND);
void glRects(glRects_ARG_EXPAND);
typedef void (*glesptr_glRects)(glRects_ARG_EXPAND);
void glRectsv(glRectsv_ARG_EXPAND);
typedef void (*glesptr_glRectsv)(glRectsv_ARG_EXPAND);
GLint glRenderMode(glRenderMode_ARG_EXPAND);
typedef GLint (*glesptr_glRenderMode)(glRenderMode_ARG_EXPAND);
void glResetHistogram(glResetHistogram_ARG_EXPAND);
typedef void (*glesptr_glResetHistogram)(glResetHistogram_ARG_EXPAND);
void glResetMinmax(glResetMinmax_ARG_EXPAND);
typedef void (*glesptr_glResetMinmax)(glResetMinmax_ARG_EXPAND);
void glRotated(glRotated_ARG_EXPAND);
typedef void (*glesptr_glRotated)(glRotated_ARG_EXPAND);
void glRotatef(glRotatef_ARG_EXPAND);
typedef void (*glesptr_glRotatef)(glRotatef_ARG_EXPAND);
void glSampleCoverage(glSampleCoverage_ARG_EXPAND);
typedef void (*glesptr_glSampleCoverage)(glSampleCoverage_ARG_EXPAND);
void glScaled(glScaled_ARG_EXPAND);
typedef void (*glesptr_glScaled)(glScaled_ARG_EXPAND);
void glScalef(glScalef_ARG_EXPAND);
typedef void (*glesptr_glScalef)(glScalef_ARG_EXPAND);
void glScissor(glScissor_ARG_EXPAND);
typedef void (*glesptr_glScissor)(glScissor_ARG_EXPAND);
void glSecondaryColor3b(glSecondaryColor3b_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3b)(glSecondaryColor3b_ARG_EXPAND);
void glSecondaryColor3bv(glSecondaryColor3bv_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3bv)(glSecondaryColor3bv_ARG_EXPAND);
void glSecondaryColor3d(glSecondaryColor3d_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3d)(glSecondaryColor3d_ARG_EXPAND);
void glSecondaryColor3dv(glSecondaryColor3dv_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3dv)(glSecondaryColor3dv_ARG_EXPAND);
void glSecondaryColor3f(glSecondaryColor3f_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3f)(glSecondaryColor3f_ARG_EXPAND);
void glSecondaryColor3fv(glSecondaryColor3fv_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3fv)(glSecondaryColor3fv_ARG_EXPAND);
void glSecondaryColor3i(glSecondaryColor3i_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3i)(glSecondaryColor3i_ARG_EXPAND);
void glSecondaryColor3iv(glSecondaryColor3iv_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3iv)(glSecondaryColor3iv_ARG_EXPAND);
void glSecondaryColor3s(glSecondaryColor3s_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3s)(glSecondaryColor3s_ARG_EXPAND);
void glSecondaryColor3sv(glSecondaryColor3sv_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3sv)(glSecondaryColor3sv_ARG_EXPAND);
void glSecondaryColor3ub(glSecondaryColor3ub_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3ub)(glSecondaryColor3ub_ARG_EXPAND);
void glSecondaryColor3ubv(glSecondaryColor3ubv_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3ubv)(glSecondaryColor3ubv_ARG_EXPAND);
void glSecondaryColor3ui(glSecondaryColor3ui_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3ui)(glSecondaryColor3ui_ARG_EXPAND);
void glSecondaryColor3uiv(glSecondaryColor3uiv_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3uiv)(glSecondaryColor3uiv_ARG_EXPAND);
void glSecondaryColor3us(glSecondaryColor3us_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3us)(glSecondaryColor3us_ARG_EXPAND);
void glSecondaryColor3usv(glSecondaryColor3usv_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColor3usv)(glSecondaryColor3usv_ARG_EXPAND);
void glSecondaryColorPointer(glSecondaryColorPointer_ARG_EXPAND);
typedef void (*glesptr_glSecondaryColorPointer)(glSecondaryColorPointer_ARG_EXPAND);
void glSelectBuffer(glSelectBuffer_ARG_EXPAND);
typedef void (*glesptr_glSelectBuffer)(glSelectBuffer_ARG_EXPAND);
void glSeparableFilter2D(glSeparableFilter2D_ARG_EXPAND);
typedef void (*glesptr_glSeparableFilter2D)(glSeparableFilter2D_ARG_EXPAND);
void glShadeModel(glShadeModel_ARG_EXPAND);
typedef void (*glesptr_glShadeModel)(glShadeModel_ARG_EXPAND);
void glStencilFunc(glStencilFunc_ARG_EXPAND);
typedef void (*glesptr_glStencilFunc)(glStencilFunc_ARG_EXPAND);
void glStencilMask(glStencilMask_ARG_EXPAND);
typedef void (*glesptr_glStencilMask)(glStencilMask_ARG_EXPAND);
void glStencilOp(glStencilOp_ARG_EXPAND);
typedef void (*glesptr_glStencilOp)(glStencilOp_ARG_EXPAND);
void glTexCoord1d(glTexCoord1d_ARG_EXPAND);
typedef void (*glesptr_glTexCoord1d)(glTexCoord1d_ARG_EXPAND);
void glTexCoord1dv(glTexCoord1dv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord1dv)(glTexCoord1dv_ARG_EXPAND);
void glTexCoord1f(glTexCoord1f_ARG_EXPAND);
typedef void (*glesptr_glTexCoord1f)(glTexCoord1f_ARG_EXPAND);
void glTexCoord1fv(glTexCoord1fv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord1fv)(glTexCoord1fv_ARG_EXPAND);
void glTexCoord1i(glTexCoord1i_ARG_EXPAND);
typedef void (*glesptr_glTexCoord1i)(glTexCoord1i_ARG_EXPAND);
void glTexCoord1iv(glTexCoord1iv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord1iv)(glTexCoord1iv_ARG_EXPAND);
void glTexCoord1s(glTexCoord1s_ARG_EXPAND);
typedef void (*glesptr_glTexCoord1s)(glTexCoord1s_ARG_EXPAND);
void glTexCoord1sv(glTexCoord1sv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord1sv)(glTexCoord1sv_ARG_EXPAND);
void glTexCoord2d(glTexCoord2d_ARG_EXPAND);
typedef void (*glesptr_glTexCoord2d)(glTexCoord2d_ARG_EXPAND);
void glTexCoord2dv(glTexCoord2dv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord2dv)(glTexCoord2dv_ARG_EXPAND);
void glTexCoord2f(glTexCoord2f_ARG_EXPAND);
typedef void (*glesptr_glTexCoord2f)(glTexCoord2f_ARG_EXPAND);
void glTexCoord2fv(glTexCoord2fv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord2fv)(glTexCoord2fv_ARG_EXPAND);
void glTexCoord2i(glTexCoord2i_ARG_EXPAND);
typedef void (*glesptr_glTexCoord2i)(glTexCoord2i_ARG_EXPAND);
void glTexCoord2iv(glTexCoord2iv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord2iv)(glTexCoord2iv_ARG_EXPAND);
void glTexCoord2s(glTexCoord2s_ARG_EXPAND);
typedef void (*glesptr_glTexCoord2s)(glTexCoord2s_ARG_EXPAND);
void glTexCoord2sv(glTexCoord2sv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord2sv)(glTexCoord2sv_ARG_EXPAND);
void glTexCoord3d(glTexCoord3d_ARG_EXPAND);
typedef void (*glesptr_glTexCoord3d)(glTexCoord3d_ARG_EXPAND);
void glTexCoord3dv(glTexCoord3dv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord3dv)(glTexCoord3dv_ARG_EXPAND);
void glTexCoord3f(glTexCoord3f_ARG_EXPAND);
typedef void (*glesptr_glTexCoord3f)(glTexCoord3f_ARG_EXPAND);
void glTexCoord3fv(glTexCoord3fv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord3fv)(glTexCoord3fv_ARG_EXPAND);
void glTexCoord3i(glTexCoord3i_ARG_EXPAND);
typedef void (*glesptr_glTexCoord3i)(glTexCoord3i_ARG_EXPAND);
void glTexCoord3iv(glTexCoord3iv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord3iv)(glTexCoord3iv_ARG_EXPAND);
void glTexCoord3s(glTexCoord3s_ARG_EXPAND);
typedef void (*glesptr_glTexCoord3s)(glTexCoord3s_ARG_EXPAND);
void glTexCoord3sv(glTexCoord3sv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord3sv)(glTexCoord3sv_ARG_EXPAND);
void glTexCoord4d(glTexCoord4d_ARG_EXPAND);
typedef void (*glesptr_glTexCoord4d)(glTexCoord4d_ARG_EXPAND);
void glTexCoord4dv(glTexCoord4dv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord4dv)(glTexCoord4dv_ARG_EXPAND);
void glTexCoord4f(glTexCoord4f_ARG_EXPAND);
typedef void (*glesptr_glTexCoord4f)(glTexCoord4f_ARG_EXPAND);
void glTexCoord4fv(glTexCoord4fv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord4fv)(glTexCoord4fv_ARG_EXPAND);
void glTexCoord4i(glTexCoord4i_ARG_EXPAND);
typedef void (*glesptr_glTexCoord4i)(glTexCoord4i_ARG_EXPAND);
void glTexCoord4iv(glTexCoord4iv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord4iv)(glTexCoord4iv_ARG_EXPAND);
void glTexCoord4s(glTexCoord4s_ARG_EXPAND);
typedef void (*glesptr_glTexCoord4s)(glTexCoord4s_ARG_EXPAND);
void glTexCoord4sv(glTexCoord4sv_ARG_EXPAND);
typedef void (*glesptr_glTexCoord4sv)(glTexCoord4sv_ARG_EXPAND);
void glTexCoordPointer(glTexCoordPointer_ARG_EXPAND);
typedef void (*glesptr_glTexCoordPointer)(glTexCoordPointer_ARG_EXPAND);
void glTexEnvf(glTexEnvf_ARG_EXPAND);
typedef void (*glesptr_glTexEnvf)(glTexEnvf_ARG_EXPAND);
void glTexEnvfv(glTexEnvfv_ARG_EXPAND);
typedef void (*glesptr_glTexEnvfv)(glTexEnvfv_ARG_EXPAND);
void glTexEnvi(glTexEnvi_ARG_EXPAND);
typedef void (*glesptr_glTexEnvi)(glTexEnvi_ARG_EXPAND);
void glTexEnviv(glTexEnviv_ARG_EXPAND);
typedef void (*glesptr_glTexEnviv)(glTexEnviv_ARG_EXPAND);
void glTexGend(glTexGend_ARG_EXPAND);
typedef void (*glesptr_glTexGend)(glTexGend_ARG_EXPAND);
void glTexGendv(glTexGendv_ARG_EXPAND);
typedef void (*glesptr_glTexGendv)(glTexGendv_ARG_EXPAND);
void glTexGenf(glTexGenf_ARG_EXPAND);
typedef void (*glesptr_glTexGenf)(glTexGenf_ARG_EXPAND);
void glTexGenfv(glTexGenfv_ARG_EXPAND);
typedef void (*glesptr_glTexGenfv)(glTexGenfv_ARG_EXPAND);
void glTexGeni(glTexGeni_ARG_EXPAND);
typedef void (*glesptr_glTexGeni)(glTexGeni_ARG_EXPAND);
void glTexGeniv(glTexGeniv_ARG_EXPAND);
typedef void (*glesptr_glTexGeniv)(glTexGeniv_ARG_EXPAND);
void glTexImage1D(glTexImage1D_ARG_EXPAND);
typedef void (*glesptr_glTexImage1D)(glTexImage1D_ARG_EXPAND);
void glTexImage2D(glTexImage2D_ARG_EXPAND);
typedef void (*glesptr_glTexImage2D)(glTexImage2D_ARG_EXPAND);
void glTexImage3D(glTexImage3D_ARG_EXPAND);
typedef void (*glesptr_glTexImage3D)(glTexImage3D_ARG_EXPAND);
void glTexParameterf(glTexParameterf_ARG_EXPAND);
typedef void (*glesptr_glTexParameterf)(glTexParameterf_ARG_EXPAND);
void glTexParameterfv(glTexParameterfv_ARG_EXPAND);
typedef void (*glesptr_glTexParameterfv)(glTexParameterfv_ARG_EXPAND);
void glTexParameteri(glTexParameteri_ARG_EXPAND);
typedef void (*glesptr_glTexParameteri)(glTexParameteri_ARG_EXPAND);
void glTexParameteriv(glTexParameteriv_ARG_EXPAND);
typedef void (*glesptr_glTexParameteriv)(glTexParameteriv_ARG_EXPAND);
void glTexSubImage1D(glTexSubImage1D_ARG_EXPAND);
typedef void (*glesptr_glTexSubImage1D)(glTexSubImage1D_ARG_EXPAND);
void glTexSubImage2D(glTexSubImage2D_ARG_EXPAND);
typedef void (*glesptr_glTexSubImage2D)(glTexSubImage2D_ARG_EXPAND);
void glTexSubImage3D(glTexSubImage3D_ARG_EXPAND);
typedef void (*glesptr_glTexSubImage3D)(glTexSubImage3D_ARG_EXPAND);
void glTranslated(glTranslated_ARG_EXPAND);
typedef void (*glesptr_glTranslated)(glTranslated_ARG_EXPAND);
void glTranslatef(glTranslatef_ARG_EXPAND);
typedef void (*glesptr_glTranslatef)(glTranslatef_ARG_EXPAND);
GLboolean glUnmapBuffer(glUnmapBuffer_ARG_EXPAND);
typedef GLboolean (*glesptr_glUnmapBuffer)(glUnmapBuffer_ARG_EXPAND);
void glVertex2d(glVertex2d_ARG_EXPAND);
typedef void (*glesptr_glVertex2d)(glVertex2d_ARG_EXPAND);
void glVertex2dv(glVertex2dv_ARG_EXPAND);
typedef void (*glesptr_glVertex2dv)(glVertex2dv_ARG_EXPAND);
void glVertex2f(glVertex2f_ARG_EXPAND);
typedef void (*glesptr_glVertex2f)(glVertex2f_ARG_EXPAND);
void glVertex2fv(glVertex2fv_ARG_EXPAND);
typedef void (*glesptr_glVertex2fv)(glVertex2fv_ARG_EXPAND);
void glVertex2i(glVertex2i_ARG_EXPAND);
typedef void (*glesptr_glVertex2i)(glVertex2i_ARG_EXPAND);
void glVertex2iv(glVertex2iv_ARG_EXPAND);
typedef void (*glesptr_glVertex2iv)(glVertex2iv_ARG_EXPAND);
void glVertex2s(glVertex2s_ARG_EXPAND);
typedef void (*glesptr_glVertex2s)(glVertex2s_ARG_EXPAND);
void glVertex2sv(glVertex2sv_ARG_EXPAND);
typedef void (*glesptr_glVertex2sv)(glVertex2sv_ARG_EXPAND);
void glVertex3d(glVertex3d_ARG_EXPAND);
typedef void (*glesptr_glVertex3d)(glVertex3d_ARG_EXPAND);
void glVertex3dv(glVertex3dv_ARG_EXPAND);
typedef void (*glesptr_glVertex3dv)(glVertex3dv_ARG_EXPAND);
void glVertex3f(glVertex3f_ARG_EXPAND);
typedef void (*glesptr_glVertex3f)(glVertex3f_ARG_EXPAND);
void glVertex3fv(glVertex3fv_ARG_EXPAND);
typedef void (*glesptr_glVertex3fv)(glVertex3fv_ARG_EXPAND);
void glVertex3i(glVertex3i_ARG_EXPAND);
typedef void (*glesptr_glVertex3i)(glVertex3i_ARG_EXPAND);
void glVertex3iv(glVertex3iv_ARG_EXPAND);
typedef void (*glesptr_glVertex3iv)(glVertex3iv_ARG_EXPAND);
void glVertex3s(glVertex3s_ARG_EXPAND);
typedef void (*glesptr_glVertex3s)(glVertex3s_ARG_EXPAND);
void glVertex3sv(glVertex3sv_ARG_EXPAND);
typedef void (*glesptr_glVertex3sv)(glVertex3sv_ARG_EXPAND);
void glVertex4d(glVertex4d_ARG_EXPAND);
typedef void (*glesptr_glVertex4d)(glVertex4d_ARG_EXPAND);
void glVertex4dv(glVertex4dv_ARG_EXPAND);
typedef void (*glesptr_glVertex4dv)(glVertex4dv_ARG_EXPAND);
void glVertex4f(glVertex4f_ARG_EXPAND);
typedef void (*glesptr_glVertex4f)(glVertex4f_ARG_EXPAND);
void glVertex4fv(glVertex4fv_ARG_EXPAND);
typedef void (*glesptr_glVertex4fv)(glVertex4fv_ARG_EXPAND);
void glVertex4i(glVertex4i_ARG_EXPAND);
typedef void (*glesptr_glVertex4i)(glVertex4i_ARG_EXPAND);
void glVertex4iv(glVertex4iv_ARG_EXPAND);
typedef void (*glesptr_glVertex4iv)(glVertex4iv_ARG_EXPAND);
void glVertex4s(glVertex4s_ARG_EXPAND);
typedef void (*glesptr_glVertex4s)(glVertex4s_ARG_EXPAND);
void glVertex4sv(glVertex4sv_ARG_EXPAND);
typedef void (*glesptr_glVertex4sv)(glVertex4sv_ARG_EXPAND);
void glVertexPointer(glVertexPointer_ARG_EXPAND);
typedef void (*glesptr_glVertexPointer)(glVertexPointer_ARG_EXPAND);
void glViewport(glViewport_ARG_EXPAND);
typedef void (*glesptr_glViewport)(glViewport_ARG_EXPAND);
void glWindowPos2d(glWindowPos2d_ARG_EXPAND);
typedef void (*glesptr_glWindowPos2d)(glWindowPos2d_ARG_EXPAND);
void glWindowPos2dv(glWindowPos2dv_ARG_EXPAND);
typedef void (*glesptr_glWindowPos2dv)(glWindowPos2dv_ARG_EXPAND);
void glWindowPos2f(glWindowPos2f_ARG_EXPAND);
typedef void (*glesptr_glWindowPos2f)(glWindowPos2f_ARG_EXPAND);
void glWindowPos2fv(glWindowPos2fv_ARG_EXPAND);
typedef void (*glesptr_glWindowPos2fv)(glWindowPos2fv_ARG_EXPAND);
void glWindowPos2i(glWindowPos2i_ARG_EXPAND);
typedef void (*glesptr_glWindowPos2i)(glWindowPos2i_ARG_EXPAND);
void glWindowPos2iv(glWindowPos2iv_ARG_EXPAND);
typedef void (*glesptr_glWindowPos2iv)(glWindowPos2iv_ARG_EXPAND);
void glWindowPos2s(glWindowPos2s_ARG_EXPAND);
typedef void (*glesptr_glWindowPos2s)(glWindowPos2s_ARG_EXPAND);
void glWindowPos2sv(glWindowPos2sv_ARG_EXPAND);
typedef void (*glesptr_glWindowPos2sv)(glWindowPos2sv_ARG_EXPAND);
void glWindowPos3d(glWindowPos3d_ARG_EXPAND);
typedef void (*glesptr_glWindowPos3d)(glWindowPos3d_ARG_EXPAND);
void glWindowPos3dv(glWindowPos3dv_ARG_EXPAND);
typedef void (*glesptr_glWindowPos3dv)(glWindowPos3dv_ARG_EXPAND);
void glWindowPos3f(glWindowPos3f_ARG_EXPAND);
typedef void (*glesptr_glWindowPos3f)(glWindowPos3f_ARG_EXPAND);
void glWindowPos3fv(glWindowPos3fv_ARG_EXPAND);
typedef void (*glesptr_glWindowPos3fv)(glWindowPos3fv_ARG_EXPAND);
void glWindowPos3i(glWindowPos3i_ARG_EXPAND);
typedef void (*glesptr_glWindowPos3i)(glWindowPos3i_ARG_EXPAND);
void glWindowPos3iv(glWindowPos3iv_ARG_EXPAND);
typedef void (*glesptr_glWindowPos3iv)(glWindowPos3iv_ARG_EXPAND);
void glWindowPos3s(glWindowPos3s_ARG_EXPAND);
typedef void (*glesptr_glWindowPos3s)(glWindowPos3s_ARG_EXPAND);
void glWindowPos3sv(glWindowPos3sv_ARG_EXPAND);
typedef void (*glesptr_glWindowPos3sv)(glWindowPos3sv_ARG_EXPAND);
int glXBindHyperpipeSGIX(glXBindHyperpipeSGIX_ARG_EXPAND);
typedef int (*glesptr_glXBindHyperpipeSGIX)(glXBindHyperpipeSGIX_ARG_EXPAND);
void glXBindSwapBarrierSGIX(glXBindSwapBarrierSGIX_ARG_EXPAND);
typedef void (*glesptr_glXBindSwapBarrierSGIX)(glXBindSwapBarrierSGIX_ARG_EXPAND);
void glXChangeDrawableAttributes(glXChangeDrawableAttributes_ARG_EXPAND);
typedef void (*glesptr_glXChangeDrawableAttributes)(glXChangeDrawableAttributes_ARG_EXPAND);
void glXChangeDrawableAttributesSGIX(glXChangeDrawableAttributesSGIX_ARG_EXPAND);
typedef void (*glesptr_glXChangeDrawableAttributesSGIX)(glXChangeDrawableAttributesSGIX_ARG_EXPAND);
GLXFBConfig * glXChooseFBConfig(glXChooseFBConfig_ARG_EXPAND);
typedef GLXFBConfig * (*glesptr_glXChooseFBConfig)(glXChooseFBConfig_ARG_EXPAND);
XVisualInfo * glXChooseVisual(glXChooseVisual_ARG_EXPAND);
typedef XVisualInfo * (*glesptr_glXChooseVisual)(glXChooseVisual_ARG_EXPAND);
void glXClientInfo(glXClientInfo_ARG_EXPAND);
typedef void (*glesptr_glXClientInfo)(glXClientInfo_ARG_EXPAND);
void glXCopyContext(glXCopyContext_ARG_EXPAND);
typedef void (*glesptr_glXCopyContext)(glXCopyContext_ARG_EXPAND);
GLXContext glXCreateContext(glXCreateContext_ARG_EXPAND);
typedef GLXContext (*glesptr_glXCreateContext)(glXCreateContext_ARG_EXPAND);
void glXCreateContextWithConfigSGIX(glXCreateContextWithConfigSGIX_ARG_EXPAND);
typedef void (*glesptr_glXCreateContextWithConfigSGIX)(glXCreateContextWithConfigSGIX_ARG_EXPAND);
void glXCreateGLXPbufferSGIX(glXCreateGLXPbufferSGIX_ARG_EXPAND);
typedef void (*glesptr_glXCreateGLXPbufferSGIX)(glXCreateGLXPbufferSGIX_ARG_EXPAND);
GLXPixmap glXCreateGLXPixmap(glXCreateGLXPixmap_ARG_EXPAND);
typedef GLXPixmap (*glesptr_glXCreateGLXPixmap)(glXCreateGLXPixmap_ARG_EXPAND);
void glXCreateGLXPixmapWithConfigSGIX(glXCreateGLXPixmapWithConfigSGIX_ARG_EXPAND);
typedef void (*glesptr_glXCreateGLXPixmapWithConfigSGIX)(glXCreateGLXPixmapWithConfigSGIX_ARG_EXPAND);
void glXCreateGLXVideoSourceSGIX(glXCreateGLXVideoSourceSGIX_ARG_EXPAND);
typedef void (*glesptr_glXCreateGLXVideoSourceSGIX)(glXCreateGLXVideoSourceSGIX_ARG_EXPAND);
GLXContext glXCreateNewContext(glXCreateNewContext_ARG_EXPAND);
typedef GLXContext (*glesptr_glXCreateNewContext)(glXCreateNewContext_ARG_EXPAND);
GLXPbuffer glXCreatePbuffer(glXCreatePbuffer_ARG_EXPAND);
typedef GLXPbuffer (*glesptr_glXCreatePbuffer)(glXCreatePbuffer_ARG_EXPAND);
GLXPixmap glXCreatePixmap(glXCreatePixmap_ARG_EXPAND);
typedef GLXPixmap (*glesptr_glXCreatePixmap)(glXCreatePixmap_ARG_EXPAND);
GLXWindow glXCreateWindow(glXCreateWindow_ARG_EXPAND);
typedef GLXWindow (*glesptr_glXCreateWindow)(glXCreateWindow_ARG_EXPAND);
void glXDestroyContext(glXDestroyContext_ARG_EXPAND);
typedef void (*glesptr_glXDestroyContext)(glXDestroyContext_ARG_EXPAND);
void glXDestroyGLXPbufferSGIX(glXDestroyGLXPbufferSGIX_ARG_EXPAND);
typedef void (*glesptr_glXDestroyGLXPbufferSGIX)(glXDestroyGLXPbufferSGIX_ARG_EXPAND);
void glXDestroyGLXPixmap(glXDestroyGLXPixmap_ARG_EXPAND);
typedef void (*glesptr_glXDestroyGLXPixmap)(glXDestroyGLXPixmap_ARG_EXPAND);
void glXDestroyGLXVideoSourceSGIX(glXDestroyGLXVideoSourceSGIX_ARG_EXPAND);
typedef void (*glesptr_glXDestroyGLXVideoSourceSGIX)(glXDestroyGLXVideoSourceSGIX_ARG_EXPAND);
int glXDestroyHyperpipeConfigSGIX(glXDestroyHyperpipeConfigSGIX_ARG_EXPAND);
typedef int (*glesptr_glXDestroyHyperpipeConfigSGIX)(glXDestroyHyperpipeConfigSGIX_ARG_EXPAND);
void glXDestroyPbuffer(glXDestroyPbuffer_ARG_EXPAND);
typedef void (*glesptr_glXDestroyPbuffer)(glXDestroyPbuffer_ARG_EXPAND);
void glXDestroyPixmap(glXDestroyPixmap_ARG_EXPAND);
typedef void (*glesptr_glXDestroyPixmap)(glXDestroyPixmap_ARG_EXPAND);
void glXDestroyWindow(glXDestroyWindow_ARG_EXPAND);
typedef void (*glesptr_glXDestroyWindow)(glXDestroyWindow_ARG_EXPAND);
Display * glXGetCurrentDisplay(glXGetCurrentDisplay_ARG_EXPAND);
typedef Display * (*glesptr_glXGetCurrentDisplay)(glXGetCurrentDisplay_ARG_EXPAND);
GLXDrawable glXGetCurrentReadDrawable(glXGetCurrentReadDrawable_ARG_EXPAND);
typedef GLXDrawable (*glesptr_glXGetCurrentReadDrawable)(glXGetCurrentReadDrawable_ARG_EXPAND);
void glXGetDrawableAttributes(glXGetDrawableAttributes_ARG_EXPAND);
typedef void (*glesptr_glXGetDrawableAttributes)(glXGetDrawableAttributes_ARG_EXPAND);
void glXGetDrawableAttributesSGIX(glXGetDrawableAttributesSGIX_ARG_EXPAND);
typedef void (*glesptr_glXGetDrawableAttributesSGIX)(glXGetDrawableAttributesSGIX_ARG_EXPAND);
int glXGetFBConfigAttrib(glXGetFBConfigAttrib_ARG_EXPAND);
typedef int (*glesptr_glXGetFBConfigAttrib)(glXGetFBConfigAttrib_ARG_EXPAND);
GLXFBConfig * glXGetFBConfigs(glXGetFBConfigs_ARG_EXPAND);
typedef GLXFBConfig * (*glesptr_glXGetFBConfigs)(glXGetFBConfigs_ARG_EXPAND);
void glXGetFBConfigsSGIX(glXGetFBConfigsSGIX_ARG_EXPAND);
typedef void (*glesptr_glXGetFBConfigsSGIX)(glXGetFBConfigsSGIX_ARG_EXPAND);
__GLXextFuncPtr glXGetProcAddress(glXGetProcAddress_ARG_EXPAND);
typedef __GLXextFuncPtr (*glesptr_glXGetProcAddress)(glXGetProcAddress_ARG_EXPAND);
void glXGetSelectedEvent(glXGetSelectedEvent_ARG_EXPAND);
typedef void (*glesptr_glXGetSelectedEvent)(glXGetSelectedEvent_ARG_EXPAND);
void glXGetVisualConfigs(glXGetVisualConfigs_ARG_EXPAND);
typedef void (*glesptr_glXGetVisualConfigs)(glXGetVisualConfigs_ARG_EXPAND);
XVisualInfo * glXGetVisualFromFBConfig(glXGetVisualFromFBConfig_ARG_EXPAND);
typedef XVisualInfo * (*glesptr_glXGetVisualFromFBConfig)(glXGetVisualFromFBConfig_ARG_EXPAND);
int glXHyperpipeAttribSGIX(glXHyperpipeAttribSGIX_ARG_EXPAND);
typedef int (*glesptr_glXHyperpipeAttribSGIX)(glXHyperpipeAttribSGIX_ARG_EXPAND);
int glXHyperpipeConfigSGIX(glXHyperpipeConfigSGIX_ARG_EXPAND);
typedef int (*glesptr_glXHyperpipeConfigSGIX)(glXHyperpipeConfigSGIX_ARG_EXPAND);
Bool glXIsDirect(glXIsDirect_ARG_EXPAND);
typedef Bool (*glesptr_glXIsDirect)(glXIsDirect_ARG_EXPAND);
void glXJoinSwapGroupSGIX(glXJoinSwapGroupSGIX_ARG_EXPAND);
typedef void (*glesptr_glXJoinSwapGroupSGIX)(glXJoinSwapGroupSGIX_ARG_EXPAND);
Bool glXMakeContextCurrent(glXMakeContextCurrent_ARG_EXPAND);
typedef Bool (*glesptr_glXMakeContextCurrent)(glXMakeContextCurrent_ARG_EXPAND);
Bool glXMakeCurrent(glXMakeCurrent_ARG_EXPAND);
typedef Bool (*glesptr_glXMakeCurrent)(glXMakeCurrent_ARG_EXPAND);
void glXMakeCurrentReadSGI(glXMakeCurrentReadSGI_ARG_EXPAND);
typedef void (*glesptr_glXMakeCurrentReadSGI)(glXMakeCurrentReadSGI_ARG_EXPAND);
int glXQueryContext(glXQueryContext_ARG_EXPAND);
typedef int (*glesptr_glXQueryContext)(glXQueryContext_ARG_EXPAND);
void glXQueryContextInfoEXT(glXQueryContextInfoEXT_ARG_EXPAND);
typedef void (*glesptr_glXQueryContextInfoEXT)(glXQueryContextInfoEXT_ARG_EXPAND);
void glXQueryDrawable(glXQueryDrawable_ARG_EXPAND);
typedef void (*glesptr_glXQueryDrawable)(glXQueryDrawable_ARG_EXPAND);
const char * glXQueryExtensionsString(glXQueryExtensionsString_ARG_EXPAND);
typedef const char * (*glesptr_glXQueryExtensionsString)(glXQueryExtensionsString_ARG_EXPAND);
int glXQueryHyperpipeAttribSGIX(glXQueryHyperpipeAttribSGIX_ARG_EXPAND);
typedef int (*glesptr_glXQueryHyperpipeAttribSGIX)(glXQueryHyperpipeAttribSGIX_ARG_EXPAND);
int glXQueryHyperpipeBestAttribSGIX(glXQueryHyperpipeBestAttribSGIX_ARG_EXPAND);
typedef int (*glesptr_glXQueryHyperpipeBestAttribSGIX)(glXQueryHyperpipeBestAttribSGIX_ARG_EXPAND);
GLXHyperpipeConfigSGIX * glXQueryHyperpipeConfigSGIX(glXQueryHyperpipeConfigSGIX_ARG_EXPAND);
typedef GLXHyperpipeConfigSGIX * (*glesptr_glXQueryHyperpipeConfigSGIX)(glXQueryHyperpipeConfigSGIX_ARG_EXPAND);
GLXHyperpipeNetworkSGIX * glXQueryHyperpipeNetworkSGIX(glXQueryHyperpipeNetworkSGIX_ARG_EXPAND);
typedef GLXHyperpipeNetworkSGIX * (*glesptr_glXQueryHyperpipeNetworkSGIX)(glXQueryHyperpipeNetworkSGIX_ARG_EXPAND);
void glXQueryMaxSwapBarriersSGIX(glXQueryMaxSwapBarriersSGIX_ARG_EXPAND);
typedef void (*glesptr_glXQueryMaxSwapBarriersSGIX)(glXQueryMaxSwapBarriersSGIX_ARG_EXPAND);
const char * glXQueryServerString(glXQueryServerString_ARG_EXPAND);
typedef const char * (*glesptr_glXQueryServerString)(glXQueryServerString_ARG_EXPAND);
Bool glXQueryVersion(glXQueryVersion_ARG_EXPAND);
typedef Bool (*glesptr_glXQueryVersion)(glXQueryVersion_ARG_EXPAND);
void glXRender(glXRender_ARG_EXPAND);
typedef void (*glesptr_glXRender)(glXRender_ARG_EXPAND);
void glXRenderLarge(glXRenderLarge_ARG_EXPAND);
typedef void (*glesptr_glXRenderLarge)(glXRenderLarge_ARG_EXPAND);
void glXSelectEvent(glXSelectEvent_ARG_EXPAND);
typedef void (*glesptr_glXSelectEvent)(glXSelectEvent_ARG_EXPAND);
void glXSwapBuffers(glXSwapBuffers_ARG_EXPAND);
typedef void (*glesptr_glXSwapBuffers)(glXSwapBuffers_ARG_EXPAND);
void glXSwapIntervalSGI(glXSwapIntervalSGI_ARG_EXPAND);
typedef void (*glesptr_glXSwapIntervalSGI)(glXSwapIntervalSGI_ARG_EXPAND);
void glXUseXFont(glXUseXFont_ARG_EXPAND);
typedef void (*glesptr_glXUseXFont)(glXUseXFont_ARG_EXPAND);
void glXVendorPrivate(glXVendorPrivate_ARG_EXPAND);
typedef void (*glesptr_glXVendorPrivate)(glXVendorPrivate_ARG_EXPAND);
void glXVendorPrivateWithReply(glXVendorPrivateWithReply_ARG_EXPAND);
typedef void (*glesptr_glXVendorPrivateWithReply)(glXVendorPrivateWithReply_ARG_EXPAND);
void glXWaitGL(glXWaitGL_ARG_EXPAND);
typedef void (*glesptr_glXWaitGL)(glXWaitGL_ARG_EXPAND);
void glXWaitX(glXWaitX_ARG_EXPAND);
typedef void (*glesptr_glXWaitX)(glXWaitX_ARG_EXPAND);

#ifndef direct_glAccum
static inline void push_glAccum(glAccum_ARG_EXPAND) {
    glAccum_PACKED *packed_data = malloc(sizeof(glAccum_PACKED));
    packed_data->format = glAccum_FORMAT;
    packed_data->func = glAccum;
    packed_data->args.a1 = op;
    packed_data->args.a2 = value;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glActiveTexture
static inline void push_glActiveTexture(glActiveTexture_ARG_EXPAND) {
    glActiveTexture_PACKED *packed_data = malloc(sizeof(glActiveTexture_PACKED));
    packed_data->format = glActiveTexture_FORMAT;
    packed_data->func = glActiveTexture;
    packed_data->args.a1 = texture;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glAlphaFunc
static inline void push_glAlphaFunc(glAlphaFunc_ARG_EXPAND) {
    glAlphaFunc_PACKED *packed_data = malloc(sizeof(glAlphaFunc_PACKED));
    packed_data->format = glAlphaFunc_FORMAT;
    packed_data->func = glAlphaFunc;
    packed_data->args.a1 = func;
    packed_data->args.a2 = ref;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glAreTexturesResident
static inline void push_glAreTexturesResident(glAreTexturesResident_ARG_EXPAND) {
    glAreTexturesResident_PACKED *packed_data = malloc(sizeof(glAreTexturesResident_PACKED));
    packed_data->format = glAreTexturesResident_FORMAT;
    packed_data->func = glAreTexturesResident;
    packed_data->args.a1 = n;
    packed_data->args.a2 = textures;
    packed_data->args.a3 = residences;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glArrayElement
static inline void push_glArrayElement(glArrayElement_ARG_EXPAND) {
    glArrayElement_PACKED *packed_data = malloc(sizeof(glArrayElement_PACKED));
    packed_data->format = glArrayElement_FORMAT;
    packed_data->func = glArrayElement;
    packed_data->args.a1 = i;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glBegin
static inline void push_glBegin(glBegin_ARG_EXPAND) {
    glBegin_PACKED *packed_data = malloc(sizeof(glBegin_PACKED));
    packed_data->format = glBegin_FORMAT;
    packed_data->func = glBegin;
    packed_data->args.a1 = mode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glBeginQuery
static inline void push_glBeginQuery(glBeginQuery_ARG_EXPAND) {
    glBeginQuery_PACKED *packed_data = malloc(sizeof(glBeginQuery_PACKED));
    packed_data->format = glBeginQuery_FORMAT;
    packed_data->func = glBeginQuery;
    packed_data->args.a1 = target;
    packed_data->args.a2 = id;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glBindBuffer
static inline void push_glBindBuffer(glBindBuffer_ARG_EXPAND) {
    glBindBuffer_PACKED *packed_data = malloc(sizeof(glBindBuffer_PACKED));
    packed_data->format = glBindBuffer_FORMAT;
    packed_data->func = glBindBuffer;
    packed_data->args.a1 = target;
    packed_data->args.a2 = buffer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glBindTexture
static inline void push_glBindTexture(glBindTexture_ARG_EXPAND) {
    glBindTexture_PACKED *packed_data = malloc(sizeof(glBindTexture_PACKED));
    packed_data->format = glBindTexture_FORMAT;
    packed_data->func = glBindTexture;
    packed_data->args.a1 = target;
    packed_data->args.a2 = texture;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glBitmap
static inline void push_glBitmap(glBitmap_ARG_EXPAND) {
    glBitmap_PACKED *packed_data = malloc(sizeof(glBitmap_PACKED));
    packed_data->format = glBitmap_FORMAT;
    packed_data->func = glBitmap;
    packed_data->args.a1 = width;
    packed_data->args.a2 = height;
    packed_data->args.a3 = xorig;
    packed_data->args.a4 = yorig;
    packed_data->args.a5 = xmove;
    packed_data->args.a6 = ymove;
    packed_data->args.a7 = bitmap;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glBlendColor
static inline void push_glBlendColor(glBlendColor_ARG_EXPAND) {
    glBlendColor_PACKED *packed_data = malloc(sizeof(glBlendColor_PACKED));
    packed_data->format = glBlendColor_FORMAT;
    packed_data->func = glBlendColor;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    packed_data->args.a4 = alpha;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glBlendEquation
static inline void push_glBlendEquation(glBlendEquation_ARG_EXPAND) {
    glBlendEquation_PACKED *packed_data = malloc(sizeof(glBlendEquation_PACKED));
    packed_data->format = glBlendEquation_FORMAT;
    packed_data->func = glBlendEquation;
    packed_data->args.a1 = mode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glBlendFunc
static inline void push_glBlendFunc(glBlendFunc_ARG_EXPAND) {
    glBlendFunc_PACKED *packed_data = malloc(sizeof(glBlendFunc_PACKED));
    packed_data->format = glBlendFunc_FORMAT;
    packed_data->func = glBlendFunc;
    packed_data->args.a1 = sfactor;
    packed_data->args.a2 = dfactor;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glBlendFuncSeparate
static inline void push_glBlendFuncSeparate(glBlendFuncSeparate_ARG_EXPAND) {
    glBlendFuncSeparate_PACKED *packed_data = malloc(sizeof(glBlendFuncSeparate_PACKED));
    packed_data->format = glBlendFuncSeparate_FORMAT;
    packed_data->func = glBlendFuncSeparate;
    packed_data->args.a1 = sfactorRGB;
    packed_data->args.a2 = dfactorRGB;
    packed_data->args.a3 = sfactorAlpha;
    packed_data->args.a4 = dfactorAlpha;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glBufferData
static inline void push_glBufferData(glBufferData_ARG_EXPAND) {
    glBufferData_PACKED *packed_data = malloc(sizeof(glBufferData_PACKED));
    packed_data->format = glBufferData_FORMAT;
    packed_data->func = glBufferData;
    packed_data->args.a1 = target;
    packed_data->args.a2 = size;
    packed_data->args.a3 = data;
    packed_data->args.a4 = usage;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glBufferSubData
static inline void push_glBufferSubData(glBufferSubData_ARG_EXPAND) {
    glBufferSubData_PACKED *packed_data = malloc(sizeof(glBufferSubData_PACKED));
    packed_data->format = glBufferSubData_FORMAT;
    packed_data->func = glBufferSubData;
    packed_data->args.a1 = target;
    packed_data->args.a2 = offset;
    packed_data->args.a3 = size;
    packed_data->args.a4 = data;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCallList
static inline void push_glCallList(glCallList_ARG_EXPAND) {
    glCallList_PACKED *packed_data = malloc(sizeof(glCallList_PACKED));
    packed_data->format = glCallList_FORMAT;
    packed_data->func = glCallList;
    packed_data->args.a1 = list;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCallLists
static inline void push_glCallLists(glCallLists_ARG_EXPAND) {
    glCallLists_PACKED *packed_data = malloc(sizeof(glCallLists_PACKED));
    packed_data->format = glCallLists_FORMAT;
    packed_data->func = glCallLists;
    packed_data->args.a1 = n;
    packed_data->args.a2 = type;
    packed_data->args.a3 = lists;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glClear
static inline void push_glClear(glClear_ARG_EXPAND) {
    glClear_PACKED *packed_data = malloc(sizeof(glClear_PACKED));
    packed_data->format = glClear_FORMAT;
    packed_data->func = glClear;
    packed_data->args.a1 = mask;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glClearAccum
static inline void push_glClearAccum(glClearAccum_ARG_EXPAND) {
    glClearAccum_PACKED *packed_data = malloc(sizeof(glClearAccum_PACKED));
    packed_data->format = glClearAccum_FORMAT;
    packed_data->func = glClearAccum;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    packed_data->args.a4 = alpha;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glClearColor
static inline void push_glClearColor(glClearColor_ARG_EXPAND) {
    glClearColor_PACKED *packed_data = malloc(sizeof(glClearColor_PACKED));
    packed_data->format = glClearColor_FORMAT;
    packed_data->func = glClearColor;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    packed_data->args.a4 = alpha;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glClearDepth
static inline void push_glClearDepth(glClearDepth_ARG_EXPAND) {
    glClearDepth_PACKED *packed_data = malloc(sizeof(glClearDepth_PACKED));
    packed_data->format = glClearDepth_FORMAT;
    packed_data->func = glClearDepth;
    packed_data->args.a1 = depth;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glClearIndex
static inline void push_glClearIndex(glClearIndex_ARG_EXPAND) {
    glClearIndex_PACKED *packed_data = malloc(sizeof(glClearIndex_PACKED));
    packed_data->format = glClearIndex_FORMAT;
    packed_data->func = glClearIndex;
    packed_data->args.a1 = c;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glClearStencil
static inline void push_glClearStencil(glClearStencil_ARG_EXPAND) {
    glClearStencil_PACKED *packed_data = malloc(sizeof(glClearStencil_PACKED));
    packed_data->format = glClearStencil_FORMAT;
    packed_data->func = glClearStencil;
    packed_data->args.a1 = s;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glClientActiveTexture
static inline void push_glClientActiveTexture(glClientActiveTexture_ARG_EXPAND) {
    glClientActiveTexture_PACKED *packed_data = malloc(sizeof(glClientActiveTexture_PACKED));
    packed_data->format = glClientActiveTexture_FORMAT;
    packed_data->func = glClientActiveTexture;
    packed_data->args.a1 = texture;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glClipPlane
static inline void push_glClipPlane(glClipPlane_ARG_EXPAND) {
    glClipPlane_PACKED *packed_data = malloc(sizeof(glClipPlane_PACKED));
    packed_data->format = glClipPlane_FORMAT;
    packed_data->func = glClipPlane;
    packed_data->args.a1 = plane;
    packed_data->args.a2 = equation;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3b
static inline void push_glColor3b(glColor3b_ARG_EXPAND) {
    glColor3b_PACKED *packed_data = malloc(sizeof(glColor3b_PACKED));
    packed_data->format = glColor3b_FORMAT;
    packed_data->func = glColor3b;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3bv
static inline void push_glColor3bv(glColor3bv_ARG_EXPAND) {
    glColor3bv_PACKED *packed_data = malloc(sizeof(glColor3bv_PACKED));
    packed_data->format = glColor3bv_FORMAT;
    packed_data->func = glColor3bv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3d
static inline void push_glColor3d(glColor3d_ARG_EXPAND) {
    glColor3d_PACKED *packed_data = malloc(sizeof(glColor3d_PACKED));
    packed_data->format = glColor3d_FORMAT;
    packed_data->func = glColor3d;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3dv
static inline void push_glColor3dv(glColor3dv_ARG_EXPAND) {
    glColor3dv_PACKED *packed_data = malloc(sizeof(glColor3dv_PACKED));
    packed_data->format = glColor3dv_FORMAT;
    packed_data->func = glColor3dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3f
static inline void push_glColor3f(glColor3f_ARG_EXPAND) {
    glColor3f_PACKED *packed_data = malloc(sizeof(glColor3f_PACKED));
    packed_data->format = glColor3f_FORMAT;
    packed_data->func = glColor3f;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3fv
static inline void push_glColor3fv(glColor3fv_ARG_EXPAND) {
    glColor3fv_PACKED *packed_data = malloc(sizeof(glColor3fv_PACKED));
    packed_data->format = glColor3fv_FORMAT;
    packed_data->func = glColor3fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3i
static inline void push_glColor3i(glColor3i_ARG_EXPAND) {
    glColor3i_PACKED *packed_data = malloc(sizeof(glColor3i_PACKED));
    packed_data->format = glColor3i_FORMAT;
    packed_data->func = glColor3i;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3iv
static inline void push_glColor3iv(glColor3iv_ARG_EXPAND) {
    glColor3iv_PACKED *packed_data = malloc(sizeof(glColor3iv_PACKED));
    packed_data->format = glColor3iv_FORMAT;
    packed_data->func = glColor3iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3s
static inline void push_glColor3s(glColor3s_ARG_EXPAND) {
    glColor3s_PACKED *packed_data = malloc(sizeof(glColor3s_PACKED));
    packed_data->format = glColor3s_FORMAT;
    packed_data->func = glColor3s;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3sv
static inline void push_glColor3sv(glColor3sv_ARG_EXPAND) {
    glColor3sv_PACKED *packed_data = malloc(sizeof(glColor3sv_PACKED));
    packed_data->format = glColor3sv_FORMAT;
    packed_data->func = glColor3sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3ub
static inline void push_glColor3ub(glColor3ub_ARG_EXPAND) {
    glColor3ub_PACKED *packed_data = malloc(sizeof(glColor3ub_PACKED));
    packed_data->format = glColor3ub_FORMAT;
    packed_data->func = glColor3ub;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3ubv
static inline void push_glColor3ubv(glColor3ubv_ARG_EXPAND) {
    glColor3ubv_PACKED *packed_data = malloc(sizeof(glColor3ubv_PACKED));
    packed_data->format = glColor3ubv_FORMAT;
    packed_data->func = glColor3ubv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3ui
static inline void push_glColor3ui(glColor3ui_ARG_EXPAND) {
    glColor3ui_PACKED *packed_data = malloc(sizeof(glColor3ui_PACKED));
    packed_data->format = glColor3ui_FORMAT;
    packed_data->func = glColor3ui;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3uiv
static inline void push_glColor3uiv(glColor3uiv_ARG_EXPAND) {
    glColor3uiv_PACKED *packed_data = malloc(sizeof(glColor3uiv_PACKED));
    packed_data->format = glColor3uiv_FORMAT;
    packed_data->func = glColor3uiv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3us
static inline void push_glColor3us(glColor3us_ARG_EXPAND) {
    glColor3us_PACKED *packed_data = malloc(sizeof(glColor3us_PACKED));
    packed_data->format = glColor3us_FORMAT;
    packed_data->func = glColor3us;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor3usv
static inline void push_glColor3usv(glColor3usv_ARG_EXPAND) {
    glColor3usv_PACKED *packed_data = malloc(sizeof(glColor3usv_PACKED));
    packed_data->format = glColor3usv_FORMAT;
    packed_data->func = glColor3usv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4b
static inline void push_glColor4b(glColor4b_ARG_EXPAND) {
    glColor4b_PACKED *packed_data = malloc(sizeof(glColor4b_PACKED));
    packed_data->format = glColor4b_FORMAT;
    packed_data->func = glColor4b;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    packed_data->args.a4 = alpha;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4bv
static inline void push_glColor4bv(glColor4bv_ARG_EXPAND) {
    glColor4bv_PACKED *packed_data = malloc(sizeof(glColor4bv_PACKED));
    packed_data->format = glColor4bv_FORMAT;
    packed_data->func = glColor4bv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4d
static inline void push_glColor4d(glColor4d_ARG_EXPAND) {
    glColor4d_PACKED *packed_data = malloc(sizeof(glColor4d_PACKED));
    packed_data->format = glColor4d_FORMAT;
    packed_data->func = glColor4d;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    packed_data->args.a4 = alpha;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4dv
static inline void push_glColor4dv(glColor4dv_ARG_EXPAND) {
    glColor4dv_PACKED *packed_data = malloc(sizeof(glColor4dv_PACKED));
    packed_data->format = glColor4dv_FORMAT;
    packed_data->func = glColor4dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4f
static inline void push_glColor4f(glColor4f_ARG_EXPAND) {
    glColor4f_PACKED *packed_data = malloc(sizeof(glColor4f_PACKED));
    packed_data->format = glColor4f_FORMAT;
    packed_data->func = glColor4f;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    packed_data->args.a4 = alpha;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4fv
static inline void push_glColor4fv(glColor4fv_ARG_EXPAND) {
    glColor4fv_PACKED *packed_data = malloc(sizeof(glColor4fv_PACKED));
    packed_data->format = glColor4fv_FORMAT;
    packed_data->func = glColor4fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4i
static inline void push_glColor4i(glColor4i_ARG_EXPAND) {
    glColor4i_PACKED *packed_data = malloc(sizeof(glColor4i_PACKED));
    packed_data->format = glColor4i_FORMAT;
    packed_data->func = glColor4i;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    packed_data->args.a4 = alpha;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4iv
static inline void push_glColor4iv(glColor4iv_ARG_EXPAND) {
    glColor4iv_PACKED *packed_data = malloc(sizeof(glColor4iv_PACKED));
    packed_data->format = glColor4iv_FORMAT;
    packed_data->func = glColor4iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4s
static inline void push_glColor4s(glColor4s_ARG_EXPAND) {
    glColor4s_PACKED *packed_data = malloc(sizeof(glColor4s_PACKED));
    packed_data->format = glColor4s_FORMAT;
    packed_data->func = glColor4s;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    packed_data->args.a4 = alpha;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4sv
static inline void push_glColor4sv(glColor4sv_ARG_EXPAND) {
    glColor4sv_PACKED *packed_data = malloc(sizeof(glColor4sv_PACKED));
    packed_data->format = glColor4sv_FORMAT;
    packed_data->func = glColor4sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4ub
static inline void push_glColor4ub(glColor4ub_ARG_EXPAND) {
    glColor4ub_PACKED *packed_data = malloc(sizeof(glColor4ub_PACKED));
    packed_data->format = glColor4ub_FORMAT;
    packed_data->func = glColor4ub;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    packed_data->args.a4 = alpha;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4ubv
static inline void push_glColor4ubv(glColor4ubv_ARG_EXPAND) {
    glColor4ubv_PACKED *packed_data = malloc(sizeof(glColor4ubv_PACKED));
    packed_data->format = glColor4ubv_FORMAT;
    packed_data->func = glColor4ubv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4ui
static inline void push_glColor4ui(glColor4ui_ARG_EXPAND) {
    glColor4ui_PACKED *packed_data = malloc(sizeof(glColor4ui_PACKED));
    packed_data->format = glColor4ui_FORMAT;
    packed_data->func = glColor4ui;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    packed_data->args.a4 = alpha;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4uiv
static inline void push_glColor4uiv(glColor4uiv_ARG_EXPAND) {
    glColor4uiv_PACKED *packed_data = malloc(sizeof(glColor4uiv_PACKED));
    packed_data->format = glColor4uiv_FORMAT;
    packed_data->func = glColor4uiv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4us
static inline void push_glColor4us(glColor4us_ARG_EXPAND) {
    glColor4us_PACKED *packed_data = malloc(sizeof(glColor4us_PACKED));
    packed_data->format = glColor4us_FORMAT;
    packed_data->func = glColor4us;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    packed_data->args.a4 = alpha;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColor4usv
static inline void push_glColor4usv(glColor4usv_ARG_EXPAND) {
    glColor4usv_PACKED *packed_data = malloc(sizeof(glColor4usv_PACKED));
    packed_data->format = glColor4usv_FORMAT;
    packed_data->func = glColor4usv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColorMask
static inline void push_glColorMask(glColorMask_ARG_EXPAND) {
    glColorMask_PACKED *packed_data = malloc(sizeof(glColorMask_PACKED));
    packed_data->format = glColorMask_FORMAT;
    packed_data->func = glColorMask;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    packed_data->args.a4 = alpha;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColorMaterial
static inline void push_glColorMaterial(glColorMaterial_ARG_EXPAND) {
    glColorMaterial_PACKED *packed_data = malloc(sizeof(glColorMaterial_PACKED));
    packed_data->format = glColorMaterial_FORMAT;
    packed_data->func = glColorMaterial;
    packed_data->args.a1 = face;
    packed_data->args.a2 = mode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColorPointer
static inline void push_glColorPointer(glColorPointer_ARG_EXPAND) {
    glColorPointer_PACKED *packed_data = malloc(sizeof(glColorPointer_PACKED));
    packed_data->format = glColorPointer_FORMAT;
    packed_data->func = glColorPointer;
    packed_data->args.a1 = size;
    packed_data->args.a2 = type;
    packed_data->args.a3 = stride;
    packed_data->args.a4 = pointer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColorSubTable
static inline void push_glColorSubTable(glColorSubTable_ARG_EXPAND) {
    glColorSubTable_PACKED *packed_data = malloc(sizeof(glColorSubTable_PACKED));
    packed_data->format = glColorSubTable_FORMAT;
    packed_data->func = glColorSubTable;
    packed_data->args.a1 = target;
    packed_data->args.a2 = start;
    packed_data->args.a3 = count;
    packed_data->args.a4 = format;
    packed_data->args.a5 = type;
    packed_data->args.a6 = data;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColorTable
static inline void push_glColorTable(glColorTable_ARG_EXPAND) {
    glColorTable_PACKED *packed_data = malloc(sizeof(glColorTable_PACKED));
    packed_data->format = glColorTable_FORMAT;
    packed_data->func = glColorTable;
    packed_data->args.a1 = target;
    packed_data->args.a2 = internalformat;
    packed_data->args.a3 = width;
    packed_data->args.a4 = format;
    packed_data->args.a5 = type;
    packed_data->args.a6 = table;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColorTableParameterfv
static inline void push_glColorTableParameterfv(glColorTableParameterfv_ARG_EXPAND) {
    glColorTableParameterfv_PACKED *packed_data = malloc(sizeof(glColorTableParameterfv_PACKED));
    packed_data->format = glColorTableParameterfv_FORMAT;
    packed_data->func = glColorTableParameterfv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glColorTableParameteriv
static inline void push_glColorTableParameteriv(glColorTableParameteriv_ARG_EXPAND) {
    glColorTableParameteriv_PACKED *packed_data = malloc(sizeof(glColorTableParameteriv_PACKED));
    packed_data->format = glColorTableParameteriv_FORMAT;
    packed_data->func = glColorTableParameteriv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCompressedTexImage1D
static inline void push_glCompressedTexImage1D(glCompressedTexImage1D_ARG_EXPAND) {
    glCompressedTexImage1D_PACKED *packed_data = malloc(sizeof(glCompressedTexImage1D_PACKED));
    packed_data->format = glCompressedTexImage1D_FORMAT;
    packed_data->func = glCompressedTexImage1D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = internalformat;
    packed_data->args.a4 = width;
    packed_data->args.a5 = border;
    packed_data->args.a6 = imageSize;
    packed_data->args.a7 = data;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCompressedTexImage2D
static inline void push_glCompressedTexImage2D(glCompressedTexImage2D_ARG_EXPAND) {
    glCompressedTexImage2D_PACKED *packed_data = malloc(sizeof(glCompressedTexImage2D_PACKED));
    packed_data->format = glCompressedTexImage2D_FORMAT;
    packed_data->func = glCompressedTexImage2D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = internalformat;
    packed_data->args.a4 = width;
    packed_data->args.a5 = height;
    packed_data->args.a6 = border;
    packed_data->args.a7 = imageSize;
    packed_data->args.a8 = data;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCompressedTexImage3D
static inline void push_glCompressedTexImage3D(glCompressedTexImage3D_ARG_EXPAND) {
    glCompressedTexImage3D_PACKED *packed_data = malloc(sizeof(glCompressedTexImage3D_PACKED));
    packed_data->format = glCompressedTexImage3D_FORMAT;
    packed_data->func = glCompressedTexImage3D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = internalformat;
    packed_data->args.a4 = width;
    packed_data->args.a5 = height;
    packed_data->args.a6 = depth;
    packed_data->args.a7 = border;
    packed_data->args.a8 = imageSize;
    packed_data->args.a9 = data;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCompressedTexSubImage1D
static inline void push_glCompressedTexSubImage1D(glCompressedTexSubImage1D_ARG_EXPAND) {
    glCompressedTexSubImage1D_PACKED *packed_data = malloc(sizeof(glCompressedTexSubImage1D_PACKED));
    packed_data->format = glCompressedTexSubImage1D_FORMAT;
    packed_data->func = glCompressedTexSubImage1D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = xoffset;
    packed_data->args.a4 = width;
    packed_data->args.a5 = format;
    packed_data->args.a6 = imageSize;
    packed_data->args.a7 = data;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCompressedTexSubImage2D
static inline void push_glCompressedTexSubImage2D(glCompressedTexSubImage2D_ARG_EXPAND) {
    glCompressedTexSubImage2D_PACKED *packed_data = malloc(sizeof(glCompressedTexSubImage2D_PACKED));
    packed_data->format = glCompressedTexSubImage2D_FORMAT;
    packed_data->func = glCompressedTexSubImage2D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = xoffset;
    packed_data->args.a4 = yoffset;
    packed_data->args.a5 = width;
    packed_data->args.a6 = height;
    packed_data->args.a7 = format;
    packed_data->args.a8 = imageSize;
    packed_data->args.a9 = data;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCompressedTexSubImage3D
static inline void push_glCompressedTexSubImage3D(glCompressedTexSubImage3D_ARG_EXPAND) {
    glCompressedTexSubImage3D_PACKED *packed_data = malloc(sizeof(glCompressedTexSubImage3D_PACKED));
    packed_data->format = glCompressedTexSubImage3D_FORMAT;
    packed_data->func = glCompressedTexSubImage3D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = xoffset;
    packed_data->args.a4 = yoffset;
    packed_data->args.a5 = zoffset;
    packed_data->args.a6 = width;
    packed_data->args.a7 = height;
    packed_data->args.a8 = depth;
    packed_data->args.a9 = format;
    packed_data->args.a10 = imageSize;
    packed_data->args.a11 = data;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glConvolutionFilter1D
static inline void push_glConvolutionFilter1D(glConvolutionFilter1D_ARG_EXPAND) {
    glConvolutionFilter1D_PACKED *packed_data = malloc(sizeof(glConvolutionFilter1D_PACKED));
    packed_data->format = glConvolutionFilter1D_FORMAT;
    packed_data->func = glConvolutionFilter1D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = internalformat;
    packed_data->args.a3 = width;
    packed_data->args.a4 = format;
    packed_data->args.a5 = type;
    packed_data->args.a6 = image;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glConvolutionFilter2D
static inline void push_glConvolutionFilter2D(glConvolutionFilter2D_ARG_EXPAND) {
    glConvolutionFilter2D_PACKED *packed_data = malloc(sizeof(glConvolutionFilter2D_PACKED));
    packed_data->format = glConvolutionFilter2D_FORMAT;
    packed_data->func = glConvolutionFilter2D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = internalformat;
    packed_data->args.a3 = width;
    packed_data->args.a4 = height;
    packed_data->args.a5 = format;
    packed_data->args.a6 = type;
    packed_data->args.a7 = image;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glConvolutionParameterf
static inline void push_glConvolutionParameterf(glConvolutionParameterf_ARG_EXPAND) {
    glConvolutionParameterf_PACKED *packed_data = malloc(sizeof(glConvolutionParameterf_PACKED));
    packed_data->format = glConvolutionParameterf_FORMAT;
    packed_data->func = glConvolutionParameterf;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glConvolutionParameterfv
static inline void push_glConvolutionParameterfv(glConvolutionParameterfv_ARG_EXPAND) {
    glConvolutionParameterfv_PACKED *packed_data = malloc(sizeof(glConvolutionParameterfv_PACKED));
    packed_data->format = glConvolutionParameterfv_FORMAT;
    packed_data->func = glConvolutionParameterfv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glConvolutionParameteri
static inline void push_glConvolutionParameteri(glConvolutionParameteri_ARG_EXPAND) {
    glConvolutionParameteri_PACKED *packed_data = malloc(sizeof(glConvolutionParameteri_PACKED));
    packed_data->format = glConvolutionParameteri_FORMAT;
    packed_data->func = glConvolutionParameteri;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glConvolutionParameteriv
static inline void push_glConvolutionParameteriv(glConvolutionParameteriv_ARG_EXPAND) {
    glConvolutionParameteriv_PACKED *packed_data = malloc(sizeof(glConvolutionParameteriv_PACKED));
    packed_data->format = glConvolutionParameteriv_FORMAT;
    packed_data->func = glConvolutionParameteriv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCopyColorSubTable
static inline void push_glCopyColorSubTable(glCopyColorSubTable_ARG_EXPAND) {
    glCopyColorSubTable_PACKED *packed_data = malloc(sizeof(glCopyColorSubTable_PACKED));
    packed_data->format = glCopyColorSubTable_FORMAT;
    packed_data->func = glCopyColorSubTable;
    packed_data->args.a1 = target;
    packed_data->args.a2 = start;
    packed_data->args.a3 = x;
    packed_data->args.a4 = y;
    packed_data->args.a5 = width;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCopyColorTable
static inline void push_glCopyColorTable(glCopyColorTable_ARG_EXPAND) {
    glCopyColorTable_PACKED *packed_data = malloc(sizeof(glCopyColorTable_PACKED));
    packed_data->format = glCopyColorTable_FORMAT;
    packed_data->func = glCopyColorTable;
    packed_data->args.a1 = target;
    packed_data->args.a2 = internalformat;
    packed_data->args.a3 = x;
    packed_data->args.a4 = y;
    packed_data->args.a5 = width;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCopyConvolutionFilter1D
static inline void push_glCopyConvolutionFilter1D(glCopyConvolutionFilter1D_ARG_EXPAND) {
    glCopyConvolutionFilter1D_PACKED *packed_data = malloc(sizeof(glCopyConvolutionFilter1D_PACKED));
    packed_data->format = glCopyConvolutionFilter1D_FORMAT;
    packed_data->func = glCopyConvolutionFilter1D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = internalformat;
    packed_data->args.a3 = x;
    packed_data->args.a4 = y;
    packed_data->args.a5 = width;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCopyConvolutionFilter2D
static inline void push_glCopyConvolutionFilter2D(glCopyConvolutionFilter2D_ARG_EXPAND) {
    glCopyConvolutionFilter2D_PACKED *packed_data = malloc(sizeof(glCopyConvolutionFilter2D_PACKED));
    packed_data->format = glCopyConvolutionFilter2D_FORMAT;
    packed_data->func = glCopyConvolutionFilter2D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = internalformat;
    packed_data->args.a3 = x;
    packed_data->args.a4 = y;
    packed_data->args.a5 = width;
    packed_data->args.a6 = height;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCopyPixels
static inline void push_glCopyPixels(glCopyPixels_ARG_EXPAND) {
    glCopyPixels_PACKED *packed_data = malloc(sizeof(glCopyPixels_PACKED));
    packed_data->format = glCopyPixels_FORMAT;
    packed_data->func = glCopyPixels;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = width;
    packed_data->args.a4 = height;
    packed_data->args.a5 = type;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCopyTexImage1D
static inline void push_glCopyTexImage1D(glCopyTexImage1D_ARG_EXPAND) {
    glCopyTexImage1D_PACKED *packed_data = malloc(sizeof(glCopyTexImage1D_PACKED));
    packed_data->format = glCopyTexImage1D_FORMAT;
    packed_data->func = glCopyTexImage1D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = internalformat;
    packed_data->args.a4 = x;
    packed_data->args.a5 = y;
    packed_data->args.a6 = width;
    packed_data->args.a7 = border;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCopyTexImage2D
static inline void push_glCopyTexImage2D(glCopyTexImage2D_ARG_EXPAND) {
    glCopyTexImage2D_PACKED *packed_data = malloc(sizeof(glCopyTexImage2D_PACKED));
    packed_data->format = glCopyTexImage2D_FORMAT;
    packed_data->func = glCopyTexImage2D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = internalformat;
    packed_data->args.a4 = x;
    packed_data->args.a5 = y;
    packed_data->args.a6 = width;
    packed_data->args.a7 = height;
    packed_data->args.a8 = border;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCopyTexSubImage1D
static inline void push_glCopyTexSubImage1D(glCopyTexSubImage1D_ARG_EXPAND) {
    glCopyTexSubImage1D_PACKED *packed_data = malloc(sizeof(glCopyTexSubImage1D_PACKED));
    packed_data->format = glCopyTexSubImage1D_FORMAT;
    packed_data->func = glCopyTexSubImage1D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = xoffset;
    packed_data->args.a4 = x;
    packed_data->args.a5 = y;
    packed_data->args.a6 = width;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCopyTexSubImage2D
static inline void push_glCopyTexSubImage2D(glCopyTexSubImage2D_ARG_EXPAND) {
    glCopyTexSubImage2D_PACKED *packed_data = malloc(sizeof(glCopyTexSubImage2D_PACKED));
    packed_data->format = glCopyTexSubImage2D_FORMAT;
    packed_data->func = glCopyTexSubImage2D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = xoffset;
    packed_data->args.a4 = yoffset;
    packed_data->args.a5 = x;
    packed_data->args.a6 = y;
    packed_data->args.a7 = width;
    packed_data->args.a8 = height;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCopyTexSubImage3D
static inline void push_glCopyTexSubImage3D(glCopyTexSubImage3D_ARG_EXPAND) {
    glCopyTexSubImage3D_PACKED *packed_data = malloc(sizeof(glCopyTexSubImage3D_PACKED));
    packed_data->format = glCopyTexSubImage3D_FORMAT;
    packed_data->func = glCopyTexSubImage3D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = xoffset;
    packed_data->args.a4 = yoffset;
    packed_data->args.a5 = zoffset;
    packed_data->args.a6 = x;
    packed_data->args.a7 = y;
    packed_data->args.a8 = width;
    packed_data->args.a9 = height;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glCullFace
static inline void push_glCullFace(glCullFace_ARG_EXPAND) {
    glCullFace_PACKED *packed_data = malloc(sizeof(glCullFace_PACKED));
    packed_data->format = glCullFace_FORMAT;
    packed_data->func = glCullFace;
    packed_data->args.a1 = mode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDeleteBuffers
static inline void push_glDeleteBuffers(glDeleteBuffers_ARG_EXPAND) {
    glDeleteBuffers_PACKED *packed_data = malloc(sizeof(glDeleteBuffers_PACKED));
    packed_data->format = glDeleteBuffers_FORMAT;
    packed_data->func = glDeleteBuffers;
    packed_data->args.a1 = n;
    packed_data->args.a2 = buffers;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDeleteLists
static inline void push_glDeleteLists(glDeleteLists_ARG_EXPAND) {
    glDeleteLists_PACKED *packed_data = malloc(sizeof(glDeleteLists_PACKED));
    packed_data->format = glDeleteLists_FORMAT;
    packed_data->func = glDeleteLists;
    packed_data->args.a1 = list;
    packed_data->args.a2 = range;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDeleteQueries
static inline void push_glDeleteQueries(glDeleteQueries_ARG_EXPAND) {
    glDeleteQueries_PACKED *packed_data = malloc(sizeof(glDeleteQueries_PACKED));
    packed_data->format = glDeleteQueries_FORMAT;
    packed_data->func = glDeleteQueries;
    packed_data->args.a1 = n;
    packed_data->args.a2 = ids;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDeleteTextures
static inline void push_glDeleteTextures(glDeleteTextures_ARG_EXPAND) {
    glDeleteTextures_PACKED *packed_data = malloc(sizeof(glDeleteTextures_PACKED));
    packed_data->format = glDeleteTextures_FORMAT;
    packed_data->func = glDeleteTextures;
    packed_data->args.a1 = n;
    packed_data->args.a2 = textures;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDepthFunc
static inline void push_glDepthFunc(glDepthFunc_ARG_EXPAND) {
    glDepthFunc_PACKED *packed_data = malloc(sizeof(glDepthFunc_PACKED));
    packed_data->format = glDepthFunc_FORMAT;
    packed_data->func = glDepthFunc;
    packed_data->args.a1 = func;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDepthMask
static inline void push_glDepthMask(glDepthMask_ARG_EXPAND) {
    glDepthMask_PACKED *packed_data = malloc(sizeof(glDepthMask_PACKED));
    packed_data->format = glDepthMask_FORMAT;
    packed_data->func = glDepthMask;
    packed_data->args.a1 = flag;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDepthRange
static inline void push_glDepthRange(glDepthRange_ARG_EXPAND) {
    glDepthRange_PACKED *packed_data = malloc(sizeof(glDepthRange_PACKED));
    packed_data->format = glDepthRange_FORMAT;
    packed_data->func = glDepthRange;
    packed_data->args.a1 = near;
    packed_data->args.a2 = far;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDisable
static inline void push_glDisable(glDisable_ARG_EXPAND) {
    glDisable_PACKED *packed_data = malloc(sizeof(glDisable_PACKED));
    packed_data->format = glDisable_FORMAT;
    packed_data->func = glDisable;
    packed_data->args.a1 = cap;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDisableClientState
static inline void push_glDisableClientState(glDisableClientState_ARG_EXPAND) {
    glDisableClientState_PACKED *packed_data = malloc(sizeof(glDisableClientState_PACKED));
    packed_data->format = glDisableClientState_FORMAT;
    packed_data->func = glDisableClientState;
    packed_data->args.a1 = array;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDrawArrays
static inline void push_glDrawArrays(glDrawArrays_ARG_EXPAND) {
    glDrawArrays_PACKED *packed_data = malloc(sizeof(glDrawArrays_PACKED));
    packed_data->format = glDrawArrays_FORMAT;
    packed_data->func = glDrawArrays;
    packed_data->args.a1 = mode;
    packed_data->args.a2 = first;
    packed_data->args.a3 = count;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDrawBuffer
static inline void push_glDrawBuffer(glDrawBuffer_ARG_EXPAND) {
    glDrawBuffer_PACKED *packed_data = malloc(sizeof(glDrawBuffer_PACKED));
    packed_data->format = glDrawBuffer_FORMAT;
    packed_data->func = glDrawBuffer;
    packed_data->args.a1 = mode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDrawElements
static inline void push_glDrawElements(glDrawElements_ARG_EXPAND) {
    glDrawElements_PACKED *packed_data = malloc(sizeof(glDrawElements_PACKED));
    packed_data->format = glDrawElements_FORMAT;
    packed_data->func = glDrawElements;
    packed_data->args.a1 = mode;
    packed_data->args.a2 = count;
    packed_data->args.a3 = type;
    packed_data->args.a4 = indices;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDrawPixels
static inline void push_glDrawPixels(glDrawPixels_ARG_EXPAND) {
    glDrawPixels_PACKED *packed_data = malloc(sizeof(glDrawPixels_PACKED));
    packed_data->format = glDrawPixels_FORMAT;
    packed_data->func = glDrawPixels;
    packed_data->args.a1 = width;
    packed_data->args.a2 = height;
    packed_data->args.a3 = format;
    packed_data->args.a4 = type;
    packed_data->args.a5 = pixels;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glDrawRangeElements
static inline void push_glDrawRangeElements(glDrawRangeElements_ARG_EXPAND) {
    glDrawRangeElements_PACKED *packed_data = malloc(sizeof(glDrawRangeElements_PACKED));
    packed_data->format = glDrawRangeElements_FORMAT;
    packed_data->func = glDrawRangeElements;
    packed_data->args.a1 = mode;
    packed_data->args.a2 = start;
    packed_data->args.a3 = end;
    packed_data->args.a4 = count;
    packed_data->args.a5 = type;
    packed_data->args.a6 = indices;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEdgeFlag
static inline void push_glEdgeFlag(glEdgeFlag_ARG_EXPAND) {
    glEdgeFlag_PACKED *packed_data = malloc(sizeof(glEdgeFlag_PACKED));
    packed_data->format = glEdgeFlag_FORMAT;
    packed_data->func = glEdgeFlag;
    packed_data->args.a1 = flag;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEdgeFlagPointer
static inline void push_glEdgeFlagPointer(glEdgeFlagPointer_ARG_EXPAND) {
    glEdgeFlagPointer_PACKED *packed_data = malloc(sizeof(glEdgeFlagPointer_PACKED));
    packed_data->format = glEdgeFlagPointer_FORMAT;
    packed_data->func = glEdgeFlagPointer;
    packed_data->args.a1 = stride;
    packed_data->args.a2 = pointer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEdgeFlagv
static inline void push_glEdgeFlagv(glEdgeFlagv_ARG_EXPAND) {
    glEdgeFlagv_PACKED *packed_data = malloc(sizeof(glEdgeFlagv_PACKED));
    packed_data->format = glEdgeFlagv_FORMAT;
    packed_data->func = glEdgeFlagv;
    packed_data->args.a1 = flag;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEnable
static inline void push_glEnable(glEnable_ARG_EXPAND) {
    glEnable_PACKED *packed_data = malloc(sizeof(glEnable_PACKED));
    packed_data->format = glEnable_FORMAT;
    packed_data->func = glEnable;
    packed_data->args.a1 = cap;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEnableClientState
static inline void push_glEnableClientState(glEnableClientState_ARG_EXPAND) {
    glEnableClientState_PACKED *packed_data = malloc(sizeof(glEnableClientState_PACKED));
    packed_data->format = glEnableClientState_FORMAT;
    packed_data->func = glEnableClientState;
    packed_data->args.a1 = array;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEnd
static inline void push_glEnd(glEnd_ARG_EXPAND) {
    glEnd_PACKED *packed_data = malloc(sizeof(glEnd_PACKED));
    packed_data->format = glEnd_FORMAT;
    packed_data->func = glEnd;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEndList
static inline void push_glEndList(glEndList_ARG_EXPAND) {
    glEndList_PACKED *packed_data = malloc(sizeof(glEndList_PACKED));
    packed_data->format = glEndList_FORMAT;
    packed_data->func = glEndList;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEndQuery
static inline void push_glEndQuery(glEndQuery_ARG_EXPAND) {
    glEndQuery_PACKED *packed_data = malloc(sizeof(glEndQuery_PACKED));
    packed_data->format = glEndQuery_FORMAT;
    packed_data->func = glEndQuery;
    packed_data->args.a1 = target;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEvalCoord1d
static inline void push_glEvalCoord1d(glEvalCoord1d_ARG_EXPAND) {
    glEvalCoord1d_PACKED *packed_data = malloc(sizeof(glEvalCoord1d_PACKED));
    packed_data->format = glEvalCoord1d_FORMAT;
    packed_data->func = glEvalCoord1d;
    packed_data->args.a1 = u;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEvalCoord1dv
static inline void push_glEvalCoord1dv(glEvalCoord1dv_ARG_EXPAND) {
    glEvalCoord1dv_PACKED *packed_data = malloc(sizeof(glEvalCoord1dv_PACKED));
    packed_data->format = glEvalCoord1dv_FORMAT;
    packed_data->func = glEvalCoord1dv;
    packed_data->args.a1 = u;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEvalCoord1f
static inline void push_glEvalCoord1f(glEvalCoord1f_ARG_EXPAND) {
    glEvalCoord1f_PACKED *packed_data = malloc(sizeof(glEvalCoord1f_PACKED));
    packed_data->format = glEvalCoord1f_FORMAT;
    packed_data->func = glEvalCoord1f;
    packed_data->args.a1 = u;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEvalCoord1fv
static inline void push_glEvalCoord1fv(glEvalCoord1fv_ARG_EXPAND) {
    glEvalCoord1fv_PACKED *packed_data = malloc(sizeof(glEvalCoord1fv_PACKED));
    packed_data->format = glEvalCoord1fv_FORMAT;
    packed_data->func = glEvalCoord1fv;
    packed_data->args.a1 = u;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEvalCoord2d
static inline void push_glEvalCoord2d(glEvalCoord2d_ARG_EXPAND) {
    glEvalCoord2d_PACKED *packed_data = malloc(sizeof(glEvalCoord2d_PACKED));
    packed_data->format = glEvalCoord2d_FORMAT;
    packed_data->func = glEvalCoord2d;
    packed_data->args.a1 = u;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEvalCoord2dv
static inline void push_glEvalCoord2dv(glEvalCoord2dv_ARG_EXPAND) {
    glEvalCoord2dv_PACKED *packed_data = malloc(sizeof(glEvalCoord2dv_PACKED));
    packed_data->format = glEvalCoord2dv_FORMAT;
    packed_data->func = glEvalCoord2dv;
    packed_data->args.a1 = u;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEvalCoord2f
static inline void push_glEvalCoord2f(glEvalCoord2f_ARG_EXPAND) {
    glEvalCoord2f_PACKED *packed_data = malloc(sizeof(glEvalCoord2f_PACKED));
    packed_data->format = glEvalCoord2f_FORMAT;
    packed_data->func = glEvalCoord2f;
    packed_data->args.a1 = u;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEvalCoord2fv
static inline void push_glEvalCoord2fv(glEvalCoord2fv_ARG_EXPAND) {
    glEvalCoord2fv_PACKED *packed_data = malloc(sizeof(glEvalCoord2fv_PACKED));
    packed_data->format = glEvalCoord2fv_FORMAT;
    packed_data->func = glEvalCoord2fv;
    packed_data->args.a1 = u;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEvalMesh1
static inline void push_glEvalMesh1(glEvalMesh1_ARG_EXPAND) {
    glEvalMesh1_PACKED *packed_data = malloc(sizeof(glEvalMesh1_PACKED));
    packed_data->format = glEvalMesh1_FORMAT;
    packed_data->func = glEvalMesh1;
    packed_data->args.a1 = mode;
    packed_data->args.a2 = i1;
    packed_data->args.a3 = i2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEvalMesh2
static inline void push_glEvalMesh2(glEvalMesh2_ARG_EXPAND) {
    glEvalMesh2_PACKED *packed_data = malloc(sizeof(glEvalMesh2_PACKED));
    packed_data->format = glEvalMesh2_FORMAT;
    packed_data->func = glEvalMesh2;
    packed_data->args.a1 = mode;
    packed_data->args.a2 = i1;
    packed_data->args.a3 = i2;
    packed_data->args.a4 = j1;
    packed_data->args.a5 = j2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEvalPoint1
static inline void push_glEvalPoint1(glEvalPoint1_ARG_EXPAND) {
    glEvalPoint1_PACKED *packed_data = malloc(sizeof(glEvalPoint1_PACKED));
    packed_data->format = glEvalPoint1_FORMAT;
    packed_data->func = glEvalPoint1;
    packed_data->args.a1 = i;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glEvalPoint2
static inline void push_glEvalPoint2(glEvalPoint2_ARG_EXPAND) {
    glEvalPoint2_PACKED *packed_data = malloc(sizeof(glEvalPoint2_PACKED));
    packed_data->format = glEvalPoint2_FORMAT;
    packed_data->func = glEvalPoint2;
    packed_data->args.a1 = i;
    packed_data->args.a2 = j;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFeedbackBuffer
static inline void push_glFeedbackBuffer(glFeedbackBuffer_ARG_EXPAND) {
    glFeedbackBuffer_PACKED *packed_data = malloc(sizeof(glFeedbackBuffer_PACKED));
    packed_data->format = glFeedbackBuffer_FORMAT;
    packed_data->func = glFeedbackBuffer;
    packed_data->args.a1 = size;
    packed_data->args.a2 = type;
    packed_data->args.a3 = buffer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFinish
static inline void push_glFinish(glFinish_ARG_EXPAND) {
    glFinish_PACKED *packed_data = malloc(sizeof(glFinish_PACKED));
    packed_data->format = glFinish_FORMAT;
    packed_data->func = glFinish;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFlush
static inline void push_glFlush(glFlush_ARG_EXPAND) {
    glFlush_PACKED *packed_data = malloc(sizeof(glFlush_PACKED));
    packed_data->format = glFlush_FORMAT;
    packed_data->func = glFlush;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFogCoordPointer
static inline void push_glFogCoordPointer(glFogCoordPointer_ARG_EXPAND) {
    glFogCoordPointer_PACKED *packed_data = malloc(sizeof(glFogCoordPointer_PACKED));
    packed_data->format = glFogCoordPointer_FORMAT;
    packed_data->func = glFogCoordPointer;
    packed_data->args.a1 = type;
    packed_data->args.a2 = stride;
    packed_data->args.a3 = pointer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFogCoordd
static inline void push_glFogCoordd(glFogCoordd_ARG_EXPAND) {
    glFogCoordd_PACKED *packed_data = malloc(sizeof(glFogCoordd_PACKED));
    packed_data->format = glFogCoordd_FORMAT;
    packed_data->func = glFogCoordd;
    packed_data->args.a1 = coord;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFogCoorddv
static inline void push_glFogCoorddv(glFogCoorddv_ARG_EXPAND) {
    glFogCoorddv_PACKED *packed_data = malloc(sizeof(glFogCoorddv_PACKED));
    packed_data->format = glFogCoorddv_FORMAT;
    packed_data->func = glFogCoorddv;
    packed_data->args.a1 = coord;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFogCoordf
static inline void push_glFogCoordf(glFogCoordf_ARG_EXPAND) {
    glFogCoordf_PACKED *packed_data = malloc(sizeof(glFogCoordf_PACKED));
    packed_data->format = glFogCoordf_FORMAT;
    packed_data->func = glFogCoordf;
    packed_data->args.a1 = coord;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFogCoordfv
static inline void push_glFogCoordfv(glFogCoordfv_ARG_EXPAND) {
    glFogCoordfv_PACKED *packed_data = malloc(sizeof(glFogCoordfv_PACKED));
    packed_data->format = glFogCoordfv_FORMAT;
    packed_data->func = glFogCoordfv;
    packed_data->args.a1 = coord;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFogf
static inline void push_glFogf(glFogf_ARG_EXPAND) {
    glFogf_PACKED *packed_data = malloc(sizeof(glFogf_PACKED));
    packed_data->format = glFogf_FORMAT;
    packed_data->func = glFogf;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFogfv
static inline void push_glFogfv(glFogfv_ARG_EXPAND) {
    glFogfv_PACKED *packed_data = malloc(sizeof(glFogfv_PACKED));
    packed_data->format = glFogfv_FORMAT;
    packed_data->func = glFogfv;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFogi
static inline void push_glFogi(glFogi_ARG_EXPAND) {
    glFogi_PACKED *packed_data = malloc(sizeof(glFogi_PACKED));
    packed_data->format = glFogi_FORMAT;
    packed_data->func = glFogi;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFogiv
static inline void push_glFogiv(glFogiv_ARG_EXPAND) {
    glFogiv_PACKED *packed_data = malloc(sizeof(glFogiv_PACKED));
    packed_data->format = glFogiv_FORMAT;
    packed_data->func = glFogiv;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFrontFace
static inline void push_glFrontFace(glFrontFace_ARG_EXPAND) {
    glFrontFace_PACKED *packed_data = malloc(sizeof(glFrontFace_PACKED));
    packed_data->format = glFrontFace_FORMAT;
    packed_data->func = glFrontFace;
    packed_data->args.a1 = mode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glFrustum
static inline void push_glFrustum(glFrustum_ARG_EXPAND) {
    glFrustum_PACKED *packed_data = malloc(sizeof(glFrustum_PACKED));
    packed_data->format = glFrustum_FORMAT;
    packed_data->func = glFrustum;
    packed_data->args.a1 = left;
    packed_data->args.a2 = right;
    packed_data->args.a3 = bottom;
    packed_data->args.a4 = top;
    packed_data->args.a5 = zNear;
    packed_data->args.a6 = zFar;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGenBuffers
static inline void push_glGenBuffers(glGenBuffers_ARG_EXPAND) {
    glGenBuffers_PACKED *packed_data = malloc(sizeof(glGenBuffers_PACKED));
    packed_data->format = glGenBuffers_FORMAT;
    packed_data->func = glGenBuffers;
    packed_data->args.a1 = n;
    packed_data->args.a2 = buffers;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGenLists
static inline void push_glGenLists(glGenLists_ARG_EXPAND) {
    glGenLists_PACKED *packed_data = malloc(sizeof(glGenLists_PACKED));
    packed_data->format = glGenLists_FORMAT;
    packed_data->func = glGenLists;
    packed_data->args.a1 = range;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGenQueries
static inline void push_glGenQueries(glGenQueries_ARG_EXPAND) {
    glGenQueries_PACKED *packed_data = malloc(sizeof(glGenQueries_PACKED));
    packed_data->format = glGenQueries_FORMAT;
    packed_data->func = glGenQueries;
    packed_data->args.a1 = n;
    packed_data->args.a2 = ids;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGenTextures
static inline void push_glGenTextures(glGenTextures_ARG_EXPAND) {
    glGenTextures_PACKED *packed_data = malloc(sizeof(glGenTextures_PACKED));
    packed_data->format = glGenTextures_FORMAT;
    packed_data->func = glGenTextures;
    packed_data->args.a1 = n;
    packed_data->args.a2 = textures;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetBooleanv
static inline void push_glGetBooleanv(glGetBooleanv_ARG_EXPAND) {
    glGetBooleanv_PACKED *packed_data = malloc(sizeof(glGetBooleanv_PACKED));
    packed_data->format = glGetBooleanv_FORMAT;
    packed_data->func = glGetBooleanv;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetBufferParameteriv
static inline void push_glGetBufferParameteriv(glGetBufferParameteriv_ARG_EXPAND) {
    glGetBufferParameteriv_PACKED *packed_data = malloc(sizeof(glGetBufferParameteriv_PACKED));
    packed_data->format = glGetBufferParameteriv_FORMAT;
    packed_data->func = glGetBufferParameteriv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetBufferPointerv
static inline void push_glGetBufferPointerv(glGetBufferPointerv_ARG_EXPAND) {
    glGetBufferPointerv_PACKED *packed_data = malloc(sizeof(glGetBufferPointerv_PACKED));
    packed_data->format = glGetBufferPointerv_FORMAT;
    packed_data->func = glGetBufferPointerv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetBufferSubData
static inline void push_glGetBufferSubData(glGetBufferSubData_ARG_EXPAND) {
    glGetBufferSubData_PACKED *packed_data = malloc(sizeof(glGetBufferSubData_PACKED));
    packed_data->format = glGetBufferSubData_FORMAT;
    packed_data->func = glGetBufferSubData;
    packed_data->args.a1 = target;
    packed_data->args.a2 = offset;
    packed_data->args.a3 = size;
    packed_data->args.a4 = data;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetClipPlane
static inline void push_glGetClipPlane(glGetClipPlane_ARG_EXPAND) {
    glGetClipPlane_PACKED *packed_data = malloc(sizeof(glGetClipPlane_PACKED));
    packed_data->format = glGetClipPlane_FORMAT;
    packed_data->func = glGetClipPlane;
    packed_data->args.a1 = plane;
    packed_data->args.a2 = equation;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetColorTable
static inline void push_glGetColorTable(glGetColorTable_ARG_EXPAND) {
    glGetColorTable_PACKED *packed_data = malloc(sizeof(glGetColorTable_PACKED));
    packed_data->format = glGetColorTable_FORMAT;
    packed_data->func = glGetColorTable;
    packed_data->args.a1 = target;
    packed_data->args.a2 = format;
    packed_data->args.a3 = type;
    packed_data->args.a4 = table;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetColorTableParameterfv
static inline void push_glGetColorTableParameterfv(glGetColorTableParameterfv_ARG_EXPAND) {
    glGetColorTableParameterfv_PACKED *packed_data = malloc(sizeof(glGetColorTableParameterfv_PACKED));
    packed_data->format = glGetColorTableParameterfv_FORMAT;
    packed_data->func = glGetColorTableParameterfv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetColorTableParameteriv
static inline void push_glGetColorTableParameteriv(glGetColorTableParameteriv_ARG_EXPAND) {
    glGetColorTableParameteriv_PACKED *packed_data = malloc(sizeof(glGetColorTableParameteriv_PACKED));
    packed_data->format = glGetColorTableParameteriv_FORMAT;
    packed_data->func = glGetColorTableParameteriv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetCompressedTexImage
static inline void push_glGetCompressedTexImage(glGetCompressedTexImage_ARG_EXPAND) {
    glGetCompressedTexImage_PACKED *packed_data = malloc(sizeof(glGetCompressedTexImage_PACKED));
    packed_data->format = glGetCompressedTexImage_FORMAT;
    packed_data->func = glGetCompressedTexImage;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = img;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetConvolutionFilter
static inline void push_glGetConvolutionFilter(glGetConvolutionFilter_ARG_EXPAND) {
    glGetConvolutionFilter_PACKED *packed_data = malloc(sizeof(glGetConvolutionFilter_PACKED));
    packed_data->format = glGetConvolutionFilter_FORMAT;
    packed_data->func = glGetConvolutionFilter;
    packed_data->args.a1 = target;
    packed_data->args.a2 = format;
    packed_data->args.a3 = type;
    packed_data->args.a4 = image;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetConvolutionParameterfv
static inline void push_glGetConvolutionParameterfv(glGetConvolutionParameterfv_ARG_EXPAND) {
    glGetConvolutionParameterfv_PACKED *packed_data = malloc(sizeof(glGetConvolutionParameterfv_PACKED));
    packed_data->format = glGetConvolutionParameterfv_FORMAT;
    packed_data->func = glGetConvolutionParameterfv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetConvolutionParameteriv
static inline void push_glGetConvolutionParameteriv(glGetConvolutionParameteriv_ARG_EXPAND) {
    glGetConvolutionParameteriv_PACKED *packed_data = malloc(sizeof(glGetConvolutionParameteriv_PACKED));
    packed_data->format = glGetConvolutionParameteriv_FORMAT;
    packed_data->func = glGetConvolutionParameteriv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetDoublev
static inline void push_glGetDoublev(glGetDoublev_ARG_EXPAND) {
    glGetDoublev_PACKED *packed_data = malloc(sizeof(glGetDoublev_PACKED));
    packed_data->format = glGetDoublev_FORMAT;
    packed_data->func = glGetDoublev;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetError
static inline void push_glGetError(glGetError_ARG_EXPAND) {
    glGetError_PACKED *packed_data = malloc(sizeof(glGetError_PACKED));
    packed_data->format = glGetError_FORMAT;
    packed_data->func = glGetError;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetFloatv
static inline void push_glGetFloatv(glGetFloatv_ARG_EXPAND) {
    glGetFloatv_PACKED *packed_data = malloc(sizeof(glGetFloatv_PACKED));
    packed_data->format = glGetFloatv_FORMAT;
    packed_data->func = glGetFloatv;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetHistogram
static inline void push_glGetHistogram(glGetHistogram_ARG_EXPAND) {
    glGetHistogram_PACKED *packed_data = malloc(sizeof(glGetHistogram_PACKED));
    packed_data->format = glGetHistogram_FORMAT;
    packed_data->func = glGetHistogram;
    packed_data->args.a1 = target;
    packed_data->args.a2 = reset;
    packed_data->args.a3 = format;
    packed_data->args.a4 = type;
    packed_data->args.a5 = values;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetHistogramParameterfv
static inline void push_glGetHistogramParameterfv(glGetHistogramParameterfv_ARG_EXPAND) {
    glGetHistogramParameterfv_PACKED *packed_data = malloc(sizeof(glGetHistogramParameterfv_PACKED));
    packed_data->format = glGetHistogramParameterfv_FORMAT;
    packed_data->func = glGetHistogramParameterfv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetHistogramParameteriv
static inline void push_glGetHistogramParameteriv(glGetHistogramParameteriv_ARG_EXPAND) {
    glGetHistogramParameteriv_PACKED *packed_data = malloc(sizeof(glGetHistogramParameteriv_PACKED));
    packed_data->format = glGetHistogramParameteriv_FORMAT;
    packed_data->func = glGetHistogramParameteriv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetIntegerv
static inline void push_glGetIntegerv(glGetIntegerv_ARG_EXPAND) {
    glGetIntegerv_PACKED *packed_data = malloc(sizeof(glGetIntegerv_PACKED));
    packed_data->format = glGetIntegerv_FORMAT;
    packed_data->func = glGetIntegerv;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetLightfv
static inline void push_glGetLightfv(glGetLightfv_ARG_EXPAND) {
    glGetLightfv_PACKED *packed_data = malloc(sizeof(glGetLightfv_PACKED));
    packed_data->format = glGetLightfv_FORMAT;
    packed_data->func = glGetLightfv;
    packed_data->args.a1 = light;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetLightiv
static inline void push_glGetLightiv(glGetLightiv_ARG_EXPAND) {
    glGetLightiv_PACKED *packed_data = malloc(sizeof(glGetLightiv_PACKED));
    packed_data->format = glGetLightiv_FORMAT;
    packed_data->func = glGetLightiv;
    packed_data->args.a1 = light;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetMapdv
static inline void push_glGetMapdv(glGetMapdv_ARG_EXPAND) {
    glGetMapdv_PACKED *packed_data = malloc(sizeof(glGetMapdv_PACKED));
    packed_data->format = glGetMapdv_FORMAT;
    packed_data->func = glGetMapdv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = query;
    packed_data->args.a3 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetMapfv
static inline void push_glGetMapfv(glGetMapfv_ARG_EXPAND) {
    glGetMapfv_PACKED *packed_data = malloc(sizeof(glGetMapfv_PACKED));
    packed_data->format = glGetMapfv_FORMAT;
    packed_data->func = glGetMapfv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = query;
    packed_data->args.a3 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetMapiv
static inline void push_glGetMapiv(glGetMapiv_ARG_EXPAND) {
    glGetMapiv_PACKED *packed_data = malloc(sizeof(glGetMapiv_PACKED));
    packed_data->format = glGetMapiv_FORMAT;
    packed_data->func = glGetMapiv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = query;
    packed_data->args.a3 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetMaterialfv
static inline void push_glGetMaterialfv(glGetMaterialfv_ARG_EXPAND) {
    glGetMaterialfv_PACKED *packed_data = malloc(sizeof(glGetMaterialfv_PACKED));
    packed_data->format = glGetMaterialfv_FORMAT;
    packed_data->func = glGetMaterialfv;
    packed_data->args.a1 = face;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetMaterialiv
static inline void push_glGetMaterialiv(glGetMaterialiv_ARG_EXPAND) {
    glGetMaterialiv_PACKED *packed_data = malloc(sizeof(glGetMaterialiv_PACKED));
    packed_data->format = glGetMaterialiv_FORMAT;
    packed_data->func = glGetMaterialiv;
    packed_data->args.a1 = face;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetMinmax
static inline void push_glGetMinmax(glGetMinmax_ARG_EXPAND) {
    glGetMinmax_PACKED *packed_data = malloc(sizeof(glGetMinmax_PACKED));
    packed_data->format = glGetMinmax_FORMAT;
    packed_data->func = glGetMinmax;
    packed_data->args.a1 = target;
    packed_data->args.a2 = reset;
    packed_data->args.a3 = format;
    packed_data->args.a4 = type;
    packed_data->args.a5 = values;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetMinmaxParameterfv
static inline void push_glGetMinmaxParameterfv(glGetMinmaxParameterfv_ARG_EXPAND) {
    glGetMinmaxParameterfv_PACKED *packed_data = malloc(sizeof(glGetMinmaxParameterfv_PACKED));
    packed_data->format = glGetMinmaxParameterfv_FORMAT;
    packed_data->func = glGetMinmaxParameterfv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetMinmaxParameteriv
static inline void push_glGetMinmaxParameteriv(glGetMinmaxParameteriv_ARG_EXPAND) {
    glGetMinmaxParameteriv_PACKED *packed_data = malloc(sizeof(glGetMinmaxParameteriv_PACKED));
    packed_data->format = glGetMinmaxParameteriv_FORMAT;
    packed_data->func = glGetMinmaxParameteriv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetPixelMapfv
static inline void push_glGetPixelMapfv(glGetPixelMapfv_ARG_EXPAND) {
    glGetPixelMapfv_PACKED *packed_data = malloc(sizeof(glGetPixelMapfv_PACKED));
    packed_data->format = glGetPixelMapfv_FORMAT;
    packed_data->func = glGetPixelMapfv;
    packed_data->args.a1 = map;
    packed_data->args.a2 = values;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetPixelMapuiv
static inline void push_glGetPixelMapuiv(glGetPixelMapuiv_ARG_EXPAND) {
    glGetPixelMapuiv_PACKED *packed_data = malloc(sizeof(glGetPixelMapuiv_PACKED));
    packed_data->format = glGetPixelMapuiv_FORMAT;
    packed_data->func = glGetPixelMapuiv;
    packed_data->args.a1 = map;
    packed_data->args.a2 = values;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetPixelMapusv
static inline void push_glGetPixelMapusv(glGetPixelMapusv_ARG_EXPAND) {
    glGetPixelMapusv_PACKED *packed_data = malloc(sizeof(glGetPixelMapusv_PACKED));
    packed_data->format = glGetPixelMapusv_FORMAT;
    packed_data->func = glGetPixelMapusv;
    packed_data->args.a1 = map;
    packed_data->args.a2 = values;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetPointerv
static inline void push_glGetPointerv(glGetPointerv_ARG_EXPAND) {
    glGetPointerv_PACKED *packed_data = malloc(sizeof(glGetPointerv_PACKED));
    packed_data->format = glGetPointerv_FORMAT;
    packed_data->func = glGetPointerv;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetPolygonStipple
static inline void push_glGetPolygonStipple(glGetPolygonStipple_ARG_EXPAND) {
    glGetPolygonStipple_PACKED *packed_data = malloc(sizeof(glGetPolygonStipple_PACKED));
    packed_data->format = glGetPolygonStipple_FORMAT;
    packed_data->func = glGetPolygonStipple;
    packed_data->args.a1 = mask;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetQueryObjectiv
static inline void push_glGetQueryObjectiv(glGetQueryObjectiv_ARG_EXPAND) {
    glGetQueryObjectiv_PACKED *packed_data = malloc(sizeof(glGetQueryObjectiv_PACKED));
    packed_data->format = glGetQueryObjectiv_FORMAT;
    packed_data->func = glGetQueryObjectiv;
    packed_data->args.a1 = id;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetQueryObjectuiv
static inline void push_glGetQueryObjectuiv(glGetQueryObjectuiv_ARG_EXPAND) {
    glGetQueryObjectuiv_PACKED *packed_data = malloc(sizeof(glGetQueryObjectuiv_PACKED));
    packed_data->format = glGetQueryObjectuiv_FORMAT;
    packed_data->func = glGetQueryObjectuiv;
    packed_data->args.a1 = id;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetQueryiv
static inline void push_glGetQueryiv(glGetQueryiv_ARG_EXPAND) {
    glGetQueryiv_PACKED *packed_data = malloc(sizeof(glGetQueryiv_PACKED));
    packed_data->format = glGetQueryiv_FORMAT;
    packed_data->func = glGetQueryiv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetSeparableFilter
static inline void push_glGetSeparableFilter(glGetSeparableFilter_ARG_EXPAND) {
    glGetSeparableFilter_PACKED *packed_data = malloc(sizeof(glGetSeparableFilter_PACKED));
    packed_data->format = glGetSeparableFilter_FORMAT;
    packed_data->func = glGetSeparableFilter;
    packed_data->args.a1 = target;
    packed_data->args.a2 = format;
    packed_data->args.a3 = type;
    packed_data->args.a4 = row;
    packed_data->args.a5 = column;
    packed_data->args.a6 = span;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetString
static inline void push_glGetString(glGetString_ARG_EXPAND) {
    glGetString_PACKED *packed_data = malloc(sizeof(glGetString_PACKED));
    packed_data->format = glGetString_FORMAT;
    packed_data->func = glGetString;
    packed_data->args.a1 = name;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetTexEnvfv
static inline void push_glGetTexEnvfv(glGetTexEnvfv_ARG_EXPAND) {
    glGetTexEnvfv_PACKED *packed_data = malloc(sizeof(glGetTexEnvfv_PACKED));
    packed_data->format = glGetTexEnvfv_FORMAT;
    packed_data->func = glGetTexEnvfv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetTexEnviv
static inline void push_glGetTexEnviv(glGetTexEnviv_ARG_EXPAND) {
    glGetTexEnviv_PACKED *packed_data = malloc(sizeof(glGetTexEnviv_PACKED));
    packed_data->format = glGetTexEnviv_FORMAT;
    packed_data->func = glGetTexEnviv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetTexGendv
static inline void push_glGetTexGendv(glGetTexGendv_ARG_EXPAND) {
    glGetTexGendv_PACKED *packed_data = malloc(sizeof(glGetTexGendv_PACKED));
    packed_data->format = glGetTexGendv_FORMAT;
    packed_data->func = glGetTexGendv;
    packed_data->args.a1 = coord;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetTexGenfv
static inline void push_glGetTexGenfv(glGetTexGenfv_ARG_EXPAND) {
    glGetTexGenfv_PACKED *packed_data = malloc(sizeof(glGetTexGenfv_PACKED));
    packed_data->format = glGetTexGenfv_FORMAT;
    packed_data->func = glGetTexGenfv;
    packed_data->args.a1 = coord;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetTexGeniv
static inline void push_glGetTexGeniv(glGetTexGeniv_ARG_EXPAND) {
    glGetTexGeniv_PACKED *packed_data = malloc(sizeof(glGetTexGeniv_PACKED));
    packed_data->format = glGetTexGeniv_FORMAT;
    packed_data->func = glGetTexGeniv;
    packed_data->args.a1 = coord;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetTexImage
static inline void push_glGetTexImage(glGetTexImage_ARG_EXPAND) {
    glGetTexImage_PACKED *packed_data = malloc(sizeof(glGetTexImage_PACKED));
    packed_data->format = glGetTexImage_FORMAT;
    packed_data->func = glGetTexImage;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = format;
    packed_data->args.a4 = type;
    packed_data->args.a5 = pixels;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetTexLevelParameterfv
static inline void push_glGetTexLevelParameterfv(glGetTexLevelParameterfv_ARG_EXPAND) {
    glGetTexLevelParameterfv_PACKED *packed_data = malloc(sizeof(glGetTexLevelParameterfv_PACKED));
    packed_data->format = glGetTexLevelParameterfv_FORMAT;
    packed_data->func = glGetTexLevelParameterfv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = pname;
    packed_data->args.a4 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetTexLevelParameteriv
static inline void push_glGetTexLevelParameteriv(glGetTexLevelParameteriv_ARG_EXPAND) {
    glGetTexLevelParameteriv_PACKED *packed_data = malloc(sizeof(glGetTexLevelParameteriv_PACKED));
    packed_data->format = glGetTexLevelParameteriv_FORMAT;
    packed_data->func = glGetTexLevelParameteriv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = pname;
    packed_data->args.a4 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetTexParameterfv
static inline void push_glGetTexParameterfv(glGetTexParameterfv_ARG_EXPAND) {
    glGetTexParameterfv_PACKED *packed_data = malloc(sizeof(glGetTexParameterfv_PACKED));
    packed_data->format = glGetTexParameterfv_FORMAT;
    packed_data->func = glGetTexParameterfv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glGetTexParameteriv
static inline void push_glGetTexParameteriv(glGetTexParameteriv_ARG_EXPAND) {
    glGetTexParameteriv_PACKED *packed_data = malloc(sizeof(glGetTexParameteriv_PACKED));
    packed_data->format = glGetTexParameteriv_FORMAT;
    packed_data->func = glGetTexParameteriv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glHint
static inline void push_glHint(glHint_ARG_EXPAND) {
    glHint_PACKED *packed_data = malloc(sizeof(glHint_PACKED));
    packed_data->format = glHint_FORMAT;
    packed_data->func = glHint;
    packed_data->args.a1 = target;
    packed_data->args.a2 = mode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glHistogram
static inline void push_glHistogram(glHistogram_ARG_EXPAND) {
    glHistogram_PACKED *packed_data = malloc(sizeof(glHistogram_PACKED));
    packed_data->format = glHistogram_FORMAT;
    packed_data->func = glHistogram;
    packed_data->args.a1 = target;
    packed_data->args.a2 = width;
    packed_data->args.a3 = internalformat;
    packed_data->args.a4 = sink;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIndexMask
static inline void push_glIndexMask(glIndexMask_ARG_EXPAND) {
    glIndexMask_PACKED *packed_data = malloc(sizeof(glIndexMask_PACKED));
    packed_data->format = glIndexMask_FORMAT;
    packed_data->func = glIndexMask;
    packed_data->args.a1 = mask;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIndexPointer
static inline void push_glIndexPointer(glIndexPointer_ARG_EXPAND) {
    glIndexPointer_PACKED *packed_data = malloc(sizeof(glIndexPointer_PACKED));
    packed_data->format = glIndexPointer_FORMAT;
    packed_data->func = glIndexPointer;
    packed_data->args.a1 = type;
    packed_data->args.a2 = stride;
    packed_data->args.a3 = pointer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIndexd
static inline void push_glIndexd(glIndexd_ARG_EXPAND) {
    glIndexd_PACKED *packed_data = malloc(sizeof(glIndexd_PACKED));
    packed_data->format = glIndexd_FORMAT;
    packed_data->func = glIndexd;
    packed_data->args.a1 = c;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIndexdv
static inline void push_glIndexdv(glIndexdv_ARG_EXPAND) {
    glIndexdv_PACKED *packed_data = malloc(sizeof(glIndexdv_PACKED));
    packed_data->format = glIndexdv_FORMAT;
    packed_data->func = glIndexdv;
    packed_data->args.a1 = c;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIndexf
static inline void push_glIndexf(glIndexf_ARG_EXPAND) {
    glIndexf_PACKED *packed_data = malloc(sizeof(glIndexf_PACKED));
    packed_data->format = glIndexf_FORMAT;
    packed_data->func = glIndexf;
    packed_data->args.a1 = c;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIndexfv
static inline void push_glIndexfv(glIndexfv_ARG_EXPAND) {
    glIndexfv_PACKED *packed_data = malloc(sizeof(glIndexfv_PACKED));
    packed_data->format = glIndexfv_FORMAT;
    packed_data->func = glIndexfv;
    packed_data->args.a1 = c;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIndexi
static inline void push_glIndexi(glIndexi_ARG_EXPAND) {
    glIndexi_PACKED *packed_data = malloc(sizeof(glIndexi_PACKED));
    packed_data->format = glIndexi_FORMAT;
    packed_data->func = glIndexi;
    packed_data->args.a1 = c;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIndexiv
static inline void push_glIndexiv(glIndexiv_ARG_EXPAND) {
    glIndexiv_PACKED *packed_data = malloc(sizeof(glIndexiv_PACKED));
    packed_data->format = glIndexiv_FORMAT;
    packed_data->func = glIndexiv;
    packed_data->args.a1 = c;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIndexs
static inline void push_glIndexs(glIndexs_ARG_EXPAND) {
    glIndexs_PACKED *packed_data = malloc(sizeof(glIndexs_PACKED));
    packed_data->format = glIndexs_FORMAT;
    packed_data->func = glIndexs;
    packed_data->args.a1 = c;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIndexsv
static inline void push_glIndexsv(glIndexsv_ARG_EXPAND) {
    glIndexsv_PACKED *packed_data = malloc(sizeof(glIndexsv_PACKED));
    packed_data->format = glIndexsv_FORMAT;
    packed_data->func = glIndexsv;
    packed_data->args.a1 = c;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIndexub
static inline void push_glIndexub(glIndexub_ARG_EXPAND) {
    glIndexub_PACKED *packed_data = malloc(sizeof(glIndexub_PACKED));
    packed_data->format = glIndexub_FORMAT;
    packed_data->func = glIndexub;
    packed_data->args.a1 = c;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIndexubv
static inline void push_glIndexubv(glIndexubv_ARG_EXPAND) {
    glIndexubv_PACKED *packed_data = malloc(sizeof(glIndexubv_PACKED));
    packed_data->format = glIndexubv_FORMAT;
    packed_data->func = glIndexubv;
    packed_data->args.a1 = c;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glInitNames
static inline void push_glInitNames(glInitNames_ARG_EXPAND) {
    glInitNames_PACKED *packed_data = malloc(sizeof(glInitNames_PACKED));
    packed_data->format = glInitNames_FORMAT;
    packed_data->func = glInitNames;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glInterleavedArrays
static inline void push_glInterleavedArrays(glInterleavedArrays_ARG_EXPAND) {
    glInterleavedArrays_PACKED *packed_data = malloc(sizeof(glInterleavedArrays_PACKED));
    packed_data->format = glInterleavedArrays_FORMAT;
    packed_data->func = glInterleavedArrays;
    packed_data->args.a1 = format;
    packed_data->args.a2 = stride;
    packed_data->args.a3 = pointer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIsBuffer
static inline void push_glIsBuffer(glIsBuffer_ARG_EXPAND) {
    glIsBuffer_PACKED *packed_data = malloc(sizeof(glIsBuffer_PACKED));
    packed_data->format = glIsBuffer_FORMAT;
    packed_data->func = glIsBuffer;
    packed_data->args.a1 = buffer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIsEnabled
static inline void push_glIsEnabled(glIsEnabled_ARG_EXPAND) {
    glIsEnabled_PACKED *packed_data = malloc(sizeof(glIsEnabled_PACKED));
    packed_data->format = glIsEnabled_FORMAT;
    packed_data->func = glIsEnabled;
    packed_data->args.a1 = cap;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIsList
static inline void push_glIsList(glIsList_ARG_EXPAND) {
    glIsList_PACKED *packed_data = malloc(sizeof(glIsList_PACKED));
    packed_data->format = glIsList_FORMAT;
    packed_data->func = glIsList;
    packed_data->args.a1 = list;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIsQuery
static inline void push_glIsQuery(glIsQuery_ARG_EXPAND) {
    glIsQuery_PACKED *packed_data = malloc(sizeof(glIsQuery_PACKED));
    packed_data->format = glIsQuery_FORMAT;
    packed_data->func = glIsQuery;
    packed_data->args.a1 = id;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glIsTexture
static inline void push_glIsTexture(glIsTexture_ARG_EXPAND) {
    glIsTexture_PACKED *packed_data = malloc(sizeof(glIsTexture_PACKED));
    packed_data->format = glIsTexture_FORMAT;
    packed_data->func = glIsTexture;
    packed_data->args.a1 = texture;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLightModelf
static inline void push_glLightModelf(glLightModelf_ARG_EXPAND) {
    glLightModelf_PACKED *packed_data = malloc(sizeof(glLightModelf_PACKED));
    packed_data->format = glLightModelf_FORMAT;
    packed_data->func = glLightModelf;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLightModelfv
static inline void push_glLightModelfv(glLightModelfv_ARG_EXPAND) {
    glLightModelfv_PACKED *packed_data = malloc(sizeof(glLightModelfv_PACKED));
    packed_data->format = glLightModelfv_FORMAT;
    packed_data->func = glLightModelfv;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLightModeli
static inline void push_glLightModeli(glLightModeli_ARG_EXPAND) {
    glLightModeli_PACKED *packed_data = malloc(sizeof(glLightModeli_PACKED));
    packed_data->format = glLightModeli_FORMAT;
    packed_data->func = glLightModeli;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLightModeliv
static inline void push_glLightModeliv(glLightModeliv_ARG_EXPAND) {
    glLightModeliv_PACKED *packed_data = malloc(sizeof(glLightModeliv_PACKED));
    packed_data->format = glLightModeliv_FORMAT;
    packed_data->func = glLightModeliv;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLightf
static inline void push_glLightf(glLightf_ARG_EXPAND) {
    glLightf_PACKED *packed_data = malloc(sizeof(glLightf_PACKED));
    packed_data->format = glLightf_FORMAT;
    packed_data->func = glLightf;
    packed_data->args.a1 = light;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLightfv
static inline void push_glLightfv(glLightfv_ARG_EXPAND) {
    glLightfv_PACKED *packed_data = malloc(sizeof(glLightfv_PACKED));
    packed_data->format = glLightfv_FORMAT;
    packed_data->func = glLightfv;
    packed_data->args.a1 = light;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLighti
static inline void push_glLighti(glLighti_ARG_EXPAND) {
    glLighti_PACKED *packed_data = malloc(sizeof(glLighti_PACKED));
    packed_data->format = glLighti_FORMAT;
    packed_data->func = glLighti;
    packed_data->args.a1 = light;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLightiv
static inline void push_glLightiv(glLightiv_ARG_EXPAND) {
    glLightiv_PACKED *packed_data = malloc(sizeof(glLightiv_PACKED));
    packed_data->format = glLightiv_FORMAT;
    packed_data->func = glLightiv;
    packed_data->args.a1 = light;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLineStipple
static inline void push_glLineStipple(glLineStipple_ARG_EXPAND) {
    glLineStipple_PACKED *packed_data = malloc(sizeof(glLineStipple_PACKED));
    packed_data->format = glLineStipple_FORMAT;
    packed_data->func = glLineStipple;
    packed_data->args.a1 = factor;
    packed_data->args.a2 = pattern;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLineWidth
static inline void push_glLineWidth(glLineWidth_ARG_EXPAND) {
    glLineWidth_PACKED *packed_data = malloc(sizeof(glLineWidth_PACKED));
    packed_data->format = glLineWidth_FORMAT;
    packed_data->func = glLineWidth;
    packed_data->args.a1 = width;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glListBase
static inline void push_glListBase(glListBase_ARG_EXPAND) {
    glListBase_PACKED *packed_data = malloc(sizeof(glListBase_PACKED));
    packed_data->format = glListBase_FORMAT;
    packed_data->func = glListBase;
    packed_data->args.a1 = base;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLoadIdentity
static inline void push_glLoadIdentity(glLoadIdentity_ARG_EXPAND) {
    glLoadIdentity_PACKED *packed_data = malloc(sizeof(glLoadIdentity_PACKED));
    packed_data->format = glLoadIdentity_FORMAT;
    packed_data->func = glLoadIdentity;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLoadMatrixd
static inline void push_glLoadMatrixd(glLoadMatrixd_ARG_EXPAND) {
    glLoadMatrixd_PACKED *packed_data = malloc(sizeof(glLoadMatrixd_PACKED));
    packed_data->format = glLoadMatrixd_FORMAT;
    packed_data->func = glLoadMatrixd;
    packed_data->args.a1 = m;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLoadMatrixf
static inline void push_glLoadMatrixf(glLoadMatrixf_ARG_EXPAND) {
    glLoadMatrixf_PACKED *packed_data = malloc(sizeof(glLoadMatrixf_PACKED));
    packed_data->format = glLoadMatrixf_FORMAT;
    packed_data->func = glLoadMatrixf;
    packed_data->args.a1 = m;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLoadName
static inline void push_glLoadName(glLoadName_ARG_EXPAND) {
    glLoadName_PACKED *packed_data = malloc(sizeof(glLoadName_PACKED));
    packed_data->format = glLoadName_FORMAT;
    packed_data->func = glLoadName;
    packed_data->args.a1 = name;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLoadTransposeMatrixd
static inline void push_glLoadTransposeMatrixd(glLoadTransposeMatrixd_ARG_EXPAND) {
    glLoadTransposeMatrixd_PACKED *packed_data = malloc(sizeof(glLoadTransposeMatrixd_PACKED));
    packed_data->format = glLoadTransposeMatrixd_FORMAT;
    packed_data->func = glLoadTransposeMatrixd;
    packed_data->args.a1 = m;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLoadTransposeMatrixf
static inline void push_glLoadTransposeMatrixf(glLoadTransposeMatrixf_ARG_EXPAND) {
    glLoadTransposeMatrixf_PACKED *packed_data = malloc(sizeof(glLoadTransposeMatrixf_PACKED));
    packed_data->format = glLoadTransposeMatrixf_FORMAT;
    packed_data->func = glLoadTransposeMatrixf;
    packed_data->args.a1 = m;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glLogicOp
static inline void push_glLogicOp(glLogicOp_ARG_EXPAND) {
    glLogicOp_PACKED *packed_data = malloc(sizeof(glLogicOp_PACKED));
    packed_data->format = glLogicOp_FORMAT;
    packed_data->func = glLogicOp;
    packed_data->args.a1 = opcode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMap1d
static inline void push_glMap1d(glMap1d_ARG_EXPAND) {
    glMap1d_PACKED *packed_data = malloc(sizeof(glMap1d_PACKED));
    packed_data->format = glMap1d_FORMAT;
    packed_data->func = glMap1d;
    packed_data->args.a1 = target;
    packed_data->args.a2 = u1;
    packed_data->args.a3 = u2;
    packed_data->args.a4 = stride;
    packed_data->args.a5 = order;
    packed_data->args.a6 = points;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMap1f
static inline void push_glMap1f(glMap1f_ARG_EXPAND) {
    glMap1f_PACKED *packed_data = malloc(sizeof(glMap1f_PACKED));
    packed_data->format = glMap1f_FORMAT;
    packed_data->func = glMap1f;
    packed_data->args.a1 = target;
    packed_data->args.a2 = u1;
    packed_data->args.a3 = u2;
    packed_data->args.a4 = stride;
    packed_data->args.a5 = order;
    packed_data->args.a6 = points;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMap2d
static inline void push_glMap2d(glMap2d_ARG_EXPAND) {
    glMap2d_PACKED *packed_data = malloc(sizeof(glMap2d_PACKED));
    packed_data->format = glMap2d_FORMAT;
    packed_data->func = glMap2d;
    packed_data->args.a1 = target;
    packed_data->args.a2 = u1;
    packed_data->args.a3 = u2;
    packed_data->args.a4 = ustride;
    packed_data->args.a5 = uorder;
    packed_data->args.a6 = v1;
    packed_data->args.a7 = v2;
    packed_data->args.a8 = vstride;
    packed_data->args.a9 = vorder;
    packed_data->args.a10 = points;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMap2f
static inline void push_glMap2f(glMap2f_ARG_EXPAND) {
    glMap2f_PACKED *packed_data = malloc(sizeof(glMap2f_PACKED));
    packed_data->format = glMap2f_FORMAT;
    packed_data->func = glMap2f;
    packed_data->args.a1 = target;
    packed_data->args.a2 = u1;
    packed_data->args.a3 = u2;
    packed_data->args.a4 = ustride;
    packed_data->args.a5 = uorder;
    packed_data->args.a6 = v1;
    packed_data->args.a7 = v2;
    packed_data->args.a8 = vstride;
    packed_data->args.a9 = vorder;
    packed_data->args.a10 = points;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMapBuffer
static inline void push_glMapBuffer(glMapBuffer_ARG_EXPAND) {
    glMapBuffer_PACKED *packed_data = malloc(sizeof(glMapBuffer_PACKED));
    packed_data->format = glMapBuffer_FORMAT;
    packed_data->func = glMapBuffer;
    packed_data->args.a1 = target;
    packed_data->args.a2 = access;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMapGrid1d
static inline void push_glMapGrid1d(glMapGrid1d_ARG_EXPAND) {
    glMapGrid1d_PACKED *packed_data = malloc(sizeof(glMapGrid1d_PACKED));
    packed_data->format = glMapGrid1d_FORMAT;
    packed_data->func = glMapGrid1d;
    packed_data->args.a1 = un;
    packed_data->args.a2 = u1;
    packed_data->args.a3 = u2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMapGrid1f
static inline void push_glMapGrid1f(glMapGrid1f_ARG_EXPAND) {
    glMapGrid1f_PACKED *packed_data = malloc(sizeof(glMapGrid1f_PACKED));
    packed_data->format = glMapGrid1f_FORMAT;
    packed_data->func = glMapGrid1f;
    packed_data->args.a1 = un;
    packed_data->args.a2 = u1;
    packed_data->args.a3 = u2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMapGrid2d
static inline void push_glMapGrid2d(glMapGrid2d_ARG_EXPAND) {
    glMapGrid2d_PACKED *packed_data = malloc(sizeof(glMapGrid2d_PACKED));
    packed_data->format = glMapGrid2d_FORMAT;
    packed_data->func = glMapGrid2d;
    packed_data->args.a1 = un;
    packed_data->args.a2 = u1;
    packed_data->args.a3 = u2;
    packed_data->args.a4 = vn;
    packed_data->args.a5 = v1;
    packed_data->args.a6 = v2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMapGrid2f
static inline void push_glMapGrid2f(glMapGrid2f_ARG_EXPAND) {
    glMapGrid2f_PACKED *packed_data = malloc(sizeof(glMapGrid2f_PACKED));
    packed_data->format = glMapGrid2f_FORMAT;
    packed_data->func = glMapGrid2f;
    packed_data->args.a1 = un;
    packed_data->args.a2 = u1;
    packed_data->args.a3 = u2;
    packed_data->args.a4 = vn;
    packed_data->args.a5 = v1;
    packed_data->args.a6 = v2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMaterialf
static inline void push_glMaterialf(glMaterialf_ARG_EXPAND) {
    glMaterialf_PACKED *packed_data = malloc(sizeof(glMaterialf_PACKED));
    packed_data->format = glMaterialf_FORMAT;
    packed_data->func = glMaterialf;
    packed_data->args.a1 = face;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMaterialfv
static inline void push_glMaterialfv(glMaterialfv_ARG_EXPAND) {
    glMaterialfv_PACKED *packed_data = malloc(sizeof(glMaterialfv_PACKED));
    packed_data->format = glMaterialfv_FORMAT;
    packed_data->func = glMaterialfv;
    packed_data->args.a1 = face;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMateriali
static inline void push_glMateriali(glMateriali_ARG_EXPAND) {
    glMateriali_PACKED *packed_data = malloc(sizeof(glMateriali_PACKED));
    packed_data->format = glMateriali_FORMAT;
    packed_data->func = glMateriali;
    packed_data->args.a1 = face;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMaterialiv
static inline void push_glMaterialiv(glMaterialiv_ARG_EXPAND) {
    glMaterialiv_PACKED *packed_data = malloc(sizeof(glMaterialiv_PACKED));
    packed_data->format = glMaterialiv_FORMAT;
    packed_data->func = glMaterialiv;
    packed_data->args.a1 = face;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMatrixMode
static inline void push_glMatrixMode(glMatrixMode_ARG_EXPAND) {
    glMatrixMode_PACKED *packed_data = malloc(sizeof(glMatrixMode_PACKED));
    packed_data->format = glMatrixMode_FORMAT;
    packed_data->func = glMatrixMode;
    packed_data->args.a1 = mode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMinmax
static inline void push_glMinmax(glMinmax_ARG_EXPAND) {
    glMinmax_PACKED *packed_data = malloc(sizeof(glMinmax_PACKED));
    packed_data->format = glMinmax_FORMAT;
    packed_data->func = glMinmax;
    packed_data->args.a1 = target;
    packed_data->args.a2 = internalformat;
    packed_data->args.a3 = sink;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultMatrixd
static inline void push_glMultMatrixd(glMultMatrixd_ARG_EXPAND) {
    glMultMatrixd_PACKED *packed_data = malloc(sizeof(glMultMatrixd_PACKED));
    packed_data->format = glMultMatrixd_FORMAT;
    packed_data->func = glMultMatrixd;
    packed_data->args.a1 = m;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultMatrixf
static inline void push_glMultMatrixf(glMultMatrixf_ARG_EXPAND) {
    glMultMatrixf_PACKED *packed_data = malloc(sizeof(glMultMatrixf_PACKED));
    packed_data->format = glMultMatrixf_FORMAT;
    packed_data->func = glMultMatrixf;
    packed_data->args.a1 = m;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultTransposeMatrixd
static inline void push_glMultTransposeMatrixd(glMultTransposeMatrixd_ARG_EXPAND) {
    glMultTransposeMatrixd_PACKED *packed_data = malloc(sizeof(glMultTransposeMatrixd_PACKED));
    packed_data->format = glMultTransposeMatrixd_FORMAT;
    packed_data->func = glMultTransposeMatrixd;
    packed_data->args.a1 = m;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultTransposeMatrixf
static inline void push_glMultTransposeMatrixf(glMultTransposeMatrixf_ARG_EXPAND) {
    glMultTransposeMatrixf_PACKED *packed_data = malloc(sizeof(glMultTransposeMatrixf_PACKED));
    packed_data->format = glMultTransposeMatrixf_FORMAT;
    packed_data->func = glMultTransposeMatrixf;
    packed_data->args.a1 = m;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiDrawArrays
static inline void push_glMultiDrawArrays(glMultiDrawArrays_ARG_EXPAND) {
    glMultiDrawArrays_PACKED *packed_data = malloc(sizeof(glMultiDrawArrays_PACKED));
    packed_data->format = glMultiDrawArrays_FORMAT;
    packed_data->func = glMultiDrawArrays;
    packed_data->args.a1 = mode;
    packed_data->args.a2 = first;
    packed_data->args.a3 = count;
    packed_data->args.a4 = drawcount;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiDrawElements
static inline void push_glMultiDrawElements(glMultiDrawElements_ARG_EXPAND) {
    glMultiDrawElements_PACKED *packed_data = malloc(sizeof(glMultiDrawElements_PACKED));
    packed_data->format = glMultiDrawElements_FORMAT;
    packed_data->func = glMultiDrawElements;
    packed_data->args.a1 = mode;
    packed_data->args.a2 = count;
    packed_data->args.a3 = type;
    packed_data->args.a4 = indices;
    packed_data->args.a5 = drawcount;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord1d
static inline void push_glMultiTexCoord1d(glMultiTexCoord1d_ARG_EXPAND) {
    glMultiTexCoord1d_PACKED *packed_data = malloc(sizeof(glMultiTexCoord1d_PACKED));
    packed_data->format = glMultiTexCoord1d_FORMAT;
    packed_data->func = glMultiTexCoord1d;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord1dv
static inline void push_glMultiTexCoord1dv(glMultiTexCoord1dv_ARG_EXPAND) {
    glMultiTexCoord1dv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord1dv_PACKED));
    packed_data->format = glMultiTexCoord1dv_FORMAT;
    packed_data->func = glMultiTexCoord1dv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord1f
static inline void push_glMultiTexCoord1f(glMultiTexCoord1f_ARG_EXPAND) {
    glMultiTexCoord1f_PACKED *packed_data = malloc(sizeof(glMultiTexCoord1f_PACKED));
    packed_data->format = glMultiTexCoord1f_FORMAT;
    packed_data->func = glMultiTexCoord1f;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord1fv
static inline void push_glMultiTexCoord1fv(glMultiTexCoord1fv_ARG_EXPAND) {
    glMultiTexCoord1fv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord1fv_PACKED));
    packed_data->format = glMultiTexCoord1fv_FORMAT;
    packed_data->func = glMultiTexCoord1fv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord1i
static inline void push_glMultiTexCoord1i(glMultiTexCoord1i_ARG_EXPAND) {
    glMultiTexCoord1i_PACKED *packed_data = malloc(sizeof(glMultiTexCoord1i_PACKED));
    packed_data->format = glMultiTexCoord1i_FORMAT;
    packed_data->func = glMultiTexCoord1i;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord1iv
static inline void push_glMultiTexCoord1iv(glMultiTexCoord1iv_ARG_EXPAND) {
    glMultiTexCoord1iv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord1iv_PACKED));
    packed_data->format = glMultiTexCoord1iv_FORMAT;
    packed_data->func = glMultiTexCoord1iv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord1s
static inline void push_glMultiTexCoord1s(glMultiTexCoord1s_ARG_EXPAND) {
    glMultiTexCoord1s_PACKED *packed_data = malloc(sizeof(glMultiTexCoord1s_PACKED));
    packed_data->format = glMultiTexCoord1s_FORMAT;
    packed_data->func = glMultiTexCoord1s;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord1sv
static inline void push_glMultiTexCoord1sv(glMultiTexCoord1sv_ARG_EXPAND) {
    glMultiTexCoord1sv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord1sv_PACKED));
    packed_data->format = glMultiTexCoord1sv_FORMAT;
    packed_data->func = glMultiTexCoord1sv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord2d
static inline void push_glMultiTexCoord2d(glMultiTexCoord2d_ARG_EXPAND) {
    glMultiTexCoord2d_PACKED *packed_data = malloc(sizeof(glMultiTexCoord2d_PACKED));
    packed_data->format = glMultiTexCoord2d_FORMAT;
    packed_data->func = glMultiTexCoord2d;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    packed_data->args.a3 = t;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord2dv
static inline void push_glMultiTexCoord2dv(glMultiTexCoord2dv_ARG_EXPAND) {
    glMultiTexCoord2dv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord2dv_PACKED));
    packed_data->format = glMultiTexCoord2dv_FORMAT;
    packed_data->func = glMultiTexCoord2dv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord2f
static inline void push_glMultiTexCoord2f(glMultiTexCoord2f_ARG_EXPAND) {
    glMultiTexCoord2f_PACKED *packed_data = malloc(sizeof(glMultiTexCoord2f_PACKED));
    packed_data->format = glMultiTexCoord2f_FORMAT;
    packed_data->func = glMultiTexCoord2f;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    packed_data->args.a3 = t;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord2fv
static inline void push_glMultiTexCoord2fv(glMultiTexCoord2fv_ARG_EXPAND) {
    glMultiTexCoord2fv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord2fv_PACKED));
    packed_data->format = glMultiTexCoord2fv_FORMAT;
    packed_data->func = glMultiTexCoord2fv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord2i
static inline void push_glMultiTexCoord2i(glMultiTexCoord2i_ARG_EXPAND) {
    glMultiTexCoord2i_PACKED *packed_data = malloc(sizeof(glMultiTexCoord2i_PACKED));
    packed_data->format = glMultiTexCoord2i_FORMAT;
    packed_data->func = glMultiTexCoord2i;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    packed_data->args.a3 = t;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord2iv
static inline void push_glMultiTexCoord2iv(glMultiTexCoord2iv_ARG_EXPAND) {
    glMultiTexCoord2iv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord2iv_PACKED));
    packed_data->format = glMultiTexCoord2iv_FORMAT;
    packed_data->func = glMultiTexCoord2iv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord2s
static inline void push_glMultiTexCoord2s(glMultiTexCoord2s_ARG_EXPAND) {
    glMultiTexCoord2s_PACKED *packed_data = malloc(sizeof(glMultiTexCoord2s_PACKED));
    packed_data->format = glMultiTexCoord2s_FORMAT;
    packed_data->func = glMultiTexCoord2s;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    packed_data->args.a3 = t;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord2sv
static inline void push_glMultiTexCoord2sv(glMultiTexCoord2sv_ARG_EXPAND) {
    glMultiTexCoord2sv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord2sv_PACKED));
    packed_data->format = glMultiTexCoord2sv_FORMAT;
    packed_data->func = glMultiTexCoord2sv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord3d
static inline void push_glMultiTexCoord3d(glMultiTexCoord3d_ARG_EXPAND) {
    glMultiTexCoord3d_PACKED *packed_data = malloc(sizeof(glMultiTexCoord3d_PACKED));
    packed_data->format = glMultiTexCoord3d_FORMAT;
    packed_data->func = glMultiTexCoord3d;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    packed_data->args.a3 = t;
    packed_data->args.a4 = r;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord3dv
static inline void push_glMultiTexCoord3dv(glMultiTexCoord3dv_ARG_EXPAND) {
    glMultiTexCoord3dv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord3dv_PACKED));
    packed_data->format = glMultiTexCoord3dv_FORMAT;
    packed_data->func = glMultiTexCoord3dv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord3f
static inline void push_glMultiTexCoord3f(glMultiTexCoord3f_ARG_EXPAND) {
    glMultiTexCoord3f_PACKED *packed_data = malloc(sizeof(glMultiTexCoord3f_PACKED));
    packed_data->format = glMultiTexCoord3f_FORMAT;
    packed_data->func = glMultiTexCoord3f;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    packed_data->args.a3 = t;
    packed_data->args.a4 = r;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord3fv
static inline void push_glMultiTexCoord3fv(glMultiTexCoord3fv_ARG_EXPAND) {
    glMultiTexCoord3fv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord3fv_PACKED));
    packed_data->format = glMultiTexCoord3fv_FORMAT;
    packed_data->func = glMultiTexCoord3fv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord3i
static inline void push_glMultiTexCoord3i(glMultiTexCoord3i_ARG_EXPAND) {
    glMultiTexCoord3i_PACKED *packed_data = malloc(sizeof(glMultiTexCoord3i_PACKED));
    packed_data->format = glMultiTexCoord3i_FORMAT;
    packed_data->func = glMultiTexCoord3i;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    packed_data->args.a3 = t;
    packed_data->args.a4 = r;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord3iv
static inline void push_glMultiTexCoord3iv(glMultiTexCoord3iv_ARG_EXPAND) {
    glMultiTexCoord3iv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord3iv_PACKED));
    packed_data->format = glMultiTexCoord3iv_FORMAT;
    packed_data->func = glMultiTexCoord3iv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord3s
static inline void push_glMultiTexCoord3s(glMultiTexCoord3s_ARG_EXPAND) {
    glMultiTexCoord3s_PACKED *packed_data = malloc(sizeof(glMultiTexCoord3s_PACKED));
    packed_data->format = glMultiTexCoord3s_FORMAT;
    packed_data->func = glMultiTexCoord3s;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    packed_data->args.a3 = t;
    packed_data->args.a4 = r;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord3sv
static inline void push_glMultiTexCoord3sv(glMultiTexCoord3sv_ARG_EXPAND) {
    glMultiTexCoord3sv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord3sv_PACKED));
    packed_data->format = glMultiTexCoord3sv_FORMAT;
    packed_data->func = glMultiTexCoord3sv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord4d
static inline void push_glMultiTexCoord4d(glMultiTexCoord4d_ARG_EXPAND) {
    glMultiTexCoord4d_PACKED *packed_data = malloc(sizeof(glMultiTexCoord4d_PACKED));
    packed_data->format = glMultiTexCoord4d_FORMAT;
    packed_data->func = glMultiTexCoord4d;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    packed_data->args.a3 = t;
    packed_data->args.a4 = r;
    packed_data->args.a5 = q;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord4dv
static inline void push_glMultiTexCoord4dv(glMultiTexCoord4dv_ARG_EXPAND) {
    glMultiTexCoord4dv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord4dv_PACKED));
    packed_data->format = glMultiTexCoord4dv_FORMAT;
    packed_data->func = glMultiTexCoord4dv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord4f
static inline void push_glMultiTexCoord4f(glMultiTexCoord4f_ARG_EXPAND) {
    glMultiTexCoord4f_PACKED *packed_data = malloc(sizeof(glMultiTexCoord4f_PACKED));
    packed_data->format = glMultiTexCoord4f_FORMAT;
    packed_data->func = glMultiTexCoord4f;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    packed_data->args.a3 = t;
    packed_data->args.a4 = r;
    packed_data->args.a5 = q;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord4fv
static inline void push_glMultiTexCoord4fv(glMultiTexCoord4fv_ARG_EXPAND) {
    glMultiTexCoord4fv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord4fv_PACKED));
    packed_data->format = glMultiTexCoord4fv_FORMAT;
    packed_data->func = glMultiTexCoord4fv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord4i
static inline void push_glMultiTexCoord4i(glMultiTexCoord4i_ARG_EXPAND) {
    glMultiTexCoord4i_PACKED *packed_data = malloc(sizeof(glMultiTexCoord4i_PACKED));
    packed_data->format = glMultiTexCoord4i_FORMAT;
    packed_data->func = glMultiTexCoord4i;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    packed_data->args.a3 = t;
    packed_data->args.a4 = r;
    packed_data->args.a5 = q;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord4iv
static inline void push_glMultiTexCoord4iv(glMultiTexCoord4iv_ARG_EXPAND) {
    glMultiTexCoord4iv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord4iv_PACKED));
    packed_data->format = glMultiTexCoord4iv_FORMAT;
    packed_data->func = glMultiTexCoord4iv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord4s
static inline void push_glMultiTexCoord4s(glMultiTexCoord4s_ARG_EXPAND) {
    glMultiTexCoord4s_PACKED *packed_data = malloc(sizeof(glMultiTexCoord4s_PACKED));
    packed_data->format = glMultiTexCoord4s_FORMAT;
    packed_data->func = glMultiTexCoord4s;
    packed_data->args.a1 = target;
    packed_data->args.a2 = s;
    packed_data->args.a3 = t;
    packed_data->args.a4 = r;
    packed_data->args.a5 = q;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glMultiTexCoord4sv
static inline void push_glMultiTexCoord4sv(glMultiTexCoord4sv_ARG_EXPAND) {
    glMultiTexCoord4sv_PACKED *packed_data = malloc(sizeof(glMultiTexCoord4sv_PACKED));
    packed_data->format = glMultiTexCoord4sv_FORMAT;
    packed_data->func = glMultiTexCoord4sv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glNewList
static inline void push_glNewList(glNewList_ARG_EXPAND) {
    glNewList_PACKED *packed_data = malloc(sizeof(glNewList_PACKED));
    packed_data->format = glNewList_FORMAT;
    packed_data->func = glNewList;
    packed_data->args.a1 = list;
    packed_data->args.a2 = mode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glNormal3b
static inline void push_glNormal3b(glNormal3b_ARG_EXPAND) {
    glNormal3b_PACKED *packed_data = malloc(sizeof(glNormal3b_PACKED));
    packed_data->format = glNormal3b_FORMAT;
    packed_data->func = glNormal3b;
    packed_data->args.a1 = nx;
    packed_data->args.a2 = ny;
    packed_data->args.a3 = nz;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glNormal3bv
static inline void push_glNormal3bv(glNormal3bv_ARG_EXPAND) {
    glNormal3bv_PACKED *packed_data = malloc(sizeof(glNormal3bv_PACKED));
    packed_data->format = glNormal3bv_FORMAT;
    packed_data->func = glNormal3bv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glNormal3d
static inline void push_glNormal3d(glNormal3d_ARG_EXPAND) {
    glNormal3d_PACKED *packed_data = malloc(sizeof(glNormal3d_PACKED));
    packed_data->format = glNormal3d_FORMAT;
    packed_data->func = glNormal3d;
    packed_data->args.a1 = nx;
    packed_data->args.a2 = ny;
    packed_data->args.a3 = nz;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glNormal3dv
static inline void push_glNormal3dv(glNormal3dv_ARG_EXPAND) {
    glNormal3dv_PACKED *packed_data = malloc(sizeof(glNormal3dv_PACKED));
    packed_data->format = glNormal3dv_FORMAT;
    packed_data->func = glNormal3dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glNormal3f
static inline void push_glNormal3f(glNormal3f_ARG_EXPAND) {
    glNormal3f_PACKED *packed_data = malloc(sizeof(glNormal3f_PACKED));
    packed_data->format = glNormal3f_FORMAT;
    packed_data->func = glNormal3f;
    packed_data->args.a1 = nx;
    packed_data->args.a2 = ny;
    packed_data->args.a3 = nz;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glNormal3fv
static inline void push_glNormal3fv(glNormal3fv_ARG_EXPAND) {
    glNormal3fv_PACKED *packed_data = malloc(sizeof(glNormal3fv_PACKED));
    packed_data->format = glNormal3fv_FORMAT;
    packed_data->func = glNormal3fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glNormal3i
static inline void push_glNormal3i(glNormal3i_ARG_EXPAND) {
    glNormal3i_PACKED *packed_data = malloc(sizeof(glNormal3i_PACKED));
    packed_data->format = glNormal3i_FORMAT;
    packed_data->func = glNormal3i;
    packed_data->args.a1 = nx;
    packed_data->args.a2 = ny;
    packed_data->args.a3 = nz;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glNormal3iv
static inline void push_glNormal3iv(glNormal3iv_ARG_EXPAND) {
    glNormal3iv_PACKED *packed_data = malloc(sizeof(glNormal3iv_PACKED));
    packed_data->format = glNormal3iv_FORMAT;
    packed_data->func = glNormal3iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glNormal3s
static inline void push_glNormal3s(glNormal3s_ARG_EXPAND) {
    glNormal3s_PACKED *packed_data = malloc(sizeof(glNormal3s_PACKED));
    packed_data->format = glNormal3s_FORMAT;
    packed_data->func = glNormal3s;
    packed_data->args.a1 = nx;
    packed_data->args.a2 = ny;
    packed_data->args.a3 = nz;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glNormal3sv
static inline void push_glNormal3sv(glNormal3sv_ARG_EXPAND) {
    glNormal3sv_PACKED *packed_data = malloc(sizeof(glNormal3sv_PACKED));
    packed_data->format = glNormal3sv_FORMAT;
    packed_data->func = glNormal3sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glNormalPointer
static inline void push_glNormalPointer(glNormalPointer_ARG_EXPAND) {
    glNormalPointer_PACKED *packed_data = malloc(sizeof(glNormalPointer_PACKED));
    packed_data->format = glNormalPointer_FORMAT;
    packed_data->func = glNormalPointer;
    packed_data->args.a1 = type;
    packed_data->args.a2 = stride;
    packed_data->args.a3 = pointer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glOrtho
static inline void push_glOrtho(glOrtho_ARG_EXPAND) {
    glOrtho_PACKED *packed_data = malloc(sizeof(glOrtho_PACKED));
    packed_data->format = glOrtho_FORMAT;
    packed_data->func = glOrtho;
    packed_data->args.a1 = left;
    packed_data->args.a2 = right;
    packed_data->args.a3 = bottom;
    packed_data->args.a4 = top;
    packed_data->args.a5 = zNear;
    packed_data->args.a6 = zFar;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPassThrough
static inline void push_glPassThrough(glPassThrough_ARG_EXPAND) {
    glPassThrough_PACKED *packed_data = malloc(sizeof(glPassThrough_PACKED));
    packed_data->format = glPassThrough_FORMAT;
    packed_data->func = glPassThrough;
    packed_data->args.a1 = token;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPixelMapfv
static inline void push_glPixelMapfv(glPixelMapfv_ARG_EXPAND) {
    glPixelMapfv_PACKED *packed_data = malloc(sizeof(glPixelMapfv_PACKED));
    packed_data->format = glPixelMapfv_FORMAT;
    packed_data->func = glPixelMapfv;
    packed_data->args.a1 = map;
    packed_data->args.a2 = mapsize;
    packed_data->args.a3 = values;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPixelMapuiv
static inline void push_glPixelMapuiv(glPixelMapuiv_ARG_EXPAND) {
    glPixelMapuiv_PACKED *packed_data = malloc(sizeof(glPixelMapuiv_PACKED));
    packed_data->format = glPixelMapuiv_FORMAT;
    packed_data->func = glPixelMapuiv;
    packed_data->args.a1 = map;
    packed_data->args.a2 = mapsize;
    packed_data->args.a3 = values;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPixelMapusv
static inline void push_glPixelMapusv(glPixelMapusv_ARG_EXPAND) {
    glPixelMapusv_PACKED *packed_data = malloc(sizeof(glPixelMapusv_PACKED));
    packed_data->format = glPixelMapusv_FORMAT;
    packed_data->func = glPixelMapusv;
    packed_data->args.a1 = map;
    packed_data->args.a2 = mapsize;
    packed_data->args.a3 = values;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPixelStoref
static inline void push_glPixelStoref(glPixelStoref_ARG_EXPAND) {
    glPixelStoref_PACKED *packed_data = malloc(sizeof(glPixelStoref_PACKED));
    packed_data->format = glPixelStoref_FORMAT;
    packed_data->func = glPixelStoref;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPixelStorei
static inline void push_glPixelStorei(glPixelStorei_ARG_EXPAND) {
    glPixelStorei_PACKED *packed_data = malloc(sizeof(glPixelStorei_PACKED));
    packed_data->format = glPixelStorei_FORMAT;
    packed_data->func = glPixelStorei;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPixelTransferf
static inline void push_glPixelTransferf(glPixelTransferf_ARG_EXPAND) {
    glPixelTransferf_PACKED *packed_data = malloc(sizeof(glPixelTransferf_PACKED));
    packed_data->format = glPixelTransferf_FORMAT;
    packed_data->func = glPixelTransferf;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPixelTransferi
static inline void push_glPixelTransferi(glPixelTransferi_ARG_EXPAND) {
    glPixelTransferi_PACKED *packed_data = malloc(sizeof(glPixelTransferi_PACKED));
    packed_data->format = glPixelTransferi_FORMAT;
    packed_data->func = glPixelTransferi;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPixelZoom
static inline void push_glPixelZoom(glPixelZoom_ARG_EXPAND) {
    glPixelZoom_PACKED *packed_data = malloc(sizeof(glPixelZoom_PACKED));
    packed_data->format = glPixelZoom_FORMAT;
    packed_data->func = glPixelZoom;
    packed_data->args.a1 = xfactor;
    packed_data->args.a2 = yfactor;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPointParameterf
static inline void push_glPointParameterf(glPointParameterf_ARG_EXPAND) {
    glPointParameterf_PACKED *packed_data = malloc(sizeof(glPointParameterf_PACKED));
    packed_data->format = glPointParameterf_FORMAT;
    packed_data->func = glPointParameterf;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPointParameterfv
static inline void push_glPointParameterfv(glPointParameterfv_ARG_EXPAND) {
    glPointParameterfv_PACKED *packed_data = malloc(sizeof(glPointParameterfv_PACKED));
    packed_data->format = glPointParameterfv_FORMAT;
    packed_data->func = glPointParameterfv;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPointParameteri
static inline void push_glPointParameteri(glPointParameteri_ARG_EXPAND) {
    glPointParameteri_PACKED *packed_data = malloc(sizeof(glPointParameteri_PACKED));
    packed_data->format = glPointParameteri_FORMAT;
    packed_data->func = glPointParameteri;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPointParameteriv
static inline void push_glPointParameteriv(glPointParameteriv_ARG_EXPAND) {
    glPointParameteriv_PACKED *packed_data = malloc(sizeof(glPointParameteriv_PACKED));
    packed_data->format = glPointParameteriv_FORMAT;
    packed_data->func = glPointParameteriv;
    packed_data->args.a1 = pname;
    packed_data->args.a2 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPointSize
static inline void push_glPointSize(glPointSize_ARG_EXPAND) {
    glPointSize_PACKED *packed_data = malloc(sizeof(glPointSize_PACKED));
    packed_data->format = glPointSize_FORMAT;
    packed_data->func = glPointSize;
    packed_data->args.a1 = size;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPolygonMode
static inline void push_glPolygonMode(glPolygonMode_ARG_EXPAND) {
    glPolygonMode_PACKED *packed_data = malloc(sizeof(glPolygonMode_PACKED));
    packed_data->format = glPolygonMode_FORMAT;
    packed_data->func = glPolygonMode;
    packed_data->args.a1 = face;
    packed_data->args.a2 = mode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPolygonOffset
static inline void push_glPolygonOffset(glPolygonOffset_ARG_EXPAND) {
    glPolygonOffset_PACKED *packed_data = malloc(sizeof(glPolygonOffset_PACKED));
    packed_data->format = glPolygonOffset_FORMAT;
    packed_data->func = glPolygonOffset;
    packed_data->args.a1 = factor;
    packed_data->args.a2 = units;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPolygonStipple
static inline void push_glPolygonStipple(glPolygonStipple_ARG_EXPAND) {
    glPolygonStipple_PACKED *packed_data = malloc(sizeof(glPolygonStipple_PACKED));
    packed_data->format = glPolygonStipple_FORMAT;
    packed_data->func = glPolygonStipple;
    packed_data->args.a1 = mask;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPopAttrib
static inline void push_glPopAttrib(glPopAttrib_ARG_EXPAND) {
    glPopAttrib_PACKED *packed_data = malloc(sizeof(glPopAttrib_PACKED));
    packed_data->format = glPopAttrib_FORMAT;
    packed_data->func = glPopAttrib;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPopClientAttrib
static inline void push_glPopClientAttrib(glPopClientAttrib_ARG_EXPAND) {
    glPopClientAttrib_PACKED *packed_data = malloc(sizeof(glPopClientAttrib_PACKED));
    packed_data->format = glPopClientAttrib_FORMAT;
    packed_data->func = glPopClientAttrib;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPopMatrix
static inline void push_glPopMatrix(glPopMatrix_ARG_EXPAND) {
    glPopMatrix_PACKED *packed_data = malloc(sizeof(glPopMatrix_PACKED));
    packed_data->format = glPopMatrix_FORMAT;
    packed_data->func = glPopMatrix;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPopName
static inline void push_glPopName(glPopName_ARG_EXPAND) {
    glPopName_PACKED *packed_data = malloc(sizeof(glPopName_PACKED));
    packed_data->format = glPopName_FORMAT;
    packed_data->func = glPopName;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPrioritizeTextures
static inline void push_glPrioritizeTextures(glPrioritizeTextures_ARG_EXPAND) {
    glPrioritizeTextures_PACKED *packed_data = malloc(sizeof(glPrioritizeTextures_PACKED));
    packed_data->format = glPrioritizeTextures_FORMAT;
    packed_data->func = glPrioritizeTextures;
    packed_data->args.a1 = n;
    packed_data->args.a2 = textures;
    packed_data->args.a3 = priorities;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPushAttrib
static inline void push_glPushAttrib(glPushAttrib_ARG_EXPAND) {
    glPushAttrib_PACKED *packed_data = malloc(sizeof(glPushAttrib_PACKED));
    packed_data->format = glPushAttrib_FORMAT;
    packed_data->func = glPushAttrib;
    packed_data->args.a1 = mask;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPushClientAttrib
static inline void push_glPushClientAttrib(glPushClientAttrib_ARG_EXPAND) {
    glPushClientAttrib_PACKED *packed_data = malloc(sizeof(glPushClientAttrib_PACKED));
    packed_data->format = glPushClientAttrib_FORMAT;
    packed_data->func = glPushClientAttrib;
    packed_data->args.a1 = mask;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPushMatrix
static inline void push_glPushMatrix(glPushMatrix_ARG_EXPAND) {
    glPushMatrix_PACKED *packed_data = malloc(sizeof(glPushMatrix_PACKED));
    packed_data->format = glPushMatrix_FORMAT;
    packed_data->func = glPushMatrix;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glPushName
static inline void push_glPushName(glPushName_ARG_EXPAND) {
    glPushName_PACKED *packed_data = malloc(sizeof(glPushName_PACKED));
    packed_data->format = glPushName_FORMAT;
    packed_data->func = glPushName;
    packed_data->args.a1 = name;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos2d
static inline void push_glRasterPos2d(glRasterPos2d_ARG_EXPAND) {
    glRasterPos2d_PACKED *packed_data = malloc(sizeof(glRasterPos2d_PACKED));
    packed_data->format = glRasterPos2d_FORMAT;
    packed_data->func = glRasterPos2d;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos2dv
static inline void push_glRasterPos2dv(glRasterPos2dv_ARG_EXPAND) {
    glRasterPos2dv_PACKED *packed_data = malloc(sizeof(glRasterPos2dv_PACKED));
    packed_data->format = glRasterPos2dv_FORMAT;
    packed_data->func = glRasterPos2dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos2f
static inline void push_glRasterPos2f(glRasterPos2f_ARG_EXPAND) {
    glRasterPos2f_PACKED *packed_data = malloc(sizeof(glRasterPos2f_PACKED));
    packed_data->format = glRasterPos2f_FORMAT;
    packed_data->func = glRasterPos2f;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos2fv
static inline void push_glRasterPos2fv(glRasterPos2fv_ARG_EXPAND) {
    glRasterPos2fv_PACKED *packed_data = malloc(sizeof(glRasterPos2fv_PACKED));
    packed_data->format = glRasterPos2fv_FORMAT;
    packed_data->func = glRasterPos2fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos2i
static inline void push_glRasterPos2i(glRasterPos2i_ARG_EXPAND) {
    glRasterPos2i_PACKED *packed_data = malloc(sizeof(glRasterPos2i_PACKED));
    packed_data->format = glRasterPos2i_FORMAT;
    packed_data->func = glRasterPos2i;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos2iv
static inline void push_glRasterPos2iv(glRasterPos2iv_ARG_EXPAND) {
    glRasterPos2iv_PACKED *packed_data = malloc(sizeof(glRasterPos2iv_PACKED));
    packed_data->format = glRasterPos2iv_FORMAT;
    packed_data->func = glRasterPos2iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos2s
static inline void push_glRasterPos2s(glRasterPos2s_ARG_EXPAND) {
    glRasterPos2s_PACKED *packed_data = malloc(sizeof(glRasterPos2s_PACKED));
    packed_data->format = glRasterPos2s_FORMAT;
    packed_data->func = glRasterPos2s;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos2sv
static inline void push_glRasterPos2sv(glRasterPos2sv_ARG_EXPAND) {
    glRasterPos2sv_PACKED *packed_data = malloc(sizeof(glRasterPos2sv_PACKED));
    packed_data->format = glRasterPos2sv_FORMAT;
    packed_data->func = glRasterPos2sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos3d
static inline void push_glRasterPos3d(glRasterPos3d_ARG_EXPAND) {
    glRasterPos3d_PACKED *packed_data = malloc(sizeof(glRasterPos3d_PACKED));
    packed_data->format = glRasterPos3d_FORMAT;
    packed_data->func = glRasterPos3d;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos3dv
static inline void push_glRasterPos3dv(glRasterPos3dv_ARG_EXPAND) {
    glRasterPos3dv_PACKED *packed_data = malloc(sizeof(glRasterPos3dv_PACKED));
    packed_data->format = glRasterPos3dv_FORMAT;
    packed_data->func = glRasterPos3dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos3f
static inline void push_glRasterPos3f(glRasterPos3f_ARG_EXPAND) {
    glRasterPos3f_PACKED *packed_data = malloc(sizeof(glRasterPos3f_PACKED));
    packed_data->format = glRasterPos3f_FORMAT;
    packed_data->func = glRasterPos3f;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos3fv
static inline void push_glRasterPos3fv(glRasterPos3fv_ARG_EXPAND) {
    glRasterPos3fv_PACKED *packed_data = malloc(sizeof(glRasterPos3fv_PACKED));
    packed_data->format = glRasterPos3fv_FORMAT;
    packed_data->func = glRasterPos3fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos3i
static inline void push_glRasterPos3i(glRasterPos3i_ARG_EXPAND) {
    glRasterPos3i_PACKED *packed_data = malloc(sizeof(glRasterPos3i_PACKED));
    packed_data->format = glRasterPos3i_FORMAT;
    packed_data->func = glRasterPos3i;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos3iv
static inline void push_glRasterPos3iv(glRasterPos3iv_ARG_EXPAND) {
    glRasterPos3iv_PACKED *packed_data = malloc(sizeof(glRasterPos3iv_PACKED));
    packed_data->format = glRasterPos3iv_FORMAT;
    packed_data->func = glRasterPos3iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos3s
static inline void push_glRasterPos3s(glRasterPos3s_ARG_EXPAND) {
    glRasterPos3s_PACKED *packed_data = malloc(sizeof(glRasterPos3s_PACKED));
    packed_data->format = glRasterPos3s_FORMAT;
    packed_data->func = glRasterPos3s;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos3sv
static inline void push_glRasterPos3sv(glRasterPos3sv_ARG_EXPAND) {
    glRasterPos3sv_PACKED *packed_data = malloc(sizeof(glRasterPos3sv_PACKED));
    packed_data->format = glRasterPos3sv_FORMAT;
    packed_data->func = glRasterPos3sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos4d
static inline void push_glRasterPos4d(glRasterPos4d_ARG_EXPAND) {
    glRasterPos4d_PACKED *packed_data = malloc(sizeof(glRasterPos4d_PACKED));
    packed_data->format = glRasterPos4d_FORMAT;
    packed_data->func = glRasterPos4d;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    packed_data->args.a4 = w;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos4dv
static inline void push_glRasterPos4dv(glRasterPos4dv_ARG_EXPAND) {
    glRasterPos4dv_PACKED *packed_data = malloc(sizeof(glRasterPos4dv_PACKED));
    packed_data->format = glRasterPos4dv_FORMAT;
    packed_data->func = glRasterPos4dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos4f
static inline void push_glRasterPos4f(glRasterPos4f_ARG_EXPAND) {
    glRasterPos4f_PACKED *packed_data = malloc(sizeof(glRasterPos4f_PACKED));
    packed_data->format = glRasterPos4f_FORMAT;
    packed_data->func = glRasterPos4f;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    packed_data->args.a4 = w;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos4fv
static inline void push_glRasterPos4fv(glRasterPos4fv_ARG_EXPAND) {
    glRasterPos4fv_PACKED *packed_data = malloc(sizeof(glRasterPos4fv_PACKED));
    packed_data->format = glRasterPos4fv_FORMAT;
    packed_data->func = glRasterPos4fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos4i
static inline void push_glRasterPos4i(glRasterPos4i_ARG_EXPAND) {
    glRasterPos4i_PACKED *packed_data = malloc(sizeof(glRasterPos4i_PACKED));
    packed_data->format = glRasterPos4i_FORMAT;
    packed_data->func = glRasterPos4i;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    packed_data->args.a4 = w;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos4iv
static inline void push_glRasterPos4iv(glRasterPos4iv_ARG_EXPAND) {
    glRasterPos4iv_PACKED *packed_data = malloc(sizeof(glRasterPos4iv_PACKED));
    packed_data->format = glRasterPos4iv_FORMAT;
    packed_data->func = glRasterPos4iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos4s
static inline void push_glRasterPos4s(glRasterPos4s_ARG_EXPAND) {
    glRasterPos4s_PACKED *packed_data = malloc(sizeof(glRasterPos4s_PACKED));
    packed_data->format = glRasterPos4s_FORMAT;
    packed_data->func = glRasterPos4s;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    packed_data->args.a4 = w;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRasterPos4sv
static inline void push_glRasterPos4sv(glRasterPos4sv_ARG_EXPAND) {
    glRasterPos4sv_PACKED *packed_data = malloc(sizeof(glRasterPos4sv_PACKED));
    packed_data->format = glRasterPos4sv_FORMAT;
    packed_data->func = glRasterPos4sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glReadBuffer
static inline void push_glReadBuffer(glReadBuffer_ARG_EXPAND) {
    glReadBuffer_PACKED *packed_data = malloc(sizeof(glReadBuffer_PACKED));
    packed_data->format = glReadBuffer_FORMAT;
    packed_data->func = glReadBuffer;
    packed_data->args.a1 = mode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glReadPixels
static inline void push_glReadPixels(glReadPixels_ARG_EXPAND) {
    glReadPixels_PACKED *packed_data = malloc(sizeof(glReadPixels_PACKED));
    packed_data->format = glReadPixels_FORMAT;
    packed_data->func = glReadPixels;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = width;
    packed_data->args.a4 = height;
    packed_data->args.a5 = format;
    packed_data->args.a6 = type;
    packed_data->args.a7 = pixels;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRectd
static inline void push_glRectd(glRectd_ARG_EXPAND) {
    glRectd_PACKED *packed_data = malloc(sizeof(glRectd_PACKED));
    packed_data->format = glRectd_FORMAT;
    packed_data->func = glRectd;
    packed_data->args.a1 = x1;
    packed_data->args.a2 = y1;
    packed_data->args.a3 = x2;
    packed_data->args.a4 = y2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRectdv
static inline void push_glRectdv(glRectdv_ARG_EXPAND) {
    glRectdv_PACKED *packed_data = malloc(sizeof(glRectdv_PACKED));
    packed_data->format = glRectdv_FORMAT;
    packed_data->func = glRectdv;
    packed_data->args.a1 = v1;
    packed_data->args.a2 = v2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRectf
static inline void push_glRectf(glRectf_ARG_EXPAND) {
    glRectf_PACKED *packed_data = malloc(sizeof(glRectf_PACKED));
    packed_data->format = glRectf_FORMAT;
    packed_data->func = glRectf;
    packed_data->args.a1 = x1;
    packed_data->args.a2 = y1;
    packed_data->args.a3 = x2;
    packed_data->args.a4 = y2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRectfv
static inline void push_glRectfv(glRectfv_ARG_EXPAND) {
    glRectfv_PACKED *packed_data = malloc(sizeof(glRectfv_PACKED));
    packed_data->format = glRectfv_FORMAT;
    packed_data->func = glRectfv;
    packed_data->args.a1 = v1;
    packed_data->args.a2 = v2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRecti
static inline void push_glRecti(glRecti_ARG_EXPAND) {
    glRecti_PACKED *packed_data = malloc(sizeof(glRecti_PACKED));
    packed_data->format = glRecti_FORMAT;
    packed_data->func = glRecti;
    packed_data->args.a1 = x1;
    packed_data->args.a2 = y1;
    packed_data->args.a3 = x2;
    packed_data->args.a4 = y2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRectiv
static inline void push_glRectiv(glRectiv_ARG_EXPAND) {
    glRectiv_PACKED *packed_data = malloc(sizeof(glRectiv_PACKED));
    packed_data->format = glRectiv_FORMAT;
    packed_data->func = glRectiv;
    packed_data->args.a1 = v1;
    packed_data->args.a2 = v2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRects
static inline void push_glRects(glRects_ARG_EXPAND) {
    glRects_PACKED *packed_data = malloc(sizeof(glRects_PACKED));
    packed_data->format = glRects_FORMAT;
    packed_data->func = glRects;
    packed_data->args.a1 = x1;
    packed_data->args.a2 = y1;
    packed_data->args.a3 = x2;
    packed_data->args.a4 = y2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRectsv
static inline void push_glRectsv(glRectsv_ARG_EXPAND) {
    glRectsv_PACKED *packed_data = malloc(sizeof(glRectsv_PACKED));
    packed_data->format = glRectsv_FORMAT;
    packed_data->func = glRectsv;
    packed_data->args.a1 = v1;
    packed_data->args.a2 = v2;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRenderMode
static inline void push_glRenderMode(glRenderMode_ARG_EXPAND) {
    glRenderMode_PACKED *packed_data = malloc(sizeof(glRenderMode_PACKED));
    packed_data->format = glRenderMode_FORMAT;
    packed_data->func = glRenderMode;
    packed_data->args.a1 = mode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glResetHistogram
static inline void push_glResetHistogram(glResetHistogram_ARG_EXPAND) {
    glResetHistogram_PACKED *packed_data = malloc(sizeof(glResetHistogram_PACKED));
    packed_data->format = glResetHistogram_FORMAT;
    packed_data->func = glResetHistogram;
    packed_data->args.a1 = target;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glResetMinmax
static inline void push_glResetMinmax(glResetMinmax_ARG_EXPAND) {
    glResetMinmax_PACKED *packed_data = malloc(sizeof(glResetMinmax_PACKED));
    packed_data->format = glResetMinmax_FORMAT;
    packed_data->func = glResetMinmax;
    packed_data->args.a1 = target;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRotated
static inline void push_glRotated(glRotated_ARG_EXPAND) {
    glRotated_PACKED *packed_data = malloc(sizeof(glRotated_PACKED));
    packed_data->format = glRotated_FORMAT;
    packed_data->func = glRotated;
    packed_data->args.a1 = angle;
    packed_data->args.a2 = x;
    packed_data->args.a3 = y;
    packed_data->args.a4 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glRotatef
static inline void push_glRotatef(glRotatef_ARG_EXPAND) {
    glRotatef_PACKED *packed_data = malloc(sizeof(glRotatef_PACKED));
    packed_data->format = glRotatef_FORMAT;
    packed_data->func = glRotatef;
    packed_data->args.a1 = angle;
    packed_data->args.a2 = x;
    packed_data->args.a3 = y;
    packed_data->args.a4 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSampleCoverage
static inline void push_glSampleCoverage(glSampleCoverage_ARG_EXPAND) {
    glSampleCoverage_PACKED *packed_data = malloc(sizeof(glSampleCoverage_PACKED));
    packed_data->format = glSampleCoverage_FORMAT;
    packed_data->func = glSampleCoverage;
    packed_data->args.a1 = value;
    packed_data->args.a2 = invert;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glScaled
static inline void push_glScaled(glScaled_ARG_EXPAND) {
    glScaled_PACKED *packed_data = malloc(sizeof(glScaled_PACKED));
    packed_data->format = glScaled_FORMAT;
    packed_data->func = glScaled;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glScalef
static inline void push_glScalef(glScalef_ARG_EXPAND) {
    glScalef_PACKED *packed_data = malloc(sizeof(glScalef_PACKED));
    packed_data->format = glScalef_FORMAT;
    packed_data->func = glScalef;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glScissor
static inline void push_glScissor(glScissor_ARG_EXPAND) {
    glScissor_PACKED *packed_data = malloc(sizeof(glScissor_PACKED));
    packed_data->format = glScissor_FORMAT;
    packed_data->func = glScissor;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = width;
    packed_data->args.a4 = height;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3b
static inline void push_glSecondaryColor3b(glSecondaryColor3b_ARG_EXPAND) {
    glSecondaryColor3b_PACKED *packed_data = malloc(sizeof(glSecondaryColor3b_PACKED));
    packed_data->format = glSecondaryColor3b_FORMAT;
    packed_data->func = glSecondaryColor3b;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3bv
static inline void push_glSecondaryColor3bv(glSecondaryColor3bv_ARG_EXPAND) {
    glSecondaryColor3bv_PACKED *packed_data = malloc(sizeof(glSecondaryColor3bv_PACKED));
    packed_data->format = glSecondaryColor3bv_FORMAT;
    packed_data->func = glSecondaryColor3bv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3d
static inline void push_glSecondaryColor3d(glSecondaryColor3d_ARG_EXPAND) {
    glSecondaryColor3d_PACKED *packed_data = malloc(sizeof(glSecondaryColor3d_PACKED));
    packed_data->format = glSecondaryColor3d_FORMAT;
    packed_data->func = glSecondaryColor3d;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3dv
static inline void push_glSecondaryColor3dv(glSecondaryColor3dv_ARG_EXPAND) {
    glSecondaryColor3dv_PACKED *packed_data = malloc(sizeof(glSecondaryColor3dv_PACKED));
    packed_data->format = glSecondaryColor3dv_FORMAT;
    packed_data->func = glSecondaryColor3dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3f
static inline void push_glSecondaryColor3f(glSecondaryColor3f_ARG_EXPAND) {
    glSecondaryColor3f_PACKED *packed_data = malloc(sizeof(glSecondaryColor3f_PACKED));
    packed_data->format = glSecondaryColor3f_FORMAT;
    packed_data->func = glSecondaryColor3f;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3fv
static inline void push_glSecondaryColor3fv(glSecondaryColor3fv_ARG_EXPAND) {
    glSecondaryColor3fv_PACKED *packed_data = malloc(sizeof(glSecondaryColor3fv_PACKED));
    packed_data->format = glSecondaryColor3fv_FORMAT;
    packed_data->func = glSecondaryColor3fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3i
static inline void push_glSecondaryColor3i(glSecondaryColor3i_ARG_EXPAND) {
    glSecondaryColor3i_PACKED *packed_data = malloc(sizeof(glSecondaryColor3i_PACKED));
    packed_data->format = glSecondaryColor3i_FORMAT;
    packed_data->func = glSecondaryColor3i;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3iv
static inline void push_glSecondaryColor3iv(glSecondaryColor3iv_ARG_EXPAND) {
    glSecondaryColor3iv_PACKED *packed_data = malloc(sizeof(glSecondaryColor3iv_PACKED));
    packed_data->format = glSecondaryColor3iv_FORMAT;
    packed_data->func = glSecondaryColor3iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3s
static inline void push_glSecondaryColor3s(glSecondaryColor3s_ARG_EXPAND) {
    glSecondaryColor3s_PACKED *packed_data = malloc(sizeof(glSecondaryColor3s_PACKED));
    packed_data->format = glSecondaryColor3s_FORMAT;
    packed_data->func = glSecondaryColor3s;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3sv
static inline void push_glSecondaryColor3sv(glSecondaryColor3sv_ARG_EXPAND) {
    glSecondaryColor3sv_PACKED *packed_data = malloc(sizeof(glSecondaryColor3sv_PACKED));
    packed_data->format = glSecondaryColor3sv_FORMAT;
    packed_data->func = glSecondaryColor3sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3ub
static inline void push_glSecondaryColor3ub(glSecondaryColor3ub_ARG_EXPAND) {
    glSecondaryColor3ub_PACKED *packed_data = malloc(sizeof(glSecondaryColor3ub_PACKED));
    packed_data->format = glSecondaryColor3ub_FORMAT;
    packed_data->func = glSecondaryColor3ub;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3ubv
static inline void push_glSecondaryColor3ubv(glSecondaryColor3ubv_ARG_EXPAND) {
    glSecondaryColor3ubv_PACKED *packed_data = malloc(sizeof(glSecondaryColor3ubv_PACKED));
    packed_data->format = glSecondaryColor3ubv_FORMAT;
    packed_data->func = glSecondaryColor3ubv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3ui
static inline void push_glSecondaryColor3ui(glSecondaryColor3ui_ARG_EXPAND) {
    glSecondaryColor3ui_PACKED *packed_data = malloc(sizeof(glSecondaryColor3ui_PACKED));
    packed_data->format = glSecondaryColor3ui_FORMAT;
    packed_data->func = glSecondaryColor3ui;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3uiv
static inline void push_glSecondaryColor3uiv(glSecondaryColor3uiv_ARG_EXPAND) {
    glSecondaryColor3uiv_PACKED *packed_data = malloc(sizeof(glSecondaryColor3uiv_PACKED));
    packed_data->format = glSecondaryColor3uiv_FORMAT;
    packed_data->func = glSecondaryColor3uiv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3us
static inline void push_glSecondaryColor3us(glSecondaryColor3us_ARG_EXPAND) {
    glSecondaryColor3us_PACKED *packed_data = malloc(sizeof(glSecondaryColor3us_PACKED));
    packed_data->format = glSecondaryColor3us_FORMAT;
    packed_data->func = glSecondaryColor3us;
    packed_data->args.a1 = red;
    packed_data->args.a2 = green;
    packed_data->args.a3 = blue;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColor3usv
static inline void push_glSecondaryColor3usv(glSecondaryColor3usv_ARG_EXPAND) {
    glSecondaryColor3usv_PACKED *packed_data = malloc(sizeof(glSecondaryColor3usv_PACKED));
    packed_data->format = glSecondaryColor3usv_FORMAT;
    packed_data->func = glSecondaryColor3usv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSecondaryColorPointer
static inline void push_glSecondaryColorPointer(glSecondaryColorPointer_ARG_EXPAND) {
    glSecondaryColorPointer_PACKED *packed_data = malloc(sizeof(glSecondaryColorPointer_PACKED));
    packed_data->format = glSecondaryColorPointer_FORMAT;
    packed_data->func = glSecondaryColorPointer;
    packed_data->args.a1 = size;
    packed_data->args.a2 = type;
    packed_data->args.a3 = stride;
    packed_data->args.a4 = pointer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSelectBuffer
static inline void push_glSelectBuffer(glSelectBuffer_ARG_EXPAND) {
    glSelectBuffer_PACKED *packed_data = malloc(sizeof(glSelectBuffer_PACKED));
    packed_data->format = glSelectBuffer_FORMAT;
    packed_data->func = glSelectBuffer;
    packed_data->args.a1 = size;
    packed_data->args.a2 = buffer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glSeparableFilter2D
static inline void push_glSeparableFilter2D(glSeparableFilter2D_ARG_EXPAND) {
    glSeparableFilter2D_PACKED *packed_data = malloc(sizeof(glSeparableFilter2D_PACKED));
    packed_data->format = glSeparableFilter2D_FORMAT;
    packed_data->func = glSeparableFilter2D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = internalformat;
    packed_data->args.a3 = width;
    packed_data->args.a4 = height;
    packed_data->args.a5 = format;
    packed_data->args.a6 = type;
    packed_data->args.a7 = row;
    packed_data->args.a8 = column;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glShadeModel
static inline void push_glShadeModel(glShadeModel_ARG_EXPAND) {
    glShadeModel_PACKED *packed_data = malloc(sizeof(glShadeModel_PACKED));
    packed_data->format = glShadeModel_FORMAT;
    packed_data->func = glShadeModel;
    packed_data->args.a1 = mode;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glStencilFunc
static inline void push_glStencilFunc(glStencilFunc_ARG_EXPAND) {
    glStencilFunc_PACKED *packed_data = malloc(sizeof(glStencilFunc_PACKED));
    packed_data->format = glStencilFunc_FORMAT;
    packed_data->func = glStencilFunc;
    packed_data->args.a1 = func;
    packed_data->args.a2 = ref;
    packed_data->args.a3 = mask;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glStencilMask
static inline void push_glStencilMask(glStencilMask_ARG_EXPAND) {
    glStencilMask_PACKED *packed_data = malloc(sizeof(glStencilMask_PACKED));
    packed_data->format = glStencilMask_FORMAT;
    packed_data->func = glStencilMask;
    packed_data->args.a1 = mask;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glStencilOp
static inline void push_glStencilOp(glStencilOp_ARG_EXPAND) {
    glStencilOp_PACKED *packed_data = malloc(sizeof(glStencilOp_PACKED));
    packed_data->format = glStencilOp_FORMAT;
    packed_data->func = glStencilOp;
    packed_data->args.a1 = fail;
    packed_data->args.a2 = zfail;
    packed_data->args.a3 = zpass;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord1d
static inline void push_glTexCoord1d(glTexCoord1d_ARG_EXPAND) {
    glTexCoord1d_PACKED *packed_data = malloc(sizeof(glTexCoord1d_PACKED));
    packed_data->format = glTexCoord1d_FORMAT;
    packed_data->func = glTexCoord1d;
    packed_data->args.a1 = s;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord1dv
static inline void push_glTexCoord1dv(glTexCoord1dv_ARG_EXPAND) {
    glTexCoord1dv_PACKED *packed_data = malloc(sizeof(glTexCoord1dv_PACKED));
    packed_data->format = glTexCoord1dv_FORMAT;
    packed_data->func = glTexCoord1dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord1f
static inline void push_glTexCoord1f(glTexCoord1f_ARG_EXPAND) {
    glTexCoord1f_PACKED *packed_data = malloc(sizeof(glTexCoord1f_PACKED));
    packed_data->format = glTexCoord1f_FORMAT;
    packed_data->func = glTexCoord1f;
    packed_data->args.a1 = s;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord1fv
static inline void push_glTexCoord1fv(glTexCoord1fv_ARG_EXPAND) {
    glTexCoord1fv_PACKED *packed_data = malloc(sizeof(glTexCoord1fv_PACKED));
    packed_data->format = glTexCoord1fv_FORMAT;
    packed_data->func = glTexCoord1fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord1i
static inline void push_glTexCoord1i(glTexCoord1i_ARG_EXPAND) {
    glTexCoord1i_PACKED *packed_data = malloc(sizeof(glTexCoord1i_PACKED));
    packed_data->format = glTexCoord1i_FORMAT;
    packed_data->func = glTexCoord1i;
    packed_data->args.a1 = s;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord1iv
static inline void push_glTexCoord1iv(glTexCoord1iv_ARG_EXPAND) {
    glTexCoord1iv_PACKED *packed_data = malloc(sizeof(glTexCoord1iv_PACKED));
    packed_data->format = glTexCoord1iv_FORMAT;
    packed_data->func = glTexCoord1iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord1s
static inline void push_glTexCoord1s(glTexCoord1s_ARG_EXPAND) {
    glTexCoord1s_PACKED *packed_data = malloc(sizeof(glTexCoord1s_PACKED));
    packed_data->format = glTexCoord1s_FORMAT;
    packed_data->func = glTexCoord1s;
    packed_data->args.a1 = s;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord1sv
static inline void push_glTexCoord1sv(glTexCoord1sv_ARG_EXPAND) {
    glTexCoord1sv_PACKED *packed_data = malloc(sizeof(glTexCoord1sv_PACKED));
    packed_data->format = glTexCoord1sv_FORMAT;
    packed_data->func = glTexCoord1sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord2d
static inline void push_glTexCoord2d(glTexCoord2d_ARG_EXPAND) {
    glTexCoord2d_PACKED *packed_data = malloc(sizeof(glTexCoord2d_PACKED));
    packed_data->format = glTexCoord2d_FORMAT;
    packed_data->func = glTexCoord2d;
    packed_data->args.a1 = s;
    packed_data->args.a2 = t;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord2dv
static inline void push_glTexCoord2dv(glTexCoord2dv_ARG_EXPAND) {
    glTexCoord2dv_PACKED *packed_data = malloc(sizeof(glTexCoord2dv_PACKED));
    packed_data->format = glTexCoord2dv_FORMAT;
    packed_data->func = glTexCoord2dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord2f
static inline void push_glTexCoord2f(glTexCoord2f_ARG_EXPAND) {
    glTexCoord2f_PACKED *packed_data = malloc(sizeof(glTexCoord2f_PACKED));
    packed_data->format = glTexCoord2f_FORMAT;
    packed_data->func = glTexCoord2f;
    packed_data->args.a1 = s;
    packed_data->args.a2 = t;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord2fv
static inline void push_glTexCoord2fv(glTexCoord2fv_ARG_EXPAND) {
    glTexCoord2fv_PACKED *packed_data = malloc(sizeof(glTexCoord2fv_PACKED));
    packed_data->format = glTexCoord2fv_FORMAT;
    packed_data->func = glTexCoord2fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord2i
static inline void push_glTexCoord2i(glTexCoord2i_ARG_EXPAND) {
    glTexCoord2i_PACKED *packed_data = malloc(sizeof(glTexCoord2i_PACKED));
    packed_data->format = glTexCoord2i_FORMAT;
    packed_data->func = glTexCoord2i;
    packed_data->args.a1 = s;
    packed_data->args.a2 = t;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord2iv
static inline void push_glTexCoord2iv(glTexCoord2iv_ARG_EXPAND) {
    glTexCoord2iv_PACKED *packed_data = malloc(sizeof(glTexCoord2iv_PACKED));
    packed_data->format = glTexCoord2iv_FORMAT;
    packed_data->func = glTexCoord2iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord2s
static inline void push_glTexCoord2s(glTexCoord2s_ARG_EXPAND) {
    glTexCoord2s_PACKED *packed_data = malloc(sizeof(glTexCoord2s_PACKED));
    packed_data->format = glTexCoord2s_FORMAT;
    packed_data->func = glTexCoord2s;
    packed_data->args.a1 = s;
    packed_data->args.a2 = t;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord2sv
static inline void push_glTexCoord2sv(glTexCoord2sv_ARG_EXPAND) {
    glTexCoord2sv_PACKED *packed_data = malloc(sizeof(glTexCoord2sv_PACKED));
    packed_data->format = glTexCoord2sv_FORMAT;
    packed_data->func = glTexCoord2sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord3d
static inline void push_glTexCoord3d(glTexCoord3d_ARG_EXPAND) {
    glTexCoord3d_PACKED *packed_data = malloc(sizeof(glTexCoord3d_PACKED));
    packed_data->format = glTexCoord3d_FORMAT;
    packed_data->func = glTexCoord3d;
    packed_data->args.a1 = s;
    packed_data->args.a2 = t;
    packed_data->args.a3 = r;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord3dv
static inline void push_glTexCoord3dv(glTexCoord3dv_ARG_EXPAND) {
    glTexCoord3dv_PACKED *packed_data = malloc(sizeof(glTexCoord3dv_PACKED));
    packed_data->format = glTexCoord3dv_FORMAT;
    packed_data->func = glTexCoord3dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord3f
static inline void push_glTexCoord3f(glTexCoord3f_ARG_EXPAND) {
    glTexCoord3f_PACKED *packed_data = malloc(sizeof(glTexCoord3f_PACKED));
    packed_data->format = glTexCoord3f_FORMAT;
    packed_data->func = glTexCoord3f;
    packed_data->args.a1 = s;
    packed_data->args.a2 = t;
    packed_data->args.a3 = r;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord3fv
static inline void push_glTexCoord3fv(glTexCoord3fv_ARG_EXPAND) {
    glTexCoord3fv_PACKED *packed_data = malloc(sizeof(glTexCoord3fv_PACKED));
    packed_data->format = glTexCoord3fv_FORMAT;
    packed_data->func = glTexCoord3fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord3i
static inline void push_glTexCoord3i(glTexCoord3i_ARG_EXPAND) {
    glTexCoord3i_PACKED *packed_data = malloc(sizeof(glTexCoord3i_PACKED));
    packed_data->format = glTexCoord3i_FORMAT;
    packed_data->func = glTexCoord3i;
    packed_data->args.a1 = s;
    packed_data->args.a2 = t;
    packed_data->args.a3 = r;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord3iv
static inline void push_glTexCoord3iv(glTexCoord3iv_ARG_EXPAND) {
    glTexCoord3iv_PACKED *packed_data = malloc(sizeof(glTexCoord3iv_PACKED));
    packed_data->format = glTexCoord3iv_FORMAT;
    packed_data->func = glTexCoord3iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord3s
static inline void push_glTexCoord3s(glTexCoord3s_ARG_EXPAND) {
    glTexCoord3s_PACKED *packed_data = malloc(sizeof(glTexCoord3s_PACKED));
    packed_data->format = glTexCoord3s_FORMAT;
    packed_data->func = glTexCoord3s;
    packed_data->args.a1 = s;
    packed_data->args.a2 = t;
    packed_data->args.a3 = r;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord3sv
static inline void push_glTexCoord3sv(glTexCoord3sv_ARG_EXPAND) {
    glTexCoord3sv_PACKED *packed_data = malloc(sizeof(glTexCoord3sv_PACKED));
    packed_data->format = glTexCoord3sv_FORMAT;
    packed_data->func = glTexCoord3sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord4d
static inline void push_glTexCoord4d(glTexCoord4d_ARG_EXPAND) {
    glTexCoord4d_PACKED *packed_data = malloc(sizeof(glTexCoord4d_PACKED));
    packed_data->format = glTexCoord4d_FORMAT;
    packed_data->func = glTexCoord4d;
    packed_data->args.a1 = s;
    packed_data->args.a2 = t;
    packed_data->args.a3 = r;
    packed_data->args.a4 = q;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord4dv
static inline void push_glTexCoord4dv(glTexCoord4dv_ARG_EXPAND) {
    glTexCoord4dv_PACKED *packed_data = malloc(sizeof(glTexCoord4dv_PACKED));
    packed_data->format = glTexCoord4dv_FORMAT;
    packed_data->func = glTexCoord4dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord4f
static inline void push_glTexCoord4f(glTexCoord4f_ARG_EXPAND) {
    glTexCoord4f_PACKED *packed_data = malloc(sizeof(glTexCoord4f_PACKED));
    packed_data->format = glTexCoord4f_FORMAT;
    packed_data->func = glTexCoord4f;
    packed_data->args.a1 = s;
    packed_data->args.a2 = t;
    packed_data->args.a3 = r;
    packed_data->args.a4 = q;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord4fv
static inline void push_glTexCoord4fv(glTexCoord4fv_ARG_EXPAND) {
    glTexCoord4fv_PACKED *packed_data = malloc(sizeof(glTexCoord4fv_PACKED));
    packed_data->format = glTexCoord4fv_FORMAT;
    packed_data->func = glTexCoord4fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord4i
static inline void push_glTexCoord4i(glTexCoord4i_ARG_EXPAND) {
    glTexCoord4i_PACKED *packed_data = malloc(sizeof(glTexCoord4i_PACKED));
    packed_data->format = glTexCoord4i_FORMAT;
    packed_data->func = glTexCoord4i;
    packed_data->args.a1 = s;
    packed_data->args.a2 = t;
    packed_data->args.a3 = r;
    packed_data->args.a4 = q;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord4iv
static inline void push_glTexCoord4iv(glTexCoord4iv_ARG_EXPAND) {
    glTexCoord4iv_PACKED *packed_data = malloc(sizeof(glTexCoord4iv_PACKED));
    packed_data->format = glTexCoord4iv_FORMAT;
    packed_data->func = glTexCoord4iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord4s
static inline void push_glTexCoord4s(glTexCoord4s_ARG_EXPAND) {
    glTexCoord4s_PACKED *packed_data = malloc(sizeof(glTexCoord4s_PACKED));
    packed_data->format = glTexCoord4s_FORMAT;
    packed_data->func = glTexCoord4s;
    packed_data->args.a1 = s;
    packed_data->args.a2 = t;
    packed_data->args.a3 = r;
    packed_data->args.a4 = q;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoord4sv
static inline void push_glTexCoord4sv(glTexCoord4sv_ARG_EXPAND) {
    glTexCoord4sv_PACKED *packed_data = malloc(sizeof(glTexCoord4sv_PACKED));
    packed_data->format = glTexCoord4sv_FORMAT;
    packed_data->func = glTexCoord4sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexCoordPointer
static inline void push_glTexCoordPointer(glTexCoordPointer_ARG_EXPAND) {
    glTexCoordPointer_PACKED *packed_data = malloc(sizeof(glTexCoordPointer_PACKED));
    packed_data->format = glTexCoordPointer_FORMAT;
    packed_data->func = glTexCoordPointer;
    packed_data->args.a1 = size;
    packed_data->args.a2 = type;
    packed_data->args.a3 = stride;
    packed_data->args.a4 = pointer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexEnvf
static inline void push_glTexEnvf(glTexEnvf_ARG_EXPAND) {
    glTexEnvf_PACKED *packed_data = malloc(sizeof(glTexEnvf_PACKED));
    packed_data->format = glTexEnvf_FORMAT;
    packed_data->func = glTexEnvf;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexEnvfv
static inline void push_glTexEnvfv(glTexEnvfv_ARG_EXPAND) {
    glTexEnvfv_PACKED *packed_data = malloc(sizeof(glTexEnvfv_PACKED));
    packed_data->format = glTexEnvfv_FORMAT;
    packed_data->func = glTexEnvfv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexEnvi
static inline void push_glTexEnvi(glTexEnvi_ARG_EXPAND) {
    glTexEnvi_PACKED *packed_data = malloc(sizeof(glTexEnvi_PACKED));
    packed_data->format = glTexEnvi_FORMAT;
    packed_data->func = glTexEnvi;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexEnviv
static inline void push_glTexEnviv(glTexEnviv_ARG_EXPAND) {
    glTexEnviv_PACKED *packed_data = malloc(sizeof(glTexEnviv_PACKED));
    packed_data->format = glTexEnviv_FORMAT;
    packed_data->func = glTexEnviv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexGend
static inline void push_glTexGend(glTexGend_ARG_EXPAND) {
    glTexGend_PACKED *packed_data = malloc(sizeof(glTexGend_PACKED));
    packed_data->format = glTexGend_FORMAT;
    packed_data->func = glTexGend;
    packed_data->args.a1 = coord;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexGendv
static inline void push_glTexGendv(glTexGendv_ARG_EXPAND) {
    glTexGendv_PACKED *packed_data = malloc(sizeof(glTexGendv_PACKED));
    packed_data->format = glTexGendv_FORMAT;
    packed_data->func = glTexGendv;
    packed_data->args.a1 = coord;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexGenf
static inline void push_glTexGenf(glTexGenf_ARG_EXPAND) {
    glTexGenf_PACKED *packed_data = malloc(sizeof(glTexGenf_PACKED));
    packed_data->format = glTexGenf_FORMAT;
    packed_data->func = glTexGenf;
    packed_data->args.a1 = coord;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexGenfv
static inline void push_glTexGenfv(glTexGenfv_ARG_EXPAND) {
    glTexGenfv_PACKED *packed_data = malloc(sizeof(glTexGenfv_PACKED));
    packed_data->format = glTexGenfv_FORMAT;
    packed_data->func = glTexGenfv;
    packed_data->args.a1 = coord;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexGeni
static inline void push_glTexGeni(glTexGeni_ARG_EXPAND) {
    glTexGeni_PACKED *packed_data = malloc(sizeof(glTexGeni_PACKED));
    packed_data->format = glTexGeni_FORMAT;
    packed_data->func = glTexGeni;
    packed_data->args.a1 = coord;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexGeniv
static inline void push_glTexGeniv(glTexGeniv_ARG_EXPAND) {
    glTexGeniv_PACKED *packed_data = malloc(sizeof(glTexGeniv_PACKED));
    packed_data->format = glTexGeniv_FORMAT;
    packed_data->func = glTexGeniv;
    packed_data->args.a1 = coord;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexImage1D
static inline void push_glTexImage1D(glTexImage1D_ARG_EXPAND) {
    glTexImage1D_PACKED *packed_data = malloc(sizeof(glTexImage1D_PACKED));
    packed_data->format = glTexImage1D_FORMAT;
    packed_data->func = glTexImage1D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = internalformat;
    packed_data->args.a4 = width;
    packed_data->args.a5 = border;
    packed_data->args.a6 = format;
    packed_data->args.a7 = type;
    packed_data->args.a8 = pixels;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexImage2D
static inline void push_glTexImage2D(glTexImage2D_ARG_EXPAND) {
    glTexImage2D_PACKED *packed_data = malloc(sizeof(glTexImage2D_PACKED));
    packed_data->format = glTexImage2D_FORMAT;
    packed_data->func = glTexImage2D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = internalformat;
    packed_data->args.a4 = width;
    packed_data->args.a5 = height;
    packed_data->args.a6 = border;
    packed_data->args.a7 = format;
    packed_data->args.a8 = type;
    packed_data->args.a9 = pixels;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexImage3D
static inline void push_glTexImage3D(glTexImage3D_ARG_EXPAND) {
    glTexImage3D_PACKED *packed_data = malloc(sizeof(glTexImage3D_PACKED));
    packed_data->format = glTexImage3D_FORMAT;
    packed_data->func = glTexImage3D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = internalformat;
    packed_data->args.a4 = width;
    packed_data->args.a5 = height;
    packed_data->args.a6 = depth;
    packed_data->args.a7 = border;
    packed_data->args.a8 = format;
    packed_data->args.a9 = type;
    packed_data->args.a10 = pixels;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexParameterf
static inline void push_glTexParameterf(glTexParameterf_ARG_EXPAND) {
    glTexParameterf_PACKED *packed_data = malloc(sizeof(glTexParameterf_PACKED));
    packed_data->format = glTexParameterf_FORMAT;
    packed_data->func = glTexParameterf;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexParameterfv
static inline void push_glTexParameterfv(glTexParameterfv_ARG_EXPAND) {
    glTexParameterfv_PACKED *packed_data = malloc(sizeof(glTexParameterfv_PACKED));
    packed_data->format = glTexParameterfv_FORMAT;
    packed_data->func = glTexParameterfv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexParameteri
static inline void push_glTexParameteri(glTexParameteri_ARG_EXPAND) {
    glTexParameteri_PACKED *packed_data = malloc(sizeof(glTexParameteri_PACKED));
    packed_data->format = glTexParameteri_FORMAT;
    packed_data->func = glTexParameteri;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = param;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexParameteriv
static inline void push_glTexParameteriv(glTexParameteriv_ARG_EXPAND) {
    glTexParameteriv_PACKED *packed_data = malloc(sizeof(glTexParameteriv_PACKED));
    packed_data->format = glTexParameteriv_FORMAT;
    packed_data->func = glTexParameteriv;
    packed_data->args.a1 = target;
    packed_data->args.a2 = pname;
    packed_data->args.a3 = params;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexSubImage1D
static inline void push_glTexSubImage1D(glTexSubImage1D_ARG_EXPAND) {
    glTexSubImage1D_PACKED *packed_data = malloc(sizeof(glTexSubImage1D_PACKED));
    packed_data->format = glTexSubImage1D_FORMAT;
    packed_data->func = glTexSubImage1D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = xoffset;
    packed_data->args.a4 = width;
    packed_data->args.a5 = format;
    packed_data->args.a6 = type;
    packed_data->args.a7 = pixels;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexSubImage2D
static inline void push_glTexSubImage2D(glTexSubImage2D_ARG_EXPAND) {
    glTexSubImage2D_PACKED *packed_data = malloc(sizeof(glTexSubImage2D_PACKED));
    packed_data->format = glTexSubImage2D_FORMAT;
    packed_data->func = glTexSubImage2D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = xoffset;
    packed_data->args.a4 = yoffset;
    packed_data->args.a5 = width;
    packed_data->args.a6 = height;
    packed_data->args.a7 = format;
    packed_data->args.a8 = type;
    packed_data->args.a9 = pixels;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTexSubImage3D
static inline void push_glTexSubImage3D(glTexSubImage3D_ARG_EXPAND) {
    glTexSubImage3D_PACKED *packed_data = malloc(sizeof(glTexSubImage3D_PACKED));
    packed_data->format = glTexSubImage3D_FORMAT;
    packed_data->func = glTexSubImage3D;
    packed_data->args.a1 = target;
    packed_data->args.a2 = level;
    packed_data->args.a3 = xoffset;
    packed_data->args.a4 = yoffset;
    packed_data->args.a5 = zoffset;
    packed_data->args.a6 = width;
    packed_data->args.a7 = height;
    packed_data->args.a8 = depth;
    packed_data->args.a9 = format;
    packed_data->args.a10 = type;
    packed_data->args.a11 = pixels;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTranslated
static inline void push_glTranslated(glTranslated_ARG_EXPAND) {
    glTranslated_PACKED *packed_data = malloc(sizeof(glTranslated_PACKED));
    packed_data->format = glTranslated_FORMAT;
    packed_data->func = glTranslated;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glTranslatef
static inline void push_glTranslatef(glTranslatef_ARG_EXPAND) {
    glTranslatef_PACKED *packed_data = malloc(sizeof(glTranslatef_PACKED));
    packed_data->format = glTranslatef_FORMAT;
    packed_data->func = glTranslatef;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glUnmapBuffer
static inline void push_glUnmapBuffer(glUnmapBuffer_ARG_EXPAND) {
    glUnmapBuffer_PACKED *packed_data = malloc(sizeof(glUnmapBuffer_PACKED));
    packed_data->format = glUnmapBuffer_FORMAT;
    packed_data->func = glUnmapBuffer;
    packed_data->args.a1 = target;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex2d
static inline void push_glVertex2d(glVertex2d_ARG_EXPAND) {
    glVertex2d_PACKED *packed_data = malloc(sizeof(glVertex2d_PACKED));
    packed_data->format = glVertex2d_FORMAT;
    packed_data->func = glVertex2d;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex2dv
static inline void push_glVertex2dv(glVertex2dv_ARG_EXPAND) {
    glVertex2dv_PACKED *packed_data = malloc(sizeof(glVertex2dv_PACKED));
    packed_data->format = glVertex2dv_FORMAT;
    packed_data->func = glVertex2dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex2f
static inline void push_glVertex2f(glVertex2f_ARG_EXPAND) {
    glVertex2f_PACKED *packed_data = malloc(sizeof(glVertex2f_PACKED));
    packed_data->format = glVertex2f_FORMAT;
    packed_data->func = glVertex2f;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex2fv
static inline void push_glVertex2fv(glVertex2fv_ARG_EXPAND) {
    glVertex2fv_PACKED *packed_data = malloc(sizeof(glVertex2fv_PACKED));
    packed_data->format = glVertex2fv_FORMAT;
    packed_data->func = glVertex2fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex2i
static inline void push_glVertex2i(glVertex2i_ARG_EXPAND) {
    glVertex2i_PACKED *packed_data = malloc(sizeof(glVertex2i_PACKED));
    packed_data->format = glVertex2i_FORMAT;
    packed_data->func = glVertex2i;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex2iv
static inline void push_glVertex2iv(glVertex2iv_ARG_EXPAND) {
    glVertex2iv_PACKED *packed_data = malloc(sizeof(glVertex2iv_PACKED));
    packed_data->format = glVertex2iv_FORMAT;
    packed_data->func = glVertex2iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex2s
static inline void push_glVertex2s(glVertex2s_ARG_EXPAND) {
    glVertex2s_PACKED *packed_data = malloc(sizeof(glVertex2s_PACKED));
    packed_data->format = glVertex2s_FORMAT;
    packed_data->func = glVertex2s;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex2sv
static inline void push_glVertex2sv(glVertex2sv_ARG_EXPAND) {
    glVertex2sv_PACKED *packed_data = malloc(sizeof(glVertex2sv_PACKED));
    packed_data->format = glVertex2sv_FORMAT;
    packed_data->func = glVertex2sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex3d
static inline void push_glVertex3d(glVertex3d_ARG_EXPAND) {
    glVertex3d_PACKED *packed_data = malloc(sizeof(glVertex3d_PACKED));
    packed_data->format = glVertex3d_FORMAT;
    packed_data->func = glVertex3d;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex3dv
static inline void push_glVertex3dv(glVertex3dv_ARG_EXPAND) {
    glVertex3dv_PACKED *packed_data = malloc(sizeof(glVertex3dv_PACKED));
    packed_data->format = glVertex3dv_FORMAT;
    packed_data->func = glVertex3dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex3f
static inline void push_glVertex3f(glVertex3f_ARG_EXPAND) {
    glVertex3f_PACKED *packed_data = malloc(sizeof(glVertex3f_PACKED));
    packed_data->format = glVertex3f_FORMAT;
    packed_data->func = glVertex3f;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex3fv
static inline void push_glVertex3fv(glVertex3fv_ARG_EXPAND) {
    glVertex3fv_PACKED *packed_data = malloc(sizeof(glVertex3fv_PACKED));
    packed_data->format = glVertex3fv_FORMAT;
    packed_data->func = glVertex3fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex3i
static inline void push_glVertex3i(glVertex3i_ARG_EXPAND) {
    glVertex3i_PACKED *packed_data = malloc(sizeof(glVertex3i_PACKED));
    packed_data->format = glVertex3i_FORMAT;
    packed_data->func = glVertex3i;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex3iv
static inline void push_glVertex3iv(glVertex3iv_ARG_EXPAND) {
    glVertex3iv_PACKED *packed_data = malloc(sizeof(glVertex3iv_PACKED));
    packed_data->format = glVertex3iv_FORMAT;
    packed_data->func = glVertex3iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex3s
static inline void push_glVertex3s(glVertex3s_ARG_EXPAND) {
    glVertex3s_PACKED *packed_data = malloc(sizeof(glVertex3s_PACKED));
    packed_data->format = glVertex3s_FORMAT;
    packed_data->func = glVertex3s;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex3sv
static inline void push_glVertex3sv(glVertex3sv_ARG_EXPAND) {
    glVertex3sv_PACKED *packed_data = malloc(sizeof(glVertex3sv_PACKED));
    packed_data->format = glVertex3sv_FORMAT;
    packed_data->func = glVertex3sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex4d
static inline void push_glVertex4d(glVertex4d_ARG_EXPAND) {
    glVertex4d_PACKED *packed_data = malloc(sizeof(glVertex4d_PACKED));
    packed_data->format = glVertex4d_FORMAT;
    packed_data->func = glVertex4d;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    packed_data->args.a4 = w;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex4dv
static inline void push_glVertex4dv(glVertex4dv_ARG_EXPAND) {
    glVertex4dv_PACKED *packed_data = malloc(sizeof(glVertex4dv_PACKED));
    packed_data->format = glVertex4dv_FORMAT;
    packed_data->func = glVertex4dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex4f
static inline void push_glVertex4f(glVertex4f_ARG_EXPAND) {
    glVertex4f_PACKED *packed_data = malloc(sizeof(glVertex4f_PACKED));
    packed_data->format = glVertex4f_FORMAT;
    packed_data->func = glVertex4f;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    packed_data->args.a4 = w;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex4fv
static inline void push_glVertex4fv(glVertex4fv_ARG_EXPAND) {
    glVertex4fv_PACKED *packed_data = malloc(sizeof(glVertex4fv_PACKED));
    packed_data->format = glVertex4fv_FORMAT;
    packed_data->func = glVertex4fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex4i
static inline void push_glVertex4i(glVertex4i_ARG_EXPAND) {
    glVertex4i_PACKED *packed_data = malloc(sizeof(glVertex4i_PACKED));
    packed_data->format = glVertex4i_FORMAT;
    packed_data->func = glVertex4i;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    packed_data->args.a4 = w;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex4iv
static inline void push_glVertex4iv(glVertex4iv_ARG_EXPAND) {
    glVertex4iv_PACKED *packed_data = malloc(sizeof(glVertex4iv_PACKED));
    packed_data->format = glVertex4iv_FORMAT;
    packed_data->func = glVertex4iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex4s
static inline void push_glVertex4s(glVertex4s_ARG_EXPAND) {
    glVertex4s_PACKED *packed_data = malloc(sizeof(glVertex4s_PACKED));
    packed_data->format = glVertex4s_FORMAT;
    packed_data->func = glVertex4s;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    packed_data->args.a4 = w;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertex4sv
static inline void push_glVertex4sv(glVertex4sv_ARG_EXPAND) {
    glVertex4sv_PACKED *packed_data = malloc(sizeof(glVertex4sv_PACKED));
    packed_data->format = glVertex4sv_FORMAT;
    packed_data->func = glVertex4sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glVertexPointer
static inline void push_glVertexPointer(glVertexPointer_ARG_EXPAND) {
    glVertexPointer_PACKED *packed_data = malloc(sizeof(glVertexPointer_PACKED));
    packed_data->format = glVertexPointer_FORMAT;
    packed_data->func = glVertexPointer;
    packed_data->args.a1 = size;
    packed_data->args.a2 = type;
    packed_data->args.a3 = stride;
    packed_data->args.a4 = pointer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glViewport
static inline void push_glViewport(glViewport_ARG_EXPAND) {
    glViewport_PACKED *packed_data = malloc(sizeof(glViewport_PACKED));
    packed_data->format = glViewport_FORMAT;
    packed_data->func = glViewport;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = width;
    packed_data->args.a4 = height;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos2d
static inline void push_glWindowPos2d(glWindowPos2d_ARG_EXPAND) {
    glWindowPos2d_PACKED *packed_data = malloc(sizeof(glWindowPos2d_PACKED));
    packed_data->format = glWindowPos2d_FORMAT;
    packed_data->func = glWindowPos2d;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos2dv
static inline void push_glWindowPos2dv(glWindowPos2dv_ARG_EXPAND) {
    glWindowPos2dv_PACKED *packed_data = malloc(sizeof(glWindowPos2dv_PACKED));
    packed_data->format = glWindowPos2dv_FORMAT;
    packed_data->func = glWindowPos2dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos2f
static inline void push_glWindowPos2f(glWindowPos2f_ARG_EXPAND) {
    glWindowPos2f_PACKED *packed_data = malloc(sizeof(glWindowPos2f_PACKED));
    packed_data->format = glWindowPos2f_FORMAT;
    packed_data->func = glWindowPos2f;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos2fv
static inline void push_glWindowPos2fv(glWindowPos2fv_ARG_EXPAND) {
    glWindowPos2fv_PACKED *packed_data = malloc(sizeof(glWindowPos2fv_PACKED));
    packed_data->format = glWindowPos2fv_FORMAT;
    packed_data->func = glWindowPos2fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos2i
static inline void push_glWindowPos2i(glWindowPos2i_ARG_EXPAND) {
    glWindowPos2i_PACKED *packed_data = malloc(sizeof(glWindowPos2i_PACKED));
    packed_data->format = glWindowPos2i_FORMAT;
    packed_data->func = glWindowPos2i;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos2iv
static inline void push_glWindowPos2iv(glWindowPos2iv_ARG_EXPAND) {
    glWindowPos2iv_PACKED *packed_data = malloc(sizeof(glWindowPos2iv_PACKED));
    packed_data->format = glWindowPos2iv_FORMAT;
    packed_data->func = glWindowPos2iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos2s
static inline void push_glWindowPos2s(glWindowPos2s_ARG_EXPAND) {
    glWindowPos2s_PACKED *packed_data = malloc(sizeof(glWindowPos2s_PACKED));
    packed_data->format = glWindowPos2s_FORMAT;
    packed_data->func = glWindowPos2s;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos2sv
static inline void push_glWindowPos2sv(glWindowPos2sv_ARG_EXPAND) {
    glWindowPos2sv_PACKED *packed_data = malloc(sizeof(glWindowPos2sv_PACKED));
    packed_data->format = glWindowPos2sv_FORMAT;
    packed_data->func = glWindowPos2sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos3d
static inline void push_glWindowPos3d(glWindowPos3d_ARG_EXPAND) {
    glWindowPos3d_PACKED *packed_data = malloc(sizeof(glWindowPos3d_PACKED));
    packed_data->format = glWindowPos3d_FORMAT;
    packed_data->func = glWindowPos3d;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos3dv
static inline void push_glWindowPos3dv(glWindowPos3dv_ARG_EXPAND) {
    glWindowPos3dv_PACKED *packed_data = malloc(sizeof(glWindowPos3dv_PACKED));
    packed_data->format = glWindowPos3dv_FORMAT;
    packed_data->func = glWindowPos3dv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos3f
static inline void push_glWindowPos3f(glWindowPos3f_ARG_EXPAND) {
    glWindowPos3f_PACKED *packed_data = malloc(sizeof(glWindowPos3f_PACKED));
    packed_data->format = glWindowPos3f_FORMAT;
    packed_data->func = glWindowPos3f;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos3fv
static inline void push_glWindowPos3fv(glWindowPos3fv_ARG_EXPAND) {
    glWindowPos3fv_PACKED *packed_data = malloc(sizeof(glWindowPos3fv_PACKED));
    packed_data->format = glWindowPos3fv_FORMAT;
    packed_data->func = glWindowPos3fv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos3i
static inline void push_glWindowPos3i(glWindowPos3i_ARG_EXPAND) {
    glWindowPos3i_PACKED *packed_data = malloc(sizeof(glWindowPos3i_PACKED));
    packed_data->format = glWindowPos3i_FORMAT;
    packed_data->func = glWindowPos3i;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos3iv
static inline void push_glWindowPos3iv(glWindowPos3iv_ARG_EXPAND) {
    glWindowPos3iv_PACKED *packed_data = malloc(sizeof(glWindowPos3iv_PACKED));
    packed_data->format = glWindowPos3iv_FORMAT;
    packed_data->func = glWindowPos3iv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos3s
static inline void push_glWindowPos3s(glWindowPos3s_ARG_EXPAND) {
    glWindowPos3s_PACKED *packed_data = malloc(sizeof(glWindowPos3s_PACKED));
    packed_data->format = glWindowPos3s_FORMAT;
    packed_data->func = glWindowPos3s;
    packed_data->args.a1 = x;
    packed_data->args.a2 = y;
    packed_data->args.a3 = z;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glWindowPos3sv
static inline void push_glWindowPos3sv(glWindowPos3sv_ARG_EXPAND) {
    glWindowPos3sv_PACKED *packed_data = malloc(sizeof(glWindowPos3sv_PACKED));
    packed_data->format = glWindowPos3sv_FORMAT;
    packed_data->func = glWindowPos3sv;
    packed_data->args.a1 = v;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXBindHyperpipeSGIX
static inline void push_glXBindHyperpipeSGIX(glXBindHyperpipeSGIX_ARG_EXPAND) {
    glXBindHyperpipeSGIX_PACKED *packed_data = malloc(sizeof(glXBindHyperpipeSGIX_PACKED));
    packed_data->format = glXBindHyperpipeSGIX_FORMAT;
    packed_data->func = glXBindHyperpipeSGIX;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = hpId;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXBindSwapBarrierSGIX
static inline void push_glXBindSwapBarrierSGIX(glXBindSwapBarrierSGIX_ARG_EXPAND) {
    glXBindSwapBarrierSGIX_PACKED *packed_data = malloc(sizeof(glXBindSwapBarrierSGIX_PACKED));
    packed_data->format = glXBindSwapBarrierSGIX_FORMAT;
    packed_data->func = glXBindSwapBarrierSGIX;
    packed_data->args.a1 = window;
    packed_data->args.a2 = barrier;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXChangeDrawableAttributes
static inline void push_glXChangeDrawableAttributes(glXChangeDrawableAttributes_ARG_EXPAND) {
    glXChangeDrawableAttributes_PACKED *packed_data = malloc(sizeof(glXChangeDrawableAttributes_PACKED));
    packed_data->format = glXChangeDrawableAttributes_FORMAT;
    packed_data->func = glXChangeDrawableAttributes;
    packed_data->args.a1 = drawable;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXChangeDrawableAttributesSGIX
static inline void push_glXChangeDrawableAttributesSGIX(glXChangeDrawableAttributesSGIX_ARG_EXPAND) {
    glXChangeDrawableAttributesSGIX_PACKED *packed_data = malloc(sizeof(glXChangeDrawableAttributesSGIX_PACKED));
    packed_data->format = glXChangeDrawableAttributesSGIX_FORMAT;
    packed_data->func = glXChangeDrawableAttributesSGIX;
    packed_data->args.a1 = drawable;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXChooseFBConfig
static inline void push_glXChooseFBConfig(glXChooseFBConfig_ARG_EXPAND) {
    glXChooseFBConfig_PACKED *packed_data = malloc(sizeof(glXChooseFBConfig_PACKED));
    packed_data->format = glXChooseFBConfig_FORMAT;
    packed_data->func = glXChooseFBConfig;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = screen;
    packed_data->args.a3 = attrib_list;
    packed_data->args.a4 = nelements;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXChooseVisual
static inline void push_glXChooseVisual(glXChooseVisual_ARG_EXPAND) {
    glXChooseVisual_PACKED *packed_data = malloc(sizeof(glXChooseVisual_PACKED));
    packed_data->format = glXChooseVisual_FORMAT;
    packed_data->func = glXChooseVisual;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = screen;
    packed_data->args.a3 = attribList;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXClientInfo
static inline void push_glXClientInfo(glXClientInfo_ARG_EXPAND) {
    glXClientInfo_PACKED *packed_data = malloc(sizeof(glXClientInfo_PACKED));
    packed_data->format = glXClientInfo_FORMAT;
    packed_data->func = glXClientInfo;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXCopyContext
static inline void push_glXCopyContext(glXCopyContext_ARG_EXPAND) {
    glXCopyContext_PACKED *packed_data = malloc(sizeof(glXCopyContext_PACKED));
    packed_data->format = glXCopyContext_FORMAT;
    packed_data->func = glXCopyContext;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = src;
    packed_data->args.a3 = dst;
    packed_data->args.a4 = mask;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXCreateContext
static inline void push_glXCreateContext(glXCreateContext_ARG_EXPAND) {
    glXCreateContext_PACKED *packed_data = malloc(sizeof(glXCreateContext_PACKED));
    packed_data->format = glXCreateContext_FORMAT;
    packed_data->func = glXCreateContext;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = vis;
    packed_data->args.a3 = shareList;
    packed_data->args.a4 = direct;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXCreateContextWithConfigSGIX
static inline void push_glXCreateContextWithConfigSGIX(glXCreateContextWithConfigSGIX_ARG_EXPAND) {
    glXCreateContextWithConfigSGIX_PACKED *packed_data = malloc(sizeof(glXCreateContextWithConfigSGIX_PACKED));
    packed_data->format = glXCreateContextWithConfigSGIX_FORMAT;
    packed_data->func = glXCreateContextWithConfigSGIX;
    packed_data->args.a1 = gc_id;
    packed_data->args.a2 = screen;
    packed_data->args.a3 = config;
    packed_data->args.a4 = share_list;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXCreateGLXPbufferSGIX
static inline void push_glXCreateGLXPbufferSGIX(glXCreateGLXPbufferSGIX_ARG_EXPAND) {
    glXCreateGLXPbufferSGIX_PACKED *packed_data = malloc(sizeof(glXCreateGLXPbufferSGIX_PACKED));
    packed_data->format = glXCreateGLXPbufferSGIX_FORMAT;
    packed_data->func = glXCreateGLXPbufferSGIX;
    packed_data->args.a1 = config;
    packed_data->args.a2 = pbuffer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXCreateGLXPixmap
static inline void push_glXCreateGLXPixmap(glXCreateGLXPixmap_ARG_EXPAND) {
    glXCreateGLXPixmap_PACKED *packed_data = malloc(sizeof(glXCreateGLXPixmap_PACKED));
    packed_data->format = glXCreateGLXPixmap_FORMAT;
    packed_data->func = glXCreateGLXPixmap;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = visual;
    packed_data->args.a3 = pixmap;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXCreateGLXPixmapWithConfigSGIX
static inline void push_glXCreateGLXPixmapWithConfigSGIX(glXCreateGLXPixmapWithConfigSGIX_ARG_EXPAND) {
    glXCreateGLXPixmapWithConfigSGIX_PACKED *packed_data = malloc(sizeof(glXCreateGLXPixmapWithConfigSGIX_PACKED));
    packed_data->format = glXCreateGLXPixmapWithConfigSGIX_FORMAT;
    packed_data->func = glXCreateGLXPixmapWithConfigSGIX;
    packed_data->args.a1 = config;
    packed_data->args.a2 = pixmap;
    packed_data->args.a3 = glxpixmap;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXCreateGLXVideoSourceSGIX
static inline void push_glXCreateGLXVideoSourceSGIX(glXCreateGLXVideoSourceSGIX_ARG_EXPAND) {
    glXCreateGLXVideoSourceSGIX_PACKED *packed_data = malloc(sizeof(glXCreateGLXVideoSourceSGIX_PACKED));
    packed_data->format = glXCreateGLXVideoSourceSGIX_FORMAT;
    packed_data->func = glXCreateGLXVideoSourceSGIX;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = screen;
    packed_data->args.a3 = server;
    packed_data->args.a4 = path;
    packed_data->args.a5 = class;
    packed_data->args.a6 = node;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXCreateNewContext
static inline void push_glXCreateNewContext(glXCreateNewContext_ARG_EXPAND) {
    glXCreateNewContext_PACKED *packed_data = malloc(sizeof(glXCreateNewContext_PACKED));
    packed_data->format = glXCreateNewContext_FORMAT;
    packed_data->func = glXCreateNewContext;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = config;
    packed_data->args.a3 = render_type;
    packed_data->args.a4 = share_list;
    packed_data->args.a5 = direct;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXCreatePbuffer
static inline void push_glXCreatePbuffer(glXCreatePbuffer_ARG_EXPAND) {
    glXCreatePbuffer_PACKED *packed_data = malloc(sizeof(glXCreatePbuffer_PACKED));
    packed_data->format = glXCreatePbuffer_FORMAT;
    packed_data->func = glXCreatePbuffer;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = config;
    packed_data->args.a3 = attrib_list;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXCreatePixmap
static inline void push_glXCreatePixmap(glXCreatePixmap_ARG_EXPAND) {
    glXCreatePixmap_PACKED *packed_data = malloc(sizeof(glXCreatePixmap_PACKED));
    packed_data->format = glXCreatePixmap_FORMAT;
    packed_data->func = glXCreatePixmap;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = config;
    packed_data->args.a3 = pixmap;
    packed_data->args.a4 = attrib_list;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXCreateWindow
static inline void push_glXCreateWindow(glXCreateWindow_ARG_EXPAND) {
    glXCreateWindow_PACKED *packed_data = malloc(sizeof(glXCreateWindow_PACKED));
    packed_data->format = glXCreateWindow_FORMAT;
    packed_data->func = glXCreateWindow;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = config;
    packed_data->args.a3 = win;
    packed_data->args.a4 = attrib_list;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXDestroyContext
static inline void push_glXDestroyContext(glXDestroyContext_ARG_EXPAND) {
    glXDestroyContext_PACKED *packed_data = malloc(sizeof(glXDestroyContext_PACKED));
    packed_data->format = glXDestroyContext_FORMAT;
    packed_data->func = glXDestroyContext;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = ctx;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXDestroyGLXPbufferSGIX
static inline void push_glXDestroyGLXPbufferSGIX(glXDestroyGLXPbufferSGIX_ARG_EXPAND) {
    glXDestroyGLXPbufferSGIX_PACKED *packed_data = malloc(sizeof(glXDestroyGLXPbufferSGIX_PACKED));
    packed_data->format = glXDestroyGLXPbufferSGIX_FORMAT;
    packed_data->func = glXDestroyGLXPbufferSGIX;
    packed_data->args.a1 = pbuffer;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXDestroyGLXPixmap
static inline void push_glXDestroyGLXPixmap(glXDestroyGLXPixmap_ARG_EXPAND) {
    glXDestroyGLXPixmap_PACKED *packed_data = malloc(sizeof(glXDestroyGLXPixmap_PACKED));
    packed_data->format = glXDestroyGLXPixmap_FORMAT;
    packed_data->func = glXDestroyGLXPixmap;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = pixmap;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXDestroyGLXVideoSourceSGIX
static inline void push_glXDestroyGLXVideoSourceSGIX(glXDestroyGLXVideoSourceSGIX_ARG_EXPAND) {
    glXDestroyGLXVideoSourceSGIX_PACKED *packed_data = malloc(sizeof(glXDestroyGLXVideoSourceSGIX_PACKED));
    packed_data->format = glXDestroyGLXVideoSourceSGIX_FORMAT;
    packed_data->func = glXDestroyGLXVideoSourceSGIX;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = glxvideosource;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXDestroyHyperpipeConfigSGIX
static inline void push_glXDestroyHyperpipeConfigSGIX(glXDestroyHyperpipeConfigSGIX_ARG_EXPAND) {
    glXDestroyHyperpipeConfigSGIX_PACKED *packed_data = malloc(sizeof(glXDestroyHyperpipeConfigSGIX_PACKED));
    packed_data->format = glXDestroyHyperpipeConfigSGIX_FORMAT;
    packed_data->func = glXDestroyHyperpipeConfigSGIX;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = hpId;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXDestroyPbuffer
static inline void push_glXDestroyPbuffer(glXDestroyPbuffer_ARG_EXPAND) {
    glXDestroyPbuffer_PACKED *packed_data = malloc(sizeof(glXDestroyPbuffer_PACKED));
    packed_data->format = glXDestroyPbuffer_FORMAT;
    packed_data->func = glXDestroyPbuffer;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = pbuf;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXDestroyPixmap
static inline void push_glXDestroyPixmap(glXDestroyPixmap_ARG_EXPAND) {
    glXDestroyPixmap_PACKED *packed_data = malloc(sizeof(glXDestroyPixmap_PACKED));
    packed_data->format = glXDestroyPixmap_FORMAT;
    packed_data->func = glXDestroyPixmap;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = pixmap;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXDestroyWindow
static inline void push_glXDestroyWindow(glXDestroyWindow_ARG_EXPAND) {
    glXDestroyWindow_PACKED *packed_data = malloc(sizeof(glXDestroyWindow_PACKED));
    packed_data->format = glXDestroyWindow_FORMAT;
    packed_data->func = glXDestroyWindow;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = win;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXGetCurrentDisplay
static inline void push_glXGetCurrentDisplay(glXGetCurrentDisplay_ARG_EXPAND) {
    glXGetCurrentDisplay_PACKED *packed_data = malloc(sizeof(glXGetCurrentDisplay_PACKED));
    packed_data->format = glXGetCurrentDisplay_FORMAT;
    packed_data->func = glXGetCurrentDisplay;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXGetCurrentReadDrawable
static inline void push_glXGetCurrentReadDrawable(glXGetCurrentReadDrawable_ARG_EXPAND) {
    glXGetCurrentReadDrawable_PACKED *packed_data = malloc(sizeof(glXGetCurrentReadDrawable_PACKED));
    packed_data->format = glXGetCurrentReadDrawable_FORMAT;
    packed_data->func = glXGetCurrentReadDrawable;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXGetDrawableAttributes
static inline void push_glXGetDrawableAttributes(glXGetDrawableAttributes_ARG_EXPAND) {
    glXGetDrawableAttributes_PACKED *packed_data = malloc(sizeof(glXGetDrawableAttributes_PACKED));
    packed_data->format = glXGetDrawableAttributes_FORMAT;
    packed_data->func = glXGetDrawableAttributes;
    packed_data->args.a1 = drawable;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXGetDrawableAttributesSGIX
static inline void push_glXGetDrawableAttributesSGIX(glXGetDrawableAttributesSGIX_ARG_EXPAND) {
    glXGetDrawableAttributesSGIX_PACKED *packed_data = malloc(sizeof(glXGetDrawableAttributesSGIX_PACKED));
    packed_data->format = glXGetDrawableAttributesSGIX_FORMAT;
    packed_data->func = glXGetDrawableAttributesSGIX;
    packed_data->args.a1 = drawable;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXGetFBConfigAttrib
static inline void push_glXGetFBConfigAttrib(glXGetFBConfigAttrib_ARG_EXPAND) {
    glXGetFBConfigAttrib_PACKED *packed_data = malloc(sizeof(glXGetFBConfigAttrib_PACKED));
    packed_data->format = glXGetFBConfigAttrib_FORMAT;
    packed_data->func = glXGetFBConfigAttrib;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = config;
    packed_data->args.a3 = attribute;
    packed_data->args.a4 = value;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXGetFBConfigs
static inline void push_glXGetFBConfigs(glXGetFBConfigs_ARG_EXPAND) {
    glXGetFBConfigs_PACKED *packed_data = malloc(sizeof(glXGetFBConfigs_PACKED));
    packed_data->format = glXGetFBConfigs_FORMAT;
    packed_data->func = glXGetFBConfigs;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = screen;
    packed_data->args.a3 = nelements;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXGetFBConfigsSGIX
static inline void push_glXGetFBConfigsSGIX(glXGetFBConfigsSGIX_ARG_EXPAND) {
    glXGetFBConfigsSGIX_PACKED *packed_data = malloc(sizeof(glXGetFBConfigsSGIX_PACKED));
    packed_data->format = glXGetFBConfigsSGIX_FORMAT;
    packed_data->func = glXGetFBConfigsSGIX;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXGetProcAddress
static inline void push_glXGetProcAddress(glXGetProcAddress_ARG_EXPAND) {
    glXGetProcAddress_PACKED *packed_data = malloc(sizeof(glXGetProcAddress_PACKED));
    packed_data->format = glXGetProcAddress_FORMAT;
    packed_data->func = glXGetProcAddress;
    packed_data->args.a1 = procName;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXGetSelectedEvent
static inline void push_glXGetSelectedEvent(glXGetSelectedEvent_ARG_EXPAND) {
    glXGetSelectedEvent_PACKED *packed_data = malloc(sizeof(glXGetSelectedEvent_PACKED));
    packed_data->format = glXGetSelectedEvent_FORMAT;
    packed_data->func = glXGetSelectedEvent;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = draw;
    packed_data->args.a3 = event_mask;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXGetVisualConfigs
static inline void push_glXGetVisualConfigs(glXGetVisualConfigs_ARG_EXPAND) {
    glXGetVisualConfigs_PACKED *packed_data = malloc(sizeof(glXGetVisualConfigs_PACKED));
    packed_data->format = glXGetVisualConfigs_FORMAT;
    packed_data->func = glXGetVisualConfigs;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXGetVisualFromFBConfig
static inline void push_glXGetVisualFromFBConfig(glXGetVisualFromFBConfig_ARG_EXPAND) {
    glXGetVisualFromFBConfig_PACKED *packed_data = malloc(sizeof(glXGetVisualFromFBConfig_PACKED));
    packed_data->format = glXGetVisualFromFBConfig_FORMAT;
    packed_data->func = glXGetVisualFromFBConfig;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = config;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXHyperpipeAttribSGIX
static inline void push_glXHyperpipeAttribSGIX(glXHyperpipeAttribSGIX_ARG_EXPAND) {
    glXHyperpipeAttribSGIX_PACKED *packed_data = malloc(sizeof(glXHyperpipeAttribSGIX_PACKED));
    packed_data->format = glXHyperpipeAttribSGIX_FORMAT;
    packed_data->func = glXHyperpipeAttribSGIX;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = timeSlice;
    packed_data->args.a3 = attrib;
    packed_data->args.a4 = size;
    packed_data->args.a5 = attribList;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXHyperpipeConfigSGIX
static inline void push_glXHyperpipeConfigSGIX(glXHyperpipeConfigSGIX_ARG_EXPAND) {
    glXHyperpipeConfigSGIX_PACKED *packed_data = malloc(sizeof(glXHyperpipeConfigSGIX_PACKED));
    packed_data->format = glXHyperpipeConfigSGIX_FORMAT;
    packed_data->func = glXHyperpipeConfigSGIX;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = networkId;
    packed_data->args.a3 = npipes;
    packed_data->args.a4 = cfg;
    packed_data->args.a5 = hpId;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXIsDirect
static inline void push_glXIsDirect(glXIsDirect_ARG_EXPAND) {
    glXIsDirect_PACKED *packed_data = malloc(sizeof(glXIsDirect_PACKED));
    packed_data->format = glXIsDirect_FORMAT;
    packed_data->func = glXIsDirect;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = ctx;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXJoinSwapGroupSGIX
static inline void push_glXJoinSwapGroupSGIX(glXJoinSwapGroupSGIX_ARG_EXPAND) {
    glXJoinSwapGroupSGIX_PACKED *packed_data = malloc(sizeof(glXJoinSwapGroupSGIX_PACKED));
    packed_data->format = glXJoinSwapGroupSGIX_FORMAT;
    packed_data->func = glXJoinSwapGroupSGIX;
    packed_data->args.a1 = window;
    packed_data->args.a2 = group;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXMakeContextCurrent
static inline void push_glXMakeContextCurrent(glXMakeContextCurrent_ARG_EXPAND) {
    glXMakeContextCurrent_PACKED *packed_data = malloc(sizeof(glXMakeContextCurrent_PACKED));
    packed_data->format = glXMakeContextCurrent_FORMAT;
    packed_data->func = glXMakeContextCurrent;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = draw;
    packed_data->args.a3 = read;
    packed_data->args.a4 = ctx;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXMakeCurrent
static inline void push_glXMakeCurrent(glXMakeCurrent_ARG_EXPAND) {
    glXMakeCurrent_PACKED *packed_data = malloc(sizeof(glXMakeCurrent_PACKED));
    packed_data->format = glXMakeCurrent_FORMAT;
    packed_data->func = glXMakeCurrent;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = drawable;
    packed_data->args.a3 = ctx;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXMakeCurrentReadSGI
static inline void push_glXMakeCurrentReadSGI(glXMakeCurrentReadSGI_ARG_EXPAND) {
    glXMakeCurrentReadSGI_PACKED *packed_data = malloc(sizeof(glXMakeCurrentReadSGI_PACKED));
    packed_data->format = glXMakeCurrentReadSGI_FORMAT;
    packed_data->func = glXMakeCurrentReadSGI;
    packed_data->args.a1 = drawable;
    packed_data->args.a2 = readdrawable;
    packed_data->args.a3 = context;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXQueryContext
static inline void push_glXQueryContext(glXQueryContext_ARG_EXPAND) {
    glXQueryContext_PACKED *packed_data = malloc(sizeof(glXQueryContext_PACKED));
    packed_data->format = glXQueryContext_FORMAT;
    packed_data->func = glXQueryContext;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = ctx;
    packed_data->args.a3 = attribute;
    packed_data->args.a4 = value;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXQueryContextInfoEXT
static inline void push_glXQueryContextInfoEXT(glXQueryContextInfoEXT_ARG_EXPAND) {
    glXQueryContextInfoEXT_PACKED *packed_data = malloc(sizeof(glXQueryContextInfoEXT_PACKED));
    packed_data->format = glXQueryContextInfoEXT_FORMAT;
    packed_data->func = glXQueryContextInfoEXT;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXQueryDrawable
static inline void push_glXQueryDrawable(glXQueryDrawable_ARG_EXPAND) {
    glXQueryDrawable_PACKED *packed_data = malloc(sizeof(glXQueryDrawable_PACKED));
    packed_data->format = glXQueryDrawable_FORMAT;
    packed_data->func = glXQueryDrawable;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = draw;
    packed_data->args.a3 = attribute;
    packed_data->args.a4 = value;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXQueryExtensionsString
static inline void push_glXQueryExtensionsString(glXQueryExtensionsString_ARG_EXPAND) {
    glXQueryExtensionsString_PACKED *packed_data = malloc(sizeof(glXQueryExtensionsString_PACKED));
    packed_data->format = glXQueryExtensionsString_FORMAT;
    packed_data->func = glXQueryExtensionsString;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = screen;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXQueryHyperpipeAttribSGIX
static inline void push_glXQueryHyperpipeAttribSGIX(glXQueryHyperpipeAttribSGIX_ARG_EXPAND) {
    glXQueryHyperpipeAttribSGIX_PACKED *packed_data = malloc(sizeof(glXQueryHyperpipeAttribSGIX_PACKED));
    packed_data->format = glXQueryHyperpipeAttribSGIX_FORMAT;
    packed_data->func = glXQueryHyperpipeAttribSGIX;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = timeSlice;
    packed_data->args.a3 = attrib;
    packed_data->args.a4 = size;
    packed_data->args.a5 = returnAttribList;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXQueryHyperpipeBestAttribSGIX
static inline void push_glXQueryHyperpipeBestAttribSGIX(glXQueryHyperpipeBestAttribSGIX_ARG_EXPAND) {
    glXQueryHyperpipeBestAttribSGIX_PACKED *packed_data = malloc(sizeof(glXQueryHyperpipeBestAttribSGIX_PACKED));
    packed_data->format = glXQueryHyperpipeBestAttribSGIX_FORMAT;
    packed_data->func = glXQueryHyperpipeBestAttribSGIX;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = timeSlice;
    packed_data->args.a3 = attrib;
    packed_data->args.a4 = size;
    packed_data->args.a5 = attribList;
    packed_data->args.a6 = returnAttribList;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXQueryHyperpipeConfigSGIX
static inline void push_glXQueryHyperpipeConfigSGIX(glXQueryHyperpipeConfigSGIX_ARG_EXPAND) {
    glXQueryHyperpipeConfigSGIX_PACKED *packed_data = malloc(sizeof(glXQueryHyperpipeConfigSGIX_PACKED));
    packed_data->format = glXQueryHyperpipeConfigSGIX_FORMAT;
    packed_data->func = glXQueryHyperpipeConfigSGIX;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = hpId;
    packed_data->args.a3 = npipes;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXQueryHyperpipeNetworkSGIX
static inline void push_glXQueryHyperpipeNetworkSGIX(glXQueryHyperpipeNetworkSGIX_ARG_EXPAND) {
    glXQueryHyperpipeNetworkSGIX_PACKED *packed_data = malloc(sizeof(glXQueryHyperpipeNetworkSGIX_PACKED));
    packed_data->format = glXQueryHyperpipeNetworkSGIX_FORMAT;
    packed_data->func = glXQueryHyperpipeNetworkSGIX;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = npipes;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXQueryMaxSwapBarriersSGIX
static inline void push_glXQueryMaxSwapBarriersSGIX(glXQueryMaxSwapBarriersSGIX_ARG_EXPAND) {
    glXQueryMaxSwapBarriersSGIX_PACKED *packed_data = malloc(sizeof(glXQueryMaxSwapBarriersSGIX_PACKED));
    packed_data->format = glXQueryMaxSwapBarriersSGIX_FORMAT;
    packed_data->func = glXQueryMaxSwapBarriersSGIX;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXQueryServerString
static inline void push_glXQueryServerString(glXQueryServerString_ARG_EXPAND) {
    glXQueryServerString_PACKED *packed_data = malloc(sizeof(glXQueryServerString_PACKED));
    packed_data->format = glXQueryServerString_FORMAT;
    packed_data->func = glXQueryServerString;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = screen;
    packed_data->args.a3 = name;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXQueryVersion
static inline void push_glXQueryVersion(glXQueryVersion_ARG_EXPAND) {
    glXQueryVersion_PACKED *packed_data = malloc(sizeof(glXQueryVersion_PACKED));
    packed_data->format = glXQueryVersion_FORMAT;
    packed_data->func = glXQueryVersion;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = maj;
    packed_data->args.a3 = min;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXRender
static inline void push_glXRender(glXRender_ARG_EXPAND) {
    glXRender_PACKED *packed_data = malloc(sizeof(glXRender_PACKED));
    packed_data->format = glXRender_FORMAT;
    packed_data->func = glXRender;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXRenderLarge
static inline void push_glXRenderLarge(glXRenderLarge_ARG_EXPAND) {
    glXRenderLarge_PACKED *packed_data = malloc(sizeof(glXRenderLarge_PACKED));
    packed_data->format = glXRenderLarge_FORMAT;
    packed_data->func = glXRenderLarge;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXSelectEvent
static inline void push_glXSelectEvent(glXSelectEvent_ARG_EXPAND) {
    glXSelectEvent_PACKED *packed_data = malloc(sizeof(glXSelectEvent_PACKED));
    packed_data->format = glXSelectEvent_FORMAT;
    packed_data->func = glXSelectEvent;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = draw;
    packed_data->args.a3 = event_mask;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXSwapBuffers
static inline void push_glXSwapBuffers(glXSwapBuffers_ARG_EXPAND) {
    glXSwapBuffers_PACKED *packed_data = malloc(sizeof(glXSwapBuffers_PACKED));
    packed_data->format = glXSwapBuffers_FORMAT;
    packed_data->func = glXSwapBuffers;
    packed_data->args.a1 = dpy;
    packed_data->args.a2 = drawable;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXSwapIntervalSGI
static inline void push_glXSwapIntervalSGI(glXSwapIntervalSGI_ARG_EXPAND) {
    glXSwapIntervalSGI_PACKED *packed_data = malloc(sizeof(glXSwapIntervalSGI_PACKED));
    packed_data->format = glXSwapIntervalSGI_FORMAT;
    packed_data->func = glXSwapIntervalSGI;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXUseXFont
static inline void push_glXUseXFont(glXUseXFont_ARG_EXPAND) {
    glXUseXFont_PACKED *packed_data = malloc(sizeof(glXUseXFont_PACKED));
    packed_data->format = glXUseXFont_FORMAT;
    packed_data->func = glXUseXFont;
    packed_data->args.a1 = font;
    packed_data->args.a2 = first;
    packed_data->args.a3 = count;
    packed_data->args.a4 = list;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXVendorPrivate
static inline void push_glXVendorPrivate(glXVendorPrivate_ARG_EXPAND) {
    glXVendorPrivate_PACKED *packed_data = malloc(sizeof(glXVendorPrivate_PACKED));
    packed_data->format = glXVendorPrivate_FORMAT;
    packed_data->func = glXVendorPrivate;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXVendorPrivateWithReply
static inline void push_glXVendorPrivateWithReply(glXVendorPrivateWithReply_ARG_EXPAND) {
    glXVendorPrivateWithReply_PACKED *packed_data = malloc(sizeof(glXVendorPrivateWithReply_PACKED));
    packed_data->format = glXVendorPrivateWithReply_FORMAT;
    packed_data->func = glXVendorPrivateWithReply;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXWaitGL
static inline void push_glXWaitGL(glXWaitGL_ARG_EXPAND) {
    glXWaitGL_PACKED *packed_data = malloc(sizeof(glXWaitGL_PACKED));
    packed_data->format = glXWaitGL_FORMAT;
    packed_data->func = glXWaitGL;
    glPushCall((void *)packed_data);
}
#endif
#ifndef direct_glXWaitX
static inline void push_glXWaitX(glXWaitX_ARG_EXPAND) {
    glXWaitX_PACKED *packed_data = malloc(sizeof(glXWaitX_PACKED));
    packed_data->format = glXWaitX_FORMAT;
    packed_data->func = glXWaitX;
    glPushCall((void *)packed_data);
}
#endif

#endif
