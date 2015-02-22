#include "gl.h"

glstate_t state = {.color = {1.0f, 1.0f, 1.0f, 1.0f}};

// config functions
const GLubyte *glGetString(GLenum name) {
    LOAD_GLES(glGetString);
    switch (name) {
        case GL_VERSION:
#ifdef USE_ES2
            return (GLubyte *)"4.3 glshim wrapper";
#else
            return (GLubyte *)"1.4 glshim wrapper";
#endif
        case GL_EXTENSIONS:
        {
            static GLubyte *extensions = NULL;
            if(!extensions) {
                const char *builtin_extensions = {
#ifndef USE_ES2
                // "GL_ARB_vertex_buffer_object "
                "GL_EXT_secondary_color "
#else
                "GL_ARB_vertex_shader "
                "GL_ARB_fragment_shader "
                "GL_ARB_vertex_buffer_object "
                "GL_EXT_framebuffer_object "
#endif
                };

                const char *query_extensions = gles_glGetString(GL_EXTENSIONS);

                extensions = malloc(strlen(builtin_extensions)
                                    + strlen(query_extensions) + 2);
                sprintf(extensions, "%s %s", builtin_extensions,
                    query_extensions);
            }

            return extensions;
        }
        default:
            return gles_glGetString(name);
    }
}

void glGetIntegerv(GLenum pname, GLint *params) {
    LOAD_GLES(glGetIntegerv);
    switch (pname) {
        case GL_MAX_ELEMENTS_INDICES:
            *params = 1024;
            break;
        case GL_AUX_BUFFERS:
            *params = 0;
            break;
        default:
            gles_glGetIntegerv(pname, params);
    }
}

static void proxy_glEnable(GLenum cap, bool enable, void (*next)(GLenum)) {
    #define proxy_enable(constant, name) \
        case constant: state.enable.name = enable; next(cap); break
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
        proxy_enable(GL_TEXTURE_2D, texture_2d);
        enable(GL_TEXTURE_GEN_S, texgen_s);
        enable(GL_TEXTURE_GEN_T, texgen_t);
        enable(GL_LINE_STIPPLE, line_stipple);

        // for glDrawArrays
        proxy_enable(GL_VERTEX_ARRAY, vertex_array);
        proxy_enable(GL_NORMAL_ARRAY, normal_array);
        proxy_enable(GL_COLOR_ARRAY, color_array);
        proxy_enable(GL_TEXTURE_COORD_ARRAY, tex_coord_array);
        default: next(cap); break;
    }
    #undef proxy_enable
    #undef enable
}

void glEnable(GLenum cap) {
    if (state.list.compiling && state.list.active) {
        push_glEnable(cap);
    } else {
        LOAD_GLES(glEnable);
        proxy_glEnable(cap, true, gles_glEnable);
    }

}

void glDisable(GLenum cap) {
    if (state.list.compiling && state.list.active) {
        push_glDisable(cap);
    } else {
        LOAD_GLES(glDisable);
        proxy_glEnable(cap, false, gles_glDisable);
    }
}

#ifndef USE_ES2
void glEnableClientState(GLenum cap) {
    LOAD_GLES(glEnableClientState);
    proxy_glEnable(cap, true, gles_glEnableClientState);
}

void glDisableClientState(GLenum cap) {
    LOAD_GLES(glDisableClientState);
    proxy_glEnable(cap, false, gles_glDisableClientState);
}
#endif

GLboolean glIsEnabled(GLenum cap) {
    LOAD_GLES(glIsEnabled);
    switch (cap) {
        case GL_LINE_STIPPLE:
            return state.enable.line_stipple;
        case GL_TEXTURE_GEN_S:
            return state.enable.texgen_s;
        case GL_TEXTURE_GEN_T:
            return state.enable.texgen_t;
        default:
            return gles_glIsEnabled(cap);
    }
}

static renderlist_t *arrays_to_renderlist(renderlist_t *list, GLenum mode,
                                        GLsizei skip, GLsizei count) {
    if (! list)
        list = alloc_renderlist();

    list->mode = mode;
    list->len = count;
    list->cap = count;
    if (state.enable.vertex_array) {
        list->vert = copy_gl_pointer(&state.pointers.vertex, 3, skip, count);
    }
    if (state.enable.color_array) {
        list->color = copy_gl_pointer(&state.pointers.color, 4, skip, count);
    }
    if (state.enable.normal_array) {
        list->normal = copy_gl_pointer(&state.pointers.normal, 3, skip, count);
    }
    if (state.enable.tex_coord_array) {
        list->tex = copy_gl_pointer(&state.pointers.tex_coord, 2, skip, count);
    }

    end_renderlist(list);
    return list;
}

