#ifdef USE_ES2
#include "gles2.h"

void glPackedCall(const packed_call_t *packed) {
    switch (packed->format) {
        case FORMAT_void_GLenum: {
            PACKED_void_GLenum *unpacked = (PACKED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            unpacked->func(args.a1);
            break;
        }
        case FORMAT_void_GLuint_GLuint: {
            PACKED_void_GLuint_GLuint *unpacked = (PACKED_void_GLuint_GLuint *)packed;
            ARGS_void_GLuint_GLuint args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLuint_GLuint_const_GLchar___GENPT__: {
            PACKED_void_GLuint_GLuint_const_GLchar___GENPT__ *unpacked = (PACKED_void_GLuint_GLuint_const_GLchar___GENPT__ *)packed;
            ARGS_void_GLuint_GLuint_const_GLchar___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLenum_GLuint: {
            PACKED_void_GLenum_GLuint *unpacked = (PACKED_void_GLenum_GLuint *)packed;
            ARGS_void_GLenum_GLuint args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLclampf_GLclampf_GLclampf_GLclampf: {
            PACKED_void_GLclampf_GLclampf_GLclampf_GLclampf *unpacked = (PACKED_void_GLclampf_GLclampf_GLclampf_GLclampf *)packed;
            ARGS_void_GLclampf_GLclampf_GLclampf_GLclampf args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLenum_GLenum: {
            PACKED_void_GLenum_GLenum *unpacked = (PACKED_void_GLenum_GLenum *)packed;
            ARGS_void_GLenum_GLenum args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLenum_GLenum_GLenum_GLenum: {
            PACKED_void_GLenum_GLenum_GLenum_GLenum *unpacked = (PACKED_void_GLenum_GLenum_GLenum_GLenum *)packed;
            ARGS_void_GLenum_GLenum_GLenum_GLenum args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum: {
            PACKED_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum *unpacked = (PACKED_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum *)packed;
            ARGS_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__: {
            PACKED_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__ *unpacked = (PACKED_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_GLenum_GLenum: {
            PACKED_GLenum_GLenum *unpacked = (PACKED_GLenum_GLenum *)packed;
            ARGS_GLenum_GLenum args = unpacked->args;
            unpacked->func(args.a1);
            break;
        }
        case FORMAT_void_GLbitfield: {
            PACKED_void_GLbitfield *unpacked = (PACKED_void_GLbitfield *)packed;
            ARGS_void_GLbitfield args = unpacked->args;
            unpacked->func(args.a1);
            break;
        }
        case FORMAT_void_GLclampf: {
            PACKED_void_GLclampf *unpacked = (PACKED_void_GLclampf *)packed;
            ARGS_void_GLclampf args = unpacked->args;
            unpacked->func(args.a1);
            break;
        }
        case FORMAT_void_GLint: {
            PACKED_void_GLint *unpacked = (PACKED_void_GLint *)packed;
            ARGS_void_GLint args = unpacked->args;
            unpacked->func(args.a1);
            break;
        }
        case FORMAT_void_GLboolean_GLboolean_GLboolean_GLboolean: {
            PACKED_void_GLboolean_GLboolean_GLboolean_GLboolean *unpacked = (PACKED_void_GLboolean_GLboolean_GLboolean_GLboolean *)packed;
            ARGS_void_GLboolean_GLboolean_GLboolean_GLboolean args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLuint: {
            PACKED_void_GLuint *unpacked = (PACKED_void_GLuint *)packed;
            ARGS_void_GLuint args = unpacked->args;
            unpacked->func(args.a1);
            break;
        }
        case FORMAT_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__: {
            PACKED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ *unpacked = (PACKED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8);
            break;
        }
        case FORMAT_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__: {
            PACKED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ *unpacked = (PACKED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9);
            break;
        }
        case FORMAT_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint: {
            PACKED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint *unpacked = (PACKED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint *)packed;
            ARGS_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8);
            break;
        }
        case FORMAT_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei: {
            PACKED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei *unpacked = (PACKED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei *)packed;
            ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8);
            break;
        }
        case FORMAT_GLuint: {
            PACKED_GLuint *unpacked = (PACKED_GLuint *)packed;
            unpacked->func();
            break;
        }
        case FORMAT_GLuint_GLenum: {
            PACKED_GLuint_GLenum *unpacked = (PACKED_GLuint_GLenum *)packed;
            ARGS_GLuint_GLenum args = unpacked->args;
            unpacked->func(args.a1);
            break;
        }
        case FORMAT_void_GLsizei_const_GLuint___GENPT__: {
            PACKED_void_GLsizei_const_GLuint___GENPT__ *unpacked = (PACKED_void_GLsizei_const_GLuint___GENPT__ *)packed;
            ARGS_void_GLsizei_const_GLuint___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLboolean: {
            PACKED_void_GLboolean *unpacked = (PACKED_void_GLboolean *)packed;
            ARGS_void_GLboolean args = unpacked->args;
            unpacked->func(args.a1);
            break;
        }
        case FORMAT_void_GLclampf_GLclampf: {
            PACKED_void_GLclampf_GLclampf *unpacked = (PACKED_void_GLclampf_GLclampf *)packed;
            ARGS_void_GLclampf_GLclampf args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLenum_GLint_GLsizei: {
            PACKED_void_GLenum_GLint_GLsizei *unpacked = (PACKED_void_GLenum_GLint_GLsizei *)packed;
            ARGS_void_GLenum_GLint_GLsizei args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__: {
            PACKED_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__ *unpacked = (PACKED_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void: {
            PACKED_void *unpacked = (PACKED_void *)packed;
            unpacked->func();
            break;
        }
        case FORMAT_void_GLenum_GLenum_GLenum_GLuint: {
            PACKED_void_GLenum_GLenum_GLenum_GLuint *unpacked = (PACKED_void_GLenum_GLenum_GLenum_GLuint *)packed;
            ARGS_void_GLenum_GLenum_GLenum_GLuint args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLenum_GLenum_GLenum_GLuint_GLint: {
            PACKED_void_GLenum_GLenum_GLenum_GLuint_GLint *unpacked = (PACKED_void_GLenum_GLenum_GLenum_GLuint_GLint *)packed;
            ARGS_void_GLenum_GLenum_GLenum_GLuint_GLint args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        case FORMAT_void_GLsizei_GLuint___GENPT__: {
            PACKED_void_GLsizei_GLuint___GENPT__ *unpacked = (PACKED_void_GLsizei_GLuint___GENPT__ *)packed;
            ARGS_void_GLsizei_GLuint___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLuint_GLuint_GLsizei_GLsizei___GENPT___GLint___GENPT___GLenum___GENPT___GLchar___GENPT__: {
            PACKED_void_GLuint_GLuint_GLsizei_GLsizei___GENPT___GLint___GENPT___GLenum___GENPT___GLchar___GENPT__ *unpacked = (PACKED_void_GLuint_GLuint_GLsizei_GLsizei___GENPT___GLint___GENPT___GLenum___GENPT___GLchar___GENPT__ *)packed;
            ARGS_void_GLuint_GLuint_GLsizei_GLsizei___GENPT___GLint___GENPT___GLenum___GENPT___GLchar___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7);
            break;
        }
        case FORMAT_void_GLuint_GLsizei_GLsizei___GENPT___GLuint___GENPT__: {
            PACKED_void_GLuint_GLsizei_GLsizei___GENPT___GLuint___GENPT__ *unpacked = (PACKED_void_GLuint_GLsizei_GLsizei___GENPT___GLuint___GENPT__ *)packed;
            ARGS_void_GLuint_GLsizei_GLsizei___GENPT___GLuint___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_GLint_GLuint_const_GLchar___GENPT__: {
            PACKED_GLint_GLuint_const_GLchar___GENPT__ *unpacked = (PACKED_GLint_GLuint_const_GLchar___GENPT__ *)packed;
            ARGS_GLint_GLuint_const_GLchar___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLenum_GLboolean___GENPT__: {
            PACKED_void_GLenum_GLboolean___GENPT__ *unpacked = (PACKED_void_GLenum_GLboolean___GENPT__ *)packed;
            ARGS_void_GLenum_GLboolean___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLenum_GLenum_GLint___GENPT__: {
            PACKED_void_GLenum_GLenum_GLint___GENPT__ *unpacked = (PACKED_void_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_GLenum: {
            PACKED_GLenum *unpacked = (PACKED_GLenum *)packed;
            unpacked->func();
            break;
        }
        case FORMAT_void_GLenum_GLfloat___GENPT__: {
            PACKED_void_GLenum_GLfloat___GENPT__ *unpacked = (PACKED_void_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLfloat___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLenum_GLenum_GLenum_GLint___GENPT__: {
            PACKED_void_GLenum_GLenum_GLenum_GLint___GENPT__ *unpacked = (PACKED_void_GLenum_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLenum_GLint___GENPT__: {
            PACKED_void_GLenum_GLint___GENPT__ *unpacked = (PACKED_void_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLint___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLuint_GLsizei_GLsizei___GENPT___GLchar___GENPT__: {
            PACKED_void_GLuint_GLsizei_GLsizei___GENPT___GLchar___GENPT__ *unpacked = (PACKED_void_GLuint_GLsizei_GLsizei___GENPT___GLchar___GENPT__ *)packed;
            ARGS_void_GLuint_GLsizei_GLsizei___GENPT___GLchar___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLuint_GLenum_GLint___GENPT__: {
            PACKED_void_GLuint_GLenum_GLint___GENPT__ *unpacked = (PACKED_void_GLuint_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLuint_GLenum_GLint___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLenum_GLenum_GLint___GENPT___GLint___GENPT__: {
            PACKED_void_GLenum_GLenum_GLint___GENPT___GLint___GENPT__ *unpacked = (PACKED_void_GLenum_GLenum_GLint___GENPT___GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT___GLint___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_const_GLubyte___GENPT___GLenum: {
            PACKED_const_GLubyte___GENPT___GLenum *unpacked = (PACKED_const_GLubyte___GENPT___GLenum *)packed;
            ARGS_const_GLubyte___GENPT___GLenum args = unpacked->args;
            unpacked->func(args.a1);
            break;
        }
        case FORMAT_void_GLenum_GLenum_GLfloat___GENPT__: {
            PACKED_void_GLenum_GLenum_GLfloat___GENPT__ *unpacked = (PACKED_void_GLenum_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLfloat___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLuint_GLint_GLfloat___GENPT__: {
            PACKED_void_GLuint_GLint_GLfloat___GENPT__ *unpacked = (PACKED_void_GLuint_GLint_GLfloat___GENPT__ *)packed;
            ARGS_void_GLuint_GLint_GLfloat___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLuint_GLint_GLint___GENPT__: {
            PACKED_void_GLuint_GLint_GLint___GENPT__ *unpacked = (PACKED_void_GLuint_GLint_GLint___GENPT__ *)packed;
            ARGS_void_GLuint_GLint_GLint___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLuint_GLenum_GLvoid___GENPT____GENPT__: {
            PACKED_void_GLuint_GLenum_GLvoid___GENPT____GENPT__ *unpacked = (PACKED_void_GLuint_GLenum_GLvoid___GENPT____GENPT__ *)packed;
            ARGS_void_GLuint_GLenum_GLvoid___GENPT____GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLuint_GLenum_GLfloat___GENPT__: {
            PACKED_void_GLuint_GLenum_GLfloat___GENPT__ *unpacked = (PACKED_void_GLuint_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLuint_GLenum_GLfloat___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_GLboolean_GLuint: {
            PACKED_GLboolean_GLuint *unpacked = (PACKED_GLboolean_GLuint *)packed;
            ARGS_GLboolean_GLuint args = unpacked->args;
            unpacked->func(args.a1);
            break;
        }
        case FORMAT_GLboolean_GLenum: {
            PACKED_GLboolean_GLenum *unpacked = (PACKED_GLboolean_GLenum *)packed;
            ARGS_GLboolean_GLenum args = unpacked->args;
            unpacked->func(args.a1);
            break;
        }
        case FORMAT_void_GLfloat: {
            PACKED_void_GLfloat *unpacked = (PACKED_void_GLfloat *)packed;
            ARGS_void_GLfloat args = unpacked->args;
            unpacked->func(args.a1);
            break;
        }
        case FORMAT_void_GLenum_GLint: {
            PACKED_void_GLenum_GLint *unpacked = (PACKED_void_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLint args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLfloat_GLfloat: {
            PACKED_void_GLfloat_GLfloat *unpacked = (PACKED_void_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__: {
            PACKED_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__ *unpacked = (PACKED_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__ *)packed;
            ARGS_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7);
            break;
        }
        case FORMAT_void_GLenum_GLenum_GLsizei_GLsizei: {
            PACKED_void_GLenum_GLenum_GLsizei_GLsizei *unpacked = (PACKED_void_GLenum_GLenum_GLsizei_GLsizei *)packed;
            ARGS_void_GLenum_GLenum_GLsizei_GLsizei args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLclampf_GLboolean: {
            PACKED_void_GLclampf_GLboolean *unpacked = (PACKED_void_GLclampf_GLboolean *)packed;
            ARGS_void_GLclampf_GLboolean args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLint_GLint_GLsizei_GLsizei: {
            PACKED_void_GLint_GLint_GLsizei_GLsizei *unpacked = (PACKED_void_GLint_GLint_GLsizei_GLsizei *)packed;
            ARGS_void_GLint_GLint_GLsizei_GLsizei args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLsizei_const_GLuint___GENPT___GLenum_const_GLvoid___GENPT___GLsizei: {
            PACKED_void_GLsizei_const_GLuint___GENPT___GLenum_const_GLvoid___GENPT___GLsizei *unpacked = (PACKED_void_GLsizei_const_GLuint___GENPT___GLenum_const_GLvoid___GENPT___GLsizei *)packed;
            ARGS_void_GLsizei_const_GLuint___GENPT___GLenum_const_GLvoid___GENPT___GLsizei args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        case FORMAT_void_GLuint_GLsizei_const_GLchar___GENPT___const___GENPT___const_GLint___GENPT__: {
            PACKED_void_GLuint_GLsizei_const_GLchar___GENPT___const___GENPT___const_GLint___GENPT__ *unpacked = (PACKED_void_GLuint_GLsizei_const_GLchar___GENPT___const___GENPT___const_GLint___GENPT__ *)packed;
            ARGS_void_GLuint_GLsizei_const_GLchar___GENPT___const___GENPT___const_GLint___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLenum_GLint_GLuint: {
            PACKED_void_GLenum_GLint_GLuint *unpacked = (PACKED_void_GLenum_GLint_GLuint *)packed;
            ARGS_void_GLenum_GLint_GLuint args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLenum_GLenum_GLint_GLuint: {
            PACKED_void_GLenum_GLenum_GLint_GLuint *unpacked = (PACKED_void_GLenum_GLenum_GLint_GLuint *)packed;
            ARGS_void_GLenum_GLenum_GLint_GLuint args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLenum_GLenum_GLenum: {
            PACKED_void_GLenum_GLenum_GLenum *unpacked = (PACKED_void_GLenum_GLenum_GLenum *)packed;
            ARGS_void_GLenum_GLenum_GLenum args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__: {
            PACKED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ *unpacked = (PACKED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9);
            break;
        }
        case FORMAT_void_GLenum_GLenum_GLfloat: {
            PACKED_void_GLenum_GLenum_GLfloat *unpacked = (PACKED_void_GLenum_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLenum_GLfloat args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLenum_GLenum_const_GLfloat___GENPT__: {
            PACKED_void_GLenum_GLenum_const_GLfloat___GENPT__ *unpacked = (PACKED_void_GLenum_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLenum_GLenum_GLint: {
            PACKED_void_GLenum_GLenum_GLint *unpacked = (PACKED_void_GLenum_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLenum_GLint args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLenum_GLenum_const_GLint___GENPT__: {
            PACKED_void_GLenum_GLenum_const_GLint___GENPT__ *unpacked = (PACKED_void_GLenum_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLint___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__: {
            PACKED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *unpacked = (PACKED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9);
            break;
        }
        case FORMAT_void_GLint_GLfloat: {
            PACKED_void_GLint_GLfloat *unpacked = (PACKED_void_GLint_GLfloat *)packed;
            ARGS_void_GLint_GLfloat args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLint_GLsizei_const_GLfloat___GENPT__: {
            PACKED_void_GLint_GLsizei_const_GLfloat___GENPT__ *unpacked = (PACKED_void_GLint_GLsizei_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLint_GLsizei_const_GLfloat___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLint_GLint: {
            PACKED_void_GLint_GLint *unpacked = (PACKED_void_GLint_GLint *)packed;
            ARGS_void_GLint_GLint args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLint_GLsizei_const_GLint___GENPT__: {
            PACKED_void_GLint_GLsizei_const_GLint___GENPT__ *unpacked = (PACKED_void_GLint_GLsizei_const_GLint___GENPT__ *)packed;
            ARGS_void_GLint_GLsizei_const_GLint___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLint_GLfloat_GLfloat: {
            PACKED_void_GLint_GLfloat_GLfloat *unpacked = (PACKED_void_GLint_GLfloat_GLfloat *)packed;
            ARGS_void_GLint_GLfloat_GLfloat args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLint_GLint_GLint: {
            PACKED_void_GLint_GLint_GLint *unpacked = (PACKED_void_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLint_GLfloat_GLfloat_GLfloat: {
            PACKED_void_GLint_GLfloat_GLfloat_GLfloat *unpacked = (PACKED_void_GLint_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLint_GLfloat_GLfloat_GLfloat args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLint_GLint_GLint_GLint: {
            PACKED_void_GLint_GLint_GLint_GLint *unpacked = (PACKED_void_GLint_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint_GLint args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLint_GLfloat_GLfloat_GLfloat_GLfloat: {
            PACKED_void_GLint_GLfloat_GLfloat_GLfloat_GLfloat *unpacked = (PACKED_void_GLint_GLfloat_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLint_GLfloat_GLfloat_GLfloat_GLfloat args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        case FORMAT_void_GLint_GLint_GLint_GLint_GLint: {
            PACKED_void_GLint_GLint_GLint_GLint_GLint *unpacked = (PACKED_void_GLint_GLint_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint_GLint_GLint args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        case FORMAT_void_GLint_GLsizei_GLboolean_const_GLfloat___GENPT__: {
            PACKED_void_GLint_GLsizei_GLboolean_const_GLfloat___GENPT__ *unpacked = (PACKED_void_GLint_GLsizei_GLboolean_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLint_GLsizei_GLboolean_const_GLfloat___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLuint_GLfloat: {
            PACKED_void_GLuint_GLfloat *unpacked = (PACKED_void_GLuint_GLfloat *)packed;
            ARGS_void_GLuint_GLfloat args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLuint_const_GLfloat___GENPT__: {
            PACKED_void_GLuint_const_GLfloat___GENPT__ *unpacked = (PACKED_void_GLuint_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLuint_const_GLfloat___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2);
            break;
        }
        case FORMAT_void_GLuint_GLfloat_GLfloat: {
            PACKED_void_GLuint_GLfloat_GLfloat *unpacked = (PACKED_void_GLuint_GLfloat_GLfloat *)packed;
            ARGS_void_GLuint_GLfloat_GLfloat args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3);
            break;
        }
        case FORMAT_void_GLuint_GLfloat_GLfloat_GLfloat: {
            PACKED_void_GLuint_GLfloat_GLfloat_GLfloat *unpacked = (PACKED_void_GLuint_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLuint_GLfloat_GLfloat_GLfloat args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        case FORMAT_void_GLuint_GLfloat_GLfloat_GLfloat_GLfloat: {
            PACKED_void_GLuint_GLfloat_GLfloat_GLfloat_GLfloat *unpacked = (PACKED_void_GLuint_GLfloat_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLuint_GLfloat_GLfloat_GLfloat_GLfloat args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        case FORMAT_void_GLuint_GLint_GLenum_GLboolean_GLsizei_const_GLvoid___GENPT__: {
            PACKED_void_GLuint_GLint_GLenum_GLboolean_GLsizei_const_GLvoid___GENPT__ *unpacked = (PACKED_void_GLuint_GLint_GLenum_GLboolean_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLuint_GLint_GLenum_GLboolean_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            unpacked->func(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
    }
}
#ifndef skip_glActiveTexture
void glActiveTexture(GLenum texture) {
    LOAD_GLES(glActiveTexture);
#ifndef direct_glActiveTexture
    PUSH_IF_COMPILING(glActiveTexture)
#endif
    gles_glActiveTexture(texture);
}
#endif
#ifndef skip_glAttachShader
void glAttachShader(GLuint program, GLuint shader) {
    LOAD_GLES(glAttachShader);
#ifndef direct_glAttachShader
    PUSH_IF_COMPILING(glAttachShader)
#endif
    gles_glAttachShader(program, shader);
}
#endif
#ifndef skip_glBindAttribLocation
void glBindAttribLocation(GLuint program, GLuint index, const GLchar * name) {
    LOAD_GLES(glBindAttribLocation);
#ifndef direct_glBindAttribLocation
    PUSH_IF_COMPILING(glBindAttribLocation)
#endif
    gles_glBindAttribLocation(program, index, name);
}
#endif
#ifndef skip_glBindBuffer
void glBindBuffer(GLenum target, GLuint buffer) {
    LOAD_GLES(glBindBuffer);
#ifndef direct_glBindBuffer
    PUSH_IF_COMPILING(glBindBuffer)
#endif
    gles_glBindBuffer(target, buffer);
}
#endif
#ifndef skip_glBindFramebuffer
void glBindFramebuffer(GLenum target, GLuint framebuffer) {
    LOAD_GLES(glBindFramebuffer);
#ifndef direct_glBindFramebuffer
    PUSH_IF_COMPILING(glBindFramebuffer)
#endif
    gles_glBindFramebuffer(target, framebuffer);
}
#endif
#ifndef skip_glBindRenderbuffer
void glBindRenderbuffer(GLenum target, GLuint renderbuffer) {
    LOAD_GLES(glBindRenderbuffer);
#ifndef direct_glBindRenderbuffer
    PUSH_IF_COMPILING(glBindRenderbuffer)
#endif
    gles_glBindRenderbuffer(target, renderbuffer);
}
#endif
#ifndef skip_glBindTexture
void glBindTexture(GLenum target, GLuint texture) {
    LOAD_GLES(glBindTexture);
#ifndef direct_glBindTexture
    PUSH_IF_COMPILING(glBindTexture)
#endif
    gles_glBindTexture(target, texture);
}
#endif
#ifndef skip_glBlendColor
void glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
    LOAD_GLES(glBlendColor);
#ifndef direct_glBlendColor
    PUSH_IF_COMPILING(glBlendColor)
#endif
    gles_glBlendColor(red, green, blue, alpha);
}
#endif
#ifndef skip_glBlendEquation
void glBlendEquation(GLenum mode) {
    LOAD_GLES(glBlendEquation);
#ifndef direct_glBlendEquation
    PUSH_IF_COMPILING(glBlendEquation)
#endif
    gles_glBlendEquation(mode);
}
#endif
#ifndef skip_glBlendEquationSeparate
void glBlendEquationSeparate(GLenum modeRGB, GLenum modeA) {
    LOAD_GLES(glBlendEquationSeparate);
#ifndef direct_glBlendEquationSeparate
    PUSH_IF_COMPILING(glBlendEquationSeparate)
#endif
    gles_glBlendEquationSeparate(modeRGB, modeA);
}
#endif
#ifndef skip_glBlendFunc
void glBlendFunc(GLenum sfactor, GLenum dfactor) {
    LOAD_GLES(glBlendFunc);
#ifndef direct_glBlendFunc
    PUSH_IF_COMPILING(glBlendFunc)
#endif
    gles_glBlendFunc(sfactor, dfactor);
}
#endif
#ifndef skip_glBlendFuncSeparate
void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
    LOAD_GLES(glBlendFuncSeparate);
#ifndef direct_glBlendFuncSeparate
    PUSH_IF_COMPILING(glBlendFuncSeparate)
#endif
    gles_glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}
#endif
#ifndef skip_glBufferData
void glBufferData(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage) {
    LOAD_GLES(glBufferData);
#ifndef direct_glBufferData
    PUSH_IF_COMPILING(glBufferData)
#endif
    gles_glBufferData(target, size, data, usage);
}
#endif
#ifndef skip_glBufferSubData
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data) {
    LOAD_GLES(glBufferSubData);
#ifndef direct_glBufferSubData
    PUSH_IF_COMPILING(glBufferSubData)
#endif
    gles_glBufferSubData(target, offset, size, data);
}
#endif
#ifndef skip_glCheckFramebufferStatus
GLenum glCheckFramebufferStatus(GLenum target) {
    LOAD_GLES(glCheckFramebufferStatus);
#ifndef direct_glCheckFramebufferStatus
    PUSH_IF_COMPILING(glCheckFramebufferStatus)
#endif
    return gles_glCheckFramebufferStatus(target);
}
#endif
#ifndef skip_glClear
void glClear(GLbitfield mask) {
    LOAD_GLES(glClear);
#ifndef direct_glClear
    PUSH_IF_COMPILING(glClear)
#endif
    gles_glClear(mask);
}
#endif
#ifndef skip_glClearColor
void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
    LOAD_GLES(glClearColor);
#ifndef direct_glClearColor
    PUSH_IF_COMPILING(glClearColor)
#endif
    gles_glClearColor(red, green, blue, alpha);
}
#endif
#ifndef skip_glClearDepthf
void glClearDepthf(GLclampf depth) {
    LOAD_GLES(glClearDepthf);
#ifndef direct_glClearDepthf
    PUSH_IF_COMPILING(glClearDepthf)
#endif
    gles_glClearDepthf(depth);
}
#endif
#ifndef skip_glClearStencil
void glClearStencil(GLint s) {
    LOAD_GLES(glClearStencil);
#ifndef direct_glClearStencil
    PUSH_IF_COMPILING(glClearStencil)
#endif
    gles_glClearStencil(s);
}
#endif
#ifndef skip_glColorMask
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    LOAD_GLES(glColorMask);
#ifndef direct_glColorMask
    PUSH_IF_COMPILING(glColorMask)
#endif
    gles_glColorMask(red, green, blue, alpha);
}
#endif
#ifndef skip_glCompileShader
void glCompileShader(GLuint shader) {
    LOAD_GLES(glCompileShader);
#ifndef direct_glCompileShader
    PUSH_IF_COMPILING(glCompileShader)
#endif
    gles_glCompileShader(shader);
}
#endif
#ifndef skip_glCompressedTexImage2D
void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data) {
    LOAD_GLES(glCompressedTexImage2D);
#ifndef direct_glCompressedTexImage2D
    PUSH_IF_COMPILING(glCompressedTexImage2D)
#endif
    gles_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}
#endif
#ifndef skip_glCompressedTexSubImage2D
void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data) {
    LOAD_GLES(glCompressedTexSubImage2D);
#ifndef direct_glCompressedTexSubImage2D
    PUSH_IF_COMPILING(glCompressedTexSubImage2D)
#endif
    gles_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}
#endif
#ifndef skip_glCopyTexImage2D
void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    LOAD_GLES(glCopyTexImage2D);
#ifndef direct_glCopyTexImage2D
    PUSH_IF_COMPILING(glCopyTexImage2D)
#endif
    gles_glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}
#endif
#ifndef skip_glCopyTexSubImage2D
void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    LOAD_GLES(glCopyTexSubImage2D);
#ifndef direct_glCopyTexSubImage2D
    PUSH_IF_COMPILING(glCopyTexSubImage2D)
#endif
    gles_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}
#endif
#ifndef skip_glCreateProgram
GLuint glCreateProgram() {
    LOAD_GLES(glCreateProgram);
#ifndef direct_glCreateProgram
    PUSH_IF_COMPILING(glCreateProgram)
#endif
    return gles_glCreateProgram();
}
#endif
#ifndef skip_glCreateShader
GLuint glCreateShader(GLenum type) {
    LOAD_GLES(glCreateShader);
#ifndef direct_glCreateShader
    PUSH_IF_COMPILING(glCreateShader)
#endif
    return gles_glCreateShader(type);
}
#endif
#ifndef skip_glCullFace
void glCullFace(GLenum mode) {
    LOAD_GLES(glCullFace);
#ifndef direct_glCullFace
    PUSH_IF_COMPILING(glCullFace)
#endif
    gles_glCullFace(mode);
}
#endif
#ifndef skip_glDeleteBuffers
void glDeleteBuffers(GLsizei n, const GLuint * buffer) {
    LOAD_GLES(glDeleteBuffers);
#ifndef direct_glDeleteBuffers
    PUSH_IF_COMPILING(glDeleteBuffers)
#endif
    gles_glDeleteBuffers(n, buffer);
}
#endif
#ifndef skip_glDeleteFramebuffers
void glDeleteFramebuffers(GLsizei n, const GLuint * framebuffers) {
    LOAD_GLES(glDeleteFramebuffers);
#ifndef direct_glDeleteFramebuffers
    PUSH_IF_COMPILING(glDeleteFramebuffers)
#endif
    gles_glDeleteFramebuffers(n, framebuffers);
}
#endif
#ifndef skip_glDeleteProgram
void glDeleteProgram(GLuint program) {
    LOAD_GLES(glDeleteProgram);
#ifndef direct_glDeleteProgram
    PUSH_IF_COMPILING(glDeleteProgram)
#endif
    gles_glDeleteProgram(program);
}
#endif
#ifndef skip_glDeleteRenderbuffers
void glDeleteRenderbuffers(GLsizei n, const GLuint * renderbuffers) {
    LOAD_GLES(glDeleteRenderbuffers);
#ifndef direct_glDeleteRenderbuffers
    PUSH_IF_COMPILING(glDeleteRenderbuffers)
#endif
    gles_glDeleteRenderbuffers(n, renderbuffers);
}
#endif
#ifndef skip_glDeleteShader
void glDeleteShader(GLuint program) {
    LOAD_GLES(glDeleteShader);
#ifndef direct_glDeleteShader
    PUSH_IF_COMPILING(glDeleteShader)
#endif
    gles_glDeleteShader(program);
}
#endif
#ifndef skip_glDeleteTextures
void glDeleteTextures(GLsizei n, const GLuint * textures) {
    LOAD_GLES(glDeleteTextures);
#ifndef direct_glDeleteTextures
    PUSH_IF_COMPILING(glDeleteTextures)
#endif
    gles_glDeleteTextures(n, textures);
}
#endif
#ifndef skip_glDepthFunc
void glDepthFunc(GLenum func) {
    LOAD_GLES(glDepthFunc);
#ifndef direct_glDepthFunc
    PUSH_IF_COMPILING(glDepthFunc)
#endif
    gles_glDepthFunc(func);
}
#endif
#ifndef skip_glDepthMask
void glDepthMask(GLboolean flag) {
    LOAD_GLES(glDepthMask);
#ifndef direct_glDepthMask
    PUSH_IF_COMPILING(glDepthMask)
#endif
    gles_glDepthMask(flag);
}
#endif
#ifndef skip_glDepthRangef
void glDepthRangef(GLclampf zNear, GLclampf zFar) {
    LOAD_GLES(glDepthRangef);
#ifndef direct_glDepthRangef
    PUSH_IF_COMPILING(glDepthRangef)
#endif
    gles_glDepthRangef(zNear, zFar);
}
#endif
#ifndef skip_glDetachShader
void glDetachShader(GLuint program, GLuint shader) {
    LOAD_GLES(glDetachShader);
#ifndef direct_glDetachShader
    PUSH_IF_COMPILING(glDetachShader)
#endif
    gles_glDetachShader(program, shader);
}
#endif
#ifndef skip_glDisable
void glDisable(GLenum cap) {
    LOAD_GLES(glDisable);
#ifndef direct_glDisable
    PUSH_IF_COMPILING(glDisable)
#endif
    gles_glDisable(cap);
}
#endif
#ifndef skip_glDisableVertexAttribArray
void glDisableVertexAttribArray(GLuint index) {
    LOAD_GLES(glDisableVertexAttribArray);
#ifndef direct_glDisableVertexAttribArray
    PUSH_IF_COMPILING(glDisableVertexAttribArray)
#endif
    gles_glDisableVertexAttribArray(index);
}
#endif
#ifndef skip_glDrawArrays
void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    LOAD_GLES(glDrawArrays);
#ifndef direct_glDrawArrays
    PUSH_IF_COMPILING(glDrawArrays)
#endif
    gles_glDrawArrays(mode, first, count);
}
#endif
#ifndef skip_glDrawElements
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices) {
    LOAD_GLES(glDrawElements);
#ifndef direct_glDrawElements
    PUSH_IF_COMPILING(glDrawElements)
#endif
    gles_glDrawElements(mode, count, type, indices);
}
#endif
#ifndef skip_glEnable
void glEnable(GLenum cap) {
    LOAD_GLES(glEnable);
#ifndef direct_glEnable
    PUSH_IF_COMPILING(glEnable)
#endif
    gles_glEnable(cap);
}
#endif
#ifndef skip_glEnableVertexAttribArray
void glEnableVertexAttribArray(GLuint index) {
    LOAD_GLES(glEnableVertexAttribArray);
#ifndef direct_glEnableVertexAttribArray
    PUSH_IF_COMPILING(glEnableVertexAttribArray)
#endif
    gles_glEnableVertexAttribArray(index);
}
#endif
#ifndef skip_glFinish
void glFinish() {
    LOAD_GLES(glFinish);
#ifndef direct_glFinish
    PUSH_IF_COMPILING(glFinish)
#endif
    gles_glFinish();
}
#endif
#ifndef skip_glFlush
void glFlush() {
    LOAD_GLES(glFlush);
#ifndef direct_glFlush
    PUSH_IF_COMPILING(glFlush)
#endif
    gles_glFlush();
}
#endif
#ifndef skip_glFramebufferRenderbuffer
void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
    LOAD_GLES(glFramebufferRenderbuffer);
#ifndef direct_glFramebufferRenderbuffer
    PUSH_IF_COMPILING(glFramebufferRenderbuffer)
#endif
    gles_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}
#endif
#ifndef skip_glFramebufferTexture2D
void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    LOAD_GLES(glFramebufferTexture2D);
#ifndef direct_glFramebufferTexture2D
    PUSH_IF_COMPILING(glFramebufferTexture2D)
#endif
    gles_glFramebufferTexture2D(target, attachment, textarget, texture, level);
}
#endif
#ifndef skip_glFrontFace
void glFrontFace(GLenum mode) {
    LOAD_GLES(glFrontFace);
#ifndef direct_glFrontFace
    PUSH_IF_COMPILING(glFrontFace)
#endif
    gles_glFrontFace(mode);
}
#endif
#ifndef skip_glGenBuffers
void glGenBuffers(GLsizei n, GLuint * buffer) {
    LOAD_GLES(glGenBuffers);
#ifndef direct_glGenBuffers
    PUSH_IF_COMPILING(glGenBuffers)
#endif
    gles_glGenBuffers(n, buffer);
}
#endif
#ifndef skip_glGenFramebuffers
void glGenFramebuffers(GLsizei n, GLuint * framebuffers) {
    LOAD_GLES(glGenFramebuffers);
#ifndef direct_glGenFramebuffers
    PUSH_IF_COMPILING(glGenFramebuffers)
#endif
    gles_glGenFramebuffers(n, framebuffers);
}
#endif
#ifndef skip_glGenRenderbuffers
void glGenRenderbuffers(GLsizei n, GLuint * renderbuffers) {
    LOAD_GLES(glGenRenderbuffers);
#ifndef direct_glGenRenderbuffers
    PUSH_IF_COMPILING(glGenRenderbuffers)
#endif
    gles_glGenRenderbuffers(n, renderbuffers);
}
#endif
#ifndef skip_glGenTextures
void glGenTextures(GLsizei n, GLuint * textures) {
    LOAD_GLES(glGenTextures);
#ifndef direct_glGenTextures
    PUSH_IF_COMPILING(glGenTextures)
#endif
    gles_glGenTextures(n, textures);
}
#endif
#ifndef skip_glGenerateMipmap
void glGenerateMipmap(GLenum target) {
    LOAD_GLES(glGenerateMipmap);
#ifndef direct_glGenerateMipmap
    PUSH_IF_COMPILING(glGenerateMipmap)
#endif
    gles_glGenerateMipmap(target);
}
#endif
#ifndef skip_glGetActiveAttrib
void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name) {
    LOAD_GLES(glGetActiveAttrib);
#ifndef direct_glGetActiveAttrib
    PUSH_IF_COMPILING(glGetActiveAttrib)
#endif
    gles_glGetActiveAttrib(program, index, bufSize, length, size, type, name);
}
#endif
#ifndef skip_glGetActiveUniform
void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name) {
    LOAD_GLES(glGetActiveUniform);
#ifndef direct_glGetActiveUniform
    PUSH_IF_COMPILING(glGetActiveUniform)
#endif
    gles_glGetActiveUniform(program, index, bufSize, length, size, type, name);
}
#endif
#ifndef skip_glGetAttachedShaders
void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei * count, GLuint * obj) {
    LOAD_GLES(glGetAttachedShaders);
#ifndef direct_glGetAttachedShaders
    PUSH_IF_COMPILING(glGetAttachedShaders)
#endif
    gles_glGetAttachedShaders(program, maxCount, count, obj);
}
#endif
#ifndef skip_glGetAttribLocation
GLint glGetAttribLocation(GLuint program, const GLchar * name) {
    LOAD_GLES(glGetAttribLocation);
#ifndef direct_glGetAttribLocation
    PUSH_IF_COMPILING(glGetAttribLocation)
#endif
    return gles_glGetAttribLocation(program, name);
}
#endif
#ifndef skip_glGetBooleanv
void glGetBooleanv(GLenum pname, GLboolean * params) {
    LOAD_GLES(glGetBooleanv);
#ifndef direct_glGetBooleanv
    PUSH_IF_COMPILING(glGetBooleanv)
#endif
    gles_glGetBooleanv(pname, params);
}
#endif
#ifndef skip_glGetBufferParameteriv
void glGetBufferParameteriv(GLenum target, GLenum pname, GLint * params) {
    LOAD_GLES(glGetBufferParameteriv);
#ifndef direct_glGetBufferParameteriv
    PUSH_IF_COMPILING(glGetBufferParameteriv)
#endif
    gles_glGetBufferParameteriv(target, pname, params);
}
#endif
#ifndef skip_glGetError
GLenum glGetError() {
    LOAD_GLES(glGetError);
#ifndef direct_glGetError
    PUSH_IF_COMPILING(glGetError)
#endif
    return gles_glGetError();
}
#endif
#ifndef skip_glGetFloatv
void glGetFloatv(GLenum pname, GLfloat * params) {
    LOAD_GLES(glGetFloatv);
#ifndef direct_glGetFloatv
    PUSH_IF_COMPILING(glGetFloatv)
#endif
    gles_glGetFloatv(pname, params);
}
#endif
#ifndef skip_glGetFramebufferAttachmentParameteriv
void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint * params) {
    LOAD_GLES(glGetFramebufferAttachmentParameteriv);
#ifndef direct_glGetFramebufferAttachmentParameteriv
    PUSH_IF_COMPILING(glGetFramebufferAttachmentParameteriv)
#endif
    gles_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}
#endif
#ifndef skip_glGetIntegerv
void glGetIntegerv(GLenum pname, GLint * params) {
    LOAD_GLES(glGetIntegerv);
#ifndef direct_glGetIntegerv
    PUSH_IF_COMPILING(glGetIntegerv)
#endif
    gles_glGetIntegerv(pname, params);
}
#endif
#ifndef skip_glGetProgramInfoLog
void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog) {
    LOAD_GLES(glGetProgramInfoLog);
#ifndef direct_glGetProgramInfoLog
    PUSH_IF_COMPILING(glGetProgramInfoLog)
#endif
    gles_glGetProgramInfoLog(program, bufSize, length, infoLog);
}
#endif
#ifndef skip_glGetProgramiv
void glGetProgramiv(GLuint program, GLenum pname, GLint * params) {
    LOAD_GLES(glGetProgramiv);
#ifndef direct_glGetProgramiv
    PUSH_IF_COMPILING(glGetProgramiv)
#endif
    gles_glGetProgramiv(program, pname, params);
}
#endif
#ifndef skip_glGetRenderbufferParameteriv
void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint * params) {
    LOAD_GLES(glGetRenderbufferParameteriv);
#ifndef direct_glGetRenderbufferParameteriv
    PUSH_IF_COMPILING(glGetRenderbufferParameteriv)
#endif
    gles_glGetRenderbufferParameteriv(target, pname, params);
}
#endif
#ifndef skip_glGetShaderInfoLog
void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog) {
    LOAD_GLES(glGetShaderInfoLog);
#ifndef direct_glGetShaderInfoLog
    PUSH_IF_COMPILING(glGetShaderInfoLog)
#endif
    gles_glGetShaderInfoLog(shader, bufSize, length, infoLog);
}
#endif
#ifndef skip_glGetShaderPrecisionFormat
void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint * range, GLint * precision) {
    LOAD_GLES(glGetShaderPrecisionFormat);
#ifndef direct_glGetShaderPrecisionFormat
    PUSH_IF_COMPILING(glGetShaderPrecisionFormat)
#endif
    gles_glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
}
#endif
#ifndef skip_glGetShaderSource
void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source) {
    LOAD_GLES(glGetShaderSource);
#ifndef direct_glGetShaderSource
    PUSH_IF_COMPILING(glGetShaderSource)
#endif
    gles_glGetShaderSource(shader, bufSize, length, source);
}
#endif
#ifndef skip_glGetShaderiv
void glGetShaderiv(GLuint shader, GLenum pname, GLint * params) {
    LOAD_GLES(glGetShaderiv);
#ifndef direct_glGetShaderiv
    PUSH_IF_COMPILING(glGetShaderiv)
#endif
    gles_glGetShaderiv(shader, pname, params);
}
#endif
#ifndef skip_glGetString
const GLubyte * glGetString(GLenum name) {
    LOAD_GLES(glGetString);
#ifndef direct_glGetString
    PUSH_IF_COMPILING(glGetString)
#endif
    return gles_glGetString(name);
}
#endif
#ifndef skip_glGetTexParameterfv
void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat * params) {
    LOAD_GLES(glGetTexParameterfv);
#ifndef direct_glGetTexParameterfv
    PUSH_IF_COMPILING(glGetTexParameterfv)
#endif
    gles_glGetTexParameterfv(target, pname, params);
}
#endif
#ifndef skip_glGetTexParameteriv
void glGetTexParameteriv(GLenum target, GLenum pname, GLint * params) {
    LOAD_GLES(glGetTexParameteriv);
#ifndef direct_glGetTexParameteriv
    PUSH_IF_COMPILING(glGetTexParameteriv)
#endif
    gles_glGetTexParameteriv(target, pname, params);
}
#endif
#ifndef skip_glGetUniformLocation
GLint glGetUniformLocation(GLuint program, const GLchar * name) {
    LOAD_GLES(glGetUniformLocation);
#ifndef direct_glGetUniformLocation
    PUSH_IF_COMPILING(glGetUniformLocation)
#endif
    return gles_glGetUniformLocation(program, name);
}
#endif
#ifndef skip_glGetUniformfv
void glGetUniformfv(GLuint program, GLint location, GLfloat * params) {
    LOAD_GLES(glGetUniformfv);
#ifndef direct_glGetUniformfv
    PUSH_IF_COMPILING(glGetUniformfv)
#endif
    gles_glGetUniformfv(program, location, params);
}
#endif
#ifndef skip_glGetUniformiv
void glGetUniformiv(GLuint program, GLint location, GLint * params) {
    LOAD_GLES(glGetUniformiv);
#ifndef direct_glGetUniformiv
    PUSH_IF_COMPILING(glGetUniformiv)
#endif
    gles_glGetUniformiv(program, location, params);
}
#endif
#ifndef skip_glGetVertexAttribPointerv
void glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid ** pointer) {
    LOAD_GLES(glGetVertexAttribPointerv);
#ifndef direct_glGetVertexAttribPointerv
    PUSH_IF_COMPILING(glGetVertexAttribPointerv)
#endif
    gles_glGetVertexAttribPointerv(index, pname, pointer);
}
#endif
#ifndef skip_glGetVertexAttribfv
void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat * params) {
    LOAD_GLES(glGetVertexAttribfv);
#ifndef direct_glGetVertexAttribfv
    PUSH_IF_COMPILING(glGetVertexAttribfv)
#endif
    gles_glGetVertexAttribfv(index, pname, params);
}
#endif
#ifndef skip_glGetVertexAttribiv
void glGetVertexAttribiv(GLuint index, GLenum pname, GLint * params) {
    LOAD_GLES(glGetVertexAttribiv);
#ifndef direct_glGetVertexAttribiv
    PUSH_IF_COMPILING(glGetVertexAttribiv)
#endif
    gles_glGetVertexAttribiv(index, pname, params);
}
#endif
#ifndef skip_glHint
void glHint(GLenum target, GLenum mode) {
    LOAD_GLES(glHint);
#ifndef direct_glHint
    PUSH_IF_COMPILING(glHint)
#endif
    gles_glHint(target, mode);
}
#endif
#ifndef skip_glIsBuffer
GLboolean glIsBuffer(GLuint buffer) {
    LOAD_GLES(glIsBuffer);
#ifndef direct_glIsBuffer
    PUSH_IF_COMPILING(glIsBuffer)
#endif
    return gles_glIsBuffer(buffer);
}
#endif
#ifndef skip_glIsEnabled
GLboolean glIsEnabled(GLenum cap) {
    LOAD_GLES(glIsEnabled);
#ifndef direct_glIsEnabled
    PUSH_IF_COMPILING(glIsEnabled)
#endif
    return gles_glIsEnabled(cap);
}
#endif
#ifndef skip_glIsFramebuffer
GLboolean glIsFramebuffer(GLuint framebuffer) {
    LOAD_GLES(glIsFramebuffer);
#ifndef direct_glIsFramebuffer
    PUSH_IF_COMPILING(glIsFramebuffer)
#endif
    return gles_glIsFramebuffer(framebuffer);
}
#endif
#ifndef skip_glIsProgram
GLboolean glIsProgram(GLuint program) {
    LOAD_GLES(glIsProgram);
#ifndef direct_glIsProgram
    PUSH_IF_COMPILING(glIsProgram)
#endif
    return gles_glIsProgram(program);
}
#endif
#ifndef skip_glIsRenderbuffer
GLboolean glIsRenderbuffer(GLuint renderbuffer) {
    LOAD_GLES(glIsRenderbuffer);
#ifndef direct_glIsRenderbuffer
    PUSH_IF_COMPILING(glIsRenderbuffer)
#endif
    return gles_glIsRenderbuffer(renderbuffer);
}
#endif
#ifndef skip_glIsShader
GLboolean glIsShader(GLuint shader) {
    LOAD_GLES(glIsShader);
#ifndef direct_glIsShader
    PUSH_IF_COMPILING(glIsShader)
#endif
    return gles_glIsShader(shader);
}
#endif
#ifndef skip_glIsTexture
GLboolean glIsTexture(GLuint texture) {
    LOAD_GLES(glIsTexture);
#ifndef direct_glIsTexture
    PUSH_IF_COMPILING(glIsTexture)
#endif
    return gles_glIsTexture(texture);
}
#endif
#ifndef skip_glLineWidth
void glLineWidth(GLfloat width) {
    LOAD_GLES(glLineWidth);
#ifndef direct_glLineWidth
    PUSH_IF_COMPILING(glLineWidth)
#endif
    gles_glLineWidth(width);
}
#endif
#ifndef skip_glLinkProgram
void glLinkProgram(GLuint program) {
    LOAD_GLES(glLinkProgram);
#ifndef direct_glLinkProgram
    PUSH_IF_COMPILING(glLinkProgram)
#endif
    gles_glLinkProgram(program);
}
#endif
#ifndef skip_glPixelStorei
void glPixelStorei(GLenum pname, GLint param) {
    LOAD_GLES(glPixelStorei);
#ifndef direct_glPixelStorei
    PUSH_IF_COMPILING(glPixelStorei)
#endif
    gles_glPixelStorei(pname, param);
}
#endif
#ifndef skip_glPolygonOffset
void glPolygonOffset(GLfloat factor, GLfloat units) {
    LOAD_GLES(glPolygonOffset);
#ifndef direct_glPolygonOffset
    PUSH_IF_COMPILING(glPolygonOffset)
#endif
    gles_glPolygonOffset(factor, units);
}
#endif
#ifndef skip_glReadPixels
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * pixels) {
    LOAD_GLES(glReadPixels);
#ifndef direct_glReadPixels
    PUSH_IF_COMPILING(glReadPixels)
#endif
    gles_glReadPixels(x, y, width, height, format, type, pixels);
}
#endif
#ifndef skip_glReleaseShaderCompiler
void glReleaseShaderCompiler() {
    LOAD_GLES(glReleaseShaderCompiler);
#ifndef direct_glReleaseShaderCompiler
    PUSH_IF_COMPILING(glReleaseShaderCompiler)
#endif
    gles_glReleaseShaderCompiler();
}
#endif
#ifndef skip_glRenderbufferStorage
void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
    LOAD_GLES(glRenderbufferStorage);
#ifndef direct_glRenderbufferStorage
    PUSH_IF_COMPILING(glRenderbufferStorage)
#endif
    gles_glRenderbufferStorage(target, internalformat, width, height);
}
#endif
#ifndef skip_glSampleCoverage
void glSampleCoverage(GLclampf value, GLboolean invert) {
    LOAD_GLES(glSampleCoverage);
#ifndef direct_glSampleCoverage
    PUSH_IF_COMPILING(glSampleCoverage)
#endif
    gles_glSampleCoverage(value, invert);
}
#endif
#ifndef skip_glScissor
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
    LOAD_GLES(glScissor);
#ifndef direct_glScissor
    PUSH_IF_COMPILING(glScissor)
#endif
    gles_glScissor(x, y, width, height);
}
#endif
#ifndef skip_glShaderBinary
void glShaderBinary(GLsizei n, const GLuint * shaders, GLenum binaryformat, const GLvoid * binary, GLsizei length) {
    LOAD_GLES(glShaderBinary);
#ifndef direct_glShaderBinary
    PUSH_IF_COMPILING(glShaderBinary)
#endif
    gles_glShaderBinary(n, shaders, binaryformat, binary, length);
}
#endif
#ifndef skip_glShaderSource
void glShaderSource(GLuint shader, GLsizei count, const GLchar * const * string, const GLint * length) {
    LOAD_GLES(glShaderSource);
#ifndef direct_glShaderSource
    PUSH_IF_COMPILING(glShaderSource)
#endif
    gles_glShaderSource(shader, count, string, length);
}
#endif
#ifndef skip_glStencilFunc
void glStencilFunc(GLenum func, GLint ref, GLuint mask) {
    LOAD_GLES(glStencilFunc);
#ifndef direct_glStencilFunc
    PUSH_IF_COMPILING(glStencilFunc)
#endif
    gles_glStencilFunc(func, ref, mask);
}
#endif
#ifndef skip_glStencilFuncSeparate
void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
    LOAD_GLES(glStencilFuncSeparate);
#ifndef direct_glStencilFuncSeparate
    PUSH_IF_COMPILING(glStencilFuncSeparate)
#endif
    gles_glStencilFuncSeparate(face, func, ref, mask);
}
#endif
#ifndef skip_glStencilMask
void glStencilMask(GLuint mask) {
    LOAD_GLES(glStencilMask);
#ifndef direct_glStencilMask
    PUSH_IF_COMPILING(glStencilMask)
#endif
    gles_glStencilMask(mask);
}
#endif
#ifndef skip_glStencilMaskSeparate
void glStencilMaskSeparate(GLenum face, GLuint mask) {
    LOAD_GLES(glStencilMaskSeparate);
#ifndef direct_glStencilMaskSeparate
    PUSH_IF_COMPILING(glStencilMaskSeparate)
#endif
    gles_glStencilMaskSeparate(face, mask);
}
#endif
#ifndef skip_glStencilOp
void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
    LOAD_GLES(glStencilOp);
#ifndef direct_glStencilOp
    PUSH_IF_COMPILING(glStencilOp)
#endif
    gles_glStencilOp(fail, zfail, zpass);
}
#endif
#ifndef skip_glStencilOpSeparate
void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum zfail, GLenum zpass) {
    LOAD_GLES(glStencilOpSeparate);
#ifndef direct_glStencilOpSeparate
    PUSH_IF_COMPILING(glStencilOpSeparate)
#endif
    gles_glStencilOpSeparate(face, sfail, zfail, zpass);
}
#endif
#ifndef skip_glTexImage2D
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels) {
    LOAD_GLES(glTexImage2D);
#ifndef direct_glTexImage2D
    PUSH_IF_COMPILING(glTexImage2D)
#endif
    gles_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}
#endif
#ifndef skip_glTexParameterf
void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
    LOAD_GLES(glTexParameterf);
#ifndef direct_glTexParameterf
    PUSH_IF_COMPILING(glTexParameterf)
#endif
    gles_glTexParameterf(target, pname, param);
}
#endif
#ifndef skip_glTexParameterfv
void glTexParameterfv(GLenum target, GLenum pname, const GLfloat * params) {
    LOAD_GLES(glTexParameterfv);
#ifndef direct_glTexParameterfv
    PUSH_IF_COMPILING(glTexParameterfv)
#endif
    gles_glTexParameterfv(target, pname, params);
}
#endif
#ifndef skip_glTexParameteri
void glTexParameteri(GLenum target, GLenum pname, GLint param) {
    LOAD_GLES(glTexParameteri);
#ifndef direct_glTexParameteri
    PUSH_IF_COMPILING(glTexParameteri)
#endif
    gles_glTexParameteri(target, pname, param);
}
#endif
#ifndef skip_glTexParameteriv
void glTexParameteriv(GLenum target, GLenum pname, const GLint * params) {
    LOAD_GLES(glTexParameteriv);
#ifndef direct_glTexParameteriv
    PUSH_IF_COMPILING(glTexParameteriv)
#endif
    gles_glTexParameteriv(target, pname, params);
}
#endif
#ifndef skip_glTexSubImage2D
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels) {
    LOAD_GLES(glTexSubImage2D);
#ifndef direct_glTexSubImage2D
    PUSH_IF_COMPILING(glTexSubImage2D)
#endif
    gles_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}
#endif
#ifndef skip_glUniform1f
void glUniform1f(GLint location, GLfloat v0) {
    LOAD_GLES(glUniform1f);
#ifndef direct_glUniform1f
    PUSH_IF_COMPILING(glUniform1f)
#endif
    gles_glUniform1f(location, v0);
}
#endif
#ifndef skip_glUniform1fv
void glUniform1fv(GLint location, GLsizei count, const GLfloat * value) {
    LOAD_GLES(glUniform1fv);
#ifndef direct_glUniform1fv
    PUSH_IF_COMPILING(glUniform1fv)
#endif
    gles_glUniform1fv(location, count, value);
}
#endif
#ifndef skip_glUniform1i
void glUniform1i(GLint location, GLint v0) {
    LOAD_GLES(glUniform1i);
#ifndef direct_glUniform1i
    PUSH_IF_COMPILING(glUniform1i)
#endif
    gles_glUniform1i(location, v0);
}
#endif
#ifndef skip_glUniform1iv
void glUniform1iv(GLint location, GLsizei count, const GLint * value) {
    LOAD_GLES(glUniform1iv);
#ifndef direct_glUniform1iv
    PUSH_IF_COMPILING(glUniform1iv)
#endif
    gles_glUniform1iv(location, count, value);
}
#endif
#ifndef skip_glUniform2f
void glUniform2f(GLint location, GLfloat v0, GLfloat v1) {
    LOAD_GLES(glUniform2f);
#ifndef direct_glUniform2f
    PUSH_IF_COMPILING(glUniform2f)
#endif
    gles_glUniform2f(location, v0, v1);
}
#endif
#ifndef skip_glUniform2fv
void glUniform2fv(GLint location, GLsizei count, const GLfloat * value) {
    LOAD_GLES(glUniform2fv);
#ifndef direct_glUniform2fv
    PUSH_IF_COMPILING(glUniform2fv)
#endif
    gles_glUniform2fv(location, count, value);
}
#endif
#ifndef skip_glUniform2i
void glUniform2i(GLint location, GLint v0, GLint v1) {
    LOAD_GLES(glUniform2i);
#ifndef direct_glUniform2i
    PUSH_IF_COMPILING(glUniform2i)
#endif
    gles_glUniform2i(location, v0, v1);
}
#endif
#ifndef skip_glUniform2iv
void glUniform2iv(GLint location, GLsizei count, const GLint * value) {
    LOAD_GLES(glUniform2iv);
#ifndef direct_glUniform2iv
    PUSH_IF_COMPILING(glUniform2iv)
#endif
    gles_glUniform2iv(location, count, value);
}
#endif
#ifndef skip_glUniform3f
void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    LOAD_GLES(glUniform3f);
#ifndef direct_glUniform3f
    PUSH_IF_COMPILING(glUniform3f)
#endif
    gles_glUniform3f(location, v0, v1, v2);
}
#endif
#ifndef skip_glUniform3fv
void glUniform3fv(GLint location, GLsizei count, const GLfloat * value) {
    LOAD_GLES(glUniform3fv);
#ifndef direct_glUniform3fv
    PUSH_IF_COMPILING(glUniform3fv)
#endif
    gles_glUniform3fv(location, count, value);
}
#endif
#ifndef skip_glUniform3i
void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) {
    LOAD_GLES(glUniform3i);
#ifndef direct_glUniform3i
    PUSH_IF_COMPILING(glUniform3i)
#endif
    gles_glUniform3i(location, v0, v1, v2);
}
#endif
#ifndef skip_glUniform3iv
void glUniform3iv(GLint location, GLsizei count, const GLint * value) {
    LOAD_GLES(glUniform3iv);
#ifndef direct_glUniform3iv
    PUSH_IF_COMPILING(glUniform3iv)
#endif
    gles_glUniform3iv(location, count, value);
}
#endif
#ifndef skip_glUniform4f
void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    LOAD_GLES(glUniform4f);
#ifndef direct_glUniform4f
    PUSH_IF_COMPILING(glUniform4f)
#endif
    gles_glUniform4f(location, v0, v1, v2, v3);
}
#endif
#ifndef skip_glUniform4fv
void glUniform4fv(GLint location, GLsizei count, const GLfloat * value) {
    LOAD_GLES(glUniform4fv);
#ifndef direct_glUniform4fv
    PUSH_IF_COMPILING(glUniform4fv)
#endif
    gles_glUniform4fv(location, count, value);
}
#endif
#ifndef skip_glUniform4i
void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
    LOAD_GLES(glUniform4i);
#ifndef direct_glUniform4i
    PUSH_IF_COMPILING(glUniform4i)
#endif
    gles_glUniform4i(location, v0, v1, v2, v3);
}
#endif
#ifndef skip_glUniform4iv
void glUniform4iv(GLint location, GLsizei count, const GLint * value) {
    LOAD_GLES(glUniform4iv);
#ifndef direct_glUniform4iv
    PUSH_IF_COMPILING(glUniform4iv)
#endif
    gles_glUniform4iv(location, count, value);
}
#endif
#ifndef skip_glUniformMatrix2fv
void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    LOAD_GLES(glUniformMatrix2fv);
#ifndef direct_glUniformMatrix2fv
    PUSH_IF_COMPILING(glUniformMatrix2fv)
#endif
    gles_glUniformMatrix2fv(location, count, transpose, value);
}
#endif
#ifndef skip_glUniformMatrix3fv
void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    LOAD_GLES(glUniformMatrix3fv);
#ifndef direct_glUniformMatrix3fv
    PUSH_IF_COMPILING(glUniformMatrix3fv)
#endif
    gles_glUniformMatrix3fv(location, count, transpose, value);
}
#endif
#ifndef skip_glUniformMatrix4fv
void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    LOAD_GLES(glUniformMatrix4fv);
#ifndef direct_glUniformMatrix4fv
    PUSH_IF_COMPILING(glUniformMatrix4fv)
#endif
    gles_glUniformMatrix4fv(location, count, transpose, value);
}
#endif
#ifndef skip_glUseProgram
void glUseProgram(GLuint program) {
    LOAD_GLES(glUseProgram);
#ifndef direct_glUseProgram
    PUSH_IF_COMPILING(glUseProgram)
#endif
    gles_glUseProgram(program);
}
#endif
#ifndef skip_glValidateProgram
void glValidateProgram(GLuint program) {
    LOAD_GLES(glValidateProgram);
#ifndef direct_glValidateProgram
    PUSH_IF_COMPILING(glValidateProgram)
#endif
    gles_glValidateProgram(program);
}
#endif
#ifndef skip_glVertexAttrib1f
void glVertexAttrib1f(GLuint index, GLfloat x) {
    LOAD_GLES(glVertexAttrib1f);
#ifndef direct_glVertexAttrib1f
    PUSH_IF_COMPILING(glVertexAttrib1f)
#endif
    gles_glVertexAttrib1f(index, x);
}
#endif
#ifndef skip_glVertexAttrib1fv
void glVertexAttrib1fv(GLuint index, const GLfloat * v) {
    LOAD_GLES(glVertexAttrib1fv);
#ifndef direct_glVertexAttrib1fv
    PUSH_IF_COMPILING(glVertexAttrib1fv)
#endif
    gles_glVertexAttrib1fv(index, v);
}
#endif
#ifndef skip_glVertexAttrib2f
void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y) {
    LOAD_GLES(glVertexAttrib2f);
#ifndef direct_glVertexAttrib2f
    PUSH_IF_COMPILING(glVertexAttrib2f)
#endif
    gles_glVertexAttrib2f(index, x, y);
}
#endif
#ifndef skip_glVertexAttrib2fv
void glVertexAttrib2fv(GLuint index, const GLfloat * v) {
    LOAD_GLES(glVertexAttrib2fv);
#ifndef direct_glVertexAttrib2fv
    PUSH_IF_COMPILING(glVertexAttrib2fv)
#endif
    gles_glVertexAttrib2fv(index, v);
}
#endif
#ifndef skip_glVertexAttrib3f
void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) {
    LOAD_GLES(glVertexAttrib3f);
#ifndef direct_glVertexAttrib3f
    PUSH_IF_COMPILING(glVertexAttrib3f)
#endif
    gles_glVertexAttrib3f(index, x, y, z);
}
#endif
#ifndef skip_glVertexAttrib3fv
void glVertexAttrib3fv(GLuint index, const GLfloat * v) {
    LOAD_GLES(glVertexAttrib3fv);
#ifndef direct_glVertexAttrib3fv
    PUSH_IF_COMPILING(glVertexAttrib3fv)
#endif
    gles_glVertexAttrib3fv(index, v);
}
#endif
#ifndef skip_glVertexAttrib4f
void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    LOAD_GLES(glVertexAttrib4f);
#ifndef direct_glVertexAttrib4f
    PUSH_IF_COMPILING(glVertexAttrib4f)
#endif
    gles_glVertexAttrib4f(index, x, y, z, w);
}
#endif
#ifndef skip_glVertexAttrib4fv
void glVertexAttrib4fv(GLuint index, const GLfloat * v) {
    LOAD_GLES(glVertexAttrib4fv);
#ifndef direct_glVertexAttrib4fv
    PUSH_IF_COMPILING(glVertexAttrib4fv)
#endif
    gles_glVertexAttrib4fv(index, v);
}
#endif
#ifndef skip_glVertexAttribPointer
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer) {
    LOAD_GLES(glVertexAttribPointer);
#ifndef direct_glVertexAttribPointer
    PUSH_IF_COMPILING(glVertexAttribPointer)
#endif
    gles_glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}
#endif
#ifndef skip_glViewport
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    LOAD_GLES(glViewport);
#ifndef direct_glViewport
    PUSH_IF_COMPILING(glViewport)
#endif
    gles_glViewport(x, y, width, height);
}
#endif
#endif
