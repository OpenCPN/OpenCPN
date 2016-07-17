#include "array.h"
#include "block.h"
#include "error.h"
#include "list.h"
#include "loader.h"
#include "texture.h"
#include "types.h"
#include "remote.h"

glstate_t state = {
    0,
    .current = {
        .color = {1.0f, 1.0f, 1.0f, 1.0f},
        .normal = {0.0f, 0.0f, 1.0f},
        .tex = {[0 ... MAX_TEX - 1] = {0.0f, 0.0f}},
    },
    .matrix = {
        .mode = GL_MODELVIEW,
    },
    .render = {
        .mode = GL_RENDER,
    },
    .raster = {
        .buf = NULL,
        .pos = {0.0f, 0.0f, 0.0f, 1.0f},
        .color = {1.0f, 1.0f, 1.0f, 1.0f},
        .pixel = 0xFFFFFFFF,
        .valid = 1,
    },
    .stack = {0},
    .texgen = {[0 ... MAX_TEX - 1] = {
        .R = GL_EYE_LINEAR,
        .Q = GL_EYE_LINEAR,
        .S = GL_EYE_LINEAR,
        .T = GL_EYE_LINEAR,
        .Rv = {0, 0, 0, 0},
        .Qv = {0, 0, 0, 0},
        .Sv = {1, 0, 0, 0},
        .Tv = {0, 1, 0, 0},
    }},
};

static void proxy_glEnable(GLenum cap, bool enable, void (*next)(GLenum)) {
    #define proxy_enable(constant, name) \
        case constant: \
            if (state.enable.name != enable) { \
                state.enable.name = enable; \
                next(cap); \
            } \
            break
    #define enable(constant, name) \
        case constant: state.enable.name = enable; break;

    // TODO: maybe could be weird behavior if someone tried to:
    // 1. enable GL_TEXTURE_1D
    // 2. enable GL_TEXTURE_2D
    // 3. disable GL_TEXTURE_1D
    // 4. render. GL_TEXTURE_2D would be disabled.
    cap = map_tex_target(cap);

    switch (cap) {
        proxy_enable(GL_BLEND, blend);
        proxy_enable(GL_TEXTURE_2D, texture_2d[state.texture.active]);
        enable(GL_TEXTURE_GEN_R, texgen_r[state.texture.active]);
        enable(GL_TEXTURE_GEN_Q, texgen_q[state.texture.active]);
        enable(GL_TEXTURE_GEN_S, texgen_s[state.texture.active]);
        enable(GL_TEXTURE_GEN_T, texgen_t[state.texture.active]);
        enable(GL_LINE_STIPPLE, line_stipple);

        // for glDrawArrays
        proxy_enable(GL_VERTEX_ARRAY, vertex_array);
        proxy_enable(GL_NORMAL_ARRAY, normal_array);
        proxy_enable(GL_COLOR_ARRAY, color_array);
        proxy_enable(GL_TEXTURE_COORD_ARRAY, tex_coord_array[state.texture.client]);
        default: next(cap); break;
    }
    #undef proxy_enable
    #undef enable
}

void glEnable(GLenum cap) {
    PUSH_IF_COMPILING(glEnable);
    LOAD_GLES_SILENT(glEnable);
    ERROR_IN_BLOCK();
    proxy_glEnable(cap, true, gles_glEnable);
}

void glDisable(GLenum cap) {
    PUSH_IF_COMPILING(glDisable);
    LOAD_GLES_SILENT(glDisable);
    ERROR_IN_BLOCK();
    proxy_glEnable(cap, false, gles_glDisable);
}

#ifndef USE_ES2
void glEnableClientState(GLenum array) {
    LOAD_GLES_SILENT(glEnableClientState);
    ERROR_IN_BLOCK();
    proxy_glEnable(array, true, gles_glEnableClientState);
}

void glDisableClientState(GLenum array) {
    LOAD_GLES_SILENT(glDisableClientState);
    ERROR_IN_BLOCK();
    proxy_glEnable(array, false, gles_glDisableClientState);
}
#endif

