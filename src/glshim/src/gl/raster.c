#include "error.h"
#include "loader.h"
#include "pixel.h"
#include "raster.h"
#include "texture.h"
#include "matrix.h"
#include "remote.h"

/* raster engine:
    we render pixels to memory somewhere
    until someone else wants to use the framebuffer
    then we throw 'em quickly into a texture, render to the whole screen
    then let the other function do their thing
*/

static void update_viewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    viewport_state_t *vs = &state.viewport;
    vs->x = x, vs->y = y;
    vs->width = width, vs->height = height;
    vs->nwidth = npot(width);
    vs->nheight = npot(height);
}

static void init_raster() {
    viewport_state_t *vp = &state.viewport;
    if (!vp->width || !vp->height) {
        GLint tmp[4];
        glGetIntegerv(GL_VIEWPORT, tmp);
        update_viewport(tmp[0], tmp[1], tmp[2], tmp[3]);
    }
    if (! state.raster.buf) {
        state.raster.buf = (GLubyte *)malloc(4 * vp->nwidth * vp->nheight * sizeof(GLubyte));
    }
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    PUSH_IF_COMPILING(glViewport);
    if (state.raster.buf) {
        render_raster();
    }
    if (width < 0 || height < 0) {
        ERROR(GL_INVALID_VALUE);
    }
    update_viewport(x, y, width, height);
    LOAD_GLES(glViewport);
    gles_glViewport(x, y, width, height);
}

void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z) {
    ERROR_IN_BLOCK();
    PUSH_IF_COMPILING(glRasterPos3f);
    PROXY_GLES(glRasterPos3f);
    GLfloat v[3] = {x, y, z};
    gl_transform_vertex(v, v);
    init_raster();
    viewport_state_t *vs = &state.viewport;
    v[0] = (((v[0] + 1.0f) * 0.5f) * vs->width) + vs->x;
    v[1] = (((-v[1] + 1.0f) * 0.5f) * vs->height) + vs->y;
    // TODO: deal with Z
    glWindowPos3f(v[0], v[1], v[2]);
}

void glWindowPos3f(GLfloat x, GLfloat y, GLfloat z) {
    ERROR_IN_BLOCK();
    PUSH_IF_COMPILING(glWindowPos3f);
    PROXY_GLES(glWindowPos3f);
    raster_state_t *raster = &state.raster;
    raster->pos.x = x;
    raster->pos.y = y;
    raster->pos.z = z;
    init_raster();
    viewport_state_t *v = &state.viewport;
    if (x < v->x || x >= v->width || y < v->y || y >= v->height) {
        raster->valid = 0;
    } else {
        raster->valid = 1;
    }
    GLuint *dst = NULL;
    GLfloat *color = raster->color;
    if (pixel_convert(CURRENT->color, (GLvoid **)&dst, 1, 1, GL_RGBA, GL_FLOAT, GL_RGBA, GL_UNSIGNED_BYTE)) {
        memcpy(color, CURRENT->color, sizeof(GLfloat) * 4);
        raster->pixel = *dst;
        free(dst);
    } else {
        for (int i = 0; i < 4; i++) {
            color[i] = 1.0f;
        }
        raster->pixel = 0xFFFFFFFF;
    }
}

void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig,
              GLfloat xmove, GLfloat ymove, const GLubyte *bitmap) {
    // TODO: support xorig/yorig
    PUSH_IF_COMPILING(glBitmap);
    PROXY_GLES(glBitmap);
    raster_state_t *raster = &state.raster;
    struct { GLfloat x, y, z, w; } *pos = (void *)&raster->pos;
    if (! raster->valid) {
        return;
    }
    // TODO: negative width/height mirrors bitmap?
    if (!width && !height) {
        pos->x += xmove;
        pos->y += ymove;
        return;
    }
    init_raster();

    const GLubyte *from;
    GLuint *to;
    int x, y;

    // copy to pixel data
    // TODO: strip blank lines and mirror vertically?
    for (y = 0; y < height; y++) {
        float dy = pos->y - y;
        to = (GLuint *)raster->buf + (GLuint)(pos->x + (dy * state.viewport.nwidth));
        from = bitmap + (y * 2);
        for (x = 0; x < width; x += 8) {
            float dx = pos->x + x;
            if (dx < 0 || dx > state.viewport.width ||
                dy < 0 || dy > state.viewport.height)
                continue;
            int max = 8;
            if (dx + 8 > state.viewport.width)
                max = state.viewport.width - dx;

            GLubyte b = *from++;
            for (int j = max - 1; j >= 0; j--) {
                *to++ = (b & (1 << j)) ? raster->pixel : 0;
            }
        }
    }

    pos->x += xmove;
    pos->y += ymove;
}

void glDrawPixels(GLsizei width, GLsizei height, GLenum format,
                  GLenum type, const GLvoid *data) {
    const GLubyte *from, *pixels = data;
    PUSH_IF_COMPILING(glDrawPixels);
    PROXY_GLES(glDrawPixels);
    raster_state_t *raster = &state.raster;
    if (! raster->valid) {
        return;
    }
    GLubyte *to;
    GLvoid *dst = NULL;

    init_raster();
    if (! pixel_convert(data, &dst, width, height,
                        format, type, GL_RGBA, GL_UNSIGNED_BYTE)) {
        return;
    }
    pixels = (GLubyte *)dst;

    // shrink our pixel ranges to stay inside the viewport
    int ystart = MAX(0, -raster->pos.y);
    height = MIN(state.viewport.height - raster->pos.y, height);

    int xstart = MAX(0, -raster->pos.x);
    int screen_width = MIN(state.viewport.width - raster->pos.x, width);

    for (int y = ystart; y < height; y++) {
        to = raster->buf + 4 * (GLuint)(raster->pos.x + ((raster->pos.y + y) * state.viewport.nwidth));
        from = pixels + 4 * (xstart + y * width);
        memcpy(to, from, 4 * screen_width);
    }
    if (pixels != data)
        free((void *)pixels);
}

void render_raster() {
    if (!state.viewport.width || !state.viewport.height || !state.raster.buf || state.remote)
        return;

// FIXME
#ifndef USE_ES2
    glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT | GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    GLfloat vert[] = {
        -1, -1, 0,
        1, -1, 0,
        1, 1, 0,
        -1, 1, 0,
    };

    float sw = state.viewport.width / (GLfloat)state.viewport.nwidth;
    float sh = state.viewport.height / (GLfloat)state.viewport.nheight;

    GLfloat tex[] = {
        0, sh,
        sw, sh,
        sw, 0,
        0, 0,
    };

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT | GL_CLIENT_PIXEL_STORE_BIT);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vert);
    glTexCoordPointer(2, GL_FLOAT, 0, tex);

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state.viewport.nwidth, state.viewport.nheight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, state.raster.buf);

    LOAD_GLES(glDrawArrays);
    gles_glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDeleteTextures(1, &texture);

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopClientAttrib();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
#endif
    free(state.raster.buf);
    state.raster.buf = NULL;
}
