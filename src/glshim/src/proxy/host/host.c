#include "../proxy.h"

#ifndef HOST_C
#define HOST_C
void glIndexedCall(const indexed_call_t *packed, void *ret_v) {
    switch (packed->func) {
        #ifndef skip_index_glAccum
        case glAccum_INDEX: {
            INDEXED_void_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLfloat args = unpacked->args;
            glAccum(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glActiveTexture
        case glActiveTexture_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glActiveTexture(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glAlphaFunc
        case glAlphaFunc_INDEX: {
            INDEXED_void_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLfloat args = unpacked->args;
            glAlphaFunc(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glAreTexturesResident
        case glAreTexturesResident_INDEX: {
            INDEXED_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__ *unpacked = (INDEXED_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__ *)packed;
            ARGS_GLboolean_GLsizei_const_GLuint___GENPT___GLboolean___GENPT__ args = unpacked->args;
            GLboolean *ret = (GLboolean *)ret_v;
            *ret =
            glAreTexturesResident(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glArrayElement
        case glArrayElement_INDEX: {
            INDEXED_void_GLint *unpacked = (INDEXED_void_GLint *)packed;
            ARGS_void_GLint args = unpacked->args;
            glArrayElement(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glBegin
        case glBegin_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glBegin(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glBeginQuery
        case glBeginQuery_INDEX: {
            INDEXED_void_GLenum_GLuint *unpacked = (INDEXED_void_GLenum_GLuint *)packed;
            ARGS_void_GLenum_GLuint args = unpacked->args;
            glBeginQuery(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glBindBuffer
        case glBindBuffer_INDEX: {
            INDEXED_void_GLenum_GLuint *unpacked = (INDEXED_void_GLenum_GLuint *)packed;
            ARGS_void_GLenum_GLuint args = unpacked->args;
            glBindBuffer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glBindTexture
        case glBindTexture_INDEX: {
            INDEXED_void_GLenum_GLuint *unpacked = (INDEXED_void_GLenum_GLuint *)packed;
            ARGS_void_GLenum_GLuint args = unpacked->args;
            glBindTexture(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glBitmap
        case glBitmap_INDEX: {
            INDEXED_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__ *unpacked = (INDEXED_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__ *)packed;
            ARGS_void_GLsizei_GLsizei_GLfloat_GLfloat_GLfloat_GLfloat_const_GLubyte___GENPT__ args = unpacked->args;
            glBitmap(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7);
            break;
        }
        #endif
        #ifndef skip_index_glBlendColor
        case glBlendColor_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glBlendColor(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glBlendEquation
        case glBlendEquation_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glBlendEquation(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glBlendFunc
        case glBlendFunc_INDEX: {
            INDEXED_void_GLenum_GLenum *unpacked = (INDEXED_void_GLenum_GLenum *)packed;
            ARGS_void_GLenum_GLenum args = unpacked->args;
            glBlendFunc(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glBlendFuncSeparate
        case glBlendFuncSeparate_INDEX: {
            INDEXED_void_GLenum_GLenum_GLenum_GLenum *unpacked = (INDEXED_void_GLenum_GLenum_GLenum_GLenum *)packed;
            ARGS_void_GLenum_GLenum_GLenum_GLenum args = unpacked->args;
            glBlendFuncSeparate(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glBufferData
        case glBufferData_INDEX: {
            INDEXED_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum *unpacked = (INDEXED_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum *)packed;
            ARGS_void_GLenum_GLsizeiptr_const_GLvoid___GENPT___GLenum args = unpacked->args;
            glBufferData(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glBufferSubData
        case glBufferSubData_INDEX: {
            INDEXED_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLintptr_GLsizeiptr_const_GLvoid___GENPT__ args = unpacked->args;
            glBufferSubData(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glCallList
        case glCallList_INDEX: {
            INDEXED_void_GLuint *unpacked = (INDEXED_void_GLuint *)packed;
            ARGS_void_GLuint args = unpacked->args;
            glCallList(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glCallLists
        case glCallLists_INDEX: {
            INDEXED_void_GLsizei_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLsizei_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLsizei_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glCallLists(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glClear
        case glClear_INDEX: {
            INDEXED_void_GLbitfield *unpacked = (INDEXED_void_GLbitfield *)packed;
            ARGS_void_GLbitfield args = unpacked->args;
            glClear(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glClearAccum
        case glClearAccum_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glClearAccum(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glClearColor
        case glClearColor_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glClearColor(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glClearDepth
        case glClearDepth_INDEX: {
            INDEXED_void_GLdouble *unpacked = (INDEXED_void_GLdouble *)packed;
            ARGS_void_GLdouble args = unpacked->args;
            glClearDepth(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glClearIndex
        case glClearIndex_INDEX: {
            INDEXED_void_GLfloat *unpacked = (INDEXED_void_GLfloat *)packed;
            ARGS_void_GLfloat args = unpacked->args;
            glClearIndex(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glClearStencil
        case glClearStencil_INDEX: {
            INDEXED_void_GLint *unpacked = (INDEXED_void_GLint *)packed;
            ARGS_void_GLint args = unpacked->args;
            glClearStencil(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glClientActiveTexture
        case glClientActiveTexture_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glClientActiveTexture(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glClipPlane
        case glClipPlane_INDEX: {
            INDEXED_void_GLenum_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLdouble___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLdouble___GENPT__ args = unpacked->args;
            glClipPlane(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glColor3b
        case glColor3b_INDEX: {
            INDEXED_void_GLbyte_GLbyte_GLbyte *unpacked = (INDEXED_void_GLbyte_GLbyte_GLbyte *)packed;
            ARGS_void_GLbyte_GLbyte_GLbyte args = unpacked->args;
            glColor3b(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glColor3bv
        case glColor3bv_INDEX: {
            INDEXED_void_const_GLbyte___GENPT__ *unpacked = (INDEXED_void_const_GLbyte___GENPT__ *)packed;
            ARGS_void_const_GLbyte___GENPT__ args = unpacked->args;
            glColor3bv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor3d
        case glColor3d_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glColor3d(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glColor3dv
        case glColor3dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glColor3dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor3f
        case glColor3f_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glColor3f(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glColor3fv
        case glColor3fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glColor3fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor3i
        case glColor3i_INDEX: {
            INDEXED_void_GLint_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint args = unpacked->args;
            glColor3i(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glColor3iv
        case glColor3iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glColor3iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor3s
        case glColor3s_INDEX: {
            INDEXED_void_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort_GLshort args = unpacked->args;
            glColor3s(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glColor3sv
        case glColor3sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glColor3sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor3ub
        case glColor3ub_INDEX: {
            INDEXED_void_GLubyte_GLubyte_GLubyte *unpacked = (INDEXED_void_GLubyte_GLubyte_GLubyte *)packed;
            ARGS_void_GLubyte_GLubyte_GLubyte args = unpacked->args;
            glColor3ub(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glColor3ubv
        case glColor3ubv_INDEX: {
            INDEXED_void_const_GLubyte___GENPT__ *unpacked = (INDEXED_void_const_GLubyte___GENPT__ *)packed;
            ARGS_void_const_GLubyte___GENPT__ args = unpacked->args;
            glColor3ubv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor3ui
        case glColor3ui_INDEX: {
            INDEXED_void_GLuint_GLuint_GLuint *unpacked = (INDEXED_void_GLuint_GLuint_GLuint *)packed;
            ARGS_void_GLuint_GLuint_GLuint args = unpacked->args;
            glColor3ui(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glColor3uiv
        case glColor3uiv_INDEX: {
            INDEXED_void_const_GLuint___GENPT__ *unpacked = (INDEXED_void_const_GLuint___GENPT__ *)packed;
            ARGS_void_const_GLuint___GENPT__ args = unpacked->args;
            glColor3uiv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor3us
        case glColor3us_INDEX: {
            INDEXED_void_GLushort_GLushort_GLushort *unpacked = (INDEXED_void_GLushort_GLushort_GLushort *)packed;
            ARGS_void_GLushort_GLushort_GLushort args = unpacked->args;
            glColor3us(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glColor3usv
        case glColor3usv_INDEX: {
            INDEXED_void_const_GLushort___GENPT__ *unpacked = (INDEXED_void_const_GLushort___GENPT__ *)packed;
            ARGS_void_const_GLushort___GENPT__ args = unpacked->args;
            glColor3usv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor4b
        case glColor4b_INDEX: {
            INDEXED_void_GLbyte_GLbyte_GLbyte_GLbyte *unpacked = (INDEXED_void_GLbyte_GLbyte_GLbyte_GLbyte *)packed;
            ARGS_void_GLbyte_GLbyte_GLbyte_GLbyte args = unpacked->args;
            glColor4b(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glColor4bv
        case glColor4bv_INDEX: {
            INDEXED_void_const_GLbyte___GENPT__ *unpacked = (INDEXED_void_const_GLbyte___GENPT__ *)packed;
            ARGS_void_const_GLbyte___GENPT__ args = unpacked->args;
            glColor4bv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor4d
        case glColor4d_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glColor4d(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glColor4dv
        case glColor4dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glColor4dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor4f
        case glColor4f_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glColor4f(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glColor4fv
        case glColor4fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glColor4fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor4i
        case glColor4i_INDEX: {
            INDEXED_void_GLint_GLint_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint_GLint args = unpacked->args;
            glColor4i(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glColor4iv
        case glColor4iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glColor4iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor4s
        case glColor4s_INDEX: {
            INDEXED_void_GLshort_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort_GLshort_GLshort args = unpacked->args;
            glColor4s(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glColor4sv
        case glColor4sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glColor4sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor4ub
        case glColor4ub_INDEX: {
            INDEXED_void_GLubyte_GLubyte_GLubyte_GLubyte *unpacked = (INDEXED_void_GLubyte_GLubyte_GLubyte_GLubyte *)packed;
            ARGS_void_GLubyte_GLubyte_GLubyte_GLubyte args = unpacked->args;
            glColor4ub(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glColor4ubv
        case glColor4ubv_INDEX: {
            INDEXED_void_const_GLubyte___GENPT__ *unpacked = (INDEXED_void_const_GLubyte___GENPT__ *)packed;
            ARGS_void_const_GLubyte___GENPT__ args = unpacked->args;
            glColor4ubv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor4ui
        case glColor4ui_INDEX: {
            INDEXED_void_GLuint_GLuint_GLuint_GLuint *unpacked = (INDEXED_void_GLuint_GLuint_GLuint_GLuint *)packed;
            ARGS_void_GLuint_GLuint_GLuint_GLuint args = unpacked->args;
            glColor4ui(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glColor4uiv
        case glColor4uiv_INDEX: {
            INDEXED_void_const_GLuint___GENPT__ *unpacked = (INDEXED_void_const_GLuint___GENPT__ *)packed;
            ARGS_void_const_GLuint___GENPT__ args = unpacked->args;
            glColor4uiv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColor4us
        case glColor4us_INDEX: {
            INDEXED_void_GLushort_GLushort_GLushort_GLushort *unpacked = (INDEXED_void_GLushort_GLushort_GLushort_GLushort *)packed;
            ARGS_void_GLushort_GLushort_GLushort_GLushort args = unpacked->args;
            glColor4us(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glColor4usv
        case glColor4usv_INDEX: {
            INDEXED_void_const_GLushort___GENPT__ *unpacked = (INDEXED_void_const_GLushort___GENPT__ *)packed;
            ARGS_void_const_GLushort___GENPT__ args = unpacked->args;
            glColor4usv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glColorMask
        case glColorMask_INDEX: {
            INDEXED_void_GLboolean_GLboolean_GLboolean_GLboolean *unpacked = (INDEXED_void_GLboolean_GLboolean_GLboolean_GLboolean *)packed;
            ARGS_void_GLboolean_GLboolean_GLboolean_GLboolean args = unpacked->args;
            glColorMask(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glColorMaterial
        case glColorMaterial_INDEX: {
            INDEXED_void_GLenum_GLenum *unpacked = (INDEXED_void_GLenum_GLenum *)packed;
            ARGS_void_GLenum_GLenum args = unpacked->args;
            glColorMaterial(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glColorPointer
        case glColorPointer_INDEX: {
            INDEXED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glColorPointer(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glColorSubTable
        case glColorSubTable_INDEX: {
            INDEXED_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glColorSubTable(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glColorTable
        case glColorTable_INDEX: {
            INDEXED_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glColorTable(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glColorTableParameterfv
        case glColorTableParameterfv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glColorTableParameterfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glColorTableParameteriv
        case glColorTableParameteriv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glColorTableParameteriv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glCompressedTexImage1D
        case glCompressedTexImage1D_INDEX: {
            INDEXED_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLenum_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glCompressedTexImage1D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7);
            break;
        }
        #endif
        #ifndef skip_index_glCompressedTexImage2D
        case glCompressedTexImage2D_INDEX: {
            INDEXED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glCompressedTexImage2D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8);
            break;
        }
        #endif
        #ifndef skip_index_glCompressedTexImage3D
        case glCompressedTexImage3D_INDEX: {
            INDEXED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLenum_GLsizei_GLsizei_GLsizei_GLint_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glCompressedTexImage3D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9);
            break;
        }
        #endif
        #ifndef skip_index_glCompressedTexSubImage1D
        case glCompressedTexSubImage1D_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLint_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glCompressedTexSubImage1D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7);
            break;
        }
        #endif
        #ifndef skip_index_glCompressedTexSubImage2D
        case glCompressedTexSubImage2D_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glCompressedTexSubImage2D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9);
            break;
        }
        #endif
        #ifndef skip_index_glCompressedTexSubImage3D
        case glCompressedTexSubImage3D_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glCompressedTexSubImage3D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9, args.a10, args.a11);
            break;
        }
        #endif
        #ifndef skip_index_glConvolutionFilter1D
        case glConvolutionFilter1D_INDEX: {
            INDEXED_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glConvolutionFilter1D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glConvolutionFilter2D
        case glConvolutionFilter2D_INDEX: {
            INDEXED_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glConvolutionFilter2D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7);
            break;
        }
        #endif
        #ifndef skip_index_glConvolutionParameterf
        case glConvolutionParameterf_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLenum_GLfloat args = unpacked->args;
            glConvolutionParameterf(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glConvolutionParameterfv
        case glConvolutionParameterfv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glConvolutionParameterfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glConvolutionParameteri
        case glConvolutionParameteri_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint *unpacked = (INDEXED_void_GLenum_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLenum_GLint args = unpacked->args;
            glConvolutionParameteri(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glConvolutionParameteriv
        case glConvolutionParameteriv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glConvolutionParameteriv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glCopyColorSubTable
        case glCopyColorSubTable_INDEX: {
            INDEXED_void_GLenum_GLsizei_GLint_GLint_GLsizei *unpacked = (INDEXED_void_GLenum_GLsizei_GLint_GLint_GLsizei *)packed;
            ARGS_void_GLenum_GLsizei_GLint_GLint_GLsizei args = unpacked->args;
            glCopyColorSubTable(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glCopyColorTable
        case glCopyColorTable_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint_GLint_GLsizei *unpacked = (INDEXED_void_GLenum_GLenum_GLint_GLint_GLsizei *)packed;
            ARGS_void_GLenum_GLenum_GLint_GLint_GLsizei args = unpacked->args;
            glCopyColorTable(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glCopyConvolutionFilter1D
        case glCopyConvolutionFilter1D_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint_GLint_GLsizei *unpacked = (INDEXED_void_GLenum_GLenum_GLint_GLint_GLsizei *)packed;
            ARGS_void_GLenum_GLenum_GLint_GLint_GLsizei args = unpacked->args;
            glCopyConvolutionFilter1D(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glCopyConvolutionFilter2D
        case glCopyConvolutionFilter2D_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei *unpacked = (INDEXED_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei *)packed;
            ARGS_void_GLenum_GLenum_GLint_GLint_GLsizei_GLsizei args = unpacked->args;
            glCopyConvolutionFilter2D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glCopyPixels
        case glCopyPixels_INDEX: {
            INDEXED_void_GLint_GLint_GLsizei_GLsizei_GLenum *unpacked = (INDEXED_void_GLint_GLint_GLsizei_GLsizei_GLenum *)packed;
            ARGS_void_GLint_GLint_GLsizei_GLsizei_GLenum args = unpacked->args;
            glCopyPixels(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glCopyTexImage1D
        case glCopyTexImage1D_INDEX: {
            INDEXED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint *unpacked = (INDEXED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint *)packed;
            ARGS_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLint args = unpacked->args;
            glCopyTexImage1D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7);
            break;
        }
        #endif
        #ifndef skip_index_glCopyTexImage2D
        case glCopyTexImage2D_INDEX: {
            INDEXED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint *unpacked = (INDEXED_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint *)packed;
            ARGS_void_GLenum_GLint_GLenum_GLint_GLint_GLsizei_GLsizei_GLint args = unpacked->args;
            glCopyTexImage2D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8);
            break;
        }
        #endif
        #ifndef skip_index_glCopyTexSubImage1D
        case glCopyTexSubImage1D_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei *)packed;
            ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLsizei args = unpacked->args;
            glCopyTexSubImage1D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glCopyTexSubImage2D
        case glCopyTexSubImage2D_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei *)packed;
            ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei args = unpacked->args;
            glCopyTexSubImage2D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8);
            break;
        }
        #endif
        #ifndef skip_index_glCopyTexSubImage3D
        case glCopyTexSubImage3D_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei *)packed;
            ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLint_GLint_GLsizei_GLsizei args = unpacked->args;
            glCopyTexSubImage3D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9);
            break;
        }
        #endif
        #ifndef skip_index_glCullFace
        case glCullFace_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glCullFace(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glDeleteBuffers
        case glDeleteBuffers_INDEX: {
            INDEXED_void_GLsizei_const_GLuint___GENPT__ *unpacked = (INDEXED_void_GLsizei_const_GLuint___GENPT__ *)packed;
            ARGS_void_GLsizei_const_GLuint___GENPT__ args = unpacked->args;
            glDeleteBuffers(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glDeleteLists
        case glDeleteLists_INDEX: {
            INDEXED_void_GLuint_GLsizei *unpacked = (INDEXED_void_GLuint_GLsizei *)packed;
            ARGS_void_GLuint_GLsizei args = unpacked->args;
            glDeleteLists(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glDeleteQueries
        case glDeleteQueries_INDEX: {
            INDEXED_void_GLsizei_const_GLuint___GENPT__ *unpacked = (INDEXED_void_GLsizei_const_GLuint___GENPT__ *)packed;
            ARGS_void_GLsizei_const_GLuint___GENPT__ args = unpacked->args;
            glDeleteQueries(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glDeleteTextures
        case glDeleteTextures_INDEX: {
            INDEXED_void_GLsizei_const_GLuint___GENPT__ *unpacked = (INDEXED_void_GLsizei_const_GLuint___GENPT__ *)packed;
            ARGS_void_GLsizei_const_GLuint___GENPT__ args = unpacked->args;
            glDeleteTextures(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glDepthFunc
        case glDepthFunc_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glDepthFunc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glDepthMask
        case glDepthMask_INDEX: {
            INDEXED_void_GLboolean *unpacked = (INDEXED_void_GLboolean *)packed;
            ARGS_void_GLboolean args = unpacked->args;
            glDepthMask(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glDepthRange
        case glDepthRange_INDEX: {
            INDEXED_void_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble args = unpacked->args;
            glDepthRange(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glDisable
        case glDisable_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glDisable(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glDisableClientState
        case glDisableClientState_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glDisableClientState(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glDrawArrays
        case glDrawArrays_INDEX: {
            INDEXED_void_GLenum_GLint_GLsizei *unpacked = (INDEXED_void_GLenum_GLint_GLsizei *)packed;
            ARGS_void_GLenum_GLint_GLsizei args = unpacked->args;
            glDrawArrays(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glDrawBuffer
        case glDrawBuffer_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glDrawBuffer(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glDrawElements
        case glDrawElements_INDEX: {
            INDEXED_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLsizei_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glDrawElements(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glDrawPixels
        case glDrawPixels_INDEX: {
            INDEXED_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glDrawPixels(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glDrawRangeElements
        case glDrawRangeElements_INDEX: {
            INDEXED_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLuint_GLuint_GLsizei_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glDrawRangeElements(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glEdgeFlag
        case glEdgeFlag_INDEX: {
            INDEXED_void_GLboolean *unpacked = (INDEXED_void_GLboolean *)packed;
            ARGS_void_GLboolean args = unpacked->args;
            glEdgeFlag(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glEdgeFlagPointer
        case glEdgeFlagPointer_INDEX: {
            INDEXED_void_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glEdgeFlagPointer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glEdgeFlagv
        case glEdgeFlagv_INDEX: {
            INDEXED_void_const_GLboolean___GENPT__ *unpacked = (INDEXED_void_const_GLboolean___GENPT__ *)packed;
            ARGS_void_const_GLboolean___GENPT__ args = unpacked->args;
            glEdgeFlagv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glEnable
        case glEnable_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glEnable(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glEnableClientState
        case glEnableClientState_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glEnableClientState(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glEnd
        case glEnd_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glEnd();
            break;
        }
        #endif
        #ifndef skip_index_glEndList
        case glEndList_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glEndList();
            break;
        }
        #endif
        #ifndef skip_index_glEndQuery
        case glEndQuery_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glEndQuery(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glEvalCoord1d
        case glEvalCoord1d_INDEX: {
            INDEXED_void_GLdouble *unpacked = (INDEXED_void_GLdouble *)packed;
            ARGS_void_GLdouble args = unpacked->args;
            glEvalCoord1d(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glEvalCoord1dv
        case glEvalCoord1dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glEvalCoord1dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glEvalCoord1f
        case glEvalCoord1f_INDEX: {
            INDEXED_void_GLfloat *unpacked = (INDEXED_void_GLfloat *)packed;
            ARGS_void_GLfloat args = unpacked->args;
            glEvalCoord1f(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glEvalCoord1fv
        case glEvalCoord1fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glEvalCoord1fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glEvalCoord2d
        case glEvalCoord2d_INDEX: {
            INDEXED_void_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble args = unpacked->args;
            glEvalCoord2d(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glEvalCoord2dv
        case glEvalCoord2dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glEvalCoord2dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glEvalCoord2f
        case glEvalCoord2f_INDEX: {
            INDEXED_void_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat args = unpacked->args;
            glEvalCoord2f(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glEvalCoord2fv
        case glEvalCoord2fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glEvalCoord2fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glEvalMesh1
        case glEvalMesh1_INDEX: {
            INDEXED_void_GLenum_GLint_GLint *unpacked = (INDEXED_void_GLenum_GLint_GLint *)packed;
            ARGS_void_GLenum_GLint_GLint args = unpacked->args;
            glEvalMesh1(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glEvalMesh2
        case glEvalMesh2_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLint_GLint *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLint_GLint *)packed;
            ARGS_void_GLenum_GLint_GLint_GLint_GLint args = unpacked->args;
            glEvalMesh2(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glEvalPoint1
        case glEvalPoint1_INDEX: {
            INDEXED_void_GLint *unpacked = (INDEXED_void_GLint *)packed;
            ARGS_void_GLint args = unpacked->args;
            glEvalPoint1(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glEvalPoint2
        case glEvalPoint2_INDEX: {
            INDEXED_void_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint *)packed;
            ARGS_void_GLint_GLint args = unpacked->args;
            glEvalPoint2(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glFeedbackBuffer
        case glFeedbackBuffer_INDEX: {
            INDEXED_void_GLsizei_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLsizei_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLsizei_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glFeedbackBuffer(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glFinish
        case glFinish_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glFinish();
            break;
        }
        #endif
        #ifndef skip_index_glFlush
        case glFlush_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glFlush();
            break;
        }
        #endif
        #ifndef skip_index_glFogCoordPointer
        case glFogCoordPointer_INDEX: {
            INDEXED_void_GLenum_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glFogCoordPointer(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glFogCoordd
        case glFogCoordd_INDEX: {
            INDEXED_void_GLdouble *unpacked = (INDEXED_void_GLdouble *)packed;
            ARGS_void_GLdouble args = unpacked->args;
            glFogCoordd(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glFogCoorddv
        case glFogCoorddv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glFogCoorddv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glFogCoordf
        case glFogCoordf_INDEX: {
            INDEXED_void_GLfloat *unpacked = (INDEXED_void_GLfloat *)packed;
            ARGS_void_GLfloat args = unpacked->args;
            glFogCoordf(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glFogCoordfv
        case glFogCoordfv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glFogCoordfv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glFogf
        case glFogf_INDEX: {
            INDEXED_void_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLfloat args = unpacked->args;
            glFogf(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glFogfv
        case glFogfv_INDEX: {
            INDEXED_void_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glFogfv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glFogi
        case glFogi_INDEX: {
            INDEXED_void_GLenum_GLint *unpacked = (INDEXED_void_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLint args = unpacked->args;
            glFogi(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glFogiv
        case glFogiv_INDEX: {
            INDEXED_void_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glFogiv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glFrontFace
        case glFrontFace_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glFrontFace(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glFrustum
        case glFrustum_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glFrustum(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glGenBuffers
        case glGenBuffers_INDEX: {
            INDEXED_void_GLsizei_GLuint___GENPT__ *unpacked = (INDEXED_void_GLsizei_GLuint___GENPT__ *)packed;
            ARGS_void_GLsizei_GLuint___GENPT__ args = unpacked->args;
            glGenBuffers(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glGenLists
        case glGenLists_INDEX: {
            INDEXED_GLuint_GLsizei *unpacked = (INDEXED_GLuint_GLsizei *)packed;
            ARGS_GLuint_GLsizei args = unpacked->args;
            GLuint *ret = (GLuint *)ret_v;
            *ret =
            glGenLists(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glGenQueries
        case glGenQueries_INDEX: {
            INDEXED_void_GLsizei_GLuint___GENPT__ *unpacked = (INDEXED_void_GLsizei_GLuint___GENPT__ *)packed;
            ARGS_void_GLsizei_GLuint___GENPT__ args = unpacked->args;
            glGenQueries(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glGenTextures
        case glGenTextures_INDEX: {
            INDEXED_void_GLsizei_GLuint___GENPT__ *unpacked = (INDEXED_void_GLsizei_GLuint___GENPT__ *)packed;
            ARGS_void_GLsizei_GLuint___GENPT__ args = unpacked->args;
            glGenTextures(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glGetBooleanv
        case glGetBooleanv_INDEX: {
            INDEXED_void_GLenum_GLboolean___GENPT__ *unpacked = (INDEXED_void_GLenum_GLboolean___GENPT__ *)packed;
            ARGS_void_GLenum_GLboolean___GENPT__ args = unpacked->args;
            glGetBooleanv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glGetBufferParameteriv
        case glGetBufferParameteriv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetBufferParameteriv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetBufferPointerv
        case glGetBufferPointerv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLvoid___GENPT__ args = unpacked->args;
            glGetBufferPointerv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetBufferSubData
        case glGetBufferSubData_INDEX: {
            INDEXED_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLintptr_GLsizeiptr_GLvoid___GENPT__ args = unpacked->args;
            glGetBufferSubData(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glGetClipPlane
        case glGetClipPlane_INDEX: {
            INDEXED_void_GLenum_GLdouble___GENPT__ *unpacked = (INDEXED_void_GLenum_GLdouble___GENPT__ *)packed;
            ARGS_void_GLenum_GLdouble___GENPT__ args = unpacked->args;
            glGetClipPlane(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glGetColorTable
        case glGetColorTable_INDEX: {
            INDEXED_void_GLenum_GLenum_GLenum_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLenum_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLenum_GLvoid___GENPT__ args = unpacked->args;
            glGetColorTable(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glGetColorTableParameterfv
        case glGetColorTableParameterfv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glGetColorTableParameterfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetColorTableParameteriv
        case glGetColorTableParameteriv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetColorTableParameteriv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetCompressedTexImage
        case glGetCompressedTexImage_INDEX: {
            INDEXED_void_GLenum_GLint_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLvoid___GENPT__ args = unpacked->args;
            glGetCompressedTexImage(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetConvolutionFilter
        case glGetConvolutionFilter_INDEX: {
            INDEXED_void_GLenum_GLenum_GLenum_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLenum_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLenum_GLvoid___GENPT__ args = unpacked->args;
            glGetConvolutionFilter(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glGetConvolutionParameterfv
        case glGetConvolutionParameterfv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glGetConvolutionParameterfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetConvolutionParameteriv
        case glGetConvolutionParameteriv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetConvolutionParameteriv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetDoublev
        case glGetDoublev_INDEX: {
            INDEXED_void_GLenum_GLdouble___GENPT__ *unpacked = (INDEXED_void_GLenum_GLdouble___GENPT__ *)packed;
            ARGS_void_GLenum_GLdouble___GENPT__ args = unpacked->args;
            glGetDoublev(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glGetError
        case glGetError_INDEX: {
            INDEXED_GLenum *unpacked = (INDEXED_GLenum *)packed;
            GLenum *ret = (GLenum *)ret_v;
            *ret =
            glGetError();
            break;
        }
        #endif
        #ifndef skip_index_glGetFloatv
        case glGetFloatv_INDEX: {
            INDEXED_void_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glGetFloatv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glGetHistogram
        case glGetHistogram_INDEX: {
            INDEXED_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__ args = unpacked->args;
            glGetHistogram(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glGetHistogramParameterfv
        case glGetHistogramParameterfv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glGetHistogramParameterfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetHistogramParameteriv
        case glGetHistogramParameteriv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetHistogramParameteriv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetIntegerv
        case glGetIntegerv_INDEX: {
            INDEXED_void_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetIntegerv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glGetLightfv
        case glGetLightfv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glGetLightfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetLightiv
        case glGetLightiv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetLightiv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetMapdv
        case glGetMapdv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLdouble___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLdouble___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLdouble___GENPT__ args = unpacked->args;
            glGetMapdv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetMapfv
        case glGetMapfv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glGetMapfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetMapiv
        case glGetMapiv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetMapiv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetMaterialfv
        case glGetMaterialfv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glGetMaterialfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetMaterialiv
        case glGetMaterialiv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetMaterialiv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetMinmax
        case glGetMinmax_INDEX: {
            INDEXED_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLboolean_GLenum_GLenum_GLvoid___GENPT__ args = unpacked->args;
            glGetMinmax(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glGetMinmaxParameterfv
        case glGetMinmaxParameterfv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glGetMinmaxParameterfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetMinmaxParameteriv
        case glGetMinmaxParameteriv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetMinmaxParameteriv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetPixelMapfv
        case glGetPixelMapfv_INDEX: {
            INDEXED_void_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glGetPixelMapfv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glGetPixelMapuiv
        case glGetPixelMapuiv_INDEX: {
            INDEXED_void_GLenum_GLuint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLuint___GENPT__ *)packed;
            ARGS_void_GLenum_GLuint___GENPT__ args = unpacked->args;
            glGetPixelMapuiv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glGetPixelMapusv
        case glGetPixelMapusv_INDEX: {
            INDEXED_void_GLenum_GLushort___GENPT__ *unpacked = (INDEXED_void_GLenum_GLushort___GENPT__ *)packed;
            ARGS_void_GLenum_GLushort___GENPT__ args = unpacked->args;
            glGetPixelMapusv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glGetPointerv
        case glGetPointerv_INDEX: {
            INDEXED_void_GLenum_GLvoid___GENPT____GENPT__ *unpacked = (INDEXED_void_GLenum_GLvoid___GENPT____GENPT__ *)packed;
            ARGS_void_GLenum_GLvoid___GENPT____GENPT__ args = unpacked->args;
            glGetPointerv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glGetPolygonStipple
        case glGetPolygonStipple_INDEX: {
            INDEXED_void_GLubyte___GENPT__ *unpacked = (INDEXED_void_GLubyte___GENPT__ *)packed;
            ARGS_void_GLubyte___GENPT__ args = unpacked->args;
            glGetPolygonStipple(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glGetQueryObjectiv
        case glGetQueryObjectiv_INDEX: {
            INDEXED_void_GLuint_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLuint_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLuint_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetQueryObjectiv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetQueryObjectuiv
        case glGetQueryObjectuiv_INDEX: {
            INDEXED_void_GLuint_GLenum_GLuint___GENPT__ *unpacked = (INDEXED_void_GLuint_GLenum_GLuint___GENPT__ *)packed;
            ARGS_void_GLuint_GLenum_GLuint___GENPT__ args = unpacked->args;
            glGetQueryObjectuiv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetQueryiv
        case glGetQueryiv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetQueryiv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetSeparableFilter
        case glGetSeparableFilter_INDEX: {
            INDEXED_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLenum_GLvoid___GENPT___GLvoid___GENPT___GLvoid___GENPT__ args = unpacked->args;
            glGetSeparableFilter(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glGetString
        case glGetString_INDEX: {
            INDEXED_const_GLubyte___GENPT___GLenum *unpacked = (INDEXED_const_GLubyte___GENPT___GLenum *)packed;
            ARGS_const_GLubyte___GENPT___GLenum args = unpacked->args;
            const GLubyte * *ret = (const GLubyte * *)ret_v;
            *ret =
            glGetString(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glGetTexEnvfv
        case glGetTexEnvfv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glGetTexEnvfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetTexEnviv
        case glGetTexEnviv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetTexEnviv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetTexGendv
        case glGetTexGendv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLdouble___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLdouble___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLdouble___GENPT__ args = unpacked->args;
            glGetTexGendv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetTexGenfv
        case glGetTexGenfv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glGetTexGenfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetTexGeniv
        case glGetTexGeniv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetTexGeniv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetTexImage
        case glGetTexImage_INDEX: {
            INDEXED_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLenum_GLenum_GLvoid___GENPT__ args = unpacked->args;
            glGetTexImage(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glGetTexLevelParameterfv
        case glGetTexLevelParameterfv_INDEX: {
            INDEXED_void_GLenum_GLint_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glGetTexLevelParameterfv(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glGetTexLevelParameteriv
        case glGetTexLevelParameteriv_INDEX: {
            INDEXED_void_GLenum_GLint_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetTexLevelParameteriv(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glGetTexParameterfv
        case glGetTexParameterfv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLfloat___GENPT__ args = unpacked->args;
            glGetTexParameterfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glGetTexParameteriv
        case glGetTexParameteriv_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLint___GENPT__ args = unpacked->args;
            glGetTexParameteriv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glHint
        case glHint_INDEX: {
            INDEXED_void_GLenum_GLenum *unpacked = (INDEXED_void_GLenum_GLenum *)packed;
            ARGS_void_GLenum_GLenum args = unpacked->args;
            glHint(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glHistogram
        case glHistogram_INDEX: {
            INDEXED_void_GLenum_GLsizei_GLenum_GLboolean *unpacked = (INDEXED_void_GLenum_GLsizei_GLenum_GLboolean *)packed;
            ARGS_void_GLenum_GLsizei_GLenum_GLboolean args = unpacked->args;
            glHistogram(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glIndexMask
        case glIndexMask_INDEX: {
            INDEXED_void_GLuint *unpacked = (INDEXED_void_GLuint *)packed;
            ARGS_void_GLuint args = unpacked->args;
            glIndexMask(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIndexPointer
        case glIndexPointer_INDEX: {
            INDEXED_void_GLenum_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glIndexPointer(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glIndexd
        case glIndexd_INDEX: {
            INDEXED_void_GLdouble *unpacked = (INDEXED_void_GLdouble *)packed;
            ARGS_void_GLdouble args = unpacked->args;
            glIndexd(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIndexdv
        case glIndexdv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glIndexdv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIndexf
        case glIndexf_INDEX: {
            INDEXED_void_GLfloat *unpacked = (INDEXED_void_GLfloat *)packed;
            ARGS_void_GLfloat args = unpacked->args;
            glIndexf(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIndexfv
        case glIndexfv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glIndexfv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIndexi
        case glIndexi_INDEX: {
            INDEXED_void_GLint *unpacked = (INDEXED_void_GLint *)packed;
            ARGS_void_GLint args = unpacked->args;
            glIndexi(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIndexiv
        case glIndexiv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glIndexiv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIndexs
        case glIndexs_INDEX: {
            INDEXED_void_GLshort *unpacked = (INDEXED_void_GLshort *)packed;
            ARGS_void_GLshort args = unpacked->args;
            glIndexs(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIndexsv
        case glIndexsv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glIndexsv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIndexub
        case glIndexub_INDEX: {
            INDEXED_void_GLubyte *unpacked = (INDEXED_void_GLubyte *)packed;
            ARGS_void_GLubyte args = unpacked->args;
            glIndexub(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIndexubv
        case glIndexubv_INDEX: {
            INDEXED_void_const_GLubyte___GENPT__ *unpacked = (INDEXED_void_const_GLubyte___GENPT__ *)packed;
            ARGS_void_const_GLubyte___GENPT__ args = unpacked->args;
            glIndexubv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glInitNames
        case glInitNames_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glInitNames();
            break;
        }
        #endif
        #ifndef skip_index_glInterleavedArrays
        case glInterleavedArrays_INDEX: {
            INDEXED_void_GLenum_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glInterleavedArrays(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glIsBuffer
        case glIsBuffer_INDEX: {
            INDEXED_GLboolean_GLuint *unpacked = (INDEXED_GLboolean_GLuint *)packed;
            ARGS_GLboolean_GLuint args = unpacked->args;
            GLboolean *ret = (GLboolean *)ret_v;
            *ret =
            glIsBuffer(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIsEnabled
        case glIsEnabled_INDEX: {
            INDEXED_GLboolean_GLenum *unpacked = (INDEXED_GLboolean_GLenum *)packed;
            ARGS_GLboolean_GLenum args = unpacked->args;
            GLboolean *ret = (GLboolean *)ret_v;
            *ret =
            glIsEnabled(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIsList
        case glIsList_INDEX: {
            INDEXED_GLboolean_GLuint *unpacked = (INDEXED_GLboolean_GLuint *)packed;
            ARGS_GLboolean_GLuint args = unpacked->args;
            GLboolean *ret = (GLboolean *)ret_v;
            *ret =
            glIsList(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIsQuery
        case glIsQuery_INDEX: {
            INDEXED_GLboolean_GLuint *unpacked = (INDEXED_GLboolean_GLuint *)packed;
            ARGS_GLboolean_GLuint args = unpacked->args;
            GLboolean *ret = (GLboolean *)ret_v;
            *ret =
            glIsQuery(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glIsTexture
        case glIsTexture_INDEX: {
            INDEXED_GLboolean_GLuint *unpacked = (INDEXED_GLboolean_GLuint *)packed;
            ARGS_GLboolean_GLuint args = unpacked->args;
            GLboolean *ret = (GLboolean *)ret_v;
            *ret =
            glIsTexture(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glLightModelf
        case glLightModelf_INDEX: {
            INDEXED_void_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLfloat args = unpacked->args;
            glLightModelf(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glLightModelfv
        case glLightModelfv_INDEX: {
            INDEXED_void_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glLightModelfv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glLightModeli
        case glLightModeli_INDEX: {
            INDEXED_void_GLenum_GLint *unpacked = (INDEXED_void_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLint args = unpacked->args;
            glLightModeli(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glLightModeliv
        case glLightModeliv_INDEX: {
            INDEXED_void_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glLightModeliv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glLightf
        case glLightf_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLenum_GLfloat args = unpacked->args;
            glLightf(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glLightfv
        case glLightfv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glLightfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glLighti
        case glLighti_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint *unpacked = (INDEXED_void_GLenum_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLenum_GLint args = unpacked->args;
            glLighti(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glLightiv
        case glLightiv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glLightiv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glLineStipple
        case glLineStipple_INDEX: {
            INDEXED_void_GLint_GLushort *unpacked = (INDEXED_void_GLint_GLushort *)packed;
            ARGS_void_GLint_GLushort args = unpacked->args;
            glLineStipple(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glLineWidth
        case glLineWidth_INDEX: {
            INDEXED_void_GLfloat *unpacked = (INDEXED_void_GLfloat *)packed;
            ARGS_void_GLfloat args = unpacked->args;
            glLineWidth(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glListBase
        case glListBase_INDEX: {
            INDEXED_void_GLuint *unpacked = (INDEXED_void_GLuint *)packed;
            ARGS_void_GLuint args = unpacked->args;
            glListBase(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glLoadIdentity
        case glLoadIdentity_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glLoadIdentity();
            break;
        }
        #endif
        #ifndef skip_index_glLoadMatrixd
        case glLoadMatrixd_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glLoadMatrixd(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glLoadMatrixf
        case glLoadMatrixf_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glLoadMatrixf(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glLoadName
        case glLoadName_INDEX: {
            INDEXED_void_GLuint *unpacked = (INDEXED_void_GLuint *)packed;
            ARGS_void_GLuint args = unpacked->args;
            glLoadName(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glLoadTransposeMatrixd
        case glLoadTransposeMatrixd_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glLoadTransposeMatrixd(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glLoadTransposeMatrixf
        case glLoadTransposeMatrixf_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glLoadTransposeMatrixf(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glLogicOp
        case glLogicOp_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glLogicOp(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glMap1d
        case glMap1d_INDEX: {
            INDEXED_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__ *)packed;
            ARGS_void_GLenum_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__ args = unpacked->args;
            glMap1d(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glMap1f
        case glMap1f_INDEX: {
            INDEXED_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__ args = unpacked->args;
            glMap1f(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glMap2d
        case glMap2d_INDEX: {
            INDEXED_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__ *)packed;
            ARGS_void_GLenum_GLdouble_GLdouble_GLint_GLint_GLdouble_GLdouble_GLint_GLint_const_GLdouble___GENPT__ args = unpacked->args;
            glMap2d(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9, args.a10);
            break;
        }
        #endif
        #ifndef skip_index_glMap2f
        case glMap2f_INDEX: {
            INDEXED_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLfloat_GLfloat_GLint_GLint_GLfloat_GLfloat_GLint_GLint_const_GLfloat___GENPT__ args = unpacked->args;
            glMap2f(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9, args.a10);
            break;
        }
        #endif
        #ifndef skip_index_glMapBuffer
        case glMapBuffer_INDEX: {
            INDEXED_GLvoid___GENPT___GLenum_GLenum *unpacked = (INDEXED_GLvoid___GENPT___GLenum_GLenum *)packed;
            ARGS_GLvoid___GENPT___GLenum_GLenum args = unpacked->args;
            GLvoid * *ret = (GLvoid * *)ret_v;
            *ret =
            glMapBuffer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMapGrid1d
        case glMapGrid1d_INDEX: {
            INDEXED_void_GLint_GLdouble_GLdouble *unpacked = (INDEXED_void_GLint_GLdouble_GLdouble *)packed;
            ARGS_void_GLint_GLdouble_GLdouble args = unpacked->args;
            glMapGrid1d(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glMapGrid1f
        case glMapGrid1f_INDEX: {
            INDEXED_void_GLint_GLfloat_GLfloat *unpacked = (INDEXED_void_GLint_GLfloat_GLfloat *)packed;
            ARGS_void_GLint_GLfloat_GLfloat args = unpacked->args;
            glMapGrid1f(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glMapGrid2d
        case glMapGrid2d_INDEX: {
            INDEXED_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble *unpacked = (INDEXED_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble *)packed;
            ARGS_void_GLint_GLdouble_GLdouble_GLint_GLdouble_GLdouble args = unpacked->args;
            glMapGrid2d(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glMapGrid2f
        case glMapGrid2f_INDEX: {
            INDEXED_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat *unpacked = (INDEXED_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat *)packed;
            ARGS_void_GLint_GLfloat_GLfloat_GLint_GLfloat_GLfloat args = unpacked->args;
            glMapGrid2f(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glMaterialf
        case glMaterialf_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLenum_GLfloat args = unpacked->args;
            glMaterialf(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glMaterialfv
        case glMaterialfv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glMaterialfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glMateriali
        case glMateriali_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint *unpacked = (INDEXED_void_GLenum_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLenum_GLint args = unpacked->args;
            glMateriali(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glMaterialiv
        case glMaterialiv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glMaterialiv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glMatrixMode
        case glMatrixMode_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glMatrixMode(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glMinmax
        case glMinmax_INDEX: {
            INDEXED_void_GLenum_GLenum_GLboolean *unpacked = (INDEXED_void_GLenum_GLenum_GLboolean *)packed;
            ARGS_void_GLenum_GLenum_GLboolean args = unpacked->args;
            glMinmax(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glMultMatrixd
        case glMultMatrixd_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glMultMatrixd(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glMultMatrixf
        case glMultMatrixf_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glMultMatrixf(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glMultTransposeMatrixd
        case glMultTransposeMatrixd_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glMultTransposeMatrixd(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glMultTransposeMatrixf
        case glMultTransposeMatrixf_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glMultTransposeMatrixf(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glMultiDrawArrays
        case glMultiDrawArrays_INDEX: {
            INDEXED_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei *unpacked = (INDEXED_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei *)packed;
            ARGS_void_GLenum_const_GLint___GENPT___const_GLsizei___GENPT___GLsizei args = unpacked->args;
            glMultiDrawArrays(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glMultiDrawElements
        case glMultiDrawElements_INDEX: {
            INDEXED_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei *unpacked = (INDEXED_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei *)packed;
            ARGS_void_GLenum_const_GLsizei___GENPT___GLenum_GLvoid__GENPT__const___GENPT___GLsizei args = unpacked->args;
            glMultiDrawElements(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord1d
        case glMultiTexCoord1d_INDEX: {
            INDEXED_void_GLenum_GLdouble *unpacked = (INDEXED_void_GLenum_GLdouble *)packed;
            ARGS_void_GLenum_GLdouble args = unpacked->args;
            glMultiTexCoord1d(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord1dv
        case glMultiTexCoord1dv_INDEX: {
            INDEXED_void_GLenum_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLdouble___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLdouble___GENPT__ args = unpacked->args;
            glMultiTexCoord1dv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord1f
        case glMultiTexCoord1f_INDEX: {
            INDEXED_void_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLfloat args = unpacked->args;
            glMultiTexCoord1f(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord1fv
        case glMultiTexCoord1fv_INDEX: {
            INDEXED_void_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glMultiTexCoord1fv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord1i
        case glMultiTexCoord1i_INDEX: {
            INDEXED_void_GLenum_GLint *unpacked = (INDEXED_void_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLint args = unpacked->args;
            glMultiTexCoord1i(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord1iv
        case glMultiTexCoord1iv_INDEX: {
            INDEXED_void_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glMultiTexCoord1iv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord1s
        case glMultiTexCoord1s_INDEX: {
            INDEXED_void_GLenum_GLshort *unpacked = (INDEXED_void_GLenum_GLshort *)packed;
            ARGS_void_GLenum_GLshort args = unpacked->args;
            glMultiTexCoord1s(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord1sv
        case glMultiTexCoord1sv_INDEX: {
            INDEXED_void_GLenum_const_GLshort___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLshort___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLshort___GENPT__ args = unpacked->args;
            glMultiTexCoord1sv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord2d
        case glMultiTexCoord2d_INDEX: {
            INDEXED_void_GLenum_GLdouble_GLdouble *unpacked = (INDEXED_void_GLenum_GLdouble_GLdouble *)packed;
            ARGS_void_GLenum_GLdouble_GLdouble args = unpacked->args;
            glMultiTexCoord2d(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord2dv
        case glMultiTexCoord2dv_INDEX: {
            INDEXED_void_GLenum_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLdouble___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLdouble___GENPT__ args = unpacked->args;
            glMultiTexCoord2dv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord2f
        case glMultiTexCoord2f_INDEX: {
            INDEXED_void_GLenum_GLfloat_GLfloat *unpacked = (INDEXED_void_GLenum_GLfloat_GLfloat *)packed;
            ARGS_void_GLenum_GLfloat_GLfloat args = unpacked->args;
            glMultiTexCoord2f(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord2fv
        case glMultiTexCoord2fv_INDEX: {
            INDEXED_void_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glMultiTexCoord2fv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord2i
        case glMultiTexCoord2i_INDEX: {
            INDEXED_void_GLenum_GLint_GLint *unpacked = (INDEXED_void_GLenum_GLint_GLint *)packed;
            ARGS_void_GLenum_GLint_GLint args = unpacked->args;
            glMultiTexCoord2i(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord2iv
        case glMultiTexCoord2iv_INDEX: {
            INDEXED_void_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glMultiTexCoord2iv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord2s
        case glMultiTexCoord2s_INDEX: {
            INDEXED_void_GLenum_GLshort_GLshort *unpacked = (INDEXED_void_GLenum_GLshort_GLshort *)packed;
            ARGS_void_GLenum_GLshort_GLshort args = unpacked->args;
            glMultiTexCoord2s(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord2sv
        case glMultiTexCoord2sv_INDEX: {
            INDEXED_void_GLenum_const_GLshort___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLshort___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLshort___GENPT__ args = unpacked->args;
            glMultiTexCoord2sv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord3d
        case glMultiTexCoord3d_INDEX: {
            INDEXED_void_GLenum_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLenum_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLenum_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glMultiTexCoord3d(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord3dv
        case glMultiTexCoord3dv_INDEX: {
            INDEXED_void_GLenum_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLdouble___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLdouble___GENPT__ args = unpacked->args;
            glMultiTexCoord3dv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord3f
        case glMultiTexCoord3f_INDEX: {
            INDEXED_void_GLenum_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLenum_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLenum_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glMultiTexCoord3f(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord3fv
        case glMultiTexCoord3fv_INDEX: {
            INDEXED_void_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glMultiTexCoord3fv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord3i
        case glMultiTexCoord3i_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLint *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLint *)packed;
            ARGS_void_GLenum_GLint_GLint_GLint args = unpacked->args;
            glMultiTexCoord3i(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord3iv
        case glMultiTexCoord3iv_INDEX: {
            INDEXED_void_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glMultiTexCoord3iv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord3s
        case glMultiTexCoord3s_INDEX: {
            INDEXED_void_GLenum_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLenum_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLenum_GLshort_GLshort_GLshort args = unpacked->args;
            glMultiTexCoord3s(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord3sv
        case glMultiTexCoord3sv_INDEX: {
            INDEXED_void_GLenum_const_GLshort___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLshort___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLshort___GENPT__ args = unpacked->args;
            glMultiTexCoord3sv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord4d
        case glMultiTexCoord4d_INDEX: {
            INDEXED_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLenum_GLdouble_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glMultiTexCoord4d(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord4dv
        case glMultiTexCoord4dv_INDEX: {
            INDEXED_void_GLenum_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLdouble___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLdouble___GENPT__ args = unpacked->args;
            glMultiTexCoord4dv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord4f
        case glMultiTexCoord4f_INDEX: {
            INDEXED_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLenum_GLfloat_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glMultiTexCoord4f(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord4fv
        case glMultiTexCoord4fv_INDEX: {
            INDEXED_void_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glMultiTexCoord4fv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord4i
        case glMultiTexCoord4i_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLint_GLint *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLint_GLint *)packed;
            ARGS_void_GLenum_GLint_GLint_GLint_GLint args = unpacked->args;
            glMultiTexCoord4i(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord4iv
        case glMultiTexCoord4iv_INDEX: {
            INDEXED_void_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glMultiTexCoord4iv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord4s
        case glMultiTexCoord4s_INDEX: {
            INDEXED_void_GLenum_GLshort_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLenum_GLshort_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLenum_GLshort_GLshort_GLshort_GLshort args = unpacked->args;
            glMultiTexCoord4s(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glMultiTexCoord4sv
        case glMultiTexCoord4sv_INDEX: {
            INDEXED_void_GLenum_const_GLshort___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLshort___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLshort___GENPT__ args = unpacked->args;
            glMultiTexCoord4sv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glNewList
        case glNewList_INDEX: {
            INDEXED_void_GLuint_GLenum *unpacked = (INDEXED_void_GLuint_GLenum *)packed;
            ARGS_void_GLuint_GLenum args = unpacked->args;
            glNewList(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glNormal3b
        case glNormal3b_INDEX: {
            INDEXED_void_GLbyte_GLbyte_GLbyte *unpacked = (INDEXED_void_GLbyte_GLbyte_GLbyte *)packed;
            ARGS_void_GLbyte_GLbyte_GLbyte args = unpacked->args;
            glNormal3b(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glNormal3bv
        case glNormal3bv_INDEX: {
            INDEXED_void_const_GLbyte___GENPT__ *unpacked = (INDEXED_void_const_GLbyte___GENPT__ *)packed;
            ARGS_void_const_GLbyte___GENPT__ args = unpacked->args;
            glNormal3bv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glNormal3d
        case glNormal3d_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glNormal3d(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glNormal3dv
        case glNormal3dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glNormal3dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glNormal3f
        case glNormal3f_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glNormal3f(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glNormal3fv
        case glNormal3fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glNormal3fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glNormal3i
        case glNormal3i_INDEX: {
            INDEXED_void_GLint_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint args = unpacked->args;
            glNormal3i(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glNormal3iv
        case glNormal3iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glNormal3iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glNormal3s
        case glNormal3s_INDEX: {
            INDEXED_void_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort_GLshort args = unpacked->args;
            glNormal3s(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glNormal3sv
        case glNormal3sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glNormal3sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glNormalPointer
        case glNormalPointer_INDEX: {
            INDEXED_void_GLenum_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glNormalPointer(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glOrtho
        case glOrtho_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glOrtho(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glPassThrough
        case glPassThrough_INDEX: {
            INDEXED_void_GLfloat *unpacked = (INDEXED_void_GLfloat *)packed;
            ARGS_void_GLfloat args = unpacked->args;
            glPassThrough(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glPixelMapfv
        case glPixelMapfv_INDEX: {
            INDEXED_void_GLenum_GLsizei_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLsizei_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLsizei_const_GLfloat___GENPT__ args = unpacked->args;
            glPixelMapfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glPixelMapuiv
        case glPixelMapuiv_INDEX: {
            INDEXED_void_GLenum_GLsizei_const_GLuint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLsizei_const_GLuint___GENPT__ *)packed;
            ARGS_void_GLenum_GLsizei_const_GLuint___GENPT__ args = unpacked->args;
            glPixelMapuiv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glPixelMapusv
        case glPixelMapusv_INDEX: {
            INDEXED_void_GLenum_GLsizei_const_GLushort___GENPT__ *unpacked = (INDEXED_void_GLenum_GLsizei_const_GLushort___GENPT__ *)packed;
            ARGS_void_GLenum_GLsizei_const_GLushort___GENPT__ args = unpacked->args;
            glPixelMapusv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glPixelStoref
        case glPixelStoref_INDEX: {
            INDEXED_void_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLfloat args = unpacked->args;
            glPixelStoref(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glPixelStorei
        case glPixelStorei_INDEX: {
            INDEXED_void_GLenum_GLint *unpacked = (INDEXED_void_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLint args = unpacked->args;
            glPixelStorei(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glPixelTransferf
        case glPixelTransferf_INDEX: {
            INDEXED_void_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLfloat args = unpacked->args;
            glPixelTransferf(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glPixelTransferi
        case glPixelTransferi_INDEX: {
            INDEXED_void_GLenum_GLint *unpacked = (INDEXED_void_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLint args = unpacked->args;
            glPixelTransferi(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glPixelZoom
        case glPixelZoom_INDEX: {
            INDEXED_void_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat args = unpacked->args;
            glPixelZoom(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glPointParameterf
        case glPointParameterf_INDEX: {
            INDEXED_void_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLfloat args = unpacked->args;
            glPointParameterf(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glPointParameterfv
        case glPointParameterfv_INDEX: {
            INDEXED_void_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glPointParameterfv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glPointParameteri
        case glPointParameteri_INDEX: {
            INDEXED_void_GLenum_GLint *unpacked = (INDEXED_void_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLint args = unpacked->args;
            glPointParameteri(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glPointParameteriv
        case glPointParameteriv_INDEX: {
            INDEXED_void_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glPointParameteriv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glPointSize
        case glPointSize_INDEX: {
            INDEXED_void_GLfloat *unpacked = (INDEXED_void_GLfloat *)packed;
            ARGS_void_GLfloat args = unpacked->args;
            glPointSize(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glPolygonMode
        case glPolygonMode_INDEX: {
            INDEXED_void_GLenum_GLenum *unpacked = (INDEXED_void_GLenum_GLenum *)packed;
            ARGS_void_GLenum_GLenum args = unpacked->args;
            glPolygonMode(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glPolygonOffset
        case glPolygonOffset_INDEX: {
            INDEXED_void_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat args = unpacked->args;
            glPolygonOffset(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glPolygonStipple
        case glPolygonStipple_INDEX: {
            INDEXED_void_const_GLubyte___GENPT__ *unpacked = (INDEXED_void_const_GLubyte___GENPT__ *)packed;
            ARGS_void_const_GLubyte___GENPT__ args = unpacked->args;
            glPolygonStipple(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glPopAttrib
        case glPopAttrib_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glPopAttrib();
            break;
        }
        #endif
        #ifndef skip_index_glPopClientAttrib
        case glPopClientAttrib_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glPopClientAttrib();
            break;
        }
        #endif
        #ifndef skip_index_glPopMatrix
        case glPopMatrix_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glPopMatrix();
            break;
        }
        #endif
        #ifndef skip_index_glPopName
        case glPopName_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glPopName();
            break;
        }
        #endif
        #ifndef skip_index_glPrioritizeTextures
        case glPrioritizeTextures_INDEX: {
            INDEXED_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLsizei_const_GLuint___GENPT___const_GLfloat___GENPT__ args = unpacked->args;
            glPrioritizeTextures(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glPushAttrib
        case glPushAttrib_INDEX: {
            INDEXED_void_GLbitfield *unpacked = (INDEXED_void_GLbitfield *)packed;
            ARGS_void_GLbitfield args = unpacked->args;
            glPushAttrib(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glPushClientAttrib
        case glPushClientAttrib_INDEX: {
            INDEXED_void_GLbitfield *unpacked = (INDEXED_void_GLbitfield *)packed;
            ARGS_void_GLbitfield args = unpacked->args;
            glPushClientAttrib(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glPushMatrix
        case glPushMatrix_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glPushMatrix();
            break;
        }
        #endif
        #ifndef skip_index_glPushName
        case glPushName_INDEX: {
            INDEXED_void_GLuint *unpacked = (INDEXED_void_GLuint *)packed;
            ARGS_void_GLuint args = unpacked->args;
            glPushName(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos2d
        case glRasterPos2d_INDEX: {
            INDEXED_void_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble args = unpacked->args;
            glRasterPos2d(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos2dv
        case glRasterPos2dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glRasterPos2dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos2f
        case glRasterPos2f_INDEX: {
            INDEXED_void_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat args = unpacked->args;
            glRasterPos2f(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos2fv
        case glRasterPos2fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glRasterPos2fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos2i
        case glRasterPos2i_INDEX: {
            INDEXED_void_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint *)packed;
            ARGS_void_GLint_GLint args = unpacked->args;
            glRasterPos2i(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos2iv
        case glRasterPos2iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glRasterPos2iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos2s
        case glRasterPos2s_INDEX: {
            INDEXED_void_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort args = unpacked->args;
            glRasterPos2s(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos2sv
        case glRasterPos2sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glRasterPos2sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos3d
        case glRasterPos3d_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glRasterPos3d(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos3dv
        case glRasterPos3dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glRasterPos3dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos3f
        case glRasterPos3f_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glRasterPos3f(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos3fv
        case glRasterPos3fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glRasterPos3fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos3i
        case glRasterPos3i_INDEX: {
            INDEXED_void_GLint_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint args = unpacked->args;
            glRasterPos3i(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos3iv
        case glRasterPos3iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glRasterPos3iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos3s
        case glRasterPos3s_INDEX: {
            INDEXED_void_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort_GLshort args = unpacked->args;
            glRasterPos3s(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos3sv
        case glRasterPos3sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glRasterPos3sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos4d
        case glRasterPos4d_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glRasterPos4d(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos4dv
        case glRasterPos4dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glRasterPos4dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos4f
        case glRasterPos4f_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glRasterPos4f(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos4fv
        case glRasterPos4fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glRasterPos4fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos4i
        case glRasterPos4i_INDEX: {
            INDEXED_void_GLint_GLint_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint_GLint args = unpacked->args;
            glRasterPos4i(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos4iv
        case glRasterPos4iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glRasterPos4iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos4s
        case glRasterPos4s_INDEX: {
            INDEXED_void_GLshort_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort_GLshort_GLshort args = unpacked->args;
            glRasterPos4s(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glRasterPos4sv
        case glRasterPos4sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glRasterPos4sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glReadBuffer
        case glReadBuffer_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glReadBuffer(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glReadPixels
        case glReadPixels_INDEX: {
            INDEXED_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__ *)packed;
            ARGS_void_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_GLvoid___GENPT__ args = unpacked->args;
            glReadPixels(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7);
            break;
        }
        #endif
        #ifndef skip_index_glRectd
        case glRectd_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glRectd(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glRectdv
        case glRectdv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT___const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT___const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT___const_GLdouble___GENPT__ args = unpacked->args;
            glRectdv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glRectf
        case glRectf_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glRectf(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glRectfv
        case glRectfv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT___const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT___const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT___const_GLfloat___GENPT__ args = unpacked->args;
            glRectfv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glRecti
        case glRecti_INDEX: {
            INDEXED_void_GLint_GLint_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint_GLint args = unpacked->args;
            glRecti(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glRectiv
        case glRectiv_INDEX: {
            INDEXED_void_const_GLint___GENPT___const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT___const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT___const_GLint___GENPT__ args = unpacked->args;
            glRectiv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glRects
        case glRects_INDEX: {
            INDEXED_void_GLshort_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort_GLshort_GLshort args = unpacked->args;
            glRects(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glRectsv
        case glRectsv_INDEX: {
            INDEXED_void_const_GLshort___GENPT___const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT___const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT___const_GLshort___GENPT__ args = unpacked->args;
            glRectsv(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glRenderMode
        case glRenderMode_INDEX: {
            INDEXED_GLint_GLenum *unpacked = (INDEXED_GLint_GLenum *)packed;
            ARGS_GLint_GLenum args = unpacked->args;
            GLint *ret = (GLint *)ret_v;
            *ret =
            glRenderMode(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glResetHistogram
        case glResetHistogram_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glResetHistogram(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glResetMinmax
        case glResetMinmax_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glResetMinmax(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glRotated
        case glRotated_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glRotated(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glRotatef
        case glRotatef_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glRotatef(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glSampleCoverage
        case glSampleCoverage_INDEX: {
            INDEXED_void_GLfloat_GLboolean *unpacked = (INDEXED_void_GLfloat_GLboolean *)packed;
            ARGS_void_GLfloat_GLboolean args = unpacked->args;
            glSampleCoverage(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glScaled
        case glScaled_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glScaled(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glScalef
        case glScalef_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glScalef(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glScissor
        case glScissor_INDEX: {
            INDEXED_void_GLint_GLint_GLsizei_GLsizei *unpacked = (INDEXED_void_GLint_GLint_GLsizei_GLsizei *)packed;
            ARGS_void_GLint_GLint_GLsizei_GLsizei args = unpacked->args;
            glScissor(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3b
        case glSecondaryColor3b_INDEX: {
            INDEXED_void_GLbyte_GLbyte_GLbyte *unpacked = (INDEXED_void_GLbyte_GLbyte_GLbyte *)packed;
            ARGS_void_GLbyte_GLbyte_GLbyte args = unpacked->args;
            glSecondaryColor3b(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3bv
        case glSecondaryColor3bv_INDEX: {
            INDEXED_void_const_GLbyte___GENPT__ *unpacked = (INDEXED_void_const_GLbyte___GENPT__ *)packed;
            ARGS_void_const_GLbyte___GENPT__ args = unpacked->args;
            glSecondaryColor3bv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3d
        case glSecondaryColor3d_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glSecondaryColor3d(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3dv
        case glSecondaryColor3dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glSecondaryColor3dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3f
        case glSecondaryColor3f_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glSecondaryColor3f(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3fv
        case glSecondaryColor3fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glSecondaryColor3fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3i
        case glSecondaryColor3i_INDEX: {
            INDEXED_void_GLint_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint args = unpacked->args;
            glSecondaryColor3i(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3iv
        case glSecondaryColor3iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glSecondaryColor3iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3s
        case glSecondaryColor3s_INDEX: {
            INDEXED_void_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort_GLshort args = unpacked->args;
            glSecondaryColor3s(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3sv
        case glSecondaryColor3sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glSecondaryColor3sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3ub
        case glSecondaryColor3ub_INDEX: {
            INDEXED_void_GLubyte_GLubyte_GLubyte *unpacked = (INDEXED_void_GLubyte_GLubyte_GLubyte *)packed;
            ARGS_void_GLubyte_GLubyte_GLubyte args = unpacked->args;
            glSecondaryColor3ub(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3ubv
        case glSecondaryColor3ubv_INDEX: {
            INDEXED_void_const_GLubyte___GENPT__ *unpacked = (INDEXED_void_const_GLubyte___GENPT__ *)packed;
            ARGS_void_const_GLubyte___GENPT__ args = unpacked->args;
            glSecondaryColor3ubv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3ui
        case glSecondaryColor3ui_INDEX: {
            INDEXED_void_GLuint_GLuint_GLuint *unpacked = (INDEXED_void_GLuint_GLuint_GLuint *)packed;
            ARGS_void_GLuint_GLuint_GLuint args = unpacked->args;
            glSecondaryColor3ui(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3uiv
        case glSecondaryColor3uiv_INDEX: {
            INDEXED_void_const_GLuint___GENPT__ *unpacked = (INDEXED_void_const_GLuint___GENPT__ *)packed;
            ARGS_void_const_GLuint___GENPT__ args = unpacked->args;
            glSecondaryColor3uiv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3us
        case glSecondaryColor3us_INDEX: {
            INDEXED_void_GLushort_GLushort_GLushort *unpacked = (INDEXED_void_GLushort_GLushort_GLushort *)packed;
            ARGS_void_GLushort_GLushort_GLushort args = unpacked->args;
            glSecondaryColor3us(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColor3usv
        case glSecondaryColor3usv_INDEX: {
            INDEXED_void_const_GLushort___GENPT__ *unpacked = (INDEXED_void_const_GLushort___GENPT__ *)packed;
            ARGS_void_const_GLushort___GENPT__ args = unpacked->args;
            glSecondaryColor3usv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glSecondaryColorPointer
        case glSecondaryColorPointer_INDEX: {
            INDEXED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glSecondaryColorPointer(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glSelectBuffer
        case glSelectBuffer_INDEX: {
            INDEXED_void_GLsizei_GLuint___GENPT__ *unpacked = (INDEXED_void_GLsizei_GLuint___GENPT__ *)packed;
            ARGS_void_GLsizei_GLuint___GENPT__ args = unpacked->args;
            glSelectBuffer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glSeparableFilter2D
        case glSeparableFilter2D_INDEX: {
            INDEXED_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT___const_GLvoid___GENPT__ args = unpacked->args;
            glSeparableFilter2D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8);
            break;
        }
        #endif
        #ifndef skip_index_glShadeModel
        case glShadeModel_INDEX: {
            INDEXED_void_GLenum *unpacked = (INDEXED_void_GLenum *)packed;
            ARGS_void_GLenum args = unpacked->args;
            glShadeModel(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glStencilFunc
        case glStencilFunc_INDEX: {
            INDEXED_void_GLenum_GLint_GLuint *unpacked = (INDEXED_void_GLenum_GLint_GLuint *)packed;
            ARGS_void_GLenum_GLint_GLuint args = unpacked->args;
            glStencilFunc(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glStencilMask
        case glStencilMask_INDEX: {
            INDEXED_void_GLuint *unpacked = (INDEXED_void_GLuint *)packed;
            ARGS_void_GLuint args = unpacked->args;
            glStencilMask(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glStencilOp
        case glStencilOp_INDEX: {
            INDEXED_void_GLenum_GLenum_GLenum *unpacked = (INDEXED_void_GLenum_GLenum_GLenum *)packed;
            ARGS_void_GLenum_GLenum_GLenum args = unpacked->args;
            glStencilOp(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord1d
        case glTexCoord1d_INDEX: {
            INDEXED_void_GLdouble *unpacked = (INDEXED_void_GLdouble *)packed;
            ARGS_void_GLdouble args = unpacked->args;
            glTexCoord1d(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord1dv
        case glTexCoord1dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glTexCoord1dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord1f
        case glTexCoord1f_INDEX: {
            INDEXED_void_GLfloat *unpacked = (INDEXED_void_GLfloat *)packed;
            ARGS_void_GLfloat args = unpacked->args;
            glTexCoord1f(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord1fv
        case glTexCoord1fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glTexCoord1fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord1i
        case glTexCoord1i_INDEX: {
            INDEXED_void_GLint *unpacked = (INDEXED_void_GLint *)packed;
            ARGS_void_GLint args = unpacked->args;
            glTexCoord1i(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord1iv
        case glTexCoord1iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glTexCoord1iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord1s
        case glTexCoord1s_INDEX: {
            INDEXED_void_GLshort *unpacked = (INDEXED_void_GLshort *)packed;
            ARGS_void_GLshort args = unpacked->args;
            glTexCoord1s(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord1sv
        case glTexCoord1sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glTexCoord1sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord2d
        case glTexCoord2d_INDEX: {
            INDEXED_void_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble args = unpacked->args;
            glTexCoord2d(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord2dv
        case glTexCoord2dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glTexCoord2dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord2f
        case glTexCoord2f_INDEX: {
            INDEXED_void_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat args = unpacked->args;
            glTexCoord2f(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord2fv
        case glTexCoord2fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glTexCoord2fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord2i
        case glTexCoord2i_INDEX: {
            INDEXED_void_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint *)packed;
            ARGS_void_GLint_GLint args = unpacked->args;
            glTexCoord2i(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord2iv
        case glTexCoord2iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glTexCoord2iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord2s
        case glTexCoord2s_INDEX: {
            INDEXED_void_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort args = unpacked->args;
            glTexCoord2s(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord2sv
        case glTexCoord2sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glTexCoord2sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord3d
        case glTexCoord3d_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glTexCoord3d(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord3dv
        case glTexCoord3dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glTexCoord3dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord3f
        case glTexCoord3f_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glTexCoord3f(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord3fv
        case glTexCoord3fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glTexCoord3fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord3i
        case glTexCoord3i_INDEX: {
            INDEXED_void_GLint_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint args = unpacked->args;
            glTexCoord3i(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord3iv
        case glTexCoord3iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glTexCoord3iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord3s
        case glTexCoord3s_INDEX: {
            INDEXED_void_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort_GLshort args = unpacked->args;
            glTexCoord3s(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord3sv
        case glTexCoord3sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glTexCoord3sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord4d
        case glTexCoord4d_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glTexCoord4d(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord4dv
        case glTexCoord4dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glTexCoord4dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord4f
        case glTexCoord4f_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glTexCoord4f(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord4fv
        case glTexCoord4fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glTexCoord4fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord4i
        case glTexCoord4i_INDEX: {
            INDEXED_void_GLint_GLint_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint_GLint args = unpacked->args;
            glTexCoord4i(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord4iv
        case glTexCoord4iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glTexCoord4iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord4s
        case glTexCoord4s_INDEX: {
            INDEXED_void_GLshort_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort_GLshort_GLshort args = unpacked->args;
            glTexCoord4s(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoord4sv
        case glTexCoord4sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glTexCoord4sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glTexCoordPointer
        case glTexCoordPointer_INDEX: {
            INDEXED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glTexCoordPointer(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glTexEnvf
        case glTexEnvf_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLenum_GLfloat args = unpacked->args;
            glTexEnvf(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexEnvfv
        case glTexEnvfv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glTexEnvfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexEnvi
        case glTexEnvi_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint *unpacked = (INDEXED_void_GLenum_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLenum_GLint args = unpacked->args;
            glTexEnvi(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexEnviv
        case glTexEnviv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glTexEnviv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexGend
        case glTexGend_INDEX: {
            INDEXED_void_GLenum_GLenum_GLdouble *unpacked = (INDEXED_void_GLenum_GLenum_GLdouble *)packed;
            ARGS_void_GLenum_GLenum_GLdouble args = unpacked->args;
            glTexGend(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexGendv
        case glTexGendv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLdouble___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLdouble___GENPT__ args = unpacked->args;
            glTexGendv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexGenf
        case glTexGenf_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLenum_GLfloat args = unpacked->args;
            glTexGenf(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexGenfv
        case glTexGenfv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glTexGenfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexGeni
        case glTexGeni_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint *unpacked = (INDEXED_void_GLenum_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLenum_GLint args = unpacked->args;
            glTexGeni(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexGeniv
        case glTexGeniv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glTexGeniv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexImage1D
        case glTexImage1D_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLint_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glTexImage1D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8);
            break;
        }
        #endif
        #ifndef skip_index_glTexImage2D
        case glTexImage2D_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glTexImage2D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9);
            break;
        }
        #endif
        #ifndef skip_index_glTexImage3D
        case glTexImage3D_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLint_GLsizei_GLsizei_GLsizei_GLint_GLenum_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glTexImage3D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9, args.a10);
            break;
        }
        #endif
        #ifndef skip_index_glTexParameterf
        case glTexParameterf_INDEX: {
            INDEXED_void_GLenum_GLenum_GLfloat *unpacked = (INDEXED_void_GLenum_GLenum_GLfloat *)packed;
            ARGS_void_GLenum_GLenum_GLfloat args = unpacked->args;
            glTexParameterf(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexParameterfv
        case glTexParameterfv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLfloat___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLfloat___GENPT__ args = unpacked->args;
            glTexParameterfv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexParameteri
        case glTexParameteri_INDEX: {
            INDEXED_void_GLenum_GLenum_GLint *unpacked = (INDEXED_void_GLenum_GLenum_GLint *)packed;
            ARGS_void_GLenum_GLenum_GLint args = unpacked->args;
            glTexParameteri(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexParameteriv
        case glTexParameteriv_INDEX: {
            INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *unpacked = (INDEXED_void_GLenum_GLenum_const_GLint___GENPT__ *)packed;
            ARGS_void_GLenum_GLenum_const_GLint___GENPT__ args = unpacked->args;
            glTexParameteriv(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTexSubImage1D
        case glTexSubImage1D_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLint_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glTexSubImage1D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7);
            break;
        }
        #endif
        #ifndef skip_index_glTexSubImage2D
        case glTexSubImage2D_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLint_GLint_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glTexSubImage2D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9);
            break;
        }
        #endif
        #ifndef skip_index_glTexSubImage3D
        case glTexSubImage3D_INDEX: {
            INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLenum_GLint_GLint_GLint_GLint_GLsizei_GLsizei_GLsizei_GLenum_GLenum_const_GLvoid___GENPT__ args = unpacked->args;
            glTexSubImage3D(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9, args.a10, args.a11);
            break;
        }
        #endif
        #ifndef skip_index_glTranslated
        case glTranslated_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glTranslated(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glTranslatef
        case glTranslatef_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glTranslatef(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glUnmapBuffer
        case glUnmapBuffer_INDEX: {
            INDEXED_GLboolean_GLenum *unpacked = (INDEXED_GLboolean_GLenum *)packed;
            ARGS_GLboolean_GLenum args = unpacked->args;
            GLboolean *ret = (GLboolean *)ret_v;
            *ret =
            glUnmapBuffer(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glVertex2d
        case glVertex2d_INDEX: {
            INDEXED_void_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble args = unpacked->args;
            glVertex2d(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glVertex2dv
        case glVertex2dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glVertex2dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glVertex2f
        case glVertex2f_INDEX: {
            INDEXED_void_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat args = unpacked->args;
            glVertex2f(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glVertex2fv
        case glVertex2fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glVertex2fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glVertex2i
        case glVertex2i_INDEX: {
            INDEXED_void_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint *)packed;
            ARGS_void_GLint_GLint args = unpacked->args;
            glVertex2i(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glVertex2iv
        case glVertex2iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glVertex2iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glVertex2s
        case glVertex2s_INDEX: {
            INDEXED_void_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort args = unpacked->args;
            glVertex2s(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glVertex2sv
        case glVertex2sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glVertex2sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glVertex3d
        case glVertex3d_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glVertex3d(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glVertex3dv
        case glVertex3dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glVertex3dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glVertex3f
        case glVertex3f_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glVertex3f(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glVertex3fv
        case glVertex3fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glVertex3fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glVertex3i
        case glVertex3i_INDEX: {
            INDEXED_void_GLint_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint args = unpacked->args;
            glVertex3i(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glVertex3iv
        case glVertex3iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glVertex3iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glVertex3s
        case glVertex3s_INDEX: {
            INDEXED_void_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort_GLshort args = unpacked->args;
            glVertex3s(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glVertex3sv
        case glVertex3sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glVertex3sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glVertex4d
        case glVertex4d_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glVertex4d(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glVertex4dv
        case glVertex4dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glVertex4dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glVertex4f
        case glVertex4f_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glVertex4f(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glVertex4fv
        case glVertex4fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glVertex4fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glVertex4i
        case glVertex4i_INDEX: {
            INDEXED_void_GLint_GLint_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint_GLint args = unpacked->args;
            glVertex4i(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glVertex4iv
        case glVertex4iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glVertex4iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glVertex4s
        case glVertex4s_INDEX: {
            INDEXED_void_GLshort_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort_GLshort_GLshort args = unpacked->args;
            glVertex4s(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glVertex4sv
        case glVertex4sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glVertex4sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glVertexPointer
        case glVertexPointer_INDEX: {
            INDEXED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ *unpacked = (INDEXED_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ *)packed;
            ARGS_void_GLint_GLenum_GLsizei_const_GLvoid___GENPT__ args = unpacked->args;
            glVertexPointer(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glViewport
        case glViewport_INDEX: {
            INDEXED_void_GLint_GLint_GLsizei_GLsizei *unpacked = (INDEXED_void_GLint_GLint_GLsizei_GLsizei *)packed;
            ARGS_void_GLint_GLint_GLsizei_GLsizei args = unpacked->args;
            glViewport(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos2d
        case glWindowPos2d_INDEX: {
            INDEXED_void_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble args = unpacked->args;
            glWindowPos2d(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos2dv
        case glWindowPos2dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glWindowPos2dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos2f
        case glWindowPos2f_INDEX: {
            INDEXED_void_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat args = unpacked->args;
            glWindowPos2f(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos2fv
        case glWindowPos2fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glWindowPos2fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos2i
        case glWindowPos2i_INDEX: {
            INDEXED_void_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint *)packed;
            ARGS_void_GLint_GLint args = unpacked->args;
            glWindowPos2i(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos2iv
        case glWindowPos2iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glWindowPos2iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos2s
        case glWindowPos2s_INDEX: {
            INDEXED_void_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort args = unpacked->args;
            glWindowPos2s(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos2sv
        case glWindowPos2sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glWindowPos2sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos3d
        case glWindowPos3d_INDEX: {
            INDEXED_void_GLdouble_GLdouble_GLdouble *unpacked = (INDEXED_void_GLdouble_GLdouble_GLdouble *)packed;
            ARGS_void_GLdouble_GLdouble_GLdouble args = unpacked->args;
            glWindowPos3d(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos3dv
        case glWindowPos3dv_INDEX: {
            INDEXED_void_const_GLdouble___GENPT__ *unpacked = (INDEXED_void_const_GLdouble___GENPT__ *)packed;
            ARGS_void_const_GLdouble___GENPT__ args = unpacked->args;
            glWindowPos3dv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos3f
        case glWindowPos3f_INDEX: {
            INDEXED_void_GLfloat_GLfloat_GLfloat *unpacked = (INDEXED_void_GLfloat_GLfloat_GLfloat *)packed;
            ARGS_void_GLfloat_GLfloat_GLfloat args = unpacked->args;
            glWindowPos3f(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos3fv
        case glWindowPos3fv_INDEX: {
            INDEXED_void_const_GLfloat___GENPT__ *unpacked = (INDEXED_void_const_GLfloat___GENPT__ *)packed;
            ARGS_void_const_GLfloat___GENPT__ args = unpacked->args;
            glWindowPos3fv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos3i
        case glWindowPos3i_INDEX: {
            INDEXED_void_GLint_GLint_GLint *unpacked = (INDEXED_void_GLint_GLint_GLint *)packed;
            ARGS_void_GLint_GLint_GLint args = unpacked->args;
            glWindowPos3i(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos3iv
        case glWindowPos3iv_INDEX: {
            INDEXED_void_const_GLint___GENPT__ *unpacked = (INDEXED_void_const_GLint___GENPT__ *)packed;
            ARGS_void_const_GLint___GENPT__ args = unpacked->args;
            glWindowPos3iv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos3s
        case glWindowPos3s_INDEX: {
            INDEXED_void_GLshort_GLshort_GLshort *unpacked = (INDEXED_void_GLshort_GLshort_GLshort *)packed;
            ARGS_void_GLshort_GLshort_GLshort args = unpacked->args;
            glWindowPos3s(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glWindowPos3sv
        case glWindowPos3sv_INDEX: {
            INDEXED_void_const_GLshort___GENPT__ *unpacked = (INDEXED_void_const_GLshort___GENPT__ *)packed;
            ARGS_void_const_GLshort___GENPT__ args = unpacked->args;
            glWindowPos3sv(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glXBindHyperpipeSGIX
        case glXBindHyperpipeSGIX_INDEX: {
            INDEXED_int_Display___GENPT___int *unpacked = (INDEXED_int_Display___GENPT___int *)packed;
            ARGS_int_Display___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            glXBindHyperpipeSGIX(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXBindSwapBarrierSGIX
        case glXBindSwapBarrierSGIX_INDEX: {
            INDEXED_void_uint32_t_uint32_t *unpacked = (INDEXED_void_uint32_t_uint32_t *)packed;
            ARGS_void_uint32_t_uint32_t args = unpacked->args;
            glXBindSwapBarrierSGIX(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXChangeDrawableAttributes
        case glXChangeDrawableAttributes_INDEX: {
            INDEXED_void_uint32_t *unpacked = (INDEXED_void_uint32_t *)packed;
            ARGS_void_uint32_t args = unpacked->args;
            glXChangeDrawableAttributes(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glXChangeDrawableAttributesSGIX
        case glXChangeDrawableAttributesSGIX_INDEX: {
            INDEXED_void_uint32_t *unpacked = (INDEXED_void_uint32_t *)packed;
            ARGS_void_uint32_t args = unpacked->args;
            glXChangeDrawableAttributesSGIX(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glXChooseFBConfig
        case glXChooseFBConfig_INDEX: {
            INDEXED_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__ *unpacked = (INDEXED_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__ *)packed;
            ARGS_GLXFBConfig___GENPT___Display___GENPT___int_const_int___GENPT___int___GENPT__ args = unpacked->args;
            GLXFBConfig * *ret = (GLXFBConfig * *)ret_v;
            *ret =
            glXChooseFBConfig(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glXChooseVisual
        case glXChooseVisual_INDEX: {
            INDEXED_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__ *unpacked = (INDEXED_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__ *)packed;
            ARGS_XVisualInfo___GENPT___Display___GENPT___int_int___GENPT__ args = unpacked->args;
            XVisualInfo * *ret = (XVisualInfo * *)ret_v;
            *ret =
            glXChooseVisual(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glXClientInfo
        case glXClientInfo_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glXClientInfo();
            break;
        }
        #endif
        #ifndef skip_index_glXCopyContext
        case glXCopyContext_INDEX: {
            INDEXED_void_Display___GENPT___GLXContext_GLXContext_unsigned_long *unpacked = (INDEXED_void_Display___GENPT___GLXContext_GLXContext_unsigned_long *)packed;
            ARGS_void_Display___GENPT___GLXContext_GLXContext_unsigned_long args = unpacked->args;
            glXCopyContext(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glXCreateContext
        case glXCreateContext_INDEX: {
            INDEXED_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool *unpacked = (INDEXED_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool *)packed;
            ARGS_GLXContext_Display___GENPT___XVisualInfo___GENPT___GLXContext_Bool args = unpacked->args;
            GLXContext *ret = (GLXContext *)ret_v;
            *ret =
            glXCreateContext(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glXCreateContextAttribsARB
        case glXCreateContextAttribsARB_INDEX: {
            INDEXED_GLXContext_Display___GENPT___void___GENPT___GLXContext_Bool_const_int___GENPT__ *unpacked = (INDEXED_GLXContext_Display___GENPT___void___GENPT___GLXContext_Bool_const_int___GENPT__ *)packed;
            ARGS_GLXContext_Display___GENPT___void___GENPT___GLXContext_Bool_const_int___GENPT__ args = unpacked->args;
            GLXContext *ret = (GLXContext *)ret_v;
            *ret =
            glXCreateContextAttribsARB(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glXCreateContextWithConfigSGIX
        case glXCreateContextWithConfigSGIX_INDEX: {
            INDEXED_void_uint32_t_uint32_t_uint32_t_uint32_t *unpacked = (INDEXED_void_uint32_t_uint32_t_uint32_t_uint32_t *)packed;
            ARGS_void_uint32_t_uint32_t_uint32_t_uint32_t args = unpacked->args;
            glXCreateContextWithConfigSGIX(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glXCreateGLXPbufferSGIX
        case glXCreateGLXPbufferSGIX_INDEX: {
            INDEXED_void_uint32_t_uint32_t *unpacked = (INDEXED_void_uint32_t_uint32_t *)packed;
            ARGS_void_uint32_t_uint32_t args = unpacked->args;
            glXCreateGLXPbufferSGIX(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXCreateGLXPixmap
        case glXCreateGLXPixmap_INDEX: {
            INDEXED_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap *unpacked = (INDEXED_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap *)packed;
            ARGS_GLXPixmap_Display___GENPT___XVisualInfo___GENPT___Pixmap args = unpacked->args;
            GLXPixmap *ret = (GLXPixmap *)ret_v;
            *ret =
            glXCreateGLXPixmap(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glXCreateGLXPixmapWithConfigSGIX
        case glXCreateGLXPixmapWithConfigSGIX_INDEX: {
            INDEXED_void_uint32_t_uint32_t_uint32_t *unpacked = (INDEXED_void_uint32_t_uint32_t_uint32_t *)packed;
            ARGS_void_uint32_t_uint32_t_uint32_t args = unpacked->args;
            glXCreateGLXPixmapWithConfigSGIX(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glXCreateGLXVideoSourceSGIX
        case glXCreateGLXVideoSourceSGIX_INDEX: {
            INDEXED_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t *unpacked = (INDEXED_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t *)packed;
            ARGS_void_Display___GENPT___uint32_t_uint32_t_uint32_t_uint32_t_uint32_t args = unpacked->args;
            glXCreateGLXVideoSourceSGIX(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glXCreateNewContext
        case glXCreateNewContext_INDEX: {
            INDEXED_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool *unpacked = (INDEXED_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool *)packed;
            ARGS_GLXContext_Display___GENPT___GLXFBConfig_int_GLXContext_Bool args = unpacked->args;
            GLXContext *ret = (GLXContext *)ret_v;
            *ret =
            glXCreateNewContext(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glXCreatePbuffer
        case glXCreatePbuffer_INDEX: {
            INDEXED_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__ *unpacked = (INDEXED_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__ *)packed;
            ARGS_GLXPbuffer_Display___GENPT___GLXFBConfig_const_int___GENPT__ args = unpacked->args;
            GLXPbuffer *ret = (GLXPbuffer *)ret_v;
            *ret =
            glXCreatePbuffer(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glXCreatePixmap
        case glXCreatePixmap_INDEX: {
            INDEXED_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__ *unpacked = (INDEXED_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__ *)packed;
            ARGS_GLXPixmap_Display___GENPT___GLXFBConfig_Pixmap_const_int___GENPT__ args = unpacked->args;
            GLXPixmap *ret = (GLXPixmap *)ret_v;
            *ret =
            glXCreatePixmap(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glXCreateWindow
        case glXCreateWindow_INDEX: {
            INDEXED_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__ *unpacked = (INDEXED_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__ *)packed;
            ARGS_GLXWindow_Display___GENPT___GLXFBConfig_Window_const_int___GENPT__ args = unpacked->args;
            GLXWindow *ret = (GLXWindow *)ret_v;
            *ret =
            glXCreateWindow(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glXDestroyContext
        case glXDestroyContext_INDEX: {
            INDEXED_void_Display___GENPT___GLXContext *unpacked = (INDEXED_void_Display___GENPT___GLXContext *)packed;
            ARGS_void_Display___GENPT___GLXContext args = unpacked->args;
            glXDestroyContext(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXDestroyGLXPbufferSGIX
        case glXDestroyGLXPbufferSGIX_INDEX: {
            INDEXED_void_uint32_t *unpacked = (INDEXED_void_uint32_t *)packed;
            ARGS_void_uint32_t args = unpacked->args;
            glXDestroyGLXPbufferSGIX(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glXDestroyGLXPixmap
        case glXDestroyGLXPixmap_INDEX: {
            INDEXED_void_Display___GENPT___GLXPixmap *unpacked = (INDEXED_void_Display___GENPT___GLXPixmap *)packed;
            ARGS_void_Display___GENPT___GLXPixmap args = unpacked->args;
            glXDestroyGLXPixmap(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXDestroyGLXVideoSourceSGIX
        case glXDestroyGLXVideoSourceSGIX_INDEX: {
            INDEXED_void_Display___GENPT___uint32_t *unpacked = (INDEXED_void_Display___GENPT___uint32_t *)packed;
            ARGS_void_Display___GENPT___uint32_t args = unpacked->args;
            glXDestroyGLXVideoSourceSGIX(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXDestroyHyperpipeConfigSGIX
        case glXDestroyHyperpipeConfigSGIX_INDEX: {
            INDEXED_int_Display___GENPT___int *unpacked = (INDEXED_int_Display___GENPT___int *)packed;
            ARGS_int_Display___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            glXDestroyHyperpipeConfigSGIX(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXDestroyPbuffer
        case glXDestroyPbuffer_INDEX: {
            INDEXED_void_Display___GENPT___GLXPbuffer *unpacked = (INDEXED_void_Display___GENPT___GLXPbuffer *)packed;
            ARGS_void_Display___GENPT___GLXPbuffer args = unpacked->args;
            glXDestroyPbuffer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXDestroyPixmap
        case glXDestroyPixmap_INDEX: {
            INDEXED_void_Display___GENPT___GLXPixmap *unpacked = (INDEXED_void_Display___GENPT___GLXPixmap *)packed;
            ARGS_void_Display___GENPT___GLXPixmap args = unpacked->args;
            glXDestroyPixmap(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXDestroyWindow
        case glXDestroyWindow_INDEX: {
            INDEXED_void_Display___GENPT___GLXWindow *unpacked = (INDEXED_void_Display___GENPT___GLXWindow *)packed;
            ARGS_void_Display___GENPT___GLXWindow args = unpacked->args;
            glXDestroyWindow(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXGetClientString
        case glXGetClientString_INDEX: {
            INDEXED_const_char___GENPT___Display___GENPT___int *unpacked = (INDEXED_const_char___GENPT___Display___GENPT___int *)packed;
            ARGS_const_char___GENPT___Display___GENPT___int args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            glXGetClientString(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXGetConfig
        case glXGetConfig_INDEX: {
            INDEXED_int_Display___GENPT___XVisualInfo___GENPT___int_int___GENPT__ *unpacked = (INDEXED_int_Display___GENPT___XVisualInfo___GENPT___int_int___GENPT__ *)packed;
            ARGS_int_Display___GENPT___XVisualInfo___GENPT___int_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            glXGetConfig(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glXGetCurrentContext
        case glXGetCurrentContext_INDEX: {
            INDEXED_GLXContext *unpacked = (INDEXED_GLXContext *)packed;
            GLXContext *ret = (GLXContext *)ret_v;
            *ret =
            glXGetCurrentContext();
            break;
        }
        #endif
        #ifndef skip_index_glXGetCurrentDisplay
        case glXGetCurrentDisplay_INDEX: {
            INDEXED_Display___GENPT__ *unpacked = (INDEXED_Display___GENPT__ *)packed;
            Display * *ret = (Display * *)ret_v;
            *ret =
            glXGetCurrentDisplay();
            break;
        }
        #endif
        #ifndef skip_index_glXGetCurrentDrawable
        case glXGetCurrentDrawable_INDEX: {
            INDEXED_GLXDrawable *unpacked = (INDEXED_GLXDrawable *)packed;
            GLXDrawable *ret = (GLXDrawable *)ret_v;
            *ret =
            glXGetCurrentDrawable();
            break;
        }
        #endif
        #ifndef skip_index_glXGetCurrentReadDrawable
        case glXGetCurrentReadDrawable_INDEX: {
            INDEXED_GLXDrawable *unpacked = (INDEXED_GLXDrawable *)packed;
            GLXDrawable *ret = (GLXDrawable *)ret_v;
            *ret =
            glXGetCurrentReadDrawable();
            break;
        }
        #endif
        #ifndef skip_index_glXGetDrawableAttributes
        case glXGetDrawableAttributes_INDEX: {
            INDEXED_void_uint32_t *unpacked = (INDEXED_void_uint32_t *)packed;
            ARGS_void_uint32_t args = unpacked->args;
            glXGetDrawableAttributes(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glXGetDrawableAttributesSGIX
        case glXGetDrawableAttributesSGIX_INDEX: {
            INDEXED_void_uint32_t *unpacked = (INDEXED_void_uint32_t *)packed;
            ARGS_void_uint32_t args = unpacked->args;
            glXGetDrawableAttributesSGIX(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glXGetFBConfigAttrib
        case glXGetFBConfigAttrib_INDEX: {
            INDEXED_int_Display___GENPT___GLXFBConfig_int_int___GENPT__ *unpacked = (INDEXED_int_Display___GENPT___GLXFBConfig_int_int___GENPT__ *)packed;
            ARGS_int_Display___GENPT___GLXFBConfig_int_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            glXGetFBConfigAttrib(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glXGetFBConfigs
        case glXGetFBConfigs_INDEX: {
            INDEXED_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__ *unpacked = (INDEXED_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__ *)packed;
            ARGS_GLXFBConfig___GENPT___Display___GENPT___int_int___GENPT__ args = unpacked->args;
            GLXFBConfig * *ret = (GLXFBConfig * *)ret_v;
            *ret =
            glXGetFBConfigs(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glXGetFBConfigsSGIX
        case glXGetFBConfigsSGIX_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glXGetFBConfigsSGIX();
            break;
        }
        #endif
        #ifndef skip_index_glXGetProcAddress
        case glXGetProcAddress_INDEX: {
            INDEXED___GLXextFuncPtr_const_GLubyte___GENPT__ *unpacked = (INDEXED___GLXextFuncPtr_const_GLubyte___GENPT__ *)packed;
            ARGS___GLXextFuncPtr_const_GLubyte___GENPT__ args = unpacked->args;
            __GLXextFuncPtr *ret = (__GLXextFuncPtr *)ret_v;
            *ret =
            glXGetProcAddress(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glXGetSelectedEvent
        case glXGetSelectedEvent_INDEX: {
            INDEXED_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__ *unpacked = (INDEXED_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__ *)packed;
            ARGS_void_Display___GENPT___GLXDrawable_unsigned_long___GENPT__ args = unpacked->args;
            glXGetSelectedEvent(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glXGetVisualConfigs
        case glXGetVisualConfigs_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glXGetVisualConfigs();
            break;
        }
        #endif
        #ifndef skip_index_glXGetVisualFromFBConfig
        case glXGetVisualFromFBConfig_INDEX: {
            INDEXED_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig *unpacked = (INDEXED_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig *)packed;
            ARGS_XVisualInfo___GENPT___Display___GENPT___GLXFBConfig args = unpacked->args;
            XVisualInfo * *ret = (XVisualInfo * *)ret_v;
            *ret =
            glXGetVisualFromFBConfig(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXHyperpipeAttribSGIX
        case glXHyperpipeAttribSGIX_INDEX: {
            INDEXED_int_Display___GENPT___int_int_int_const_void___GENPT__ *unpacked = (INDEXED_int_Display___GENPT___int_int_int_const_void___GENPT__ *)packed;
            ARGS_int_Display___GENPT___int_int_int_const_void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            glXHyperpipeAttribSGIX(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glXHyperpipeConfigSGIX
        case glXHyperpipeConfigSGIX_INDEX: {
            INDEXED_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__ *unpacked = (INDEXED_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__ *)packed;
            ARGS_int_Display___GENPT___int_int_GLXHyperpipeConfigSGIX_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            glXHyperpipeConfigSGIX(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glXIsDirect
        case glXIsDirect_INDEX: {
            INDEXED_Bool_Display___GENPT___GLXContext *unpacked = (INDEXED_Bool_Display___GENPT___GLXContext *)packed;
            ARGS_Bool_Display___GENPT___GLXContext args = unpacked->args;
            Bool *ret = (Bool *)ret_v;
            *ret =
            glXIsDirect(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXJoinSwapGroupSGIX
        case glXJoinSwapGroupSGIX_INDEX: {
            INDEXED_void_uint32_t_uint32_t *unpacked = (INDEXED_void_uint32_t_uint32_t *)packed;
            ARGS_void_uint32_t_uint32_t args = unpacked->args;
            glXJoinSwapGroupSGIX(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXMakeContextCurrent
        case glXMakeContextCurrent_INDEX: {
            INDEXED_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext *unpacked = (INDEXED_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext *)packed;
            ARGS_Bool_Display___GENPT___GLXDrawable_GLXDrawable_GLXContext args = unpacked->args;
            Bool *ret = (Bool *)ret_v;
            *ret =
            glXMakeContextCurrent(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glXMakeCurrent
        case glXMakeCurrent_INDEX: {
            INDEXED_Bool_Display___GENPT___GLXDrawable_GLXContext *unpacked = (INDEXED_Bool_Display___GENPT___GLXDrawable_GLXContext *)packed;
            ARGS_Bool_Display___GENPT___GLXDrawable_GLXContext args = unpacked->args;
            Bool *ret = (Bool *)ret_v;
            *ret =
            glXMakeCurrent(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glXMakeCurrentReadSGI
        case glXMakeCurrentReadSGI_INDEX: {
            INDEXED_void_uint32_t_uint32_t_uint32_t *unpacked = (INDEXED_void_uint32_t_uint32_t_uint32_t *)packed;
            ARGS_void_uint32_t_uint32_t_uint32_t args = unpacked->args;
            glXMakeCurrentReadSGI(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glXQueryContext
        case glXQueryContext_INDEX: {
            INDEXED_int_Display___GENPT___GLXContext_int_int___GENPT__ *unpacked = (INDEXED_int_Display___GENPT___GLXContext_int_int___GENPT__ *)packed;
            ARGS_int_Display___GENPT___GLXContext_int_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            glXQueryContext(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glXQueryContextInfoEXT
        case glXQueryContextInfoEXT_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glXQueryContextInfoEXT();
            break;
        }
        #endif
        #ifndef skip_index_glXQueryDrawable
        case glXQueryDrawable_INDEX: {
            INDEXED_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__ *unpacked = (INDEXED_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__ *)packed;
            ARGS_void_Display___GENPT___GLXDrawable_int_unsigned_int___GENPT__ args = unpacked->args;
            glXQueryDrawable(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glXQueryExtension
        case glXQueryExtension_INDEX: {
            INDEXED_Bool_Display___GENPT___int___GENPT___int___GENPT__ *unpacked = (INDEXED_Bool_Display___GENPT___int___GENPT___int___GENPT__ *)packed;
            ARGS_Bool_Display___GENPT___int___GENPT___int___GENPT__ args = unpacked->args;
            Bool *ret = (Bool *)ret_v;
            *ret =
            glXQueryExtension(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glXQueryExtensionsString
        case glXQueryExtensionsString_INDEX: {
            INDEXED_const_char___GENPT___Display___GENPT___int *unpacked = (INDEXED_const_char___GENPT___Display___GENPT___int *)packed;
            ARGS_const_char___GENPT___Display___GENPT___int args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            glXQueryExtensionsString(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXQueryHyperpipeAttribSGIX
        case glXQueryHyperpipeAttribSGIX_INDEX: {
            INDEXED_int_Display___GENPT___int_int_int_const_void___GENPT__ *unpacked = (INDEXED_int_Display___GENPT___int_int_int_const_void___GENPT__ *)packed;
            ARGS_int_Display___GENPT___int_int_int_const_void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            glXQueryHyperpipeAttribSGIX(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_glXQueryHyperpipeBestAttribSGIX
        case glXQueryHyperpipeBestAttribSGIX_INDEX: {
            INDEXED_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__ *unpacked = (INDEXED_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__ *)packed;
            ARGS_int_Display___GENPT___int_int_int_const_void___GENPT___void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            glXQueryHyperpipeBestAttribSGIX(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_glXQueryHyperpipeConfigSGIX
        case glXQueryHyperpipeConfigSGIX_INDEX: {
            INDEXED_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__ *unpacked = (INDEXED_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__ *)packed;
            ARGS_GLXHyperpipeConfigSGIX___GENPT___Display___GENPT___int_int___GENPT__ args = unpacked->args;
            GLXHyperpipeConfigSGIX * *ret = (GLXHyperpipeConfigSGIX * *)ret_v;
            *ret =
            glXQueryHyperpipeConfigSGIX(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glXQueryHyperpipeNetworkSGIX
        case glXQueryHyperpipeNetworkSGIX_INDEX: {
            INDEXED_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__ *unpacked = (INDEXED_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__ *)packed;
            ARGS_GLXHyperpipeNetworkSGIX___GENPT___Display___GENPT___int___GENPT__ args = unpacked->args;
            GLXHyperpipeNetworkSGIX * *ret = (GLXHyperpipeNetworkSGIX * *)ret_v;
            *ret =
            glXQueryHyperpipeNetworkSGIX(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXQueryMaxSwapBarriersSGIX
        case glXQueryMaxSwapBarriersSGIX_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glXQueryMaxSwapBarriersSGIX();
            break;
        }
        #endif
        #ifndef skip_index_glXQueryServerString
        case glXQueryServerString_INDEX: {
            INDEXED_const_char___GENPT___Display___GENPT___int_int *unpacked = (INDEXED_const_char___GENPT___Display___GENPT___int_int *)packed;
            ARGS_const_char___GENPT___Display___GENPT___int_int args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            glXQueryServerString(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glXQueryVersion
        case glXQueryVersion_INDEX: {
            INDEXED_Bool_Display___GENPT___int___GENPT___int___GENPT__ *unpacked = (INDEXED_Bool_Display___GENPT___int___GENPT___int___GENPT__ *)packed;
            ARGS_Bool_Display___GENPT___int___GENPT___int___GENPT__ args = unpacked->args;
            Bool *ret = (Bool *)ret_v;
            *ret =
            glXQueryVersion(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glXReleaseBuffersMESA
        case glXReleaseBuffersMESA_INDEX: {
            INDEXED_Bool_Display___GENPT___GLXDrawable *unpacked = (INDEXED_Bool_Display___GENPT___GLXDrawable *)packed;
            ARGS_Bool_Display___GENPT___GLXDrawable args = unpacked->args;
            Bool *ret = (Bool *)ret_v;
            *ret =
            glXReleaseBuffersMESA(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXRender
        case glXRender_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glXRender();
            break;
        }
        #endif
        #ifndef skip_index_glXRenderLarge
        case glXRenderLarge_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glXRenderLarge();
            break;
        }
        #endif
        #ifndef skip_index_glXSelectEvent
        case glXSelectEvent_INDEX: {
            INDEXED_void_Display___GENPT___GLXDrawable_unsigned_long *unpacked = (INDEXED_void_Display___GENPT___GLXDrawable_unsigned_long *)packed;
            ARGS_void_Display___GENPT___GLXDrawable_unsigned_long args = unpacked->args;
            glXSelectEvent(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_glXSwapBuffers
        case glXSwapBuffers_INDEX: {
            INDEXED_void_Display___GENPT___GLXDrawable *unpacked = (INDEXED_void_Display___GENPT___GLXDrawable *)packed;
            ARGS_void_Display___GENPT___GLXDrawable args = unpacked->args;
            glXSwapBuffers(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_glXSwapIntervalMESA
        case glXSwapIntervalMESA_INDEX: {
            INDEXED_int_unsigned_int *unpacked = (INDEXED_int_unsigned_int *)packed;
            ARGS_int_unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            glXSwapIntervalMESA(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glXSwapIntervalSGI
        case glXSwapIntervalSGI_INDEX: {
            INDEXED_void_unsigned_int *unpacked = (INDEXED_void_unsigned_int *)packed;
            ARGS_void_unsigned_int args = unpacked->args;
            glXSwapIntervalSGI(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_glXUseXFont
        case glXUseXFont_INDEX: {
            INDEXED_void_Font_int_int_int *unpacked = (INDEXED_void_Font_int_int_int *)packed;
            ARGS_void_Font_int_int_int args = unpacked->args;
            glXUseXFont(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_glXVendorPrivate
        case glXVendorPrivate_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glXVendorPrivate();
            break;
        }
        #endif
        #ifndef skip_index_glXVendorPrivateWithReply
        case glXVendorPrivateWithReply_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glXVendorPrivateWithReply();
            break;
        }
        #endif
        #ifndef skip_index_glXWaitGL
        case glXWaitGL_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glXWaitGL();
            break;
        }
        #endif
        #ifndef skip_index_glXWaitX
        case glXWaitX_INDEX: {
            INDEXED_void *unpacked = (INDEXED_void *)packed;
            glXWaitX();
            break;
        }
        #endif
        #ifndef skip_index_snd_asoundlib_version
        case snd_asoundlib_version_INDEX: {
            INDEXED_const_char___GENPT__ *unpacked = (INDEXED_const_char___GENPT__ *)packed;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_asoundlib_version();
            break;
        }
        #endif
        #ifndef skip_index_snd_async_add_ctl_handler
        case snd_async_add_ctl_handler_INDEX: {
            INDEXED_int_snd_async_handler_t___GENPT____GENPT___snd_ctl_t___GENPT___snd_async_callback_t_void___GENPT__ *unpacked = (INDEXED_int_snd_async_handler_t___GENPT____GENPT___snd_ctl_t___GENPT___snd_async_callback_t_void___GENPT__ *)packed;
            ARGS_int_snd_async_handler_t___GENPT____GENPT___snd_ctl_t___GENPT___snd_async_callback_t_void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_async_add_ctl_handler(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_async_add_handler
        case snd_async_add_handler_INDEX: {
            INDEXED_int_snd_async_handler_t___GENPT____GENPT___int_snd_async_callback_t_void___GENPT__ *unpacked = (INDEXED_int_snd_async_handler_t___GENPT____GENPT___int_snd_async_callback_t_void___GENPT__ *)packed;
            ARGS_int_snd_async_handler_t___GENPT____GENPT___int_snd_async_callback_t_void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_async_add_handler(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_async_add_pcm_handler
        case snd_async_add_pcm_handler_INDEX: {
            INDEXED_int_snd_async_handler_t___GENPT____GENPT___snd_pcm_t___GENPT___snd_async_callback_t_void___GENPT__ *unpacked = (INDEXED_int_snd_async_handler_t___GENPT____GENPT___snd_pcm_t___GENPT___snd_async_callback_t_void___GENPT__ *)packed;
            ARGS_int_snd_async_handler_t___GENPT____GENPT___snd_pcm_t___GENPT___snd_async_callback_t_void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_async_add_pcm_handler(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_async_add_timer_handler
        case snd_async_add_timer_handler_INDEX: {
            INDEXED_int_snd_async_handler_t___GENPT____GENPT___snd_timer_t___GENPT___snd_async_callback_t_void___GENPT__ *unpacked = (INDEXED_int_snd_async_handler_t___GENPT____GENPT___snd_timer_t___GENPT___snd_async_callback_t_void___GENPT__ *)packed;
            ARGS_int_snd_async_handler_t___GENPT____GENPT___snd_timer_t___GENPT___snd_async_callback_t_void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_async_add_timer_handler(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_async_del_handler
        case snd_async_del_handler_INDEX: {
            INDEXED_int_snd_async_handler_t___GENPT__ *unpacked = (INDEXED_int_snd_async_handler_t___GENPT__ *)packed;
            ARGS_int_snd_async_handler_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_async_del_handler(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_async_handler_get_callback_private
        case snd_async_handler_get_callback_private_INDEX: {
            INDEXED_void___GENPT___snd_async_handler_t___GENPT__ *unpacked = (INDEXED_void___GENPT___snd_async_handler_t___GENPT__ *)packed;
            ARGS_void___GENPT___snd_async_handler_t___GENPT__ args = unpacked->args;
            void * *ret = (void * *)ret_v;
            *ret =
            snd_async_handler_get_callback_private(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_async_handler_get_ctl
        case snd_async_handler_get_ctl_INDEX: {
            INDEXED_snd_ctl_t___GENPT___snd_async_handler_t___GENPT__ *unpacked = (INDEXED_snd_ctl_t___GENPT___snd_async_handler_t___GENPT__ *)packed;
            ARGS_snd_ctl_t___GENPT___snd_async_handler_t___GENPT__ args = unpacked->args;
            snd_ctl_t * *ret = (snd_ctl_t * *)ret_v;
            *ret =
            snd_async_handler_get_ctl(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_async_handler_get_fd
        case snd_async_handler_get_fd_INDEX: {
            INDEXED_int_snd_async_handler_t___GENPT__ *unpacked = (INDEXED_int_snd_async_handler_t___GENPT__ *)packed;
            ARGS_int_snd_async_handler_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_async_handler_get_fd(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_async_handler_get_pcm
        case snd_async_handler_get_pcm_INDEX: {
            INDEXED_snd_pcm_t___GENPT___snd_async_handler_t___GENPT__ *unpacked = (INDEXED_snd_pcm_t___GENPT___snd_async_handler_t___GENPT__ *)packed;
            ARGS_snd_pcm_t___GENPT___snd_async_handler_t___GENPT__ args = unpacked->args;
            snd_pcm_t * *ret = (snd_pcm_t * *)ret_v;
            *ret =
            snd_async_handler_get_pcm(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_async_handler_get_signo
        case snd_async_handler_get_signo_INDEX: {
            INDEXED_int_snd_async_handler_t___GENPT__ *unpacked = (INDEXED_int_snd_async_handler_t___GENPT__ *)packed;
            ARGS_int_snd_async_handler_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_async_handler_get_signo(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_async_handler_get_timer
        case snd_async_handler_get_timer_INDEX: {
            INDEXED_snd_timer_t___GENPT___snd_async_handler_t___GENPT__ *unpacked = (INDEXED_snd_timer_t___GENPT___snd_async_handler_t___GENPT__ *)packed;
            ARGS_snd_timer_t___GENPT___snd_async_handler_t___GENPT__ args = unpacked->args;
            snd_timer_t * *ret = (snd_timer_t * *)ret_v;
            *ret =
            snd_async_handler_get_timer(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_card_get_index
        case snd_card_get_index_INDEX: {
            INDEXED_int_const_char___GENPT__ *unpacked = (INDEXED_int_const_char___GENPT__ *)packed;
            ARGS_int_const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_card_get_index(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_card_get_longname
        case snd_card_get_longname_INDEX: {
            INDEXED_int_int_char___GENPT____GENPT__ *unpacked = (INDEXED_int_int_char___GENPT____GENPT__ *)packed;
            ARGS_int_int_char___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_card_get_longname(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_card_get_name
        case snd_card_get_name_INDEX: {
            INDEXED_int_int_char___GENPT____GENPT__ *unpacked = (INDEXED_int_int_char___GENPT____GENPT__ *)packed;
            ARGS_int_int_char___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_card_get_name(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_card_load
        case snd_card_load_INDEX: {
            INDEXED_int_int *unpacked = (INDEXED_int_int *)packed;
            ARGS_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_card_load(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_card_next
        case snd_card_next_INDEX: {
            INDEXED_int_int___GENPT__ *unpacked = (INDEXED_int_int___GENPT__ *)packed;
            ARGS_int_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_card_next(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_add
        case snd_config_add_INDEX: {
            INDEXED_int_snd_config_t___GENPT___snd_config_t___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT___snd_config_t___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT___snd_config_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_add(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_copy
        case snd_config_copy_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___snd_config_t___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___snd_config_t___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___snd_config_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_delete
        case snd_config_delete_INDEX: {
            INDEXED_int_snd_config_t___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_delete(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_delete_compound_members
        case snd_config_delete_compound_members_INDEX: {
            INDEXED_int_const_snd_config_t___GENPT__ *unpacked = (INDEXED_int_const_snd_config_t___GENPT__ *)packed;
            ARGS_int_const_snd_config_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_delete_compound_members(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_evaluate
        case snd_config_evaluate_INDEX: {
            INDEXED_int_snd_config_t___GENPT___snd_config_t___GENPT___snd_config_t___GENPT___snd_config_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT___snd_config_t___GENPT___snd_config_t___GENPT___snd_config_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT___snd_config_t___GENPT___snd_config_t___GENPT___snd_config_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_evaluate(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_expand
        case snd_config_expand_INDEX: {
            INDEXED_int_snd_config_t___GENPT___snd_config_t___GENPT___const_char___GENPT___snd_config_t___GENPT___snd_config_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT___snd_config_t___GENPT___const_char___GENPT___snd_config_t___GENPT___snd_config_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT___snd_config_t___GENPT___const_char___GENPT___snd_config_t___GENPT___snd_config_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_expand(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_get_ascii
        case snd_config_get_ascii_INDEX: {
            INDEXED_int_const_snd_config_t___GENPT___char___GENPT____GENPT__ *unpacked = (INDEXED_int_const_snd_config_t___GENPT___char___GENPT____GENPT__ *)packed;
            ARGS_int_const_snd_config_t___GENPT___char___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_get_ascii(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_get_bool
        case snd_config_get_bool_INDEX: {
            INDEXED_int_const_snd_config_t___GENPT__ *unpacked = (INDEXED_int_const_snd_config_t___GENPT__ *)packed;
            ARGS_int_const_snd_config_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_get_bool(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_get_bool_ascii
        case snd_config_get_bool_ascii_INDEX: {
            INDEXED_int_const_char___GENPT__ *unpacked = (INDEXED_int_const_char___GENPT__ *)packed;
            ARGS_int_const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_get_bool_ascii(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_get_ctl_iface
        case snd_config_get_ctl_iface_INDEX: {
            INDEXED_int_const_snd_config_t___GENPT__ *unpacked = (INDEXED_int_const_snd_config_t___GENPT__ *)packed;
            ARGS_int_const_snd_config_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_get_ctl_iface(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_get_ctl_iface_ascii
        case snd_config_get_ctl_iface_ascii_INDEX: {
            INDEXED_int_const_char___GENPT__ *unpacked = (INDEXED_int_const_char___GENPT__ *)packed;
            ARGS_int_const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_get_ctl_iface_ascii(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_get_id
        case snd_config_get_id_INDEX: {
            INDEXED_int_const_snd_config_t___GENPT___const_char___GENPT____GENPT__ *unpacked = (INDEXED_int_const_snd_config_t___GENPT___const_char___GENPT____GENPT__ *)packed;
            ARGS_int_const_snd_config_t___GENPT___const_char___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_get_id(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_get_integer
        case snd_config_get_integer_INDEX: {
            INDEXED_int_const_snd_config_t___GENPT___long___GENPT__ *unpacked = (INDEXED_int_const_snd_config_t___GENPT___long___GENPT__ *)packed;
            ARGS_int_const_snd_config_t___GENPT___long___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_get_integer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_get_integer64
        case snd_config_get_integer64_INDEX: {
            INDEXED_int_const_snd_config_t___GENPT___long_long___GENPT__ *unpacked = (INDEXED_int_const_snd_config_t___GENPT___long_long___GENPT__ *)packed;
            ARGS_int_const_snd_config_t___GENPT___long_long___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_get_integer64(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_get_ireal
        case snd_config_get_ireal_INDEX: {
            INDEXED_int_const_snd_config_t___GENPT___double___GENPT__ *unpacked = (INDEXED_int_const_snd_config_t___GENPT___double___GENPT__ *)packed;
            ARGS_int_const_snd_config_t___GENPT___double___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_get_ireal(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_get_pointer
        case snd_config_get_pointer_INDEX: {
            INDEXED_int_const_snd_config_t___GENPT___const_void___GENPT____GENPT__ *unpacked = (INDEXED_int_const_snd_config_t___GENPT___const_void___GENPT____GENPT__ *)packed;
            ARGS_int_const_snd_config_t___GENPT___const_void___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_get_pointer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_get_real
        case snd_config_get_real_INDEX: {
            INDEXED_int_const_snd_config_t___GENPT___double___GENPT__ *unpacked = (INDEXED_int_const_snd_config_t___GENPT___double___GENPT__ *)packed;
            ARGS_int_const_snd_config_t___GENPT___double___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_get_real(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_get_string
        case snd_config_get_string_INDEX: {
            INDEXED_int_const_snd_config_t___GENPT___const_char___GENPT____GENPT__ *unpacked = (INDEXED_int_const_snd_config_t___GENPT___const_char___GENPT____GENPT__ *)packed;
            ARGS_int_const_snd_config_t___GENPT___const_char___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_get_string(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_get_type
        case snd_config_get_type_INDEX: {
            INDEXED_snd_config_type_t_const_snd_config_t___GENPT__ *unpacked = (INDEXED_snd_config_type_t_const_snd_config_t___GENPT__ *)packed;
            ARGS_snd_config_type_t_const_snd_config_t___GENPT__ args = unpacked->args;
            snd_config_type_t *ret = (snd_config_type_t *)ret_v;
            *ret =
            snd_config_get_type(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_imake_integer
        case snd_config_imake_integer_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_long *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_long *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_long args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_imake_integer(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_imake_integer64
        case snd_config_imake_integer64_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_long_long *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_long_long *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_long_long args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_imake_integer64(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_imake_pointer
        case snd_config_imake_pointer_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_void___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_void___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_imake_pointer(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_imake_real
        case snd_config_imake_real_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_double *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_double *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_double args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_imake_real(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_imake_string
        case snd_config_imake_string_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___const_char___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_imake_string(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_iterator_end
        case snd_config_iterator_end_INDEX: {
            INDEXED_snd_config_iterator_t_const_snd_config_t___GENPT__ *unpacked = (INDEXED_snd_config_iterator_t_const_snd_config_t___GENPT__ *)packed;
            ARGS_snd_config_iterator_t_const_snd_config_t___GENPT__ args = unpacked->args;
            snd_config_iterator_t *ret = (snd_config_iterator_t *)ret_v;
            *ret =
            snd_config_iterator_end(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_iterator_entry
        case snd_config_iterator_entry_INDEX: {
            INDEXED_snd_config_t___GENPT___const_snd_config_iterator_t *unpacked = (INDEXED_snd_config_t___GENPT___const_snd_config_iterator_t *)packed;
            ARGS_snd_config_t___GENPT___const_snd_config_iterator_t args = unpacked->args;
            snd_config_t * *ret = (snd_config_t * *)ret_v;
            *ret =
            snd_config_iterator_entry(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_iterator_first
        case snd_config_iterator_first_INDEX: {
            INDEXED_snd_config_iterator_t_const_snd_config_t___GENPT__ *unpacked = (INDEXED_snd_config_iterator_t_const_snd_config_t___GENPT__ *)packed;
            ARGS_snd_config_iterator_t_const_snd_config_t___GENPT__ args = unpacked->args;
            snd_config_iterator_t *ret = (snd_config_iterator_t *)ret_v;
            *ret =
            snd_config_iterator_first(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_iterator_next
        case snd_config_iterator_next_INDEX: {
            INDEXED_snd_config_iterator_t_const_snd_config_iterator_t *unpacked = (INDEXED_snd_config_iterator_t_const_snd_config_iterator_t *)packed;
            ARGS_snd_config_iterator_t_const_snd_config_iterator_t args = unpacked->args;
            snd_config_iterator_t *ret = (snd_config_iterator_t *)ret_v;
            *ret =
            snd_config_iterator_next(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_load
        case snd_config_load_INDEX: {
            INDEXED_int_snd_config_t___GENPT___snd_input_t___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT___snd_input_t___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT___snd_input_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_load(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_load_override
        case snd_config_load_override_INDEX: {
            INDEXED_int_snd_config_t___GENPT___snd_input_t___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT___snd_input_t___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT___snd_input_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_load_override(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_make
        case snd_config_make_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___snd_config_type_t *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___snd_config_type_t *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___const_char___GENPT___snd_config_type_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_make(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_make_compound
        case snd_config_make_compound_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___int *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT___int *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___const_char___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_make_compound(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_make_integer
        case snd_config_make_integer_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_make_integer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_make_integer64
        case snd_config_make_integer64_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_make_integer64(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_make_pointer
        case snd_config_make_pointer_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_make_pointer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_make_real
        case snd_config_make_real_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_make_real(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_make_string
        case snd_config_make_string_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_make_string(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_save
        case snd_config_save_INDEX: {
            INDEXED_int_snd_config_t___GENPT___snd_output_t___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT___snd_output_t___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT___snd_output_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_save(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_search
        case snd_config_search_INDEX: {
            INDEXED_int_snd_config_t___GENPT___const_char___GENPT___snd_config_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT___const_char___GENPT___snd_config_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT___const_char___GENPT___snd_config_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_search(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_search_definition
        case snd_config_search_definition_INDEX: {
            INDEXED_int_snd_config_t___GENPT___const_char___GENPT___const_char___GENPT___snd_config_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT___const_char___GENPT___const_char___GENPT___snd_config_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT___const_char___GENPT___const_char___GENPT___snd_config_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_search_definition(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_set_ascii
        case snd_config_set_ascii_INDEX: {
            INDEXED_int_snd_config_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_set_ascii(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_set_id
        case snd_config_set_id_INDEX: {
            INDEXED_int_snd_config_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_set_id(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_set_integer
        case snd_config_set_integer_INDEX: {
            INDEXED_int_snd_config_t___GENPT___long *unpacked = (INDEXED_int_snd_config_t___GENPT___long *)packed;
            ARGS_int_snd_config_t___GENPT___long args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_set_integer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_set_integer64
        case snd_config_set_integer64_INDEX: {
            INDEXED_int_snd_config_t___GENPT___long_long *unpacked = (INDEXED_int_snd_config_t___GENPT___long_long *)packed;
            ARGS_int_snd_config_t___GENPT___long_long args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_set_integer64(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_set_pointer
        case snd_config_set_pointer_INDEX: {
            INDEXED_int_snd_config_t___GENPT___const_void___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT___const_void___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT___const_void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_set_pointer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_set_real
        case snd_config_set_real_INDEX: {
            INDEXED_int_snd_config_t___GENPT___double *unpacked = (INDEXED_int_snd_config_t___GENPT___double *)packed;
            ARGS_int_snd_config_t___GENPT___double args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_set_real(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_set_string
        case snd_config_set_string_INDEX: {
            INDEXED_int_snd_config_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_set_string(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_test_id
        case snd_config_test_id_INDEX: {
            INDEXED_int_const_snd_config_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_const_snd_config_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_const_snd_config_t___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_test_id(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_top
        case snd_config_top_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_top(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_update
        case snd_config_update_INDEX: {
            INDEXED_int *unpacked = (INDEXED_int *)packed;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_update();
            break;
        }
        #endif
        #ifndef skip_index_snd_config_update_free
        case snd_config_update_free_INDEX: {
            INDEXED_int_snd_config_update_t___GENPT__ *unpacked = (INDEXED_int_snd_config_update_t___GENPT__ *)packed;
            ARGS_int_snd_config_update_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_update_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_config_update_free_global
        case snd_config_update_free_global_INDEX: {
            INDEXED_int *unpacked = (INDEXED_int *)packed;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_update_free_global();
            break;
        }
        #endif
        #ifndef skip_index_snd_config_update_r
        case snd_config_update_r_INDEX: {
            INDEXED_int_snd_config_t___GENPT____GENPT___snd_config_update_t___GENPT____GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_config_t___GENPT____GENPT___snd_config_update_t___GENPT____GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_config_t___GENPT____GENPT___snd_config_update_t___GENPT____GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_config_update_r(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_card_info
        case snd_ctl_card_info_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___snd_ctl_card_info_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___snd_ctl_card_info_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___snd_ctl_card_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_card_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_card_info_clear
        case snd_ctl_card_info_clear_INDEX: {
            INDEXED_void_snd_ctl_card_info_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_card_info_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_card_info_t___GENPT__ args = unpacked->args;
            snd_ctl_card_info_clear(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_card_info_copy
        case snd_ctl_card_info_copy_INDEX: {
            INDEXED_void_snd_ctl_card_info_t___GENPT___const_snd_ctl_card_info_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_card_info_t___GENPT___const_snd_ctl_card_info_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_card_info_t___GENPT___const_snd_ctl_card_info_t___GENPT__ args = unpacked->args;
            snd_ctl_card_info_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_card_info_free
        case snd_ctl_card_info_free_INDEX: {
            INDEXED_void_snd_ctl_card_info_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_card_info_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_card_info_t___GENPT__ args = unpacked->args;
            snd_ctl_card_info_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_card_info_get_card
        case snd_ctl_card_info_get_card_INDEX: {
            INDEXED_int_const_snd_ctl_card_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_ctl_card_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_ctl_card_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_card_info_get_card(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_card_info_get_components
        case snd_ctl_card_info_get_components_INDEX: {
            INDEXED_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_card_info_get_components(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_card_info_get_driver
        case snd_ctl_card_info_get_driver_INDEX: {
            INDEXED_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_card_info_get_driver(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_card_info_get_id
        case snd_ctl_card_info_get_id_INDEX: {
            INDEXED_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_card_info_get_id(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_card_info_get_longname
        case snd_ctl_card_info_get_longname_INDEX: {
            INDEXED_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_card_info_get_longname(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_card_info_get_mixername
        case snd_ctl_card_info_get_mixername_INDEX: {
            INDEXED_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_card_info_get_mixername(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_card_info_get_name
        case snd_ctl_card_info_get_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_ctl_card_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_card_info_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_card_info_malloc
        case snd_ctl_card_info_malloc_INDEX: {
            INDEXED_int_snd_ctl_card_info_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_ctl_card_info_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_ctl_card_info_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_card_info_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_card_info_sizeof
        case snd_ctl_card_info_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_ctl_card_info_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_close
        case snd_ctl_close_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_close(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_add_boolean
        case snd_ctl_elem_add_boolean_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___unsigned_int *)packed;
            ARGS_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_add_boolean(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_add_iec958
        case snd_ctl_elem_add_iec958_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_add_iec958(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_add_integer
        case snd_ctl_elem_add_integer_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___unsigned_int_long_long_long *unpacked = (INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___unsigned_int_long_long_long *)packed;
            ARGS_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___unsigned_int_long_long_long args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_add_integer(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_add_integer64
        case snd_ctl_elem_add_integer64_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___unsigned_int_long_long_long_long_long_long *unpacked = (INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___unsigned_int_long_long_long_long_long_long *)packed;
            ARGS_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___unsigned_int_long_long_long_long_long_long args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_add_integer64(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_clear
        case snd_ctl_elem_id_clear_INDEX: {
            INDEXED_void_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_id_clear(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_copy
        case snd_ctl_elem_id_copy_INDEX: {
            INDEXED_void_snd_ctl_elem_id_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_id_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_id_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_id_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_free
        case snd_ctl_elem_id_free_INDEX: {
            INDEXED_void_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_id_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_get_device
        case snd_ctl_elem_id_get_device_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_id_get_device(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_get_index
        case snd_ctl_elem_id_get_index_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_id_get_index(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_get_interface
        case snd_ctl_elem_id_get_interface_INDEX: {
            INDEXED_snd_ctl_elem_iface_t_const_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_snd_ctl_elem_iface_t_const_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_snd_ctl_elem_iface_t_const_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_iface_t *ret = (snd_ctl_elem_iface_t *)ret_v;
            *ret =
            snd_ctl_elem_id_get_interface(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_get_name
        case snd_ctl_elem_id_get_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_elem_id_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_get_numid
        case snd_ctl_elem_id_get_numid_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_id_get_numid(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_get_subdevice
        case snd_ctl_elem_id_get_subdevice_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_id_get_subdevice(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_malloc
        case snd_ctl_elem_id_malloc_INDEX: {
            INDEXED_int_snd_ctl_elem_id_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_ctl_elem_id_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_ctl_elem_id_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_id_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_set_device
        case snd_ctl_elem_id_set_device_INDEX: {
            INDEXED_void_snd_ctl_elem_id_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_id_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_id_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_id_set_device(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_set_index
        case snd_ctl_elem_id_set_index_INDEX: {
            INDEXED_void_snd_ctl_elem_id_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_id_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_id_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_id_set_index(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_set_interface
        case snd_ctl_elem_id_set_interface_INDEX: {
            INDEXED_void_snd_ctl_elem_id_t___GENPT___snd_ctl_elem_iface_t *unpacked = (INDEXED_void_snd_ctl_elem_id_t___GENPT___snd_ctl_elem_iface_t *)packed;
            ARGS_void_snd_ctl_elem_id_t___GENPT___snd_ctl_elem_iface_t args = unpacked->args;
            snd_ctl_elem_id_set_interface(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_set_name
        case snd_ctl_elem_id_set_name_INDEX: {
            INDEXED_void_snd_ctl_elem_id_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_id_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_id_t___GENPT___const_char___GENPT__ args = unpacked->args;
            snd_ctl_elem_id_set_name(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_set_numid
        case snd_ctl_elem_id_set_numid_INDEX: {
            INDEXED_void_snd_ctl_elem_id_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_id_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_id_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_id_set_numid(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_set_subdevice
        case snd_ctl_elem_id_set_subdevice_INDEX: {
            INDEXED_void_snd_ctl_elem_id_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_id_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_id_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_id_set_subdevice(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_id_sizeof
        case snd_ctl_elem_id_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_ctl_elem_id_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_iface_name
        case snd_ctl_elem_iface_name_INDEX: {
            INDEXED_const_char___GENPT___snd_ctl_elem_iface_t *unpacked = (INDEXED_const_char___GENPT___snd_ctl_elem_iface_t *)packed;
            ARGS_const_char___GENPT___snd_ctl_elem_iface_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_elem_iface_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info
        case snd_ctl_elem_info_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_clear
        case snd_ctl_elem_info_clear_INDEX: {
            INDEXED_void_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_info_clear(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_copy
        case snd_ctl_elem_info_copy_INDEX: {
            INDEXED_void_snd_ctl_elem_info_t___GENPT___const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_info_t___GENPT___const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_info_t___GENPT___const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_info_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_free
        case snd_ctl_elem_info_free_INDEX: {
            INDEXED_void_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_info_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_count
        case snd_ctl_elem_info_get_count_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_info_get_count(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_device
        case snd_ctl_elem_info_get_device_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_info_get_device(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_dimension
        case snd_ctl_elem_info_get_dimension_INDEX: {
            INDEXED_int_const_snd_ctl_elem_info_t___GENPT___unsigned_int *unpacked = (INDEXED_int_const_snd_ctl_elem_info_t___GENPT___unsigned_int *)packed;
            ARGS_int_const_snd_ctl_elem_info_t___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info_get_dimension(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_dimensions
        case snd_ctl_elem_info_get_dimensions_INDEX: {
            INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info_get_dimensions(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_id
        case snd_ctl_elem_info_get_id_INDEX: {
            INDEXED_void_const_snd_ctl_elem_info_t___GENPT___snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_void_const_snd_ctl_elem_info_t___GENPT___snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_void_const_snd_ctl_elem_info_t___GENPT___snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_info_get_id(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_index
        case snd_ctl_elem_info_get_index_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_info_get_index(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_interface
        case snd_ctl_elem_info_get_interface_INDEX: {
            INDEXED_snd_ctl_elem_iface_t_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_snd_ctl_elem_iface_t_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_snd_ctl_elem_iface_t_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_iface_t *ret = (snd_ctl_elem_iface_t *)ret_v;
            *ret =
            snd_ctl_elem_info_get_interface(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_item_name
        case snd_ctl_elem_info_get_item_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_elem_info_get_item_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_items
        case snd_ctl_elem_info_get_items_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_info_get_items(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_max
        case snd_ctl_elem_info_get_max_INDEX: {
            INDEXED_long_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_long_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_long_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_ctl_elem_info_get_max(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_max64
        case snd_ctl_elem_info_get_max64_INDEX: {
            INDEXED_long_long_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_long_long_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_long_long_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            long long *ret = (long long *)ret_v;
            *ret =
            snd_ctl_elem_info_get_max64(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_min
        case snd_ctl_elem_info_get_min_INDEX: {
            INDEXED_long_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_long_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_long_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_ctl_elem_info_get_min(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_min64
        case snd_ctl_elem_info_get_min64_INDEX: {
            INDEXED_long_long_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_long_long_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_long_long_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            long long *ret = (long long *)ret_v;
            *ret =
            snd_ctl_elem_info_get_min64(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_name
        case snd_ctl_elem_info_get_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_elem_info_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_numid
        case snd_ctl_elem_info_get_numid_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_info_get_numid(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_owner
        case snd_ctl_elem_info_get_owner_INDEX: {
            INDEXED_pid_t_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_pid_t_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_pid_t_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            pid_t *ret = (pid_t *)ret_v;
            *ret =
            snd_ctl_elem_info_get_owner(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_step
        case snd_ctl_elem_info_get_step_INDEX: {
            INDEXED_long_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_long_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_long_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_ctl_elem_info_get_step(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_step64
        case snd_ctl_elem_info_get_step64_INDEX: {
            INDEXED_long_long_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_long_long_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_long_long_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            long long *ret = (long long *)ret_v;
            *ret =
            snd_ctl_elem_info_get_step64(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_subdevice
        case snd_ctl_elem_info_get_subdevice_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_info_get_subdevice(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_get_type
        case snd_ctl_elem_info_get_type_INDEX: {
            INDEXED_snd_ctl_elem_type_t_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_snd_ctl_elem_type_t_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_snd_ctl_elem_type_t_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_type_t *ret = (snd_ctl_elem_type_t *)ret_v;
            *ret =
            snd_ctl_elem_info_get_type(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_is_inactive
        case snd_ctl_elem_info_is_inactive_INDEX: {
            INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info_is_inactive(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_is_locked
        case snd_ctl_elem_info_is_locked_INDEX: {
            INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info_is_locked(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_is_owner
        case snd_ctl_elem_info_is_owner_INDEX: {
            INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info_is_owner(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_is_readable
        case snd_ctl_elem_info_is_readable_INDEX: {
            INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info_is_readable(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_is_tlv_commandable
        case snd_ctl_elem_info_is_tlv_commandable_INDEX: {
            INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info_is_tlv_commandable(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_is_tlv_readable
        case snd_ctl_elem_info_is_tlv_readable_INDEX: {
            INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info_is_tlv_readable(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_is_tlv_writable
        case snd_ctl_elem_info_is_tlv_writable_INDEX: {
            INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info_is_tlv_writable(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_is_user
        case snd_ctl_elem_info_is_user_INDEX: {
            INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info_is_user(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_is_volatile
        case snd_ctl_elem_info_is_volatile_INDEX: {
            INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info_is_volatile(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_is_writable
        case snd_ctl_elem_info_is_writable_INDEX: {
            INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info_is_writable(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_malloc
        case snd_ctl_elem_info_malloc_INDEX: {
            INDEXED_int_snd_ctl_elem_info_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_ctl_elem_info_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_ctl_elem_info_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_info_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_set_device
        case snd_ctl_elem_info_set_device_INDEX: {
            INDEXED_void_snd_ctl_elem_info_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_info_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_info_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_info_set_device(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_set_id
        case snd_ctl_elem_info_set_id_INDEX: {
            INDEXED_void_snd_ctl_elem_info_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_info_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_info_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_info_set_id(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_set_index
        case snd_ctl_elem_info_set_index_INDEX: {
            INDEXED_void_snd_ctl_elem_info_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_info_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_info_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_info_set_index(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_set_interface
        case snd_ctl_elem_info_set_interface_INDEX: {
            INDEXED_void_snd_ctl_elem_info_t___GENPT___snd_ctl_elem_iface_t *unpacked = (INDEXED_void_snd_ctl_elem_info_t___GENPT___snd_ctl_elem_iface_t *)packed;
            ARGS_void_snd_ctl_elem_info_t___GENPT___snd_ctl_elem_iface_t args = unpacked->args;
            snd_ctl_elem_info_set_interface(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_set_item
        case snd_ctl_elem_info_set_item_INDEX: {
            INDEXED_void_snd_ctl_elem_info_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_info_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_info_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_info_set_item(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_set_name
        case snd_ctl_elem_info_set_name_INDEX: {
            INDEXED_void_snd_ctl_elem_info_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_info_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_info_t___GENPT___const_char___GENPT__ args = unpacked->args;
            snd_ctl_elem_info_set_name(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_set_numid
        case snd_ctl_elem_info_set_numid_INDEX: {
            INDEXED_void_snd_ctl_elem_info_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_info_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_info_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_info_set_numid(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_set_subdevice
        case snd_ctl_elem_info_set_subdevice_INDEX: {
            INDEXED_void_snd_ctl_elem_info_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_info_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_info_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_info_set_subdevice(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_info_sizeof
        case snd_ctl_elem_info_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_ctl_elem_info_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list
        case snd_ctl_elem_list_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_list_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_list_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___snd_ctl_elem_list_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_list(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_alloc_space
        case snd_ctl_elem_list_alloc_space_INDEX: {
            INDEXED_int_snd_ctl_elem_list_t___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_ctl_elem_list_t___GENPT___unsigned_int *)packed;
            ARGS_int_snd_ctl_elem_list_t___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_list_alloc_space(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_clear
        case snd_ctl_elem_list_clear_INDEX: {
            INDEXED_void_snd_ctl_elem_list_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_list_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_list_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_list_clear(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_copy
        case snd_ctl_elem_list_copy_INDEX: {
            INDEXED_void_snd_ctl_elem_list_t___GENPT___const_snd_ctl_elem_list_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_list_t___GENPT___const_snd_ctl_elem_list_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_list_t___GENPT___const_snd_ctl_elem_list_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_list_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_free
        case snd_ctl_elem_list_free_INDEX: {
            INDEXED_void_snd_ctl_elem_list_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_list_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_list_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_list_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_free_space
        case snd_ctl_elem_list_free_space_INDEX: {
            INDEXED_void_snd_ctl_elem_list_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_list_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_list_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_list_free_space(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_get_count
        case snd_ctl_elem_list_get_count_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_list_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_list_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_list_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_list_get_count(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_get_device
        case snd_ctl_elem_list_get_device_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_list_t___GENPT___unsigned_int *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_list_t___GENPT___unsigned_int *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_list_t___GENPT___unsigned_int args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_list_get_device(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_get_id
        case snd_ctl_elem_list_get_id_INDEX: {
            INDEXED_void_const_snd_ctl_elem_list_t___GENPT___unsigned_int_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_void_const_snd_ctl_elem_list_t___GENPT___unsigned_int_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_void_const_snd_ctl_elem_list_t___GENPT___unsigned_int_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_list_get_id(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_get_index
        case snd_ctl_elem_list_get_index_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_list_t___GENPT___unsigned_int *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_list_t___GENPT___unsigned_int *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_list_t___GENPT___unsigned_int args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_list_get_index(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_get_interface
        case snd_ctl_elem_list_get_interface_INDEX: {
            INDEXED_snd_ctl_elem_iface_t_const_snd_ctl_elem_list_t___GENPT___unsigned_int *unpacked = (INDEXED_snd_ctl_elem_iface_t_const_snd_ctl_elem_list_t___GENPT___unsigned_int *)packed;
            ARGS_snd_ctl_elem_iface_t_const_snd_ctl_elem_list_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_iface_t *ret = (snd_ctl_elem_iface_t *)ret_v;
            *ret =
            snd_ctl_elem_list_get_interface(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_get_name
        case snd_ctl_elem_list_get_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_ctl_elem_list_t___GENPT___unsigned_int *unpacked = (INDEXED_const_char___GENPT___const_snd_ctl_elem_list_t___GENPT___unsigned_int *)packed;
            ARGS_const_char___GENPT___const_snd_ctl_elem_list_t___GENPT___unsigned_int args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_elem_list_get_name(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_get_numid
        case snd_ctl_elem_list_get_numid_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_list_t___GENPT___unsigned_int *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_list_t___GENPT___unsigned_int *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_list_t___GENPT___unsigned_int args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_list_get_numid(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_get_subdevice
        case snd_ctl_elem_list_get_subdevice_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_list_t___GENPT___unsigned_int *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_list_t___GENPT___unsigned_int *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_list_t___GENPT___unsigned_int args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_list_get_subdevice(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_get_used
        case snd_ctl_elem_list_get_used_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_list_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_list_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_list_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_list_get_used(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_malloc
        case snd_ctl_elem_list_malloc_INDEX: {
            INDEXED_int_snd_ctl_elem_list_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_ctl_elem_list_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_ctl_elem_list_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_list_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_set_offset
        case snd_ctl_elem_list_set_offset_INDEX: {
            INDEXED_void_snd_ctl_elem_list_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_list_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_list_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_list_set_offset(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_list_sizeof
        case snd_ctl_elem_list_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_ctl_elem_list_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_lock
        case snd_ctl_elem_lock_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_lock(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_read
        case snd_ctl_elem_read_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_read(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_remove
        case snd_ctl_elem_remove_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_remove(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_set_bytes
        case snd_ctl_elem_set_bytes_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___void___GENPT___size_t *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___void___GENPT___size_t *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___void___GENPT___size_t args = unpacked->args;
            snd_ctl_elem_set_bytes(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_tlv_command
        case snd_ctl_elem_tlv_command_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___const_unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___const_unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___const_unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_tlv_command(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_tlv_read
        case snd_ctl_elem_tlv_read_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___unsigned_int___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___unsigned_int___GENPT___unsigned_int *)packed;
            ARGS_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___unsigned_int___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_tlv_read(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_tlv_write
        case snd_ctl_elem_tlv_write_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___const_unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___const_unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT___const_unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_tlv_write(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_type_name
        case snd_ctl_elem_type_name_INDEX: {
            INDEXED_const_char___GENPT___snd_ctl_elem_type_t *unpacked = (INDEXED_const_char___GENPT___snd_ctl_elem_type_t *)packed;
            ARGS_const_char___GENPT___snd_ctl_elem_type_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_elem_type_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_unlock
        case snd_ctl_elem_unlock_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_unlock(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_clear
        case snd_ctl_elem_value_clear_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_value_clear(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_copy
        case snd_ctl_elem_value_copy_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___const_snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___const_snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___const_snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_value_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_free
        case snd_ctl_elem_value_free_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_value_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_boolean
        case snd_ctl_elem_value_get_boolean_INDEX: {
            INDEXED_int_const_snd_ctl_elem_value_t___GENPT___unsigned_int *unpacked = (INDEXED_int_const_snd_ctl_elem_value_t___GENPT___unsigned_int *)packed;
            ARGS_int_const_snd_ctl_elem_value_t___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_value_get_boolean(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_byte
        case snd_ctl_elem_value_get_byte_INDEX: {
            INDEXED_unsigned_char_const_snd_ctl_elem_value_t___GENPT___unsigned_int *unpacked = (INDEXED_unsigned_char_const_snd_ctl_elem_value_t___GENPT___unsigned_int *)packed;
            ARGS_unsigned_char_const_snd_ctl_elem_value_t___GENPT___unsigned_int args = unpacked->args;
            unsigned char *ret = (unsigned char *)ret_v;
            *ret =
            snd_ctl_elem_value_get_byte(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_bytes
        case snd_ctl_elem_value_get_bytes_INDEX: {
            INDEXED_const_void___GENPT___const_snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_const_void___GENPT___const_snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_const_void___GENPT___const_snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            const void * *ret = (const void * *)ret_v;
            *ret =
            snd_ctl_elem_value_get_bytes(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_device
        case snd_ctl_elem_value_get_device_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_value_get_device(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_enumerated
        case snd_ctl_elem_value_get_enumerated_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_value_t___GENPT___unsigned_int *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_value_t___GENPT___unsigned_int *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_value_t___GENPT___unsigned_int args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_value_get_enumerated(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_id
        case snd_ctl_elem_value_get_id_INDEX: {
            INDEXED_void_const_snd_ctl_elem_value_t___GENPT___snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_void_const_snd_ctl_elem_value_t___GENPT___snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_void_const_snd_ctl_elem_value_t___GENPT___snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_value_get_id(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_iec958
        case snd_ctl_elem_value_get_iec958_INDEX: {
            INDEXED_void_const_snd_ctl_elem_value_t___GENPT___snd_aes_iec958_t___GENPT__ *unpacked = (INDEXED_void_const_snd_ctl_elem_value_t___GENPT___snd_aes_iec958_t___GENPT__ *)packed;
            ARGS_void_const_snd_ctl_elem_value_t___GENPT___snd_aes_iec958_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_value_get_iec958(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_index
        case snd_ctl_elem_value_get_index_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_value_get_index(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_integer
        case snd_ctl_elem_value_get_integer_INDEX: {
            INDEXED_long_const_snd_ctl_elem_value_t___GENPT___unsigned_int *unpacked = (INDEXED_long_const_snd_ctl_elem_value_t___GENPT___unsigned_int *)packed;
            ARGS_long_const_snd_ctl_elem_value_t___GENPT___unsigned_int args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_ctl_elem_value_get_integer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_integer64
        case snd_ctl_elem_value_get_integer64_INDEX: {
            INDEXED_long_long_const_snd_ctl_elem_value_t___GENPT___unsigned_int *unpacked = (INDEXED_long_long_const_snd_ctl_elem_value_t___GENPT___unsigned_int *)packed;
            ARGS_long_long_const_snd_ctl_elem_value_t___GENPT___unsigned_int args = unpacked->args;
            long long *ret = (long long *)ret_v;
            *ret =
            snd_ctl_elem_value_get_integer64(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_interface
        case snd_ctl_elem_value_get_interface_INDEX: {
            INDEXED_snd_ctl_elem_iface_t_const_snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_snd_ctl_elem_iface_t_const_snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_snd_ctl_elem_iface_t_const_snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_iface_t *ret = (snd_ctl_elem_iface_t *)ret_v;
            *ret =
            snd_ctl_elem_value_get_interface(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_name
        case snd_ctl_elem_value_get_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_elem_value_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_numid
        case snd_ctl_elem_value_get_numid_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_value_get_numid(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_get_subdevice
        case snd_ctl_elem_value_get_subdevice_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_elem_value_get_subdevice(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_malloc
        case snd_ctl_elem_value_malloc_INDEX: {
            INDEXED_int_snd_ctl_elem_value_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_ctl_elem_value_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_ctl_elem_value_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_value_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_set_boolean
        case snd_ctl_elem_value_set_boolean_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int_long *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int_long *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___unsigned_int_long args = unpacked->args;
            snd_ctl_elem_value_set_boolean(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_set_byte
        case snd_ctl_elem_value_set_byte_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int_unsigned_char *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int_unsigned_char *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___unsigned_int_unsigned_char args = unpacked->args;
            snd_ctl_elem_value_set_byte(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_set_device
        case snd_ctl_elem_value_set_device_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_value_set_device(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_set_enumerated
        case snd_ctl_elem_value_set_enumerated_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int_unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int_unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___unsigned_int_unsigned_int args = unpacked->args;
            snd_ctl_elem_value_set_enumerated(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_set_id
        case snd_ctl_elem_value_set_id_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_value_set_id(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_set_iec958
        case snd_ctl_elem_value_set_iec958_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___const_snd_aes_iec958_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___const_snd_aes_iec958_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___const_snd_aes_iec958_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_value_set_iec958(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_set_index
        case snd_ctl_elem_value_set_index_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_value_set_index(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_set_integer
        case snd_ctl_elem_value_set_integer_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int_long *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int_long *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___unsigned_int_long args = unpacked->args;
            snd_ctl_elem_value_set_integer(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_set_integer64
        case snd_ctl_elem_value_set_integer64_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int_long_long *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int_long_long *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___unsigned_int_long_long args = unpacked->args;
            snd_ctl_elem_value_set_integer64(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_set_interface
        case snd_ctl_elem_value_set_interface_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___snd_ctl_elem_iface_t *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___snd_ctl_elem_iface_t *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___snd_ctl_elem_iface_t args = unpacked->args;
            snd_ctl_elem_value_set_interface(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_set_name
        case snd_ctl_elem_value_set_name_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___const_char___GENPT__ args = unpacked->args;
            snd_ctl_elem_value_set_name(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_set_numid
        case snd_ctl_elem_value_set_numid_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_value_set_numid(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_set_subdevice
        case snd_ctl_elem_value_set_subdevice_INDEX: {
            INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_ctl_elem_value_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_ctl_elem_value_t___GENPT___unsigned_int args = unpacked->args;
            snd_ctl_elem_value_set_subdevice(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_value_sizeof
        case snd_ctl_elem_value_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_ctl_elem_value_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_elem_write
        case snd_ctl_elem_write_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_elem_write(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_clear
        case snd_ctl_event_clear_INDEX: {
            INDEXED_void_snd_ctl_event_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_event_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_event_t___GENPT__ args = unpacked->args;
            snd_ctl_event_clear(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_copy
        case snd_ctl_event_copy_INDEX: {
            INDEXED_void_snd_ctl_event_t___GENPT___const_snd_ctl_event_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_event_t___GENPT___const_snd_ctl_event_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_event_t___GENPT___const_snd_ctl_event_t___GENPT__ args = unpacked->args;
            snd_ctl_event_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_elem_get_device
        case snd_ctl_event_elem_get_device_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_event_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_event_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_event_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_event_elem_get_device(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_elem_get_id
        case snd_ctl_event_elem_get_id_INDEX: {
            INDEXED_void_const_snd_ctl_event_t___GENPT___snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_void_const_snd_ctl_event_t___GENPT___snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_void_const_snd_ctl_event_t___GENPT___snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            snd_ctl_event_elem_get_id(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_elem_get_index
        case snd_ctl_event_elem_get_index_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_event_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_event_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_event_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_event_elem_get_index(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_elem_get_interface
        case snd_ctl_event_elem_get_interface_INDEX: {
            INDEXED_snd_ctl_elem_iface_t_const_snd_ctl_event_t___GENPT__ *unpacked = (INDEXED_snd_ctl_elem_iface_t_const_snd_ctl_event_t___GENPT__ *)packed;
            ARGS_snd_ctl_elem_iface_t_const_snd_ctl_event_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_iface_t *ret = (snd_ctl_elem_iface_t *)ret_v;
            *ret =
            snd_ctl_event_elem_get_interface(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_elem_get_mask
        case snd_ctl_event_elem_get_mask_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_event_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_event_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_event_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_event_elem_get_mask(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_elem_get_name
        case snd_ctl_event_elem_get_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_ctl_event_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_ctl_event_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_ctl_event_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_event_elem_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_elem_get_numid
        case snd_ctl_event_elem_get_numid_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_event_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_event_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_event_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_event_elem_get_numid(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_elem_get_subdevice
        case snd_ctl_event_elem_get_subdevice_INDEX: {
            INDEXED_unsigned_int_const_snd_ctl_event_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_ctl_event_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_ctl_event_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_ctl_event_elem_get_subdevice(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_free
        case snd_ctl_event_free_INDEX: {
            INDEXED_void_snd_ctl_event_t___GENPT__ *unpacked = (INDEXED_void_snd_ctl_event_t___GENPT__ *)packed;
            ARGS_void_snd_ctl_event_t___GENPT__ args = unpacked->args;
            snd_ctl_event_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_get_type
        case snd_ctl_event_get_type_INDEX: {
            INDEXED_snd_ctl_event_type_t_const_snd_ctl_event_t___GENPT__ *unpacked = (INDEXED_snd_ctl_event_type_t_const_snd_ctl_event_t___GENPT__ *)packed;
            ARGS_snd_ctl_event_type_t_const_snd_ctl_event_t___GENPT__ args = unpacked->args;
            snd_ctl_event_type_t *ret = (snd_ctl_event_type_t *)ret_v;
            *ret =
            snd_ctl_event_get_type(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_malloc
        case snd_ctl_event_malloc_INDEX: {
            INDEXED_int_snd_ctl_event_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_ctl_event_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_ctl_event_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_event_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_sizeof
        case snd_ctl_event_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_ctl_event_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_event_type_name
        case snd_ctl_event_type_name_INDEX: {
            INDEXED_const_char___GENPT___snd_ctl_event_type_t *unpacked = (INDEXED_const_char___GENPT___snd_ctl_event_type_t *)packed;
            ARGS_const_char___GENPT___snd_ctl_event_type_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_event_type_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_get_power_state
        case snd_ctl_get_power_state_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_get_power_state(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_hwdep_info
        case snd_ctl_hwdep_info_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___snd_hwdep_info_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___snd_hwdep_info_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___snd_hwdep_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_hwdep_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_hwdep_next_device
        case snd_ctl_hwdep_next_device_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_hwdep_next_device(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_name
        case snd_ctl_name_INDEX: {
            INDEXED_const_char___GENPT___snd_ctl_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___snd_ctl_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___snd_ctl_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_ctl_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_nonblock
        case snd_ctl_nonblock_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___int *unpacked = (INDEXED_int_snd_ctl_t___GENPT___int *)packed;
            ARGS_int_snd_ctl_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_nonblock(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_open
        case snd_ctl_open_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT____GENPT___const_char___GENPT___int *unpacked = (INDEXED_int_snd_ctl_t___GENPT____GENPT___const_char___GENPT___int *)packed;
            ARGS_int_snd_ctl_t___GENPT____GENPT___const_char___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_open(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_open_lconf
        case snd_ctl_open_lconf_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT____GENPT___const_char___GENPT___int_snd_config_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT____GENPT___const_char___GENPT___int_snd_config_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT____GENPT___const_char___GENPT___int_snd_config_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_open_lconf(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_pcm_info
        case snd_ctl_pcm_info_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___snd_pcm_info_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___snd_pcm_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_pcm_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_pcm_next_device
        case snd_ctl_pcm_next_device_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_pcm_next_device(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_pcm_prefer_subdevice
        case snd_ctl_pcm_prefer_subdevice_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___int *unpacked = (INDEXED_int_snd_ctl_t___GENPT___int *)packed;
            ARGS_int_snd_ctl_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_pcm_prefer_subdevice(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_poll_descriptors
        case snd_ctl_poll_descriptors_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___struct_pollfd___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_ctl_t___GENPT___struct_pollfd___GENPT___unsigned_int *)packed;
            ARGS_int_snd_ctl_t___GENPT___struct_pollfd___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_poll_descriptors(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_poll_descriptors_count
        case snd_ctl_poll_descriptors_count_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_poll_descriptors_count(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_poll_descriptors_revents
        case snd_ctl_poll_descriptors_revents_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_poll_descriptors_revents(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_rawmidi_info
        case snd_ctl_rawmidi_info_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_rawmidi_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_rawmidi_next_device
        case snd_ctl_rawmidi_next_device_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_rawmidi_next_device(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_rawmidi_prefer_subdevice
        case snd_ctl_rawmidi_prefer_subdevice_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___int *unpacked = (INDEXED_int_snd_ctl_t___GENPT___int *)packed;
            ARGS_int_snd_ctl_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_rawmidi_prefer_subdevice(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_read
        case snd_ctl_read_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___snd_ctl_event_t___GENPT__ *unpacked = (INDEXED_int_snd_ctl_t___GENPT___snd_ctl_event_t___GENPT__ *)packed;
            ARGS_int_snd_ctl_t___GENPT___snd_ctl_event_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_read(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_set_power_state
        case snd_ctl_set_power_state_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_ctl_t___GENPT___unsigned_int *)packed;
            ARGS_int_snd_ctl_t___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_set_power_state(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_subscribe_events
        case snd_ctl_subscribe_events_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___int *unpacked = (INDEXED_int_snd_ctl_t___GENPT___int *)packed;
            ARGS_int_snd_ctl_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_subscribe_events(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_type
        case snd_ctl_type_INDEX: {
            INDEXED_snd_ctl_type_t_snd_ctl_t___GENPT__ *unpacked = (INDEXED_snd_ctl_type_t_snd_ctl_t___GENPT__ *)packed;
            ARGS_snd_ctl_type_t_snd_ctl_t___GENPT__ args = unpacked->args;
            snd_ctl_type_t *ret = (snd_ctl_type_t *)ret_v;
            *ret =
            snd_ctl_type(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_ctl_wait
        case snd_ctl_wait_INDEX: {
            INDEXED_int_snd_ctl_t___GENPT___int *unpacked = (INDEXED_int_snd_ctl_t___GENPT___int *)packed;
            ARGS_int_snd_ctl_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_ctl_wait(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_dlclose
        case snd_dlclose_INDEX: {
            INDEXED_int_void___GENPT__ *unpacked = (INDEXED_int_void___GENPT__ *)packed;
            ARGS_int_void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_dlclose(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_dlopen
        case snd_dlopen_INDEX: {
            INDEXED_void___GENPT___const_char___GENPT___int *unpacked = (INDEXED_void___GENPT___const_char___GENPT___int *)packed;
            ARGS_void___GENPT___const_char___GENPT___int args = unpacked->args;
            void * *ret = (void * *)ret_v;
            *ret =
            snd_dlopen(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_dlsym
        case snd_dlsym_INDEX: {
            INDEXED_void___GENPT___void___GENPT___const_char___GENPT___const_char___GENPT__ *unpacked = (INDEXED_void___GENPT___void___GENPT___const_char___GENPT___const_char___GENPT__ *)packed;
            ARGS_void___GENPT___void___GENPT___const_char___GENPT___const_char___GENPT__ args = unpacked->args;
            void * *ret = (void * *)ret_v;
            *ret =
            snd_dlsym(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_close
        case snd_hctl_close_INDEX: {
            INDEXED_int_snd_hctl_t___GENPT__ *unpacked = (INDEXED_int_snd_hctl_t___GENPT__ *)packed;
            ARGS_int_snd_hctl_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_close(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_compare_fast
        case snd_hctl_compare_fast_INDEX: {
            INDEXED_int_const_snd_hctl_elem_t___GENPT___const_snd_hctl_elem_t___GENPT__ *unpacked = (INDEXED_int_const_snd_hctl_elem_t___GENPT___const_snd_hctl_elem_t___GENPT__ *)packed;
            ARGS_int_const_snd_hctl_elem_t___GENPT___const_snd_hctl_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_compare_fast(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_ctl
        case snd_hctl_ctl_INDEX: {
            INDEXED_snd_ctl_t___GENPT___snd_hctl_t___GENPT__ *unpacked = (INDEXED_snd_ctl_t___GENPT___snd_hctl_t___GENPT__ *)packed;
            ARGS_snd_ctl_t___GENPT___snd_hctl_t___GENPT__ args = unpacked->args;
            snd_ctl_t * *ret = (snd_ctl_t * *)ret_v;
            *ret =
            snd_hctl_ctl(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_get_callback_private
        case snd_hctl_elem_get_callback_private_INDEX: {
            INDEXED_void___GENPT___const_snd_hctl_elem_t___GENPT__ *unpacked = (INDEXED_void___GENPT___const_snd_hctl_elem_t___GENPT__ *)packed;
            ARGS_void___GENPT___const_snd_hctl_elem_t___GENPT__ args = unpacked->args;
            void * *ret = (void * *)ret_v;
            *ret =
            snd_hctl_elem_get_callback_private(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_get_device
        case snd_hctl_elem_get_device_INDEX: {
            INDEXED_unsigned_int_const_snd_hctl_elem_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_hctl_elem_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_hctl_elem_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_hctl_elem_get_device(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_get_hctl
        case snd_hctl_elem_get_hctl_INDEX: {
            INDEXED_snd_hctl_t___GENPT___snd_hctl_elem_t___GENPT__ *unpacked = (INDEXED_snd_hctl_t___GENPT___snd_hctl_elem_t___GENPT__ *)packed;
            ARGS_snd_hctl_t___GENPT___snd_hctl_elem_t___GENPT__ args = unpacked->args;
            snd_hctl_t * *ret = (snd_hctl_t * *)ret_v;
            *ret =
            snd_hctl_elem_get_hctl(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_get_id
        case snd_hctl_elem_get_id_INDEX: {
            INDEXED_void_const_snd_hctl_elem_t___GENPT___snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_void_const_snd_hctl_elem_t___GENPT___snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_void_const_snd_hctl_elem_t___GENPT___snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            snd_hctl_elem_get_id(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_get_index
        case snd_hctl_elem_get_index_INDEX: {
            INDEXED_unsigned_int_const_snd_hctl_elem_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_hctl_elem_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_hctl_elem_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_hctl_elem_get_index(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_get_interface
        case snd_hctl_elem_get_interface_INDEX: {
            INDEXED_snd_ctl_elem_iface_t_const_snd_hctl_elem_t___GENPT__ *unpacked = (INDEXED_snd_ctl_elem_iface_t_const_snd_hctl_elem_t___GENPT__ *)packed;
            ARGS_snd_ctl_elem_iface_t_const_snd_hctl_elem_t___GENPT__ args = unpacked->args;
            snd_ctl_elem_iface_t *ret = (snd_ctl_elem_iface_t *)ret_v;
            *ret =
            snd_hctl_elem_get_interface(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_get_name
        case snd_hctl_elem_get_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_hctl_elem_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_hctl_elem_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_hctl_elem_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_hctl_elem_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_get_numid
        case snd_hctl_elem_get_numid_INDEX: {
            INDEXED_unsigned_int_const_snd_hctl_elem_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_hctl_elem_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_hctl_elem_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_hctl_elem_get_numid(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_get_subdevice
        case snd_hctl_elem_get_subdevice_INDEX: {
            INDEXED_unsigned_int_const_snd_hctl_elem_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_hctl_elem_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_hctl_elem_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_hctl_elem_get_subdevice(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_info
        case snd_hctl_elem_info_INDEX: {
            INDEXED_int_snd_hctl_elem_t___GENPT___snd_ctl_elem_info_t___GENPT__ *unpacked = (INDEXED_int_snd_hctl_elem_t___GENPT___snd_ctl_elem_info_t___GENPT__ *)packed;
            ARGS_int_snd_hctl_elem_t___GENPT___snd_ctl_elem_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_elem_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_next
        case snd_hctl_elem_next_INDEX: {
            INDEXED_snd_hctl_elem_t___GENPT___snd_hctl_elem_t___GENPT__ *unpacked = (INDEXED_snd_hctl_elem_t___GENPT___snd_hctl_elem_t___GENPT__ *)packed;
            ARGS_snd_hctl_elem_t___GENPT___snd_hctl_elem_t___GENPT__ args = unpacked->args;
            snd_hctl_elem_t * *ret = (snd_hctl_elem_t * *)ret_v;
            *ret =
            snd_hctl_elem_next(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_prev
        case snd_hctl_elem_prev_INDEX: {
            INDEXED_snd_hctl_elem_t___GENPT___snd_hctl_elem_t___GENPT__ *unpacked = (INDEXED_snd_hctl_elem_t___GENPT___snd_hctl_elem_t___GENPT__ *)packed;
            ARGS_snd_hctl_elem_t___GENPT___snd_hctl_elem_t___GENPT__ args = unpacked->args;
            snd_hctl_elem_t * *ret = (snd_hctl_elem_t * *)ret_v;
            *ret =
            snd_hctl_elem_prev(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_read
        case snd_hctl_elem_read_INDEX: {
            INDEXED_int_snd_hctl_elem_t___GENPT___snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_int_snd_hctl_elem_t___GENPT___snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_int_snd_hctl_elem_t___GENPT___snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_elem_read(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_set_callback
        case snd_hctl_elem_set_callback_INDEX: {
            INDEXED_void_snd_hctl_elem_t___GENPT___snd_hctl_elem_callback_t *unpacked = (INDEXED_void_snd_hctl_elem_t___GENPT___snd_hctl_elem_callback_t *)packed;
            ARGS_void_snd_hctl_elem_t___GENPT___snd_hctl_elem_callback_t args = unpacked->args;
            snd_hctl_elem_set_callback(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_set_callback_private
        case snd_hctl_elem_set_callback_private_INDEX: {
            INDEXED_void_snd_hctl_elem_t___GENPT___void___GENPT__ *unpacked = (INDEXED_void_snd_hctl_elem_t___GENPT___void___GENPT__ *)packed;
            ARGS_void_snd_hctl_elem_t___GENPT___void___GENPT__ args = unpacked->args;
            snd_hctl_elem_set_callback_private(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_tlv_command
        case snd_hctl_elem_tlv_command_INDEX: {
            INDEXED_int_snd_hctl_elem_t___GENPT___const_unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_hctl_elem_t___GENPT___const_unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_hctl_elem_t___GENPT___const_unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_elem_tlv_command(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_tlv_read
        case snd_hctl_elem_tlv_read_INDEX: {
            INDEXED_int_snd_hctl_elem_t___GENPT___unsigned_int___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_hctl_elem_t___GENPT___unsigned_int___GENPT___unsigned_int *)packed;
            ARGS_int_snd_hctl_elem_t___GENPT___unsigned_int___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_elem_tlv_read(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_tlv_write
        case snd_hctl_elem_tlv_write_INDEX: {
            INDEXED_int_snd_hctl_elem_t___GENPT___const_unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_hctl_elem_t___GENPT___const_unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_hctl_elem_t___GENPT___const_unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_elem_tlv_write(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_elem_write
        case snd_hctl_elem_write_INDEX: {
            INDEXED_int_snd_hctl_elem_t___GENPT___snd_ctl_elem_value_t___GENPT__ *unpacked = (INDEXED_int_snd_hctl_elem_t___GENPT___snd_ctl_elem_value_t___GENPT__ *)packed;
            ARGS_int_snd_hctl_elem_t___GENPT___snd_ctl_elem_value_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_elem_write(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_find_elem
        case snd_hctl_find_elem_INDEX: {
            INDEXED_snd_hctl_elem_t___GENPT___snd_hctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ *unpacked = (INDEXED_snd_hctl_elem_t___GENPT___snd_hctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ *)packed;
            ARGS_snd_hctl_elem_t___GENPT___snd_hctl_t___GENPT___const_snd_ctl_elem_id_t___GENPT__ args = unpacked->args;
            snd_hctl_elem_t * *ret = (snd_hctl_elem_t * *)ret_v;
            *ret =
            snd_hctl_find_elem(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_first_elem
        case snd_hctl_first_elem_INDEX: {
            INDEXED_snd_hctl_elem_t___GENPT___snd_hctl_t___GENPT__ *unpacked = (INDEXED_snd_hctl_elem_t___GENPT___snd_hctl_t___GENPT__ *)packed;
            ARGS_snd_hctl_elem_t___GENPT___snd_hctl_t___GENPT__ args = unpacked->args;
            snd_hctl_elem_t * *ret = (snd_hctl_elem_t * *)ret_v;
            *ret =
            snd_hctl_first_elem(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_free
        case snd_hctl_free_INDEX: {
            INDEXED_int_snd_hctl_t___GENPT__ *unpacked = (INDEXED_int_snd_hctl_t___GENPT__ *)packed;
            ARGS_int_snd_hctl_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_get_callback_private
        case snd_hctl_get_callback_private_INDEX: {
            INDEXED_void___GENPT___snd_hctl_t___GENPT__ *unpacked = (INDEXED_void___GENPT___snd_hctl_t___GENPT__ *)packed;
            ARGS_void___GENPT___snd_hctl_t___GENPT__ args = unpacked->args;
            void * *ret = (void * *)ret_v;
            *ret =
            snd_hctl_get_callback_private(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_get_count
        case snd_hctl_get_count_INDEX: {
            INDEXED_unsigned_int_snd_hctl_t___GENPT__ *unpacked = (INDEXED_unsigned_int_snd_hctl_t___GENPT__ *)packed;
            ARGS_unsigned_int_snd_hctl_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_hctl_get_count(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_handle_events
        case snd_hctl_handle_events_INDEX: {
            INDEXED_int_snd_hctl_t___GENPT__ *unpacked = (INDEXED_int_snd_hctl_t___GENPT__ *)packed;
            ARGS_int_snd_hctl_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_handle_events(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_last_elem
        case snd_hctl_last_elem_INDEX: {
            INDEXED_snd_hctl_elem_t___GENPT___snd_hctl_t___GENPT__ *unpacked = (INDEXED_snd_hctl_elem_t___GENPT___snd_hctl_t___GENPT__ *)packed;
            ARGS_snd_hctl_elem_t___GENPT___snd_hctl_t___GENPT__ args = unpacked->args;
            snd_hctl_elem_t * *ret = (snd_hctl_elem_t * *)ret_v;
            *ret =
            snd_hctl_last_elem(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_load
        case snd_hctl_load_INDEX: {
            INDEXED_int_snd_hctl_t___GENPT__ *unpacked = (INDEXED_int_snd_hctl_t___GENPT__ *)packed;
            ARGS_int_snd_hctl_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_load(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_name
        case snd_hctl_name_INDEX: {
            INDEXED_const_char___GENPT___snd_hctl_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___snd_hctl_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___snd_hctl_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_hctl_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_nonblock
        case snd_hctl_nonblock_INDEX: {
            INDEXED_int_snd_hctl_t___GENPT___int *unpacked = (INDEXED_int_snd_hctl_t___GENPT___int *)packed;
            ARGS_int_snd_hctl_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_nonblock(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_open
        case snd_hctl_open_INDEX: {
            INDEXED_int_snd_hctl_t___GENPT____GENPT___const_char___GENPT___int *unpacked = (INDEXED_int_snd_hctl_t___GENPT____GENPT___const_char___GENPT___int *)packed;
            ARGS_int_snd_hctl_t___GENPT____GENPT___const_char___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_open(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_open_ctl
        case snd_hctl_open_ctl_INDEX: {
            INDEXED_int_snd_hctl_t___GENPT____GENPT___snd_ctl_t___GENPT__ *unpacked = (INDEXED_int_snd_hctl_t___GENPT____GENPT___snd_ctl_t___GENPT__ *)packed;
            ARGS_int_snd_hctl_t___GENPT____GENPT___snd_ctl_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_open_ctl(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_poll_descriptors
        case snd_hctl_poll_descriptors_INDEX: {
            INDEXED_int_snd_hctl_t___GENPT___struct_pollfd___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_hctl_t___GENPT___struct_pollfd___GENPT___unsigned_int *)packed;
            ARGS_int_snd_hctl_t___GENPT___struct_pollfd___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_poll_descriptors(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_poll_descriptors_count
        case snd_hctl_poll_descriptors_count_INDEX: {
            INDEXED_int_snd_hctl_t___GENPT__ *unpacked = (INDEXED_int_snd_hctl_t___GENPT__ *)packed;
            ARGS_int_snd_hctl_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_poll_descriptors_count(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_poll_descriptors_revents
        case snd_hctl_poll_descriptors_revents_INDEX: {
            INDEXED_int_snd_hctl_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *unpacked = (INDEXED_int_snd_hctl_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *)packed;
            ARGS_int_snd_hctl_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_poll_descriptors_revents(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_set_callback
        case snd_hctl_set_callback_INDEX: {
            INDEXED_void_snd_hctl_t___GENPT___snd_hctl_callback_t *unpacked = (INDEXED_void_snd_hctl_t___GENPT___snd_hctl_callback_t *)packed;
            ARGS_void_snd_hctl_t___GENPT___snd_hctl_callback_t args = unpacked->args;
            snd_hctl_set_callback(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_set_callback_private
        case snd_hctl_set_callback_private_INDEX: {
            INDEXED_void_snd_hctl_t___GENPT___void___GENPT__ *unpacked = (INDEXED_void_snd_hctl_t___GENPT___void___GENPT__ *)packed;
            ARGS_void_snd_hctl_t___GENPT___void___GENPT__ args = unpacked->args;
            snd_hctl_set_callback_private(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_set_compare
        case snd_hctl_set_compare_INDEX: {
            INDEXED_int_snd_hctl_t___GENPT___snd_hctl_compare_t *unpacked = (INDEXED_int_snd_hctl_t___GENPT___snd_hctl_compare_t *)packed;
            ARGS_int_snd_hctl_t___GENPT___snd_hctl_compare_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_set_compare(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hctl_wait
        case snd_hctl_wait_INDEX: {
            INDEXED_int_snd_hctl_t___GENPT___int *unpacked = (INDEXED_int_snd_hctl_t___GENPT___int *)packed;
            ARGS_int_snd_hctl_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hctl_wait(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_close
        case snd_hwdep_close_INDEX: {
            INDEXED_int_snd_hwdep_t___GENPT__ *unpacked = (INDEXED_int_snd_hwdep_t___GENPT__ *)packed;
            ARGS_int_snd_hwdep_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hwdep_close(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_image_copy
        case snd_hwdep_dsp_image_copy_INDEX: {
            INDEXED_void_snd_hwdep_dsp_image_t___GENPT___const_snd_hwdep_dsp_image_t___GENPT__ *unpacked = (INDEXED_void_snd_hwdep_dsp_image_t___GENPT___const_snd_hwdep_dsp_image_t___GENPT__ *)packed;
            ARGS_void_snd_hwdep_dsp_image_t___GENPT___const_snd_hwdep_dsp_image_t___GENPT__ args = unpacked->args;
            snd_hwdep_dsp_image_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_image_free
        case snd_hwdep_dsp_image_free_INDEX: {
            INDEXED_void_snd_hwdep_dsp_image_t___GENPT__ *unpacked = (INDEXED_void_snd_hwdep_dsp_image_t___GENPT__ *)packed;
            ARGS_void_snd_hwdep_dsp_image_t___GENPT__ args = unpacked->args;
            snd_hwdep_dsp_image_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_image_get_image
        case snd_hwdep_dsp_image_get_image_INDEX: {
            INDEXED_const_void___GENPT___const_snd_hwdep_dsp_image_t___GENPT__ *unpacked = (INDEXED_const_void___GENPT___const_snd_hwdep_dsp_image_t___GENPT__ *)packed;
            ARGS_const_void___GENPT___const_snd_hwdep_dsp_image_t___GENPT__ args = unpacked->args;
            const void * *ret = (const void * *)ret_v;
            *ret =
            snd_hwdep_dsp_image_get_image(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_image_get_index
        case snd_hwdep_dsp_image_get_index_INDEX: {
            INDEXED_unsigned_int_const_snd_hwdep_dsp_image_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_hwdep_dsp_image_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_hwdep_dsp_image_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_hwdep_dsp_image_get_index(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_image_get_length
        case snd_hwdep_dsp_image_get_length_INDEX: {
            INDEXED_size_t_const_snd_hwdep_dsp_image_t___GENPT__ *unpacked = (INDEXED_size_t_const_snd_hwdep_dsp_image_t___GENPT__ *)packed;
            ARGS_size_t_const_snd_hwdep_dsp_image_t___GENPT__ args = unpacked->args;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_hwdep_dsp_image_get_length(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_image_get_name
        case snd_hwdep_dsp_image_get_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_hwdep_dsp_image_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_hwdep_dsp_image_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_hwdep_dsp_image_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_hwdep_dsp_image_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_image_malloc
        case snd_hwdep_dsp_image_malloc_INDEX: {
            INDEXED_int_snd_hwdep_dsp_image_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_hwdep_dsp_image_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_hwdep_dsp_image_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hwdep_dsp_image_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_image_set_image
        case snd_hwdep_dsp_image_set_image_INDEX: {
            INDEXED_void_snd_hwdep_dsp_image_t___GENPT___void___GENPT__ *unpacked = (INDEXED_void_snd_hwdep_dsp_image_t___GENPT___void___GENPT__ *)packed;
            ARGS_void_snd_hwdep_dsp_image_t___GENPT___void___GENPT__ args = unpacked->args;
            snd_hwdep_dsp_image_set_image(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_image_set_index
        case snd_hwdep_dsp_image_set_index_INDEX: {
            INDEXED_void_snd_hwdep_dsp_image_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_hwdep_dsp_image_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_hwdep_dsp_image_t___GENPT___unsigned_int args = unpacked->args;
            snd_hwdep_dsp_image_set_index(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_image_set_length
        case snd_hwdep_dsp_image_set_length_INDEX: {
            INDEXED_void_snd_hwdep_dsp_image_t___GENPT___size_t *unpacked = (INDEXED_void_snd_hwdep_dsp_image_t___GENPT___size_t *)packed;
            ARGS_void_snd_hwdep_dsp_image_t___GENPT___size_t args = unpacked->args;
            snd_hwdep_dsp_image_set_length(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_image_set_name
        case snd_hwdep_dsp_image_set_name_INDEX: {
            INDEXED_void_snd_hwdep_dsp_image_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_void_snd_hwdep_dsp_image_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_void_snd_hwdep_dsp_image_t___GENPT___const_char___GENPT__ args = unpacked->args;
            snd_hwdep_dsp_image_set_name(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_image_sizeof
        case snd_hwdep_dsp_image_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_hwdep_dsp_image_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_load
        case snd_hwdep_dsp_load_INDEX: {
            INDEXED_int_snd_hwdep_t___GENPT___snd_hwdep_dsp_image_t___GENPT__ *unpacked = (INDEXED_int_snd_hwdep_t___GENPT___snd_hwdep_dsp_image_t___GENPT__ *)packed;
            ARGS_int_snd_hwdep_t___GENPT___snd_hwdep_dsp_image_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hwdep_dsp_load(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_status
        case snd_hwdep_dsp_status_INDEX: {
            INDEXED_int_snd_hwdep_t___GENPT___snd_hwdep_dsp_status_t___GENPT__ *unpacked = (INDEXED_int_snd_hwdep_t___GENPT___snd_hwdep_dsp_status_t___GENPT__ *)packed;
            ARGS_int_snd_hwdep_t___GENPT___snd_hwdep_dsp_status_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hwdep_dsp_status(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_status_copy
        case snd_hwdep_dsp_status_copy_INDEX: {
            INDEXED_void_snd_hwdep_dsp_status_t___GENPT___const_snd_hwdep_dsp_status_t___GENPT__ *unpacked = (INDEXED_void_snd_hwdep_dsp_status_t___GENPT___const_snd_hwdep_dsp_status_t___GENPT__ *)packed;
            ARGS_void_snd_hwdep_dsp_status_t___GENPT___const_snd_hwdep_dsp_status_t___GENPT__ args = unpacked->args;
            snd_hwdep_dsp_status_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_status_free
        case snd_hwdep_dsp_status_free_INDEX: {
            INDEXED_void_snd_hwdep_dsp_status_t___GENPT__ *unpacked = (INDEXED_void_snd_hwdep_dsp_status_t___GENPT__ *)packed;
            ARGS_void_snd_hwdep_dsp_status_t___GENPT__ args = unpacked->args;
            snd_hwdep_dsp_status_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_status_get_chip_ready
        case snd_hwdep_dsp_status_get_chip_ready_INDEX: {
            INDEXED_unsigned_int_const_snd_hwdep_dsp_status_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_hwdep_dsp_status_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_hwdep_dsp_status_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_hwdep_dsp_status_get_chip_ready(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_status_get_dsp_loaded
        case snd_hwdep_dsp_status_get_dsp_loaded_INDEX: {
            INDEXED_unsigned_int_const_snd_hwdep_dsp_status_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_hwdep_dsp_status_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_hwdep_dsp_status_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_hwdep_dsp_status_get_dsp_loaded(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_status_get_id
        case snd_hwdep_dsp_status_get_id_INDEX: {
            INDEXED_const_char___GENPT___const_snd_hwdep_dsp_status_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_hwdep_dsp_status_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_hwdep_dsp_status_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_hwdep_dsp_status_get_id(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_status_get_num_dsps
        case snd_hwdep_dsp_status_get_num_dsps_INDEX: {
            INDEXED_unsigned_int_const_snd_hwdep_dsp_status_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_hwdep_dsp_status_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_hwdep_dsp_status_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_hwdep_dsp_status_get_num_dsps(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_status_get_version
        case snd_hwdep_dsp_status_get_version_INDEX: {
            INDEXED_unsigned_int_const_snd_hwdep_dsp_status_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_hwdep_dsp_status_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_hwdep_dsp_status_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_hwdep_dsp_status_get_version(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_status_malloc
        case snd_hwdep_dsp_status_malloc_INDEX: {
            INDEXED_int_snd_hwdep_dsp_status_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_hwdep_dsp_status_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_hwdep_dsp_status_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hwdep_dsp_status_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_dsp_status_sizeof
        case snd_hwdep_dsp_status_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_hwdep_dsp_status_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_info
        case snd_hwdep_info_INDEX: {
            INDEXED_int_snd_hwdep_t___GENPT___snd_hwdep_info_t___GENPT__ *unpacked = (INDEXED_int_snd_hwdep_t___GENPT___snd_hwdep_info_t___GENPT__ *)packed;
            ARGS_int_snd_hwdep_t___GENPT___snd_hwdep_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hwdep_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_info_sizeof
        case snd_hwdep_info_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_hwdep_info_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_ioctl
        case snd_hwdep_ioctl_INDEX: {
            INDEXED_int_snd_hwdep_t___GENPT___unsigned_int_void___GENPT__ *unpacked = (INDEXED_int_snd_hwdep_t___GENPT___unsigned_int_void___GENPT__ *)packed;
            ARGS_int_snd_hwdep_t___GENPT___unsigned_int_void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hwdep_ioctl(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_nonblock
        case snd_hwdep_nonblock_INDEX: {
            INDEXED_int_snd_hwdep_t___GENPT___int *unpacked = (INDEXED_int_snd_hwdep_t___GENPT___int *)packed;
            ARGS_int_snd_hwdep_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hwdep_nonblock(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_open
        case snd_hwdep_open_INDEX: {
            INDEXED_int_snd_hwdep_t___GENPT____GENPT___const_char___GENPT___int *unpacked = (INDEXED_int_snd_hwdep_t___GENPT____GENPT___const_char___GENPT___int *)packed;
            ARGS_int_snd_hwdep_t___GENPT____GENPT___const_char___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hwdep_open(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_poll_descriptors
        case snd_hwdep_poll_descriptors_INDEX: {
            INDEXED_int_snd_hwdep_t___GENPT___struct_pollfd___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_hwdep_t___GENPT___struct_pollfd___GENPT___unsigned_int *)packed;
            ARGS_int_snd_hwdep_t___GENPT___struct_pollfd___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hwdep_poll_descriptors(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_poll_descriptors_revents
        case snd_hwdep_poll_descriptors_revents_INDEX: {
            INDEXED_int_snd_hwdep_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *unpacked = (INDEXED_int_snd_hwdep_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *)packed;
            ARGS_int_snd_hwdep_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_hwdep_poll_descriptors_revents(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_read
        case snd_hwdep_read_INDEX: {
            INDEXED_ssize_t_snd_hwdep_t___GENPT___void___GENPT___size_t *unpacked = (INDEXED_ssize_t_snd_hwdep_t___GENPT___void___GENPT___size_t *)packed;
            ARGS_ssize_t_snd_hwdep_t___GENPT___void___GENPT___size_t args = unpacked->args;
            ssize_t *ret = (ssize_t *)ret_v;
            *ret =
            snd_hwdep_read(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_hwdep_write
        case snd_hwdep_write_INDEX: {
            INDEXED_ssize_t_snd_hwdep_t___GENPT___const_void___GENPT___size_t *unpacked = (INDEXED_ssize_t_snd_hwdep_t___GENPT___const_void___GENPT___size_t *)packed;
            ARGS_ssize_t_snd_hwdep_t___GENPT___const_void___GENPT___size_t args = unpacked->args;
            ssize_t *ret = (ssize_t *)ret_v;
            *ret =
            snd_hwdep_write(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_input_buffer_open
        case snd_input_buffer_open_INDEX: {
            INDEXED_int_snd_input_t___GENPT____GENPT___const_char___GENPT___ssize_t *unpacked = (INDEXED_int_snd_input_t___GENPT____GENPT___const_char___GENPT___ssize_t *)packed;
            ARGS_int_snd_input_t___GENPT____GENPT___const_char___GENPT___ssize_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_input_buffer_open(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_input_close
        case snd_input_close_INDEX: {
            INDEXED_int_snd_input_t___GENPT__ *unpacked = (INDEXED_int_snd_input_t___GENPT__ *)packed;
            ARGS_int_snd_input_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_input_close(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_input_getc
        case snd_input_getc_INDEX: {
            INDEXED_int_snd_input_t___GENPT__ *unpacked = (INDEXED_int_snd_input_t___GENPT__ *)packed;
            ARGS_int_snd_input_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_input_getc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_input_gets
        case snd_input_gets_INDEX: {
            INDEXED_char___GENPT___snd_input_t___GENPT___char___GENPT___size_t *unpacked = (INDEXED_char___GENPT___snd_input_t___GENPT___char___GENPT___size_t *)packed;
            ARGS_char___GENPT___snd_input_t___GENPT___char___GENPT___size_t args = unpacked->args;
            char * *ret = (char * *)ret_v;
            *ret =
            snd_input_gets(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_input_stdio_attach
        case snd_input_stdio_attach_INDEX: {
            INDEXED_int_snd_input_t___GENPT____GENPT___FILE___GENPT___int *unpacked = (INDEXED_int_snd_input_t___GENPT____GENPT___FILE___GENPT___int *)packed;
            ARGS_int_snd_input_t___GENPT____GENPT___FILE___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_input_stdio_attach(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_input_stdio_open
        case snd_input_stdio_open_INDEX: {
            INDEXED_int_snd_input_t___GENPT____GENPT___const_char___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_input_t___GENPT____GENPT___const_char___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_input_t___GENPT____GENPT___const_char___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_input_stdio_open(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_input_ungetc
        case snd_input_ungetc_INDEX: {
            INDEXED_int_snd_input_t___GENPT___int *unpacked = (INDEXED_int_snd_input_t___GENPT___int *)packed;
            ARGS_int_snd_input_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_input_ungetc(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_lib_error_set_handler
        case snd_lib_error_set_handler_INDEX: {
            INDEXED_int_snd_lib_error_handler_t *unpacked = (INDEXED_int_snd_lib_error_handler_t *)packed;
            ARGS_int_snd_lib_error_handler_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_lib_error_set_handler(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_midi_event_decode
        case snd_midi_event_decode_INDEX: {
            INDEXED_long_snd_midi_event_t___GENPT___unsigned_char___GENPT___long_const_snd_seq_event_t___GENPT__ *unpacked = (INDEXED_long_snd_midi_event_t___GENPT___unsigned_char___GENPT___long_const_snd_seq_event_t___GENPT__ *)packed;
            ARGS_long_snd_midi_event_t___GENPT___unsigned_char___GENPT___long_const_snd_seq_event_t___GENPT__ args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_midi_event_decode(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_midi_event_encode
        case snd_midi_event_encode_INDEX: {
            INDEXED_long_snd_midi_event_t___GENPT___const_unsigned_char___GENPT___long_snd_seq_event_t___GENPT__ *unpacked = (INDEXED_long_snd_midi_event_t___GENPT___const_unsigned_char___GENPT___long_snd_seq_event_t___GENPT__ *)packed;
            ARGS_long_snd_midi_event_t___GENPT___const_unsigned_char___GENPT___long_snd_seq_event_t___GENPT__ args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_midi_event_encode(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_midi_event_encode_byte
        case snd_midi_event_encode_byte_INDEX: {
            INDEXED_int_snd_midi_event_t___GENPT___int_snd_seq_event_t___GENPT__ *unpacked = (INDEXED_int_snd_midi_event_t___GENPT___int_snd_seq_event_t___GENPT__ *)packed;
            ARGS_int_snd_midi_event_t___GENPT___int_snd_seq_event_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_midi_event_encode_byte(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_midi_event_free
        case snd_midi_event_free_INDEX: {
            INDEXED_void_snd_midi_event_t___GENPT__ *unpacked = (INDEXED_void_snd_midi_event_t___GENPT__ *)packed;
            ARGS_void_snd_midi_event_t___GENPT__ args = unpacked->args;
            snd_midi_event_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_midi_event_init
        case snd_midi_event_init_INDEX: {
            INDEXED_void_snd_midi_event_t___GENPT__ *unpacked = (INDEXED_void_snd_midi_event_t___GENPT__ *)packed;
            ARGS_void_snd_midi_event_t___GENPT__ args = unpacked->args;
            snd_midi_event_init(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_midi_event_new
        case snd_midi_event_new_INDEX: {
            INDEXED_int_size_t_snd_midi_event_t___GENPT____GENPT__ *unpacked = (INDEXED_int_size_t_snd_midi_event_t___GENPT____GENPT__ *)packed;
            ARGS_int_size_t_snd_midi_event_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_midi_event_new(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_midi_event_no_status
        case snd_midi_event_no_status_INDEX: {
            INDEXED_void_snd_midi_event_t___GENPT___int *unpacked = (INDEXED_void_snd_midi_event_t___GENPT___int *)packed;
            ARGS_void_snd_midi_event_t___GENPT___int args = unpacked->args;
            snd_midi_event_no_status(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_midi_event_reset_decode
        case snd_midi_event_reset_decode_INDEX: {
            INDEXED_void_snd_midi_event_t___GENPT__ *unpacked = (INDEXED_void_snd_midi_event_t___GENPT__ *)packed;
            ARGS_void_snd_midi_event_t___GENPT__ args = unpacked->args;
            snd_midi_event_reset_decode(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_midi_event_reset_encode
        case snd_midi_event_reset_encode_INDEX: {
            INDEXED_void_snd_midi_event_t___GENPT__ *unpacked = (INDEXED_void_snd_midi_event_t___GENPT__ *)packed;
            ARGS_void_snd_midi_event_t___GENPT__ args = unpacked->args;
            snd_midi_event_reset_encode(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_midi_event_resize_buffer
        case snd_midi_event_resize_buffer_INDEX: {
            INDEXED_int_snd_midi_event_t___GENPT___size_t *unpacked = (INDEXED_int_snd_midi_event_t___GENPT___size_t *)packed;
            ARGS_int_snd_midi_event_t___GENPT___size_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_midi_event_resize_buffer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_attach
        case snd_mixer_attach_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_mixer_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_mixer_t___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_attach(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_attach_hctl
        case snd_mixer_attach_hctl_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT___snd_hctl_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_t___GENPT___snd_hctl_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_t___GENPT___snd_hctl_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_attach_hctl(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_class_copy
        case snd_mixer_class_copy_INDEX: {
            INDEXED_void_snd_mixer_class_t___GENPT___const_snd_mixer_class_t___GENPT__ *unpacked = (INDEXED_void_snd_mixer_class_t___GENPT___const_snd_mixer_class_t___GENPT__ *)packed;
            ARGS_void_snd_mixer_class_t___GENPT___const_snd_mixer_class_t___GENPT__ args = unpacked->args;
            snd_mixer_class_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_class_free
        case snd_mixer_class_free_INDEX: {
            INDEXED_void_snd_mixer_class_t___GENPT__ *unpacked = (INDEXED_void_snd_mixer_class_t___GENPT__ *)packed;
            ARGS_void_snd_mixer_class_t___GENPT__ args = unpacked->args;
            snd_mixer_class_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_class_get_compare
        case snd_mixer_class_get_compare_INDEX: {
            INDEXED_snd_mixer_compare_t_const_snd_mixer_class_t___GENPT__ *unpacked = (INDEXED_snd_mixer_compare_t_const_snd_mixer_class_t___GENPT__ *)packed;
            ARGS_snd_mixer_compare_t_const_snd_mixer_class_t___GENPT__ args = unpacked->args;
            snd_mixer_compare_t *ret = (snd_mixer_compare_t *)ret_v;
            *ret =
            snd_mixer_class_get_compare(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_class_get_event
        case snd_mixer_class_get_event_INDEX: {
            INDEXED_snd_mixer_event_t_const_snd_mixer_class_t___GENPT__ *unpacked = (INDEXED_snd_mixer_event_t_const_snd_mixer_class_t___GENPT__ *)packed;
            ARGS_snd_mixer_event_t_const_snd_mixer_class_t___GENPT__ args = unpacked->args;
            snd_mixer_event_t *ret = (snd_mixer_event_t *)ret_v;
            *ret =
            snd_mixer_class_get_event(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_class_get_mixer
        case snd_mixer_class_get_mixer_INDEX: {
            INDEXED_snd_mixer_t___GENPT___const_snd_mixer_class_t___GENPT__ *unpacked = (INDEXED_snd_mixer_t___GENPT___const_snd_mixer_class_t___GENPT__ *)packed;
            ARGS_snd_mixer_t___GENPT___const_snd_mixer_class_t___GENPT__ args = unpacked->args;
            snd_mixer_t * *ret = (snd_mixer_t * *)ret_v;
            *ret =
            snd_mixer_class_get_mixer(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_class_get_private
        case snd_mixer_class_get_private_INDEX: {
            INDEXED_void___GENPT___const_snd_mixer_class_t___GENPT__ *unpacked = (INDEXED_void___GENPT___const_snd_mixer_class_t___GENPT__ *)packed;
            ARGS_void___GENPT___const_snd_mixer_class_t___GENPT__ args = unpacked->args;
            void * *ret = (void * *)ret_v;
            *ret =
            snd_mixer_class_get_private(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_class_malloc
        case snd_mixer_class_malloc_INDEX: {
            INDEXED_int_snd_mixer_class_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_mixer_class_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_mixer_class_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_class_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_class_register
        case snd_mixer_class_register_INDEX: {
            INDEXED_int_snd_mixer_class_t___GENPT___snd_mixer_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_class_t___GENPT___snd_mixer_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_class_t___GENPT___snd_mixer_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_class_register(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_class_set_compare
        case snd_mixer_class_set_compare_INDEX: {
            INDEXED_int_snd_mixer_class_t___GENPT___snd_mixer_compare_t *unpacked = (INDEXED_int_snd_mixer_class_t___GENPT___snd_mixer_compare_t *)packed;
            ARGS_int_snd_mixer_class_t___GENPT___snd_mixer_compare_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_class_set_compare(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_class_set_event
        case snd_mixer_class_set_event_INDEX: {
            INDEXED_int_snd_mixer_class_t___GENPT___snd_mixer_event_t *unpacked = (INDEXED_int_snd_mixer_class_t___GENPT___snd_mixer_event_t *)packed;
            ARGS_int_snd_mixer_class_t___GENPT___snd_mixer_event_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_class_set_event(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_class_set_private
        case snd_mixer_class_set_private_INDEX: {
            INDEXED_int_snd_mixer_class_t___GENPT___void___GENPT__ *unpacked = (INDEXED_int_snd_mixer_class_t___GENPT___void___GENPT__ *)packed;
            ARGS_int_snd_mixer_class_t___GENPT___void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_class_set_private(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_class_sizeof
        case snd_mixer_class_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_mixer_class_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_class_unregister
        case snd_mixer_class_unregister_INDEX: {
            INDEXED_int_snd_mixer_class_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_class_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_class_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_class_unregister(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_close
        case snd_mixer_close_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_close(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_detach
        case snd_mixer_detach_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_mixer_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_mixer_t___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_detach(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_detach_hctl
        case snd_mixer_detach_hctl_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT___snd_hctl_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_t___GENPT___snd_hctl_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_t___GENPT___snd_hctl_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_detach_hctl(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_add
        case snd_mixer_elem_add_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_class_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_class_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_class_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_elem_add(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_attach
        case snd_mixer_elem_attach_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_hctl_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_hctl_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_hctl_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_elem_attach(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_detach
        case snd_mixer_elem_detach_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_hctl_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_hctl_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_hctl_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_elem_detach(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_empty
        case snd_mixer_elem_empty_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_elem_empty(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_free
        case snd_mixer_elem_free_INDEX: {
            INDEXED_void_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_void_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_void_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            snd_mixer_elem_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_get_callback_private
        case snd_mixer_elem_get_callback_private_INDEX: {
            INDEXED_void___GENPT___const_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_void___GENPT___const_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_void___GENPT___const_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            void * *ret = (void * *)ret_v;
            *ret =
            snd_mixer_elem_get_callback_private(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_get_private
        case snd_mixer_elem_get_private_INDEX: {
            INDEXED_void___GENPT___const_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_void___GENPT___const_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_void___GENPT___const_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            void * *ret = (void * *)ret_v;
            *ret =
            snd_mixer_elem_get_private(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_get_type
        case snd_mixer_elem_get_type_INDEX: {
            INDEXED_snd_mixer_elem_type_t_const_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_snd_mixer_elem_type_t_const_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_snd_mixer_elem_type_t_const_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            snd_mixer_elem_type_t *ret = (snd_mixer_elem_type_t *)ret_v;
            *ret =
            snd_mixer_elem_get_type(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_info
        case snd_mixer_elem_info_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_elem_info(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_next
        case snd_mixer_elem_next_INDEX: {
            INDEXED_snd_mixer_elem_t___GENPT___snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_snd_mixer_elem_t___GENPT___snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_snd_mixer_elem_t___GENPT___snd_mixer_elem_t___GENPT__ args = unpacked->args;
            snd_mixer_elem_t * *ret = (snd_mixer_elem_t * *)ret_v;
            *ret =
            snd_mixer_elem_next(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_prev
        case snd_mixer_elem_prev_INDEX: {
            INDEXED_snd_mixer_elem_t___GENPT___snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_snd_mixer_elem_t___GENPT___snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_snd_mixer_elem_t___GENPT___snd_mixer_elem_t___GENPT__ args = unpacked->args;
            snd_mixer_elem_t * *ret = (snd_mixer_elem_t * *)ret_v;
            *ret =
            snd_mixer_elem_prev(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_remove
        case snd_mixer_elem_remove_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_elem_remove(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_set_callback
        case snd_mixer_elem_set_callback_INDEX: {
            INDEXED_void_snd_mixer_elem_t___GENPT___snd_mixer_elem_callback_t *unpacked = (INDEXED_void_snd_mixer_elem_t___GENPT___snd_mixer_elem_callback_t *)packed;
            ARGS_void_snd_mixer_elem_t___GENPT___snd_mixer_elem_callback_t args = unpacked->args;
            snd_mixer_elem_set_callback(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_set_callback_private
        case snd_mixer_elem_set_callback_private_INDEX: {
            INDEXED_void_snd_mixer_elem_t___GENPT___void___GENPT__ *unpacked = (INDEXED_void_snd_mixer_elem_t___GENPT___void___GENPT__ *)packed;
            ARGS_void_snd_mixer_elem_t___GENPT___void___GENPT__ args = unpacked->args;
            snd_mixer_elem_set_callback_private(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_elem_value
        case snd_mixer_elem_value_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_elem_value(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_find_selem
        case snd_mixer_find_selem_INDEX: {
            INDEXED_snd_mixer_elem_t___GENPT___snd_mixer_t___GENPT___const_snd_mixer_selem_id_t___GENPT__ *unpacked = (INDEXED_snd_mixer_elem_t___GENPT___snd_mixer_t___GENPT___const_snd_mixer_selem_id_t___GENPT__ *)packed;
            ARGS_snd_mixer_elem_t___GENPT___snd_mixer_t___GENPT___const_snd_mixer_selem_id_t___GENPT__ args = unpacked->args;
            snd_mixer_elem_t * *ret = (snd_mixer_elem_t * *)ret_v;
            *ret =
            snd_mixer_find_selem(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_first_elem
        case snd_mixer_first_elem_INDEX: {
            INDEXED_snd_mixer_elem_t___GENPT___snd_mixer_t___GENPT__ *unpacked = (INDEXED_snd_mixer_elem_t___GENPT___snd_mixer_t___GENPT__ *)packed;
            ARGS_snd_mixer_elem_t___GENPT___snd_mixer_t___GENPT__ args = unpacked->args;
            snd_mixer_elem_t * *ret = (snd_mixer_elem_t * *)ret_v;
            *ret =
            snd_mixer_first_elem(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_free
        case snd_mixer_free_INDEX: {
            INDEXED_void_snd_mixer_t___GENPT__ *unpacked = (INDEXED_void_snd_mixer_t___GENPT__ *)packed;
            ARGS_void_snd_mixer_t___GENPT__ args = unpacked->args;
            snd_mixer_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_get_callback_private
        case snd_mixer_get_callback_private_INDEX: {
            INDEXED_void___GENPT___const_snd_mixer_t___GENPT__ *unpacked = (INDEXED_void___GENPT___const_snd_mixer_t___GENPT__ *)packed;
            ARGS_void___GENPT___const_snd_mixer_t___GENPT__ args = unpacked->args;
            void * *ret = (void * *)ret_v;
            *ret =
            snd_mixer_get_callback_private(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_get_count
        case snd_mixer_get_count_INDEX: {
            INDEXED_unsigned_int_const_snd_mixer_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_mixer_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_mixer_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_mixer_get_count(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_get_hctl
        case snd_mixer_get_hctl_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT___const_char___GENPT___snd_hctl_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_mixer_t___GENPT___const_char___GENPT___snd_hctl_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_mixer_t___GENPT___const_char___GENPT___snd_hctl_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_get_hctl(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_handle_events
        case snd_mixer_handle_events_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_handle_events(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_last_elem
        case snd_mixer_last_elem_INDEX: {
            INDEXED_snd_mixer_elem_t___GENPT___snd_mixer_t___GENPT__ *unpacked = (INDEXED_snd_mixer_elem_t___GENPT___snd_mixer_t___GENPT__ *)packed;
            ARGS_snd_mixer_elem_t___GENPT___snd_mixer_t___GENPT__ args = unpacked->args;
            snd_mixer_elem_t * *ret = (snd_mixer_elem_t * *)ret_v;
            *ret =
            snd_mixer_last_elem(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_load
        case snd_mixer_load_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_load(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_open
        case snd_mixer_open_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT____GENPT___int *unpacked = (INDEXED_int_snd_mixer_t___GENPT____GENPT___int *)packed;
            ARGS_int_snd_mixer_t___GENPT____GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_open(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_poll_descriptors
        case snd_mixer_poll_descriptors_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT___struct_pollfd___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_mixer_t___GENPT___struct_pollfd___GENPT___unsigned_int *)packed;
            ARGS_int_snd_mixer_t___GENPT___struct_pollfd___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_poll_descriptors(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_poll_descriptors_count
        case snd_mixer_poll_descriptors_count_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_poll_descriptors_count(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_poll_descriptors_revents
        case snd_mixer_poll_descriptors_revents_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *unpacked = (INDEXED_int_snd_mixer_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *)packed;
            ARGS_int_snd_mixer_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_poll_descriptors_revents(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_channel_name
        case snd_mixer_selem_channel_name_INDEX: {
            INDEXED_const_char___GENPT___snd_mixer_selem_channel_id_t *unpacked = (INDEXED_const_char___GENPT___snd_mixer_selem_channel_id_t *)packed;
            ARGS_const_char___GENPT___snd_mixer_selem_channel_id_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_mixer_selem_channel_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_capture_dB
        case snd_mixer_selem_get_capture_dB_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_capture_dB(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_capture_dB_range
        case snd_mixer_selem_get_capture_dB_range_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___long___GENPT___long___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___long___GENPT___long___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___long___GENPT___long___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_capture_dB_range(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_capture_group
        case snd_mixer_selem_get_capture_group_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_capture_group(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_capture_switch
        case snd_mixer_selem_get_capture_switch_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_int___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_int___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_capture_switch(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_capture_volume
        case snd_mixer_selem_get_capture_volume_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_capture_volume(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_capture_volume_range
        case snd_mixer_selem_get_capture_volume_range_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___long___GENPT___long___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___long___GENPT___long___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___long___GENPT___long___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_capture_volume_range(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_enum_item
        case snd_mixer_selem_get_enum_item_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_enum_item(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_enum_item_name
        case snd_mixer_selem_get_enum_item_name_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___unsigned_int_size_t_char___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___unsigned_int_size_t_char___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___unsigned_int_size_t_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_enum_item_name(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_enum_items
        case snd_mixer_selem_get_enum_items_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_enum_items(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_id
        case snd_mixer_selem_get_id_INDEX: {
            INDEXED_void_snd_mixer_elem_t___GENPT___snd_mixer_selem_id_t___GENPT__ *unpacked = (INDEXED_void_snd_mixer_elem_t___GENPT___snd_mixer_selem_id_t___GENPT__ *)packed;
            ARGS_void_snd_mixer_elem_t___GENPT___snd_mixer_selem_id_t___GENPT__ args = unpacked->args;
            snd_mixer_selem_get_id(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_index
        case snd_mixer_selem_get_index_INDEX: {
            INDEXED_unsigned_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_unsigned_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_unsigned_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_mixer_selem_get_index(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_name
        case snd_mixer_selem_get_name_INDEX: {
            INDEXED_const_char___GENPT___snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___snd_mixer_elem_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_mixer_selem_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_playback_dB
        case snd_mixer_selem_get_playback_dB_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_playback_dB(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_playback_dB_range
        case snd_mixer_selem_get_playback_dB_range_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___long___GENPT___long___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___long___GENPT___long___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___long___GENPT___long___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_playback_dB_range(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_playback_switch
        case snd_mixer_selem_get_playback_switch_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_int___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_int___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_playback_switch(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_playback_volume
        case snd_mixer_selem_get_playback_volume_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_playback_volume(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_get_playback_volume_range
        case snd_mixer_selem_get_playback_volume_range_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___long___GENPT___long___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___long___GENPT___long___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___long___GENPT___long___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_get_playback_volume_range(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_has_capture_channel
        case snd_mixer_selem_has_capture_channel_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_has_capture_channel(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_has_capture_switch
        case snd_mixer_selem_has_capture_switch_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_has_capture_switch(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_has_capture_switch_exclusive
        case snd_mixer_selem_has_capture_switch_exclusive_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_has_capture_switch_exclusive(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_has_capture_switch_joined
        case snd_mixer_selem_has_capture_switch_joined_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_has_capture_switch_joined(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_has_capture_volume
        case snd_mixer_selem_has_capture_volume_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_has_capture_volume(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_has_capture_volume_joined
        case snd_mixer_selem_has_capture_volume_joined_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_has_capture_volume_joined(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_has_common_switch
        case snd_mixer_selem_has_common_switch_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_has_common_switch(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_has_common_volume
        case snd_mixer_selem_has_common_volume_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_has_common_volume(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_has_playback_channel
        case snd_mixer_selem_has_playback_channel_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_has_playback_channel(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_has_playback_switch
        case snd_mixer_selem_has_playback_switch_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_has_playback_switch(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_has_playback_switch_joined
        case snd_mixer_selem_has_playback_switch_joined_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_has_playback_switch_joined(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_has_playback_volume
        case snd_mixer_selem_has_playback_volume_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_has_playback_volume(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_has_playback_volume_joined
        case snd_mixer_selem_has_playback_volume_joined_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_has_playback_volume_joined(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_id_copy
        case snd_mixer_selem_id_copy_INDEX: {
            INDEXED_void_snd_mixer_selem_id_t___GENPT___const_snd_mixer_selem_id_t___GENPT__ *unpacked = (INDEXED_void_snd_mixer_selem_id_t___GENPT___const_snd_mixer_selem_id_t___GENPT__ *)packed;
            ARGS_void_snd_mixer_selem_id_t___GENPT___const_snd_mixer_selem_id_t___GENPT__ args = unpacked->args;
            snd_mixer_selem_id_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_id_free
        case snd_mixer_selem_id_free_INDEX: {
            INDEXED_void_snd_mixer_selem_id_t___GENPT__ *unpacked = (INDEXED_void_snd_mixer_selem_id_t___GENPT__ *)packed;
            ARGS_void_snd_mixer_selem_id_t___GENPT__ args = unpacked->args;
            snd_mixer_selem_id_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_id_get_index
        case snd_mixer_selem_id_get_index_INDEX: {
            INDEXED_unsigned_int_const_snd_mixer_selem_id_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_mixer_selem_id_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_mixer_selem_id_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_mixer_selem_id_get_index(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_id_get_name
        case snd_mixer_selem_id_get_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_mixer_selem_id_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_mixer_selem_id_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_mixer_selem_id_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_mixer_selem_id_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_id_malloc
        case snd_mixer_selem_id_malloc_INDEX: {
            INDEXED_int_snd_mixer_selem_id_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_mixer_selem_id_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_mixer_selem_id_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_id_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_id_set_index
        case snd_mixer_selem_id_set_index_INDEX: {
            INDEXED_void_snd_mixer_selem_id_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_mixer_selem_id_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_mixer_selem_id_t___GENPT___unsigned_int args = unpacked->args;
            snd_mixer_selem_id_set_index(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_id_set_name
        case snd_mixer_selem_id_set_name_INDEX: {
            INDEXED_void_snd_mixer_selem_id_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_void_snd_mixer_selem_id_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_void_snd_mixer_selem_id_t___GENPT___const_char___GENPT__ args = unpacked->args;
            snd_mixer_selem_id_set_name(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_id_sizeof
        case snd_mixer_selem_id_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_mixer_selem_id_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_is_active
        case snd_mixer_selem_is_active_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_is_active(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_is_capture_mono
        case snd_mixer_selem_is_capture_mono_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_is_capture_mono(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_is_enum_capture
        case snd_mixer_selem_is_enum_capture_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_is_enum_capture(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_is_enum_playback
        case snd_mixer_selem_is_enum_playback_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_is_enum_playback(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_is_enumerated
        case snd_mixer_selem_is_enumerated_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_is_enumerated(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_is_playback_mono
        case snd_mixer_selem_is_playback_mono_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT__ *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT__ *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_is_playback_mono(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_register
        case snd_mixer_selem_register_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT___struct_snd_mixer_selem_regopt___GENPT___snd_mixer_class_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_mixer_t___GENPT___struct_snd_mixer_selem_regopt___GENPT___snd_mixer_class_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_mixer_t___GENPT___struct_snd_mixer_selem_regopt___GENPT___snd_mixer_class_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_register(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_capture_dB
        case snd_mixer_selem_set_capture_dB_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long_int *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long_int *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_capture_dB(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_capture_dB_all
        case snd_mixer_selem_set_capture_dB_all_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___long_int *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___long_int *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___long_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_capture_dB_all(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_capture_switch
        case snd_mixer_selem_set_capture_switch_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_int *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_int *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_capture_switch(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_capture_switch_all
        case snd_mixer_selem_set_capture_switch_all_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___int *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___int *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_capture_switch_all(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_capture_volume
        case snd_mixer_selem_set_capture_volume_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_capture_volume(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_capture_volume_all
        case snd_mixer_selem_set_capture_volume_all_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___long *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___long *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___long args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_capture_volume_all(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_capture_volume_range
        case snd_mixer_selem_set_capture_volume_range_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___long_long *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___long_long *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___long_long args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_capture_volume_range(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_enum_item
        case snd_mixer_selem_set_enum_item_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_unsigned_int *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_unsigned_int *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_enum_item(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_playback_dB
        case snd_mixer_selem_set_playback_dB_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long_int *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long_int *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_playback_dB(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_playback_dB_all
        case snd_mixer_selem_set_playback_dB_all_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___long_int *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___long_int *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___long_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_playback_dB_all(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_playback_switch
        case snd_mixer_selem_set_playback_switch_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_int *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_int *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_playback_switch(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_playback_switch_all
        case snd_mixer_selem_set_playback_switch_all_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___int *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___int *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_playback_switch_all(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_playback_volume
        case snd_mixer_selem_set_playback_volume_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___snd_mixer_selem_channel_id_t_long args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_playback_volume(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_playback_volume_all
        case snd_mixer_selem_set_playback_volume_all_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___long *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___long *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___long args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_playback_volume_all(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_selem_set_playback_volume_range
        case snd_mixer_selem_set_playback_volume_range_INDEX: {
            INDEXED_int_snd_mixer_elem_t___GENPT___long_long *unpacked = (INDEXED_int_snd_mixer_elem_t___GENPT___long_long *)packed;
            ARGS_int_snd_mixer_elem_t___GENPT___long_long args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_selem_set_playback_volume_range(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_set_callback
        case snd_mixer_set_callback_INDEX: {
            INDEXED_void_snd_mixer_t___GENPT___snd_mixer_callback_t *unpacked = (INDEXED_void_snd_mixer_t___GENPT___snd_mixer_callback_t *)packed;
            ARGS_void_snd_mixer_t___GENPT___snd_mixer_callback_t args = unpacked->args;
            snd_mixer_set_callback(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_set_callback_private
        case snd_mixer_set_callback_private_INDEX: {
            INDEXED_void_snd_mixer_t___GENPT___void___GENPT__ *unpacked = (INDEXED_void_snd_mixer_t___GENPT___void___GENPT__ *)packed;
            ARGS_void_snd_mixer_t___GENPT___void___GENPT__ args = unpacked->args;
            snd_mixer_set_callback_private(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_set_compare
        case snd_mixer_set_compare_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT___snd_mixer_compare_t *unpacked = (INDEXED_int_snd_mixer_t___GENPT___snd_mixer_compare_t *)packed;
            ARGS_int_snd_mixer_t___GENPT___snd_mixer_compare_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_set_compare(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_mixer_wait
        case snd_mixer_wait_INDEX: {
            INDEXED_int_snd_mixer_t___GENPT___int *unpacked = (INDEXED_int_snd_mixer_t___GENPT___int *)packed;
            ARGS_int_snd_mixer_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_mixer_wait(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_names_list
        case snd_names_list_INDEX: {
            INDEXED_int_const_char___GENPT___snd_devname_t___GENPT____GENPT__ *unpacked = (INDEXED_int_const_char___GENPT___snd_devname_t___GENPT____GENPT__ *)packed;
            ARGS_int_const_char___GENPT___snd_devname_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_names_list(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_names_list_free
        case snd_names_list_free_INDEX: {
            INDEXED_void_snd_devname_t___GENPT__ *unpacked = (INDEXED_void_snd_devname_t___GENPT__ *)packed;
            ARGS_void_snd_devname_t___GENPT__ args = unpacked->args;
            snd_names_list_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_output_buffer_open
        case snd_output_buffer_open_INDEX: {
            INDEXED_int_snd_output_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_output_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_output_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_output_buffer_open(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_output_buffer_string
        case snd_output_buffer_string_INDEX: {
            INDEXED_size_t_snd_output_t___GENPT___char___GENPT____GENPT__ *unpacked = (INDEXED_size_t_snd_output_t___GENPT___char___GENPT____GENPT__ *)packed;
            ARGS_size_t_snd_output_t___GENPT___char___GENPT____GENPT__ args = unpacked->args;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_output_buffer_string(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_output_close
        case snd_output_close_INDEX: {
            INDEXED_int_snd_output_t___GENPT__ *unpacked = (INDEXED_int_snd_output_t___GENPT__ *)packed;
            ARGS_int_snd_output_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_output_close(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_output_flush
        case snd_output_flush_INDEX: {
            INDEXED_int_snd_output_t___GENPT__ *unpacked = (INDEXED_int_snd_output_t___GENPT__ *)packed;
            ARGS_int_snd_output_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_output_flush(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_output_putc
        case snd_output_putc_INDEX: {
            INDEXED_int_snd_output_t___GENPT___int *unpacked = (INDEXED_int_snd_output_t___GENPT___int *)packed;
            ARGS_int_snd_output_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_output_putc(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_output_puts
        case snd_output_puts_INDEX: {
            INDEXED_int_snd_output_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_output_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_output_t___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_output_puts(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_output_stdio_attach
        case snd_output_stdio_attach_INDEX: {
            INDEXED_int_snd_output_t___GENPT____GENPT___FILE___GENPT___int *unpacked = (INDEXED_int_snd_output_t___GENPT____GENPT___FILE___GENPT___int *)packed;
            ARGS_int_snd_output_t___GENPT____GENPT___FILE___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_output_stdio_attach(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_output_stdio_open
        case snd_output_stdio_open_INDEX: {
            INDEXED_int_snd_output_t___GENPT____GENPT___const_char___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_output_t___GENPT____GENPT___const_char___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_output_t___GENPT____GENPT___const_char___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_output_stdio_open(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_output_vprintf
        case snd_output_vprintf_INDEX: {
            INDEXED_int_snd_output_t___GENPT___const_char___GENPT___va_list *unpacked = (INDEXED_int_snd_output_t___GENPT___const_char___GENPT___va_list *)packed;
            ARGS_int_snd_output_t___GENPT___const_char___GENPT___va_list args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_output_vprintf(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_access_mask_any
        case snd_pcm_access_mask_any_INDEX: {
            INDEXED_void_snd_pcm_access_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_access_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_access_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_access_mask_any(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_access_mask_copy
        case snd_pcm_access_mask_copy_INDEX: {
            INDEXED_void_snd_pcm_access_mask_t___GENPT___const_snd_pcm_access_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_access_mask_t___GENPT___const_snd_pcm_access_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_access_mask_t___GENPT___const_snd_pcm_access_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_access_mask_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_access_mask_empty
        case snd_pcm_access_mask_empty_INDEX: {
            INDEXED_int_const_snd_pcm_access_mask_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_access_mask_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_access_mask_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_access_mask_empty(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_access_mask_free
        case snd_pcm_access_mask_free_INDEX: {
            INDEXED_void_snd_pcm_access_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_access_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_access_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_access_mask_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_access_mask_malloc
        case snd_pcm_access_mask_malloc_INDEX: {
            INDEXED_int_snd_pcm_access_mask_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_pcm_access_mask_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_pcm_access_mask_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_access_mask_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_access_mask_none
        case snd_pcm_access_mask_none_INDEX: {
            INDEXED_void_snd_pcm_access_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_access_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_access_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_access_mask_none(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_access_mask_reset
        case snd_pcm_access_mask_reset_INDEX: {
            INDEXED_void_snd_pcm_access_mask_t___GENPT___snd_pcm_access_t *unpacked = (INDEXED_void_snd_pcm_access_mask_t___GENPT___snd_pcm_access_t *)packed;
            ARGS_void_snd_pcm_access_mask_t___GENPT___snd_pcm_access_t args = unpacked->args;
            snd_pcm_access_mask_reset(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_access_mask_set
        case snd_pcm_access_mask_set_INDEX: {
            INDEXED_void_snd_pcm_access_mask_t___GENPT___snd_pcm_access_t *unpacked = (INDEXED_void_snd_pcm_access_mask_t___GENPT___snd_pcm_access_t *)packed;
            ARGS_void_snd_pcm_access_mask_t___GENPT___snd_pcm_access_t args = unpacked->args;
            snd_pcm_access_mask_set(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_access_mask_sizeof
        case snd_pcm_access_mask_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_pcm_access_mask_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_access_mask_test
        case snd_pcm_access_mask_test_INDEX: {
            INDEXED_int_const_snd_pcm_access_mask_t___GENPT___snd_pcm_access_t *unpacked = (INDEXED_int_const_snd_pcm_access_mask_t___GENPT___snd_pcm_access_t *)packed;
            ARGS_int_const_snd_pcm_access_mask_t___GENPT___snd_pcm_access_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_access_mask_test(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_access_name
        case snd_pcm_access_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_pcm_access_t *unpacked = (INDEXED_const_char___GENPT___const_snd_pcm_access_t *)packed;
            ARGS_const_char___GENPT___const_snd_pcm_access_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_access_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_area_copy
        case snd_pcm_area_copy_INDEX: {
            INDEXED_int_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_unsigned_int_snd_pcm_format_t *unpacked = (INDEXED_int_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_unsigned_int_snd_pcm_format_t *)packed;
            ARGS_int_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_unsigned_int_snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_area_copy(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_area_silence
        case snd_pcm_area_silence_INDEX: {
            INDEXED_int_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_unsigned_int_snd_pcm_format_t *unpacked = (INDEXED_int_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_unsigned_int_snd_pcm_format_t *)packed;
            ARGS_int_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_unsigned_int_snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_area_silence(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_areas_copy
        case snd_pcm_areas_copy_INDEX: {
            INDEXED_int_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_unsigned_int_snd_pcm_uframes_t_snd_pcm_format_t *unpacked = (INDEXED_int_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_unsigned_int_snd_pcm_uframes_t_snd_pcm_format_t *)packed;
            ARGS_int_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_unsigned_int_snd_pcm_uframes_t_snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_areas_copy(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_areas_silence
        case snd_pcm_areas_silence_INDEX: {
            INDEXED_int_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_unsigned_int_snd_pcm_uframes_t_snd_pcm_format_t *unpacked = (INDEXED_int_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_unsigned_int_snd_pcm_uframes_t_snd_pcm_format_t *)packed;
            ARGS_int_const_snd_pcm_channel_area_t___GENPT___snd_pcm_uframes_t_unsigned_int_snd_pcm_uframes_t_snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_areas_silence(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_avail_update
        case snd_pcm_avail_update_INDEX: {
            INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT__ *unpacked = (INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT__ *)packed;
            ARGS_snd_pcm_sframes_t_snd_pcm_t___GENPT__ args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_avail_update(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_build_linear_format
        case snd_pcm_build_linear_format_INDEX: {
            INDEXED_snd_pcm_format_t_int_int_int_int *unpacked = (INDEXED_snd_pcm_format_t_int_int_int_int *)packed;
            ARGS_snd_pcm_format_t_int_int_int_int args = unpacked->args;
            snd_pcm_format_t *ret = (snd_pcm_format_t *)ret_v;
            *ret =
            snd_pcm_build_linear_format(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_bytes_to_frames
        case snd_pcm_bytes_to_frames_INDEX: {
            INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___ssize_t *unpacked = (INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___ssize_t *)packed;
            ARGS_snd_pcm_sframes_t_snd_pcm_t___GENPT___ssize_t args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_bytes_to_frames(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_bytes_to_samples
        case snd_pcm_bytes_to_samples_INDEX: {
            INDEXED_long_snd_pcm_t___GENPT___ssize_t *unpacked = (INDEXED_long_snd_pcm_t___GENPT___ssize_t *)packed;
            ARGS_long_snd_pcm_t___GENPT___ssize_t args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_pcm_bytes_to_samples(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_close
        case snd_pcm_close_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_close(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_delay
        case snd_pcm_delay_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sframes_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sframes_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_sframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_delay(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_drain
        case snd_pcm_drain_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_drain(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_drop
        case snd_pcm_drop_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_drop(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_dump
        case snd_pcm_dump_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_output_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_output_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_output_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_dump(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_dump_hw_setup
        case snd_pcm_dump_hw_setup_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_output_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_output_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_output_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_dump_hw_setup(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_dump_setup
        case snd_pcm_dump_setup_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_output_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_output_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_output_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_dump_setup(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_dump_sw_setup
        case snd_pcm_dump_sw_setup_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_output_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_output_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_output_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_dump_sw_setup(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_big_endian
        case snd_pcm_format_big_endian_INDEX: {
            INDEXED_int_snd_pcm_format_t *unpacked = (INDEXED_int_snd_pcm_format_t *)packed;
            ARGS_int_snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_format_big_endian(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_cpu_endian
        case snd_pcm_format_cpu_endian_INDEX: {
            INDEXED_int_snd_pcm_format_t *unpacked = (INDEXED_int_snd_pcm_format_t *)packed;
            ARGS_int_snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_format_cpu_endian(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_description
        case snd_pcm_format_description_INDEX: {
            INDEXED_const_char___GENPT___const_snd_pcm_format_t *unpacked = (INDEXED_const_char___GENPT___const_snd_pcm_format_t *)packed;
            ARGS_const_char___GENPT___const_snd_pcm_format_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_format_description(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_float
        case snd_pcm_format_float_INDEX: {
            INDEXED_int_snd_pcm_format_t *unpacked = (INDEXED_int_snd_pcm_format_t *)packed;
            ARGS_int_snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_format_float(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_linear
        case snd_pcm_format_linear_INDEX: {
            INDEXED_int_snd_pcm_format_t *unpacked = (INDEXED_int_snd_pcm_format_t *)packed;
            ARGS_int_snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_format_linear(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_little_endian
        case snd_pcm_format_little_endian_INDEX: {
            INDEXED_int_snd_pcm_format_t *unpacked = (INDEXED_int_snd_pcm_format_t *)packed;
            ARGS_int_snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_format_little_endian(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_mask_any
        case snd_pcm_format_mask_any_INDEX: {
            INDEXED_void_snd_pcm_format_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_format_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_format_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_format_mask_any(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_mask_copy
        case snd_pcm_format_mask_copy_INDEX: {
            INDEXED_void_snd_pcm_format_mask_t___GENPT___const_snd_pcm_format_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_format_mask_t___GENPT___const_snd_pcm_format_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_format_mask_t___GENPT___const_snd_pcm_format_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_format_mask_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_mask_empty
        case snd_pcm_format_mask_empty_INDEX: {
            INDEXED_int_const_snd_pcm_format_mask_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_format_mask_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_format_mask_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_format_mask_empty(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_mask_free
        case snd_pcm_format_mask_free_INDEX: {
            INDEXED_void_snd_pcm_format_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_format_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_format_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_format_mask_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_mask_malloc
        case snd_pcm_format_mask_malloc_INDEX: {
            INDEXED_int_snd_pcm_format_mask_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_pcm_format_mask_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_pcm_format_mask_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_format_mask_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_mask_none
        case snd_pcm_format_mask_none_INDEX: {
            INDEXED_void_snd_pcm_format_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_format_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_format_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_format_mask_none(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_mask_reset
        case snd_pcm_format_mask_reset_INDEX: {
            INDEXED_void_snd_pcm_format_mask_t___GENPT___snd_pcm_format_t *unpacked = (INDEXED_void_snd_pcm_format_mask_t___GENPT___snd_pcm_format_t *)packed;
            ARGS_void_snd_pcm_format_mask_t___GENPT___snd_pcm_format_t args = unpacked->args;
            snd_pcm_format_mask_reset(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_mask_set
        case snd_pcm_format_mask_set_INDEX: {
            INDEXED_void_snd_pcm_format_mask_t___GENPT___snd_pcm_format_t *unpacked = (INDEXED_void_snd_pcm_format_mask_t___GENPT___snd_pcm_format_t *)packed;
            ARGS_void_snd_pcm_format_mask_t___GENPT___snd_pcm_format_t args = unpacked->args;
            snd_pcm_format_mask_set(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_mask_sizeof
        case snd_pcm_format_mask_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_pcm_format_mask_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_mask_test
        case snd_pcm_format_mask_test_INDEX: {
            INDEXED_int_const_snd_pcm_format_mask_t___GENPT___snd_pcm_format_t *unpacked = (INDEXED_int_const_snd_pcm_format_mask_t___GENPT___snd_pcm_format_t *)packed;
            ARGS_int_const_snd_pcm_format_mask_t___GENPT___snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_format_mask_test(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_name
        case snd_pcm_format_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_pcm_format_t *unpacked = (INDEXED_const_char___GENPT___const_snd_pcm_format_t *)packed;
            ARGS_const_char___GENPT___const_snd_pcm_format_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_format_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_physical_width
        case snd_pcm_format_physical_width_INDEX: {
            INDEXED_int_snd_pcm_format_t *unpacked = (INDEXED_int_snd_pcm_format_t *)packed;
            ARGS_int_snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_format_physical_width(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_set_silence
        case snd_pcm_format_set_silence_INDEX: {
            INDEXED_int_snd_pcm_format_t_void___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_pcm_format_t_void___GENPT___unsigned_int *)packed;
            ARGS_int_snd_pcm_format_t_void___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_format_set_silence(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_signed
        case snd_pcm_format_signed_INDEX: {
            INDEXED_int_snd_pcm_format_t *unpacked = (INDEXED_int_snd_pcm_format_t *)packed;
            ARGS_int_snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_format_signed(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_silence
        case snd_pcm_format_silence_INDEX: {
            INDEXED_u_int8_t_snd_pcm_format_t *unpacked = (INDEXED_u_int8_t_snd_pcm_format_t *)packed;
            ARGS_u_int8_t_snd_pcm_format_t args = unpacked->args;
            u_int8_t *ret = (u_int8_t *)ret_v;
            *ret =
            snd_pcm_format_silence(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_silence_16
        case snd_pcm_format_silence_16_INDEX: {
            INDEXED_u_int16_t_snd_pcm_format_t *unpacked = (INDEXED_u_int16_t_snd_pcm_format_t *)packed;
            ARGS_u_int16_t_snd_pcm_format_t args = unpacked->args;
            u_int16_t *ret = (u_int16_t *)ret_v;
            *ret =
            snd_pcm_format_silence_16(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_silence_32
        case snd_pcm_format_silence_32_INDEX: {
            INDEXED_u_int32_t_snd_pcm_format_t *unpacked = (INDEXED_u_int32_t_snd_pcm_format_t *)packed;
            ARGS_u_int32_t_snd_pcm_format_t args = unpacked->args;
            u_int32_t *ret = (u_int32_t *)ret_v;
            *ret =
            snd_pcm_format_silence_32(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_silence_64
        case snd_pcm_format_silence_64_INDEX: {
            INDEXED_u_int64_t_snd_pcm_format_t *unpacked = (INDEXED_u_int64_t_snd_pcm_format_t *)packed;
            ARGS_u_int64_t_snd_pcm_format_t args = unpacked->args;
            u_int64_t *ret = (u_int64_t *)ret_v;
            *ret =
            snd_pcm_format_silence_64(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_size
        case snd_pcm_format_size_INDEX: {
            INDEXED_ssize_t_snd_pcm_format_t_size_t *unpacked = (INDEXED_ssize_t_snd_pcm_format_t_size_t *)packed;
            ARGS_ssize_t_snd_pcm_format_t_size_t args = unpacked->args;
            ssize_t *ret = (ssize_t *)ret_v;
            *ret =
            snd_pcm_format_size(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_unsigned
        case snd_pcm_format_unsigned_INDEX: {
            INDEXED_int_snd_pcm_format_t *unpacked = (INDEXED_int_snd_pcm_format_t *)packed;
            ARGS_int_snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_format_unsigned(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_value
        case snd_pcm_format_value_INDEX: {
            INDEXED_snd_pcm_format_t_const_char__GENPT__ *unpacked = (INDEXED_snd_pcm_format_t_const_char__GENPT__ *)packed;
            ARGS_snd_pcm_format_t_const_char__GENPT__ args = unpacked->args;
            snd_pcm_format_t *ret = (snd_pcm_format_t *)ret_v;
            *ret =
            snd_pcm_format_value(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_format_width
        case snd_pcm_format_width_INDEX: {
            INDEXED_int_snd_pcm_format_t *unpacked = (INDEXED_int_snd_pcm_format_t *)packed;
            ARGS_int_snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_format_width(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_forward
        case snd_pcm_forward_INDEX: {
            INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_snd_pcm_sframes_t_snd_pcm_t___GENPT___snd_pcm_uframes_t args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_forward(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_frames_to_bytes
        case snd_pcm_frames_to_bytes_INDEX: {
            INDEXED_ssize_t_snd_pcm_t___GENPT___snd_pcm_sframes_t *unpacked = (INDEXED_ssize_t_snd_pcm_t___GENPT___snd_pcm_sframes_t *)packed;
            ARGS_ssize_t_snd_pcm_t___GENPT___snd_pcm_sframes_t args = unpacked->args;
            ssize_t *ret = (ssize_t *)ret_v;
            *ret =
            snd_pcm_frames_to_bytes(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_get_params
        case snd_pcm_get_params_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_uframes_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_uframes_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_uframes_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_get_params(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hook_add
        case snd_pcm_hook_add_INDEX: {
            INDEXED_int_snd_pcm_hook_t___GENPT____GENPT___snd_pcm_t___GENPT___snd_pcm_hook_type_t_snd_pcm_hook_func_t_void___GENPT__ *unpacked = (INDEXED_int_snd_pcm_hook_t___GENPT____GENPT___snd_pcm_t___GENPT___snd_pcm_hook_type_t_snd_pcm_hook_func_t_void___GENPT__ *)packed;
            ARGS_int_snd_pcm_hook_t___GENPT____GENPT___snd_pcm_t___GENPT___snd_pcm_hook_type_t_snd_pcm_hook_func_t_void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hook_add(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hook_get_pcm
        case snd_pcm_hook_get_pcm_INDEX: {
            INDEXED_snd_pcm_t___GENPT___snd_pcm_hook_t___GENPT__ *unpacked = (INDEXED_snd_pcm_t___GENPT___snd_pcm_hook_t___GENPT__ *)packed;
            ARGS_snd_pcm_t___GENPT___snd_pcm_hook_t___GENPT__ args = unpacked->args;
            snd_pcm_t * *ret = (snd_pcm_t * *)ret_v;
            *ret =
            snd_pcm_hook_get_pcm(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hook_get_private
        case snd_pcm_hook_get_private_INDEX: {
            INDEXED_void___GENPT___snd_pcm_hook_t___GENPT__ *unpacked = (INDEXED_void___GENPT___snd_pcm_hook_t___GENPT__ *)packed;
            ARGS_void___GENPT___snd_pcm_hook_t___GENPT__ args = unpacked->args;
            void * *ret = (void * *)ret_v;
            *ret =
            snd_pcm_hook_get_private(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hook_remove
        case snd_pcm_hook_remove_INDEX: {
            INDEXED_int_snd_pcm_hook_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_hook_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_hook_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hook_remove(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hook_set_private
        case snd_pcm_hook_set_private_INDEX: {
            INDEXED_void_snd_pcm_hook_t___GENPT___void___GENPT__ *unpacked = (INDEXED_void_snd_pcm_hook_t___GENPT___void___GENPT__ *)packed;
            ARGS_void_snd_pcm_hook_t___GENPT___void___GENPT__ args = unpacked->args;
            snd_pcm_hook_set_private(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_free
        case snd_pcm_hw_free_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params
        case snd_pcm_hw_params_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_any
        case snd_pcm_hw_params_any_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_any(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_can_mmap_sample_resolution
        case snd_pcm_hw_params_can_mmap_sample_resolution_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_can_mmap_sample_resolution(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_can_overrange
        case snd_pcm_hw_params_can_overrange_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_can_overrange(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_can_pause
        case snd_pcm_hw_params_can_pause_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_can_pause(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_can_resume
        case snd_pcm_hw_params_can_resume_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_can_resume(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_can_sync_start
        case snd_pcm_hw_params_can_sync_start_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_can_sync_start(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_copy
        case snd_pcm_hw_params_copy_INDEX: {
            INDEXED_void_snd_pcm_hw_params_t___GENPT___const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_hw_params_t___GENPT___const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_hw_params_t___GENPT___const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            snd_pcm_hw_params_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_current
        case snd_pcm_hw_params_current_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_current(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_dump
        case snd_pcm_hw_params_dump_INDEX: {
            INDEXED_int_snd_pcm_hw_params_t___GENPT___snd_output_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_hw_params_t___GENPT___snd_output_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_hw_params_t___GENPT___snd_output_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_dump(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_free
        case snd_pcm_hw_params_free_INDEX: {
            INDEXED_void_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            snd_pcm_hw_params_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_access
        case snd_pcm_hw_params_get_access_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_access_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_access_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_access_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_access(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_access_mask
        case snd_pcm_hw_params_get_access_mask_INDEX: {
            INDEXED_int_snd_pcm_hw_params_t___GENPT___snd_pcm_access_mask_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_hw_params_t___GENPT___snd_pcm_access_mask_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_hw_params_t___GENPT___snd_pcm_access_mask_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_access_mask(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_buffer_size
        case snd_pcm_hw_params_get_buffer_size_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_buffer_size(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_buffer_size_max
        case snd_pcm_hw_params_get_buffer_size_max_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_buffer_size_max(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_buffer_size_min
        case snd_pcm_hw_params_get_buffer_size_min_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_buffer_size_min(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_buffer_time
        case snd_pcm_hw_params_get_buffer_time_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_buffer_time(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_buffer_time_max
        case snd_pcm_hw_params_get_buffer_time_max_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_buffer_time_max(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_buffer_time_min
        case snd_pcm_hw_params_get_buffer_time_min_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_buffer_time_min(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_channels
        case snd_pcm_hw_params_get_channels_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_channels(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_channels_max
        case snd_pcm_hw_params_get_channels_max_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_channels_max(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_channels_min
        case snd_pcm_hw_params_get_channels_min_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_channels_min(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_export_buffer
        case snd_pcm_hw_params_get_export_buffer_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_export_buffer(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_fifo_size
        case snd_pcm_hw_params_get_fifo_size_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_fifo_size(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_format
        case snd_pcm_hw_params_get_format_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_format_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_format_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_format_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_format(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_format_mask
        case snd_pcm_hw_params_get_format_mask_INDEX: {
            INDEXED_void_snd_pcm_hw_params_t___GENPT___snd_pcm_format_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_hw_params_t___GENPT___snd_pcm_format_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_hw_params_t___GENPT___snd_pcm_format_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_hw_params_get_format_mask(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_min_align
        case snd_pcm_hw_params_get_min_align_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_min_align(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_period_size
        case snd_pcm_hw_params_get_period_size_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_period_size(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_period_size_max
        case snd_pcm_hw_params_get_period_size_max_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_period_size_max(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_period_size_min
        case snd_pcm_hw_params_get_period_size_min_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_period_size_min(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_period_time
        case snd_pcm_hw_params_get_period_time_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_period_time(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_period_time_max
        case snd_pcm_hw_params_get_period_time_max_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_period_time_max(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_period_time_min
        case snd_pcm_hw_params_get_period_time_min_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_period_time_min(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_periods
        case snd_pcm_hw_params_get_periods_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_periods(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_periods_max
        case snd_pcm_hw_params_get_periods_max_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_periods_max(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_periods_min
        case snd_pcm_hw_params_get_periods_min_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_periods_min(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_rate
        case snd_pcm_hw_params_get_rate_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_rate(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_rate_max
        case snd_pcm_hw_params_get_rate_max_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_rate_max(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_rate_min
        case snd_pcm_hw_params_get_rate_min_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_rate_min(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_rate_numden
        case snd_pcm_hw_params_get_rate_numden_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_rate_numden(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_rate_resample
        case snd_pcm_hw_params_get_rate_resample_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_rate_resample(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_sbits
        case snd_pcm_hw_params_get_sbits_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_sbits(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_subformat
        case snd_pcm_hw_params_get_subformat_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_subformat(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_subformat_mask
        case snd_pcm_hw_params_get_subformat_mask_INDEX: {
            INDEXED_void_snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_hw_params_get_subformat_mask(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_tick_time
        case snd_pcm_hw_params_get_tick_time_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_tick_time(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_tick_time_max
        case snd_pcm_hw_params_get_tick_time_max_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_tick_time_max(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_get_tick_time_min
        case snd_pcm_hw_params_get_tick_time_min_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_get_tick_time_min(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_is_batch
        case snd_pcm_hw_params_is_batch_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_is_batch(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_is_block_transfer
        case snd_pcm_hw_params_is_block_transfer_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_is_block_transfer(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_is_double
        case snd_pcm_hw_params_is_double_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_is_double(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_is_half_duplex
        case snd_pcm_hw_params_is_half_duplex_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_is_half_duplex(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_is_joint_duplex
        case snd_pcm_hw_params_is_joint_duplex_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_is_joint_duplex(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_is_monotonic
        case snd_pcm_hw_params_is_monotonic_INDEX: {
            INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_is_monotonic(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_malloc
        case snd_pcm_hw_params_malloc_INDEX: {
            INDEXED_int_snd_pcm_hw_params_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_pcm_hw_params_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_pcm_hw_params_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_access
        case snd_pcm_hw_params_set_access_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_access(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_access_first
        case snd_pcm_hw_params_set_access_first_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_access_first(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_access_last
        case snd_pcm_hw_params_set_access_last_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_access_last(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_access_mask
        case snd_pcm_hw_params_set_access_mask_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_mask_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_mask_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_mask_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_access_mask(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_size
        case snd_pcm_hw_params_set_buffer_size_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_size(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_size_first
        case snd_pcm_hw_params_set_buffer_size_first_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_size_first(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_size_last
        case snd_pcm_hw_params_set_buffer_size_last_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_size_last(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_size_max
        case snd_pcm_hw_params_set_buffer_size_max_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_size_max(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_size_min
        case snd_pcm_hw_params_set_buffer_size_min_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_size_min(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_size_minmax
        case snd_pcm_hw_params_set_buffer_size_minmax_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_size_minmax(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_size_near
        case snd_pcm_hw_params_set_buffer_size_near_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_size_near(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_time
        case snd_pcm_hw_params_set_buffer_time_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_time(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_time_first
        case snd_pcm_hw_params_set_buffer_time_first_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_time_first(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_time_last
        case snd_pcm_hw_params_set_buffer_time_last_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_time_last(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_time_max
        case snd_pcm_hw_params_set_buffer_time_max_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_time_max(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_time_min
        case snd_pcm_hw_params_set_buffer_time_min_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_time_min(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_time_minmax
        case snd_pcm_hw_params_set_buffer_time_minmax_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_time_minmax(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_buffer_time_near
        case snd_pcm_hw_params_set_buffer_time_near_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_buffer_time_near(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_channels
        case snd_pcm_hw_params_set_channels_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_channels(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_channels_first
        case snd_pcm_hw_params_set_channels_first_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_channels_first(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_channels_last
        case snd_pcm_hw_params_set_channels_last_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_channels_last(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_channels_max
        case snd_pcm_hw_params_set_channels_max_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_channels_max(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_channels_min
        case snd_pcm_hw_params_set_channels_min_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_channels_min(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_channels_minmax
        case snd_pcm_hw_params_set_channels_minmax_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_channels_minmax(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_channels_near
        case snd_pcm_hw_params_set_channels_near_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_channels_near(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_export_buffer
        case snd_pcm_hw_params_set_export_buffer_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_export_buffer(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_format
        case snd_pcm_hw_params_set_format_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_format(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_format_first
        case snd_pcm_hw_params_set_format_first_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_format_first(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_format_last
        case snd_pcm_hw_params_set_format_last_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_format_last(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_format_mask
        case snd_pcm_hw_params_set_format_mask_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_mask_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_mask_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_mask_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_format_mask(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_size
        case snd_pcm_hw_params_set_period_size_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_size(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_size_first
        case snd_pcm_hw_params_set_period_size_first_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_size_first(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_size_integer
        case snd_pcm_hw_params_set_period_size_integer_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_size_integer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_size_last
        case snd_pcm_hw_params_set_period_size_last_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_size_last(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_size_max
        case snd_pcm_hw_params_set_period_size_max_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_size_max(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_size_min
        case snd_pcm_hw_params_set_period_size_min_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_size_min(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_size_minmax
        case snd_pcm_hw_params_set_period_size_minmax_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_size_minmax(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_size_near
        case snd_pcm_hw_params_set_period_size_near_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_size_near(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_time
        case snd_pcm_hw_params_set_period_time_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_time(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_time_first
        case snd_pcm_hw_params_set_period_time_first_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_time_first(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_time_last
        case snd_pcm_hw_params_set_period_time_last_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_time_last(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_time_max
        case snd_pcm_hw_params_set_period_time_max_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_time_max(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_time_min
        case snd_pcm_hw_params_set_period_time_min_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_time_min(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_time_minmax
        case snd_pcm_hw_params_set_period_time_minmax_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_time_minmax(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_period_time_near
        case snd_pcm_hw_params_set_period_time_near_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_period_time_near(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_periods
        case snd_pcm_hw_params_set_periods_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_periods(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_periods_first
        case snd_pcm_hw_params_set_periods_first_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_periods_first(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_periods_integer
        case snd_pcm_hw_params_set_periods_integer_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_periods_integer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_periods_last
        case snd_pcm_hw_params_set_periods_last_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_periods_last(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_periods_max
        case snd_pcm_hw_params_set_periods_max_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_periods_max(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_periods_min
        case snd_pcm_hw_params_set_periods_min_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_periods_min(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_periods_minmax
        case snd_pcm_hw_params_set_periods_minmax_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_periods_minmax(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_periods_near
        case snd_pcm_hw_params_set_periods_near_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_periods_near(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_rate
        case snd_pcm_hw_params_set_rate_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_rate(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_rate_first
        case snd_pcm_hw_params_set_rate_first_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_rate_first(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_rate_last
        case snd_pcm_hw_params_set_rate_last_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_rate_last(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_rate_max
        case snd_pcm_hw_params_set_rate_max_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_rate_max(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_rate_min
        case snd_pcm_hw_params_set_rate_min_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_rate_min(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_rate_minmax
        case snd_pcm_hw_params_set_rate_minmax_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_rate_minmax(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_rate_near
        case snd_pcm_hw_params_set_rate_near_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_rate_near(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_rate_resample
        case snd_pcm_hw_params_set_rate_resample_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_rate_resample(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_subformat
        case snd_pcm_hw_params_set_subformat_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_subformat(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_subformat_first
        case snd_pcm_hw_params_set_subformat_first_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_subformat_first(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_subformat_last
        case snd_pcm_hw_params_set_subformat_last_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_subformat_last(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_subformat_mask
        case snd_pcm_hw_params_set_subformat_mask_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_mask_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_mask_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_mask_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_subformat_mask(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_tick_time
        case snd_pcm_hw_params_set_tick_time_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_tick_time(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_tick_time_first
        case snd_pcm_hw_params_set_tick_time_first_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_tick_time_first(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_tick_time_last
        case snd_pcm_hw_params_set_tick_time_last_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_tick_time_last(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_tick_time_max
        case snd_pcm_hw_params_set_tick_time_max_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_tick_time_max(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_tick_time_min
        case snd_pcm_hw_params_set_tick_time_min_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_tick_time_min(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_tick_time_minmax
        case snd_pcm_hw_params_set_tick_time_minmax_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_tick_time_minmax(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_set_tick_time_near
        case snd_pcm_hw_params_set_tick_time_near_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int___GENPT___int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_set_tick_time_near(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_sizeof
        case snd_pcm_hw_params_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_pcm_hw_params_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_test_access
        case snd_pcm_hw_params_test_access_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_access_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_test_access(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_test_buffer_size
        case snd_pcm_hw_params_test_buffer_size_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_test_buffer_size(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_test_buffer_time
        case snd_pcm_hw_params_test_buffer_time_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_test_buffer_time(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_test_channels
        case snd_pcm_hw_params_test_channels_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_test_channels(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_test_format
        case snd_pcm_hw_params_test_format_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_format_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_test_format(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_test_period_size
        case snd_pcm_hw_params_test_period_size_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_uframes_t_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_test_period_size(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_test_period_time
        case snd_pcm_hw_params_test_period_time_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_test_period_time(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_test_periods
        case snd_pcm_hw_params_test_periods_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_test_periods(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_test_rate
        case snd_pcm_hw_params_test_rate_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_test_rate(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_test_subformat
        case snd_pcm_hw_params_test_subformat_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___snd_pcm_subformat_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_test_subformat(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hw_params_test_tick_time
        case snd_pcm_hw_params_test_tick_time_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_hw_params_t___GENPT___unsigned_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hw_params_test_tick_time(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_hwsync
        case snd_pcm_hwsync_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_hwsync(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info
        case snd_pcm_info_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_info_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_copy
        case snd_pcm_info_copy_INDEX: {
            INDEXED_void_snd_pcm_info_t___GENPT___const_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_info_t___GENPT___const_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_info_t___GENPT___const_snd_pcm_info_t___GENPT__ args = unpacked->args;
            snd_pcm_info_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_free
        case snd_pcm_info_free_INDEX: {
            INDEXED_void_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_info_t___GENPT__ args = unpacked->args;
            snd_pcm_info_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_get_card
        case snd_pcm_info_get_card_INDEX: {
            INDEXED_int_const_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_info_get_card(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_get_class
        case snd_pcm_info_get_class_INDEX: {
            INDEXED_snd_pcm_class_t_const_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_snd_pcm_class_t_const_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_snd_pcm_class_t_const_snd_pcm_info_t___GENPT__ args = unpacked->args;
            snd_pcm_class_t *ret = (snd_pcm_class_t *)ret_v;
            *ret =
            snd_pcm_info_get_class(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_get_device
        case snd_pcm_info_get_device_INDEX: {
            INDEXED_unsigned_int_const_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_pcm_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_pcm_info_get_device(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_get_id
        case snd_pcm_info_get_id_INDEX: {
            INDEXED_const_char___GENPT___const_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_pcm_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_info_get_id(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_get_name
        case snd_pcm_info_get_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_pcm_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_info_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_get_stream
        case snd_pcm_info_get_stream_INDEX: {
            INDEXED_snd_pcm_stream_t_const_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_snd_pcm_stream_t_const_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_snd_pcm_stream_t_const_snd_pcm_info_t___GENPT__ args = unpacked->args;
            snd_pcm_stream_t *ret = (snd_pcm_stream_t *)ret_v;
            *ret =
            snd_pcm_info_get_stream(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_get_subclass
        case snd_pcm_info_get_subclass_INDEX: {
            INDEXED_snd_pcm_subclass_t_const_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_snd_pcm_subclass_t_const_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_snd_pcm_subclass_t_const_snd_pcm_info_t___GENPT__ args = unpacked->args;
            snd_pcm_subclass_t *ret = (snd_pcm_subclass_t *)ret_v;
            *ret =
            snd_pcm_info_get_subclass(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_get_subdevice
        case snd_pcm_info_get_subdevice_INDEX: {
            INDEXED_unsigned_int_const_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_pcm_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_pcm_info_get_subdevice(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_get_subdevice_name
        case snd_pcm_info_get_subdevice_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_pcm_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_info_get_subdevice_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_get_subdevices_avail
        case snd_pcm_info_get_subdevices_avail_INDEX: {
            INDEXED_unsigned_int_const_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_pcm_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_pcm_info_get_subdevices_avail(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_get_subdevices_count
        case snd_pcm_info_get_subdevices_count_INDEX: {
            INDEXED_unsigned_int_const_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_pcm_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_pcm_info_get_subdevices_count(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_get_sync
        case snd_pcm_info_get_sync_INDEX: {
            INDEXED_snd_pcm_sync_id_t_const_snd_pcm_info_t___GENPT__ *unpacked = (INDEXED_snd_pcm_sync_id_t_const_snd_pcm_info_t___GENPT__ *)packed;
            ARGS_snd_pcm_sync_id_t_const_snd_pcm_info_t___GENPT__ args = unpacked->args;
            snd_pcm_sync_id_t *ret = (snd_pcm_sync_id_t *)ret_v;
            *ret =
            snd_pcm_info_get_sync(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_malloc
        case snd_pcm_info_malloc_INDEX: {
            INDEXED_int_snd_pcm_info_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_pcm_info_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_pcm_info_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_info_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_set_device
        case snd_pcm_info_set_device_INDEX: {
            INDEXED_void_snd_pcm_info_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_pcm_info_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_pcm_info_t___GENPT___unsigned_int args = unpacked->args;
            snd_pcm_info_set_device(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_set_stream
        case snd_pcm_info_set_stream_INDEX: {
            INDEXED_void_snd_pcm_info_t___GENPT___snd_pcm_stream_t *unpacked = (INDEXED_void_snd_pcm_info_t___GENPT___snd_pcm_stream_t *)packed;
            ARGS_void_snd_pcm_info_t___GENPT___snd_pcm_stream_t args = unpacked->args;
            snd_pcm_info_set_stream(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_set_subdevice
        case snd_pcm_info_set_subdevice_INDEX: {
            INDEXED_void_snd_pcm_info_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_pcm_info_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_pcm_info_t___GENPT___unsigned_int args = unpacked->args;
            snd_pcm_info_set_subdevice(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_info_sizeof
        case snd_pcm_info_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_pcm_info_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_link
        case snd_pcm_link_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_link(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_meter_add_scope
        case snd_pcm_meter_add_scope_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_scope_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_scope_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_scope_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_meter_add_scope(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_meter_get_boundary
        case snd_pcm_meter_get_boundary_INDEX: {
            INDEXED_snd_pcm_uframes_t_snd_pcm_t___GENPT__ *unpacked = (INDEXED_snd_pcm_uframes_t_snd_pcm_t___GENPT__ *)packed;
            ARGS_snd_pcm_uframes_t_snd_pcm_t___GENPT__ args = unpacked->args;
            snd_pcm_uframes_t *ret = (snd_pcm_uframes_t *)ret_v;
            *ret =
            snd_pcm_meter_get_boundary(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_meter_get_bufsize
        case snd_pcm_meter_get_bufsize_INDEX: {
            INDEXED_snd_pcm_uframes_t_snd_pcm_t___GENPT__ *unpacked = (INDEXED_snd_pcm_uframes_t_snd_pcm_t___GENPT__ *)packed;
            ARGS_snd_pcm_uframes_t_snd_pcm_t___GENPT__ args = unpacked->args;
            snd_pcm_uframes_t *ret = (snd_pcm_uframes_t *)ret_v;
            *ret =
            snd_pcm_meter_get_bufsize(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_meter_get_channels
        case snd_pcm_meter_get_channels_INDEX: {
            INDEXED_unsigned_int_snd_pcm_t___GENPT__ *unpacked = (INDEXED_unsigned_int_snd_pcm_t___GENPT__ *)packed;
            ARGS_unsigned_int_snd_pcm_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_pcm_meter_get_channels(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_meter_get_now
        case snd_pcm_meter_get_now_INDEX: {
            INDEXED_snd_pcm_uframes_t_snd_pcm_t___GENPT__ *unpacked = (INDEXED_snd_pcm_uframes_t_snd_pcm_t___GENPT__ *)packed;
            ARGS_snd_pcm_uframes_t_snd_pcm_t___GENPT__ args = unpacked->args;
            snd_pcm_uframes_t *ret = (snd_pcm_uframes_t *)ret_v;
            *ret =
            snd_pcm_meter_get_now(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_meter_get_rate
        case snd_pcm_meter_get_rate_INDEX: {
            INDEXED_unsigned_int_snd_pcm_t___GENPT__ *unpacked = (INDEXED_unsigned_int_snd_pcm_t___GENPT__ *)packed;
            ARGS_unsigned_int_snd_pcm_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_pcm_meter_get_rate(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_meter_search_scope
        case snd_pcm_meter_search_scope_INDEX: {
            INDEXED_snd_pcm_scope_t___GENPT___snd_pcm_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_snd_pcm_scope_t___GENPT___snd_pcm_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_snd_pcm_scope_t___GENPT___snd_pcm_t___GENPT___const_char___GENPT__ args = unpacked->args;
            snd_pcm_scope_t * *ret = (snd_pcm_scope_t * *)ret_v;
            *ret =
            snd_pcm_meter_search_scope(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_mmap_begin
        case snd_pcm_mmap_begin_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___const_snd_pcm_channel_area_t___GENPT____GENPT___snd_pcm_uframes_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___const_snd_pcm_channel_area_t___GENPT____GENPT___snd_pcm_uframes_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___const_snd_pcm_channel_area_t___GENPT____GENPT___snd_pcm_uframes_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_mmap_begin(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_mmap_commit
        case snd_pcm_mmap_commit_INDEX: {
            INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___snd_pcm_uframes_t_snd_pcm_uframes_t *unpacked = (INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___snd_pcm_uframes_t_snd_pcm_uframes_t *)packed;
            ARGS_snd_pcm_sframes_t_snd_pcm_t___GENPT___snd_pcm_uframes_t_snd_pcm_uframes_t args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_mmap_commit(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_mmap_readi
        case snd_pcm_mmap_readi_INDEX: {
            INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT___snd_pcm_uframes_t args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_mmap_readi(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_mmap_readn
        case snd_pcm_mmap_readn_INDEX: {
            INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT____GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT____GENPT___snd_pcm_uframes_t *)packed;
            ARGS_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT____GENPT___snd_pcm_uframes_t args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_mmap_readn(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_mmap_writei
        case snd_pcm_mmap_writei_INDEX: {
            INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___const_void___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___const_void___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_snd_pcm_sframes_t_snd_pcm_t___GENPT___const_void___GENPT___snd_pcm_uframes_t args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_mmap_writei(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_mmap_writen
        case snd_pcm_mmap_writen_INDEX: {
            INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT____GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT____GENPT___snd_pcm_uframes_t *)packed;
            ARGS_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT____GENPT___snd_pcm_uframes_t args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_mmap_writen(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_name
        case snd_pcm_name_INDEX: {
            INDEXED_const_char___GENPT___snd_pcm_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___snd_pcm_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___snd_pcm_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_nonblock
        case snd_pcm_nonblock_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___int *)packed;
            ARGS_int_snd_pcm_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_nonblock(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_open
        case snd_pcm_open_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT____GENPT___const_char___GENPT___snd_pcm_stream_t_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT____GENPT___const_char___GENPT___snd_pcm_stream_t_int *)packed;
            ARGS_int_snd_pcm_t___GENPT____GENPT___const_char___GENPT___snd_pcm_stream_t_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_open(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_open_lconf
        case snd_pcm_open_lconf_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT____GENPT___const_char___GENPT___snd_pcm_stream_t_int_snd_config_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT____GENPT___const_char___GENPT___snd_pcm_stream_t_int_snd_config_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT____GENPT___const_char___GENPT___snd_pcm_stream_t_int_snd_config_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_open_lconf(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_pause
        case snd_pcm_pause_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___int *)packed;
            ARGS_int_snd_pcm_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_pause(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_poll_descriptors
        case snd_pcm_poll_descriptors_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___struct_pollfd___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___struct_pollfd___GENPT___unsigned_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___struct_pollfd___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_poll_descriptors(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_poll_descriptors_count
        case snd_pcm_poll_descriptors_count_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_poll_descriptors_count(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_poll_descriptors_revents
        case snd_pcm_poll_descriptors_revents_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_poll_descriptors_revents(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_prepare
        case snd_pcm_prepare_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_prepare(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_readi
        case snd_pcm_readi_INDEX: {
            INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT___snd_pcm_uframes_t args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_readi(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_readn
        case snd_pcm_readn_INDEX: {
            INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT____GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT____GENPT___snd_pcm_uframes_t *)packed;
            ARGS_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT____GENPT___snd_pcm_uframes_t args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_readn(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_recover
        case snd_pcm_recover_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___int_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___int_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_recover(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_reset
        case snd_pcm_reset_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_reset(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_resume
        case snd_pcm_resume_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_resume(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_rewind
        case snd_pcm_rewind_INDEX: {
            INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_snd_pcm_sframes_t_snd_pcm_t___GENPT___snd_pcm_uframes_t args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_rewind(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_samples_to_bytes
        case snd_pcm_samples_to_bytes_INDEX: {
            INDEXED_ssize_t_snd_pcm_t___GENPT___long *unpacked = (INDEXED_ssize_t_snd_pcm_t___GENPT___long *)packed;
            ARGS_ssize_t_snd_pcm_t___GENPT___long args = unpacked->args;
            ssize_t *ret = (ssize_t *)ret_v;
            *ret =
            snd_pcm_samples_to_bytes(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_scope_get_callback_private
        case snd_pcm_scope_get_callback_private_INDEX: {
            INDEXED_void___GENPT___snd_pcm_scope_t___GENPT__ *unpacked = (INDEXED_void___GENPT___snd_pcm_scope_t___GENPT__ *)packed;
            ARGS_void___GENPT___snd_pcm_scope_t___GENPT__ args = unpacked->args;
            void * *ret = (void * *)ret_v;
            *ret =
            snd_pcm_scope_get_callback_private(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_scope_get_name
        case snd_pcm_scope_get_name_INDEX: {
            INDEXED_const_char___GENPT___snd_pcm_scope_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___snd_pcm_scope_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___snd_pcm_scope_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_scope_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_scope_malloc
        case snd_pcm_scope_malloc_INDEX: {
            INDEXED_int_snd_pcm_scope_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_pcm_scope_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_pcm_scope_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_scope_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_scope_s16_get_channel_buffer
        case snd_pcm_scope_s16_get_channel_buffer_INDEX: {
            INDEXED_int16_t___GENPT___snd_pcm_scope_t___GENPT___unsigned_int *unpacked = (INDEXED_int16_t___GENPT___snd_pcm_scope_t___GENPT___unsigned_int *)packed;
            ARGS_int16_t___GENPT___snd_pcm_scope_t___GENPT___unsigned_int args = unpacked->args;
            int16_t * *ret = (int16_t * *)ret_v;
            *ret =
            snd_pcm_scope_s16_get_channel_buffer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_scope_s16_open
        case snd_pcm_scope_s16_open_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___const_char___GENPT___snd_pcm_scope_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___const_char___GENPT___snd_pcm_scope_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___const_char___GENPT___snd_pcm_scope_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_scope_s16_open(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_scope_set_callback_private
        case snd_pcm_scope_set_callback_private_INDEX: {
            INDEXED_void_snd_pcm_scope_t___GENPT___void___GENPT__ *unpacked = (INDEXED_void_snd_pcm_scope_t___GENPT___void___GENPT__ *)packed;
            ARGS_void_snd_pcm_scope_t___GENPT___void___GENPT__ args = unpacked->args;
            snd_pcm_scope_set_callback_private(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_scope_set_name
        case snd_pcm_scope_set_name_INDEX: {
            INDEXED_void_snd_pcm_scope_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_void_snd_pcm_scope_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_void_snd_pcm_scope_t___GENPT___const_char___GENPT__ args = unpacked->args;
            snd_pcm_scope_set_name(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_scope_set_ops
        case snd_pcm_scope_set_ops_INDEX: {
            INDEXED_void_snd_pcm_scope_t___GENPT___const_snd_pcm_scope_ops_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_scope_t___GENPT___const_snd_pcm_scope_ops_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_scope_t___GENPT___const_snd_pcm_scope_ops_t___GENPT__ args = unpacked->args;
            snd_pcm_scope_set_ops(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_set_params
        case snd_pcm_set_params_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_format_t_snd_pcm_access_t_unsigned_int_unsigned_int_int_unsigned_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_format_t_snd_pcm_access_t_unsigned_int_unsigned_int_int_unsigned_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_format_t_snd_pcm_access_t_unsigned_int_unsigned_int_int_unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_set_params(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_start
        case snd_pcm_start_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_start(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_start_mode_name
        case snd_pcm_start_mode_name_INDEX: {
            INDEXED_const_char___GENPT___snd_pcm_start_t *unpacked = (INDEXED_const_char___GENPT___snd_pcm_start_t *)packed;
            ARGS_const_char___GENPT___snd_pcm_start_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_start_mode_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_state
        case snd_pcm_state_INDEX: {
            INDEXED_snd_pcm_state_t_snd_pcm_t___GENPT__ *unpacked = (INDEXED_snd_pcm_state_t_snd_pcm_t___GENPT__ *)packed;
            ARGS_snd_pcm_state_t_snd_pcm_t___GENPT__ args = unpacked->args;
            snd_pcm_state_t *ret = (snd_pcm_state_t *)ret_v;
            *ret =
            snd_pcm_state(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_state_name
        case snd_pcm_state_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_pcm_state_t *unpacked = (INDEXED_const_char___GENPT___const_snd_pcm_state_t *)packed;
            ARGS_const_char___GENPT___const_snd_pcm_state_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_state_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status
        case snd_pcm_status_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_status_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_status_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_status_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_status(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_copy
        case snd_pcm_status_copy_INDEX: {
            INDEXED_void_snd_pcm_status_t___GENPT___const_snd_pcm_status_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_status_t___GENPT___const_snd_pcm_status_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_status_t___GENPT___const_snd_pcm_status_t___GENPT__ args = unpacked->args;
            snd_pcm_status_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_dump
        case snd_pcm_status_dump_INDEX: {
            INDEXED_int_snd_pcm_status_t___GENPT___snd_output_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_status_t___GENPT___snd_output_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_status_t___GENPT___snd_output_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_status_dump(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_free
        case snd_pcm_status_free_INDEX: {
            INDEXED_void_snd_pcm_status_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_status_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_status_t___GENPT__ args = unpacked->args;
            snd_pcm_status_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_get_avail
        case snd_pcm_status_get_avail_INDEX: {
            INDEXED_snd_pcm_uframes_t_const_snd_pcm_status_t___GENPT__ *unpacked = (INDEXED_snd_pcm_uframes_t_const_snd_pcm_status_t___GENPT__ *)packed;
            ARGS_snd_pcm_uframes_t_const_snd_pcm_status_t___GENPT__ args = unpacked->args;
            snd_pcm_uframes_t *ret = (snd_pcm_uframes_t *)ret_v;
            *ret =
            snd_pcm_status_get_avail(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_get_avail_max
        case snd_pcm_status_get_avail_max_INDEX: {
            INDEXED_snd_pcm_uframes_t_const_snd_pcm_status_t___GENPT__ *unpacked = (INDEXED_snd_pcm_uframes_t_const_snd_pcm_status_t___GENPT__ *)packed;
            ARGS_snd_pcm_uframes_t_const_snd_pcm_status_t___GENPT__ args = unpacked->args;
            snd_pcm_uframes_t *ret = (snd_pcm_uframes_t *)ret_v;
            *ret =
            snd_pcm_status_get_avail_max(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_get_delay
        case snd_pcm_status_get_delay_INDEX: {
            INDEXED_snd_pcm_sframes_t_const_snd_pcm_status_t___GENPT__ *unpacked = (INDEXED_snd_pcm_sframes_t_const_snd_pcm_status_t___GENPT__ *)packed;
            ARGS_snd_pcm_sframes_t_const_snd_pcm_status_t___GENPT__ args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_status_get_delay(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_get_htstamp
        case snd_pcm_status_get_htstamp_INDEX: {
            INDEXED_void_const_snd_pcm_status_t___GENPT___snd_htimestamp_t___GENPT__ *unpacked = (INDEXED_void_const_snd_pcm_status_t___GENPT___snd_htimestamp_t___GENPT__ *)packed;
            ARGS_void_const_snd_pcm_status_t___GENPT___snd_htimestamp_t___GENPT__ args = unpacked->args;
            snd_pcm_status_get_htstamp(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_get_overrange
        case snd_pcm_status_get_overrange_INDEX: {
            INDEXED_snd_pcm_uframes_t_const_snd_pcm_status_t___GENPT__ *unpacked = (INDEXED_snd_pcm_uframes_t_const_snd_pcm_status_t___GENPT__ *)packed;
            ARGS_snd_pcm_uframes_t_const_snd_pcm_status_t___GENPT__ args = unpacked->args;
            snd_pcm_uframes_t *ret = (snd_pcm_uframes_t *)ret_v;
            *ret =
            snd_pcm_status_get_overrange(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_get_state
        case snd_pcm_status_get_state_INDEX: {
            INDEXED_snd_pcm_state_t_const_snd_pcm_status_t___GENPT__ *unpacked = (INDEXED_snd_pcm_state_t_const_snd_pcm_status_t___GENPT__ *)packed;
            ARGS_snd_pcm_state_t_const_snd_pcm_status_t___GENPT__ args = unpacked->args;
            snd_pcm_state_t *ret = (snd_pcm_state_t *)ret_v;
            *ret =
            snd_pcm_status_get_state(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_get_trigger_htstamp
        case snd_pcm_status_get_trigger_htstamp_INDEX: {
            INDEXED_void_const_snd_pcm_status_t___GENPT___snd_htimestamp_t___GENPT__ *unpacked = (INDEXED_void_const_snd_pcm_status_t___GENPT___snd_htimestamp_t___GENPT__ *)packed;
            ARGS_void_const_snd_pcm_status_t___GENPT___snd_htimestamp_t___GENPT__ args = unpacked->args;
            snd_pcm_status_get_trigger_htstamp(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_get_trigger_tstamp
        case snd_pcm_status_get_trigger_tstamp_INDEX: {
            INDEXED_void_const_snd_pcm_status_t___GENPT___snd_timestamp_t___GENPT__ *unpacked = (INDEXED_void_const_snd_pcm_status_t___GENPT___snd_timestamp_t___GENPT__ *)packed;
            ARGS_void_const_snd_pcm_status_t___GENPT___snd_timestamp_t___GENPT__ args = unpacked->args;
            snd_pcm_status_get_trigger_tstamp(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_get_tstamp
        case snd_pcm_status_get_tstamp_INDEX: {
            INDEXED_void_const_snd_pcm_status_t___GENPT___snd_timestamp_t___GENPT__ *unpacked = (INDEXED_void_const_snd_pcm_status_t___GENPT___snd_timestamp_t___GENPT__ *)packed;
            ARGS_void_const_snd_pcm_status_t___GENPT___snd_timestamp_t___GENPT__ args = unpacked->args;
            snd_pcm_status_get_tstamp(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_malloc
        case snd_pcm_status_malloc_INDEX: {
            INDEXED_int_snd_pcm_status_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_pcm_status_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_pcm_status_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_status_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_status_sizeof
        case snd_pcm_status_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_pcm_status_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_stream
        case snd_pcm_stream_INDEX: {
            INDEXED_snd_pcm_stream_t_snd_pcm_t___GENPT__ *unpacked = (INDEXED_snd_pcm_stream_t_snd_pcm_t___GENPT__ *)packed;
            ARGS_snd_pcm_stream_t_snd_pcm_t___GENPT__ args = unpacked->args;
            snd_pcm_stream_t *ret = (snd_pcm_stream_t *)ret_v;
            *ret =
            snd_pcm_stream(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_stream_name
        case snd_pcm_stream_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_pcm_stream_t *unpacked = (INDEXED_const_char___GENPT___const_snd_pcm_stream_t *)packed;
            ARGS_const_char___GENPT___const_snd_pcm_stream_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_stream_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_subformat_description
        case snd_pcm_subformat_description_INDEX: {
            INDEXED_const_char___GENPT___const_snd_pcm_subformat_t *unpacked = (INDEXED_const_char___GENPT___const_snd_pcm_subformat_t *)packed;
            ARGS_const_char___GENPT___const_snd_pcm_subformat_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_subformat_description(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_subformat_mask_any
        case snd_pcm_subformat_mask_any_INDEX: {
            INDEXED_void_snd_pcm_subformat_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_subformat_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_subformat_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_subformat_mask_any(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_subformat_mask_copy
        case snd_pcm_subformat_mask_copy_INDEX: {
            INDEXED_void_snd_pcm_subformat_mask_t___GENPT___const_snd_pcm_subformat_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_subformat_mask_t___GENPT___const_snd_pcm_subformat_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_subformat_mask_t___GENPT___const_snd_pcm_subformat_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_subformat_mask_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_subformat_mask_empty
        case snd_pcm_subformat_mask_empty_INDEX: {
            INDEXED_int_const_snd_pcm_subformat_mask_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_subformat_mask_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_subformat_mask_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_subformat_mask_empty(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_subformat_mask_free
        case snd_pcm_subformat_mask_free_INDEX: {
            INDEXED_void_snd_pcm_subformat_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_subformat_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_subformat_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_subformat_mask_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_subformat_mask_malloc
        case snd_pcm_subformat_mask_malloc_INDEX: {
            INDEXED_int_snd_pcm_subformat_mask_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_pcm_subformat_mask_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_pcm_subformat_mask_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_subformat_mask_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_subformat_mask_none
        case snd_pcm_subformat_mask_none_INDEX: {
            INDEXED_void_snd_pcm_subformat_mask_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_subformat_mask_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_subformat_mask_t___GENPT__ args = unpacked->args;
            snd_pcm_subformat_mask_none(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_subformat_mask_reset
        case snd_pcm_subformat_mask_reset_INDEX: {
            INDEXED_void_snd_pcm_subformat_mask_t___GENPT___snd_pcm_subformat_t *unpacked = (INDEXED_void_snd_pcm_subformat_mask_t___GENPT___snd_pcm_subformat_t *)packed;
            ARGS_void_snd_pcm_subformat_mask_t___GENPT___snd_pcm_subformat_t args = unpacked->args;
            snd_pcm_subformat_mask_reset(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_subformat_mask_set
        case snd_pcm_subformat_mask_set_INDEX: {
            INDEXED_void_snd_pcm_subformat_mask_t___GENPT___snd_pcm_subformat_t *unpacked = (INDEXED_void_snd_pcm_subformat_mask_t___GENPT___snd_pcm_subformat_t *)packed;
            ARGS_void_snd_pcm_subformat_mask_t___GENPT___snd_pcm_subformat_t args = unpacked->args;
            snd_pcm_subformat_mask_set(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_subformat_mask_sizeof
        case snd_pcm_subformat_mask_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_pcm_subformat_mask_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_subformat_mask_test
        case snd_pcm_subformat_mask_test_INDEX: {
            INDEXED_int_const_snd_pcm_subformat_mask_t___GENPT___snd_pcm_subformat_t *unpacked = (INDEXED_int_const_snd_pcm_subformat_mask_t___GENPT___snd_pcm_subformat_t *)packed;
            ARGS_int_const_snd_pcm_subformat_mask_t___GENPT___snd_pcm_subformat_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_subformat_mask_test(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_subformat_name
        case snd_pcm_subformat_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_pcm_subformat_t *unpacked = (INDEXED_const_char___GENPT___const_snd_pcm_subformat_t *)packed;
            ARGS_const_char___GENPT___const_snd_pcm_subformat_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_subformat_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params
        case snd_pcm_sw_params_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_copy
        case snd_pcm_sw_params_copy_INDEX: {
            INDEXED_void_snd_pcm_sw_params_t___GENPT___const_snd_pcm_sw_params_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_sw_params_t___GENPT___const_snd_pcm_sw_params_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_sw_params_t___GENPT___const_snd_pcm_sw_params_t___GENPT__ args = unpacked->args;
            snd_pcm_sw_params_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_current
        case snd_pcm_sw_params_current_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_current(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_dump
        case snd_pcm_sw_params_dump_INDEX: {
            INDEXED_int_snd_pcm_sw_params_t___GENPT___snd_output_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_sw_params_t___GENPT___snd_output_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_sw_params_t___GENPT___snd_output_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_dump(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_free
        case snd_pcm_sw_params_free_INDEX: {
            INDEXED_void_snd_pcm_sw_params_t___GENPT__ *unpacked = (INDEXED_void_snd_pcm_sw_params_t___GENPT__ *)packed;
            ARGS_void_snd_pcm_sw_params_t___GENPT__ args = unpacked->args;
            snd_pcm_sw_params_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_get_avail_min
        case snd_pcm_sw_params_get_avail_min_INDEX: {
            INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_get_avail_min(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_get_boundary
        case snd_pcm_sw_params_get_boundary_INDEX: {
            INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_get_boundary(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_get_silence_size
        case snd_pcm_sw_params_get_silence_size_INDEX: {
            INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_get_silence_size(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_get_silence_threshold
        case snd_pcm_sw_params_get_silence_threshold_INDEX: {
            INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_get_silence_threshold(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_get_sleep_min
        case snd_pcm_sw_params_get_sleep_min_INDEX: {
            INDEXED_int_const_snd_pcm_sw_params_t___GENPT___unsigned_int___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_sw_params_t___GENPT___unsigned_int___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_sw_params_t___GENPT___unsigned_int___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_get_sleep_min(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_get_start_mode
        case snd_pcm_sw_params_get_start_mode_INDEX: {
            INDEXED_snd_pcm_start_t_const_snd_pcm_sw_params_t___GENPT__ *unpacked = (INDEXED_snd_pcm_start_t_const_snd_pcm_sw_params_t___GENPT__ *)packed;
            ARGS_snd_pcm_start_t_const_snd_pcm_sw_params_t___GENPT__ args = unpacked->args;
            snd_pcm_start_t *ret = (snd_pcm_start_t *)ret_v;
            *ret =
            snd_pcm_sw_params_get_start_mode(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_get_start_threshold
        case snd_pcm_sw_params_get_start_threshold_INDEX: {
            INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_get_start_threshold(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_get_stop_threshold
        case snd_pcm_sw_params_get_stop_threshold_INDEX: {
            INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_get_stop_threshold(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_get_tstamp_mode
        case snd_pcm_sw_params_get_tstamp_mode_INDEX: {
            INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_tstamp_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_tstamp_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_tstamp_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_get_tstamp_mode(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_get_xfer_align
        case snd_pcm_sw_params_get_xfer_align_INDEX: {
            INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_const_snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_get_xfer_align(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_get_xrun_mode
        case snd_pcm_sw_params_get_xrun_mode_INDEX: {
            INDEXED_snd_pcm_xrun_t_const_snd_pcm_sw_params_t___GENPT__ *unpacked = (INDEXED_snd_pcm_xrun_t_const_snd_pcm_sw_params_t___GENPT__ *)packed;
            ARGS_snd_pcm_xrun_t_const_snd_pcm_sw_params_t___GENPT__ args = unpacked->args;
            snd_pcm_xrun_t *ret = (snd_pcm_xrun_t *)ret_v;
            *ret =
            snd_pcm_sw_params_get_xrun_mode(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_malloc
        case snd_pcm_sw_params_malloc_INDEX: {
            INDEXED_int_snd_pcm_sw_params_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_pcm_sw_params_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_pcm_sw_params_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_set_avail_min
        case snd_pcm_sw_params_set_avail_min_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_set_avail_min(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_set_silence_size
        case snd_pcm_sw_params_set_silence_size_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_set_silence_size(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_set_silence_threshold
        case snd_pcm_sw_params_set_silence_threshold_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_set_silence_threshold(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_set_sleep_min
        case snd_pcm_sw_params_set_sleep_min_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___unsigned_int *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_set_sleep_min(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_set_start_mode
        case snd_pcm_sw_params_set_start_mode_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_start_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_start_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_start_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_set_start_mode(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_set_start_threshold
        case snd_pcm_sw_params_set_start_threshold_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_set_start_threshold(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_set_stop_threshold
        case snd_pcm_sw_params_set_stop_threshold_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_set_stop_threshold(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_set_tstamp_mode
        case snd_pcm_sw_params_set_tstamp_mode_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_tstamp_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_tstamp_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_tstamp_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_set_tstamp_mode(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_set_xfer_align
        case snd_pcm_sw_params_set_xfer_align_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_uframes_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_set_xfer_align(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_set_xrun_mode
        case snd_pcm_sw_params_set_xrun_mode_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_xrun_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_xrun_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_sw_params_t___GENPT___snd_pcm_xrun_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_sw_params_set_xrun_mode(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_sw_params_sizeof
        case snd_pcm_sw_params_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_pcm_sw_params_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_tstamp_mode_name
        case snd_pcm_tstamp_mode_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_pcm_tstamp_t *unpacked = (INDEXED_const_char___GENPT___const_snd_pcm_tstamp_t *)packed;
            ARGS_const_char___GENPT___const_snd_pcm_tstamp_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_tstamp_mode_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_type
        case snd_pcm_type_INDEX: {
            INDEXED_snd_pcm_type_t_snd_pcm_t___GENPT__ *unpacked = (INDEXED_snd_pcm_type_t_snd_pcm_t___GENPT__ *)packed;
            ARGS_snd_pcm_type_t_snd_pcm_t___GENPT__ args = unpacked->args;
            snd_pcm_type_t *ret = (snd_pcm_type_t *)ret_v;
            *ret =
            snd_pcm_type(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_type_name
        case snd_pcm_type_name_INDEX: {
            INDEXED_const_char___GENPT___snd_pcm_type_t *unpacked = (INDEXED_const_char___GENPT___snd_pcm_type_t *)packed;
            ARGS_const_char___GENPT___snd_pcm_type_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_type_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_unlink
        case snd_pcm_unlink_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_unlink(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_wait
        case snd_pcm_wait_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___int *unpacked = (INDEXED_int_snd_pcm_t___GENPT___int *)packed;
            ARGS_int_snd_pcm_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_pcm_wait(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_writei
        case snd_pcm_writei_INDEX: {
            INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___const_void___GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___const_void___GENPT___snd_pcm_uframes_t *)packed;
            ARGS_snd_pcm_sframes_t_snd_pcm_t___GENPT___const_void___GENPT___snd_pcm_uframes_t args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_writei(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_writen
        case snd_pcm_writen_INDEX: {
            INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT____GENPT___snd_pcm_uframes_t *unpacked = (INDEXED_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT____GENPT___snd_pcm_uframes_t *)packed;
            ARGS_snd_pcm_sframes_t_snd_pcm_t___GENPT___void___GENPT____GENPT___snd_pcm_uframes_t args = unpacked->args;
            snd_pcm_sframes_t *ret = (snd_pcm_sframes_t *)ret_v;
            *ret =
            snd_pcm_writen(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_pcm_xrun_mode_name
        case snd_pcm_xrun_mode_name_INDEX: {
            INDEXED_const_char___GENPT___snd_pcm_xrun_t *unpacked = (INDEXED_const_char___GENPT___snd_pcm_xrun_t *)packed;
            ARGS_const_char___GENPT___snd_pcm_xrun_t args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_pcm_xrun_mode_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_close
        case snd_rawmidi_close_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT__ *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT__ *)packed;
            ARGS_int_snd_rawmidi_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_close(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_drain
        case snd_rawmidi_drain_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT__ *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT__ *)packed;
            ARGS_int_snd_rawmidi_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_drain(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_drop
        case snd_rawmidi_drop_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT__ *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT__ *)packed;
            ARGS_int_snd_rawmidi_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_drop(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info
        case snd_rawmidi_info_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_int_snd_rawmidi_t___GENPT___snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_copy
        case snd_rawmidi_info_copy_INDEX: {
            INDEXED_void_snd_rawmidi_info_t___GENPT___const_snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_void_snd_rawmidi_info_t___GENPT___const_snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_void_snd_rawmidi_info_t___GENPT___const_snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            snd_rawmidi_info_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_free
        case snd_rawmidi_info_free_INDEX: {
            INDEXED_void_snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_void_snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_void_snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            snd_rawmidi_info_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_get_card
        case snd_rawmidi_info_get_card_INDEX: {
            INDEXED_int_const_snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_info_get_card(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_get_device
        case snd_rawmidi_info_get_device_INDEX: {
            INDEXED_unsigned_int_const_snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_rawmidi_info_get_device(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_get_flags
        case snd_rawmidi_info_get_flags_INDEX: {
            INDEXED_unsigned_int_const_snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_rawmidi_info_get_flags(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_get_id
        case snd_rawmidi_info_get_id_INDEX: {
            INDEXED_const_char___GENPT___const_snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_rawmidi_info_get_id(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_get_name
        case snd_rawmidi_info_get_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_rawmidi_info_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_get_stream
        case snd_rawmidi_info_get_stream_INDEX: {
            INDEXED_snd_rawmidi_stream_t_const_snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_snd_rawmidi_stream_t_const_snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_snd_rawmidi_stream_t_const_snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            snd_rawmidi_stream_t *ret = (snd_rawmidi_stream_t *)ret_v;
            *ret =
            snd_rawmidi_info_get_stream(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_get_subdevice
        case snd_rawmidi_info_get_subdevice_INDEX: {
            INDEXED_unsigned_int_const_snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_rawmidi_info_get_subdevice(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_get_subdevice_name
        case snd_rawmidi_info_get_subdevice_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_rawmidi_info_get_subdevice_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_get_subdevices_avail
        case snd_rawmidi_info_get_subdevices_avail_INDEX: {
            INDEXED_unsigned_int_const_snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_rawmidi_info_get_subdevices_avail(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_get_subdevices_count
        case snd_rawmidi_info_get_subdevices_count_INDEX: {
            INDEXED_unsigned_int_const_snd_rawmidi_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_rawmidi_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_rawmidi_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_rawmidi_info_get_subdevices_count(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_malloc
        case snd_rawmidi_info_malloc_INDEX: {
            INDEXED_int_snd_rawmidi_info_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_rawmidi_info_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_rawmidi_info_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_info_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_set_device
        case snd_rawmidi_info_set_device_INDEX: {
            INDEXED_void_snd_rawmidi_info_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_rawmidi_info_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_rawmidi_info_t___GENPT___unsigned_int args = unpacked->args;
            snd_rawmidi_info_set_device(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_set_stream
        case snd_rawmidi_info_set_stream_INDEX: {
            INDEXED_void_snd_rawmidi_info_t___GENPT___snd_rawmidi_stream_t *unpacked = (INDEXED_void_snd_rawmidi_info_t___GENPT___snd_rawmidi_stream_t *)packed;
            ARGS_void_snd_rawmidi_info_t___GENPT___snd_rawmidi_stream_t args = unpacked->args;
            snd_rawmidi_info_set_stream(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_set_subdevice
        case snd_rawmidi_info_set_subdevice_INDEX: {
            INDEXED_void_snd_rawmidi_info_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_rawmidi_info_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_rawmidi_info_t___GENPT___unsigned_int args = unpacked->args;
            snd_rawmidi_info_set_subdevice(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_info_sizeof
        case snd_rawmidi_info_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_rawmidi_info_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_name
        case snd_rawmidi_name_INDEX: {
            INDEXED_const_char___GENPT___snd_rawmidi_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___snd_rawmidi_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___snd_rawmidi_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_rawmidi_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_nonblock
        case snd_rawmidi_nonblock_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT___int *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT___int *)packed;
            ARGS_int_snd_rawmidi_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_nonblock(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_open
        case snd_rawmidi_open_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT____GENPT___snd_rawmidi_t___GENPT____GENPT___const_char___GENPT___int *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT____GENPT___snd_rawmidi_t___GENPT____GENPT___const_char___GENPT___int *)packed;
            ARGS_int_snd_rawmidi_t___GENPT____GENPT___snd_rawmidi_t___GENPT____GENPT___const_char___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_open(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_open_lconf
        case snd_rawmidi_open_lconf_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT____GENPT___snd_rawmidi_t___GENPT____GENPT___const_char___GENPT___int_snd_config_t___GENPT__ *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT____GENPT___snd_rawmidi_t___GENPT____GENPT___const_char___GENPT___int_snd_config_t___GENPT__ *)packed;
            ARGS_int_snd_rawmidi_t___GENPT____GENPT___snd_rawmidi_t___GENPT____GENPT___const_char___GENPT___int_snd_config_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_open_lconf(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_params
        case snd_rawmidi_params_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT__ *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT__ *)packed;
            ARGS_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_params(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_params_copy
        case snd_rawmidi_params_copy_INDEX: {
            INDEXED_void_snd_rawmidi_params_t___GENPT___const_snd_rawmidi_params_t___GENPT__ *unpacked = (INDEXED_void_snd_rawmidi_params_t___GENPT___const_snd_rawmidi_params_t___GENPT__ *)packed;
            ARGS_void_snd_rawmidi_params_t___GENPT___const_snd_rawmidi_params_t___GENPT__ args = unpacked->args;
            snd_rawmidi_params_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_params_current
        case snd_rawmidi_params_current_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT__ *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT__ *)packed;
            ARGS_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_params_current(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_params_free
        case snd_rawmidi_params_free_INDEX: {
            INDEXED_void_snd_rawmidi_params_t___GENPT__ *unpacked = (INDEXED_void_snd_rawmidi_params_t___GENPT__ *)packed;
            ARGS_void_snd_rawmidi_params_t___GENPT__ args = unpacked->args;
            snd_rawmidi_params_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_params_get_avail_min
        case snd_rawmidi_params_get_avail_min_INDEX: {
            INDEXED_size_t_const_snd_rawmidi_params_t___GENPT__ *unpacked = (INDEXED_size_t_const_snd_rawmidi_params_t___GENPT__ *)packed;
            ARGS_size_t_const_snd_rawmidi_params_t___GENPT__ args = unpacked->args;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_rawmidi_params_get_avail_min(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_params_get_buffer_size
        case snd_rawmidi_params_get_buffer_size_INDEX: {
            INDEXED_size_t_const_snd_rawmidi_params_t___GENPT__ *unpacked = (INDEXED_size_t_const_snd_rawmidi_params_t___GENPT__ *)packed;
            ARGS_size_t_const_snd_rawmidi_params_t___GENPT__ args = unpacked->args;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_rawmidi_params_get_buffer_size(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_params_get_no_active_sensing
        case snd_rawmidi_params_get_no_active_sensing_INDEX: {
            INDEXED_int_const_snd_rawmidi_params_t___GENPT__ *unpacked = (INDEXED_int_const_snd_rawmidi_params_t___GENPT__ *)packed;
            ARGS_int_const_snd_rawmidi_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_params_get_no_active_sensing(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_params_malloc
        case snd_rawmidi_params_malloc_INDEX: {
            INDEXED_int_snd_rawmidi_params_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_rawmidi_params_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_rawmidi_params_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_params_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_params_set_avail_min
        case snd_rawmidi_params_set_avail_min_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT___size_t *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT___size_t *)packed;
            ARGS_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT___size_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_params_set_avail_min(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_params_set_buffer_size
        case snd_rawmidi_params_set_buffer_size_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT___size_t *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT___size_t *)packed;
            ARGS_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT___size_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_params_set_buffer_size(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_params_set_no_active_sensing
        case snd_rawmidi_params_set_no_active_sensing_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT___int *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT___int *)packed;
            ARGS_int_snd_rawmidi_t___GENPT___snd_rawmidi_params_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_params_set_no_active_sensing(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_params_sizeof
        case snd_rawmidi_params_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_rawmidi_params_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_poll_descriptors
        case snd_rawmidi_poll_descriptors_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT___struct_pollfd___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT___struct_pollfd___GENPT___unsigned_int *)packed;
            ARGS_int_snd_rawmidi_t___GENPT___struct_pollfd___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_poll_descriptors(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_poll_descriptors_count
        case snd_rawmidi_poll_descriptors_count_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT__ *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT__ *)packed;
            ARGS_int_snd_rawmidi_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_poll_descriptors_count(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_poll_descriptors_revents
        case snd_rawmidi_poll_descriptors_revents_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *)packed;
            ARGS_int_snd_rawmidi_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_poll_descriptors_revents(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_read
        case snd_rawmidi_read_INDEX: {
            INDEXED_ssize_t_snd_rawmidi_t___GENPT___void___GENPT___size_t *unpacked = (INDEXED_ssize_t_snd_rawmidi_t___GENPT___void___GENPT___size_t *)packed;
            ARGS_ssize_t_snd_rawmidi_t___GENPT___void___GENPT___size_t args = unpacked->args;
            ssize_t *ret = (ssize_t *)ret_v;
            *ret =
            snd_rawmidi_read(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_status
        case snd_rawmidi_status_INDEX: {
            INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_status_t___GENPT__ *unpacked = (INDEXED_int_snd_rawmidi_t___GENPT___snd_rawmidi_status_t___GENPT__ *)packed;
            ARGS_int_snd_rawmidi_t___GENPT___snd_rawmidi_status_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_status(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_status_copy
        case snd_rawmidi_status_copy_INDEX: {
            INDEXED_void_snd_rawmidi_status_t___GENPT___const_snd_rawmidi_status_t___GENPT__ *unpacked = (INDEXED_void_snd_rawmidi_status_t___GENPT___const_snd_rawmidi_status_t___GENPT__ *)packed;
            ARGS_void_snd_rawmidi_status_t___GENPT___const_snd_rawmidi_status_t___GENPT__ args = unpacked->args;
            snd_rawmidi_status_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_status_free
        case snd_rawmidi_status_free_INDEX: {
            INDEXED_void_snd_rawmidi_status_t___GENPT__ *unpacked = (INDEXED_void_snd_rawmidi_status_t___GENPT__ *)packed;
            ARGS_void_snd_rawmidi_status_t___GENPT__ args = unpacked->args;
            snd_rawmidi_status_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_status_get_avail
        case snd_rawmidi_status_get_avail_INDEX: {
            INDEXED_size_t_const_snd_rawmidi_status_t___GENPT__ *unpacked = (INDEXED_size_t_const_snd_rawmidi_status_t___GENPT__ *)packed;
            ARGS_size_t_const_snd_rawmidi_status_t___GENPT__ args = unpacked->args;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_rawmidi_status_get_avail(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_status_get_tstamp
        case snd_rawmidi_status_get_tstamp_INDEX: {
            INDEXED_void_const_snd_rawmidi_status_t___GENPT___snd_htimestamp_t___GENPT__ *unpacked = (INDEXED_void_const_snd_rawmidi_status_t___GENPT___snd_htimestamp_t___GENPT__ *)packed;
            ARGS_void_const_snd_rawmidi_status_t___GENPT___snd_htimestamp_t___GENPT__ args = unpacked->args;
            snd_rawmidi_status_get_tstamp(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_status_get_xruns
        case snd_rawmidi_status_get_xruns_INDEX: {
            INDEXED_size_t_const_snd_rawmidi_status_t___GENPT__ *unpacked = (INDEXED_size_t_const_snd_rawmidi_status_t___GENPT__ *)packed;
            ARGS_size_t_const_snd_rawmidi_status_t___GENPT__ args = unpacked->args;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_rawmidi_status_get_xruns(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_status_malloc
        case snd_rawmidi_status_malloc_INDEX: {
            INDEXED_int_snd_rawmidi_status_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_rawmidi_status_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_rawmidi_status_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_rawmidi_status_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_status_sizeof
        case snd_rawmidi_status_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_rawmidi_status_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_stream
        case snd_rawmidi_stream_INDEX: {
            INDEXED_snd_rawmidi_stream_t_snd_rawmidi_t___GENPT__ *unpacked = (INDEXED_snd_rawmidi_stream_t_snd_rawmidi_t___GENPT__ *)packed;
            ARGS_snd_rawmidi_stream_t_snd_rawmidi_t___GENPT__ args = unpacked->args;
            snd_rawmidi_stream_t *ret = (snd_rawmidi_stream_t *)ret_v;
            *ret =
            snd_rawmidi_stream(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_type
        case snd_rawmidi_type_INDEX: {
            INDEXED_snd_rawmidi_type_t_snd_rawmidi_t___GENPT__ *unpacked = (INDEXED_snd_rawmidi_type_t_snd_rawmidi_t___GENPT__ *)packed;
            ARGS_snd_rawmidi_type_t_snd_rawmidi_t___GENPT__ args = unpacked->args;
            snd_rawmidi_type_t *ret = (snd_rawmidi_type_t *)ret_v;
            *ret =
            snd_rawmidi_type(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_rawmidi_write
        case snd_rawmidi_write_INDEX: {
            INDEXED_ssize_t_snd_rawmidi_t___GENPT___const_void___GENPT___size_t *unpacked = (INDEXED_ssize_t_snd_rawmidi_t___GENPT___const_void___GENPT___size_t *)packed;
            ARGS_ssize_t_snd_rawmidi_t___GENPT___const_void___GENPT___size_t args = unpacked->args;
            ssize_t *ret = (ssize_t *)ret_v;
            *ret =
            snd_rawmidi_write(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_sctl_build
        case snd_sctl_build_INDEX: {
            INDEXED_int_snd_sctl_t___GENPT____GENPT___snd_ctl_t___GENPT___snd_config_t___GENPT___snd_config_t___GENPT___int *unpacked = (INDEXED_int_snd_sctl_t___GENPT____GENPT___snd_ctl_t___GENPT___snd_config_t___GENPT___snd_config_t___GENPT___int *)packed;
            ARGS_int_snd_sctl_t___GENPT____GENPT___snd_ctl_t___GENPT___snd_config_t___GENPT___snd_config_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_sctl_build(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_snd_sctl_free
        case snd_sctl_free_INDEX: {
            INDEXED_int_snd_sctl_t___GENPT__ *unpacked = (INDEXED_int_snd_sctl_t___GENPT__ *)packed;
            ARGS_int_snd_sctl_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_sctl_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_sctl_install
        case snd_sctl_install_INDEX: {
            INDEXED_int_snd_sctl_t___GENPT__ *unpacked = (INDEXED_int_snd_sctl_t___GENPT__ *)packed;
            ARGS_int_snd_sctl_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_sctl_install(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_sctl_remove
        case snd_sctl_remove_INDEX: {
            INDEXED_int_snd_sctl_t___GENPT__ *unpacked = (INDEXED_int_snd_sctl_t___GENPT__ *)packed;
            ARGS_int_snd_sctl_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_sctl_remove(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_alloc_named_queue
        case snd_seq_alloc_named_queue_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_alloc_named_queue(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_alloc_queue
        case snd_seq_alloc_queue_INDEX: {
            INDEXED_int_snd_seq_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_alloc_queue(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_change_bit
        case snd_seq_change_bit_INDEX: {
            INDEXED_int_int_void___GENPT__ *unpacked = (INDEXED_int_int_void___GENPT__ *)packed;
            ARGS_int_int_void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_change_bit(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_id
        case snd_seq_client_id_INDEX: {
            INDEXED_int_snd_seq_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_client_id(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_copy
        case snd_seq_client_info_copy_INDEX: {
            INDEXED_void_snd_seq_client_info_t___GENPT___const_snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_client_info_t___GENPT___const_snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_void_snd_seq_client_info_t___GENPT___const_snd_seq_client_info_t___GENPT__ args = unpacked->args;
            snd_seq_client_info_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_free
        case snd_seq_client_info_free_INDEX: {
            INDEXED_void_snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_void_snd_seq_client_info_t___GENPT__ args = unpacked->args;
            snd_seq_client_info_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_get_broadcast_filter
        case snd_seq_client_info_get_broadcast_filter_INDEX: {
            INDEXED_int_const_snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_client_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_client_info_get_broadcast_filter(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_get_client
        case snd_seq_client_info_get_client_INDEX: {
            INDEXED_int_const_snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_client_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_client_info_get_client(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_get_error_bounce
        case snd_seq_client_info_get_error_bounce_INDEX: {
            INDEXED_int_const_snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_client_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_client_info_get_error_bounce(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_get_event_filter
        case snd_seq_client_info_get_event_filter_INDEX: {
            INDEXED_const_unsigned_char___GENPT___const_snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_const_unsigned_char___GENPT___const_snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_const_unsigned_char___GENPT___const_snd_seq_client_info_t___GENPT__ args = unpacked->args;
            const unsigned char * *ret = (const unsigned char * *)ret_v;
            *ret =
            snd_seq_client_info_get_event_filter(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_get_event_lost
        case snd_seq_client_info_get_event_lost_INDEX: {
            INDEXED_int_const_snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_client_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_client_info_get_event_lost(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_get_name
        case snd_seq_client_info_get_name_INDEX: {
            INDEXED_const_char___GENPT___snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___snd_seq_client_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_seq_client_info_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_get_num_ports
        case snd_seq_client_info_get_num_ports_INDEX: {
            INDEXED_int_const_snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_client_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_client_info_get_num_ports(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_get_type
        case snd_seq_client_info_get_type_INDEX: {
            INDEXED_snd_seq_client_type_t_const_snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_snd_seq_client_type_t_const_snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_snd_seq_client_type_t_const_snd_seq_client_info_t___GENPT__ args = unpacked->args;
            snd_seq_client_type_t *ret = (snd_seq_client_type_t *)ret_v;
            *ret =
            snd_seq_client_info_get_type(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_malloc
        case snd_seq_client_info_malloc_INDEX: {
            INDEXED_int_snd_seq_client_info_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_seq_client_info_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_seq_client_info_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_client_info_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_set_broadcast_filter
        case snd_seq_client_info_set_broadcast_filter_INDEX: {
            INDEXED_void_snd_seq_client_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_client_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_client_info_t___GENPT___int args = unpacked->args;
            snd_seq_client_info_set_broadcast_filter(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_set_client
        case snd_seq_client_info_set_client_INDEX: {
            INDEXED_void_snd_seq_client_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_client_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_client_info_t___GENPT___int args = unpacked->args;
            snd_seq_client_info_set_client(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_set_error_bounce
        case snd_seq_client_info_set_error_bounce_INDEX: {
            INDEXED_void_snd_seq_client_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_client_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_client_info_t___GENPT___int args = unpacked->args;
            snd_seq_client_info_set_error_bounce(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_set_event_filter
        case snd_seq_client_info_set_event_filter_INDEX: {
            INDEXED_void_snd_seq_client_info_t___GENPT___unsigned_char___GENPT__ *unpacked = (INDEXED_void_snd_seq_client_info_t___GENPT___unsigned_char___GENPT__ *)packed;
            ARGS_void_snd_seq_client_info_t___GENPT___unsigned_char___GENPT__ args = unpacked->args;
            snd_seq_client_info_set_event_filter(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_set_name
        case snd_seq_client_info_set_name_INDEX: {
            INDEXED_void_snd_seq_client_info_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_void_snd_seq_client_info_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_void_snd_seq_client_info_t___GENPT___const_char___GENPT__ args = unpacked->args;
            snd_seq_client_info_set_name(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_info_sizeof
        case snd_seq_client_info_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_client_info_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_pool_copy
        case snd_seq_client_pool_copy_INDEX: {
            INDEXED_void_snd_seq_client_pool_t___GENPT___const_snd_seq_client_pool_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_client_pool_t___GENPT___const_snd_seq_client_pool_t___GENPT__ *)packed;
            ARGS_void_snd_seq_client_pool_t___GENPT___const_snd_seq_client_pool_t___GENPT__ args = unpacked->args;
            snd_seq_client_pool_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_pool_free
        case snd_seq_client_pool_free_INDEX: {
            INDEXED_void_snd_seq_client_pool_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_client_pool_t___GENPT__ *)packed;
            ARGS_void_snd_seq_client_pool_t___GENPT__ args = unpacked->args;
            snd_seq_client_pool_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_pool_get_client
        case snd_seq_client_pool_get_client_INDEX: {
            INDEXED_int_const_snd_seq_client_pool_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_client_pool_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_client_pool_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_client_pool_get_client(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_pool_get_input_free
        case snd_seq_client_pool_get_input_free_INDEX: {
            INDEXED_size_t_const_snd_seq_client_pool_t___GENPT__ *unpacked = (INDEXED_size_t_const_snd_seq_client_pool_t___GENPT__ *)packed;
            ARGS_size_t_const_snd_seq_client_pool_t___GENPT__ args = unpacked->args;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_client_pool_get_input_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_pool_get_input_pool
        case snd_seq_client_pool_get_input_pool_INDEX: {
            INDEXED_size_t_const_snd_seq_client_pool_t___GENPT__ *unpacked = (INDEXED_size_t_const_snd_seq_client_pool_t___GENPT__ *)packed;
            ARGS_size_t_const_snd_seq_client_pool_t___GENPT__ args = unpacked->args;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_client_pool_get_input_pool(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_pool_get_output_free
        case snd_seq_client_pool_get_output_free_INDEX: {
            INDEXED_size_t_const_snd_seq_client_pool_t___GENPT__ *unpacked = (INDEXED_size_t_const_snd_seq_client_pool_t___GENPT__ *)packed;
            ARGS_size_t_const_snd_seq_client_pool_t___GENPT__ args = unpacked->args;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_client_pool_get_output_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_pool_get_output_pool
        case snd_seq_client_pool_get_output_pool_INDEX: {
            INDEXED_size_t_const_snd_seq_client_pool_t___GENPT__ *unpacked = (INDEXED_size_t_const_snd_seq_client_pool_t___GENPT__ *)packed;
            ARGS_size_t_const_snd_seq_client_pool_t___GENPT__ args = unpacked->args;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_client_pool_get_output_pool(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_pool_get_output_room
        case snd_seq_client_pool_get_output_room_INDEX: {
            INDEXED_size_t_const_snd_seq_client_pool_t___GENPT__ *unpacked = (INDEXED_size_t_const_snd_seq_client_pool_t___GENPT__ *)packed;
            ARGS_size_t_const_snd_seq_client_pool_t___GENPT__ args = unpacked->args;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_client_pool_get_output_room(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_pool_malloc
        case snd_seq_client_pool_malloc_INDEX: {
            INDEXED_int_snd_seq_client_pool_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_seq_client_pool_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_seq_client_pool_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_client_pool_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_pool_set_input_pool
        case snd_seq_client_pool_set_input_pool_INDEX: {
            INDEXED_void_snd_seq_client_pool_t___GENPT___size_t *unpacked = (INDEXED_void_snd_seq_client_pool_t___GENPT___size_t *)packed;
            ARGS_void_snd_seq_client_pool_t___GENPT___size_t args = unpacked->args;
            snd_seq_client_pool_set_input_pool(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_pool_set_output_pool
        case snd_seq_client_pool_set_output_pool_INDEX: {
            INDEXED_void_snd_seq_client_pool_t___GENPT___size_t *unpacked = (INDEXED_void_snd_seq_client_pool_t___GENPT___size_t *)packed;
            ARGS_void_snd_seq_client_pool_t___GENPT___size_t args = unpacked->args;
            snd_seq_client_pool_set_output_pool(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_pool_set_output_room
        case snd_seq_client_pool_set_output_room_INDEX: {
            INDEXED_void_snd_seq_client_pool_t___GENPT___size_t *unpacked = (INDEXED_void_snd_seq_client_pool_t___GENPT___size_t *)packed;
            ARGS_void_snd_seq_client_pool_t___GENPT___size_t args = unpacked->args;
            snd_seq_client_pool_set_output_room(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_client_pool_sizeof
        case snd_seq_client_pool_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_client_pool_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_close
        case snd_seq_close_INDEX: {
            INDEXED_int_snd_seq_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_close(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_connect_from
        case snd_seq_connect_from_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_int_int *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_int_int *)packed;
            ARGS_int_snd_seq_t___GENPT___int_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_connect_from(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_connect_to
        case snd_seq_connect_to_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_int_int *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_int_int *)packed;
            ARGS_int_snd_seq_t___GENPT___int_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_connect_to(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_control_queue
        case snd_seq_control_queue_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_int_int_snd_seq_event_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_int_int_snd_seq_event_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___int_int_int_snd_seq_event_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_control_queue(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_create_port
        case snd_seq_create_port_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_create_port(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_create_queue
        case snd_seq_create_queue_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_queue_info_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_queue_info_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_queue_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_create_queue(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_create_simple_port
        case snd_seq_create_simple_port_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___const_char___GENPT___unsigned_int_unsigned_int *unpacked = (INDEXED_int_snd_seq_t___GENPT___const_char___GENPT___unsigned_int_unsigned_int *)packed;
            ARGS_int_snd_seq_t___GENPT___const_char___GENPT___unsigned_int_unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_create_simple_port(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_delete_port
        case snd_seq_delete_port_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int *unpacked = (INDEXED_int_snd_seq_t___GENPT___int *)packed;
            ARGS_int_snd_seq_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_delete_port(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_delete_simple_port
        case snd_seq_delete_simple_port_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int *unpacked = (INDEXED_int_snd_seq_t___GENPT___int *)packed;
            ARGS_int_snd_seq_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_delete_simple_port(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_disconnect_from
        case snd_seq_disconnect_from_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_int_int *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_int_int *)packed;
            ARGS_int_snd_seq_t___GENPT___int_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_disconnect_from(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_disconnect_to
        case snd_seq_disconnect_to_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_int_int *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_int_int *)packed;
            ARGS_int_snd_seq_t___GENPT___int_int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_disconnect_to(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_drain_output
        case snd_seq_drain_output_INDEX: {
            INDEXED_int_snd_seq_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_drain_output(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_drop_input
        case snd_seq_drop_input_INDEX: {
            INDEXED_int_snd_seq_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_drop_input(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_drop_input_buffer
        case snd_seq_drop_input_buffer_INDEX: {
            INDEXED_int_snd_seq_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_drop_input_buffer(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_drop_output
        case snd_seq_drop_output_INDEX: {
            INDEXED_int_snd_seq_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_drop_output(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_drop_output_buffer
        case snd_seq_drop_output_buffer_INDEX: {
            INDEXED_int_snd_seq_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_drop_output_buffer(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_event_input
        case snd_seq_event_input_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_event_input(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_event_input_pending
        case snd_seq_event_input_pending_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int *unpacked = (INDEXED_int_snd_seq_t___GENPT___int *)packed;
            ARGS_int_snd_seq_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_event_input_pending(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_event_length
        case snd_seq_event_length_INDEX: {
            INDEXED_ssize_t_snd_seq_event_t___GENPT__ *unpacked = (INDEXED_ssize_t_snd_seq_event_t___GENPT__ *)packed;
            ARGS_ssize_t_snd_seq_event_t___GENPT__ args = unpacked->args;
            ssize_t *ret = (ssize_t *)ret_v;
            *ret =
            snd_seq_event_length(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_event_output
        case snd_seq_event_output_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_event_output(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_event_output_buffer
        case snd_seq_event_output_buffer_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_event_output_buffer(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_event_output_direct
        case snd_seq_event_output_direct_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_event_output_direct(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_event_output_pending
        case snd_seq_event_output_pending_INDEX: {
            INDEXED_int_snd_seq_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_event_output_pending(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_extract_output
        case snd_seq_extract_output_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_event_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_extract_output(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_free_event
        case snd_seq_free_event_INDEX: {
            INDEXED_int_snd_seq_event_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_event_t___GENPT__ *)packed;
            ARGS_int_snd_seq_event_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_free_event(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_free_queue
        case snd_seq_free_queue_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int *unpacked = (INDEXED_int_snd_seq_t___GENPT___int *)packed;
            ARGS_int_snd_seq_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_free_queue(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_any_client_info
        case snd_seq_get_any_client_info_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___int_snd_seq_client_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_get_any_client_info(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_any_port_info
        case snd_seq_get_any_port_info_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_int_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_int_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___int_int_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_get_any_port_info(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_bit
        case snd_seq_get_bit_INDEX: {
            INDEXED_int_int_void___GENPT__ *unpacked = (INDEXED_int_int_void___GENPT__ *)packed;
            ARGS_int_int_void___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_get_bit(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_client_info
        case snd_seq_get_client_info_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_client_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_get_client_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_client_pool
        case snd_seq_get_client_pool_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_client_pool_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_client_pool_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_client_pool_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_get_client_pool(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_input_buffer_size
        case snd_seq_get_input_buffer_size_INDEX: {
            INDEXED_size_t_snd_seq_t___GENPT__ *unpacked = (INDEXED_size_t_snd_seq_t___GENPT__ *)packed;
            ARGS_size_t_snd_seq_t___GENPT__ args = unpacked->args;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_get_input_buffer_size(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_output_buffer_size
        case snd_seq_get_output_buffer_size_INDEX: {
            INDEXED_size_t_snd_seq_t___GENPT__ *unpacked = (INDEXED_size_t_snd_seq_t___GENPT__ *)packed;
            ARGS_size_t_snd_seq_t___GENPT__ args = unpacked->args;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_get_output_buffer_size(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_port_info
        case snd_seq_get_port_info_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___int_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_get_port_info(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_port_subscription
        case snd_seq_get_port_subscription_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_port_subscribe_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_port_subscribe_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_port_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_get_port_subscription(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_queue_info
        case snd_seq_get_queue_info_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_info_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_info_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___int_snd_seq_queue_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_get_queue_info(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_queue_status
        case snd_seq_get_queue_status_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_status_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_status_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___int_snd_seq_queue_status_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_get_queue_status(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_queue_tempo
        case snd_seq_get_queue_tempo_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_tempo_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_tempo_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___int_snd_seq_queue_tempo_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_get_queue_tempo(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_queue_timer
        case snd_seq_get_queue_timer_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_timer_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_timer_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___int_snd_seq_queue_timer_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_get_queue_timer(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_get_queue_usage
        case snd_seq_get_queue_usage_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int *unpacked = (INDEXED_int_snd_seq_t___GENPT___int *)packed;
            ARGS_int_snd_seq_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_get_queue_usage(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_name
        case snd_seq_name_INDEX: {
            INDEXED_const_char___GENPT___snd_seq_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___snd_seq_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___snd_seq_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_seq_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_nonblock
        case snd_seq_nonblock_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int *unpacked = (INDEXED_int_snd_seq_t___GENPT___int *)packed;
            ARGS_int_snd_seq_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_nonblock(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_open
        case snd_seq_open_INDEX: {
            INDEXED_int_snd_seq_t___GENPT____GENPT___const_char___GENPT___int_int *unpacked = (INDEXED_int_snd_seq_t___GENPT____GENPT___const_char___GENPT___int_int *)packed;
            ARGS_int_snd_seq_t___GENPT____GENPT___const_char___GENPT___int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_open(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_open_lconf
        case snd_seq_open_lconf_INDEX: {
            INDEXED_int_snd_seq_t___GENPT____GENPT___const_char___GENPT___int_int_snd_config_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT____GENPT___const_char___GENPT___int_int_snd_config_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT____GENPT___const_char___GENPT___int_int_snd_config_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_open_lconf(args.a1, args.a2, args.a3, args.a4, args.a5);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_parse_address
        case snd_seq_parse_address_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_addr_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_addr_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_addr_t___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_parse_address(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_poll_descriptors
        case snd_seq_poll_descriptors_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___struct_pollfd___GENPT___unsigned_int_short *unpacked = (INDEXED_int_snd_seq_t___GENPT___struct_pollfd___GENPT___unsigned_int_short *)packed;
            ARGS_int_snd_seq_t___GENPT___struct_pollfd___GENPT___unsigned_int_short args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_poll_descriptors(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_poll_descriptors_count
        case snd_seq_poll_descriptors_count_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___short *unpacked = (INDEXED_int_snd_seq_t___GENPT___short *)packed;
            ARGS_int_snd_seq_t___GENPT___short args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_poll_descriptors_count(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_poll_descriptors_revents
        case snd_seq_poll_descriptors_revents_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_poll_descriptors_revents(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_copy
        case snd_seq_port_info_copy_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            snd_seq_port_info_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_free
        case snd_seq_port_info_free_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            snd_seq_port_info_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_addr
        case snd_seq_port_info_get_addr_INDEX: {
            INDEXED_const_snd_seq_addr_t___GENPT___const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_const_snd_seq_addr_t___GENPT___const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_const_snd_seq_addr_t___GENPT___const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            const snd_seq_addr_t * *ret = (const snd_seq_addr_t * *)ret_v;
            *ret =
            snd_seq_port_info_get_addr(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_capability
        case snd_seq_port_info_get_capability_INDEX: {
            INDEXED_unsigned_int_const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_seq_port_info_get_capability(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_client
        case snd_seq_port_info_get_client_INDEX: {
            INDEXED_int_const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_info_get_client(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_midi_channels
        case snd_seq_port_info_get_midi_channels_INDEX: {
            INDEXED_int_const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_info_get_midi_channels(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_midi_voices
        case snd_seq_port_info_get_midi_voices_INDEX: {
            INDEXED_int_const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_info_get_midi_voices(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_name
        case snd_seq_port_info_get_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_seq_port_info_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_port
        case snd_seq_port_info_get_port_INDEX: {
            INDEXED_int_const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_info_get_port(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_port_specified
        case snd_seq_port_info_get_port_specified_INDEX: {
            INDEXED_int_const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_info_get_port_specified(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_read_use
        case snd_seq_port_info_get_read_use_INDEX: {
            INDEXED_int_const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_info_get_read_use(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_synth_voices
        case snd_seq_port_info_get_synth_voices_INDEX: {
            INDEXED_int_const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_info_get_synth_voices(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_timestamp_queue
        case snd_seq_port_info_get_timestamp_queue_INDEX: {
            INDEXED_int_const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_info_get_timestamp_queue(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_timestamp_real
        case snd_seq_port_info_get_timestamp_real_INDEX: {
            INDEXED_int_const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_info_get_timestamp_real(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_timestamping
        case snd_seq_port_info_get_timestamping_INDEX: {
            INDEXED_int_const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_info_get_timestamping(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_type
        case snd_seq_port_info_get_type_INDEX: {
            INDEXED_unsigned_int_const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_seq_port_info_get_type(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_get_write_use
        case snd_seq_port_info_get_write_use_INDEX: {
            INDEXED_int_const_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_info_get_write_use(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_malloc
        case snd_seq_port_info_malloc_INDEX: {
            INDEXED_int_snd_seq_port_info_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_seq_port_info_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_seq_port_info_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_info_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_set_addr
        case snd_seq_port_info_set_addr_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___const_snd_seq_addr_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___const_snd_seq_addr_t___GENPT__ *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___const_snd_seq_addr_t___GENPT__ args = unpacked->args;
            snd_seq_port_info_set_addr(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_set_capability
        case snd_seq_port_info_set_capability_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___unsigned_int args = unpacked->args;
            snd_seq_port_info_set_capability(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_set_client
        case snd_seq_port_info_set_client_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___int args = unpacked->args;
            snd_seq_port_info_set_client(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_set_midi_channels
        case snd_seq_port_info_set_midi_channels_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___int args = unpacked->args;
            snd_seq_port_info_set_midi_channels(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_set_midi_voices
        case snd_seq_port_info_set_midi_voices_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___int args = unpacked->args;
            snd_seq_port_info_set_midi_voices(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_set_name
        case snd_seq_port_info_set_name_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___const_char___GENPT__ args = unpacked->args;
            snd_seq_port_info_set_name(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_set_port
        case snd_seq_port_info_set_port_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___int args = unpacked->args;
            snd_seq_port_info_set_port(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_set_port_specified
        case snd_seq_port_info_set_port_specified_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___int args = unpacked->args;
            snd_seq_port_info_set_port_specified(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_set_synth_voices
        case snd_seq_port_info_set_synth_voices_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___int args = unpacked->args;
            snd_seq_port_info_set_synth_voices(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_set_timestamp_queue
        case snd_seq_port_info_set_timestamp_queue_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___int args = unpacked->args;
            snd_seq_port_info_set_timestamp_queue(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_set_timestamp_real
        case snd_seq_port_info_set_timestamp_real_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___int args = unpacked->args;
            snd_seq_port_info_set_timestamp_real(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_set_timestamping
        case snd_seq_port_info_set_timestamping_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___int args = unpacked->args;
            snd_seq_port_info_set_timestamping(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_set_type
        case snd_seq_port_info_set_type_INDEX: {
            INDEXED_void_snd_seq_port_info_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_seq_port_info_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_seq_port_info_t___GENPT___unsigned_int args = unpacked->args;
            snd_seq_port_info_set_type(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_info_sizeof
        case snd_seq_port_info_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_port_info_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_copy
        case snd_seq_port_subscribe_copy_INDEX: {
            INDEXED_void_snd_seq_port_subscribe_t___GENPT___const_snd_seq_port_subscribe_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_port_subscribe_t___GENPT___const_snd_seq_port_subscribe_t___GENPT__ *)packed;
            ARGS_void_snd_seq_port_subscribe_t___GENPT___const_snd_seq_port_subscribe_t___GENPT__ args = unpacked->args;
            snd_seq_port_subscribe_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_free
        case snd_seq_port_subscribe_free_INDEX: {
            INDEXED_void_snd_seq_port_subscribe_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_port_subscribe_t___GENPT__ *)packed;
            ARGS_void_snd_seq_port_subscribe_t___GENPT__ args = unpacked->args;
            snd_seq_port_subscribe_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_get_dest
        case snd_seq_port_subscribe_get_dest_INDEX: {
            INDEXED_const_snd_seq_addr_t___GENPT___const_snd_seq_port_subscribe_t___GENPT__ *unpacked = (INDEXED_const_snd_seq_addr_t___GENPT___const_snd_seq_port_subscribe_t___GENPT__ *)packed;
            ARGS_const_snd_seq_addr_t___GENPT___const_snd_seq_port_subscribe_t___GENPT__ args = unpacked->args;
            const snd_seq_addr_t * *ret = (const snd_seq_addr_t * *)ret_v;
            *ret =
            snd_seq_port_subscribe_get_dest(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_get_exclusive
        case snd_seq_port_subscribe_get_exclusive_INDEX: {
            INDEXED_int_const_snd_seq_port_subscribe_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_subscribe_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_subscribe_get_exclusive(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_get_queue
        case snd_seq_port_subscribe_get_queue_INDEX: {
            INDEXED_int_const_snd_seq_port_subscribe_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_subscribe_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_subscribe_get_queue(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_get_sender
        case snd_seq_port_subscribe_get_sender_INDEX: {
            INDEXED_const_snd_seq_addr_t___GENPT___const_snd_seq_port_subscribe_t___GENPT__ *unpacked = (INDEXED_const_snd_seq_addr_t___GENPT___const_snd_seq_port_subscribe_t___GENPT__ *)packed;
            ARGS_const_snd_seq_addr_t___GENPT___const_snd_seq_port_subscribe_t___GENPT__ args = unpacked->args;
            const snd_seq_addr_t * *ret = (const snd_seq_addr_t * *)ret_v;
            *ret =
            snd_seq_port_subscribe_get_sender(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_get_time_real
        case snd_seq_port_subscribe_get_time_real_INDEX: {
            INDEXED_int_const_snd_seq_port_subscribe_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_subscribe_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_subscribe_get_time_real(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_get_time_update
        case snd_seq_port_subscribe_get_time_update_INDEX: {
            INDEXED_int_const_snd_seq_port_subscribe_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_port_subscribe_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_port_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_subscribe_get_time_update(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_malloc
        case snd_seq_port_subscribe_malloc_INDEX: {
            INDEXED_int_snd_seq_port_subscribe_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_seq_port_subscribe_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_seq_port_subscribe_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_port_subscribe_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_set_dest
        case snd_seq_port_subscribe_set_dest_INDEX: {
            INDEXED_void_snd_seq_port_subscribe_t___GENPT___const_snd_seq_addr_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_port_subscribe_t___GENPT___const_snd_seq_addr_t___GENPT__ *)packed;
            ARGS_void_snd_seq_port_subscribe_t___GENPT___const_snd_seq_addr_t___GENPT__ args = unpacked->args;
            snd_seq_port_subscribe_set_dest(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_set_exclusive
        case snd_seq_port_subscribe_set_exclusive_INDEX: {
            INDEXED_void_snd_seq_port_subscribe_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_port_subscribe_t___GENPT___int *)packed;
            ARGS_void_snd_seq_port_subscribe_t___GENPT___int args = unpacked->args;
            snd_seq_port_subscribe_set_exclusive(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_set_queue
        case snd_seq_port_subscribe_set_queue_INDEX: {
            INDEXED_void_snd_seq_port_subscribe_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_port_subscribe_t___GENPT___int *)packed;
            ARGS_void_snd_seq_port_subscribe_t___GENPT___int args = unpacked->args;
            snd_seq_port_subscribe_set_queue(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_set_sender
        case snd_seq_port_subscribe_set_sender_INDEX: {
            INDEXED_void_snd_seq_port_subscribe_t___GENPT___const_snd_seq_addr_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_port_subscribe_t___GENPT___const_snd_seq_addr_t___GENPT__ *)packed;
            ARGS_void_snd_seq_port_subscribe_t___GENPT___const_snd_seq_addr_t___GENPT__ args = unpacked->args;
            snd_seq_port_subscribe_set_sender(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_set_time_real
        case snd_seq_port_subscribe_set_time_real_INDEX: {
            INDEXED_void_snd_seq_port_subscribe_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_port_subscribe_t___GENPT___int *)packed;
            ARGS_void_snd_seq_port_subscribe_t___GENPT___int args = unpacked->args;
            snd_seq_port_subscribe_set_time_real(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_set_time_update
        case snd_seq_port_subscribe_set_time_update_INDEX: {
            INDEXED_void_snd_seq_port_subscribe_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_port_subscribe_t___GENPT___int *)packed;
            ARGS_void_snd_seq_port_subscribe_t___GENPT___int args = unpacked->args;
            snd_seq_port_subscribe_set_time_update(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_port_subscribe_sizeof
        case snd_seq_port_subscribe_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_port_subscribe_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_named_queue
        case snd_seq_query_named_queue_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_query_named_queue(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_next_client
        case snd_seq_query_next_client_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_client_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_query_next_client(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_next_port
        case snd_seq_query_next_port_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_query_next_port(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_port_subscribers
        case snd_seq_query_port_subscribers_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_query_port_subscribers(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_copy
        case snd_seq_query_subscribe_copy_INDEX: {
            INDEXED_void_snd_seq_query_subscribe_t___GENPT___const_snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_query_subscribe_t___GENPT___const_snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_void_snd_seq_query_subscribe_t___GENPT___const_snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            snd_seq_query_subscribe_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_free
        case snd_seq_query_subscribe_free_INDEX: {
            INDEXED_void_snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_void_snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            snd_seq_query_subscribe_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_get_addr
        case snd_seq_query_subscribe_get_addr_INDEX: {
            INDEXED_const_snd_seq_addr_t___GENPT___const_snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_const_snd_seq_addr_t___GENPT___const_snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_const_snd_seq_addr_t___GENPT___const_snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            const snd_seq_addr_t * *ret = (const snd_seq_addr_t * *)ret_v;
            *ret =
            snd_seq_query_subscribe_get_addr(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_get_client
        case snd_seq_query_subscribe_get_client_INDEX: {
            INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_query_subscribe_get_client(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_get_exclusive
        case snd_seq_query_subscribe_get_exclusive_INDEX: {
            INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_query_subscribe_get_exclusive(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_get_index
        case snd_seq_query_subscribe_get_index_INDEX: {
            INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_query_subscribe_get_index(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_get_num_subs
        case snd_seq_query_subscribe_get_num_subs_INDEX: {
            INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_query_subscribe_get_num_subs(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_get_port
        case snd_seq_query_subscribe_get_port_INDEX: {
            INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_query_subscribe_get_port(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_get_queue
        case snd_seq_query_subscribe_get_queue_INDEX: {
            INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_query_subscribe_get_queue(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_get_root
        case snd_seq_query_subscribe_get_root_INDEX: {
            INDEXED_const_snd_seq_addr_t___GENPT___const_snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_const_snd_seq_addr_t___GENPT___const_snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_const_snd_seq_addr_t___GENPT___const_snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            const snd_seq_addr_t * *ret = (const snd_seq_addr_t * *)ret_v;
            *ret =
            snd_seq_query_subscribe_get_root(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_get_time_real
        case snd_seq_query_subscribe_get_time_real_INDEX: {
            INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_query_subscribe_get_time_real(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_get_time_update
        case snd_seq_query_subscribe_get_time_update_INDEX: {
            INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_query_subscribe_get_time_update(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_get_type
        case snd_seq_query_subscribe_get_type_INDEX: {
            INDEXED_snd_seq_query_subs_type_t_const_snd_seq_query_subscribe_t___GENPT__ *unpacked = (INDEXED_snd_seq_query_subs_type_t_const_snd_seq_query_subscribe_t___GENPT__ *)packed;
            ARGS_snd_seq_query_subs_type_t_const_snd_seq_query_subscribe_t___GENPT__ args = unpacked->args;
            snd_seq_query_subs_type_t *ret = (snd_seq_query_subs_type_t *)ret_v;
            *ret =
            snd_seq_query_subscribe_get_type(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_malloc
        case snd_seq_query_subscribe_malloc_INDEX: {
            INDEXED_int_snd_seq_query_subscribe_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_seq_query_subscribe_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_seq_query_subscribe_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_query_subscribe_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_set_client
        case snd_seq_query_subscribe_set_client_INDEX: {
            INDEXED_void_snd_seq_query_subscribe_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_query_subscribe_t___GENPT___int *)packed;
            ARGS_void_snd_seq_query_subscribe_t___GENPT___int args = unpacked->args;
            snd_seq_query_subscribe_set_client(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_set_index
        case snd_seq_query_subscribe_set_index_INDEX: {
            INDEXED_void_snd_seq_query_subscribe_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_query_subscribe_t___GENPT___int *)packed;
            ARGS_void_snd_seq_query_subscribe_t___GENPT___int args = unpacked->args;
            snd_seq_query_subscribe_set_index(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_set_port
        case snd_seq_query_subscribe_set_port_INDEX: {
            INDEXED_void_snd_seq_query_subscribe_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_query_subscribe_t___GENPT___int *)packed;
            ARGS_void_snd_seq_query_subscribe_t___GENPT___int args = unpacked->args;
            snd_seq_query_subscribe_set_port(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_set_root
        case snd_seq_query_subscribe_set_root_INDEX: {
            INDEXED_void_snd_seq_query_subscribe_t___GENPT___const_snd_seq_addr_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_query_subscribe_t___GENPT___const_snd_seq_addr_t___GENPT__ *)packed;
            ARGS_void_snd_seq_query_subscribe_t___GENPT___const_snd_seq_addr_t___GENPT__ args = unpacked->args;
            snd_seq_query_subscribe_set_root(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_set_type
        case snd_seq_query_subscribe_set_type_INDEX: {
            INDEXED_void_snd_seq_query_subscribe_t___GENPT___snd_seq_query_subs_type_t *unpacked = (INDEXED_void_snd_seq_query_subscribe_t___GENPT___snd_seq_query_subs_type_t *)packed;
            ARGS_void_snd_seq_query_subscribe_t___GENPT___snd_seq_query_subs_type_t args = unpacked->args;
            snd_seq_query_subscribe_set_type(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_query_subscribe_sizeof
        case snd_seq_query_subscribe_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_query_subscribe_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_info_copy
        case snd_seq_queue_info_copy_INDEX: {
            INDEXED_void_snd_seq_queue_info_t___GENPT___const_snd_seq_queue_info_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_queue_info_t___GENPT___const_snd_seq_queue_info_t___GENPT__ *)packed;
            ARGS_void_snd_seq_queue_info_t___GENPT___const_snd_seq_queue_info_t___GENPT__ args = unpacked->args;
            snd_seq_queue_info_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_info_free
        case snd_seq_queue_info_free_INDEX: {
            INDEXED_void_snd_seq_queue_info_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_queue_info_t___GENPT__ *)packed;
            ARGS_void_snd_seq_queue_info_t___GENPT__ args = unpacked->args;
            snd_seq_queue_info_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_info_get_flags
        case snd_seq_queue_info_get_flags_INDEX: {
            INDEXED_unsigned_int_const_snd_seq_queue_info_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_seq_queue_info_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_seq_queue_info_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_seq_queue_info_get_flags(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_info_get_locked
        case snd_seq_queue_info_get_locked_INDEX: {
            INDEXED_int_const_snd_seq_queue_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_queue_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_queue_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_queue_info_get_locked(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_info_get_name
        case snd_seq_queue_info_get_name_INDEX: {
            INDEXED_const_char___GENPT___const_snd_seq_queue_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___const_snd_seq_queue_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___const_snd_seq_queue_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_seq_queue_info_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_info_get_owner
        case snd_seq_queue_info_get_owner_INDEX: {
            INDEXED_int_const_snd_seq_queue_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_queue_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_queue_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_queue_info_get_owner(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_info_get_queue
        case snd_seq_queue_info_get_queue_INDEX: {
            INDEXED_int_const_snd_seq_queue_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_queue_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_queue_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_queue_info_get_queue(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_info_malloc
        case snd_seq_queue_info_malloc_INDEX: {
            INDEXED_int_snd_seq_queue_info_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_seq_queue_info_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_seq_queue_info_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_queue_info_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_info_set_flags
        case snd_seq_queue_info_set_flags_INDEX: {
            INDEXED_void_snd_seq_queue_info_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_seq_queue_info_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_seq_queue_info_t___GENPT___unsigned_int args = unpacked->args;
            snd_seq_queue_info_set_flags(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_info_set_locked
        case snd_seq_queue_info_set_locked_INDEX: {
            INDEXED_void_snd_seq_queue_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_queue_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_queue_info_t___GENPT___int args = unpacked->args;
            snd_seq_queue_info_set_locked(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_info_set_name
        case snd_seq_queue_info_set_name_INDEX: {
            INDEXED_void_snd_seq_queue_info_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_void_snd_seq_queue_info_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_void_snd_seq_queue_info_t___GENPT___const_char___GENPT__ args = unpacked->args;
            snd_seq_queue_info_set_name(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_info_set_owner
        case snd_seq_queue_info_set_owner_INDEX: {
            INDEXED_void_snd_seq_queue_info_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_queue_info_t___GENPT___int *)packed;
            ARGS_void_snd_seq_queue_info_t___GENPT___int args = unpacked->args;
            snd_seq_queue_info_set_owner(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_info_sizeof
        case snd_seq_queue_info_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_queue_info_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_status_copy
        case snd_seq_queue_status_copy_INDEX: {
            INDEXED_void_snd_seq_queue_status_t___GENPT___const_snd_seq_queue_status_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_queue_status_t___GENPT___const_snd_seq_queue_status_t___GENPT__ *)packed;
            ARGS_void_snd_seq_queue_status_t___GENPT___const_snd_seq_queue_status_t___GENPT__ args = unpacked->args;
            snd_seq_queue_status_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_status_free
        case snd_seq_queue_status_free_INDEX: {
            INDEXED_void_snd_seq_queue_status_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_queue_status_t___GENPT__ *)packed;
            ARGS_void_snd_seq_queue_status_t___GENPT__ args = unpacked->args;
            snd_seq_queue_status_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_status_get_events
        case snd_seq_queue_status_get_events_INDEX: {
            INDEXED_int_const_snd_seq_queue_status_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_queue_status_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_queue_status_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_queue_status_get_events(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_status_get_queue
        case snd_seq_queue_status_get_queue_INDEX: {
            INDEXED_int_const_snd_seq_queue_status_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_queue_status_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_queue_status_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_queue_status_get_queue(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_status_get_real_time
        case snd_seq_queue_status_get_real_time_INDEX: {
            INDEXED_const_snd_seq_real_time_t___GENPT___const_snd_seq_queue_status_t___GENPT__ *unpacked = (INDEXED_const_snd_seq_real_time_t___GENPT___const_snd_seq_queue_status_t___GENPT__ *)packed;
            ARGS_const_snd_seq_real_time_t___GENPT___const_snd_seq_queue_status_t___GENPT__ args = unpacked->args;
            const snd_seq_real_time_t * *ret = (const snd_seq_real_time_t * *)ret_v;
            *ret =
            snd_seq_queue_status_get_real_time(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_status_get_status
        case snd_seq_queue_status_get_status_INDEX: {
            INDEXED_unsigned_int_const_snd_seq_queue_status_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_seq_queue_status_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_seq_queue_status_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_seq_queue_status_get_status(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_status_get_tick_time
        case snd_seq_queue_status_get_tick_time_INDEX: {
            INDEXED_snd_seq_tick_time_t_const_snd_seq_queue_status_t___GENPT__ *unpacked = (INDEXED_snd_seq_tick_time_t_const_snd_seq_queue_status_t___GENPT__ *)packed;
            ARGS_snd_seq_tick_time_t_const_snd_seq_queue_status_t___GENPT__ args = unpacked->args;
            snd_seq_tick_time_t *ret = (snd_seq_tick_time_t *)ret_v;
            *ret =
            snd_seq_queue_status_get_tick_time(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_status_malloc
        case snd_seq_queue_status_malloc_INDEX: {
            INDEXED_int_snd_seq_queue_status_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_seq_queue_status_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_seq_queue_status_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_queue_status_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_status_sizeof
        case snd_seq_queue_status_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_queue_status_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_tempo_copy
        case snd_seq_queue_tempo_copy_INDEX: {
            INDEXED_void_snd_seq_queue_tempo_t___GENPT___const_snd_seq_queue_tempo_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_queue_tempo_t___GENPT___const_snd_seq_queue_tempo_t___GENPT__ *)packed;
            ARGS_void_snd_seq_queue_tempo_t___GENPT___const_snd_seq_queue_tempo_t___GENPT__ args = unpacked->args;
            snd_seq_queue_tempo_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_tempo_free
        case snd_seq_queue_tempo_free_INDEX: {
            INDEXED_void_snd_seq_queue_tempo_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_queue_tempo_t___GENPT__ *)packed;
            ARGS_void_snd_seq_queue_tempo_t___GENPT__ args = unpacked->args;
            snd_seq_queue_tempo_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_tempo_get_ppq
        case snd_seq_queue_tempo_get_ppq_INDEX: {
            INDEXED_int_const_snd_seq_queue_tempo_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_queue_tempo_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_queue_tempo_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_queue_tempo_get_ppq(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_tempo_get_queue
        case snd_seq_queue_tempo_get_queue_INDEX: {
            INDEXED_int_const_snd_seq_queue_tempo_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_queue_tempo_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_queue_tempo_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_queue_tempo_get_queue(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_tempo_get_skew
        case snd_seq_queue_tempo_get_skew_INDEX: {
            INDEXED_unsigned_int_const_snd_seq_queue_tempo_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_seq_queue_tempo_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_seq_queue_tempo_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_seq_queue_tempo_get_skew(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_tempo_get_skew_base
        case snd_seq_queue_tempo_get_skew_base_INDEX: {
            INDEXED_unsigned_int_const_snd_seq_queue_tempo_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_seq_queue_tempo_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_seq_queue_tempo_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_seq_queue_tempo_get_skew_base(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_tempo_get_tempo
        case snd_seq_queue_tempo_get_tempo_INDEX: {
            INDEXED_unsigned_int_const_snd_seq_queue_tempo_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_seq_queue_tempo_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_seq_queue_tempo_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_seq_queue_tempo_get_tempo(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_tempo_malloc
        case snd_seq_queue_tempo_malloc_INDEX: {
            INDEXED_int_snd_seq_queue_tempo_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_seq_queue_tempo_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_seq_queue_tempo_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_queue_tempo_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_tempo_set_ppq
        case snd_seq_queue_tempo_set_ppq_INDEX: {
            INDEXED_void_snd_seq_queue_tempo_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_queue_tempo_t___GENPT___int *)packed;
            ARGS_void_snd_seq_queue_tempo_t___GENPT___int args = unpacked->args;
            snd_seq_queue_tempo_set_ppq(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_tempo_set_skew
        case snd_seq_queue_tempo_set_skew_INDEX: {
            INDEXED_void_snd_seq_queue_tempo_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_seq_queue_tempo_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_seq_queue_tempo_t___GENPT___unsigned_int args = unpacked->args;
            snd_seq_queue_tempo_set_skew(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_tempo_set_skew_base
        case snd_seq_queue_tempo_set_skew_base_INDEX: {
            INDEXED_void_snd_seq_queue_tempo_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_seq_queue_tempo_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_seq_queue_tempo_t___GENPT___unsigned_int args = unpacked->args;
            snd_seq_queue_tempo_set_skew_base(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_tempo_set_tempo
        case snd_seq_queue_tempo_set_tempo_INDEX: {
            INDEXED_void_snd_seq_queue_tempo_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_seq_queue_tempo_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_seq_queue_tempo_t___GENPT___unsigned_int args = unpacked->args;
            snd_seq_queue_tempo_set_tempo(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_tempo_sizeof
        case snd_seq_queue_tempo_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_queue_tempo_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_timer_copy
        case snd_seq_queue_timer_copy_INDEX: {
            INDEXED_void_snd_seq_queue_timer_t___GENPT___const_snd_seq_queue_timer_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_queue_timer_t___GENPT___const_snd_seq_queue_timer_t___GENPT__ *)packed;
            ARGS_void_snd_seq_queue_timer_t___GENPT___const_snd_seq_queue_timer_t___GENPT__ args = unpacked->args;
            snd_seq_queue_timer_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_timer_free
        case snd_seq_queue_timer_free_INDEX: {
            INDEXED_void_snd_seq_queue_timer_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_queue_timer_t___GENPT__ *)packed;
            ARGS_void_snd_seq_queue_timer_t___GENPT__ args = unpacked->args;
            snd_seq_queue_timer_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_timer_get_id
        case snd_seq_queue_timer_get_id_INDEX: {
            INDEXED_const_snd_timer_id_t___GENPT___const_snd_seq_queue_timer_t___GENPT__ *unpacked = (INDEXED_const_snd_timer_id_t___GENPT___const_snd_seq_queue_timer_t___GENPT__ *)packed;
            ARGS_const_snd_timer_id_t___GENPT___const_snd_seq_queue_timer_t___GENPT__ args = unpacked->args;
            const snd_timer_id_t * *ret = (const snd_timer_id_t * *)ret_v;
            *ret =
            snd_seq_queue_timer_get_id(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_timer_get_queue
        case snd_seq_queue_timer_get_queue_INDEX: {
            INDEXED_int_const_snd_seq_queue_timer_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_queue_timer_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_queue_timer_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_queue_timer_get_queue(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_timer_get_resolution
        case snd_seq_queue_timer_get_resolution_INDEX: {
            INDEXED_unsigned_int_const_snd_seq_queue_timer_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_seq_queue_timer_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_seq_queue_timer_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_seq_queue_timer_get_resolution(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_timer_get_type
        case snd_seq_queue_timer_get_type_INDEX: {
            INDEXED_snd_seq_queue_timer_type_t_const_snd_seq_queue_timer_t___GENPT__ *unpacked = (INDEXED_snd_seq_queue_timer_type_t_const_snd_seq_queue_timer_t___GENPT__ *)packed;
            ARGS_snd_seq_queue_timer_type_t_const_snd_seq_queue_timer_t___GENPT__ args = unpacked->args;
            snd_seq_queue_timer_type_t *ret = (snd_seq_queue_timer_type_t *)ret_v;
            *ret =
            snd_seq_queue_timer_get_type(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_timer_malloc
        case snd_seq_queue_timer_malloc_INDEX: {
            INDEXED_int_snd_seq_queue_timer_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_seq_queue_timer_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_seq_queue_timer_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_queue_timer_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_timer_set_id
        case snd_seq_queue_timer_set_id_INDEX: {
            INDEXED_void_snd_seq_queue_timer_t___GENPT___const_snd_timer_id_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_queue_timer_t___GENPT___const_snd_timer_id_t___GENPT__ *)packed;
            ARGS_void_snd_seq_queue_timer_t___GENPT___const_snd_timer_id_t___GENPT__ args = unpacked->args;
            snd_seq_queue_timer_set_id(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_timer_set_resolution
        case snd_seq_queue_timer_set_resolution_INDEX: {
            INDEXED_void_snd_seq_queue_timer_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_seq_queue_timer_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_seq_queue_timer_t___GENPT___unsigned_int args = unpacked->args;
            snd_seq_queue_timer_set_resolution(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_timer_set_type
        case snd_seq_queue_timer_set_type_INDEX: {
            INDEXED_void_snd_seq_queue_timer_t___GENPT___snd_seq_queue_timer_type_t *unpacked = (INDEXED_void_snd_seq_queue_timer_t___GENPT___snd_seq_queue_timer_type_t *)packed;
            ARGS_void_snd_seq_queue_timer_t___GENPT___snd_seq_queue_timer_type_t args = unpacked->args;
            snd_seq_queue_timer_set_type(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_queue_timer_sizeof
        case snd_seq_queue_timer_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_queue_timer_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events
        case snd_seq_remove_events_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_remove_events_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_remove_events_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_remove_events_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_remove_events(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_copy
        case snd_seq_remove_events_copy_INDEX: {
            INDEXED_void_snd_seq_remove_events_t___GENPT___const_snd_seq_remove_events_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_remove_events_t___GENPT___const_snd_seq_remove_events_t___GENPT__ *)packed;
            ARGS_void_snd_seq_remove_events_t___GENPT___const_snd_seq_remove_events_t___GENPT__ args = unpacked->args;
            snd_seq_remove_events_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_free
        case snd_seq_remove_events_free_INDEX: {
            INDEXED_void_snd_seq_remove_events_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_remove_events_t___GENPT__ *)packed;
            ARGS_void_snd_seq_remove_events_t___GENPT__ args = unpacked->args;
            snd_seq_remove_events_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_get_channel
        case snd_seq_remove_events_get_channel_INDEX: {
            INDEXED_int_const_snd_seq_remove_events_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_remove_events_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_remove_events_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_remove_events_get_channel(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_get_condition
        case snd_seq_remove_events_get_condition_INDEX: {
            INDEXED_unsigned_int_const_snd_seq_remove_events_t___GENPT__ *unpacked = (INDEXED_unsigned_int_const_snd_seq_remove_events_t___GENPT__ *)packed;
            ARGS_unsigned_int_const_snd_seq_remove_events_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_seq_remove_events_get_condition(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_get_dest
        case snd_seq_remove_events_get_dest_INDEX: {
            INDEXED_const_snd_seq_addr_t___GENPT___const_snd_seq_remove_events_t___GENPT__ *unpacked = (INDEXED_const_snd_seq_addr_t___GENPT___const_snd_seq_remove_events_t___GENPT__ *)packed;
            ARGS_const_snd_seq_addr_t___GENPT___const_snd_seq_remove_events_t___GENPT__ args = unpacked->args;
            const snd_seq_addr_t * *ret = (const snd_seq_addr_t * *)ret_v;
            *ret =
            snd_seq_remove_events_get_dest(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_get_event_type
        case snd_seq_remove_events_get_event_type_INDEX: {
            INDEXED_int_const_snd_seq_remove_events_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_remove_events_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_remove_events_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_remove_events_get_event_type(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_get_queue
        case snd_seq_remove_events_get_queue_INDEX: {
            INDEXED_int_const_snd_seq_remove_events_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_remove_events_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_remove_events_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_remove_events_get_queue(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_get_tag
        case snd_seq_remove_events_get_tag_INDEX: {
            INDEXED_int_const_snd_seq_remove_events_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_remove_events_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_remove_events_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_remove_events_get_tag(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_get_time
        case snd_seq_remove_events_get_time_INDEX: {
            INDEXED_const_snd_seq_timestamp_t___GENPT___const_snd_seq_remove_events_t___GENPT__ *unpacked = (INDEXED_const_snd_seq_timestamp_t___GENPT___const_snd_seq_remove_events_t___GENPT__ *)packed;
            ARGS_const_snd_seq_timestamp_t___GENPT___const_snd_seq_remove_events_t___GENPT__ args = unpacked->args;
            const snd_seq_timestamp_t * *ret = (const snd_seq_timestamp_t * *)ret_v;
            *ret =
            snd_seq_remove_events_get_time(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_malloc
        case snd_seq_remove_events_malloc_INDEX: {
            INDEXED_int_snd_seq_remove_events_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_seq_remove_events_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_seq_remove_events_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_remove_events_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_set_channel
        case snd_seq_remove_events_set_channel_INDEX: {
            INDEXED_void_snd_seq_remove_events_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_remove_events_t___GENPT___int *)packed;
            ARGS_void_snd_seq_remove_events_t___GENPT___int args = unpacked->args;
            snd_seq_remove_events_set_channel(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_set_condition
        case snd_seq_remove_events_set_condition_INDEX: {
            INDEXED_void_snd_seq_remove_events_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_seq_remove_events_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_seq_remove_events_t___GENPT___unsigned_int args = unpacked->args;
            snd_seq_remove_events_set_condition(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_set_dest
        case snd_seq_remove_events_set_dest_INDEX: {
            INDEXED_void_snd_seq_remove_events_t___GENPT___const_snd_seq_addr_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_remove_events_t___GENPT___const_snd_seq_addr_t___GENPT__ *)packed;
            ARGS_void_snd_seq_remove_events_t___GENPT___const_snd_seq_addr_t___GENPT__ args = unpacked->args;
            snd_seq_remove_events_set_dest(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_set_event_type
        case snd_seq_remove_events_set_event_type_INDEX: {
            INDEXED_void_snd_seq_remove_events_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_remove_events_t___GENPT___int *)packed;
            ARGS_void_snd_seq_remove_events_t___GENPT___int args = unpacked->args;
            snd_seq_remove_events_set_event_type(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_set_queue
        case snd_seq_remove_events_set_queue_INDEX: {
            INDEXED_void_snd_seq_remove_events_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_remove_events_t___GENPT___int *)packed;
            ARGS_void_snd_seq_remove_events_t___GENPT___int args = unpacked->args;
            snd_seq_remove_events_set_queue(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_set_tag
        case snd_seq_remove_events_set_tag_INDEX: {
            INDEXED_void_snd_seq_remove_events_t___GENPT___int *unpacked = (INDEXED_void_snd_seq_remove_events_t___GENPT___int *)packed;
            ARGS_void_snd_seq_remove_events_t___GENPT___int args = unpacked->args;
            snd_seq_remove_events_set_tag(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_set_time
        case snd_seq_remove_events_set_time_INDEX: {
            INDEXED_void_snd_seq_remove_events_t___GENPT___const_snd_seq_timestamp_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_remove_events_t___GENPT___const_snd_seq_timestamp_t___GENPT__ *)packed;
            ARGS_void_snd_seq_remove_events_t___GENPT___const_snd_seq_timestamp_t___GENPT__ args = unpacked->args;
            snd_seq_remove_events_set_time(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_remove_events_sizeof
        case snd_seq_remove_events_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_remove_events_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_reset_pool_input
        case snd_seq_reset_pool_input_INDEX: {
            INDEXED_int_snd_seq_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_reset_pool_input(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_reset_pool_output
        case snd_seq_reset_pool_output_INDEX: {
            INDEXED_int_snd_seq_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_reset_pool_output(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_bit
        case snd_seq_set_bit_INDEX: {
            INDEXED_void_int_void___GENPT__ *unpacked = (INDEXED_void_int_void___GENPT__ *)packed;
            ARGS_void_int_void___GENPT__ args = unpacked->args;
            snd_seq_set_bit(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_client_event_filter
        case snd_seq_set_client_event_filter_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int *unpacked = (INDEXED_int_snd_seq_t___GENPT___int *)packed;
            ARGS_int_snd_seq_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_set_client_event_filter(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_client_info
        case snd_seq_set_client_info_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_client_info_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_client_info_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_client_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_set_client_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_client_name
        case snd_seq_set_client_name_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___const_char___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___const_char___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___const_char___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_set_client_name(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_client_pool
        case snd_seq_set_client_pool_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_client_pool_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_client_pool_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_client_pool_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_set_client_pool(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_client_pool_input
        case snd_seq_set_client_pool_input_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___size_t *unpacked = (INDEXED_int_snd_seq_t___GENPT___size_t *)packed;
            ARGS_int_snd_seq_t___GENPT___size_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_set_client_pool_input(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_client_pool_output_room
        case snd_seq_set_client_pool_output_room_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___size_t *unpacked = (INDEXED_int_snd_seq_t___GENPT___size_t *)packed;
            ARGS_int_snd_seq_t___GENPT___size_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_set_client_pool_output_room(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_input_buffer_size
        case snd_seq_set_input_buffer_size_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___size_t *unpacked = (INDEXED_int_snd_seq_t___GENPT___size_t *)packed;
            ARGS_int_snd_seq_t___GENPT___size_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_set_input_buffer_size(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_output_buffer_size
        case snd_seq_set_output_buffer_size_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___size_t *unpacked = (INDEXED_int_snd_seq_t___GENPT___size_t *)packed;
            ARGS_int_snd_seq_t___GENPT___size_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_set_output_buffer_size(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_port_info
        case snd_seq_set_port_info_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_snd_seq_port_info_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_snd_seq_port_info_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___int_snd_seq_port_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_set_port_info(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_queue_info
        case snd_seq_set_queue_info_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_info_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_info_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___int_snd_seq_queue_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_set_queue_info(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_queue_tempo
        case snd_seq_set_queue_tempo_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_tempo_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_tempo_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___int_snd_seq_queue_tempo_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_set_queue_tempo(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_queue_timer
        case snd_seq_set_queue_timer_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_timer_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_snd_seq_queue_timer_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___int_snd_seq_queue_timer_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_set_queue_timer(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_set_queue_usage
        case snd_seq_set_queue_usage_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___int_int *unpacked = (INDEXED_int_snd_seq_t___GENPT___int_int *)packed;
            ARGS_int_snd_seq_t___GENPT___int_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_set_queue_usage(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_subscribe_port
        case snd_seq_subscribe_port_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_port_subscribe_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_port_subscribe_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_port_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_subscribe_port(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_sync_output_queue
        case snd_seq_sync_output_queue_INDEX: {
            INDEXED_int_snd_seq_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_sync_output_queue(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_system_info
        case snd_seq_system_info_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_system_info_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_system_info_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_system_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_system_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_system_info_copy
        case snd_seq_system_info_copy_INDEX: {
            INDEXED_void_snd_seq_system_info_t___GENPT___const_snd_seq_system_info_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_system_info_t___GENPT___const_snd_seq_system_info_t___GENPT__ *)packed;
            ARGS_void_snd_seq_system_info_t___GENPT___const_snd_seq_system_info_t___GENPT__ args = unpacked->args;
            snd_seq_system_info_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_system_info_free
        case snd_seq_system_info_free_INDEX: {
            INDEXED_void_snd_seq_system_info_t___GENPT__ *unpacked = (INDEXED_void_snd_seq_system_info_t___GENPT__ *)packed;
            ARGS_void_snd_seq_system_info_t___GENPT__ args = unpacked->args;
            snd_seq_system_info_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_system_info_get_channels
        case snd_seq_system_info_get_channels_INDEX: {
            INDEXED_int_const_snd_seq_system_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_system_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_system_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_system_info_get_channels(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_system_info_get_clients
        case snd_seq_system_info_get_clients_INDEX: {
            INDEXED_int_const_snd_seq_system_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_system_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_system_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_system_info_get_clients(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_system_info_get_cur_clients
        case snd_seq_system_info_get_cur_clients_INDEX: {
            INDEXED_int_const_snd_seq_system_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_system_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_system_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_system_info_get_cur_clients(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_system_info_get_cur_queues
        case snd_seq_system_info_get_cur_queues_INDEX: {
            INDEXED_int_const_snd_seq_system_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_system_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_system_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_system_info_get_cur_queues(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_system_info_get_ports
        case snd_seq_system_info_get_ports_INDEX: {
            INDEXED_int_const_snd_seq_system_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_system_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_system_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_system_info_get_ports(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_system_info_get_queues
        case snd_seq_system_info_get_queues_INDEX: {
            INDEXED_int_const_snd_seq_system_info_t___GENPT__ *unpacked = (INDEXED_int_const_snd_seq_system_info_t___GENPT__ *)packed;
            ARGS_int_const_snd_seq_system_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_system_info_get_queues(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_system_info_malloc
        case snd_seq_system_info_malloc_INDEX: {
            INDEXED_int_snd_seq_system_info_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_seq_system_info_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_seq_system_info_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_system_info_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_system_info_sizeof
        case snd_seq_system_info_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_seq_system_info_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_type
        case snd_seq_type_INDEX: {
            INDEXED_snd_seq_type_t_snd_seq_t___GENPT__ *unpacked = (INDEXED_snd_seq_type_t_snd_seq_t___GENPT__ *)packed;
            ARGS_snd_seq_type_t_snd_seq_t___GENPT__ args = unpacked->args;
            snd_seq_type_t *ret = (snd_seq_type_t *)ret_v;
            *ret =
            snd_seq_type(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_seq_unsubscribe_port
        case snd_seq_unsubscribe_port_INDEX: {
            INDEXED_int_snd_seq_t___GENPT___snd_seq_port_subscribe_t___GENPT__ *unpacked = (INDEXED_int_snd_seq_t___GENPT___snd_seq_port_subscribe_t___GENPT__ *)packed;
            ARGS_int_snd_seq_t___GENPT___snd_seq_port_subscribe_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_seq_unsubscribe_port(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_shm_area_create
        case snd_shm_area_create_INDEX: {
            INDEXED_struct_snd_shm_area___GENPT___int_void___GENPT__ *unpacked = (INDEXED_struct_snd_shm_area___GENPT___int_void___GENPT__ *)packed;
            ARGS_struct_snd_shm_area___GENPT___int_void___GENPT__ args = unpacked->args;
            struct snd_shm_area * *ret = (struct snd_shm_area * *)ret_v;
            *ret =
            snd_shm_area_create(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_shm_area_destroy
        case snd_shm_area_destroy_INDEX: {
            INDEXED_int_struct_snd_shm_area___GENPT__ *unpacked = (INDEXED_int_struct_snd_shm_area___GENPT__ *)packed;
            ARGS_int_struct_snd_shm_area___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_shm_area_destroy(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_shm_area_share
        case snd_shm_area_share_INDEX: {
            INDEXED_struct_snd_shm_area___GENPT___struct_snd_shm_area___GENPT__ *unpacked = (INDEXED_struct_snd_shm_area___GENPT___struct_snd_shm_area___GENPT__ *)packed;
            ARGS_struct_snd_shm_area___GENPT___struct_snd_shm_area___GENPT__ args = unpacked->args;
            struct snd_shm_area * *ret = (struct snd_shm_area * *)ret_v;
            *ret =
            snd_shm_area_share(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_spcm_init
        case snd_spcm_init_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___unsigned_int_unsigned_int_snd_pcm_format_t_snd_pcm_subformat_t_snd_spcm_latency_t_snd_pcm_access_t_snd_spcm_xrun_type_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___unsigned_int_unsigned_int_snd_pcm_format_t_snd_pcm_subformat_t_snd_spcm_latency_t_snd_pcm_access_t_snd_spcm_xrun_type_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___unsigned_int_unsigned_int_snd_pcm_format_t_snd_pcm_subformat_t_snd_spcm_latency_t_snd_pcm_access_t_snd_spcm_xrun_type_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_spcm_init(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8);
            break;
        }
        #endif
        #ifndef skip_index_snd_spcm_init_duplex
        case snd_spcm_init_duplex_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___snd_pcm_t___GENPT___unsigned_int_unsigned_int_snd_pcm_format_t_snd_pcm_subformat_t_snd_spcm_latency_t_snd_pcm_access_t_snd_spcm_xrun_type_t_snd_spcm_duplex_type_t *unpacked = (INDEXED_int_snd_pcm_t___GENPT___snd_pcm_t___GENPT___unsigned_int_unsigned_int_snd_pcm_format_t_snd_pcm_subformat_t_snd_spcm_latency_t_snd_pcm_access_t_snd_spcm_xrun_type_t_snd_spcm_duplex_type_t *)packed;
            ARGS_int_snd_pcm_t___GENPT___snd_pcm_t___GENPT___unsigned_int_unsigned_int_snd_pcm_format_t_snd_pcm_subformat_t_snd_spcm_latency_t_snd_pcm_access_t_snd_spcm_xrun_type_t_snd_spcm_duplex_type_t args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_spcm_init_duplex(args.a1, args.a2, args.a3, args.a4, args.a5, args.a6, args.a7, args.a8, args.a9, args.a10);
            break;
        }
        #endif
        #ifndef skip_index_snd_spcm_init_get_params
        case snd_spcm_init_get_params_INDEX: {
            INDEXED_int_snd_pcm_t___GENPT___unsigned_int___GENPT___snd_pcm_uframes_t___GENPT___snd_pcm_uframes_t___GENPT__ *unpacked = (INDEXED_int_snd_pcm_t___GENPT___unsigned_int___GENPT___snd_pcm_uframes_t___GENPT___snd_pcm_uframes_t___GENPT__ *)packed;
            ARGS_int_snd_pcm_t___GENPT___unsigned_int___GENPT___snd_pcm_uframes_t___GENPT___snd_pcm_uframes_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_spcm_init_get_params(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_strerror
        case snd_strerror_INDEX: {
            INDEXED_const_char___GENPT___int *unpacked = (INDEXED_const_char___GENPT___int *)packed;
            ARGS_const_char___GENPT___int args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_strerror(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_close
        case snd_timer_close_INDEX: {
            INDEXED_int_snd_timer_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_t___GENPT__ *)packed;
            ARGS_int_snd_timer_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_close(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_continue
        case snd_timer_continue_INDEX: {
            INDEXED_int_snd_timer_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_t___GENPT__ *)packed;
            ARGS_int_snd_timer_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_continue(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_copy
        case snd_timer_ginfo_copy_INDEX: {
            INDEXED_void_snd_timer_ginfo_t___GENPT___const_snd_timer_ginfo_t___GENPT__ *unpacked = (INDEXED_void_snd_timer_ginfo_t___GENPT___const_snd_timer_ginfo_t___GENPT__ *)packed;
            ARGS_void_snd_timer_ginfo_t___GENPT___const_snd_timer_ginfo_t___GENPT__ args = unpacked->args;
            snd_timer_ginfo_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_free
        case snd_timer_ginfo_free_INDEX: {
            INDEXED_void_snd_timer_ginfo_t___GENPT__ *unpacked = (INDEXED_void_snd_timer_ginfo_t___GENPT__ *)packed;
            ARGS_void_snd_timer_ginfo_t___GENPT__ args = unpacked->args;
            snd_timer_ginfo_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_get_card
        case snd_timer_ginfo_get_card_INDEX: {
            INDEXED_int_snd_timer_ginfo_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_ginfo_t___GENPT__ *)packed;
            ARGS_int_snd_timer_ginfo_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_ginfo_get_card(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_get_clients
        case snd_timer_ginfo_get_clients_INDEX: {
            INDEXED_unsigned_int_snd_timer_ginfo_t___GENPT__ *unpacked = (INDEXED_unsigned_int_snd_timer_ginfo_t___GENPT__ *)packed;
            ARGS_unsigned_int_snd_timer_ginfo_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_timer_ginfo_get_clients(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_get_flags
        case snd_timer_ginfo_get_flags_INDEX: {
            INDEXED_unsigned_int_snd_timer_ginfo_t___GENPT__ *unpacked = (INDEXED_unsigned_int_snd_timer_ginfo_t___GENPT__ *)packed;
            ARGS_unsigned_int_snd_timer_ginfo_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_timer_ginfo_get_flags(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_get_id
        case snd_timer_ginfo_get_id_INDEX: {
            INDEXED_char___GENPT___snd_timer_ginfo_t___GENPT__ *unpacked = (INDEXED_char___GENPT___snd_timer_ginfo_t___GENPT__ *)packed;
            ARGS_char___GENPT___snd_timer_ginfo_t___GENPT__ args = unpacked->args;
            char * *ret = (char * *)ret_v;
            *ret =
            snd_timer_ginfo_get_id(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_get_name
        case snd_timer_ginfo_get_name_INDEX: {
            INDEXED_char___GENPT___snd_timer_ginfo_t___GENPT__ *unpacked = (INDEXED_char___GENPT___snd_timer_ginfo_t___GENPT__ *)packed;
            ARGS_char___GENPT___snd_timer_ginfo_t___GENPT__ args = unpacked->args;
            char * *ret = (char * *)ret_v;
            *ret =
            snd_timer_ginfo_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_get_resolution
        case snd_timer_ginfo_get_resolution_INDEX: {
            INDEXED_unsigned_long_snd_timer_ginfo_t___GENPT__ *unpacked = (INDEXED_unsigned_long_snd_timer_ginfo_t___GENPT__ *)packed;
            ARGS_unsigned_long_snd_timer_ginfo_t___GENPT__ args = unpacked->args;
            unsigned long *ret = (unsigned long *)ret_v;
            *ret =
            snd_timer_ginfo_get_resolution(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_get_resolution_max
        case snd_timer_ginfo_get_resolution_max_INDEX: {
            INDEXED_unsigned_long_snd_timer_ginfo_t___GENPT__ *unpacked = (INDEXED_unsigned_long_snd_timer_ginfo_t___GENPT__ *)packed;
            ARGS_unsigned_long_snd_timer_ginfo_t___GENPT__ args = unpacked->args;
            unsigned long *ret = (unsigned long *)ret_v;
            *ret =
            snd_timer_ginfo_get_resolution_max(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_get_resolution_min
        case snd_timer_ginfo_get_resolution_min_INDEX: {
            INDEXED_unsigned_long_snd_timer_ginfo_t___GENPT__ *unpacked = (INDEXED_unsigned_long_snd_timer_ginfo_t___GENPT__ *)packed;
            ARGS_unsigned_long_snd_timer_ginfo_t___GENPT__ args = unpacked->args;
            unsigned long *ret = (unsigned long *)ret_v;
            *ret =
            snd_timer_ginfo_get_resolution_min(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_get_tid
        case snd_timer_ginfo_get_tid_INDEX: {
            INDEXED_snd_timer_id_t___GENPT___snd_timer_ginfo_t___GENPT__ *unpacked = (INDEXED_snd_timer_id_t___GENPT___snd_timer_ginfo_t___GENPT__ *)packed;
            ARGS_snd_timer_id_t___GENPT___snd_timer_ginfo_t___GENPT__ args = unpacked->args;
            snd_timer_id_t * *ret = (snd_timer_id_t * *)ret_v;
            *ret =
            snd_timer_ginfo_get_tid(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_malloc
        case snd_timer_ginfo_malloc_INDEX: {
            INDEXED_int_snd_timer_ginfo_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_timer_ginfo_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_timer_ginfo_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_ginfo_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_set_tid
        case snd_timer_ginfo_set_tid_INDEX: {
            INDEXED_int_snd_timer_ginfo_t___GENPT___snd_timer_id_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_ginfo_t___GENPT___snd_timer_id_t___GENPT__ *)packed;
            ARGS_int_snd_timer_ginfo_t___GENPT___snd_timer_id_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_ginfo_set_tid(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_ginfo_sizeof
        case snd_timer_ginfo_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_timer_ginfo_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_copy
        case snd_timer_id_copy_INDEX: {
            INDEXED_void_snd_timer_id_t___GENPT___const_snd_timer_id_t___GENPT__ *unpacked = (INDEXED_void_snd_timer_id_t___GENPT___const_snd_timer_id_t___GENPT__ *)packed;
            ARGS_void_snd_timer_id_t___GENPT___const_snd_timer_id_t___GENPT__ args = unpacked->args;
            snd_timer_id_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_free
        case snd_timer_id_free_INDEX: {
            INDEXED_void_snd_timer_id_t___GENPT__ *unpacked = (INDEXED_void_snd_timer_id_t___GENPT__ *)packed;
            ARGS_void_snd_timer_id_t___GENPT__ args = unpacked->args;
            snd_timer_id_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_get_card
        case snd_timer_id_get_card_INDEX: {
            INDEXED_int_snd_timer_id_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_id_t___GENPT__ *)packed;
            ARGS_int_snd_timer_id_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_id_get_card(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_get_class
        case snd_timer_id_get_class_INDEX: {
            INDEXED_int_snd_timer_id_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_id_t___GENPT__ *)packed;
            ARGS_int_snd_timer_id_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_id_get_class(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_get_device
        case snd_timer_id_get_device_INDEX: {
            INDEXED_int_snd_timer_id_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_id_t___GENPT__ *)packed;
            ARGS_int_snd_timer_id_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_id_get_device(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_get_sclass
        case snd_timer_id_get_sclass_INDEX: {
            INDEXED_int_snd_timer_id_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_id_t___GENPT__ *)packed;
            ARGS_int_snd_timer_id_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_id_get_sclass(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_get_subdevice
        case snd_timer_id_get_subdevice_INDEX: {
            INDEXED_int_snd_timer_id_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_id_t___GENPT__ *)packed;
            ARGS_int_snd_timer_id_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_id_get_subdevice(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_malloc
        case snd_timer_id_malloc_INDEX: {
            INDEXED_int_snd_timer_id_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_timer_id_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_timer_id_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_id_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_set_card
        case snd_timer_id_set_card_INDEX: {
            INDEXED_void_snd_timer_id_t___GENPT___int *unpacked = (INDEXED_void_snd_timer_id_t___GENPT___int *)packed;
            ARGS_void_snd_timer_id_t___GENPT___int args = unpacked->args;
            snd_timer_id_set_card(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_set_class
        case snd_timer_id_set_class_INDEX: {
            INDEXED_void_snd_timer_id_t___GENPT___int *unpacked = (INDEXED_void_snd_timer_id_t___GENPT___int *)packed;
            ARGS_void_snd_timer_id_t___GENPT___int args = unpacked->args;
            snd_timer_id_set_class(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_set_device
        case snd_timer_id_set_device_INDEX: {
            INDEXED_void_snd_timer_id_t___GENPT___int *unpacked = (INDEXED_void_snd_timer_id_t___GENPT___int *)packed;
            ARGS_void_snd_timer_id_t___GENPT___int args = unpacked->args;
            snd_timer_id_set_device(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_set_sclass
        case snd_timer_id_set_sclass_INDEX: {
            INDEXED_void_snd_timer_id_t___GENPT___int *unpacked = (INDEXED_void_snd_timer_id_t___GENPT___int *)packed;
            ARGS_void_snd_timer_id_t___GENPT___int args = unpacked->args;
            snd_timer_id_set_sclass(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_set_subdevice
        case snd_timer_id_set_subdevice_INDEX: {
            INDEXED_void_snd_timer_id_t___GENPT___int *unpacked = (INDEXED_void_snd_timer_id_t___GENPT___int *)packed;
            ARGS_void_snd_timer_id_t___GENPT___int args = unpacked->args;
            snd_timer_id_set_subdevice(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_id_sizeof
        case snd_timer_id_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_timer_id_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_info
        case snd_timer_info_INDEX: {
            INDEXED_int_snd_timer_t___GENPT___snd_timer_info_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_t___GENPT___snd_timer_info_t___GENPT__ *)packed;
            ARGS_int_snd_timer_t___GENPT___snd_timer_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_info_copy
        case snd_timer_info_copy_INDEX: {
            INDEXED_void_snd_timer_info_t___GENPT___const_snd_timer_info_t___GENPT__ *unpacked = (INDEXED_void_snd_timer_info_t___GENPT___const_snd_timer_info_t___GENPT__ *)packed;
            ARGS_void_snd_timer_info_t___GENPT___const_snd_timer_info_t___GENPT__ args = unpacked->args;
            snd_timer_info_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_info_free
        case snd_timer_info_free_INDEX: {
            INDEXED_void_snd_timer_info_t___GENPT__ *unpacked = (INDEXED_void_snd_timer_info_t___GENPT__ *)packed;
            ARGS_void_snd_timer_info_t___GENPT__ args = unpacked->args;
            snd_timer_info_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_info_get_card
        case snd_timer_info_get_card_INDEX: {
            INDEXED_int_snd_timer_info_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_info_t___GENPT__ *)packed;
            ARGS_int_snd_timer_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_info_get_card(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_info_get_id
        case snd_timer_info_get_id_INDEX: {
            INDEXED_const_char___GENPT___snd_timer_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___snd_timer_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___snd_timer_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_timer_info_get_id(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_info_get_name
        case snd_timer_info_get_name_INDEX: {
            INDEXED_const_char___GENPT___snd_timer_info_t___GENPT__ *unpacked = (INDEXED_const_char___GENPT___snd_timer_info_t___GENPT__ *)packed;
            ARGS_const_char___GENPT___snd_timer_info_t___GENPT__ args = unpacked->args;
            const char * *ret = (const char * *)ret_v;
            *ret =
            snd_timer_info_get_name(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_info_get_resolution
        case snd_timer_info_get_resolution_INDEX: {
            INDEXED_long_snd_timer_info_t___GENPT__ *unpacked = (INDEXED_long_snd_timer_info_t___GENPT__ *)packed;
            ARGS_long_snd_timer_info_t___GENPT__ args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_timer_info_get_resolution(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_info_get_ticks
        case snd_timer_info_get_ticks_INDEX: {
            INDEXED_long_snd_timer_info_t___GENPT__ *unpacked = (INDEXED_long_snd_timer_info_t___GENPT__ *)packed;
            ARGS_long_snd_timer_info_t___GENPT__ args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_timer_info_get_ticks(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_info_is_slave
        case snd_timer_info_is_slave_INDEX: {
            INDEXED_int_snd_timer_info_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_info_t___GENPT__ *)packed;
            ARGS_int_snd_timer_info_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_info_is_slave(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_info_malloc
        case snd_timer_info_malloc_INDEX: {
            INDEXED_int_snd_timer_info_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_timer_info_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_timer_info_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_info_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_info_sizeof
        case snd_timer_info_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_timer_info_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_open
        case snd_timer_open_INDEX: {
            INDEXED_int_snd_timer_t___GENPT____GENPT___const_char___GENPT___int *unpacked = (INDEXED_int_snd_timer_t___GENPT____GENPT___const_char___GENPT___int *)packed;
            ARGS_int_snd_timer_t___GENPT____GENPT___const_char___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_open(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_open_lconf
        case snd_timer_open_lconf_INDEX: {
            INDEXED_int_snd_timer_t___GENPT____GENPT___const_char___GENPT___int_snd_config_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_t___GENPT____GENPT___const_char___GENPT___int_snd_config_t___GENPT__ *)packed;
            ARGS_int_snd_timer_t___GENPT____GENPT___const_char___GENPT___int_snd_config_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_open_lconf(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params
        case snd_timer_params_INDEX: {
            INDEXED_int_snd_timer_t___GENPT___snd_timer_params_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_t___GENPT___snd_timer_params_t___GENPT__ *)packed;
            ARGS_int_snd_timer_t___GENPT___snd_timer_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_params(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_copy
        case snd_timer_params_copy_INDEX: {
            INDEXED_void_snd_timer_params_t___GENPT___const_snd_timer_params_t___GENPT__ *unpacked = (INDEXED_void_snd_timer_params_t___GENPT___const_snd_timer_params_t___GENPT__ *)packed;
            ARGS_void_snd_timer_params_t___GENPT___const_snd_timer_params_t___GENPT__ args = unpacked->args;
            snd_timer_params_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_free
        case snd_timer_params_free_INDEX: {
            INDEXED_void_snd_timer_params_t___GENPT__ *unpacked = (INDEXED_void_snd_timer_params_t___GENPT__ *)packed;
            ARGS_void_snd_timer_params_t___GENPT__ args = unpacked->args;
            snd_timer_params_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_get_auto_start
        case snd_timer_params_get_auto_start_INDEX: {
            INDEXED_int_snd_timer_params_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_params_t___GENPT__ *)packed;
            ARGS_int_snd_timer_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_params_get_auto_start(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_get_early_event
        case snd_timer_params_get_early_event_INDEX: {
            INDEXED_int_snd_timer_params_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_params_t___GENPT__ *)packed;
            ARGS_int_snd_timer_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_params_get_early_event(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_get_exclusive
        case snd_timer_params_get_exclusive_INDEX: {
            INDEXED_int_snd_timer_params_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_params_t___GENPT__ *)packed;
            ARGS_int_snd_timer_params_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_params_get_exclusive(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_get_filter
        case snd_timer_params_get_filter_INDEX: {
            INDEXED_unsigned_int_snd_timer_params_t___GENPT__ *unpacked = (INDEXED_unsigned_int_snd_timer_params_t___GENPT__ *)packed;
            ARGS_unsigned_int_snd_timer_params_t___GENPT__ args = unpacked->args;
            unsigned int *ret = (unsigned int *)ret_v;
            *ret =
            snd_timer_params_get_filter(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_get_queue_size
        case snd_timer_params_get_queue_size_INDEX: {
            INDEXED_long_snd_timer_params_t___GENPT__ *unpacked = (INDEXED_long_snd_timer_params_t___GENPT__ *)packed;
            ARGS_long_snd_timer_params_t___GENPT__ args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_timer_params_get_queue_size(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_get_ticks
        case snd_timer_params_get_ticks_INDEX: {
            INDEXED_long_snd_timer_params_t___GENPT__ *unpacked = (INDEXED_long_snd_timer_params_t___GENPT__ *)packed;
            ARGS_long_snd_timer_params_t___GENPT__ args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_timer_params_get_ticks(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_malloc
        case snd_timer_params_malloc_INDEX: {
            INDEXED_int_snd_timer_params_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_timer_params_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_timer_params_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_params_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_set_auto_start
        case snd_timer_params_set_auto_start_INDEX: {
            INDEXED_int_snd_timer_params_t___GENPT___int *unpacked = (INDEXED_int_snd_timer_params_t___GENPT___int *)packed;
            ARGS_int_snd_timer_params_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_params_set_auto_start(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_set_early_event
        case snd_timer_params_set_early_event_INDEX: {
            INDEXED_int_snd_timer_params_t___GENPT___int *unpacked = (INDEXED_int_snd_timer_params_t___GENPT___int *)packed;
            ARGS_int_snd_timer_params_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_params_set_early_event(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_set_exclusive
        case snd_timer_params_set_exclusive_INDEX: {
            INDEXED_int_snd_timer_params_t___GENPT___int *unpacked = (INDEXED_int_snd_timer_params_t___GENPT___int *)packed;
            ARGS_int_snd_timer_params_t___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_params_set_exclusive(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_set_filter
        case snd_timer_params_set_filter_INDEX: {
            INDEXED_void_snd_timer_params_t___GENPT___unsigned_int *unpacked = (INDEXED_void_snd_timer_params_t___GENPT___unsigned_int *)packed;
            ARGS_void_snd_timer_params_t___GENPT___unsigned_int args = unpacked->args;
            snd_timer_params_set_filter(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_set_queue_size
        case snd_timer_params_set_queue_size_INDEX: {
            INDEXED_void_snd_timer_params_t___GENPT___long *unpacked = (INDEXED_void_snd_timer_params_t___GENPT___long *)packed;
            ARGS_void_snd_timer_params_t___GENPT___long args = unpacked->args;
            snd_timer_params_set_queue_size(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_set_ticks
        case snd_timer_params_set_ticks_INDEX: {
            INDEXED_void_snd_timer_params_t___GENPT___long *unpacked = (INDEXED_void_snd_timer_params_t___GENPT___long *)packed;
            ARGS_void_snd_timer_params_t___GENPT___long args = unpacked->args;
            snd_timer_params_set_ticks(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_params_sizeof
        case snd_timer_params_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_timer_params_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_poll_descriptors
        case snd_timer_poll_descriptors_INDEX: {
            INDEXED_int_snd_timer_t___GENPT___struct_pollfd___GENPT___unsigned_int *unpacked = (INDEXED_int_snd_timer_t___GENPT___struct_pollfd___GENPT___unsigned_int *)packed;
            ARGS_int_snd_timer_t___GENPT___struct_pollfd___GENPT___unsigned_int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_poll_descriptors(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_poll_descriptors_count
        case snd_timer_poll_descriptors_count_INDEX: {
            INDEXED_int_snd_timer_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_t___GENPT__ *)packed;
            ARGS_int_snd_timer_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_poll_descriptors_count(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_poll_descriptors_revents
        case snd_timer_poll_descriptors_revents_INDEX: {
            INDEXED_int_snd_timer_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *unpacked = (INDEXED_int_snd_timer_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ *)packed;
            ARGS_int_snd_timer_t___GENPT___struct_pollfd___GENPT___unsigned_int_unsigned_short___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_poll_descriptors_revents(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_query_close
        case snd_timer_query_close_INDEX: {
            INDEXED_int_snd_timer_query_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_query_t___GENPT__ *)packed;
            ARGS_int_snd_timer_query_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_query_close(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_query_info
        case snd_timer_query_info_INDEX: {
            INDEXED_int_snd_timer_query_t___GENPT___snd_timer_ginfo_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_query_t___GENPT___snd_timer_ginfo_t___GENPT__ *)packed;
            ARGS_int_snd_timer_query_t___GENPT___snd_timer_ginfo_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_query_info(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_query_next_device
        case snd_timer_query_next_device_INDEX: {
            INDEXED_int_snd_timer_query_t___GENPT___snd_timer_id_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_query_t___GENPT___snd_timer_id_t___GENPT__ *)packed;
            ARGS_int_snd_timer_query_t___GENPT___snd_timer_id_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_query_next_device(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_query_open
        case snd_timer_query_open_INDEX: {
            INDEXED_int_snd_timer_query_t___GENPT____GENPT___const_char___GENPT___int *unpacked = (INDEXED_int_snd_timer_query_t___GENPT____GENPT___const_char___GENPT___int *)packed;
            ARGS_int_snd_timer_query_t___GENPT____GENPT___const_char___GENPT___int args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_query_open(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_query_open_lconf
        case snd_timer_query_open_lconf_INDEX: {
            INDEXED_int_snd_timer_query_t___GENPT____GENPT___const_char___GENPT___int_snd_config_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_query_t___GENPT____GENPT___const_char___GENPT___int_snd_config_t___GENPT__ *)packed;
            ARGS_int_snd_timer_query_t___GENPT____GENPT___const_char___GENPT___int_snd_config_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_query_open_lconf(args.a1, args.a2, args.a3, args.a4);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_query_params
        case snd_timer_query_params_INDEX: {
            INDEXED_int_snd_timer_query_t___GENPT___snd_timer_gparams_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_query_t___GENPT___snd_timer_gparams_t___GENPT__ *)packed;
            ARGS_int_snd_timer_query_t___GENPT___snd_timer_gparams_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_query_params(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_query_status
        case snd_timer_query_status_INDEX: {
            INDEXED_int_snd_timer_query_t___GENPT___snd_timer_gstatus_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_query_t___GENPT___snd_timer_gstatus_t___GENPT__ *)packed;
            ARGS_int_snd_timer_query_t___GENPT___snd_timer_gstatus_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_query_status(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_read
        case snd_timer_read_INDEX: {
            INDEXED_ssize_t_snd_timer_t___GENPT___void___GENPT___size_t *unpacked = (INDEXED_ssize_t_snd_timer_t___GENPT___void___GENPT___size_t *)packed;
            ARGS_ssize_t_snd_timer_t___GENPT___void___GENPT___size_t args = unpacked->args;
            ssize_t *ret = (ssize_t *)ret_v;
            *ret =
            snd_timer_read(args.a1, args.a2, args.a3);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_start
        case snd_timer_start_INDEX: {
            INDEXED_int_snd_timer_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_t___GENPT__ *)packed;
            ARGS_int_snd_timer_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_start(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_status
        case snd_timer_status_INDEX: {
            INDEXED_int_snd_timer_t___GENPT___snd_timer_status_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_t___GENPT___snd_timer_status_t___GENPT__ *)packed;
            ARGS_int_snd_timer_t___GENPT___snd_timer_status_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_status(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_status_copy
        case snd_timer_status_copy_INDEX: {
            INDEXED_void_snd_timer_status_t___GENPT___const_snd_timer_status_t___GENPT__ *unpacked = (INDEXED_void_snd_timer_status_t___GENPT___const_snd_timer_status_t___GENPT__ *)packed;
            ARGS_void_snd_timer_status_t___GENPT___const_snd_timer_status_t___GENPT__ args = unpacked->args;
            snd_timer_status_copy(args.a1, args.a2);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_status_free
        case snd_timer_status_free_INDEX: {
            INDEXED_void_snd_timer_status_t___GENPT__ *unpacked = (INDEXED_void_snd_timer_status_t___GENPT__ *)packed;
            ARGS_void_snd_timer_status_t___GENPT__ args = unpacked->args;
            snd_timer_status_free(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_status_get_lost
        case snd_timer_status_get_lost_INDEX: {
            INDEXED_long_snd_timer_status_t___GENPT__ *unpacked = (INDEXED_long_snd_timer_status_t___GENPT__ *)packed;
            ARGS_long_snd_timer_status_t___GENPT__ args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_timer_status_get_lost(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_status_get_overrun
        case snd_timer_status_get_overrun_INDEX: {
            INDEXED_long_snd_timer_status_t___GENPT__ *unpacked = (INDEXED_long_snd_timer_status_t___GENPT__ *)packed;
            ARGS_long_snd_timer_status_t___GENPT__ args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_timer_status_get_overrun(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_status_get_queue
        case snd_timer_status_get_queue_INDEX: {
            INDEXED_long_snd_timer_status_t___GENPT__ *unpacked = (INDEXED_long_snd_timer_status_t___GENPT__ *)packed;
            ARGS_long_snd_timer_status_t___GENPT__ args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_timer_status_get_queue(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_status_get_resolution
        case snd_timer_status_get_resolution_INDEX: {
            INDEXED_long_snd_timer_status_t___GENPT__ *unpacked = (INDEXED_long_snd_timer_status_t___GENPT__ *)packed;
            ARGS_long_snd_timer_status_t___GENPT__ args = unpacked->args;
            long *ret = (long *)ret_v;
            *ret =
            snd_timer_status_get_resolution(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_status_get_timestamp
        case snd_timer_status_get_timestamp_INDEX: {
            INDEXED_snd_htimestamp_t_snd_timer_status_t___GENPT__ *unpacked = (INDEXED_snd_htimestamp_t_snd_timer_status_t___GENPT__ *)packed;
            ARGS_snd_htimestamp_t_snd_timer_status_t___GENPT__ args = unpacked->args;
            snd_htimestamp_t *ret = (snd_htimestamp_t *)ret_v;
            *ret =
            snd_timer_status_get_timestamp(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_status_malloc
        case snd_timer_status_malloc_INDEX: {
            INDEXED_int_snd_timer_status_t___GENPT____GENPT__ *unpacked = (INDEXED_int_snd_timer_status_t___GENPT____GENPT__ *)packed;
            ARGS_int_snd_timer_status_t___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_status_malloc(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_status_sizeof
        case snd_timer_status_sizeof_INDEX: {
            INDEXED_size_t *unpacked = (INDEXED_size_t *)packed;
            size_t *ret = (size_t *)ret_v;
            *ret =
            snd_timer_status_sizeof();
            break;
        }
        #endif
        #ifndef skip_index_snd_timer_stop
        case snd_timer_stop_INDEX: {
            INDEXED_int_snd_timer_t___GENPT__ *unpacked = (INDEXED_int_snd_timer_t___GENPT__ *)packed;
            ARGS_int_snd_timer_t___GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_timer_stop(args.a1);
            break;
        }
        #endif
        #ifndef skip_index_snd_user_file
        case snd_user_file_INDEX: {
            INDEXED_int_const_char___GENPT___char___GENPT____GENPT__ *unpacked = (INDEXED_int_const_char___GENPT___char___GENPT____GENPT__ *)packed;
            ARGS_int_const_char___GENPT___char___GENPT____GENPT__ args = unpacked->args;
            int *ret = (int *)ret_v;
            *ret =
            snd_user_file(args.a1, args.a2);
            break;
        }
        #endif
    }
}
#endif