GLboolean glIsEnabled(GLenum cap) {
    if (state.block.active) {
        gl_set_error(GL_INVALID_OPERATION);
        return 0;
    }
    FORWARD_IF_REMOTE(glIsEnabled);
    switch (cap) {
        case GL_LINE_STIPPLE:
            return state.enable.line_stipple;
        case GL_TEXTURE_GEN_S:
            return state.enable.texgen_s[state.texture.active];
        case GL_TEXTURE_GEN_T:
            return state.enable.texgen_t[state.texture.active];
        case GL_TEXTURE_COORD_ARRAY:
            return state.enable.tex_coord_array[state.texture.client];
        default:
        {
            LOAD_GLES(glIsEnabled);
            return gles_glIsEnabled(cap);
        }
    }
}

static block_t *block_from_arrays(GLenum mode, GLsizei skip, GLsizei count) {
    block_t *block = bl_new(mode);
    block->artificial = true;

    block->len = block->cap = count;
    if (state.enable.vertex_array) {
        block->vert = gl_copy_pointer(&state.pointers.vertex, 3, skip, count, false);
    }
    if (state.enable.color_array) {
        block->color = gl_copy_pointer(&state.pointers.color, 4, skip, count, true);
    }
    if (state.enable.normal_array) {
        block->normal = gl_copy_pointer(&state.pointers.normal, 3, skip, count, false);
    }
    for (int i = 0; i < MAX_TEX; i++) {
        if (state.enable.tex_coord_array[i]) {
            block->tex[i] = gl_copy_pointer(&state.pointers.tex_coord[i], 2, skip, count, false);
        }
    }
    return block;
}

static inline bool should_intercept_render(GLenum mode) {
#ifdef LOCAL_MATRIX
    // gotta force this for matrix stack
    return true;
#endif

    bool texgen_enabled = false;
    for (int i = 0; i < MAX_TEX; i++) {
        texgen_enabled |= state.enable.texgen_s[i] || state.enable.texgen_t[i];
    }
    return (
        (state.remote) ||
        (state.enable.vertex_array && ! gl_valid_vertex_type(state.pointers.vertex.type)) ||
        (texgen_enabled) ||
        (mode == GL_LINES && state.enable.line_stipple) ||
        (mode == GL_QUADS) ||
        (state.render.mode == GL_FEEDBACK || state.render.mode == GL_SELECT)
    );
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *uindices) {
    // TODO: split for count > 65535?
    GLushort *indices = gl_copy_array(uindices, type, 1, 0, GL_UNSIGNED_SHORT, 1, 0, count, false);
    // TODO: do this in a more direct fashion.
    if (should_intercept_render(mode)) {
        glBegin(mode);
        state.block.active->artificial = true;
        for (int i = 0; i < count; i++) {
            glArrayElement(indices[i]);
        }
        glEnd();
        free(indices);
        return;
    }

    displaylist_t *list = state.list.active;
    if (list) {
        GLsizei min, max;

        normalize_indices(indices, &max, &min, count);
        block_t *block = block_from_arrays(mode, min, max + 1);
        block->indices = indices;
        block->count = count;

        bl_end(block);
        if (list) {
            dl_append_block(list, block);
        } else {
            bl_draw(block);
            bl_free(block);
        }
    } else {
        LOAD_GLES(glDrawElements);
        gles_glDrawElements(mode, count, type, indices);
        free(indices);
    }
}

void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices) {
    if (end < start || count < 0) {
        gl_set_error(GL_INVALID_VALUE);
        return;
    }
    glDrawElements(mode, count, type, indices);
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    if (mode == GL_QUAD_STRIP)
        mode = GL_TRIANGLE_STRIP;

    displaylist_t *active = state.list.active;
    if (active) {
        block_t *block = block_from_arrays(mode, first, count);
        bl_end(block);
        dl_append_block(active, block);
        return;
    }

    if (should_intercept_render(mode)) {
        block_t *block = block_from_arrays(mode, first, count);
        bl_end(block);
        bl_draw(block);
        bl_free(block);
    } else {
        LOAD_GLES(glDrawArrays);
        gles_glDrawArrays(mode, first, count);
    }
}

#ifndef USE_ES2
#define clone_gl_pointer(t, s)\
    t.size = s; t.type = type; t.stride = stride; t.pointer = pointer;
