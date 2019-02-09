#include "gl.h"
#include "list.h"

#define alloc_sublist(n, cap) \
    (GLfloat *)malloc(n * sizeof(GLfloat) * cap)

#define realloc_sublist(ref, n, cap) \
    if (ref)                         \
        ref = (GLfloat *)realloc(ref, n * sizeof(GLfloat) * cap)

static GLushort *cached_q2t = NULL;
static unsigned long cached_q2t_len = 0;

renderlist_t *alloc_renderlist() {
    renderlist_t *list = (renderlist_t *)malloc(sizeof(renderlist_t));
    list->len = 0;
    list->cap = DEFAULT_RENDER_LIST_CAPACITY;

    list->calls.len = 0;
    list->calls.cap = 0;
    list->calls.calls = NULL;

    list->mode = 0;
    list->vert = NULL;
    list->normal = NULL;
    list->color = NULL;
    list->tex = NULL;
    list->material = NULL;
    list->indices = NULL;
    list->q2t = false;
    list->texture = 0;

    list->prev = NULL;
    list->next = NULL;
    list->open = true;
    return list;
}

renderlist_t *extend_renderlist(renderlist_t *list) {
    renderlist_t *new = alloc_renderlist();
    list->next = new;
    new->prev = list;
    if (list->open)
        end_renderlist(list);
    return new;
}

void free_renderlist(renderlist_t *list) {
    // we want the first list in the chain
    while (list->prev)
        list = list->prev;

    renderlist_t *next;
    do {
        if (list->calls.len > 0) {
            for (int i = 0; i < list->calls.len; i++) {
                free(list->calls.calls[i]);
            }
            free(list->calls.calls);
        }
        if (list->vert) free(list->vert);
        if (list->normal) free(list->normal);
        if (list->color) free(list->color);
        if (list->tex) free(list->tex);
        if (list->material) {
            rendermaterial_t *m;
            kh_foreach_value(list->material, m,
                free(m);
            )
            kh_destroy(material, list->material);
        }
        if (list->indices) free(list->indices);
        next = list->next;
        free(list);
    } while ((list = next));
}

static inline
void resize_renderlist(renderlist_t *list) {
    if (list->len >= list->cap) {
        list->cap += DEFAULT_RENDER_LIST_CAPACITY;
        realloc_sublist(list->vert, 3, list->cap);
        realloc_sublist(list->normal, 3, list->cap);
        realloc_sublist(list->color, 4, list->cap);
        realloc_sublist(list->tex, 2, list->cap);
    }
}

void q2t_renderlist(renderlist_t *list) {
    if (!list->len || !list->vert || list->q2t) return;
    // TODO: split to multiple lists if list->len > 65535

    int a = 0, b = 1, c = 2, d = 3;
    int winding[6] = {
        a, b, d,
        b, c, d,
    };
    unsigned long len = list->len * 1.5;

    // TODO: q2t on glDrawElements?
    if (list->indices)
        free(list->indices);

    if (len > cached_q2t_len) {
        if (cached_q2t)
            free(cached_q2t);
        cached_q2t = malloc(len * sizeof(GLushort));
        cached_q2t_len = len;

        GLushort *indices = cached_q2t;
        for (int i = 0; i < list->len; i += 4) {
            for (int j = 0; j < 6; j++) {
                indices[j] = i + winding[j];
            }
            indices += 6;
        }
    }

    list->q2t = true;
    list->len = len;
    return;
}

void end_renderlist(renderlist_t *list) {
    if (! list->open)
        return;

    list->open = false;
    gltexture_t *bound = state.texture.bound;
    if (list->tex && bound && (bound->width != bound->nwidth || bound->height != bound->nheight)) {
        tex_coord_npot(list->tex, list->len, bound->width, bound->height, bound->nwidth, bound->nheight);
    }
    // GL_ARB_texture_rectangle
    if (list->tex && state.texture.rect_arb && bound) {
        tex_coord_rect_arb(list->tex, list->len, bound->width, bound->height);
    }
    switch (list->mode) {
        case GL_QUADS:
            list->mode = GL_TRIANGLES;
            q2t_renderlist(list);
            break;
        case GL_POLYGON:
            list->mode = GL_TRIANGLE_FAN;
            break;
        case GL_QUAD_STRIP:
            list->mode = GL_TRIANGLE_STRIP;
            break;
    }
}

