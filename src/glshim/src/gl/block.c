#include "block.h"
#include "gl_helpers.h"
#include "line.h"
#include "loader.h"
#include "matrix.h"
#include "render.h"
#include "texgen.h"
#include "texture.h"
#include "remote.h"

#define alloc_sublist(n, type, cap) \
    (GLfloat *)malloc(n * gl_sizeof(type) * cap)

#define realloc_sublist(ref, n, type, cap) \
    if (ref)                         \
        ref = (GLfloat *)realloc(ref, n * gl_sizeof(type) * cap)

// q2t winding doesn't change, so we can globally precalculate/cache it
struct {
    GLushort *cache;
    uint32_t len;
} q2t = {0};

static void q2t_calc(int len) {
    if (len <= q2t.len)
        return;

    if (q2t.cache)
        free(q2t.cache);
    q2t.cache = malloc(len * 1.5 * sizeof(GLushort));
    q2t.len = len;

    int a = 0, b = 1, c = 2, d = 3;
    int winding[6] = {
        a, b, d,
        b, c, d,
    };

    GLushort *indices = q2t.cache;
    for (int i = 0; i < len; i += 4) {
        for (int j = 0; j < 6; j++) {
            indices[j] = i + winding[j];
        }
        indices += 6;
    }
}

block_t *bl_new(GLenum mode) {
    block_t *block = calloc(1, sizeof(block_t));
    block->cap = DEFAULT_BLOCK_CAPACITY;
    block->open = true;
    block->mode = mode;

    block->incomplete.color = -1;
    block->incomplete.normal = -1;
    for (int i = 0; i < MAX_TEX; i++) {
        block->incomplete.tex[i] = -1;
    }

    return block;
}

void bl_free(block_t *block) {
    if (block->solid) {
        free(block->solid);
        return;
    }
    free(block->vert);
    free(block->normal);
    free(block->color);
    for (int i = 0; i < MAX_TEX; i++) {
        free(block->tex[i]);
    }
    free(block->indices);
    free(block);
}

static inline void bl_grow(block_t *block) {
    if (! block->vert) {
        block->vert = alloc_sublist(3, GL_FLOAT, block->cap);
    }
    if (block->len >= block->cap) {
        block->cap += DEFAULT_BLOCK_CAPACITY;
        // TODO: store list types on block and use block->types.vert, etc directly?
        realloc_sublist(block->vert, 3, GL_FLOAT, block->cap);
        realloc_sublist(block->normal, 3, GL_FLOAT, block->cap);
        realloc_sublist(block->color, 4, GL_FLOAT, block->cap);
        for (int i = 0; i < MAX_TEX; i++) {
            realloc_sublist(block->tex[i], 2, GL_FLOAT, block->cap);
        }
    }
}

void bl_q2t(block_t *block) {
    if (!block->len || !block->vert || block->q2t) return;
    // TODO: split to multiple blocks if block->len > 65535

    q2t_calc(block->len);
    block->q2t = true;
    block->count = block->len * 1.5;
    if (block->indices) {
        GLushort *indices = malloc(block->count * sizeof(GLushort));
        for (int i = 0; i < block->count; i++) {
            indices[i] = block->indices[q2t.cache[i]];
        }
        free(block->indices);
        block->indices = indices;
    }
    return;
}

// affects global state based on the ending state of a block
// (used when a display list executes or a global block ends)
void bl_pollute(block_t *block) {
    // artificial (non-glBegin) blocks don't pollute global state
    // and empty blocks would cause undefined memcpy (negative index)
    if (!block || block->len <= 0 || block->artificial) {
        return;
    }
    int last = (block->len - 1);
    for (int i = 0; i < MAX_TEX; i++) {
        if (block->tex[i]) {
            glMultiTexCoord2fv(GL_TEXTURE0 + i, block->tex[i] + (2 * last));
        }
    }
    if (block->color) {
        glColor4fv(block->color + (4 * last));
    }
    if (block->normal) {
        glNormal3fv(block->normal + (3 * last));
    }
}