void glVertexPointer(GLint size, GLenum type,
                     GLsizei stride, const GLvoid *pointer) {
    clone_gl_pointer(state.pointers.vertex, size);
    if (state.remote) return;
    LOAD_GLES(glVertexPointer);
    gles_glVertexPointer(size, type, stride, pointer);
}
void glColorPointer(GLint size, GLenum type,
                     GLsizei stride, const GLvoid *pointer) {
    clone_gl_pointer(state.pointers.color, size);
    if (state.remote) return;
    LOAD_GLES(glColorPointer);
    gles_glColorPointer(size, type, stride, pointer);
}
void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer) {
    clone_gl_pointer(state.pointers.normal, 3);
    if (state.remote) return;
    LOAD_GLES(glNormalPointer);
    gles_glNormalPointer(type, stride, pointer);
}
void glTexCoordPointer(GLint size, GLenum type,
                     GLsizei stride, const GLvoid *pointer) {
    clone_gl_pointer(state.pointers.tex_coord[state.texture.client], size);
    if (state.remote) return;
    LOAD_GLES(glTexCoordPointer);
    gles_glTexCoordPointer(size, type, stride, pointer);
}
#undef clone_gl_pointer
#endif

void glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer) {
    if (stride < 0) {
        ERROR(GL_INVALID_VALUE);
    }
    uintptr_t ptr = (uintptr_t)pointer;
    // element lengths
    GLsizei tex = 0, color = 0, normal = 0, vert = 0;
    // element formats
    GLenum tf, cf, nf, vf;
    tf = cf = nf = vf = GL_FLOAT;

    switch (format) {
        case GL_V2F: vert = 2; break;
        case GL_V3F: vert = 3; break;
        case GL_C4UB_V2F:
            color = 4; cf = GL_UNSIGNED_BYTE;
            vert = 2;
            break;
        case GL_C4UB_V3F:
            color = 4; cf = GL_UNSIGNED_BYTE;
            vert = 3;
            break;
        case GL_C3F_V3F:
            color = 3;
            vert = 4;
            break;
        case GL_N3F_V3F:
            normal = 3;
            vert = 3;
            break;
        case GL_C4F_N3F_V3F:
            color = 4;
            normal = 3;
            vert = 3;
            break;
        case GL_T2F_V3F:
            tex = 2;
            vert = 3;
            break;
        case GL_T4F_V4F:
            tex = 4;
            vert = 4;
            break;
        case GL_T2F_C4UB_V3F:
            tex = 2;
            color = 4; cf = GL_UNSIGNED_BYTE;
            vert = 3;
            break;
        case GL_T2F_C3F_V3F:
            tex = 2;
            color = 3;
            vert = 3;
            break;
        case GL_T2F_N3F_V3F:
            tex = 2;
            normal = 3;
            vert = 3;
            break;
        case GL_T2F_C4F_N3F_V3F:
            tex = 2;
            color = 4;
            normal = 3;
            vert = 3;
            break;
        case GL_T4F_C4F_N3F_V4F:
            tex = 4;
            color = 4;
            normal = 3;
            vert = 4;
            break;
        default:
            ERROR(GL_INVALID_ENUM);
    }
    if (! stride)
        stride = tex * gl_sizeof(tf) +
                 color * gl_sizeof(cf) +
                 normal * gl_sizeof(nf) +
                 vert * gl_sizeof(vf);
    if (tex) {
        glTexCoordPointer(tex, tf, stride, (GLvoid *)ptr);
        ptr += tex * gl_sizeof(tf);
    }
    if (color) {
        glColorPointer(color, cf, stride, (GLvoid *)ptr);
        ptr += color * gl_sizeof(cf);
    }
    if (normal) {
        glNormalPointer(nf, stride, (GLvoid *)ptr);
        ptr += normal * gl_sizeof(nf);
    }
    if (vert)
        glVertexPointer(vert, vf, stride, (GLvoid *)ptr);
}

// immediate mode functions

void glBegin(GLenum mode) {
    if (! gl_valid_mode(mode)) {
        ERROR(GL_INVALID_ENUM);
    }
    ERROR_IN_BLOCK();
    block_t *block = state.block.active = bl_new(mode);
    displaylist_t *list = state.list.active;
    if (list) {
        dl_append_block(list, block);
    }
}