static inline bool should_intercept_render(GLenum mode) {
    return (
        (state.enable.vertex_array && ! valid_vertex_type(state.pointers.vertex.type)) ||
        state.enable.texgen_s || state.enable.texgen_t ||
        (mode == GL_LINES && state.enable.line_stipple) ||
        mode == GL_QUADS
    );
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *uindices) {
    // TODO: split for count > 65535?
    GLushort *indices = copy_gl_array(uindices, type, 1, 0, GL_UNSIGNED_SHORT, 1, 0, count);
    // TODO: do this in a more direct fashion.
    if (should_intercept_render(mode)) {
        glBegin(mode);
        for (int i = 0; i < count; i++) {
            glArrayElement(indices[i]);
        }
        glEnd();
        free(indices);
        return;
    }

    bool compiling = (state.list.active && state.list.compiling);
    if (compiling) {
        renderlist_t *list = NULL;
        GLsizei min, max;

        if (compiling)
            list = state.list.active = extend_renderlist(state.list.active);

        normalize_indices(indices, &max, &min, count);
        list = arrays_to_renderlist(list, mode, 0, max + 1);
        list->indices = indices;
        list->len = count;

        end_renderlist(list);
        if (! compiling) {
            draw_renderlist(list);
            free_renderlist(list);
        }
    } else {
        LOAD_GLES(glDrawElements);
        gles_glDrawElements(mode, count, type, indices);
        free(indices);
    }
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    if (mode == GL_QUAD_STRIP)
        mode = GL_TRIANGLE_STRIP;

    renderlist_t *list, *active = state.list.active;
    if (active && state.list.compiling) {
        list = state.list.active = extend_renderlist(active);
        arrays_to_renderlist(list, mode, first, count);
        return;
    }

    if (should_intercept_render(mode)) {
        list = arrays_to_renderlist(NULL, mode, first, count);
        end_renderlist(list);
        draw_renderlist(list);
        free_renderlist(list);
    } else {
        // TODO: some draw states require us to use the full pipeline here
        // like texgen, stipple, npot
        LOAD_GLES(glDrawArrays);
        gles_glDrawArrays(mode, first, count);
    }
}

#ifndef USE_ES2
#define clone_gl_pointer(t, s)\
    t.size = s; t.type = type; t.stride = stride; t.pointer = pointer;
void glVertexPointer(GLint size, GLenum type,
                     GLsizei stride, const GLvoid *pointer) {
    LOAD_GLES(glVertexPointer);
    clone_gl_pointer(state.pointers.vertex, size);
    glGetError();
    gles_glVertexPointer(size, type, stride, pointer);
}
void glColorPointer(GLint size, GLenum type,
                     GLsizei stride, const GLvoid *pointer) {
    LOAD_GLES(glColorPointer);
    clone_gl_pointer(state.pointers.color, size);
    gles_glColorPointer(size, type, stride, pointer);
}
void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer) {
    LOAD_GLES(glNormalPointer);
    clone_gl_pointer(state.pointers.normal, 3);
    gles_glNormalPointer(type, stride, pointer);
}
void glTexCoordPointer(GLint size, GLenum type,
                     GLsizei stride, const GLvoid *pointer) {
    LOAD_GLES(glTexCoordPointer);
    clone_gl_pointer(state.pointers.tex_coord, size);
    gles_glTexCoordPointer(size, type, stride, pointer);
}
#undef clone_gl_pointer
#endif

void glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer) {
    uintptr_t ptr = (uintptr_t)pointer;
    // element lengths
    GLsizei tex, color, normal, vert;
    // element formats
    GLenum tf, cf, nf, vf;
    tf = cf = nf = vf = GL_FLOAT;

    switch (format) {
        GL_V2F: vert = 2; break;
        GL_V3F: vert = 3; break;
        GL_C4UB_V2F:
            color = 4; cf = GL_UNSIGNED_BYTE;
            vert = 2;
            break;
        GL_C4UB_V3F:
            color = 4; cf = GL_UNSIGNED_BYTE;
            vert = 3;
            break;
        GL_C3F_V3F:
            color = 3;
            vert = 4;
            break;
        GL_N3F_V3F:
            normal = 3;
            vert = 3;
            break;
        GL_C4F_N3F_V3F:
            color = 4;
            normal = 3;
            vert = 3;
            break;
        GL_T2F_V3F:
            tex = 2;
            vert = 3;
            break;
        GL_T4F_V4F:
            tex = 4;
            vert = 4;
            break;
        GL_T2F_C4UB_V3F:
            tex = 2;
            color = 4; cf = GL_UNSIGNED_BYTE;
            vert = 3;
            break;
        GL_T2F_C3F_V3F:
            tex = 2;
            color = 3;
            vert = 3;
            break;
        GL_T2F_N3F_V3F:
            tex = 2;
            normal = 3;
            vert = 3;
            break;
        GL_T2F_C4F_N3F_V3F:
            tex = 2;
            color = 4;
            normal = 3;
            vert = 3;
            break;
        GL_T4F_C4F_N3F_V4F:
            tex = 4;
            color = 4;
            normal = 4;
            vert = 4;
            break;
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
    if (! state.list.compiling) {
        state.list.active = alloc_renderlist();
    }
    state.list.active->mode = mode;
}

void glEnd() {
    if (! state.list.active) return;

    // render if we're not in a display list
    if (! state.list.compiling) {
        end_renderlist(state.list.active);
        draw_renderlist(state.list.active);
        free_renderlist(state.list.active);
        state.list.active = NULL;
    } else {
        state.list.active = extend_renderlist(state.list.active);
    }
}

void glNormal3f(GLfloat x, GLfloat y, GLfloat z) {
    if (state.list.active) {
        rlNormal3f(state.list.active, x, y, z);
    }
#ifndef USE_ES2
    else {
        LOAD_GLES(glNormal3f);
        gles_glNormal3f(x, y, z);
    }
#endif
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z) {
    if (state.list.active) {
        rlVertex3f(state.list.active, x, y, z);
    }
}

void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    state.color[0] = r; state.color[1] = g;
    state.color[2] = b; state.color[3] = a;

    if (state.list.active) {
        rlColor4f(state.list.active, r, g, b, a);
    }
#ifndef USE_ES2
    else {
        LOAD_GLES(glColor4f);
        gles_glColor4f(r, g, b, a);
    }
#endif
}

#ifndef USE_ES2
void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params) {
    LOAD_GLES(glMaterialfv);
    if (state.list.active) {
        rlMaterialfv(state.list.active, face, pname, params);
    } else {
        gles_glMaterialfv(face, pname, params);
    }
}
#endif

void glTexCoord2f(GLfloat s, GLfloat t) {
    if (state.list.active) {
        rlTexCoord2f(state.list.active, s, t);
    }
}

void glArrayElement(GLint i) {
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
    p = &state.pointers.tex_coord;
    if (state.enable.tex_coord_array && p->pointer) {
        v = gl_pointer_index(p, i);
        glTexCoord2fv(v);
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
// so I can build a renderlist_t on the first call and hold onto it
// maybe I need a way to call a renderlist_t with (first, count)
void glLockArraysEXT(GLint first, GLsizei count) {
    state.list.locked = true;
}

void glUnlockArraysEXT() {
    state.list.locked = false;
}

// display lists

static renderlist_t *glGetList(GLuint list) {
    if (glIsList(list))
        return state.lists[list - 1];

    return NULL;
}

GLuint glGenLists(GLsizei range) {
    int start = state.list.count;
    if (state.lists == NULL) {
        state.list.cap += range + 100;
        state.lists = malloc(state.list.cap * sizeof(uintptr_t));
    } else if (state.list.count + range > state.list.cap) {
        state.list.cap += range + 100;
        state.lists = realloc(state.lists, state.list.cap * sizeof(uintptr_t));
    }
    state.list.count += range;

    for (int i = 0; i < range; i++) {
        state.lists[start+i] = NULL;
    }
    return start + 1;
}

void glNewList(GLuint list, GLenum mode) {
    if (! glIsList(list))
        return;

    state.list.name = list;
    state.list.mode = mode;
    // TODO: if state.list.active is already defined, we probably need to clean up here
    state.list.active = state.list.first = alloc_renderlist();
    state.list.compiling = true;
}

void glEndList() {
    GLuint list = state.list.name;
    if (state.list.compiling) {
        state.lists[list - 1] = state.list.first;
        state.list.compiling = false;
        state.list.active = NULL;
        if (state.list.mode == GL_COMPILE_AND_EXECUTE) {
            glCallList(list);
        }
    }
}

void glCallList(GLuint list) {
    // TODO: the output of this call can be compiled into another display list
    renderlist_t *l = glGetList(list);
    if (l)
        draw_renderlist(l);
}

void glPushCall(void *call) {
    if (state.list.compiling && state.list.active) {
        rlPushCall(state.list.active, call);
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
        }
    }
    #undef call
    #undef call_bytes
}

void glDeleteList(GLuint list) {
    renderlist_t *l = glGetList(list);
    if (l) {
        free_renderlist(l);
        state.lists[list-1] = NULL;
    }

    // lists just grow upwards, maybe use a better storage mechanism?
}

void glDeleteLists(GLuint list, GLsizei range) {
    for (int i = 0; i < range; i++) {
        glDeleteList(list+i);
    }
}

void glListBase(GLuint base) {
    state.list.base = base;
}

GLboolean glIsList(GLuint list) {
    if (list - 1 < state.list.count) {
        return true;
    }
    return false;
}