void draw_renderlist(renderlist_t *list) {
    if (!list) return;
    LOAD_GLES(glDrawArrays);
    LOAD_GLES(glDrawElements);

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    do {
        // optimize zero-length segments out earlier?
        call_list_t *cl = &list->calls;
        if (cl->len > 0) {
            for (int i = 0; i < cl->len; i++) {
                glPackedCall(cl->calls[i]);
            }
        }
        if (list->texture)
            glBindTexture(GL_TEXTURE_2D, list->texture);
        if (! list->len)
            continue;

#ifdef USE_ES2
        if (list->vert) {
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, list->vert);
        }
        gles_glDrawArrays(list->mode, 0, list->len);
#else
        if (list->vert) {
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, list->vert);
        } else {
            glDisableClientState(GL_VERTEX_ARRAY);
        }

        if (list->normal) {
            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, 0, list->normal);
        } else {
            glDisableClientState(GL_NORMAL_ARRAY);
        }

        if (list->color) {
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(4, GL_FLOAT, 0, list->color);
        } else {
            glDisableClientState(GL_COLOR_ARRAY);
        }

        if (list->material) {
            khash_t(material) *map = list->material;
            rendermaterial_t *m;
            kh_foreach_value(map, m,
                glMaterialfv(GL_FRONT_AND_BACK, m->pname, m->color);
            )
        }

        GLuint texture;

        bool stipple = false;
        if (! list->tex) {
            // TODO: do we need to support GL_LINE_STRIP?
            if (list->mode == GL_LINES && state.enable.line_stipple) {
                stipple = true;
                glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT);
                glEnable(GL_BLEND);
                glEnable(GL_TEXTURE_2D);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                list->tex = gen_stipple_tex_coords(list->vert, list->len);
            } else if (state.enable.texgen_s || state.enable.texgen_t) {
                gen_tex_coords(list->vert, &list->tex, list->len);
            }
        }

        if (list->tex) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, 0, list->tex);
        } else {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }

        GLushort *indices = list->indices;
        if (list->q2t)
            indices = cached_q2t;

        if (indices) {
            gles_glDrawElements(list->mode, list->len, GL_UNSIGNED_SHORT, indices);
        } else {
            gles_glDrawArrays(list->mode, 0, list->len);
        }
        if (stipple) {
            glPopAttrib();
        }
#endif
    } while ((list = list->next));
    glPopClientAttrib();
}

// gl function wrappers

void rlVertex3f(renderlist_t *list, GLfloat x, GLfloat y, GLfloat z) {
    if (list->vert == NULL) {
        list->vert = alloc_sublist(3, list->cap);
    } else {
        resize_renderlist(list);
    }

    if (list->normal) {
        GLfloat *normal = list->normal + (list->len * 3);
        memcpy(normal, list->lastNormal, sizeof(GLfloat) * 3);
    }

    if (list->color) {
        GLfloat *color = list->color + (list->len * 4);
        memcpy(color, state.color, sizeof(GLfloat) * 4);
    }

    if (list->tex) {
        GLfloat *tex = list->tex + (list->len * 2);
        memcpy(tex, list->lastTex, sizeof(GLfloat) * 2);
    }

    GLfloat *vert = list->vert + (list->len++ * 3);
    vert[0] = x; vert[1] = y; vert[2] = z;
}

void rlNormal3f(renderlist_t *list, GLfloat x, GLfloat y, GLfloat z) {
    GLfloat *normal = list->lastNormal;
    normal[0] = x; normal[1] = y; normal[2] = z;

    if (list->normal == NULL) {
        list->normal = alloc_sublist(3, list->cap);
        // catch up
        int i;
        for (i = 0; i < list->len; i++) {
            GLfloat *normal = (list->normal + (i * 4));
            memcpy(normal, list->lastNormal, sizeof(GLfloat) * 4);
        }
    } else {
        resize_renderlist(list);
    }
}

void rlColor4f(renderlist_t *list, GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    if (list->color == NULL) {
        list->color = alloc_sublist(4, list->cap);
        // catch up
        int i;
        for (i = 0; i < list->len; i++) {
            GLfloat *color = (list->color + (i * 4));
            memcpy(color, state.color, sizeof(GLfloat) * 4);
        }
    } else {
        resize_renderlist(list);
    }
}

void rlMaterialfv(renderlist_t *list, GLenum face, GLenum pname, const GLfloat * params) {
    rendermaterial_t *m;
    khash_t(material) *map;
    khint_t k;
    int ret;
    if (! list->material) {
        list->material = map = kh_init(material);
        // segfaults if we don't do a single put
        kh_put(material, map, 1, &ret);
        kh_del(material, map, 1);
    } else {
        map = list->material;
    }

    // TODO: currently puts all faces in the same map
    k = kh_get(material, map, pname);
    if (k == kh_end(map)) {
        k = kh_put(material, map, pname, &ret);
        m = kh_value(map, k) = malloc(sizeof(rendermaterial_t));
    } else {
        m = kh_value(map, k);
    }

    m->face = face;
    m->pname = pname;
    m->color[0] = params[0];
    m->color[1] = params[1];
    m->color[2] = params[2];
    m->color[3] = params[3];
}

void rlTexCoord2f(renderlist_t *list, GLfloat s, GLfloat t) {
    if (list->tex == NULL) {
        list->tex = alloc_sublist(2, list->cap);
        // catch up
        GLfloat *tex = list->tex;
        for (int i = 0; i < list->len; i++) {
            memcpy(tex, list->lastTex, sizeof(GLfloat) * 2);
            tex += 2;
        }
    } else {
        resize_renderlist(list);
    }
    GLfloat *tex = list->lastTex;
    tex[0] = s; tex[1] = t;
}

void rlBindTexture(renderlist_t *list, GLuint texture) {
    list->texture = texture;
}

void rlPushCall(renderlist_t *list, packed_call_t *data) {
    call_list_t *cl = &list->calls;
    if (!cl->calls) {
        cl->cap = DEFAULT_CALL_LIST_CAPACITY;
        cl->calls = malloc(DEFAULT_CALL_LIST_CAPACITY * sizeof(uintptr_t));
    } else if (list->calls.len == list->calls.cap) {
        cl->cap += DEFAULT_CALL_LIST_CAPACITY;
        cl->calls = realloc(cl->calls, cl->cap * sizeof(uintptr_t));
    }
    cl->calls[cl->len++] = data;
}

#undef alloc_sublist
#undef realloc_sublist