void glEnd() {
    block_t *block = state.block.active;
    if (! block) {
        ERROR(GL_INVALID_OPERATION);
    }

    state.block.active = NULL;
    bl_end(block);
    // render if we're not in a display list
    if (! state.list.active) {
        bl_draw(block);
        bl_free(block);
    }
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z) {
    block_t *block = state.block.active;
    if (block) {
        bl_vertex3f(block, x, y, z);
    }
}

void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) {
    block_t *block = state.block.active;
    if (block) {
        bl_track_normal(block);
    }

    GLfloat *normal = CURRENT->normal;
    normal[0] = nx;
    normal[1] = ny;
    normal[2] = nz;

    if (! block) {
        PUSH_IF_COMPILING(glNormal3f);
        LOAD_GLES(glNormal3f);
        gles_glNormal3f(nx, ny, nz);
    }
}

void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    block_t *block = state.block.active;
    if (block) {
        bl_track_color(block);
    }

    GLfloat *color = CURRENT->color;
    color[0] = red;
    color[1] = green;
    color[2] = blue;
    color[3] = alpha;

#ifndef USE_ES2
    if (! block) {
        PUSH_IF_COMPILING(glColor4f);
        LOAD_GLES(glColor4f);
        gles_glColor4f(red, green, blue, alpha);
    }
#endif
}

void glTexCoord2f(GLfloat s, GLfloat t) {
    glMultiTexCoord2f(GL_TEXTURE0, s, t);
}

void glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t) {
    block_t *block = state.block.active;
    if (block) {
        bl_track_tex(block, target);
    }

    GLfloat *tex = CURRENT->tex[target - GL_TEXTURE0];
    tex[0] = s;
    tex[1] = t;

    if (! block) {
        PUSH_IF_COMPILING(glMultiTexCoord2f);
    }
}

void glArrayElement(GLint i) {
    if (i < 0) {
        ERROR(GL_INVALID_VALUE);
    }
    GLfloat *v;
    pointer_state_t *p;
    p = &state.pointers.color;
    if (state.enable.color_array && p->pointer) {
        v = gl_pointer_index(p, i);
        GLuint scale = gl_max_value(p->type);
        // color[3] defaults to 1.0f
        if (p->size < 4)
            v[3] = 1.0f;

        // scale color coordinates to a 0 - 1.0 range
        for (int i = 0; i < p->size; i++) {
            v[i] /= scale;
        }
        glColor4fv(v);
    }
    p = &state.pointers.normal;
    if (state.enable.normal_array && p->pointer) {
        v = gl_pointer_index(p, i);
        glNormal3fv(v);
    }
    for (int i = 0; i < MAX_TEX; i++) {
        p = &state.pointers.tex_coord[i];
        if (state.enable.tex_coord_array[i] && p->pointer) {
            v = gl_pointer_index(p, i);
            glMultiTexCoord2fv(GL_TEXTURE0 + i, v);
        }
    }
    p = &state.pointers.vertex;
    if (state.enable.vertex_array && p->pointer) {
        v = gl_pointer_index(p, i);
        if (p->size == 4) {
            glVertex4fv(v);
        } else {
            glVertex3fv(v);
        }
    }
}

// TODO: between a lock and unlock, I can assume the array pointers are unchanged
// so I can build a block_t on the first call and hold onto it
// maybe I need a way to call a block_t with (first, count)
void glLockArraysEXT(GLint first, GLsizei count) {
    state.block.locked = true;
}

void glUnlockArraysEXT() {
    state.block.locked = false;
}

// display lists

static displaylist_t *get_list(GLuint list) {
    return tack_get(&state.lists, list - 1);
}

GLuint glGenLists(GLsizei range) {
    FORWARD_IF_REMOTE(glGenLists);
    if (range < 0) {
        gl_set_error(GL_INVALID_VALUE);
        return 0;
    }
    if (state.block.active) {
        gl_set_error(GL_INVALID_OPERATION);
        return 0;
    }
    int start = tack_len(&state.lists);
    for (int i = 0; i < range; i++) {
        tack_set(&state.lists, start + i, NULL);
    }
    return start + 1;
}