void bl_end(block_t *block) {
    if (! block->open)
        return;

    // only call this here if we're not compiling
    if (! state.list.active) {
        bl_pollute(block);
    }

    block->open = false;
    for (int i = 0; i < MAX_TEX; i++) {
        gltexture_t *bound = state.texture.bound[i];
        if (block->tex[i] && bound) {
            if (bound->width != bound->nwidth || bound->height != bound->nheight) {
                tex_coord_npot(block->tex[i], block->len, bound->width, bound->height, bound->nwidth, bound->nheight);
            }
            // GL_ARB_texture_rectangle
            if (state.texture.rect_arb[i]) {
                tex_coord_rect_arb(block->tex[i], block->len, bound->width, bound->height);
            }
        }
    }
    switch (block->mode) {
        case GL_QUADS:
            block->mode = GL_TRIANGLES;
            bl_q2t(block);
            break;
        case GL_POLYGON:
            block->mode = GL_TRIANGLE_FAN;
            break;
        case GL_QUAD_STRIP:
            block->mode = GL_TRIANGLE_STRIP;
            break;
    }
}

void bl_draw(block_t *block) {
    if (! block || block->len == 0) {
        return;
    }

    int pos;
    // texture is never incomplete, because we check enabled state on start?
    // TODO: the texture array could exist but be incomplete :(
    for (int i = 0; i < MAX_TEX; i++) {
        if ((pos = block->incomplete.tex[i]) >= 0) {
            for (int j = 0; j < block->len; j++) {
                memcpy(block->tex[i] + (2 * j), CURRENT->tex[i], 2 * sizeof(GLfloat));
            }
        }
    }
    if ((pos = block->incomplete.color) >= 0) {
        for (int i = 0; i < pos; i++) {
            memcpy(block->color + (4 * i), CURRENT->color, 4 * sizeof(GLfloat));
        }
    }
    if ((pos = block->incomplete.normal) >= 0) {
        for (int i = 0; i < pos; i++) {
            memcpy(block->normal + (3 * i), CURRENT->normal, 3 * sizeof(GLfloat));
        }
    }
    if (state.render.mode == GL_SELECT) {
        return gl_select_block(block);
    } else if (state.render.mode == GL_FEEDBACK) {
        return gl_feedback_block(block);
    }

    // glTexGen
    for (int i = 0; i < MAX_TEX; i++) {
        if (state.enable.texgen_s[i] || state.enable.texgen_t[i]) {
            gen_tex_coords(block, i);
        }
    }

    // copy vertex data for local matrix calculations
    GLfloat *vert, *tex[MAX_TEX] = {0};
#ifdef LOCAL_MATRIX
    vert = malloc(block->len * 3 * sizeof(GLfloat));
    for (int i = 0; i < block->len; i++) {
        gl_transform_vertex(&vert[i * 3], &block->vert[i * 3]);
    }
    for (int t = 0; t < MAX_TEX; t++) {
        if (block->tex[t]) {
            tex[t] = malloc(block->len * 2 * sizeof(GLfloat));
            for (int i = 0; i < block->len; i++) {
                gl_transform_texture(GL_TEXTURE0 + t, &tex[t][i * 2], &block->tex[t][i * 2]);
            }
        }
    }
#else
    vert = block->vert;
    for (int i = 0; i < MAX_TEX; i++) {
        tex[i] = block->tex[i];
    }
#endif
    if (state.remote) {
#ifdef LOCAL_MATRIX
        void *tmp = malloc(sizeof(block_t));
        memcpy(tmp, block, sizeof(block_t));
        tmp->vert = vert;
        for (int i = 0; i < MAX_TEX; i++) {
            tmp->tex[i] = tex[i];
        }
        remote_block_draw(tmp);
        free(vert);
        for (int i = 0; i < MAX_TEX; i++) {
            free(tex[i]);
        }
#else
        remote_block_draw(block);
#endif
        return;
    }

    LOAD_GLES(glDrawArrays);
    LOAD_GLES(glDrawElements);

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
#ifdef USE_ES2
    if (block->vert) {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, block->vert);
    }
    gles_glDrawArrays(block->mode, 0, block->len);