void glNewList(GLuint list, GLenum mode) {
    FORWARD_IF_REMOTE(glNewList);
    // TODO: make sure this doesn't break anything
    if (list == 0) {
        ERROR(GL_INVALID_VALUE);
    }
    if (mode != GL_COMPILE && mode != GL_COMPILE_AND_EXECUTE) {
        ERROR(GL_INVALID_ENUM);
    }
    if (state.block.active || state.list.active) {
        ERROR(GL_INVALID_OPERATION);
    }
    displaylist_t *dl = dl_alloc();
    if (dl == NULL) {
        ERROR(GL_OUT_OF_MEMORY);
    }
    state.list.name = list;
    state.list.mode = mode;
    state.list.active = dl;
}

void glEndList() {
    FORWARD_IF_REMOTE(glEndList);
    GLuint list = state.list.name;
    displaylist_t *dl = state.list.active;
    if (!dl || state.block.active) {
        ERROR(GL_INVALID_OPERATION);
    }

    displaylist_t *old = get_list(list);
    if (old) {
        dl_free(old);
    }

    tack_set(&state.lists, list - 1, dl);
    state.list.active = NULL;
    if (state.list.mode == GL_COMPILE_AND_EXECUTE) {
        glCallList(list);
    }
}

void glCallList(GLuint list) {
    PUSH_IF_COMPILING(glCallList);
    if (state.list.recursion >= 64) {
        return;
    }
    state.list.recursion++;
    displaylist_t *l = get_list(list);
    displaylist_t *active = state.list.active;
    if (l) {
        dl_call(l);
    }
    state.list.recursion--;
}

void glPushCall(void *call) {
    displaylist_t *active = state.list.active;
    if (active) {
        dl_append(active, call);
    }
}

void glCallLists(GLsizei n, GLenum type, const GLvoid *lists) {
    #define call(name, type) \
        case name: glCallList(((type *)lists)[i] + state.list.base); break

    // seriously wtf
    #define call_bytes(name, stride)                             \
        case name:                                               \
            l = (GLubyte *)lists;                                \
            list = 0;                                            \
            for (j = 0; j < stride; j++) {                       \
                list += *(l + (i * stride + j)) << (stride - j); \
            }                                                    \
            glCallList(list + state.list.base);                  \
            break

    if (n < 0) {
        ERROR(GL_INVALID_VALUE);
    }
    unsigned int i, j;
    GLuint list;
    GLubyte *l;
    for (i = 0; i < n; i++) {
        switch (type) {
            call(GL_BYTE, GLbyte);
            call(GL_UNSIGNED_BYTE, GLubyte);
            call(GL_SHORT, GLshort);
            call(GL_UNSIGNED_SHORT, GLushort);
            call(GL_INT, GLint);
            call(GL_UNSIGNED_INT, GLuint);
            call(GL_FLOAT, GLfloat);
            call_bytes(GL_2_BYTES, 2);
            call_bytes(GL_3_BYTES, 3);
            call_bytes(GL_4_BYTES, 4);
            default:
                ERROR(GL_INVALID_ENUM);
        }
    }
    #undef call
    #undef call_bytes
}

void glDeleteLists(GLuint list, GLsizei range) {
    FORWARD_IF_REMOTE(glDeleteLists);
    if (range < 0) {
        ERROR(GL_INVALID_VALUE);
    }
    ERROR_IN_BLOCK();
    for (int i = 0; i < range; i++) {
        displaylist_t *l = get_list(list);
        if (l) {
            if (state.list.active == l) {
                state.list.active = NULL;
            }
            dl_free(l);
            tack_set(&state.lists, list - 1, NULL);
        }
    }
    // lists just grow upwards, maybe use a better storage mechanism?
}

void glListBase(GLuint base) {
    state.list.base = base;
}

GLboolean glIsList(GLuint list) {
    FORWARD_IF_REMOTE(glIsList);
    if (state.block.active) {
        gl_set_error(GL_INVALID_OPERATION);
        return 0;
    }
    return get_list(list) ? true : false;
}