#else
    if (vert) {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vert);
    } else {
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    if (block->normal) {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, block->normal);
    } else {
        glDisableClientState(GL_NORMAL_ARRAY);
    }

    if (block->color) {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_FLOAT, 0, block->color);
    } else {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    bool stipple = false;
    // TODO: how do I stipple with texture?
    // TODO: what about multitexturing?
    if (! tex[0]) {
        // TODO: do we need to support GL_LINE_STRIP?
        if (block->mode == GL_LINES && state.enable.line_stipple) {
            stipple = true;
            glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT);
            glEnable(GL_BLEND);
            glEnable(GL_TEXTURE_2D);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            tex[0] = gen_stipple_tex_coords(vert, block->len);
            bind_stipple_tex();
        }
    }
    for (int i = 0; i < MAX_TEX; i++) {
        GLuint old = state.texture.client + GL_TEXTURE0;
        if (tex[i]) {
            glClientActiveTexture(GL_TEXTURE0 + i);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, 0, tex[i]);
            glClientActiveTexture(old);
        } else if (state.enable.tex_coord_array[i]) {
            glClientActiveTexture(GL_TEXTURE0 + i);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glClientActiveTexture(old);
        }
    }

    GLushort *indices = block->indices;
    // if glDrawElements or glArrayElement was used, we should have already updated block->indices with q2t
    if (block->q2t && !indices) {
        // make sure we resized q2t. this block might be from remote.
        q2t_calc(block->len);
        indices = q2t.cache;
    }

    if (indices) {
        gles_glDrawElements(block->mode, block->count, GL_UNSIGNED_SHORT, indices);
    } else {
        gles_glDrawArrays(block->mode, 0, block->len);
    }
    if (stipple) {
        glPopAttrib();
        free(tex[0]);
    }
#endif
    glPopClientAttrib();
#ifdef LOCAL_MATRIX
    free(vert);
    for (int i = 0; i < MAX_TEX; i++) {
        free(tex[i]);
    }
#endif
}

void bl_vertex3f(block_t *block, GLfloat x, GLfloat y, GLfloat z) {
    bl_grow(block);

    if (block->normal) {
        GLfloat *normal = block->normal + (block->len * 3);
        memcpy(normal, CURRENT->normal, sizeof(GLfloat) * 3);
    }

    if (block->color) {
        GLfloat *color = block->color + (block->len * 4);
        memcpy(color, CURRENT->color, sizeof(GLfloat) * 4);
    }

    for (int i = 0; i < MAX_TEX; i++) {
        if (block->tex[i]) {
            GLfloat *tex = block->tex[i] + (block->len * 2);
            memcpy(tex, CURRENT->tex[i], sizeof(GLfloat) * 2);
        }
    }

    GLfloat *vert = block->vert + (block->len++ * 3);
    vert[0] = x;
    vert[1] = y;
    vert[2] = z;
}

void bl_track_color(block_t *block) {
    if (! block->color) {
        block->color = alloc_sublist(4, GL_FLOAT, block->cap);
        if (state.list.active) {
            block->incomplete.color = block->len - 1;
        } else {
            for (int i = 0; i < block->len; i++) {
                memcpy(block->color + (4 * i), CURRENT->color, 4 * sizeof(GLfloat));
            }
        }
    }
}

void bl_track_normal(block_t *block) {
    if (! block->normal) {
        block->normal = alloc_sublist(3, GL_FLOAT, block->cap);
        if (state.list.active) {
            block->incomplete.normal = block->len - 1;
        } else if (! block->normal) {
            for (int i = 0; i < block->len; i++) {
                memcpy(block->normal + (3 * i), CURRENT->normal, 3 * sizeof(GLfloat));
            }
        }
    }
}

void bl_track_tex(block_t *block, GLenum target) {
    target -= GL_TEXTURE0;
    if (! block->tex[target]) {
        block->tex[target] = alloc_sublist(2, GL_FLOAT, block->cap);
        if (state.list.active) {
            block->incomplete.tex[target] = block->len - 1;
        } else {
            for (int j = 0; j < block->len; j++) {
                memcpy(block->tex[target] + (2 * j), CURRENT->tex[target], 2 * sizeof(GLfloat));
            }
        }
    }
}
